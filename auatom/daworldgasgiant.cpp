//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldgasgiant.cpp,v 1.354 2022/05/17 18:27:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daworldgasgiant.h"
//
#include "auatomrenderinfo.h"
#include "aufilever.h"
#include "aufloradna.h"
#include "aufogofwar.h"
#include "auglmodelmaker.h"
#include "aunoise.h"
#include "auquad.h"
#include "ausensorblip.h"
#include "ausphere.h"
#include "auterraininfo.h"
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
#include "dasc.h"
#include "dateleporter.h"
#include "davehicle.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "defauna.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dprock.h"
#include "hashpack.h"
#include <QSqlError>
#include <QSqlQuery>

#define TERRAIN_TEXTURE_REPS			(5*30)

// These textures are used by all cities so they can be statically allocated.
static const float s_seaHardness = 150.f;	// Larger number makes a smaller smaller spot
static GLuint							s_TexGasGiant = 0;		// Texture strip used for gas giants.
static GLuint							s_TexNoise = 0;			// Random noise texture for more variations
static GLuint							s_TexRock = 0;			// Texture used for all developments for rocks.
static GLuint							s_texSea = 0;			// Sea surface textures by orbit zone.
static GLuint							s_texSeaN = 0;			// Sea surface bump maps by orbit zone.
static GLuint 							s_TexTerPalette[WORLDTEX_Count];	// Terrain texture tiles.

DaWorldGasGiant::DaWorldGasGiant()
{
	//m_AtmColor constructor sets it to 'invalid' state
	m_byAtmDensity = 0;
	m_AtmType = ATMTYPE_Standard;
	m_byCoreRadius = 0;
	m_FloraVAOsHigh = 0;
	m_FogOfWar = new AuFogOfWar;
	m_glTexFarSurface = 0;
	m_glTexGrass = 0;
	m_glTexNearSurface = 0;
	m_glTexNearValid = false;
	m_HengeColumn = 0;
	m_HengeFirePit = 0;
	m_HengeRock = 0;
	m_pSentient = 0;
	m_pFauna = 0;
	m_pFlora = 0;
	m_Population = (quint32)-1;
	m_Resources = 0;
	m_Radius1000km = 0;
	m_RockVAOs = 0;
	m_RotationAngle = 0;
	m_Surface = 0;
	m_TerrainFrequency = 0;
	m_TerrainPersFactor = 0;
	m_TerrainPersistence = 0;
	m_TerrainXOff = 0;
	m_TerrainYOff = 0;
	m_ThinkRotation = 0;
	m_ViewerNear = false;
	m_WaterOffX = 0;
	m_WaterOffY = 0;
	m_WindDir = 1000;	// Starting value is undetermined.
	m_WindForce = -1;	// Starting value is undetermined.
	m_WxSPOffset = 0;
	m_WxVPOffset = 0;
}

DaWorldGasGiant::~DaWorldGasGiant()
{
	/*
#if !defined(QT_NO_DEBUG)
	if(m_AtmosphereLayer[0].VAO())
		qDebug() << "DaWorldGasGiant::~DaWorldGasGiant - AtmosphereLayer0 =" << m_AtmosphereLayer[0].VAO();
	if(m_AtmosphereLayer[1].VAO())
		qDebug() << "DaWorldGasGiant::~DaWorldGasGiant - AtmosphereLayer1 =" << m_AtmosphereLayer[1].VAO();
	if(m_AtmosphereLayer[2].VAO())
		qDebug() << "DaWorldGasGiant::~DaWorldGasGiant - AtmosphereLayer2 =" << m_AtmosphereLayer[2].VAO();
	if(m_NearGlobe.VAO())
		qDebug() << "DaWorldGasGiant::~DaWorldGasGiant - NearGlobe =" << m_NearGlobe.VAO();
#endif
*/
	if(m_FogOfWar)
		delete m_FogOfWar;

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

DaWorldGasGiant::DaWorldGasGiant(const DaWorldGasGiant &rhs) : DaWorldSphere(rhs)
{
	//qDebug("DaWorldGasGiant:: copy constructor id %d", rhs.Id().GetID());
	m_byAtmDensity = rhs.m_byAtmDensity;
	m_AtmType = rhs.m_AtmType;
	m_byCoreRadius = rhs.m_byCoreRadius;
	m_Radius1000km = rhs.m_Radius1000km;
	m_WindDir = rhs.m_WindDir;
	m_WindForce = rhs.m_WindForce;
	m_WxSPOffset = rhs.m_WxSPOffset;
	m_WxVPOffset = rhs.m_WxVPOffset;

	rhs.m_GasGiantLock.lockForRead();
	m_AtmColor = rhs.m_AtmColor;
	m_Population = rhs.m_Population;
	if(rhs.m_pSentient)
		m_pSentient = new AuSpecies(*rhs.m_pSentient);
	else
		m_pSentient = 0;

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

	if(rhs.m_FogOfWar)
		m_FogOfWar = new AuFogOfWar(*rhs.m_FogOfWar);
	else
		m_FogOfWar = new AuFogOfWar;

	m_RotationAngle = rhs.m_RotationAngle;
	m_TerrainFrequency = rhs.m_TerrainFrequency;
	m_TerrainPersFactor = rhs.m_TerrainPersFactor;
	m_TerrainPersistence = rhs.m_TerrainPersistence;
	m_TerrainXOff = rhs.m_TerrainXOff;
	m_TerrainYOff = rhs.m_TerrainYOff;

	// Do not copy Think variables.
	m_ThinkRotation = 0;
	rhs.m_GasGiantLock.unlock();

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

	m_FloraVAOsHigh = 0;
	m_glTexFarSurface = 0;
	m_glTexGrass = 0;
	m_glTexNearSurface = 0;
	m_glTexNearValid = false;
	m_HengeColumn = 0;
	m_HengeFirePit = 0;
	m_HengeRock = 0;
	m_RockVAOs = 0;
	m_ViewerNear = false;
	m_WaterOffX = 0;
	m_WaterOffY = 0;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaWorldGasGiant::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaWorldSphere::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;
	if(lastTx.IsNull())
		hasWhat |= 0x01;
	if(lastTx.IsNull() || GeologyChangeTime() >= lastTx)
		hasWhat |= 0x03;	// Include terrain info too. This doesn't happen much.

	// Client does not receive the BaseHere bit, doesn't need it.
	if(BdHere())
		hasWhat |= 0x40;
	if(HQHere())
		hasWhat |= 0x80;

	AuId sceneId(sceneAtom.Id());
	quint32 empireId = sceneAtom.EmpireId();

	m_GasGiantLock.lockForRead();
	if(lastTx.IsNull() || m_FogOfWar->HasChanged(sceneId, empireId, lastTx))
		hasWhat |= 0x04;
	if(m_pSentient)
		hasWhat |= 0x08;
	if(tmLastUpdate.IsNull() || (!m_tmFloraChange.IsNull() && m_tmFloraChange >= tmLastUpdate))
		hasWhat |= 0x10;
	m_GasGiantLock.unlock();

	m_LLLock.lockForRead();
	if(tmLastUpdate.IsNull() || (!m_ClaimTime.IsNull() && m_ClaimTime >= tmLastUpdate))
		hasWhat |= 0x20;
	m_LLLock.unlock();

	toClient << hasWhat;

	if(hasWhat & 0x01)
	{
		toClient << m_Radius1000km;
		toClient << m_byCoreRadius;
		toClient << (quint8)m_AtmType;
		toClient << m_byAtmDensity;
		//toClient << m_RotationAngle;

		m_GasGiantLock.lockForRead();
		toClient << m_TerrainFrequency;
		toClient << m_TerrainPersFactor;
		toClient << m_TerrainPersistence;
		toClient << m_TerrainXOff;
		toClient << m_TerrainYOff;
		m_GasGiantLock.unlock();
	}

	if(hasWhat & 0x02)
	{
		toClient << LockGeologicalFeatures();
		UnlockGeologicalFeatures();
	}

	if(hasWhat & 0x04)
	{
		m_GasGiantLock.lockForRead();
		m_FogOfWar->Tx(toClient, sceneId, empireId);
		m_GasGiantLock.unlock();
	}

	if(hasWhat & 0x08)
	{
		m_GasGiantLock.lockForRead();
		toClient << *m_pSentient;
		m_GasGiantLock.unlock();
	}

	if(hasWhat & 0x10)
	{
		// This allows for farm and orchard plants in harsh environments.
		m_GasGiantLock.lockForRead();
		Q_ASSERT(m_pFlora);
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz].TxUpdate(toClient);
		m_tmFloraChange >> toClient;
		m_GasGiantLock.unlock();
	}

	if(hasWhat & 0x20)
	{
		m_LLLock.lockForRead();
		toClient << m_ClaimAtLL;
		toClient << m_ClaimEmpire;
		m_LLLock.unlock();
	}
}

// Receive updates to this atom from a server.
void DaWorldGasGiant::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaWorldSphere::RxUpdate(fromServer, isFirstUpdate);

	bool dirtyIrrigCache = false;
	quint8 hasWhat;
	fromServer >> hasWhat;
	if(hasWhat & 0x01)
	{
		fromServer >> m_Radius1000km;
		fromServer >> m_byCoreRadius;
		quint8 ui8;
		fromServer >> ui8;
		m_AtmType = (ATMTYPE)ui8;
		fromServer >> m_byAtmDensity;
		//fromServer >> m_RotationAngle;
		fromServer >> m_TerrainFrequency;
		fromServer >> m_TerrainPersFactor;
		fromServer >> m_TerrainPersistence;
		fromServer >> m_TerrainXOff;
		fromServer >> m_TerrainYOff;
	}

	if(hasWhat & 0x02)
	{
		//qDebug() << "DaWorldGasGiant::RxUpdate" << Id() << Name() << "geological features";
		fromServer >> LockGeologicalFeaturesForWrite();
		UnlockGeologicalFeatures();
		InvalidateHengePresence();
		//DumpGeo();
		dirtyIrrigCache = true;
	}

	if(hasWhat & 0x04)
	{
		m_GasGiantLock.lockForWrite();
		m_FogOfWar->Rx(fromServer);
		m_GasGiantLock.unlock();
	}

	if(hasWhat & 0x08)
	{
		if(!m_pSentient)
			m_pSentient = new AuSpecies;
		fromServer >> *m_pSentient;
	}

	if(hasWhat & 0x10)
	{
		// This allows for farm and orchard plants in harsh environments.
		if(!m_pFlora)
			m_pFlora = new DePlanetFlora[ResourceZones()];
		for(int rz=0; rz<ResourceZones(); rz++)
			m_pFlora[rz].RxUpdate(fromServer);
		m_tmFloraChange << fromServer;
	}

	if(hasWhat & 0x20)
	{
		m_LLLock.lockForWrite();
		fromServer >> m_ClaimAtLL;
		fromServer >> m_ClaimEmpire;
		m_LLLock.unlock();
	}

	// Client does not receive the base here bit, doesn't need it.
	m_BdBits = 0;
	if(hasWhat & 0x40)
		m_BdBits |= 0x04;
	if(hasWhat & 0x80)
		m_BdBits |= 0x10;

	if(dirtyIrrigCache)
		UpdateIrrigationCache();
}

// Refresh data from valid counterpart.
bool DaWorldGasGiant::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaWorldSphere::Refresh(pAtom, ari);

	const DaWorldGasGiant* pA = (const DaWorldGasGiant*)pAtom;
	m_byAtmDensity = pA->m_byAtmDensity;
	m_AtmType = pA->m_AtmType;
	m_Radius1000km = pA->m_Radius1000km;
	m_byCoreRadius = pA->m_byCoreRadius;
	m_BdBits = pA->m_BdBits;

	pA->m_GasGiantLock.lockForRead();
	m_GasGiantLock.lockForWrite();

	m_TerrainFrequency = pA->m_TerrainFrequency;
	m_TerrainPersFactor = pA->m_TerrainPersFactor;
	m_TerrainPersistence = pA->m_TerrainPersistence;
	m_TerrainXOff = pA->m_TerrainXOff;
	m_TerrainYOff = pA->m_TerrainYOff;
	//m_Resources = pA->m_Resources;
	//m_RotationAngle = pA->m_RotationAngle;
	delete m_FogOfWar;
	m_FogOfWar = new AuFogOfWar(*pA->m_FogOfWar);

	// Let's try following the same logic the vegetation uses.
	// If this atom has DNA data and the refresh atom has DNA
	// data then update the DNA data. Otherwise leave it alone.
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

	m_GasGiantLock.unlock();
	pA->m_GasGiantLock.unlock();

	// We do not refresh animal encounter tables due to the volume of data. The client
	// doesn't need the info anyway. The animal encounter tables are only used by the
	// server. If an animal appears, it will show up in the scene.

	return false;
}

DaAtom *DaWorldGasGiant::Clone() const
{
	return new DaWorldGasGiant(*this);
}

void DaWorldGasGiant::Create(const AuDice& Cdice, double orbitRadius, ORBITZONE orbitZone)
{
	AuDice dice(Cdice);
	DaWorldSphere::Create(orbitRadius, orbitZone);

	//qDebug("DaWorldGasGiant::Create");
	m_Radius1000km = dice.Roll(2, 11, 18);	// Creates size range [20-40].
	m_byAtmDensity = (unsigned char)(88 + dice.Roll(2, 6));	// Atmos density [90-100].
	int iroll = dice.Roll(2, 6);
	if (iroll <= 4)
		m_AtmType = ATMTYPE_Exotic;
	else if (iroll >= 9)
		m_AtmType = ATMTYPE_Insidious;
	else
		m_AtmType = ATMTYPE_Corrosive;

	InitCoreRadius(dice);
	dice.Random();
	CreateTerrainData(dice);			// Create the 3D terrain data.
	dice.Random();
	CreateGeologicalFeatures(dice);
	dice.Random();

	// All bodies that could have cities on them must have plant DNA so the farms and orchards can make plants.
	m_GasGiantLock.lockForWrite();
	if(!m_pFlora)
		m_pFlora = new DePlanetFlora[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFlora[rz].Init(m_Radius1000km*10, dice);
		dice.Random();
	}
	m_GasGiantLock.unlock();

	// Create animal encounter tables.
	if(CanHaveFauna())
	{
		InitAnimalEncounterTable(dice);
		dice.Random();
	}
}

void DaWorldGasGiant::InitAnimalEncounterTable(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	ORBITZONE orbitZone = OrbitZone();
	quint8 atmDensity = m_byAtmDensity;

	m_GasGiantLock.lockForWrite();
	if(m_pFauna)
		delete [] m_pFauna;
	m_pFauna = new DePlanetFauna[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFauna[rz].InitGasGiant(orbitZone, atmDensity, dice);
		dice.Random();
	}
	m_GasGiantLock.unlock();
}

void DaWorldGasGiant::CreateForScan(const AuDice& Cdice, double orbitRadius, ORBITZONE orbitZone)
{
	AuDice dice(Cdice);
	DaWorldSphere::CreateForScan(orbitRadius, orbitZone);

	//qDebug("DaWorldGasGiant::Create");
	m_Radius1000km = dice.Roll(2, 11, 18);	// Creates size range [20-40].
	m_byAtmDensity = (unsigned char)(88 + dice.Roll(2, 6));	// Atmos density [90-100].
	int iroll = dice.Roll(2, 6);
	if (iroll <= 4)
		m_AtmType = ATMTYPE_Exotic;
	else if (iroll >= 9)
		m_AtmType = ATMTYPE_Insidious;
	else
		m_AtmType = ATMTYPE_Corrosive;

	InitCoreRadius(dice);
	dice.Random();
	CreateTerrainData(dice);			// Create the 3D terrain data.
	dice.Random();
	CreateGeologicalFeatures(dice);
	dice.Random();

	// All bodies that could have cities on them must have plant DNA so the farms and orchards can make plants.
	m_GasGiantLock.lockForWrite();
	if(!m_pFlora)
		m_pFlora = new DePlanetFlora[ResourceZones()];
	for(int rz=0; rz<ResourceZones(); rz++)
	{
		m_pFlora[rz].Init(m_Radius1000km*10, dice);
		dice.Random();
	}
	m_GasGiantLock.unlock();

	// Create animal encounter tables.
	if(CanHaveFauna())
	{
		InitAnimalEncounterTable(dice);
		dice.Random();
	}
	//qDebug("DaWorldGasGiant::Create - done");
}

QString DaWorldGasGiant::DefaultName(const QString& solarSystemName) const
{
	// The name is empty so do the work to build the name.
	const DaStar* pstar = FindParentStarInEnv();
	if(!pstar)
		return StringId();

	// Develop a name based on this planets position in the solarsystem.

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

	// Get orbit radius so I can determine which position I hold around my parent.
	double myrad = OrbitRadius();

	// Determine which orbit position I am holding around my parent star.
	int ipos = 0;
	DaAtomList alplanets;
	pstar->ListPlanets(alplanets);
	for(DaAtomList::const_iterator it = alplanets.begin(); it != alplanets.end(); it++)
	{
		const DaWorld *pb = (const DaWorld*)(*it);
		if(pb->OrbitRadius() < myrad - (FeetPerOrbit()/2.0))
			ipos++;
	}
	return solarSystemName + squal + " " + RomanNumber(ipos+1);
}

// Report a longer description of this Atom for examination purposes.
QString DaWorldGasGiant::Description() const
{
	QString desc;

	// Planet nature and size.
	desc += tr("Gas Giant");

	// Include diameter...
	double dia = FROUND(SeaLevelRadius() * 2. / FeetPerMeter(), 100);
	QString sdia = QString(tr(" %L1m dia")).arg(dia);			//...in meters.
	desc += sdia;

	// Parent star name
	const DaStar* star = FindParentStarInEnv();
	if(star)
		desc += tr(", %1").arg(star->Name());

	// Orbit zone around parent star
	const QString ozones[] =
	{
		tr("Inferno zone"),
		tr("Inner zone"),
		tr("Habitable zone"),
		tr("Outer zone"),
		tr("Frigid zone")
	};
	desc += tr(", %1").arg(ozones[(int)OrbitZone()]);

	return desc;
}

void DaWorldGasGiant::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Draw orbit ring through planets, not moons or rings.
	// These are only drawn in the far pass.
	if(pass == PASS_Far)
	{
		if(sceneAtom->CanSeeOrbitRings() && ari.m_ShowHUD)
		{
			ari.re->GLSL().LineShader().Select();
			ari.re->GLSL().LineShader().SetProjectionModelView(projection * rmod.Matrix(ParentId()));

			// Ring color indicates solar output zone.
			switch(OrbitZone())
			{
			case OZONE_Inferno:
				ari.re->GLSL().LineShader().SetColor(QColor(96,0,0));
				break;
			case OZONE_Inner:
				ari.re->GLSL().LineShader().SetColor(QColor(72,72,0));
				break;
			case OZONE_Habitable:
				ari.re->GLSL().LineShader().SetColor(QColor(0,96,0));
				break;
			case OZONE_Outer:
				ari.re->GLSL().LineShader().SetColor(QColor(0,72,72));
				break;
			case OZONE_Frigid:
				ari.re->GLSL().LineShader().SetColor(QColor(48,48,48));
				break;
			case OZONE_DeepSpace:
				break;
			}

			// Compile a list of vertex locations.
			AuPoint poleVec = Transform().ProjectVector(0,0,FeetPerOrbit()/2);
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

	// Determine where the viewer is with respect to this planet.
	const AuPoint& pv = rmod.ViewLoc(Id());
	double viewDistance = pv.Length();
	bool bIsInAtm = viewDistance <= AtmosphereRadius();			// true if the viewer is inside my atmosphere.
	m_ViewerNear = viewDistance <= FeetPerOrbit()/2;		// true if all detail must be shown.

	bool cgUnderWater = false;
	bool eyeUnderWater = false;
	bool eyeUnderGround = false;	// true if the person's eye is below the ground surface
	bool lookingThroughWater = false;	// true when viewer's eye is in the water and not inside an enclosure or building.
	float fatmd = 0.f;			// atm density at viewer (.5 at sea level on Earth)
	float fillum = 0.f;			// solar illumination at viewer
	QColor colAtm(0,0,0);
	if(m_ViewerNear)
	{
		if(bIsInAtm)
		{
			// The viewer is inside my atmosphere radius, whatever the density.
			fatmd = sceneAtom->AtmosphereDensity();	// Adjusted for altitude.
			fillum = sceneAtom->Illumination();		// Adjusted for sea depth.
			colAtm = sceneAtom->ColorOfAtmosphere();
			cgUnderWater = sceneAtom->IsCGUnderWater();
			eyeUnderWater = sceneAtom->IsEyeUnderWater();
			eyeUnderGround = sceneAtom->IsUnderGround();
			lookingThroughWater = eyeUnderWater && !sceneAtom->IsInsideBd() && !sceneAtom->IsInsideEnclosure();
		}
	}

	// Render according to what the viewer perceives.
	DaSolarSystem* pss = FindParentSolarSystem();
	const float starLight = pss->StarLight();		// Actual starlight level based on local star density.
	if(bIsInAtm)
	{
		// The viewer is a person who is within atmosphere range, even if the density is zero.
		if(pass == PASS_Far)
			return;

		float atmillum = starLight + fillum*2.f;	// illumination of the atmosphere shell
		if(atmillum > 1)
			atmillum = 1;
		fillum += starLight;			// illumination at viewer
		float fogDensity = 0;			// Fog density for GL_EXP fog computation.
		float fogcol[4] = {0.f, 0.f, 0.f, 0.f};
		float florafogDensity = 0;		// Fog density for GL_EXP fog computation.
		float florafogcol[4] = {0.f, 0.f, 0.f, 0.f};
		if(!ari.IsViewTopDown())
		{
			if(lookingThroughWater)
			{
				// Viewer's eye is under water.
				// Setup atmospheric fog to present underwater environment.
				fogDensity = SeaFogDensity();

				QColor colSea(SeaColor());
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
				// Setup atmospheric fog based on atmosphere density.
				if(lookingThroughWater)
					fogDensity = SeaFogDensity();
				else
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
		// Ambient light is modulated to 50% so the noon-day sun doesn't wash out all detail.
		float ambientLight = fillum*.5f;

		AuPoint viewerPos;
		sceneAtom->GetLocationRelativeToParent(viewerPos, this);

		const AuMatrix4x4& model = rmod.Matrix(Id());
		if(eyeUnderWater)
		{
			// Render core globe model.
			RenderAtGlobe(projection, model, ari, viewerPos, ambientLight, fogDensity, fogcol[0], fogcol[1], fogcol[2]);

			// Render plants
			if(m_pFlora)
				RenderFlora(projection, model, *ari.re, ari.mm, viewerPos, ambientLight, florafogDensity, florafogcol[0],
						florafogcol[1], florafogcol[2]);

			// Render rocks.
			RenderRocks(projection, model, *ari.re, ari.mm, viewerPos, ambientLight, fogDensity, fogcol[0], fogcol[1], fogcol[2]);

			// Render henges
			RenderHenges(projection, model, ari, ambientLight, ambientLight, ambientLight, fogDensity, fogcol[0], fogcol[1],
					fogcol[2]);

			// Render claim flag.
			RenderScWorld(projection, model, ari, sceneAtom);
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
			// When eye is under water, we render the sea surface opaque. Atmosphere is not needed.
			RenderSea(projection, model, *ari.re, eyeUnderWater, cgUnderWater, fillum, ambientLight, fogDensity,
					fogcol[0], fogcol[1], fogcol[2], ari.IsViewTopDown());
		}
		else
		{
			// Eye is not under water.
			// Sea globe is not rendered when eye is under the surface of the ground.
			// Sea globe is not rendered when the polygon collisions look bad.
			if(ari.IsViewTopDown() || !eyeUnderGround)
				RenderSea(projection, model, *ari.re, eyeUnderWater, cgUnderWater, fillum, ambientLight, fogDensity,
						fogcol[0], fogcol[1], fogcol[2], ari.IsViewTopDown());

			if(!subs.empty())
			{
				// Render non-ring satellites and ships in orbit.
				DaAtomList rings;		// Save rings for last.
				for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
				{
					DaWorldGlobe* moon = dynamic_cast<DaWorldGlobe*>(*it);
					if(moon)
					{
						moon->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
								shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
						continue;
					}

					DaAsteroid* roid = dynamic_cast<DaAsteroid*>(*it);
					if(roid)
					{
						roid->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
								shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
						continue;
					}

					DaWorldRing* ring = dynamic_cast<DaWorldRing*>(*it);
					if(ring)
					{
						rings.append(ring);
						continue;
					}

					// Remaining atoms are not rendered when the viewer is under the ground surface.
					if(eyeUnderGround)
						continue;

					DaVessel* ship = dynamic_cast<DaVessel*>(*it);
					if(ship && !ship->IsInAtmosphere())
					{
						ship->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
								shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
						continue;
					}
				}

				// Render ONLY rings subordinate to this planet.
				for(DaAtomList::const_iterator it = rings.begin(); it != rings.end(); it++)
					(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
							shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
			}
		}

		// Render the atmosphere last because my parent gas giant, if I have one, is going to render its atmosphere last. If
		// I don't render mine after that, the gas giant's atmosphere is clipped by my atmosphere.
		// Don't render the atmosphere until we have a globe model.
		// Render terrain info sprites last.
		renderLast.Insert(rmod, this);
	}
	else if(m_ViewerNear)
	{
		// The viewer is outside my atmosphere but close enough to see planet details.
		if(rmod.RenderThisPass(pass, viewDistance))
		{
			// Render sea surface, viewer near atmos.
			RenderSea(projection, rmod.Matrix(Id()), *ari.re, false, false, starLight, starLight, 0, 0,0,0, false);

			// Render the atmosphere last due to transparency.
			renderLast.Insert(rmod, this);
		}

		// Render rings and moons and units outside the atmosphere that are subordinate to this planet.
		DaAtomList subs;
		ListSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			if((*it)->IsVisibleNearWorld())
				(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
						shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
	}
	else
	{
		// The viewer is outside my atmosphere.
		if(pv.z < 0 && rmod.RenderThisPass(pass, viewDistance))
		{
			// This planet is in front of the camera plane.
			// We don't need the globe itself at this distance.
			DaSolarSystem* pss = FindParentSolarSystem();
			float starLight = pss->StarLight();				// Actual starlight level based on local star density.
			RenderAtmosphere(projection, rmod.Matrix(Id()), *ari.re, 2, 0, starLight);
		}

		// Render rings and moons and units outside the atmosphere that are subordinate to this planet.
		DaAtomList subs;
		ListSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			if((*it)->IsVisibleFarWorld())
				(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog,
						shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
	}

	// Base atom class needs to render special effects.
	DaWorldSphere::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
	/*
	float fillum = 0.f;											// solar illumination at viewer
	if(bIsInAtm)
		fillum = sceneAtom->Illumination();						// Adjusted for sea depth.

	// Render according to what the viewer perceives.
	//bool isInView = true;
	if(bIsInAtm)
	{
		// The viewer is a person who is within atmosphere range, even if the density is zero.
		if(pass == PASS_Far)
			return;

		const DaAtomList* subatoms = LockSubatoms();
		if(subatoms)
		{
			// Render non-terrain atoms inside the atmosphere.
			for(DaAtomList::const_iterator it = subatoms->cbegin(); it != subatoms->cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(pAtom->DoesQueriedEnvironment() && pAtom->IsInAtmosphere())
					pAtom->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass);
			}
		}

		// Render this planet's model. (but save the atmosphere for later).
		const AuMatrix4x4& model = rmod.Matrix(Id());
		RenderGlobe(projection, model, *ari.re, fillum);	// Render with viewer in atmos.

		if(subatoms)
		{
			// Render non-ring subatoms.
			for(DaAtomList::const_iterator it = subatoms->cbegin(); it != subatoms->cend(); it++)
			{
				DaWorld* pAtom = dynamic_cast<DaWorld*>(*it);
				if(pAtom && !dynamic_cast<DaWorldRing*>(*it))
					pAtom->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass);
			}

			// Render ONLY rings subordinate to this planet so they get rendered last.
			for(DaAtomList::const_iterator it = subatoms->cbegin(); it != subatoms->cend(); it++)
			{
				DaWorldRing* pAtom = dynamic_cast<DaWorldRing*>(*it);
				if(pAtom)
					pAtom->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass);
			}
		}
		UnlockSubatoms();

		// Render the atmosphere last due to transparency.
		renderLast.Insert(rmod, this);
	}
	else if(m_ViewerNear)
	{
		// The viewer is outside my atmosphere but close enough to see planet details.
		if(rmod.RenderThisPass(pass, viewDistance))
		{
			// Render globe, viewer near atmos.
			const AuMatrix4x4& model = rmod.Matrix(Id());
			RenderGlobe(projection, model, *ari.re, fillum);

			// Render the atmosphere last due to transparency.
			renderLast.Insert(rmod, this);
		}

		// Render rings and moons and units outside the atmosphere that are subordinate to this planet.
		const DaAtomList* subatoms = LockSubatoms();
		if(subatoms)
			for(DaAtomList::const_iterator it = subatoms->cbegin(); it != subatoms->cend(); it++)
				if((*it)->IsVisibleNearWorld())
					(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass);
		UnlockSubatoms();
	}
	else
	{
		// The viewer is outside my atmosphere.
		if(pv.z < 0 && rmod.RenderThisPass(pass, viewDistance))
		{
			// This planet is in front of the camera plane.
			// We don't need the globe itself at this distance.
			const AuMatrix4x4& model = rmod.Matrix(Id());
			DaSolarSystem* pss = FindParentSolarSystem();
			float starLight = pss->StarLight();				// Actual starlight level based on local star density.
			RenderAtmosphere(projection, model, *ari.re, 2, starLight);
		}

		// Render rings and moons and units outside the atmosphere that are subordinate to this planet.
		const DaAtomList* subatoms = LockSubatoms();
		if(subatoms)
			for(DaAtomList::const_iterator it = subatoms->cbegin(); it != subatoms->cend(); it++)
				if((*it)->IsVisibleFarWorld())
					(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass);
		UnlockSubatoms();
	}

	// Base atom class needs to render special effects.
	DaWorldSphere::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass);
	*/
}

// Render atmosphere and weather.
// We "know" the scene atom is at this planet, either inside or outside the atmosphere.
void DaWorldGasGiant::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	const AuMatrix4x4& model = rmod.Matrix(Id());
	// Render this planet's atmosphere.
	DaSolarSystem* pss = FindParentSolarSystem();
	GLfloat starLight = pss->StarLight();
	if(sceneAtom->IsInAtmosphere() && sceneAtom->FindParentPlanetInEnv() == this)
	{
		// The scene atom is inside my atmosphere and their eye is above water or we wouldn't be rendering the atmosphere last.
		double viewerAlt = sceneAtom->CGAltitude();				// Viewer altitude above center of globe.
		float fillum = sceneAtom->Illumination();				// Adjusted for sea depth.
		float eyeUnderWater = sceneAtom->IsEyeUnderWater();
		float atmillum = starLight + fillum*2.f;				// illumination of the atmosphere shell
		if(atmillum > 1)
			atmillum = 1;
		else if(atmillum < 0)
			atmillum = 0;

		if(!eyeUnderWater)
			RenderAtmosphere(projection, model, *ari.re, 0, viewerAlt, atmillum);

		// Render smoke at volcanos and vents and bomb craters.
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

		// Render various overlays of information onto the terrain.
		if(ari.IsViewTopDown())
			RenderTerrainInfo(model, ari, sceneAtom);
	}
	else
	{
		// Render smoke at volcanos and vents and bomb craters.
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
		float eyeUnderWater = sceneAtom->IsEyeUnderWater();
		if(!eyeUnderWater)
			RenderAtmosphere(projection, model, *ari.re, 1, 0, starLight);
	}

	DaWorldSphere::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);
}

// Render the globe.
// Lights have already been set up.
// View and World transforms have already been set up.
void DaWorldGasGiant::RenderGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight)
{
	if(!m_NearGlobe.VAO())
		CreateGasGiantModel(re);

	if(!s_TexGasGiant)
		s_TexGasGiant = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "GasGiant", ":data/txGasGiants.png");

	re.GLSL().GasGiantShader().Select();
	re.GLSL().GasGiantShader().SetProjectionModelView(projection * model);
	re.GLSL().GasGiantShader().SetSurfaceTexture(s_TexGasGiant);

	float intensity = .5f;
	QColor colAtm = AtmosphereColor();
	GLfloat r = colAtm.redF() * intensity;
	GLfloat g = colAtm.greenF() * intensity;
	GLfloat b = colAtm.blueF() * intensity;
	re.GLSL().GasGiantShader().SetAtmosphereColor(r,g,b);

	// Set eye position in object space for specular lighting calculations.
	AuPoint eyeLoc = model.UnProject(0,0,0);
	re.GLSL().GasGiantShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

	// Ambient light
	re.GLSL().GasGiantShader().SetAmbientLight(ambientLight);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = re.Sun(sunIndex);
		AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

		// Configure shader for object space lighting.
		re.GLSL().GasGiantShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
		re.GLSL().GasGiantShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ,
				light.m_ColorR, light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
				light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
	}

	m_NearGlobe.SelectGL();
	m_NearGlobe.DrawGL();
	m_NearGlobe.DeselectGL();

	re.GLSL().GasGiantShader().Deselect();
}

// Render the atmosphere.
// Lights have already been set up.
// View and World transforms have already been set up.
// viewerContext: 0=inside, 1=outside nearby, 2=outside at a distance
void DaWorldGasGiant::RenderAtmosphere(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		int viewerContext, double viewerAlt, float atmIllumination)
{
	if(!s_TexGasGiant)
		s_TexGasGiant = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "GasGiant", ":data/txGasGiants.png");

	if(!m_AtmosphereLayer[2].VAO())
		m_AtmosphereLayer[2] = CreateGasGiantCloudModel(re, AuWeather::UpperLimit);

	re.GLSL().GasGiantLayerShader().Select();
	re.GLSL().GasGiantLayerShader().SetProjectionModelView(projection * model);
	re.GLSL().GasGiantLayerShader().SetSurfaceTexture(s_TexGasGiant);

	// Set eye position in object space for specular lighting calculations.
	AuPoint eyeLoc = model.UnProject(0,0,0);
	re.GLSL().GasGiantLayerShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = re.Sun(sunIndex);
		AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

		// Configure shader for object space lighting.
		re.GLSL().GasGiantLayerShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
		re.GLSL().GasGiantLayerShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ,
				light.m_ColorR, light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
				light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
	}

	QColor colAtm = AtmosphereColor();
	GLfloat r = colAtm.redF();
	GLfloat g = colAtm.greenF();
	GLfloat b = colAtm.blueF();
	re.GLSL().GasGiantLayerShader().SetAtmosphereColor(r,g,b);

	switch(viewerContext)
	{
	case 0:		// The viewer is inside the atmosphere.
		{
			re.GLSL().GasGiantLayerShader().SetAmbientLight(atmIllumination);

			// Cloud layers.
			if(!m_AtmosphereLayer[0].VAO())
				m_AtmosphereLayer[0] = CreateGasGiantCloudModel(re, AuWeather::LowClouds);

			if(!m_AtmosphereLayer[1].VAO())
				m_AtmosphereLayer[1] = CreateGasGiantCloudModel(re, AuWeather::MiddleClouds);

			// Alpha blending is enabled for transparent effect.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Additive blending is enabled for transparent effect that enhances suns.

			// Draw outer atmosphere shell.
			re.glFrontFace(GL_CCW);
			re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxSPOffset);	// fast upper layer

			float maxRad = AtmosphereTopGasGiantMSL() * .75;	// Highest layer radius above sea level.
			float midRad = AtmosphereTopGasGiantMSL() * .25;
			double seaRad = SeaLevelRadius();

			m_AtmosphereLayer[2].SelectGL();
			int shells = 5;
			double baseRad = seaRad + maxRad;
			double topRad = AtmosphereRadius();
			double spanRad = topRad - baseRad;
			for(int shell=shells; shell>=1; shell--)
			{
				double rad = baseRad + spanRad * shell / shells;
				re.GLSL().GasGiantLayerShader().SetAtmosphereRadius(rad);
				m_AtmosphereLayer[2].DrawGL();
			}
			m_AtmosphereLayer[2].DeselectGL();
			re.glFrontFace(GL_CW);

			// Gas giant cloud layer radii.
			double layerRadius[2];
			layerRadius[0] = seaRad;							// Low clouds
			layerRadius[1] = seaRad + maxRad;					// High clouds

			// Display cloud layers in low to high order that are below the viewer.
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for(int lyr=0; lyr<2; lyr++)
			{
				if(layerRadius[lyr] >= viewerAlt)
					continue;	// Skip layer that is above the viewer.

				if(lyr == 0)
				{
					re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxVPOffset);	// slow lower layer
					baseRad = seaRad;
					topRad = seaRad + midRad;
					shells = 5;
				}
				else
				{
					re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxSPOffset);	// fast upper layer
					baseRad = seaRad + midRad;
					topRad = seaRad + maxRad;
					shells = 10;
				}

				m_AtmosphereLayer[lyr].SelectGL();

				double spanRad = topRad - baseRad;
				for(int shell=1; shell<=shells; shell++)
				{
					double rad = baseRad + spanRad * shell / shells;
					re.GLSL().GasGiantLayerShader().SetAtmosphereRadius(rad);

					re.glFrontFace(GL_CCW);
					m_AtmosphereLayer[lyr].DrawGL();

					re.glFrontFace(GL_CW);
					m_AtmosphereLayer[lyr].DrawGL();
				}

				m_AtmosphereLayer[lyr].DeselectGL();
			}

			// Display cloud layers in high to low order that are above the viewer.
			re.glFrontFace(GL_CCW);
			for(int lyr=1; lyr>=0; lyr--)
			{
				if(layerRadius[lyr] <= viewerAlt)
					continue;	// Skip layer that is below the viewer.

				if(lyr == 0)
				{
					re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxVPOffset);	// slow lower layer
					baseRad = seaRad;
					topRad = seaRad + midRad;
					shells = 5;
				}
				else
				{
					re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxSPOffset);	// fast upper layer
					baseRad = seaRad + midRad;
					topRad = seaRad + maxRad;
					shells = 10;
				}

				m_AtmosphereLayer[lyr].SelectGL();

				double spanRad = topRad - baseRad;
				for(int shell=shells; shell>=1; shell--)
				{
					double rad = baseRad + spanRad * shell / shells;
					re.GLSL().GasGiantLayerShader().SetAtmosphereRadius(rad);
					m_AtmosphereLayer[lyr].DrawGL();
				}

				m_AtmosphereLayer[lyr].DeselectGL();
			}
			re.glFrontFace(GL_CW);

			re.glDisable(GL_BLEND);
		}
		break;
	case 1:		// viewer is outside the atmosphere nearby
		{
			if(!m_AtmosphereLayer[0].VAO())
				m_AtmosphereLayer[0] = CreateGasGiantCloudModel(re, AuWeather::LowClouds);

			if(!m_AtmosphereLayer[1].VAO())
				m_AtmosphereLayer[1] = CreateGasGiantCloudModel(re, AuWeather::MiddleClouds);

			// Viewer is not inside the atmosphere but can see the contents of the atmosphere.
			// Alpha blending is enabled for transparent effect.
			// We must draw from lowest level to the highest level so blending looks right.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			float maxRad = AtmosphereTopGasGiantMSL() * .75;	// Highest layer radius above sea level.
			float midRad = AtmosphereTopGasGiantMSL() * .25;
			double seaRad = SeaLevelRadius();

			// Display cloud layers in low to high order.
			int shells;
			double baseRad;
			double topRad;
			for(int lyr=0; lyr<2; lyr++)
			{
				if(lyr == 0)
				{
					re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxVPOffset);	// slow lower layer
					baseRad = seaRad;
					topRad = seaRad + midRad;
					shells = 5;
				}
				else
				{
					re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxSPOffset);	// fast upper layer
					baseRad = seaRad + midRad;
					topRad = seaRad + maxRad;
					shells = 10;
				}

				m_AtmosphereLayer[lyr].SelectGL();

				double spanRad = topRad - baseRad;
				for(int shell=1; shell<=shells; shell++)
				{
					double rad = baseRad + spanRad * shell / shells;
					re.GLSL().GasGiantLayerShader().SetAtmosphereRadius(rad);

					// Set ambient light level darker at each lower level.
					float layerIllum = atmIllumination * (.1f + .9f * (rad - seaRad) / maxRad);
					re.GLSL().GasGiantLayerShader().SetAmbientLight(layerIllum);

					re.glFrontFace(GL_CCW);
					m_AtmosphereLayer[lyr].DrawGL();

					re.glFrontFace(GL_CW);
					m_AtmosphereLayer[lyr].DrawGL();
				}

				m_AtmosphereLayer[lyr].DeselectGL();
			}

			re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxSPOffset);	// fast upper layer

			// Render outer atmosphere shell.
			m_AtmosphereLayer[2].SelectGL();
			shells = 5;
			baseRad = seaRad + maxRad;
			topRad = AtmosphereRadius();
			double spanRad = topRad - baseRad;
			for(int shell=1; shell<=shells; shell++)
			{
				double rad = baseRad + spanRad * shell / shells;
				re.GLSL().GasGiantLayerShader().SetAtmosphereRadius(rad);
				m_AtmosphereLayer[2].DrawGL();
			}
			m_AtmosphereLayer[2].DeselectGL();

			re.glDisable(GL_BLEND);
		}
		break;
	case 2:		// Viewer is not inside the atmosphere and is far enough away that we can just render it as an opaque sphere.
		{
			re.GLSL().GasGiantLayerShader().SetAmbientLight(atmIllumination);
			re.GLSL().GasGiantLayerShader().SetAtmosphereRadius(AtmosphereRadius());
			re.GLSL().GasGiantLayerShader().SetWeatherOffset(m_WxSPOffset);	// fast upper layer

			m_AtmosphereLayer[2].SelectGL();
			m_AtmosphereLayer[2].DrawGL();
			m_AtmosphereLayer[2].DeselectGL();
		}
		break;
	}
	re.GLSL().GasGiantLayerShader().Deselect();
}

// Create the sphere model of a gas giant.
void DaWorldGasGiant::CreateGasGiantModel(AuOpenGLFunctions& GLf)
{
	Q_ASSERT(!m_NearGlobe.VAO());
	int latdivs = LatitudeDivisions()/2;
	int londivs = LongitudeDivisions()/2;
	if(latdivs == 0 || londivs == 0)
		return;
	int latdivs2 = latdivs/2;

	// Calculate texture coords for this gas giant.
	// There are 16 tiles in a column in our gas giants texture strip; each is 128x128 pixels. By flipping them in Y we can double
	// the number of gas giant looks.
	float t,v;
	int oy = Id().GetID()%32;
	bool flipY;
	if(oy&1)
		flipY = true;
	else
		flipY = false;
	oy /= 2;
	oy *= 256;
	const float maxy = 4096;
	t = 1.f - (oy + 256 - .5f) / maxy;
	v = 1.f - (oy + .5f) / maxy;

	// Calculate the radius of the sea layer of this gas giant.
	double radius = SeaLevelRadius();

	QList<AuTriTexNS> tris;

	double latdiv = (double)(PI/latdivs);		// latitude division sizes.
	double londiv = (double)(PI*2.0/londivs);	// radians per longitude div.
	for(int lat=0; lat<latdivs; lat++)
	{
		double latang = (double)(PI/2.0 - latdiv * lat);
		double latang2 = (double)(PI/2.0 - latdiv * (lat+1));
		double latcos = (double)cos(latang);
		double latcos2 = (double)cos(latang2);
		double latsin = (double)sin(latang);
		double latsin2 = (double)sin(latang2);

		AuPointTexNS prevl, prevu;
		for(int lon=0; lon<=londivs; lon++)
		{
			double rangle = lon*londiv;
			double rx = (double)cos(rangle);
			double ry = (double)sin(rangle);

			double ts = (double)lon/(double)londivs;
			double ttu, ttl;
			if(flipY)
			{
				// Invert Y to get a different appearance.
				if(lat < latdivs2)
					ttu = t + (1. - (double)lat/(double)latdivs2) * (v - t);
				else
					ttu = t + (double)(lat-latdivs2)/(double)latdivs2 * (v - t);
				if((lat+1) < latdivs2)
					ttl = t + (1. - (double)(lat+1)/(double)latdivs2) * (v - t);
				else
					ttl = t + (double)((lat+1)-latdivs2)/(double)latdivs2 * (v - t);
			}
			else
			{
				if(lat < latdivs2)
					ttu = t + (double)lat/(double)latdivs2 * (v - t);
				else
					ttu = t + (1. - (double)(lat-latdivs2)/(double)latdivs2) * (v - t);
				if((lat+1) < latdivs2)
					ttl = t + (double)(lat+1)/(double)latdivs2 * (v - t);
				else
					ttl = t + (1. - (double)((lat+1)-latdivs2)/(double)latdivs2) * (v - t);
			}

			// Calculate vertex distances from planet center.
			AuPoint nu(rx*latcos, ry*latcos, latsin);		// Upper vertex
			AuPoint vu = nu * radius;
			AuPoint nl(rx*latcos2, ry*latcos2, latsin2);		// Lower vertex
			AuPoint vl = nl * radius;

			AuPointTexNS tvu(vu, ts,ttu, nu, s_seaHardness);
			AuPointTexNS tvl(vl, ts,ttl, nl, s_seaHardness);
			if(lon)
			{
				tris.append(AuTriTexNS(prevl, prevu, tvu));
				tris.append(AuTriTexNS(tvu, tvl, prevl));
			}
			prevl = tvl;
			prevu = tvu;
		}
	}
	m_NearGlobe.CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
}

// Create the atmosphere model of a gas giant.
// viewerContext: 1=outside nearby, 2=outside at a distance
AuGLVAO DaWorldGasGiant::CreateGasGiantAtmosphereModel(AuOpenGLFunctions& GLf, int viewerContext)
{
	// Seed the random number generator for consistent values for the planet.
	AuDice dice;
	dice.Seed(GenSeed());

	// Calculate the radius of the atmosphere at its outer limit.
	double radius = AtmosphereRadius();

	// The atmosphere determines the color and the density for transparency.
	int londivs;
	int latdivs;

	if(viewerContext == 1)
	{
		// Viewer nearby but outside atmosphere.
		londivs = 60;
		latdivs = 30;
	}
	else
	{
		// Distant viewer
		londivs = 36;
		latdivs = 18;
	}

	int latdivs2 = latdivs/2;

	// Calculate texture coords for this gas giant.
	// There are 16 tiles in a column in our gas giants texture strip; each is 128x128 pixels. By flipping them in Y we can double
	// the number of gas giant looks.
	float t,v;
	int oy = Id().GetID()%32;
	bool flipY;
	if(oy&1)
		flipY = true;
	else
		flipY = false;
	oy /= 2;
	oy *= 256;
	const float maxy = 4096;
	t = 1.f - (oy + 256 - .5f) / maxy;
	v = 1.f - (oy + .5f) / maxy;

	QList<AuTriTexNS> tris;

	double latdiv = (double)(PI/latdivs);		// latitude division sizes.
	double londiv = (double)(PI*2.0/londivs);	// radians per longitude div.
	for(int lat=0; lat<latdivs; lat++)
	{
		double latang = (double)(PI/2.0 - latdiv * lat);
		double latang2 = (double)(PI/2.0 - latdiv * (lat+1));
		double latcos = (double)cos(latang);
		double latcos2 = (double)cos(latang2);
		double latsin = (double)sin(latang);
		double latsin2 = (double)sin(latang2);

		AuPointTexNS prevl, prevu;
		for(int lon=0; lon<=londivs; lon++)
		{
			double rangle = lon*londiv;
			double rx = (double)cos(rangle);
			double ry = (double)sin(rangle);

			double ts = (double)lon/(double)londivs;
			double ttu, ttl;
			if(flipY)
			{
				// Invert Y to get a different appearance.
				if(lat < latdivs2)
					ttu = t + (1. - (double)lat/(double)latdivs2) * (v - t);
				else
					ttu = t + (double)(lat-latdivs2)/(double)latdivs2 * (v - t);
				if((lat+1) < latdivs2)
					ttl = t + (1. - (double)(lat+1)/(double)latdivs2) * (v - t);
				else
					ttl = t + (double)((lat+1)-latdivs2)/(double)latdivs2 * (v - t);
			}
			else
			{
				if(lat < latdivs2)
					ttu = t + (double)lat/(double)latdivs2 * (v - t);
				else
					ttu = t + (1. - (double)(lat-latdivs2)/(double)latdivs2) * (v - t);
				if((lat+1) < latdivs2)
					ttl = t + (double)(lat+1)/(double)latdivs2 * (v - t);
				else
					ttl = t + (1. - (double)((lat+1)-latdivs2)/(double)latdivs2) * (v - t);
			}

			// Calculate vertex distances from planet center.
			AuPoint nu(rx*latcos, ry*latcos, latsin);		// Upper vertex
			AuPoint vu = nu * radius;
			AuPoint nl(rx*latcos2, ry*latcos2, latsin2);		// Lower vertex
			AuPoint vl = nl * radius;

			AuPointTexNS tvu(vu, ts,ttu, nu, s_seaHardness);
			AuPointTexNS tvl(vl, ts,ttl, nl, s_seaHardness);
			if(lon)
			{
				tris.append(AuTriTexNS(prevl, prevu, tvu));
				tris.append(AuTriTexNS(tvu, tvl, prevl));
			}
			prevl = tvl;
			prevu = tvu;
		}
	}

	AuGLVAO vao;
	vao.CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
	return vao;
}

// Create the sphere model of a gas giant.
AuGLVAO DaWorldGasGiant::CreateGasGiantCloudModel(AuOpenGLFunctions& GLf, AuWeather::LAYER layer)
{
	int latdivs = LatitudeDivisions()*3/4;
	int londivs = LongitudeDivisions()*3/4;
	if(latdivs == 0 || londivs == 0)
		return AuGLVAO();
	int latdivs2 = latdivs/2;

	// Seed the random number generator for consistent values for the planet.
	AuDice dice;
	dice.Seed(GenSeed());

	// Calculate texture coords for this gas giant.
	// There are 16 tiles in a column in our gas giants texture strip; each is 128x128 pixels. By flipping them in Y we can double
	// the number of gas giant looks.
	float t,v;
	int oy = Id().GetID()%32;
	bool flipY;
	if(oy&1)
		flipY = true;
	else
		flipY = false;
	oy /= 2;
	oy *= 256;
	const float maxy = 4096;
	t = 1.f - (oy + 256 - .5f) / maxy;
	v = 1.f - (oy + .5f) / maxy;

	// Calculate the radius of the cloud layer.
	double layerRadius, opacity;
	if(layer == AuWeather::LowClouds)
		opacity = .99f;		// lowest layer is least transparent
	else if(layer == AuWeather::MiddleClouds)
		opacity = .50f;		// upper layer is more transparent
	else
		opacity = .20f;		// upper limit is most transparent
	layerRadius = 1;	// Unit layers so we can scale them.

	float* preOpac = new float[londivs+1];

	QList<AuTriTexNS> tris;

	double latdiv = (double)(PI/latdivs);		// latitude division sizes.
	double londiv = (double)(PI*2.0/londivs);	// radians per longitude div.
	for(int lat=0; lat<latdivs; lat++)
	{
		double latang = (double)(PI/2.0 - latdiv * lat);
		double latang2 = (double)(PI/2.0 - latdiv * (lat+1));
		double latcos = (double)cos(latang);
		double latcos2 = (double)cos(latang2);
		double latsin = (double)sin(latang);
		double latsin2 = (double)sin(latang2);

		// Calculate sin for latitude banding.
		float latOpacSin = fabs(sinf(latang * 6));
		float latOpacSin2 = fabs(sinf(latang2 * 6));
		if(layer == AuWeather::LowClouds)
		{
			latOpacSin *= 2;
			latOpacSin2 *= 2;
		}
		else
		{
			latOpacSin *= 1.5f;
			latOpacSin2 *= 1.5f;
		}
		if(latOpacSin > 1)
			latOpacSin = 1;
		if(latOpacSin2 > 1)
			latOpacSin2 = 1;

		AuPointTexNS prevl, prevu;
		for(int lon=0; lon<=londivs; lon++)
		{
			// This calculation causes transparency to increase at the margins between the weather bands, with some mottling.
			float vertOpacity;
			if(lat == 0)
				vertOpacity = (latOpacSin + (1.f - latOpacSin) * (float)dice.Roll(1,30,-1)/100.f) * opacity;
			else
				vertOpacity = preOpac[lon];
			float vertOpacity2;
			if(lon < londivs)
				vertOpacity2 = (latOpacSin2 + (1.f - latOpacSin2) * (float)dice.Roll(1,30,-1)/100.f) * opacity;
			else
				vertOpacity2 = preOpac[0];
			preOpac[lon] = vertOpacity2;

			double rangle = lon*londiv;
			double rx = (double)cos(rangle);
			double ry = (double)sin(rangle);

			double ts = (double)lon/(double)londivs;
			double ttu, ttl;
			if(flipY)
			{
				// Invert Y to get a different appearance.
				if(lat < latdivs2)
					ttu = t + (1. - (double)lat/(double)latdivs2) * (v - t);
				else
					ttu = t + (double)(lat-latdivs2)/(double)latdivs2 * (v - t);
				if((lat+1) < latdivs2)
					ttl = t + (1. - (double)(lat+1)/(double)latdivs2) * (v - t);
				else
					ttl = t + (double)((lat+1)-latdivs2)/(double)latdivs2 * (v - t);
			}
			else
			{
				if(lat < latdivs2)
					ttu = t + (double)lat/(double)latdivs2 * (v - t);
				else
					ttu = t + (1. - (double)(lat-latdivs2)/(double)latdivs2) * (v - t);
				if((lat+1) < latdivs2)
					ttl = t + (double)(lat+1)/(double)latdivs2 * (v - t);
				else
					ttl = t + (1. - (double)((lat+1)-latdivs2)/(double)latdivs2) * (v - t);
			}

			// Calculate vertex distances from planet center.
			AuPoint nu(rx*latcos, ry*latcos, latsin);		// Upper vertex
			AuPoint vu = nu * layerRadius;
			AuPoint nl(rx*latcos2, ry*latcos2, latsin2);		// Lower vertex
			AuPoint vl = nl * layerRadius;

			AuPointTexNS tvu(vu, ts,ttu, nu, vertOpacity);
			AuPointTexNS tvl(vl, ts,ttl, nl, vertOpacity2);
			if(lon)
			{
				tris.append(AuTriTexNS(prevl, prevu, tvu));
				tris.append(AuTriTexNS(tvu, tvl, prevl));
			}
			prevl = tvl;
			prevu = tvu;
		}
	}
	delete [] preOpac;

	AuGLVAO vao;
	vao.CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
	return vao;
}

QColor DaWorldGasGiant::ChartColor() const
{
	return ChartColor(OrbitZone());
}

// Static version of ChartColor
QColor DaWorldGasGiant::ChartColor(ORBITZONE eZone)
{
	int r,g,b;
	switch (eZone)
	{
	case OZONE_Inferno:
		r = 128;
		g = 0;
		b = 0;
		break;
	case OZONE_Inner:
		r = 192;
		g = 64;
		b = 0;
		break;
	case OZONE_Habitable:
		r = 64;
		g = 192;
		b = 0;
		break;
	default:
	case OZONE_Outer:
		r = 128;
		g = 64;
		b = 0;
		break;
	case OZONE_Frigid:
		r = 192;
		g = 128;
		b = 0;
		break;
	}
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

QColor DaWorldGasGiant::AtmosphereColor() const
{
	// NOTE: This color value should be seeded using GenSeed. However, doing so requires some changes to AuSensorBlip. This works
	// for now since the only consequence is that the atmo color might change if the system is regenerated. D.H.H. Tue Dec 18, 2012.
	m_GasGiantLock.lockForRead();
	if(!m_AtmColor.isValid())
	{
		m_GasGiantLock.unlock();

		m_GasGiantLock.lockForWrite();
		m_AtmColor = AtmosphereColorStatic(m_AtmType, m_byAtmDensity, Id().GetID());
	}
	QColor c(m_AtmColor);
	m_GasGiantLock.unlock();
	return c;
}

void DaWorldGasGiant::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	//qDebug() << "DaWorldGasGiant::CreateDeviceObjects" << Id() << Name();
	if(!s_TexGasGiant)
		s_TexGasGiant = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "GasGiant", ":data/txGasGiants.png");

	if(!s_TexNoise)
		s_TexNoise = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "TerNoise", ":data/TerNoise.png");

	// Create textures used for developments and rocks.
	if(!s_TexRock)
		s_TexRock = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Rock", ":data/txRock.png");

	// Create sea surface textures.
	s_texSea = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SeaFrigid", ":data/SeaFrigid.png", GL_LINEAR, GL_LINEAR, true);

	// Create sea surface bump maps.
	if(g_Au.BumpMapSeas())
		s_texSeaN = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "NIce", ":data/N_Ice.png", GL_LINEAR, GL_LINEAR, true);

	// NOTE: 1 minus the alpha in this texture is treated as glow, for lava. e.g. Alpha .5 glows at .5. Alpha 1 does not glow.
	if(!s_TexTerPalette[0])
		for(int wt=0; wt<WORLDTEX_Count; wt++)
			s_TexTerPalette[wt] = CreateTerrainTexture(*ari.re, (WORLDTEX_TYPE)wt, g_Au.GroundTextureSize());

	// Grass texture is only needed if this world has grass growing wild in the environment.
	// It is also used for the lawns of homes.
	if(!m_glTexGrass && (CanHaveFlora() || BdHere()))
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

	DaWorldSphere::CreateDeviceObjects(ari);
}

void DaWorldGasGiant::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaWorldSphere::DeleteDeviceObjects(ari);

	if(m_AtmosphereLayer[0].VAO())
		m_AtmosphereLayer[0].DeleteGL();

	if(m_AtmosphereLayer[1].VAO())
		m_AtmosphereLayer[1].DeleteGL();

	if(m_AtmosphereLayer[2].VAO())
		m_AtmosphereLayer[2].DeleteGL();

	if(m_ClaimPoleVAO.VAO())
		m_ClaimPoleVAO.DeleteGL();

	if(m_NearGlobe.VAO())
	{
		m_NearGlobe.DeleteGL();
		m_NearGlobe = AuGLVAO();
	}

	ari.mm.DeleteFlatGlobeModel(Id());

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
		ari.mm.DeleteGlobeModel(Id());

		m_AtGlobe.m_Low = AuGLVAO();
		m_AtGlobe.m_TriVAOs.clear();
	}

	if(m_SeaGlobe.VAO())
		m_SeaGlobe.DeleteGL();

	if(m_UnderseaGlobe.VAO())
		m_UnderseaGlobe.DeleteGL();

	// Deletes all flora textures and textures that depend upon them.
	DeleteFloraDeviceObjects(*ari.re, ari.mm);
}

void DaWorldGasGiant::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	InitSystemPosition();

	// This will take care of the subatoms, particularly the terrain patches.
	DaWorldSphere::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

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

		// Modify the number of days by the rotation speed factor.
		rUDays *= RotationSpeedFactor();

		{
			double pfpd = SeaLevelRadius() * (PIf*2);	// Circumference of planet in feet is equator speed in feet per universe day.
			double spfpd = pfpd;	// Feet saturation pressure travels in a day.
			// Orbit zone is reversed for gas giant wind speed, so it's more animate near the sun and less so in the frigid
			// zone where you can't see them very well anyway.
			ORBITZONE windZone = (ORBITZONE)(OZONE_Frigid - OrbitZone());
			spfpd += AuWeather::PrevailingWindSpeed(windZone) * 5 * 24;	// convert to miles per hour	//DHH wtf calc?
			spfpd *= 20;		// Make gas giant atmospheres more animate.
			double vpfpd = spfpd/2;	// Vapor pressure travels half as fast as SP
			double spday = spfpd/pfpd;		// SP periods per day.
			double vpday = vpfpd/pfpd;		// VP periods per day.
			double spper = spday * rUDays;	// Current SP period
			double vpper = vpday * rUDays;	// Current VP period

			// Calculate movement amount from the fractional period that we're in.
			// Use the id to determine if this body rotates clockwise or counter-clockwise.
			if(Id().GetID()&0x01)
			{
				m_WxSPOffset = (spper - (int)spper);
				m_WxVPOffset = (vpper - (int)vpper);
			}
			else
			{
				m_WxSPOffset = -(spper - (int)spper);
				m_WxVPOffset = -(vpper - (int)vpper);
			}
		}
	}
}

// Only atoms that return true to IsTicker will have their Tick function called.
bool DaWorldGasGiant::IsTicker() const
{
	if(DaWorldSphere::IsTicker())
		return true;
	return IsInAnyScene() || Transform().IsIdentity();
}

void DaWorldGasGiant::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// Must perform axial rotation in tick so things rotate smoothly with the planet's rotation.
	if(!last.isValid())
		InitSystemPosition();
	else if(IsInAnyScene())
	{
		m_GasGiantLock.lockForWrite();
		m_RotationAngle = m_ThinkRotation;
		AuMatrix4x4 t(m_ThinkTransform);
		m_GasGiantLock.unlock();
		ChangePosNotParent(AuAtomPos(ParentId(), t));
	}
	DaWorldSphere::Tick(last, now);
}

void DaWorldGasGiant::TickOnClient(const AuElapsedTimer& last)
{
	DaWorldSphere::TickOnClient(last);
	InitSystemPosition();
}

unsigned int DaWorldGasGiant::IsTocker() const
{
	if(IsInAnyScene())
		return 100;
	return DaWorldSphere::IsTocker();
}

void DaWorldGasGiant::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
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
	DaWorldSphere::Tock(last, now, scripts, tockTrans);
}

void DaWorldGasGiant::ThinkPerformAxialRotation()
{
	// Change the world transformation matrix for this planetary body.
	m_GasGiantLock.lockForWrite();
	double rangle = AngleToPlanet();
	m_ThinkTransform.LoadIdentity();

	// Planet origin is at its orbit position.
	m_ThinkTransform.RotateZ(rangle);							// Rotate the model space to the planet's orbit position.
	m_ThinkTransform.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Location of this planet relative to the parent star.
	m_ThinkTransform.RotateZ(-rangle);							// Rotate the model space to the planet's orbit position.

	// Apply axial rotation to planet.
	m_ThinkRotation = CalcAngleOfRotation();					// Update axis rotation based on real time.
	m_ThinkTransform.RotateZ(m_ThinkRotation);
	m_ThinkTransform.RotateX(AxialTilt());
	m_GasGiantLock.unlock();
}

void DaWorldGasGiant::InitSystemPosition()
{
	// Change the world transformation matrix for this planetary body.
	double rangle = AngleToPlanet();
	AuMatrix4x4 tBody;

	// Planet origin is at its orbit position.
	tBody.RotateZ(rangle);								// Rotate the model space to the planet's orbit position.
	tBody.Translate(AuPoint(OrbitRadius(), 0.0, 0.0));	// Translate to the location of this planet relative to the parent star.
	tBody.RotateZ(-rangle);								// Rotate the model space to the planet's orbit position.

	// Apply axial rotation to planet.
	m_GasGiantLock.lockForWrite();
	m_RotationAngle = CalcAngleOfRotation();			// Update axis rotation based on real time.
	tBody.RotateZ(m_RotationAngle);
	tBody.RotateX(AxialTilt());
	m_ThinkRotation = m_RotationAngle;
	m_ThinkTransform = tBody;
	m_GasGiantLock.unlock();
	ChangePosNotParent(AuAtomPos(ParentId(), tBody));
}

// Provides information about the planet's terrain at a given surface location.
void DaWorldGasGiant::QueryTerrainAtLoc(AuTerrainInfo &ter, const AuPoint& loc) const
{
	Q_ASSERT(!loc.IsNaN());
	AuGlobeLL ll;
	LLOfLoc(ll, loc);											// Get longitude/latitude coordinate of surface location.
	QueryTerrain(ter, ll);
}

// Provides information about the planet's terrain at a given surface location.
void DaWorldGasGiant::QueryTerrain(AuTerrainInfo& ter, const AuGlobeLL& ll) const
{
	//AuPoint loc = GetTerrainLocation(ll);	// Get 3D surface location at the longitude/latitude coordinate.
	float altMSL = SurfaceAltitudeMSL(ll);
	int inEnclosure = InEnclosure(ll, altMSL);
	if(inEnclosure < 0)
		inEnclosure = 0;
	int isIrrigated = IsIrrigated(ll);

	// Fill the AuTerrainInfo object with data about this world.
	ter.m_ElevType = TerrainElevationType(altMSL);
	if(SnowCoverage(altMSL, ll.Latitude(), inEnclosure) > .3f)
		ter.m_ElevType = TER_Alpine;							// This results in the sound of walking in snow.
	ter.m_byFoliage = VegetationAt(ll, altMSL, inEnclosure, isIrrigated);		// World foliage density at long/lat.
	ter.m_Radius100km = m_Radius1000km*10;
	ter.m_byWater = 100;
	ter.m_InEnclosure = inEnclosure;
	ter.m_Irrigation = isIrrigated;
	ter.m_rSeaLevelRadius = SeaLevelRadius();
	ter.m_rSurfaceRadius = ter.m_rSeaLevelRadius + altMSL;
}

// Determine the quantity of sunlight striking the surface location from each sun.
// This does not consider sloping terrain or shadows.
// In habitable zone, noon sun intensity is 1.0.
float DaWorldGasGiant::QuerySunlight(const AuPoint &fSurfLoc) const
{
	// Search up the atom hierarchy for a solar system.
	const DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return 0;	// wut?
	DaStar* star = env->FindStar();
	if(!star)
		return 0;	// Possible in design studio.

	// Applies illumination dropoff everywhere due to thick cloud cover.
	double seaDepth;
	if(InDesignerPreview())
		seaDepth = .1 * SeaDysphoticDepth();		// Block most sunlight at the surface.
	else
	{
		seaDepth = .9 * SeaDysphoticDepth();		// Block most sunlight at the surface.
		double altitude = AltitudeMSL(fSurfLoc);		// Add illumination dropoff when below sea level.
		if(altitude < 0.f)
			seaDepth -= altitude;
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
		totalSunlight += ComputeSunLight(fSurfLoc, sunLoc, atten, ps->Radius());
	}
	return totalSunlight;
}

// Moving a subatom uses the following logic.
// - New location is calculated.
// - Delta to remain on terrain is calculated.
//   - Too much rise and movement is stopped.
//   - Too much loss and movement falls downward a fixed distance.
// - New location is checked for potential handoff to a subatom or to parent atom.
bool DaWorldGasGiant::MoveSubatom(DaAtom *pMove, const AuPoint &vMove, int elapsedMs)
{
	// Determine the destination point of the move.
	AuPoint foldloc(pMove->Location());
	AuPoint fnuloc = foldloc + vMove;
	bool moveInAtmo = FilterDestinationLoc(pMove, foldloc, fnuloc, elapsedMs);

#if !defined(QT_NO_DEBUG)
	if(fnuloc.IsNaN())
	{
		qDebug() << "DaWorldGasGiant::MoveSubatom" << Id() << Name() << "atom" << pMove->Id() << pMove->Name()
			<< "fnuloc" << fnuloc << fnuloc.IsNaN() << "vMove" << vMove << vMove.IsNaN() << "IsNaN ERROR!";
	}
#endif

	// Determine the result of attempting to move to the nu location.
	DaAtom *pHit = NULL;
	AuPoint hitLoc, hitNormal;
	COLLISION_TYPE eCollide = pMove->CheckCollision(foldloc, fnuloc, pHit, hitLoc, hitNormal);
	pMove->SetMoveResult(eCollide);
	//qDebug() << "DaWorldGasGiant::MoveSubatom" << Id() << Name() << "move" << pMove->Id() << pMove->Name()
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
			//qDebug("DaWorldGasGiant::MoveSubatom - %s Entered %s %s",
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
		//qDebug("DaWorldGasGiant::MoveSubatom - %s Missed",
				//(const char*)pMove->Name());
		{
			// The moving atom did not hit anything.
			// Watch for spacecraft entering wormholes when they are not subordinate to the primary star.
			DaVessel* ship = dynamic_cast<DaVessel*>(pMove);
			if(ship && ship->IsReadyToWormhole())
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
								qDebug("DaWorldGasGiant::MoveSubatom %d %s spacecraft %d %s entered wormhole.",
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

// Returns true if a child moving to 'fnuloc' from 'foldloc' departs the influence of this atom.
bool DaWorldGasGiant::CheckChildExit(const AuPoint &/*foldloc*/, const AuPoint &fnuloc, const DaAtom* /*who*/) const
{
	// This body orbits a star so it controls a larger region of space.
	double rd = fnuloc.Length();
	if(rd > FeetPerOrbit()/2.)
		return true;
	return false;
}

// Returns true if movement from foldloc to fnuloc would enter the area of
// influence of this atom.
// 'foldloc' and 'fnuloc' are relative to the parent.
bool DaWorldGasGiant::CheckPeerJoin(const AuPoint &/*foldloc*/, const AuPoint &fnuloc, const DaAtom*)
{
	// This body orbits a star so it controls a larger region of space.
	double rd = fnuloc.Distance(Location());
	if(rd < FeetPerOrbit()/2.)
		return true;
	return false;
}

AuPoint DaWorldGasGiant::GetLocation(const QPoint &pLongLat) const
{
	const int ilondivs = LongitudeDivisions();
	const int ilatdivs = LatitudeDivisions();
	double rxyangle = pLongLat.x() * (PI*2.0) / ilondivs;
	double rincline = PI/2.0 - pLongLat.y() * PI / ilatdivs;
	AuPoint fLoc;
	fLoc.PolarValue(SeaLevelRadius(), rxyangle, rincline);
	return fLoc;
}

// The result is rotated to the planets current axial rotation.
AuPoint DaWorldGasGiant::GetLocation(const AuPoint &longLat) const
{
	const int ilondivs = LongitudeDivisions();
	const int ilatdivs = LatitudeDivisions();
	double rxyangle = longLat.x * (PI*2.0) / ilondivs;
	double rincline = PI/2.0 - longLat.y * PI / ilatdivs;
	AuPoint fLoc;
	fLoc.PolarValue(SeaLevelRadius(), rxyangle, rincline);
	return fLoc;
}

// The point returned takes into account the fact that the surface is divided
// into trapezoidal quads. A strictly polar calculation produces the wrong
// result because the top and bottom edges of the trapezoid are straight lines
// instead of curves.
AuPoint DaWorldGasGiant::GetRealLongLat(const AuPoint& fSurfaceLoc) const
{
#if !defined(QT_NO_DEBUG)
	if(fSurfaceLoc.IsNaN())
	{

		const DaStar* star = FindParentStarInEnv();
		const DaSolarSystem* system = FindParentSolarSystem();
		qDebug() << "DaWorldGasGiant::GetRealLongLat" << Id() << Name() << Description() << "IsNaN ERROR! surfaceLoc"
			<< fSurfaceLoc;
		if(star)
			qDebug() << "DaWorldGasGiant::GetRealLongLat" << Id() << Name() << "orbits star" << star->Id() << star->Name()
				<< star->Description();
		if(system)
			qDebug() << "DaWorldGasGiant::GetRealLongLat" << Id() << Name() << "in system" << system->Id() << system->Name()
				<< system->Description();
		return AuPoint(0,0);
	}
#endif

	// Rotate the surface location to my angle of rotation.
	/*
	AuPoint fu;
	AuMatrix4x4 tAxisRot;
	tAxisRot.RotateZ(-AngleOfRotation());
	fu = tAxisRot.Project(fSurfaceLoc);
	*/
	AuPoint fu(fSurfaceLoc);
	fu.Unit();

	// Calculate longitude.
	double rxyangle = fu.ArcTangent_2D();
	if(rxyangle < 0.0)
		rxyangle += PI*2;
	const double rlondivs = LongitudeDivisions();
	double rlong = rxyangle * rlondivs / (PI*2);
	if(rlong >= rlondivs)
		rlong -= rlondivs;

	// Calculate latitude.
	double rlat;
	const double rlatdivs = LatitudeDivisions();
	double radius = SeaLevelRadius();
	double rincline = fu.PolarIncline();
	double latrad = cos(rincline) * radius;
	double latsin = sin(rincline);
	rincline = PI/2.0 - rincline;
	if(rincline < 0.0)
		rincline += PI*2;
	rlat = rincline * rlatdivs / PI;

	// Latitude needs to be skewed due to the chord problem created by latitude
	// lines curving across straight edged surface quads.
	// Thanks to Jacob for the Law of Sines solution here.
	double angper = PI*2/rlondivs;		// angle per longitude division
	double lonfrac = rlong - (int)rlong;	// use fraction only
	double angA = angper * lonfrac;			// angle into the longitude division
	double angB = (rlondivs - 2.) * PI / rlondivs / 2.;
	double angC = PI - (angA + angB);
	double sideB = latrad * sin(angC) / sin(angB);
	double latsize = radius * PI / rlatdivs;
	double skew = (latrad - sideB) * latsin / latsize;
	rlat -= skew;
	if(rlat < 0)
		rlat = 0;
	else if(rlat >= rlatdivs)
		rlat = rlatdivs - FUZZ;	// Barely short of the limit. Int truncation will give a safe answer.

#if !defined(QT_NO_DEBUG)
	if(!(rlat >= 0 && rlat < rlatdivs))
	{
		const DaStar* star = FindParentStarInEnv();
		const DaSolarSystem* system = FindParentSolarSystem();
		qDebug() << "DaWorldGasGiant::GetRealLongLat" << Id() << Name() << Description() << "rlat" << rlat << "rlatdivs" << rlatdivs
			<< "byRadius" << m_Radius1000km << "surfaceLoc" << fSurfaceLoc;
		if(star)
			qDebug() << "DaWorldGasGiant::GetRealLongLat" << Id() << Name() << "orbits star" << star->Id() << star->Name()
				<< star->Description();
		if(system)
			qDebug() << "DaWorldGasGiant::GetRealLongLat" << Id() << Name() << "in system" << system->Id() << system->Name()
				<< system->Description();
	}
	Q_ASSERT(rlat >= 0 && rlat < rlatdivs);
#endif

	return AuPoint(rlong, rlat);
}

QPoint DaWorldGasGiant::GetLongLat(const AuPoint &fSurfaceLoc) const
{
	AuPoint rll = GetRealLongLat(fSurfaceLoc);
	return QPoint((int)rll.x, (int)rll.y);
}

// Wraps pLongLat to be within the acceptable range.
// Returns false if latitude rolls over to wrap to the bottom/top of the planet.
// The random terrain generator uses this.
// Returns true if there is no latitude wrap.
bool DaWorldGasGiant::ClampLongLat(QPoint &pLongLat) const
{
	// Longitude wraps horizontally.
	const int iLonDivs = LongitudeDivisions();
	if (pLongLat.x() < 0)
		pLongLat.setX(pLongLat.x() + iLonDivs);
	else if (pLongLat.x() > iLonDivs-1)
		pLongLat.setX(pLongLat.x() - iLonDivs);

	// Latitude wraps vertically for use by the terrain generator. This isn't generally
	// useful for navigation.
	const int iLatDivs = LatitudeDivisions();
	if (pLongLat.y() < 0)
	{
		pLongLat.setY(pLongLat.y() + iLatDivs);
		return false;
	}
	else if (pLongLat.y() > iLatDivs-1)
	{
		pLongLat.setY(pLongLat.y() - iLatDivs);
		return false;
	}

	return true;
}

// List atoms that should be in the scene when this atom is in the scene, i.e. IsInScene.
void DaWorldGasGiant::ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList& sceneAtoms)
{
	if(!HasSubatoms())
		return;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	if(pSceneAtom->FindParentPlanetInEnv() == this)
	{
		// Scene atom is subordinate to this planet, either in or out of the atmosphere.
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
				// Add other atoms and their subatoms that want to be included.
				if(!(*it)->IsInScene(pSceneAtom, cameraEnvLoc))
					continue;
				sceneAtoms.AddIfUnique(*it);
				(*it)->ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
			}
		}
	}
	else
	{
		// The scene atom is not at this planet.
		// List asteroid, rings and moons around this planet.
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			if(!dynamic_cast<DaWorld*>(*it) && !dynamic_cast<DaAsteroid*>(*it))
				continue;

			// Add the atom to the list.
			sceneAtoms.AddIfUnique(*it);
		}
	}
}

// Returns an information about the natural resources available in the environment of this world.
void DaWorldGasGiant::QueryNaturalResources(AuZoneResource& res, int resourceZone) const
{
	QueryNaturalResources(res, resourceZone, GenSeed());
}

// Returns an information about the natural resources available in the environment of this world.
void DaWorldGasGiant::QueryNaturalResources(AuZoneResource& res, int resourceZone, quint32 worldGenSeed) const
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

			m_Resources[rz].m_Radius100km = m_Radius1000km*10;
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
			m_Resources[rz].m_Vegetation = 0;
			m_Resources[rz].m_VegetationQ = dice.RandomQuality();

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

			// Vegetation quantities for places that don't have vegetation, for plants in Enclosures.
			if(!m_Resources[rz].m_Vegetation)
			{
				int domeVeg = 60;
				m_Resources[rz].m_Beans = dice.Roll(1, (domeVeg+1)/2);
				m_Resources[rz].m_Fiber = dice.Roll(1, domeVeg);
				m_Resources[rz].m_Fruit = dice.Roll(1, (domeVeg+1)/2);
				m_Resources[rz].m_Grain = dice.Roll(1, domeVeg);
				m_Resources[rz].m_Grapes = dice.Roll(1, (domeVeg+2)/3);
				m_Resources[rz].m_Herbs = dice.Roll(1, (domeVeg+1)/2);
				m_Resources[rz].m_Hops = dice.Roll(1, (domeVeg+2)/3,-2);
				m_Resources[rz].m_Nuts = dice.Roll(1, (domeVeg+1)/2);
				m_Resources[rz].m_Spices = dice.Roll(1, (domeVeg+1)/2);
				m_Resources[rz].m_Vegetables = dice.Roll(1, (domeVeg+1)/2);
				m_Resources[rz].m_Wood = dice.Roll(1, (domeVeg+1)/2);
			}
		}

		// Animal Resources
		{
			if(CanHaveFauna())
			{
				// Surprise, fish may be found in exotic seas on gas giants.
				m_Resources[rz].m_Animals = dice.Roll(1, 50);
				m_Resources[rz].m_Fish = dice.Roll(1,Hydrographics());
				m_Resources[rz].m_FishQ = dice.RandomQuality();
			}
			else
			{
				// Fish don't live in seas of lava or liquid gases.
				m_Resources[rz].m_Animals = 0;
				m_Resources[rz].m_Fish = 0;
				m_Resources[rz].m_FishQ = 0;
			}
			m_Resources[rz].m_AnimalsQ = dice.RandomQuality();
			m_Resources[rz].m_CheeseQ = dice.RandomQuality();
			m_Resources[rz].m_EggsQ = dice.RandomQuality();
			m_Resources[rz].m_FertilizerQ = dice.RandomQuality();
			m_Resources[rz].m_MilkQ = dice.RandomQuality();
		}

		// Mineral Resources
		{
			// Stone prevalence.
			m_Resources[rz].m_Stone = dice.Roll(1,100);
			m_Resources[rz].m_Stone = MAX(m_Resources[rz].m_Stone, 10);	// >= 10
			m_Resources[rz].m_StoneQ = dice.RandomQuality();

			m_Resources[rz].m_Crystals = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Crystals = MAX(m_Resources[rz].m_Crystals, 2); // >= 2
			m_Resources[rz].m_CrystalsQ = dice.RandomQuality();

			m_Resources[rz].m_Eludium = 0;	// No eludium here.
			m_Resources[rz].m_EludiumQ = dice.RandomQuality();

			m_Resources[rz].m_Gems = dice.Roll(1,m_Resources[rz].m_Stone);
			m_Resources[rz].m_Gems = MAX(m_Resources[rz].m_Gems, 2);	// >= 2
			m_Resources[rz].m_GemsQ = dice.RandomQuality();

			// Gold is more common on larger worlds.
			// Gold is more common closer to the sun.
			int prob = m_Resources[rz].m_Stone/2 + m_byCoreRadius - (OrbitZone()+1);	// Core radius is 1 to 6.
			if(prob <= 0)
				prob = 0;
			else
				prob = dice.Roll(1, prob+1, -1);	// Range is [0..prob].
			m_Resources[rz].m_GoldOre = prob;
			m_Resources[rz].m_GoldOreQ = dice.RandomQuality();

			m_Resources[rz].m_Ice = 100;
			m_Resources[rz].m_IceQ = dice.RandomQuality();

			if(galaxy == MilkyWay)
				m_Resources[rz].m_Lumenite = 0;	// No lumenite in Milky Way galaxy.
			else
			{
				// Lumenite is rare.
				// Prevalence increases as orbit zone increases.
				// Prevalence is reduced by planet size.
				int lumenite = 12;
				m_Resources[rz].m_Lumenite = dice.Roll(2,lumenite,-2);	// Bell curve so zero is less common when possible.
			}
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

			m_Resources[rz].m_Bolite = dice.Roll(1,CoreRadius1000km()-2,-1);
			m_Resources[rz].m_Phlogiston = 0;
			int roll = dice.Roll(1,CoreRadius1000km()/2,-1);
			if(roll > 0)
				m_Resources[rz].m_Polytaride = roll;
			else
				m_Resources[rz].m_Polytaride = 0;
			m_Resources[rz].m_Vulcanite = 0;

			// Gas-giant only resources.
			m_Resources[rz].m_Adamantite = dice.Roll(1, m_Resources[rz].m_Stone);
			m_Resources[rz].m_AdamantiteQ = dice.RandomQuality();

			m_Resources[rz].m_Viathol = dice.Roll(2,CoreRadius1000km(),-1);
			m_Resources[rz].m_ViatholQ = dice.RandomQuality();

			m_Resources[rz].m_Flomentum = m_Radius1000km / 4;
			if(rz == 0)
				m_Resources[rz].m_FlomentumQ = dice.RandomQuality();
			else
				m_Resources[rz].m_FlomentumQ = m_Resources[0].m_FlomentumQ;
		}

		// Hydrocarbon Resources
		{
			// Presence of natural resources is determined by core size
			int icoal=0, ioil=0, igas=0;
			icoal = dice.Roll(2,CoreRadius1000km(),-1);
			ioil = dice.Roll(2,CoreRadius1000km(),-1);
			igas = dice.Roll(2,CoreRadius1000km(),-1);
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
	const double radius = GlobeRadius();
	int eqDivs = (int)(radius*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 20;												// 20' spacing for rocks at highest density.
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

void DaWorldGasGiant::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
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
		blips.Add(AuBlipGasGiant(m_Radius1000km, CoreRadius1000km(), inSensorRange, Id(), EnvLocation()));
	}

	if(HasSubatoms())
	{
		// Zone ranges are taken from :CheckChildExit.
		int scanSubs = 1;	// 0 to not scan any subatoms, 1 to scan all subatoms, 2 to scan only subordinate planets/moons
		if(blipRange > scannerSensorRange + FeetPerOrbit()/2.)
			scanSubs = 2;			// Too far to scan tiny subatoms in my zone of influence. Must get moons though.
		// I am scanning my subatoms and I actually have some.
		if(scanSubs == 1)
			DaWorldSphere::SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
					scannerWeaponRange, scanContent, lockedBlip, stanceCache);
		else if(scanSubs == 2)
		{
			// Only scan sub planets.
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaWorld* pAtom = dynamic_cast<DaWorld*>(*it);
				if(!pAtom)
					continue;
				pAtom->SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
						scannerWeaponRange, scanContent, lockedBlip, stanceCache);
			}
		}
	}
}

// Insert first time database record.
void DaWorldGasGiant::Insert(QSqlDatabase& db) const
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
			"claimedBy,claimLon,claimLat,claimId,claimName,claimTime,geoFeatures, "
			"atmDensity,atmType,dna,indSpec,radius,terrain,fogOfWar");
	QString qmarks("?,?, ?,?,?, ?,?,?, ?,?,?,?, "
			"?,?,?,?, "
			"?,?,?,?,?,?,?, "
			"?,?,?,?,?,?,?");

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
	QString sq = QString("INSERT INTO a_WorldGasGiant (") + fields + ") VALUES (" + qmarks + ");";
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

	// a_WorldGasGiant fields
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
	qa.addBindValue(m_Radius1000km);
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
		out << m_byCoreRadius;

		qa.addBindValue(ba);
	}

	{
		// Fog of War
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << *m_FogOfWar;
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
		//qDebug() << "DaWorldGasGiant::Insert" << *this << "resZones" << ResourceZones();
		const unsigned int uisize = m_pFauna->DataSize()*ResourceZones();
		QByteArray ba = qCompress(m_pFauna[0].Data(), uisize);
		qa.addBindValue(ba);
	}

	if(!qa.exec())
		qWarning("DaWorldGasGiant::Insert %s FAILED. %s sq '%s'",
				(const char*)Name().toLatin1(),
				(const char*)qa.lastError().text().toLatin1(),
				(const char*)sq.toLatin1());
}

// Update existing database record.
void DaWorldGasGiant::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq = QString("UPDATE a_WorldGasGiant SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, timeModified=?, "
			"timeTock=?, timeDeath=?, "
			"name=?, claimedBy=?, claimLon=?, claimLat=?, claimId=?, claimName=?, claimTime=?, geoFeatures=?, dna=?, indSpec=?, "
			"fogOfWar=?");

	if(m_pFlora)
		sq += ", flora=?";
	if(m_pFauna)
		sq += ", fauna=?";
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

	// a_WorldGasGiant fields
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
		// Fog of War
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << *m_FogOfWar;
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
		//qDebug() << "DaWorldGasGiant::Update" << *this << "resZones" << ResourceZones();
		const unsigned int uisize = m_pFauna->DataSize()*ResourceZones();
		QByteArray ba = qCompress(m_pFauna[0].Data(), uisize);
		qa.addBindValue(ba);
	}
	
	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaWorldGasGiant::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaWorldGasGiant::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully
// loaded.
DaWorldGasGiant* DaWorldGasGiant::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, 
		LOADSUBATOMS loadWhat, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,timeTock,timeDeath, "
			"name,orbitRadius,orbitZone, "
			"claimedBy,claimLon,claimLat,claimId,claimName,claimTime,geoFeatures, "
			"atmDensity,atmType,radius,dna,indSpec,terrain,fauna,flora,fogOfWar "
			"FROM a_WorldGasGiant WHERE ");
	if(!idToLoad.IsNull())
		sq += QString("id=%1;").arg(idToLoad.GetID());
	else
		sq += QString("idp=%1;").arg(parentId.GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaWorldGasGiant *pnu = new DaWorldGasGiant;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
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

					// Activate my henge portal.
					henge->SetActive(false);
				}

				pnu->UnlockGeologicalFeatures();
			}

			// a_WorldGasGiant fields
			pnu->m_byAtmDensity = qa.value(qv++).toInt();
			pnu->m_AtmType = (ATMTYPE)qa.value(qv++).toInt();
			pnu->m_Radius1000km = qa.value(qv++).toInt();

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
				}
				if(pnu->m_pSentient && pnu->m_pSentient->IsNull())
				{
					delete pnu->m_pSentient;
					pnu->m_pSentient = 0;
				}
			}

			// Must get terrain m_byCoreRadius for proper ResourceZones count.
			qvi=qv++;
			if(!qa.value(qvi).isNull())
			{
				// Terrain
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				in >> pnu->m_TerrainFrequency;
				in >> pnu->m_TerrainPersFactor;
				in >> pnu->m_TerrainPersistence;
				in >> pnu->m_TerrainXOff;
				in >> pnu->m_TerrainYOff;
				in >> pnu->m_byCoreRadius;
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
				if(uisize*pnu->ResourceZones() == ba.count())
				{
					for(int rz=0; rz<pnu->ResourceZones(); rz++)
						pnu->m_pFauna[rz].SetData(ba.begin() + rz*uisize);
				}
				else
				{
					qWarning() << "warning: DaWorldGasGiant::LoadSubatoms" << *pnu << "fauna data size mismatch." << bau.count()
						<< "uncompressed to" << ba.count() << "instead of" << uisize*pnu->ResourceZones() << "creating new data";
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
					qWarning("ERROR - DaWorldGasGiant::LoadSubatoms - %d %s Flora data error. %d bytes loaded",
						pnu->Id().GetID(), (const char*)pnu->Name().toLatin1(), ba.count());
			}

			qvi=qv++;
			if(!qa.value(qvi).isNull())
			{
				// Fog of war
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				AuFogOfWar fow;
				in >> fow;
				pnu->SetFogOfWar(fow);
			}

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			// We never get here if we're the SceneMaster.
			// The SceneManager needs planets and moons.
			if((loadWhat & LOAD_Moons) == LOAD_Moons)
			{
				DaWorldGlobe::LoadSubatoms(ether, db, pnu->Id(), loadWhat);			// Moons
				DaWorldRing::LoadSubatoms(ether, db, pnu->Id(), loadWhat);			// Rings
			}
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

				pnu->m_BdBits = 0x80 | 0x20 | 0x08;
				pnu->JurisdictionUpdate();
				pnu->UpdateRoadMap();
				pnu->UpdateIrrigationCache();
				pnu->CitizenSupplierUpdate();
				pnu->TroopSupplierUpdate();
				pnu->SendBdFacilities();	// Notify master server of bd cities and their facilities.
				pnu->InvalidateEnclosureCache();
			}
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

DaAtom* DaWorldGasGiant::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
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
		// Test the segment to see if it enters the zone of control of this world.
		// Spherical bodies can be tested readily to see if the segment passes within their zone of control.
		// A miss means we can ignore subatoms too.
		double zoneOfControl = FeetPerOrbit()/2.;				// This body orbits a star so it controls a larger region of space.

		AuSphere sphere(tModel.Project(AuPoint(0,0)), zoneOfControl);
		AuPoint hit1, hit2;
		AuPoint sightLoc(sightedLoc);
		int isect = sphere.Intersect(eyeLoc, sightLoc, hit1, hit2);
		if(!isect)
		{
			SetCachedSightTarget(eyeLoc, sightedLoc, isect, false, AuPoint(0,0));
			return pBest;									// The segment does not enter the zone of control of this world.
		}

		// The rough test passed, we need to get more detailed.
		if(SightTargetDetails(eyeLoc, tModel, sightLoc, sightedPart, sightedShield) && sightLoc.Length2() < sightedLoc.Length2())
		{
			SetCachedSightTarget(eyeLoc, sightedLoc, isect, true, sightLoc);
			pBest = (DaAtom*)this;								// Must cast me to non-const.
			sightedLoc = sightLoc;
			if(sightedSurfaceNormal)
				*sightedSurfaceNormal = AuPoint(0,0);
		}
		else
			SetCachedSightTarget(eyeLoc, sightedLoc, isect, false, AuPoint(0,0));
	}

	// Base class performs a recursive search.
	return DaWorldSphere::SightTarget(sightingAtom, eyeLoc, tModel, pBest, sightedLoc, sightedPart, sightedShield,
			sightedSurfaceNormal);
}

bool DaWorldGasGiant::IsSightTarget(const DaAtom* toWho) const
{
	// Planets must be a sight target to spacecraft and cities because they block shots by bay weapons and base weapons.
	// Planets must not be a sight target to anyone else because then they can't shoot things below the surface of the sea.
	if(dynamic_cast<const DaCapsule*>(toWho))
		return true;											// Bay|base weapons are blocked by planets.

	return false;
}

// Returns true if all the flags in all the cities on this planet are controlled by the 'empireId' and if none of those
// cities is an occupied city.
int DaWorldGasGiant::OkToRename(quint32 empireId) const
{
	// Gas giant can be renamed if no moons have the flag of another empire or an occupied city.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Only spheres are tested. Rings call the parent's OkToRename function, which would cause recursive death here.
		const DaWorld* moon = dynamic_cast<const DaWorld*>(*it);
		if(!moon || dynamic_cast<const DaWorldRing*>(*it))
			continue;					// Not a world or it's a ring.

		int okToRename = moon->OkToRename(empireId);
		if(okToRename == 2 || okToRename == 3)
			return okToRename;			// Occupied city or flag of another empire on one of my moons.
	}
	return 0;
}

// Calculate sea level temperature in kelvin at a given long/lat.
// This does not account for altitude. The effect of altitude must be applied to the
// result of this calculation. See AuWeather::TemperatureAt.
float DaWorldGasGiant::SeaLevelTempAt(const AuGlobeLLF& longLat, float solarOrbitRadius, float sunLightAtten) const
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
	switch(OrbitZone())
	{
	case OZONE_Inferno:
	case OZONE_Frigid:
		// Determine solar intensity at this orbit range. Intensity will be 1 in the
		// habitable zone. This gives us a way to attenuate the equatorial temperature.
		equatorTemp /= (sunLightAtten * solarOrbitRadius * solarOrbitRadius);
		break;
	case OZONE_Inner:
		// Inner zone is a few degrees hotter than the Habitable zone.
		// Using the solar intensity calc produces too hot of temperatures.
		// This gives us a nice muggy planet without incinerating all life.
		equatorTemp += 15;
		break;
	case OZONE_Habitable:
	case OZONE_DeepSpace:
		// Use the nominal equator temperature that we already calculated.
		break;
	case OZONE_Outer:
		// Outer zone is a few degrees colder than the Habitable zone.
		// Using the solar intensity calc produces too cold of temperatures.
		// This gives us a nice cold planet without freezing all life.
		equatorTemp -= 20;
		break;
	}

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

void DaWorldGasGiant::AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const
{
	AuPoint sysLoc = EnvLocation();
	double navDist = navLoc.Distance(sysLoc);
	double safeDist = AtmosphereRadius() + ParkAltAboveSun();		// Navigators never plan to go closer than parking orbit.
	if(navDist < safeDist)
	{
		// Nav location is too close to this planet. Move away.
		if(shipLoc.FuzzyEquals(sysLoc))
		{
			// Ship location is at the center of this planet.
			// Move navLoc straight away from the planet to a safe distance.
			AuPoint navVec = navLoc - sysLoc;
			navVec.Unit(safeDist);
			navLoc = sysLoc + navVec;
		}
		else
		{
			// Nav point is too close to the planet.
			// Move the nav point toward the shipLoc to a safe distance.
			AuPoint navVec = shipLoc - sysLoc;
			navVec.Unit(safeDist);
			navLoc = sysLoc + navVec;
		}
	}

	DaAtom::AdjustNavLocation(shipLoc, navLoc);
}

// Returns true if a compass can provide directional guidance on this world at 'loc', which is relative to this world.
// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
bool DaWorldGasGiant::CompassVectors(const AuPoint& loc, AuPoint& north, AuPoint& east, AuPoint& up) const
{
	// Determine north direction vector.
	if(loc.y > 0)
	{
		// loc is north of the equator.
		AuPoint vn = AuPoint(0,0,SeaLevelRadius()) - loc;					// Vector to north pole from loc.
		double rdn = vn.Length();									// Distance to north pole.
		if(rdn != 0.0)
			north = vn/rdn;
		else
			north = AuPoint(0,0,1);
	}
	else
	{
		// loc is south of the equator.
		AuPoint vn = loc - AuPoint(0,0,-SeaLevelRadius());					// Vector from south pole to loc.
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
	return true;
}

void DaWorldGasGiant::PrepareForScan()
{
	DaWorldSphere::PrepareForScan();
	InitSystemPosition();
}

QString DaWorldGasGiant::SystemScanReport(int scanEx) const
{
	// Biosphere exists where it is possible. It is shown as 'potential' where it can be grown at a colony. Blank where
	// no biosphere can exist, such as gas giants and rocky rings.
	QString sBio;
	if(CanHaveFauna() || CanHaveFlora())
		sBio = tr("Biosphere");									// Animals or plants exist there.
	else
		sBio = tr("Biosphere Potential");						// No animals or plants exist there but a colony could make them.

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
	QString sGeoSize;
	QString sGeoGrav;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	{
		sBodyType = tr("Gas Giant");
		double rdia = SeaLevelAltitude() * 2.;	// Diameter in feet.
		rdia /= FeetPerMeter();			// Convert to meters.
		rdia = round(rdia/100) * 100;	// Round to nearest 100 meters.
		sSize = QObject::tr("%L1m Diameter").arg(rdia);
		sGrav = QObject::tr("%L1m/s%2 Sea Level Gravity")
			.arg(Gravity(SeaLevelAltitude())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());
		double rgeodia = GlobeRadius() * 2.;	// Diameter in feet.
		rgeodia /= FeetPerMeter();			// Convert to meters.
		rgeodia = round(rgeodia/100) * 100;	// Round to nearest 100 meters.
		sGeoSize = QObject::tr("%L1m Diameter").arg(rgeodia);
		sGeoGrav = QObject::tr("%L1m/s%2 Core Gravity")
			.arg(Gravity(GlobeRadius())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());

		if(MSLAtmosphereDensity() > 0)
			sAtm = QObject::tr("%2, %1% Density, Toxic")
				.arg(MSLAtmosphereDensity()*100)
				.arg(AtmoTypeName(AtmosphereType()));
		else
			sAtm = QObject::tr("No Atmosphere");

		sWater = QObject::tr("%1% Surface %2").arg(Hydrographics()).arg(sLiquid);
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
	rpt += sBodyType + ", " + sGeoSize;
	rpt += "<br>" + sGeoGrav;
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
				QString r0, r1;
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
		else
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
		rpt += "</tbody></table>";
	}

	// Hydrosphere
	if(!sWater.isEmpty())
	{
		rpt += QObject::tr("<p><hr><b>Hydrosphere</b><br>");
		rpt += sWater;
		rpt += "<br>" + sSize;
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
		rpt += QObject::tr("<p><hr><b>Atmosphere</b>");
		if(!sAtm.isEmpty())
			rpt += "<br>" + sAtm;
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
		else
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
	}

	rpt += "</html>";
	return rpt;
}

void DaWorldGasGiant::SystemScanReportXML(QXmlStreamWriter& stream, int scanEx) const
{
	// Biosphere exists where it is possible. It is shown as 'potential' where it can be grown at a colony. Blank where
	// no biosphere can exist, such as gas giants and rocky rings.
	QString sBio;
	if(CanHaveFauna() || CanHaveFlora())
		sBio = tr("Biosphere");									// Animals or plants exist there.
	else
		sBio = tr("Biosphere Potential");						// No animals or plants exist there but a colony could make them.

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
	QString sGeoSize;
	QString sGeoGrav;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	{
		sBodyType = tr("Gas Giant");
		double rdia = SeaLevelAltitude() * 2.;	// Diameter in feet.
		rdia /= FeetPerMeter();			// Convert to meters.
		rdia = round(rdia/100) * 100;	// Round to nearest 100 meters.
		sSize = QObject::tr("%1m Diameter").arg(rdia);
		sGrav = QObject::tr("%1m/s%2 Sea Level Gravity")
			.arg(Gravity(SeaLevelAltitude())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());
		double rgeodia = GlobeRadius() * 2.;	// Diameter in feet.
		rgeodia /= FeetPerMeter();			// Convert to meters.
		rgeodia = round(rgeodia/100) * 100;	// Round to nearest 100 meters.
		sGeoSize = QObject::tr("%1m Diameter").arg(rgeodia);
		sGeoGrav = QObject::tr("%1m/s%2 Core Gravity")
			.arg(Gravity(GlobeRadius())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());
		if(MSLAtmosphereDensity() > 0)
			sAtm = QObject::tr("%2, %1% Density")
				.arg(MSLAtmosphereDensity()*100)
				.arg(AtmoTypeName(AtmosphereType()));
		else
			sAtm = QObject::tr("No Atmosphere");
		sWater = QObject::tr("%1% Surface %2").arg(Hydrographics()).arg(sLiquid);
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
	stream.writeAttribute("diameter", sGeoSize);
	stream.writeAttribute("gravity", sGeoGrav);
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
		stream.writeAttribute("diameter", sSize);
		stream.writeAttribute("gravity", sGrav);
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
				stream.writeEndElement();
			}
		}
		stream.writeEndElement();
	}

	stream.writeEndElement();
}

void DaWorldGasGiant::FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& shipSysLoc, float gunRange,
		const DaAtom*& targetAtom, double& targetRange) const
{
	// Check moons.
	DaWorldSphere::FindClosestHarvesterTargetRecursive(harvesterModule, shipSysLoc, gunRange, targetAtom, targetRange);

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

AuPoint DaWorldGasGiant::WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const
{
	Q_UNUSED(harvesterModule);
	// Scale vector to the weapon lock location.
	AuPoint vecToWeapon = shipSysLoc - EnvLocation();		// Vec to ship from planet, relative to the system.
	vecToWeapon.Unit(AtmosphereRadius());						// Vec scaled to planet atmosphere.
	return EnvLocation() + vecToWeapon;						// System relative target location.
}

void DaWorldGasGiant::SystemScanResources(QMultiMap<quint8, COMMODITYID>& atmRes, QMultiMap<quint8, COMMODITYID>& hydRes,
		QMultiMap<quint8, COMMODITYID>& geoRes, QMultiMap<quint8, COMMODITYID>& bioRes) const
{
	Q_UNUSED(bioRes);
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

quint8 DaWorldGasGiant::NaturalResourceQuality(COMMODITYID cid, int resourceZone) const
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

int DaWorldGasGiant::GenMoonCount(const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	int moons = dice.Roll(2, 3, m_Radius1000km/10 - 3);
	if(moons < 1 && OrbitZone() == OZONE_Habitable)
		return 1;
	else if(moons < 0)
		return 0;
	return moons;
}

int DaWorldGasGiant::ResourceZone(float longitude) const
{
	longitude += PIf;
	while(longitude < 0)
		longitude += PIf*2;
	const int zones = ResourceZones();
	return (int)(longitude*zones/(PIf*2)) % zones;
}

int DaWorldGasGiant::ResourceZone(double longitude) const
{
	longitude += PI;
	while(longitude < 0)
		longitude += PI*2;
	const int zones = ResourceZones();
	return (int)(longitude*zones/(PI*2)) % zones;
}

double DaWorldGasGiant::GeologicalTerrainAltitudeMSL(const AuGlobeLL& ll) const
{
	// Longitude of PI is changed to -PI to insure perfect closure at the seam of the world, even though a shear cliff may be there.
	double longitude;
	if(FuzzyEquals((float)ll.Longitude(), PIf))
		longitude = -PI;
	else
		longitude = ll.Longitude();

	double terrainWaterFactor = 0;

	// 7 octaves of Perlin noise should do the trick.
	//const double xfact = cos(ll.Latitude());
	const double xfact = cos(ll.Latitude()) * .999999 + .000001;
	m_GasGiantLock.lockForRead();
	const double fx = m_TerrainXOff + longitude/PI * xfact * m_TerrainFrequency;		// X noise offset
	const double fy = m_TerrainYOff + ll.Latitude()/PI * m_TerrainFrequency;	// Y noise offset
	double altFactor = AuNoise::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/m_TerrainPersFactor;	// Scale near -1 to 1.
	altFactor -= terrainWaterFactor;							// Shift altitude up with more water level.

	// Blend the seam in the last few degrees of arc on the positive side.
	const double blendArc = 10*DEG2RAD;							// Number of degrees of arc angle to blend.
	if(longitude > PI - blendArc)
	{
		double blendLongitude = longitude - PI*2;				// Angle to longitude from the negative side.
		const double fx = m_TerrainXOff + blendLongitude/PI * xfact * m_TerrainFrequency;	// X noise offset
		const double fy = m_TerrainYOff + ll.Latitude()/PI * m_TerrainFrequency;	// Y noise offset
		double blendAltFactor = AuNoise::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/m_TerrainPersFactor;
		blendAltFactor -= terrainWaterFactor;					// Shift altitude up with more water level.

		double blendAmt = (PI - longitude) / blendArc;			// 1 at start of blend (170 degrees) down to 0 at 180 degrees.
		altFactor = AuNoise::Cosine_Interpolate(blendAltFactor, altFactor, blendAmt);
	}

	// Blend the polar regions to avoid the giant sphincters at the poles of the world.
	const double polarBlendArc = 10*DEG2RAD;					// Number of degrees of arc angle to blend.
	const double polarBlendComp = 80*DEG2RAD;					// Number of degrees of arc angle to blend.
	if(ll.Latitude() > polarBlendComp)
	{
		// North polar blend. (+ latitude)
		const double fx = m_TerrainXOff;	// X noise offset
		const double fy = m_TerrainYOff + .5*m_TerrainFrequency;	// Y noise offset
		double blendAltFactor = AuNoise::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/m_TerrainPersFactor;
		blendAltFactor -= terrainWaterFactor;					// Shift altitude up with more water level.

		double blendAmt = (PI/2 - ll.Latitude()) / polarBlendArc;	// 1 at start of blend (80 degrees) down to 0 at 90 degrees.
		altFactor = altFactor * blendAmt + blendAltFactor * (1-blendAmt);
	}
	else if(ll.Latitude() < -polarBlendComp)
	{
		// South polar blend. (- latitude)
		const double fx = m_TerrainXOff;	// X noise offset
		const double fy = m_TerrainYOff - .5*m_TerrainFrequency;	// Y noise offset
		double blendAltFactor = AuNoise::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/m_TerrainPersFactor;
		blendAltFactor -= terrainWaterFactor;					// Shift altitude up with more water level.

		double blendAmt = (PI/2 + ll.Latitude()) / polarBlendArc;	// 1 at start of blend (80 degrees) down to 0 at 90 degrees.
		altFactor = altFactor * blendAmt + blendAltFactor * (1-blendAmt);
	}

	// Below sea level, altitude is negative.
	altFactor = -altFactor / (-2 - terrainWaterFactor);

	// Pull the land up tighter to sea level, less and less as we get closer to the sea trenches.
	//if(altFactor < -.5f && altFactor > -1)
		//altFactor = -(altFactor * altFactor);				// Preserve the sign.

	// Pull the land up tighter to sea level near the coast lines for nice beaches.
	//double altExp = 2 + altFactor;		// Range is 2..1, decreasing with depth for less effect.
	//if(altExp < 1)
		//altExp = 1;
	//altFactor = -pow(-altFactor, altExp);

	m_GasGiantLock.unlock();

	// Scale the altitude factor into the range we want.
	double coreRadius = GlobeRadius();
	double coreMSL = coreRadius - SeaLevelRadius();
	return coreMSL + altFactor * coreRadius;
}

void DaWorldGasGiant::ResourceDistributionAt(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
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

	const double radius = GlobeRadius();

	// Rocks (cobbled from CreateRocks) and water coverage
	{
		int eqDivs = (int)(radius*2*PI);						// Circumference of globe in whole feet.
		eqDivs /= 20;											// 20' spacing for rocks at highest density.

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
				LLOfLoc(rockLL, rockLoc);
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
				TerrainAltitudeMSLPave(rockLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub, farmShrubMSL,
						orchardTree, orchardTreeMSL);
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

		waterCoverage = 100;
	}

	// Trees and shrubs can be found inside an enclosure.
	if(!maxICareAbout || (!CanHaveFlora() && !HasEnclosure()))
		return;

	// Shrubs (cobbled from CreateShrubs)
	{
		int eqDivs = (int)(radius*2*PI);							// Circumference of globe in whole feet.
		eqDivs /= 12;												// 12' spacing for shrubs at highest density.

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
				LLOfLoc(plantLL, plantLoc);
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
		int eqDivs = (int)(radius*2*PI);							// Circumference of globe in whole feet.
		eqDivs /= 24;												// 24' spacing for trees at highest density.

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
				LLOfLoc(plantLL, plantLoc);
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

void DaWorldGasGiant::CreateAtTerrainTriangles(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLLTri& terTri,
		const bool& killed) const
{
	//qDebug() << "DaWorldGasGiant::CreateAtTerrainTriangles" << Id() << Name() << "tris" << tris.count();

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
	//if(killed)
		//qDebug() << "DaWorldGasGiant::CreateAtTerrainTriangles" << Id() << Name() << "tris" << tris.count() << "stopped";
}

void DaWorldGasGiant::CreateAtTerrainSurfaceTriangle(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
		const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const
{
	int texreps = GroundLevelEquatorDivisions() * TERRAIN_TEXTURE_REPS;

	// Wrap texture properly at the seam.
	float ll0long = ll0.Longitude();
	if(FuzzyEquals((float)ll0.Longitude(), -PIf) && (ll1.Longitude() > 0 || ll2.Longitude() > 0))
		ll0long = PIf;
	float ll1long = ll1.Longitude();
	if(FuzzyEquals((float)ll1.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll2.Longitude() > 0))
		ll1long = PIf;
	float ll2long = ll2.Longitude();
	if(FuzzyEquals((float)ll2.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll1.Longitude() > 0))
		ll2long = PIf;

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
		t.m_TerTex.setX((ll0long)/(PIf*2) * texreps * cosf(ll0.Latitude()));
		t.m_TerTex.setY((ll0.Latitude() + PIf/2)/PIf * texreps);
		ti0 = terAltInfo.insert(ll0, t);
	}
	else
	{
		(*ti0).m_TerTex.setX((ll0long)/(PIf*2) * texreps * cosf(ll0.Latitude()));
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
		t.m_TerTex.setX((ll1long)/(PIf*2) * texreps * cosf(ll1.Latitude()));
		t.m_TerTex.setY((ll1.Latitude() + PIf/2)/PIf * texreps);
		ti1 = terAltInfo.insert(ll1, t);
	}
	else
	{
		(*ti1).m_TerTex.setX((ll1long)/(PIf*2) * texreps * cosf(ll1.Latitude()));
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
		t.m_TerTex.setX((ll2long)/(PIf*2) * texreps * cosf(ll2.Latitude()));
		t.m_TerTex.setY((ll2.Latitude() + PIf/2)/PIf * texreps);
		ti2 = terAltInfo.insert(ll2, t);
	}
	else
	{
		(*ti2).m_TerTex.setX((ll2long)/(PIf*2) * texreps * cosf(ll2.Latitude()));
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
	tri.m_Vert0 = AuGlobeVertAltVegNPave((*ti0).m_TerCoord, (*ti0).m_TerTex, SeaLevelRadius() + (*ti0).m_TerAltMSL, (*ti0).m_Snow,
			(*ti0).m_Veg, (*ti0).m_TerNormal, paveCover0, paveType, (*ti0).m_CityAmbience);
	tri.m_Vert1 = AuGlobeVertAltVegNPave((*ti1).m_TerCoord, (*ti1).m_TerTex, SeaLevelRadius() + (*ti1).m_TerAltMSL, (*ti1).m_Snow,
			(*ti1).m_Veg, (*ti1).m_TerNormal, paveCover1, paveType, (*ti1).m_CityAmbience);
	tri.m_Vert2 = AuGlobeVertAltVegNPave((*ti2).m_TerCoord, (*ti2).m_TerTex, SeaLevelRadius() + (*ti2).m_TerAltMSL, (*ti2).m_Snow,
			(*ti2).m_Veg, (*ti2).m_TerNormal, paveCover2, paveType, (*ti2).m_CityAmbience);
	tris.append(tri);
}

quint8 DaWorldGasGiant::VegetationAt(const AuGlobeLL& ll, float terAltMSL, int inEnclosure, int irrigated) const
{
	Q_UNUSED(terAltMSL);
	// No foliage unless there is an enclosure there.
	if(!inEnclosure)
		return 0;
	// In undersea enclosure or in harsh environment enclosure.
	unsigned char avgFoliage;
	if(irrigated)
		avgFoliage = 60;
	else
		avgFoliage = 50;

	// Calculate max noise due to persistence value at 7 octaves.
	m_GasGiantLock.lockForRead();
	float vegPersFactor = 1.f + m_TerrainPersistence + m_TerrainPersistence*m_TerrainPersistence + powf(m_TerrainPersistence,3) +
		powf(m_TerrainPersistence,4) + powf(m_TerrainPersistence,5) + powf(m_TerrainPersistence,6);

	//const float xfact = cosf(ll.Latitude());
	const float xfact = cosf(ll.Latitude()) * .999999f + .000001f;
	const float fx = m_TerrainXOff - ll.Longitude()/PIf * xfact * m_TerrainFrequency;	// X noise offset
	const float fy = m_TerrainYOff - ll.Latitude()/PIf * m_TerrainFrequency;	// Y noise offset
	float vegFactor = fabs(AuNoiseF::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/vegPersFactor);	// scale result to 0 to 1

	// Blend the seam in the last few degrees of arc on the positive side.
	const float blendArc = 10*DEG2RADf;							// Number of degrees of arc angle to blend.
	if(ll.Longitude() > PI - blendArc)
	{
		float blendLongitude = ll.Longitude() - PIf*2;			// Angle to longitude from the negative side.
		const float fx = m_TerrainXOff - blendLongitude/PIf * xfact * m_TerrainFrequency;	// X noise offset
		const float fy = m_TerrainYOff - ll.Latitude()/PIf * m_TerrainFrequency;	// Y noise offset
		float blendVegFactor = fabs(AuNoiseF::CubicPerlinNoise(7, m_TerrainPersistence, fx, fy)/vegPersFactor);	// 0 to 1

		float blendAmt = (PIf - ll.Longitude()) / blendArc;		// 1 at start of blend (170 degrees) down to 0 at 180 degrees.
		vegFactor = AuNoiseF::Cosine_Interpolate(blendVegFactor, vegFactor, blendAmt);
	}
	m_GasGiantLock.unlock();

	// Nobody expects deserts in their Enclosures or irrigated areas.
	if((inEnclosure || irrigated) && vegFactor < .25f)
		vegFactor = .25f;

	// Calculate the amount based on overall plant density attributed to water and atmosphere.
	float vegAt = vegFactor * avgFoliage * 2;
	if(vegAt > 100)
		vegAt = 100;
	return (quint8)(vegAt);
}

// Returns snow coverage in the range of 0 to 1.
float DaWorldGasGiant::SnowCoverage(float altMSL, float latitude, int inEnclosure) const
{
	Q_UNUSED(altMSL);
	Q_UNUSED(latitude);
	Q_UNUSED(inEnclosure);
	return 0;
}

void DaWorldGasGiant::CreateSurfaceTrianglesFlat(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const
{
	//qDebug() << "DaWorldGasGiant::CreateSurfaceTriangles" << Id() << Name() << "eqdivs" << eqdivs;
	// Create terrain cells for the globe based on longitude and latitude.
	// Each pole will have a single terrain cell.
	tris.clear();

	// Poles
	AuGlobeLL poleN(0,PI/2);									// North pole
	AuGlobeLL poleS(0,-PI/2);									// South pole

	// Remember highest and lowest terrain altitudes.
	maxAltMSL = TerrainAltitudeMSL(poleN);
	minAltMSL = maxAltMSL;
	float alt = TerrainAltitudeMSL(poleS);
	if(alt > maxAltMSL)
		maxAltMSL = alt;
	else if(alt < minAltMSL)
		minAltMSL = alt;

	// North and south hemisphere with poles, except equator.
	int latdivshemi = eqdivs/4;									// Latitude divisions per hemisphere.
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	QList<AuGlobeLL> lastRowN, lastRowS;						// Previous rows of north and south globe locations.
	for(int latinc=0; latinc<latdivshemi; latinc++)
	{
		// We can do both hemispheres at once.
		double latitude = PI/2 - latDivAng * latinc;			// Range is PI/2 to 0.
		Q_ASSERT(latitude <= PI/2);
		Q_ASSERT(latitude > 0);
		int londivs = (int)ceilf(cos(latitude) * eqdivs);
		if(londivs == 0)
			londivs = 3;
		else if(londivs < 4)
			londivs = 4;										// Need at least 3 to draw triangles.
		double lonDivAng = (PI*2) / londivs;					// Radian angle per longitude division
		QList<AuGlobeLL> rowN, rowS;							// Current rows of north and south globe locations.
		AuGlobeLL lastN, lastS;
		int lastRowIdx=0;
		//qDebug() << "latinc" << latinc << "latdivshemi" << latdivshemi << "londivs" << londivs;
		for(int loninc=0; loninc<=londivs; loninc++)
		{
			double longitude;
			if(loninc == londivs)
				longitude = -PI;								// For tight closure we don't calculate that last longitude.
			else
				longitude = lonDivAng * loninc - PI;			// Range is -PI to PI.
			Q_ASSERT(longitude >= -PI);
			Q_ASSERT(longitude <= PI);
			//qDebug() << "loninc" << loninc;
			AuGlobeLL llN(longitude, latitude);					// North hemisphere
			AuGlobeLL llS(longitude, -latitude);				// South hemisphere

			// Remember highest and lowest terrain altitudes.
			alt = TerrainAltitudeMSL(llN);
			if(alt > maxAltMSL)
				maxAltMSL = alt;
			else if(alt < minAltMSL)
				minAltMSL = alt;
			alt = TerrainAltitudeMSL(llS);
			if(alt > maxAltMSL)
				maxAltMSL = alt;
			else if(alt < minAltMSL)
				minAltMSL = alt;

			if(loninc)
			{
				// Not the first location at this latitude so we can draw a triangle.
				if(latinc)
				{
					// First row (0) is not tessellated; we just gather information.
					// Tessellate subsequent row to the previous row.
					tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], llN, lastN));
					tris.append(AuGlobeLLTri(lastRowS[lastRowIdx], lastS, llS));

					if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
					{
						// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
						// there and bump the previous row index.
						int nextLastRowIdx = lastRowIdx+1;
						if(nextLastRowIdx >= lastRowN.count())
							nextLastRowIdx = 0;					// Wrap to beginning of last row.
						tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llN));
						tris.append(AuGlobeLLTri(lastRowS[nextLastRowIdx], lastRowS[lastRowIdx], llS));
						lastRowIdx = nextLastRowIdx;
					}
				}
			}
			lastN = llN;
			lastS = llS;
			rowN.append(llN);
			rowS.append(llS);
		}
		lastRowN = rowN;
		lastRowS = rowS;
	}

	// Equator
	int londivs = eqdivs;
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	AuGlobeLL lastE;
	int lastRowIdx=0;
	for(int loninc=0; loninc<=londivs; loninc++)
	{
		double longitude;
		if(loninc == londivs)
			longitude = -PI;									// For tight closure we don't calculate that last longitude.
		else
			longitude = lonDivAng * loninc - PI;				// Range is -PI to PI.
		Q_ASSERT(longitude >= -PI);
		Q_ASSERT(longitude <= PI);
		AuGlobeLL llE(longitude, 0);							// Equator

		// Remember highest and lowest terrain altitudes.
		alt = TerrainAltitudeMSL(llE);
		if(alt > maxAltMSL)
			maxAltMSL = alt;
		else if(alt < minAltMSL)
			minAltMSL = alt;

		if(loninc)
		{
			// Not the first location at the equator so we can draw a triangle.
			// Tessellate equator row to the previous row.
			tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], llE, lastE));
			tris.append(AuGlobeLLTri(lastRowS[lastRowIdx], lastE, llE));

			if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
			{
				// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
				// there and bump the previous row index.
				int nextLastRowIdx = lastRowIdx+1;
				if(nextLastRowIdx >= lastRowN.count())
					nextLastRowIdx = 0;					// Wrap to beginning of last row.
				tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llE));
				tris.append(AuGlobeLLTri(lastRowS[nextLastRowIdx], lastRowS[lastRowIdx], llE));
				lastRowIdx = nextLastRowIdx;
			}
		}
		lastE = llE;
	}

	//qDebug() << "DaWorldGasGiant::CreateSurfaceTriangles" << Id() << Name() << "maxAlt" << maxAltMSL << "minAltMSL" << minAltMSL
		//<< "tris" << tris.count();
}

void DaWorldGasGiant::BuildingChanged(const AuMatrix4x4& bldM, double radius, AuGLModelMaker& mm)
{
	DaWorldLL::BuildingChanged(bldM, radius, mm);
	//qDebug() << "DaWorldGasGiant::BuildingChanged" << Id() << Name() << "iid" << bld->IndustryId()
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
		if(bldLL.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Globe))
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
			// The building intersects the globe surface triangle.
			// Invalidate the high tessellation model of the surface triangle.
			// This does not delete it so it can still be used until the nu one is requested and becomes ready.
			m_AtGlobe.InvalidateGlobeTriVAO(Id(), (*tvi).first, mm);
		}

		tvi++;
	}

	// Trigger regen of rocks and plants.
	m_FloraViewer = AuPoint(0,0);
	m_RockViewer = AuPoint(0,0);
	InvalidateTerrainInfoSprites();
}

void DaWorldGasGiant::CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, quint32 worldGenSeed,
		float range, bool visibleOnly) const
{
	//qDebug() << "DaWorldGasGiant::CreateRocks" << Id() << Name();
	AuPoint vup = -GravityDir(groundLoc);

	AuPoint vx;
	vup.ArbitraryAxis(vx);

	AuPoint vy = vup.Cross(vx);
	vy.Unit();

	CreateRocks(rocks, groundLoc, vx, vy, worldGenSeed, range, visibleOnly);
	//qDebug() << "DaWorldGasGiant::CreateRocks" << Id() << Name() << "done";
}

void DaWorldGasGiant::CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, const AuPoint& vx,
		const AuPoint& vy, quint32 worldGenSeed, float range, bool visibleOnly) const
{
	int eqDivs = (int)(GlobeRadius()*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 20;												// 20' spacing for rocks at highest density.

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
			LLOfLoc(rockLL, rockLoc);
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
			rockLoc = rockLL.GlobeLocation(SeaLevelRadius() + TerrainAltitudeMSLPave(rockLL, hideRocks, hideGrass, hideShrubs,
						hideTrees, pavementType, farmShrub, farmShrubMSL, orchardTree, orchardTreeMSL));
			if(hideRocks && visibleOnly)
				continue;

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
	//qDebug() << "DaWorldGasGiant::CreateShrubs" << shrubs.count();
}

// 'longLat' is longitude latitude angles, in radians
QPointF DaWorldGasGiant::WindAt(const AuGlobeLLF& longLat) const
{
	// Check for completed enclosure.
	if(InEnclosure(longLat, 0) > 0)
		return QPointF(10.f, 0.f);			// Light wind inside enclosure so plants move and wind does not howl.
	else
		return QPointF(0,0);
}

AuPoint DaWorldGasGiant::ResourceZoneLandingSite(int zone) const
{
	Q_UNUSED(zone);
	AuPoint bestLoc;					// 0,0 means we didn't find one.
	return bestLoc;
}

bool DaWorldGasGiant::CanHaveFauna() const
{
	return AtmosphereType() == ATMTYPE_Exotic;
}

// Test whether animals can exist at a specific terrain level on this
// planet. Inner and outer worlds restrict animals to the seas and coasts.
bool DaWorldGasGiant::CanHaveFaunaAtElev(TER_ELEV_TYPE terType) const
{
	// We will assume the liquid is not too in exotic atmosphere.
	if(AtmosphereType() != ATMTYPE_Exotic)
		return false;

	// We will allow creatures at the bottom of the sea, where it is warm.
	if(terType != TER_SeaFloor && terType != TER_SeaTrench)
		return false;

	// Passed all the tests.
	return true;
}

double DaWorldGasGiant::UndevelopedVirginTerrainAltitudeMSL(const AuGlobeLL& ll) const
{
	// Determine the altitude of the terrain due to geology.
	double altMSL = GeologicalTerrainAltitudeMSL(ll);

	// Adjust altitude for geological features.
	altMSL = LockGeologicalFeatures().AdjustAltMSLUndeveloped(*this, ll, altMSL);
	UnlockGeologicalFeatures();
	return altMSL;
}

double DaWorldGasGiant::VirginTerrainAltitudeMSL(const AuGlobeLL& ll, WORLDTEX_TYPE& pavementType, AuTime& newest) const
{
	// Determine the altitude of the terrain due to geology.
	double altMSL = GeologicalTerrainAltitudeMSL(ll);

	// Adjust altitude for geological features.
	pavementType = WORLDTEX_Count;
	altMSL = LockGeologicalFeatures().AdjustAltMSL(*this, ll, altMSL, pavementType, newest);
	UnlockGeologicalFeatures();
	return altMSL;
}

double DaWorldGasGiant::VirginTerrainAltitudeMSL(const AuGlobeLL& ll, bool& hideRocks, int& hideGrass, bool& hideShrubs,
		bool& hideTrees, WORLDTEX_TYPE& pavementType, AuTime& newest) const
{
	// Native terrain is textured according to terrain altitude.
	hideRocks = false;
	hideGrass = 0;
	hideShrubs = false;
	hideTrees = false;
	pavementType = WORLDTEX_Count;

	// Determine the altitude of the terrain due to geology.
	double altMSL = GeologicalTerrainAltitudeMSL(ll);

	// Adjust altitude for geological features.
	altMSL = LockGeologicalFeatures().AdjustAltMSL(*this, ll, altMSL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType,
			newest);
	UnlockGeologicalFeatures();
	return altMSL;
}

void DaWorldGasGiant::WeatherAt(const AuGlobeLLF& ll, float altitudeMSL, float& relHumidity, float& oat, float& bar) const
{
	Q_UNUSED(ll);
	Q_UNUSED(altitudeMSL);
	relHumidity = 2;
	oat = 0;							// DHH gas giant Calculate outside air temperature.
	bar = 0;							// DHH gas giant Calculate outside air perssure.
}

void DaWorldGasGiant::QueryMineResources(const DaBd* mine, QList<COMMODITYID>& rockCIDs) const
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

void DaWorldGasGiant::QueryWellResources(const DaBd* well, QList<COMMODITYID>& wellCIDs) const
{
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

// Returns the commodity id of significant rock outcroppings at 'll'.
// This is used for spotting and foraging. 'll' is rounded to the nearest actual rock grid location and that is the rock that
// is returned, using RockAt.
// Returns CID_Invalid if no significant rocks are present.
COMMODITYID DaWorldGasGiant::SpotRockAt(const AuGlobeLL& ll, quint32 worldGenSeed) const
{
	const double radius = GlobeRadius();
	int eqDivs = (int)(radius*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 20;												// 20' spacing for rocks at highest density.

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
COMMODITYID DaWorldGasGiant::SpotShrubAt(const AuGlobeLL& ll, quint32 worldGenSeed, bool& aquaticPlant) const
{
	// Try to keep a minimal amount of shrubs so players can still find a shrub after turning off the grass.
	const float userDensity = .2f;

	const double radius = GlobeRadius();
	int eqDivs = (int)(radius*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 12;												// 12' spacing for shrubs at highest density.

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
	aquaticPlant = false;	// No natural aquatic plants on gas giants.
	float vegPercent;	// veg density present in the range of 0 to 1
	if(HasEnclosure())
	{
		double plantAltMSL = TerrainAltitudeMSL(plantLL);
		int inEnclosure = InEnclosure(plantLL, plantAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		if(inEnclosure == 0)
			return CID_Invalid;	// No plants outside enclosures on gas giant.
		int irrigated = IsIrrigated(plantLL);
		vegPercent = VegetationAt(plantLL, plantAltMSL, inEnclosure, irrigated)/100.f;
	}
	else
		vegPercent = 0;	// No plants outside enclosures on gas giant.
	if(vegPercent < 2.f/8.f || vegPercent >= 7.5f/8.f)
		return CID_Invalid;				// First two and last flora density bands (0, 1, and 7) have no shrubs.

	// Shrubs are most abundant where there is no vegetation and least abundant in the midst of heavy vegetation.
	float shrubPercent = 1.f - (vegPercent - .1f);
 	shrubPercent *= userDensity;								// Modulate with user density setting.

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
COMMODITYID DaWorldGasGiant::SpotTreeAt(const AuGlobeLL& ll, quint32 worldGenSeed) const
{
	const double radius = GlobeRadius();
	int eqDivs = (int)(radius*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 24;												// 24' spacing for trees at highest density.

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
	if(HasEnclosure())
	{
		double terAltMSL = TerrainAltitudeMSL(plantLL);
		int inEnclosure = InEnclosure(plantLL, terAltMSL);
		if(inEnclosure < 0)
			inEnclosure = 0;
		int irrigated = IsIrrigated(plantLL);
		vegPercent = VegetationAt(plantLL, terAltMSL, inEnclosure, irrigated)/100.f;
	}
	else
		vegPercent = 0;	// No plants outside enclosures on gas giant.
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

COMMODITYID DaWorldGasGiant::RockAt(const AuGlobeLL& rockLL, quint32 worldGenSeed, int resourceZone) const
{
	int inEnclosure = InEnclosure(rockLL, 0);
	if(inEnclosure < 0)
		inEnclosure = 0;
	float snowCoverage = SnowCoverage(TerrainAltitudeMSL(rockLL), rockLL.Latitude(), inEnclosure);

	// Generate a coherent noise factor for a clumpy specie distribution. Range is -1 to 1.
	float nfreq = GroundLevelEquatorDivisions() * 6;						// Smaller creates larger squarish patches of each type.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float nx = ceilf(rockLL.Longitude()/(PI*2)*nfreq);
	float ny = ceilf(rockLL.Latitude()/PI*nfreq);
	m_GasGiantLock.lockForRead();
	float typeN = AuNoiseF::CoherentNoise(randF, m_TerrainXOff + nx, m_TerrainYOff + ny);
	m_GasGiantLock.unlock();

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

COMMODITYID DaWorldGasGiant::ShrubAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const
{
	// Generate a coherent noise factor for a clumpy specie distribution. Range is -1 to 1.
	float nfreq = GroundLevelEquatorDivisions() * 20;						// Smaller creates larger squarish patches of each specie.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float nx = ceilf(plantLL.Longitude()/(PI*2)*nfreq);
	float ny = ceilf(plantLL.Latitude()/PI*nfreq);
	m_GasGiantLock.lockForRead();
	float specieN = AuNoiseF::CoherentNoise(randF, m_TerrainXOff + nx, m_TerrainYOff + ny);
	m_GasGiantLock.unlock();

	// Get information about natural resources present at the plant location.
	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(plantLL.Longitude()), worldGenSeed);

	// Return the commodity type that is there.
	return res.GenerateShrubResource(specieN);
}

COMMODITYID DaWorldGasGiant::TreeAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const
{
	// Generate a coherent noise factor for a clumpy specie distribution. Range is -1 to 1.
	float nfreq = GroundLevelEquatorDivisions() * 5;						// Smaller creates larger squarish patches of each specie.
	unsigned int randF = worldGenSeed % 12;						// Select the random number algorithm for noise.
	float nx = ceilf(plantLL.Longitude()/(PI*2)*nfreq);
	float ny = ceilf(plantLL.Latitude()/PI*nfreq);
	m_GasGiantLock.lockForRead();
	float specieN = AuNoiseF::CoherentNoise(randF, m_TerrainXOff + nx, m_TerrainYOff + ny);
	m_GasGiantLock.unlock();

	// Get information about natural resources present at the plant location.
	AuZoneResource res;
	QueryNaturalResources(res, ResourceZone(plantLL.Longitude()), worldGenSeed);

	// Return the commodity type that is there.
	return res.GenerateTreeResource(specieN);
}

float DaWorldGasGiant::TreeLineMSL() const
{
	return AuWeather::LayerAltitude(*this, AuWeather::LowClouds);
}

// Create all the data for the low tessellation 'at' globe VAOs that can be created in a background thread.
void DaWorldGasGiant::CreateAtGlobeSurfaceTriangles(QList<AuGlobeTriAltVegN>& tris, float& maxAltMSL, float& minAltMSL) const
{
	//qDebug() << "DaWorldGasGiant::CreateAtGlobeSurfaceTriangles" << Id() << Name() << "started";

	// Tessellate surface of globe for at-globe model. This tessellation only stores longitudes and latitudes. Terrain altitudes
	// are returned for efficiency but they are not stored in the terrainTris data.
	AuGlobeLLTriList terrainTris;
	CreateSurfaceTriangles(terrainTris, GroundLevelEquatorDivisions()*2, maxAltMSL, minAltMSL);
	//qDebug() << "DaWorldGasGiant::CreateAtGlobeSurfaceTriangles" << Id() << Name() << "terrainTris" << terrainTris.count();

	// Create the triangles that make up the model.
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo> terAltInfo;	// Info that gets cached along the way.
	AuGlobeTriAltVegN tri;
	tris.clear();
	for(AuGlobeLLTriList::const_iterator ti = terrainTris.cbegin(); ti != terrainTris.cend(); ti++)
	{
		CreateAtGlobeSurfaceTriangle(tri, (*ti).m_LL0, (*ti).m_LL1, (*ti).m_LL2, terAltInfo);
		tris.append(tri);
	}
	//qDebug() << "DaWorldGasGiant::CreateAtGlobeSurfaceTriangles" << Id() << Name() << "done" << tris.count() << "triangles";
}

void DaWorldGasGiant::CreateFlora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs,
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

void DaWorldGasGiant::CreateGrass(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& aquaticgrass,
		const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const
{
	const float userDensity = g_Au.GroundCoverQty() / 5000.f;
	//qDebug() << "DaWorldGasGiant::CreateGrass userDensity" << userDensity;
	if(userDensity == 0)
		return;

	int eqDivs = (int)(GlobeRadius()*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 3;												// 3' spacing for grass at highest density.

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
			bool aquatic = false;
			bool hideRocks, hideShrubs, hideTrees;
			int hideGrass;
			WORLDTEX_TYPE pavementType;
			COMMODITYID farmShrub, orchardTree;
			double farmShrubMSL, orchardTreeMSL;
			plantLoc = plantLL.GlobeLocation(SeaLevelRadius() + TerrainAltitudeMSLPave(plantLL, hideRocks, hideGrass, hideShrubs,
						hideTrees, pavementType, farmShrub, farmShrubMSL, orchardTree, orchardTreeMSL));
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
					vegPercent = VegetationAt(plantLL, plantAltMSL, inEnclosure, irrigated)/100.f;
				}
				else
					vegPercent = VegetationAt(plantLL, plantAltMSL, false, irrigated)/100.f;
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

			if(aquatic)
				aquaticgrass.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex), plant);
			else
				grass.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex), plant);
		}
	}
	//qDebug() << "DaWorldGasGiant::CreateGrass" << grass.count();
}

void DaWorldGasGiant::CreateShrubs(QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs,
		const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const
{
	int eqDivs = (int)(GlobeRadius()*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 12;												// 12' spacing for shrubs at highest density.

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
			LLOfLoc(plantLL, plantLoc);
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
			bool aquatic = false;;
			if(plant.m_CID == CID_Invalid)
			{
				// No plant is cultivated here.
				// Check to see if there is a natural growing shrub at that grid location.
				plant.m_CID = SpotShrubAt(plantLL, worldGenSeed, aquatic);
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

			if(aquatic)
				aquaticshrubs.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex),plant);
			else
				shrubs.insert(AuResKey(plant.m_VAOIndex, plant.m_ResourceZone, plantLL, latRegion, plant.m_WindIndex), plant);
		}
	}
	//qDebug() << "DaWorldGasGiant::CreateShrubs" << shrubs.count();
}

void DaWorldGasGiant::CreateTrees(QMap<AuResKey, AuGlobeTree>& trees, const AuPoint& groundLoc, const AuPoint& vx,
		const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const
{
	int eqDivs = (int)(GlobeRadius()*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 24;												// 24' spacing for trees at highest density.

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
			LLOfLoc(plantLL, plantLoc);
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
	//qDebug() << "DaWorldGasGiant::CreateTrees" << trees.count();
}

int DaWorldGasGiant::GroundLevelEquatorDivisions() const
{
	return EquatorDivisions100km(CoreRadius1000km()*10);
}

quint8 DaWorldGasGiant::CoreRadius1000km() const
{
	return m_byCoreRadius;
}

void DaWorldGasGiant::InitCoreRadius(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// Size of core is larger as the orbit zone gets cooler.
	// My radius is 20 to 40.
	int minCoreSize = m_Radius1000km/13;	// 1 to 3
	int maxCoreSize = m_Radius1000km/6;		// 3 to 6

	// Size of core tends to be larger as the orbit zone gets cooler.
	int mod;
	switch(OrbitZone())
	{
	case OZONE_Inferno:
		mod = -2;	// max core size in inferno zone is 4
		break;
	case OZONE_Inner:
		mod = -1;	// max core size in inner zone is 5
		break;
	default:
	case OZONE_Habitable:
	case OZONE_Outer:
		mod = 0;	// max core size in habitable and outer zones is 6
		break;
	case OZONE_Frigid:
		mod = 1;	// max core size in frigid zone is 7
		break;
	}

	// Generate core radius size of at least 1.
	int coreRadius;
	if(minCoreSize == maxCoreSize)
		coreRadius = minCoreSize + mod;
	else
	{
		int sizeRange = maxCoreSize - minCoreSize + 1;
		coreRadius = minCoreSize + dice.Roll(1,sizeRange,-1) + mod;
	}
	if(coreRadius <= 1)
		m_byCoreRadius = 1;
	else
		m_byCoreRadius = coreRadius;
}

int DaWorldGasGiant::SeaLevelEquatorDivisions() const
{
	return EquatorDivisions100km(CoreRadius1000km()*10) * 2;
}

void DaWorldGasGiant::CreateSurfaceTriangles(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const
{
	//qDebug() << "DaWorldGasGiant::CreateSurfaceTriangles" << Id() << Name() << "eqdivs" << eqdivs;
	// Create terrain cells for the globe based on longitude and latitude.
	// Each pole will have a single terrain cell.
	tris.clear();

	// Poles
	AuGlobeLL poleN(0,PI/2);									// North pole
	AuGlobeLL poleS(0,-PI/2);									// South pole

	// Remember highest and lowest terrain altitudes.
	maxAltMSL = TerrainAltitudeMSL(poleN);
	minAltMSL = maxAltMSL;
	float alt = TerrainAltitudeMSL(poleS);
	if(alt > maxAltMSL)
		maxAltMSL = alt;
	else if(alt < minAltMSL)
		minAltMSL = alt;

	// North and south hemisphere except poles and equator.
	int latdivshemi = eqdivs/4;									// Latitude divisions per hemisphere.
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	QList<AuGlobeLL> lastRowN, lastRowS;						// Previous rows of north and south globe locations.
	for(int latinc=1; latinc<latdivshemi; latinc++)
	{
		// We can do both hemispheres at once.
		double latitude = PI/2 - latDivAng * latinc;			// Range is PI/2 to 0.
		Q_ASSERT(latitude < PI/2);
		Q_ASSERT(latitude > 0);
		int londivs = (int)ceilf(cos(latitude) * eqdivs);
		if(londivs < 3)
			londivs = 3;										// Need at least 3 to draw triangles.
		double lonDivAng = (PI*2) / londivs;					// Radian angle per longitude division
		QList<AuGlobeLL> rowN, rowS;							// Current rows of north and south globe locations.
		AuGlobeLL lastN, lastS;
		int lastRowIdx=0;
		//qDebug() << "latinc" << latinc << "latdivshemi" << latdivshemi << "londivs" << londivs;
		for(int loninc=0; loninc<=londivs; loninc++)
		{
			double longitude;
			if(loninc == londivs)
				longitude = -PI;								// For tight closure we don't calculate that last longitude.
			else
				longitude = lonDivAng * loninc - PI;			// Range is -PI to PI.
			Q_ASSERT(longitude >= -PI);
			Q_ASSERT(longitude <= PI);
			//qDebug() << "loninc" << loninc;
			AuGlobeLL llN(longitude, latitude);					// North hemisphere
			AuGlobeLL llS(longitude, -latitude);				// South hemisphere

			// Remember highest and lowest terrain altitudes.
			alt = TerrainAltitudeMSL(llN);
			if(alt > maxAltMSL)
				maxAltMSL = alt;
			else if(alt < minAltMSL)
				minAltMSL = alt;
			alt = TerrainAltitudeMSL(llS);
			if(alt > maxAltMSL)
				maxAltMSL = alt;
			else if(alt < minAltMSL)
				minAltMSL = alt;

			if(loninc)
			{
				// Not the first location at this latitude so we can draw a triangle.
				if(latinc == 1)
				{
					// Tessellate the first row to the poles.
					tris.append(AuGlobeLLTri(poleN, llN, lastN));
					tris.append(AuGlobeLLTri(poleS, lastS, llS));
				}
				else
				{
					// Tessellate subsequent row to the previous row.
					tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], llN, lastN));
					tris.append(AuGlobeLLTri(lastRowS[lastRowIdx], lastS, llS));

					if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
					{
						// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
						// there and bump the previous row index.
						int nextLastRowIdx = lastRowIdx+1;
						if(nextLastRowIdx >= lastRowN.count())
							nextLastRowIdx = 0;					// Wrap to beginning of last row.
						tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llN));
						tris.append(AuGlobeLLTri(lastRowS[nextLastRowIdx], lastRowS[lastRowIdx], llS));
						lastRowIdx = nextLastRowIdx;
					}
				}
			}
			lastN = llN;
			lastS = llS;
			rowN.append(llN);
			rowS.append(llS);
		}
		lastRowN = rowN;
		lastRowS = rowS;
	}

	// Equator
	int londivs = eqdivs;
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	AuGlobeLL lastE;
	int lastRowIdx=0;
	for(int loninc=0; loninc<=londivs; loninc++)
	{
		double longitude;
		if(loninc == londivs)
			longitude = -PI;									// For tight closure we don't calculate that last longitude.
		else
			longitude = lonDivAng * loninc - PI;				// Range is -PI to PI.
		Q_ASSERT(longitude >= -PI);
		Q_ASSERT(longitude <= PI);
		AuGlobeLL llE(longitude, 0);							// Equator

		// Remember highest and lowest terrain altitudes.
		alt = TerrainAltitudeMSL(llE);
		if(alt > maxAltMSL)
			maxAltMSL = alt;
		else if(alt < minAltMSL)
			minAltMSL = alt;

		if(loninc)
		{
			// Not the first location at the equator so we can draw a triangle.
			// Tessellate equator row to the previous row.
			tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], llE, lastE));
			tris.append(AuGlobeLLTri(lastRowS[lastRowIdx], lastE, llE));

			if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
			{
				// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
				// there and bump the previous row index.
				int nextLastRowIdx = lastRowIdx+1;
				if(nextLastRowIdx >= lastRowN.count())
					nextLastRowIdx = 0;					// Wrap to beginning of last row.
				tris.append(AuGlobeLLTri(lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llE));
				tris.append(AuGlobeLLTri(lastRowS[nextLastRowIdx], lastRowS[lastRowIdx], llE));
				lastRowIdx = nextLastRowIdx;
			}
		}
		lastE = llE;
	}

	//qDebug() << "DaWorldGasGiant::CreateSurfaceTriangles" << Id() << Name() << "maxAlt" << maxAltMSL << "minAltMSL" << minAltMSL
		//<< "tris" << tris.count();
}

void DaWorldGasGiant::CreateAtGlobeSurfaceTriangle(AuGlobeTriAltVegN& tri, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
		const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const
{
	int texreps = GroundLevelEquatorDivisions() * TERRAIN_TEXTURE_REPS;

	// Wrap texture properly at the seam.
	float ll0long = ll0.Longitude();
	if(FuzzyEquals((float)ll0.Longitude(), -PIf) && (ll1.Longitude() > 0 || ll2.Longitude() > 0))
		ll0long = PIf;
	float ll1long = ll1.Longitude();
	if(FuzzyEquals((float)ll1.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll2.Longitude() > 0))
		ll1long = PIf;
	float ll2long = ll2.Longitude();
	if(FuzzyEquals((float)ll2.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll1.Longitude() > 0))
		ll2long = PIf;

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
		t.m_TerTex.setX((ll0long)/(PIf*2) * texreps * cosf(ll0.Latitude()));
		t.m_TerTex.setY((ll0.Latitude() + PIf/2)/PIf * texreps);
		ti0 = terAltInfo.insert(ll0, t);
	}
	else
	{
		(*ti0).m_TerTex.setX((ll0long)/(PIf*2) * texreps * cosf(ll0.Latitude()));
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
		t.m_TerTex.setX((ll1long)/(PIf*2) * texreps * cosf(ll1.Latitude()));
		t.m_TerTex.setY((ll1.Latitude() + PIf/2)/PIf * texreps);
		ti1 = terAltInfo.insert(ll1, t);
	}
	else
	{
		(*ti1).m_TerTex.setX((ll1long)/(PIf*2) * texreps * cosf(ll1.Latitude()));
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
		t.m_TerTex.setX((ll2long)/(PIf*2) * texreps * cosf(ll2.Latitude()));
		t.m_TerTex.setY((ll2.Latitude() + PIf/2)/PIf * texreps);
		ti2 = terAltInfo.insert(ll2, t);
	}
	else
	{
		(*ti2).m_TerTex.setX((ll2long)/(PIf*2) * texreps * cosf(ll2.Latitude()));
	}

	// Construct a globe triangle.
	tri.m_Vert0 = AuGlobeVertAltVegN((*ti0).m_TerCoord, (*ti0).m_TerTex, SeaLevelRadius() + (*ti0).m_TerAltMSL, (*ti0).m_Snow,
			(*ti0).m_Veg, (*ti0).m_TerNormal);
	tri.m_Vert1 = AuGlobeVertAltVegN((*ti1).m_TerCoord, (*ti1).m_TerTex, SeaLevelRadius() + (*ti1).m_TerAltMSL, (*ti1).m_Snow,
			(*ti1).m_Veg, (*ti1).m_TerNormal);
	tri.m_Vert2 = AuGlobeVertAltVegN((*ti2).m_TerCoord, (*ti2).m_TerTex, SeaLevelRadius() + (*ti2).m_TerAltMSL, (*ti2).m_Snow,
			(*ti2).m_Veg, (*ti2).m_TerNormal);
}

void DaWorldGasGiant::CreateAtSurfaceTriangles(QList<AuGlobeTerrainTriCen>& tricens) const
{
	// Each triangle in this tessellation becomes a region for higher tessellation on a per triangle basis.
	AuGlobeLLTriList tris;
	float maxAltMSL, minAltMSL;
	CreateSurfaceTrianglesFlat(tris, GroundLevelEquatorDivisions(), maxAltMSL, minAltMSL);

	// Build result with precalculated center points at core radius.
	double radius = GlobeRadius();
	for(AuGlobeLLTriList::const_iterator gti = tris.cbegin(); gti != tris.cend(); gti++)
	{
		AuGlobeTerrainTriCen tricen(*gti, radius);
		tricens.append(tricen);
	}
}

void DaWorldGasGiant::CreateFlatGlobeVAOTriangle(QList<AuGlobeTriAltVeg>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
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

	// Wrap texture properly at the seam.
	float ll0long = ll0.Longitude();
	if(FuzzyEquals((float)ll0.Longitude(), -PIf) && (ll1.Longitude() > 0 || ll2.Longitude() > 0))
	{
		ll0long = PIf;
		terCoord0.x += 2;
	}
	float ll1long = ll1.Longitude();
	if(FuzzyEquals((float)ll1.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll2.Longitude() > 0))
	{
		ll1long = PIf;
		terCoord1.x += 2;
	}
	float ll2long = ll2.Longitude();
	if(FuzzyEquals((float)ll2.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll1.Longitude() > 0))
	{
		ll2long = PIf;
		terCoord2.x += 2;
	}

	// Calculate texture coordinates.
	int texreps = GroundLevelEquatorDivisions() * TERRAIN_TEXTURE_REPS;
	QPointF terTex0;
	terTex0.setX((ll0long)/(PIf*2) * texreps * cosf(ll0.Latitude()));
	terTex0.setY((ll0.Latitude() + PIf/2)/PIf * texreps);
	QPointF terTex1;
	terTex1.setX((ll1long)/(PIf*2) * texreps * cosf(ll1.Latitude()));
	terTex1.setY((ll1.Latitude() + PIf/2)/PIf * texreps);
	QPointF terTex2;
	terTex2.setX((ll2long)/(PIf*2) * texreps * cosf(ll2.Latitude()));
	terTex2.setY((ll2.Latitude() + PIf/2)/PIf * texreps);

	// Calculate vegetation density.
	quint8 veg0 = VegetationAt(ll0, terAltMSL0, 0, false);
	quint8 veg1 = VegetationAt(ll1, terAltMSL1, 0, false);
	quint8 veg2 = VegetationAt(ll2, terAltMSL2, 0, false);

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

// Create the nearby model surface. Called by the model maker thread.
void DaWorldGasGiant::CreateNearGlobeSurfaceTriangles(QList<AuGlobeTriTexNShineSnow>& tris) const
{
	//qDebug() << "DaWorldGasGiant::CreateNearGlobeSurfaceTriangles" << Id() << Name() << "started...";
	// Tessellate surface for far model. This tessellation does not include any altitude information, though it does return the
	// altitude minimum and maximums MSL so each vertex only gets that calculation once. The min and max are used when creating
	// the terrain lookup table for the shader.
	AuGlobeLLTriList terrainTris;
	float maxAltMSL, minAltMSL;
	CreateSurfaceTriangles(terrainTris, GroundLevelEquatorDivisions()*7/2, maxAltMSL, minAltMSL);

	// Create the triangles that make up the model. These will have vertices at the appropriate altitude and texture coordinates
	// that accomodate the surface map.
	QMap<AuGlobeLL, AuGlobeTerrainAltInfo> terAltInfo;	// Info that gets cached along the way.
	tris.clear();
	for(AuGlobeLLTriList::const_iterator ti = terrainTris.cbegin(); ti != terrainTris.cend(); ti++)
		CreateNearGlobeSurfaceTriangle(tris, (*ti).m_LL0, (*ti).m_LL1, (*ti).m_LL2, terAltInfo);
	//qDebug() << "DaWorldGasGiant::CreateNearGlobeSurfaceTriangles" << Id() << Name() << "tris" << tris.count();
}

void DaWorldGasGiant::CreateNearGlobeSurfaceTriangle(QList<AuGlobeTriTexNShineSnow>& tris, const AuGlobeLL& ll0,
		const AuGlobeLL& ll1, const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const
{
	// Wrap texture properly at the seam.
	float ll0long = ll0.Longitude();
	if(FuzzyEquals((float)ll0.Longitude(), -PIf) && (ll1.Longitude() > 0 || ll2.Longitude() > 0))
		ll0long = PIf;
	float ll1long = ll1.Longitude();
	if(FuzzyEquals((float)ll1.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll2.Longitude() > 0))
		ll1long = PIf;
	float ll2long = ll2.Longitude();
	if(FuzzyEquals((float)ll2.Longitude(), -PIf) && (ll0.Longitude() > 0 || ll1.Longitude() > 0))
		ll2long = PIf;

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti0 = terAltInfo.find(ll0);
	if(ti0 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll0);
		t.m_TerCoord = GetNearGlobeSurfaceLocation(ll0, t.m_TerAltMSL, t.m_TerNormal);
		t.m_Shine = s_seaHardness;
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll0.Latitude(), 0);
		t.m_TerTex.setX(ll0long/(PIf*2) + .5f);
		t.m_TerTex.setY(ll0.Latitude()/PIf + .5f);
		ti0 = terAltInfo.insert(ll0, t);
	}
	else
	{
		(*ti0).m_TerTex.setX(ll0long/(PIf*2) + .5f);
	}

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti1 = terAltInfo.find(ll1);
	if(ti1 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll1);
		t.m_TerCoord = GetNearGlobeSurfaceLocation(ll1, t.m_TerAltMSL, t.m_TerNormal);
		t.m_Shine = s_seaHardness;
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll1.Latitude(), 0);
		t.m_TerTex.setX(ll1long/(PIf*2) + .5f);
		t.m_TerTex.setY(ll1.Latitude()/PIf + .5f);
		ti1 = terAltInfo.insert(ll1, t);
	}
	else
	{
		(*ti1).m_TerTex.setX(ll1long/(PIf*2) + .5f);
	}

	QMap<AuGlobeLL, AuGlobeTerrainAltInfo>::iterator ti2 = terAltInfo.find(ll2);
	if(ti2 == terAltInfo.end())
	{
		AuGlobeTerrainAltInfo t;
		t.m_TerAltMSL = TerrainAltitudeMSL(ll2);
		t.m_TerCoord = GetNearGlobeSurfaceLocation(ll2, t.m_TerAltMSL, t.m_TerNormal);
		t.m_Shine = s_seaHardness;
		t.m_Snow = SnowCoverage(t.m_TerAltMSL, ll2.Latitude(), 0);
		t.m_TerTex.setX(ll2long/(PIf*2) + .5f);
		t.m_TerTex.setY(ll2.Latitude()/PIf + .5f);
		ti2 = terAltInfo.insert(ll2, t);
	}
	else
	{
		(*ti2).m_TerTex.setX(ll2long/(PIf*2) + .5f);
	}

	// Construct a globe triangle.
	AuGlobeTriTexNShineSnow tri;
	tri.m_Vert0 = AuGlobeVertTexNShineSnow((*ti0).m_TerCoord, (*ti0).m_TerTex, (*ti0).m_TerNormal, (*ti0).m_Shine, (*ti0).m_Snow);
	tri.m_Vert1 = AuGlobeVertTexNShineSnow((*ti1).m_TerCoord, (*ti1).m_TerTex, (*ti1).m_TerNormal, (*ti1).m_Shine, (*ti1).m_Snow);
	tri.m_Vert2 = AuGlobeVertTexNShineSnow((*ti2).m_TerCoord, (*ti2).m_TerTex, (*ti2).m_TerNormal, (*ti2).m_Shine, (*ti2).m_Snow);

	// Append the globe triangle to the list.
	tris.append(tri);
}

// 'normal' is a return value
AuPoint DaWorldGasGiant::GetNearGlobeSurfaceLocation(const AuGlobeLL& ll, float altitudeMSL, AuPoint& normal) const
{
	// Calculate above-water locations at their appropriate altitude. This makes the model bumpy. It is not the altitude
	// used by the shader to determine texturing. That is placed artificially on each vertex to get deep sea colors even though
	// the sea surface is flat.
	if(altitudeMSL > 0)
	{
		normal = TerrainNormal(ll);
		return ll.GlobeLocation(SeaLevelRadius() + altitudeMSL);
	}

	// Undersea locations are at the sea surface for smooth sea surface on far globe model.
	normal = ll.GlobeLocation(1);	// Normal is globe location at radius 1, for flat sea surface.
	return normal * SeaLevelRadius();		// Location is at sea level radius.
}

void DaWorldGasGiant::DeleteFloraDeviceObjects(AuGLEffects& re, AuGLModelMaker& mm)
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

// Generates terrain parameters for this world.
void DaWorldGasGiant::CreateTerrainData(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_GasGiantLock.lockForWrite();

	// Configure parameters for generating terrain using 2D Perlin noise.
	m_TerrainXOff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	m_TerrainYOff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	m_TerrainFrequency = (float)dice.Roll(1, CoreRadius1000km()*10000, 10000) / 10000.f;
	m_TerrainFrequency *= (float)CoreRadius1000km() / (float)::Radius100km(20);
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
	float persAdj = .399f + (float)dice.Roll(1,261)/1000.f;	// Adjustment is .400 to .660, by thousandths.
	m_TerrainPersFactor *= persAdj;

	//qDebug() << "DaWorldGasGiant::CreateTerrainData" << Id() << Name()
		//<< "freq" << m_TerrainFrequency << "pers" << m_TerrainPersistence << "persFactor" << m_TerrainPersFactor
		//<< "persAdj" << persAdj << "treeLine" << TreeLineMSL();

	m_GasGiantLock.unlock();
}

void DaWorldGasGiant::CreateGeologicalFeatures(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// Fissures and vents are the only things that work way under the sea.
	quint8 geoTypes[] =
	{
		AuGeoFissure::GeoType(),
		AuGeoFissure::GeoType(),
		AuGeoVent::GeoType(),
		AuGeoVent::GeoType(),
		AuGeoVent::GeoType(),
	};
	int geoTypeCount = sizeof(geoTypes) / sizeof(quint8);

	// Larger worlds have more geological features.
	int features = dice.Roll(2, 4, 2);											// 4 to 10 features

	// Create the geological features, including at least one vent for smoke.
	bool vent = false;
	AuGeologicalFeatures& geof = LockGeologicalFeaturesForWrite();
	Q_ASSERT(geof.isEmpty());
	do
	{
		quint8 geoType = dice.Roll(1, geoTypeCount, -1);
		AuGeologicalFeature* geo = AuGeologicalFeature::Create(geoTypes[geoType]);
		if(!geo)
			continue;	// Shouldn't happen.
		geo->RandomInit(*this, dice, geof);
		dice.Random();
		geof.append(geo);
		if(dynamic_cast<AuGeoVent*>(geo))
			vent = true;
		features--;
	}
	while(features > 0 || !vent);
	UnlockGeologicalFeatures();
}

// Returns false if nuLoc or oldLoc is outside the atmosphere.
bool DaWorldGasGiant::FilterDestinationLoc(DaAtom* moveAtom, const AuPoint& oldLoc, AuPoint& nuLoc, int elapsedMs) const
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
 		double terrainAlt = SurfaceAltitudeMSL(nuLL) + SeaLevelRadius();		// Altitude of terrain ground surface.
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

void DaWorldGasGiant::RenderFlora(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
		const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	Q_UNUSED(projection);
	if(g_Au.m_DebugSwitches & 0x00000200)
		return;

	AuGlobeLLF viewerLL;
	//if(!LLOfLoc(viewerLL, viewerPos))
		//return;
	LLOfLoc(viewerLL, viewerPos);	// There are no invalid loc to LL conversions on a globe.

	Q_ASSERT(m_pFlora);
	if(!m_FloraVAOsHigh)
	{
		// We will create a static array big enough for every possible VAO. We will only create the individual VAOs as they are
		// referenced so we don't do all that grunt work at once.
		int maxFlora = PLANT_VAOS * AuFloraDNA::Zones * (1 + ZONE_TREES + ZONE_SHRUBS) * ResourceZones();
		m_FloraVAOsHigh = new AuGLVAO*[maxFlora];
		AuGLVAO** pvao = m_FloraVAOsHigh;
		for(int fauna=0; fauna<maxFlora; fauna++, pvao++)
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
		//qDebug() << "DaWorldGasGiant::RenderFlora" << Id() << Name() << "dist" << m_FloraViewer.Distance(groundPos)
			//<< "updating flora...";
		// mm returns plants when it is ready
		if(mm.CreateFlora(this, groundPos, m_Grass, m_Shrubs, m_Trees, m_AquaticGrass, m_AquaticShrubs))
		{
			// Model maker is instructed to delete the plant distribution model. We have what we need. When the viewer moves far
			// enough, we'll ask for a nu plant distribution model. They're too transient to keep around.
			m_FloraViewer = groundPos;							// mm updates viewer pos with pos of actual plant distribution
			mm.DeleteFloraModel(Id());
		}
	}

//#if !defined(QT_NO_DEBUG)
	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_BACK, GL_LINE);
//#endif

	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.GLSL().FloraShader().Select(projection, re);
	re.GLSL().FloraShader().SetModelView(model);				// Model view matrix of the globe.
	re.GLSL().FloraShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Globe position in eye space is needed to calculate sun over-the-horizon effect.
	AuPoint globeP = model.Project(AuPoint(0,0));
	re.GLSL().FloraShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Ambient light
	re.GLSL().FloraShader().SetAmbientLight(ambientLight, ambientLight, ambientLight);
	AuPoint ambientUp = model.ProjectVector(vup);				// Up vector in eye space for spherical ambient light calc.
	re.GLSL().FloraShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Configure glow state.
	float glowR, glowG, glowB;
	if(!m_Grass.isEmpty())
	{
		// All grass is the same specie so we only do this once for the grass.
		// We also do not apply city ambient light glow to grass because it's too numerous.
		glowR = (*m_Grass.begin()).m_GlowR;
		glowG = (*m_Grass.begin()).m_GlowG;
		glowB = (*m_Grass.begin()).m_GlowB;
	}
	else
	{
		glowR = 0;
		glowG = 0;
		glowB = 0;
	}
	float plantAlpha = 1.f, curPlantAlpha = 1.f;
	re.GLSL().FloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);

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
			re.GLSL().FloraShader().SetTexture(m_pFlora[resourceZone].Texture());
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curPlantAlpha = plantAlpha;
			re.GLSL().FloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
			if(plantAlpha >= 1.f)
				re.glDisable(GL_BLEND);
			else
				re.glEnable(GL_BLEND);
		}

		// Select a wind vector for the plant.
		if((*gi).m_WindIndex != windIndex)
		{
			windIndex = (*gi).m_WindIndex;
			re.GLSL().FloraShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
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
		re.GLSL().FloraShader().SetPlantMatrix((*gi).Transform());

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
			re.GLSL().FloraShader().SetTexture(m_pFlora[resourceZone].Texture());
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
			re.GLSL().FloraShader().SetAmbientLight(ambR, ambG, ambB);
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curPlantAlpha = plantAlpha;
			re.GLSL().FloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
			if(plantAlpha >= 1.f)
				re.glDisable(GL_BLEND);
			else
				re.glEnable(GL_BLEND);
		}

		// Select a wind vector for the plant.
		if((*gi).m_WindIndex != windIndex)
		{
			windIndex = (*gi).m_WindIndex;
			re.GLSL().FloraShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
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
		re.GLSL().FloraShader().SetPlantMatrix((*gi).Transform());

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
			re.GLSL().FloraShader().SetTexture(m_pFlora[resourceZone].Texture());
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
			re.GLSL().FloraShader().SetAmbientLight(ambR, ambG, ambB);
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curPlantAlpha = plantAlpha;
			re.GLSL().FloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
			if(plantAlpha >= 1.f)
				re.glDisable(GL_BLEND);
			else
				re.glEnable(GL_BLEND);
		}

		// Select a wind vector for the plant.
		if((*gi).m_WindIndex != windIndex)
		{
			windIndex = (*gi).m_WindIndex;
			re.GLSL().FloraShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
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
		re.GLSL().FloraShader().SetPlantMatrix((*gi).Transform());

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

	re.GLSL().FloraShader().Deselect();

	// Aquatic plants
	if(!m_AquaticGrass.empty() || !m_AquaticShrubs.empty())
	{
		// Create water current vectors by scaling down the wind vectors.
		AuPoint waterVec[10];
		for(int iwv=0; iwv<10; iwv++)
			waterVec[iwv] = windVec[iwv]/2;

		re.GLSL().AquaticFloraShader().Select(projection, re);
		re.GLSL().AquaticFloraShader().SetModelView(model);				// Model view matrix of the globe.
		re.GLSL().AquaticFloraShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Globe position in eye space is needed to calculate sun over-the-horizon effect.
		re.GLSL().AquaticFloraShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

		// Ambient light
		re.GLSL().AquaticFloraShader().SetAmbientLight(ambientLight, ambientLight, ambientLight);
		re.GLSL().AquaticFloraShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		float plantAlpha = 1.f, curPlantAlpha = 1.f;
		int resourceZone = -1;
		int windIndex = -1;
		AuGLVAO* curvao = 0;
		if(!m_AquaticGrass.empty())
		{
			// Configure glow state.
			// All grass is the same specie so we only do this once for the grass.
			// We also do not apply city ambient light glow to grass because it's too numerous.
			glowR = (*m_AquaticGrass.begin()).m_GlowR;
			glowG = (*m_AquaticGrass.begin()).m_GlowG;
			glowB = (*m_AquaticGrass.begin()).m_GlowB;
			re.GLSL().AquaticFloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);

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
					re.GLSL().AquaticFloraShader().SetTexture(m_pFlora[resourceZone].Texture());
				}

				// Configure glow.
				if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
				{
					glowR = (*gi).m_GlowR;
					glowG = (*gi).m_GlowG;
					glowB = (*gi).m_GlowB;
					curPlantAlpha = plantAlpha;
					re.GLSL().AquaticFloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
					if(plantAlpha >= 1.f)
						re.glDisable(GL_BLEND);
					else
						re.glEnable(GL_BLEND);
				}

				// Select a wind vector for the plant.
				if((*gi).m_WindIndex != windIndex)
				{
					windIndex = (*gi).m_WindIndex;
					re.GLSL().AquaticFloraShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
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
				re.GLSL().AquaticFloraShader().SetPlantMatrix((*gi).Transform());

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
			re.GLSL().AquaticFloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
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
					re.GLSL().AquaticFloraShader().SetTexture(m_pFlora[resourceZone].Texture());
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
					re.GLSL().AquaticFloraShader().SetAmbientLight(ambR, ambG, ambB);
				}

				// Configure glow.
				if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || plantAlpha != curPlantAlpha)
				{
					glowR = (*gi).m_GlowR;
					glowG = (*gi).m_GlowG;
					glowB = (*gi).m_GlowB;
					curPlantAlpha = plantAlpha;
					re.GLSL().AquaticFloraShader().SetGlowAlpha(glowR, glowG, glowB, plantAlpha);
					if(plantAlpha >= 1.f)
						re.glDisable(GL_BLEND);
					else
						re.glEnable(GL_BLEND);
				}

				// Select a wind vector for the plant.
				if((*gi).m_WindIndex != windIndex)
				{
					windIndex = (*gi).m_WindIndex;
					re.GLSL().AquaticFloraShader().SetForceVector(windVec[windIndex].x, windVec[windIndex].y, windVec[windIndex].z);
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
				re.GLSL().AquaticFloraShader().SetPlantMatrix((*gi).Transform());

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
		re.GLSL().AquaticFloraShader().Deselect();
	}

	re.glDisable(GL_BLEND);
//#if !defined(QT_NO_DEBUG)
	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
//#endif
}

void DaWorldGasGiant::RenderRocks(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
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
	LLOfLoc(viewerLL, viewerPos);
	AuPoint vup = -GravityDir(viewerPos);
	AuPoint groundPos = GetSurfaceLocation(viewerLL);
	if(m_RockViewer.Distance2(groundPos) > 50*FeetPerMeter()*50*FeetPerMeter())
	{
		//qDebug() << "DaWorldGasGiant::RenderRocks" << Id() << Name() << "dist" << m_RockViewer.Distance(groundPos)
			//<< "updating rocks...";
		if(mm.CreateRocks(this, groundPos, m_Rocks))
		{
			// Model maker is instructed to delete the rock distribution model. We have what we need. When the viewer moves far
			// enough, we'll ask for a nu rock distribution model. They're too transient to keep around.
			m_RockViewer = groundPos;
			mm.DeleteRockModel(Id());
			//qDebug() << "DaWorldGasGiant::RenderRocks" << Id() << Name() << "updated" << m_Rocks.count() << "rocks";
		}
	}

//#if !defined(QT_NO_DEBUG)
	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_BACK, GL_LINE);
//#endif

	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.GLSL().RockShader().Select(projection, re);
	re.GLSL().RockShader().SetModelView(model);				// Model view matrix of the globe.
	re.GLSL().RockShader().SetFog(fogDensity, fogR, fogG, fogB);
	re.GLSL().RockShader().SetTexture(s_TexRock);

	// Globe position in eye space is needed to calculate sun over-the-horizon effect.
	AuPoint globeP = model.Project(AuPoint(0,0));
	re.GLSL().RockShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Ambient light
	re.GLSL().RockShader().SetAmbientLight(ambientLight, ambientLight, ambientLight);
	AuPoint ambientUp = model.ProjectVector(vup);				// Up vector in eye space for spherical ambient light calc.
	re.GLSL().RockShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Configure glow state.
	float glowR=0, glowG=0, glowB=0;
	float rockAlpha = 1.f, curRockAlpha = 1.f;
	re.GLSL().RockShader().SetGlowAlpha(glowR, glowG, glowB, rockAlpha);

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
			re.GLSL().RockShader().SetAmbientLight(ambR, ambG, ambB);
		}

		// Configure glow.
		if((*gi).m_GlowR != glowR || (*gi).m_GlowG != glowG || (*gi).m_GlowB != glowB || rockAlpha != curRockAlpha)
		{
			glowR = (*gi).m_GlowR;
			glowG = (*gi).m_GlowG;
			glowB = (*gi).m_GlowB;
			curRockAlpha = rockAlpha;
			re.GLSL().RockShader().SetGlowAlpha(glowR, glowG, glowB, rockAlpha);
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
			re.GLSL().RockShader().SetRockMatrix((*gi).Transform());

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

	re.GLSL().RockShader().Deselect();

	re.glDisable(GL_BLEND);
//#if !defined(QT_NO_DEBUG)
	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
//#endif
}

void DaWorldGasGiant::RenderHenges(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
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
		m_HengeColumn->GLModel(*ari.re);					// GL display list isn't done yet. Attempt to create the GL display list.

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

	AuPoint globeP = model.Translation();		// Globe's position in eye space.

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
			m_HengeFirePit->RenderGLSLModelTexFogGlobe(projection, hengeM, *ari.re, globeP, ambientR, ambientG, ambientB, ambientUp,
					false, fogDensity, fogR, fogG, fogB);

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
				// Slab sunken and tilted.
				rockM.Translate(x, y, -dice.Roll(1,40,20)/10.);	// Sunken
				rockM.RotateZ(ang);

				// Tilted
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

			m_HengeRock->RenderGLSLModelTexFogGlobe(projection, rockM, *ari.re, globeP, ambientR, ambientG, ambientB, ambientUp,
					false, fogDensity, fogR, fogG, fogB);
		}

		// Draw a ring of outer columns with lintels.
		double cola = PI*2/colCount;
		for(int col=0; col<colCount; col++)
		{
			double ang = cola * col;
			double x = cos(ang) * colRad;
			double y = sin(ang) * colRad;

			AuMatrix4x4 colM(hengeM);
			int orient = dice.Roll(1,100);
			if(orient <= 25)
			{
				// Lintel missing. Flip the column so the lintel is hidden under ground.
				colM.Translate(x, y, 8);
				colM.RotateY(PI);
				colM.RotateZ(-(ang - PI/2) - cola/2);
			}
			else
			{
				// Lintel present.
				colM.Translate(x, y, 0);
				colM.RotateZ(ang + PI/2 + cola/2);
			}

			// Skip columns behind the viewer.
			AuPoint px = colM.Translation();
			if(px.z > 0)
				continue;

			m_HengeColumn->RenderGLSLModelTexFogGlobe(projection, colM, *ari.re, globeP, ambientR, ambientG, ambientB, ambientUp,
					false, fogDensity, fogR, fogG, fogB);
		}
	}
	UnlockGeologicalFeatures();
}

void DaWorldGasGiant::UpdateNearSurfaceTexture(AuAtomRenderInfo& ari)
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
void DaWorldGasGiant::RenderAtGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB)
{
	//qDebug() << "DaWorldGasGiant::RenderAtGlobe" << Id() << Name() << "vao" << m_AtGlobe.VAO();
	if(!m_AtGlobe.m_Low.VAO())
	{
		CreateAtGlobe(*ari.re, ari.mm);			// Request low-tessellated 'at' globe model from background model maker thread.
		if(!m_AtGlobe.m_Low.VAO())
		{
			// At-this-world globe model is not ready so render the near globe model instead.
			//RenderNearGlobeFog(projection, model, ari, ambientLight, fogDensity, fogR, fogG, fogB);
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
	AuGlobeLL viewerLL;
	LLOfLoc(viewerLL, viewerPos);
	double radius = GlobeRadius();
	AuPoint groundPos = viewerLL.GlobeLocation(radius);
	if(CanHaveFlora() || BdHere())
	{
		//qDebug() << "TerrainLitFloraShader" << m_AtGlobe.m_MinAltMSL << m_AtGlobe.m_MaxAltMSL << m_AtGlobe.m_TexLookup;
		// Flora shader.
		ari.re->GLSL().TerrainLitFloraShader().Select(projection);
		ari.re->GLSL().TerrainLitFloraShader().SetModelView(model);
		ari.re->GLSL().TerrainLitFloraShader().SetNoiseTexture(s_TexNoise);
		ari.re->GLSL().TerrainLitFloraShader().SetPavementPalette(s_TexTerPalette);	// First ones are the commonly used pavements.
		ari.re->GLSL().TerrainLitFloraShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
		ari.re->GLSL().TerrainLitFloraShader().SetTerrainPalette(m_AtGlobe.m_TerElevTex);
		ari.re->GLSL().TerrainLitFloraShader().SetGrassTexture(m_glTexGrass);
		ari.re->GLSL().TerrainLitFloraShader().SetTerrainLookup(m_AtGlobe.m_TexLookup);
		ari.re->GLSL().TerrainLitFloraShader().SetAltitudeNoiseFactor(m_AtGlobe.AltNoiseFactor(AltitudeIncrement()/3));
		ari.re->GLSL().TerrainLitFloraShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Ambient light
		ari.re->GLSL().TerrainLitFloraShader().SetAmbientLight(ambientLight);
		const QColor& c = AuGLLight::ColorSodiumVapor();
		ari.re->GLSL().TerrainLitFloraShader().SetCityAmbientLight(c.redF() * (float)AuGLLight::AmbientLitCity(),
				c.greenF() * (float)AuGLLight::AmbientLitCity(), c.blueF() * (float)AuGLLight::AmbientLitCity());

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

			// Configure shader for object space lighting.
			ari.re->GLSL().TerrainLitFloraShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
			ari.re->GLSL().TerrainLitFloraShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
					light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
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
					ari.mm.GlobeModelTri(*ari.re, vao, this, vidx, *tti, m_AtGlobe.m_MaxAltMSL, m_AtGlobe.m_MinAltMSL);
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
				m_AtGlobe.DeleteGlobeTriVAO(Id(), vidx, ari.mm);
			}
		}
		ari.re->GLSL().TerrainLitFloraShader().Deselect();
	}
	else
	{
		// No flora shader.
		ari.re->GLSL().TerrainLitShader().Select(projection);
		ari.re->GLSL().TerrainLitShader().SetModelView(model);
		ari.re->GLSL().TerrainLitShader().SetNoiseTexture(s_TexNoise);
		ari.re->GLSL().TerrainLitShader().SetPavementPalette(s_TexTerPalette);	// First ones are the commonly used pavements.
		ari.re->GLSL().TerrainLitShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
		ari.re->GLSL().TerrainLitShader().SetTerrainPalette(m_AtGlobe.m_TerElevTex);
		ari.re->GLSL().TerrainLitShader().SetTerrainLookup(m_AtGlobe.m_TexLookup);
		ari.re->GLSL().TerrainLitShader().SetAltitudeNoiseFactor(m_AtGlobe.AltNoiseFactor(AltitudeIncrement()/3));
		ari.re->GLSL().TerrainLitShader().SetFog(fogDensity, fogR, fogG, fogB);

		// Ambient light
		ari.re->GLSL().TerrainLitShader().SetAmbientLight(ambientLight);
		const QColor& c = AuGLLight::ColorSodiumVapor();
		ari.re->GLSL().TerrainLitShader().SetCityAmbientLight(c.redF() * (float)AuGLLight::AmbientLitCity(),
				c.greenF() * (float)AuGLLight::AmbientLitCity(), c.blueF() * (float)AuGLLight::AmbientLitCity());

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

			// Configure shader for object space lighting.
			ari.re->GLSL().TerrainLitShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
			ari.re->GLSL().TerrainLitShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
					light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
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
					ari.mm.GlobeModelTri(*ari.re, vao, this, vidx, *tti, m_AtGlobe.m_MaxAltMSL, m_AtGlobe.m_MinAltMSL);
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
				m_AtGlobe.DeleteGlobeTriVAO(Id(), vidx, ari.mm);
			}
		}
		ari.re->GLSL().TerrainLitShader().Deselect();
	}
	//AuGLTexture::OutputGlError("DaWorldGasGiant::RenderAtGlobe finished terrain patches.");

	// Select clipping parameters to omit the fragments of the globe that are within the highly tessellated globe arc of the
	// viewer's position.
	float clipCos;
	AuPoint clipUp;
	if((float)countShown/(float)countAttempted < .7071f)
	{
		// We did not draw all of the highly tessellated ground patches. Don't make a hole until they are all present.
		clipCos = 1;											// No clipping when there are no highly tessellated VAOs ready yet.
	}
	else
	{
		// We drew all of the highly tessellated ground patches we wanted to display. Set clipping to cut out the circle.
		float arcAngle = g_Au.DetailedTerrainRangeFt() / radius;	// Angle of arc to the detailed terrain creation range.
		clipCos = cosf(arcAngle);
		clipUp = viewerPos;
		clipUp.Unit();
	}

	//qDebug() << "DaWorldGasGiant::RenderAtGlobe" << Id() << Name() << "attempted" << countAttempted << "shown" << countShown
		//<< "clipCos" << clipCos;

	// Render low tessellation of globe model, with shader configured to omit the portion near the viewer.
	if(CanHaveFlora())
	{
		//qDebug() << "GlobeLitFloraShader" << m_AtGlobe.m_MinAltMSL << m_AtGlobe.m_MaxAltMSL << m_AtGlobe.m_TexLookup;
		// Flora shader.
		ari.re->GLSL().GlobeLitFloraShader().Select();
		ari.re->GLSL().GlobeLitFloraShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().GlobeLitFloraShader().SetNoiseTexture(s_TexNoise);
		ari.re->GLSL().GlobeLitFloraShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
		ari.re->GLSL().GlobeLitFloraShader().SetTerrainPalette(m_AtGlobe.m_TerElevTex);
		ari.re->GLSL().GlobeLitFloraShader().SetGrassTexture(m_glTexGrass);
		ari.re->GLSL().GlobeLitFloraShader().SetTerrainLookup(m_AtGlobe.m_TexLookup);
		ari.re->GLSL().GlobeLitFloraShader().SetAltitudeNoiseFactor(m_AtGlobe.AltNoiseFactor(AltitudeIncrement()/3));
		ari.re->GLSL().GlobeLitFloraShader().SetFog(fogDensity, fogR, fogG, fogB);
		ari.re->GLSL().GlobeLitFloraShader().SetClip(clipCos, clipUp.x, clipUp.y, clipUp.z);

		// Ambient light
		ari.re->GLSL().GlobeLitFloraShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

			// Configure shader for object space lighting.
			ari.re->GLSL().GlobeLitFloraShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
		}
		//AuGLTexture::OutputGlError("DaWorldGasGiant::RenderAtGlobe lit.");

		// Display low tessellation of globe.
		m_AtGlobe.m_Low.SelectGL();
		m_AtGlobe.m_Low.DrawGL();
		m_AtGlobe.m_Low.DeselectGL();
		//AuGLTexture::OutputGlError("DaWorldGasGiant::RenderAtGlobe rendered.");
		ari.re->GLSL().GlobeLitFloraShader().Deselect();
		//AuGLTexture::OutputGlError("DaWorldGasGiant::RenderAtGlobe deselect.");
	}
	else
	{
		// No flora shader.
		ari.re->GLSL().GlobeLitShader().Select();
		ari.re->GLSL().GlobeLitShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().GlobeLitShader().SetNoiseTexture(s_TexNoise);
		ari.re->GLSL().GlobeLitShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
		ari.re->GLSL().GlobeLitShader().SetTerrainPalette(m_AtGlobe.m_TerElevTex);
		ari.re->GLSL().GlobeLitShader().SetTerrainLookup(m_AtGlobe.m_TexLookup);
		ari.re->GLSL().GlobeLitShader().SetAltitudeNoiseFactor(m_AtGlobe.AltNoiseFactor(AltitudeIncrement()/3));
		ari.re->GLSL().GlobeLitShader().SetFog(fogDensity, fogR, fogG, fogB);
		ari.re->GLSL().GlobeLitShader().SetClip(clipCos, clipUp.x, clipUp.y, clipUp.z);

		// Ambient light
		ari.re->GLSL().GlobeLitShader().SetAmbientLight(ambientLight);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

			// Configure shader for object space lighting.
			ari.re->GLSL().GlobeLitShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
					sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
		}

		// Display low tessellation of globe.
		m_AtGlobe.m_Low.SelectGL();
		m_AtGlobe.m_Low.DrawGL();
		m_AtGlobe.m_Low.DeselectGL();
		ari.re->GLSL().GlobeLitShader().Deselect();
	}
	//AuGLTexture::OutputGlError("DaWorldGasGiant::RenderAtGlobe finished globe.");

	// Delete terrain triangle VAOs that get too old so we don't keep too many of them around.
	m_AtGlobe.DeleteOldGlobeTriVAOs(Id(), ari.mm);

//#if !defined(QT_NO_DEBUG)
	//glPolygonMode(GL_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
//#endif
}

TER_ELEV_TYPE DaWorldGasGiant::TerrainElevationType(float altitudeMSL) const
{
	float altInc = (float)AltitudeIncrement();
	if(altitudeMSL >= 0)
		return TER_Alpine;	// wut?
	double coreRadius = GlobeRadius();
	double altMSCore = (SeaLevelRadius() + altitudeMSL) - coreRadius;	// Altitude above/below the core.
	if(altMSCore > altInc*2)
		return TER_Highlands;
	else if(altMSCore > altInc)
		return TER_Midlands;
	else if(altMSCore > 0)
		return TER_Lowlands;
	else if(altMSCore >= -altInc)
		return TER_Coastal;
	else if(altMSCore >= -altInc*2)
		return TER_ContinentalShelf;
	return TER_SeaFloor;
}

QImage DaWorldGasGiant::CreateAtGasGiantLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const
{
	// Select textures to use for the various terrain elevations.
	/*
	// Inferno
	terElevTex[0] = s_TexTerPalette[WORLDTEX_RockGrayHeavy];	// TER_Peak, TER_Alpine, and TER_Highlands are ALWAYS the same.
	terElevTex[1] = s_TexTerPalette[WORLDTEX_RockGrayCoarse];	// TER_Midlands
	terElevTex[2] = s_TexTerPalette[WORLDTEX_RockRedModerate];	// TER_Lowlands
	terElevTex[3] = s_TexTerPalette[WORLDTEX_RockRedHeavy];		// TER_Coastal
	terElevTex[4] = s_TexTerPalette[WORLDTEX_RockGrayCoarse];	// TER_ContinentalShelf, TER_SeaFloor, TER_SeaTrenct
	Q_ASSERT(terElevTex[0]);
	// Frigid
	terElevTex[0] = s_TexTerPalette[WORLDTEX_IceModerate];		// TER_Peak, TER_Alpine, and TER_Highlands are ALWAYS the same.
	terElevTex[1] = s_TexTerPalette[WORLDTEX_RockGrayModerate];	// TER_Midlands
	terElevTex[2] = s_TexTerPalette[WORLDTEX_DirtModerate];		// TER_Lowlands
	terElevTex[3] = s_TexTerPalette[WORLDTEX_Pebbles];			// TER_Coastal
	terElevTex[4] = s_TexTerPalette[WORLDTEX_RockGrayHeavy];	// TER_ContinentalShelf, TER_SeaFloor, TER_SeaTrenct
	*/
	terElevTex[0] = s_TexTerPalette[WORLDTEX_IceModerate];		// TER_Peak, TER_Alpine, and TER_Highlands are ALWAYS the same.
	terElevTex[1] = s_TexTerPalette[WORLDTEX_SnowModerate];		// TER_Midlands
	terElevTex[2] = s_TexTerPalette[WORLDTEX_RockGraySmooth];	// TER_Lowlands
	terElevTex[3] = s_TexTerPalette[WORLDTEX_RockGrayHeavy];	// TER_Coastal
	terElevTex[4] = s_TexTerPalette[WORLDTEX_RockGrayCoarse];	// TER_ContinentalShelf, TER_SeaFloor, TER_SeaTrenct

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

void DaWorldGasGiant::CreateAtGlobe(AuGLEffects& re, AuGLModelMaker& mm)
{
	Q_ASSERT(!m_AtGlobe.m_Low.VAO());

	// Request low-tessellation of at-globe model from model maker thread. We may make this request many times before it succeeds.
	mm.GlobeModel(re, m_AtGlobe, this);
	if(m_AtGlobe.m_Low.VAO())
	{
		//qDebug() << "DaWorldGasGiant::CreateAtGlobe" << Id() << Name() << "terrainTris" << m_AtGlobe.m_TerrainTris.count();
		// We got a globe model so create the lookup tables used for texture selection based on altitudes of terrain.
		// Altitude range of all land, for altitude factor calculation.
		m_AtGlobe.m_TexLookup = CreateAtLookupTable(re, m_AtGlobe.m_MaxAltMSL, m_AtGlobe.m_MinAltMSL, m_AtGlobe.m_TerElevTex);
	}
}

// Create the lookup table used to index the tiles in the texture atlas
// Some planet types have different rules to follow
GLuint DaWorldGasGiant::CreateAtLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const
{
	//qDebug() << "DaWorldGasGiant::CreateAtLookupTable" << Id() << Name() << maxAltMSL << minAltMSL;
	QImage image = CreateAtGasGiantLookupTable(maxAltMSL, minAltMSL, terElevTex);

	// Use nearest filter for the lookup table texture because the values stored in the texture must not be linear interpolated.
	QString lutName = QString("%1.LUT").arg(Id().StringId());
	GLuint lu = re.CreateTextureFromBitmap(Id().GetID(), lutName, image, GL_NEAREST, GL_NEAREST);
	Q_ASSERT(lu);
	return lu;
}

void DaWorldGasGiant::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaAtom* sceneAtom) const
{
	Q_UNUSED(sceneAtom);
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
		idPanels.append(0x4000|CID_Hay);		// aquatic plants only produce vegetation
		qres1.Transform(tModel);
		aControlPanels.append(qres1);
		idPanels.append(0x4000|CID_Hay);		// aquatic plants only produce vegetation
		qres2.Transform(tModel);
		aControlPanels.append(qres2);
		idPanels.append(0x4000|CID_Hay);		// aquatic plants only produce vegetation
		qresV0.Transform(tModel);
		aControlPanels.append(qresV0);
		idPanels.append(0x4000|CID_Hay);		// aquatic plants only produce vegetation
		qresV1.Transform(tModel);
		aControlPanels.append(qresV1);
		idPanels.append(0x4000|CID_Hay);		// aquatic plants only produce vegetation
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
}

void DaWorldGasGiant::EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
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
void DaWorldGasGiant::SampleControlPanelInput(qint16 idPanel, const AuPoint& /*fPanelLoc*/,
		DiControlInputArray& aActiveControls, bool /*bContinuous*/)
{
	aActiveControls.append(DiControlInput(idPanel, CI_Forage));
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DaWorldGasGiant::ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& pos,
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

double DaWorldGasGiant::SeaLevelAltitude() const
{
	return SeaLevelRadius();
}

void DaWorldGasGiant::RenderSea(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool eyeUnderWater,
		bool cgUnderWater, float illumination, float ambientLight, float fogDensity, float fogR, float fogG, float fogB,
		bool viewTopDown)
{
	if(eyeUnderWater)
	{
		// Render only the transparent sea surface model.
		// Must reverse cull direction because eye is under the water.
		re.glFrontFace(GL_CCW);
		RenderSeaGlobe(projection, model, re, false, illumination, ambientLight, cgUnderWater, fogDensity, fogR, fogG, fogB,
				viewTopDown);
		re.glFrontFace(GL_CW);
	}
	else
		RenderSeaGlobe(projection, model, re, false, illumination, ambientLight, cgUnderWater, fogDensity, fogR, fogG, fogB,
				viewTopDown);
}

// Render the sea surface.
// Lights have already been set up.
// View and World transforms have already been set up.
// This model is only displayed when the viewer is in the atmosphere.
void DaWorldGasGiant::RenderSeaGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool bothLayers, float illumination, float ambientLight, bool showUndersideOfSurface, float fogDensity,
		float fogR, float fogG, float fogB, bool topDownView)
{
	Q_UNUSED(projection);
	if(g_Au.m_DebugSwitches & 0x00000400)
		return;
	// Not much sunlight reaches sea level.
	illumination *= .1f;
	ambientLight *= .1f;

	if(!m_SeaGlobe.VAO() || (bothLayers && !m_UnderseaGlobe.VAO()))
	{
		// We need to create one or both of the sea globe vertex array objects.
		// Tessellate a globe for the sea surfaces. This tessellation only expresses latitude and longitude, not altitude of points.
		//qDebug() << "DaWorldGasGiant::RenderSeaGlobe" << Id() << Name() << "tessellating...";
		AuGlobeLLTriList seaTris;
		seaTris.CreateGlobeTriangles(SeaLevelEquatorDivisions()*2);
		//qDebug() << "DaWorldGasGiant::RenderSeaGlobe" << Id() << Name() << "done tessellating" << seaTris.count() << "tris";

		double radius = SeaLevelRadius();

		if(!m_SeaGlobe.VAO())
		{
			// Create the sea surface globe vertex array object.
			int texreps = SeaLevelEquatorDivisions() * 3;			// More than the lower layer makes this layer move more quickly.

			// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
			QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
			for(AuGlobeLLTriList::const_iterator ti = seaTris.cbegin(); ti != seaTris.cend(); ti++)
			{
				//if(VirginTerrainAltitudeMSL((*ti).m_LL0) > 1 &&
						//VirginTerrainAltitudeMSL((*ti).m_LL1) > 1 &&
						//VirginTerrainAltitudeMSL((*ti).m_LL2) > 1)
					//continue;

				// Calculate triangle vertex coordinates.
				AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(radius);
				AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(radius);
				AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(radius);

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
				terTex0.setX((ll0long)/(PIf*2) * texreps * cosf((*ti).m_LL0.Latitude()));
				terTex0.setY(((*ti).m_LL0.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex1;
				terTex1.setX((ll1long)/(PIf*2) * texreps * cosf((*ti).m_LL1.Latitude()));
				terTex1.setY(((*ti).m_LL1.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex2;
				terTex2.setX((ll2long)/(PIf*2) * texreps * cosf((*ti).m_LL2.Latitude()));
				terTex2.setY(((*ti).m_LL2.Latitude() + PIf/2)/PIf * texreps);

				// Construct a globe triangle.
				AuGlobeTriTex tri;
				tri.m_Vert0 = AuGlobeVertTex(terCoord0, terTex0);
				tri.m_Vert1 = AuGlobeVertTex(terCoord1, terTex1);
				tri.m_Vert2 = AuGlobeVertTex(terCoord2, terTex2);

				// Append the globe triangle to the list.
				tris.append(tri);
			}

			// Store count of triangles in the far globe vertex array object.
			unsigned int vertices = tris.count()*3;

			// Copy surface triangles into gl array buffers.
			GLfloat* globePos = new GLfloat[vertices*3];		// vertices for triangles that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*2];		// texture coords for each of the vertices
			GLfloat* globeTan = new GLfloat[vertices*3];		// tangent vectors

			unsigned int globePosIdx = 0;
			unsigned int globeTexIdx = 0;
			unsigned int globeTanIdx = 0;
			for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
			{
				// Calculate tangent vectors.
				AuPoint n0 = (*vi).m_Vert0.m_Loc;
				n0.Unit();
				AuPoint t0;
				n0.ArbitraryAxis(t0);
				AuPoint n1 = (*vi).m_Vert1.m_Loc;
				n1.Unit();
				AuPoint t1;
				n1.ArbitraryAxis(t1);
				AuPoint n2 = (*vi).m_Vert2.m_Loc;
				n2.Unit();
				AuPoint t2;
				n2.ArbitraryAxis(t2);

				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t0.x;
				globeTan[globeTanIdx++] = (float)t0.y;
				globeTan[globeTanIdx++] = (float)t0.z;

				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t1.x;
				globeTan[globeTanIdx++] = (float)t1.y;
				globeTan[globeTanIdx++] = (float)t1.z;

				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t2.x;
				globeTan[globeTanIdx++] = (float)t2.y;
				globeTan[globeTanIdx++] = (float)t2.z;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_SeaGlobe.CreateGL(re, vertices, 3);

			// Select vertex array to construct.
			m_SeaGlobe.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(0));
			re.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for colors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(1));
			re.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			re.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for tangent vectors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_SeaGlobe.Buffer(2));
			re.glBufferData(GL_ARRAY_BUFFER, globeTanIdx*sizeof(GLfloat), globeTan, GL_STATIC_DRAW);
			re.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			re.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_SeaGlobe.DeselectGL();

			delete [] globePos;
			delete [] globeTex;
			delete [] globeTan;
		}

		// Sometimes we need to obscure the sea bottom.
		// This is done by displaying an opaque darker smaller copy of the sea surface model.
		if(bothLayers && !m_UnderseaGlobe.VAO())
		{
			// Create the undersea subsurface globe vertex array object.
			seaTris.CreateGlobeTriangles(SeaLevelEquatorDivisions());
			int texreps = SeaLevelEquatorDivisions();				// Less than the upper layer makes this layer move more slowly.

			// NOTE: This radius drop must always be at least the distance between the viewer's eye position and body cg.
			radius -= 5;			// depth of opaque lower liquid layer

			// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
			QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
			for(AuGlobeLLTriList::const_iterator ti = seaTris.cbegin(); ti != seaTris.cend(); ti++)
			{
				//if(VirginTerrainAltitudeMSL((*ti).m_LL0) > 1 &&
						//VirginTerrainAltitudeMSL((*ti).m_LL1) > 1 &&
						//VirginTerrainAltitudeMSL((*ti).m_LL2) > 1)
					//continue;

				// Calculate triangle vertex coordinates.
				AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(radius);
				AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(radius);
				AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(radius);

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
				terTex0.setX((ll0long)/(PIf*2) * texreps * cosf((*ti).m_LL0.Latitude()));
				terTex0.setY(((*ti).m_LL0.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex1;
				terTex1.setX((ll1long)/(PIf*2) * texreps * cosf((*ti).m_LL1.Latitude()));
				terTex1.setY(((*ti).m_LL1.Latitude() + PIf/2)/PIf * texreps);
				QPointF terTex2;
				terTex2.setX((ll2long)/(PIf*2) * texreps * cosf((*ti).m_LL2.Latitude()));
				terTex2.setY(((*ti).m_LL2.Latitude() + PIf/2)/PIf * texreps);

				// Construct a globe triangle.
				AuGlobeTriTex tri;
				tri.m_Vert0 = AuGlobeVertTex(terCoord0, terTex0);
				tri.m_Vert1 = AuGlobeVertTex(terCoord1, terTex1);
				tri.m_Vert2 = AuGlobeVertTex(terCoord2, terTex2);

				// Append the globe triangle to the list.
				tris.append(tri);
			}

			// Store count of triangles in the far globe vertex array object.
			unsigned int vertices = tris.count()*3;

			// Copy surface triangles into gl array buffers.
			GLfloat* globePos = new GLfloat[vertices*3];		// vertices for triangles that make up the model.
			GLfloat* globeTex = new GLfloat[vertices*2];		// texture coords for each of the vertices
			GLfloat* globeTan = new GLfloat[vertices*3];		// tangent vectors

			unsigned int globePosIdx = 0;
			unsigned int globeTexIdx = 0;
			unsigned int globeTanIdx = 0;
			for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
			{
				// Calculate tangent vectors.
				AuPoint n0 = (*vi).m_Vert0.m_Loc;
				n0.Unit();
				AuPoint t0;
				n0.ArbitraryAxis(t0);
				AuPoint n1 = (*vi).m_Vert1.m_Loc;
				n1.Unit();
				AuPoint t1;
				n1.ArbitraryAxis(t1);
				AuPoint n2 = (*vi).m_Vert2.m_Loc;
				n2.Unit();
				AuPoint t2;
				n2.ArbitraryAxis(t2);

				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t0.x;
				globeTan[globeTanIdx++] = (float)t0.y;
				globeTan[globeTanIdx++] = (float)t0.z;

				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t1.x;
				globeTan[globeTanIdx++] = (float)t1.y;
				globeTan[globeTanIdx++] = (float)t1.z;

				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
				globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
				globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();

				globeTan[globeTanIdx++] = (float)t2.x;
				globeTan[globeTanIdx++] = (float)t2.y;
				globeTan[globeTanIdx++] = (float)t2.z;
			}

			// Generate vertex array(s) for this object. We only need one.
			m_UnderseaGlobe.CreateGL(re, vertices, 3);

			// Select vertex array to construct.
			m_UnderseaGlobe.SelectGL();

			// bind buffer for vertices and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_UnderseaGlobe.Buffer(0));
			re.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
			re.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for colors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_UnderseaGlobe.Buffer(1));
			re.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
			re.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			// bind buffer for colors and copy data into buffer.
			re.glBindBuffer(GL_ARRAY_BUFFER, m_UnderseaGlobe.Buffer(2));
			re.glBufferData(GL_ARRAY_BUFFER, globeTanIdx*sizeof(GLfloat), globeTan, GL_STATIC_DRAW);
			re.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

			re.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Disable the vertex array object.
			m_UnderseaGlobe.DeselectGL();

			delete [] globePos;
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
			re.GLSL().WaterShader().Select(projection);
			re.GLSL().WaterShader().SetModelView(model);
			re.GLSL().WaterShader().SetTexture(s_texSea);
			re.GLSL().WaterShader().SetBumpMap(s_texSeaN);
			re.GLSL().WaterShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Top down view aperture.
			float arcAngle = 250*FeetPerMeter() / GlobeRadius();	// Angle of arc to the detailed terrain creation range.
			float clipCos = cosf(arcAngle);
			clipCos = 1;	// Aperture is pointless with such deep seas.
			AuPoint clipUp = eyeLoc;
			clipUp.Unit();
			re.GLSL().WaterShader().SetClip(clipCos, clipUp.x, clipUp.y, clipUp.z);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
				re.GLSL().WaterShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			// Prepare for rendering the sea surface.
			// Face culling is disabled in situations that enable the viewer to see the underside of the transparent surface layer,
			// such as when the camera is close to the water line.
			if(showUndersideOfSurface)
				re.glDisable(GL_CULL_FACE);

			// Prepare for rendering sea surface transparently.
			re.GLSL().WaterShader().SetOpacity(WaterOpacity(illumination));

			// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
			re.GLSL().WaterShader().SetAmbientLight(illumination*.8f);

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

			re.GLSL().WaterShader().Deselect();
		}
		else
		{
			// No aperture.
			if(bothLayers)
			{
				// Display the opaque lower layer without costly bump mapping or blending.
				// Render the sea globe vertex array object.
				re.GLSL().WaterNoBumpNoHoleShader().Select(projection);
				re.GLSL().WaterNoBumpNoHoleShader().SetModelView(model);
				re.GLSL().WaterNoBumpNoHoleShader().SetTexture(s_texSea);
				re.GLSL().WaterNoBumpNoHoleShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
				re.GLSL().WaterNoBumpNoHoleShader().SetOpacity(1);
				re.GLSL().WaterNoBumpNoHoleShader().SetFog(fogDensity, fogR, fogG, fogB);

				// Set eye position in object space for specular lighting calculations.
				AuPoint eyeLoc = model.UnProject(re.EyeLocation());
				re.GLSL().WaterNoBumpNoHoleShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

				// Ambient light
				re.GLSL().WaterNoBumpNoHoleShader().SetAmbientLight(ambientLight);

				// Suns
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
				{
					// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
					const AuGLSun& sun = re.Sun(sunIndex);
					AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

					// Configure shader for object space lighting.
					re.GLSL().WaterNoBumpNoHoleShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
							sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
					re.GLSL().WaterNoBumpNoHoleShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
							light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
							light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
				}

				// Display the opaque lower layer.
				m_UnderseaGlobe.SelectGL();
				m_UnderseaGlobe.DrawGL();
				m_UnderseaGlobe.DeselectGL();

				re.GLSL().WaterNoBumpNoHoleShader().Deselect();
			}

			// Render the sea globe vertex array object.
			re.GLSL().WaterNoHoleShader().Select(projection);
			re.GLSL().WaterNoHoleShader().SetModelView(model);
			re.GLSL().WaterNoHoleShader().SetTexture(s_texSea);
			re.GLSL().WaterNoHoleShader().SetBumpMap(s_texSeaN);
			re.GLSL().WaterNoHoleShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterNoHoleShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterNoHoleShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterNoHoleShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
				re.GLSL().WaterNoHoleShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			// Prepare for rendering the sea surface.
			// Face culling is disabled in situations that enable the viewer to see the underside of the transparent surface layer,
			// such as when the camera is close to the water line.
			if(showUndersideOfSurface)
				re.glDisable(GL_CULL_FACE);

			if(bothLayers)
			{
				// Prepare for rendering sea surface transparently.
				re.GLSL().WaterNoHoleShader().SetOpacity(WaterOpacity(illumination));

				// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
				re.GLSL().WaterNoHoleShader().SetAmbientLight(illumination*.8f);

				// Alpha blending is enabled for transparent effect.
				re.glEnable(GL_BLEND);
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				m_SeaGlobe.SelectGL();
				m_SeaGlobe.DrawGL();
				m_SeaGlobe.DeselectGL();

				re.glDisable(GL_BLEND);
			}
			else
			{
				// Prepare for rendering sea surface opaque.
				re.GLSL().WaterNoHoleShader().SetOpacity(1);

				// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
				re.GLSL().WaterNoHoleShader().SetAmbientLight(illumination*.8f);

				m_SeaGlobe.SelectGL();
				m_SeaGlobe.DrawGL();
				m_SeaGlobe.DeselectGL();
			}

			// Restore face culling if we changed it.
			if(showUndersideOfSurface)
				re.glEnable(GL_CULL_FACE);

			re.GLSL().WaterNoHoleShader().Deselect();
		}
	}
	else
	{
		// No bump mapping.
		// Render the sea globe vertex array object.
		if(!bothLayers && topDownView)
		{
			// Render with aperture.
			re.GLSL().WaterNoBumpShader().Select(projection);
			re.GLSL().WaterNoBumpShader().SetModelView(model);
			re.GLSL().WaterNoBumpShader().SetTexture(s_texSea);
			re.GLSL().WaterNoBumpShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterNoBumpShader().SetOpacity(1);
			re.GLSL().WaterNoBumpShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterNoBumpShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Top down view aperture.
			float arcAngle = 250*FeetPerMeter() / GlobeRadius();	// Angle of arc to the detailed terrain creation range.
			float clipCos = cosf(arcAngle);
			clipCos = 1;	// Aperture is pointless with such deep seas.
			AuPoint clipUp = eyeLoc;
			clipUp.Unit();
			re.GLSL().WaterNoBumpShader().SetClip(clipCos, clipUp.x, clipUp.y, clipUp.z);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterNoBumpShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
				re.GLSL().WaterNoBumpShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			if(bothLayers)
			{
				// Ambient light
				re.GLSL().WaterNoBumpShader().SetAmbientLight(ambientLight);

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
			re.GLSL().WaterNoBumpShader().SetOpacity(WaterOpacity(illumination));

			// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
			re.GLSL().WaterNoBumpShader().SetAmbientLight(illumination*.8f);

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

			re.GLSL().WaterNoBumpShader().Deselect();
		}
		else
		{
			// No aperture.
			re.GLSL().WaterNoBumpNoHoleShader().Select(projection);
			re.GLSL().WaterNoBumpNoHoleShader().SetModelView(model);
			re.GLSL().WaterNoBumpNoHoleShader().SetTexture(s_texSea);
			re.GLSL().WaterNoBumpNoHoleShader().SetTexOffset(m_WaterOffX/5, m_WaterOffY/5);
			re.GLSL().WaterNoBumpNoHoleShader().SetOpacity(1);
			re.GLSL().WaterNoBumpNoHoleShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Set eye position in object space for specular lighting calculations.
			AuPoint eyeLoc = model.UnProject(re.EyeLocation());
			re.GLSL().WaterNoBumpNoHoleShader().SetEyeLocation(eyeLoc.x, eyeLoc.y, eyeLoc.z);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = re.Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

				// Configure shader for object space lighting.
				re.GLSL().WaterNoBumpNoHoleShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
						sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
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
				re.GLSL().WaterNoBumpNoHoleShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}

			if(bothLayers)
			{
				// Ambient light
				re.GLSL().WaterNoBumpNoHoleShader().SetAmbientLight(ambientLight);

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

			if(bothLayers)
			{
				// Prepare for rendering sea surface transparently.
				re.GLSL().WaterNoBumpNoHoleShader().SetOpacity(WaterOpacity(illumination));

				// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
				re.GLSL().WaterNoBumpNoHoleShader().SetAmbientLight(illumination*.8f);

				// Alpha blending is enabled for transparent effect.
				re.glEnable(GL_BLEND);
				re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				m_SeaGlobe.SelectGL();
				m_SeaGlobe.DrawGL();
				m_SeaGlobe.DeselectGL();

				re.glDisable(GL_BLEND);
			}
			else
			{
				// Prepare for rendering sea surface opaque.
				re.GLSL().WaterNoBumpNoHoleShader().SetOpacity(1);

				// Ambient light is modulated to 80% so the noon-day sun doesn't wash out all detail.
				re.GLSL().WaterNoBumpNoHoleShader().SetAmbientLight(illumination*.8f);

				m_SeaGlobe.SelectGL();
				m_SeaGlobe.DrawGL();
				m_SeaGlobe.DeselectGL();
			}

			// Restore face culling if we changed it.
			if(showUndersideOfSurface)
				re.glEnable(GL_CULL_FACE);

			re.GLSL().WaterNoBumpNoHoleShader().Deselect();
		}
	}
}

QColor DaWorldGasGiant::SeaColor() const
{
	return AtmosphereColor();
}

float DaWorldGasGiant::SeaFogDensity() const
{
	if(InDesignerPreview())
		return DaWorldSphere::SeaFogDensity();
	return .05f;
}

void DaWorldGasGiant::CreateSurfaceMap(AuGLEffects& re)
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
		// Create surface texture from created planet texture, if it's ready.
		if(!m_glTexFarSurface)
			return;
		fboImage = QImage(iViewSizeX, iViewSizeX/2, QImage::Format_RGB32);
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

void DaWorldGasGiant::LoadCachedSurfaceImage(const AuId& id, QImage& surface, int iViewSizeX, const AuTime& floraChange) const
{
	if(g_Au.IsSceneSolo())
		return;
	// Try loading the far world texture image from a cache file.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("WTF%1X%2").arg(id.GetID()).arg(iViewSizeX));
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
				(floraChange.IsNull() || storeTime == floraChange.UTC()))
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

		//qDebug() << "DaWorldGlobe::LoadCachedSurfaceImage" << id << Name() << "image loaded from cache"
			//<< surface.width() << surface.height() << surface.depth() << surface.bitPlaneCount();
	}
}

void DaWorldGasGiant::GrabSurfaceTextureImage(AuGLEffects& re, QImage& imtex, GLuint glTexture) const
{
	int iViewSizeX = imtex.width();
	int iViewSizeY = imtex.height();
	//qDebug() << "DaWorldGlobe::GrabSurfaceTextureImage" << Id() << Name() << "size" << iViewSizeX << iViewSizeY;

	GLfloat* buffer = (GLfloat*)malloc(iViewSizeX*iViewSizeY*4*sizeof(GLfloat));

	re.glBindTexture(GL_TEXTURE_2D, glTexture);
	re.glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, buffer);
	for(int w=0; w<iViewSizeX; w++)
	{
		for(int h=0; h<iViewSizeY; h++)
		{
			int i = ((h*iViewSizeX) + w)*3;
			//QRgb value = qRgba((int)(buffer[i]*255), (int)(buffer[i+1]*255), (int)(buffer[i+2]*255), (int)(buffer[i+3]*255));
			QRgb value = qRgba((int)(buffer[i]*255), (int)(buffer[i+1]*255), (int)(buffer[i+2]*255), 255);
			imtex.setPixel(w, iViewSizeY-h-1, value);	// Mirror vertically.
		}
	}
	re.glBindTexture(GL_TEXTURE_2D, 0);	// Unbind

	free(buffer);
}

// iViewSizeX is the pixel width of the resulting texture. The height of the texture will be iViewSizeX/2.
GLuint DaWorldGasGiant::CreatePlanetTexture(AuAtomRenderInfo& ari, int iViewSizeX, bool drawCities)
{
	if(!drawCities)
	{
		// Try loading the surface texture image from a cache file.
		QImage imtex;
		LoadCachedSurfaceImage(imtex, iViewSizeX);
		if(!imtex.isNull())
		{
			// Create a texture from the image.
			//qDebug() << "DaWorldGlobe::CreatePlanetTexture" << Id() << Name() << "image loaded from cache"
				//<< imtex.width() << imtex.height() << imtex.depth() << imtex.bitPlaneCount();
			GLuint glTexSurface;
			ari.re->glGenTextures(1, &glTexSurface);
			ari.re->glBindTexture(GL_TEXTURE_2D, glTexSurface);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			ari.re->CreateTextureFromBitmap(imtex, true);			// Creates the texture without adding it to re's texture cache.
			ari.re->glBindTexture(GL_TEXTURE_2D, 0);
			return glTexSurface;
		}
	}

	if(!m_glTexGrass && CanHaveFlora())
		return 0;

	// Render globe with vertices flattened into the range -1,-1,0 to 1,1,0. This creates the appropriate texture to wrap around
	// a simple globe.
	// Create the vertex array object of the peeled-open flat globe.
	float globeMaxAltMSL, globeMinAltMSL;
	AuGLVAO flatGlobeVAO = ari.mm.FlatGlobeModel(*ari.re, this, globeMinAltMSL, globeMaxAltMSL);
	if(!flatGlobeVAO.VAO())
		return 0;						// Model maker is not ready with our surface model yet.
	//float altRange = globeMaxAltMSL - globeMinAltMSL;			// Altitude range of all land, for altitude factor calculation.
	//float flatGlobeAltNoiseFactor = (AltitudeIncrement()/3)/altRange;			// Altitude deviation due to noise.

	// Create the lookup tables used for the planet texturing based on altitude of terrain.
	GLuint terElevTex[5];
	GLuint texFlatLookup = CreateAtLookupTable(*ari.re, globeMaxAltMSL, globeMinAltMSL, terElevTex);

	// Spherical world texture is twice as wide as it is high.
	int iViewSizeY = iViewSizeX/2;
	//qDebug() << "DaWorldGlobe::CreatePlanetTexture" << Id() << Name() << "size" << iViewSizeX << iViewSizeY;

	// Get current fbo binding. If it is not restored properly, things in the scene blink.
	GLint oldfbo;
	ari.re->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);

	// Setup FBO so that we render the terrain at full screen resolution
	GLuint fbo;					// FBO used to create planet texture
	ari.re->glGenFramebuffers(1, &fbo);
	ari.re->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);

	// Now setup the environment map texture
	GLuint glTexSurface;
	ari.re->glGenTextures(1, &glTexSurface);
	ari.re->glActiveTexture(GL_TEXTURE0);
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
		qDebug("DaWorldGlobe::CreatePlanetTexture glCheckFramebufferStatusEXT returned %d", status);
		ari.re->glBindTexture(GL_TEXTURE_2D, 0);
		ari.re->glBindFramebuffer(GL_FRAMEBUFFER_EXT, oldfbo);	// Unbind the FBO for now
		return 0;
	}

	ari.re->glViewport(0, 0, iViewSizeX, iViewSizeY);
	ari.re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	// Render the vertex array object.
	AuMatrix4x4 projection;
	AuMatrix4x4 model;
	if(CanHaveFlora() || true)
	{
		ari.re->GLSL().GlobeFloraShader().Select();
		ari.re->GLSL().GlobeFloraShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().GlobeFloraShader().SetGrassTexture(m_glTexGrass);
		ari.re->GLSL().GlobeFloraShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
		ari.re->GLSL().GlobeFloraShader().SetTerrainLookup(texFlatLookup);
		ari.re->GLSL().GlobeFloraShader().SetTerrainPalette(terElevTex);

		flatGlobeVAO.SelectGL();
		flatGlobeVAO.DrawGL();
		flatGlobeVAO.DeselectGL();

		ari.re->GLSL().GlobeFloraShader().Deselect();
	}
	else
	{
		ari.re->GLSL().GlobeShader().Select();
		ari.re->GLSL().GlobeShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().GlobeShader().SetSnowTexture(s_TexTerPalette[WORLDTEX_SnowModerate]);
		ari.re->GLSL().GlobeShader().SetTerrainLookup(texFlatLookup);
		ari.re->GLSL().GlobeShader().SetTerrainPalette(terElevTex);

		flatGlobeVAO.SelectGL();
		flatGlobeVAO.DrawGL();
		flatGlobeVAO.DeselectGL();

		ari.re->GLSL().GlobeShader().Deselect();
	}

	// Free the lookup texture.
	QString farLUT = QString("%1FLUT").arg(Id().StringId());
	ari.re->DeleteTexture(Id().GetID(), farLUT);

	// Free the vertex array object.
	//mm.DeleteFlatGlobeModel(Id());

	if(drawCities)
	{
		// Render geo roads, if any.
		float radiansPerFootX = 1.f / SeaLevelRadius();
		float radiansPerFootY = radiansPerFootX;
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
		qDebug("DaWorldGlobe::CreatePlanetTexture glCheckFramebufferStatusEXT returned %d", status);

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

				//qDebug() << "DaWorldGasGiant::CreatePlanetTexture" << Id() << Name() << "cached world texture"
					//<< cacheFileName;
			}
			else
				qWarning() << QString("DaWorldGasGiant::CreatePlanetTexture FAILED TO CREATE FILE %1").arg(cacheFileName);
		}
		else
			qWarning() << QString("DaWorldGasGiant::CreatePlanetTexture FAILED TO CREATE DIR FOR FILE %1").arg(cacheFileName);
	}
	return glTexSurface;
}

int DaWorldGasGiant::PopulationMax() const
{
	return CoreRadius1000km() * 100;
}

void DaWorldGasGiant::CreateGrassTexture(AuGLEffects& re, AuGLModelMaker& mm)
{
	if(!m_pFlora)
		return;

	quint32 genSeed = GenSeed();
	//qDebug() << "DaWorldGasGiant::CreateGrassTexture" << Id() << Name();

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
	//qDebug() << "DaWorldGasGiant::CreateGrassTexture" << Id() << Name() << "grasses" << grasses.count() << "tileSize" << tileSize;

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
	//qDebug() << "DaWorldGasGiant::CreateGrassTexture" << Id() << Name() << "finished" << texName;
}

void DaWorldGasGiant::CreateFloraTextures(AuGLEffects& re, int resourceZone)
{
	Q_ASSERT(m_pFlora);											// Caller insured this to be true.
	Q_ASSERT(!m_pFlora[resourceZone].Texture());				// Caller already checked this for us.
	//qDebug("DaWorldGasGiant::CreateFloraTextures %d %s creating flora textures for zone %d", Id().GetID(),
			//(const char*)Name().toAscii(), resourceZone);
	m_pFlora[resourceZone].CreateDeviceObjects(re, Id(), GenSeed(), resourceZone, m_tmFloraChange);
	//qDebug("DaWorldGasGiant::CreateFloraTextures %d %s creating flora textures for zone %d done", Id().GetID(),
			//(const char*)Name().toAscii(), resourceZone);
}

void DaWorldGasGiant::RenderScWorld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const DaPerson* sceneAtom)
{
	if(ClaimedByEmpire())
		CreateClaimVAOs(*ari.re);	// Create now for flag texture id.

	// Get current ambient light and fog states.
	float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
	bool sunsBlocked;
	SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);

	// Globe over the horizon effect is desirable when at a globe.
	// Globe position in eye space is needed to calculate sun over-the-horizon effect.
	AuPoint globeP = model.Translation();		// Globe's position in eye space.
	AuGLSLScBase* shader;
	if(fogDensity > 0)
	{
		shader = &ari.re->GLSL().ScGlobeFogShader();
		shader->Select(projection, *ari.re);
		ari.re->GLSL().ScGlobeFogShader().SetFog(fogDensity, fogR, fogG, fogB);
		ari.re->GLSL().ScGlobeFogShader().SetGlobeLocation(globeP);
	}
	else
	{
		shader = &ari.re->GLSL().ScGlobeShader();
		shader->Select(projection, *ari.re);
		ari.re->GLSL().ScGlobeShader().SetGlobeLocation(globeP);
	}

	// Block suns.
	if(sunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			shader->SetSunEnabled(sunIndex, false);

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

void DaWorldGasGiant::RenderSc_Shader(AuGLSLScBase* shader, const AuMatrix4x4& projection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, const DaPerson* sceneAtom)
{
	DaWorldSphere::RenderSc_Shader(shader, projection, model, ari, sceneAtom);
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

int DaWorldGasGiant::AirTravellerInterval(qint8 morale) const
{
	// Population does not increase when overpopulated.
	if(WorldPopulation() > WorldPopulationMax())
		return -1;							// Someone wants to leave immediately.
	// Nominal rate is three air travellers per day, at morale 1, one per city report cycle.
	int arrivalsPerDay;						// Morale affects willingness to travel to/from here.
	if(morale == 0)
		arrivalsPerDay = 1;
	else
		arrivalsPerDay = 3 * morale;
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

int DaWorldGasGiant::IndigenousPopulationInterval(qint8 morale) const
{
	Q_UNUSED(morale);
	return 0;
}

void DaWorldGasGiant::GeologicalFeatureChanged(const AuGeologicalFeature* feat, AuGLModelMaker& mm)
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
		if(roadLL.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Globe))
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
						LLOfLoc(testLL0, testLoc0);
						if(testLL0.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Globe))
						{
							roadInTri = true;
							break;
						}

						// Test negative side of circle.
						AuPoint testLoc1 = roadLoc - testVec;
						AuGlobeLL testLL1;
						LLOfLoc(testLL1, testLoc1);
						if(testLL1.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Globe))
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
						LLOfLoc(testLL0, testLoc0);
						if(testLL0.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Globe))
						{
							roadInTri = true;
							break;
						}

						// Test negative side of circle.
						AuPoint testLoc1 = roadLoc - testVec;
						AuGlobeLL testLL1;
						LLOfLoc(testLL1, testLoc1);
						if(testLL1.InTriangleClockwise(tri.m_LL0, tri.m_LL1, tri.m_LL2, LL_Globe))
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
			// The road intersects the globe surface triangle.
			// Invalidate the high tessellation model of the surface triangle.
			// This does not delete it so it can still be used until the nu one is requested and becomes ready.
			m_AtGlobe.InvalidateGlobeTriVAO(Id(), (*tvi).first, mm);
		}

		tvi++;
	}

	// Trigger regen of rocks and plants.
	m_FloraViewer = AuPoint(0,0);
	m_RockViewer = AuPoint(0,0);
	InvalidateTerrainInfoSprites();
}

quint64 DaWorldGasGiant::MemoryUsage() const
{
	quint64 mem = DaWorldSphere::MemoryUsage();
	mem += sizeof(DaWorldGasGiant) - sizeof(DaWorldSphere);
	mem += sizeof(DePlanetFauna) * ResourceZones();
	mem += sizeof(DePlanetFlora) * ResourceZones();
	mem += sizeof(AuZoneResource) * ResourceZones();
	return mem;
}

bool DaWorldGasGiant::UpdateEnclosureCache() const
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

DaAtom::ATOM_TYPE DaWorldGasGiant::AtomType() const
{
	return ATOM_WorldGasGiant;
}

void DaWorldGasGiant::operator>>(QDataStream& out) const
{
	DaWorldSphere::operator>>(out);
	quint8 verAtm = 0x10 | m_AtmType;
	out << m_byAtmDensity;
	out << verAtm;
	out << m_byCoreRadius;
	out << m_Radius1000km;
	out << m_WindDir;
	out << m_WindForce;

	m_GasGiantLock.lockForRead();
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

	// ver 0x10
	out << *m_FogOfWar;
	m_GasGiantLock.unlock();
}

void DaWorldGasGiant::operator<<(QDataStream& in)
{
	DaWorldSphere::operator<<(in);
	in >> m_byAtmDensity;
	quint8 verAtm;
	in >> verAtm;
	m_AtmType = (ATMTYPE)(verAtm & 0x0f);
	in >> m_byCoreRadius;
	in >> m_Radius1000km;
	in >> m_WindDir;
	in >> m_WindForce;

	m_GasGiantLock.lockForWrite();
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

	if(verAtm & 0x10)
		in >> *m_FogOfWar;

	m_ThinkRotation = m_RotationAngle;
	m_ThinkTransform = Transform();
	m_GasGiantLock.unlock();
}

bool DaWorldGasGiant::HitByAsteroid(quint8 asteroidSize, const AuPoint& asteroidLoc)
{
	Q_UNUSED(asteroidSize);
	Q_UNUSED(asteroidLoc);
	return false;			// Let the asteroid burn, like Shoemaker Levy 9.
}

bool DaWorldGasGiant::HasWorkerSpecies() const
{
	m_GasGiantLock.lockForRead();
	bool v = m_pSentient;
	m_GasGiantLock.unlock();
	return v;
}

void DaWorldGasGiant::SetWorkerSpecies(const AuSpecies& species)
{
	m_GasGiantLock.lockForWrite();
	if(!m_pSentient)
		m_pSentient = new AuSpecies(species);
	else
		*m_pSentient = species;
	if(m_pSentient->IsNull())
	{
		delete m_pSentient;
		m_pSentient = 0;
	}
	m_GasGiantLock.unlock();
}

AuSpecies DaWorldGasGiant::WorkerSpecies() const
{
	AuSpecies s;
	m_GasGiantLock.lockForRead();
	if(m_pSentient)
		s = *m_pSentient;
	m_GasGiantLock.unlock();
	return s;
}

COMMODITYID DaWorldGasGiant::GuaranteedRockAt(const AuGlobeLLF& rockLL, int resourceZone) const
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

AuGlobeLLF DaWorldGasGiant::GuaranteedRockLocationResLocked(const AuDice& Cdice, int eqDivs, int resourceZone) const
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

void DaWorldGasGiant::SetFogOfWar(const AuFogOfWar& fow)
{
	m_GasGiantLock.lockForWrite();
	delete m_FogOfWar;
	m_FogOfWar = new AuFogOfWar(fow);
	m_GasGiantLock.unlock();
}

void DaWorldGasGiant::Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double radius)
{
	m_GasGiantLock.lockForRead();
	bool needsInit = m_FogOfWar->NeedsInit(avatarId, empireId);
	m_GasGiantLock.unlock();
	if(needsInit)
	{
		// Init fog of war locations.
		double radius = GlobeRadius();
		double circ = PI * radius * 2;	// circumference
		quint16 equatorSteps = circ / (AuFogOfWarData::Radius() * 1.5);
		AuScFaceList faces;
		faces.MakeSphereUniform(AuPoint(0,0), radius, equatorSteps, 1, AuMatrix4x4());
		AuPointArray verts;
		faces.VertexLocations(verts);	// All vertices including overlaps.
		AuPointArray uniqueVerts;
		for(AuPointArray::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
			uniqueVerts.AddIfUnique(*vi);
		m_GasGiantLock.lockForWrite();
		m_FogOfWar->Init(avatarId, empireId, uniqueVerts);
	}
	else
		m_GasGiantLock.lockForWrite();
	bool chg = m_FogOfWar->Explore(avatarId, empireId, seaLevelLoc, radius);
	m_GasGiantLock.unlock();
	if(chg)
		SetModified();
}

void DaWorldGasGiant::ExploreAll(const AuId& avatarId, quint32 empireId)
{
	m_GasGiantLock.lockForWrite();
	bool chg = m_FogOfWar->ExploreAll(avatarId, empireId);
	m_GasGiantLock.unlock();
	if(chg)
		SetModified();
}

void DaWorldGasGiant::EmpireDeleted(quint32 empireId)
{
	m_GasGiantLock.lockForWrite();
	bool chg = m_FogOfWar->EmpireDeleted(empireId);
	m_GasGiantLock.unlock();
	if(chg)
		SetModified();
}

bool DaWorldGasGiant::FogOfWar(const AuId& avatarId, quint32 empireId, AuFogOfWarData& fow) const
{
	m_GasGiantLock.lockForRead();
	m_FogOfWar->GetData(avatarId, empireId, fow);
	m_GasGiantLock.unlock();
	return true;
}

bool DaWorldGasGiant::FogOfWarTest(const AuId& avatarId, quint32 empireId, const AuGlobeLL& ll) const
{
	AuPoint loc = LocOfLL(ll);
	m_GasGiantLock.lockForRead();
	bool isExplored = m_FogOfWar->IsExplored(avatarId, empireId, loc);
	m_GasGiantLock.unlock();
	return isExplored;
}

const QImage* DaWorldGasGiant::SurfaceImage() const
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

double DaWorldGasGiant::SurfaceImageHeight() const
{
	return GlobeRadius() * PI;	// 1/2 circumference of core
}

GLuint DaWorldGasGiant::SurfaceTexture() const
{
	if(m_glTexNearSurface)
		return m_glTexNearSurface;
	else
		return m_glTexFarSurface;
}
