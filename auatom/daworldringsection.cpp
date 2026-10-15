//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldringsection.cpp,v 1.598 2022/05/17 18:44:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daworldringsection.h"
//
#include "auatomrenderinfo.h"
#include "aucityreport.h"
#include "aucombat.h"
#include "aufilever.h"
#include "auglmodelmaker.h"
#include "aunoise.h"
#include "auquad.h"
#include "auscdesign.h"
#include "ausensorblip.h"
#include "auterraininfo.h"
#include "auvillage.h"
#include "daanimal.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "daeffect.h"
#include "daitem.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dateleporter.h"
#include "davehicle.h"
#include "daworldring.h"
#include "defauna.h"
#include "deflora.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dprock.h"
#include "hashpack.h"
#include <QSqlError>
#include <QSqlQuery>

#define GRASS_DETAIL_RANGE				(50*FeetPerMeter())		// Range at which grass switches to low poly model.
#define SHRUB_DETAIL_RANGE				(75*FeetPerMeter())		// Range at which shrub switches to low poly model.
#define TERRAIN_TEXTURE_REPS			30
#define TREE_DETAIL_RANGE				(100*FeetPerMeter())	// Range at which tree switches to low poly model.
#define LON_STRETCH_FACTOR				13						// X size multiple of Y size when calculating MinimumSectionLength.
																// Controls number of sections in a ringworld.
																// 13 = 5 @ .25au, 10 @ .5au, 15 @ .75au, and 20 @ 1au radius.
																// 15 = 5 @ .25au, 9 @ .5au, 13 @ .75au, and 18 @ 1au radius.
																// 16 = 5 @ .25au, 9 @ .5au, 13 @ .75au, and 17 @ 1au radius.

// These textures are used by all cities so they can be statically allocated.
static GLuint s_FlagEmpireTex = 0;		// Texture id of flag icon for top-down view.
static GLuint s_FlagEnemyTex = 0;		// Texture id of flag icon for top-down view.
static GLuint s_FlagFriendTex = 0;		// Texture id of flag icon for top-down view.
static GLuint s_FlagNeutralTex = 0;		// Texture id of flag icon for top-down view.
static const float s_seaHardness = 150.f;	// Larger number makes a smaller smaller spot
static GLuint s_TexNoise = 0;			// Random noise texture for more variations
static GLuint s_TexObelisk = 0;			// Obelisk texture.
static GLuint s_TexRingworld = 0;		// Ringworld hull and shadow square texture.
static GLuint s_TexRock = 0;			// Texture used for all developments for rocks.
static GLuint s_texSea = 0;				// Sea surface texture.
static GLuint s_texSeaN = 0;			// Sea surface bump map.
static GLuint 							s_TexTerPalette[WORLDTEX_Count];	// Terrain texture tiles.

DaWorldRingSection::DaWorldRingSection()
{
	//m_AtmColor constructor sets it to 'invalid' state
	m_byAtmDensity = 0;
	m_AtmType = ATMTYPE_Standard;
	m_byWater = 0;
	m_ControlCenterActive = false;
	m_FloraVAOsHigh = 0;
	m_HengeColumn = 0;
	m_HengeFirePit = 0;
	m_HengeRock = 0;
	m_pSentient = 0;
	m_pFauna = NULL;
	m_pFlora = NULL;
	m_Population = (quint32)-1;
	m_Resources = 0;
	m_RockVAOs = 0;
	m_RotationAngle = 0.0;
	m_RotationAngleOffset = 0;
	m_SectionAngle = 0;
	m_SectionIndex = 0;
	m_Surface = 0;
	m_TerrainFrequency = 0;
	m_TerrainPersFactor = 0;
	m_TerrainPersistence = 0;
	m_TerrainXOff = 0;
	m_TerrainYOff = 0;
	m_TerrainWaterFactor = 0;
	m_ThinkRotation = 0;
	m_WaterOffX = 0;
	m_WaterOffY = 0;
	m_WindDir = 1000;	// Starting value is undetermined.
	m_WindForce = -1;	// Starting value is undetermined.
	m_glTexWx = 0;
	m_ViewerNear = false;
	m_WxMaxSP = 0;
	m_WxMaxVP = 0;
	m_WxMinSP = 0;
	m_WxMinVP = 0;
	m_WxSPOffset = 0;
	m_WxVPOffset = 0;
	m_glTexFarSurface = 0;
	m_glTexGrass = 0;
	m_glTexNearSurface = 0;
	m_glTexNearValid = false;
}

DaWorldRingSection::~DaWorldRingSection()
{
	/*
#if !defined(QT_NO_DEBUG)
	if(m_Atmosphere.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - Atmosphere =" << m_Atmosphere.VAO();
	if(m_Clouds.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - Clouds =" << m_Clouds.VAO();
	if(m_NearGlobe.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - NearGlobe =" << m_NearGlobe.VAO();
	if(m_AtGlobe.m_Low.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - AtGlobe Low =" << m_AtGlobe.m_Low.VAO();
	for(unordered_map<int, AuGLTriVAO, AuTriHash>::const_iterator tvi = m_AtGlobe.m_TriVAOs.begin();
			tvi != m_AtGlobe.m_TriVAOs.end(); tvi++)
	{
		if((*tvi).second.VAO())
			qDebug() << "DaWorldRingSection::~DaWorldRingSection - AtGlobe High" << (*tvi).first << "=" << (*tvi).second.VAO();
	}
	if(m_SeaGlobe.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - SeaGlobe =" << m_SeaGlobe.VAO();
	if(m_UnderseaGlobe.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - UnderseaGlobe =" << m_UnderseaGlobe.VAO();
	if(m_Hull.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - Hull =" << m_Hull.VAO();
	if(m_Obelisk.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - Obelisk =" << m_Obelisk.VAO();
	if(m_ShadowSquare.VAO())
		qDebug() << "DaWorldRingSection::~DaWorldRingSection - Shadow Square =" << m_ShadowSquare.VAO();
#endif
*/
	if(m_FloraVAOsHigh)
	{
		// We must delete the individually allocated vao objects in this array.
		int maxFlora = PLANT_VAOS * AuFloraDNA::Zones * (1 + ZONE_TREES + ZONE_SHRUBS) * ResourceZones();
		AuGLVAO** pvao = m_FloraVAOsHigh;
		for(int flora=0; flora<maxFlora; flora++, pvao++)
			if(*pvao)
				delete *pvao;

		// Now we can delete the array itself.
		delete [] m_FloraVAOsHigh;
	}

	if(m_RockVAOs)
	{
		// We must delete the individually allocated vao objects in this array.
		int maxRocks = DpRock::RockTypes();
		AuGLVAO** pvao = m_RockVAOs;
		for(int rock=0; rock<maxRocks; rock++, pvao++)
			if(*pvao)
				delete *pvao;

		// Now we can delete the array itself.
		delete [] m_RockVAOs;
	}

	if(m_pFauna)
	{
		delete [] m_pFauna;
		m_pFauna = 0;
	}

	if(m_pFlora)
	{
		delete [] m_pFlora;
		m_pFlora = 0;
	}

	if(m_Surface && m_Surface != (QImage*)-1)
		delete m_Surface;
	m_Surface = 0;

	if(m_pSentient)
		delete m_pSentient;

	if(m_Resources)
	{
		delete [] m_Resources;
		m_Resources = 0;
	}
}

DaWorldRingSection::DaWorldRingSection(const DaWorldRingSection &rhs) : DaWorldLL(rhs)
{
	//qDebug("DaWorldRingSection:: copy constructor id %d", rhs.Id().GetID());
	m_byAtmDensity = rhs.m_byAtmDensity;
	m_AtmType = rhs.m_AtmType;
	m_byWater = rhs.m_byWater;
	m_ControlCenterActive = rhs.m_ControlCenterActive;
	m_SectionIndex = rhs.m_SectionIndex;

	rhs.m_RingSectionLock.lockForRead();
	m_AtmColor = rhs.m_AtmColor;
	m_Population = rhs.m_Population;
	if(rhs.m_pSentient)
		m_pSentient = new AuSpecies(*rhs.m_pSentient);
	else
		m_pSentient = 0;
	m_RotationAngle = rhs.m_RotationAngle;
	m_RotationAngleOffset = rhs.m_RotationAngleOffset;
	m_SectionAngle = rhs.m_SectionAngle;
	if(rhs.m_pFauna)
	{
		m_pFauna = new DePlanetFauna[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFauna[rz] = rhs.m_pFauna[rz];
	}
	else
		m_pFauna = 0;
	if(rhs.m_pFlora)
	{
		m_pFlora = new DePlanetFlora[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz] = rhs.m_pFlora[rz];
	}
	else
		m_pFlora = 0;
	m_tmFloraChange = rhs.m_tmFloraChange;

	m_TerrainFrequency = rhs.m_TerrainFrequency;
	m_TerrainPersFactor = rhs.m_TerrainPersFactor;
	m_TerrainPersistence = rhs.m_TerrainPersistence;
	m_TerrainXOff = rhs.m_TerrainXOff;
	m_TerrainYOff = rhs.m_TerrainYOff;
	m_TerrainWaterFactor = rhs.m_TerrainWaterFactor;
	rhs.m_RingSectionLock.unlock();

	m_Atmosphere = AuGLVAO();
	m_Clouds = AuGLVAO();
	m_FloraVAOsHigh = 0;
	m_Hull = AuGLVAO();
	m_NearGlobe = AuGLVAO();
	m_Obelisk = AuGLVAO();
	m_RockVAOs = 0;
	m_SeaGlobe = AuGLVAO();
	m_ShadowSquare = AuGLVAO();
	m_UnderseaGlobe = AuGLVAO();

	rhs.m_ResourcesLock.lockForRead();
	if(rhs.m_Resources)
	{
		m_Resources = new AuZoneResource[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_Resources[rz] = rhs.m_Resources[rz];
	}
	else
		m_Resources = 0;
	rhs.m_ResourcesLock.unlock();




	if(rhs.m_Surface && rhs.m_Surface != (QImage*)-1)
		m_Surface = new QImage(*rhs.m_Surface);
	else
		m_Surface = 0;

	m_WaterOffX = rhs.m_WaterOffX;
	m_WaterOffY = rhs.m_WaterOffY;
	m_WindDir = rhs.m_WindDir;
	m_WindForce = rhs.m_WindForce;
	m_Wx = rhs.m_Wx;
	m_glTexWx = 0;
	m_ViewerNear = false;
	m_WxMaxSP = rhs.m_WxMaxSP;
	m_WxMaxVP = rhs.m_WxMaxVP;
	m_WxMinSP = rhs.m_WxMinSP;
	m_WxMinVP = rhs.m_WxMinVP;
	m_WxSPOffset = rhs.m_WxSPOffset;
	m_WxVPOffset = rhs.m_WxVPOffset;
	m_glTexFarSurface = 0;
	m_glTexGrass = 0;
	m_glTexNearSurface = 0;
	m_glTexNearValid = false;
	m_HengeColumn = 0;
	m_HengeFirePit = 0;
	m_HengeRock = 0;

	// Do not copy Think variables.
	m_ThinkRotation = 0;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaWorldRingSection::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaWorldLL::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint16 hasWhat = 0;
	if(lastTx.IsNull())
		hasWhat |= 0x0001;
	if(lastTx.IsNull() || GeologyChangeTime() >= lastTx)	// This includes activation of henges.
		hasWhat |= 0x0003;	// Include terrain info too. This doesn't happen much.

	// Client does not receive the base here bit, doesn't need it.
	if(BdHere())
		hasWhat |= 0x0080;
	if(HQHere())
		hasWhat |= 0x0100;

	if(m_ControlCenterActive)
		hasWhat |= 0x0010;

	m_RingSectionLock.lockForRead();
	if(m_pSentient)
		hasWhat |= 0x0008;

	if(tmLastUpdate.IsNull() || (!m_tmFloraChange.IsNull() && m_tmFloraChange >= tmLastUpdate))
		hasWhat |= 0x0020;

	m_RingSectionLock.unlock();

	m_LLLock.lockForRead();
	if(tmLastUpdate.IsNull() || (!m_ClaimTime.IsNull() && m_ClaimTime >= tmLastUpdate))
		hasWhat |= 0x0040;
	m_LLLock.unlock();

	toClient << hasWhat;

	if(hasWhat & 0x0001)
	{
		m_RingSectionLock.lockForRead();
		toClient << (quint8)m_AtmType;
		toClient << m_byAtmDensity;
		toClient << m_byWater;
		toClient << m_SectionIndex;
		toClient << m_TerrainFrequency;
		toClient << m_TerrainPersFactor;
		toClient << m_TerrainPersistence;
		toClient << m_TerrainXOff;
		toClient << m_TerrainYOff;
		toClient << m_TerrainWaterFactor;
		m_RingSectionLock.unlock();
	}

	if(hasWhat & 0x0002)
	{
		toClient << LockGeologicalFeatures();
		UnlockGeologicalFeatures();
	}

	if(hasWhat & 0x0008)
	{
		m_RingSectionLock.lockForRead();
		toClient << *m_pSentient;
		m_RingSectionLock.unlock();
	}

	if(hasWhat & 0x0020)
	{
		// This allows for farm and orchard plants in harsh environments.
		m_RingSectionLock.lockForRead();
		Q_ASSERT(m_pFlora);
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz].TxUpdate(toClient);
		m_tmFloraChange >> toClient;
		m_RingSectionLock.unlock();
	}

	if(hasWhat & 0x0040)
	{
		m_LLLock.lockForRead();
		toClient << m_ClaimAtLL;
		toClient << m_ClaimEmpire;
		m_LLLock.unlock();
	}
}

// Receive updates to this atom from a server.
void DaWorldRingSection::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaWorldLL::RxUpdate(fromServer, isFirstUpdate);

	bool dirtyIrrigCache = false;
	quint16 hasWhat;
	fromServer >> hasWhat;
	if(hasWhat & 0x0001)
	{
		quint8 ui8;
		fromServer >> ui8;
		m_AtmType = (ATMTYPE)ui8;
		fromServer >> m_byAtmDensity;
		fromServer >> m_byWater;
		fromServer >> m_SectionIndex;
		fromServer >> m_TerrainFrequency;
		fromServer >> m_TerrainPersFactor;
		fromServer >> m_TerrainPersistence;
		fromServer >> m_TerrainXOff;
		fromServer >> m_TerrainYOff;
		fromServer >> m_TerrainWaterFactor;
	}

	if(hasWhat & 0x0002)
	{
		//qDebug() << "DaWorldRingSection::RxUpdate" << Id() << Name() << "geological features";
		fromServer >> LockGeologicalFeaturesForWrite();
		UnlockGeologicalFeatures();
		InvalidateHengePresence();
		//DumpGeo();
		dirtyIrrigCache = true;
	}

	if(hasWhat & 0x0008)
	{
		if(!m_pSentient)
			m_pSentient = new AuSpecies;
		fromServer >> *m_pSentient;
	}

	m_ControlCenterActive = (hasWhat & 0x0010) ? true : false;

	if(hasWhat & 0x0020)
	{
		// This allows for farm and orchard plants in harsh environments.
		if(!m_pFlora)
			m_pFlora = new DePlanetFlora[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz].RxUpdate(fromServer);
		m_tmFloraChange << fromServer;
	}

	if(hasWhat & 0x0040)
	{
		m_LLLock.lockForWrite();
		fromServer >> m_ClaimAtLL;
		fromServer >> m_ClaimEmpire;
		m_LLLock.unlock();
	}

	// Client does not receive the base here bit, doesn't need it.
	m_BdBits = 0;
	if(hasWhat & 0x0080)
		m_BdBits |= 0x04;
	if(hasWhat & 0x0100)
		m_BdBits |= 0x10;

	if(dirtyIrrigCache)
		UpdateIrrigationCache();
}

// Refresh data from valid counterpart.
bool DaWorldRingSection::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaWorldLL::Refresh(pAtom, ari);

	const DaWorldRingSection* pA = (const DaWorldRingSection*)pAtom;
	m_byAtmDensity = pA->m_byAtmDensity;
	m_AtmType = pA->m_AtmType;
	m_byWater = pA->m_byWater;
	m_BdBits = pA->m_BdBits;
	m_ControlCenterActive = pA->m_ControlCenterActive;
	m_SectionIndex = pA->m_SectionIndex;

	pA->m_RingSectionLock.lockForRead();
	m_RingSectionLock.lockForWrite();

	m_TerrainFrequency = pA->m_TerrainFrequency;
	m_TerrainPersFactor = pA->m_TerrainPersFactor;
	m_TerrainPersistence = pA->m_TerrainPersistence;
	m_TerrainXOff = pA->m_TerrainXOff;
	m_TerrainYOff = pA->m_TerrainYOff;
	m_TerrainWaterFactor = pA->m_TerrainWaterFactor;
	//m_Resources = pA->m_Resources;

	// Let's try following the same logic the vegetation uses.
	// If this atom has DNA data and the refresh atom has DNA
	// data then update the DNA data.  Otherwise leave it alone.
	if(m_pSentient && pA->m_pSentient)
		*m_pSentient = *pA->m_pSentient;
	else if(!m_pSentient && pA->m_pSentient)
		m_pSentient = new AuSpecies(*pA->m_pSentient);
	else if(m_pSentient && !pA->m_pSentient)
	{
		delete m_pSentient;
		m_pSentient = 0;
	}

	// m_pFlora data
	if(m_tmFloraChange != pA->m_tmFloraChange)
	{
		// Delete flora device objects before the flora time changes; it's part of the cache file name.
		if(ari)
			DeleteFloraDeviceObjects(*ari->re, ari->mm);

		if(!m_pFlora)
			m_pFlora = new DePlanetFlora[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz] = pA->m_pFlora[rz];

		m_tmFloraChange = pA->m_tmFloraChange;
	}

	m_RingSectionLock.unlock();
	pA->m_RingSectionLock.unlock();

	// We do not refresh animal encounter tables due to the volume of data. The client
	// doesn't need the info anyway. The animal encounter tables are only used by the
	// server. If an animal appears, it will show up in the scene.

	return false;
}

DaAtom* DaWorldRingSection::Clone() const
{
	return new DaWorldRingSection(*this);
}

void DaWorldRingSection::CreateFirstRingworldSection(const AuDice& Cdice, double orbitRadiusAu, ORBITZONE orbitZone)
{
	AuDice dice(Cdice);
	Q_ASSERT(orbitZone == OZONE_Habitable);
	DaWorldLL::Create(orbitRadiusAu, orbitZone);
	//qDebug() << "DaWorldRingSection::CreateFirstRingworldSection" << *this;

	m_SectionIndex = 0;

	// Determine ringworld atmosphere density: 25-75
	int iroll;
	do
	{
		m_byAtmDensity = dice.Roll(2, 6);	// Use the same atm density roll as an Earth standard world.
		// Generate a ones digit also after the multiply.
		m_byAtmDensity = (m_byAtmDensity*10) + dice.Roll(1, 10, -1);
	}
	while(m_byAtmDensity < 25 || m_byAtmDensity > 75);

	// Determine ringworld atmosphere type.
	// A habitable world has Standard or Tainted atmosphere.
	iroll = dice.Roll(2, 6);
	if(iroll <= 8)
		m_AtmType = ATMTYPE_Standard;
	else
		m_AtmType = ATMTYPE_Tainted;

	// Calculate water percentage. 25 to 75
	do
	{
		m_byWater = dice.Roll(2, 6, 1);
		if(m_byAtmDensity <= 20)
			m_byWater -= 4;
		// Generate a ones digit also after the multiply.
		m_byWater = (m_byWater*10) + dice.Roll(1, 10, -1);
	}
	while(m_byWater < 25 || m_byWater > 75);
	if(g_Au.m_LogMessages & LOG_AtomCreate)
		qDebug("DaWorldRingSection::CreateFirstRingworldSection %d Ringworld water %d atmosphere %d", Id().GetID(), m_byWater,
				m_byAtmDensity);

	CreateTerrainData(dice);			// Create the 3D terrain data.
	dice.Random();
	CreateGeologicalFeatures(dice);
	dice.Random();

	// All bodies that could have cities on them must have plant DNA so the farms and orchards can make plants.
	m_RingSectionLock.lockForWrite();
	if(!m_pFlora)
		m_pFlora = new DePlanetFlora[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFlora[rz].InitRingworld(dice);
		dice.Random();
	}
	m_RingSectionLock.unlock();

	// Create animal encounter tables.
	if(CanHaveFauna())
		InitAnimalEncounterTable(dice);
}

void DaWorldRingSection::InitAnimalEncounterTable(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	ORBITZONE orbitZone = OrbitZone();
	quint8 atmDensity = AtmosphereDensity();
	quint8 waterPercent = Hydrographics();

	m_RingSectionLock.lockForWrite();
	if(m_pFauna)
		delete [] m_pFauna;
	m_pFauna = new DePlanetFauna[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFauna[rz].InitRingworld(orbitZone, atmDensity, waterPercent, dice);
		dice.Random();
	}
	m_RingSectionLock.unlock();
}

void DaWorldRingSection::CreateFirstRingworldSectionForScan(const AuDice& Cdice, double orbitRadiusAu, ORBITZONE orbitZone)
{
	AuDice dice(Cdice);
	Q_ASSERT(orbitZone == OZONE_Habitable);
	DaWorldLL::CreateForScan(orbitRadiusAu, orbitZone);

	m_SectionIndex = 0;

	// Determine ringworld atmosphere density: 25-75
	int iroll;
	do
	{
		m_byAtmDensity = dice.Roll(2, 6);	// Use the same atm density roll as an Earth standard world.
		// Generate a ones digit also after the multiply.
		m_byAtmDensity = (m_byAtmDensity*10) + dice.Roll(1, 10, -1);
	}
	while(m_byAtmDensity < 25 || m_byAtmDensity > 75);

	// Determine ringworld atmosphere type.
	// A habitable world has Standard or Tainted atmosphere.
	iroll = dice.Roll(2, 6);
	if(iroll <= 8)
		m_AtmType = ATMTYPE_Standard;
	else
		m_AtmType = ATMTYPE_Tainted;

	// Calculate water percentage. 25 to 75
	do
	{
		m_byWater = dice.Roll(2, 6, 1);
		if(m_byAtmDensity <= 20)
			m_byWater -= 4;
		// Generate a ones digit also after the multiply.
		m_byWater = (m_byWater*10) + dice.Roll(1, 10, -1);
	}
	while(m_byWater < 25 || m_byWater > 75);
	if(g_Au.m_LogMessages & LOG_AtomCreate)
		qDebug("DaWorldRingSection::CreateFirstRingworldSection %d Ringworld water %d atmosphere %d", Id().GetID(), m_byWater,
				m_byAtmDensity);

	CreateTerrainData(dice);			// Create the 3D terrain data.
	dice.Random();
	CreateGeologicalFeatures(dice);
	dice.Random();

	// All bodies that could have cities on them must have plant DNA so the farms and orchards can make plants.
	m_RingSectionLock.lockForWrite();
	if(!m_pFlora)
		m_pFlora = new DePlanetFlora[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFlora[rz].InitRingworld(dice);
		dice.Random();
	}
	m_RingSectionLock.unlock();

	// Create animal encounter tables.
	if(CanHaveFauna())
		InitAnimalEncounterTable(dice);
}

void DaWorldRingSection::CreateRingworldSection(const AuDice& Cdice, int sectionIndex, const DaWorldRingSection& firstSection)
{
	AuDice dice(Cdice);
	DaWorldLL::Create(firstSection.OrbitRadiusAu(), firstSection.OrbitZone());

	Q_ASSERT(sectionIndex > 0);
	m_SectionIndex = sectionIndex;

	// Some values are shared by all ringworld sections.
	m_byAtmDensity = firstSection.AtmosphereDensity();
	m_AtmType = firstSection.AtmosphereType();

	// Calculate water percentage. 25 to 75
	do
	{
		m_byWater = dice.Roll(2, 6, 1);
		if(m_byAtmDensity <= 20)
			m_byWater -= 4;
		// Generate a ones digit also after the multiply.
		m_byWater = (m_byWater*10) + dice.Roll(1, 10, -1);
	}
	while(m_byWater < 25 || m_byWater > 75);
	if(g_Au.m_LogMessages & LOG_AtomCreate)
		qDebug("DaWorldRingSection::CreateRingworldSection %d Ringworld water %d atmosphere %d", Id().GetID(), m_byWater,
				m_byAtmDensity);

	CreateTerrainData(dice);			// Create the 3D terrain data.
	dice.Random();
	CreateGeologicalFeatures(dice);
	dice.Random();

	// All bodies that could have cities on them must have plant DNA so the farms and orchards can make plants.
	m_RingSectionLock.lockForWrite();
	if(!m_pFlora)
		m_pFlora = new DePlanetFlora[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFlora[rz].InitRingworld(dice);
		dice.Random();
	}
	m_RingSectionLock.unlock();

	// Create animal encounter tables.
	if(CanHaveFauna())
		InitAnimalEncounterTable(dice);
}

void DaWorldRingSection::CreateRingworldSectionForScan(const AuDice& Cdice, int sectionIndex,
		const DaWorldRingSection& firstSection)
{
	AuDice dice(Cdice);
	DaWorldLL::CreateForScan(firstSection.OrbitRadiusAu(), firstSection.OrbitZone());

	Q_ASSERT(sectionIndex > 0);
	m_SectionIndex = sectionIndex;

	// Some values are shared by all ringworld sections.
	m_byAtmDensity = firstSection.AtmosphereDensity();
	m_AtmType = firstSection.AtmosphereType();

	// Calculate water percentage. 25 to 75
	do
	{
		m_byWater = dice.Roll(2, 6, 1);
		if(m_byAtmDensity <= 20)
			m_byWater -= 4;
		// Generate a ones digit also after the multiply.
		m_byWater = (m_byWater*10) + dice.Roll(1, 10, -1);
	}
	while(m_byWater < 25 || m_byWater > 75);
	if(g_Au.m_LogMessages & LOG_AtomCreate)
		qDebug("DaWorldRingSection::CreateRingworldSection %d Ringworld water %d atmosphere %d", Id().GetID(), m_byWater,
				m_byAtmDensity);

	CreateTerrainData(dice);			// Create the 3D terrain data.
	dice.Random();
	CreateGeologicalFeatures(dice);
	dice.Random();

	// All bodies that could have cities on them must have plant DNA so the farms and orchards can make plants.
	m_RingSectionLock.lockForWrite();
	if(!m_pFlora)
		m_pFlora = new DePlanetFlora[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFlora[rz].InitRingworld(dice);
		dice.Random();
	}
	m_RingSectionLock.unlock();

	// Create animal encounter tables.
	if(CanHaveFauna())
		InitAnimalEncounterTable(dice);
}

QString DaWorldRingSection::DefaultName(const QString& solarSystemName) const
{
	// The name is empty so do the work to build the name.
	const DaStar* pstar = FindParentStarInEnv();
	//Q_ASSERT(pstar);
	if(!pstar)
		return StringId();

	// What is the star's position in the solar system.
	int istar = pstar->StarNumber();
	const char *squal;
	switch(istar)
	{
		case -1:
		default:
			squal = "";
			break;
		case 0:
			//squal = "¹";
			squal = " Alpha";
			break;
		case 1:
			//squal = "²";
			squal = " Beta";
			break;
		case 2:
			//squal = "³";
			squal = " Gamma";
			break;
	}

	return tr("%1%2 Ringworld Arc %3").arg(solarSystemName).arg(squal).arg(m_SectionIndex+1);
}

// Report a longer description of this Atom for examination purposes.
QString DaWorldRingSection::Description() const
{
	QString desc;

	// Planet nature and size.
	double width = FROUND(TerrainYSize() / FeetPerMeter(), 100.);				// meters width of ringworld terrain
	double arclen = FROUND(TerrainXSize() / FeetPerMeter(), 1000.);				// meters arc length of ringworld terrain
	desc += tr("Ringworld Arc %3 %L1m wide x %L2m arc").arg(width).arg(arclen).arg(m_SectionIndex+1);

	// Parent star name
	const DaStar* star = FindParentStarInEnv();
	if(star)
		desc += tr(", %1").arg(star->Name());

	// Orbit zone around parent star
	// Ringworlds are always in the habitable zone.
	desc += tr(", Habitable zone");

	return desc;
}

void DaWorldRingSection::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari,
		AuModelMatrixHash& rmod, const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc,
		DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList&
		shadScRingworldFog)
{
	//qDebug() << "DaWorldRingSection::RenderEnvironmentObject" << Id() << Name();
	// Determine where the viewer is with respect to this planet.
	bool bIsInAtm = false;		// true if the viewer is inside the atmosphere of 'this' planet.
	bool cgUnderWater = false;
	bool eyeUnderWater = false;
	bool lookingThroughWater = false;	// true when viewer's eye is in the water and not inside an enclosure or building.
	m_ViewerNear = false;	// true if the viewer is subordinate to this planet so all detail must be shown.
	bool viewerAtThisPlanet = false;
	bool eyeUnderGround = false;	// true if the person's eye is below the ground surface
	float fatmd = 0.f;			// atm density at viewer (.5 at sea level on Earth)
	float fillum = 0.f;			// solar illumination at viewer
	float viewerAlt = 0.f;		// 0 unless inside atmosphere.
	QColor colAtm(0,0,0);
	const DaWorld* pParent = sceneAtom->FindParentPlanetInEnv();
	if(pParent == this)
	{
		viewerAlt = sceneAtom->CGAltitude();
		if(sceneAtom->IsInAtmosphere())
		{
			// The viewer is inside my atmosphere radius, whatever the density.
			bIsInAtm = true;
			fatmd = sceneAtom->AtmosphereDensity();	// Adjusted for altitude.
			fillum = sceneAtom->Illumination();		// Adjusted for sea depth.
			colAtm = sceneAtom->ColorOfAtmosphere();
			cgUnderWater = sceneAtom->IsCGUnderWater();
			eyeUnderWater = sceneAtom->IsEyeUnderWater();
			lookingThroughWater = eyeUnderWater && !sceneAtom->IsInsideBd() && !sceneAtom->IsInsideEnclosure();
			eyeUnderGround = sceneAtom->IsUnderGround();
		}

		// The scene atom is subordinate to this planet so show near detail
		m_ViewerNear = true;
		viewerAtThisPlanet = true;
	}
	else if(pParent)
	{
		// The scene atom is subordinate to a planet other than this one.
		// Check for adjacent ringworld section so we can show sufficient detail there.
		const DaWorldRingSection* sceneRingSect = dynamic_cast<const DaWorldRingSection*>(pParent);
		if(sceneRingSect && sceneRingSect->Parent() == Parent())
		{
			// Scene atom is at a ringworld section around the same star as me.
			int sections = RingworldSections();
			int prevSect = (sceneRingSect->SectionIndex() + (sections - 1)) % sections;
			int nextSect = (sceneRingSect->SectionIndex() + 1) % sections;
			if(prevSect == SectionIndex() || nextSect == SectionIndex())
			{
				// Viewer is near enough to say they are "near" this planet.
				viewerAlt = sceneAtom->CGAltitude();
				if(sceneAtom->IsInAtmosphere())
				{
					// The viewer is inside the atmosphere of an adjacent ring section.
					// Show atmospheric detail only if this is the closest adjacent section, not the one at the other end.
					if(sceneAtom->GlobeLL().Longitude() > 0)
					{
						if(prevSect == SectionIndex())
							bIsInAtm = true;
					}
					else
					{
						if(nextSect == SectionIndex())
							bIsInAtm = true;
					}

					if(bIsInAtm)
					{
						fatmd = sceneAtom->AtmosphereDensity();	// Adjusted for altitude.
						fillum = sceneAtom->Illumination();		// Adjusted for sea depth.
						colAtm = sceneAtom->ColorOfAtmosphere();
						cgUnderWater = sceneAtom->IsCGUnderWater();
						eyeUnderWater = sceneAtom->IsEyeUnderWater();
						lookingThroughWater = eyeUnderWater && !sceneAtom->IsInsideBd() && !sceneAtom->IsInsideEnclosure();
						eyeUnderGround = sceneAtom->IsUnderGround();
					}
				}

				// The scene atom is close to this planet so show near detail
				m_ViewerNear = true;
			}
		}
	}

	// Ringworlds always have a 0 rotation angle. Their rotation is based on orbital rotation, not axial rotation.
	const AuMatrix4x4& model = rmod.Matrix(Id());

	// Render according to what the viewer perceives.
	//bool isInView = true;
	DaSolarSystem* pss = FindParentSolarSystem();
	const float starLight = pss->StarLight();		// Actual starlight level based on local star density.
	float fogDensity = 0;							// Fog density for GL_EXP fog computation.
	float fogcol[4] = {0.f, 0.f, 0.f, 0.f};
	float florafogDensity = 0;						// Fog density for GL_EXP fog computation.
	float florafogcol[4] = {0.f, 0.f, 0.f, 0.f};
	if(bIsInAtm)
	{
		// The viewer is a person who is within atmosphere range, even if the density is zero.
		if(pass == PASS_Far)
			return;

		// The viewer is a person who is within atmosphere range, even if the density is zero.
		float atmillum = starLight + fillum*2.f;	// illumination of the atmosphere shell
		if(atmillum > 1)
			atmillum = 1;
		//float atmalpha = 0;
		fillum += starLight;			// illumination at viewer
		if(!ari.IsViewTopDown())
		{
			if(lookingThroughWater)
			{
				// Viewer's eye is under water.
				// Setup atmospheric fog to present underwater environment.
				fogDensity = SeaFogDensity();

				QColor colSea = SeaColor();
				fogcol[0] = colSea.redF() * fillum;
				fogcol[1] = colSea.greenF() * fillum;
				fogcol[2] = colSea.blueF() * fillum;
				fogcol[3] = 1.0f;

				// Could be inside an under sea city.
				if(sceneAtom->IsInsideEnclosure())
				{
					fogDensity /= 5;

					// Under sea inside enclosure. The only visible plants are not in the water.
					florafogDensity = fatmd/1000.f;
					florafogcol[0] = colAtm.redF() * fillum;
					florafogcol[1] = colAtm.greenF() * fillum;
					florafogcol[2] = colAtm.blueF() * fillum;
					florafogcol[3] = 1.0f;
				}
				else
				{
					florafogDensity = fogDensity;
					florafogcol[0] = fogcol[0];
					florafogcol[1] = fogcol[1];
					florafogcol[2] = fogcol[2];
					florafogcol[3] = fogcol[3];
				}
			}
			else if(fatmd > 0.0)
			{
				// Viewer is inside an atmosphere but not under water.
				// They could be in top down view but we open the sea for them to see to the bottom.
				// Setup atmospheric fog based on atmosphere density.
				fogDensity = fatmd/1000.f;
				//qDebug() << "fogDensity" << fogDensity << "fatmd" << fatmd << "atmDen" << m_byAtmDensity;
				fogcol[0] = colAtm.redF() * fillum;
				fogcol[1] = colAtm.greenF() * fillum;
				fogcol[2] = colAtm.blueF() * fillum;
				fogcol[3] = 1.0f;

				florafogDensity = fogDensity;
				florafogcol[0] = fogcol[0];
				florafogcol[1] = fogcol[1];
				florafogcol[2] = fogcol[2];
				florafogcol[3] = fogcol[3];
			}
		}

		// Configure ambient lighting for scene near viewer.
		//qDebug("starLight %g fillum %g atmillum %g atmalpha %g", starLight, fillum, atmillum, atmalpha);
		// Ambient light is modulated to 50% so the noon-day sun doesn't wash out all detail.
		float ambientLight = fillum*.5f;

		AuPoint viewerPos;
		if(viewerAtThisPlanet)
			sceneAtom->GetLocationRelativeToParent(viewerPos, this);
		else
		{
			// viewer must be at a ring section adjacent to this one, close enough to see in-atmosphere details.
			// Calculate their location relative to me.
			const DaStar* myStar = dynamic_cast<const DaStar*>(Parent());
			sceneAtom->GetLocationRelativeToParent(viewerPos, myStar);	// Viewer location relative to my parent star.
			viewerPos = Transform().UnProject(viewerPos);				// Viewer location relative to me.
		}

		// Render close up globe model.
		RenderAtGlobe(projection, model, ari, viewerPos, ambientLight, fogDensity, fogcol[0], fogcol[1], fogcol[2]);

		// Render rocks.
		RenderRocks(projection, model, *ari.re, ari.mm, viewerPos, ambientLight, fogDensity, fogcol[0], fogcol[1], fogcol[2]);

		// Render henges
		RenderHenges(projection, model, ari, ambientLight, ambientLight, ambientLight, fogDensity, fogcol[0], fogcol[1], fogcol[2]);

		// Render claim flag.
		RenderScWorld(projection, model, ari, sceneAtom);

		// Render plants
		if(m_pFlora)
			RenderFlora(projection, model, *ari.re, ari.mm, viewerPos, ambientLight, florafogDensity, florafogcol[0],
					florafogcol[1], florafogcol[2]);

		// Render control center.
		if(!SectionIndex())
		{
			// The control center lights up when the scene atom stands in the activation zone.
			bool isPreonPresent = sceneAtom->Parent() == this && IsActivationLocation(sceneAtom->Location());

			RenderControlCenter(projection, model, *ari.re, starLight, fogDensity, fogcol[0], fogcol[1], fogcol[2],
					isPreonPresent);
		}

		// Render atoms inside the atmosphere after terrain and globe so we don't see space through the windshield of the
		// motor bike and other such anomalies.
		DaAtomList subs;
		ListSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			if((*it)->DoesQueryEnvironment() && (*it)->IsInAtmosphere())
				(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
						shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		if(eyeUnderWater)
		{
			// Eye is under water.
			// Render atmosphere opaque when eye is under water.
			RenderAtmosphere(projection, model, *ari.re, ari.mm, 0, viewerAlt, !ari.IsViewTopDown(), atmillum, 1);

			// When eye is under water, we must render the sea surface after the atmosphere or the atmosphere doesn't work.
			// Sea globe is not rendered when we do not have the low tessellation model or the polygon collisions look bad.
			if(m_AtGlobe.m_Low.VAO())
				RenderSea(projection, model, *ari.re, eyeUnderWater, cgUnderWater, fillum, ambientLight, fogDensity,
						fogcol[0], fogcol[1], fogcol[2], ari.IsViewTopDown());
		}
		else
		{
			// Eye is not under water.
			// Sea globe is not rendered when eye is under the surface of the ground.
			// Sea globe is not rendered when we do not have the low tessellation model or the polygon collisions look bad.
			if(m_byWater && (ari.IsViewTopDown() || !eyeUnderGround) && m_AtGlobe.m_Low.VAO())
				RenderSea(projection, model, *ari.re, eyeUnderWater, cgUnderWater, fillum, ambientLight, fogDensity,
						fogcol[0], fogcol[1], fogcol[2], ari.IsViewTopDown());

			// Render non-ring satellites and ships in orbit.
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				// Remaining atoms are not rendered when the viewer is under the ground surface.
				if(eyeUnderGround)
					continue;

				DaAsteroid* roid = dynamic_cast<DaAsteroid*>(*it);
				if(roid)
				{
					roid->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
							shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
					continue;
				}

				DaVessel* ship = dynamic_cast<DaVessel*>(*it);
				if(ship && !ship->IsInAtmosphere())
				{
					ship->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
							shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
					continue;
				}
			}
		}

		// Render the atmosphere last because my parent gas giant, if I have one, is going to render its atmosphere last. If
		// I don't render mine after that, the gas giant's atmosphere is clipped by my atmosphere.
		// Don't render the atmosphere until we have a globe model.
		// Render terrain info sprites last.
		renderLast.Insert(rmod, this);

		RenderShadowSquare(projection, model, *ari.re, starLight);

		RenderHull(projection, model, *ari.re, starLight, fogDensity, fogcol[0], fogcol[1], fogcol[2]);

		// Base atom class needs to render special effects.
		DaWorldLL::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);
	}
	else
	{
		const AuPoint& pv = rmod.ViewLoc(Id());	// My location relative to the viewer.
		bool renderThisPass;
		bool renderHull = true;
		if(m_ViewerNear)
		{
			// The viewer is outside my atmosphere but close enough to see planet details.
			renderThisPass = pass == PASS_Near;
			if(renderThisPass)
			{
				// Render the globe.
				//if(pv.Length() < MinimumSectionLength()/2 + FeetPerMoonOrbit()*2.25)
				{
					RenderNearGlobe(projection, model, ari, starLight);

					// Render the atmosphere last so it doesn't obscure the sun.
					renderLast.Insert(rmod, this);
				}
				//else
				//{
					// Viewer is far enough away to just render the composite globe model. Otherwise cloud polygon collisions look
					// horrible.
					//RenderAtmosphere(projection, model, *ari.re, ari.mm, 2, 0, false, starLight,1);
				//}
			}

			// Render units outside the atmosphere that are subordinate to this planet in the near pass.
			DaAtomList subs;
			ListSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
				if((*it)->IsVisibleNearWorld())
					(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
							shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
		}
		else
		{
			// The viewer is outside the atmosphere and is not subordinate to this planet.
			renderThisPass = pass == PASS_Far;
			AuPoint pvs = model.Project(LocOfLL(AuGlobeLL(-PI,0)));	// Start-center location of ring section relative to the viewer.
			AuPoint pve = model.Project(LocOfLL(AuGlobeLL(PI,0)));	// End-center location of ring section relative to the viewer.
			if(pv.z < 0. || pvs.z < 0. || pve.z < 0.)
			{
				// This planet is in front of the camera plane.
				// We don't need the globe itself at this distance.
				if(renderThisPass)
					RenderAtmosphere(projection, model, *ari.re, ari.mm, 2, 0, false, starLight,1);
			}
			else
				renderHull = false;

			// Render units outside the atmosphere subordinate to this planet.
			DaAtomList subs;
			ListSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
				if((*it)->IsVisibleFarWorld())
					(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
							shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
		}

		if(renderThisPass)
		{
			RenderShadowSquare(projection, model, *ari.re, starLight);

			if(renderHull)
				RenderHull(projection, model, *ari.re, starLight, fogDensity, fogcol[0], fogcol[1], fogcol[2]);

			// Base atom class needs to render special effects.
			DaWorldLL::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
					shadScGlobeFog, shadScRingworld, shadScRingworldFog);
		}
	}
}

// Render atmosphere and weather.
// The scene atom is at this ring section, either inside or outside the atmosphere.
// The scene atom may also be at an adjacent ring section.
void DaWorldRingSection::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	const AuMatrix4x4& model = rmod.Matrix(Id());
	// Render this planet's atmosphere.
	DaSolarSystem* pss = FindParentSolarSystem();
	GLfloat starLight = pss->StarLight();
	if(sceneAtom->IsInAtmosphere())
	{
		// The scene atom is inside my atmosphere and their eye is above water or we wouldn't be rendering the atmosphere last.
		double viewerAlt = sceneAtom->CGAltitude();				// Viewer altitude above center of globe.
		float fatmd = sceneAtom->AtmosphereDensity();			// Adjusted for altitude.
		float fillum = sceneAtom->Illumination();				// Adjusted for sea depth.
		float atmillum = starLight + fillum*2.f;				// illumination of the atmosphere shell
		if(atmillum > 1)
			atmillum = 1;
		else if(atmillum < 0)
			atmillum = 0;

		float atmalpha = 0;
		if(fatmd > 0.0)
		{
			// Viewer is inside an atmosphere but not under water.
			// Adjust alpha for altitude so atmosphere becomes less opaque.
			fillum += starLight;								// illumination at viewer
			atmalpha = fillum * fatmd/MSLAtmosphereDensity();
			if(atmalpha > 1)
				atmalpha = 1;
		}

		//qDebug() << "DaWorldRingSection::RenderLast" << Id() << Name();
		RenderAtmosphere(projection, model, *ari.re, ari.mm, 0, viewerAlt, !ari.IsViewTopDown(), atmillum, atmalpha);

		// Render smoke at volcanos, vents and bomb craters.
		//if(HasAtmosphere())
		{
			QMap<AuGlobeLLF, float> smokers;	// Key=LL, value=radius
			const AuGeologicalFeatures& geof = LockGeologicalFeatures();
			for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
				if((*gi)->IsSmokey())
					smokers.insert((*gi)->LL(), (*gi)->SmokeRadius());
			UnlockGeologicalFeatures();
			if(!smokers.empty())
			{
				QColor atmColor = AtmosphereColor();
				for(QMap<AuGlobeLLF, float>::const_iterator vi = smokers.cbegin(); vi != smokers.cend(); vi++)
				{
					QPointF wind = WindAt(vi.key());	// Use wind at cone center.
					wind /= ((OrbitZone()+1) * 2);
					const double smokeRadius = *vi;
					AuPoint loc = GetTerrainLocation(vi.key());
					float smokeIllum = QuerySunlight(loc);
					if(smokeIllum > 1)
						smokeIllum = 1;
					RenderSmoke(vi.key(), projection, model, *ari.re, loc, smokeRadius, wind, smokeIllum*atmColor.redF(),
							smokeIllum*atmColor.greenF(), smokeIllum*atmColor.blueF());
				}
			}
		}

		// Render various overlays of information onto the scene.
		if(ari.IsViewTopDown())
			RenderTerrainInfo(model, ari, sceneAtom);
	}
	else
	{
		// Render smoke at volcanos, vents and bomb craters.
		//if(HasAtmosphere())
		{
			QMap<AuGlobeLLF, float> smokers;	// Key=LL, value=radius
			const AuGeologicalFeatures& geof = LockGeologicalFeatures();
			for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
				if((*gi)->IsSmokey())
					smokers.insert((*gi)->LL(), (*gi)->SmokeRadius());
			UnlockGeologicalFeatures();
			if(!smokers.empty())
			{
				QColor atmColor = AtmosphereColor();
				for(QMap<AuGlobeLLF, float>::const_iterator vi = smokers.cbegin(); vi != smokers.cend(); vi++)
				{
					QPointF wind = WindAt(vi.key());	// Use wind at cone center.
					wind /= ((OrbitZone()+1) * 2);
					const double smokeRadius = *vi;
					AuPoint loc = GetTerrainLocation(vi.key());
					float smokeIllum = QuerySunlight(loc);
					if(smokeIllum > 1)
						smokeIllum = 1;
					RenderSmoke(vi.key(), projection, model, *ari.re, loc, smokeRadius, wind, smokeIllum*atmColor.redF(),
							smokeIllum*atmColor.greenF(), smokeIllum*atmColor.blueF());
				}
			}
		}

		// The scene atom is near this planet but not inside my atmosphere.
		RenderAtmosphere(projection, model, *ari.re, ari.mm, 1, 0, true, starLight, starLight);
	}

	DaWorldLL::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);
}

// Render the sea surface.
// Lights have already been set up.
// View and World transforms have already been set up.
// This model is only displayed when the viewer is in the atmosphere.
void DaWorldRingSection::RenderSeaGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool bothLayers, float illumination, float ambientLight, bool showUndersideOfSurface, float fogDensity,
		float fogR, float fogG, float fogB, bool topDownView)
{
	Q_UNUSED(projection);
	if(g_Au.m_DebugSwitches & 0x00000400)
		return;

	if(!m_SeaGlobe.VAO() || (bothLayers && !m_UnderseaGlobe.VAO()))
	{
		// We need to create one or both of the sea globe vertex array objects.
		// Tessellate a globe for the sea surfaces. This tessellation only expresses latitude and longitude, not altitude of points.
		//qDebug() << "DaWorldRingSection::RenderSeaGlobe" << Id() << Name() << "tessellating...";
		int eqdivs = SeaLevelEquatorDivisions() * 2;
		int londivs = eqdivs/2;
		int latdivs = londivs * TerrainYSize() / TerrainXSize() / 10;

		AuGlobeLLTriList seaTris;
		CreateCylinderTriangles(seaTris, londivs, latdivs);
		double aspectXY = TerrainXSize() / TerrainYSize();
		//qDebug() << "DaWorldRingSection::RenderSeaGlobe" << Id() << Name() << "done tessellating" << seaTris.count() << "tris"
			//<< "aspect" << aspectXY;

		if(!m_SeaGlobe.VAO())
		{
			// Create the sea surface globe vertex array object.
			int texreps = eqdivs * 3;				// More than the lower layer makes this layer move more quickly.

			// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
			QList<AuGlobeTriTexN> tris;							// Triangles that make up this model.
			for(AuGlobeLLTriList::const_iterator ti = seaTris.cbegin(); ti != seaTris.cend(); ti++)
			{
				// Calculate triangle vertex coordinates.
				AuPoint terCoord0 = LocOfLL((*ti).m_LL0);
				AuPoint terCoord1 = LocOfLL((*ti).m_LL1);
				AuPoint terCoord2 = LocOfLL((*ti).m_LL2);

				// Calculate texture coordinates.
				QPointF terTex0;
				terTex0.setX(((*ti).m_LL0.Longitude())/(PIf*2) * texreps * aspectXY);
				terTex0.setY(((*ti).m_LL0.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex1;
				terTex1.setX(((*ti).m_LL1.Longitude())/(PIf*2) * texreps * aspectXY);
				terTex1.setY(((*ti).m_LL1.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex2;
				terTex2.setX(((*ti).m_LL2.Longitude())/(PIf*2) * texreps * aspectXY);
				terTex2.setY(((*ti).m_LL2.Latitude() + PIf/2)/PIf * texreps);

				// Construct a globe triangle.
				AuGlobeTriTexN tri;
				tri.m_Vert0 = AuGlobeVertTexN(terCoord0, terTex0, -GravityDir(terCoord0));
				tri.m_Vert1 = AuGlobeVertTexN(terCoord1, terTex1, -GravityDir(terCoord1));
				tri.m_Vert2 = AuGlobeVertTexN(terCoord2, terTex2, -GravityDir(terCoord2));

				// Append the globe triangle to the list.
				tris.append(tri);
			}

			// Store count of triangles in the far globe vertex array object.
			unsigned int vertices = tris.count()*3;

			// Copy surface triangles into gl array buffers.
			GLfloat* globePos = new GLfloat[vertices*3];		// vertices for triangles that make up the model.
			GLfloat* globeN = new GLfloat[vertices*3];			// normals for triangles that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*2];		// texture coords for each of the vertices
			GLfloat* globeTan = new GLfloat[vertices*3];		// tangent vectors

			unsigned int globePosIdx = 0;
			unsigned int globeNIdx = 0;
			unsigned int globeTexIdx = 0;
			unsigned int globeTanIdx = 0;
			for(QList<AuGlobeTriTexN>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
			{
				// Calculate tangent vectors.
				AuPoint t0;
				(*vi).m_Vert0.m_N.ArbitraryAxis(t0);
				AuPoint t1;
				(*vi).m_Vert1.m_N.ArbitraryAxis(t1);
				AuPoint t2;
				(*vi).m_Vert2.m_N.ArbitraryAxis(t2);

				// Vert 0
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t0.x;
				globeTan[globeTanIdx++] = (float)t0.y;
				globeTan[globeTanIdx++] = (float)t0.z;

				// Vert 2
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t1.x;
				globeTan[globeTanIdx++] = (float)t1.y;
				globeTan[globeTanIdx++] = (float)t1.z;

				// Vert 2
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t2.x;
				globeTan[globeTanIdx++] = (float)t2.y;
				globeTan[globeTanIdx++] = (float)t2.z;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_SeaGlobe.CreateGL(re, vertices, 4);

			// Select vertex array to construct.
			m_SeaGlobe.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(0));
			re.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for vertices and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(1));
			re.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
			re.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for colors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(2));
			re.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			re.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for tangent vectors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(3));
			re.glBufferData(GL_ARRAY_BUFFER, globeTanIdx*sizeof(GLfloat), globeTan, GL_STATIC_DRAW);
			re.glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			re.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_SeaGlobe.DeselectGL();

			delete [] globePos;
			delete [] globeN;
			delete [] globeTex;
			delete [] globeTan;
		}

		// Sometimes we need to obscure the sea bottom.
		// This is done by displaying an opaque darker smaller copy of the sea surface model.
		if(bothLayers && !m_UnderseaGlobe.VAO())
		{
			// Create the undersea subsurface globe vertex array object.
			CreateCylinderTriangles(seaTris, eqdivs, latdivs);
			int texreps = eqdivs;					// Less than the upper layer makes this layer move more slowly.

			double depth = -7;		// depth of opaque lower water layer

			// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
			QList<AuGlobeTriTexN> tris;		// Triangles that make up this model.
			for(AuGlobeLLTriList::const_iterator ti = seaTris.cbegin(); ti != seaTris.cend(); ti++)
			{
				// Calculate triangle vertex coordinates.
				AuPoint terCoord0 = LocOfLL((*ti).m_LL0, depth);
				AuPoint terCoord1 = LocOfLL((*ti).m_LL1, depth);
				AuPoint terCoord2 = LocOfLL((*ti).m_LL2, depth);

				// Calculate texture coordinates.
				QPointF terTex0;
				terTex0.setX(((*ti).m_LL0.Longitude())/(PIf*2) * texreps * aspectXY);
				terTex0.setY(((*ti).m_LL0.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex1;
				terTex1.setX(((*ti).m_LL1.Longitude())/(PIf*2) * texreps * aspectXY);
				terTex1.setY(((*ti).m_LL1.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex2;
				terTex2.setX(((*ti).m_LL2.Longitude())/(PIf*2) * texreps * aspectXY);
				terTex2.setY(((*ti).m_LL2.Latitude() + PIf/2)/PIf * texreps);

				// Construct a globe triangle.
				AuGlobeTriTexN tri;
				tri.m_Vert0 = AuGlobeVertTexN(terCoord0, terTex0, -GravityDir(terCoord0));
				tri.m_Vert1 = AuGlobeVertTexN(terCoord1, terTex1, -GravityDir(terCoord0));
				tri.m_Vert2 = AuGlobeVertTexN(terCoord2, terTex2, -GravityDir(terCoord0));

				// Append the globe triangle to the list.
				tris.append(tri);
			}

			// Store count of triangles in the far globe vertex array object.
			unsigned int vertices = tris.count()*3;

			// Copy surface triangles into gl array buffers.
			GLfloat* globePos = new GLfloat[vertices*3];		// vertices for triangles that make up the model.
			GLfloat* globeN = new GLfloat[vertices*3];			// normals for triangles that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*2];		// texture coords for each of the vertices
			GLfloat* globeTan = new GLfloat[vertices*3];		// tangent vectors

			unsigned int globePosIdx = 0;
			unsigned int globeNIdx = 0;
			unsigned int globeTexIdx = 0;
			unsigned int globeTanIdx = 0;
			for(QList<AuGlobeTriTexN>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
			{
				// Calculate tangent vectors.
				AuPoint t0;
				(*vi).m_Vert0.m_N.ArbitraryAxis(t0);
				AuPoint t1;
				(*vi).m_Vert1.m_N.ArbitraryAxis(t1);
				AuPoint t2;
				(*vi).m_Vert2.m_N.ArbitraryAxis(t2);

				// Vert 0
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t0.x;
				globeTan[globeTanIdx++] = (float)t0.y;
				globeTan[globeTanIdx++] = (float)t0.z;

				// Vert 1
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t1.x;
				globeTan[globeTanIdx++] = (float)t1.y;
				globeTan[globeTanIdx++] = (float)t1.z;

				// Vert 2
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
				globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t2.x;
				globeTan[globeTanIdx++] = (float)t2.y;
				globeTan[globeTanIdx++] = (float)t2.z;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_UnderseaGlobe.CreateGL(re, vertices, 4);

			// Select vertex array to construct.
			m_UnderseaGlobe.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_UnderseaGlobe.Buffer(0));
			re.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for vertices and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_UnderseaGlobe.Buffer(1));
			re.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
			re.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for colors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_UnderseaGlobe.Buffer(2));
			re.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			re.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for tangent vectors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(3));
			re.glBufferData(GL_ARRAY_BUFFER, globeTanIdx*sizeof(GLfloat), globeTan, GL_STATIC_DRAW);
			re.glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			re.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_UnderseaGlobe.DeselectGL();

			delete [] globePos;
			delete [] globeN;
			delete [] globeTex;
			delete [] globeTan;
		}
	}

	if(g_Au.BumpMapSeas())
	{
		// Use bump mapping.
		if(!bothLayers && topDownView)
		{
			// Render with aperture.
			// Render the sea globe vertex array object.
			re.GLSL().WaterRingworldShader().Select(projection);
			re.GLSL().WaterRingworldShader().SetModelView(model);
			re.GLSL().WaterRingworldShader().SetTexture(s_texSea);
			re.GLSL().WaterRingworldShader().SetBumpMap(s_texSeaN);
			re.GLSL().WaterRingworldShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterRingworldShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Ambient light
			re.GLSL().WaterRingworldShader().SetAmbientLight(ambientLight);

			// Top down view aperture.
			float clipDist = 250*(float)FeetPerMeter();
			AuPoint clipCenter(eyeLoc);
			ElevateLocMSL(clipCenter, 0);
			re.GLSL().WaterRingworldShader().SetClip(clipDist, clipCenter.x, clipCenter.y, clipCenter.z);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
						ShadowPanelSunAngle(sun.m_ShadowAngle),
						sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = re.Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			// Prepare for rendering the sea surface.
			// Face culling is disabled in situations that enable the viewer to see the underside of the transparent surface layer,
			// such as when the camera is close to the water line.
			if(showUndersideOfSurface)
				re.glDisable(GL_CULL_FACE);

			// Prepare for rendering sea surface transparently.
			re.GLSL().WaterRingworldShader().SetOpacity(WaterOpacity(illumination));

			// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
			re.GLSL().WaterRingworldShader().SetAmbientLight(illumination*.8f);

			// Alpha blending is enabled for transparent effect.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			m_SeaGlobe.SelectGL();
			m_SeaGlobe.DrawGL();
			m_SeaGlobe.DeselectGL();

			re.glDisable(GL_BLEND);

			// Restore face culling if we changed it.
			if(showUndersideOfSurface)
				re.glEnable(GL_CULL_FACE);

			re.GLSL().WaterRingworldShader().Deselect();
		}
		else
		{
			// No aperture.
			// Display the opaque lower layer without costly bump mapping or blending.
			if(bothLayers)
			{
				// No bump mapping.
				// Render the sea globe vertex array object.
				re.GLSL().WaterRingworldNoBumpNoHoleShader().Select(projection);
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetModelView(model);
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetTexture(s_texSea);
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetOpacity(1);
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetFog(fogDensity, fogR, fogG, fogB);

				// Set eye position in object space for specular lighting calculations.
				AuPoint eyeLoc = model.UnProject(re.EyeLocation());
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

				// Ambient light
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetAmbientLight(ambientLight);

				// Suns
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
				{
					// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
					const AuGLSun& sun = re.Sun(sunIndex);
					AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
					AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
					AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

					// Configure shader for object space lighting.
					re.GLSL().WaterRingworldNoBumpNoHoleShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR,
							sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
							ShadowPanelSunAngle(sun.m_ShadowAngle),
							sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
				}

				// Point lights
				for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
				{
					// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
					const AuGLLight& light = re.Light(lightIndex);
					AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
					AuPoint spotDir;
					if(!light.m_Direction.IsZero())
						spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

					// Configure shader for object space lighting.
					re.GLSL().WaterRingworldNoBumpNoHoleShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ,
							light.m_ColorR, light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
							light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
				}

				// Display the opaque lower layer.
				m_UnderseaGlobe.SelectGL();
				m_UnderseaGlobe.DrawGL();
				m_UnderseaGlobe.DeselectGL();

				re.GLSL().WaterRingworldNoBumpNoHoleShader().Deselect();
			}

			// Render the sea globe vertex array object.
			re.GLSL().WaterRingworldNoHoleShader().Select(projection);
			re.GLSL().WaterRingworldNoHoleShader().SetModelView(model);
			re.GLSL().WaterRingworldNoHoleShader().SetTexture(s_texSea);
			re.GLSL().WaterRingworldNoHoleShader().SetBumpMap(s_texSeaN);
			re.GLSL().WaterRingworldNoHoleShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterRingworldNoHoleShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterRingworldNoHoleShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Ambient light
			re.GLSL().WaterRingworldNoHoleShader().SetAmbientLight(ambientLight);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldNoHoleShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
						ShadowPanelSunAngle(sun.m_ShadowAngle),
						sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = re.Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldNoHoleShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			// Prepare for rendering the sea surface.
			// Face culling is disabled in situations that enable the viewer to see the underside of the transparent surface layer,
			// such as when the camera is close to the water line.
			if(showUndersideOfSurface)
				re.glDisable(GL_CULL_FACE);

			// Prepare for rendering sea surface transparently.
			re.GLSL().WaterRingworldNoHoleShader().SetOpacity(WaterOpacity(illumination));

			// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
			re.GLSL().WaterRingworldNoHoleShader().SetAmbientLight(illumination*.8f);

			// Alpha blending is enabled for transparent effect.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			m_SeaGlobe.SelectGL();
			m_SeaGlobe.DrawGL();
			m_SeaGlobe.DeselectGL();

			re.glDisable(GL_BLEND);

			// Restore face culling if we changed it.
			if(showUndersideOfSurface)
				re.glEnable(GL_CULL_FACE);

			re.GLSL().WaterRingworldNoHoleShader().Deselect();
		}
	}
	else
	{
		// No bump mapping.
		if(!bothLayers && topDownView)
		{
			// Render with aperture.
			// Render the sea globe vertex array object.
			re.GLSL().WaterRingworldNoBumpShader().Select(projection);
			re.GLSL().WaterRingworldNoBumpShader().SetModelView(model);
			re.GLSL().WaterRingworldNoBumpShader().SetTexture(s_texSea);
			re.GLSL().WaterRingworldNoBumpShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterRingworldNoBumpShader().SetOpacity(1);
			re.GLSL().WaterRingworldNoBumpShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterRingworldNoBumpShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Ambient light
			re.GLSL().WaterRingworldNoBumpShader().SetAmbientLight(ambientLight);

			// Top down view aperture.
			float clipDist = 250*(float)FeetPerMeter();
			AuPoint clipCenter(eyeLoc);
			ElevateLocMSL(clipCenter, 0);
			re.GLSL().WaterRingworldNoBumpShader().SetClip(clipDist, clipCenter.x, clipCenter.y, clipCenter.z);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldNoBumpShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
						ShadowPanelSunAngle(sun.m_ShadowAngle),
						sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = re.Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldNoBumpShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			if(bothLayers)
			{
				// Display the opaque lower layer.
				m_UnderseaGlobe.SelectGL();
				m_UnderseaGlobe.DrawGL();
				m_UnderseaGlobe.DeselectGL();
			}

			// Prepare for rendering the sea surface.
			// Face culling is disabled in situations that enable the viewer to see the underside of the transparent surface layer,
			// such as when the camera is close to the water line.
			if(showUndersideOfSurface)
				re.glDisable(GL_CULL_FACE);

			// Prepare for rendering sea surface transparently.
			re.GLSL().WaterRingworldNoBumpShader().SetOpacity(WaterOpacity(illumination));

			// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
			re.GLSL().WaterRingworldNoBumpShader().SetAmbientLight(illumination*.8f);

			// Alpha blending is enabled for transparent effect.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			m_SeaGlobe.SelectGL();
			m_SeaGlobe.DrawGL();
			m_SeaGlobe.DeselectGL();

			re.glDisable(GL_BLEND);

			// Restore face culling if we changed it.
			if(showUndersideOfSurface)
				re.glEnable(GL_CULL_FACE);

			re.GLSL().WaterRingworldNoBumpShader().Deselect();
		}
		else
		{
			// No aperture.
			// Render the sea globe vertex array object.
			re.GLSL().WaterRingworldNoBumpNoHoleShader().Select(projection);
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetModelView(model);
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetTexture(s_texSea);
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetOpacity(1);
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Ambient light
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetAmbientLight(ambientLight);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
						ShadowPanelSunAngle(sun.m_ShadowAngle),
						sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = re.Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterRingworldNoBumpNoHoleShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			if(bothLayers)
			{
				// Display the opaque lower layer.
				m_UnderseaGlobe.SelectGL();
				m_UnderseaGlobe.DrawGL();
				m_UnderseaGlobe.DeselectGL();
			}

			// Prepare for rendering the sea surface.
			// Face culling is disabled in situations that enable the viewer to see the underside of the transparent surface layer,
			// such as when the camera is close to the water line.
			if(showUndersideOfSurface)
				re.glDisable(GL_CULL_FACE);

			// Prepare for rendering sea surface transparently.
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetOpacity(WaterOpacity(illumination));

			// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
			re.GLSL().WaterRingworldNoBumpNoHoleShader().SetAmbientLight(illumination*.8f);

			// Alpha blending is enabled for transparent effect.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			m_SeaGlobe.SelectGL();
			m_SeaGlobe.DrawGL();
			m_SeaGlobe.DeselectGL();

			re.glDisable(GL_BLEND);

			// Restore face culling if we changed it.
			if(showUndersideOfSurface)
				re.glEnable(GL_CULL_FACE);

			re.GLSL().WaterRingworldNoBumpNoHoleShader().Deselect();
		}
	}
}

void DaWorldRingSection::CreateWeatherRenderData(AuGLEffects& re, AuGLModelMaker& mm)
{
	if(HasAtmosphere() && m_glTexWx == 0)
	{
		//qDebug() << "DaWorldRingSection::CreateWeatherRenderData" << Id() << Name();
		if(!mm.Weather(m_Wx, this))
			return;
		AuDice dice;
		dice.Seed(GenSeed());

		unsigned int wxwid = m_Wx.Width();
		unsigned int wxht = m_Wx.Height();
		unsigned int wxsize = wxwid * wxht;
		wxsize *= 3;	// 3 floats at each wx cell
		float* wxdata = new float[wxsize];
		m_Wx.ShaderData(*this, wxdata,
				m_WxMaxSP, m_WxMaxVP,
				m_WxMinSP, m_WxMinVP,
				AuWeather::LowClouds, dice);
		QString stid(Id().StringId());
		stid += "WX";	// Make an ID unique to this planet.
		m_glTexWx = re.CreateTextureFromFloats(Id().GetID(), stid, wxdata, wxwid, wxht);
		delete [] wxdata;
		mm.DeleteWeather(Id());
		//qDebug() << "DaWorldRingSection::CreateWeatherRenderData" << Id() << Name() << "done";
	}
}

void DaWorldRingSection::CreateAtmosphereVAO(AuOpenGLFunctions& GLf)
{
	Q_ASSERT(!m_Atmosphere.VAO());
	// Tessellate a globe for the atmosphere with vertices at sea level.
	AuGlobeLLTriList atmoTris;
	CreateCylinderTriangles(atmoTris, SeaLevelEquatorDivisions()/50, 1);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
	QList<AuGlobeTriTexN> tris;						// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = atmoTris.cbegin(); ti != atmoTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates at sea level.
		AuPoint terCoord0 = LocOfLL((*ti).m_LL0);
		AuPoint terCoord1 = LocOfLL((*ti).m_LL1);
		AuPoint terCoord2 = LocOfLL((*ti).m_LL2);

		float ll0long = (*ti).m_LL0.Longitude();			// Range is -PI to PI
		float ll1long = (*ti).m_LL1.Longitude();
		float ll2long = (*ti).m_LL2.Longitude();

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
		AuGlobeTriTexN tri;
		tri.m_Vert0 = AuGlobeVertTexN(terCoord0, terTex0, -GravityDir(terCoord0));
		tri.m_Vert1 = AuGlobeVertTexN(terCoord1, terTex1, -GravityDir(terCoord1));
		tri.m_Vert2 = AuGlobeVertTexN(terCoord2, terTex2, -GravityDir(terCoord2));

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the far globe vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeN = new GLfloat[vertices*3];				// normals for triangles that make up the model.

	unsigned int globePosIdx = 0;
	unsigned int globeNIdx = 0;
	for(QList<AuGlobeTriTexN>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		// Vert 0
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

		// Vert 1
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

		// Vert 2
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;
	}

	// Generate vertex array(s) for this object. We only need one.
	m_Atmosphere.CreateGL(GLf, vertices, 2);

	// Select vertex array to construct.
	m_Atmosphere.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Atmosphere.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Atmosphere.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_Atmosphere.DeselectGL();

	delete [] globePos;
	delete [] globeN;
}

void DaWorldRingSection::CreateCloudsVAO(AuOpenGLFunctions& GLf)
{
	Q_ASSERT(!m_Clouds.VAO());
	// Tessellate a globe for the atmosphere with vertices at sea level.
	int londivs = SeaLevelEquatorDivisions();
	int latdivs = londivs * TerrainYSize() / TerrainXSize() / 10;
	AuGlobeLLTriList atmoTris;
	CreateCylinderTriangles(atmoTris, londivs, latdivs);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
	QList<AuGlobeTriTexN> tris;						// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = atmoTris.cbegin(); ti != atmoTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates at sea level.
		AuPoint terCoord0 = LocOfLL((*ti).m_LL0);
		AuPoint terCoord1 = LocOfLL((*ti).m_LL1);
		AuPoint terCoord2 = LocOfLL((*ti).m_LL2);

		float ll0long = (*ti).m_LL0.Longitude();			// Range is -PI to PI
		float ll1long = (*ti).m_LL1.Longitude();
		float ll2long = (*ti).m_LL2.Longitude();

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
		AuGlobeTriTexN tri;
		tri.m_Vert0 = AuGlobeVertTexN(terCoord0, terTex0, -GravityDir(terCoord0));
		tri.m_Vert1 = AuGlobeVertTexN(terCoord1, terTex1, -GravityDir(terCoord1));
		tri.m_Vert2 = AuGlobeVertTexN(terCoord2, terTex2, -GravityDir(terCoord2));

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the far globe vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeN = new GLfloat[vertices*3];				// normals for triangles that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*2];			// texture coords for each of the vertices

	unsigned int globePosIdx = 0;
	unsigned int globeNIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTexN>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		// Vert 0
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

		// Vert 1
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

		// Vert 2
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
	}

	// Generate vertex array(s) for this object. We only need one.
	m_Clouds.CreateGL(GLf, vertices, 3);

	// Select vertex array to construct.
	m_Clouds.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Clouds.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Clouds.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Clouds.Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_Clouds.DeselectGL();

	delete [] globePos;
	delete [] globeN;
	delete [] globeTex;
}

// Render the atmosphere.
// Lights have already been set up.
// View and World transforms have already been set up.
// 'viewerAlt' is altitude of viewer when they are inside the atmosphere, otherwise 0.
// 'clouds' is true to render cloud layers
// viewerContext: 0=in atmosphere, 1=outside nearby, 2=outside at a distance, 3=at globe but at model not ready yet.
void DaWorldRingSection::RenderAtmosphere(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		AuGLModelMaker& mm, int viewerContext, double viewerAlt, bool clouds, float atmIllumination, float atmOpacity)
{
	if(viewerContext == 2)
	{
		// Far Globe
		if(g_Au.m_DebugSwitches & 0x00000080)
			return;
	}
	else
	{
		// Atmosphere
		if(g_Au.m_DebugSwitches & 0x00000100)
			return;
	}

	if(viewerContext < 2)
	{
		// Some worlds have no atmosphere at all.
		// In viewer context 2, this call to render the atmosphere is the only call to render this world. We end up drawing a
		// composite of the planet surface texture, combined with clouds and atmosphere onto a single atmosphere-sized sphere.
		if(!m_byAtmDensity)
			return;
	}

	switch(viewerContext)
	{
	case 0:		// The viewer is inside the atmosphere.
		{
			if(!m_Atmosphere.VAO())
				CreateAtmosphereVAO(re);

			// Insure weather render data has been created. This way we only create it when we need it.
			if(g_Au.DrawClouds())
				CreateWeatherRenderData(re, mm);

			GLfloat a = (float)m_byAtmDensity/100.f;			// Range is 0.0 for 0% density to 1.0 for 100% density.

			// Atmosphere and clouds require blending.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			// Change the atmosphere color opacity value.
			QColor atmColor = AtmosphereColor();
			atmColor.setAlphaF(a*.6f);

			// Configure atmosphere shader.
			re.GLSL().AtmosphereRingworldShader().Select();
			re.GLSL().AtmosphereRingworldShader().SetProjectionModelView(projection * model);
			re.GLSL().AtmosphereRingworldShader().SetAtmosphereAlpha(atmOpacity);
			re.GLSL().AtmosphereRingworldShader().SetAtmosphereAltitudeMSL(AtmosphereHeightMSL());
			re.GLSL().AtmosphereRingworldShader().SetAtmosphereColor(atmColor);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				re.GLSL().AtmosphereRingworldShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
						ShadowPanelSunAngle(sun.m_ShadowAngle),
						sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Render outer atmosphere shell.
			// Render outer atmosphere shell with culling reversed so it is visible from the inside.
			re.glFrontFace(GL_CCW);

			m_Atmosphere.SelectGL();
			m_Atmosphere.DrawGL();
			m_Atmosphere.DeselectGL();

			re.GLSL().AtmosphereRingworldShader().Deselect();

			// Restore normal clockwise triangle facing.
			re.glFrontFace(GL_CW);

			// Cloud layers.
			if(clouds && g_Au.DrawClouds())
			{
				if(!m_Clouds.VAO())
					CreateCloudsVAO(re);

				// Select a color for the clouds.
				QColor cloudColor;
				cloudColor.setRedF(1);
				cloudColor.setGreenF(1);
				cloudColor.setBlueF(1);
				cloudColor.setAlphaF(1);

				// Clouds use combining blend method.
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				// Configure cloud shader.
				re.GLSL().CloudRingworldShader().Select();
				re.GLSL().CloudRingworldShader().SetProjectionModelView(projection * model);
				re.GLSL().CloudRingworldShader().SetCloudColor(cloudColor);
				re.GLSL().CloudRingworldShader().SetWxOffset(m_WxSPOffset, m_WxVPOffset);

				// Ambient light
				re.GLSL().CloudRingworldShader().SetAmbientLight(atmIllumination);

				// Suns
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
				{
					// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
					const AuGLSun& sun = re.Sun(sunIndex);
					AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
					AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
					AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

					// Configure shader for object space lighting.
					re.GLSL().CloudRingworldShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
							sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
							ShadowPanelSunAngle(sun.m_ShadowAngle),
							sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
				}

				float liftMax = (m_Wx.LayerAltitude(*this, AuWeather::MiddleClouds) -
					m_Wx.LayerAltitude(*this, AuWeather::LowClouds)) * .5f;
				re.glDepthFunc(GL_LEQUAL);
				re.glDisable(GL_CULL_FACE);
				{
					float layerAlt = SeaLevelAltitude() + m_Wx.LayerAltitude(*this, AuWeather::LowClouds);
					re.GLSL().CloudRingworldShader().SetCloudLayerRadius(layerAlt);
					re.GLSL().CloudRingworldShader().SetWxMinimums(m_WxMinSP, m_WxMinVP);
					re.GLSL().CloudRingworldShader().SetWxMaximums(m_WxMaxSP, m_WxMaxVP);
					re.GLSL().CloudRingworldShader().SetWxTexture(m_glTexWx);
					switch(g_Au.DrawClouds())
					{
					case 1:
						{
							// Display a single cloud layer.
							re.GLSL().CloudRingworldShader().SetCloudLiftMax(0);
							m_Clouds.SelectGL();
							m_Clouds.DrawGL();
							m_Clouds.DeselectGL();
						}
						break;
					case 2:
						{
							// Display thick clouds using two layers.
							if(layerAlt < viewerAlt)
							{
								// Display cloud layers in low to high order that are below the viewer.
								m_Clouds.SelectGL();
								for(int tops=0; tops<=1; tops++)
								{
									float lift = liftMax * tops;
									re.GLSL().CloudRingworldShader().SetCloudLiftMax(lift);
									m_Clouds.DrawGL();
								}
								m_Clouds.DeselectGL();
							}
							else
							{
								// Display cloud layers in high to low order that are above the viewer.
								m_Clouds.SelectGL();
								for(int tops=1; tops>=0; tops--)
								{
									float lift = liftMax * tops;
									re.GLSL().CloudRingworldShader().SetCloudLiftMax(lift);
									m_Clouds.DrawGL();
								}
								m_Clouds.DeselectGL();
							}
						}
						break;
					case 3:
						{
							// Display thick clouds using multiple layers.
							if(layerAlt < viewerAlt)
							{
								// Display cloud layers in low to high order that are below the viewer.
								m_Clouds.SelectGL();
								for(int tops=-1; tops<=4; tops++)
								{
									float lift = liftMax * tops / 4;
									re.GLSL().CloudRingworldShader().SetCloudLiftMax(lift);
									m_Clouds.DrawGL();
								}
								m_Clouds.DeselectGL();
							}
							else
							{
								// Display cloud layers in high to low order that are above the viewer.
								m_Clouds.SelectGL();
								for(int tops=4; tops>=-1; tops--)
								{
									float lift = liftMax * tops / 4;
									re.GLSL().CloudRingworldShader().SetCloudLiftMax(lift);
									m_Clouds.DrawGL();
								}
								m_Clouds.DeselectGL();
							}
						}
						break;
					}
				}
				re.glEnable(GL_CULL_FACE);
				re.glDepthFunc(GL_LESS);

				re.GLSL().CloudRingworldShader().Deselect();
			}

			// Restore blending state to off.
			re.glDisable(GL_BLEND);
		}
		break;

	case 1:		// viewer is outside the atmosphere nearby
		{
			if(!m_Atmosphere.VAO())
				CreateAtmosphereVAO(re);

			// Insure weather render data has been created. This way we only create it when we need it.
			if(g_Au.DrawClouds())
				CreateWeatherRenderData(re, mm);

			// Viewer is not inside the atmosphere but can see the contents of the atmosphere.
			// Atmosphere and clouds require blending.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			re.glDepthFunc(GL_LEQUAL);								// Blend even when z depth is equal.

			// We must draw from lowest level to the highest level so blending looks right.
			if(clouds && g_Au.DrawClouds())
			{
				if(!m_Clouds.VAO())
					CreateCloudsVAO(re);

				// Select a color for the clouds.
				QColor cloudColor;
				cloudColor.setRedF(1);
				cloudColor.setGreenF(1);
				cloudColor.setBlueF(1);
				cloudColor.setAlphaF(1);

				// Configure cloud shader.
				re.GLSL().CloudRingworldShader().Select();
				re.GLSL().CloudRingworldShader().SetProjectionModelView(projection * model);
				re.GLSL().CloudRingworldShader().SetCloudColor(cloudColor);
				re.GLSL().CloudRingworldShader().SetWxOffset(m_WxSPOffset, m_WxVPOffset);

				// Ambient light
				re.GLSL().CloudRingworldShader().SetAmbientLight(atmIllumination);

				// Suns
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
				{
					// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
					const AuGLSun& sun = re.Sun(sunIndex);
					AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
					AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
					AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

					// Configure shader for object space lighting.
					re.GLSL().CloudRingworldShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
							sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
							ShadowPanelSunAngle(sun.m_ShadowAngle),
							sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
				}

				// Display cloud layers in low to high order.
				float liftMax = (m_Wx.LayerAltitude(*this, AuWeather::MiddleClouds) -
					m_Wx.LayerAltitude(*this, AuWeather::LowClouds)) * .5f;
				re.glDepthFunc(GL_LEQUAL);
				re.glDisable(GL_CULL_FACE);
				{
					float layerAlt = SeaLevelAltitude() + m_Wx.LayerAltitude(*this, AuWeather::LowClouds);
					re.GLSL().CloudRingworldShader().SetCloudLayerRadius(layerAlt);
					re.GLSL().CloudRingworldShader().SetWxMinimums(m_WxMinSP, m_WxMinVP);
					re.GLSL().CloudRingworldShader().SetWxMaximums(m_WxMaxSP, m_WxMaxVP);
					re.GLSL().CloudRingworldShader().SetWxTexture(m_glTexWx);
					switch(g_Au.DrawClouds())
					{
					case 1:
						{
							// Display a single cloud layer.
							re.GLSL().CloudRingworldShader().SetCloudLiftMax(0);
							m_Clouds.SelectGL();
							m_Clouds.DrawGL();
							m_Clouds.DeselectGL();
						}
						break;
					case 2:
						{
							// Display thick clouds using two layers.
							// Layered cloud
							m_Clouds.SelectGL();
							for(int tops=0; tops<=1; tops++)
							{
								float lift = liftMax * tops;
								re.GLSL().CloudRingworldShader().SetCloudLiftMax(lift);
								m_Clouds.DrawGL();
							}
							m_Clouds.DeselectGL();
						}
						break;
					case 3:
						{
							// Display thick clouds using multiple layers.
							// Layered cloud
							m_Clouds.SelectGL();
							for(int tops=-1; tops<=4; tops++)
							{
								float lift = liftMax * tops / 4;
								re.GLSL().CloudRingworldShader().SetCloudLiftMax(lift);
								m_Clouds.DrawGL();
							}
							m_Clouds.DeselectGL();
						}
						break;
					}
				}
				re.glEnable(GL_CULL_FACE);
				re.glDepthFunc(GL_LESS);

				re.GLSL().CloudRingworldShader().Deselect();
			}

			// Change the atmosphere color opacity value.
			QColor atmColor = AtmosphereColor();
			atmColor.setAlphaF(0);								// Material alpha will totally decide the final alpha.

			// Configure atmosphere shader.
			re.GLSL().AtmosphereRingworldShader().Select();
			re.GLSL().AtmosphereRingworldShader().SetProjectionModelView(projection * model);
			re.GLSL().AtmosphereRingworldShader().SetAtmosphereColor(atmColor);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				re.GLSL().AtmosphereRingworldShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
						ShadowPanelSunAngle(sun.m_ShadowAngle),
						sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Render outer atmosphere shell.
			m_Atmosphere.SelectGL();
			GLfloat a = (float)m_byAtmDensity/100.f;			// Range is 0.0 for 0% density to 1.0 for 100% density.
			int shells = 20;
			re.GLSL().AtmosphereRingworldShader().SetAtmosphereAlpha(a/shells);
			double topRad = AtmosphereHeightMSL();
			for(int shell=1; shell<=shells; shell++)
			{
				double altMSL = topRad * shell / shells;
				re.GLSL().AtmosphereRingworldShader().SetAtmosphereAltitudeMSL(altMSL);
				m_Atmosphere.DrawGL();
			}

			m_Atmosphere.DeselectGL();

			re.GLSL().AtmosphereRingworldShader().Deselect();

			// Restore blending state to off.
			re.glDisable(GL_BLEND);
			re.glDepthFunc(GL_LESS);								// Restore default depth function.
		}
		break;

	case 2:		// Viewer is not inside the atmosphere and is far enough away that we can just render it as a blended sphere.
	case 3:		// Viewer is at world but at globe isn't ready yet.
		{
			if(!m_Clouds.VAO())
				CreateCloudsVAO(re);

			// Insure weather render data has been created. This way we only create it when we need it.
			if(viewerContext == 2 && g_Au.DrawClouds())
				CreateWeatherRenderData(re, mm);

			// Calculate a radius for this model that accounts for the radius of the visible atmosphere.
			float atmAlpha = (float)m_byAtmDensity / 110.f;		// This calc prevents reaching 1 to let lower level details show.
			//double radius = Radius();
			//double atmRadius = radius + (AtmosphereRadius() - radius) * atmAlpha;

			QColor atmColor = AtmosphereColor();
			atmColor.setAlphaF(atmAlpha);						// Atmosphere density is the alpha value of the atmosphere color.

			// Viewer is not inside the atmosphere but can see the contents of the atmosphere from a distance.
			if(viewerContext == 2 && g_Au.DrawClouds())
			{
				// Draw world with clouds
				// Configure far globe shader. All planet details are blended into a single surface texture.
				re.GLSL().FarRingworldLitCloudsShader().Select();
				re.GLSL().FarRingworldLitCloudsShader().SetProjectionModelView(projection * model);
				re.GLSL().FarRingworldLitCloudsShader().SetAtmosphereColor(atmColor);
				re.GLSL().FarRingworldLitCloudsShader().SetPlanetTexture(m_glTexFarSurface);
				re.GLSL().FarRingworldLitCloudsShader().SetWxOffset(m_WxSPOffset, m_WxVPOffset);
				re.GLSL().FarRingworldLitCloudsShader().SetLowWxMinimums(m_WxMinSP, m_WxMinVP);
				re.GLSL().FarRingworldLitCloudsShader().SetLowWxMaximums(m_WxMaxSP, m_WxMaxVP);
				re.GLSL().FarRingworldLitCloudsShader().SetLowWxTexture(m_glTexWx);

				// Ambient light
				re.GLSL().FarRingworldLitCloudsShader().SetAmbientLight(atmIllumination);

				// Suns
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
				{
					// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
					const AuGLSun& sun = re.Sun(sunIndex);
					AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
					AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
					AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

					// Configure shader for object space lighting.
					re.GLSL().FarRingworldLitCloudsShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR,
							sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
							ShadowPanelSunAngle(sun.m_ShadowAngle),
							sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
				}

				// Render the planet.
				m_Clouds.SelectGL();
				m_Clouds.DrawGL();
				m_Clouds.DeselectGL();

				re.GLSL().FarRingworldLitCloudsShader().Deselect();
			}
			else
			{
				// Draw world without clouds.
				// Configure far globe shader. All planet details are blended into a single surface texture.
				re.GLSL().FarRingworldLitShader().Select();
				re.GLSL().FarRingworldLitShader().SetProjectionModelView(projection * model);
				re.GLSL().FarRingworldLitShader().SetAtmosphereColor(atmColor);
				re.GLSL().FarRingworldLitShader().SetPlanetTexture(m_glTexFarSurface);

				// Ambient light
				re.GLSL().FarRingworldLitShader().SetAmbientLight(atmIllumination);

				// Suns
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
				{
					// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
					const AuGLSun& sun = re.Sun(sunIndex);
					AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
					AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
					AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

					// Configure shader for object space lighting.
					re.GLSL().FarRingworldLitShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
							sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
							ShadowPanelSunAngle(sun.m_ShadowAngle),
							sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
				}

				// Render the planet.
				m_Clouds.SelectGL();
				m_Clouds.DrawGL();
				m_Clouds.DeselectGL();

				re.GLSL().FarRingworldLitShader().Deselect();
			}
		}
		break;
	}
}

void DaWorldRingSection::RenderTerrainStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale,
		AuGLEffects& re, float ambientLight, float fogDensity, float fogR, float fogG, float fogB, bool isActive)
{
	if(!m_Clouds.VAO())
		CreateCloudsVAO(re);

	// Configure far globe shader. All planet details are blended into a single surface texture.
	re.GLSL().RingworldTerrainStatueShader().Select();
	re.GLSL().RingworldTerrainStatueShader().SetProjectionModelView(projection * model);
	re.GLSL().RingworldTerrainStatueShader().SetTexture(m_glTexFarSurface);
	re.GLSL().RingworldTerrainStatueShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().RingworldTerrainStatueShader().SetAmbientLight(ambientLight);

	// Suns
	DaStar* star = 0;
	const DaWorldRingSection* refSection = 0;
	if(isActive)
	{
		// Place a single mini sun at the center of the ringworld statue.
		star = FindParentStarInEnv();
		Q_ASSERT(star);
		refSection = star->FindRingworldReferenceSection();
		AuPoint sunLoc(0, 0, OrbitRadius());									// Statue sun location relative to this arc section.
		AuPoint sunXVec = Transform().UnProjectVector(AuPoint(1,0,0));			// Sun's X vector relative to this arc section.
		AuPoint sunYVec = Transform().UnProjectVector(AuPoint(0,1,0));			// Sun's Y vector relative to this arc section.
		float attenuation = 1.f/(OrbitRadius() * OrbitRadius());
		re.GLSL().RingworldTerrainStatueShader().SetSun(0, true, attenuation,
				star->SunColor().redF(), star->SunColor().greenF(), star->SunColor().blueF(),
				sunLoc.x, sunLoc.y, sunLoc.z, star->Radius() * statueScale,
				ShadowPanelSunAngle(refSection->AngleOfRotation()), refSection->RingworldSections(),
				sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
	}
	else
	{
		// Use real suns for lighting.
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = re.Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
			AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
			AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);
			sunXVec.Unit();
			sunYVec.Unit();

			// Configure shader for object space lighting.
			// Must scale the attenuation of the light. In the scaled statue's object space, that sun is a long way away.
			// Must scale the radius of the light. At its scaled distance, the radius is pretty tiny.
			re.GLSL().RingworldTerrainStatueShader().SetSun(sunIndex, sun.m_Enabled, sun.Attenuation(statueScale), sun.m_ColorR,
					sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius/statueScale,
					ShadowPanelSunAngle(sun.m_ShadowAngle),
					sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
		}
	}

	// Point lights
	for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
	{
		// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLLight& light = re.Light(lightIndex);
		AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
		AuPoint spotDir;
		if(!light.m_Direction.IsZero())
		{
			spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.
			spotDir.Unit();
		}

		// Configure shader for object space lighting.
		// Must scale the attenuation of the light. In the scaled statue's object space, that light is a long way away.
		re.GLSL().RingworldTerrainStatueShader().SetLight(lightIndex, light.m_bEnabled, light.kL(statueScale),
				light.kQ(statueScale), light.m_ColorR, light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
				light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
	}

	// Render the surface terrain statue.
	m_Clouds.SelectGL();
	m_Clouds.DrawGL();
	m_Clouds.DeselectGL();

	re.GLSL().RingworldTerrainStatueShader().Deselect();

	// Render the star at the command center location on section 0 as a "You Are Here" indication.
	if(isActive && !SectionIndex() && star && refSection)
	{
		double sunscale = TerrainYSize() / star->Radius();
		sunscale /= 5;
		AuMatrix4x4 sunModel(model);
		sunModel.Scale(sunscale, sunscale, sunscale);
		star->RenderSun(projection, sunModel, re, 1.f);
	}
}

void DaWorldRingSection::CreateShadowSquareVAO(AuOpenGLFunctions& GLf)
{
	Q_ASSERT(!m_ShadowSquare.VAO());
	// Create a curving slab for the shadow square that is 1/2 the arc length of this ringworld section.
	QList<AuGlobeTriTexN> tris;												// Triangles that make up the shadow square model.

	double orbitRad = OrbitRadius();
	double shadowZIn = orbitRad/2;											// Dist from sun to inside face of shadow square.
	double shadowZOut = orbitRad/2 + 100*FeetPerMeter();					// Dist from sun to outside face of shadow square.
	double ringY2 = TerrainYSize()/2;
	const DaStar* star = FindParentStarInEnv();
	double shadowY2 = (star->Radius() - ringY2) / 2 + ringY2;				// 1/2 width of shadow square 1/2 way to the sun
	int shadowSections = 10;												// Number of sections in the shadow square.
	double shadSectAng = (SectionAngle()/2.) / (double)shadowSections;		// Sweep angle of each section of the shadow square.
	AuPoint botInPrev;
	AuPoint topInPrev;
	AuPoint botOutPrev;
	AuPoint topOutPrev;
	AuPoint nOutPrev;
	float texXPrev = 0;
	for(int sect=0; sect<=shadowSections; sect++)
	{
		double ang = shadSectAng * sect - SectionAngle()/4;
		AuPoint nOut = AuPoint(0,0,-1).RotateY(ang);
		AuPoint cenIn = AuPoint(0,0,orbitRad) + nOut * shadowZIn; 
		AuPoint cenOut = AuPoint(0,0,orbitRad) + nOut * shadowZOut;
		AuPoint botIn = cenIn - AuPoint(0,shadowY2);
		AuPoint topIn = cenIn + AuPoint(0,shadowY2);
		AuPoint botOut = cenOut - AuPoint(0,shadowY2);
		AuPoint topOut = cenOut + AuPoint(0,shadowY2);
		float texX = (float)sect / (float)shadowSections;

		if(sect == 0)
		{
			// Cap the first end.
			AuPoint nEnd = nOut.RotateY(-PI/2);
			AuGlobeTriTexN triEnd1;
			triEnd1.m_Vert0 = AuGlobeVertTexN(botIn, QPointF(0,0), nEnd);
			triEnd1.m_Vert1 = AuGlobeVertTexN(topIn, QPointF(0,1), nEnd);
			triEnd1.m_Vert2 = AuGlobeVertTexN(topOut, QPointF(1,1), nEnd);
			tris.append(triEnd1);

			AuGlobeTriTexN triEnd2;
			triEnd2.m_Vert0 = triEnd1.m_Vert2;
			triEnd2.m_Vert1 = AuGlobeVertTexN(botOut, QPointF(1,0), nEnd);
			triEnd2.m_Vert2 = triEnd1.m_Vert0;
			tris.append(triEnd2);
		}
		else
		{
			// Create a section of the shadow square.
			// Outside face
			AuGlobeTriTexN triOut1;
			triOut1.m_Vert0 = AuGlobeVertTexN(botOutPrev, QPointF(texXPrev,0), nOutPrev);
			triOut1.m_Vert1 = AuGlobeVertTexN(topOutPrev, QPointF(texXPrev,1), nOutPrev);
			triOut1.m_Vert2 = AuGlobeVertTexN(topOut, QPointF(texX,1), nOut);
			tris.append(triOut1);

			AuGlobeTriTexN triOut2;
			triOut2.m_Vert0 = triOut1.m_Vert2;
			triOut2.m_Vert1 = AuGlobeVertTexN(botOut, QPointF(texX,0), nOut);
			triOut2.m_Vert2 = triOut1.m_Vert0;
			tris.append(triOut2);

			// Inside face
			AuGlobeTriTexN triIn1;
			triIn1.m_Vert0 = AuGlobeVertTexN(topInPrev, QPointF(texXPrev,1), -nOutPrev);
			triIn1.m_Vert1 = AuGlobeVertTexN(botInPrev, QPointF(texXPrev,0), -nOutPrev);
			triIn1.m_Vert2 = AuGlobeVertTexN(botIn, QPointF(texX,0), -nOut);
			tris.append(triIn1);

			AuGlobeTriTexN triIn2;
			triIn2.m_Vert0 = triIn1.m_Vert2;
			triIn2.m_Vert1 = AuGlobeVertTexN(topIn, QPointF(texX,1), -nOut);
			triIn2.m_Vert2 = triIn1.m_Vert0;
			tris.append(triIn2);

			// Top face
			AuGlobeTriTexN triTop1;
			triTop1.m_Vert0 = AuGlobeVertTexN(topOutPrev, QPointF(texXPrev,0), AuPoint(0,1));
			triTop1.m_Vert1 = AuGlobeVertTexN(topInPrev, QPointF(texXPrev,1), AuPoint(0,1));
			triTop1.m_Vert2 = AuGlobeVertTexN(topIn, QPointF(texX,1), AuPoint(0,1));
			tris.append(triTop1);

			AuGlobeTriTexN triTop2;
			triTop2.m_Vert0 = triTop1.m_Vert2;
			triTop2.m_Vert1 = AuGlobeVertTexN(topOut, QPointF(texX,0), AuPoint(0,1));
			triTop2.m_Vert2 = triTop1.m_Vert0;
			tris.append(triTop2);

			// Bottom face
			AuGlobeTriTexN triBot1;
			triBot1.m_Vert0 = AuGlobeVertTexN(botInPrev, QPointF(texXPrev,0), AuPoint(0,-1));
			triBot1.m_Vert1 = AuGlobeVertTexN(botOutPrev, QPointF(texXPrev,1), AuPoint(0,-1));
			triBot1.m_Vert2 = AuGlobeVertTexN(botOut, QPointF(texX,1), AuPoint(0,-1));
			tris.append(triBot1);

			AuGlobeTriTexN triBot2;
			triBot2.m_Vert0 = triBot1.m_Vert2;
			triBot2.m_Vert1 = AuGlobeVertTexN(botIn, QPointF(texX,0), AuPoint(0,-1));
			triBot2.m_Vert2 = triBot1.m_Vert0;
			tris.append(triBot2);

			if(sect == shadowSections)
			{
				// Cap the end.
				AuPoint nEnd = nOut.RotateY(PI/2);
				AuGlobeTriTexN triEnd1;
				triEnd1.m_Vert0 = AuGlobeVertTexN(botOut, QPointF(0,0), nEnd);
				triEnd1.m_Vert1 = AuGlobeVertTexN(topOut, QPointF(0,1), nEnd);
				triEnd1.m_Vert2 = AuGlobeVertTexN(topIn, QPointF(1,1), nEnd);
				tris.append(triEnd1);

				AuGlobeTriTexN triEnd2;
				triEnd2.m_Vert0 = triEnd1.m_Vert2;
				triEnd2.m_Vert1 = AuGlobeVertTexN(botIn, QPointF(1,0), nEnd);
				triEnd2.m_Vert2 = triEnd1.m_Vert0;
				tris.append(triEnd2);
				break;
			}
		}

		botInPrev = botIn;
		topInPrev = topIn;
		botOutPrev = botOut;
		topOutPrev = topOut;
		nOutPrev = nOut;
		texXPrev = texX;
	}

	// Store count of triangles in the vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeN = new GLfloat[vertices*3];				// normals for triangles that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*2];			// texture coords for each of the vertices

	unsigned int globePosIdx = 0;
	unsigned int globeNIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTexN>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		// Vert 0
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

		// Vert 1
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

		// Vert 2
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
	}

	// Generate vertex array(s) for this object. We only need one.
	m_ShadowSquare.CreateGL(GLf, vertices, 3);

	// Select vertex array to construct.
	m_ShadowSquare.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_ShadowSquare.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_ShadowSquare.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_ShadowSquare.Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_ShadowSquare.DeselectGL();

	delete [] globePos;
	delete [] globeN;
	delete [] globeTex;
}

void DaWorldRingSection::RenderShadowSquare(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		float ambientLight)
{
	if(!m_ShadowSquare.VAO())
		CreateShadowSquareVAO(re);

	// Position the panel according to its current rotation around the sun.
	AuMatrix4x4 panelM(model);
	AuPoint panelVec(0,0,-OrbitRadius());
	panelM.Translate(-panelVec);
	panelM.RotateY(m_RotationAngle);
	panelM.Translate(panelVec);

	// Configure far globe shader. All planet details are blended into a single surface texture.
	re.GLSL().ShadowSquareShader().Select();
	re.GLSL().ShadowSquareShader().SetProjectionModelView(projection * panelM);
	re.GLSL().ShadowSquareShader().SetTexture(s_TexRingworld);

	// Ambient light
	re.GLSL().ShadowSquareShader().SetAmbientLight(ambientLight);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = re.Sun(sunIndex);
		AuPoint sunLoc = panelM.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

		// Configure shader for object space lighting.
		re.GLSL().ShadowSquareShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
	}

	// Render the shadow square.
	m_ShadowSquare.SelectGL();
	m_ShadowSquare.DrawGL();
	m_ShadowSquare.DeselectGL();

	re.GLSL().ShadowSquareShader().Deselect();
}

void DaWorldRingSection::RenderShadowSquareStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale,
		AuGLEffects& re, float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	if(!m_ShadowSquare.VAO())
		CreateShadowSquareVAO(re);

	// Position the panel according to its current rotation around the sun.
	AuMatrix4x4 panelM(model);
	AuPoint panelVec(0,0,-OrbitRadius());
	panelM.Translate(-panelVec);
	panelM.RotateY(m_RotationAngle);
	panelM.Translate(panelVec);

	// Configure far globe shader. All planet details are blended into a single surface texture.
	re.GLSL().RingworldStatueShader().Select();
	re.GLSL().RingworldStatueShader().SetProjectionModelView(projection * panelM);
	re.GLSL().RingworldStatueShader().SetTexture(s_TexRingworld);
	re.GLSL().RingworldStatueShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().RingworldStatueShader().SetAmbientLight(ambientLight);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = re.Sun(sunIndex);
		AuPoint sunLoc = panelM.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
		AuPoint sunXVec = panelM.UnProjectVector(sun.m_XVec);
		AuPoint sunYVec = panelM.UnProjectVector(sun.m_YVec);
		sunXVec.Unit();
		sunYVec.Unit();

		// Configure shader for object space lighting.
		// Must scale the attenuation of the light. In the scaled statue's object space, that sun is a long way away.
		// Must scale the radius of the light. At its scaled distance, the radius is pretty tiny.
		re.GLSL().RingworldStatueShader().SetSun(sunIndex, sun.m_Enabled, sun.Attenuation(statueScale), sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius/statueScale,
				ShadowPanelSunAngle(sun.m_ShadowAngle),
				sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
	}

	// Point lights
	for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
	{
		// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLLight& light = re.Light(lightIndex);
		AuPoint lightLoc = panelM.UnProject(light.m_Position);					// Light location relative to me.
		AuPoint spotDir;
		if(!light.m_Direction.IsZero())
		{
			spotDir = panelM.UnProjectVector(light.m_Direction);					// Light direction relative to me.
			spotDir.Unit();
		}

		// Configure shader for object space lighting.
		// Must scale the attenuation of the light. In the scaled statue's object space, that light is a long way away.
		re.GLSL().RingworldStatueShader().SetLight(lightIndex, light.m_bEnabled, light.kL(statueScale), light.kQ(statueScale),
				light.m_ColorR, light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
				light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
	}

	// Render the shadow square.
	m_ShadowSquare.SelectGL();
	m_ShadowSquare.DrawGL();
	m_ShadowSquare.DeselectGL();

	re.GLSL().RingworldStatueShader().Deselect();
}

void DaWorldRingSection::CreateHullVAO(AuOpenGLFunctions& GLf)
{
	Q_ASSERT(!m_Hull.VAO());

	// Create a curving slab for the shadow square that is 1/2 the arc length of this ringworld section.
	QList<AuGlobeTriTexN> tris;												// Triangles that make up the shadow square model.

	// Terrain is not allowed to go farther down than 3 AltitudeIncrements below MSL, see VirginTerrainAltitudeMSL.
	// An altitude increment on a ringworld section is about 373ft(114m).
	// A 4 level mine built at the bottom of the sea (?) could reach 8 levels below that. That is 64' at 8' per level so the
	// deepest mine will fit into one terrain level.
	// The atmosphere envelope of the ring ends at four increments below MSL, as returned by MinRadius.
	// The hull will be one terrain altitude increment farther than that (5), so people outside can't clip into that atmosphere
	// envelope due to polygon tessellations of the hull.
	double ringY2 = TerrainYSize()/2;
	double orbitRad = OrbitRadius();
	double hullWallBotMSL = MinRadius();									// MSL altitude (-) at atmosphere envelope radius.
	double hullWallTopMSL = AtmosphereHeightMSL() + 100*FeetPerMeter();
	double hullY2 = ringY2 + 100*FeetPerMeter();							// Y offset to outside of hull wall.
	double hullMSL = -AltitudeIncrement() * 5;								// MSL of outer hull.
	int hullSections = 20;													// Number of sections in the hull.
	double hullSectAng = SectionAngle() / (double)hullSections;				// Sweep angle of each section of the hull.
	float texXPrev = 0;
	AuPoint swbiP;					// south wall bottom inside, previous iteration
	AuPoint swtiP;					// south wall top inside, previous iteration
	AuPoint swtoP;					// south wall top outside, previous iteration
	AuPoint swboP;					// south wall bottom outside, previous iteration
	AuPoint nwbiP;					// north wall bottom inside, previous iteration
	AuPoint nwtiP;					// north wall top inside, previous iteration
	AuPoint nwtoP;					// north wall top outside, previous iteration
	AuPoint nwboP;					// north wall bottom outside, previous iteration
	AuPoint nUpPrev;
	for(int sect=0; sect<=hullSections; sect++)
	{
		double ang = hullSectAng * sect - SectionAngle()/2;
		float texX = texXPrev + 1;
		if(texX > 1)
			texX = 0;
		AuPoint nUp = AuPoint(0,0,1).RotateY(ang);							// normal pointing toward the sun
		AuPoint cen = AuPoint(0,0,orbitRad) - nUp * orbitRad; 				// Center line of ringworld at sea level.

		AuPoint swbi = cen + AuPoint(0, -ringY2) + nUp * hullWallBotMSL;	// south wall bottom inside
		AuPoint swti = cen + AuPoint(0, -ringY2) + nUp * hullWallTopMSL;	// south wall top inside
		AuPoint swto = cen + AuPoint(0, -hullY2) + nUp * hullWallTopMSL;	// south wall top outside
		AuPoint swbo = cen + AuPoint(0, -hullY2) + nUp * hullMSL;			// south wall bottom outside
		AuPoint nwbi = cen + AuPoint(0, ringY2) + nUp * hullWallBotMSL;		// north wall bottom inside
		AuPoint nwti = cen + AuPoint(0, ringY2) + nUp * hullWallTopMSL;		// north wall top inside
		AuPoint nwto = cen + AuPoint(0, hullY2) + nUp * hullWallTopMSL;		// north wall top outside
		AuPoint nwbo = cen + AuPoint(0, hullY2) + nUp * hullMSL;			// north wall bottom outside

		if(sect)
		{
			// Create a section of the hull.
			// Hull Outside
			AuGlobeTriTexN ho1;
			ho1.m_Vert0 = AuGlobeVertTexN(swboP, QPointF(texXPrev,0), -nUpPrev);
			ho1.m_Vert1 = AuGlobeVertTexN(nwboP, QPointF(texXPrev,1), -nUpPrev);
			ho1.m_Vert2 = AuGlobeVertTexN(nwbo, QPointF(texX,1), -nUp);
			tris.append(ho1);

			AuGlobeTriTexN ho2;
			ho2.m_Vert0 = ho1.m_Vert2;
			ho2.m_Vert1 = AuGlobeVertTexN(swbo, QPointF(texX,0), -nUp);
			ho2.m_Vert2 = ho1.m_Vert0;
			tris.append(ho2);

			// North Wall Outside
			AuGlobeTriTexN nwo1;
			nwo1.m_Vert0 = AuGlobeVertTexN(nwboP, QPointF(texXPrev,0), AuPoint(0,1));
			nwo1.m_Vert1 = AuGlobeVertTexN(nwtoP, QPointF(texXPrev,1), AuPoint(0,1));
			nwo1.m_Vert2 = AuGlobeVertTexN(nwto, QPointF(texX,1), AuPoint(0,1));
			tris.append(nwo1);

			AuGlobeTriTexN nwo2;
			nwo2.m_Vert0 = nwo1.m_Vert2;
			nwo2.m_Vert1 = AuGlobeVertTexN(nwbo, QPointF(texX,0), AuPoint(0,1));
			nwo2.m_Vert2 = nwo1.m_Vert0;
			tris.append(nwo2);

			// South Wall Outside
			AuGlobeTriTexN swo1;
			swo1.m_Vert0 = AuGlobeVertTexN(swtoP, QPointF(texXPrev,0), AuPoint(0,-1));
			swo1.m_Vert1 = AuGlobeVertTexN(swboP, QPointF(texXPrev,1), AuPoint(0,-1));
			swo1.m_Vert2 = AuGlobeVertTexN(swbo, QPointF(texX,1), AuPoint(0,-1));
			tris.append(swo1);

			AuGlobeTriTexN swo2;
			swo2.m_Vert0 = swo1.m_Vert2;
			swo2.m_Vert1 = AuGlobeVertTexN(swto, QPointF(texX,0), AuPoint(0,-1));
			swo2.m_Vert2 = swo1.m_Vert0;
			tris.append(swo2);

			// North Wall Top
			AuGlobeTriTexN nwt1;
			nwt1.m_Vert0 = AuGlobeVertTexN(nwtoP, QPointF(texXPrev,0), nUpPrev);
			nwt1.m_Vert1 = AuGlobeVertTexN(nwtiP, QPointF(texXPrev,1), nUpPrev);
			nwt1.m_Vert2 = AuGlobeVertTexN(nwti, QPointF(texX,1), nUp);
			tris.append(nwt1);

			AuGlobeTriTexN nwt2;
			nwt2.m_Vert0 = nwt1.m_Vert2;
			nwt2.m_Vert1 = AuGlobeVertTexN(nwto, QPointF(texX,0), nUp);
			nwt2.m_Vert2 = nwt1.m_Vert0;
			tris.append(nwt2);

			// South Wall Top
			AuGlobeTriTexN swt1;
			swt1.m_Vert0 = AuGlobeVertTexN(swtiP, QPointF(texXPrev,0), nUpPrev);
			swt1.m_Vert1 = AuGlobeVertTexN(swtoP, QPointF(texXPrev,1), nUpPrev);
			swt1.m_Vert2 = AuGlobeVertTexN(swto, QPointF(texX,1), nUp);
			tris.append(swt1);

			AuGlobeTriTexN swt2;
			swt2.m_Vert0 = swt1.m_Vert2;
			swt2.m_Vert1 = AuGlobeVertTexN(swti, QPointF(texX,0), nUp);
			swt2.m_Vert2 = swt1.m_Vert0;
			tris.append(swt2);

			// North Wall Inside
			AuGlobeTriTexN nwi1;
			nwi1.m_Vert0 = AuGlobeVertTexN(nwtiP, QPointF(texXPrev,0), AuPoint(0,-1));
			nwi1.m_Vert1 = AuGlobeVertTexN(nwbiP, QPointF(texXPrev,1), AuPoint(0,-1));
			nwi1.m_Vert2 = AuGlobeVertTexN(nwbi, QPointF(texX,1), AuPoint(0,-1));
			tris.append(nwi1);

			AuGlobeTriTexN nwi2;
			nwi2.m_Vert0 = nwi1.m_Vert2;
			nwi2.m_Vert1 = AuGlobeVertTexN(nwti, QPointF(texX,0), AuPoint(0,-1));
			nwi2.m_Vert2 = nwi1.m_Vert0;
			tris.append(nwi2);

			// South Wall Inside
			AuGlobeTriTexN swi1;
			swi1.m_Vert0 = AuGlobeVertTexN(swbiP, QPointF(texXPrev,0), AuPoint(0,1));
			swi1.m_Vert1 = AuGlobeVertTexN(swtiP, QPointF(texXPrev,1), AuPoint(0,1));
			swi1.m_Vert2 = AuGlobeVertTexN(swti, QPointF(texX,1), AuPoint(0,1));
			tris.append(swi1);

			AuGlobeTriTexN swi2;
			swi2.m_Vert0 = swi1.m_Vert2;
			swi2.m_Vert1 = AuGlobeVertTexN(swbi, QPointF(texX,0), AuPoint(0,1));
			swi2.m_Vert2 = swi1.m_Vert0;
			tris.append(swi2);
		}

		nUpPrev = nUp;
		texXPrev = texX;
		swbiP = swbi;				// south wall bottom inside, previous iteration
		swtiP = swti;				// south wall top inside, previous iteration
		swtoP = swto;				// south wall top outside, previous iteration
		swboP = swbo;				// south wall bottom outside, previous iteration
		nwbiP = nwbi;				// north wall bottom inside, previous iteration
		nwtiP = nwti;				// north wall top inside, previous iteration
		nwtoP = nwto;				// north wall top outside, previous iteration
		nwboP = nwbo;				// north wall bottom outside, previous iteration
	}

	// Store count of triangles in the vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeN = new GLfloat[vertices*3];				// normals for triangles that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*2];			// texture coords for each of the vertices

	unsigned int globePosIdx = 0;
	unsigned int globeNIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTexN>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		// Vert 0
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

		// Vert 1
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

		// Vert 2
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
	}

	// Generate vertex array(s) for this object. We only need one.
	m_Hull.CreateGL(GLf, vertices, 3);

	// Select vertex array to construct.
	m_Hull.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Hull.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Hull.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_Hull.Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_Hull.DeselectGL();

	delete [] globePos;
	delete [] globeN;
	delete [] globeTex;
}

void DaWorldRingSection::RenderHull(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight,
		float fogDensity, float fogR, float fogG, float fogB)
{
	if(!m_Hull.VAO())
		CreateHullVAO(re);

	m_Hull.SelectGL();

	// Configure hull shader.
	if(fogDensity > 0.f)
	{
		re.GLSL().RingworldHullFogShader().Select();
		re.GLSL().RingworldHullFogShader().SetProjectionModelView(projection * model);
		re.GLSL().RingworldHullFogShader().SetTexture(s_TexRingworld);
		re.GLSL().RingworldHullFogShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Ambient light
		re.GLSL().RingworldHullFogShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = re.Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
			AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
			AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

			// Configure shader for object space lighting.
			re.GLSL().RingworldHullFogShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
					ShadowPanelSunAngle(sun.m_ShadowAngle),
					sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
		}

		// Render the hull.
		m_Hull.DrawGL();

		re.GLSL().RingworldHullFogShader().Deselect();
	}
	else
	{
		re.GLSL().RingworldHullShader().Select();
		re.GLSL().RingworldHullShader().SetProjectionModelView(projection * model);
		re.GLSL().RingworldHullShader().SetTexture(s_TexRingworld);

		// Ambient light
		re.GLSL().RingworldHullShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = re.Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
			AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
			AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

			// Configure shader for object space lighting.
			re.GLSL().RingworldHullShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
					ShadowPanelSunAngle(sun.m_ShadowAngle),
					sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
		}

		// Render the shadow square.
		m_Hull.DrawGL();

		re.GLSL().RingworldHullShader().Deselect();
	}

	m_Hull.DeselectGL();
}

void DaWorldRingSection::RenderHullStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale,
		AuGLEffects& re, float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	if(!m_Hull.VAO())
		CreateHullVAO(re);

	// Configure statue shader.
	re.GLSL().RingworldStatueShader().Select();
	re.GLSL().RingworldStatueShader().SetProjectionModelView(projection * model);
	re.GLSL().RingworldStatueShader().SetTexture(s_TexRingworld);
	re.GLSL().RingworldStatueShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().RingworldStatueShader().SetAmbientLight(ambientLight);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = re.Sun(sunIndex);
		AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
		AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
		AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);
		sunXVec.Unit();
		sunYVec.Unit();

		// Configure shader for object space lighting.
		// Must scale the attenuation of the light. In the scaled statue's object space, that sun is a long way away.
		// Must scale the radius of the light. At its scaled distance, the radius is pretty tiny.
		re.GLSL().RingworldStatueShader().SetSun(sunIndex, sun.m_Enabled, sun.Attenuation(statueScale), sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius/statueScale,
				ShadowPanelSunAngle(sun.m_ShadowAngle),
				sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
	}

	// Point lights
	for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
	{
		// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLLight& light = re.Light(lightIndex);
		AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
		AuPoint spotDir;
		if(!light.m_Direction.IsZero())
		{
			spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.
			spotDir.Unit();
		}

		// Configure shader for object space lighting.
		// Must scale the attenuation of the light. In the scaled statue's object space, that light is a long way away.
		re.GLSL().RingworldStatueShader().SetLight(lightIndex, light.m_bEnabled, light.kL(statueScale), light.kQ(statueScale),
				light.m_ColorR, light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
				light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
	}

	// Render the hull.
	m_Hull.SelectGL();
	m_Hull.DrawGL();
	m_Hull.DeselectGL();

	re.GLSL().RingworldStatueShader().Deselect();
}

void DaWorldRingSection::RenderControlCenter(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		float ambientLight, float fogDensity, float fogR, float fogG, float fogB, bool isPreonPresent)
{
	// Calculate the center of the control center. This is the ground level location beneath the center of the dais.
	AuTime newest;
	WORLDTEX_TYPE pavementType = WORLDTEX_Count;
	AuPoint loc = LocOfLL(AuGlobeLL(0,0), VirginTerrainAltitudeMSL(AuGlobeLL(0,0), pavementType, newest));

	if(!m_Obelisk.VAO())
	{
		// Create an obelisk.
		QList<AuTriNTex> tris;													// Triangles that make up the model.

		// Create a single model of 4 obelisks arranged in a circle on a raised area.
		const double daisRad = 10.*FeetPerMeter();				// Control center dais radius (loosely, this is a square).
		const double baseRad = 1.*FeetPerMeter();
		const double shoulderRad = .5*FeetPerMeter();
		const double shoulderHt = 10*FeetPerMeter();
		const double peakHt = shoulderHt + 1.*FeetPerMeter();
		const double daisHt = 2*FeetPerMeter();
		const double gndRad = daisRad + daisHt*2;				// Ground level radius of dais.
		for(int ob=0; ob<4; ob++)
		{
			// Make a transformation matrix for the points of the obelisk.
			double angle = (PI*2) * ob / 4;
			AuMatrix4x4 tob;
			tob.Translate(loc);
			tob.RotateZ(angle);

			// Make a quadrant of the dais.
			{
				// Ground level points.
				AuPoint g1 = tob.Project(0,gndRad,0);
				AuPoint g2 = tob.Project(gndRad,gndRad,0);
				AuPoint g3 = tob.Project(gndRad,0,0);

				// Dais level points.
				AuPoint d0 = tob.Project(0,0,daisHt);
				AuPoint d1 = tob.Project(0,daisRad,daisHt);
				AuPoint d2 = tob.Project(daisRad,daisRad,daisHt);
				AuPoint d3 = tob.Project(daisRad,0,daisHt);

				// Make triangles.
				// Top
				tris.append(AuTriNTex(d0, QPointF(0,0), d1, QPointF(0,.49), d2, QPointF(1,.49)));
				tris.append(AuTriNTex(d2, QPointF(1,.49), d3, QPointF(1,0), d0, QPointF(0,0)));
				// Slopes
				tris.append(AuTriNTex(d1, QPointF(0,0), g1, QPointF(0,.49), g2, QPointF(1,.49)));
				tris.append(AuTriNTex(g2, QPointF(1,.49), d2, QPointF(1,0), d1, QPointF(0,0)));
				tris.append(AuTriNTex(d2, QPointF(0,0), g2, QPointF(0,.49), g3, QPointF(1,.49)));
				tris.append(AuTriNTex(g3, QPointF(1,.49), d3, QPointF(1,0), d2, QPointF(0,0)));
			}

			// Create an obelisk model.
			tob.Translate(daisRad - baseRad, daisRad - baseRad, daisHt);
			{
				// Base points
				AuPoint b0 = tob.Project(-baseRad, -baseRad, 0);
				AuPoint b1 = tob.Project(-baseRad, baseRad, 0);
				AuPoint b2 = tob.Project(baseRad, baseRad, 0);
				AuPoint b3 = tob.Project(baseRad, -baseRad, 0);
				// Top points
				AuPoint t0 = tob.Project(-shoulderRad, -shoulderRad, shoulderHt);
				AuPoint t1 = tob.Project(-shoulderRad, shoulderRad, shoulderHt);
				AuPoint t2 = tob.Project(shoulderRad, shoulderRad, shoulderHt);
				AuPoint t3 = tob.Project(shoulderRad, -shoulderRad, shoulderHt);
				// Peak point
				AuPoint peak = tob.Project(0, 0, peakHt);

				// Texture X wraps the texture onto 4 sides.
				float tx0 = 0.0f;
				float tx1 = 0.25f;
				float tx2 = 0.5f;
				float tx3 = 0.75f;
				float tx4 = 1.0f;

				// Make triangles.
				// Front side
				tris.append(AuTriNTex(b0, QPointF(tx0,.5), t0, QPointF(tx0,1), t3, QPointF(tx1,1)));
				tris.append(AuTriNTex(t3, QPointF(tx1,1), b3, QPointF(tx1,.5), b0, QPointF(tx0,.5)));
				tris.append(AuTriNTex(t0, QPointF(.45,.45/2+.5), peak, QPointF(.5,.5/2+.5), t3, QPointF(.65,.45/2+.5)));
				// Left side
				tris.append(AuTriNTex(b1, QPointF(tx3,.5), t1, QPointF(tx3,1), t0, QPointF(tx4,1)));
				tris.append(AuTriNTex(t0, QPointF(tx4,1), b0, QPointF(tx4,.5), b1, QPointF(tx3,.5)));
				tris.append(AuTriNTex(t1, QPointF(.45,.65/2+.5), peak, QPointF(.5,.5/2+.5), t0, QPointF(.45,.45/2+.5)));
				// Back side
				tris.append(AuTriNTex(b2, QPointF(tx2,.5), t2, QPointF(tx2,1), t1, QPointF(tx3,1)));
				tris.append(AuTriNTex(t1, QPointF(tx3,1), b1, QPointF(tx3,.5), b2, QPointF(tx2,.5)));
				tris.append(AuTriNTex(t2, QPointF(.65,.65/2+.5), peak, QPointF(.5,.5/2+.5), t1, QPointF(.45,.65/2+.5)));
				// Right side
				tris.append(AuTriNTex(b3, QPointF(tx1,.5), t3, QPointF(tx1,1), t2, QPointF(tx2,1)));
				tris.append(AuTriNTex(t2, QPointF(tx2,1), b2, QPointF(tx2,.5), b3, QPointF(tx1,.5)));
				tris.append(AuTriNTex(t3, QPointF(.65,.45/2+.5), peak, QPointF(.5,.5/2+.5), t2, QPointF(.65,.65/2+.5)));
			}
		}

		// Store count of triangles in the vertex array object.
		GLuint vertices = tris.count()*3;

		// Copy surface triangles into gl array buffers.
		GLfloat* pos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
		GLfloat* surfN = new GLfloat[vertices*3];				// normals for triangles that make up the model.
		GLfloat* tex = new GLfloat[vertices*2];			// texture coords for each of the vertices

		unsigned int posIdx = 0;
		unsigned int surfNIdx = 0;
		unsigned int texIdx = 0;
		for(QList<AuTriNTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
		{
			// Vert 0
			pos[posIdx++] = (float)(*vi).m_V0.x;
			pos[posIdx++] = (float)(*vi).m_V0.y;
			pos[posIdx++] = (float)(*vi).m_V0.z;

			surfN[surfNIdx++] = (float)(*vi).m_N.x;
			surfN[surfNIdx++] = (float)(*vi).m_N.y;
			surfN[surfNIdx++] = (float)(*vi).m_N.z;

			tex[texIdx++] = (*vi).m_V0.m_Tx;
			tex[texIdx++] = (*vi).m_V0.m_Ty;

			// Vert 1
			pos[posIdx++] = (float)(*vi).m_V1.x;
			pos[posIdx++] = (float)(*vi).m_V1.y;
			pos[posIdx++] = (float)(*vi).m_V1.z;

			surfN[surfNIdx++] = (float)(*vi).m_N.x;
			surfN[surfNIdx++] = (float)(*vi).m_N.y;
			surfN[surfNIdx++] = (float)(*vi).m_N.z;

			tex[texIdx++] = (*vi).m_V1.m_Tx;
			tex[texIdx++] = (*vi).m_V1.m_Ty;

			// Vert 2
			pos[posIdx++] = (float)(*vi).m_V2.x;
			pos[posIdx++] = (float)(*vi).m_V2.y;
			pos[posIdx++] = (float)(*vi).m_V2.z;

			surfN[surfNIdx++] = (float)(*vi).m_N.x;
			surfN[surfNIdx++] = (float)(*vi).m_N.y;
			surfN[surfNIdx++] = (float)(*vi).m_N.z;

			tex[texIdx++] = (*vi).m_V2.m_Tx;
			tex[texIdx++] = (*vi).m_V2.m_Ty;
		}

		// Generate vertex array(s) for this object. We only need one.
		m_Obelisk.CreateGL(re, vertices, 3);

		// Select vertex array to construct.
		m_Obelisk.SelectGL();

		// bind buffer for vertices and copy data into buffer.
		re.glBindBuffer(GL_ARRAY_BUFFER, m_Obelisk.Buffer(0));
		re.glBufferData(GL_ARRAY_BUFFER, posIdx*sizeof(GLfloat), pos, GL_STATIC_DRAW);
		re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

		// bind buffer for colors and copy data into buffer.
		re.glBindBuffer(GL_ARRAY_BUFFER, m_Obelisk.Buffer(1));
		re.glBufferData(GL_ARRAY_BUFFER, surfNIdx*sizeof(GLfloat), surfN, GL_STATIC_DRAW);
		re.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

		// bind buffer for colors and copy data into buffer.
		re.glBindBuffer(GL_ARRAY_BUFFER, m_Obelisk.Buffer(2));
		re.glBufferData(GL_ARRAY_BUFFER, texIdx*sizeof(GLfloat), tex, GL_STATIC_DRAW);
		re.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

		re.glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Disable the vertex array object.
		m_Obelisk.DeselectGL();

		delete [] pos;
		delete [] surfN;
		delete [] tex;
	}

	m_Obelisk.SelectGL();

	// Configure statue shader.
	re.GLSL().RingworldStatueShader().Select();
	re.GLSL().RingworldStatueShader().SetProjectionModelView(projection * model);
	re.GLSL().RingworldStatueShader().SetTexture(s_TexObelisk);
	re.GLSL().RingworldStatueShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().RingworldStatueShader().SetAmbientLight(ambientLight);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = re.Sun(sunIndex);
		AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
		AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
		AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

		// Configure shader for object space lighting.
		re.GLSL().RingworldStatueShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
				ShadowPanelSunAngle(sun.m_ShadowAngle),
				sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
	}

	// Point lights
	for(unsigned int lightIndex=0; lightIndex<re.MaxLights(); lightIndex++)
	{
		// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLLight& light = re.Light(lightIndex);
		AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
		AuPoint spotDir;
		if(!light.m_Direction.IsZero())
			spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

		// Configure shader for object space lighting.
		re.GLSL().RingworldStatueShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
				light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
				light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
	}

	// Render the control center obelisks.
	m_Obelisk.DrawGL();

	re.GLSL().RingworldStatueShader().Deselect();

	m_Obelisk.DeselectGL();

	// Render a statue of the ringworld at the center.
	// 'loc' is the ground location under the center of the dais, which is 2m high.
	double statueScale = (2*FeetPerMeter()) / TerrainXSize();	// Scale for rendering the ringworld statue.
	double statueRadius = OrbitRadius() * statueScale;			// Radius of the scaled ringworld statue.
	AuPoint statueCen = loc - GravityDir(loc) * (statueRadius*.7071 + 3*FeetPerMeter());	// Low edge 1m above floor.
	AuMatrix4x4 statueM(model);
	statueM.Translate(statueCen);
	statueM.RotateX(-45*DEG2RAD);
	RenderRingworldStatue(projection, statueM, statueScale, re, ambientLight, fogDensity, fogR, fogG, fogB, isPreonPresent);
}

void DaWorldRingSection::RenderRingworldStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale,
		AuGLEffects& re, float ambientLight, float fogDensity, float fogR, float fogG, float fogB, bool isPreonPresent)
{
	DaStar* star = FindParentStarInEnv();
	if(!star)
		return;

	// We have to access these subatoms directly because they are already locked.
	if(!star->HasSubatoms())
		return;

	AuMatrix4x4 statueM(model);
	statueM.Scale(statueScale, statueScale, statueScale);

	int arcsections = RingworldSections();
	DaAtomList subs;
	star->ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldRingSection* ringarc = dynamic_cast<DaWorldRingSection*>(*it);
		if(!ringarc)
			continue;

		AuMatrix4x4 arcM(statueM);
		double rangle = ringarc->AngleToRingSection();
		arcM.RotateZ(rangle);								// Rotate the model space to the planet's orbit position.
		arcM.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Translate to the loc of this planet relative to the parent star.
		arcM.RotateZ(-rangle);								// Rotate the model space to the planet's orbit position.
		arcM.RotateX(PI/2.);								// Stand the ring section on its edge.
		arcM.RotateY(rangle - PI/2.);						// Orient to face the sun.

		ringarc->RenderHullStatue(projection, arcM, statueScale, re, ambientLight, fogDensity, fogR, fogG, fogB);

		ringarc->RenderTerrainStatue(projection, arcM, statueScale, re, ambientLight, fogDensity, fogR, fogG, fogB,
				m_ControlCenterActive || isPreonPresent);

		ringarc->RenderShadowSquareStatue(projection, arcM, statueScale, re, ambientLight, fogDensity, fogR, fogG, fogB);

		arcsections--;
		if(!arcsections)
			break;
	}

	// Render the sun at the center of the ringworld statue.
	if(m_ControlCenterActive || isPreonPresent)
		star->RenderSun(projection, statueM, re, 1.f);
}

// Lookup table for all worlds with vegetation.
// The resulting image is only 1 pixel tall in Y.
// The single row of pixels indicates the WORLDTEX_TYPE at each altitude interpolated across the range of MinRadius to PeakRadius.
// The WORLDTEX_TYPE is stored in the R (red) value of each pixel. An altitude factor is placed on each vertex of the model that is
// also interpolated in the range of MinRadius to PeakRadius. The fragment shader uses the value it receives, which gets
// interpolated between the relevant vertexes, to decide which texture tile in the terrain texture palette to use at that fragment's
// altitude. A little noise is mixed in to avoid perfectly straight lines along altitude boundaries on flat terrain polygons.
QImage DaWorldRingSection::CreateFarVegetationLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const
{
	// Select textures to use for the various terrain elevations.
	terElevTex[0] = s_TexTerPalette[WORLDTEX_RockGrayHeavy];	// TER_Peak, TER_Alpine, and TER_Highlands are ALWAYS the same.
	terElevTex[1] = s_TexTerPalette[WORLDTEX_DirtModerate];		// TER_Midlands
	terElevTex[2] = s_TexTerPalette[WORLDTEX_DirtCoarse];		// TER_Lowlands
	terElevTex[3] = s_TexTerPalette[WORLDTEX_DirtHeavy];		// TER_Coastal
	terElevTex[4] = s_TexTerPalette[WORLDTEX_SeaWater];			// TER_ContinentalShelf, TER_SeaFloor, TER_SeaTrenct
	Q_ASSERT(terElevTex[0]);

	float altRange = maxAltMSL - minAltMSL;						// Range of altitudes on this planet

	const int iSize = 1024;
	QImage image = QImage(iSize, 1, QImage::Format_RGB32);
	for(int x=0; x<iSize; x++)
	{
		float texCoord = (float)x/iSize;	// Calculate texture coordinate of the 'x' pixel in the range 0..1.

		// Calculate WORLDTEX_TYPE index for 'r' value of lookup table pixel.
		float altitudeMSL = (minAltMSL + texCoord * altRange);
		int terIndex = TerrainElevationType(altitudeMSL) - 2;	// Make TER_Peak, TER_Alpine and TER_Highlands the same.
		if(terIndex < 0)
			terIndex = 0;
		else if(terIndex > 4)
			terIndex = 4;

		image.setPixel(x, 0, qRgba(terIndex*255/5, 0, 0, 255));
	}

	return image;
}

QImage DaWorldRingSection::CreateAtVegetationLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const
{
	// Select textures to use for the various terrain elevations.
	terElevTex[0] = s_TexTerPalette[WORLDTEX_RockGrayHeavy];	// TER_Peak, TER_Alpine, and TER_Highlands are ALWAYS the same.
	terElevTex[1] = s_TexTerPalette[WORLDTEX_DirtModerate];		// TER_Midlands
	terElevTex[2] = s_TexTerPalette[WORLDTEX_DirtCoarse];		// TER_Lowlands
	terElevTex[3] = s_TexTerPalette[WORLDTEX_DirtHeavy];		// TER_Coastal
	terElevTex[4] = s_TexTerPalette[WORLDTEX_SandSmooth];		// TER_ContinentalShelf, TER_SeaFloor, TER_SeaTrenct
	Q_ASSERT(terElevTex[0]);

	float altRange = maxAltMSL - minAltMSL;						// Range of altitudes on this planet

	const int iSize = 1024;
	QImage image = QImage(iSize, 1, QImage::Format_RGB32);
	for(int x=0; x<iSize; x++)
	{
		float texCoord = (float)x/iSize;	// Calculate texture coordinate of the 'x' pixel in the range 0..1.

		// Calculate WORLDTEX_TYPE index for 'r' value of lookup table pixel.
		float altitudeMSL = (minAltMSL + texCoord * altRange);
		int terIndex = TerrainElevationType(altitudeMSL) - 2;	// Make TER_Peak, TER_Alpine and TER_Highlands the same.
		if(terIndex < 0)
			terIndex = 0;
		else if(terIndex > 4)
			terIndex = 4;

		image.setPixel(x, 0, qRgba(terIndex*255/5, 0, 0, 255));
	}

	return image;
}

// Create the lookup table used to index the tiles in the texture atlas
// Some planet types have different rules to follow
GLuint DaWorldRingSection::CreateFarLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const
{
	//qDebug() << "DaWorldRingSection::CreateFarLookupTable" << Id() << Name() << maxAltMSL << minAltMSL;
	QImage image = CreateFarVegetationLookupTable(maxAltMSL, minAltMSL, terElevTex);

	// Use nearest filter for the lookup table texture because the values stored in the texture must not be linear interpolated.
	QString farLUT = QString("%1FLUT").arg(Id().StringId());
	GLuint lu = re.CreateTextureFromBitmap(Id().GetID(), farLUT, image, GL_NEAREST, GL_NEAREST);
	Q_ASSERT(lu);
	return lu;
}

// Create the lookup table used to index the tiles in the texture atlas
// Some planet types have different rules to follow
GLuint DaWorldRingSection::CreateAtLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const
{
	//qDebug() << "DaWorldRingSection::CreateAtLookupTable" << Id() << Name() << maxAltMSL << minAltMSL;
	QImage image = CreateAtVegetationLookupTable(maxAltMSL, minAltMSL, terElevTex);

	// Use nearest filter for the lookup table texture because the values stored in the texture must not be linear interpolated.
	QString lutName = QString("%1.LUT").arg(Id().StringId());
	GLuint lu = re.CreateTextureFromBitmap(Id().GetID(), lutName, image, GL_NEAREST, GL_NEAREST);
	Q_ASSERT(lu);
	return lu;
}

void DaWorldRingSection::CreateGrassTexture(AuGLEffects& re, AuGLModelMaker& mm)
{
	if(!m_pFlora)
		return;

	quint32 genSeed = GenSeed();

	// Get images of all the grasses on the planet.
	QList<QImage> grasses;
	for(int resourceZone=0; resourceZone<ResourceZones(); resourceZone++)
	{
		QImage grass[3];
		// QPixmap: It is not safe to use pixmaps outside the GUI thread
		//if(!mm.GrassImages(grass[0], grass[1], grass[2], m_pFlora[resourceZone], Id(), genSeed, resourceZone))
			//return;
		m_pFlora[resourceZone].PaintGroundCover(grass[0], grass[1], grass[2], genSeed, resourceZone, m_tmFloraChange);

		// Each grass image is actually 4 different ones, tiled 2x2.
		for(int glat=0; glat<3; glat++)
		{
			int width = grass[glat].width()/2;
			int height = grass[glat].height()/2;
			QImage gsub(width, height, QImage::Format_ARGB32);

			for(int tilex=0; tilex<width*2; tilex+=width)
			{
				for(int tiley=0; tiley<height*2; tiley+=height)
				{
					// Paint the subtile of the grass image into a single image of its own.
					QPainter psub(&gsub);
					psub.setCompositionMode(QPainter::CompositionMode_Clear);	// clear mode
					psub.fillRect(0,0,width,height, QBrush(Qt::transparent));
					psub.setCompositionMode(QPainter::CompositionMode_Source);
					psub.drawImage(0,0, grass[glat], tilex,tiley);
					psub.end();

					//gsub.save(QString("%1x%2y%3_gsub.png").arg(Id().StringId()).arg(tilex).arg(tiley));
					grasses.append(gsub.scaled(32,32));
				}
			}
		}
	}
	mm.DeleteGrassImages(Id());

	// Prepare dice for consitent grass texture at this planet.
	AuDice dice;
	dice.Seed(genSeed);

	const int tileSize = g_Au.GroundTextureSize();	// This texture is applied to the ground.

	// Create a repeatable grassy texture tile using the images of local grasses. We will paint more and more grass into
	// the image, grabbing copies of it at several steps along the way. That way the transition from ligth to moderate to dense
	// vegetation connects nicely because the underlying texture remains the same, just more plants are added.
	QImage imrep(tileSize*3, tileSize*3, QImage::Format_ARGB32);
	imrep.fill(Qt::transparent);

	QPainter prep;
	prep.begin(&imrep);

	// Draw a bunch of grass. Each iteration of the loop paints another layer of grass on top. After each iteration, we
	// measure for the desired vegetation density due to variations between grasses; we can't always just draw a certain
	// number of them.
	float targetDensity = .9f;
	float density;
	int farside = tileSize*2;
	do
	{
		// Select a grass image at random.
		int grass = dice.Roll(1,grasses.count(),-1);

		// Generate a random coordinate within a tile.
		int ix = dice.Roll(1,tileSize,-1);
		int iy = dice.Roll(1,tileSize,-1);

		// Generate a random angle for this spot of grass.
		int angle = dice.Roll(1,360,-1);

		// Draw a tile pattern of the grasses.
		for(int tx=0; tx<3; tx++)
			for(int ty=0; ty<3; ty++)
			{
				prep.translate(ix + tx*tileSize, iy + ty*tileSize);
				prep.rotate(angle);
				prep.drawImage(0,0, grasses[grass]);
				prep.resetTransform();
			}

		// Measure the current density in the center tile.
		density = 0;
		for(int tx=tileSize; tx<farside; tx++)
			for(int ty=tileSize; ty<farside; ty++)
			{
				QRgb rgb = imrep.pixel(tx,ty);
				float alpha = (float)qAlpha(rgb) / 255.f;
				density += alpha;
			}
		density /= tileSize * tileSize;	// Average alpha of tile.
	}
	while(density < targetDensity);

	prep.end();

	// Copy the center tile from the 3x3 tile image to the grass texture.
	QImage imgrass(tileSize, tileSize, QImage::Format_ARGB32);
	imgrass.fill(Qt::transparent);
	QPainter phigh(&imgrass);
	phigh.drawImage(0,0, imrep, tileSize,tileSize, tileSize,tileSize);
	phigh.end();

	QString texName = QString("%1GHigh").arg(Id().StringId());
	m_glTexGrass = re.CreateTextureFromBitmap(Id().GetID(), texName, imgrass, GL_LINEAR, GL_LINEAR, true);
	//qDebug() << "DaWorldRingSection::CreateGrassTexture" << Id() << Name() << "finished" << texName;
}

void DaWorldRingSection::LoadCachedSurfaceImage(QImage& surface, int iViewSizeX) const
{
	if(g_Au.IsSceneSolo())
		return;
	// Try loading the far world texture image from a cache file.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("WTF%1X%2").arg(Id().GetID()).arg(iViewSizeX));
	QFile cacheFile(cacheFileName);
	if(cacheFile.open(QIODevice::ReadOnly))
	{
		// Load from cache file.
		QDataStream dsfile(&cacheFile);
		dsfile.setVersion(QDataStream::Qt_4_1);

		// Read the header from the file.
		quint8 fileType;
		quint8 fileVersion;
		QDateTime storeTime;								// Date/time the cache file was created.
		dsfile >> fileType;
		dsfile >> fileVersion;
		dsfile >> storeTime;
		storeTime.setTimeSpec(Qt::UTC);
		if(fileType == WORLD_TEXTURE_FAR_CACHE && fileVersion == WORLD_TEXTURE_FAR_VER &&
				(m_tmFloraChange.IsNull() || storeTime == m_tmFloraChange.UTC()))
		{
			// Cache file looks good.
			// Read the compressed byte array from the file.
			QByteArray bcomp;
			dsfile >> bcomp;

			if(!bcomp.isEmpty())
			{
				// Uncompress the byte array.
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,cacheFileName);

				// Read the model cache from the uncompressed byte array.
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> surface;
			}
		}

		// Close the file.
		cacheFile.close();

		//qDebug() << "DaWorldRingSection::LoadCachedSurfaceImage" << Id() << Name() << "image loaded from cache"
			//<< surface.width() << surface.height() << surface.depth() << surface.bitPlaneCount();
	}
}

// iViewSizeX is the pixel width of the resulting texture. The height of the texture will be iViewSizeX/2.
GLuint DaWorldRingSection::CreatePlanetTexture(AuAtomRenderInfo& ari, int iViewSizeX, bool drawCities)
{
	if(!drawCities)
	{
		// Try loading the surface texture image from a cache file.
		QImage imtex;
		LoadCachedSurfaceImage(imtex, iViewSizeX);
		if(!imtex.isNull())
		{
			// Create a texture from the image.
			//qDebug() << "DaWorldRingSection::CreatePlanetTexture" << Id() << Name() << "image loaded from cache"
				//<< imtex.width() << imtex.height() << imtex.depth() << imtex.bitPlaneCount();
			GLuint glTexSurface;
			ari.re->glGenTextures(1, &glTexSurface);
			ari.re->glBindTexture(GL_TEXTURE_2D, glTexSurface);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			ari.re->CreateTextureFromBitmap(imtex, true);		// Creates the texture without adding it to re's texture cache.
			ari.re->glBindTexture(GL_TEXTURE_2D, 0);
			return glTexSurface;
		}
	}

	if(!m_glTexGrass)
		return 0;

	// Render globe with vertices flattened into the range -1,-1,0 to 1,1,0. This creates the appropriate texture to wrap around
	// a simple globe.
	// Create the vertex array object of the peeled-open flat globe.
	float globeMaxAltMSL, globeMinAltMSL;
	AuGLVAO flatGlobeVAO = ari.mm.FlatRingworldSectionModel(*ari.re, this, globeMinAltMSL, globeMaxAltMSL);
	if(!flatGlobeVAO.VAO())
		return 0;						// Model maker is not ready with our surface model yet.
	//float altRange = globeMaxAltMSL - globeMinAltMSL;			// Altitude range of all land, for altitude factor calculation.
	//float flatGlobeAltNoiseFactor = (AltitudeIncrement()/3)/altRange;			// Altitude deviation due to noise.

	GLuint glTexSurface;

	// Spherical world texture is twice as wide as it is high.
	int iViewSizeY = NorthSouthDivisions(iViewSizeX);
	//qDebug() << "DaWorldRingSection::CreatePlanetTexture" << Id() << Name() << "size" << iViewSizeX << iViewSizeY;

	// Get current fbo binding. If it is not restored properly, things in the scene blink.
	GLint oldfbo;
	ari.re->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);

	// Setup FBO so that we render the terrain at full screen resolution
	GLuint fbo;					// FBO used to create planet texture
	ari.re->glGenFramebuffers(1, &fbo);
	ari.re->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);

	// Now setup the environment map texture
	ari.re->glGenTextures(1, &glTexSurface);
	ari.re->glBindTexture(GL_TEXTURE_2D, glTexSurface);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	ari.re->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iViewSizeX, iViewSizeY, 0, GL_RGBA, GL_FLOAT, NULL);
	// Attach it to the FBO so we can render to it
	ari.re->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, glTexSurface, 0);

	GLenum status = ari.re->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		qDebug("DaWorldRingSection::CreatePlanetTexture glCheckFramebufferStatusEXT returned %d", status);
		ari.re->glBindTexture(GL_TEXTURE_2D, 0);
		ari.re->glBindFramebuffer(GL_FRAMEBUFFER_EXT, oldfbo);	// Unbind the FBO for now
		return 0;
	}

	ari.re->glViewport(0, 0, iViewSizeX, iViewSizeY);
	ari.re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	// Create the lookup tables used for the planet texturing based on altitude of terrain.
	GLuint terElevTex[5];
	GLuint texFlatLookup = CreateFarLookupTable(*ari.re, globeMaxAltMSL, globeMinAltMSL, terElevTex);

	// Render the vertex array object.
	AuMatrix4x4 projection;
	AuMatrix4x4 model;
	ari.re->GLSL().RingworldFloraShader().Select();
	ari.re->GLSL().RingworldFloraShader().SetProjectionModelView(projection * model);
	ari.re->GLSL().RingworldFloraShader().SetGrassTexture(m_glTexGrass);
	ari.re->GLSL().RingworldFloraShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
	ari.re->GLSL().RingworldFloraShader().SetTerrainLookup(texFlatLookup);
	ari.re->GLSL().RingworldFloraShader().SetTerrainPalette(terElevTex);

	flatGlobeVAO.SelectGL();
	flatGlobeVAO.DrawGL();
	flatGlobeVAO.DeselectGL();

	ari.re->GLSL().RingworldFloraShader().Deselect();

	// Free the lookup texture.
	QString farLUT = QString("%1FLUT").arg(Id().StringId());
	ari.re->DeleteTexture(Id().GetID(), farLUT);

	// Free the vertex array object.
	//mm.DeleteFlatRingworldSectionModel(Id());

	if(drawCities)
	{
		// Render geo roads, if any.
		float radiansPerFootX = (2.f * PIf) / TerrainXSize();
		float radiansPerFootY = PIf / TerrainYSize();
		ari.re->GLSL().LineShader().Select();
		ari.re->GLSL().LineShader().SetProjectionModelView(projection * model);
		const AuGeologicalFeatures& geof = LockGeologicalFeatures();
		geof.RenderOnSurfaceTexture(*this, *ari.re, radiansPerFootX, radiansPerFootY);
		UnlockGeologicalFeatures();

		// Render buildings and cities, if any.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			(*it)->RenderOnSurfaceTexture(*ari.re, radiansPerFootX, radiansPerFootY);
		ari.re->GLSL().LineShader().Deselect();
	}

	ari.ResetViewport();

	status = ari.re->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
		qDebug("DaWorldRingSection::CreatePlanetTexture glCheckFramebufferStatusEXT returned %d", status);

	ari.re->glBindTexture(GL_TEXTURE_2D, 0);
	ari.re->glBindFramebuffer(GL_FRAMEBUFFER_EXT, oldfbo);	// Unbind
	ari.re->glDeleteFramebuffers(1, &fbo);

	if(!drawCities && !g_Au.IsSceneSolo())
	{
		// Store the result in a cache file.
		QImage imtex(iViewSizeX, iViewSizeY, QImage::Format_RGB32);
		GrabSurfaceTextureImage(*ari.re, imtex, glTexSurface);

		// Insure the cache folder exists.
		QDir cacheDir(g_Au.CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("WTF%1X%2").arg(Id().GetID()).arg(iViewSizeX));
		QFile cacheFile(cacheFileName);
		if(g_Au.CreateCacheGame())
		{
			// The cache folder exists.
			if(cacheFile.open(QFile::WriteOnly))
			{
				// Prepare to write to the file.
				QDataStream dsfile(&cacheFile);
				dsfile.setVersion(QDataStream::Qt_4_1);

				// Write a header to the file that includes the date/time the cache file was created.
				const quint8 fileType = WORLD_TEXTURE_FAR_CACHE;
				const quint8 fileVersion = WORLD_TEXTURE_FAR_VER;
				dsfile << fileType;
				dsfile << fileVersion;
				dsfile << g_Au.CurrentUTC();

				// Store path cache into a byte array so it can be compressed.
				QByteArray ba;
				QDataStream ds(&ba, QIODevice::WriteOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds << imtex;

				// Compress the byte array.
				QByteArray bcomp = qCompress(ba);

				// Write the compressed byte array to the file.
				dsfile << bcomp;

				// Close the file.
				cacheFile.close();

				//qDebug() << "DaWorldRingSection::CreatePlanetTexture" << Id() << Name() << "cached world texture"
					//<< cacheFileName;
			}
			else
				qWarning() << QString("DaWorldRingSection::CreatePlanetTexture FAILED TO CREATE FILE %1").arg(cacheFileName);
		}
		else
			qWarning() << QString("DaWorldRingSection::CreatePlanetTexture FAILED TO CREATE DIR FOR FILE %1").arg(cacheFileName);
	}
	return glTexSurface;
}

void DaWorldRingSection::CreateFlatGlobeVAOTriangle(QList<AuGlobeTriAltVeg>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
		const AuGlobeLL& ll2) const
{
	// Calculate triangle vertex altitudes above the center of the globe.
	double terAltMSL0 = TerrainAltitudeMSL(ll0);
	double terAltMSL1 = TerrainAltitudeMSL(ll1);
	double terAltMSL2 = TerrainAltitudeMSL(ll2);

	// Calculate triangle vertex coordinates in the range -1 to 1.
	AuPoint terCoord0 = GetFlatGlobeSurfaceLocation(ll0);
	AuPoint terCoord1 = GetFlatGlobeSurfaceLocation(ll1);
	AuPoint terCoord2 = GetFlatGlobeSurfaceLocation(ll2);

	// Calculate texture coordinates.
	int texreps = SeaLevelEquatorDivisions() * TERRAIN_TEXTURE_REPS;
	double aspectXY = TerrainXSize() / TerrainYSize();
	QPointF terTex0;
	terTex0.setX((ll0.Longitude())/(PIf*2) * texreps * aspectXY);
	terTex0.setY((ll0.Latitude() + PIf/2)/PIf * texreps);
	QPointF terTex1;
	terTex1.setX((ll1.Longitude())/(PIf*2) * texreps * aspectXY);
	terTex1.setY((ll1.Latitude() + PIf/2)/PIf * texreps);
	QPointF terTex2;
	terTex2.setX((ll2.Longitude())/(PIf*2) * texreps * aspectXY);
	terTex2.setY((ll2.Latitude() + PIf/2)/PIf * texreps);

	// Calculate vegetation density.
	quint8 veg0;
	if(m_byWater && terAltMSL0 < 0)
		veg0 = 0;	// Don't show aquatic plants on the map.
	else
		veg0 = VegetationAt(ll0, terAltMSL0, 0, false);
	quint8 veg1;
	if(m_byWater && terAltMSL1 < 0)
		veg1 = 0;	// Don't show aquatic plants on the map.
	else
		veg1 = VegetationAt(ll1, terAltMSL1, 0, false);
	quint8 veg2;
	if(m_byWater && terAltMSL2 < 0)
		veg2 = 0;	// Don't show aquatic plants on the map.
	else
		veg2 = VegetationAt(ll2, terAltMSL2, 0, false);

	// Calculate snow due to polar caps and mountain peaks.
	float snow0 = SnowCoverage(terAltMSL0, ll0.Latitude(), 0);
	float snow1 = SnowCoverage(terAltMSL1, ll1.Latitude(), 0);
	float snow2 = SnowCoverage(terAltMSL2, ll2.Latitude(), 0);

	// Construct a globe triangle.
	AuGlobeTriAltVeg tri;
	tri.m_Vert0 = AuGlobeVertAltVeg(terCoord0, terTex0, terAltMSL0, snow0, veg0);
	tri.m_Vert1 = AuGlobeVertAltVeg(terCoord1, terTex1, terAltMSL1, snow1, veg1);
	tri.m_Vert2 = AuGlobeVertAltVeg(terCoord2, terTex2, terAltMSL2, snow2, veg2);

	// Append the globe triangle to the list.
	tris.append(tri);
}

void DaWorldRingSection::GrabSurfaceTextureImage(AuGLEffects& re, QImage& imtex, GLuint glTexture) const
{
	int iViewSizeX = imtex.width();
	int iViewSizeY = imtex.height();
	//qDebug() << "DaWorldRingSection::GrabSurfaceTextureImage" << Id() << Name() << "size" << iViewSizeX << iViewSizeY;

	GLfloat* buffer = (GLfloat*)malloc(iViewSizeX*iViewSizeY*4*sizeof(GLfloat));

	re.glBindTexture(GL_TEXTURE_2D, glTexture);
	re.glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
	for(int w=0; w<iViewSizeX; w++)
	{
		for(int h=0; h<iViewSizeY; h++)
		{
			int i = ((h*iViewSizeX) + w)*4;
			//QRgb value = qRgba((int)(buffer[i]*255), (int)(buffer[i+1]*255), (int)(buffer[i+2]*255), (int)(buffer[i+3]*255));
			QRgb value = qRgba((int)(buffer[i]*255), (int)(buffer[i+1]*255), (int)(buffer[i+2]*255), 255);
			imtex.setPixel(w, iViewSizeY-h-1, value);	// Mirror vertically.
		}
	}
	re.glBindTexture(GL_TEXTURE_2D, 0);	// Unbind

	free(buffer);
}

void DaWorldRingSection::CreateSurfaceMap(AuGLEffects& re)
{
	// Free the previous surface image, if we already made one.
	if(m_Surface && m_Surface != (QImage*)-1)
	{
		delete m_Surface;
		m_Surface = 0;
	}

	QImage fboImage;
	const int londivs = GroundLevelEquatorDivisions();
	int iViewSizeX = londivs*g_Au.DivTexels();	// NOTE: This MUST match the iViewSizeX value passed to CreatePlanetTexture.
	LoadCachedSurfaceImage(fboImage, iViewSizeX);
	if(fboImage.isNull())
	{
		// Create surface texture from created planet texture
		if(!m_glTexFarSurface)
			return;
		fboImage = QImage(iViewSizeX, NorthSouthDivisions(iViewSizeX), QImage::Format_RGB32);
		GrabSurfaceTextureImage(re, fboImage, m_glTexFarSurface);
	}

	/*
	// Scale image to 1/2 its size to smooth out the crisp pixels.
	fboImage = fboImage.scaled(fboImage.width()/2, fboImage.height()/2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	// Scale image back up to have enough resolution for the map.
	fboImage = fboImage.scaled(fboImage.width()*4, fboImage.height()*4, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	*/
	fboImage = fboImage.scaled(fboImage.width()*2, fboImage.height()*2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	// Paint the terrain using the texture.
	m_Surface = new QImage(fboImage);
	QPainter p(m_Surface);

	{
		// Paint longitude lines at zone divisions.
		p.setPen(QPen(QColor(0,0,0,64), 0.f));
		for(int zone = 0; zone < ResourceZones(); zone++)
		{
			int xpos = (zone * fboImage.width() + ResourceZones() - 1) / ResourceZones();
			p.drawLine(xpos,0, xpos,fboImage.height());
		}

		// Paint important latitude lines.
		// Equator
		int equator = fboImage.height()/2;
		p.drawLine(0,equator, fboImage.width(),equator);

		// Tropic
		float tropic = TropicalLatitude();
		if(tropic < PIf/2)
		{
			int tropicPixels = tropic/(PIf/2) * fboImage.height()/2;
			int ypos = equator - tropicPixels;
			p.drawLine(0,ypos, fboImage.width(),ypos);
			ypos = equator + tropicPixels;
			p.drawLine(0,ypos, fboImage.width(),ypos);
		}

		// Polar
		float polar = PolarLatitude();
		if(polar < PIf/2)
		{
			int polarPixels = polar/(PIf/2) * fboImage.height()/2;
			int ypos = equator - polarPixels;
			p.drawLine(0,ypos, fboImage.width(),ypos);
			ypos = equator + polarPixels;
			p.drawLine(0,ypos, fboImage.width(),ypos);
		}

		// Paint a spot at the control center.
		if(SectionIndex() == 0)
		{
			// Control center is a square forbidden zone on section 0 located at LL 0 0.
			// It is a square with top/bottom sides 1 degree N/S and left/right sides 2 degrees E/W of center.
			float zoneLat = DEG2RADf;
			float aspectYX = (float)(TerrainYSize() / TerrainXSize());
			float zoneLon = aspectYX * 2.f*DEG2RADf;

			int zoneLatPixels = zoneLat/(PIf/2) * fboImage.height()/2;
			int zoneLonPixels = zoneLon/PIf * fboImage.width()/2;

			int czt = fboImage.height()/2 - zoneLatPixels;
			int czl = fboImage.width()/2 - zoneLonPixels;

			int czh = (zoneLat*2/(PIf/2)) * fboImage.height()/2;
			int czw = (zoneLon*2/PIf) * fboImage.width()/2;

			p.setPen(QPen(QColor(0,0,0,0), 0.f));
			p.setBrush(QColor(0,0,0,100));
			p.drawRect(czl,czt, czw,czh);
		}
	}

	// Paint geo roads, if any.
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	geof.PaintOnSurfaceMap(*this, p, fboImage.width(), fboImage.height());
	UnlockGeologicalFeatures();

	// Paint cities and buildings, if any.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->PaintOnSurfaceMap(*this, p, fboImage.width(), fboImage.height());

	// Paint claim flag.
	if(ClaimedByEmpire())
	{
		AuGlobeLLF claimLL = ClaimedAtLL();
		p.setPen(QPen(DbIndustry::MapColor(IID_TownSquare), 0.f));
		int x = (int)((claimLL.Longitude() + PIf) * fboImage.width() / (PIf*2.f));
		int y = (int)(((PIf/2.f) - claimLL.Latitude()) * fboImage.height() / PIf);
		p.drawPoint(x,y);
	}
	p.end();
	//m_Surface->save(Id().StringId() + "-surface.png");
}

// Generates terrain parameters for this world.
void DaWorldRingSection::CreateTerrainData(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// Configure parameters for generating terrain using 2D Perlin noise.
	double tx = TerrainXSize();
	m_RingSectionLock.lockForWrite();
	m_TerrainXOff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	m_TerrainYOff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	m_TerrainFrequency = (float)dice.Roll(1, RadiusStandard100km()*1000, 10000) / 10000.f;
	float C8 = (float)RadiusStandard() * 2.f * PIf;		// Circumference of an Earth size world.
	m_TerrainFrequency *= (float)tx / C8;				// So frequency is like an Earth size world.
	if(m_TerrainFrequency < ResourceZones())
		m_TerrainFrequency = ResourceZones();

	// Persistence causes jaggier terrain boundaries as it goes up. Persistence also causes the potential magnitude of the Perlin
	// noise function to increase so we must calculate an associated factor to scale the result into a manageable range.
	m_TerrainPersistence = (float)dice.Roll(1, 40, 45)/100.f;

	// Max Perlin noise due to persistence value at 7 octaves. This factor is needed to scale the return result of the Perlin noise
	// function into a range of -1..1.
	m_TerrainPersFactor = 1.f + m_TerrainPersistence + m_TerrainPersistence*m_TerrainPersistence + powf(m_TerrainPersistence,3) +
		powf(m_TerrainPersistence,4) + powf(m_TerrainPersistence,5) + powf(m_TerrainPersistence,6);

	// The terrain generator scales the Perlin noise return value so that 1 is at the tree line (TreeLineMSL). It does this by
	// dividing by the desired range, which is m_TerrainPersFactor. By playing with the persistence factor, we can cause varying
	// amounts of the noise to go above 1, resulting in snow capped peaks that round off nicely. A smaller m_TerrainPersFactor
	// results in more peaks; a larger m_TerrainPersFactor results in smooth rolling terrain.
	//float persAdj = .45f + (float)dice.Roll(1,50)/100.f;	// Adjustment is .46 to .95. About 1 in 20 worlds reported too steep to
															// build on. Some have mountains that stick out of the atmosphere.
	float persAdj = .699f + (float)dice.Roll(1,261)/1000.f;	// Adjustment is .700 to .960, by thousandths.
	m_TerrainPersFactor *= persAdj;

	// Square the water factor to preserve more land surface. It's not a linear dropoff anyway and this is a closer approximation.
	// No land is useless. The divisor >100 here insures that there is still a tiny bit of land on worlds with 100% coverage.
	m_TerrainWaterFactor = (m_byWater/111.111f - .5f) * 2;		// Range -1..1: 0 = 50% coverage, positive = >50%, negative = <50%
	if(m_TerrainWaterFactor < 0)
		m_TerrainWaterFactor = -(m_TerrainWaterFactor*m_TerrainWaterFactor);	// Square the water factor and preserve the sign.
	else
		m_TerrainWaterFactor *= m_TerrainWaterFactor;

	//qDebug() << "DaWorldRingSection::CreateTerrainData" << Id() << Name() << "water" << m_byWater << "wf" << m_TerrainWaterFactor
		//<< "freq" << m_TerrainFrequency << "pers" << m_TerrainPersistence << "persFactor" << m_TerrainPersFactor
		//<< "persAdj" << persAdj << "treeLine" << TreeLineMSL();
	m_RingSectionLock.unlock();
}

AuGlobeLL DaWorldRingSection::RoundedGlobeLoc(const AuGlobeLL& locLL, int eqdivs) const
{
	AuGlobeLL rndLL;											// This will be our return value.

	// Determine closest latitude.
	int latdivshemi = NorthSouthDivisions(eqdivs)/2;			// Latitude divisions per hemisphere.
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	double pole = PI/2 - latDivAng/2;
	if(locLL.Latitude() >= pole)
		return AuGlobeLL(0, PI/2);								// North pole
	else if(locLL.Latitude() <= -pole)
		return AuGlobeLL(0, -PI/2);								// South pole
	rndLL.SetLatitude(FROUND(locLL.Latitude(), latDivAng));		// Round locLL to the nearest grid latitude.

	// Determine closest longitude.
	int londivs = (int)ceil(cos(rndLL.Latitude()) * eqdivs);	// Number of vertices around globe at the latitude.
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	rndLL.SetLongitude(FROUND(locLL.Longitude(), lonDivAng));	// Round locLL to the nearest grid longitude.

	return rndLL;
}

AuGlobeLLF DaWorldRingSection::RoundedGlobeLoc(const AuGlobeLLF& locLL, int eqdivs) const
{
	AuGlobeLLF rndLL;											// This will be our return value.

	// Determine closest latitude.
	int latdivshemi = NorthSouthDivisions(eqdivs)/2;			// Latitude divisions per hemisphere.
	float latDivAng = (PIf/2) / latdivshemi;					// Radian angle per latitude division
	float pole = PIf/2 - latDivAng/2;
	if(locLL.Latitude() >= pole)
		return AuGlobeLLF(0, PIf/2);							// North pole
	else if(locLL.Latitude() <= -pole)
		return AuGlobeLLF(0, -PIf/2);							// South pole
	rndLL.SetLatitude(FROUND(locLL.Latitude(), latDivAng));		// Round locLL to the nearest grid latitude.

	// Determine closest longitude.
	int londivs = (int)ceil(cosf(rndLL.Latitude()) * eqdivs);	// Number of vertices around globe at the latitude.
	float lonDivAng = (PIf*2) / londivs;						// Radian angle per longitude division
	rndLL.SetLongitude(FROUND(locLL.Longitude(), lonDivAng));	// Round locLL to the nearest grid longitude.

	return rndLL;
}

void DaWorldRingSection::CreateAtSurfaceTriangles(QList<AuGlobeTerrainTriCen>& tricens) const
{
	// Each triangle in this tessellation becomes a region for higher tessellation on a per triangle basis.
	AuGlobeLLTriList tris;
	float maxAltMSL, minAltMSL;
	CreateSurfaceTriangles(tris, GroundLevelEquatorDivisions(), maxAltMSL, minAltMSL);

	// Build result with precalculated center points at globe sea-level radius.
	for(AuGlobeLLTriList::const_iterator gti = tris.cbegin(); gti != tris.cend(); gti++)
	{
		AuPoint loc0 = LocOfLL((*gti).m_LL0);
		AuPoint loc1 = LocOfLL((*gti).m_LL1);
		AuPoint loc2 = LocOfLL((*gti).m_LL2);
		AuGlobeTerrainTriCen tricen(*gti, loc0, loc1, loc2);
		tricens.append(tricen);
	}
}

void DaWorldRingSection::CreateSurfaceTriangles(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL,
		float& minAltMSL) const
{
	//qDebug() << "DaWorldRingSection::CreateSurfaceTriangles" << Id() << Name() << "eqdivs" << eqdivs;
	// Create terrain cells for the globe based on longitude and latitude.
	// Each pole will have a single terrain cell.
	// Equator has 'eqdivs' terrain cells.
	tris.clear();

	// Create a uniform rectangular patch.
	int londivs = eqdivs/2;
	int latdivs = NorthSouthDivisions(eqdivs)/2;
	bool hasMinMax = false;
	for(int iy=-latdivs; iy<latdivs; iy++)
	{
		double lat = (PI/2.) * iy/latdivs;
		double latnext = (PI/2.) * (iy+1)/latdivs;
		for(int ix=-londivs; ix<londivs; ix++)
		{
			double lon = PI * ix/londivs;
			double lonnext = PI * (ix+1)/londivs;

			AuGlobeLL ll(lon, lat);
			AuGlobeLL ul(lon, latnext);
			AuGlobeLL ur(lonnext, latnext);
			AuGlobeLL lr(lonnext, lat);

			// Remember highest and lowest terrain altitudes.
			float alt = TerrainAltitudeMSL(ll);
			if(!hasMinMax)
			{
				maxAltMSL = alt;
				minAltMSL = alt;
				hasMinMax = true;
			}
			else
			{
				if(alt > maxAltMSL)
					maxAltMSL = alt;
				else if(alt < minAltMSL)
					minAltMSL = alt;
			}

			tris.append(AuGlobeLLTri(ll, ul, ur));
			tris.append(AuGlobeLLTri(ur, lr, ll));
		}
	}

	//qDebug() << "DaWorldRingSection::CreateSurfaceTriangles" << Id() << Name() << "maxAlt" << maxAltMSL
		//<< "minAltMSL" << minAltMSL << "tris" << tris.count();
}

void DaWorldRingSection::CreateCylinderTriangles(AuGlobeLLTriList& tris, int londivs, int latdivs) const
{
	// Create triangles for the globe based on longitude and latitude.
	// Each pole will have a single vertex.
	// Equator has 'EquatorDivisions' vertexes.
	tris.clear();

	// Create a uniform rectangular patch.
	//qDebug() << "DaWorldRingSection::CreateCylinderTriangles londivs" << londivs << "latdivs" << latdivs;
	if(latdivs < 1)
		latdivs = 1;
	else if(latdivs > 8)
		latdivs = 8;
	for(int iy=-latdivs; iy<latdivs; iy++)
	{
		double lat = (PI/2.) * iy/latdivs;
		double latnext = (PI/2.) * (iy+1)/latdivs;
		for(int ix=-londivs; ix<londivs; ix++)
		{
			double lon = PI * ix/londivs;
			double lonnext = PI * (ix+1)/londivs;

			AuGlobeLL ll(lon, lat);
			AuGlobeLL ul(lon, latnext);
			AuGlobeLL ur(lonnext, latnext);
			AuGlobeLL lr(lonnext, lat);

			tris.append(AuGlobeLLTri(ll, ul, ur));
			tris.append(AuGlobeLLTri(ur, lr, ll));
		}
	}
}

// NOTE: This code is identical to DaStar::CalcAngleOfRotation.
// This calculates the angle around this planets axis based on real time.
// The time scale is modified such that four days in real time are equal to an earth 
// year in the universe.
// This results in a standard earth day of about 15.77 minutes in real time.
// All planets rotate at the same rate. Mass is not used in the calculation.
// Ringworlds do not rotate on their axis. Instead this value is the rotation of the shadow squares relative to the ring.
double DaWorldRingSection::CalcAngleOfRotation() const
{
	double rRealDays = (double)g_Au.CoordinatedTime().MSecsSinceEpoch() / (double)MillisecondsPerDay();
	double rUYears = rRealDays / RealDaysPerUYear();			// This is how many years have passed in the game universe.
	double rUDays = rUYears * DaysPerYear();					// How many days have passed.

	//rUDays *= 180;	//DHH Really fast day/night cycles for debugging.

	// Calculate the orbit angle from the fractional day that we're in.
	double rFracDays = rUDays - (int)rUDays;
	double rAngle = rFracDays * SectionAngle() - SectionAngle()/2;
	if(RotationAngleOffset())
		rAngle += RotationAngleOffset() / RingworldSections();	// In designer preview, day angle can be offset to adjust lighting.
	return rAngle;
}

QColor DaWorldRingSection::ChartColor() const
{
	return ChartColor(m_byWater);
}

// Static version of ChartColor
QColor DaWorldRingSection::ChartColor(unsigned char byWater)
{
	int r,g,b;
	r = 64;
	g = 192;
	b = byWater * 255/100;	// Water level determines the amount of blue.

	if(r < 0)
		r = 0;
	if(r > 255)
		r = 255;
	if(g < 0)
		g = 0;
	if(g > 255)
		g = 255;
	if(b < 0)
		b = 0;
	if(b > 255)
		b = 255;
	return QColor(r,g,b);
}

QColor DaWorldRingSection::AtmosphereColor() const
{
	m_RingSectionLock.lockForRead();
	if(!m_AtmColor.isValid())
	{
		m_RingSectionLock.unlock();

		// Get genSeed from parent star so all ring sections have the same color of atmosphere.
		const DaStar* star = dynamic_cast<const DaStar*>(Parent());
		quint32 genSeed;
		if(star)
			genSeed = star->GenSeed();
		else
			genSeed = 1;

		m_RingSectionLock.lockForWrite();
		m_AtmColor = AtmosphereColorStatic(m_AtmType, m_byAtmDensity, genSeed);
		//qDebug() << "DaWorldRingSection::AtmosphereColor" << Id() << Name() << "color" << m_AtmColor;
	}
	QColor v(m_AtmColor);
	m_RingSectionLock.unlock();
	return v;
}

void DaWorldRingSection::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	if(!s_TexNoise)
		s_TexNoise = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "TerNoise", ":data/TerNoise.png");

	// NOTE: 1 minus the alpha in this texture is treated as glow, for lava. e.g. Alpha .5 glows at .5. Alpha 1 does not glow.
	if(!s_TexTerPalette[0])
		for(int wt=0; wt<WORLDTEX_Count; wt++)
			s_TexTerPalette[wt] = CreateTerrainTexture(*ari.re, (WORLDTEX_TYPE)wt, g_Au.GroundTextureSize());

	// Create textures used for developments and rocks.
	if(!s_TexRock)
		s_TexRock = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Rock", ":data/txRock.png");

	if(!s_FlagEmpireTex)
	{
		s_FlagEmpireTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPEmpire", ":data/FlagEmpire.png");
		s_FlagEnemyTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPEnemy", ":data/FlagEnemy.png");
		s_FlagFriendTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPFriend", ":data/FlagFriend.png");
		s_FlagNeutralTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FPNeutral", ":data/FlagNeutral.png");
	}

	// Create sea surface textures.
	if(!s_texSea)
		s_texSea = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SeaWater", ":data/SeaWater.png", GL_LINEAR, GL_LINEAR, true);

	// Create sea surface bump maps.
	if(g_Au.BumpMapSeas())
	{
		if(!s_texSeaN)
			s_texSeaN = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "NWater", ":data/N_Water.png", GL_LINEAR, GL_LINEAR,
					true);
	}

	if(!s_TexObelisk)
		s_TexObelisk = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Obelisk", ":data/Obelisk.png", GL_LINEAR, GL_LINEAR,
				true, 1);

	if(!s_TexRingworld)
		s_TexRingworld = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Ringworld", ":data/Ringworld.png", GL_LINEAR,
				GL_LINEAR, true, 1);

	// Grass texture is only needed if this world has grass growing wild in the environment.
	// It is also used for the lawns of homes.
	if(!m_glTexGrass)
		CreateGrassTexture(*ari.re, ari.mm);

	if(!m_glTexFarSurface)
	{
		// Create the planet surface texture. This texture is rendered on the far globe model and it is used as the surface image
		// by CreateSurfaceMap.
		int iViewSizeX = GroundLevelEquatorDivisions()*g_Au.DivTexels();	// NOTE: This MUST match the calc in CreateSurfaceMap.
		m_glTexFarSurface = CreatePlanetTexture(ari, iViewSizeX, false);
	}

	if(m_ViewerNear)
		UpdateNearSurfaceTexture(ari);

	DaWorldLL::CreateDeviceObjects(ari);
}

void DaWorldRingSection::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	//qDebug("DaWorldRingSection::DeleteDeviceObjects - %d glModel %d", Id().GetID());
	DaWorldLL::DeleteDeviceObjects(ari);

	if(m_Atmosphere.VAO())
		m_Atmosphere.DeleteGL();

	if(m_ClaimPoleVAO.VAO())
		m_ClaimPoleVAO.DeleteGL();

	if(m_Clouds.VAO())
		m_Clouds.DeleteGL();

	if(m_Hull.VAO())
		m_Hull.DeleteGL();

	if(m_Obelisk.VAO())
		m_Obelisk.DeleteGL();

	if(m_ShadowSquare.VAO())
		m_ShadowSquare.DeleteGL();

	if(m_NearGlobe.VAO())
	{
		ari.mm.DeleteNearRingworldSectionModel(Id());
		m_NearGlobe = AuGLVAO();
	}

	ari.mm.DeleteFlatRingworldSectionModel(Id());

	if(m_RockVAOs)
	{
		// We must delete the individually allocated vao objects in this array.
		int maxRocks = DpRock::RockTypes();
		AuGLVAO** pvao = m_RockVAOs;
		for(int rock=0; rock<maxRocks; rock++, pvao++)
			if(*pvao)
				(*pvao)->DeleteGL();
	}

	if(m_AtGlobe.m_Low.VAO())
	{
		ari.mm.DeleteRingworldSectionModel(Id());

		m_AtGlobe.m_Low = AuGLVAO();
		m_AtGlobe.m_TriVAOs.clear();
	}

	if(m_SeaGlobe.VAO())
		m_SeaGlobe.DeleteGL();

	if(m_UnderseaGlobe.VAO())
		m_UnderseaGlobe.DeleteGL();

	// Deletes all flora textures and textures that depend upon them.
	DeleteFloraDeviceObjects(*ari.re, ari.mm);

	// Textures associated with my id were freed by the base class. I just need to forget about them.
	m_glTexWx = 0;
}

void DaWorldRingSection::DeleteFloraDeviceObjects(AuGLEffects& re, AuGLModelMaker& mm)
{
	if(m_FloraVAOsHigh)
	{
		// We must delete the individually allocated vao objects in this array.
		int maxFlora = PLANT_VAOS * AuFloraDNA::Zones * (1 + ZONE_TREES + ZONE_SHRUBS) * ResourceZones();
		AuGLVAO** pvao = m_FloraVAOsHigh;
		for(int flora=0; flora<maxFlora; flora++, pvao++)
			if(*pvao)
				(*pvao)->DeleteGL();
	}

	// Kill any processes making plants, if any.
	mm.DeleteFloraModel(Id());

	mm.DeleteFloraImages(Id());

	for(int rz=0; rz<ResourceZones(); rz++)
	{
		if(m_pFlora)
			m_pFlora[rz].DeleteDeviceObjects(re);
		QString stid = QString("F%1Z%2C%3").arg(GenSeed()).arg(rz).arg(m_tmFloraChange.MSecsSinceEpoch());
		re.DeleteTexture(Id().GetID(), stid);
	}

	if(m_glTexNearSurface)
	{
		re.glDeleteTextures(1, &m_glTexNearSurface);
		m_glTexNearSurface = 0;
		m_glTexNearValid = false;
	}

	if(m_glTexFarSurface)
	{
		re.glDeleteTextures(1, &m_glTexFarSurface);
		m_glTexFarSurface = 0;
	}

	QString texName = QString("%1GHigh").arg(Id().StringId());
	re.DeleteTexture(Id().GetID(), texName);
	m_glTexGrass = 0;
}

void DaWorldRingSection::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	// Change the world transformation matrix for this planetary body.
	AuMatrix4x4 tBody;
	// Select an angle for this ringworld section that is coordinated with all sections of the ringworld.
	double rangle = AngleToRingSection();
	{
		// Planet origin is at its orbit position.
		tBody.RotateZ(rangle);								// Rotate the model space to the planet's orbit position.
		tBody.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Translate to the location of this planet relative to the parent star.
		tBody.RotateZ(-rangle);								// Rotate the model space to the planet's orbit position.
		tBody.RotateX(PI/2.);								// Stand the ring section on its edge.
		tBody.RotateY(rangle - PI/2.);						// Orient to face the sun.
	}
	ChangePosNotParent(AuAtomPos(ParentId(), tBody));

	// Update shadow square rotation angle.
	m_RotationAngle = CalcAngleOfRotation();

	// This will take care of the subatoms, particularly the terrain patches.
	DaWorldLL::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

	// Respond to request for a surface map. It must be created while the GL context is current because it uses a GL texture.
	//if(this == sceneAtom->FindParentPlanetInEnv())
		//qDebug() << m_Surface;
	if(m_Surface == (QImage*)-1)
		CreateSurfaceMap(*ari.re);
	else if(m_Surface && this != sceneAtom->FindParentPlanetInEnv())
	{
		// My surface map is no longer needed. Delete it now or it might be out of sync when they come back.
		delete m_Surface;
		m_Surface = 0;
	}

	// Weather movement.
	{
		double rRealDays = (double)g_Au.CoordinatedTime().MSecsSinceEpoch() / (double)MillisecondsPerDay();
		double rUYears = rRealDays / RealDaysPerUYear();		// This is how many years have passed in the game universe.
		double rUDays = rUYears * DaysPerYear();				// How many days have passed.
		if(IsHengeActive())// || true)
			rUDays *= 100;

		//double pfpd = Radius() * (PIf*2);	// Circumference of planet in feet is equator speed in feet per universe day.
		double pfpd = TerrainXSize();
		double spfpd = pfpd;	// Feet saturation pressure travels in a day.
		spfpd += m_Wx.PrevailingWindSpeed(OrbitZone()) * 5 * 24;	// convert to miles per hour	//DHH wtf calc?
		double vpfpd = spfpd/2;	// Vapor pressure travels half as fast as SP
		double spday = spfpd/pfpd;		// SP periods per day.
		double vpday = vpfpd/pfpd;		// VP periods per day.
		double spper = spday * rUDays;	// Current SP period
		double vpper = vpday * rUDays;	// Current VP period

		// Ringworld rotation is about the sun. All bodies orbit the sun in the same direction.
		m_WxSPOffset = (spper - (int)spper);
		m_WxVPOffset = (vpper - (int)vpper);
	}

	// Water movement
	{
		// Calculate offset for sea surface texture.
		float ca = 0, sa = 0;	// cos and sin of texture movement direction angle
		// Water texture moves at a 58.5 degree angle.
		ca = .5225f;
		sa = .8526f;
		float wsec = 30;	// Water seconds per cycle.
		const float wper = ((float)lastAnim.elapsed()/1000.f) / wsec;
		m_WaterOffX += wper * ca;
		m_WaterOffY += wper * sa;
	}
}

// 'longLat' is longitude latitude angles, in radians
void DaWorldRingSection::WeatherAt(const AuGlobeLLF& ll, float altitudeMSL, float& relHumidity, float& oat, float& bar) const
{
	m_Wx.ConditionsAt(*this, ll, altitudeMSL, m_WxSPOffset, m_WxVPOffset, relHumidity, oat, bar);
}

// 'longLat' is longitude latitude angles, in radians
QPointF DaWorldRingSection::WindAt(const AuGlobeLLF& longLat) const
{
	// Check for completed enclosure.
	if(InEnclosure(longLat, 0) > 0)
		return QPointF(10.f, 0.f);			// Light wind inside enclosure so plants move and wind does not howl.
	else if(m_byAtmDensity)
		return m_Wx.WindAt(this, longLat, m_WxSPOffset);
	else
		return QPointF(0,0);
}

bool DaWorldRingSection::IsTicker() const
{
	if(DaWorldLL::IsTicker())
		return true;
	return IsInAnyScene() || Transform().IsIdentity();
}

void DaWorldRingSection::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// Must perform axial rotation in tick so things rotate smoothly with the planet's rotation.
	if(!last.isValid())
		InitSystemPosition();
	else if(IsInAnyScene())
	{
		m_RingSectionLock.lockForWrite();
		m_RotationAngle = m_ThinkRotation;
		AuMatrix4x4 tBody(m_ThinkTransform);
		m_RingSectionLock.unlock();
		ChangePosNotParent(AuAtomPos(ParentId(), tBody));
	}
	DaWorldLL::Tick(last, now);
}

void DaWorldRingSection::TickOnClient(const AuElapsedTimer& last)
{
	DaWorldLL::TickOnClient(last);
	InitSystemPosition();
}

unsigned int DaWorldRingSection::IsTocker() const
{
	if(IsInAnyScene())
		return 100;
	return DaWorldLL::IsTocker();
}

void DaWorldRingSection::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	// Perform axial rotation in tock because tick does not get called when no people or officers are in this system.
	// Axial rotation is needed to calculate sunlight properly.
	if(!IsInAnyScene())
	{
		InitSystemPosition();			// Do the rotation.
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else
	{
		ThinkPerformAxialRotation();	// Calculate the rotation.
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	DaWorldLL::Tock(last, now, scripts, tockTrans);
}

double DaWorldRingSection::AngleToRingSection() const
{
	// The star decides which ringworld section is the reference section for rotation of the ringworld.
	const DaStar* star = dynamic_cast<const DaStar*>(Parent());
	if(!star)
		return 0;
	const DaWorldRingSection* refSection = star->FindRingworldReferenceSection();
	if(!refSection)
		return 0;
	return refSection->AngleToPlanet() + (SectionAngle() * m_SectionIndex);
}

void DaWorldRingSection::ThinkPerformAxialRotation()
{
	// Select an angle for this ringworld section that is coordinated with all sections of the ringworld.
	double rangle = AngleToRingSection();

	double rot = CalcAngleOfRotation();

	// Change the world transformation matrix for this ringworld section.
	m_RingSectionLock.lockForWrite();
	m_ThinkTransform.LoadIdentity();

	// Ringworld section origin is at its orbit position.
	m_ThinkTransform.RotateZ(rangle);							// Rotate the model space to the planet's orbit position.
	m_ThinkTransform.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Location of this planet relative to the parent star.
	m_ThinkTransform.RotateZ(-rangle);							// Rotate the model space to the planet's orbit position.
	m_ThinkTransform.RotateX(PI/2.);							// Stand the ring section on its edge.
	m_ThinkTransform.RotateY(rangle - PI/2.);					// Orient to face the sun.

	// Update shadow square rotation angle.
	m_ThinkRotation = rot;
	m_RingSectionLock.unlock();
}

void DaWorldRingSection::InitSystemPosition()
{
	// Select an angle for this ringworld section that is coordinated with all sections of the ringworld.
	double rangle = AngleToRingSection();

	// Change the world transformation matrix for this ringworld section.
	AuMatrix4x4 tBody;

	// Ringworld section origin is at its orbit position.
	tBody.RotateZ(rangle);								// Rotate the model space to the planet's orbit position.
	tBody.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Translate to the location of this planet relative to the parent star.
	tBody.RotateZ(-rangle);								// Rotate the model space to the planet's orbit position.
	tBody.RotateX(PI/2.);								// Stand the ring section on its edge.
	tBody.RotateY(rangle - PI/2.);						// Orient to face the sun.

	// Update shadow square rotation angle.
	double v = CalcAngleOfRotation();
	m_RingSectionLock.lockForWrite();
	m_RotationAngle = v;
	m_RingSectionLock.unlock();
	m_ThinkRotation = v;
	m_ThinkTransform = tBody;
	ChangePosNotParent(AuAtomPos(ParentId(), tBody));
}

// Provides information about the planet's terrain at a given surface location.
void DaWorldRingSection::QueryTerrainAtLoc(AuTerrainInfo& ter, const AuPoint& loc) const
{
	Q_ASSERT(!loc.IsNaN());
	AuGlobeLL ll;												// Get longitude/latitude coordinate of surface location.
	if(!LLOfLoc(ll, loc))
	{
		qWarning() << "DaWorldRingSection::QueryTerrainAtLoc" << Id() << Name() << "loc" << loc << "outside terrain space";
		return;
	}
	QueryTerrain(ter, ll);
}

// Provides information about the planet's terrain at a given surface location.
void DaWorldRingSection::QueryTerrain(AuTerrainInfo& ter, const AuGlobeLL& ll) const
{
	double altMSL = SurfaceAltitudeMSL(ll);
	int inEnclosure = InEnclosure(ll, altMSL);
	if(inEnclosure < 0)
		inEnclosure = 0;
	int isIrrigated = IsIrrigated(ll);

	// Fill the AuTerrainInfo object with data about this world.
	ter.m_ElevType = TerrainElevationType(altMSL);
	if(SnowCoverage(altMSL, ll.Latitude(), inEnclosure) > .3f)
		ter.m_ElevType = TER_Alpine;							// This results in the sound of walking in snow.
	ter.m_byFoliage = VegetationAt(ll, altMSL, inEnclosure, isIrrigated);		// World foliage density at long/lat.
	ter.m_Radius100km = 500;										// Ringworld
	ter.m_byWater = m_byWater;
	ter.m_InEnclosure = inEnclosure;
	ter.m_Irrigation = isIrrigated;
	ter.m_rSeaLevelRadius = 0;
	ter.m_rSurfaceRadius = altMSL;
}

// Determine the quantity of sunlight striking the surface location from each sun.
// Light level is adjusted for water depth.
// This does not consider sloping terrain or shadows.
// In habitable zone, noon sun intensity is 1.0.
float DaWorldRingSection::QuerySunlight(const AuPoint &fSurfLoc) const
{
	// Search up the atom hierarchy for a solar system.
	const DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return 0;	// wut?
	DaStar* star = env->FindStar();
	if(!star)
		return 0;	// Possible in design studio.

	double seaDepth = 0;
	if(Hydrographics())
	{
		// World with water applies illumination dropoff when below sea level.
		double altitude = AltitudeMSL(fSurfLoc);
		if(altitude < 0)
			seaDepth = -altitude;
	}

	// Make a list of the suns, up to three.
	DaAtomList suns;
	suns.append(star);											// Get the primary star.
	star->ListStars(suns);										// Get companion star(s), if any.

	// Determine the direction and intensity of the light from each star.
	float totalSunlight = 0;
	const AuMatrix4x4& mySysT = EnvTransform();
	for(DaAtomList::const_iterator si = suns.begin(); si != suns.end(); si++)
	{
		const DaStar* ps = (const DaStar*)*si;
		if(ps->IsAnyBlackHole())
			continue;
		float atten = ps->LightAttenuation(seaDepth);
		if(!atten)
			continue;
		AuPoint sunLoc = mySysT.UnProject(ps->EnvLocation());				// Sun location relative to this planet.
		if(ps->Is(ParentId()))
		{
			// The sun is my parent sun. Modulate lighting using its shadow panels.
			const DaWorldRingSection* ringsect = ps->FindRingworldReferenceSection();
			if(!ringsect)
				ringsect = this;
			AuPoint xvec = mySysT.UnProjectVector(ps->EnvTransform().ProjectVector(AuPoint(1,0)));
			AuPoint yvec = mySysT.UnProjectVector(ps->EnvTransform().ProjectVector(AuPoint(0,1)));
			totalSunlight += ComputeSunLight(fSurfLoc, sunLoc, atten, ps->Radius(), ringsect->RingworldSections(),
					ShadowPanelSunAngle(ringsect->AngleOfRotation()), xvec, yvec);
		}
		else
		{
			// The sun is not at the center of this ringworld.
			totalSunlight += ComputeSunLight(fSurfLoc, sunLoc, atten, ps->Radius());
		}
	}
	return totalSunlight;
}

// Moving a subatom uses the following logic.
// - New location is calculated.
// - Delta to remain on terrain is calculated.
//   - Too much rise and movement is stopped.
//   - Too much loss and movement falls downward a fixed distance.
// - New location is checked for potential handoff to a subatom or to parent atom.
bool DaWorldRingSection::MoveSubatom(DaAtom *pMove, const AuPoint &vMove, int elapsedMs)
{
	// Determine the destination point of the move.
	AuPoint foldloc(pMove->Location());
	AuPoint fnuloc = foldloc + vMove;
	bool moveInAtmo = FilterDestinationLoc(pMove, foldloc, fnuloc, elapsedMs);

#if !defined(QT_NO_DEBUG)
	if(fnuloc.IsNaN())
	{
		qDebug() << "DaWorldRingSection::MoveSubatom" << Id() << Name() << "atom" << pMove->Id() << pMove->Name()
			<< "fnuloc" << fnuloc << fnuloc.IsNaN() << "vMove" << vMove << vMove.IsNaN() << "IsNaN ERROR!";
	}
#endif

	// Determine the result of attempting to move to the nu location.
	DaAtom *pHit = NULL;
	AuPoint hitLoc, hitNormal;
	COLLISION_TYPE eCollide = pMove->CheckCollision(foldloc, fnuloc, pHit, hitLoc, hitNormal);
	pMove->SetMoveResult(eCollide);
	//qDebug() << "DaWorldRingSection::MoveSubatom" << Id() << Name() << "move" << pMove->Id() << pMove->Name()
		//<< "CheckCollision" << eCollide << "oldloc" << foldloc << "nuloc" << fnuloc
		//<< "fallDist" << pMove->FallDistance() << "vector" << pMove->Vector();

	switch(eCollide)
	{
	case CT_HitParent:
		//qDebug("Hit parent %s", pHit->Name());
		// The moving sub atom hit this atom.
		// Calculate surface normal relative to the moving atom.
		hitNormal = pMove->Transform().UnProjectVector(hitNormal);
		pMove->MovementBlocked(this, hitLoc, hitNormal, false);
		return false;
	case CT_HitPeer:
	case CT_HitPeerShield:
		//qDebug("Hit Peer %s", pHit->Name());
		{
			// Reflect its movement vector.
			// Calculate surface normal relative to the hit atom's parent, which is this atom.
			hitNormal = pHit->Transform().ProjectVector(hitNormal);

			// Calculate surface normal relative to the moving atom.
			hitNormal = pMove->Transform().UnProjectVector(hitNormal);
			pMove->MovementBlocked(pHit, hitLoc, hitNormal, eCollide == CT_HitPeerShield);
		}
		return false;
	case CT_JoinedPeer:
		// Move the atom subordinate to its nu parent.
		if(pHit)
		{
			//qDebug("DaWorldRingSection::MoveSubatom - %s Entered %s %s",
					//(const char*)pMove->Name(),
					//(const char *)pHit->Name(),
					//(const char*)pHit->StringId());
			bool wasInAtmosphere = pMove->IsInAtmosphere();
			//double oldcgdist = pMove->Location().Length();
			// Change the world transformation to be subordinate to the nu
			// parent atom.
			AuMatrix4x4 tBody;
			tBody.Translate(fnuloc);
			tBody.Rotate(pMove->Rotation());
			AuMatrix4x4 t = pHit->Transform().Inverse() * tBody;

			// Combine the movement vectors.
			AuPoint nuMove(pMove->Vector());
			if(!pHit->Vector().IsZero())
				nuMove -= pMove->Transform().UnProjectVector(pHit->Vector());
			pMove->ForcePosMove(AuAtomPos(pHit->Id(), t), nuMove);
			pMove->SetModified();
			pMove->InvalidateEnvironment();	// Refresh environment on atom when it moves, in this thread.

			// Player Statistics
			if(!wasInAtmosphere && pMove->IsInAtmosphere())
			{
				// Moving atom entered the atmosphere range of this planet.
				DaPerson* person = dynamic_cast<DaPerson*>(pMove);
				if(person)
					person->Stats().PlanetEntry(this, 0);
				DaAtomList people;
				pMove->FindPeopleAwakeRecursive(people);
				for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
					((DaPerson*)*pi)->Stats().PlanetEntry(this, pMove);
			}
		}
		return true;
	case CT_Miss:
		//qDebug("DaWorldRingSection::MoveSubatom - %s Missed",
				//(const char*)pMove->Name());
		{
			// The moving atom did not hit anything.
			// Watch for spacecraft entering wormholes when they are not subordinate to the primary star.
			DaVessel* ship = dynamic_cast<DaVessel*>(pMove);
			if(ship && ship->IsReadyToWormhole() && !ship->InDesignEnvironment())
			{
				// The ship is ready to wormhole.
				// Check the movement vector against the primary star's wormholes.
				DaSolarSystem* pss = FindParentSolarSystem();
				DaStar* star;
				if(ship->FTLDriveSetting(ship->FTLWormholeDrive()) == 2)
					star = pss->FindPrimaryStar();
				else
					star = pss->FindPrimaryStarNotBlackHole();
				if(star)
				{
					AuMatrix4x4 tstar;
					if(GetTransformRelativeToParent(tstar, star))
					{
						// This atom is subordinate to the "primary" star.
						AuPoint ostarloc = tstar.Project(foldloc);		// Ship's old location relative to the primary star.
						AuPoint nstarloc = tstar.Project(fnuloc);		// Ship's nu location relative to the primary star.
						float entryAngle = -1;
						AuWormhole wh = star->CheckWormholeEntry(ostarloc, nstarloc, entryAngle);
						if(wh.IsValid() && entryAngle >= 0)
						{
							if(g_Au.m_LogMessages & LOG_Travel)
								qDebug("DaWorldRingSection::MoveSubatom %d %s spacecraft %d %s entered wormhole.",
										Id().GetID(), (const char*)Name().toLatin1(),
										ship->Id().GetID(), (const char*)ship->Name().toLatin1());
							return star->TraverseWormhole(ship, wh, entryAngle);
						}
					}
				}
			}

			bool bCombine = true;
			if(moveInAtmo)
			{
				// Atom is inside the atmosphere.
				// Give the atom an opportunity to orient to the terrain.
				AuGlobeLL nuLL;
				LLOfLoc(nuLL, fnuloc);
				AuPoint terN = TerrainNormal(nuLL);
				if(pMove->Orient(fnuloc, terN))
					bCombine = false;
			}
			if(bCombine && !fnuloc.FuzzyEquals(foldloc))
			{
				// Move the atom to the nu location relative to 'this' atom.
				AuMatrix4x4 tBody;
				tBody.Translate(fnuloc-foldloc);
				tBody *= pMove->Transform();
				if(!pMove->ChangePosNotParent(AuAtomPos(Id(), tBody)))
					return false;
				pMove->InvalidateEnvironmentRecursive();
			}

			// Check to see if the nu location is outside the scope of
			// influence of this atom.
			if(CheckChildExit(foldloc, fnuloc, pMove))
			{
				// Move the atom to be subordinate to my parent atom.
				// Separate the movement vectors.
				AuPoint nuMove = pMove->Vector() + pMove->Transform().UnProjectVector(Vector());

				// Change the world transformation to be subordinate to the nu parent atom.
				AuMatrix4x4 t = Transform() * pMove->Transform();
				pMove->ForcePosMove(AuAtomPos(ParentId(), t), nuMove);

				// Immediately try moving into the next atom domain.
				if(pMove->IsNormal())
					Parent()->MoveSubatom(pMove, AuPoint(0,0,0), 0);
			}
			else
				pMove->InvalidateEnvironment();	// Refresh environment on atom when it moves, in this thread.
			pMove->SetModified();
			return true;
		}
		break;
	}
	return false;
}

// Returns false if nuLoc or oldLoc is outside the atmosphere.
bool DaWorldRingSection::FilterDestinationLoc(DaAtom* moveAtom, const AuPoint& oldLoc, AuPoint& nuLoc, int elapsedMs) const
{
	double oldCgAlt = InAtmosphereCGAltitude(oldLoc);
	if(oldCgAlt == -100000)
		return false;
	double nuCgAlt = InAtmosphereCGAltitude(nuLoc);
	if(nuCgAlt == -100000)
		return false;

	// Moving atom is inside the atmosphere.
	double stopMove = 0;				// How much movement was stopped for collision damage.
	int stopReason;
	if(moveAtom->IsAtBd())
	{
		// Moving atom is in a building, could be on the roof.
		if(nuCgAlt <= oldCgAlt)
		{
			// Moving atom is losing altitude or staying at the same altitude.
			// Keep feet from dropping through the floor|ground.
			// Calculate maximum distance the creature can move down before hitting the floor.
			double maxDrop = moveAtom->FallDistance();
			double drop = oldCgAlt - nuCgAlt;
			if(drop > maxDrop)
			{
				stopMove = drop - maxDrop;
				stopReason = 0;
				ElevateLoc(nuLoc, oldCgAlt - maxDrop);
			}
		}
		else
		{
			// Moving atom is gaining altitude.
			if(dynamic_cast<DaCreature*>(moveAtom))
			{
				// Prevent creature's head from going up through the ceiling.
				// Calculate maximum distance the creature can move up before hitting the ceiling.
				float maxClimb = moveAtom->CeilingDistance() - 1;
				float climb = nuCgAlt - oldCgAlt;								// Amount the creature is trying to move up.
				if(climb > maxClimb)
				{
					stopMove = climb - maxClimb;
					stopReason = 1;
					ElevateLoc(nuLoc, oldCgAlt + maxClimb);					// Limit nu cg altitude.
				}
			}
		}
	}
	else
	{
		// Moving atom is not in a building and it cannot go underground, like a subterranean creature.
		bool canBeUg = moveAtom->CanBeUnderGround();
		if(!canBeUg && nuCgAlt <= oldCgAlt)
		{
			// Moving atom is losing altitude or staying at the same altitude.
			// Keep feet from dropping through the floor|ground.
			// Calculate maximum distance the creature can move down before hitting the floor.
			double maxDrop = moveAtom->FallDistance();
			double drop = oldCgAlt - nuCgAlt;
			if(drop > maxDrop)
			{
				stopMove = drop - maxDrop;
				stopReason = 2;
				ElevateLoc(nuLoc, oldCgAlt - maxDrop);
			}
		}

 		// Atom is inside the atmosphere.
 		// Calculate terrain elevation info at nu cg location.
		AuGlobeLL nuLL;
		if(!LLOfLoc(nuLL, nuLoc))
			return false;
 		double terrainAlt = SurfaceAltitudeMSL(nuLL);
		double cght = moveAtom->GroundHeightAboveCG();
 		double nuGroundAlt = nuCgAlt - cght;									// Altitude of atom at the ground beneath its CG.
 
 		// Stop at terrain surface if atom cannot be under ground.
		// Calculate the terrain elevation delta from the destination ground location to the terrain surface.
		double adjustAlt = terrainAlt - nuGroundAlt;							// Positive is up; negative is down.
		if(!IsFuzzyZero(adjustAlt))
		{
			if(adjustAlt > 0)
			{
				// Atom is moving below the terrain.
				if(!canBeUg)
				{
					stopMove += adjustAlt;
					stopReason = 2;
					ElevateLoc(nuLoc, terrainAlt + cght);						// Adjust the altitude up to the terrain.
				}
			}
			else if(moveAtom->IsOnGround() && !moveAtom->IsInWater() && !moveAtom->MoveResultBlocked())
			{
				// Atom is moving above the terrain while near the ground.
				// Help them to adhere to the terrain as they go around my spherical surface.
				// Otherwise vehicles have problems going down hill at high speed, especially in low gravity.
				if(moveAtom->Vector().IsZero())
				{
					ElevateLoc(nuLoc, terrainAlt + cght);						// Adjust the altitude down to the terrain.
				}
				else if(nuCgAlt <= oldCgAlt)
				{
					// Moving atom is losing altitude or staying at the same altitude.
					double gdot = moveAtom->Transform().ProjectVector(moveAtom->Vector()).Dot(GravityDir(nuLoc));
					if(gdot > 0)
						ElevateLoc(nuLoc, terrainAlt + cght);					// Adjust the altitude down to the terrain.
				}
			}
		}
	}

	if(stopMove > 0 && elapsedMs > 0)
		moveAtom->FallStopped(oldCgAlt, nuCgAlt, stopMove, elapsedMs, stopReason);
	return true;
}

// Returns true if a child moving to 'fnuloc' from 'foldloc' departs the influence of this atom.
bool DaWorldRingSection::CheckChildExit(const AuPoint &/*foldloc*/, const AuPoint &fnuloc, const DaAtom* /*who*/) const
{
	// Check if too far above/below the plane of the ecliptic of the ringworld.
	if(fabs(fnuloc.y) > FeetPerOrbit()/5.)
		return true;											// too far above or below the plane of the ecliptic
	
	// Check if too far inside or outside the radius of the ringworld.
	double ringRad = OrbitRadius();								// ringworld radius in feet at sea level
	double solarRadius = AuPoint(fnuloc.x, fnuloc.z - ringRad).Length_2D();	// radius from sun, in plane of the ecliptic
	if(solarRadius > ringRad + FeetPerMoonOrbit())
		return true;											// too far beyond the ring
	//if(solarRadius < ringRad - FeetPerOrbit()/2.)
	if(solarRadius < ringRad - FeetPerMoonOrbit()*2.25)
		return true;											// too far inside the ring

	// Check if fnuloc is outside the sweep angle of this ringworld section. No margin is permitted so atoms can transition to
	// other sections.
	double lon = AuPoint(ringRad - fnuloc.z, fnuloc.x).ZRotation();
	if(fabs(lon) > SectionAngle()/2.)
		return true;											// outside sweep angle of ringworld section.

	return false;
}

// Returns true if movement from foldloc to fnuloc would enter the area of
// influence of this atom.
// 'foldloc' and 'fnuloc' are relative to the parent.
bool DaWorldRingSection::CheckPeerJoin(const AuPoint &/*foldloc*/, const AuPoint &fnuloc, const DaAtom*)
{
	// Check if too far above/below the plane of the ecliptic of the ringworld.
	AuPoint nloc = Transform().UnProject(fnuloc);				// fnuloc in my coordinate space.
	if(fabs(nloc.y) > FeetPerOrbit()/5.)
		return false;											// too far above or below the plane of the ecliptic
	
	// Check if too far inside or outside the radius of the ringworld.
	double solarRadius = AuPoint(nloc.x, nloc.z - OrbitRadius()).Length_2D();	// radius from sun, in plane of the ecliptic
	double ringRad = OrbitRadius();								// ringworld radius in feet at sea level
	if(solarRadius > ringRad + FeetPerMoonOrbit())
		return false;											// too far beyond the ring
	//if(solarRadius < ringRad - FeetPerOrbit()/2.)
	if(solarRadius < ringRad - FeetPerMoonOrbit()*2.25)
		return false;											// too far inside the ring

	// Check if nloc is outside the sweep angle of this ringworld section. No margin is permitted so atoms can transition to
	// other sections.
	double lon = AuPoint(ringRad - nloc.z, nloc.x).ZRotation();
	if(fabs(lon) > SectionAngle()/2.)
		return false;											// outside sweep angle of ringworld section.

	return true;
}

// List atoms that should be in the scene when this atom is in the scene, i.e. IsInScene.
void DaWorldRingSection::ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList& sceneAtoms)
{
	if(!HasSubatoms())
		return;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	if(pSceneAtom->FindParentPlanetInEnv() == this)
	{
		// Scene atom is AT this planet.
		if(pSceneAtom->IsInAtmosphere())
		{
			// Scene atom is inside the atmosphere.
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				// Only list moons and terrain patches subordinate to this planet.
				DaWorld* moon = dynamic_cast<DaWorld*>(*it);
				if(moon)
				{
					// Add moons to the list but do not add any of their subatoms.
					sceneAtoms.AddIfUnique(moon);
				}
				else
				{
					if((*it)->IsInScene(pSceneAtom, cameraEnvLoc))
					{
						sceneAtoms.AddIfUnique(*it);
						(*it)->ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
					}
				}
			}
		}
		else
		{
			// Scene atom is not inside the atmosphere.
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaWorld* moon = dynamic_cast<DaWorld*>(*it);
				if(moon)
				{
					// Add moons to the list but do not add any of their subatoms.
					sceneAtoms.AddIfUnique(moon);
				}
				else
				{
					// Add other atoms and their subatoms that want to be included. Terrain will exclude itself here.
					if(!(*it)->IsInScene(pSceneAtom, cameraEnvLoc))
						continue;
					sceneAtoms.AddIfUnique(*it);
					(*it)->ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
				}
			}
		}
	}
	else
	{
		// The scene atom is not at this ring section.
		// List asteroid at this ring section.
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			if(!dynamic_cast<DaAsteroid*>(*it))
				continue;
			// Add the atom to the list.
			sceneAtoms.AddIfUnique(*it);
		}
	}
}

// Returns an information about the natural resources available in the
// environment of this world.
void DaWorldRingSection::QueryNaturalResources(AuZoneResource& res, int resourceZone) const
{
	QueryNaturalResources(res, resourceZone, GenSeed());
}

// Returns an information about the natural resources available in the
// environment of this world.
void DaWorldRingSection::QueryNaturalResources(AuZoneResource& res, int resourceZone, quint32 worldGenSeed) const
{
	Q_ASSERT(resourceZone >= 0 && resourceZone < ResourceZones());
	m_ResourcesLock.lockForRead();
	bool hasRes = m_Resources;
	if(hasRes)
		res = m_Resources[resourceZone];
	m_ResourcesLock.unlock();
	if(hasRes)
		return;

	GALAXY galaxy = Galaxy();
	m_ResourcesLock.lockForWrite();
	if(m_Resources)
	{
		m_ResourcesLock.unlock();
		return;
	}
	m_Resources = new AuZoneResource[ResourceZones()];

	// Seed the random number generator for consistent values for the planet.
	AuDice dice;
	dice.Seed(worldGenSeed);
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		// Atmospheric Resources don't vary between zones.
		if(rz == 0)
		{
			// Orbit zone.
			m_Resources[rz].m_AtmType = AtmosphereType();
			m_Resources[rz].m_Zone = OrbitZone();

			m_Resources[rz].m_Radius100km = 500;	// Ringworld
			const DaStar* star = FindParentStarInEnv();
			if(star)
				m_Resources[rz].m_StarType = star->StarType();

			// General atmosphere.
			m_Resources[rz].m_Atmosphere = (unsigned char)(MSLAtmosphereDensity() * 100);	// 0-100
			m_Resources[rz].m_AtmosphereQ = dice.RandomQuality();

			// Atmospheric components.
			m_Resources[rz].m_AirQ = dice.RandomQuality();
			m_Resources[rz].m_AntifluxParticleQ = dice.RandomQuality();
			m_Resources[rz].m_BorexinoPrecipQ = dice.RandomQuality();
			m_Resources[rz].m_CryozineQ = dice.RandomQuality();
			m_Resources[rz].m_HydrogenQ = dice.RandomQuality();
			m_Resources[rz].m_IoplasmaQ = dice.RandomQuality();

			// Sunlight in habitable zone = 50
			m_Resources[rz].m_Sunlight = 90 - (OrbitZone() * 20);
			int sunQ = dice.RandomQuality();
			m_Resources[rz].m_SunlightQ = MAX(sunQ, 10);
		}
		else
		{
			// Orbit zone.
			m_Resources[rz].m_AtmType = m_Resources[0].m_AtmType;
			m_Resources[rz].m_Zone = m_Resources[0].m_Zone;

			m_Resources[rz].m_Radius100km = m_Resources[0].m_Radius100km;
			m_Resources[rz].m_StarType = m_Resources[0].m_StarType;

			// General atmosphere.
			m_Resources[rz].m_Atmosphere = m_Resources[0].m_Atmosphere;
			m_Resources[rz].m_AtmosphereQ = m_Resources[0].m_AtmosphereQ;

			// Atmospheric components.
			m_Resources[rz].m_AirQ = m_Resources[0].m_AirQ;
			m_Resources[rz].m_AntifluxParticleQ = m_Resources[0].m_AntifluxParticleQ;
			m_Resources[rz].m_BorexinoPrecipQ = m_Resources[0].m_BorexinoPrecipQ;
			m_Resources[rz].m_CryozineQ = m_Resources[0].m_CryozineQ;
			m_Resources[rz].m_HydrogenQ = m_Resources[0].m_HydrogenQ;
			m_Resources[rz].m_IoplasmaQ = m_Resources[0].m_IoplasmaQ;

			// Sunlight in habitable zone = 50
			m_Resources[rz].m_Sunlight = m_Resources[0].m_Sunlight;
			m_Resources[rz].m_SunlightQ = m_Resources[0].m_SunlightQ;
		}

		// Plant Resources
		{
			// Use average foliage density of terrain that is above water in the zone.
			m_Resources[rz].m_Vegetation = AverageFoliage();		
			if(m_Resources[rz].m_Vegetation > 100)
				m_Resources[rz].m_Vegetation = 100;
			m_Resources[rz].m_VegetationQ = dice.RandomQuality();

			if(m_Resources[rz].m_Vegetation)
			{
				m_Resources[rz].m_Beans = dice.Roll(1, (m_Resources[rz].m_Vegetation+1)/2);
				m_Resources[rz].m_BeansQ = dice.RandomQuality();

				m_Resources[rz].m_Fiber = dice.Roll(1,m_Resources[rz].m_Vegetation);
				m_Resources[rz].m_FiberQ = dice.RandomQuality();

				m_Resources[rz].m_Fruit = dice.Roll(1, (m_Resources[rz].m_Vegetation+1)/2);
				m_Resources[rz].m_FruitQ = dice.RandomQuality();

				m_Resources[rz].m_Grain = dice.Roll(1,m_Resources[rz].m_Vegetation);
				m_Resources[rz].m_GrainQ = dice.RandomQuality();

				m_Resources[rz].m_Grapes = dice.Roll(1, (m_Resources[rz].m_Vegetation+2)/3);
				m_Resources[rz].m_GrapesQ = dice.RandomQuality();

				m_Resources[rz].m_Herbs = dice.Roll(1, (m_Resources[rz].m_Vegetation+1)/2);
				m_Resources[rz].m_HerbsQ = dice.RandomQuality();

				m_Resources[rz].m_Hops = dice.Roll(1, (m_Resources[rz].m_Vegetation+2)/3,-2);
				m_Resources[rz].m_HopsQ = dice.RandomQuality();

				m_Resources[rz].m_Nuts = dice.Roll(1,(m_Resources[rz].m_Vegetation+1)/2);
				m_Resources[rz].m_NutsQ = dice.RandomQuality();

				m_Resources[rz].m_Spices = dice.Roll(1, (m_Resources[rz].m_Vegetation+1)/2);
				m_Resources[rz].m_SpicesQ = dice.RandomQuality();

				m_Resources[rz].m_Vegetables = dice.Roll(1, (m_Resources[rz].m_Vegetation+1)/2);
				m_Resources[rz].m_VegetablesQ = dice.RandomQuality();

				m_Resources[rz].m_Wood = dice.Roll(1,(m_Resources[rz].m_Vegetation+1)/2);
				m_Resources[rz].m_WoodQ = dice.RandomQuality();
			}
			else
			{
				// Quality factors are required even though the environment doesn't provide these things.
				m_Resources[rz].m_Beans = 0;
				m_Resources[rz].m_BeansQ = dice.RandomQuality();

				m_Resources[rz].m_Fiber = 0;
				m_Resources[rz].m_FiberQ = dice.RandomQuality();

				m_Resources[rz].m_Fruit = 0;
				m_Resources[rz].m_FruitQ = dice.RandomQuality();

				m_Resources[rz].m_Grain = 0;
				m_Resources[rz].m_GrainQ = dice.RandomQuality();

				m_Resources[rz].m_Grapes = 0;
				m_Resources[rz].m_GrapesQ = dice.RandomQuality();

				m_Resources[rz].m_Herbs = 0;
				m_Resources[rz].m_HerbsQ = dice.RandomQuality();

				m_Resources[rz].m_Hops = 0;
				m_Resources[rz].m_HopsQ = dice.RandomQuality();

				m_Resources[rz].m_Nuts = 0;
				m_Resources[rz].m_NutsQ = dice.RandomQuality();

				m_Resources[rz].m_Spices = 0;
				m_Resources[rz].m_SpicesQ = dice.RandomQuality();

				m_Resources[rz].m_Vegetables = 0;
				m_Resources[rz].m_VegetablesQ = dice.RandomQuality();

				m_Resources[rz].m_Wood = 0;
				m_Resources[rz].m_WoodQ = dice.RandomQuality();
			}
		}

		// Animal Resources
		{
			if(CanHaveFauna())
				m_Resources[rz].m_Animals = dice.Roll(1, m_Resources[rz].m_Vegetation);
			else
				m_Resources[rz].m_Animals = 0;
			m_Resources[rz].m_AnimalsQ = dice.RandomQuality();
			m_Resources[rz].m_CheeseQ = dice.RandomQuality();
			m_Resources[rz].m_EggsQ = dice.RandomQuality();
			m_Resources[rz].m_FertilizerQ = dice.RandomQuality();
			m_Resources[rz].m_MilkQ = dice.RandomQuality();

			if(HydrographicLiquid() == HYDRO_Water)
			{
				// Surprise, fish may be found in any liquid seas.
				m_Resources[rz].m_Fish = dice.Roll(1,Hydrographics());
				m_Resources[rz].m_FishQ = dice.RandomQuality();
			}
			else
			{
				// Fish don't live in seas of lava or liquid gases.
				m_Resources[rz].m_Fish = 0;
				m_Resources[rz].m_FishQ = 0;
			}
		}

		// Mineral Resources
		{
			// Stone prevalence reduced by vegetation
			m_Resources[rz].m_Stone = dice.Roll(1,100, -m_Resources[rz].m_Vegetation*2/3);
			m_Resources[rz].m_Stone = MAX(m_Resources[rz].m_Stone, 10);	// >= 10
			m_Resources[rz].m_StoneQ = dice.RandomQuality();

			m_Resources[rz].m_Crystals = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Crystals = MAX(m_Resources[rz].m_Crystals, 2); // >= 2
			m_Resources[rz].m_CrystalsQ = dice.RandomQuality();

			// Eludium is not found on planets with a significant atmosphere.
			if(galaxy == MilkyWay)
				m_Resources[rz].m_Eludium = 0;	// No eludium in Milky Way galaxy.
			else if(m_Resources[rz].m_Atmosphere < 15)
			{
				// This should be fairly abundant.
				m_Resources[rz].m_Eludium = dice.Roll(2,m_Resources[rz].m_Stone, -m_Resources[rz].m_Atmosphere);
				m_Resources[rz].m_Eludium = MAX(m_Resources[rz].m_Eludium, 1); // >= 1
			}
			else
				m_Resources[rz].m_Eludium = 0;	// No eludium here.
			m_Resources[rz].m_EludiumQ = dice.RandomQuality();

			m_Resources[rz].m_Gems = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Gems = MAX(m_Resources[rz].m_Gems, 2);	// >= 2
			m_Resources[rz].m_GemsQ = dice.RandomQuality();

			m_Resources[rz].m_GoldOre = 0;	// No concentrations of gold ore on ringworlds.
			m_Resources[rz].m_GoldOreQ = dice.RandomQuality();

			m_Resources[rz].m_Ice = 100;
			m_Resources[rz].m_IceQ = dice.RandomQuality();

			// Lumenite is not found on ringworlds or in the Milky Way.
			m_Resources[rz].m_Lumenite = 0;
			m_Resources[rz].m_LumeniteQ = dice.RandomQuality();

			m_Resources[rz].m_Minerals = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Minerals = MAX(m_Resources[rz].m_Minerals, 2);
			m_Resources[rz].m_MineralsQ = dice.RandomQuality();

			m_Resources[rz].m_Ore = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Ore = MAX(m_Resources[rz].m_Ore, 2);
			m_Resources[rz].m_OreQ = dice.RandomQuality();

			m_Resources[rz].m_Radioactives = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Radioactives = MAX(m_Resources[rz].m_Radioactives, 2);
			m_Resources[rz].m_RadioactivesQ = dice.RandomQuality();

			// Water is considered to be lava or supercooled liquid in inferno and frigid environments respectively.
			// The quality does not vary by resource zone.
			m_Resources[rz].m_WaterInEnvironment = Hydrographics();
			m_Resources[rz].m_WaterInEnvironmentQ = dice.RandomQuality();

			// Nonessential specialty mineral resources with quantity based on hydrographics.
			m_Resources[rz].m_MagmexQ = dice.RandomQuality();
			m_Resources[rz].m_MyrathaneQ = dice.RandomQuality();
			if(rz != 0)
			{
				// These are overridden in this way rather than no rolling them so as not to upset the current resource balance
				// in the existing terrain.
				m_Resources[rz].m_WaterInEnvironmentQ = m_Resources[0].m_WaterInEnvironmentQ;

				// Nonessential specialty mineral resources with quantity based on hydrographics.
				m_Resources[rz].m_MagmexQ = m_Resources[0].m_MagmexQ;
				m_Resources[rz].m_MyrathaneQ = m_Resources[0].m_MyrathaneQ;
			}

			// Nonessential specialty mineral resources that exist as visible resources in the environment.
			m_Resources[rz].m_BoliteQ = dice.RandomQuality();
			m_Resources[rz].m_PhlogistonQ = dice.RandomQuality();
			m_Resources[rz].m_PolytarideQ = dice.RandomQuality();
			m_Resources[rz].m_VulcaniteQ = dice.RandomQuality();

			// These resources are never found on the cozy worlds.
			m_Resources[rz].m_Bolite = 0;
			m_Resources[rz].m_Phlogiston = 0;
			m_Resources[rz].m_Polytaride = 0;
			m_Resources[rz].m_Vulcanite = 0;
		}

		// Hydrocarbon Resources
		{
			// Presence of natural resources is determined by orbit zone.
			int icoal=0, ioil=0, igas=0;
			// Hydrocarbon resources are scarce on ringworlds.
			icoal = dice.Roll(2,8,-1);
			ioil = dice.Roll(2,6,-1);
			igas = dice.Roll(2,4,-1);
			icoal = MIN(icoal, 100);
			icoal = MAX(icoal, 0);
			ioil = MIN(ioil, 100);
			ioil = MAX(ioil, 0);
			igas = MIN(igas, 100);
			igas = MAX(igas, 0);

			m_Resources[rz].m_Coal = icoal;
			m_Resources[rz].m_CoalQ = dice.RandomQuality();

			m_Resources[rz].m_CrudeOil = ioil;
			m_Resources[rz].m_CrudeOilQ = dice.RandomQuality();

			m_Resources[rz].m_NaturalGas = igas;
			m_Resources[rz].m_NaturalGasQ = dice.RandomQuality();
		}
	}

	// Guaranteed valid location of mineral resources. To prevent the situation where a resource is reported but can never be found.
	// Calculate eqDivs exactly the way SpotRockAt does so.
	int eqDivs = (int)(TerrainXSize() / 20);					// 20' spacing for rocks at highest density.
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_Resources[rz].m_AdamantiteLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_BoliteLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_CoalLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_CrudeOilLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_CrystalsLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_EludiumLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_GemsLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_IceLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_LumeniteLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_MineralsLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_OreLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_PhlogistonLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_PolytarideLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_RadioactivesLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_ViatholLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_VulcaniteLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
		m_Resources[rz].m_GoldOreLL = GuaranteedRockLocationResLocked(dice, eqDivs, rz);
		dice.Random();
	}

	res = m_Resources[resourceZone];
	m_ResourcesLock.unlock();
}

void DaWorldRingSection::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId,
		quint32 scannerEmpire, int scannerDomain, const AuId& scannerWorld,
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

	if(scanMe)
	{
		AuPoint sysLoc = EnvLocation();
		AuPoint sysLocw = EnvTransform().Project(LocOfLL(AuGlobeLL(-PI,0)));	// West end location
		AuPoint sysLoce = EnvTransform().Project(LocOfLL(AuGlobeLL(PI,0)));	// East end location
		bool inSensorRange = sysLoc.Distance(scannerLoc) < scannerSensorRange ||
			sysLocw.Distance(scannerLoc) < scannerSensorRange ||
			sysLoce.Distance(scannerLoc) < scannerSensorRange;

		blips.Add(AuBlipRingSection(SectionIndex(), inSensorRange, Id(), sysLoc));

		// Add anomalies at active henge portals.
		// All ringworld arc sections have henge portals.
		//if((scanContent & 0x0001) && HasHenge())
		if(scanContent & 0x0001)
		{
			const AuGeologicalFeatures& geof = LockGeologicalFeatures();
			for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
			{
				const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
				if(!henge || !henge->IsActive())
					continue;

				AuPoint hengeCenterLoc = LocOfLL(henge->LL(), henge->CentralSlabAltitudeMSL());
				hengeCenterLoc = EnvTransform().Project(hengeCenterLoc);
				if(hengeCenterLoc.Distance2(scannerLoc) > AuSensorBlip::AnomalyDetectionRange())
					continue;	// Anomalies require close proximity to detect.
				blips.Add(AuBlipAnomaly(Id(), hengeCenterLoc));
			}
			UnlockGeologicalFeatures();
		}
	}

	if(HasSubatoms())
	{
		// I am scanning my subatoms and I actually have some.
		DaWorldLL::SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
				scannerWeaponRange, scanContent, lockedBlip, stanceCache);
	}
}

// Insert first time database record.
void DaWorldRingSection::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	// Storing the solar system id with planets makes the empire status report queries a lot faster.
	// The id of the parent solar system never changes so we don't update it ever again.
	AuId systemId;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(system)
		systemId = system->Id();

	QSqlQuery qa(QString(), db);

	// Assemble an INSERT query based on the variable presense of vegetation, flora, and fauna data.
	// Start with the static pieces of the INSERT query.
	QString fields("id,idp, locX,locY,locZ, rotX,rotY,rotZ, timeCreate,timeModified,timeTock,timeDeath, "
			"ids,name,orbitRadius,orbitZone, "
			"captId,captName,claimedBy,claimLon,claimLat,claimId,claimName,claimTime,geoFeatures, "
			"atmDensity,atmType,dna,indSpec,sectionIndex,terrain,water,village");
	QString qmarks("?,?, ?,?,?, ?,?,?, ?,?,?,?, "
			"?,?,?,?, "
			"?,?,?,?,?,?,?,?,?, "
			"?,?,?,?,?,?,?,?");

	// Add the variable pieces to the fields and question marks.
	if(m_pFlora)
	{
		fields += ",flora";
		qmarks += ",?";
	}
	if(m_pFauna)
	{
		fields += ",fauna";
		qmarks += ",?";
	}

	// Build the full query out of the pieces;
	QString sq = QString("INSERT INTO a_WorldRingSection (") + fields + ") VALUES (" + qmarks + ");";
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

	// a_World fields
	qa.addBindValue(systemId.GetID());
	qa.addBindValue(Name());
	qa.addBindValue(OrbitRadiusAu());
	qa.addBindValue(OrbitZone());

	// a_WorldLL fields
	qa.addBindValue(CaptId().GetID());
	qa.addBindValue(CaptName());
	qa.addBindValue(ClaimedByEmpire());
	qa.addBindValue(ClaimedAtLL().Longitude());
	qa.addBindValue(ClaimedAtLL().Latitude());
	qa.addBindValue(ClaimedByPersonId().GetID());
	qa.addBindValue(ClaimedByPersonName());
	qa.addBindValue(ClaimTime().UTC());
	{
		// Geological Features
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		LockGeologicalFeatures() >> out;
		UnlockGeologicalFeatures();
		qa.addBindValue(ba);
	}

	// a_WorldRingSection fields
	qa.addBindValue(m_byAtmDensity);
	qa.addBindValue(m_AtmType);
	{
		// DNA of indigenous.
		QByteArray ba;
		qa.addBindValue(ba);	// Empty the old dna field.
		if(m_pSentient)
		{
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_5_7);
			out << *m_pSentient;
		}
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_SectionIndex);
	{
		// Terrain data.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);

		out << m_TerrainFrequency;
		out << m_TerrainPersFactor;
		out << m_TerrainPersistence;
		out << m_TerrainXOff;
		out << m_TerrainYOff;
		out << m_TerrainWaterFactor;

		qa.addBindValue(ba);
	}
	qa.addBindValue(m_byWater);

	{
		// Village(s)
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);

		quint8 count;	// In case we decide to have more than one village per planet.
		const AuVillage* village = LockVillage();
		if(village)
		{
			count = 1;
			out << count;
			out << *village;
		}
		else
		{
			count = 0;
			out << count;
		}
		UnlockVillage();

		qa.addBindValue(ba);
	}

	// Flora
	if(m_pFlora)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz] >> out;
		out << m_tmFloraChange.UTC();
		QByteArray bac = qCompress(ba);                 // Compress the flora data and store it.
		qa.addBindValue(bac);
	}

	// Fauna
	if(m_pFauna)
	{
		const unsigned int uisize = m_pFauna->DataSize()*ResourceZones();
		QByteArray ba = qCompress(m_pFauna[0].Data(), uisize);
		qa.addBindValue(ba);
	}

	if(!qa.exec())
		qWarning("DaWorldRingSection::Insert %s FAILED. %s sq '%s'",
				(const char*)Name().toLatin1(),
				(const char*)qa.lastError().text().toLatin1(),
				(const char*)sq.toLatin1());
}

// Update existing database record.
void DaWorldRingSection::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq("UPDATE a_WorldRingSection SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, timeModified=?, "
			"timeTock=?, timeDeath=?, "
			"name=?, "
			"captId=?, captName=?, claimedBy=?, claimLon=?, claimLat=?, claimId=?, claimName=?, claimTime=?, geoFeatures=?, "
			"dna=?, indSpec=?, village=?");
	if(m_pFlora)
		sq += ", flora=?";
	sq += QString(" WHERE id=%1;").arg(Id().GetID());

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

	// a_World fields
	qa.addBindValue(Name());

	// a_WorldLL fields
	qa.addBindValue(CaptId().GetID());
	qa.addBindValue(CaptName());
	qa.addBindValue(ClaimedByEmpire());
	qa.addBindValue(ClaimedAtLL().Longitude());
	qa.addBindValue(ClaimedAtLL().Latitude());
	qa.addBindValue(ClaimedByPersonId().GetID());
	qa.addBindValue(ClaimedByPersonName());
	qa.addBindValue(ClaimTime().UTC());
	{
		// Geological Features
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		LockGeologicalFeatures() >> out;
		UnlockGeologicalFeatures();
		qa.addBindValue(ba);
	}

	// a_WorldRingSection fields
	{
		// DNA of indigenous.
		QByteArray ba;
		qa.addBindValue(ba);	// Empty the old dna field.
		if(m_pSentient)
		{
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_5_7);
			out << *m_pSentient;
		}
		qa.addBindValue(ba);
	}

	{
		// Village(s)
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);

		quint8 count;	// In case we decide to have more than one village per planet.
		const AuVillage* village = LockVillage();
		if(village)
		{
			count = 1;
			out << count;
			out << *village;
		}
		else
		{
			count = 0;
			out << count;
		}
		UnlockVillage();

		qa.addBindValue(ba);
	}

	// Flora
	if(m_pFlora)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz] >> out;
		out << m_tmFloraChange.UTC();
		QByteArray bac = qCompress(ba);                 // Compress the flora data and store it.
		qa.addBindValue(bac);
	}
	
	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaWorldRingSection::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaWorldRingSection::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully
// loaded.
DaWorldRingSection* DaWorldRingSection::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, 
		LOADSUBATOMS loadWhat, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
			"timeModified,timeTock,timeDeath, "
			"name,orbitRadius,orbitZone, "
			"captId,captName,claimedBy,claimLon,claimLat,claimId,claimName,claimTime,geoFeatures, "
			"atmDensity,atmType,dna,indSpec,fauna,flora,sectionIndex,terrain,water,village "
			"FROM a_WorldRingSection WHERE ");
	if(!idToLoad.IsNull())
		sq += QString("id=%1;").arg(idToLoad.GetID());
	else
		sq += QString("idp=%1;").arg(parentId.GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaWorldRingSection *pnu = new DaWorldRingSection;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;	// query value index
			AuId id = AuId(qa.value(qv++).toUInt());
			pnu->SetId(id);
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

			// a_World fields
			pnu->SetName(qa.value(qv++).toString());
			pnu->SetOrbitRadiusAu(qa.value(qv++).toDouble());
			pnu->m_OrbitZone = (ORBITZONE)qa.value(qv++).toInt();

			// a_WorldLL fields
			AuId captId = AuId(qa.value(qv++).toUInt());
			QString captName = qa.value(qv++).toString();
			pnu->SetCapt(captId, captName);
			quint32 empireId = qa.value(qv++).toUInt();
			float clon = qa.value(qv++).toFloat();
			float clat = qa.value(qv++).toFloat();
			AuId claimId = AuId(qa.value(qv++).toUInt());
			QString claimName = qa.value(qv++).toString();
			QDateTime tmClaimed = qa.value(qv++).toDateTime();
			tmClaimed.setTimeSpec(Qt::UTC);
			if(tmClaimed > now)
				tmClaimed = now;
			pnu->SetClaim(empireId, AuGlobeLLF(clon, clat), claimId, claimName, tmClaimed);
			int qvi=qv++;
			bool changedGeology = false;
			if(!qa.value(qvi).isNull())
			{
				// Load geological features.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				AuGeologicalFeatures& geof = pnu->LockGeologicalFeaturesForWrite();
				geof << in;

				// Make sure no henges remain open after a reload.
				for(AuGeologicalFeatures::iterator gi = geof.begin(); gi != geof.end(); gi++)
				{
					AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
					if(!henge)
						continue;

					// Deactivate my henge portal.
					henge->SetActive(false);

					// Make sure the henge is not in a forbidden zone.
					if(pnu->IsForbiddenZone(henge->LL(), henge->CentralSlabAltitudeMSL(), henge->TerrainRadius()))
					{
						henge->RandomInit(*pnu, g_Au.Dice(), geof);
						changedGeology = true;
					}
				}

				pnu->UnlockGeologicalFeatures();
			}

			// a_WorldRingSection fields
			pnu->m_byAtmDensity = qa.value(qv++).toInt();
			pnu->m_AtmType = (ATMTYPE)qa.value(qv++).toInt();

			{
				// DNA of indigenous.
				QByteArray badna = qa.value(qv++).toByteArray();
				QByteArray badna4 = qa.value(qv++).toByteArray();
				if(!badna.isEmpty())
				{
					DhDNA dna(badna);
					pnu->m_pSentient = new AuSpecies(dna.Species());
				}
				else if(!badna4.isEmpty())
				{
					QDataStream in(badna4);
					in.setVersion(QDataStream::Qt_5_7);
					pnu->m_pSentient = new AuSpecies;
					in >> *pnu->m_pSentient;
					if(pnu->m_pSentient->IsNull())
					{
						delete pnu->m_pSentient;
						pnu->m_pSentient = 0;
					}
				}
				if(pnu->m_pSentient && pnu->m_pSentient->IsNull())
				{
					delete pnu->m_pSentient;
					pnu->m_pSentient = 0;
				}
			}

			pnu->m_pFauna = 0;
			qvi=qv++;
			if(!qa.value(qvi).isNull())
			{
				// Fauna
				pnu->m_pFauna = new DePlanetFauna[pnu->ResourceZones()];
				const int uisize = pnu->m_pFauna->DataSize();
				QByteArray bau = qa.value(qvi).toByteArray();
				QByteArray ba;
				if(!bau.isEmpty())
					ba = AuUncompress(bau, __FILE__, __LINE__,id.StringId());
				if(uisize*pnu->ResourceZones() <= ba.count())
				{
					for(int rz=0; rz<pnu->ResourceZones(); rz++)
						pnu->m_pFauna[rz].SetData(ba.begin() + rz*uisize);
				}
				else
				{
					qWarning() << "warning: DaWorldRingSection::LoadSubatoms fauna data size mismatch." << bau.count()
							<< "uncompressed to" << ba.count() << "instead of" << uisize*pnu->ResourceZones()
							<< "creating new data";
					pnu->InitAnimalEncounterTable(g_Au.Dice());
				}
			}
			pnu->m_pFlora = 0;
			qvi=qv++;
			if(!qa.value(qvi).isNull())
			{
				// Flora
				QByteArray bac = qa.value(qvi).toByteArray();	// Load compressed byte array from blob.
				QByteArray ba;
				if(!bac.isEmpty())
					ba = AuUncompress(bac, __FILE__, __LINE__,id.StringId());
				if(!ba.isEmpty())
				{
					QDataStream in(ba);
					in.setVersion(QDataStream::Qt_4_1);
					pnu->m_pFlora = new DePlanetFlora[pnu->ResourceZones()];
					for(int rz=0; rz<pnu->ResourceZones(); rz++)
						pnu->m_pFlora[rz] << in;

					QDateTime tm;
					in >> tm;
					tm.setTimeSpec(Qt::UTC);
					pnu->m_tmFloraChange = tm;
				}
				else
					qWarning("ERROR - DaWorldRingSection::LoadSubatoms - %d %s Flora data error. %d bytes loaded",
						pnu->Id().GetID(), (const char*)pnu->Name().toLatin1(), ba.count());
			}
			pnu->m_SectionIndex = qa.value(qv++).toUInt();
			qvi=qv++;
			if(!qa.value(qvi).isNull())
			{
				// Terrain array.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				in >> pnu->m_TerrainFrequency;
				in >> pnu->m_TerrainPersFactor;
				in >> pnu->m_TerrainPersistence;
				in >> pnu->m_TerrainXOff;
				in >> pnu->m_TerrainYOff;
				in >> pnu->m_TerrainWaterFactor;
			}
			pnu->m_byWater = qa.value(qv++).toInt();

			qvi=qv++;
			if(!qa.value(qvi).isNull())
			{
				// Village(s)
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				quint8 count;
				in >> count;
				if(count)
				{
					AuVillage village;
					in >> village;
					pnu->SetVillage(village);
				}
			}

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			// We never get here if we're the SceneMaster.
			if(loadWhat & LOAD_GamePieces)
			{
				pnu->SetEnclosureNotReady();

				DaAsteroid::LoadSubatoms(ether, db, pnu->Id());
				DaBd::LoadSubatoms(ether, db, pnu->Id());
				DaSc::LoadSubatoms(ether, db, pnu->Id());
				DaVehicle::LoadSubatoms(ether, db, pnu->Id());
				DaPerson::LoadSubatoms(ether, db, pnu->Id());
				DaAnimal::LoadSubatoms(ether, db, pnu->Id());
				DaItem::LoadSubatoms(ether, db, pnu->Id());
				DaDesigner::LoadSubatoms(ether, db, pnu->Id());
				DaArena::LoadSubatoms(ether, db, pnu->Id());
				DaNpc::LoadSubatoms(ether, db, pnu->Id());

				if(changedGeology)
				{
					pnu->GeologyChanged();
					pnu->SetModified();
				}

				pnu->m_BdBits = 0x80 | 0x20 | 0x08;
				pnu->JurisdictionUpdate();
				pnu->UpdateRoadMap();
				pnu->UpdateIrrigationCache();
				pnu->CitizenSupplierUpdate();
				pnu->TroopSupplierUpdate();
				pnu->SendBdFacilities();	// Notify master server of bd cities and their facilities.
				pnu->InvalidateEnclosureCache();
			}
			//pnu->SetModified();	//DHH force update of nu blob fields
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

double DaWorldRingSection::UndevelopedVirginTerrainAltitudeMSL(const AuGlobeLL& ll) const
{
	// Apply some special interconnection terrain logic within 1 degree of the ends.
	if(fabs(ll.Longitude()) > 170.*DEG2RAD)
	{
		// How about a row of large troughs along the seam, with a mountainous tower with hollow core in the middle of each.
		// Start with the trough depth calculation.
		double troughLat;
		if(ll.Latitude() > PI/4.)
			troughLat = PI/4. + PI/8.;
		else if(ll.Latitude() > 0)
			troughLat = PI/8.;
		else if(ll.Latitude() < -PI/4.)
			troughLat = -(PI/4. + PI/8.);
		else
			troughLat = -PI/8.;
		double troughRad = TerrainYSize() / 8. * .8;			// Radius of the trough. Half the width of the trough.
		double troughLon = (ll.Longitude() > 0) ? PI : -PI;
		AuPoint troughCen = LocOfLL(AuGlobeLL(troughLon, troughLat));
		AuPoint loc = LocOfLL(ll);
		double troughCenDist = fabs(loc.y - troughCen.y);		// ll location distance from center line of trough.
		if(troughCenDist >= troughRad)
		{
			// The band/margin between the troughs is 1m above water.
			return FeetPerMeter();
		}

		// Make a perfectly curved bottom to our trough, deepest at the center.
		double troughCenRatio = troughCenDist / troughRad;
		double troughMSL = sin(acos(troughCenRatio)) * -AltitudeIncrement() * 3.;

		// Now for the mountainous towering cone.
		double distance = loc.Distance(troughCen);
		if(distance < troughRad)
		{
			// The terrain location is inside the radius of a circle.
			double fracrad = distance/troughRad;	// Fraction of the distance from the center to the max radius of the circle.
			if(fracrad <= 2./32.)
			{
				// Core of central column reaches deep into the sea.
				return -AltitudeIncrement() * 3.75;
			}
			else if(fracrad <= 3./32.)
			{
				// Ledge around core slopes from 1m down to sea level (0 MSL).
				double frac = (fracrad - 2./32.) * 32.;
				return FeetPerMeter() * frac;
			}
			else if(fracrad <= 1./8.)
			{
				// Top of central column reaches just above highest clouds, which are at .4 * AtmosphereHeightMSL.
				return AtmosphereHeightMSL() * .75;
			}
			else if(fracrad <= 3./8.)
			{
				// Linearly interpolate a sloping base on the central column, from the bottom of the basin around the cone up to
				// 100m MSL.
				double frac = 1. - (fracrad - 1./8.) * 4.;
				double maxAlt = AtmosphereHeightMSL()* .75;
				double basinDepth = -AltitudeIncrement() * 3.;
				return (maxAlt - basinDepth) * frac + basinDepth;
			}
			else if(fracrad <= 6./8.)
			{
				// Basin around central cone.
				return -AltitudeIncrement() * 3.;
			}
		}

		return troughMSL;
	}

	// Determine the altitude of the terrain due to geology.
	double altMSL;
	double maxMSL = 0;
	if(SectionIndex() == 0)
	{
		// Section zero is the control center of the ringworld. It provides a level space above sea level for that purpose at the
		// center of the world, LL 0 0.
		double aspectYX = TerrainYSize() / TerrainXSize();
		if(fabs(ll.Longitude()) < aspectYX*2.*DEG2RAD && fabs(ll.Latitude()) < DEG2RAD)
		{
			// ll is inside the control center zone.
			// The center of the ring section determines the base altitude of the entire control center; it must be above water.
			AuGlobeLL ll0(0,0);
			altMSL = GeologicalTerrainAltitudeMSL(ll0);	// Altitude at center of zone.
			if(altMSL < 1)
				altMSL = 1;
			return altMSL;
		}
		else if(fabs(ll.Longitude()) < aspectYX*2.5*DEG2RAD && fabs(ll.Latitude()) < DEG2RAD*1.5)
		{
			// ll is near the control center zone. Here the terrain is pulled down to the level of the zone but not up.
			AuGlobeLL ll0(0,0);
			maxMSL = GeologicalTerrainAltitudeMSL(ll0);
			if(maxMSL < 1)
				maxMSL = 1;
			altMSL = GeologicalTerrainAltitudeMSL(ll);
		}
		else
			altMSL = GeologicalTerrainAltitudeMSL(ll);
	}
	else
		altMSL = GeologicalTerrainAltitudeMSL(ll);

	// Adjust altitude for geological features.
	altMSL = LockGeologicalFeatures().AdjustAltMSLUndeveloped(*this, ll, altMSL);
	UnlockGeologicalFeatures();
	if(maxMSL > 0. && altMSL > maxMSL)
		altMSL = maxMSL;

	// Ensure the depth doesn't get too close to the back hull wall of the ringworld.
	double maxDepth = -AltitudeIncrement() * 3.;
	if(altMSL < maxDepth)
		return maxDepth;
	return altMSL;
}

double DaWorldRingSection::VirginTerrainAltitudeMSL(const AuGlobeLL& ll, WORLDTEX_TYPE& pavementType, AuTime& newest) const
{
	// Apply some special interconnection terrain logic within 1 degree of the ends.
	if(fabs(ll.Longitude()) > 170.*DEG2RAD)
	{
		// How about a row of large troughs along the seam, with a mountainous tower with hollow core in the middle of each.
		// Start with the trough depth calculation.
		double troughLat;
		if(ll.Latitude() > PI/4.)
			troughLat = PI/4. + PI/8.;
		else if(ll.Latitude() > 0)
			troughLat = PI/8.;
		else if(ll.Latitude() < -PI/4.)
			troughLat = -(PI/4. + PI/8.);
		else
			troughLat = -PI/8.;
		double troughRad = TerrainYSize() / 8. * .8;			// Radius of the trough. Half the width of the trough.
		double troughLon = (ll.Longitude() > 0) ? PI : -PI;
		AuPoint troughCen = LocOfLL(AuGlobeLL(troughLon, troughLat));
		AuPoint loc = LocOfLL(ll);
		double troughCenDist = fabs(loc.y - troughCen.y);		// ll location distance from center line of trough.
		if(troughCenDist >= troughRad)
		{
			// The band/margin between the troughs is 1m above water.
			pavementType = WORLDTEX_ConcreteHeavy;
			return FeetPerMeter();
		}

		// Make a perfectly curved bottom to our trough, deepest at the center.
		double troughCenRatio = troughCenDist / troughRad;
		double troughMSL = sin(acos(troughCenRatio)) * -AltitudeIncrement() * 3.;

		// Now for the mountainous towering cone.
		double distance = loc.Distance(troughCen);
		if(distance < troughRad)
		{
			// The terrain location is inside the radius of a circle.
			double fracrad = distance/troughRad;	// Fraction of the distance from the center to the max radius of the circle.
			if(fracrad <= 2./32.)
			{
				// Core of central column reaches deep into the sea.
				pavementType = WORLDTEX_Tarmac;
				return -AltitudeIncrement() * 3.75;
			}
			else if(fracrad <= 3./32.)
			{
				// Ledge around core slopes from 1m down to sea level (0 MSL).
				double frac = (fracrad - 2./32.) * 32.;
				pavementType = WORLDTEX_Tarmac;
				return FeetPerMeter() * frac;
			}
			else if(fracrad <= 1./8.)
			{
				// Top of central column reaches just above highest clouds, which are at .4 * AtmosphereHeightMSL.
				pavementType = WORLDTEX_Tarmac;
				return AtmosphereHeightMSL() * .75;
			}
			else if(fracrad <= 3./8.)
			{
				// Linearly interpolate a sloping base on the central column, from the bottom of the basin around the cone up to
				// 100m MSL.
				pavementType = WORLDTEX_Tarmac;
				double frac = 1. - (fracrad - 1./8.) * 4.;
				double maxAlt = AtmosphereHeightMSL()* .75;
				double basinDepth = -AltitudeIncrement() * 3.;
				return (maxAlt - basinDepth) * frac + basinDepth;
			}
			else if(fracrad <= 6./8.)
			{
				// Basin around central cone.
				pavementType = WORLDTEX_Tarmac;
				return -AltitudeIncrement() * 3.;
			}
		}

		// Paved areas at ends of ring do not show rocks or plants. This hides an issue that prevents rendering these
		// object on adjacent ringworld sections. Now they're just not there; problem solved.
		pavementType = WORLDTEX_ConcreteHeavy;
		return troughMSL;
	}

	// Native terrain is textured according to terrain altitude.
	pavementType = WORLDTEX_Count;

	// Determine the altitude of the terrain due to geology.
	double altMSL;
	double maxMSL = 0;
	if(SectionIndex() == 0)
	{
		// Section zero is the control center of the ringworld. It provides a level space above sea level for that purpose at the
		// center of the world, LL 0 0.
		double aspectYX = TerrainYSize() / TerrainXSize();
		if(fabs(ll.Longitude()) < aspectYX*2.*DEG2RAD && fabs(ll.Latitude()) < DEG2RAD)
		{
			// ll is inside the control center zone.
			// The center of the ring section determines the base altitude of the entire control center; it must be above water.
			AuGlobeLL ll0(0,0);
			altMSL = GeologicalTerrainAltitudeMSL(ll0);	// Altitude at center of zone.
			if(altMSL < 1)
				altMSL = 1;
			pavementType = WORLDTEX_Tarmac;
			return altMSL;
		}
		else if(fabs(ll.Longitude()) < aspectYX*2.5*DEG2RAD && fabs(ll.Latitude()) < DEG2RAD*1.5)
		{
			// ll is near the control center zone. Here the terrain is pulled down to the level of the zone but not up.
			AuGlobeLL ll0(0,0);
			maxMSL = GeologicalTerrainAltitudeMSL(ll0);
			if(maxMSL < 1)
				maxMSL = 1;
			altMSL = GeologicalTerrainAltitudeMSL(ll);
		}
		else
			altMSL = GeologicalTerrainAltitudeMSL(ll);
	}
	else
		altMSL = GeologicalTerrainAltitudeMSL(ll);

	// Adjust altitude for geological features.
	altMSL = LockGeologicalFeatures().AdjustAltMSL(*this, ll, altMSL, pavementType, newest);
	UnlockGeologicalFeatures();
	if(maxMSL > 0. && altMSL > maxMSL)
		altMSL = maxMSL;

	// Ensure the depth doesn't get too close to the back hull wall of the ringworld.
	double maxDepth = -AltitudeIncrement() * 3.;
	if(altMSL < maxDepth)
		return maxDepth;
	return altMSL;
}

double DaWorldRingSection::VirginTerrainAltitudeMSL(const AuGlobeLL& ll, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		bool& hideTrees, WORLDTEX_TYPE& pavementType, AuTime& newest) const
{
	// Apply some special interconnection terrain logic at the ends.
	// The result needs to connect adjacent ring sections with a consistent terrain profile.
	if(fabs(ll.Longitude()) > 170.*DEG2RAD)
	{
		// How about a row of large troughs along the seam, with a mountainous tower with hollow core in the middle of each.
		// Start with the trough depth calculation.
		double troughLat;
		if(ll.Latitude() > PI/4.)
			troughLat = PI/4. + PI/8.;
		else if(ll.Latitude() > 0)
			troughLat = PI/8.;
		else if(ll.Latitude() < -PI/4.)
			troughLat = -(PI/4. + PI/8.);
		else
			troughLat = -PI/8.;
		double troughRad = TerrainYSize() / 8. * .8;			// Radius of the trough. Half the width of the trough.
		double troughLon = (ll.Longitude() > 0) ? PI : -PI;
		AuPoint troughCen = LocOfLL(AuGlobeLL(troughLon, troughLat));
		AuPoint loc = LocOfLL(ll);
		double troughCenDist = fabs(loc.y - troughCen.y);		// ll location distance from center line of trough.
		if(troughCenDist >= troughRad)
		{
			// The band/margin between the troughs is 1m above water and textured like concrete.
			// Paved areas at ends of ring do not show rocks or plants. This hides an issue that prevents rendering these
			// object on adjacent ringworld sections. Now they're just not there; problem solved.
			hideRocks = true;
			hideGrass = 1;
			hideShrubs = true;
			hideTrees = true;
			pavementType = WORLDTEX_ConcreteHeavy;
			return FeetPerMeter();
		}

		// Make a perfectly curved bottom to our trough, deepest at the center.
		double troughCenRatio = troughCenDist / troughRad;
		double troughMSL = sin(acos(troughCenRatio)) * -AltitudeIncrement() * 3.;

		// Now for the mountainous towering cone.
		double distance = loc.Distance(troughCen);
		if(distance < troughRad)
		{
			// The terrain location is inside the radius of a circle.
			double fracrad = distance/troughRad;	// Fraction of the distance from the center to the max radius of the circle.
			if(fracrad <= 2./32.)
			{
				// Core of central column reaches deep into the sea.
				// Paved areas at ends of ring do not show rocks or plants. This hides an issue that prevents rendering these
				// object on adjacent ringworld sections. Now they're just not there; problem solved.
				hideRocks = true;
				hideGrass = 1;
				hideShrubs = true;
				hideTrees = true;
				pavementType = WORLDTEX_Tarmac;
				return -AltitudeIncrement() * 3.75;
			}
			else if(fracrad <= 3./32.)
			{
				// Ledge around core slopes from 1m down to sea level (0 MSL).
				double frac = (fracrad - 2./32.) * 32.;
				hideGrass = 1;
				hideShrubs = true;
				hideTrees = true;
				pavementType = WORLDTEX_Tarmac;
				return FeetPerMeter() * frac;
			}
			else if(fracrad <= 1./8.)
			{
				// Top of central column reaches just above highest clouds, which are at .4 * AtmosphereHeightMSL.
				// Paved areas at ends of ring do not show rocks or plants. This hides an issue that prevents rendering these
				// object on adjacent ringworld sections. Now they're just not there; problem solved.
				hideRocks = true;
				hideGrass = 1;
				hideShrubs = true;
				hideTrees = true;
				pavementType = WORLDTEX_Tarmac;
				return AtmosphereHeightMSL() * .75;
			}
			else if(fracrad <= 3./8.)
			{
				// Linearly interpolate a sloping base on the central column, from the bottom of the basin around the cone up to
				// 100m MSL.
				// Paved areas at ends of ring do not show rocks or plants. This hides an issue that prevents rendering these
				// object on adjacent ringworld sections. Now they're just not there; problem solved.
				hideRocks = true;
				hideGrass = 1;
				hideShrubs = true;
				hideTrees = true;
				pavementType = WORLDTEX_Tarmac;
				double frac = 1. - (fracrad - 1./8.) * 4.;
				double maxAlt = AtmosphereHeightMSL()* .75;
				double basinDepth = -AltitudeIncrement() * 3.;
				return (maxAlt - basinDepth) * frac + basinDepth;
			}
			else if(fracrad <= 6./8.)
			{
				// Basin around central cone.
				// Paved areas at ends of ring do not show rocks or plants. This hides an issue that prevents rendering these
				// object on adjacent ringworld sections. Now they're just not there; problem solved.
				hideRocks = true;
				hideGrass = 1;
				hideShrubs = true;
				hideTrees = true;
				pavementType = WORLDTEX_Tarmac;
				return -AltitudeIncrement() * 3.;
			}
		}

		// Paved areas at ends of ring do not show rocks or plants. This hides an issue that prevents rendering these
		// object on adjacent ringworld sections. Now they're just not there; problem solved.
		hideRocks = true;
		hideGrass = 1;
		hideShrubs = true;
		hideTrees = true;
		pavementType = WORLDTEX_ConcreteHeavy;
		return troughMSL;
	}

	// Native terrain is textured according to terrain altitude.
	hideRocks = false;
	hideGrass = 0;
	hideShrubs = false;
	hideTrees = false;
	pavementType = WORLDTEX_Count;

	// Determine the altitude of the terrain due to geology.
	double altMSL;
	double maxMSL = 0;
	if(SectionIndex() == 0)
	{
		// Section zero is the control center of the ringworld. It provides a level space above sea level for that purpose at the
		// center of the world, LL 0 0.
		double aspectYX = TerrainYSize() / TerrainXSize();
		if(fabs(ll.Longitude()) < aspectYX*2.*DEG2RAD && fabs(ll.Latitude()) < DEG2RAD)
		{
			// ll is inside the control center zone.
			// The center of the ring section determines the base altitude of the entire control center; it must be above water.
			AuGlobeLL ll0(0,0);
			altMSL = GeologicalTerrainAltitudeMSL(ll0);	// Altitude at center of zone.
			if(altMSL < 1)
				altMSL = 1;
			hideRocks = true;
			hideGrass = 1;
			hideShrubs = true;
			hideTrees = true;
			pavementType = WORLDTEX_Tarmac;
			return altMSL;
		}
		else if(fabs(ll.Longitude()) < aspectYX*2.5*DEG2RAD && fabs(ll.Latitude()) < DEG2RAD*1.5)
		{
			// ll is near the control center zone. Here the terrain is pulled down to the level of the zone but not up.
			AuGlobeLL ll0(0,0);
			maxMSL = GeologicalTerrainAltitudeMSL(ll0);
			if(maxMSL < 1)
				maxMSL = 1;
			altMSL = GeologicalTerrainAltitudeMSL(ll);
		}
		else
			altMSL = GeologicalTerrainAltitudeMSL(ll);
	}
	else
		altMSL = GeologicalTerrainAltitudeMSL(ll);

	// Adjust altitude for geological features.
	altMSL = LockGeologicalFeatures().AdjustAltMSL(*this, ll, altMSL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType,
			newest);
	UnlockGeologicalFeatures();
	if(maxMSL > 0. && altMSL > maxMSL)
		altMSL = maxMSL;

	// Ensure the depth doesn't get too close to the back hull wall of the ringworld.
	double maxDepth = -AltitudeIncrement() * 3.;
	if(altMSL < maxDepth)
		return maxDepth;
	return altMSL;
}

double DaWorldRingSection::GeologicalTerrainAltitudeMSL(const AuGlobeLL& ll) const
{
	// 7 octaves of Perlin noise should do the trick.
	double aspectYX = TerrainYSize() / TerrainXSize();
	m_RingSectionLock.lockForRead();
	const double fx = m_TerrainXOff + ll.Longitude()/PI * m_TerrainFrequency;	// X noise offset
	const double fy = m_TerrainYOff + ll.Latitude()/PI * m_TerrainFrequency * aspectYX; 	// Y noise offset
	double altFactor = AuNoise::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/m_TerrainPersFactor;	// Scale near -1 to 1.
	altFactor -= m_TerrainWaterFactor;							// Shift altitude up with more water level.

	if(m_TerrainWaterFactor != 1 && (altFactor > 0 || m_TerrainWaterFactor == -1))
	{
		// Above sea level.
		altFactor /= (1 - m_TerrainWaterFactor);

		// Pull the land down tighter to sea level, less and less as we get closer to the peaks.
		if(altFactor > 0 && altFactor < 1)
			altFactor *= altFactor;
	}
	else
	{
		// Below sea level, altitude is negative.
		altFactor = -altFactor / (-1 - m_TerrainWaterFactor);

		// Pull the land up tighter to sea level, less and less as we get closer to the sea trenches.
		//if(altFactor < -.5f && altFactor > -1)
			//altFactor = -(altFactor * altFactor);				// Preserve the sign.

		// Pull the land up tighter to sea level near the coast lines for nice beaches.
		double altExp = 2 + altFactor;		// Range is 2..1, decreasing with depth for less effect.
		if(altExp < 1)
			altExp = 1;
		altFactor = -pow(-altFactor, altExp);
	}
	m_RingSectionLock.unlock();

	// Scale the altitude factor into the range we want for mountain peaks and sea trenches. 1 is scaled up to the tree
	// line. Values in excess of 1 exceed the tree line for snow capped peaks. Sea trenches act similarly; shall we have lava
	// there?
	return altFactor * TreeLineMSL();
}

// Returns the altitude of the surface at ll relative to MSL.
// Checks for permanent terrain-like features such as the control center dais on a ringworld.
double DaWorldRingSection::SurfaceAltitudeMSL(const AuGlobeLL& ll) const
{
	double altMSL = TerrainAltitudeMSL(ll);

	// Check for the raised dais at the center of the control center on section 0.
	if(SectionIndex() != 0)
		return altMSL;

	// Section zero is the control center of the ringworld. It provides a level space above sea level for that purpose at the
	// center of the world, LL 0 0. The altitude of the level space has been returned by VirginTerrainAltitudeMSL.
	// At the center of that space is a raised dais.
	double aspectYX = TerrainYSize() / TerrainXSize();
	if(fabs(ll.Longitude()) >= aspectYX*2.*DEG2RAD || fabs(ll.Latitude()) >= DEG2RAD)
		return altMSL;

	// ll is inside the control center zone.
	// The center of the ring section determines the base altitude of the entire control center; it must be above water.
	AuGlobeLL ll0(0,0);
	AuPoint loc = LocOfLL(ll, altMSL);
	AuPoint loc0 = LocOfLL(ll0, altMSL);
	AuPoint veast(1,0,0);
	AuPoint vec(loc - loc0);	// Vector to ll from center of control center.
	float edist = fabs(vec.Dot(veast));
	if(edist >= 14*FeetPerMeter())
		return altMSL;

	// East/west distance is within range of the dais.
	AuPoint vnorth(0,1,0);
	float ndist = fabs(vec.Dot(vnorth));
	if(ndist >= 14*FeetPerMeter())
		return altMSL;

	// North/south distance is within range of the dais.
	if(ndist <= 10*FeetPerMeter() && edist <= 10*FeetPerMeter())
	{
		// On the flat portion of the dais, which is 2m above the terrain.
		altMSL += 2*FeetPerMeter();

		// Check on top of an obelisk.
		const double daisRad = 10.*FeetPerMeter();	// Control center dais radius (loosely, this is a square).
		const double baseRad = 1.*FeetPerMeter();
		if(ndist > daisRad - baseRad*2 && ndist < daisRad &&
				edist > daisRad - baseRad*2 && edist < daisRad)
		{
			const double shoulderHt = 10*FeetPerMeter();
			altMSL += shoulderHt;
		}
	}
	else if(ndist > edist)
	{
		// On north or south slope to the dais.
		// Linear interpolate the height.
		float ratio = (ndist - 10*FeetPerMeter()) / (4*FeetPerMeter());
		altMSL = AuNoiseF::Linear_Interpolate(altMSL + 2*FeetPerMeter(), altMSL, ratio);
	}
	else
	{
		// On east or west slope to the dais.
		// Linear interpolate the height.
		float ratio = (edist - 10*FeetPerMeter()) / (4*FeetPerMeter());
		altMSL = AuNoiseF::Linear_Interpolate(altMSL + 2*FeetPerMeter(), altMSL, ratio);
	}

	return altMSL;
}

// Spawns spacecraft in orbit.
// Returns:
//   0 = ok
//   1 = blocked by unanalyzed ship
//   2 = Cannot spawn another station in orbit or replace an existing station.
//   3 = Cannot spawn another nu ship in orbit or replace an existing nu ship.
//   4 = Orbit spawn location could not be found.
//   5 = No station at which to spawn.
int DaWorldRingSection::SpawnSc(const AuScReport& shipDesign, quint32 bdEmpireId, quint8 quality,
		const AuId& ownerId, const QString& ownerName, quint32 ownerEmpire, const AuId& cityOrBdId)
{
	AuEther* ether = Ether();
	if(!ether)
		return 4;
	//qDebug("DaWorldRingSection::SpawnSpacecraft %d %s design %d %s", Id().GetID(), (const char*)Name().toAscii(),
			//shipDesign.DesignId(), (const char*)shipDesign.DesignName().toAscii());
	// Postpone this if there are any spacecraft in orbit that are still waiting for their design data cache to be loaded.
	// Count nu ships in orbit while we're doing this loop.
	DaVessel* replaceableShip = 0;		// Replaceable empire ship.
	DaVessel* replaceableStation = 0;	// Replaceable empire station.
	const DaAtom* anyStation = 0;			// New station will spawn opposite this station on planet, ignored on ringworld.
	int myNewShips = 0;						// Number of nu empire ships in orbit.
	int maxStations = 1;					// Maximum empire stations that can be in orbit. 1 added for each AI commanded station.
	DaAtomList myStations;					// Empire stations in orbit.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaVessel* ship = dynamic_cast<DaVessel*>(*it);
		if(!ship)
			continue;
		if(ship->IsPrivatelyOwned() && ship->HasHumanOfficer())
			continue;				// Ignore privately owned vessels that have a human officer.
		if(!ship->IsDesignDataCacheValid())
		{
			//qDebug("DaWorldRingSection::SpawnSpacecraft %d %s cannot spawn design %d in orbit. "
			//"Ship %d %s design %d already in orbit has invalid design cache.",
			//Id().GetID(), (const char*)Name().toAscii(), shipDesign.DesignId(),
			//ship->Id().GetID(), (const char*)ship->Name().toAscii(), ship->DesignId());
			//qDebug("DaWorldRingSection::SpawnSpacecraft %d %s not yet", Id().GetID(), (const char*)Name().toAscii());
			return 1;	// Must not spawn spacecraft yet.
		}
		if(ship->IsInAtmosphere())
			continue;
		if(ship->IsSpaceStation())
			anyStation = ship;		// A station will spawn opposite any station if we don't find one in our own empire.
		if(ship->EmpireId() != bdEmpireId)
			continue;				// Ignore spacecraft that belong to other empires.
		if(ship->IsSpaceStation())
		{
			// Found a station of my empire.
			myStations.append(ship);		// Remember each station we find, for nu ship spawn.
			if(ship->HasFirstOfficer())
				maxStations++;				// Each station with an AI officer increases our maximum station count by one.

			// Newer design replaces older design of same Q
			if((shipDesign.Quality() > ship->QualityOfDesign() ||			// Design Q is higher or
						(shipDesign.Quality() == ship->QualityOfDesign() &&	// Design Q is the same and
						 shipDesign.DesignId() > ship->DesignId())) &&					// Newer design id and
					!ship->FindPersonOnLineRecursive())									// No officers and nobody online.
				replaceableStation = ship;	// The station can be replaced.
		}
		else if(ship->IsNew())
		{
			// Found a nu ship of my empire.
			if((shipDesign.Quality() > ship->QualityOfDesign() ||			// Design Q is higher or
						(shipDesign.Quality() == ship->QualityOfDesign() &&	// Design Q is the same and
						 shipDesign.DesignId() > ship->DesignId())) &&					// Newer design id and
					!ship->FindPersonRecursive())										// No officers and nobody aboard.
				replaceableShip = ship;		// The nu ship can be replaced.
			myNewShips++;
		}
	}

	// All spacecraft in orbit (if any) have up-to-date design data so we can tell what they are and how big they are.
	if(shipDesign.IsSpaceStation())
	{
		QString replaceName;
		DaAtomList transfer;
		if(ownerId.IsNull())
		{
			// We are building an empire station. Each empire can build a limited number per planet. The limit is one more than the
			// the number of empire stations in orbit that has an AI officer. So, one station will spawn. Once an officer is
			// assigned to that station, another station will spawn, etc.
			if(myStations.count() > maxStations || (myStations.count() == maxStations && !replaceableStation))
				return 2;	// Cannot spawn another station in orbit or replace an existing station.

			// Remove the replaceable station, if we are replacing one.
			if(replaceableStation && myStations.count() == maxStations)
			{
				// If the station to be replaced has been assigned a name, carry the name forward to the nu station.
				replaceName = replaceableStation->AssignedName();

				// Remove people and citizens from the station that will be replaced because we must destroy that station now,
				// otherwise it obstructs placement of the nu station.
				DaAtomList allAboard;
				replaceableStation->ListNormalSubatomsRecursive(allAboard);
				for(DaAtomList::const_iterator aai = allAboard.begin(); aai != allAboard.end(); aai++)
				{
					DaCitizen* citizenAboard = dynamic_cast<DaCitizen*>(*aai);
					if(citizenAboard)
					{
						citizenAboard->ClearShipAssignment();		// Clear assigned ship so citizen will join roster of nu ship.
						transfer.append(citizenAboard);
					}
					else
					{
						DaPerson* personAboard = dynamic_cast<DaPerson*>(*aai);
						if(personAboard)
						{
							// Clear person's spawn ship if it is the station we're replacing.
							if(replaceableStation->Is(personAboard->SpawnShip()))
								personAboard->SetSpawnShip(AuId(0), "", AuRosterEntry::Invalid, false);
							transfer.append(personAboard);
						}
					}
				}

				// The replaceable station can now be removed.
				myStations.removeAll(replaceableStation);
				if(anyStation == replaceableStation)
					anyStation = 0;				// No need to place opposite this defunct station.
				replaceableStation->SetDormant();	// Set dormant now so it is not an obstacle. Can't kill it yet due to crew.
			}

			// Set 'anyStation' set to any remaining station of my empire. It can remain set to another empire's station otherwise,
			// or nothing at all.
			if(!myStations.isEmpty())
			{
				// I have at least one empire station present.
				if(myStations.count() == 1)
					anyStation = myStations.first();
				else
					anyStation = *(myStations.begin() + g_Au.Dice().Roll(1,myStations.count(),-1));	// Select random station.
			}
		}

		DaSc* ship = new DaSc();
		ship->ConnectSignals();
		ship->Create();

		ship->SetDesign(shipDesign);

		if(!ownerId.IsNull() && ownerEmpire)
		{
			// Privately owned. Owner is responsible for their own fuel and repairs but no subsidy tax.
			ship->SetPrivateOwnership(true);
			ship->SetPrivateOwner(ownerId, ownerName);
			ship->SetEmpireId(ownerEmpire);
		}
		else
			ship->SetEmpireId(bdEmpireId);

		// Determine altitude of the spawn location.
		// Atmosphere to atmosphere distance between a planet and its moon is 5000mi at the closest, 4000mi between moons. 
		// The zone of control of a moon extends 2000mi beyond its atmosphere.
		// We put the station 1000mi above the atmosphere so it will be centered in the control zone of moons; this is also a good
		// distance away from planets.
		// The closest other station could pass within 2000mi if both orbited moons.
		const float orbitAlt = AtmosphereRadius() + ParkAltAboveAtm();

		// Calculate spawn location at altitude.
		AuPoint spawnAt;
		DaAtom* builder = FindSubatomId(cityOrBdId);
		if(builder)
			spawnAt = builder->Location();						// Start the search above the city or building.
		else
			spawnAt = LocOfLL(AuGlobeLL(0,0));					// No city? Start search above the center of the ringworld section.
		spawnAt = FindOrbitSpawnLoc(spawnAt, ship->Id(), ship->RoughRadius(), orbitAlt, true);

		// Place station at initial spawn location.
		AuMatrix4x4 t;
		t.Translate(spawnAt);
		switch(shipDesign.StationOrient())
		{
		default:
		case AuScDesign::SORI_Random:
			t *= AuPoint(0,0,1).GetAlignmentTransform(AuParticleSystem::GetRandomVector());	// Orient to a random vector.
			break;

		case AuScDesign::SORI_Upright:
			t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Orient so down is toward the planet.
			break;

		case AuScDesign::SORI_UpsideDown:
			t *= AuPoint(0,0,1).GetAlignmentTransform(GravityDir(spawnAt));		// Orient so up is toward the planet.
			break;

		case AuScDesign::SORI_NoseDown:
			t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Orient so down is toward the planet.
			t.RotateX(-PI/2);													// Pitch nose down.
			break;

		case AuScDesign::SORI_TailDown:
			t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Orient so down is toward the planet.
			t.RotateX(PI/2);													// Pitch nose up.
			break;

		case AuScDesign::SORI_PortDown:
			t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Orient so down is toward the planet.
			t.RotateY(-PI/2);													// Roll port side down.
			break;

		case AuScDesign::SORI_StarboardDown:
			t *= AuPoint(0,0,1).GetAlignmentTransform(-GravityDir(spawnAt));	// Orient so down is toward the planet.
			t.RotateY(PI/2);													// Roll starboard side down.
			break;
		}
		ship->SetPosAndOrient(AuAtomPos(Id(), t));
		ether->AddAtom(ship);

		// Station starts out with power plant, gravity drive and interior lights on.
		// Fill capacitor so it has enough power to stay in orbit until it fetches some hydrogen from the ground.
		ship->InitSystems(quality);
		ship->TurnPowerPlantOn();
		ship->RechargeCapacitor();
		ship->SetGravityDriveState(DaVessel::GD_On);
		ship->HeadlightToggle(true);
		ship->NavLightToggle(true);
		ship->RotatingBeaconToggle(true);
		ship->StrobeToggle(true);
		ship->UpdateEnvironment();

		DaBd* bd = dynamic_cast<DaBd*>(builder);
		if(bd)
		{
			// Fill up with fuel from the building.
			ship->FillUpFuel(bd);

			// Name the station after the city if it is not picking up the name of the station it is replacing.
			if(replaceName.isEmpty())
			{
				replaceName = bd->CityName();
				if(replaceName.isEmpty())
					replaceName = bd->AssignedName();
				if(replaceName.isEmpty())
					replaceName = tr("Station");
				else
					replaceName = tr("%1 Station").arg(replaceName);
			}
		}

		// Set the name of the nu station to match the one that was replaced.
		if(ownerId.IsNull() && !replaceName.isEmpty())
			ship->SetName(replaceName);

		// Stations do not start out moored.
		ship->Unmoor();
		ship->SetAtRest();

		// Develop a description of where this ship was made.
		// e.g. Imperium city Bay of Magellan on Hazeron Alpha V in Hazeron, Hazeron Sector
		const DaSolarSystem* system = FindParentSolarSystem();
		const DaSector* sector = FindParentSector();
		QString empire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(ship->EmpireId(), empire);
		g_Au.UnlockData();
		QString whereMade;
		if(bd && system && sector)
			whereMade = tr("%1 %2 on %3 in %4, %5")
				.arg(empire).arg(bd->Name()).arg(Name()).arg(system->Name()).arg(sector->Name());
		else if(system && sector)
			whereMade = tr("%1 city on %3 in %4, %5").arg(empire).arg(Name()).arg(system->Name()).arg(sector->Name());
		ship->SetWhereMade(whereMade);

		// Board people and citizens taken from the old station onto the nu station.
		// We must put them somewhere because their parent atom is no longer valid.
		for(DaAtomList::const_iterator tai = transfer.begin(); tai != transfer.end(); tai++)
		{
			// Add the atom to the nu ship.
			DaCreature* transferAtom = dynamic_cast<DaCreature*>(*tai);
			if(!transferAtom)
				continue;								// We only added citizens and people to the list. What happened?
			transferAtom->Stand();						// Ensure that they don't take their console with them.

			// Find an appropriate location for the atom.
			AuPoint boardLoc;
			if(!ship->ColonistVoidLocation(boardLoc) &&
					!ship->TroopOrCrewVoidLocation(boardLoc) &&
					!ship->TransporterVoidLocation(boardLoc))
			{
				// Could not find a place to put the atom aboard the nu station.
				transferAtom->UpdateEnvironment();
				DaPerson* transferPerson = dynamic_cast<DaPerson*>(transferAtom);
				if(transferPerson)
					transferPerson->Respawn(false);	// Respawn a person atom somewhere else. Other creatures will die with the ship.
			}
			else
			{
				// Position the atom aboard the nu station.
				AuMatrix4x4 tSpawn;
				tSpawn.Translate(boardLoc);
				transferAtom->ChangePosAndOrient(AuAtomPos(ship->Id(), tSpawn));
				transferAtom->InvalidateEnvironment();
				DaCitizen* transCitiz = dynamic_cast<DaCitizen*>(transferAtom);
				if(transCitiz)
				{
					if(transCitiz->IsNpcOfficer())
					{
						if(ship->OfficerBerthsAvailable())
							transCitiz->OccupyBerth(ship);	// Occupy the appropriate berth type aboard the ship.
					}
					else if(transCitiz->IsNpcCrew())
					{
						if(ship->CrewBerthsAvailable())
							transCitiz->OccupyBerth(ship);	// Occupy the appropriate berth type aboard the ship.
					}
					else if(transCitiz->IsNpcTroop())
					{
						if(ship->TroopBerthsAvailable())
							transCitiz->OccupyBerth(ship);	// Occupy the appropriate berth type aboard the ship.
					}
					else if(transCitiz->IsNpcPassenger())
					{
						if(ship->PassengerBerthsAvailable())
							transCitiz->OccupyBerth(ship);	// Occupy the appropriate berth type aboard the ship.
					}
				}
				transferAtom->SetModified();
			}
		}

		// Kill the station and all of its remaining subatoms recursively, now that the crew has been transferred.
		if(replaceableStation)
			replaceableStation->Kill();

		// The station spawned fine. It can stay where it is.
		g_Au.AtomAdded(ship->Id());
		ship->SetNormal();	// New atom is ready for normal thread processing.
		ship->TickTockRecursive();
		//qDebug("DaWorldRingSection::SpawnSpacecraft %d %s ok station", Id().GetID(), (const char*)Name().toAscii());

		// Send mail to the new owner.
		if(bd && !ownerId.IsNull() && ownerEmpire)
			bd->NotifyNewSpacecraftOwner(ownerId, ownerName, ownerEmpire, ship);
		return 0;
	}
	else if(!myStations.isEmpty())
	{
		// Can only spawn a limited number of nu ships at once.
		const int maxNewShips = 4;
		if(ownerId.IsNull())
		{
			if(myNewShips > maxNewShips || (myNewShips == maxNewShips && !replaceableShip))
				return 3;	// Cannot spawn another nu ship in orbit or replace an existing nu ship.
		}

		// Choose an empire station to spawn near.
		if(myStations.count() == 1)
			anyStation = myStations.first();
		else
			anyStation = *(myStations.begin() + g_Au.Dice().Roll(1,myStations.count(),-1));	// Select a station at random.

		double roughRad;
		{
			// Copied from DaSc::RoughRadius but we don't have a DaSc yet.
			AuPoint min,max;
			shipDesign.Extents(min, max);
			double rmin = min.Length2();
			double rmax = max.Length2();
			if(rmin > rmax)
				roughRad = sqrt(rmin);
			else
				roughRad = sqrt(rmax);
		}

		// Determine spawn location relative to the station.
		const double orbitAlt = AtmosphereRadius() + ParkAltAboveAtm();
		AuPoint spawnAt = FindOrbitSpawnLoc(anyStation->Location(), AuId(0), roughRad, orbitAlt, true);
		if(spawnAt.IsZero())
			return 4;

		// Remove the replaceable ship, if we replaced one.
		if(ownerId.IsNull() && replaceableShip && myNewShips == maxNewShips)
			replaceableShip->Kill();		// Kill the ship and all of its subatoms recursively. We know no people are aboard.

		// Create the spaceship at the spawn location.
		DaSc* ship = new DaSc();
		ship->ConnectSignals();
		ship->Create();

		ship->SetDesign(shipDesign);

		if(!ownerId.IsNull() && ownerEmpire)
		{
			// Privately owned. Owner is responsible for their own fuel and repairs but no subsidy tax.
			ship->SetPrivateOwnership(true);
			ship->SetPrivateOwner(ownerId, ownerName);
			ship->SetEmpireId(ownerEmpire);
		}
		else
			ship->SetEmpireId(bdEmpireId);

		// Calculate up vector from spawn location.
		AuPoint upvec = -GravityDir(spawnAt);

		// Place ship at initial spawn location.
		AuMatrix4x4 t;
		t.Translate(spawnAt);
		t *= AuPoint(0,0,1).GetAlignmentTransform(upvec);	// Orient so down is toward the planet.
		ship->SetPosAndOrient(AuAtomPos(Id(), t));
		ether->AddAtom(ship);

		ship->InitSystems(quality);

		DaAtom* builder = FindSubatomId(cityOrBdId);
		DaBd* bd = dynamic_cast<DaBd*>(builder);

		// Develop a description of where this ship was made.
		// e.g. Imperium city Bay of Magellan on Hazeron Alpha V in Hazeron, Hazeron Sector
		const DaSolarSystem* system = FindParentSolarSystem();
		const DaSector* sector = FindParentSector();
		QString empire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(ship->EmpireId(), empire);
		g_Au.UnlockData();
		QString whereMade;
		if(bd && system && sector)
			whereMade = tr("%1 %2 on %3 in %4, %5")
				.arg(empire).arg(bd->Name()).arg(Name()).arg(system->Name()).arg(sector->Name());
		else if(system && sector)
			whereMade = tr("%1 city on %3 in %4, %5").arg(empire).arg(Name()).arg(system->Name()).arg(sector->Name());
		ship->SetWhereMade(whereMade);

		// Fill up with fuel from the city.
		if(bd)
			ship->FillUpFuel(bd);

		// The spacecraft spawned fine. It can stay where it is.
		g_Au.AtomAdded(ship->Id());
		ship->SetNormal();	// New atom is ready for normal thread processing.
		ship->TickTock();

		// Send mail to the new owner.
		if(bd && !ownerId.IsNull() && ownerEmpire)
			bd->NotifyNewSpacecraftOwner(ownerId, ownerName, ownerEmpire, ship);
		return 0;
	}

	//qDebug("DaWorldRingSection::SpawnSpacecraft %d %s station needed", Id().GetID(), (const char*)Name().toAscii());
	return 5;
}

// Returns the GL texture id for a given terrain patch.
GLuint DaWorldRingSection::FloraTexture(int resourceZone) const
{
	if(m_pFlora)
		return m_pFlora[resourceZone].Texture();
	return 0;
}

// Calculate sea level temperature in kelvin at a given long/lat.
// This does not account for altitude. The effect of altitude must be applied to the
// result of this calculation. See AuWeather::TemperatureAt.
float DaWorldRingSection::SeaLevelTempAt(const AuGlobeLLF& longLat, float /*solarOrbitRadius*/, float /*sunLightAtten*/) const
{
	// Insure longitude and latitude are within valid ranges we're looking for.
	float fy = longLat.Latitude();
	// We can wrap longitude (fx) to be within the range 0 <= fx < PI*2
	float fx = longLat.Longitude();
	while(fx >= PIf*2)
		fx -= PIf*2;
	while(fx < 0)
		fx += PIf*2;

	// Estimate the equatorial temperature based on solar intensity and atmosphere
	// density. Intensity is 1 in the habitable zone.
	// Earth maximum is around 50 degrees celsius; Earth actually lessens slightly at
	// the equator to about 40 degrees C due to the increased cooling due to increased
	// rainfall. We'll just call the maximum 45 and split the difference.
	float equatorTemp = 273.f + 20.f + 40.f * MSLAtmosphereDensity();	// Nominal equator temp in kelvin.

	// Min pole temperature is moderated by atmosphere density.
	// We consider Earth to be about 70% atmosphere density.
	// This calc results in Earth pole temp of about 223 kelvin = -50C = -58F, which is
	// just about right. Lowest recorded Earth temperature was -90C.
	float poleTemp = equatorTemp * MSLAtmosphereDensity();

	// Adjust the temp for latitude.
	float tempRange = equatorTemp - poleTemp;
	float cfy = cosf(fy);
	float seaLevelTemp = poleTemp + cfy * tempRange;

	// Return the sea level temperature in Kelvin. The actual ground level might
	// be higher. This calculation doesn't account for altitude.
	return seaLevelTemp;
}

bool DaWorldRingSection::IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal,
		const DaAtom* toWho)
{
	if(DaWorldLL::IsPathBlocked(oloc, nloc, hitLoc, hitNormal, toWho))
		return true;

	// Check collision with obelisks at control center on section 0.
	if(SectionIndex() == 0)
	{
		// World does not block path if both locations are outside the atmosphere.
		double nalt = InAtmosphereCGAltitude(nloc);
		if(nalt == -10000 && InAtmosphereCGAltitude(oloc) == -10000)
			return false;

		const double daisHt = 2*FeetPerMeter();
		const double shoulderHt = 10*FeetPerMeter();
		if(nalt > daisHt && nalt < shoulderHt)
		{
			// nloc is in the proper altitude range.
			AuPoint loc0 = GetTerrainLocation(AuGlobeLL(0,0));
			AuPoint vec = nloc - loc0;
			AuPoint nvec = AuPoint(0,1);
			AuPoint evec = AuPoint(1,0);
			double ndist = fabs(vec.Dot(nvec));
			double edist = fabs(vec.Dot(evec));
			const double daisRad = 10.*FeetPerMeter();			// Control center dais radius (loosely, this is a square).
			const double baseRad = 1.*FeetPerMeter();
			if(ndist > daisRad - baseRad*2 && ndist < daisRad &&
					edist > daisRad - baseRad*2 && edist < daisRad)
			{
				hitLoc = nloc;
				return true;	// nloc is inside one of the obelisks.
			}
		}
	}
	return false;
}

// Locate a spawn location near 'startLoc' for a ship with id 'shipId' that has a rough radius of 'shipRadius'.
AuPoint DaWorldRingSection::FindOrbitSpawnLoc(const AuPoint& startLoc, const AuId& spawnShipId, double spawnShipRadius,
		double orbitAlt, bool avoidCreatures) const
{
	// Get LL of requested spawn location.
	AuGlobeLL ll;
	if(!LLOfLoc(ll, startLoc))
		ll = AuGlobeLL(0,0);									// Location was not above my terrain? Use LL 0 0.

	// Move around the ringworld section until we find a spot that is far enough from the station and other vessels.
	const double segs = TerrainXSize() / (100. * FeetPerMeter());	// Number of 100m arc segments at sea level.
	const double segAng = (PI*2) / segs;						// Angle per arc segment to move 100m.
	const double mindist = spawnShipRadius*2;					// min dist required from existing atoms when the ship spawns

	// Walk east, one segment angle at a time. East is the trailing direction around the ringworld.
	AuGlobeLL tryLL(ll);
	AuPoint spawnAt = LocOfLL(tryLL, orbitAlt);
	int tries = 100;
	while(ClosestShipInOrbit(spawnAt, spawnShipId, avoidCreatures) < mindist && tries)
	{
		tryLL.SetLongitude(tryLL.Longitude() + segAng);
		spawnAt = LocOfLL(tryLL, orbitAlt);
		tries--;
	}
	if(tries)
		return spawnAt;	// Found a spawn location.

	// Walk west, one segment angle at a time. West is the spinward direction around the ringworld.
	tryLL = AuGlobeLL(ll.Longitude() - segAng, ll.Latitude());
	spawnAt = LocOfLL(tryLL, orbitAlt);
	tries = 100;
	while(ClosestShipInOrbit(spawnAt, spawnShipId, avoidCreatures) < mindist && tries)
	{
		tryLL.SetLongitude(tryLL.Longitude() - segAng);
		spawnAt = LocOfLL(tryLL, orbitAlt);
		tries--;
	}
	if(tries)
		return spawnAt;	// Found a spawn location.

	// This is highly unlikely but we must not fail.
	// Recursively call this function, moving up 1000' per call, until we find a spot.
	qWarning("DaWorldRingSection::FindOrbitSpawnLoc %d %s could not find a spawn location in 300 tries at %g' alt, moving up.",
			Id().GetID(), (const char*)Name().toLatin1(), orbitAlt);
	return FindOrbitSpawnLoc(startLoc, spawnShipId, spawnShipRadius, orbitAlt + 1000, avoidCreatures);
}

void DaWorldRingSection::FindStartupResources(int& eludium, int& lumenite)
{
	// Check each resource zone on this planet for the resources.
	AuZoneResource res;
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		QueryNaturalResources(res, rz);
		eludium += res.m_Eludium;
		lumenite += res.m_Lumenite;
	}
}

void DaWorldRingSection::CreateFloraTextures(AuGLEffects& re, int resourceZone)
{
	Q_ASSERT(m_pFlora);											// Caller insured this to be true.
	Q_ASSERT(!m_pFlora[resourceZone].Texture());				// Caller already checked this for us.
	//qDebug("DaWorldRingSection::CreateFloraTextures %d %s creating flora textures for zone %d", Id().GetID(),
			//(const char*)Name().toAscii(), resourceZone);
	m_pFlora[resourceZone].CreateDeviceObjects(re, Id(), GenSeed(), resourceZone, m_tmFloraChange);
	//qDebug("DaWorldRingSection::CreateFloraTextures %d %s creating flora textures for zone %d done", Id().GetID(),
			//(const char*)Name().toAscii(), resourceZone);
}

void DaWorldRingSection::CopyFlora(const DePlanetFlora* flora)
{
	m_RingSectionLock.lockForWrite();
	if(m_pFlora)
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz] = flora[rz];
	m_RingSectionLock.unlock();
}

DaAtom* DaWorldRingSection::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
		AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	if(dynamic_cast<const DaPerson*>(sightingAtom) && IsCachedSightTarget(eyeLoc, sightedLoc))
	{
		bool detailsOk;
		AuPoint sightLoc;
		if(!CachedSightTarget(detailsOk, sightLoc))
			return pBest;
		if(detailsOk)
		{
			pBest = (DaAtom*)this;								// Must cast me to non-const.
			sightedLoc = sightLoc;
			sightedShield = false;
			if(sightedSurfaceNormal)
				*sightedSurfaceNormal = AuPoint(0,0);
		}
	}
	else
	{
		// Perform coarse checks to determine if we can skip this section and its subatoms entirely, based on CheckChildExit.
		// Project the sight segment into the space of this atom.
		AuPoint seg0 = tModel.UnProject(eyeLoc);
		AuPoint seg1 = tModel.UnProject(sightedLoc);

		// Check if too far above/below the plane of the ecliptic of the ringworld.
		if((seg0.y > FeetPerOrbit()/5. && seg1.y > FeetPerOrbit()/5.) ||
				(seg0.y < -FeetPerOrbit()/5. && seg1.y < -FeetPerOrbit()/5.))
		{
			SetCachedSightTarget(eyeLoc, sightedLoc, 0, false, AuPoint(0,0));
			return pBest;											// too far above or below the plane of the ecliptic
		}
		
		// Check if too far inside the radius of the ringworld.
		double ringRad = OrbitRadius();								// ringworld radius in feet at sea level
		double solarRadius0 = AuPoint(seg0.x, seg0.z - ringRad).Length_2D();	// radius from sun, in plane of the ecliptic
		double solarRadius1 = AuPoint(seg1.x, seg1.z - ringRad).Length_2D();	// radius from sun, in plane of the ecliptic
		double solarRadMin = ringRad - FeetPerMoonOrbit()*2.25;
		if(solarRadius0 < solarRadMin && solarRadius1 < solarRadMin)
		{
			SetCachedSightTarget(eyeLoc, sightedLoc, 0, false, AuPoint(0,0));
			return pBest;											// too far inside the ring
		}

		// Check if too far outside the radius of the ringworld.
		AuPoint sunLoc(0,0,ringRad);
		double dist2 = sunLoc.DistToSegment2_2D(seg0, seg1);
		double solarRadMax = ringRad + FeetPerMoonOrbit();
		if(dist2 > solarRadMax*solarRadMax)
		{
			SetCachedSightTarget(eyeLoc, sightedLoc, 0, false, AuPoint(0,0));
			return pBest;											// Sight segment is beyond my outer zone.
		}

		// Check if segment is outside the sweep angle of this ringworld section.
		double lon0 = AuPoint(ringRad - seg0.z, seg0.x).ZRotation();
		double lon1 = AuPoint(ringRad - seg1.z, seg1.x).ZRotation();
		double lonMax = SectionAngle()/2.;
		if((lon0 > lonMax && lon1 > lonMax) || (lon0 < -lonMax && lon1 < -lonMax))
		{
			SetCachedSightTarget(eyeLoc, sightedLoc, 0, false, AuPoint(0,0));
			return pBest;											// outside sweep angle of ringworld section.
		}

		// Time to do the hard work on me and my subatoms.
		//qDebug() << "DaWorldRingSection::SightTarget" << Id() << Name();
		AuPoint sightLoc(sightedLoc);
		if(SightTargetDetails(eyeLoc, tModel, sightLoc, sightedPart, sightedShield) && sightLoc.Length2() < sightedLoc.Length2())
		{
			SetCachedSightTarget(eyeLoc, sightedLoc, 1, true, sightLoc);
			pBest = (DaAtom*)this;								// Must cast me to non-const.
			sightedLoc = sightLoc;
			if(sightedSurfaceNormal)
				*sightedSurfaceNormal = AuPoint(0,0);
		}
		else
			SetCachedSightTarget(eyeLoc, sightedLoc, 1, false, AuPoint(0,0));
	}

	// Base class performs a recursive search.
	return DaWorldLL::SightTarget(sightingAtom, eyeLoc, tModel, pBest, sightedLoc, sightedPart, sightedShield,
			sightedSurfaceNormal);
}

// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
// 'sightedLoc' and 'sightedPart' are not changed if false is returned.
bool DaWorldRingSection::SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
		unsigned int& sightedPart, bool& sightedShield) const
{
	// Project the sight segment into the space of this atom.
	AuPoint seg0 = tModel.UnProject(eyeLoc);
	AuPoint seg1 = tModel.UnProject(sightedLoc);

	// There can be no collision if both ends of the sight segment are above or both ends are below the width of the ringworld.
	double ringWid2 = TerrainYSize()/2;
	if(seg0.y > ringWid2 && seg1.y > ringWid2)
		return false;											// Sight segment is above the highest plane of this ringworld.
	if(seg0.y < -ringWid2 && seg1.y < -ringWid2)
		return false;											// Sight segment is below the lowest plane of this ringworld.

	// There can be no collision if both ends of the sight segment are closer to the sun than the height of the ring walls.
	double ringAltMax = AtmosphereHeightMSL() + 100*FeetPerMeter();
	double alt0 = AltitudeMSL(seg0);
	double alt1 = AltitudeMSL(seg1);
	if(alt0 > ringAltMax && alt1 > ringAltMax)
		return false;											// Sight segment is above the atmosphere.

	// There can be no collision if the closest point from the sun to the sight segment is beyond the outer hull radius.
	double ringRad = OrbitRadius();
	AuPoint sunLoc(0,0,ringRad);
	double dist2 = sunLoc.DistToSegment2_2D(seg0, seg1);
	double ringHullMax2 = ringRad - MinRadius();
	ringHullMax2 *= ringHullMax2;
	if(dist2 > ringHullMax2)
		return false;											// Sight segment is beyond my outer hull.

	// There can be no collision if both ends of the sight segment are before or after the beginning of my arc sweep.
	/* Already checked in SightTarget.
	double lonFrac0 = AuPoint(ringRad - seg0.z, seg0.x).ZRotation() / (SectionAngle()/2.);
	double lonFrac1 = AuPoint(ringRad - seg1.z, seg1.x).ZRotation() / (SectionAngle()/2.);
	if(lonFrac0 > 1 && lonFrac1 > 1)
		return false;											// Sight segment is beyond my arc sweep angle.
	if(lonFrac0 < -1 && lonFrac1 < -1)
		return false;											// Sight segment is beyond my arc sweep angle.
		*/

	// Check side hull walls.
	double hullY2 = ringWid2 + 100*FeetPerMeter();				// Y offset to outside of hull wall.
	double hullWallBotMSL = MinRadius();									// MSL altitude (-) at atmosphere envelope radius.
	double hullWallTopMSL = AtmosphereHeightMSL() + 100*FeetPerMeter();
	AuPlane sideLeft(AuPoint(0,hullY2), AuPoint(0,1));
	AuPoint sideHit;
	if(sideLeft.SegmentIntersection(seg0, seg1, sideHit))
	{
		// The plane was hit. How high is the hit location above/below the ring radius?
		double alt = AltitudeMSL(sideHit);
		if(alt > hullWallBotMSL && alt < hullWallTopMSL)
		{
			// Hit left hull wall.
			sightedLoc = tModel.Project(sideHit);
			sightedPart = 0;	// Left hull wall.
			sightedShield = false;
			return true;
		}
	}
	AuPlane sideRight(AuPoint(0,-hullY2), AuPoint(0,-1));
	if(sideRight.SegmentIntersection(seg0, seg1, sideHit))
	{
		// The plane was hit. How high is the hit location above/below the ring radius?
		double alt = AltitudeMSL(sideHit);
		if(alt > hullWallBotMSL && alt < hullWallTopMSL)
		{
			// Hit left hull wall.
			sightedLoc = tModel.Project(sideHit);
			sightedPart = 1;	// Right hull wall.
			sightedShield = false;
			return true;
		}
	}

	// Do some hard work to find out if the sight segment hits me.
	AuPoint segVec = seg1 - seg0;
	double segLength = segVec.Length();
	if(segLength == 0)
		return false;
	segVec /= segLength;				// Unit vector.

	// Integrate the sight segment in coarse 5' increments for speed, looking for the segment to go under ground.
	AuPoint prevTestLoc = seg0;
	bool ug = false;					// True if the segment goes under ground.
	AuPoint ugLoc;						// Underground location, if we find one.
	for(double testDist = 0; testDist <= segLength; testDist += 6)
	{
		AuPoint testLoc = seg0 + segVec*testDist;
		AuGlobeLL testLL;
		if(LLOfLoc(testLL, testLoc))
		{
			double testAlt = SurfaceAltitudeMSL(testLL);
			double alt = AltitudeMSL(testLoc);
			if(alt == testAlt)
			{
				// testLoc is exactly at the terrain altitude.
				sightedLoc = tModel.Project(testLoc);
				sightedPart = 0xffff;								// Dirt was hit
				sightedShield = false;
				return true;
			}
			else if(alt < testAlt)
			{
				// testLoc is below the terrain.
				ug = true;
				ugLoc = testLoc;
				break;
			}
		}
		prevTestLoc = testLoc;
	}

	if(ug)
	{
		// The coarse segment test went under ground.
		// Integrate the final segment in finer increments, looking for terrain intersections. The first point we find under ground
		// is the terrain hit location we will return.
		segLength = prevTestLoc.Distance(ugLoc) + .5;
		for(double testDist = .5; testDist <= segLength; testDist += .5)		// We already know testDist 0 is not under ground.
		{
			AuPoint testLoc = prevTestLoc + segVec*testDist;
			AuGlobeLL testLL;
			if(LLOfLoc(testLL, testLoc))
			{
				double testAlt = SurfaceAltitudeMSL(testLL);
				double alt = AltitudeMSL(testLoc);
				if(alt <= testAlt)
				{
					// testLoc is at or below the terrain.
					ElevateLocMSL(testLoc, testAlt);				// Move testLoc to be exactly at the terrain surface.
					sightedLoc = tModel.Project(testLoc);
					sightedPart = 0xffff;							// Dirt was hit
					sightedShield = false;
					return true;
				}
			}
		}
	}

	// No point along the test segment was beneath the ground.
	return false;
}

bool DaWorldRingSection::IsSightTarget(const DaAtom* /*toWho*/) const
{
	return true;
}

// Returns true if all the flags in all the cities on this planet are controlled by the 'empireId' and if none of those
// cities is an occupied city.
int DaWorldRingSection::OkToRename(quint32 empireId) const
{
	if(!HasSubatoms())
		return 1;

	// Check all cities. All flags of all cities must belong to 'empireId' and no city may be occupied.
	bool anyCityOfEmpire = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* city = dynamic_cast<const DaBd*>(*it);
		if(!city)
			continue;
		if(city->IndustryId() != IID_Capitol && city->IndustryId() != IID_BaseHeadquarters)
			continue;
		if(city->EmpireId() != empireId)
			return 3;										// Any flag controlled by a different empire prevents rename.

		// We have found one city that enables rename of the planet.
		anyCityOfEmpire = true;
	}

	// If we get here, we need to have found a city of the empire to enable rename.
	return anyCityOfEmpire ? 0 : 1;
}

// Create the nearby model surface. Called by the model maker thread.
void DaWorldRingSection::CreateNearGlobeSurfaceTriangles(QList<AuGlobeTriTexNShineSnow>& tris) const
{
	//qDebug() << "DaWorldRingSection::CreateNearGlobeSurfaceTriangles" << Id() << Name() << "started...";
	// Tessellate surface for far model. This tessellation does not include any altitude information, though it does return the
	// altitude minimum and maximums MSL so each vertex only gets that calculation once. The min and max are used when creating
	// the terrain lookup table for the shader.
	AuGlobeLLTriList terrainTris;
	float maxAltMSL, minAltMSL;
	CreateSurfaceTriangles(terrainTris, SeaLevelEquatorDivisions()*7/2, maxAltMSL, minAltMSL);

	// Create the triangles that make up the model. These will have vertices at the appropriate altitude and texture coordinates
	// that accomodate the surface map.
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo> terAltInfo;	// Info that gets cached along the way.
	tris.clear();
	for(AuGlobeLLTriList::const_iterator ti = terrainTris.cbegin(); ti != terrainTris.cend(); ti++)
		CreateNearGlobeSurfaceTriangle(tris, (*ti).m_LL0, (*ti).m_LL1, (*ti).m_LL2, terAltInfo);
	//qDebug() << "DaWorldRingSection::CreateNearGlobeSurfaceTriangles" << Id() << Name() << "tris" << tris.count();
}

void DaWorldRingSection::CreateNearGlobeSurfaceTriangle(QList<AuGlobeTriTexNShineSnow>& tris, const AuGlobeLL& ll0,
		const AuGlobeLL& ll1, const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const
{
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti0 = terAltInfo.find(ll0);
	if(ti0 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll0);
		t.m_TerCoord = GetNearGlobeSurfaceLocation(ll0, t.m_TerAltMSL, t.m_TerNormal);
		t.m_Shine = (!m_byWater || t.m_TerAltMSL > 0) ? 0 : s_seaHardness;
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll0.Latitude(), 0);
		t.m_TerTex.setX(ll0.Longitude()/(PIf*2) + .5f);
		t.m_TerTex.setY(ll0.Latitude()/PIf + .5f);
		ti0 = terAltInfo.insert(ll0, t);
	}

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti1 = terAltInfo.find(ll1);
	if(ti1 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll1);
		t.m_TerCoord = GetNearGlobeSurfaceLocation(ll1, t.m_TerAltMSL, t.m_TerNormal);
		t.m_Shine = (!m_byWater || t.m_TerAltMSL > 0) ? 0 : s_seaHardness;
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll1.Latitude(), 0);
		t.m_TerTex.setX(ll1.Longitude()/(PIf*2) + .5f);
		t.m_TerTex.setY(ll1.Latitude()/PIf + .5f);
		ti1 = terAltInfo.insert(ll1, t);
	}

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti2 = terAltInfo.find(ll2);
	if(ti2 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll2);
		t.m_TerCoord = GetNearGlobeSurfaceLocation(ll2, t.m_TerAltMSL, t.m_TerNormal);
		t.m_Shine = (!m_byWater || t.m_TerAltMSL > 0) ? 0 : s_seaHardness;
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll2.Latitude(), 0);
		t.m_TerTex.setX(ll2.Longitude()/(PIf*2) + .5f);
		t.m_TerTex.setY(ll2.Latitude()/PIf + .5f);
		ti2 = terAltInfo.insert(ll2, t);
	}

	// Construct a globe triangle.
	AuGlobeTriTexNShineSnow tri;
	tri.m_Vert0 = AuGlobeVertTexNShineSnow((*ti0).m_TerCoord, (*ti0).m_TerTex, (*ti0).m_TerNormal, (*ti0).m_Shine, (*ti0).m_Snow);
	tri.m_Vert1 = AuGlobeVertTexNShineSnow((*ti1).m_TerCoord, (*ti1).m_TerTex, (*ti1).m_TerNormal, (*ti1).m_Shine, (*ti1).m_Snow);
	tri.m_Vert2 = AuGlobeVertTexNShineSnow((*ti2).m_TerCoord, (*ti2).m_TerTex, (*ti2).m_TerNormal, (*ti2).m_Shine, (*ti2).m_Snow);

	// Append the globe triangle to the list.
	tris.append(tri);
}

// Create all the data for the low tessellation 'at' globe VAOs that can be created in a background thread.
void DaWorldRingSection::CreateAtGlobeSurfaceTriangles(QList<AuGlobeTriAltVegN>& tris, float& maxAltMSL, float& minAltMSL) const
{
	//qDebug() << "DaWorldRingSection::CreateAtGlobeSurfaceTriangles" << Id() << Name() << "started";

	// Tessellate surface of globe for at-globe model. This tessellation only stores longitudes and latitudes. Terrain altitudes
	// are returned for efficiency but they are not stored in the terrainTris data.
	AuGlobeLLTriList terrainTris;
	CreateSurfaceTriangles(terrainTris, GroundLevelEquatorDivisions()*2, maxAltMSL, minAltMSL);
	//qDebug() << "DaWorldRingSection::CreateAtGlobeSurfaceTriangles" << Id() << Name() << "terrainTris" << terrainTris.count();

	// Create the triangles that make up the model.
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo> terAltInfo;	// Info that gets cached along the way.
	AuGlobeTriAltVegN tri;
	tris.clear();
	for(AuGlobeLLTriList::const_iterator ti = terrainTris.cbegin(); ti != terrainTris.cend(); ti++)
	{
		CreateAtGlobeSurfaceTriangle(tri, (*ti).m_LL0, (*ti).m_LL1, (*ti).m_LL2, terAltInfo);
		tris.append(tri);
	}
	//qDebug() << "DaWorldRingSection::CreateAtGlobeSurfaceTriangles" << Id() << Name() << "done" << tris.count() << "triangles";
}

void DaWorldRingSection::CreateAtTerrainSurfaceTriangle(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLL& ll0,
		const AuGlobeLL& ll1, const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const
{
	int texreps = GroundLevelEquatorDivisions() * TERRAIN_TEXTURE_REPS;
	double aspectXY = TerrainXSize() / TerrainYSize();

	// Calculate triangle vertex altitudes above the center of the globe.
	// Pavement type is the TERTEX tile index as a single linear value from 0..1.
	bool hideRocks, hideShrubs, hideTrees;
	COMMODITYID farmShrub, orchardTree;
	double farmShrubMSL, orchardTreeMSL;
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti0 = terAltInfo.find(ll0);
	if(ti0 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSLPave(ll0, hideRocks, t.m_HideGrass, hideShrubs, hideTrees, t.m_PavementType, farmShrub,
				farmShrubMSL, orchardTree, orchardTreeMSL);
		t.m_CityAmbience = CityAmbientLight(ll0.ToLLF(), t.m_PavementType);
		int inEnclosure = InEnclosure(ll0, t.m_TerAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		t.m_TerCoord = GetTerrainLocation(ll0, -2./12.);
		t.m_TerNormal = TerrainNormal(t.m_TerCoord);
		int irrigated = IsIrrigated(ll0);

		// Calculate vegetation density.
		if(t.m_HideGrass == 1)
			t.m_Veg = 0;	// No grass at all.
		else if(t.m_HideGrass == 3)
			t.m_Veg = 100;	// Grassy lawn.
		else if(t.m_TerAltMSL < 0 && inEnclosure != 2)
			t.m_Veg = 0;
		else
			t.m_Veg = VegetationAt(ll0, t.m_TerAltMSL, inEnclosure, irrigated);

		// Calculate snow due to polar caps and mountain peaks.
		// Do not put snow on pavement that is used on roads and around buildings. This allows snow in other "paved" areas such as
		// yards and farmland.
		t.m_Snow = (t.m_PavementType <= WORLDTEX_CobblestoneLight) ? 0 : SnowCoverage(t.m_TerAltMSL, ll0.Latitude(), inEnclosure);
		t.m_TerTex.setX((ll0.Longitude())/(PIf*2) * texreps * aspectXY);
		t.m_TerTex.setY((ll0.Latitude() + PIf/2)/PIf * texreps);
		ti0 = terAltInfo.insert(ll0, t);
	}
	
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti1 = terAltInfo.find(ll1);
	if(ti1 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSLPave(ll1, hideRocks, t.m_HideGrass, hideShrubs, hideTrees, t.m_PavementType, farmShrub,
				farmShrubMSL, orchardTree, orchardTreeMSL);
		t.m_CityAmbience = CityAmbientLight(ll1.ToLLF(), t.m_PavementType);
		int inEnclosure = InEnclosure(ll1, t.m_TerAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		t.m_TerCoord = GetTerrainLocation(ll1, -2./12.);
		t.m_TerNormal = TerrainNormal(t.m_TerCoord);
		int irrigated = IsIrrigated(ll1);

		// Calculate vegetation density.
		if(t.m_HideGrass == 1)
			t.m_Veg = 0;	// No grass at all.
		else if(t.m_HideGrass == 3)
			t.m_Veg = 100;	// Grassy lawn.
		else if(t.m_TerAltMSL < 0 && inEnclosure != 2)
			t.m_Veg = 0;
		else
			t.m_Veg = VegetationAt(ll1, t.m_TerAltMSL, inEnclosure, irrigated);

		// Calculate snow due to polar caps and mountain peaks.
		// Do not put snow on pavement that is used on roads and around buildings. This allows snow in other "paved" areas such as
		// yards and farmland.
		t.m_Snow = (t.m_PavementType <= WORLDTEX_CobblestoneLight) ? 0 : SnowCoverage(t.m_TerAltMSL, ll1.Latitude(), inEnclosure);
		t.m_TerTex.setX((ll1.Longitude())/(PIf*2) * texreps * aspectXY);
		t.m_TerTex.setY((ll1.Latitude() + PIf/2)/PIf * texreps);
		ti1 = terAltInfo.insert(ll1, t);
	}
	
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti2 = terAltInfo.find(ll2);
	if(ti2 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSLPave(ll2, hideRocks, t.m_HideGrass, hideShrubs, hideTrees, t.m_PavementType, farmShrub,
				farmShrubMSL, orchardTree, orchardTreeMSL);
		t.m_CityAmbience = CityAmbientLight(ll2.ToLLF(), t.m_PavementType);
		int inEnclosure = InEnclosure(ll2, t.m_TerAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		t.m_TerCoord = GetTerrainLocation(ll2, -2./12.);
		t.m_TerNormal = TerrainNormal(t.m_TerCoord);
		int irrigated = IsIrrigated(ll2);

		// Calculate vegetation density.
		if(t.m_HideGrass == 1)
			t.m_Veg = 0;	// No grass at all.
		else if(t.m_HideGrass == 3)
			t.m_Veg = 100;	// Grassy lawn.
		else if(t.m_TerAltMSL < 0 && inEnclosure != 2)
			t.m_Veg = 0;
		else
			t.m_Veg = VegetationAt(ll2, t.m_TerAltMSL, inEnclosure, irrigated);

		// Calculate snow due to polar caps and mountain peaks.
		// Do not put snow on pavement that is used on roads and around buildings. This allows snow in other "paved" areas such as
		// yards and farmland.
		t.m_Snow = (t.m_PavementType <= WORLDTEX_CobblestoneLight) ? 0 : SnowCoverage(t.m_TerAltMSL, ll2.Latitude(), inEnclosure);
		t.m_TerTex.setX((ll2.Longitude())/(PIf*2) * texreps * aspectXY);
		t.m_TerTex.setY((ll2.Latitude() + PIf/2)/PIf * texreps);
		ti2 = terAltInfo.insert(ll2, t);
	}

	// Pavement coverage fades off to the vertexes with no pavement.
	float paveCover0 = ((*ti0).m_PavementType == WORLDTEX_Count) ? 0 : 1;
	float paveCover1 = ((*ti1).m_PavementType == WORLDTEX_Count) ? 0 : 1;
	float paveCover2 = ((*ti2).m_PavementType == WORLDTEX_Count) ? 0 : 1;

	// We have to select only one of the pave values and use it for every vertex.
	// Prefer the lowest numbered pavement type to look more consistent when different types areas collide.
	float paveType;
	if((*ti0).m_PavementType == WORLDTEX_Count && (*ti1).m_PavementType == WORLDTEX_Count &&
			(*ti2).m_PavementType == WORLDTEX_Count)
		paveType = 0;					// No pavement at all.
	else
	{
		// Select the lowest numbered pavement type for consistency. This also avoids the one(s) that are WORLDTEX_Count.
		if((*ti0).m_PavementType < (*ti1).m_PavementType && (*ti0).m_PavementType < (*ti2).m_PavementType)
			paveType = (float)(*ti0).m_PavementType / 7.f;
		else if((*ti1).m_PavementType < (*ti2).m_PavementType)
			paveType = (float)(*ti1).m_PavementType / 7.f;
		else
			paveType = (float)(*ti2).m_PavementType / 7.f;
	}

	// Construct a terrain triangle.
	AuGlobeTriAltVegNPave tri;
	tri.m_Vert0 = AuGlobeVertAltVegNPave((*ti0).m_TerCoord, (*ti0).m_TerTex, (*ti0).m_TerAltMSL, (*ti0).m_Snow, (*ti0).m_Veg,
			(*ti0).m_TerNormal, paveCover0, paveType, (*ti0).m_CityAmbience);
	tri.m_Vert1 = AuGlobeVertAltVegNPave((*ti1).m_TerCoord, (*ti1).m_TerTex, (*ti1).m_TerAltMSL, (*ti1).m_Snow, (*ti1).m_Veg,
			(*ti1).m_TerNormal, paveCover1, paveType, (*ti1).m_CityAmbience);
	tri.m_Vert2 = AuGlobeVertAltVegNPave((*ti2).m_TerCoord, (*ti2).m_TerTex, (*ti2).m_TerAltMSL, (*ti2).m_Snow, (*ti2).m_Veg,
			(*ti2).m_TerNormal, paveCover2, paveType, (*ti2).m_CityAmbience);
	tris.append(tri);
}

void DaWorldRingSection::CreateAtGlobeSurfaceTriangle(AuGlobeTriAltVegN& tri, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
		const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const
{
	int texreps = GroundLevelEquatorDivisions() * TERRAIN_TEXTURE_REPS;
	double aspectXY = TerrainXSize() / TerrainYSize();

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti0 = terAltInfo.find(ll0);
	if(ti0 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll0);
		t.m_TerCoord = GetTerrainLocation(ll0);
		t.m_TerNormal = TerrainNormal(t.m_TerCoord);
		int inEnclosure = InEnclosure(ll0, t.m_TerAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		int irrigated = IsIrrigated(ll0);
		t.m_Veg = VegetationAt(ll0, t.m_TerAltMSL, inEnclosure, irrigated);
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll0.Latitude(), inEnclosure);
		t.m_TerTex.setX((ll0.Longitude())/(PIf*2) * texreps * aspectXY);
		t.m_TerTex.setY((ll0.Latitude() + PIf/2)/PIf * texreps);
		ti0 = terAltInfo.insert(ll0, t);
	}

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti1 = terAltInfo.find(ll1);
	if(ti1 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll1);
		t.m_TerCoord = GetTerrainLocation(ll1);
		t.m_TerNormal = TerrainNormal(t.m_TerCoord);
		int inEnclosure = InEnclosure(ll1, t.m_TerAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		int irrigated = IsIrrigated(ll1);
		t.m_Veg = VegetationAt(ll1, t.m_TerAltMSL, inEnclosure, irrigated);
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll1.Latitude(), inEnclosure);
		t.m_TerTex.setX((ll1.Longitude())/(PIf*2) * texreps * aspectXY);
		t.m_TerTex.setY((ll1.Latitude() + PIf/2)/PIf * texreps);
		ti1 = terAltInfo.insert(ll1, t);
	}

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti2 = terAltInfo.find(ll2);
	if(ti2 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll2);
		t.m_TerCoord = GetTerrainLocation(ll2);
		t.m_TerNormal = TerrainNormal(t.m_TerCoord);
		int inEnclosure = InEnclosure(ll2, t.m_TerAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		int irrigated = IsIrrigated(ll2);
		t.m_Veg = VegetationAt(ll2, t.m_TerAltMSL, inEnclosure, irrigated);
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll2.Latitude(), inEnclosure);
		t.m_TerTex.setX((ll2.Longitude())/(PIf*2) * texreps * aspectXY);
		t.m_TerTex.setY((ll2.Latitude() + PIf/2)/PIf * texreps);
		ti2 = terAltInfo.insert(ll2, t);
	}

	// Construct a globe triangle.
	tri.m_Vert0 = AuGlobeVertAltVegN((*ti0).m_TerCoord, (*ti0).m_TerTex, (*ti0).m_TerAltMSL, (*ti0).m_Snow, (*ti0).m_Veg,
			(*ti0).m_TerNormal);
	tri.m_Vert1 = AuGlobeVertAltVegN((*ti1).m_TerCoord, (*ti1).m_TerTex, (*ti1).m_TerAltMSL, (*ti1).m_Snow, (*ti1).m_Veg,
			(*ti1).m_TerNormal);
	tri.m_Vert2 = AuGlobeVertAltVegN((*ti2).m_TerCoord, (*ti2).m_TerTex, (*ti2).m_TerAltMSL, (*ti2).m_Snow, (*ti2).m_Veg,
			(*ti2).m_TerNormal);
}

int DaWorldRingSection::CanHaveFloraAtAltitude(float altitudeMSL) const
{
	// Test for under water.
	if(m_byWater && altitudeMSL <= 0)
	{
		// Location is beneath the waves.
		// Verify underwater altitude is ok for plants. They exist in a narrow band just below the waves.
		if(altitudeMSL > -10 || altitudeMSL < -SeaEuphoticDepth())
			return 0;	// Not in the depth zone for plants.
	}
	else
	{
		// Location is above the waves.
		// Check for snow capped peaks.
		if(altitudeMSL >= TreeLineMSL())
			return 2;
	}

	// Passed all the tests.
	return 1;
}

quint8 DaWorldRingSection::VegetationAt(const AuGlobeLL& ll, float terAltMSL, int inEnclosure, int irrigated) const
{
	// No vegetation in the connector zones at the ends.
	if(fabs(ll.Longitude()) > 170.*DEG2RAD)
		return 0;

	unsigned char avgFoliage;
	if(inEnclosure == 2)
	{
		// In undersea enclosure.
		if(irrigated)
			avgFoliage = 60;
		else
			avgFoliage = 50;
	}
	else
	{
		int fat = CanHaveFloraAtAltitude(terAltMSL);
		if(fat == 0)
			return 0;
		else if(fat == 2)
		{
			// No foliage unless there is an enclosure there.
			if(!inEnclosure)
				return 0;
			avgFoliage = 50;
		}
		else if(irrigated)
			avgFoliage = 60;
		else
		{
			avgFoliage = AverageFoliage();
			if(inEnclosure && avgFoliage < 50)
				avgFoliage = 50;
		}
	}

	bool underwater = m_byWater && terAltMSL < 0;

	//return (int)(0.5f/8.f * 100);	// Desert
	//return (int)(1.5f/8.f * 100);	// Grassland
	//return (int)(2.5f/8.f * 100);	// Shrubland
	//return (int)(3.5f/8.f * 100);	// Light forest
	//return (int)(4.5f/8.f * 100);	// Moderate forest
	//return (int)(5.5f/8.f * 100);	// Heavy forest
	//return (int)(6.5f/8.f * 100);	// Dense forest
	//return (int)(7.5f/8.f * 100);	// Heart of the forest

	double aspectYX = TerrainYSize() / TerrainXSize();

	// Calculate max noise due to persistence value at 7 octaves.
	m_RingSectionLock.lockForRead();
	float vegPersFactor = 1.f + m_TerrainPersistence + m_TerrainPersistence*m_TerrainPersistence + powf(m_TerrainPersistence,3) +
		powf(m_TerrainPersistence,4) + powf(m_TerrainPersistence,5) + powf(m_TerrainPersistence,6);
	float fx = m_TerrainXOff - ll.Longitude()/PIf * m_TerrainFrequency;	// X noise offset
	if(underwater)
		fx = -fx;
	const float fy = m_TerrainYOff - ll.Latitude()/PIf * m_TerrainFrequency * aspectYX;	// Y noise offset
	float vegFactor = fabs(AuNoiseF::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/vegPersFactor);	// scale result to 0 to 1
	m_RingSectionLock.unlock();

	// Nobody expects deserts in their Enclosures.
	if((inEnclosure || irrigated) && vegFactor < .25f)
		vegFactor = .25f;

	// Calculate the amount based on overall plant density attributed to water and atmosphere.
	float vegAt = vegFactor * avgFoliage * 2;
	if(vegAt > 100)
		vegAt = 100;
	return (quint8)(vegAt);
}

void DaWorldRingSection::RenderNearGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		float ambientLight)
{
	if(g_Au.m_DebugSwitches & 0x00000008)
		return;

	if(!m_NearGlobe.VAO())
		m_NearGlobe = ari.mm.NearRingworldSectionModel(*ari.re, this);

	if(!m_NearGlobe.VAO() || !m_glTexNearSurface)
	{
		// Near globe model or surface texture is not ready yet so render distant globe model.
		RenderAtmosphere(projection, model, *ari.re, ari.mm, 3, 0, false, ambientLight,1);
		return;
	}

	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_BACK, GL_LINE);

	m_NearGlobe.SelectGL();

	// Cities use the glow shader for the glow effect of the city lights.
	if(BdHere())
	{
		ari.re->GLSL().NearRingworldGlowShader().Select();
		ari.re->GLSL().NearRingworldGlowShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().NearRingworldGlowShader().SetSurfaceTexture(m_glTexNearSurface);

		// Set eye position in object space for specular lighting calculations.
		AuPoint eyeLoc = model.UnProject(ari.re->EyeLocation());
		ari.re->GLSL().NearRingworldGlowShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

		// Ambient light
		ari.re->GLSL().NearRingworldGlowShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. LavaShaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
			AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
			AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

			// Configure shader for object space lighting.
			ari.re->GLSL().NearRingworldGlowShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
					ShadowPanelSunAngle(sun.m_ShadowAngle),
					sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
		}

		m_NearGlobe.DrawGL();

		ari.re->GLSL().NearRingworldGlowShader().Deselect();
	}
	else
	{
		ari.re->GLSL().NearRingworldShader().Select();
		ari.re->GLSL().NearRingworldShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().NearRingworldShader().SetSurfaceTexture(m_glTexNearSurface);

		// Set eye position in object space for specular lighting calculations.
		AuPoint eyeLoc = model.UnProject(ari.re->EyeLocation());
		ari.re->GLSL().NearRingworldShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

		// Ambient light
		ari.re->GLSL().NearRingworldShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
			AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
			AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

			// Configure shader for object space lighting.
			ari.re->GLSL().NearRingworldShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
					ShadowPanelSunAngle(sun.m_ShadowAngle),
					sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
		}

		m_NearGlobe.DrawGL();

		ari.re->GLSL().NearRingworldShader().Deselect();
	}

	m_NearGlobe.DeselectGL();

	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
}

void DaWorldRingSection::RenderNearGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	if(fogDensity <= 0)
	{
		// Don't bother with fog calculations when there is no fog.
		RenderNearGlobe(projection, model, ari, ambientLight);
		return;
	}

	if(!m_NearGlobe.VAO())
		m_NearGlobe = ari.mm.NearRingworldSectionModel(*ari.re, this);

	if(!m_NearGlobe.VAO() || !m_glTexNearSurface)
	{
		// Near globe model or surface texture is not ready yet so render distant globe model.
		RenderAtmosphere(projection, model, *ari.re, ari.mm, 3, 0, false, ambientLight,1);
		return;
	}

	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_BACK, GL_LINE);

	m_NearGlobe.SelectGL();

	// Cities use the glow shader for the glow effect of the city lights.
	if(BdHere())
	{
		ari.re->GLSL().NearRingworldGlowFogShader().Select();
		ari.re->GLSL().NearRingworldGlowFogShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().NearRingworldGlowFogShader().SetSurfaceTexture(m_glTexNearSurface);
		ari.re->GLSL().NearRingworldGlowFogShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Set eye position in object space for specular lighting calculations.
		AuPoint eyeLoc = model.UnProject(ari.re->EyeLocation());
		ari.re->GLSL().NearRingworldGlowFogShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

		// Ambient light
		ari.re->GLSL().NearRingworldGlowFogShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. LavaShaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
			AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
			AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

			// Configure shader for object space lighting.
			ari.re->GLSL().NearRingworldGlowFogShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR,
					sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
					ShadowPanelSunAngle(sun.m_ShadowAngle),
					sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
		}

		m_NearGlobe.DrawGL();

		ari.re->GLSL().NearRingworldGlowFogShader().Deselect();
	}
	else
	{
		ari.re->GLSL().NearRingworldFogShader().Select();
		ari.re->GLSL().NearRingworldFogShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().NearRingworldFogShader().SetSurfaceTexture(m_glTexNearSurface);
		ari.re->GLSL().NearRingworldFogShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Set eye position in object space for specular lighting calculations.
		AuPoint eyeLoc = model.UnProject(ari.re->EyeLocation());
		ari.re->GLSL().NearRingworldFogShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

		// Ambient light
		ari.re->GLSL().NearRingworldFogShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
			AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
			AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

			// Configure shader for object space lighting.
			ari.re->GLSL().NearRingworldFogShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
					ShadowPanelSunAngle(sun.m_ShadowAngle),
					sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
		}

		m_NearGlobe.DrawGL();

		ari.re->GLSL().NearRingworldFogShader().Deselect();
	}

	m_NearGlobe.DeselectGL();

	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
}

void DaWorldRingSection::CreateAtGlobe(AuGLEffects& re, AuGLModelMaker& mm)
{
	Q_ASSERT(!m_AtGlobe.m_Low.VAO());

	// Request low-tessellation of at-globe model from model maker thread. We may make this request many times before it succeeds.
	mm.RingworldSectionModel(re, m_AtGlobe, this);
	if(m_AtGlobe.m_Low.VAO())
	{
		//qDebug() << "DaWorldRingSection::CreateAtGlobe" << Id() << Name() << "terrainTris" << m_AtGlobe.m_TerrainTris.count();
		// We got a globe model so create the lookup tables used for texture selection based on altitudes of terrain.
		// Altitude range of all land, for altitude factor calculation.
		m_AtGlobe.m_TexLookup = CreateAtLookupTable(re, m_AtGlobe.m_MaxAltMSL, m_AtGlobe.m_MinAltMSL, m_AtGlobe.m_TerElevTex);
	}
}

void DaWorldRingSection::UpdateNearSurfaceTexture(AuAtomRenderInfo& ari)
{
	if(m_glTexNearSurface && !m_glTexNearValid)
	{
		ari.re->glDeleteTextures(1, &m_glTexNearSurface);
		m_glTexNearSurface = 0;
	}
	if(!m_glTexNearSurface)
	{
		m_glTexNearSurface = CreatePlanetTexture(ari, 4096, true);	// Create detailed surface texture for near globe model.
		if(m_glTexNearSurface)
			m_glTexNearValid = true;
		else
			m_glTexNearValid = false;
	}
}

// Render the globe when the viewer is inside the atmosphere.
void DaWorldRingSection::RenderAtGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	//qDebug() << "DaWorldRingSection::RenderAtGlobe" << Id() << Name() << "ambient" << ambientLight << "fog" << fogDensity
		//<< "fogRgb" << fogR << fogG << fogB;
	//qDebug() << "DaWorldRingSection::RenderAtGlobe" << Id() << Name() << "vao" << m_AtGlobe.VAO();
	if(!m_AtGlobe.m_Low.VAO())
	{
		CreateAtGlobe(*ari.re, ari.mm);			// Request low-tessellated 'at' globe model from background model maker thread.
		if(!m_AtGlobe.m_Low.VAO())
		{
			// At-this-world globe model is not ready so render the near globe model instead.
			RenderNearGlobeFog(projection, model, ari, ambientLight, fogDensity, fogR, fogG, fogB);
			return;
		}
	}

	if(g_Au.m_DebugSwitches & 0x00000040)
		return;

//#if !defined(QT_NO_DEBUG)
	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_BACK, GL_LINE);
//#endif

	// Periodically we cleanup unneeded terrain patches. Our frame rate drops to a crawl if we do this in every frame.
	bool timeForCleanup = !m_TerrainCleanupTime.isValid() || m_TerrainCleanupTime.elapsed() > 3000;
	if(timeForCleanup)
		m_TerrainCleanupTime.restart();

	// Render high tessellation of nearby globe patches.
	// Need to know viewer pos on the ground so we don't clip away terrain when they are high in the air.
	int countAttempted = 0;				// Number of highly tessellated patches we attempt to display.
	int countShown = 0;					// Number of highly tessellated patches we actually displayed.

	AuPoint groundPos(viewerPos);
	ElevateLocMSL(groundPos, 0);		// Adjust viewerPos to sea level.

	//qDebug() << "RingworldTerrainLitFloraShader" << m_AtGlobe.m_MinAltMSL << m_AtGlobe.m_MaxAltMSL << m_AtGlobe.m_TexLookup;
	// Flora shader.
	ari.re->GLSL().RingworldTerrainLitFloraShader().Select(projection);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetModelView(model);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetNoiseTexture(s_TexNoise);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetPavementPalette(s_TexTerPalette);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetTerrainPalette(m_AtGlobe.m_TerElevTex);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetGrassTexture(m_glTexGrass);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetTerrainLookup(m_AtGlobe.m_TexLookup);
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetAltitudeNoiseFactor(m_AtGlobe.AltNoiseFactor(AltitudeIncrement()/3));
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetAmbientLight(ambientLight);
	const QColor& c = AuGLLight::ColorSodiumVapor();
	ari.re->GLSL().RingworldTerrainLitFloraShader().SetCityAmbientLight(c.redF() * (float)AuGLLight::AmbientLitCity(),
			c.greenF() * (float)AuGLLight::AmbientLitCity(), c.blueF() * (float)AuGLLight::AmbientLitCity());

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
		AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
		AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

		// Configure shader for object space lighting.
		ari.re->GLSL().RingworldTerrainLitFloraShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR,
				sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
				ShadowPanelSunAngle(sun.m_ShadowAngle),
				sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
	}

	// Point lights
	for(unsigned int lightIndex=0; lightIndex<ari.re->MaxLights(); lightIndex++)
	{
		// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLLight& light = ari.re->Light(lightIndex);
		AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
		AuPoint spotDir;
		if(!light.m_Direction.IsZero())
			spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

		// Configure shader for object space lighting.
		ari.re->GLSL().RingworldTerrainLitFloraShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ,
				light.m_ColorR, light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
				light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
	}

	// Display highly tessellated globe VAOs.
	int vidx=0;
	for(QList<AuGlobeTerrainTriCen>::const_iterator tti = m_AtGlobe.m_TerrainTris.cbegin();
			tti != m_AtGlobe.m_TerrainTris.cend(); tti++, vidx++)
	{
		// When they are close enough, we show the high tessellation VAO of the surface triangle.
		double d0 = (*tti).m_Loc0.Distance2(groundPos);
		double d1 = (*tti).m_Loc1.Distance2(groundPos);
		double d2 = (*tti).m_Loc2.Distance2(groundPos);
		if(d0 < g_Au.DetailedTerrainRangeFt2() ||
				d1 < g_Au.DetailedTerrainRangeFt2() ||
				d2 < g_Au.DetailedTerrainRangeFt2())
		{
			// Triangle location is close enough to see high level of detail.
			countAttempted++;
			unordered_map<int, AuGLTriVAO, AuTriHash>::iterator vi = m_AtGlobe.m_TriVAOs.find(vidx);
			if(vi == m_AtGlobe.m_TriVAOs.end() || !(*vi).second.VAO() || !(*vi).second.IsValid())
			{
				// Request high tessellation VAO of surface triangle from model maker thread.
				// We could get none, or the same invalid one back, again until the nu model becomes valid.
				AuGLTriVAO vao;
				ari.mm.RingworldSectionModelTri(*ari.re, vao, this, vidx, *tti, m_AtGlobe.m_MaxAltMSL, m_AtGlobe.m_MinAltMSL);
				if(vao.VAO())
				{
					// Got a VAO, could be invalid but still usable.
					vi = m_AtGlobe.m_TriVAOs.find(vidx);
					if(vi != m_AtGlobe.m_TriVAOs.end())
						m_AtGlobe.m_TriVAOs.erase(vi);
					vi = m_AtGlobe.m_TriVAOs.insert(pair<int, AuGLTriVAO>(vidx, vao)).first;
					m_AtGlobe.m_TriWait.remove(vidx);
				}
				else
					m_AtGlobe.m_TriWait.insert(vidx);	// Remember that the VAO was requested so we can delete it.
			}
			if(vi != m_AtGlobe.m_TriVAOs.end())
			{
				// Display the high-tessellation VAO.
				(*vi).second.SelectGL();
				(*vi).second.DrawGL();
				(*vi).second.DeselectGL();
				countShown++;
			}
		}
		else if(timeForCleanup)
		{
			// Terrain patch is far enough away to kill it. Otherwise they build up.
			m_AtGlobe.DeleteRingworldSectionTriVAO(Id(), vidx, ari.mm);
		}
	}
	ari.re->GLSL().RingworldTerrainLitFloraShader().Deselect();
	//qDebug() << "DaWorldRingSection::RenderAtGlobe" << Id() << Name() << "attempted" << countAttempted << "shown" << countShown;

	// Select clipping parameters to omit the fragments of the globe that are within the highly tessellated globe arc of the
	// viewer's position.
	float clipDist;
	AuPoint clipCenter;
	if((float)countShown/(float)countAttempted < .7071f)
	{
		// We did not draw all of the highly tessellated ground patches. Don't make a hole until they are all present.
		clipDist = 0;											// No clipping when there are no highly tessellated VAOs ready yet.
	}
	else
	{
		// We drew all of the highly tessellated ground patches we wanted to display. Set clipping to cut out the circle.
		clipDist = g_Au.DetailedTerrainRangeFt();
		clipCenter = groundPos;
	}

	// Render low tessellation of globe model, with shader configured to omit the portion near the viewer.
	//qDebug() << "RingworldLitFloraShader" << m_AtGlobe.m_MinAltMSL << m_AtGlobe.m_MaxAltMSL << m_AtGlobe.m_TexLookup;
	// Flora shader.
	ari.re->GLSL().RingworldLitFloraShader().Select();
	ari.re->GLSL().RingworldLitFloraShader().SetProjectionModelView(projection * model);
	ari.re->GLSL().RingworldLitFloraShader().SetNoiseTexture(s_TexNoise);
	ari.re->GLSL().RingworldLitFloraShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
	ari.re->GLSL().RingworldLitFloraShader().SetTerrainPalette(m_AtGlobe.m_TerElevTex);
	ari.re->GLSL().RingworldLitFloraShader().SetGrassTexture(m_glTexGrass);
	ari.re->GLSL().RingworldLitFloraShader().SetTerrainLookup(m_AtGlobe.m_TexLookup);
	ari.re->GLSL().RingworldLitFloraShader().SetAltitudeNoiseFactor(m_AtGlobe.AltNoiseFactor(AltitudeIncrement()/3));
	ari.re->GLSL().RingworldLitFloraShader().SetFog(fogDensity, fogR, fogG, fogB);
	ari.re->GLSL().RingworldLitFloraShader().SetClip(clipDist, clipCenter.x, clipCenter.y, clipCenter.z);

	// Ambient light
	ari.re->GLSL().RingworldLitFloraShader().SetAmbientLight(ambientLight);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
		AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
		AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

		// Configure shader for object space lighting.
		ari.re->GLSL().RingworldLitFloraShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius,
				ShadowPanelSunAngle(sun.m_ShadowAngle),
				sun.m_ShadowCount, sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
	}

	// Display low tessellation of globe.
	m_AtGlobe.m_Low.SelectGL();
	m_AtGlobe.m_Low.DrawGL();
	m_AtGlobe.m_Low.DeselectGL();
	ari.re->GLSL().RingworldLitFloraShader().Deselect();

	// Delete terrain triangle VAOs that get too old so we don't keep too many of them around.
	m_AtGlobe.DeleteOldRingworldSectionTriVAOs(Id(), ari.mm);

//#if !defined(QT_NO_DEBUG)
	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
//#endif
}

void DaWorldRingSection::RenderFlora(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
		const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	Q_UNUSED(projection);
	if(g_Au.m_DebugSwitches & 0x00000200)
		return;

	AuGlobeLLF viewerLL;
	if(!LLOfLoc(viewerLL, viewerPos))
		return;

	Q_ASSERT(m_pFlora);
	if(!m_FloraVAOsHigh)
	{
		// We will create a static array big enough for every possible VAO. We will only create the individual VAOs as they are
		// referenced so we don't do all that grunt work at once.
		int maxFlora = PLANT_VAOS * AuFloraDNA::Zones * (1 + ZONE_TREES + ZONE_SHRUBS) * ResourceZones();
		m_FloraVAOsHigh = new AuGLVAO*[maxFlora];
		AuGLVAO** pvao = m_FloraVAOsHigh;
		for(int flora=0; flora<maxFlora; flora++, pvao++)
			*pvao = 0;
	}

	// Determine wind vectors.
	AuPoint vup = -GravityDir(viewerPos);
	AuPoint windVec[10];
	// Get wind force and direction from planet's weather model or enclosure.
	QPointF wind = WindAt(viewerLL);
	if(wind.x() != 0.f || wind.y() != 0.f)
	{
		AuPoint w(wind.x(), wind.y());
		float windDir = w.ArcTangent_2D();
		if(m_WindDir > 100)
			m_WindDir = windDir;
		else
		{
			float diff = windDir - m_WindDir;
			if(diff < -PIf)
				diff += PIf*2;
			else if(diff > PIf)
				diff -= PIf*2;

			if(diff > DEG2RADf)
				m_WindDir += DEG2RADf/10.f;
			else if(diff < -DEG2RADf)
				m_WindDir -= DEG2RADf/10.f;
			else
				m_WindDir = windDir;
			//qDebug() << "windDir" << (windDir*RAD2DEGf) << "wind" << wind << "diff" << (diff*RAD2DEGf);
		}
		float windForce = w.Length();	// miles per hour
		windForce = sqrtf(windForce / 100);
		if(windForce > 1)
			windForce = 1;
		if(m_WindForce < 0)
			m_WindForce = windForce;
		else
		{
			float diff = windForce - m_WindForce;
			if(diff > .001f)
				m_WindForce += .001f;
			else if(diff < -.001f)
				m_WindForce -= .001f;
			else
				m_WindForce = windForce;
			//qDebug() << "windForce" << windForce << "wind" << wind << "diff" << diff;
		}

		// Calculate wind vector.
		AuPoint vnorth(0,0,1);
		AuPoint veast = vnorth.Cross(vup);
		veast.Unit(m_WindForce);			// Wind vec pointing east.
		AuMatrix4x4 tWind;
		tWind.RotateAxis(m_WindDir, vup);
		AuPoint vwind = tWind.Project(veast);

		// Project wind vector into eye space for the shader.
		vwind = model.ProjectVector(vwind);

		// Now calculate ten individual wind vectors based on the nominal wind.
		float time = AuTime::CurrentMinCycle() * 60.f;	// 0 to 60 seconds.
		float period = 10;		// seconds per cycle
		float periods = time/period;
		for(int iwv=0; iwv<10; iwv++)
		{
			float force = sinf(periods * PIf*2) * 2;
			if(force < 0)
				force /= 2.f;		// Spring back a little weaker.
			windVec[iwv] = vwind * force;
			periods += .1f;			// Offset each wind vector from the next
		}
	}
	else
	{
		for(int iwv=0; iwv<10; iwv++)
			windVec[iwv] = AuPoint(0,0);
	}

	// Place plants in the environment.
	// Calculate location of viewer on the ground.
	AuPoint groundPos = GetSurfaceLocation(viewerLL);
	if(m_FloraViewer.Distance2(groundPos) > 50*FeetPerMeter()*50*FeetPerMeter())
	{
		//qDebug() << "DaWorldRingSection::RenderFlora" << Id() << Name() << "dist" << m_FloraViewer.Distance(groundPos)
			//<< "updating flora...";
		// mm returns plants when it is ready
		if(mm.CreateFlora(this, groundPos, m_Grass, m_Shrubs, m_Trees, m_AquaticGrass, m_AquaticShrubs))
		{
			// Model maker is instructed to delete the plant distribution model. We have what we need. When the viewer moves far
			// enough, we'll ask for a nu plant distribution model. They're too transient to keep around.
			m_FloraViewer = groundPos;							// mm updates viewer pos with pos of actual plant distribution
			mm.DeleteFloraModel(Id());
			//qDebug() << "DaWorldRingSection::RenderFlora" << Id() << Name() << "flora updated";
		}
	}

//#if !defined(QT_NO_DEBUG)
	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_BACK, GL_LINE);
//#endif

	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.GLSL().FloraRingworldShader().Select(projection, re);
	re.GLSL().FloraRingworldShader().SetModelView(model);				// Model view matrix of the globe.
	re.GLSL().FloraRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().FloraRingworldShader().SetAmbientLight(ambientLight, ambientLight, ambientLight);
	AuPoint ambientUp = model.ProjectVector(vup);				// Up vector in eye space for spherical ambient light calc.
	re.GLSL().FloraRingworldShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = re.Sun(sunIndex);
		re.GLSL().FloraRingworldShader().SetSunShadowAngle(sunIndex, ShadowPanelSunAngle(sun.m_ShadowAngle));
	}

	// Configure glow state.
	float glowR, glowG, glowB;
	if(!m_Grass.isEmpty())
	{
		// All grass is the same specie so we only do this once for the grass.
		// We also do not apply city ambient light glow to grass because it's too numerous.
		glowR = m_Grass.first().m_GlowR;
		glowG = m_Grass.first().m_GlowG;
		glowB = m_Grass.first().m_GlowB;
	}
	else
	{
		glowR = 0;
		glowG = 0;
		glowB = 0;
	}
	float plantAlpha = 1.f, curPlantAlpha = 1.f;
	re.GLSL().FloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);

	int resourceZone = -1;
	int windIndex = -1;
	AuGLVAO* curvao = 0;
	double gfadeDist = g_Au.GrassViewRangeFt() * .8;				// Distance at which plants start to become transparent
	double gfadeRange = g_Au.GrassViewRangeFt() - gfadeDist;
	for(QMap<AuResKey, AuGlobeFlora>::const_iterator gi = m_Grass.cbegin(); gi != m_Grass.cend(); gi++)
	{
		double dist2 = (*gi).Location().Distance2(groundPos);
		if(dist2 > g_Au.GrassViewRangeFt2())
			continue;												// Grass is too far away from the viewer.
		if(dist2 > 4*4)
		{
			// Clip when too far behind the viewer.
			AuPoint viewPt = model.Project((*gi).Location());
			if(viewPt.z > 0)
				continue;	// Grass is behind viewer.
		}

		// Calculate alpha based on distance from viewer.
		double dist = sqrt(dist2);
		if(dist > gfadeDist)
			plantAlpha = 1.f - (float)((dist - gfadeDist)/gfadeRange);
		else
			plantAlpha = 1.f;

		// Insure we have the appropriate texture loaded based on resource zone.
		// This MUST be completed before we create the 3D model of the plant.
		if((*gi).m_ResourceZone != resourceZone)
		{
			resourceZone = (*gi).m_ResourceZone;
			if(!m_pFlora[resourceZone].Texture())
			{
				CreateFloraTextures(re, resourceZone);
				if(!m_pFlora[resourceZone].Texture())
					continue;
			}
			re.GLSL().FloraRingworldShader().SetTexture(m_pFlora[resourceZone].Texture());
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curPlantAlpha = plantAlpha;
			re.GLSL().FloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
			if(plantAlpha >= 1.f)
				re.glDisable(GL_BLEND);
			else
				re.glEnable(GL_BLEND);
		}

		// Select a wind vector for the plant.
		if((*gi).m_WindIndex != windIndex)
		{
			windIndex = (*gi).m_WindIndex;
			re.GLSL().FloraRingworldShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
		}

		// Insure we have the specific VAO that is referenced.
		AuGLVAO* vao = m_FloraVAOsHigh[(*gi).m_VAOIndex];
		if(!vao || !vao->VAO())
		{
			// Create the specific VAO model we need.
			if(!vao)
			{
				vao = new AuGLVAO;
				m_FloraVAOsHigh[(*gi).m_VAOIndex] = vao;
			}

			// Create VAO of typical grass.
			AuDice dice;
			dice.Seed(GenSeed() + (*gi).m_VAOIndex);
			m_pFlora[resourceZone].CreateGroundCoverVAO(re, *vao, gi.key().LatitudeRegion(), dice, 2);
		}

		Q_ASSERT(vao->VAO());

		// Multiply by model matrix to place the plant in the view.
		re.GLSL().FloraRingworldShader().SetPlantMatrix((*gi).Transform());

		if(vao != curvao)
		{
			if(curvao)
				curvao->DeselectGL();
			vao->SelectGL();
			curvao = vao;
		}
		vao->DrawGL();
	}

	if(curvao)
	{
		curvao->DeselectGL();
		curvao = 0;
	}

	float ambR = ambientLight;
	float ambG = ambientLight;
	float ambB = ambientLight;

	for(QMap<AuResKey, AuGlobeFlora>::iterator gi = m_Shrubs.begin(); gi != m_Shrubs.end(); gi++)
	{
		// Calculate alpha based on position in the scene.
		AuPoint viewPt = model.Project((*gi).Location());
		float plantAlpha = (*gi).ViewAlpha(33, viewPt, groundPos, g_Au.ShrubViewRangeFt2(), 50);
		if(plantAlpha <= 0.)
			continue;

		// Insure we have the appropriate texture loaded based on resource zone.
		// This MUST be completed before we create the 3D model of the plant.
		if((*gi).m_ResourceZone != resourceZone)
		{
			resourceZone = (*gi).m_ResourceZone;
			if(!m_pFlora[resourceZone].Texture())
			{
				CreateFloraTextures(re, resourceZone);
				if(!m_pFlora[resourceZone].Texture())
					continue;
			}
			re.GLSL().FloraRingworldShader().SetTexture(m_pFlora[resourceZone].Texture());
		}

		// Configure ambient light.
		float iambR = ambientLight + (*gi).m_CityR;
		float iambG = ambientLight + (*gi).m_CityG;
		float iambB = ambientLight + (*gi).m_CityB;
		if(iambR != ambR || iambG != ambG || iambB != ambB)
		{
			ambR = iambR;
			ambG = iambG;
			ambB = iambB;
			re.GLSL().FloraRingworldShader().SetAmbientLight(ambR, ambG, ambB);
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curPlantAlpha = plantAlpha;
			re.GLSL().FloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
			if(plantAlpha >= 1.f)
				re.glDisable(GL_BLEND);
			else
				re.glEnable(GL_BLEND);
		}

		// Select a wind vector for the plant.
		if((*gi).m_WindIndex != windIndex)
		{
			windIndex = (*gi).m_WindIndex;
			re.GLSL().FloraRingworldShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
		}

		// Insure we have the specific VAO that is referenced.
		AuGLVAO* vao = m_FloraVAOsHigh[(*gi).m_VAOIndex];
		if(!vao || !vao->VAO())
		{
			// Create the specific VAO model we need.
			if(!vao)
			{
				vao = new AuGLVAO;
				m_FloraVAOsHigh[(*gi).m_VAOIndex] = vao;
			}

			// Create VAO of typical shrub.
			AuDice dice;
			dice.Seed(GenSeed() + (*gi).m_VAOIndex);
			m_pFlora[resourceZone].CreateShrubVAO(re, *vao, gi.key().LatitudeRegion(), (*gi).m_CID, dice, 2);
		}

		Q_ASSERT(vao->VAO());

		// Multiply by model matrix to place the plant in the view.
		re.GLSL().FloraRingworldShader().SetPlantMatrix((*gi).Transform());

		if(vao != curvao)
		{
			if(curvao)
				curvao->DeselectGL();
			vao->SelectGL();
			curvao = vao;
		}
		vao->DrawGL();
	}

	if(curvao)
	{
		curvao->DeselectGL();
		curvao = 0;
	}

	for(QMap<AuResKey, AuGlobeTree>::iterator gi = m_Trees.begin(); gi != m_Trees.end(); gi++)
	{
		// Calculate alpha based on position in the scene.
		AuPoint viewPt = model.Project((*gi).Location());
		float plantAlpha = (*gi).ViewAlpha(33, viewPt, groundPos, g_Au.TreeViewRangeFt2(), 250);
		if(plantAlpha <= 0.)
			continue;

		// Insure we have the appropriate texture loaded based on resource zone.
		// This MUST be completed before we create the 3D model of the plant.
		if((*gi).m_ResourceZone != resourceZone)
		{
			resourceZone = (*gi).m_ResourceZone;
			if(!m_pFlora[resourceZone].Texture())
			{
				CreateFloraTextures(re, resourceZone);
				if(!m_pFlora[resourceZone].Texture())
					continue;
			}
			re.GLSL().FloraRingworldShader().SetTexture(m_pFlora[resourceZone].Texture());
		}

		// Configure ambient light.
		float iambR = ambientLight + (*gi).m_CityR;
		float iambG = ambientLight + (*gi).m_CityG;
		float iambB = ambientLight + (*gi).m_CityB;
		if(iambR != ambR || iambG != ambG || iambB != ambB)
		{
			ambR = iambR;
			ambG = iambG;
			ambB = iambB;
			re.GLSL().FloraRingworldShader().SetAmbientLight(ambR, ambG, ambB);
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curPlantAlpha = plantAlpha;
			re.GLSL().FloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
			if(plantAlpha >= 1.f)
				re.glDisable(GL_BLEND);
			else
				re.glEnable(GL_BLEND);
		}

		// Select a wind vector for the plant.
		if((*gi).m_WindIndex != windIndex)
		{
			windIndex = (*gi).m_WindIndex;
			re.GLSL().FloraRingworldShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
		}

		// Insure we have the specific VAO that is referenced.
		AuGLVAO* vao = m_FloraVAOsHigh[(*gi).m_VAOIndex];
		if(!vao || !vao->VAO())
		{
			// Create the specific VAO model we need.
			if(!vao)
			{
				vao = new AuGLVAO;
				m_FloraVAOsHigh[(*gi).m_VAOIndex] = vao;
			}

			// Create VAO of typical tree.
			AuDice dice;
			dice.Seed(GenSeed() + (*gi).m_VAOIndex);
			m_pFlora[resourceZone].CreateTreeVAO(re, *vao, gi.key().LatitudeRegion(), (*gi).m_CID, dice, 2);
		}

		Q_ASSERT(vao->VAO());

		// Multiply by model matrix to place the plant in the view.
		re.GLSL().FloraRingworldShader().SetPlantMatrix((*gi).Transform());

		if(vao != curvao)
		{
			if(curvao)
				curvao->DeselectGL();
			vao->SelectGL();
			curvao = vao;
		}
		vao->DrawGL();
	}

	if(curvao)
	{
		curvao->DeselectGL();
		curvao = 0;
	}

	re.GLSL().FloraRingworldShader().Deselect();

	// Aquatic plants
	if(!m_AquaticGrass.empty() || !m_AquaticShrubs.empty())
	{
		// Create water current vectors by scaling down the wind vectors.
		AuPoint waterVec[10];
		for(int iwv=0; iwv<10; iwv++)
			waterVec[iwv] = windVec[iwv]/2;

		re.GLSL().AquaticFloraRingworldShader().Select(projection, re);
		re.GLSL().AquaticFloraRingworldShader().SetModelView(model);				// Model view matrix of the globe.
		re.GLSL().AquaticFloraRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Ambient light
		re.GLSL().AquaticFloraRingworldShader().SetAmbientLight(ambientLight, ambientLight, ambientLight);
		re.GLSL().AquaticFloraRingworldShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = re.Sun(sunIndex);
			re.GLSL().AquaticFloraRingworldShader().SetSunShadowAngle(sunIndex, ShadowPanelSunAngle(sun.m_ShadowAngle));
		}

		float plantAlpha = 1.f, curPlantAlpha = 1.f;
		int resourceZone = -1;
		int windIndex = -1;
		AuGLVAO* curvao = 0;
		double gfadeDist = g_Au.GrassViewRangeFt() * .8;				// Distance at which plants start to become transparent
		double gfadeRange = g_Au.GrassViewRangeFt() - gfadeDist;
		if(!m_AquaticGrass.empty())
		{
			// Configure glow state.
			// All grass is the same specie so we only do this once for the grass.
			// We also do not apply city ambient light glow to grass because it's too numerous.
			glowR = (*m_AquaticGrass.begin()).m_GlowR;
			glowG = (*m_AquaticGrass.begin()).m_GlowG;
			glowB = (*m_AquaticGrass.begin()).m_GlowB;
			re.GLSL().AquaticFloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);

			for(QMap<AuResKey, AuGlobeFlora>::const_iterator gi = m_AquaticGrass.cbegin(); gi != m_AquaticGrass.cend(); gi++)
			{
				double dist2 = (*gi).Location().Distance2(groundPos);
				if(dist2 > g_Au.GrassViewRangeFt2())
					continue;												// Grass is too far away from the viewer.
				if(dist2 > 4*4)
				{
					// Clip when too far behind the viewer.
					AuPoint viewPt = model.Project((*gi).Location());
					if(viewPt.z > 0)
						continue;	// Grass is behind viewer.
				}

				// Calculate alpha based on distance from viewer.
				double dist = sqrt(dist2);
				if(dist > gfadeDist)
					plantAlpha = 1.f - (float)((dist - gfadeDist)/gfadeRange);
				else
					plantAlpha = 1.f;

				// Insure we have the appropriate texture loaded based on resource zone.
				// This MUST be completed before we create the 3D model of the plant.
				if((*gi).m_ResourceZone != resourceZone)
				{
					resourceZone = (*gi).m_ResourceZone;
					if(!m_pFlora[resourceZone].Texture())
					{
						CreateFloraTextures(re, resourceZone);
						if(!m_pFlora[resourceZone].Texture())
							continue;
					}
					re.GLSL().AquaticFloraRingworldShader().SetTexture(m_pFlora[resourceZone].Texture());
				}

				// Configure glow.
				if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
				{
					glowR = (*gi).m_GlowR;
					glowG = (*gi).m_GlowG;
					glowB = (*gi).m_GlowB;
					curPlantAlpha = plantAlpha;
					re.GLSL().AquaticFloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
					if(plantAlpha >= 1.f)
						re.glDisable(GL_BLEND);
					else
						re.glEnable(GL_BLEND);
				}

				// Select a wind vector for the plant.
				if((*gi).m_WindIndex != windIndex)
				{
					windIndex = (*gi).m_WindIndex;
					re.GLSL().AquaticFloraRingworldShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
				}

				// Insure we have the specific VAO that is referenced.
				AuGLVAO* vao = m_FloraVAOsHigh[(*gi).m_VAOIndex];
				if(!vao || !vao->VAO())
				{
					// Create the specific VAO model we need.
					if(!vao)
					{
						vao = new AuGLVAO;
						m_FloraVAOsHigh[(*gi).m_VAOIndex] = vao;
					}

					// Create VAO of typical grass.
					AuDice dice;
					dice.Seed(GenSeed() + (*gi).m_VAOIndex);
					m_pFlora[resourceZone].CreateGroundCoverVAO(re, *vao, gi.key().LatitudeRegion(), dice, 2);
				}

				Q_ASSERT(vao->VAO());

				// Multiply by model matrix to place the plant in the view.
				re.GLSL().AquaticFloraRingworldShader().SetPlantMatrix((*gi).Transform());

				if(vao != curvao)
				{
					if(curvao)
						curvao->DeselectGL();
					vao->SelectGL();
					curvao = vao;
				}
				vao->DrawGL();
			}

			if(curvao)
			{
				curvao->DeselectGL();
				curvao = 0;
			}
		}
		else
		{
			glowR = 0;
			glowG = 0;
			glowB = 0;
			re.GLSL().AquaticFloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
		}

		if(!m_AquaticShrubs.empty())
		{
			float ambR = ambientLight;
			float ambG = ambientLight;
			float ambB = ambientLight;

			for(QMap<AuResKey, AuGlobeFlora>::iterator gi = m_AquaticShrubs.begin(); gi != m_AquaticShrubs.end(); gi++)
			{
				// Calculate alpha based on position in the scene.
				AuPoint viewPt = model.Project((*gi).Location());
				float plantAlpha = (*gi).ViewAlpha(33, viewPt, groundPos, g_Au.ShrubViewRangeFt2(), 50);
				if(plantAlpha <= 0.)
					continue;

				// Insure we have the appropriate texture loaded based on resource zone.
				// This MUST be completed before we create the 3D model of the plant.
				if((*gi).m_ResourceZone != resourceZone)
				{
					resourceZone = (*gi).m_ResourceZone;
					if(!m_pFlora[resourceZone].Texture())
					{
						CreateFloraTextures(re, resourceZone);
						if(!m_pFlora[resourceZone].Texture())
							continue;
					}
					re.GLSL().AquaticFloraRingworldShader().SetTexture(m_pFlora[resourceZone].Texture());
				}

				// Configure ambient light.
				float iambR = ambientLight + (*gi).m_CityR;
				float iambG = ambientLight + (*gi).m_CityG;
				float iambB = ambientLight + (*gi).m_CityB;
				if(iambR != ambR || iambG != ambG || iambB != ambB)
				{
					ambR = iambR;
					ambG = iambG;
					ambB = iambB;
					re.GLSL().AquaticFloraRingworldShader().SetAmbientLight(ambR, ambG, ambB);
				}

				// Configure glow.
				if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
				{
					glowR = (*gi).m_GlowR;
					glowG = (*gi).m_GlowG;
					glowB = (*gi).m_GlowB;
					curPlantAlpha = plantAlpha;
					re.GLSL().AquaticFloraRingworldShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
					if(plantAlpha >= 1.f)
						re.glDisable(GL_BLEND);
					else
						re.glEnable(GL_BLEND);
				}

				// Select a wind vector for the plant.
				if((*gi).m_WindIndex != windIndex)
				{
					windIndex = (*gi).m_WindIndex;
					re.GLSL().AquaticFloraRingworldShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
				}

				// Insure we have the specific VAO that is referenced.
				AuGLVAO* vao = m_FloraVAOsHigh[(*gi).m_VAOIndex];
				if(!vao || !vao->VAO())
				{
					// Create the specific VAO model we need.
					if(!vao)
					{
						vao = new AuGLVAO;
						m_FloraVAOsHigh[(*gi).m_VAOIndex] = vao;
					}

					// Create VAO of typical shrub.
					AuDice dice;
					dice.Seed(GenSeed() + (*gi).m_VAOIndex);
					m_pFlora[resourceZone].CreateShrubVAO(re, *vao, gi.key().LatitudeRegion(), (*gi).m_CID, dice, 2);
				}

				Q_ASSERT(vao->VAO());

				// Multiply by model matrix to place the plant in the view.
				re.GLSL().AquaticFloraRingworldShader().SetPlantMatrix((*gi).Transform());

				if(vao != curvao)
				{
					if(curvao)
						curvao->DeselectGL();
					vao->SelectGL();
					curvao = vao;
				}
				vao->DrawGL();
			}

			if(curvao)
			{
				curvao->DeselectGL();
				curvao = 0;
			}
		}

		// There are no aquatic trees.
		re.GLSL().AquaticFloraRingworldShader().Deselect();
	}

	re.glDisable(GL_BLEND);
//#if !defined(QT_NO_DEBUG)
	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
//#endif
}

void DaWorldRingSection::RenderRocks(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
		const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	Q_UNUSED(projection);
	if(!m_RockVAOs)
	{
		// We will create a static array big enough for every possible VAO. We will only create the individual VAOs as they are
		// referenced so we don't do all that grunt work at once.
		int maxRocks = DpRock::RockTypes();
		m_RockVAOs = new AuGLVAO*[maxRocks];
		AuGLVAO** pvao = m_RockVAOs;
		for(int rock=0; rock<maxRocks; rock++, pvao++)
			*pvao = 0;
	}

	// Place rocks in the environment.
	// Calculate location of viewer on the ground.
	AuGlobeLL viewerLL;
	if(!LLOfLoc(viewerLL, viewerPos))
		return;

	AuPoint vup = -GravityDir(viewerPos);
	AuPoint groundPos = GetSurfaceLocation(viewerLL);
	if(m_RockViewer.Distance2(groundPos) > 50*FeetPerMeter()*50*FeetPerMeter())
	{
		//qDebug() << "DaWorldRingSection::RenderRocks" << Id() << Name() << "dist" << m_RockViewer.Distance(groundPos)
			//<< "updating rocks...";
		if(mm.CreateRocks(this, groundPos, m_Rocks))
		{
			// Model maker is instructed to delete the rock distribution model. We have what we need. When the viewer moves far
			// enough, we'll ask for a nu rock distribution model. They're too transient to keep around.
			m_RockViewer = groundPos;
			mm.DeleteRockModel(Id());
			//qDebug() << "DaWorldRingSection::RenderRocks" << Id() << Name() << "updated" << m_Rocks.count() << "rocks";
		}
	}

//#if !defined(QT_NO_DEBUG)
	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_BACK, GL_LINE);
//#endif

	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.GLSL().RockRingworldShader().Select(projection, re);
	re.GLSL().RockRingworldShader().SetModelView(model);				// Model view matrix of the globe.
	re.GLSL().RockRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);
	re.GLSL().RockRingworldShader().SetTexture(s_TexRock);

	// Ambient light
	re.GLSL().RockRingworldShader().SetAmbientLight(ambientLight, ambientLight, ambientLight);
	AuPoint ambientUp = model.ProjectVector(vup);				// Up vector in eye space for spherical ambient light calc.
	re.GLSL().RockRingworldShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = re.Sun(sunIndex);
		re.GLSL().RockRingworldShader().SetSunShadowAngle(sunIndex, ShadowPanelSunAngle(sun.m_ShadowAngle));
	}

	// Configure glow state.
	float glowR=0, glowG=0, glowB=0;
	float rockAlpha = 1.f, curRockAlpha = 1.f;
	re.GLSL().RockRingworldShader().SetGlowAlpha(glowR, glowG, glowB, rockAlpha);

	float ambR = ambientLight;
	float ambG = ambientLight;
	float ambB = ambientLight;

	AuGLVAO* curvao = 0;
	for(QMap<AuResKey, AuGlobeRock>::iterator gi = m_Rocks.begin(); gi != m_Rocks.end(); gi++)
	{
		// Calculate alpha based on position in the scene.
		AuPoint viewPt = model.Project((*gi).Location());
		float rockAlpha = (*gi).ViewAlpha(33, viewPt, groundPos, g_Au.RockViewRangeFt2(), 20);
		if(rockAlpha <= 0.)
			continue;

		// Configure ambient light.
		float iambR = ambientLight + (*gi).m_CityR;
		float iambG = ambientLight + (*gi).m_CityG;
		float iambB = ambientLight + (*gi).m_CityB;
		if(iambR != ambR || iambG != ambG || iambB != ambB)
		{
			ambR = iambR;
			ambG = iambG;
			ambB = iambB;
			re.GLSL().RockRingworldShader().SetAmbientLight(ambR, ambG, ambB);
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || rockAlpha != curRockAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curRockAlpha = rockAlpha;
			re.GLSL().RockRingworldShader().SetGlowAlpha(glowR, glowG, glowB, rockAlpha);
			if(rockAlpha >= 1.f)
				re.glDisable(GL_BLEND);
			else
				re.glEnable(GL_BLEND);
		}

		// Insure we have the specific VAO that is referenced.
		AuGLVAO* vao = m_RockVAOs[(*gi).m_VAOIndex];
		if(!vao)
		{
			// Create a vao.
			vao = new AuGLVAO;
			m_RockVAOs[(*gi).m_VAOIndex] = vao;
		}

		if(!vao->VAO())
		{
			// Create the specific VAO model we need.
			(*gi).CreateVAO(re, *vao);
		}

		if(vao->VAO())
		{
			// Multiply by model matrix to place the rock in the view.
			re.GLSL().RockRingworldShader().SetRockMatrix((*gi).Transform());

			if(vao != curvao)
			{
				if(curvao)
					curvao->DeselectGL();
				vao->SelectGL();
				curvao = vao;
			}
			vao->DrawGL();
		}
	}

	if(curvao)
	{
		curvao->DeselectGL();
		curvao = 0;
	}

	re.GLSL().RockRingworldShader().Deselect();

	re.glDisable(GL_BLEND);
//#if !defined(QT_NO_DEBUG)
	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
//#endif
}

void DaWorldRingSection::RenderHenges(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		float ambientR, float ambientG, float ambientB, float fogDensity, float fogR, float fogG, float fogB)
{
	// Check to see if we have any henges before doing this work.
	if(!HasHenge())
		return;

	if(!m_HengeColumn)
	{
		// Start loading the 3ds model using a worker thread.
		QString modelFile = ":data/mHengeColumn.3ds";
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		m_HengeColumn = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
	}
	else if(!m_HengeColumn->HasGLModel())
		m_HengeColumn->GLModel(*ari.re);				// GL display list isn't done yet. Attempt to create the GL display list.

	if(!m_HengeFirePit)
	{
		// Start loading the 3ds model using a worker thread.
		QString modelFile = ":data/mHengeFirePit.3ds";
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		m_HengeFirePit = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
	}
	else if(!m_HengeFirePit->HasGLModel())
		m_HengeFirePit->GLModel(*ari.re);				// GL display list isn't done yet. Attempt to create the GL display list.

	if(!m_HengeRock)
	{
		// Start loading the 3ds model using a worker thread.
		QString modelFile = ":data/mHengeRock.3ds";
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		m_HengeRock = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
	}
	else if(!m_HengeRock->HasGLModel())
		m_HengeRock->GLModel(*ari.re);					// GL display list isn't done yet. Attempt to create the GL display list.

	float shadowPanelSunAngles[MAX_SUNS];
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		shadowPanelSunAngles[sunIndex] = ShadowPanelSunAngle(sun.m_ShadowAngle);
	}

	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		AuPoint hengeCenterLoc = LocOfLL(henge->LL(), henge->CentralSlabAltitudeMSL());
		AuPoint hpx = model.Project(hengeCenterLoc);	// Henge location in eye space.
		if(hpx.Length2() > 1000*FeetPerMeter() * 1000*FeetPerMeter())
			continue;					// We don't need to see henges too far away.

		// Create a genseed so each henge on the same planet is not identical.
		quint32 genSeed = GenSeed() + hengeCenterLoc.x + hengeCenterLoc.y + hengeCenterLoc.z;
		AuDice dice;
		dice.Seed(genSeed);

		AuPoint hengeUpVec = -GravityDir(hengeCenterLoc);
		AuMatrix4x4 hengeM(model);
		hengeM.Translate(hengeCenterLoc);
		hengeM *= AuPoint(0,0,1).GetAlignmentTransform(hengeUpVec);

		// Draw the fire pit at the center.
		AuPoint ambientUp = model.ProjectVector(hengeUpVec);
		AuPoint px = hengeM.Translation();
		if(px.z < 10)
			m_HengeFirePit->RenderGLSLModelTexFogRingworld(projection, hengeM, *ari.re, shadowPanelSunAngles, ambientR, ambientG,
					ambientB, ambientUp, false, fogDensity, fogR, fogG, fogB);

		int rockCount = henge->CentralSlabCount();
		double rockRad = henge->CentralSlabRadius();
		int colCount = henge->OuterRingColumnCount();
		double colRad = henge->OuterRingRadius();

		// Draw a ring of central slab stones.
		for(int rock=0; rock<rockCount; rock++)
		{
			double ang = rock * PI*2 / rockCount;
			double x = cos(ang) * rockRad;
			double y = sin(ang) * rockRad;

			AuMatrix4x4 rockM(hengeM);
			int orient = dice.Roll(1,100);
			if(orient <= 15)
			{
				// Fallen
				rockM.Translate(x, y, -dice.Roll(1,40,20)/10.);	// Sunken
				rockM.RotateZ(ang);
				rockM.RotateY(dice.Roll(1,150,-75)*DEG2RAD);
				rockM.RotateX(dice.Roll(1,150,-75)*DEG2RAD);
			}
			else if(orient <= (100-15)/2)
			{
				// Flipped vertically and properly aligned.
				rockM.Translate(x, y, 13 - .5);
				rockM.RotateY(PI);
				rockM.RotateZ(-ang);
			}
			else
			{
				// Properly aligned.
				rockM.Translate(x, y, 0 - .5);
				rockM.RotateZ(ang);
			}

			// Skip rocks behind the viewer.
			AuPoint px = rockM.Translation();
			if(px.z > 0)
				continue;

			m_HengeRock->RenderGLSLModelTexFogRingworld(projection, rockM, *ari.re, shadowPanelSunAngles, ambientR, ambientG,
						ambientB, ambientUp, false, fogDensity, fogR, fogG, fogB);
		}

		// Draw a ring of outer columns with lintels.
		double cola = PI*2/colCount;
		for(int col=0; col<colCount; col++)
		{
			double ang = cola * col;
			double x = cos(ang) * colRad;
			double y = sin(ang) * colRad;

			AuMatrix4x4 colM(hengeM);
			if(dice.Roll(1,4) <= 3)
			{
				// Lintel present.
				colM.Translate(x, y, 0);
				colM.RotateZ(ang + PI/2 + cola/2);
			}
			else
			{
				// Lintel missing, hidden under ground.
				colM.Translate(x, y, 8);
				colM.RotateY(PI);
				colM.RotateZ(-(ang - PI/2) - cola/2);
			}

			// Skip columns behind the viewer.
			AuPoint px = colM.Translation();
			if(px.z > 0)
				continue;

			m_HengeColumn->RenderGLSLModelTexFogRingworld(projection, colM, *ari.re, shadowPanelSunAngles, ambientR, ambientG,
						ambientB, ambientUp, false, fogDensity, fogR, fogG, fogB);
		}
	}
	UnlockGeologicalFeatures();
}

void DaWorldRingSection::CreateFlora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs,
		QMap<AuResKey, AuGlobeTree>& trees, QMap<AuResKey, AuGlobeFlora>& aquaticgrass, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs,
		const AuPoint& groundLoc, quint32 worldGenSeed, int floraGlow) const
{
	AuPoint vup = -GravityDir(groundLoc);

	AuPoint vx;
	vup.ArbitraryAxis(vx);

	AuPoint vy = vup.Cross(vx);
	vy.Unit();

	CreateGrass(grass, aquaticgrass, groundLoc, vx, vy, worldGenSeed, floraGlow);
	CreateShrubs(shrubs, aquaticshrubs, groundLoc, vx, vy, worldGenSeed, floraGlow);
	CreateTrees(trees, groundLoc, vx, vy, worldGenSeed, floraGlow);
}

void DaWorldRingSection::CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, quint32 worldGenSeed,
		float range, bool visibleOnly) const
{
	//qDebug() << "DaWorldRingSection::CreateRocks" << Id() << Name();
	AuPoint vup = -GravityDir(groundLoc);

	AuPoint vx;
	vup.ArbitraryAxis(vx);

	AuPoint vy = vup.Cross(vx);
	vy.Unit();

	CreateRocks(rocks, groundLoc, vx, vy, worldGenSeed, range, visibleOnly);
	//qDebug() << "DaWorldRingSection::CreateRocks" << Id() << Name() << "done";
}

void DaWorldRingSection::CreateGrass(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& aquaticgrass,
		const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const
{
	const float userDensity = g_Au.GroundCoverQty() / 5000.f;
	//qDebug() << "DaWorldRingSection::CreateGrass userDensity" << userDensity;
	if(userDensity == 0)
		return;

	int eqDivs = (int)(TerrainXSize() / 3);						// 3' spacing for grass at highest density.

	const int range = g_Au.GrassViewRangeFt() + 50*FeetPerMeter();
	const float rangeOct = range * 1.414f;

	AuGlobeFlora plant;
	plant.m_CID = CID_Invalid;									// Not needed for grass.
	plant.m_FadeAlpha = 0.f;
	QMap<AuGlobeLL, bool> alreadyChecked;
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	for(int ix=-range; ix<range; ix+=2)
	{
		AuPoint vxv = vx*ix;
		for(int iy=-range; iy<range; iy+=2)
		{
			if(abs(ix) + abs(iy) > rangeOct)
				continue;				// Octagon shaped area is close enough to a circle without the math.

			// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
			// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
			AuPoint plantLoc = groundLoc + vxv + vy*iy;
			AuGlobeLL plantLL;
			if(!LLOfLoc(plantLL, plantLoc))
				continue;

			plantLL = RoundedGlobeLoc(plantLL, eqDivs);	// As if tessellated to 2' increments at equator.
			if(alreadyChecked.contains(plantLL))
				continue;
			alreadyChecked.insert(plantLL, true);

			// Calculate location at ground level. This also determines if the location falls within a development site that hides
			// certain resources.
			bool aquaticPlant = false;
			bool hideRocks, hideShrubs, hideTrees;
			int hideGrass;
			WORLDTEX_TYPE pavementType;
			COMMODITYID farmShrub, orchardTree;
			double farmShrubMSL, orchardTreeMSL;
			double altMSL = TerrainAltitudeMSLPave(plantLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub,
					farmShrubMSL, orchardTree, orchardTreeMSL);
			if(hideGrass == 1)
				continue;				// No grass at all.
			else if(hideGrass != 3)
			{
				// Test plant density at plantLL.
				// Flora density bands can be seen in the grass texture on the ground, which has 8 intensities.
				// 0 = Desert
				// 1 = Grasslands
				// 2 = Shrublands
				// 3 = Light forest and shrubs
				// 4-7 = Increasing forest, decreasing shrubs.
				float vegPercent;	// veg density present in the range of 0 to 1
				double plantAltMSL = TerrainAltitudeMSL(plantLL);
				int irrigated = IsIrrigated(plantLL);
				if(HasEnclosure())
				{
					int inEnclosure = InEnclosure(plantLL, plantAltMSL);
					if(inEnclosure < 0)
						inEnclosure = 0;
					if(plantAltMSL < 0 && inEnclosure != 2)
						aquaticPlant = true;	// Undersea enclosures have no water.
					else
						aquaticPlant = false;
					vegPercent = VegetationAt(plantLL, plantAltMSL, inEnclosure, irrigated)/100.f;
				}
				else
				{
					if(plantAltMSL < 0)
						aquaticPlant = true;
					else
						aquaticPlant = false;
					vegPercent = VegetationAt(plantLL, plantAltMSL, false, irrigated)/100.f;
				}
				if(vegPercent < 1.f/8.f)
					continue;										// No grass in desert.

				// Grass is most abundant where there is no vegetation and least abundant in the midst of heavy vegetation.
				float grassPercent = 1.f - vegPercent + .2f;
				grassPercent *= userDensity;						// Modulate with user density setting.

				// Generate a coherent noise factor for a clumpy distribution density. Range is -1 to 1.
				float densityN = AuNoiseF::CoherentNoise(randF, ceilf(plantLL.Longitude()*10000), ceilf(plantLL.Latitude()*10000));
				if(fabs(densityN) >= grassPercent)
					continue;
			}
			plantLoc = LocOfLL(plantLL, altMSL);

			// Generate a random noise factor for rotation and sub vao index. Range is -1 to 1
			float intN = AuNoiseF::IntegerNoise(randF, ceilf(plantLL.Longitude()*10000), ceilf(plantLL.Latitude()*10000));

			AuFloraDNA::ZONE latRegion = LatitudeRegion(plantLL.Latitude());
			plant.m_ResourceZone = ResourceZone(plantLL.Longitude());
			QColor glowColor = m_pFlora[plant.m_ResourceZone].GroundCover(latRegion).DNA().Glow(floraGlow);
			plant.m_GlowR = glowColor.redF();
			plant.m_GlowG = glowColor.greenF();
			plant.m_GlowB = glowColor.blueF();
			float rotZ = intN * PIf;
			float scale;
			if(hideGrass == 2)
				scale = .5f + intN/20.f;	// Shorten the grass to a uniform shortened length; don't remove it entirely.
			else if(hideGrass == 3)
				scale = .25f + intN/20.f;	// Shorten the grass to a uniform mowed length; don't remove it entirely.
			else
				scale = 1 + intN/5.f;
			plant.SetPosition(plantLoc, -GravityDir(plantLoc), rotZ, 1, 1, scale);
			int plantVAOIndex = (int)((intN + 1.f) * PLANT_VAOS) % PLANT_VAOS;		// Range is 0 to (PLANT_VAOS-1)
			plant.m_VAOIndex = FloraVAOIndex(plant.m_ResourceZone, latRegion, 0, plantVAOIndex);
			plant.m_WindIndex = (int)((intN + 1.f) * 10) % 10;	// Range is 0 to 9

			if(aquaticPlant)
				aquaticgrass.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex), plant);
			else
				grass.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex), plant);
		}
	}
	//qDebug() << "DaWorldRingSection::CreateGrass" << grass.count();
}

void DaWorldRingSection::CreateShrubs(QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs,
		const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const
{
	int eqDivs = (int)TerrainXSize() / 12;						// 12' spacing for shrubs at highest density.

	const int range = g_Au.ShrubViewRangeFt() + 100*FeetPerMeter();
	const float rangeOct = range * 1.414f;

	AuGlobeFlora plant;
	plant.m_FadeAlpha = 0.f;
	QMap<AuGlobeLL, bool> alreadyChecked;
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	for(int ix=-range; ix<range; ix+=6)
	{
		AuPoint vxv = vx*ix;
		for(int iy=-range; iy<range; iy+=6)
		{
			if(abs(ix) + abs(iy) > rangeOct)
				continue;				// Octagon shaped area is close enough to a circle without the math.

			// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
			// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
			AuPoint plantLoc = groundLoc + vxv + vy*iy;
			AuGlobeLL plantLL;
			if(!LLOfLoc(plantLL, plantLoc))
				continue;

			plantLL = RoundedGlobeLoc(plantLL, eqDivs);	// As if tessellated to 12' increments at equator.
			if(alreadyChecked.contains(plantLL))
				continue;
			alreadyChecked.insert(plantLL, true);

			// Calculate location at ground level. This also determines if the location falls within a development site that hides
			// certain resources or cultivates certain plants.
			bool hideRocks, hideShrubs, hideTrees;
			int hideGrass;
			WORLDTEX_TYPE pavementType;
			COMMODITYID farmShrub, orchardTree;
			double farmShrubMSL, orchardTreeMSL;
			double plantMSL = TerrainAltitudeMSLPave(plantLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub,
					farmShrubMSL, orchardTree, orchardTreeMSL);
			if(hideShrubs && farmShrub == CID_Invalid)
				continue;
			if(farmShrubMSL != 0)
				plantMSL = farmShrubMSL;	// Farm specified msl altitude of the shrub, must be on a platform.
			plantLoc = LocOfLL(plantLL, plantMSL);

			plant.m_CID = farmShrub;
			bool aquaticPlant = false;
			if(plant.m_CID == CID_Invalid)
			{
				// No plant is cultivated here.
				// Check to see if there is a natural growing shrub at that grid location.
				plant.m_CID = SpotShrubAt(plantLL, worldGenSeed, aquaticPlant);
				if(plant.m_CID == CID_Invalid)
					continue;
			}

			// Determine city ambient light that falls on this thing.
			float cityAmbient = CityAmbientLight(plantLL.ToLLF(), pavementType);
			const QColor& c = AuGLLight::ColorSodiumVapor();
			plant.m_CityR = cityAmbient * c.redF() * (float)AuGLLight::AmbientLitCity();
			plant.m_CityG = cityAmbient * c.greenF() * (float)AuGLLight::AmbientLitCity();
			plant.m_CityB = cityAmbient * c.blueF() * (float)AuGLLight::AmbientLitCity();

			// Generate a random noise factor for rotation and sub vao index. Range is -1 to 1
			float intN = AuNoiseF::IntegerNoise(randF, ceilf(plantLL.Longitude()*10000), ceilf(plantLL.Latitude()*10000));

			int plantType = 1 + DeTerrainFlora::ShrubIndex(plant.m_CID);			// Grass is first (type 0).
			AuFloraDNA::ZONE latRegion = LatitudeRegion(plantLL.Latitude());
			plant.m_ResourceZone = ResourceZone(plantLL.Longitude());
			QColor glowColor = m_pFlora[plant.m_ResourceZone].Shrub(latRegion, plant.m_CID).DNA().Glow(floraGlow);
			plant.m_GlowR = glowColor.redF();
			plant.m_GlowG = glowColor.greenF();
			plant.m_GlowB = glowColor.blueF();
			float rotZ = intN * PIf;
			float scale;
			if(farmShrub != CID_Invalid)
				scale = .75f;
			else
				scale = 1.3f + intN/2.f;
			plant.SetPosition(plantLoc, -GravityDir(plantLoc), rotZ, scale, scale, scale);
			int plantVAOIndex = (int)((intN + 1.f) * PLANT_VAOS) % PLANT_VAOS;		// Range is 0 to (PLANT_VAOS-1)
			plant.m_VAOIndex = FloraVAOIndex(plant.m_ResourceZone, latRegion, plantType, plantVAOIndex);
			plant.m_WindIndex = (int)((intN + 1.f) * 10) % 10;	// Range is 0 to 9

			if(aquaticPlant)
				aquaticshrubs.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex),plant);
			else
				shrubs.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex), plant);
		}
	}
	//qDebug() << "DaWorldRingSection::CreateShrubs" << shrubs.count();
}

void DaWorldRingSection::CreateTrees(QMap<AuResKey, AuGlobeTree>& trees, const AuPoint& groundLoc, const AuPoint& vx,
		const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const
{
	int eqDivs = (int)(TerrainXSize() / 24);					// 24' spacing for trees at highest density.

	const int range = g_Au.TreeViewRangeFt() + 100*FeetPerMeter();
	const float rangeOct = range * 1.414f;

	AuGlobeTree plant;
	plant.m_FadeAlpha = 0.f;
	QMap<AuGlobeLL, bool> alreadyChecked;
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	for(int ix=-range; ix<range; ix+=12)
	{
		AuPoint vxv = vx*ix;
		for(int iy=-range; iy<range; iy+=12)
		{
			if(abs(ix) + abs(iy) > rangeOct)
				continue;				// Octagon shaped area is close enough to a circle without the math.

			// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
			// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
			AuPoint plantLoc = groundLoc + vxv + vy*iy;
			AuGlobeLL plantLL;
			if(!LLOfLoc(plantLL, plantLoc))
				continue;

			plantLL = RoundedGlobeLoc(plantLL, eqDivs);	// As if tessellated to 24' increments at equator.
			if(alreadyChecked.contains(plantLL))
				continue;
			alreadyChecked.insert(plantLL, true);

			// Calculate location at ground level. This also determines if the location falls within a development site that hides
			// certain resources or cultivates certain plants.
			bool hideRocks, hideShrubs, hideTrees;
			int hideGrass;
			WORLDTEX_TYPE pavementType;
			COMMODITYID farmShrub, orchardTree;
			double farmShrubMSL, orchardTreeMSL;
			double plantMSL = TerrainAltitudeMSLPave(plantLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub,
					farmShrubMSL, orchardTree, orchardTreeMSL);
			if(hideTrees && orchardTree == CID_Invalid)
				continue;
			if(orchardTreeMSL != 0)
				plantMSL = orchardTreeMSL;	// Orchard specified msl altitude of the tree, must be on a platform.
			plantLoc = LocOfLL(plantLL, plantMSL);

			plant.m_CID = orchardTree;
			if(plant.m_CID == CID_Invalid)
			{
				// No plant is cultivated here.
				// Check to see if there is a natural growing tree at that grid location.
				plant.m_CID = SpotTreeAt(plantLL, worldGenSeed);
				if(plant.m_CID == CID_Invalid)
					continue;
			}

			// Determine city ambient light that falls on this thing.
			float cityAmbient = CityAmbientLight(plantLL.ToLLF(), pavementType);
			const QColor& c = AuGLLight::ColorSodiumVapor();
			plant.m_CityR = cityAmbient * c.redF() * (float)AuGLLight::AmbientLitCity();
			plant.m_CityG = cityAmbient * c.greenF() * (float)AuGLLight::AmbientLitCity();
			plant.m_CityB = cityAmbient * c.blueF() * (float)AuGLLight::AmbientLitCity();

			// Generate a random noise factor for rotation and sub vao index. Range is -1 to 1
			float intN = AuNoiseF::IntegerNoise(randF, ceilf(plantLL.Longitude()*10000), ceilf(plantLL.Latitude()*10000));

			int plantType = 1 + ZONE_SHRUBS + DeTerrainFlora::TreeIndex(plant.m_CID);	// Grass and shrubs are first.
			AuFloraDNA::ZONE latRegion = LatitudeRegion(plantLL.Latitude());
			plant.m_ResourceZone = ResourceZone(plantLL.Longitude());
			QColor glowColor = m_pFlora[plant.m_ResourceZone].Tree(latRegion, plant.m_CID).DNA().Glow(floraGlow);
			plant.m_GlowR = glowColor.redF();
			plant.m_GlowG = glowColor.greenF();
			plant.m_GlowB = glowColor.blueF();
			float rotZ = intN * PIf;
			float scale;
			if(orchardTree != CID_Invalid)
				scale = .75f;
			else
				scale = 1.2f + intN/5.f;
			plant.SetPosition(plantLoc, -GravityDir(plantLoc), rotZ, scale, scale, scale);
			QueryTreeTrunkSize(plant.CommodityId(), plant.m_ResourceZone, latRegion, plant.m_TrunkLength, plant.m_TrunkRadius);
			int plantVAOIndex = (int)((intN + 1.f) * PLANT_VAOS) % PLANT_VAOS;		// Range is 0 to (PLANT_VAOS-1)
			plant.m_VAOIndex = FloraVAOIndex(plant.m_ResourceZone, latRegion, plantType, plantVAOIndex);
			plant.m_WindIndex = (int)((intN + 1.f) * 10) % 10;	// Range is 0 to 9

			trees.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex), plant);
		}
	}
	//qDebug() << "DaWorldRingSection::CreateTrees" << trees.count();
}

void DaWorldRingSection::CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, const AuPoint& vx,
		const AuPoint& vy, quint32 worldGenSeed, float range, bool visibleOnly) const
{
	int eqDivs = (int)(TerrainXSize() / 20);					// 20' spacing for rocks at highest density.

	const float rangeOct = range * 1.414f;

	AuGlobeRock rock;
	rock.m_FadeAlpha = 0.f;
	QMap<AuGlobeLL, bool> alreadyChecked;
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	for(int ix=-range; ix<range; ix+=10)
	{
		AuPoint vxv = vx*ix;
		for(int iy=-range; iy<range; iy+=10)
		{
			if(abs(ix) + abs(iy) > rangeOct)
				continue;				// Octagon shaped area is close enough to a circle without the math.

			// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
			// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
			AuPoint rockLoc = groundLoc + vxv + vy*iy;
			AuGlobeLL rockLL;
			if(!LLOfLoc(rockLL, rockLoc))
				continue;

			rockLL = RoundedGlobeLoc(rockLL, eqDivs);	// As if tessellated to 20' increments at equator.
			if(alreadyChecked.contains(rockLL))
				continue;
			alreadyChecked.insert(rockLL, true);

			// Check to see if there is a rock at that grid location.
			rock.m_CID = SpotRockAt(rockLL, worldGenSeed);
			if(rock.m_CID == CID_Invalid)
				continue;

			// Calculate location at ground level. This also determines if the location falls within a development site that hides
			// certain resources.
			bool hideRocks, hideShrubs, hideTrees;
			int hideGrass;
			WORLDTEX_TYPE pavementType;
			COMMODITYID farmShrub, orchardTree;
			double farmShrubMSL, orchardTreeMSL;
			double altMSL = TerrainAltitudeMSLPave(rockLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub,
					farmShrubMSL, orchardTree, orchardTreeMSL);
			if(hideRocks && visibleOnly)
				continue;
			rockLoc = LocOfLL(rockLL, altMSL);

			// Generate random noise factors for rotation and scale. Range is -1 to 1
			float intNx = AuNoiseF::IntegerNoise(randF, ceilf(rockLL.Longitude()*10000), ceilf(rockLL.Latitude()*10000));
			float intNy = AuNoiseF::IntegerNoise(randF, ceilf(rockLL.Longitude()*10000+1000), ceilf(rockLL.Latitude()*10000+1000));
			float intNz = AuNoiseF::IntegerNoise(randF, ceilf(rockLL.Longitude()*10000+2000), ceilf(rockLL.Latitude()*10000+2000));
			float intNs = AuNoiseF::IntegerNoise(randF, ceilf(rockLL.Longitude()*10000+3000), ceilf(rockLL.Latitude()*10000+3000));
			float intNt = AuNoiseF::IntegerNoise(randF, ceilf(rockLL.Longitude()*10000+4000), ceilf(rockLL.Latitude()*10000+4000));
			float intNu = AuNoiseF::IntegerNoise(randF, ceilf(rockLL.Longitude()*10000+5000), ceilf(rockLL.Latitude()*10000+5000));

			// Some rocks glow.
			switch(rock.m_CID)
			{
			default:
				rock.m_GlowR = 0;
				rock.m_GlowG = 0;
				rock.m_GlowB = 0;
				break;
			case CID_Lumenite:
				rock.m_GlowR = .7f;
				rock.m_GlowG = .7f;
				rock.m_GlowB = 1;
				break;
			case CID_Phlogiston:
				rock.m_GlowR = .96f;
				rock.m_GlowG = .34f;
				rock.m_GlowB = 1;
				break;
			case CID_Radioactives:
				rock.m_GlowR = .05f;
				rock.m_GlowG = .3f;
				rock.m_GlowB = .05f;
				break;
			case CID_Adamantite:
				rock.m_GlowR = .5f;
				rock.m_GlowG = .05f;
				rock.m_GlowB = .05f;
				break;
			}

			// Determine city ambient light that falls on this thing.
			float cityAmbient = CityAmbientLight(rockLL.ToLLF(), pavementType);
			const QColor& c = AuGLLight::ColorSodiumVapor();
			rock.m_CityR = cityAmbient * c.redF() * (float)AuGLLight::AmbientLitCity();
			rock.m_CityG = cityAmbient * c.greenF() * (float)AuGLLight::AmbientLitCity();
			rock.m_CityB = cityAmbient * c.blueF() * (float)AuGLLight::AmbientLitCity();

			// Some rocks have scaling factors.
			float scaleX, scaleY, scaleZ;
			switch(rock.m_CID)
			{
			default:
				// 'Standard' rocks.
				scaleX = intNs * 8.f/2.f;
				if(scaleX < .5f)
					scaleX = .5f;
				scaleY = intNt * 8.f/2.f;
				if(scaleY < .5f)
					scaleY = .5f;
				scaleZ = intNu * 8.f/2.f;
				if(scaleZ < .5f)
					scaleZ = .5f;
				break;
			case CID_Crystals:
				// Crystals are symmetrically scaled.
				scaleX = fabs(intNs) * 8.f/3.f;
				if(scaleX < .3f)
					scaleX = .3f;
				scaleY = scaleX;
				scaleZ = scaleX;
				break;
			case CID_Eludium:
				// Eludium is symmetrically scaled shards.
				scaleX = fabs(intNs) * 8.f/3.f;
				if(scaleX < .3f)
					scaleX = .3f;
				scaleY = scaleX/8.f;
				scaleZ = scaleY;
				break;
			case CID_Gems:
				// Gems are symmetrically scaled very small.
				scaleX = fabs(intNs) * 8.f/20.f;
				if(scaleX < .05f)
					scaleX = .05f;
				scaleY = scaleX;
				scaleZ = scaleX;
				break;
			case CID_Lumenite:
				// Lumenite is larger than radioactives and smaller than standard rocks.
				scaleX = intNs * 8.f/3.f;
				if(scaleX < .3f)
					scaleX = .3f;
				scaleY = intNt * 8.f/3.f;
				if(scaleY < .3f)
					scaleY = .3f;
				scaleZ = intNu * 8.f/3.f;
				if(scaleZ < .3f)
					scaleZ = .3f;
				break;
			case CID_Radioactives:
				// Radioactives are smaller than standard rocks.
				scaleX = fabs(intNs) * 8.f/4.f;
				if(scaleX < .25f)
					scaleX = .25f;
				scaleY = fabs(intNt) * 8.f/4.f;
				if(scaleY < .25f)
					scaleY = .25f;
				scaleZ = fabs(intNu) * 8.f/4.f;
				if(scaleZ < .25f)
					scaleZ = .25f;
				break;
			}

			float rotX = intNx * PIf;
			float rotY = intNy * PIf;
			float rotZ = intNz * PIf;
			rock.SetPosition(rockLoc, rotX, rotY, rotZ, scaleX, scaleY, scaleZ);
			rock.m_VAOIndex = DpRock::TextureTileIndex((COMMODITYID)rock.m_CID);

			rocks.insert(AuResKey(rock.m_VAOIndex, 0, rockLL, AuFloraDNA::Temperate, 0), rock);
		}
	}
	//qDebug() << "DaWorldRingSection::CreateRocks" << shrubs.count();
}

COMMODITYID DaWorldRingSection::RockAt(const AuGlobeLL& rockLL, quint32 worldGenSeed, int resourceZone) const
{
	int inEnclosure = InEnclosure(rockLL, 0);
	if(inEnclosure < 0)
		inEnclosure = 0;
	float snowCoverage = SnowCoverage(TerrainAltitudeMSL(rockLL), rockLL.Latitude(), inEnclosure);

	// Generate a coherent noise factor for a clumpy specie distribution. Range is -1 to 1.
	float nfreq = GroundLevelEquatorDivisions() * 6;						// Smaller creates larger squarish patches of each type.
	float ofreq = NorthSouthDivisions() * 6;					// Smaller creates larger squarish patches of each type.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float nx = ceilf(rockLL.Longitude()/(PI*2)*nfreq);
	float ny = ceilf(rockLL.Latitude()/PI*ofreq);
	m_RingSectionLock.lockForRead();
	float typeN = AuNoiseF::CoherentNoise(randF, m_TerrainXOff + nx, m_TerrainYOff + ny);
	m_RingSectionLock.unlock();

	// Get information about natural resources present at the rock location.
	AuZoneResource res;
	QueryNaturalResources(res, resourceZone, worldGenSeed);

	// Ice chunks appear on snow capped peaks and in polar regions.
	// Ice is present among the rocks in the environment of cold worlds.
	int iceFactor;
	if(snowCoverage >= .3f)
		iceFactor = (int)(snowCoverage * 100.f);	// lots of ice on snow capped peaks and poles
	else if(OrbitZone() == OZONE_Outer)
		iceFactor = 1;	// cold worlds have ice lying about
	else if(OrbitZone() == OZONE_Frigid)
		iceFactor = 2;	// frozen worlds have a bit more ice lying about
	else
		iceFactor = 0;	// Warmer worlds don't have ice lying about.

	// Return the commodity type that is there.
	return res.GenerateRockResource(typeN, iceFactor);
}

COMMODITYID DaWorldRingSection::ShrubAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const
{
	// Generate a coherent noise factor for a clumpy specie distribution. Range is -1 to 1.
	float nfreq = GroundLevelEquatorDivisions() * 20;						// Smaller creates larger squarish patches of each specie.
	float ofreq = NorthSouthDivisions() * 20;					// Smaller creates larger squarish patches of each specie.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float nx = ceilf(plantLL.Longitude()/(PI*2)*nfreq);
	float ny = ceilf(plantLL.Latitude()/PI*ofreq);
	m_RingSectionLock.lockForRead();
	float specieN = AuNoiseF::CoherentNoise(randF, m_TerrainXOff + nx, m_TerrainYOff + ny);
	m_RingSectionLock.unlock();

	// Get information about natural resources present at the plant location.
	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(plantLL.Longitude()), worldGenSeed);

	// Return the commodity type that is there.
	return res.GenerateShrubResource(specieN);
}

COMMODITYID DaWorldRingSection::TreeAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const
{
	// Generate a coherent noise factor for a clumpy specie distribution. Range is -1 to 1.
	float nfreq = GroundLevelEquatorDivisions() * 5;						// Smaller creates larger squarish patches of each specie.
	float ofreq = NorthSouthDivisions() * 5;					// Smaller creates larger squarish patches of each specie.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float nx = ceilf(plantLL.Longitude()/(PI*2)*nfreq);
	float ny = ceilf(plantLL.Latitude()/PI*ofreq);
	m_RingSectionLock.lockForRead();
	float specieN = AuNoiseF::CoherentNoise(randF, m_TerrainXOff + nx, m_TerrainYOff + ny);
	m_RingSectionLock.unlock();

	// Get information about natural resources present at the plant location.
	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(plantLL.Longitude()), worldGenSeed);

	// Return the commodity type that is there.
	return res.GenerateTreeResource(specieN);
}

void DaWorldRingSection::CreateAtTerrainTriangles(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLLTri& terTri,
		const bool& killed) const
{
	//qDebug() << "DaWorldRingSection::CreateAtTerrainTriangles" << Id() << Name() << "tris" << tris.count();

	// Arranged in clockwise draw order.
	AuGlobeLL triBase0, triBase1, triApex;
	if(FuzzyEquals(terTri.m_LL0.Latitude(), terTri.m_LL1.Latitude()))
	{
		// First two vertices are the base and the third vertex is the apex.
		triApex = terTri.m_LL2;
		triBase0 = terTri.m_LL0;
		triBase1 = terTri.m_LL1;
	}
	else if(FuzzyEquals(terTri.m_LL0.Latitude(), terTri.m_LL2.Latitude()))
	{
		// Second vertex is the apex and the first and last vertices are the base.
		triApex = terTri.m_LL1;
		triBase0 = terTri.m_LL2;
		triBase1 = terTri.m_LL0;
	}
	else
	{
		// First vertex is the apex and the last two vertices are the base.
		triApex = terTri.m_LL0;
		triBase0 = terTri.m_LL1;
		triBase1 = terTri.m_LL2;
	}

	// Fix seam.
	if(FuzzyEquals(triBase0.Longitude(), -PI) && triBase1.Longitude() > 0)
	{
		triBase0.SetLongitude(PI);
		if(FuzzyEquals(triApex.Longitude(), -PI))
			triApex.SetLongitude(PI);
	}
	if(FuzzyEquals(triBase1.Longitude(), -PI) && triBase0.Longitude() > 0)
	{
		triBase1.SetLongitude(PI);
		if(FuzzyEquals(triApex.Longitude(), -PI))
			triApex.SetLongitude(PI);
	}

	// Subdivide the terTri into small globe surface triangles. CreateSurfaceTriangles insures the first and last vertices of each
	// triangle are at different latitudes. The middle vertex is at the latitude of one of either the first or the last vertex.
	// The edge of the triangle that is parallel to the latitude lines is the base.
	// We will tessellate triangles from the base to the apex in parallel rows, to match the larger scale tessellation of the globe.
	const int subdivisions = 100;
	double latrange = triApex.Latitude() - triBase0.Latitude();					// latitude range from base to apex
	double lonrange = triApex.Longitude() - triBase0.Longitude();				// longitude range from base to apex
	double latinc = latrange/subdivisions;										// latitude increment per subdivision
	double loninc = lonrange/subdivisions;										// longitude increment per subdivision

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo> terAltInfo;							// Cache for optimization of terrain info lookups.

	// Tessellate rows of triangles from the base to the apex.
	AuGlobeLL rowStartLL(triBase0);
	AuGlobeLL rowEndLL(triBase1);
	int rowdivs = subdivisions;													// divisions in the base row
	double rowloninc = (rowEndLL.Longitude() - rowStartLL.Longitude())/rowdivs;
	double lonrange2 = triApex.Longitude() - triBase1.Longitude();				// longitude range from other base vertex to apex
	double loninc2 = lonrange2/subdivisions;									// longitude increment per subdivision
	for(int row=1; row<=subdivisions && !killed; row++)
	{
		// Draw triangles in the row.
		if(rowdivs > 1)
		{
			// This is not the apex row.
			// Calculate the next row start and end coordinates on a line from the base vertices to the apex.
			AuGlobeLL nextRowStartLL;
			nextRowStartLL.SetLatitude(triBase0.Latitude() + latinc*row);
			nextRowStartLL.SetLongitude(triBase0.Longitude() + loninc*row);
			AuGlobeLL nextRowEndLL;
			nextRowEndLL.SetLatitude(nextRowStartLL.Latitude());
			nextRowEndLL.SetLongitude(triBase1.Longitude() + loninc2*row);

			int nextrowdivs = rowdivs-1;						// number of vertex divisions in the next row
			double nextrowloninc = (nextRowEndLL.Longitude() - nextRowStartLL.Longitude())/nextrowdivs;

			// Create a row of triangles, starting exactly on the row start vertices.
			AuGlobeLL rowPrevLL(rowStartLL);
			AuGlobeLL nextRowPrevLL(nextRowStartLL);
			for(int rowtri=1; rowtri<rowdivs && !killed; rowtri++)
			{
				// Calculate the next triangle in the row.
				AuGlobeLL rowLL;
				rowLL.SetLatitude(rowStartLL.Latitude());
				rowLL.SetLongitude(rowStartLL.Longitude() + rowloninc*rowtri);

				AuGlobeLL nextRowLL;
				nextRowLL.SetLatitude(nextRowStartLL.Latitude());
				nextRowLL.SetLongitude(nextRowStartLL.Longitude() + nextrowloninc*rowtri);

				// Create two triangles.
				CreateAtTerrainSurfaceTriangle(tris, rowPrevLL, rowLL, nextRowPrevLL, terAltInfo);
				CreateAtTerrainSurfaceTriangle(tris, nextRowLL, nextRowPrevLL, rowLL, terAltInfo);

				// Set up for next pair of triangles in the row.
				rowPrevLL = rowLL;
				nextRowPrevLL = nextRowLL;
			}

			if(killed)
				break;

			// Create final row triangle, ending exactly on the row end vertices.
			CreateAtTerrainSurfaceTriangle(tris, rowPrevLL, rowEndLL, nextRowEndLL, terAltInfo);

			// Set up for the next row.
			rowdivs = nextrowdivs;
			rowloninc = nextrowloninc;
			rowStartLL = nextRowStartLL;
			rowEndLL = nextRowEndLL;
		}
		else
		{
			// This is the apex triangle, the only triangle in the final row. Insure it hits the apex exactly.
			CreateAtTerrainSurfaceTriangle(tris, rowStartLL, rowEndLL, triApex, terAltInfo);
		}
	}
}

// Returns the commodity id of significant rock outcroppings at 'll'.
// This is used for spotting and foraging. 'll' is rounded to the nearest actual rock grid location and that is the rock that
// is returned, using RockAt.
// Returns CID_Invalid if no significant rocks are present.
COMMODITYID DaWorldRingSection::SpotRockAt(const AuGlobeLL& ll, quint32 worldGenSeed) const
{
	// Calculate eqDivs exactly the way QueryNaturalResources does so.
	int eqDivs = (int)(TerrainXSize() / 20);					// 20' spacing for rocks at highest density.

	// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
	// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
	AuGlobeLL rockLL = RoundedGlobeLoc(ll, eqDivs);	// As if tessellated to 20' increments at equator.

	// Check guaranteed rock locations.
	int rz = ResourceZone(rockLL.Longitude());
	COMMODITYID cid = GuaranteedRockAt(rockLL.ToLLF(), rz);
	if(cid != CID_Invalid)
		return cid;

	// Test plant density at rockLL.
	// Flora density bands can be seen in the grass texture on the ground, which has 8 intensities.
	// 0 = Desert
	// 1 = Grasslands
	// 2 = Shrublands
	// 3 = Light forest and shrubs
	// 4-7 = Increasing forest, decreasing shrubs.
	const float rockAltMSL = TerrainAltitudeMSL(rockLL);
	int inEnclosure = InEnclosure(rockLL, rockAltMSL);
	if(inEnclosure < 0)
		inEnclosure = 0;
	float vegPercent = VegetationAt(rockLL, rockAltMSL, inEnclosure, IsIrrigated(rockLL))/100.f;	// 0..1
	float rockPercent = (1.f - vegPercent)/3.f;

	// Generate a coherent noise factor for a clumpy distribution density. Range is -1 to 1.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float densityN = AuNoiseF::CoherentNoise(randF, ceilf(rockLL.Longitude()*1500), ceilf(rockLL.Latitude()*1500));
	if(fabs(densityN) > rockPercent)
		return CID_Invalid;

	return RockAt(rockLL, worldGenSeed, rz);
}

// Returns the commodity id of significant shrubs at 'll'.
// This is used for spotting and foraging. 'll' is rounded to the nearest actual shrub grid location and that is the shrub that
// is returned, using ShrubAt.
// Returns CID_Invalid if no significant shrubs are present.
COMMODITYID DaWorldRingSection::SpotShrubAt(const AuGlobeLL& ll, quint32 worldGenSeed, bool& aquaticPlant) const
{
	// Try to keep a minimal amount of shrubs so players can still find a shrub after turning off the grass.
	const float userDensity = .2f;

	int eqDivs = (int)(TerrainXSize() / 12);					// 12' spacing for shrubs at highest density.

	// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
	// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
	AuGlobeLL plantLL = RoundedGlobeLoc(ll, eqDivs);			// As if tessellated to 12' increments at equator.

	// Test plant density at plantLL.
	// Flora density bands can be seen in the grass texture on the ground, which has 8 intensities.
	// 0 = Desert
	// 1 = Grasslands
	// 2 = Shrublands
	// 3 = Light forest and shrubs
	// 4-7 = Increasing forest, decreasing shrubs.
	float vegPercent;	// veg density present in the range of 0 to 1
	double plantAltMSL = TerrainAltitudeMSL(plantLL);
	int irrigated = IsIrrigated(plantLL);
	if(HasEnclosure())
	{
		int inEnclosure = InEnclosure(plantLL, plantAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		if(plantAltMSL < 0 && inEnclosure != 2)
			aquaticPlant = true;	// Undersea enclosures have no water.
		else
			aquaticPlant = false;
		vegPercent = VegetationAt(plantLL, plantAltMSL, inEnclosure, irrigated)/100.f;
	}
	else
	{
		if(plantAltMSL < 0)
			aquaticPlant = true;
		else
			aquaticPlant = false;
		vegPercent = VegetationAt(plantLL, plantAltMSL, false, irrigated)/100.f;
	}

	if(vegPercent < 2.f/8.f)
		return CID_Invalid;							// First two flora density bands have no shrubs.

	float shrubPercent;
	if(aquaticPlant)
	{
		// No trees under water so use the veg percentage for shrubs.
		shrubPercent = vegPercent;
	}
	else
	{
		// On land, shrubs are most abundant where there is no vegetation and least abundant in the midst of heavy vegetation.
		if(vegPercent >= 7.5f/8.f)
			return CID_Invalid;						// Highest flora density band has no shrubs at all.
		shrubPercent = 1.f - (vegPercent - .1f);
	}
 	shrubPercent *= userDensity;					// Modulate with user density setting.

	// Generate a coherent noise factor for a clumpy distribution density. Range is -1 to 1.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float densityN = AuNoiseF::CoherentNoise(randF, ceilf(plantLL.Longitude()*2000), ceilf(plantLL.Latitude()*2000));
	if(fabs(densityN) >= shrubPercent)
		return CID_Invalid;

	return ShrubAt(plantLL, worldGenSeed);
}

// Returns the commodity id of significant trees outcroppings at 'll'.
// This is used for spotting and foraging. 'll' is rounded to the nearest actual tree grid location and that is the tree that
// is returned, using TreeAt.
// Returns CID_Invalid if no significant trees are present.
COMMODITYID DaWorldRingSection::SpotTreeAt(const AuGlobeLL& ll, quint32 worldGenSeed) const
{
	int eqDivs = (int)(TerrainXSize() / 24);					// 24' spacing for trees at highest density.

	// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
	// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
	AuGlobeLL plantLL = RoundedGlobeLoc(ll, eqDivs);			// As if tessellated to 24' increments at equator.

	// Test plant density at plantLL.
	// Flora density bands can be seen in the grass texture on the ground, which has 8 intensities.
	// 0 = Desert
	// 1 = Grasslands
	// 2 = Shrublands
	// 3 = Light forest and shrubs
	// 4-7 = Increasing forest, decreasing shrubs.
	float vegPercent;	// veg density present in the range of 0 to 1
	double plantAltMSL = TerrainAltitudeMSL(plantLL);
	int irrigated = IsIrrigated(plantLL);
	if(HasEnclosure())
	{
		int inEnclosure = InEnclosure(plantLL, plantAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		// Don't grow trees underwater unless in an undersea enclosure.
		if(plantAltMSL < 0 && inEnclosure != 2)
			vegPercent = 0;	// Underwater and not in an undersea enclosure.
		else
			vegPercent = VegetationAt(plantLL, plantAltMSL, inEnclosure, irrigated)/100.f;
	}
	else
	{
		// Don't grow trees underwater outside of an undersea enclosure.
		if(plantAltMSL < 0)
			vegPercent = 0;
		else
			vegPercent = VegetationAt(plantLL, plantAltMSL, false, irrigated)/100.f;
	}
	if(vegPercent < 3.f/8.f)
		return CID_Invalid;										// No trees in desert, grassland, or shrublands.
	vegPercent /= 3;											// Drop range to reduce overall tree count.

	// Generate a coherent noise factor for a clumpy distribution density. Range is -1 to 1.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float densityN = AuNoiseF::CoherentNoise(randF, ceilf(plantLL.Longitude()*1000), ceilf(plantLL.Latitude()*1000));
	if(fabs(densityN) > vegPercent)
		return CID_Invalid;

	return TreeAt(plantLL, worldGenSeed);
}

void DaWorldRingSection::ResourceDistributionAt(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
		unsigned int& rocks, unsigned int& shrubs, unsigned int& trees, float& waterCoverage, unsigned int maxICareAbout) const
{
	// Init return values.
	rocks = 0;
	shrubs = 0;
	trees = 0;
	waterCoverage = 0;

	quint32 worldGenSeed = GenSeed();
	AuPoint vx = mBld.ProjectVector(AuPoint(1,0));
	AuPoint vy = mBld.ProjectVector(AuPoint(0,1));

	// Rocks (cobbled from CreateRocks) and water coverage
	{
		int eqDivs = (int)(TerrainXSize() / 20);				// 20' spacing for rocks at highest density.

		int waterFound = 0;
		int waterCheck = 0;
		QMap<AuGlobeLL, bool> alreadyChecked;
		for(int ix=-siteBoxRadius.x; ix<siteBoxRadius.x; ix+=10)
		{
			AuPoint vxv = vx*ix;
			for(int iy=-siteBoxRadius.y; iy<siteBoxRadius.y; iy+=10)
			{
				// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
				// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
				AuPoint rockLoc = siteCen + vxv + vy*iy;
				AuGlobeLL rockLL;
				if(!LLOfLoc(rockLL, rockLoc))
					continue;

				rockLL = RoundedGlobeLoc(rockLL, eqDivs);	// As if tessellated to 20' increments at equator.
				if(alreadyChecked.contains(rockLL))
					continue;
				alreadyChecked.insert(rockLL, true);

				// Calculate altitude MSL to determine if the location falls within a development site that hides rocks.
				bool hideRocks, hideShrubs, hideTrees;
				int hideGrass;
				WORLDTEX_TYPE pavementType;
				COMMODITYID farmShrub, orchardTree;
				double farmShrubMSL, orchardTreeMSL;
				double altMSL = TerrainAltitudeMSLPave(rockLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub,
						farmShrubMSL, orchardTree, orchardTreeMSL);
				if(m_byWater && altMSL < 0)
					waterFound++;		// Count hits in the water for overall water coverage.
				waterCheck++;
				if(hideRocks && maxICareAbout != 4)
					continue;

				// Stop counting rocks when we have the max we care about. Must continue the loop for water coverage.
				if(rocks == maxICareAbout)
					continue;

				// Check to see if there is a rock at that grid location.
				COMMODITYID cid = SpotRockAt(rockLL, worldGenSeed);
				if(cid == CID_Invalid)
					continue;

				rocks++;
			}
		}

		if(waterCheck)
			waterCoverage = (float)waterFound/(float)waterCheck;
		else
			waterCoverage = 0;
		//qDebug() << "waterFound" << waterFound << "/ waterCheck" << waterCheck << "= waterCoverage" << waterCoverage;
	}

	// Trees and shrubs can be found inside an enclosure.
	if(!maxICareAbout || (!CanHaveFlora() && !HasEnclosure()))
		return;

	// Shrubs (cobbled from CreateShrubs)
	{
		int eqDivs = (int)(TerrainXSize() / 12);					// 12' spacing for shrubs at highest density.

		QMap<AuGlobeLL, bool> alreadyChecked;
		for(int ix=-siteBoxRadius.x; ix<siteBoxRadius.x && shrubs < maxICareAbout; ix+=6)
		{
			AuPoint vxv = vx*ix;
			for(int iy=-siteBoxRadius.y; iy<siteBoxRadius.y; iy+=6)
			{
				// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
				// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
				AuPoint plantLoc = siteCen + vxv + vy*iy;
				AuGlobeLL plantLL;
				if(!LLOfLoc(plantLL, plantLoc))
					continue;

				plantLL = RoundedGlobeLoc(plantLL, eqDivs);	// As if tessellated to 12' incs at equator.
				if(alreadyChecked.contains(plantLL))
					continue;
				alreadyChecked.insert(plantLL, true);

				// Check to see if there is a natural growing shrub at that grid location.
				bool aquaticPlant;
				COMMODITYID cid = SpotShrubAt(plantLL, worldGenSeed, aquaticPlant);
				if(cid == CID_Invalid)
					continue;

				// Calculate altitude MSL to determine if the location falls within a development site that hides shrubs.
				bool hideRocks, hideShrubs, hideTrees;
				int hideGrass;
				WORLDTEX_TYPE pavementType;
				COMMODITYID farmShrub, orchardTree;
				double farmShrubMSL, orchardTreeMSL;
				TerrainAltitudeMSLPave(plantLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub, farmShrubMSL,
						orchardTree, orchardTreeMSL);
				if(hideShrubs && maxICareAbout != 4)
					continue;

				shrubs++;
				if(shrubs == maxICareAbout)
					break;
			}
		}
	}

	// Trees (cobbled from CreateTrees)
	{
		int eqDivs = (int)(TerrainXSize() / 24);					// 24' spacing for trees at highest density.

		QMap<AuGlobeLL, bool> alreadyChecked;
		for(int ix=-siteBoxRadius.x; ix<siteBoxRadius.x && trees < maxICareAbout; ix+=12)
		{
			AuPoint vxv = vx*ix;
			for(int iy=-siteBoxRadius.y; iy<siteBoxRadius.y; iy+=12)
			{
				// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
				// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
				AuPoint plantLoc = siteCen + vxv + vy*iy;
				AuGlobeLL plantLL;
				if(!LLOfLoc(plantLL, plantLoc))
					continue;

				plantLL = RoundedGlobeLoc(plantLL, eqDivs);	// As if tessellated to 24' incs at equator.
				if(alreadyChecked.contains(plantLL))
					continue;
				alreadyChecked.insert(plantLL, true);

				// Check to see if there is a natural growing tree at that grid location.
				COMMODITYID cid = SpotTreeAt(plantLL, worldGenSeed);
				if(cid == CID_Invalid)
					continue;

				// Calculate altitude MSL to determine if the location falls within a development site that hides trees.
				bool hideRocks, hideShrubs, hideTrees;
				int hideGrass;
				WORLDTEX_TYPE pavementType;
				COMMODITYID farmShrub, orchardTree;
				double farmShrubMSL, orchardTreeMSL;
				TerrainAltitudeMSLPave(plantLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub, farmShrubMSL,
						orchardTree, orchardTreeMSL);
				if(hideTrees && maxICareAbout != 4)
					continue;

				trees++;
				if(trees == maxICareAbout)
					break;
			}
		}
	}
}

// Checks the area of a mine or well to see if it contains any rocks that can produce 'idRes'.
bool DaWorldRingSection::TestRockResource(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
		COMMODITYID idRes) const
{
	quint32 worldGenSeed = GenSeed();
	AuPoint vx = mBld.ProjectVector(AuPoint(1,0));
	AuPoint vy = mBld.ProjectVector(AuPoint(0,1));

	// The rest of this was cobbled from CreateRocks
	int eqDivs = (int)(TerrainXSize() / 20);					// 20' spacing for rocks at highest density.

	QMap<AuGlobeLL, bool> alreadyChecked;
	for(int ix=-siteBoxRadius.x; ix<siteBoxRadius.x; ix+=10)
	{
		AuPoint vxv = vx*ix;
		for(int iy=-siteBoxRadius.y; iy<siteBoxRadius.y; iy+=10)
		{
			// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
			// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
			AuPoint rockLoc = siteCen + vxv + vy*iy;
			AuGlobeLL rockLL;
			if(!LLOfLoc(rockLL, rockLoc))
				continue;

			rockLL = RoundedGlobeLoc(rockLL, eqDivs);	// As if tessellated to 20' increments at equator.
			if(alreadyChecked.contains(rockLL))
				continue;
			alreadyChecked.insert(rockLL, true);

			// Check to see if there is a rock at that grid location.
			COMMODITYID cid = SpotRockAt(rockLL, worldGenSeed);
			if(idRes == CID_NaturalGas)
			{
				if(cid != CID_CrudeOil)
					continue;
			}
			else if(cid != idRes)
				continue;				// Skip the ones that are not the kind we want. No need to calc their location.

			/*
			// Calculate altitude MSL to determine if the location falls within a development site that hides rocks.
			bool hideRocks, hideShrubs, hideTrees;
			int hideGrass;
			WORLDTEX_TYPE pavementType;
			COMMODITYID farmShrub, orchardTree;
			double farmShrubMSL, orchardTreeMSL;
			TerrainAltitudeMSLPave(rockLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub, farmShrubMSL,
					orchardTree, orchardTreeMSL);
			if(hideRocks)
				continue;
				*/

			return true;				// Stop as soon as we find the rock we are looking for.
		}
	}
	return false;
}

bool DaWorldRingSection::TestForLiquid(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius) const
{
	AuPoint vx = mBld.ProjectVector(AuPoint(1,0));
	AuPoint vy = mBld.ProjectVector(AuPoint(0,1));

	// The rest of this was cobbled from CreateRocks
	int eqDivs = (int)(TerrainXSize() / 20);					// 20' spacing for rocks at highest density.

	QMap<AuGlobeLL, bool> alreadyChecked;
	for(int ix=-siteBoxRadius.x; ix<siteBoxRadius.x; ix+=10)
	{
		AuPoint vxv = vx*ix;
		for(int iy=-siteBoxRadius.y; iy<siteBoxRadius.y; iy+=10)
		{
			// Calculate ground location in a way that is consistent for everyone, regardless of where they are standing.
			// This also prevents bunching at the poles that results from using longitude latitude incremented grids.
			AuPoint rockLoc = siteCen + vxv + vy*iy;
			AuGlobeLL rockLL;
			if(!LLOfLoc(rockLL, rockLoc))
				continue;
			rockLL = RoundedGlobeLoc(rockLL, eqDivs);	// As if tessellated to 20' increments at equator.
			if(alreadyChecked.contains(rockLL))
				continue;
			alreadyChecked.insert(rockLL, true);

			// Check the altitude of the terrain at that grid location.
			double surfaceMSL = SurfaceAltitudeMSL(rockLL);
			if(surfaceMSL < 1)
				return true;			// Stop as soon as we find a spot under water.
		}
	}
	return false;
}

void DaWorldRingSection::BuildingChanged(const AuMatrix4x4& bldM, double radius, AuGLModelMaker& mm)
{
	DaWorldLL::BuildingChanged(bldM, radius, mm);
	//qDebug() << "DaWorldRingSection::BuildingChanged" << Id() << Name() << "iid" << bld->IndustryId()
		//<< "at" << bld->DevLL() << "radius" << bld->BuildRadius();
	// Delete the surface map. It will be recreated when requested.
	if(m_Surface && m_Surface != (QImage*)-1)
	{
		delete m_Surface;
		m_Surface = 0;
	}

	InvalidateNearGlobeTexture();

	// Calculate some useful information about the building.
	AuPoint bldLoc = bldM.Translation();
	AuPoint vx = bldM.ProjectVector(AuPoint(1,0));
	AuPoint vy = bldM.ProjectVector(AuPoint(0,1));
	AuGlobeLL bldLL;
	LLOfLoc(bldLL, bldLoc);

	// Invalidate the high tessellation surface triangles that are affected by the building.
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator tvi = m_AtGlobe.m_TriVAOs.begin();
	while(tvi != m_AtGlobe.m_TriVAOs.end())
	{
		// Locate the corresponding low tessellation globe surface triangle, for overall triangle coordinates.
		const AuGlobeTerrainTriCen& tri = m_AtGlobe.m_TerrainTris[(*tvi).first];

		// Test the building center to see if it intersects the low tessellation globe surface triangle.
		bool bldInTri = false;
		if(bldLL.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Cylinder))
			bldInTri = true;			// Building center is in the globe surface triangle.
		else
		{
			// The center of the building is not in the globe surface triangle.
			// Do more work to check the radius of the building, plus the blend margin.
			int radits = (radius / 20*FeetPerMeter()) + 1;	// Number of 20m radii increments to test.
			double radinc = radius / radits;				// Radius per increment.
			for(int rad=1; rad<=radits; rad++)
			{
				double radtest = radinc * rad;				// Calculate radius at rad increment.
				AuPoint bldX = vx * radtest;
				AuPoint bldY = vy * radtest;
				double C = radtest*2 * PI;					// Circumference. C = PI * D
				int tests = (int)(C / (20*FeetPerMeter()));	// Increments to test around the radius, at 20m intervals.
				if(tests < 4)
					tests = 4;
				for(int test=0; test<tests; test++)
				{
					float ang = test * (PIf*2) / tests;
					float ca = cosf(ang);
					float sa = sinf(ang);

					AuPoint testLoc = bldLoc + bldX*ca + bldY*sa;
					AuGlobeLL testLL;
					if(LLOfLoc(testLL, testLoc) && testLL.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Globe))
					{
						bldInTri = true;
						break;
					}
				}
				if(bldInTri)
					break;
			}
		}

		if(bldInTri)
		{
			// The building intersects the ringworld surface triangle.
			// Invalidate the high tessellation model of the surface triangle.
			// This does not delete it so it can still be used until the nu one is requested and becomes ready.
			m_AtGlobe.InvalidateRingworldSectionTriVAO(Id(), (*tvi).first, mm);
		}

		tvi++;
	}

	// Trigger regen of rocks and plants.
	m_FloraViewer = AuPoint(0,0);
	m_RockViewer = AuPoint(0,0);
	InvalidateTerrainInfoSprites();
}

void DaWorldRingSection::GeologicalFeatureChanged(const AuGeologicalFeature* feat, AuGLModelMaker& mm)
{
	// Delete the surface map. It will be recreated when requested.
	if(m_Surface && m_Surface != (QImage*)-1)
	{
		delete m_Surface;
		m_Surface = 0;
	}

	InvalidateNearGlobeTexture();

	double roadRadius = feat->Radius();

	// Calculate road end locations.
	AuPoint roadStart = LocOfLL(feat->LL2(), feat->LL2MSL());
	AuPoint roadEnd = LocOfLL(feat->LL(), feat->LLMSL());

	// Calculate center of road segment.
	AuPoint roadLoc = (roadStart + roadEnd)/2;
	AuGlobeLL roadLL;
	LLOfLoc(roadLL, roadLoc);

	// We will search the radius of half the road length plus the terrain transition zone.
	double roadHalfLen = roadLoc.Distance(roadStart);
	double searchRadius = roadHalfLen + roadRadius + (FeetPerMeter()*10);		// Road sections are rounded at the ends.

	// Calculate some useful information about the road location.
	AuPoint vup = -GravityDir(roadLoc);
	AuPoint vx;
	vup.ArbitraryAxis(vx);										// Sets vx to a unit vector.
	AuPoint vy = vup.Cross(vx);
	vy.Unit();

	// Invalidate the high tessellation surface triangles that are affected by the road.
	// As they get invalidated, they get removed from the TriVAOs list. That saves us from checking them over again once they
	// have been invalidated.
	unordered_map<int, AuGLTriVAO, AuTriHash>::iterator tvi = m_AtGlobe.m_TriVAOs.begin();
	while(tvi != m_AtGlobe.m_TriVAOs.end())
	{
		// Locate the corresponding low tessellation globe surface triangle, for overall triangle coordinates.
		const AuGlobeTerrainTriCen& tri = m_AtGlobe.m_TerrainTris[(*tvi).first];

		// Test the road center to see if it intersects the low tessellation globe surface triangle.
		bool roadInTri = false;
		if(roadLL.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Cylinder))
			roadInTri = true;			// Road center is in the globe surface triangle.
		else
		{
			// The center of the road is not in the globe surface triangle.
			// Do more work to check the searchRadius of the road.
			for(int test=0; test<4; test++)
			{
				// We can call cosf and sinf less if we make heavy use of their valuable results to calculate four perpendicular
				// vectors in each loop iteration, instead of calculating our way all the way around the circle. This is why the
				// angle is only calculated from 0 to 90 degrees.
				float ang = test * (PIf/2) / 4;					// Calculate angle in the range of 0 to 90.
				// Loop will test at searchRadius and and 1/2 searchRadius.
				for(int rad=1; rad<=2; rad++)
				{
					double srad = searchRadius * rad / 2;
					double rx = srad * cosf(ang);
					double ry = srad * sinf(ang);

					// Calculate a vector in the plane of the circle.
					{
						AuPoint testVec = vx*rx + vy*ry;

						// Test positive side of circle.
						AuPoint testLoc0 = roadLoc + testVec;
						AuGlobeLL testLL0;
						if(!LLOfLoc(testLL0, testLoc0))
							continue;
						if(testLL0.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Cylinder))
						{
							roadInTri = true;
							break;
						}

						// Test negative side of circle.
						AuPoint testLoc1 = roadLoc - testVec;
						AuGlobeLL testLL1;
						if(!LLOfLoc(testLL1, testLoc1))
							continue;
						if(testLL1.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Cylinder))
						{
							roadInTri = true;
							break;
						}
					}

					{
						AuPoint testVec = vx*ry - vy*rx;

						// Test positive side of circle.
						AuPoint testLoc0 = roadLoc + testVec;
						AuGlobeLL testLL0;
						if(!LLOfLoc(testLL0, testLoc0))
							continue;
						if(testLL0.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Cylinder))
						{
							roadInTri = true;
							break;
						}

						// Test negative side of circle.
						AuPoint testLoc1 = roadLoc - testVec;
						AuGlobeLL testLL1;
						if(!LLOfLoc(testLL1, testLoc1))
							continue;
						if(testLL1.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Cylinder))
						{
							roadInTri = true;
							break;
						}
					}
				}
			}
		}

		if(roadInTri)
		{
			// The road intersects the ringworld surface triangle.
			// Invalidate the high tessellation model of the surface triangle.
			// This does not delete it so it can still be used until the nu one is requested and becomes ready.
			m_AtGlobe.InvalidateRingworldSectionTriVAO(Id(), (*tvi).first, mm);
		}

		tvi++;
	}

	// Trigger regen of rocks and plants.
	m_FloraViewer = AuPoint(0,0);
	m_RockViewer = AuPoint(0,0);
	InvalidateTerrainInfoSprites();
}

void DaWorldRingSection::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaAtom* sceneAtom) const
{
	// Natural resources can be foraged.
	AuQuad q0(AuPoint(-5,-5,.5), AuPoint(-5,5,.5), AuPoint(5,5,.5), AuPoint(5,-5,.5));
	AuQuad q1(AuPoint(-5,-5,2), AuPoint(-5,5,2), AuPoint(5,5,2), AuPoint(5,-5,2));
	AuQuad q2(AuPoint(-5,-5,4), AuPoint(-5,5,4), AuPoint(5,5,4), AuPoint(5,-5,4));
	AuQuad qV0(AuPoint(-5,-5,.5), AuPoint(-5,-5,8), AuPoint(5,5,8), AuPoint(5,5,.5));
	AuQuad qV1(AuPoint(-5,5,.5), AuPoint(-5,5,8), AuPoint(5,-5,8), AuPoint(5,-5,.5));

	// Rocks
	for(QMap<AuResKey, AuGlobeRock>::const_iterator ri = m_Rocks.cbegin(); ri != m_Rocks.cend(); ri++)
	{
		// Skip resources that are too far away to be reached.
		AuPoint loc = tModel.Project((*ri).Location());
		if(loc.Length2() > 20*20)
			continue;					// More than 20' away.

		// Create some quads at the resource location relative to the terrain.
		AuQuad qres0(q0);
		qres0.Transform((*ri).Transform());
		AuQuad qres1(q1);
		qres1.Transform((*ri).Transform());
		AuQuad qres2(q2);
		qres2.Transform((*ri).Transform());
		AuQuad qresV0(qV0);
		qresV0.Transform((*ri).Transform());
		AuQuad qresV1(qV1);
		qresV1.Transform((*ri).Transform());

		// Transform the resource quads to be relative to the viewer.
		qres0.Transform(tModel);
		aControlPanels.append(qres0);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qres1.Transform(tModel);
		aControlPanels.append(qres1);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qres2.Transform(tModel);
		aControlPanels.append(qres2);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qresV0.Transform(tModel);
		aControlPanels.append(qresV0);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qresV1.Transform(tModel);
		aControlPanels.append(qresV1);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
	}

	// Shrubs
	for(QMap<AuResKey, AuGlobeFlora>::const_iterator ri = m_Shrubs.cbegin(); ri != m_Shrubs.cend(); ri++)
	{
		// Skip resources that are too far away to be reached.
		AuPoint loc = tModel.Project((*ri).Location());
		if(loc.Length2() > 20*20)
			continue;					// More than 20' away.

		// Create some quads at the resource location relative to the terrain.
		AuQuad qres0(q0);
		qres0.Transform((*ri).Transform());
		AuQuad qres1(q1);
		qres1.Transform((*ri).Transform());
		AuQuad qres2(q2);
		qres2.Transform((*ri).Transform());
		AuQuad qresV0(qV0);
		qresV0.Transform((*ri).Transform());
		AuQuad qresV1(qV1);
		qresV1.Transform((*ri).Transform());

		// Transform the resource quads to be relative to the viewer.
		qres0.Transform(tModel);
		aControlPanels.append(qres0);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qres1.Transform(tModel);
		aControlPanels.append(qres1);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qres2.Transform(tModel);
		aControlPanels.append(qres2);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qresV0.Transform(tModel);
		aControlPanels.append(qresV0);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
		qresV1.Transform(tModel);
		aControlPanels.append(qresV1);
		idPanels.append((*ri).CommodityId());	// Panel id is the commodity id.
	}

	// Aquatic Shrubs
	for(QMap<AuResKey, AuGlobeFlora>::const_iterator ri = m_AquaticShrubs.cbegin(); ri != m_AquaticShrubs.cend(); ri++)
	{
		// Skip resources that are too far away to be reached.
		AuPoint loc = tModel.Project((*ri).Location());
		if(loc.Length2() > 20*20)
			continue;					// More than 20' away.

		// Create some quads at the resource location relative to the terrain.
		AuQuad qres0(q0);
		qres0.Transform((*ri).Transform());
		AuQuad qres1(q1);
		qres1.Transform((*ri).Transform());
		AuQuad qres2(q2);
		qres2.Transform((*ri).Transform());
		AuQuad qresV0(qV0);
		qresV0.Transform((*ri).Transform());
		AuQuad qresV1(qV1);
		qresV1.Transform((*ri).Transform());

		// Transform the resource quads to be relative to the viewer.
		qres0.Transform(tModel);
		aControlPanels.append(qres0);
		idPanels.append(0x4000|CID_Hay);     // aquatic plants only produce vegetation
		qres1.Transform(tModel);
		aControlPanels.append(qres1);
		idPanels.append(0x4000|CID_Hay);     // aquatic plants only produce vegetation
		qres2.Transform(tModel);
		aControlPanels.append(qres2);
		idPanels.append(0x4000|CID_Hay);     // aquatic plants only produce vegetation
		qresV0.Transform(tModel);
		aControlPanels.append(qresV0);
		idPanels.append(0x4000|CID_Hay);     // aquatic plants only produce vegetation
		qresV1.Transform(tModel);
		aControlPanels.append(qresV1);
		idPanels.append(0x4000|CID_Hay);     // aquatic plants only produce vegetation
	}

	for(QMap<AuResKey, AuGlobeTree>::const_iterator ri = m_Trees.cbegin(); ri != m_Trees.cend(); ri++)
	{
		// Make quads around the trunk of the tree.
		AuPoint preb;											// Previous base point.
		AuPoint pret;											// Previous top point.
		for(int side=0; side<=8; side++)
		{
			// Panel id is the commodity id, and a tree indicator bit.
			quint16 treePanelId = (*ri).CommodityId() | 0x8000;	// Top bit says it's a tree. We need this to get plant dna.

			float angle = PIf*2 * side / 8.f;					// Angle of this pass.
			AuPoint curb = AuPoint((*ri).TrunkRadius(), 0, 0).RotateZ(angle);
			AuPoint curt = AuPoint((*ri).TrunkRadius(), 0, (*ri).TrunkLength()).RotateZ(angle);
			if(side)
			{
				// Not the first loop iteration so we have preb and pret points to make a quad.
				// Create a quad at the resource location relative to the terrain.
				AuQuad trunkQ = AuQuad(preb, pret, curt, curb);
				trunkQ.Transform((*ri).Transform());

				// Transform the quad to be relative to the viewer.
				trunkQ.Transform(tModel);
				aControlPanels.append(trunkQ);
				idPanels.append(treePanelId);
			}
			preb = curb;
			pret = curt;
		}
	}

	// Grass
	for(QMap<AuResKey, AuGlobeFlora>::const_iterator ri = m_Grass.cbegin(); ri != m_Grass.cend(); ri++)
	{
		// Skip resources that are too far away to be reached.
		AuPoint loc = tModel.Project((*ri).Location());
		if(loc.Length2() > 10*10)
			continue;					// More than 10' away.

		// Create some quads at the resource location relative to the terrain.
		AuQuad qres0(q0);
		qres0.Transform((*ri).Transform());

		// Transform the resource quads to be relative to the viewer.
		qres0.Transform(tModel);
		aControlPanels.append(qres0);
		idPanels.append(CID_Seed);	// Panel id is the commodity id. Only seeds can be gathered from grass.
	}

	// Aquatic Grass does not produce seeds.

	// Water can be foraged if the world has water and the test location is in the water.
	if(sceneAtom->IsInWater() && HydrographicLiquid() == HYDRO_Water)
	{
		// Create a quad that is in the space of the scene atom.
		double cgHeight = sceneAtom->GroundHeightAboveCG();
		AuQuad waterQ(AuPoint(-8,-8,-cgHeight), AuPoint(-8,8,-cgHeight), AuPoint(8,8,-cgHeight), AuPoint(8,-8,-cgHeight));
		aControlPanels.append(waterQ);
		idPanels.append(CID_Water);			// Panel id is the commodity id.
	}
}

void DaWorldRingSection::EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
{
	bool isTree = panelId & 0x8000;
	bool isAquatic = panelId & 0x4000;
	COMMODITYID cid = (COMMODITYID)(panelId & 0x3fff);
	GLuint texCID = ari.CommodityTextureN(cid);
	GLuint texSeed = ari.CommodityTextureN(CID_Seed);

	if(cid != CID_Seed)
		controls.append(AuControlButton(tr("Forage"), Id(), panelId, CI_ForageCID, texCID));

	// Aquatic plants do not give up seeds.
	if(!isAquatic)
	{
		if(isTree)
			controls.append(AuControlButton(tr("Seed"), Id(), panelId, CI_ForageSeed, texSeed));
		else if(DbCommodity::IsShrubResource(cid))
			controls.append(AuControlButton(tr("Seed"), Id(), panelId, CI_ForageSeed, texSeed));
		else if(cid == CID_Seed)
			controls.append(AuControlButton(tr("Seed"), Id(), panelId, CI_ForageSeed, texSeed));
	}
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DaWorldRingSection::SampleControlPanelInput(qint16 idPanel, const AuPoint& /*fPanelLoc*/,
		DiControlInputArray& aActiveControls, bool /*bContinuous*/)
{
	aActiveControls.append(DiControlInput(idPanel, CI_Forage));
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DaWorldRingSection::ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& pos,
		qint8 /*chSelValue*/, CONTROL_CONTEXT cc, int msElapsed)
{
	Q_UNUSED(pos);
	Q_UNUSED(msElapsed);
	// When simulating local motion, we don't change atoms in the environment.
	// Just return and wait for the server to do the work.
	if(cc == CONTROL_Client)
		return;

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_Forage:
			// Determine what has been foraged from the environment.
			{
				// Forage a commodity from the environment.
				COMMODITYID cid = (COMMODITYID)(ci.PanelId() & 0x3fff);
				pSceneAtom->Forage(this, cid, ci.PanelId() & 0x8000, 0);	// Forage one or more commodities from the environment.
			}
			break;
		case CI_ForageCID:
			// Determine what has been foraged from the environment.
			{
				// Forage a commodity from the environment.
				COMMODITYID cid = (COMMODITYID)(ci.PanelId() & 0x3fff);
				pSceneAtom->Forage(this, cid, ci.PanelId() & 0x8000, 1);	// Forage a commodity from the environment.
			}
			break;
		case CI_ForageSeed:
			// Determine what has been foraged from the environment.
			{
				// Forage a commodity from the environment.
				COMMODITYID cid = (COMMODITYID)(ci.PanelId() & 0x3fff);
				pSceneAtom->Forage(this, cid, ci.PanelId() & 0x8000, 2);	// Forage a seed from the environment.
			}
			break;
		}
	}
}

// 'fromLoc' and 'toLoc' are relative to this terrain patch. Both locations are expected to be IN this terrain patch.
// Returns next location in direction of path that will avoid obstructions to get to 'toLoc' from 'fromLoc'.
// Returns 'toLoc' if the path is not obstructed.
// Returns 0,0 if no path could be found.
AuPoint DaWorldRingSection::NextPathLocation(const AuPoint& /*fromLoc*/, const AuPoint& toLoc) const
{
	//DHH DaWorldRingSection::NextPathLocation
	return toLoc;
}

bool DaWorldRingSection::ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const
{
	// Create a matrix to calculate camera locations in my space.
	AuMatrix4x4 m = subatom->Transform() * camera;

	// Prevent looking below the ground.
	AuPoint lookLoc = m.Project(0,0,-1.5);										// Look at a point 1.5' from the camera.
	double lookAlt = AltitudeMSL(lookLoc);
	AuGlobeLL lookLL;
	if(LLOfLoc(lookLL, lookLoc))
	{
		AuTerrainInfo ter;
		QueryTerrainAtLoc(ter, lookLoc);
		double lookTerAlt = ter.m_rSurfaceRadius;
		if(lookAlt < lookTerAlt)
		{
			// Look point is below the ground.
			// Move camera backward by the amount it is looking below the ground.
			camera.Translate(0, 0, lookTerAlt - lookAlt);
			return true;
		}
	}
	else
	{
		//DHH Constrain camera against ringworld wall.
	}
	return false;
}

void DaWorldRingSection::AdjustNavLocation(const AuPoint& /*shipLoc*/, AuPoint& navLoc) const
{
	AuPoint rloc = EnvTransform().UnProject(navLoc);			// Calculate navLoc in my space.
	if(CheckChildExit(rloc, rloc, 0))
		return;													// rloc is outside of my domain space.

	// rloc is in my domain space. Adjust it above or below my sides, whichever is closer.
	if(rloc.y > 0)
		rloc.y = FeetPerOrbit()/5.;
	else
		rloc.y = -FeetPerOrbit()/5.;

	// Calculate navLoc in solar system space.
	navLoc = EnvTransform().Project(rloc);

	// No subatoms of a ringworld would affect navigation.
	//DaAtom::AdjustNavLocation(shipLoc, navLoc);
}

void DaWorldRingSection::QueryMineResources(const DaBd* mine, QList<COMMODITYID>& rockCIDs) const
{
	// List all mining commodities.
	QList<COMMODITYID> mineCIDs;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator mpi = gd.m_bManufacturingProcessArray.cbegin();
			mpi != gd.m_bManufacturingProcessArray.cend(); mpi++)
	{
		if((*mpi).m_idi != mine->IndustryId())
			continue;
		if(DbCommodity::IsPatent((COMMODITYID)(*mpi).m_idc))
			continue;
		mineCIDs.append((COMMODITYID)(*mpi).m_idc);
	}
	g_Au.UnlockData();

	// Test each possible mining commodity to see if it is present at the mine.
	AuMatrix4x4 mBld = mine->Transform();
	AuPoint siteCen = mBld.Project(mine->SiteCenter());
	for(QList<COMMODITYID>::iterator it = mineCIDs.begin(); it != mineCIDs.end(); it++)
		if(TestRockResource(mBld, siteCen, mine->SiteBoxRadius(), *it))
			rockCIDs.append(*it);

	// Rocks are always available where any other mine resources exist.
	if(!rockCIDs.empty() && !rockCIDs.contains(CID_Stone))
		rockCIDs.append(CID_Stone);
}

void DaWorldRingSection::QueryWellResources(const DaBd* well, QList<COMMODITYID>& wellCIDs) const
{
	// Water is always available as a choice if the environment has it.
	if(HydrographicLiquid() == HYDRO_Water)
		wellCIDs.append(CID_Water);

	// List all well commodities.
	QList<COMMODITYID> drillableCIDs;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator mpi = gd.m_bManufacturingProcessArray.cbegin();
			mpi != gd.m_bManufacturingProcessArray.cend(); mpi++)
	{
		if((*mpi).m_idi != well->IndustryId())
			continue;
		if(DbCommodity::IsPatent((COMMODITYID)(*mpi).m_idc))
			continue;
		if((*mpi).m_idc == CID_Water)
			continue;
		drillableCIDs.append((COMMODITYID)(*mpi).m_idc);
	}
	g_Au.UnlockData();

	// Test each possible well commodity to see if it is present at the well.
	AuMatrix4x4 mBld = well->Transform();
	AuPoint siteCen = mBld.Project(well->SiteCenter());
	for(QList<COMMODITYID>::iterator it = drillableCIDs.begin(); it != drillableCIDs.end(); it++)
		if(TestRockResource(mBld, siteCen, well->SiteBoxRadius(), *it))
			wellCIDs.append(*it);
}

//static
double DaWorldRingSection::MinimumSectionLength()
{
	return TerrainYSize() * LON_STRETCH_FACTOR;
}

// Returns true if a compass can provide directional guidance on this world at 'loc', which is relative to this world.
// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
bool DaWorldRingSection::CompassVectors(const AuPoint& loc, AuPoint& north, AuPoint& east, AuPoint& up) const
{
	north = AuPoint(0,1);
	up = -GravityDir(loc);
	east = north.Cross(up);
	east.Unit();
	return true;
}

int DaWorldRingSection::GroundLevelEquatorDivisions() const
{
	// In DaWorldRingwSection::EquatorDivisions, EqDiv = Circumference * 10 / PI
	// This calculation produces equator division spacing that matches that of DaWorldGlobes.
	// Circumference is equivalent to my terrain X size, but it must be expressed in 100km units like a globe.
	double c100km = TerrainXSize() / FeetPerMeter() / 100.;
	int eqdiv = (int)(c100km / PI);
	eqdiv &= ~0x03;						// We must return a number divisible by 4.

	//qDebug() << "DaWorldRingSection::EquatorDivisions" << Id() << Name() << "c100km" << c100km << "eqdiv" << eqdiv;
	return eqdiv;
}

int DaWorldRingSection::SeaLevelEquatorDivisions() const
{
	// In DaWorldRingwSection::EquatorDivisions, EqDiv = Circumference * 10 / PI
	// This calculation produces equator division spacing that matches that of DaWorldGlobes.
	// Circumference is equivalent to my terrain X size, but it must be expressed in 100km units like a globe.
	double c100km = TerrainXSize() / FeetPerMeter() / 100.;
	int eqdiv = (int)(c100km / PI);
	eqdiv &= ~0x03;						// We must return a number divisible by 4.

	//qDebug() << "DaWorldRingSection::EquatorDivisions" << Id() << Name() << "c100km" << c100km << "eqdiv" << eqdiv;
	return eqdiv;
}

bool DaWorldRingSection::IsForbiddenZone(const AuGlobeLLF& buildLL, double buildAltMSL, double radius) const
{
	AuPoint buildLoc = LocOfLL(buildLL, buildAltMSL);
	AuPoint eastVec = GravityDir(buildLoc).Cross(AuPoint(0,1));
	eastVec.Unit(radius);

	// Check for east end zone.
	AuPoint eastLoc = buildLoc + eastVec;
	AuGlobeLL ell;
	if(!LLOfLoc(ell, eastLoc))
		return true;
	if(ell.Longitude() >= 170.*DEG2RAD)
		return true;

	// Check for west end zone.
	AuPoint westLoc = buildLoc - eastVec;
	AuGlobeLL wll;
	if(!LLOfLoc(wll, westLoc))
		return true;
	if(wll.Longitude() <= -170.*DEG2RAD)
		return true;

	// Check for too far north.
	AuPoint northLoc = buildLoc + AuPoint(0,radius);
	AuGlobeLL nll;
	if(!LLOfLoc(nll, northLoc))
		return true;
	if(nll.Latitude() >= PI/2)
		return true;

	// Check for too far south.
	AuPoint southhLoc = buildLoc - AuPoint(0,radius);
	AuGlobeLL sll;
	if(!LLOfLoc(sll, southhLoc))
		return true;
	if(sll.Latitude() <= -PI/2)
		return true;

	if(SectionIndex() == 0)
	{
		// Check for control center zone on ringworld section 0.
		double zoneLat = DEG2RAD;
		double aspectYX = TerrainYSize() / TerrainXSize();
		double zoneLon = aspectYX * 2.*DEG2RAD;
		if(ell.Longitude() >= -zoneLon &&
				wll.Longitude() <= zoneLon &&
				nll.Latitude() >= -zoneLat &&
				sll.Latitude() <= zoneLat)
			return true;
	}

	return false;
}

void DaWorldRingSection::CreateGeologicalFeatures(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	Q_ASSERT(geof.isEmpty());

	//-----
	// Table of data for creating geological features.

	quint8 geoTypes[] =
	{
		AuGeoChasm::GeoType(),
		AuGeoChasm::GeoType(),
		AuGeoChasm::GeoType(),
		AuGeoCrater::GeoType(),
		AuGeoCrater::GeoType(),
		AuGeoMesa::GeoType(),
		AuGeoMesa::GeoType(),
		AuGeoRiver::GeoType(),
		AuGeoRiver::GeoType(),
		AuGeoRiver::GeoType(),
		AuGeoRiver::GeoType(),
		AuGeoRiver::GeoType(),
		AuGeoRiver::GeoType(),
		AuGeoRiver::GeoType(),
		AuGeoValley::GeoType(),
		AuGeoValley::GeoType(),
	};
	int geoTypeCount = sizeof(geoTypes) / sizeof(quint8);

	//-----

	// Determine how many features are on this ringworld section.
	// Less geological features are visible when the atmosphere density is higher. They erode more quickly.
	// Less geological features are visible when the world is more covered with water.
	// atmDensity/20 is 1..3 since atmDensity range is 25 to 75 on ringworlds
	// hydro/20 is 1..3 since hydro range is 25 to 75 on ringworlds
	int mod = 8 - m_byAtmDensity/20 - Hydrographics()/20;						// mod range is 0 to 6 (3 at Earth atm/hyd)
	mod /= 3;																	// mod range is 0 to 2 (1 at Earth atm/hyd)
	int features = dice.Roll(2, 4, mod);										// 2 to 10 features    (3 to 9 at Earth atm/hyd)

	// Create the geological features.
	while(features--)
	{
		quint8 geoType = dice.Roll(1, geoTypeCount, -1);
		AuGeologicalFeature* geo = AuGeologicalFeature::Create(geoTypes[geoType]);
		if(!geo)
			continue;	// Shouldn't happen.
		geo->RandomInit(*this, dice, geof);
		dice.Random();
		geof.append(geo);
	}

	UnlockGeologicalFeatures();
}

AuGlobeLLF DaWorldRingSection::RandomLL(const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	float lon = dice.Roll(1,361,-181) * DEG2RADf;				// Linear longitude probability.
	float lat = dice.Roll(1,181,-91) * DEG2RADf;				// Linear latitude probability.
	return AuGlobeLLF(lon, lat);
}

AuPoint DaWorldRingSection::ResourceZoneLandingSite(int zone) const
{
	// Grid search the area for a landing site. I can work hard at this because the result is cached.
	float lonarc = PIf / ResourceZones();						// Arc from center to longitude extent of resource zone.
	float latarc = PolarLatitude();								// Arc from equator to latitude extent of resource zone.
	float lonmid = (float)zone / (float)ResourceZones() * (PIf*2) - PIf + lonarc;	// Center longitude of resource zone.

	// Search from the center to the arc extents. We'll stop when we find a land location that is good enough.
	// 2 degree increments is tight enough.
	double bestAngle = PIf;
	AuPoint bestLoc;					// 0,0 means we didn't find one.
	for(float lat=DEG2RADf; lat < latarc; lat += DEG2RADf*2)
	{
		for(float lon=DEG2RADf; lon<lonarc; lon += DEG2RADf*2/LON_STRETCH_FACTOR)
		{
			// Check four locations relative to the center.
			// +, +
			AuGlobeLL ckll(lonmid + lon, lat);
			double terAltMSL = SurfaceAltitudeMSL(ckll);
			if(terAltMSL >= 1)
			{
				// Found land. Test for presense of geological features by comparing terrain altitude without them.
				double geoAltMSL = GeologicalTerrainAltitudeMSL(ckll);
				if(fabs(geoAltMSL - terAltMSL) < FeetPerMeter())
				{
					// Test steepness of terrain.
					AuPoint ckloc(LocOfLL(ckll));
					ElevateLocMSL(ckloc, terAltMSL);
					AuPoint terN = TerrainNormal(ckloc, 20*FeetPerMeter());
					AuPoint terG = -GravityDir(ckloc);
					double angle = terN.AngleTo(terG);
					if(angle < DEG2RAD*5)
						return ckloc;								// Location is within 5 degrees of level. Good enough.
					else if(angle < bestAngle)
					{
						bestAngle = angle;
						bestLoc = ckloc;
					}
				}
			}

			// +, -
			ckll = AuGlobeLL(lonmid + lon, -lat);
			terAltMSL = SurfaceAltitudeMSL(ckll);
			if(terAltMSL >= 1)
			{
				// Found land. Test for presense of geological features by comparing terrain altitude without them.
				double geoAltMSL = GeologicalTerrainAltitudeMSL(ckll);
				if(fabs(geoAltMSL - terAltMSL) < FeetPerMeter())
				{
					// Test steepness of terrain.
					AuPoint ckloc(LocOfLL(ckll));
					ElevateLocMSL(ckloc, terAltMSL);
					AuPoint terN = TerrainNormal(ckloc, 20*FeetPerMeter());
					AuPoint terG = -GravityDir(ckloc);
					double angle = terN.AngleTo(terG);
					if(angle < DEG2RAD*5)
						return ckloc;								// Location is within 5 degrees of level. Good enough.
					else if(angle < bestAngle)
					{
						bestAngle = angle;
						bestLoc = ckloc;
					}
				}
			}

			// -, +
			ckll = AuGlobeLL(lonmid - lon, lat);
			terAltMSL = SurfaceAltitudeMSL(ckll);
			if(terAltMSL >= 1)
			{
				// Found land. Test for presense of geological features by comparing terrain altitude without them.
				double geoAltMSL = GeologicalTerrainAltitudeMSL(ckll);
				if(fabs(geoAltMSL - terAltMSL) < FeetPerMeter())
				{
					// Test steepness of terrain.
					AuPoint ckloc(LocOfLL(ckll));
					ElevateLocMSL(ckloc, terAltMSL);
					AuPoint terN = TerrainNormal(ckloc, 20*FeetPerMeter());
					AuPoint terG = -GravityDir(ckloc);
					double angle = terN.AngleTo(terG);
					if(angle < DEG2RAD*5)
						return ckloc;								// Location is within 5 degrees of level. Good enough.
					else if(angle < bestAngle)
					{
						bestAngle = angle;
						bestLoc = ckloc;
					}
				}
			}

			// -, -
			ckll = AuGlobeLL(lonmid - lon, -lat);
			terAltMSL = SurfaceAltitudeMSL(ckll);
			if(terAltMSL >= 1)
			{
				// Found land. Test for presense of geological features by comparing terrain altitude without them.
				double geoAltMSL = GeologicalTerrainAltitudeMSL(ckll);
				if(fabs(geoAltMSL - terAltMSL) < FeetPerMeter())
				{
					// Test steepness of terrain.
					AuPoint ckloc(LocOfLL(ckll));
					ElevateLocMSL(ckloc, terAltMSL);
					AuPoint terN = TerrainNormal(ckloc, 20*FeetPerMeter());
					AuPoint terG = -GravityDir(ckloc);
					double angle = terN.AngleTo(terG);
					if(angle < DEG2RAD*5)
						return ckloc;								// Location is within 5 degrees of level. Good enough.
					else if(angle < bestAngle)
					{
						bestAngle = angle;
						bestLoc = ckloc;
					}
				}
			}
		}
	}
	return bestLoc;
}

void DaWorldRingSection::PrepareForScan()
{
	DaWorldLL::PrepareForScan();
	InitSystemPosition();
}

QString DaWorldRingSection::SystemScanReport(int scanEx) const
{
	// Biosphere exists where it is possible. It is shown as 'potential' where it can be grown at a colony. Blank where
	// no biosphere can exist, such as gas giants and rocky rings.
	QString sBio;
	sBio = tr("Biosphere");									// Animals or plants exist there.

	QString sOrbitZone;
	switch(OrbitZone())
	{
	case OZONE_Inferno:
		sOrbitZone = QObject::tr("Inferno Zone");
		break;
	case OZONE_Inner:
		sOrbitZone = QObject::tr("Inner Zone");
		break;
	case OZONE_Habitable:
		sOrbitZone = QObject::tr("Habitable Zone");
		break;
	case OZONE_Outer:
		sOrbitZone = QObject::tr("Outer Zone");
		break;
	case OZONE_Frigid:
		sOrbitZone = QObject::tr("Frigid Zone");
		break;
	case OZONE_DeepSpace:
		sOrbitZone = QObject::tr("Deep Space");
		break;
	}

	QString sLiquid;
	switch(HydrographicLiquid())
	{
	case HYDRO_None:
		break;
	case HYDRO_Lava:
		sLiquid = QObject::tr("Lava");
		break;
	case HYDRO_Water:
		sLiquid = QObject::tr("Water");
		break;
	case HYDRO_SupercooledLiquid:
		sLiquid = QObject::tr("Liquid");
		break;
	}

	QString sSize;
	QString sGrav;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	{
		sBodyType = QObject::tr("Ringworld Arc %1").arg(SectionIndex()+1);
		double rad = OrbitRadius();	// Radius in feet.
		rad /= FeetPerMeter();			// Convert to meters.
		rad = round(rad/100) * 100;		// Round to nearest 100 meters.
		sSize = QObject::tr("%L1m Radius").arg(rad);
		sGrav = QObject::tr("%1m/s%2 Sea Level Gravity")
			.arg(Gravity(0)/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());

		if(HasBreathableAtmosphere())
			sAtm = QObject::tr("%2, %1% Density, Breathable")
				.arg(MSLAtmosphereDensity()*100)
				.arg(AtmoTypeName(AtmosphereType()));
		else if(MSLAtmosphereDensity() > 0)
			sAtm = QObject::tr("%2, %1% Density, Toxic")
				.arg(MSLAtmosphereDensity()*100)
				.arg(AtmoTypeName(AtmosphereType()));
		else
			sAtm = QObject::tr("No Atmosphere");

		if(Hydrographics() > 0)
			sWater = QObject::tr("%1% Surface %2").arg(Hydrographics()).arg(sLiquid);
		else
			sWater = QObject::tr("No Surface %1").arg(sLiquid);
	}

	// Make a list of the natural resources the scanner can detect.
	QMap<QString, AuWorldRes_> atmlist;			// Atmosphere resources.
	QMap<QString, AuWorldRes_> biolist;			// Biosphere resources.
	QMap<QString, AuWorldRes_> geolist;			// Geosphere resources.
	QMap<QString, AuWorldRes_> hydlist;			// Geosphere resources.
	if(scanEx == 2)
	{
		// Must scan every resource zone.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> atmcount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> biocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> geocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> hydcount;	// Count of total between all zones. We will remove the zero ones at the end.
		for(int rz=0; rz<ResourceZones(); rz++)
		{
			QString cname;
			AuZoneResource res;
			QueryNaturalResources(res, rz);

			if(rz == 0)
			{
				// These resources exist in only a single planet-wide zone.
				// There is no need to repeat the information in multiple zones.

				// Atmosphere
				COMMODITYID atmres[] =
				{
					CID_Air,
					CID_AntifluxParticles,
					CID_BorexinoPrecipitate,
					CID_Cryozine,
					CID_Hydrogen,
					CID_Ioplasma,
				};
				int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, atmres, atmrescount, atmcount, atmlist);

				// Hydrosphere
				COMMODITYID hydres[] =
				{
					CID_Hydrographics,
					CID_Magmex,
					CID_Myrathane,
					CID_Flomentum,
				};
				int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, hydres, hydrescount, hydcount, hydlist);
			}

			// Remaining resources can exist in multiple zones.
			// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
			// To do this, we'll report all of them and keep count of the total probability.
			// At the end, we'll remove the ones that ended up with zero total probability.

			// Biosphere
			if(!sBio.isEmpty())
			{
				COMMODITYID biores[] =
				{
					CID_Animal,
					CID_Beans,
					CID_Cheese,
					CID_Eggs,
					CID_Fertilizer,
					CID_Fiber,
					CID_Fish,
					CID_Fruit,
					CID_Grain,
					CID_Grapes,
					CID_Herbs,
					CID_Hops,
					CID_Milk,
					CID_Nuts,
					CID_Spices,
					CID_Vegetable,
					CID_Vegetation,
					CID_Wood,
				};
				int biorescount = sizeof(biores)/sizeof(COMMODITYID);
				ReportZoneBioResources(rz, res, biores, biorescount, biocount, biolist);
			}

			// Geosphere
			COMMODITYID geores[] =
			{
				CID_Bolite,
				CID_Coal,
				CID_CrudeOil,
				CID_Crystals,
				CID_Eludium,
				CID_Gems,
				CID_GoldOre,
				CID_Ice,
				CID_Lumenite,
				CID_Minerals,
				CID_NaturalGas,
				CID_Ore,
				CID_Phlogiston,
				CID_Polytaride,
				CID_Radioactives,
				CID_Stone,
				CID_Vulcanite,
				CID_Adamantite,
				CID_Viathol,
			};
			int georescount = sizeof(geores)/sizeof(COMMODITYID);
			ReportZoneFetchResources(rz, res, geores, georescount, geocount, geolist);
		}

		// Remove the ones that ended up with zero total probability.
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = atmcount.begin(); bci != atmcount.end(); bci++)
			if(!(*bci).second)
				atmlist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = biocount.begin(); bci != biocount.end(); bci++)
			if(!(*bci).second)
				biolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = geocount.begin(); bci != geocount.end(); bci++)
			if(!(*bci).second)
				geolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = hydcount.begin(); bci != hydcount.end(); bci++)
			if(!(*bci).second)
				hydlist.remove(g_Au.CommodityName((*bci).first));
	}

	// Orbital position.
	QString sOrbiting;
	{
		double orbitAngle = PI/2 - Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(Parent()->Name())
			.arg(AuSensorBlip::FormatBigNumber(Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	// Put it all together.
	QString rpt = QString("<html><big><b>%1</b></big><p>").arg(Name());

	// Show page of general info about the planet.
	rpt += sOrbiting + "<br>";
	rpt += sOrbitZone;

	// Geosphere
	rpt += QObject::tr("<hr><b>Geosphere</b><br>");
	rpt += sBodyType + ", " + sSize;
	rpt += "<br>" + sGrav;
	if(!geolist.isEmpty())
	{
		rpt += "<br><table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
		rpt += "<tbody>";
		if(ResourceZones() == 1)
		{
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality and Abundance</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				QString r0;
				if((*ri).prob[0])
					r0 = QString("Q%1 %2%").arg((*ri).qual[0]).arg((*ri).prob[0]);
				else
					r0 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0);
			}
		}
		else if(ResourceZones() == 2)
		{
			rpt += "<tr><td></td><td></td><td colspan=\"2\"><b>Quality and Abundance</b></td></tr>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Zone 1</b></td><td><b>Zone 2</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				QString r0, r1, r2;
				if((*ri).prob[0])
					r0 = QString("Q%1 %2%").arg((*ri).qual[0]).arg((*ri).prob[0]);
				else
					r0 = QString("None");
				if((*ri).prob[1])
					r1 = QString("Q%1 %2%").arg((*ri).qual[1]).arg((*ri).prob[1]);
				else
					r1 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td><td>%4</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0)
					.arg(r1);
			}
		}
		else if(ResourceZones() == 3)
		{
			rpt += "<tr><td></td><td></td><td colspan=\"3\"><b>Quality and Abundance</b></td></tr>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Zone 1</b></td><td><b>Zone 2</b></td><td><b>Zone 3</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				QString r0, r1, r2;
				if((*ri).prob[0])
					r0 = QString("Q%1 %2%").arg((*ri).qual[0]).arg((*ri).prob[0]);
				else
					r0 = QString("None");
				if((*ri).prob[1])
					r1 = QString("Q%1 %2%").arg((*ri).qual[1]).arg((*ri).prob[1]);
				else
					r1 = QString("None");
				if((*ri).prob[2])
					r2 = QString("Q%1 %2%").arg((*ri).qual[2]).arg((*ri).prob[2]);
				else
					r2 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0)
					.arg(r1)
					.arg(r2);
			}
		}
		else
		{
			rpt += "<tr><td></td><td></td><td colspan=\"3\"><b>Quality and Abundance</b></td></tr>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Zone 1</b></td><td><b>Zone 2</b></td><td><b>Zone 3</b></td><td><b>Zone 4</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				QString r0, r1, r2, r3;
				if((*ri).prob[0])
					r0 = QString("Q%1 %2%").arg((*ri).qual[0]).arg((*ri).prob[0]);
				else
					r0 = QString("None");
				if((*ri).prob[1])
					r1 = QString("Q%1 %2%").arg((*ri).qual[1]).arg((*ri).prob[1]);
				else
					r1 = QString("None");
				if((*ri).prob[2])
					r2 = QString("Q%1 %2%").arg((*ri).qual[2]).arg((*ri).prob[2]);
				else
					r2 = QString("None");
				if((*ri).prob[3])
					r3 = QString("Q%1 %2%").arg((*ri).qual[3]).arg((*ri).prob[3]);
				else
					r3 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0)
					.arg(r1)
					.arg(r2)
					.arg(r3);
			}
		}
		rpt += "</tbody></table>";
	}

	// Hydrosphere
	if(!sWater.isEmpty())
	{
		rpt += QObject::tr("<p><hr><b>Hydrosphere</b><br>");
		rpt += sWater;
		if(!hydlist.isEmpty())
		{
			rpt += "<br><table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
			rpt += "<tbody>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality and Abundance</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = hydlist.begin(); ri != hydlist.end(); ri++)
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>Q%3 %4%</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg((*ri).qual[0])
					.arg((*ri).prob[0]);
			rpt += "</tbody></table>";
		}
	}

	// Atmosphere
	if(!sAtm.isEmpty() || !atmlist.isEmpty())
	{
		rpt += QObject::tr("<p><hr><b>Atmosphere</b><br>");
		if(!sAtm.isEmpty())
			rpt += sAtm;
		if(!atmlist.isEmpty())
		{
			rpt += "<br><table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
			rpt += "<tbody>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality and Abundance</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = atmlist.begin(); ri != atmlist.end(); ri++)
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>Q%3 %4%</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg((*ri).qual[0])
					.arg((*ri).prob[0]);
			rpt += "</tbody></table>";
		}
	}

	// Biosphere
	if(!sBio.isEmpty() && !biolist.isEmpty())
	{
		rpt += "<p><hr><b>" + sBio + "</b><br>";
		if(CanHaveFauna())
			rpt += "Native animals present.<br>";
		else
			rpt += "No native animals. Potential for agricultural production.<br>";
		if(CanHaveFlora())
			rpt += "Native plants present.<br>";
		else
			rpt += "No native plants. Potential for agricultural production.<br>";
		rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
		rpt += "<tbody>";
		if(ResourceZones() == 1)
		{
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>Q%3</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg((*ri).qual[0]);
			rpt += "</tbody></table>";
		}
		else if(ResourceZones() == 2)
		{
			rpt += "<tr><td></td><td></td><td colspan=\"3\"><b>Quality</b></td></tr>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Zone 1</b></td><td><b>Zone 2</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			{
				QString r0, r1;
				if((*ri).prob[0])
					r0 = QString("Q%1").arg((*ri).qual[0]);
				else
					r0 = QString("None");
				if((*ri).prob[1])
					r1 = QString("Q%1").arg((*ri).qual[1]);
				else
					r1 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td><td>%4</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0)
					.arg(r1);
			}
		}
		else if(ResourceZones() == 3)
		{
			rpt += "<tr><td></td><td></td><td colspan=\"3\"><b>Quality</b></td></tr>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Zone 1</b></td><td><b>Zone 2</b></td><td><b>Zone 3</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			{
				QString r0, r1, r2;
				if((*ri).prob[0])
					r0 = QString("Q%1").arg((*ri).qual[0]);
				else
					r0 = QString("None");
				if((*ri).prob[1])
					r1 = QString("Q%1").arg((*ri).qual[1]);
				else
					r1 = QString("None");
				if((*ri).prob[2])
					r2 = QString("Q%1").arg((*ri).qual[2]);
				else
					r2 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0)
					.arg(r1)
					.arg(r2);
			}
		}
		else
		{
			rpt += "<tr><td></td><td></td><td colspan=\"3\"><b>Quality</b></td></tr>";
			rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Zone 1</b></td><td><b>Zone 2</b></td><td><b>Zone 3</b></td><td><b>Zone 4</b></td></tr>";
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			{
				QString r0, r1, r2, r3;
				if((*ri).prob[0])
					r0 = QString("Q%1").arg((*ri).qual[0]);
				else
					r0 = QString("None");
				if((*ri).prob[1])
					r1 = QString("Q%1").arg((*ri).qual[1]);
				else
					r1 = QString("None");
				if((*ri).prob[2])
					r2 = QString("Q%1").arg((*ri).qual[2]);
				else
					r2 = QString("None");
				if((*ri).prob[3])
					r3 = QString("Q%1").arg((*ri).qual[3]);
				else
					r3 = QString("None");
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
					.arg(g_Au.CommodityBitmapName((*ri).cid))
					.arg(g_Au.CommodityName((*ri).cid))
					.arg(r0)
					.arg(r1)
					.arg(r2)
					.arg(r3);
			}
		}
	}

	rpt += "</html>";
	return rpt;
}

void DaWorldRingSection::SystemScanReportXML(QXmlStreamWriter& stream, int scanEx) const
{
	// Biosphere exists where it is possible. It is shown as 'potential' where it can be grown at a colony. Blank where
	// no biosphere can exist, such as gas giants and rocky rings.
	QString sBio;
	sBio = tr("Biosphere");									// Animals or plants exist there.

	QString sOrbitZone;
	switch(OrbitZone())
	{
	case OZONE_Inferno:
		sOrbitZone = QObject::tr("Inferno Zone");
		break;
	case OZONE_Inner:
		sOrbitZone = QObject::tr("Inner Zone");
		break;
	case OZONE_Habitable:
		sOrbitZone = QObject::tr("Habitable Zone");
		break;
	case OZONE_Outer:
		sOrbitZone = QObject::tr("Outer Zone");
		break;
	case OZONE_Frigid:
		sOrbitZone = QObject::tr("Frigid Zone");
		break;
	case OZONE_DeepSpace:
		sOrbitZone = QObject::tr("Deep Space");
		break;
	}

	QString sLiquid;
	switch(HydrographicLiquid())
	{
	case HYDRO_None:
		break;
	case HYDRO_Lava:
		sLiquid = QObject::tr("Lava");
		break;
	case HYDRO_Water:
		sLiquid = QObject::tr("Water");
		break;
	case HYDRO_SupercooledLiquid:
		sLiquid = QObject::tr("Liquid");
		break;
	}

	QString sSize;
	QString sGrav;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	{
		sBodyType = QObject::tr("Ringworld Arc %1").arg(SectionIndex()+1);
		double rad = OrbitRadius();	// Radius in feet.
		rad /= FeetPerMeter();			// Convert to meters.
		rad = round(rad/100) * 100;		// Round to nearest 100 meters.
		sSize = QObject::tr("%1m Radius").arg(rad);
		sGrav = QObject::tr("%1m/s%2 Sea Level Gravity")
			.arg(Gravity(SeaLevelAltitude())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());
		if(MSLAtmosphereDensity() > 0)
			sAtm = QObject::tr("%2, %1% Density")
				.arg(MSLAtmosphereDensity()*100)
				.arg(AtmoTypeName(AtmosphereType()));
		else
			sAtm = QObject::tr("No Atmosphere");
		if(Hydrographics() > 0)
			sWater = QObject::tr("%1% Surface %2").arg(Hydrographics()).arg(sLiquid);
		else
			sWater = QObject::tr("No Surface %1").arg(sLiquid);
	}

	// Make a list of the natural resources the scanner can detect.
	QMap<QString, AuWorldRes_> atmlist;			// Atmosphere resources.
	QMap<QString, AuWorldRes_> biolist;			// Biosphere resources.
	QMap<QString, AuWorldRes_> geolist;			// Geosphere resources.
	QMap<QString, AuWorldRes_> hydlist;			// Geosphere resources.
	if(scanEx == 2)
	{
		// Must scan every resource zone.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> atmcount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> biocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> geocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> hydcount;	// Count of total between all zones. We will remove the zero ones at the end.
		for(int rz=0; rz<ResourceZones(); rz++)
		{
			QString cname;
			AuZoneResource res;
			QueryNaturalResources(res, rz);

			if(rz == 0)
			{
				// These resources exist in only a single planet-wide zone.
				// There is no need to repeat the information in multiple zones.

				// Atmosphere
				COMMODITYID atmres[] =
				{
					CID_Air,
					CID_AntifluxParticles,
					CID_BorexinoPrecipitate,
					CID_Cryozine,
					CID_Hydrogen,
					CID_Ioplasma,
				};
				int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, atmres, atmrescount, atmcount, atmlist);

				// Hydrosphere
				COMMODITYID hydres[] =
				{
					CID_Hydrographics,
					CID_Magmex,
					CID_Myrathane,
					CID_Flomentum,
				};
				int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, hydres, hydrescount, hydcount, hydlist);
			}

			// Remaining resources can exist in multiple zones.
			// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
			// To do this, we'll report all of them and keep count of the total probability.
			// At the end, we'll remove the ones that ended up with zero total probability.

			// Biosphere
			if(!sBio.isEmpty())
			{
				COMMODITYID biores[] =
				{
					CID_Animal,
					CID_Beans,
					CID_Cheese,
					CID_Eggs,
					CID_Fertilizer,
					CID_Fiber,
					CID_Fish,
					CID_Fruit,
					CID_Grain,
					CID_Grapes,
					CID_Herbs,
					CID_Hops,
					CID_Milk,
					CID_Nuts,
					CID_Spices,
					CID_Vegetable,
					CID_Vegetation,
					CID_Wood,
				};
				int biorescount = sizeof(biores)/sizeof(COMMODITYID);
				ReportZoneBioResources(rz, res, biores, biorescount, biocount, biolist);
			}

			// Geosphere
			COMMODITYID geores[] =
			{
				CID_Bolite,
				CID_Coal,
				CID_CrudeOil,
				CID_Crystals,
				CID_Eludium,
				CID_Gems,
				CID_GoldOre,
				CID_Ice,
				CID_Lumenite,
				CID_Minerals,
				CID_NaturalGas,
				CID_Ore,
				CID_Phlogiston,
				CID_Polytaride,
				CID_Radioactives,
				CID_Stone,
				CID_Vulcanite,
				CID_Adamantite,
				CID_Viathol,
			};
			int georescount = sizeof(geores)/sizeof(COMMODITYID);
			ReportZoneFetchResources(rz, res, geores, georescount, geocount, geolist);
		}

		// Remove the ones that ended up with zero total probability.
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = atmcount.begin(); bci != atmcount.end(); bci++)
			if(!(*bci).second)
				atmlist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = biocount.begin(); bci != biocount.end(); bci++)
			if(!(*bci).second)
				biolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = geocount.begin(); bci != geocount.end(); bci++)
			if(!(*bci).second)
				geolist.remove(g_Au.CommodityName((*bci).first));
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = hydcount.begin(); bci != hydcount.end(); bci++)
			if(!(*bci).second)
				hydlist.remove(g_Au.CommodityName((*bci).first));
	}

	// Orbital position.
	QString sOrbiting;
	{
		double orbitAngle = PI/2 - Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(Parent()->Name())
			.arg(AuSensorBlip::FormatBigNumber(Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	// Put it all together.
	stream.writeStartElement("planet");
	stream.writeAttribute("planetId", Id().StringId());
	stream.writeAttribute("name", Name());
	stream.writeAttribute("bodyType", sBodyType);

	// Show page of general info about the planet.
	stream.writeAttribute("orbit", sOrbiting);
	stream.writeAttribute("zone", sOrbitZone);

	// Geosphere
	stream.writeStartElement("geosphere");
	stream.writeAttribute("diameter", sSize);
	stream.writeAttribute("gravity", sGrav);
	stream.writeAttribute("resourceZones", QString("%1").arg(ResourceZones()));
	if(!geolist.isEmpty())
	{
		if(ResourceZones() == 1)
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeEndElement();
			}
		}
		else if(ResourceZones() == 2)
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeAttribute("qualityZone2", QString("%1").arg((*ri).qual[1]));
				stream.writeAttribute("abundanceZone2", QString("%1").arg((*ri).prob[1]));
				stream.writeEndElement();
			}
		}
		else if(ResourceZones() == 3)
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeAttribute("qualityZone2", QString("%1").arg((*ri).qual[1]));
				stream.writeAttribute("abundanceZone2", QString("%1").arg((*ri).prob[1]));
				stream.writeAttribute("qualityZone3", QString("%1").arg((*ri).qual[2]));
				stream.writeAttribute("abundanceZone3", QString("%1").arg((*ri).prob[2]));
				stream.writeEndElement();
			}
		}
		else
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeAttribute("qualityZone2", QString("%1").arg((*ri).qual[1]));
				stream.writeAttribute("abundanceZone2", QString("%1").arg((*ri).prob[1]));
				stream.writeAttribute("qualityZone3", QString("%1").arg((*ri).qual[2]));
				stream.writeAttribute("abundanceZone3", QString("%1").arg((*ri).prob[2]));
				stream.writeAttribute("qualityZone4", QString("%1").arg((*ri).qual[3]));
				stream.writeAttribute("abundanceZone4", QString("%1").arg((*ri).prob[3]));
				stream.writeEndElement();
			}
		}
	}
	stream.writeEndElement();

	// Hydrosphere
	if(!sWater.isEmpty())
	{
		stream.writeStartElement("hydrosphere");
		stream.writeAttribute("surfaceLiquid", sWater);
		if(!hydlist.isEmpty())
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = hydlist.begin(); ri != hydlist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeEndElement();
			}
		}
		stream.writeEndElement();
	}

	// Atmosphere
	if(!sAtm.isEmpty() || !atmlist.isEmpty())
	{
		stream.writeStartElement("atmosphere");
		if(!sAtm.isEmpty())
			stream.writeAttribute("composition", sAtm);
		if(!atmlist.isEmpty())
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = atmlist.begin(); ri != atmlist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeEndElement();
			}
		}
		stream.writeEndElement();
	}

	// Biosphere
	if(!sBio.isEmpty() && !biolist.isEmpty())
	{
		stream.writeStartElement("biosphere");
		if(CanHaveFauna())
			stream.writeAttribute("fauna", "yes");
		else
			stream.writeAttribute("fauna", "no");
		if(CanHaveFlora())
			stream.writeAttribute("flora", "yes");
		else
			stream.writeAttribute("flora", "no");

		if(ResourceZones() == 1)
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeEndElement();
			}
		}
		else if(ResourceZones() == 2)
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeAttribute("qualityZone2", QString("%1").arg((*ri).qual[1]));
				stream.writeAttribute("abundanceZone2", QString("%1").arg((*ri).prob[1]));
				stream.writeEndElement();
			}
		}
		else if(ResourceZones() == 3)
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeAttribute("qualityZone2", QString("%1").arg((*ri).qual[1]));
				stream.writeAttribute("abundanceZone2", QString("%1").arg((*ri).prob[1]));
				stream.writeAttribute("qualityZone3", QString("%1").arg((*ri).qual[2]));
				stream.writeAttribute("abundanceZone3", QString("%1").arg((*ri).prob[2]));
				stream.writeEndElement();
			}
		}
		else
		{
			for(QMap<QString,AuWorldRes_>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			{
				stream.writeStartElement("resource");
				stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
				stream.writeAttribute("qualityZone1", QString("%1").arg((*ri).qual[0]));
				stream.writeAttribute("abundanceZone1", QString("%1").arg((*ri).prob[0]));
				stream.writeAttribute("qualityZone2", QString("%1").arg((*ri).qual[1]));
				stream.writeAttribute("abundanceZone2", QString("%1").arg((*ri).prob[1]));
				stream.writeAttribute("qualityZone3", QString("%1").arg((*ri).qual[2]));
				stream.writeAttribute("abundanceZone3", QString("%1").arg((*ri).prob[2]));
				stream.writeAttribute("qualityZone4", QString("%1").arg((*ri).qual[3]));
				stream.writeAttribute("abundanceZone4", QString("%1").arg((*ri).prob[3]));
				stream.writeEndElement();
			}
		}
		stream.writeEndElement();
	}

	stream.writeEndElement();
}

void DaWorldRingSection::FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& shipSysLoc,
		float gunRange, const DaAtom*& targetAtom, double& targetRange) const
{
	if(!CanBeHarvested(harvesterModule))
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

AuPoint DaWorldRingSection::WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const
{
	// Ringworld section calculates closest point on ringworld surface from ship.
	const DaAtom* parent = Parent();
	if(!parent)
		return AuPoint();
	double seaRad = OrbitRadius();								// Radius of ringworld sea level.
	AuPoint cen = parent->EnvLocation();					// Location of center of the ringworld.
	AuPoint vn = parent->EnvTransform().ProjectVector(0,0,1);
	if(harvesterModule == CID_AtmosphereCondenserBayModule || harvesterModule == CID_ParticleCollectorBayModule)
	{
		double atmRad = seaRad - AtmosphereRadius() + 1000;		// Radius 1000' inside ringworld atmosphere.
		return shipSysLoc.ClosestPointOnCircle(cen, atmRad, vn);	// Closest loc rel to sys.
	}
	else if(harvesterModule == CID_SiphonBayModule)
	{
		return shipSysLoc.ClosestPointOnCircle(cen, seaRad, vn);	// Closest loc rel to sys.
	}
	else
	{
		AuPoint targetLoc = shipSysLoc.ClosestPointOnCircle(cen, seaRad, vn);	// Closest loc rel to sys.
		AuPoint ringloc = EnvTransform().UnProject(targetLoc);	// loc rel to ring
		AuGlobeLL ll;
		if(LLOfLoc(ll, ringloc))
		{
			// Adjust lock location to the terrain surface.
			ElevateLocMSL(ringloc, SurfaceAltitudeMSL(ll));
			return EnvTransform().Project(ringloc);			// Convert loc rel to sys.
		}
	}
	return EnvLocation();
}

// Returns true if 'loc' is in the activation area of the control center, for testing preon location for activation.
bool DaWorldRingSection::IsActivationLocation(const AuPoint& loc) const
{
	// Only section index 0 has a control center.
	if(SectionIndex())
		return false;

	// The dais is 10 meters square radius. (-10,10) to (10,-10).
	AuPoint centerLoc = GetTerrainLocation(AuGlobeLL(0,0));
	return centerLoc.Distance2(loc) < 9*FeetPerMeter()*9*FeetPerMeter();
}

void DaWorldRingSection::ActivateControlCenter()
{
	Q_ASSERT(!SectionIndex());	// Only section index 0 has a control center to activate.
	m_ControlCenterActive = true;
	SetModified(NOTIFY_Scene);	// Not stored in database.
}

void DaWorldRingSection::DeactivateControlCenter()
{
	Q_ASSERT(!SectionIndex());	// Only section index 0 has a control center to activate.
	m_ControlCenterActive = false;
	SetModified(NOTIFY_Scene);	// Not stored in database.
}

int DaWorldRingSection::PopulationMax() const
{
	return 1000;
}

quint32 DaWorldRingSection::WorldPopulationMax() const
{
	double rx = TerrainXSize() / FeetPerMeter();				// Length of the arc section, in meters.
	double ry = TerrainYSize() / FeetPerMeter();				// Wall to wall distance of the arc section, in meters.
	double area = rx * ry;										// Area of the arc section in square meters.
	double maxPop = area / 1000;   								// Maximum population, at 1000 square meters per person.
	return (quint32)round(maxPop);								// Rounded to whole people.
}

// Activates the henge portal(s) on this arc segment. Creates a henge portal if none exists.
// This signals each interconnecting system to create/activate the destination henge portal.
void DaWorldRingSection::ActivateHenges()
{
	const DaStar* star = FindParentStarInEnv();
	if(!star)
		return;
	const DaSolarSystem* system = star->FindParentSolarSystem();
	if(!system)
		return;
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;

	// Get information about my end of the connection, to be sent to the destination system.
	AuId fromSysId = system->Id();
	int sn = star->StarNumber();
	quint8 fromSun;
	if(sn <= 0)
		fromSun = 0;	// My sun is the primary.
	else
		fromSun = sn;	// My sun is secondary or tertiary.
	quint8 fromShadowPanels = RingworldSections();
	quint8 fromArcSegment = SectionIndex();
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "DaWorldRingSection::ActivateHenges" << Id() << Name() << "sys" << fromSysId << "sun" << fromSun
			<< "panels" << fromShadowPanels << "arc" << fromArcSegment;

	// Activate all my henge portals.
	AuIdList destSysIds;
	QList<quint8> destSuns;
	QList<quint8> destArcs;
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	for(AuGeologicalFeatures::iterator gi = geof.begin(); gi != geof.end(); gi++)
	{
		AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		// Activate my henge portal.
		henge->SetActive(true);
		destSysIds.append(henge->DestSysId());
		destSuns.append(henge->DestSun());
		destArcs.append(henge->DestArcSegment());
	}
	UnlockGeologicalFeatures();

	if(destSysIds.isEmpty())
	{
		// I do not have any valid henge portals.
		// Create my henge portal geology for the first time.
		// Select a destination solar system in this sector that is consistent even if it decays and least likely to overlap
		// the destination of another ringworld.
		AuIdList systemsInSector;
		sector->ListSubatomIds(systemsInSector);
		int myidx = systemsInSector.indexOf(system->Id());
		if(myidx < 0)
			return;				// My system is not in the sector?

		int destIdx = myidx + fromArcSegment + 1;
		if(destIdx >= systemsInSector.count())
		{
			// Wrap around.
			destIdx = destIdx % systemsInSector.count();
			if(destIdx >= myidx)
				return;			// Wrapped around past my own system index. This guy goes nowhere.
		}

		// Generate information about the henge portal that will be created on this ring section.
		AuId destSysId = systemsInSector[destIdx];
		quint8 destSun = 0;								// From a ringworld, the destination portal world always orbits the primary.
		quint8 destShadowPanels = fromShadowPanels;		// Dest on globe world shows my shadow panel count.
		quint8 destArcSegment = 0;						// If the destination happens to be a ringworld, arc segment 0 works best.
		AuDice dice;
		dice.Seed(GenSeed() + 1000);
		AuGlobeLLF ll = RandomLandLLNotForbidden(dice, FeetPerMeter()*2, 200*FeetPerMeter());	// At least 2 meters up because we sink it 1m into the ground.
		float altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
		if(altMSL < 2*(float)FeetPerMeter())
			altMSL = 2*(float)FeetPerMeter();
		float topMSL = altMSL - (float)FeetPerMeter();	// 1m below terrain level.

		// Create the henge geological feature.
		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		AuGeoHenge* geo = new AuGeoHenge(destSysId, destSun, destShadowPanels, destArcSegment, topMSL, ll);
		geo->SetActive(true);	// Activate the new henge portal.
		geof.append(geo);
		UnlockGeologicalFeatures();
		m_HengePresence = 1;

		destSysIds.append(destSysId);
		destSuns.append(destSun);
		destArcs.append(destArcSegment);

		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug() << "DaWorldRingSection::ActivateHenges" << Id() << Name() << "henge created at" << ll;
	}

	GeologyChanged();
	SetModified();

	// Send a signal to activate the destination henge(s).
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "DaWorldRingSection::ActivateHenges" << Id() << Name() << "signaling" << destSysIds.count() << "dest systems";
	AuIdList::const_iterator di = destSysIds.cbegin();
	QList<quint8>::const_iterator ds = destSuns.cbegin();
	QList<quint8>::const_iterator da = destArcs.cbegin();
	while(di != destSysIds.cend())
	{
		emit HengeActivatedSignal(fromSysId, fromSun, fromShadowPanels, fromArcSegment, *di, *ds, *da);

		di++;
		ds++;
		da++;
	}
}

// Creates the henge portal on this arc segment if none exists.
// This signals the interconnecting system to create the destination henge portal even if mine already exists.
void DaWorldRingSection::CreateHenges()
{
	const DaStar* star = FindParentStarInEnv();
	if(!star)
		return;
	const DaSolarSystem* system = star->FindParentSolarSystem();
	if(!system)
		return;
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;

	// Get information about my end of the connection, to be sent to the destination system.
	AuId fromSysId = system->Id();
	int sn = star->StarNumber();
	quint8 fromSun;
	if(sn <= 0)
		fromSun = 0;	// My sun is the primary.
	else
		fromSun = sn;	// My sun is secondary or tertiary.
	quint8 fromShadowPanels = RingworldSections();
	quint8 fromArcSegment = SectionIndex();
	//if(g_Au.m_LogMessages & LOG_Travel)
		//qDebug() << "DaWorldRingSection::CreateHenges" << Id() << Name() << "sys" << fromSysId << "sun" << fromSun
			//<< "panels" << fromShadowPanels << "arc" << fromArcSegment;

	// Locate an existing henge portal.
	AuIdList destSysIds;
	QList<quint8> destSuns;
	QList<quint8> destArcs;
	const AuGeologicalFeatures& geof = LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
	{
		const AuGeoHenge* henge = dynamic_cast<const AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		destSysIds.append(henge->DestSysId());
		destSuns.append(henge->DestSun());
		destArcs.append(henge->DestArcSegment());
	}
	UnlockGeologicalFeatures();

	if(destSysIds.isEmpty())
	{
		// I do not have any valid henge portals.
		// Create my henge portal geology for the first time.
		// Select a destination solar system in this sector that is consistent even if it decays and least likely to overlap
		// the destination of another ringworld.
		AuIdList systemsInSector;
		sector->ListSubatomIds(systemsInSector);
		int myidx = systemsInSector.indexOf(system->Id());
		if(myidx < 0)
			return;				// My system is not in the sector?

		int destIdx = myidx + fromArcSegment + 1;
		if(destIdx >= systemsInSector.count())
		{
			// Wrap around.
			destIdx = destIdx % systemsInSector.count();
			if(destIdx >= myidx)
				return;			// Wrapped around past my own system index. This guy goes nowhere.
		}

		// Generate information about the henge portal that will be created on this ring section.
		AuId destSysId = systemsInSector[destIdx];
		quint8 destSun = 0;								// From a ringworld, the destination portal world always orbits the primary.
		quint8 destShadowPanels = fromShadowPanels;		// Dest on globe world shows my shadow panel count.
		quint8 destArcSegment = 0;						// If the destination happens to be a ringworld, arc segment 0 works best.
		AuDice dice;
		dice.Seed(GenSeed() + 1000);
		AuGlobeLLF ll = RandomLandLLNotForbidden(dice, FeetPerMeter()*2, 200*FeetPerMeter());	// At least 2 meters up because we sink it 1m into the ground.
		float altMSL = GeologicalTerrainAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
		if(altMSL < 2*(float)FeetPerMeter())
			altMSL = 2*(float)FeetPerMeter();
		float topMSL = altMSL - (float)FeetPerMeter();	// 1m below terrain level.

		// Create the henge geological feature.
		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		AuGeoHenge* geo = new AuGeoHenge(destSysId, destSun, destShadowPanels, destArcSegment, topMSL, ll);
		geof.append(geo);
		UnlockGeologicalFeatures();
		m_HengePresence = 1;

		destSysIds.append(destSysId);
		destSuns.append(destSun);
		destArcs.append(destArcSegment);

		//if(g_Au.m_LogMessages & LOG_Travel)
			//qDebug() << "DaWorldRingSection::CreateHenges" << Id() << Name() << "henge created at" << ll;

		GeologyChanged();
		SetModified();
	}

	// Send a signal to create the destination henge(s).
	//if(g_Au.m_LogMessages & LOG_Travel)
		//qDebug() << "DaWorldRingSection::CreateHenges" << Id() << Name() << "signaling" << destSysIds.count() << "dest systems";
	AuIdList::const_iterator di = destSysIds.cbegin();
	QList<quint8>::const_iterator ds = destSuns.cbegin();
	QList<quint8>::const_iterator da = destArcs.cbegin();
	while(di != destSysIds.cend())
	{
		emit HengeCreatedSignal(fromSysId, fromSun, fromShadowPanels, fromArcSegment, *di, *ds, *da);

		di++;
		ds++;
		da++;
	}
}

// Deactivates the henge portal(s) on this arc segment.
// This signals the interconnecting system to deactivate the destination henge portals.
void DaWorldRingSection::DeactivateHenges()
{
	const DaStar* star = FindParentStarInEnv();
	if(!star)
		return;
	const DaSolarSystem* system = star->FindParentSolarSystem();
	if(!system)
		return;

	// Deactivate all my henge portals.
	AuIdList destSysIds;
	QList<quint8> destSuns;
	QList<quint8> destArcs;
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	for(AuGeologicalFeatures::iterator gi = geof.begin(); gi != geof.end(); gi++)
	{
		AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		// Activate my henge portal.
		henge->SetActive(false);
		destSysIds.append(henge->DestSysId());
		destSuns.append(henge->DestSun());
		destArcs.append(henge->DestArcSegment());
	}
	UnlockGeologicalFeatures();

	if(destSysIds.isEmpty())
		return;

	GeologyChanged();
	SetModified();

	// Get information about my end of the connection, to be sent to the destination system.
	AuId fromSysId = system->Id();
	int sn = star->StarNumber();
	quint8 fromSun;
	if(sn <= 0)
		fromSun = 0;	// My sun is the primary.
	else
		fromSun = sn;	// My sun is secondary or tertiary.
	quint8 fromShadowPanels = RingworldSections();
	quint8 fromArcSegment = SectionIndex();

	// Send a signal to activate the destination henge.
	AuIdList::const_iterator di = destSysIds.cbegin();
	QList<quint8>::const_iterator ds = destSuns.cbegin();
	QList<quint8>::const_iterator da = destArcs.cbegin();
	while(di != destSysIds.cend())
	{
		emit HengeDeactivatedSignal(fromSysId, fromSun, fromShadowPanels, fromArcSegment, *di, *ds, *da);

		di++;
		ds++;
		da++;
	}
}

void DaWorldRingSection::ActivateHengeContaining(const AuPoint& loc)
{
	if(!HasHenge())
		return;

	const DaStar* star = FindParentStarInEnv();
	if(!star)
		return;
	const DaSolarSystem* system = star->FindParentSolarSystem();
	if(!system)
		return;

	quint8 destArcSegment = 0;
	quint8 destSun = 0;
	AuId destSysId;

	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	for(AuGeologicalFeatures::const_iterator gi = geof.begin(); gi != geof.end(); gi++)
	{
		AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
		if(!henge)
			continue;

		AuPoint hengeLoc = LocOfLL(henge->LL(), henge->CentralSlabAltitudeMSL());
		if(hengeLoc.Distance(loc) < henge->CentralSlabRadius())
		{
			if(!henge->IsActive())
			{
				henge->SetActive(true);
				destArcSegment = henge->DestArcSegment();
				destSun = henge->DestSun();
				destSysId = henge->DestSysId();
			}
			break;
		}
	}
	UnlockGeologicalFeatures();

	if(!destSysId.IsNull())
	{
		GeologyChanged();
		SetModified();

		// Get information about my end of the connection, to be sent to the destination system.
		AuId fromSysId = system->Id();
		int sn = star->StarNumber();
		quint8 fromSun;
		if(sn <= 0)
			fromSun = 0;	// My sun is the primary.
		else
			fromSun = sn;	// My sun is secondary or tertiary.
		quint8 fromShadowPanels = RingworldSections();
		quint8 fromArcSegment = SectionIndex();

		emit HengeActivatedSignal(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
	}
}

bool DaWorldRingSection::IsHengeTeleportLocation(const AuPoint& loc, AuId& fromSysId, quint8& fromSun, quint8& fromArc,
		quint8& shadowPanels, AuId& destSysId, quint8& destSun, quint8& destArc) const
{
	// Base class looks for the active henge teleporter and gives us back the 'dest' values and the 'shadowPanels'.
	if(!DaWorldLL::IsHengeTeleportLocation(loc, fromSysId, fromSun, fromArc, shadowPanels, destSysId, destSun, destArc))
		return false;

	// Provide my from information since there's an active henge teleporter there.
	const DaStar* star = FindParentStarInEnv();
	if(!star)
		return false;	// wut?
	const DaSolarSystem* system = star->FindParentSolarSystem();
	if(!system)
		return false;	// wut?

	fromSysId = system->Id();
	int sn = star->StarNumber();
	if(sn <= 0)
		fromSun = 0;	// My sun is the primary.
	else
		fromSun = sn;	// My sun is secondary or tertiary.
	fromArc = SectionIndex();

	return true;
}

void DaWorldRingSection::SystemScanResources(QMultiMap<quint8, COMMODITYID>& atmRes, QMultiMap<quint8, COMMODITYID>& hydRes,
		QMultiMap<quint8, COMMODITYID>& geoRes, QMultiMap<quint8, COMMODITYID>& bioRes) const
{
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		AuZoneResource res;
		QueryNaturalResources(res, rz);

		if(rz == 0)
		{
			// These resources exist in only a single planet-wide zone.
			// There is no need to repeat the information in multiple zones.

			// Atmosphere
			COMMODITYID atmres[] =
			{
				CID_Air,
				CID_AntifluxParticles,
				CID_BorexinoPrecipitate,
				CID_Cryozine,
				CID_Hydrogen,
				CID_Ioplasma,
			};
			int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
			ReportZoneFetchResources(res, atmres, atmrescount, atmRes);

			// Hydrosphere
			COMMODITYID hydres[] =
			{
				CID_Hydrographics,
				CID_Magmex,
				CID_Myrathane,
				CID_Flomentum,
			};
			int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
			ReportZoneFetchResources(res, hydres, hydrescount, hydRes);
		}

		// Remaining resources can exist in multiple zones.
		// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
		// To do this, we'll report all of them and keep count of the total probability.
		// At the end, we'll remove the ones that ended up with zero total probability.

		// Biosphere
		{
			COMMODITYID biores[] =
			{
				CID_Animal,
				CID_Beans,
				CID_Cheese,
				CID_Eggs,
				CID_Fertilizer,
				CID_Fiber,
				CID_Fish,
				CID_Fruit,
				CID_Grain,
				CID_Grapes,
				CID_Herbs,
				CID_Hops,
				CID_Milk,
				CID_Nuts,
				CID_Spices,
				CID_Vegetable,
				CID_Vegetation,
				CID_Wood,
			};
			int biorescount = sizeof(biores)/sizeof(COMMODITYID);
			ReportZoneBioResources(res, biores, biorescount, bioRes);
		}

		// Geosphere
		COMMODITYID geores[] =
		{
			CID_Bolite,
			CID_Coal,
			CID_CrudeOil,
			CID_Crystals,
			CID_Eludium,
			CID_Gems,
			CID_GoldOre,
			CID_Ice,
			CID_Lumenite,
			CID_Minerals,
			CID_NaturalGas,
			CID_Ore,
			CID_Phlogiston,
			CID_Polytaride,
			CID_Radioactives,
			CID_Stone,
			CID_Vulcanite,
			CID_Adamantite,
			CID_Viathol,
		};
		int georescount = sizeof(geores)/sizeof(COMMODITYID);
		ReportZoneFetchResources(res, geores, georescount, geoRes);
	}
}

quint8 DaWorldRingSection::NaturalResourceQuality(COMMODITYID cid, int resourceZone) const
{
	quint8 q = 0;
	m_ResourcesLock.lockForRead();
	if(m_Resources)
		q = m_Resources[resourceZone].Quality(cid);
	m_ResourcesLock.unlock();
	if(q)
		return q;

	AuZoneResource res;
	QueryNaturalResources(res, resourceZone);
	return res.Quality(cid);
}

bool DaWorldRingSection::FetchConstructionMaterials(DeConstructionProcess& cp, const DaBd* forBuilding)
{
	bool anyFetched = DaWorldLL::FetchConstructionMaterials(cp, forBuilding);
	if(cp.SupplyAllPower())
		anyFetched = true;
	return anyFetched;
}

bool DaWorldRingSection::FetchManufacturingMaterials(DeManufacturingProcess& mp, const AuPoint& loc, bool isDark, INDUSTRYID iid,
		bool isUndersea, bool inEnclosure, quint8& elecSubQ, quint8& waterSubQ)
{
	bool fetched = DaWorldLL::FetchManufacturingMaterials(mp, loc, isDark, iid, isUndersea, inEnclosure, elecSubQ, waterSubQ);
	if(mp.SupplyAllPower(elecSubQ))
		fetched = true;
	return fetched;
}

void DaWorldRingSection::RenderScWorld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const DaPerson* sceneAtom)
{
	if(ClaimedByEmpire())
		CreateClaimVAOs(*ari.re);

	// Get current ambient light and fog states.
	float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
	bool sunsBlocked;
	SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);

	// Ringworld shadow panel shadow is desirable when at a ringworld.
	// Eye location is needed by ringworld shaders.
	AuGLSLScBase* shader;
	if(fogDensity > 0)
	{
		shader = &ari.re->GLSL().ScRingworldFogShader();
		shader->Select(projection, *ari.re);
		ari.re->GLSL().ScRingworldFogShader().SetEyeLocation(model.UnProject(ari.re->EyeLocation()));
		ari.re->GLSL().ScRingworldFogShader().SetFog(fogDensity, fogR, fogG, fogB);
	}
	else
	{
		shader = &ari.re->GLSL().ScRingworldShader();
		shader->Select(projection, *ari.re);
		ari.re->GLSL().ScRingworldShader().SetEyeLocation(model.UnProject(ari.re->EyeLocation()));
	}

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		shader->SetSunEnabled(sunIndex, sun.m_Enabled && !sunsBlocked);
		shader->SetSunShadowAngle(sunIndex, ShadowPanelSunAngle(sun.m_ShadowAngle));
	}

	AuTexturePalette texNames;
	texNames[0] = m_AsphaltTex;
	texNames[1] = m_ConcreteTex;
	texNames[2] = m_ClaimPoleTex;
	texNames[3] = m_ClaimFlagTex;

	shader->SetModelView(model);
	shader->SetSurfaceTextures(texNames);
	shader->SetGlowDiscard(0,0,0, .01f);
	shader->SetVertAlpha(1);
	shader->SetAmbientLight(ambientR, ambientG, ambientB);

	RenderSc_Shader(shader, projection, model, ari, sceneAtom);

	// Restore suns.
	if(sunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			shader->SetSunEnabled(sunIndex, sun.m_Enabled);
		}

	shader->Deselect();
}

void DaWorldRingSection::RenderSc_Shader(AuGLSLScBase* shader, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, const DaPerson* sceneAtom)
{
	DaWorldLL::RenderSc_Shader(shader, projection, model, ari, sceneAtom);
	if(ClaimedByEmpire() && m_ClaimPoleVAO.VAO())
	{
		// Get wind force and direction from planet's weather model or enclosure.
		QPointF wind = WindAt(sceneAtom->GlobeLL());
		float windDir = 0;
		float windForce = 1;
		if(wind.x() != 0.f || wind.y() != 0.f)
		{
			AuPoint w(wind.x(), wind.y());
			windDir = w.ArcTangent_2D();
			windForce = w.Length();	// miles per hour
			windForce = sqrtf(windForce / 100);
			if(windForce <= 0 || windForce > 1)
				windForce = 1;
		}

		// Calculate wind vector.
		AuPoint loc = GetSurfaceLocation(ClaimedAtLL());
		AuPoint vup = -GravityDir(loc);
		AuPoint vnorth(0,0,1);
		AuPoint veast = vnorth.Cross(vup);
		veast.Unit(windForce);			// Wind vec pointing east.
		AuMatrix4x4 tWind;
		if(windDir != 0)
			tWind.RotateAxis(windDir, vup);
		AuPoint vwind = tWind.Project(veast);
		vwind.Unit();
		AuPoint vcrosswind = vup.Cross(vwind);
		vcrosswind.Unit();
		AuMatrix4x4 poleM(loc, vwind, vcrosswind);
		poleM.RotateZ(PI/2);

		AuMatrix4x4 mpM = model * poleM;
		AuPoint ambientUp = mpM.ProjectVector(0,0,1);
		ambientUp.Unit();
		shader->SetAmbientLightUp(ambientUp);
		shader->SetModelView(mpM);

		m_ClaimPoleVAO.SelectGL();
		m_ClaimPoleVAO.DrawGL();
		m_ClaimPoleVAO.DeselectGL();
	}
}

int DaWorldRingSection::AirTravellerInterval(qint8 morale) const
{
	// Population does not increase when overpopulated.
	if(WorldPopulation() > WorldPopulationMax())
		return -1;							// Someone wants to leave immediately.
	// Nominal rate is five air travellers per day, at morale 1, one per city report cycle.
	int arrivalsPerDay;						// Morale affects willingness to travel to/from here.
	if(morale == 0)
		arrivalsPerDay = 1;
	else
		arrivalsPerDay = 5 * morale;
	int arrivalMins = (24*60) / UDaysPerRealDay() / arrivalsPerDay;
	if(arrivalMins == 0)
	{
		if(morale > 0)
			arrivalMins = 1;
		else if(morale < 0)
			arrivalMins = -1;
	}
	if(arrivalMins > 0 && WorldPopulation() == WorldPopulationMax())
		return 0;							// Population does not increase.
	return arrivalMins;						// Negative if people are moving away.
}

int DaWorldRingSection::IndigenousPopulationInterval(qint8 morale) const
{
	if(!HasWorkerSpecies())
		return 0;
	// Population does not increase when overpopulated.
	if(WorldPopulation() > WorldPopulationMax())
		return 0;
	int indPerDay;
	if(morale == 0)
		indPerDay = 1;
	else
		indPerDay = 6 * morale;				// Produce 6 migrants per day, at morale 0.
	int indIntervalMins = (24*60) / UDaysPerRealDay() / indPerDay;
	if(indIntervalMins == 0)
	{
		if(morale > 0)
			indIntervalMins = 1;
		else if(morale < 0)
			indIntervalMins = -1;
	}
	if(indIntervalMins > 0 && WorldPopulation() == WorldPopulationMax())
		return 0;							// Population does not increase but it can decrease.
	return indIntervalMins;					// Negative if people are moving away.
}

void DaWorldRingSection::SetRotationAngleOffset(double rotAngleOffset)
{
	DaStar* star = dynamic_cast<DaStar*>(Parent());
	if(!star)
	{
		// No sun? Set only my angle offset.
		SetArcRotationAngleOffset(rotAngleOffset);
		return;
	}

	DaAtomList subs;
	star->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldRingSection* rs = dynamic_cast<DaWorldRingSection*>(*it);
		if(rs)
			rs->SetArcRotationAngleOffset(rotAngleOffset);
	}
}

quint64 DaWorldRingSection::MemoryUsage() const
{
	quint64 mem = DaWorldLL::MemoryUsage();
	mem += sizeof(DaWorldRingSection) - sizeof(DaWorldLL);
	mem += sizeof(DePlanetFauna) * ResourceZones();
	mem += sizeof(DePlanetFlora) * ResourceZones();
	mem += sizeof(AuZoneResource) * ResourceZones();
	return mem;
}

quint32 DaWorldRingSection::BuildingVolumeLimit() const
{
	quint32 maxPop = WorldPopulationMax();
	return AuCityReport::WorldBuildingLimit(maxPop, true);
}

DaWorldRing* DaWorldRingSection::FindRing(const AuPoint& searchLoc) const
{
	Q_UNUSED(searchLoc);
	return 0;
}

bool DaWorldRingSection::UpdateEnclosureCache() const
{
	if(!DaWorldLL::UpdateEnclosureCache())
		return false;		// Number of Enclosures did not change.

	// Trigger regen of rocks and plants.
	m_FloraViewer = AuPoint(0,0);
	m_RockViewer = AuPoint(0,0);
	InvalidateTerrainInfoSprites();
	InvalidateEnvironmentRecursive();
	return true;
}

bool DaWorldRingSection::SightSeaSurface(const AuPoint& loc1, const AuMatrix4x4& model, AuPoint& hitLoc) const
{
	Q_UNUSED(loc1);
	Q_UNUSED(model);
	Q_UNUSED(hitLoc);
	return false;	// Probably not needed.
}

DaAtom::ATOM_TYPE DaWorldRingSection::AtomType() const
{
	return ATOM_WorldRingSection;
}

void DaWorldRingSection::operator>>(QDataStream& out) const
{
	DaWorldLL::operator>>(out);
	quint8 verAtm = 0x00 | m_AtmType;
	out << m_byAtmDensity;
	out << verAtm;
	out << m_byWater;
	out << m_ControlCenterActive;
	out << m_SectionIndex;
	out << m_WindDir;
	out << m_WindForce;

	m_RingSectionLock.lockForRead();
	out << m_AtmColor;
	out << m_Population;
	if(m_pSentient)
	{
		out << (quint8)1;
		out << *m_pSentient;
	}
	else
		out << (quint8)0;
	out << m_RotationAngle;
	out << m_RotationAngleOffset;
	out << m_SectionAngle;

	if(m_pFauna)
	{
		out << (quint8)1;
		out << (quint8)ResourceZones();
		for(int z=0; z<ResourceZones(); z++)
			out << m_pFauna[z];
	}
	else
		out << (quint8)0;
	if(m_pFlora)
	{
		out << (quint8)1;
		out << (quint8)ResourceZones();
		for(int z=0; z<ResourceZones(); z++)
			out << m_pFlora[z];
	}
	else
		out << (quint8)0;
	out << m_tmFloraChange;

	out << m_TerrainFrequency;
	out << m_TerrainPersFactor;
	out << m_TerrainPersistence;
	out << m_TerrainXOff;
	out << m_TerrainYOff;
	out << m_TerrainWaterFactor;

	m_RingSectionLock.unlock();
}

void DaWorldRingSection::operator<<(QDataStream& in)
{
	DaWorldLL::operator<<(in);
	in >> m_byAtmDensity;
	quint8 verAtm;
	in >> verAtm;
	m_AtmType = (ATMTYPE)(verAtm & 0x0f);
	in >> m_byWater;
	in >> m_ControlCenterActive;
	in >> m_SectionIndex;
	in >> m_WindDir;
	in >> m_WindForce;

	m_RingSectionLock.lockForWrite();
	in >> m_AtmColor;
	in >> m_Population;
	if(m_pSentient)
		delete m_pSentient;
	quint8 ui8;
	in >> ui8;
	if(ui8)
	{
		m_pSentient = new AuSpecies;
		in >> *m_pSentient;
	}
	else
		m_pSentient = 0;
	in >> m_RotationAngle;
	in >> m_RotationAngleOffset;
	in >> m_SectionAngle;

	if(m_pFauna)
		delete [] m_pFauna;
	in >> ui8;
	if(ui8)
	{
		quint8 zones;
		in >> zones;
		Q_ASSERT(zones == ResourceZones());
		m_pFauna = new DePlanetFauna[zones];
		for(int z=0; z<zones; z++)
			in >> m_pFauna[z];
	}
	else
		m_pFauna = 0;
	if(m_pFlora)
		delete [] m_pFlora;
	in >> ui8;
	if(ui8)
	{
		quint8 zones;
		in >> zones;
		Q_ASSERT(zones == ResourceZones());
		m_pFlora = new DePlanetFlora[zones];
		for(int z=0; z<zones; z++)
			in >> m_pFlora[z];
	}
	else
		m_pFlora = 0;
	in >> m_tmFloraChange;

	in >> m_TerrainFrequency;
	in >> m_TerrainPersFactor;
	in >> m_TerrainPersistence;
	in >> m_TerrainXOff;
	in >> m_TerrainYOff;
	in >> m_TerrainWaterFactor;

	m_ThinkRotation = m_RotationAngle;
	m_ThinkTransform = Transform();
	m_RingSectionLock.unlock();
}

bool DaWorldRingSection::HitByAsteroid(quint8 asteroidSize, const AuPoint& asteroidLoc)
{
	DestroyCities();		// CLANG!

	// Make a crater if the hit is on the inside.
	AuGlobeLLF llf;
	if(LLOfLoc(llf, asteroidLoc) && AltitudeMSL(asteroidLoc) > 0)
	{
		// This hit is on the inside. Make a crater.
		float craterRad = DaAsteroid::ObstacleRadius(asteroidSize);
		float debrisHeight = g_Au.Dice().Roll(1,craterRad/5,1);
		float bottomMSL = -craterRad/2 - g_Au.Dice().Roll(1,100,50) * FeetPerMeter();	// Size 1 asteroid is 1250' radius.
		if(bottomMSL < MinRadius())
			bottomMSL = MinRadius();
		AuGeoCrater* geo = new AuGeoCrater(craterRad, debrisHeight, bottomMSL, llf);
		AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
		geof.append(geo);
		UnlockGeologicalFeatures();
		GeologyChanged();
	}
	return false;			// The ring itself is not harmed even by the largest asteroid.
}

bool DaWorldRingSection::HasWorkerSpecies() const
{
	m_RingSectionLock.lockForRead();
	bool v = m_pSentient;
	m_RingSectionLock.unlock();
	return v;
}

void DaWorldRingSection::SetWorkerSpecies(const AuSpecies& species)
{
	m_RingSectionLock.lockForWrite();
	if(!m_pSentient)
		m_pSentient = new AuSpecies(species);
	else
		*m_pSentient = species;
	if(m_pSentient->IsNull())
	{
		delete m_pSentient;
		m_pSentient = 0;
	}
	m_RingSectionLock.unlock();
}

AuSpecies DaWorldRingSection::WorkerSpecies() const
{
	AuSpecies s;
	m_RingSectionLock.lockForRead();
	if(m_pSentient)
		s = *m_pSentient;
	m_RingSectionLock.unlock();
	return s;
}

bool DaWorldRingSection::IsHabitable(HABITABILITY hab) const
{
	switch(hab)
	{
	case HABITABLE_Unspecified:
		return IsHabitableOrbit();
	case HABITABLE_System:
		return true;
	case HABITABLE_HomeWorld:
		break;
	case HABITABLE_Ringworld:
		return true;
	case HABITABLE_EmptySystem:
		break;
	}
	return false;
}

COMMODITYID DaWorldRingSection::GuaranteedRockAt(const AuGlobeLLF& rockLL, int resourceZone) const
{
	COMMODITYID cid;
	m_ResourcesLock.lockForRead();
	if(m_Resources)
		cid = m_Resources[resourceZone].GuaranteedRockAt(rockLL);
	else
		cid = CID_Invalid;
	m_ResourcesLock.unlock();
	return cid;
}

AuGlobeLLF DaWorldRingSection::GuaranteedRockLocationResLocked(const AuDice& Cdice, int eqDivs, int resourceZone) const
{
	AuDice dice(Cdice);
	AuGlobeLLF rockLL;
	do
	{
		rockLL = RoundedGlobeLoc(RandomLL(dice), eqDivs);
		dice.Random();
	}
	while(m_Resources[resourceZone].GuaranteedRockAt(rockLL) != CID_Invalid);
	return rockLL;
}

void DaWorldRingSection::Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double radius)
{
	Q_UNUSED(avatarId);
	Q_UNUSED(empireId);
	Q_UNUSED(seaLevelLoc);
	Q_UNUSED(radius);
}

void DaWorldRingSection::ExploreAll(const AuId& avatarId, quint32 empireId)
{
	Q_UNUSED(avatarId);
	Q_UNUSED(empireId);
}

bool DaWorldRingSection::FogOfWar(const AuId& avatarId, quint32 empireId, AuFogOfWarData& fow) const
{
	Q_UNUSED(avatarId);
	Q_UNUSED(empireId);
	Q_UNUSED(fow);
	return false;
}

bool DaWorldRingSection::FogOfWarTest(const AuId& avatarId, quint32 empireId, const AuGlobeLL& ll) const
{
	Q_UNUSED(avatarId);
	Q_UNUSED(empireId);
	Q_UNUSED(ll);
	return true;
}

const QImage* DaWorldRingSection::SurfaceImage() const
{
	// Return the current surface map, if we have created one.
	if(m_Surface && m_Surface != (QImage*)-1)
		return m_Surface;

	// If the map has not been created, trigger creation of it now. We can't do it right now because we must wait until a
	// GL context is current.
	if(!m_Surface)
		m_Surface = (QImage*)-1;
	return 0;
}

GLuint DaWorldRingSection::SurfaceTexture() const
{
	if(m_glTexNearSurface)
		return m_glTexNearSurface;
	else
		return m_glTexFarSurface;
}

