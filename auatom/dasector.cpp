//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasector.cpp,v 1.284 2022/05/17 16:33:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dasector.h"
//
#include "auatomrenderinfo.h"
#include "audice.h"
#include "augleffects.h"
#include "auglobal.h"
#include "auNebula.h"
#include "aunoise.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <typeinfo>			// for bad_cast

GLuint s_StarTexture = 0;

DaSector::DaSector()
{
	m_Nebula  = NULL;
}

DaSector::~DaSector()
{
	if(m_Nebula)
	{
		delete m_Nebula;
		m_Nebula = NULL;
	}
}

DaSector::DaSector(const DaSector &rhs) : DaAtom(rhs), m_aStars(rhs.m_aStars),
	m_ExploredWormholes(rhs.m_ExploredWormholes), m_KnownWormholes(rhs.m_KnownWormholes)
{
	m_SectorLock.lockForRead();
	m_sName = rhs.m_sName;
	m_SectorLock.unlock();

	m_Nebula = 0;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaSector::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
	if(HasName() && IsNameVisible(sceneAtom.EmpireId()))
	{
		m_SectorLock.lockForRead();
		toClient << m_sName;
		m_SectorLock.unlock();
	}
	else
		toClient << "";
}

// Receive updates to this atom from a server.
void DaSector::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);
	fromServer >> m_sName;
}

// Refresh data from valid counterpart.
bool DaSector::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaSector* pA = (const DaSector*)pAtom;

	pA->m_SectorLock.lockForRead();
	m_SectorLock.lockForWrite();
	m_sName = pA->m_sName;
	m_SectorLock.unlock();
	pA->m_SectorLock.unlock();

	return false;
}

DaAtom *DaSector::Clone() const
{
	return new DaSector(*this);
}

QString DaSector::Name() const
{
	QString v;
	m_SectorLock.lockForRead();
	if(m_sName.isEmpty())
		v = tr("Sector");
	else
		v = tr("%1 Sector").arg(m_sName);
	m_SectorLock.unlock();
	return v;
}

QString DaSector::NameKnownTo(quint32 empireId) const
{
	if(!IsNameVisible(empireId))
		return tr("Sector");
	return Name();
}

QString DaSector::SectorNameCoord() const
{
	m_SectorLock.lockForRead();
	QString v(m_sName);
	m_SectorLock.unlock();
	if(v.isEmpty())
		return tr("Sector %1").arg(Description());
	return tr("%1 Sector").arg(v);
}

bool DaSector::IsNameVisible(quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(*it);
		if(!system)
			continue;
		if(!system->HasName())
			continue;
		if(!system->IsExploredBy(empireId))
			continue;
		return true;
	}
	return false;
}

int DaSector::SolarSystemsInSector() const
{
	double rStarDensity = StarDensity();		// Density value range is 0.0 to 1.0.
	int stars = (int)(rStarDensity * 50.0);		// Maximum is 50 stars in a sector.
	if(!stars && IsCore())
		return 1;
	return stars;
}

// Create a seed value for generating this sector that is based on my position in the galaxy. That way this sector will regen
// the same if it is removed and recreated due to decay or any other reason.
quint32 DaSector::GenSeed() const
{
	// Select a seed value for this sector that is based on the seed value of my parent galaxy.
	const DaGalaxy* galaxy = FindParentGalaxy();
	Q_ASSERT(galaxy);
	quint32 genSeed = galaxy->GenSeed();
	int randFunc = genSeed % 12;		// The random noise function is based on the galaxy's genSeed.

	// Use noise to calculate a gen seed for this sector based on its location in the galaxy.
	AuPoint gloc(Location());
	double grad = galaxy->Radius();		// In pc.
	gloc += AuPoint(grad, grad, grad);	// Move the center outside the galaxy, to avoid repeats near the core.
	gloc *= 10;

	// Again we incorporate the galaxy's genSeed, otherwise randFunc would only produce 12 different galaxies.
	double noiseXY = AuNoise::IntegerNoise(randFunc, round(gloc.x), round(gloc.y));	// Range is -1 to 1
	double noiseZG = AuNoise::IntegerNoise(randFunc, round(gloc.z), genSeed);		// Range is -1 to 1
	genSeed += (quint32)(noiseXY * 0xffffffff) + (quint32)(noiseZG * 0xffffffff);
	if(genSeed)
		return genSeed;

	qWarning() << "warning: DaSector::GenSeed" << Id() << Name() << "GenSeed ZERO! Returning 1 instead." << noiseXY << noiseZG
		<< galaxy->GenSeed();
	return 1;
}

bool DaSector::IsCore() const
{
	return Location() == DaGalaxy::CoreSectorLoc();
}

void DaSector::Init()
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Notify the universe that a nu atom was added.
	g_Au.AtomAdded(Id());

	// Seed a dice object for consistent regen of this sector's solar system positions.
	quint32 genSeed = GenSeed();		// This seed value will be incremented/used for each of my solar systems too.
	AuDice dice(genSeed);
	int randFunc = genSeed % 12;		// The random noise function is based on my initial genSeed.
	int stars = SolarSystemsInSector();
	if(!stars)
	{
		// Create a special place-holder solar system that has no stars, so people can transition through the area.
		// Use noise to select a genSeed value for the solar system.
		do
		{
			double noise = AuNoise::IntegerNoise(randFunc, genSeed);
			genSeed += (quint32)(noise * 0xffffffff);
		}
		while(!genSeed);

		// Create a nu solar system.
		DaSolarSystem* pSys = new DaSolarSystem;
		pSys->ConnectSignals();
		pSys->Create(HABITABLE_EmptySystem, genSeed);					// After Create, the system doesn't contain any atom.
		pSys->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(pSys);

		// Notify the universe that a nu atom was added.
		g_Au.AtomAdded(pSys->Id());
		pSys->SetNormal();	// New atom is ready for normal thread processing.
		if(g_Au.IsSceneSolo())
			pSys->TickTock();
		return;
	}

	bool coreBlackHole = IsCore();			// The core sector contains a black hole system.
	int homeSystems = HomeWorldSystems();	// Number of systems that are to be created as HABITABLE_HomeWorld systems.

	// Create solar systems in this sector.
	AuPointArray aParsecs;
	while(stars--)
	{
		HABITABILITY genHab;
		AuMatrix4x4 tSys;
		if(coreBlackHole)
		{
			// Black hole is always at the center of the sector.
			genHab = HABITABLE_System;

			// Remember the location of the parsec used.
			aParsecs.append(AuPoint());
		}
		else
		{
			// Some number of systems may be created as home world systems, so new players have a place to start. We used to pick
			// one and change it to a home world system when needed. That caused people's sensor scans to change over time and it
			// bothered them more than expected.
			if(homeSystems > 0)
			{
				genHab = HABITABLE_HomeWorld;
				homeSystems--;
			}
			else
				genHab = HABITABLE_System;

			// Randomly generate parsec coords until we don't land in one we already used.
			AuPoint fParsecLoc;
			do
			{
				double rX = dice.Roll(1,10) - 1.0;	// [0..9]
				double rY = dice.Roll(1,10) - 1.0;	// [0..9]
				double rZ = dice.Roll(1,10) - 1.0;	// [0..9]
				fParsecLoc = AuPoint(rX, rY, rZ) - AuPoint(4.5, 4.5, 4.5);
			}
			while(aParsecs.DoesContain(fParsecLoc));

			// Remember the location of the parsec used before purturbing the location of the star.
			aParsecs.append(fParsecLoc);

			// Purturb the star location so things don't look so neatly aligned.
			double rxp = dice.Roll(1,9,-5)/10.0;	// Range is [-.4 to .4]
			double ryp = dice.Roll(1,9,-5)/10.0;	// Range is [-.4 to .4]
			double rzp = dice.Roll(1,9,-5)/10.0;	// Range is [-.4 to .4]
			AuPoint starLoc = fParsecLoc + AuPoint(rxp, ryp, rzp);
			tSys.Translate(starLoc);

			// Init to random rotation.
			double rx = dice.Roll(1,360,-1) * DEG2RAD;
			double ry = dice.Roll(1,360,-1) * DEG2RAD;
			double rz = dice.Roll(1,360,-1) * DEG2RAD;
			tSys.Rotate(AuPoint(rx,ry,rz));
		}

		// Use noise to select a genSeed value for the solar system.
		do
		{
			double noise = AuNoise::IntegerNoise(randFunc, genSeed);
			genSeed += (quint32)(noise * 0xffffffff);
		}
		while(!genSeed);

		// Create a nu solar system.
		DaSolarSystem* pSys = new DaSolarSystem;
		pSys->ConnectSignals();
		pSys->Create(genHab, genSeed);						// After Create, the system doesn't contain any atoms yet.
		pSys->SetPosAndOrient(AuAtomPos(Id(), tSys));
		ether->AddAtom(pSys);

		// Notify the universe that a nu atom was added.
		g_Au.AtomAdded(pSys->Id());
		pSys->SetNormal();	// New atom is ready for normal thread processing.

		// Solar systems that contain a ringworld must be finalized immediately. Then those ringworlds can create their
		// intersystem portals, and be found by story lines.
		if(pSys->CouldHaveRingworld())
		{
			// The system may have a ringworld. Do the grunt work to be sure.
			AuEther tempEther("DaSector::Init");
			DaSolarSystem* tempSys = new DaSolarSystem;
			tempSys->SetId(pSys->Id());
			tempEther.AddAtom(tempSys);
			tempSys->PrepareForResourceScan(pSys->CatalogName(), pSys->Name(), Galaxy(), HABITABLE_System, genSeed, coreBlackHole,
					tSys, -1);
			if(tempSys->HasRingworld())
			{
				pSys->FinalizeContents();	// The system contains a ringworld. It must be finalized now. Ugh. Do it again.
				if(g_Au.IsSceneSolo())
				{
					pSys->SetStarFieldFlag(false);	// In solo all final systems are not star field systems.
					pSys->TickTockRecursive();
				}
			}
		}

		// Only one black hole is created in the core sector. Remaining systems are normal solar systems.
		coreBlackHole = false;
	}
}

// Report a longer description of this atom for examination purposes.
QString DaSector::Description() const
{
	// Convert from parsec coord to sector coord.
	AuPoint fLoc = Location()/10.0;
	return QString("(%1, %2, %3)").arg(fLoc.x).arg(fLoc.y).arg(fLoc.z);
}

// Return star density factor for this sector based on its location in the galaxy.
// The density value returned is in the range 0.0 to 1.0.
double DaSector::StarDensity() const
{
	const DaGalaxy* galaxy = FindParentGalaxy();
	Q_ASSERT(galaxy);
	return galaxy->StarDensity(Location());
}

void DaSector::ListSolarSystemLocations(DqAtomLocArray &aSystems, const AuPoint &fOffset, AuSegmentArray& exploredWormholes,
		AuSegmentArray& knownWormholes, quint32 empire) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsEmptySystem())
			continue;
		pss->ListLocationAndWormholes(aSystems, fOffset, exploredWormholes, knownWormholes, empire);
	}
}

void DaSector::ListSolarSystemNames(QStringList& names) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->HasName())
			names.append(pss->Name());
	}
}

DaSolarSystem* DaSector::FindGoodHome()
{
	// Find a home world solar system that has not been explored yet.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsFinal())
			continue;
		if(pss->GenHab() != HABITABLE_HomeWorld)
			continue;
		// Found an unexplored solar system that is a homeworld system.
		return pss;
	}
	return 0;
}

DaSolarSystem* DaSector::FindBadHome()
{
	// Find a home world solar system that has not been explored yet.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsFinal())
			continue;
		if(pss->GenHab() == HABITABLE_HomeWorld)
			continue;
		// Found an unexplored solar system that is not a homeworld system.
		return pss;
	}
	return 0;
}

DaSolarSystem* DaSector::FindSolarSystemAt(const AuPoint &fLoc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->Location().FuzzyEquals(fLoc))
			return(pss);
	}
	return 0;
}

DaSolarSystem* DaSector::FindClosestSolarSystem(const AuPoint &fLoc) const
{
	DaSolarSystem* pBest = 0;
	double rBest = -1.0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		double rDist = pss->Location().Distance2(fLoc);	// squared distance for speed
		if(rBest == -1.0 || rDist < rBest)
		{
			pBest = pss;
			rBest = rDist;
		}
	}
	return pBest;
}

DaSolarSystem* DaSector::FindClosestSolarSystemInRegion(const AuPoint &fLoc) const
{
	// Check all sectors in 3x3x3 region.
	DaSolarSystem* pBest = 0;
	double rBest = -1.0;
	const DaGalaxy* pg = (const DaGalaxy*)Parent();
	for(int ix=-10; ix<=10; ix+=10)
	{
		for(int iy=-10; iy<=10; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				AuPoint frel(ix, iy, iz);
				AuPoint fn = Location() + frel;
				const DaSector *pn = pg->FindSectorAt(fn);
				if(pn)
				{
					AuPoint fnloc = fLoc - frel;	// Offset search point to account for neighbor distance.
					DaSolarSystem *pSys = pn->FindClosestSolarSystem(fnloc);
					if(pSys)
					{
						fnloc = pSys->Location() + frel;	// Offset found location.
						double rDist = fnloc.Distance2(fLoc);	// squared distance for speed
						if(rBest == -1.0 || rDist < rBest)
						{
							pBest = pSys;
							rBest = rDist;
						}
					}
				}
			}
		}
	}
	return pBest;
}

DaSolarSystem* DaSector::FindClosestSolarSystem(const AuPoint &fLoc, const AuId& excludeSystem) const
{
	DaSolarSystem* pBest = 0;
	double rBest = -1.0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->Is(excludeSystem))
			continue;
		double rDist = pss->Location().Distance2(fLoc);	// squared distance for speed
		if(rBest == -1.0 || rDist < rBest)
		{
			pBest = pss;
			rBest = rDist;
		}
	}
	return pBest;
}

DaSolarSystem* DaSector::FindClosestSolarSystemInRegion(const AuPoint &fLoc, const AuId& excludeSystem) const
{
	// Check all sectors in 3x3x3 region.
	DaSolarSystem* pBest = 0;
	double rBest = -1.0;
	const DaGalaxy* pg = (const DaGalaxy*)Parent();
	for(int ix=-10; ix<=10; ix+=10)
	{
		for(int iy=-10; iy<=10; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				AuPoint frel(ix, iy, iz);
				AuPoint fn = Location() + frel;
				const DaSector *pn = pg->FindSectorAt(fn);
				if(pn)
				{
					AuPoint fnloc = fLoc - frel;	// Offset search point to account for neighbor distance.
					DaSolarSystem *pSys = pn->FindClosestSolarSystem(fnloc, excludeSystem);
					if(pSys)
					{
						fnloc = pSys->Location() + frel;	// Offset found location.
						double rDist = fnloc.Distance2(fLoc);	// squared distance for speed
						if(rBest == -1.0 || rDist < rBest)
						{
							pBest = pSys;
							rBest = rDist;
						}
					}
				}
			}
		}
	}
	return pBest;
}

// Renders the atom as the environment for a subatom who is rendering the scene.
// View transformation has already been set up.
void DaSector::RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, DaPerson* sceneAtom)
{
	Q_UNUSED(projection);
	if(!s_StarTexture)
		s_StarTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "StarSprite", ":data/Star.png", GL_LINEAR, GL_LINEAR,
				true, 2);

	// Get a list of the star locations relative to the current scene location (in parsecs).
	if(m_tmLastStarCache.ElapsedSec() > 15 || m_aStars.isEmpty())
	{
		m_aStars.clear();
		m_ExploredWormholes.clear();
		m_KnownWormholes.clear();
		quint32 empire = sceneAtom->EmpireId();
		// We expect all neighboring sectors to be present.
		ListStarAndWormholeLocations(m_aStars, m_ExploredWormholes, m_KnownWormholes, empire, true);
		m_tmLastStarCache.restart();

		//qDebug("DaSector::RenderEnvironment - cached %d stars and %d wormholes", m_aStars.count(), m_ExploredWormholes.count());

		// Remove segments from the known wormholes list that are overlapped by segments in the explored wormholes list.
		AuSegment* pw = m_ExploredWormholes.begin();
		for(int iw=m_ExploredWormholes.count(); iw; iw--, pw++)
		{
			int segIndex = m_KnownWormholes.FindSegment(*pw);
			if(segIndex >= 0)
				m_KnownWormholes.remove(segIndex);
		}
	}

	// Multiply by inverse of scene subatom transformation (Translation, rotation, and scale.)
	// This centers the space on the scene atom.
	const DaSolarSystem* pss = sceneAtom->FindParentSolarSystem();
	if(!pss)
		return;
	AuPoint sysloc = pss->Location();

	// Determine the center point, in parsecs, about which to offset the star field.
	AuPoint fSceneLoc;
	sceneAtom->GetLocationRelativeToParent(fSceneLoc, this);

	// Disable the Z buffer so distant stars are overwritten by suns.
	ari.re->glDepthMask(GL_FALSE);
	ari.re->glDisable(GL_DEPTH_TEST);

	// Calculate a transform that will center the stars around the viewer.
	AuMatrix4x4 sysRot;
	sysRot.Rotate(pss->Rotation());
	AuMatrix4x4 tsub = sysRot * sceneAtom->EnvTransform();
	AuPoint floc = tsub.Project(AuPoint(0,0));	// Scene atom loc rel to its parent solar system.
	if(pss->HasBlackHole())
	{
		// Offset to the non black hole companion, for wormhole lines.
		const DaStar* star = pss->FindPrimaryStarNotBlackHole();
		if(star)
		{
			AuMatrix4x4 ssub = sysRot * star->EnvTransform();
			AuPoint sloc = ssub.Project(AuPoint(0,0));
			floc += sloc;
		}
	}
	AuMatrix4x4 tpre;
	tpre.Translate(-floc);
	tsub = tpre * tsub;
	AuMatrix4x4 tmodel = model * tsub.Inverse();

	// Make the stars fade into the sky when there is local sunlight. This effect is modulated by atmosphere density.
	float opacity = 1;
	if(sceneAtom->IsInAtmosphere())
	{
		opacity = 1.f - (sceneAtom->Illumination() * sceneAtom->AtmosphereDensity() * 4);
		if(opacity < 0)
			opacity = 0;	
	}

	// Render Sky box
	// Nebula data is needed for sky box and nebula cloud effect, though it's overkill for just the cloud.
	// Nebula data is needed for black holes to draw their refraction zone.
	if(!m_Nebula)
	{
		//qDebug() << "DaSector::RenderEnvironment" << Id() << Name() << "creating sky box";
		// First determine if this Sector is part of a nebula.  Nebula's are 3x3 Sectors in size
		bool hasNebula = false;
		AuPoint nebSectorLoc;
		// Check neighbors
		AuPoint pos;
		AuDice dice;
		quint32 seed = 0;
		DaGalaxy *pg = (DaGalaxy*)Parent();
		AuPoint sectorLoc = Location();						// Location of this sector in pc.
		for(int ix=-10; ix<=10; ix+=10)
		{
			for(int iy=-10; iy<=10; iy+=10)
			{
				for(int iz=-10; iz<=10; iz+=10)
				{
					AuPoint fn = sectorLoc + AuPoint(ix, iy, iz);
					DaSector* pn = pg->FindSectorAt(fn);
					if(pn)
					{
						nebSectorLoc = pn->Location();
						AuPoint secLoc = nebSectorLoc/10.;
						if(((int)secLoc.x)%3 == 0 &&		// Only sectors divisible by 3 will contain nebula
								((int)secLoc.y)%3 == 0 &&
								((int)secLoc.z)%3 == 0)
						{
							seed = pn->GenSeed();
							dice.Seed(seed);
							if(dice.Roll(1,2) == 2)
							{
								pos = secLoc;
								hasNebula = true;
								break;
							}
						}
					}
				}
				if(hasNebula)
					break;
			}
			if(hasNebula)
				break;
		}

		int iParticles = 0;
		if(hasNebula)
			iParticles = 1048576;

		// Color of nebula not stars so this only really matters when we have nebula in which case the dice object got seeded
		// by the Sector Id which is the center of this nebula
		int iColor = dice.Roll(1, 6, -1);			// Roll 0 - 5
		bool bReverseColor = dice.Roll(1, 2, -1);	// Roll 0 or 1
		// Type: 0 - More Swirly, 6 - More Cloudy
		int iType = dice.Roll(1, 7, -1);			// 0-6
		int cloudTexture = dice.Roll(1,6,-1);	// 0 to 5
		//qDebug("Seed %d iColor %d bReverseColor %c", seed, iColor, bReverseColor?'Y':'N');
		m_Nebula = new auNebula(iParticles, 0, -2.9924924f, 0.38151205f, -1.8401746f, -1.8524430f,
				30, iColor, bReverseColor, pos, seed, iType, FindParentGalaxy()->Color());
		m_Nebula->SetCloudTexture(cloudTexture);

		// Now setup stars
		m_Nebula->m_vStars.clear();

		// Get Sector location in Sector Coordinates
		sectorLoc /= 10.;									// Convert to a "sector" location.
		int iDistance = pg->Radius() / 10;					// sectors.
		int zDist = pg->CoreRadius() / 10;
		//qDebug() << "galaxy radius" << iDistance << "sectors";
		int step = sectorLoc.Length() / (iDistance*2);
		if(step < 1)
			step = 1;
		for(int ix=-iDistance; ix<=iDistance; ix+=step)
		{
			for(int iy=-iDistance; iy<=iDistance; iy+=step)
			{
				for(int iz=-zDist; iz<=zDist; iz+=step)
				{
					AuPoint nbsec(ix, iy, iz);				// Neighboring sector. ix, iy, iz are sectors.
					float fDensity = pg->StarDensity(nbsec*10);	// Returns a number from 0 to 1.
					if(fDensity > 0)
					{
						double sdist = (nbsec - sectorLoc).Length() + 1;	// Distance from my sector, in sectors.
						int iStars = (int)ceil(fDensity*50 / (sdist*8));
						for(int i=0; i<iStars; i++)
						{
							float fX = dice.RandomMinMax(-step, step);
							float fY = dice.RandomMinMax(-step, step);
							float fZ = dice.RandomMinMax(-step, step);
							galParticle nuParticle;
							nuParticle.pos = nbsec + AuPoint(fX, fY, fZ) - sectorLoc;
							m_Nebula->m_vStars.append(nuParticle);
						}
					}
				}
			}
		}
		//qDebug() << "stars" << m_Nebula->m_vStars.count();

		// Get System location inside its Sector Coordinates
		AuPoint solarLoc = sysloc / 10.;			// Location in "sectors".
		sectorLoc += solarLoc;		// Add solar location offset to sector location. Result in "sector" coordinates (10x pc).

		m_Nebula->CreateSkyBox(ari, sectorLoc, g_Au.SkyBoxPaneSize(), g_Au.SkyBoxMipMap());
	}
	else
	{
		//qDebug() << "DaSector::RenderEnvironment" << Id() << Name() << "rendering sky box";
		if(opacity && g_Au.DrawSkyBox())
			m_Nebula->RenderSkyBox(*ari.re, farProjection, tmodel, opacity);	// The sky box is visible, at least a little bit.

		// Calculate nebula cloud density. This is the density of the cloudy nebula effect painted in the scene.
		float cloudDensity;
		if(m_Nebula->m_iParticles)
		{
			AuPoint syssecloc = Location() + sysloc;					// pc location of scene atom's solar system.
			double dist = syssecloc.Distance(m_Nebula->m_Center*10);	// pc distance from system to center of nebula.
			// 15 pc is the distance to the closest far side of any neighboring sector. Beyond that and the player cannot see
			// the nebula anyway.
			if(dist < 15.)
			{
				float linear = (float)dist / 15.f;		// Linear distance from center, in the range 0 to 1.
				cloudDensity = cosf(linear * PIf/2.f);	// Density is 1 at center, with curving dropoff to 0 at limit.
			}
			else
				cloudDensity = 0.f;
		}
		else
			cloudDensity = 0.f;
		m_Nebula->SetCloudDensity(cloudDensity);
	}

	// Calculate minimum distance from viewer to render stars.
	// They don't have to be far away since the depth buffer is disabled.
	const float rStarShellMinRadius = FeetPerOrbit();

	// Sector coordinates are in parsecs.
	// I'm going to scale them to be in the range of 1 to 2 solar systems, assuming the closest possible sector is .1 parsecs
	// and the farthest is 2 sectors (20 parsecs).
	// Draw each solar system as a star object.
	tmodel.Scale(rStarShellMinRadius,rStarShellMinRadius,rStarShellMinRadius);

	// Draw wormhole lines before stars so the stars at the vertices overlap the lines.
	AuPoint skiploc = sysloc - fSceneLoc;
	double sysDist = sceneAtom->EnvLocation().Length();
	double sysScale = sysDist / skiploc.Length();
	sysScale /= rStarShellMinRadius;

	if(sceneAtom->CanSeeWormholes() && ari.m_ShowHUD)
	{
		// Select vertex fog shader for distance fade.
		// Setup linear atmospheric fog so lines vanish into the distance.
		// 5x5x5 parsecs away = 8.66 parsecs
		ari.re->GLSL().LineFogShader().Select();
		ari.re->GLSL().LineFogShader().SetProjectionModelView(farProjection * tmodel);
		ari.re->GLSL().LineFogShader().SetFogDepth(rStarShellMinRadius, rStarShellMinRadius * 8.66);

		GLfloat lineExt[2];
		ari.re->glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, lineExt);
		//DHH glsl ari.re->glLineWidth(lineExt[0]);						// wormhole lines are thinnest of all.

		ari.re->GLSL().LineFogShader().SetFogColor(51.f/255.f, 10.f/255.f, 0.f/255.f, 1.f);
		ari.re->GLSL().LineFogShader().SetColor(179.f/255.f, 36.f/255.f, 0.f/255.f);

		/*DHH glsl
		ari.re->glBegin(GL_LINES);
			for(int iw=0; iw<m_ExploredWormholes.count(); iw++)
			{
				AuSegment seg = m_ExploredWormholes[iw];
				AuPoint segloc0 = seg.m_fLoc1 - fSceneLoc;
				AuPoint segloc1 = seg.m_fLoc2 - fSceneLoc;
				if(segloc0.FuzzyEquals(skiploc))
					segloc0 *= sysScale;
				else if(segloc1.FuzzyEquals(skiploc))
					segloc1 *= sysScale;

				// Test to see if the segment is behind the camera.
				AuPoint vt0 = tmodel.Project(segloc0);
				AuPoint vt1 = tmodel.Project(segloc1);
				if(vt0.z > 0 && vt1.z > 0)
					continue;

				ari.re->glVertex3d(segloc0.x, segloc0.y, segloc0.z);
				ari.re->glVertex3d(segloc1.x, segloc1.y, segloc1.z);
			}
		ari.re->glEnd();
		*/

		// Compile a list of vertex locations.
		AuPointArray verts;

		for(int iw=0; iw<m_ExploredWormholes.count(); iw++)
		{
			AuSegment seg = m_ExploredWormholes[iw];
			AuPoint segloc0 = seg.m_fLoc1 - fSceneLoc;
			AuPoint segloc1 = seg.m_fLoc2 - fSceneLoc;
			if(segloc0.FuzzyEquals(skiploc))
				segloc0 *= sysScale;
			else if(segloc1.FuzzyEquals(skiploc))
				segloc1 *= sysScale;

			// Test to see if the segment is behind the camera.
			AuPoint vt0 = tmodel.Project(segloc0);
			AuPoint vt1 = tmodel.Project(segloc1);
			if(vt0.z > 0 && vt1.z > 0)
				continue;

			verts.append(segloc0);
			verts.append(segloc1);
		}

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(*ari.re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		ari.re->GLSL().LineFogShader().SetFogColor(0.f/255.f, 10.f/255.f, 51.f/255.f, 1.f);
		ari.re->GLSL().LineFogShader().SetColor(0.f/255.f, 36.f/255.f, 179.f/255.f);

		/*DHH glsl
		ari.re->glBegin(GL_LINES);
			for(int iw=0; iw<m_KnownWormholes.count(); iw++)
			{
				AuSegment seg = m_KnownWormholes[iw];
				AuPoint segloc0 = seg.m_fLoc1 - fSceneLoc;
				AuPoint segloc1 = seg.m_fLoc2 - fSceneLoc;
				if(segloc0.FuzzyEquals(skiploc))
					segloc0 *= sysScale;
				else if(segloc1.FuzzyEquals(skiploc))
					segloc1 *= sysScale;

				// Test to see if the segment is behind the camera.
				AuPoint vt0 = tmodel.Project(segloc0);
				AuPoint vt1 = tmodel.Project(segloc1);
				if(vt0.z > 0 && vt1.z > 0)
					continue;

				ari.re->glVertex3d(segloc0.x, segloc0.y, segloc0.z);
				ari.re->glVertex3d(segloc1.x, segloc1.y, segloc1.z);
			}
		ari.re->glEnd();
		*/

		verts.clear();
		for(int iw=0; iw<m_KnownWormholes.count(); iw++)
		{
			AuSegment seg = m_KnownWormholes[iw];
			AuPoint segloc0 = seg.m_fLoc1 - fSceneLoc;
			AuPoint segloc1 = seg.m_fLoc2 - fSceneLoc;
			if(segloc0.FuzzyEquals(skiploc))
				segloc0 *= sysScale;
			else if(segloc1.FuzzyEquals(skiploc))
				segloc1 *= sysScale;

			// Test to see if the segment is behind the camera.
			AuPoint vt0 = tmodel.Project(segloc0);
			AuPoint vt1 = tmodel.Project(segloc1);
			if(vt0.z > 0 && vt1.z > 0)
				continue;

			verts.append(segloc0);
			verts.append(segloc1);
		}

		// Make a VAO of the vertex locations.
		vao.CreateForLines(*ari.re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		ari.re->GLSL().LineFogShader().Deselect();
	}

	// Now draw stars.
	if(opacity)
	{
		QMultiMap<float, AuPoint> stars;	// key is glPointSize parameter.
		for(DqAtomLocArray::const_iterator sti = m_aStars.begin(); sti != m_aStars.end(); sti++)
		{
			// Test to see if the star is behind the camera.
			AuPoint relLoc = (*sti).m_fLocation - fSceneLoc;
			if((*sti).m_pA == pss)
				relLoc *= sysScale;
			AuPoint vt = tmodel.Project(relLoc);
			if(vt.z > 0)
				continue;	// Skipped star behind camera.
			else
			{
				vt.Unit();
				if(vt.Dot(AuPoint(0,0,-1)) < .7071)
					continue;
			}

			// Calculate point size of this star.
			float rlen = relLoc.Length();		// Distance to the star in parsecs (up to about 25).
			float ptsize = (1.f - rlen / 25) * 10 + 6;
			if(ptsize < 4)
				ptsize = 4;
			stars.insert(ptsize, relLoc);
		}

		if(!stars.isEmpty())
		{
			// Draw stars using point sprites.
			ari.re->GLSL().NearStarShader().Select();
			ari.re->GLSL().NearStarShader().SetProjectionModelView(farProjection * tmodel);
			ari.re->GLSL().NearStarShader().SetOpacity(opacity);
			ari.re->GLSL().NearStarShader().SetTexture(s_StarTexture);
			//qDebug() << "DaSector::RenderEnvironment" << Id() << Name() << "near stars" << stars.count() << "opacity" << opacity
				//<< "tex" << s_StarTexture;

			// Create vertex locations, 4th value is point size.
			int vertices = stars.count();
			GLfloat* starPos = new GLfloat[vertices*4];
			int starPosIdx = 0;
			for(QMultiMap<float, AuPoint>::const_iterator si = stars.cbegin(); si != stars.cend(); si++)
			{
				starPos[starPosIdx++] = (*si).x;
				starPos[starPosIdx++] = (*si).y;
				starPos[starPosIdx++] = (*si).z;
				starPos[starPosIdx++] = si.key();
			}

			AuGLVAO vao;
			vao.CreateGL(*ari.re, vertices, 1);
			if(vao.VAO())
			{
				vao.SelectGL();

				// bind buffer for vertices and copy data into buffer.
				ari.re->glBindBuffer(GL_ARRAY_BUFFER, vao.Buffer(0));
				ari.re->glBufferData(GL_ARRAY_BUFFER, vertices*4*sizeof(GLfloat), starPos, GL_STREAM_DRAW);
				ari.re->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);	// Describe the data in the buffer.

				ari.re->glBindBuffer(GL_ARRAY_BUFFER, 0);

				// Display the buffer into the current display list.
				ari.re->glEnable(GL_PROGRAM_POINT_SIZE); 

				ari.re->glEnable(GL_BLEND);
				ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();

				// Delete the vertex array object. It is no longer needed; the display list has what it wants.
				vao.DeleteGL();

				ari.re->glDisable(GL_BLEND);

				ari.re->glDisable(GL_PROGRAM_POINT_SIZE); 
			}

			delete [] starPos;

			ari.re->GLSL().NearStarShader().Deselect();
		}
	}

	// System names can be visible to navigators.
	int sysNames = sceneAtom->CanSeeSystemNames();
	if(sysNames && ari.m_ShowHUD)
	{
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		for(DqAtomLocArray::const_iterator sti = m_aStars.begin(); sti != m_aStars.end(); sti++)
		{
			// Test to see if the star is behind the camera.
			AuPoint relLoc = (*sti).m_fLocation - fSceneLoc;
			if((*sti).m_pA == pss)
				relLoc *= sysScale;
			AuPoint vt = tmodel.Project(relLoc);
			if(vt.z > 0)
				continue;	// Skipped star behind camera.
			else
			{
				AuPoint vt2(vt);
				vt2.Unit();
				if(vt2.Dot(AuPoint(0,0,-1)) < .7071)
					continue;
			}

			QColor color;
			if(((const DaSolarSystem*)((*sti).m_pA))->HasName())
				color = QColor(255,255/2,255/4, 192);
			else
			{
				if(sysNames == 1)
					continue;	// Only named systems are wanted.
				color = QColor(255/2,255/2,255, 192);
			}

			ari.RenderText(farProjection, tmodel, relLoc, QString(" ") + (*sti).m_pA->Name(), color,
					Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
		}
		ari.re->glDisable(GL_BLEND);
	}

	ari.re->glDepthMask(GL_TRUE);
	ari.re->glEnable(GL_DEPTH_TEST);
}

// Lists the star and wormhole locations in this sector and the sectors adjacent to this sector.
// 'allOrNone' is true to return any data only if all neighboring sectors are present.
void DaSector::ListSolarSystemLocations(DqAtomLocArray& systems) const
{
	AuPoint fSectorLoc = Location();	// Location of this sector in the galaxy.
	const DaGalaxy *pg = (const DaGalaxy*)Parent();
	int ix=0, iy=0, iz=0;
	for(ix=-10; ix<=10; ix+=10)
	{
		for(iy=-10; iy<=10; iy+=10)
		{
			for(iz=-10; iz<=10; iz+=10)
			{
				AuPoint frel(ix, iy, iz);
				AuPoint fn = fSectorLoc + frel;
				const DaSector* pn = pg->FindSectorAt(fn);
				if(pn)
					pn->ListSolarSystemLocations(systems, frel);
			}
		}
	}
}

void DaSector::ListSolarSystemLocations(DqAtomLocArray& systems, const AuPoint& offset) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsEmptySystem())
			continue;

		// Add system offset location to the solar systems array.
		DqAtomLoc al;
		al.m_fLocation = pss->Location() + offset;
		al.m_pA = pss;
		systems.append(al);
	}
}

// Lists the star and wormhole locations in this sector and the sectors adjacent to this sector.
// 'allOrNone' is true to return any data only if all neighboring sectors are present.
void DaSector::ListStarAndWormholeLocations(DqAtomLocArray &aStars, AuSegmentArray& exploredWormholes,
		AuSegmentArray& knownWormholes, quint32 empire, bool allOrNone) const
{
	//qDebug() << "DaSector::ListStarAndWormholeLocations" << Id() << Name() << Description();
	AuPoint fSectorLoc = Location();	// Location of this sector in the galaxy.
	DaGalaxy *pg = (DaGalaxy*)Parent();
	int ix=0, iy=0, iz=0;
	for (ix=-10; ix<=10; ix+=10)
	{
		for (iy=-10; iy<=10; iy+=10)
		{
			for (iz=-10; iz<=10; iz+=10)
			{
				AuPoint frel(ix, iy, iz);
				AuPoint fn = fSectorLoc + frel;
				DaSector *pn = pg->FindSectorAt(fn);
				if(pn)
					pn->ListSolarSystemLocations(aStars, frel, exploredWormholes, knownWormholes, empire);
				else if(allOrNone)
				{
					// We have not received all neighboring sectors from the server yet. Return an invalid result until we have
					// all neighboring sectors present in the local data.
					//AuPoint fLoc = fn/10.0;
					//QString sloc = QString("(%1, %2, %3)").arg(fLoc.x).arg(fLoc.y).arg(fLoc.z);
					//qDebug() << "DaSector::ListStarAndWormholeLocations" << Id() << Name() << "No sector" << ix << iy << iz
						//<< "at" << sloc << "galaxy" << pg << pg->SubatomCount();
					aStars.clear();
					exploredWormholes.clear();
					knownWormholes.clear();
					return;
				}
			}
		}
	}
}

void DaSector::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	if(m_Nebula)
	{
		m_Nebula->DeleteDeviceObjects(*ari.re);
		delete m_Nebula;
		m_Nebula = 0;
	}

	DaAtom::DeleteDeviceObjects(ari);
}

// Insert first time database record.
void DaSector::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Sector (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
			"timeCreate,timeModified,timeTock,timeDeath, name) VALUES "
			"(?,?,?,?,?,?,?,?,?,?,?,?, ?);");
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
	qa.addBindValue(m_sName);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaSector::Insert %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
}

// Update existing database record.
void DaSector::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq = QString("UPDATE a_Sector SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, "
			"rotZ=?, timeModified=?, timeTock=?, timeDeath=?, name=? "
			"WHERE id=%1;").arg(Id().GetID());
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
	qa.addBindValue(m_sName);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaSector::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		qDebug() << "DaSector::Update" << Id() << Name() << "affected NO records. Trying INSERT instead.";
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// 'idToLoad' is the sector id to load when 'atLoc' is 0.
// 'idToLoad' is the parent galaxy id when 'atLoc' is not 0.
// Returns 0 if 'idToLoadOrGalaxy' == 0 and 'atLoc' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoadOrGalaxy' != 0 or 'atLoc' != 0 and it is successfully loaded.
DaSector* DaSector::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
		const AuId& idToLoadOrGalaxy/*=AuId(0)*/, const AuPoint* atLoc/*=0*/)
{
	QString sq("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
			"timeModified,timeTock,timeDeath,name FROM a_Sector WHERE ");
	if(atLoc)
	{
		Q_ASSERT(!idToLoadOrGalaxy.IsNull());
		sq += QString(" idp=%1").arg(idToLoadOrGalaxy.GetID());
		sq += QString(" AND locX=%1").arg(atLoc->x);
		sq += QString(" AND locY=%1").arg(atLoc->y);
		sq += QString(" AND locZ=%1").arg(atLoc->z);
	}
	else if(!idToLoadOrGalaxy.IsNull())
		sq += QString(" id=%1").arg(idToLoadOrGalaxy.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaSector *pnu = new DaSector;
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

			pnu->m_sName = qa.value(qv++).toString();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			if(g_Au.IsSceneMaster())
			{
				// When loading sectors on the master, we notify our parent galaxy of the nu sector and clear the assignable
				// sectors cache.
				DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(ether.FindAtom(parentId));
				if(galaxy)
				{
					// The parent galaxy's assignable sectors cache is no longer valid. It will be reevaluated when needed.
					galaxy->ResetAssignableSectorsCache();
					galaxy->AddSectorLoc(pnu->Id(), pnu->Location());
				}
			}

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			if(loadWhat != LOAD_NoSubatoms)
			{
				DaSolarSystem::LoadSubatoms(ether, db, pnu->Id(), loadWhat);
				if(g_Au.m_LogMessages & LOG_Database)
				{
					if(loadWhat & LOAD_StarField)
						qDebug() << pnu->Id() << pnu->Name() << pnu->Description()
							<< "loaded containing" << pnu->SolarSystemCount() << "systems for star field";
					else
						qDebug() << pnu->Id() << pnu->Name() << pnu->Description()
							<< "loaded containing" << pnu->SolarSystemCount() << "systems";
				}
			}
		}
		if(!idToLoadOrGalaxy.IsNull() || atLoc)
			return pnu;
	}
	return 0;
}

bool DaSector::HasAvailableHomeWorldSystem() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsFinal())
			continue;
		if(pss->GenHab() != HABITABLE_HomeWorld)
			continue;
		return true;
	}
	return false;
}

bool DaSector::IsReadyToDecay() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaSolarSystem* pss = dynamic_cast<const DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsFinal())
		{
			// Sector cannot decay while it contains any finalized solar system.
			return false;
		}
	}
	return true;
}

// NOTE: This is only called on the parent sector of the scene atom.
// NOTE: Atom comparisons must use Ids because two different atom lists are in use
//       when ListStarFieldSubatoms is called. See DtDb::SyncClient and how it adds the
//       star field atoms to the scene.
bool DaSector::ListStarFieldSubatoms(const DaAtom *pSceneAtom, DaAtomList& sceneAtoms) const
{
	bool allPresent = true;
	//qDebug("DaSector::ListStarFieldSubatoms - parent sector %d", Id().GetID());
	// Get solar systems and wormholes from this sector and its 26 neighboring sectors.
	AuPoint fSectorLoc = Location();		// Location of this sector in the galaxy.
	const DaGalaxy *pg = (DaGalaxy*)Parent();	// NOTE: This is sometimes the parent galaxy of the star field.
	for (int ix=-10; ix<=10; ix+=10)
	{
		for (int iy=-10; iy<=10; iy+=10)
		{
			for (int iz=-10; iz<=10; iz+=10)
			{
				AuPoint fn = fSectorLoc + AuPoint(ix,iy,iz);
				const DaSector *pn = pg->FindSectorAt(fn);
				if(pn)
				{
					// Add this atom to the list if the id is unique.
					sceneAtoms.AddIfUniqueId((DaAtom*)pn);

					// Get this atom's star field subatoms, without neighbors.
					if(!pn->ListStarFieldSubs(pSceneAtom, sceneAtoms))
						allPresent = false;
				}
				else
				{
					//AuPoint fLoc = fn/10.0;
					//QString sloc = QString("(%1, %2, %3)").arg(fLoc.x).arg(fLoc.y).arg(fLoc.z);
					//qDebug() << "DaSector::ListStarFieldSubatoms - sector at" << sloc << "not found" << pg << pg->SubatomCount();
					allPresent = false;
				}
			}
		}
	}
	return allPresent;
}

// NOTE: Atom comparisons must use Ids because two different atom lists are in use
//       when ListStarFieldSubatoms is called. See DtDb::SyncClient and how it adds the
//       star field atoms to the scene.
bool DaSector::ListStarFieldSubs(const DaAtom *pSceneAtom, DaAtomList& sceneAtoms) const
{
	// Get star field subatoms of this sector.
	return DaAtom::ListStarFieldSubatoms(pSceneAtom, sceneAtoms);
}

// Not recursive.
void DaSector::ListAssignableSolarSystems(DaAtomList& systems) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;

		// We don't assign solar systems until their contents are finalized.
		if(pss->IsFinal())
			systems.append(pss);
	}
}

void DaSector::ListSceneSubatoms(const DaPerson*, const AuPoint& cameraEnvLoc, DaAtomList&)
{
	Q_UNUSED(cameraEnvLoc);
	// Sector does not add all subordinate solar systems to the scene. This
	// would be the result if the base DaAtom::ListSceneSubatoms was called.
	// Instead, the scene solar system is going to be queried next so let it
	// get the appropriate subatoms.
}

void DaSector::RemoveGamePiecesFromStarField()
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsOnlyStarFieldLoaded())
			pss->TrashNotInStarField();
	}
}

void DaSector::FinalizePositiveWormholes(bool updateDb)
{
	//Q_ASSERT(g_Au.IsSceneMaster());
	// Get a list of my systems and those in adjacent sectors, with locations relative to this sector.
	DqAtomLocArray systems;
	ListSolarSystemLocations(systems);
	if(systems.empty())
		return;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(*it);
		if(!system)
			continue;
		system->RepairPositiveWormholes(systems, updateDb);
	}
}

void DaSector::FinalizeNegativeWormholes()
{
	//Q_ASSERT(g_Au.IsSceneMaster());
	// Get a list of my systems and those in adjacent sectors, with locations relative to this sector.
	DqAtomLocArray systems;
	ListSolarSystemLocations(systems);
	if(systems.empty())
		return;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(*it);
		if(!system)
			continue;
		system->RepairNegativeWormholes(systems, true);
	}
}

// Actual number of solar systems. May differ from SolarSystemsInSector.
int DaSector::SolarSystemCount() const
{
	int systemCount = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaSolarSystem* pss = dynamic_cast<const DaSolarSystem*>(*it);
		if(!pss)
			continue;
		systemCount++;
	}
	return systemCount;
}

DaSolarSystem* DaSector::FindCoreSolarSystem() const
{
	if(!IsCore())
		return 0;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(*it);
		if(!pss)
			continue;
		if(pss->IsCoreBlackHole())
			return pss;
	}
	return 0;
}

QColor DaSector::NebulaColor() const
{
	if(!m_Nebula)
		return QColor(0,0,0);
	return m_Nebula->CloudColor();
}

float DaSector::NebulaDensity() const
{
	if(!m_Nebula)
		return 0.f;
	return m_Nebula->CloudDensity();
}

int DaSector::NebulaTexture() const
{
	if(!m_Nebula)
		return 0;
	return m_Nebula->CloudTexture();
}

GLuint DaSector::SkyBoxTexture() const
{
	if(!m_Nebula)
		return 0;
	return m_Nebula->m_img;
}

quint64 DaSector::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaSector) - sizeof(DaAtom);
	mem += StringMemoryUsage(m_sName);
	mem += sizeof(DqAtomLoc) * m_aStars.size();
	mem += sizeof(AuSegment) * m_ExploredWormholes.capacity();
	mem += sizeof(AuSegment) * m_KnownWormholes.capacity();
	return mem;
}

DaSector* DaSector::FindParentSector()
{
	return this;
}

const DaSector* DaSector::FindParentSector() const
{
	return this;
}

int DaSector::HomeWorldSystems() const
{
	const DaGalaxy* galaxy = FindParentGalaxy();
	if(!galaxy)
	{
		qWarning() << "error: DaSector::HomeWorldSystems called with no parent galaxy" << Id() << Name();
		return 0;
	}

	// DaGalaxy::CreateNewHomeWorldSector looks for a sector with at least 10 systems.
	// Because of that, we will guarantee that sectors with 10 systems have at least one home world system.
	const int systems = SolarSystemsInSector();	// Range is 0 to 50.
	if(systems == 0)
		return 0;
	int homeSystems = (systems + 6)/16;		// Range is 0 to 3.

	// Fiddle with the number of home world systems to insure habitable systems will be found in the void regions of space, such as
	// the space outside the galaxy.
	AuDice dice(GenSeed());
	quint32 result = dice.Roll(1,16);
	if(result == 1)
	{
		// Add an extra home world system to 1 of 16 sectors, for sectors with few systems.
		if(homeSystems < systems)
			homeSystems++;
	}
	else if(result == 16)
	{
		// Subtract a habitable system from 1 of 16 sectors, for balance.
		if(homeSystems > 0)
			homeSystems--;
	}
	return homeSystems;
}

bool DaSector::HasPirateBase() const
{
	quint32 empireId;
	const DgData& gd = g_Au.LockDataForRead();
	AuId baseSystem = gd.PirateBaseSystemInSector(Id(), empireId);
	g_Au.UnlockData();
	return !baseSystem.IsNull();
}

const DaSolarSystem* DaSector::EncounterPirateBase(const AuId& encSystem, quint32& empireId)
{
	// Check for existing pirate base in this sector.
	const DgData& gd = g_Au.LockDataForRead();
	AuId baseSystem = gd.PirateBaseSystemInSector(Id(), empireId);
	g_Au.UnlockData();
	if(!baseSystem.IsNull())
	{
		// Empire data says there is a pirate base in 'baseSystem' of this sector.
		const DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindSubatomId(baseSystem));
		if(!system)
			qWarning() << "error: DaSector::EncounterPirateBase" << *this << "baseSystem not found" << baseSystem;
		return system;
	}

	// No system in this sector contains a pirate base.
	// Emit a signal so the manager server can choose a system for a new pirate base. The manager might not do anything.
	//qDebug() << "DaSector::EncounterPirateBase" << *this << "signaling for pirate base encounter, from system" << encSystem;
	emit SectorPirateBaseEncounter(Id(), encSystem);
	empireId = 0;
	return 0;
}

// Manager receives the SectorPirateBaseEncounter signal here.
// Returns the id of a system in which a pirate base can be created.
AuId DaSector::PirateBaseEncounter(const AuId& inSystemId)
{
	if(HasPirateBase())
	{
		//qDebug() << "DaSector::PirateBaseEncounter" << *this << "sector already has a pirate base";
		return AuId();	// Empire data says there is already a pirate base in this sector.
	}

	// Get a list of ids of my finalized solar systems, excluding 'inSystemId'.
	AuIdList habids,sysids,habnf,sysnf;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(*it);
		if(!system)
			continue;
		if(system->Is(inSystemId))
			continue;	// Skip the system in which the encounter was triggered.
		if(system->IsFinal())
		{
			// We prefer using visited systems, instead of finalizing new ones.
			if(system->FindHabitableWorld(HABITABLE_System))
				habids.append(system->Id());
			else
			{
				// Look for a world.
				DaAtomList planets;
				system->FindPlanetsRecursive(planets);
				if(!planets.empty())
				{
					DaAtomList worlds;
					for(DaAtomList::const_iterator pi = planets.cbegin(); pi != planets.cend(); pi++)
					{
						DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(*pi);
						if(!llworld)
							continue;	// Skip non ll worlds. We would already have found ringworlds in FindHabitableWorld.
						if(llworld->Hydrographics() > 90)
							continue;	// Skip water/lava worlds.
						sysids.append(system->Id());
					}
				}
			}
		}
		else
		{
			// If we must finalize a system, lets prefer a nice one.
			if(system->GenHab() == HABITABLE_HomeWorld)
				habnf.append(system->Id());
			else
				sysnf.append(system->Id());
		}
	}
	if(habids.empty() && sysids.empty() && habnf.empty() && sysnf.empty())
	{
		// There are no systems for a pirate base.
		//qDebug() << "DaSector::PirateBaseEncounter" << *this << "no finalized systems exist for a pirate base";
		return AuId();
	}
	else
	{
		// Exclude system ids that have cities of any kind.
		const DgData& gd = g_Au.LockDataForRead();
		gd.RemoveInhabitedSystems(habids);
		gd.RemoveInhabitedSystems(sysids);
		gd.RemoveInhabitedSystems(habnf);	// Better not find any cities in these systems, lol.
		gd.RemoveInhabitedSystems(sysnf);	// Better not find any cities in these systems, lol.
		g_Au.UnlockData();

		// The remaining lists of ids are those systems that appear uninhabited.
		if(!habids.empty())
		{
			// Habitable systems are preferred.
			//qDebug() << "DaSector::PirateBaseEncounter" << *this << "in habitable system" << habids.first();
			if(habids.size() == 1)
				return habids.first();
			else
				return habids[g_Au.Dice().Roll(1,habids.size(),-1)];
		}
		else if(!sysids.empty())
		{
			//qDebug() << "DaSector::PirateBaseEncounter" << *this << "in uninhabitable system" << sysids.first();
			if(sysids.size() == 1)
				return sysids.first();
			else
				return sysids[g_Au.Dice().Roll(1,sysids.size(),-1)];
		}
		else if(!habnf.empty())
		{
			// Home world systems are preferred.
			//qDebug() << "DaSector::PirateBaseEncounter" << *this << "in unfinalized homeworld system" << habnf.first();
			AuId sysId;
			if(habnf.size() == 1)
				sysId = habnf.first();
			else
				sysId = habnf[g_Au.Dice().Roll(1,habnf.size(),-1)];
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindSubatomId(sysId));
			if(system)
			{
				system->FinalizeContents();	// Finalize causes the system to be assigned to a worker server.
				if(g_Au.IsSceneSolo())
				{
					system->SetStarFieldFlag(false);	// In solo all final systems are not star field systems.
					system->TickTockRecursive();
				}
				return sysId;
			}
		}
		else if(!sysnf.empty())
		{
			//qDebug() << "DaSector::PirateBaseEncounter" << *this << "in unfinalized system" << sysnf.first();
			AuId sysId;
			if(sysnf.size() == 1)
				sysId = sysnf.first();
			else
				sysId = sysnf[g_Au.Dice().Roll(1,sysnf.size(),-1)];
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindSubatomId(sysId));
			if(system)
			{
				system->FinalizeContents();	// Finalize causes the system to be assigned to a worker server.
				if(g_Au.IsSceneSolo())
				{
					system->SetStarFieldFlag(false);	// In solo all final systems are not star field systems.
					system->TickTockRecursive();
				}
				return sysId;
			}
		}

		// There are no uninhabited systems for a pirate base.
		//qDebug() << "DaSector::PirateBaseEncounter" << *this << "no uninhabited systems exist for a pirate base";
		return AuId();
	}
}

bool DaSector::GalaxyLocToSectorLoc(const AuPoint& galaxyLoc, AuPoint& sectorLoc) const
{
	AuPoint galSecLoc(galaxyLoc);
	galSecLoc.Round(10.);
	if(galSecLoc != Location())
		return false;				// 'galaxyLoc' is not in this sector.
	//why? galSecLoc -= AuPoint(5,5,5);
	sectorLoc = galaxyLoc - galSecLoc;
	return true;
}

DaAtom::ATOM_TYPE DaSector::AtomType() const
{
	return ATOM_Sector;
}

bool DaSector::IsNeighbor(const DaSector& sector) const
{
	// Compare parent galaxies.
	if(ParentId() != sector.ParentId())
		return false;					// Different galaxies.

	// Check distance to the other sector.
	AuPoint myLoc = Location();			// Location of this sector in the galaxy, in pc. A sector is 10x10x10pc.
	AuPoint oLoc = sector.Location();	// Location of the other sector in the galaxy, in pc. A sector is 10x10x10pc.
	if(fabs(myLoc.x - oLoc.x) > 11)
		return false;					// Too far away in X.
	if(fabs(myLoc.y - oLoc.y) > 11)
		return false;					// Too far away in Y.
	if(fabs(myLoc.z - oLoc.z) > 11)
		return false;					// Too far away in Z.
	return true;
}

int DaSector::InSupersector(const AuId& centerSectorId) const
{
	// Check this sector.
	if(Is(centerSectorId))
		return 2;						// This is the center of the supersector.

	// Check the other sector.
	const DaGalaxy *pg = dynamic_cast<const DaGalaxy*>(Parent());
	if(!pg)
		return 0;						// wut?
	const DaSector* sector = dynamic_cast<const DaSector*>(pg->FindIdRecursive(centerSectorId));
	if(!sector)
		return 0;						// Other sector not found locally. Must be far away.

	// My neighbors comprise the supersector around 'centerSectorId'.
	return IsNeighbor(*sector) ? 1 : 0;
}

void DaSector::StarMapInfo(quint32 empireId, AuStarMapSector& sm) const
{
	sm.m_Name = NameKnownTo(empireId);
	sm.m_Loc = Location();
	sm.m_Galaxy = Galaxy();
}

void DaSector::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	m_SectorLock.lockForRead();
	out << m_sName;
	m_SectorLock.unlock();
}

void DaSector::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	m_SectorLock.lockForWrite();
	in >> m_sName;
	m_SectorLock.unlock();
}

