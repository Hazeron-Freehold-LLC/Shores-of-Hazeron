//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworld.cpp,v 1.199 2022/05/17 18:43:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daworld.h"
//
#include "aucityreport.h"
#include "aucommpacket.h"
#include "aunoise.h"
#include "ausynccontroller.h"
#include "auterraininfo.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "daworldglobe.h"
#include "dgdata.h"

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaWorld::DaWorld()
{
	m_OrbitZone = OZONE_Inferno;
	m_GenSeed = 0;
	m_InDesigner = 0;
	m_OrbitRadius = 0;
}

DaWorld::~DaWorld()
{
}

DaWorld::DaWorld(const DaWorld& c) : DaAtom(c)
{
	m_OrbitZone = c.m_OrbitZone;
	m_InDesigner = c.m_InDesigner;

	c.m_WorldLock.lockForRead();
	m_GenSeed = c.m_GenSeed;
	m_LastAirTravellerArr = c.m_LastAirTravellerArr;
	m_LastAirTravellerDep = c.m_LastAirTravellerDep;
	m_LastIndigenousArr = c.m_LastIndigenousArr;
	m_LastIndigenousDep = c.m_LastIndigenousDep;
	m_LastNewborn = c.m_LastNewborn;
	m_OrbitChanged = c.m_OrbitChanged;
	m_OrbitRadius = c.m_OrbitRadius;
	m_sName = c.m_sName;
	c.m_WorldLock.unlock();
}

void DaWorld::Create(double orbitRadius, ORBITZONE orbitZone)
{
	DaAtom::Create();

	m_WorldLock.lockForWrite();
	m_OrbitZone = orbitZone;
	m_OrbitRadius = orbitRadius;
	m_WorldLock.unlock();
}

void DaWorld::CreateForScan(double orbitRadius, ORBITZONE orbitZone)
{
	DaAtom::CreateForScan();

	m_WorldLock.lockForWrite();
	m_OrbitZone = orbitZone;
	m_OrbitRadius = orbitRadius;
	m_WorldLock.unlock();
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaWorld::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;
	if(lastTx.IsNull() || OrbitChangeTime() >= lastTx)
		hasWhat |= 0x01;

	toClient << hasWhat;

	m_WorldLock.lockForRead();

	if(hasWhat & 0x01)
	{
		toClient << (quint8)m_OrbitZone;
		toClient << m_OrbitRadius;
	}

	toClient << m_sName;

	m_WorldLock.unlock();
}

// Receive updates to this atom from a server.
void DaWorld::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);

	quint8 hasWhat;
	fromServer >> hasWhat;

	if(hasWhat & 0x01)
	{
		quint8 ui8;
		fromServer >> ui8;
		m_OrbitZone = (ORBITZONE)ui8;
		fromServer >> m_OrbitRadius;
	}

	fromServer >> m_sName;
}

// Refresh data from valid counterpart.
bool DaWorld::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaWorld* pA = (const DaWorld*)pAtom;
	m_OrbitZone = pA->m_OrbitZone;

	pA->m_WorldLock.lockForRead();
	m_WorldLock.lockForWrite();
	m_OrbitRadius = pA->m_OrbitRadius;
	m_sName = pA->m_sName;
	m_WorldLock.unlock();
	pA->m_WorldLock.unlock();

	return false;
}

void DaWorld::CombatApplyDamage(const AuCombatAttacker& /*attacker*/, const AuCombatWeapon& /*weapon*/, const AuPoint& /*wpnLoc*/,
		const AuPoint& /*hitLoc*/, AuCombatResult& /*result*/)
{
	// Don't call base class to prevent hit messages on planets.
}

// This is only called on the scene atom. It should search only atoms that are visible
// in the current scene.
// This default handler simply does a non-recursive search of its sub-atoms.
DaAtom *DaWorld::FindClosestAtom(const QPoint &pxFind, const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
		const AuMatrix4x4 &tModel) const
{
	if(!HasSubatoms())
		return 0;

	DaAtom *pClosest = NULL;
	AuPoint fpxFind(pxFind);
	double rClosest = -1.0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		AuPoint fLoc = tModel.Project(pAtom->Location());
		AuPoint fpxLocOrtho = tDeviceOrtho.Project(fLoc);
		if(fpxLocOrtho.z >= 0.0)
			continue;	// Atom is behind the camera.

		AuPoint fpxLoc = tDevicePersp.Project(fLoc);
		double rdist = fpxLoc.Distance_2D(fpxFind);
		if(rClosest == -1.0 || rdist < rClosest)
		{
			rClosest = rdist;
			pClosest = pAtom;
		}
	}
	return pClosest;
}

bool DaWorld::SolarSystemNameChanged(const QString& oldName, const QString& nuName, bool notifyDb)
{
	bool anyChange = false;

	m_WorldLock.lockForRead();
	QString n(m_sName);
	m_WorldLock.unlock();
	if(n.isEmpty() || n == DefaultName(oldName))
	{
		// I am currently unnamed or my name is the default name. Anything else was probably assigned and we won't change it.
		QString name = DefaultName(nuName);	// My nu default name, based on nu solar system name.
		if(n != name)
		{
			// My name changed as a result of the solar system name change.
			m_WorldLock.lockForWrite();
			m_sName = name;
			m_WorldLock.unlock();
			SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
			anyChange = true;
		}
	}

	// Notify subordinate worlds of this world, e.g. rings, moons.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		if((*it)->SolarSystemNameChanged(oldName, nuName, notifyDb))
			anyChange = true;
	return anyChange;
}

void DaWorld::PrepareForScan()
{
	QString dn(DefaultName(FindParentSolarSystem()->Name()));
	m_WorldLock.lockForWrite();
	m_sName = dn;
	m_WorldLock.unlock();

	// Notify subordinate worlds of this world, e.g. rings, moons.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->PrepareForScan();
}

// This is an approximation of the period based roughly on Keppler's formula:
//	t^2 = r^3 where t is time and r is radius.
// The return result is in earth years.
// The earth standard is considered OrbitRadius=1au
// The moon standard is considered OrbitRadius=28kMiles (.07au)
double DaWorld::Period() const
{
	m_WorldLock.lockForRead();
	double r3 = m_OrbitRadius * m_OrbitRadius * m_OrbitRadius;	// r^3
	m_WorldLock.unlock();
	double rYear = sqrt(r3);
	return rYear;
}

double DaWorld::AngleToPlanet() const
{
	double rUYears;
	return AngleToPlanetAt(g_Au.CoordinatedTime().MSecsSinceEpoch(), rUYears);
}

// This calculates the angle to the planet's current position based on real time.
// The time scale is modified such that four days in real time are equal to an earth 
// year in the model Universe.
// This results in a standard earth day of about 15.77 minutes in real time.
// All stars and planets orbit in the same direction.
double DaWorld::AngleToPlanetAt(qint64 msecsSinceEpoch, double& rUYears) const
{
	double rRealDays = (double)msecsSinceEpoch / (double)MillisecondsPerDay();
	rUYears = rRealDays / RealDaysPerUYear();					// This is how many years have passed in the game Universe.

	// Skew by a constant factor based on this planet's genseed. This way all planets don't start at angle zero.
	double rPeriods = rUYears / Period();						// Number of periods of revolution.
	rPeriods += AuNoise::IntegerNoise(m_OrbitZone, GenSeed());
	//rPeriods *= 100000;	//DHH Super fast

	// Calculate the orbit angle from the fractional period that we're in.
	double rFracPeriods = rPeriods - (int)rPeriods;
	double rAngle = rFracPeriods * PI*2.0;						// Angle to current position of this body.
	DaAtom* parent = Parent();
	if(!parent)
		return rAngle;
	return rAngle - parent->Rotation().z;						// Subtract Z rotation of my parent globe, which is rotating me too.
}

// Returns the distance to the ship in orbit that is closest to loc.
// Also checks for other atoms that block a ship from spawning at 'loc'.
double DaWorld::ClosestShipInOrbit(const AuPoint& loc, const AuId& shipToIgnore, bool avoidCreatures) const
{
	double bestDist = FeetPerParsec();
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaAtom* atom = *it;
		if(atom->Is(shipToIgnore))
			continue;

		if(dynamic_cast<const DaEffect*>(atom))
			continue;	// Don't be blocked by special effects.
		else if(dynamic_cast<const DaItem*>(atom))
			continue;	// Don't be blocked by items.
		else if(dynamic_cast<const DaTeleporter*>(atom))
			continue;	// Don't be blocked by teleporters.
		else if(dynamic_cast<const DaWorld*>(atom))
			continue;	// Don't be blocked by rings or moons. Based on usage of this function, they won't be a factor.

		if(!avoidCreatures && dynamic_cast<const DaCreature*>(atom))
			continue;

		double dist;
		const DaVessel* ship = dynamic_cast<const DaVessel*>(atom);
		if(ship)
		{
			// Calculate distance to ship, considering its rough radius.
			dist = loc.Distance(ship->Location()) - (ship->RoughRadius() + 10*FeetPerMeter());
		}
		else
		{
			// The non-spaceship atoms that we're concerned about are all less than 100' in radius.
			dist = loc.Distance(atom->Location()) - 100;
		}
		if(dist < bestDist)
			bestDist = dist;
	}
	if(bestDist < 0)
		bestDist = 0;
	return bestDist;
}

void DaWorld::ReportEnemySensorContact(const DaSolarSystem* system, quint32 empireId, const AuSensorBlipList& sensorBlips,
		float sensorRange)
{
	if(!HasSubatoms())
		return;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* city = dynamic_cast<DaBd*>(*it);
		if(!city)
			continue;
		if(city->IndustryId() != IID_Capitol && city->IndustryId() != IID_BaseHeadquarters)
			continue;
		if(city->EmpireId() == empireId)
			city->ReportEnemySensorContact(system, sensorBlips, sensorRange);
	}
}

void DaWorld::FindStartupResources(int& eludium, int& lumenite)
{
	// Check moons.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaWorldGlobe* pPlanet = dynamic_cast<DaWorldGlobe *>(pAtom);
		if(pPlanet)
		{
			// Found a moon.
			pPlanet->FindStartupResources(eludium, lumenite);
		}
	}
}

DaWorldGlobe* DaWorld::FindHabitableMoon(HABITABILITY hab) const
{
	if(!HasSubatoms())
		return 0;

	// No moon can be habitable if this parent world is not in the habitable orbit zone.
	if(!IsHabitableOrbit())
		return 0;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldGlobe* pAtom = dynamic_cast<DaWorldGlobe*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsHabitable(hab))
			return pAtom;
	}

	// No habitable moon was found.
	return 0;
}

// Cost of one unit of the item when the transaction occurs near a station around this planet.
quint32 DaWorld::DemandCost(const AuItem& item) const
{
	// Determine planet wide demand for the commodity.
	quint32 stock = 0;		// Stock of equal or higher quality of idCommodity.
	quint32 people = 0;	// The call picks up planet wide population.
	quint32 needCount = 0;	// Count needed by all running processes, both manufacturing and construction.
	quint32 produceCount = 0;	// Count produced by all running mfg processes.
	ProcessDemandTally(AuId(), item, stock, people, needCount, produceCount);
	//qDebug("DaWorld::DemandCost cid %d qual %d qcost %g stock %d pop %d need %d "
			//"prod %d",
			//idCommodity, quality, qualCost, stock, people, needCount, produceCount);

	// The commodity calculates the demand.
	float demand = item.Demand(people, stock, needCount, produceCount);

	// Modify quality cost by demand.
	float cost = item.UnitCost() * demand;

	// Round UP.
	cost = ceilf(cost);
	if(cost < 1)
		cost = 1;

	return (quint32)cost;
}

// True if stance of empire is stance or less toward all cities on the planet.
// True if no cities exist on the planet.
// Used when determining if a citizen has RIGHT_TO_FOUND authority.
bool DaWorld::HasCityStanceOrLess(quint32 empireId, POLITICAL_STANCE stanceOrLess)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		POLITICAL_STANCE stance;
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(bd && (bd->IndustryId() == IID_Capitol || bd->IndustryId() == IID_BaseHeadquarters))
		{
			// A normal capitol or hq was found.
			stance = PoliticalStance(empireId, bd->EmpireId());
		}
		else
			continue;
		if(stance == POL_Empire || stance == POL_Vassal)
			return false;	// Fail on presense of any city that belongs to the empire or my overlord empire.

		switch(stanceOrLess)
		{
		default:
			return false;	// Invalid 'stanceOrLess' value.
		case POL_Friend:
			// Ignore all non-empire cities. We already checked empire cities, which always fail.
			break;
		case POL_Neutral:
			// Fail on friendly city presense. Ignore all neutral and enemy cities.
			if(stance == POL_Friend)
				return false;	// Found a friendly city.
			break;
		case POL_Enemy:
			// Fail on friendly or neutral city presense. Ignore enemy cities.
			if(stance == POL_Friend || stance == POL_Neutral)
				return false;	// Found a friendly or neutral city.
			break;
		}
	}

	// We get here if no cities exist on the planet or if the one's that are present are all equal or lower than stanceOrLess.
	return true;
}

bool DaWorld::CityOrBaseHere() const
{
	bool isHere = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if((*it)->IsDying())
			continue;				// City is about to decay from existence.
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(bd && (bd->IndustryId() == IID_Capitol || bd->IndustryId() == IID_BaseHeadquarters))
		{
			isHere = true;
			break;
		}
	}
	return isHere;
}

// Returns a seed value that can be used for generating things about this world pseudo-randomly.
// This seed value is derived from the seed value of my parent star.
// The seed value is independent of the id of the world atom.
// This is not the seed used when the world was originally Create'd. That sequence is seeded in DaSolarSystem when it creates
// its contents.
quint32 DaWorld::GenSeed() const
{
	m_WorldLock.lockForRead();
	quint32 v = m_GenSeed;
	m_WorldLock.unlock();
	if(!v)
	{
		m_WorldLock.lockForWrite();
		// The gen seed of my parent world or star provides the basis for my gen seed.
		const DaWorld* world = dynamic_cast<const DaWorld*>(Parent());
		if(world)
		{
			m_GenSeed = world->GenSeed();
		}
		else
		{
			const DaAtom* par = Parent();
			if(par)
			{
				const DaStar* star = par->FindParentStarInEnv();
				if(star)
					m_GenSeed = star->GenSeed();
				else
					m_GenSeed = 0;
			}
			else
			{
				qWarning() << "error: DaWorld::GenSeed" << Id() << "null parent" << ParentId();
				m_GenSeed = 0;
			}
		}
		m_GenSeed += (quint32)(FROUND(m_OrbitRadius, .001) * FeetPerAu());
		m_GenSeed += 123456 * OrbitIndex();

		// Use noise to select a GenSeed value for this world.
		do
		{
			double noise = AuNoise::IntegerNoise(m_OrbitZone, m_GenSeed);
			m_GenSeed += (quint32)(noise * 0xffffffff);
		}
		while(!m_GenSeed);

		v = m_GenSeed;
		m_WorldLock.unlock();
	}
	return v;
}

QColor DaWorld::SeaColor() const
{
	int r = 0;
	int g = 0;
	int b = 0;
	switch(OrbitZone())
	{
	case OZONE_Inferno:
		// Seas are molten lava in the inferno zone.
		r = 255;
		g = 25;
		b = 0;
		break;
	case OZONE_Inner:
		// Light blue seas.
		r = .1f * 255.f;
		g = .36f * 255.f;	// More green due to more sunlight.
		b = .42f * 255.f;
		break;
	case OZONE_Habitable:
		r = .1f * 255.f;
		g = .31f * 255.f;
		b = .435f * 255.f;
		break;
	case OZONE_Outer:
		// Darker blue seas.
		r = .1f * 255.f;
		g = .26f * 255.f;	// Less green due to less sunlight.
		b = .45f * 255.f;
		break;
	case OZONE_Frigid:
		// Seas are always frozen in the frigid zones.
		r = 200;
		g = 255;
		b = 255;
		break;
	case OZONE_DeepSpace:
		r = 0;
		g = 0;
		b = 0;
		break;
	}
	return(QColor(r,g,b));
}

float DaWorld::SeaFogDensity() const
{
	if(InDesignerPreview())
		return 0.0005f;
	if(OrbitZone() == OZONE_Inferno)
		return 1;
	else if(OrbitZone() == OZONE_Frigid)
		return .025f;
	return .0125f;
}

void DaWorld::ListTroopsInCity(const AuId& cityId, quint32 empireId, DaAtomList& troopsInCity) const
{
	if(!HasSubatoms())
		return;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		DaNpc* troop = dynamic_cast<DaNpc*>(*ai);
		if(!troop || !troop->IsNpcTroop())
			continue;

		if(troop->InBdCity() != cityId)
			continue;

		if(troop->EmpireId() != empireId)
			continue;

		troopsInCity.append(troop);
	}
}

void DaWorld::ListTroopsAndPeopleOnlineInCity(const AuId& cityId, DaAtomList& troopsAndPeopleInCity) const
{
	if(!HasSubatoms())
		return;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		DaNpc* troop = dynamic_cast<DaNpc*>(*ai);
		if(troop)
		{
			if(!troop->IsNpcTroop())
				continue;
			if(troop->InBdCity() != cityId)
				continue;
			troopsAndPeopleInCity.append(troop);
		}
		else
		{
			DaPerson* person = dynamic_cast<DaPerson*>(*ai);
			if(person)
			{
				if(person->InBdCity() != cityId)
					continue;
				troopsAndPeopleInCity.append(person);
			}
		}
	}
}

// Init location of person who is spawning at a nuly created planet.
// Locate any land at random.
void DaWorld::InitSpawnLocation(DaPerson* pu)
{
	AuPoint loc = RandomAvatarLocation();
	if(loc.IsZero())
	{
		qWarning() << "DaWorld::InitSpawnLocation" << Id() << Name() << "failed to find loc for unit" << pu->Id() << pu->Name();
		return;
	}

	// Determine cg location for person above random land location.
	pu->Stand();
	AuPoint upvec = -GravityDir(loc);
	AuPoint cgloc = loc + upvec * pu->GroundHeightAboveCG();

	// Position their body.
	AuMatrix4x4 tBody;
	tBody.Translate(cgloc);
	tBody *= AuPoint(0,0,1).GetAlignmentTransform(upvec);	// Orient so down is toward the planet.
	pu->ChangePosAndOrient(AuAtomPos(Id(), tBody));
	pu->InvalidateEnvironment();
	pu->SetInMotion();
	pu->TickTock();
}

// Add a visual and/or audio effect to this atom.
// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
void DaWorld::AddEffect(AuEffect& effect)
{
	// Don't add bleeding effects to terrain.
	if(effect.Visual() == VISUAL_HitBludgeon)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Bludgeon, 1);
	}
	else if(effect.Visual() == VISUAL_HitPierce)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Pierce, 1);
	}
	else if(effect.Visual() == VISUAL_HitSlash)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Slash, 1);
	}

	DaAtom::AddEffect(effect);
}

void DaWorld::ReportZoneBioResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
		unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, QMap<QString, AuWorldRes_>& zonelist) const
{
	for(int zri=0; zri<zonerescount; zri++)
	{
		COMMODITYID cid = zoneres[zri];
		QString cname = g_Au.CommodityName(cid);
		quint8 quality = res.Quality(cid);
		quint8 prob = 0;
		if(quality)
			prob = 1;

		// Determine if the natural resource can be acquired here.
		if(rz == 0)
		{
			zonecount.insert(pair<COMMODITYID, int>(cid, prob));
			AuWorldRes_ r;
			r.cid = cid;
			r.qual[0] = quality;
			r.prob[0] = prob;
			zonelist.insert(cname, r);
		}
		else
		{
			zonecount[cid] += prob;
			zonelist[cname].qual[rz] = quality;
			zonelist[cname].prob[rz] = prob;
		}
	}
}

void DaWorld::ReportZoneFetchResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
		unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, QMap<QString, AuWorldRes_>& zonelist) const
{
	COMMODITYID cid;
	QString cname;
	for(int zri=0; zri<zonerescount; zri++)
	{
		cid = zoneres[zri];
		cname = g_Au.CommodityName(cid);

		// Determine if the natural resource can be acquired here.
		int prob = res.FetchProbability(cid);
		if(rz == 0)
		{
			zonecount.insert(pair<COMMODITYID, int>(cid, prob));
			AuWorldRes_ r;
			r.cid = cid;
			r.qual[0] = res.Quality(cid);
			r.prob[0] = prob;
			zonelist.insert(cname, r);
		}
		else
		{
			zonecount[cid] += prob;
			zonelist[cname].qual[rz] = res.Quality(cid);
			zonelist[cname].prob[rz] = prob;
		}
	}
}

// 'errcode' is an output parameter. It will be set to one of the following values:
// 0 = success
// 1 = an unlikely error occurred
// 2 = design not loaded, try again
// 6 = spawn location was blocked
// 27 = design not ok to manufacture
DaVessel* DaWorld::AgentSpawnSpacecraft(DaPerson* person, int designId, quint8& errcode)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
	{
		errcode = 1;
		return 0;
	}

	// Insure the design is allowed to be manufactured.
	// Need scReport to make new style designs.
	AuScReport scReport;

	// Check new style designs.
	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool okToMake = sdd.ScDesignAllowedToManufacture(designId, scReport);		// Signals design load if it's not loaded.
	bool designLoaded = sdd.ScDesignIsLoaded(designId);
	g_Au.UnlockScDesigns();
	if(!okToMake)
	{
		if(designLoaded)
			errcode = 27;
		else
			errcode = 2;
		return 0;
	}

	DaVessel* vessel = 0;
	unsigned int mfgTimeSecs;			// Number of seconds needed to manufacture the design.
	unsigned int designQ = scReport.Quality();

	if(!scReport.CanBeManufactured())
	{
		// The design has no manufacturing procees. Was it not finalized for manufacture?
		errcode = 2;
		return 0;
	}

	// Simulate fetching materials from the city. We aren't committed to doing this until we know if the person can afford it.
	unsigned char iOutputQuality = designQ;
	mfgTimeSecs = scReport.ManufacturingProcess().SecondsEstimated();

	// Determine spawn location relative to the planet.
	AuPoint spawnAt;
	person->GetLocationRelativeToParent(spawnAt, this);		// Use agent's location.
	const double orbitAlt = AtmosphereRadius() + ParkAltAboveAtm();
	double r = scReport.RoughHullRadius();
	spawnAt = FindOrbitSpawnLoc(spawnAt, AuId(0), r, orbitAlt, true);

	// Position the atom in the atmosphere.
	AuMatrix4x4 t;
	t.Translate(spawnAt);

	// Align with up.
	AuPoint vup = -GravityDir(spawnAt);
	t *= AuPoint(0,0,1).GetAlignmentTransform(vup);

	// Create a spacecraft.
	DaSc* ship = new DaSc;
	if(!ship)
	{
		errcode = 1;	// not likely
		return 0;
	}
	ship->ConnectSignals();
	ship->Create();
	ship->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(ship);

	ship->SetDesign(scReport);

	// Wait with AtomAdded signal until we've tested the location.
	// If the ship can't spawn, we can delete it immediately and there will
	// not be a record of it in the database.

	// Mark the solar system as explored by the empire owning the ship.
	system->ExploredBy(person->EmpireId());

	// Test the location of the ship to see if it collides with anything.
	// Not likely since we insured there were no other ships nearby.
	// We'll do this by trying to move up a little.
	vup.Unit(10);
	if(!MoveSubatom(ship, vup, 0))
	{
		// The ship cannot move to that location.
		// It must be blocked in place.
		TrashSubatom(ship->Id());
		errcode = 6;
		return 0;
	}

	// The spacecraft spawned fine. It can stay where it is.
	g_Au.AtomAdded(ship->Id());

	// Start up ship systems.
	ship->InitSystems(iOutputQuality);
	ship->RechargeCapacitor();
	ship->SetEmpireId(person->EmpireId());
	ship->SetGravityDriveState(DaVessel::GD_Hold);
	ship->SetModified();
	if(ship->IsNew())
		ship->SetNotNew();										// Clear 'nu' state if it is still set.

	vessel = ship;

	// Place the person aboard the nu ship, near the helm.
	AuMatrix4x4 tPerson;
	tPerson.Translate(vessel->HelmLocation() + AuPoint(1,-1,person->CGHeight()));
	person->ForcePosStop(AuAtomPos(vessel->Id(), tPerson));
	person->Stand();
	person->SyncController(AuSyncPose(AuPose::Stand, 0, 0, 0));
	person->SetModified();

	// Develop a description of where this ship was made.
	// e.g. Imperium city Bay of Magellan on Hazeron Alpha V in Hazeron, Hazeron Sector
	const DaSector* sector = FindParentSector();
	QString whereMade;
	if(system && sector)
		whereMade = tr("%1 in %2, %3").arg(Name()).arg(system->Name()).arg(sector->Name());
	vessel->SetWhereMade(whereMade);

	// Send an intercom message to the person.
	if(sector)
	{
		QString text = tr("%1 spawned by agent %2. "
				"Company purchase wait time would have been %L3 seconds. "
				"Refuel soon; the capacitor is charged but the fuel cells may be empty.")
			.arg(vessel->Description())
			.arg(person->Name())
			.arg(mfgTimeSecs);

		AuCommChat msg(text, AuCommStdChannel::IntercomChannelName(), vessel->Id(), vessel->Name(),
				sector->Id(), sector->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
		g_Au.SendAtomChat(msg);
	}

	// Return the shiny nu vessel atom.
	errcode = 0;	// success
	vessel->SetNormal();	// New atom is ready for normal thread processing.
	vessel->TickTock();
	return vessel;
}

DaAtom* DaWorld::FindGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	if(!HasSubatoms())
		return 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	DaAtom* target = 0;
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
		if((*ai)->IsGuardTowerTarget(gunnerEmpire, gunnerPosture, gunLoc, gunRange, targetLoc))
		{
			target = *ai;
			gunRange = gunLoc.Distance(targetLoc);	// Gun range is reduced to keep looking for a closer target.
		}
	return target;
}

bool DaWorld::RoughExtents(AuBox& fExt) const
{
	Q_UNUSED(fExt);
	return false;
}

void DaWorld::ReportZoneBioResources(const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
		QMultiMap<quint8, COMMODITYID>& scanRes) const
{
	for(int zri=0; zri<zonerescount; zri++)
	{
		COMMODITYID cid = zoneres[zri];
		quint8 quality = res.Quality(cid);
		if(quality)
			scanRes.insert(quality, cid);
	}
}

void DaWorld::ReportZoneFetchResources(const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
		QMultiMap<quint8, COMMODITYID>& scanRes) const
{
	for(int zri=0; zri<zonerescount; zri++)
	{
		COMMODITYID cid = zoneres[zri];
		int prob = res.FetchProbability(cid);
		if(prob)
			scanRes.insert(res.Quality(cid), cid);
	}
}

void DaWorld::ListMoons(QMap<double, double>& moons) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaWorld* world = dynamic_cast<const DaWorld*>(*it);
		if(!world)
			continue;
		moons.insert(world->OrbitRadius(), world->AtmosphereRadius());
	}
}

//static
QColor DaWorld::AtmosphereColorStatic(unsigned char byAtmType, unsigned char byAtmDensity, quint32 genSeed)
{
	int r,g,b;
	if(byAtmDensity > 0)
	{
		// Purturb the colors a bit just for fun.
		// Seed the random number generator for consistent values for the planet.
		AuDice dice;
		dice.Seed(genSeed);

		switch(byAtmType)
		{
		default:
		case ATMTYPE_Standard:
			r = 255-30;
			g = 255-30;
			b = 255-30;
			r += dice.Roll(1,61,-31);	// +/-30. This gets clamped at the end.
			g += dice.Roll(1,61,-31);	// +/-30. This gets clamped at the end.
			b += dice.Roll(1,61,-31);	// +/-30. This gets clamped at the end.
			break;
		case ATMTYPE_Tainted:
			r = 255-50;
			g = 255-50;
			b = 255-50;
			r += dice.Roll(1,101,-51);	// +/-50. This gets clamped at the end.
			g += dice.Roll(1,101,-51);	// +/-50. This gets clamped at the end.
			b += dice.Roll(1,101,-51);	// +/-50. This gets clamped at the end.
			break;
		case ATMTYPE_Exotic:
			r = 255-70;
			g = 255-70;
			b = 255-70;
			r += dice.Roll(1,141,-71);	// +/-70. This gets clamped at the end.
			g += dice.Roll(1,141,-71);	// +/-70. This gets clamped at the end.
			b += dice.Roll(1,141,-71);	// +/-70. This gets clamped at the end.
			break;
		case ATMTYPE_Corrosive:
			r = 255-90;
			g = 255-90;
			b = 255-90;
			r += dice.Roll(1,181,-91);	// +/-90. This gets clamped at the end.
			g += dice.Roll(1,181,-91);	// +/-90. This gets clamped at the end.
			b += dice.Roll(1,181,-91);	// +/-90. This gets clamped at the end.
			break;
		case ATMTYPE_Insidious:
			r = 255-110;
			g = 255-110;
			b = 255-110;
			r += dice.Roll(1,221,-111);	// +/-110. This gets clamped at the end.
			g += dice.Roll(1,221,-111);	// +/-110. This gets clamped at the end.
			b += dice.Roll(1,221,-111);	// +/-110. This gets clamped at the end.
			break;
		}
	}
	else
	{
		// Zero atmosphere density. What is this color used for?
		r = 160;
		g = 80;
		b = 0;
		//qDebug("DaWorld::AtmosphereColorStatic atmType %d atmDensity %d id %d", byAtmType, byAtmDensity, id);
	}

	if(r < 0)
		r = 0;
	else if(r > 255)
		r = 255;

	if(g < 0)
		g = 0;
	else if(g > 255)
		g = 255;

	if(b < 0)
		b = 0;
	else if(b > 255)
		b = 255;

	return QColor(r,g,b);
}

// Returns true if 'person' is in this world's chain of command.
bool DaWorld::InChainOfCommand(const AuId& personId) const
{
	quint32 empireId = EmpireId();		// Id of empire who currently has a claim to this world.
	if(!empireId)
		return true;					// Everyone is in my chain of command if nobody has staked a claim, to delete roads/craters.
	AuIdList placeChain;
	ChainOfPlaces(placeChain);			// This list includes me.
	const DgData& gd = g_Au.LockDataForRead();
	bool inDomain = gd.IsEmpirePlaceInDomain(empireId, placeChain, personId);
	g_Au.UnlockData();
	return inDomain;
}

AuSpecies DaWorld::TakeIndigenous(qint8 morale, bool hasHome, bool hasResident, int& result)
{
	// The caller can give up a resident.
	result = 0;
	int arrivalMins = IndigenousPopulationInterval(morale);	// Returns 0 if no indigenous
	if(!arrivalMins)
		return AuSpecies();
	else if(arrivalMins < 0)
	{
		// Departing
		if(!hasResident)
			return AuSpecies();		// The caller has no resident to depart.
		m_WorldLock.lockForWrite();
		if(m_LastIndigenousDep.ElapsedMin() < -arrivalMins)
		{
			m_WorldLock.unlock();
			return AuSpecies();
		}
		m_LastIndigenousDep.restart();
		m_WorldLock.unlock();
		result = -1;
		return WorkerSpecies();
	}
	else
	{
		// Arriving
		if(!hasHome)
			return AuSpecies();		// The caller has no home for the arriving resident.
		m_WorldLock.lockForWrite();
		if(m_LastIndigenousArr.ElapsedMin() < arrivalMins)
		{
			m_WorldLock.unlock();
			return AuSpecies();
		}
		m_LastIndigenousArr.restart();
		m_WorldLock.unlock();
		result = 1;
		return WorkerSpecies();
	}
}

int DaWorld::TakeAirTraveller(qint8 morale, bool hasHome, bool hasResident)
{
	int arrivalMins = AirTravellerInterval(morale);
	if(!arrivalMins)
		return 0;
	else if(arrivalMins < 0)
	{
		// Departing
		if(!hasResident)
			return 0;		// The caller has no resident to depart.
		m_WorldLock.lockForWrite();
		if(m_LastAirTravellerDep.ElapsedMin() < -arrivalMins)
		{
			m_WorldLock.unlock();
			return 0;
		}
		m_LastAirTravellerDep.restart();
		m_WorldLock.unlock();
		return -1;
	}
	else
	{
		// Arriving
		if(!hasHome)
			return 0;		// The caller has no home for the arriving resident.
		m_WorldLock.lockForWrite();
		if(m_LastAirTravellerArr.ElapsedMin() < abs(arrivalMins))
		{
			m_WorldLock.unlock();
			return 0;
		}
		m_LastAirTravellerArr.restart();
		m_WorldLock.unlock();
		return 1;
	}
}

bool DaWorld::TakeNewborn(qint8 morale)
{
	unsigned int arrivalMins = NewbornInterval(morale);
	if(!arrivalMins)
		return false;
	m_WorldLock.lockForWrite();
	if(m_LastNewborn.ElapsedMin() < arrivalMins)
	{
		m_WorldLock.unlock();
		return false;
	}
	m_LastNewborn.restart();
	m_WorldLock.unlock();
	return true;
}

bool DaWorld::HasAlienMilitaryBuilding(quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->EmpireId() == empireId)
			continue;				// Not alien.
		if(!bd->IsMilitary())
			continue;				// Not a military building.
		return true;
	}
	return false;
}

bool DaWorld::HasAlienMilitaryHQ(quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->EmpireId() == empireId)
			continue;				// Not alien.
		if(bd->IndustryId() != IID_BaseHeadquarters)
			continue;				// Not a military headquarters.
		return true;
	}
	return false;
}

bool DaWorld::HasAnyMilitaryHQ() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->IndustryId() != IID_BaseHeadquarters)
			continue;				// Not a military headquarters.
		return true;
	}
	return false;
}

bool DaWorld::HasAnyMilitaryNotHQ() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->IndustryId() == IID_BaseHeadquarters)
			continue;				// Skip military headquarters.
		if(!bd->IsMilitary())
			continue;				// Skip non-military building.
		return true;
	}
	return false;
}

bool DaWorld::HasAnyCapitol() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->IndustryId() != IID_Capitol)
			continue;				// Not a capitol building.
		return true;
	}
	return false;
}

bool DaWorld::HasAlienCapitol(quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->EmpireId() == empireId)
			continue;				// Not alien.
		if(bd->IndustryId() != IID_Capitol)
			continue;				// Not a capitol building.
		return true;
	}
	return false;
}

bool DaWorld::HasCityPolice(const AuId& capitolId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->IndustryId() != IID_PoliceDepartment)
			continue;				// Not a police department.
		if(bd->CapitolId() != capitolId)
			continue;				// Not of the city.
		return true;
	}
	return false;
}

bool DaWorld::HasEmpireMilitaryHQ(quint32 empireId, bool underConstructionOk) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->EmpireId() != empireId)
			continue;				// Not empire.
		if(bd->IndustryId() != IID_BaseHeadquarters)
			continue;				// Not a military headquarters.
		if(!underConstructionOk && bd->IsUnderConstruction())
			continue;				// Caller does not want an HQ that is under construction.
		return true;
	}
	return false;
}

AuId DaWorld::JurisdictionAt(const AuPoint& loc, quint32& cityEmpireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(!bd->HasJurisdictionAt(*this, loc))
			continue;				// Not in the building's jurisdiction.
		AuId capitolId = bd->CapitolId();
		cityEmpireId = bd->EmpireId();
		return capitolId;
	}
	return AuId(0);
}

bool DaWorld::InAnyJurisdiction(const AuPoint& loc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->CapitolId().IsNull())
			continue;				// In the one case where this function is called, caller wants to ignore lost police.
		if(!bd->HasJurisdictionAt(*this, loc))
			continue;				// Not in the building's jurisdiction.
		return true;
	}
	return false;
}

bool DaWorld::InAlienJurisdiction(const AuPoint& loc, quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->EmpireId() == empireId)
			continue;				// Not alien.
		if(!bd->HasJurisdictionAt(*this, loc))
			continue;				// Not in the building's jurisdiction.
		return true;
	}
	return false;
}

bool DaWorld::InEmpireJurisdiction(const AuPoint& loc, quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->EmpireId() != empireId)
			continue;				// Not empire.
		if(!bd->HasJurisdictionAt(*this, loc))
			continue;				// Not in the building's jurisdiction.
		return true;
	}
	return false;
}

AuId DaWorld::CapitolInProposedJurisdiction(const AuPoint& proposedBdLoc, double jr, quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		if(bd->EmpireId() != empireId)
			continue;				// Not empire.
		if(!bd->IsCapitolInJurisdictionAt(*this, proposedBdLoc, jr))
			continue;				// Not in the building's jurisdiction.
		return bd->Id();
	}
	return AuId();
}

bool DaWorld::WouldCreateOverlappingJurisdictions(const AuPoint& proposedBdLoc, double jr, bool isCapitol) const
{
	QSet<AuId> jurisdictions;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;				// Not a building.
		// This test matches the one in DaWorldLL::JurisdictionPoliceJoin.
		if(bd->IsCapitolInJurisdictionAt(*this, proposedBdLoc, jr))
			jurisdictions.insert(bd->CapitolId());			// Proposed loc is in the jurisdiction of an existing capitol.
		else if(bd->HasJurisdictionAt(*this, proposedBdLoc))
			jurisdictions.insert(bd->CapitolId());			// Proposed loc is in the jurisdiction of a city.
		else if(!bd->CapitolId().IsNull() && bd->InProposedJurisdiction(*this, proposedBdLoc, jr))
			jurisdictions.insert(bd->CapitolId());			// Existing city building is in the proposed jurisdiction.
		else
		{
			if(isCapitol)
			{
				if(bd->HasJurisdictionOverlapAt(*this, proposedBdLoc, jr))
					return true;
			}
			else
			{
				if(bd->HasJurisdictionOverlapAt(*this, proposedBdLoc, jr))
					jurisdictions.insert(bd->CapitolId());	// Proposed loc overlaps the jurisdiction of a capitol or police.
			}
		}

		if(jurisdictions.size() > 1)
			return true;
	}
	return false;	// good return, no overlap
}

quint32 DaWorld::CalcWorldPopulation() const
{
	quint32 pop = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		pop += (*it)->Population();
	return pop;
}

bool DaWorld::InDesignerPreview() const
{
	if(!m_InDesigner)
		m_InDesigner = FindParentDesignerInEnv() ? 2 : 1;
	return m_InDesigner == 2;
}

quint64 DaWorld::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaWorld) - sizeof(DaAtom);
	mem += StringMemoryUsage(m_sName);
	return mem;
}

unsigned int DaWorld::NewbornInterval(qint8 morale) const
{
	quint32 pop = WorldPopulation();
	if(pop < 2)
		return 0;
	// Population does not increase when overpopulated.
	if(pop >= WorldPopulationMax())
		return 0;
	// The population is 50% males and 50% females. They are all adults and ready to reproduce. Every couple makes one baby in
	// every gestation cycle.
	//quint32 gdays = 9;								// Human gestation is 9mo. At 28 days per game day, that is about 9 days.
	quint32 gdays = 1;
	quint32 gestationMins = gdays * (24*60);			// Gestation time in minutes.
	int pairs = pop/2;									// Maximum pairs of breeding adults if everybody is happy.
	if(morale <= -10)
	{
		// Extremely low morale destroys reproduction.
		pairs = 0;
	}
	else if(morale < 10)
	{
		// Morale is in the range of -10 < morale < 10.
		// Low morale reduces the number of breeders. 
		float perc = (morale + 10.f) / 20.f;			// Percentage number from 0 < perc < 1.
		pairs = (int)ceil(perc * pairs);
	}
	if(pairs <= 0)
		return 0;
	quint32 babyMins = gestationMins / pairs;			// Minutes between babies if distributed evenly.
	if(!babyMins)
		return 1;										// One baby per second is as fast as we go; world pop is 1,555,200 then.
	return babyMins;
}

quint32 DaWorld::BuildingCount() const
{
	quint32 bv = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		// Street lights and fences don't count against world build count limit.
		if(bd->Structure() != BD_StreetLight && bd->Structure() != BD_Fence)
			bv++;
	}
	return bv;
}

quint32 DaWorld::StreetLightCount() const
{
	quint32 bv = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(bd->Structure() == BD_StreetLight)
			bv++;
	}
	return bv;
}

quint32 DaWorld::FenceCount() const
{
	quint32 bv = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		if(bd->Structure() == BD_Fence)
			bv++;
	}
	return bv;
}

quint32 DaWorld::BuildingVolume() const
{
	quint32 bv = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*it);
		if(!bd)
			continue;
		quint32 obv = bv;
		bv += bd->HullVolume();
		if(bv < obv)
		{
			// bv rolled over
			bv = 0xffffffff;
			break;
		}
	}
	return bv;
}

quint32 DaWorld::BuildingVolumeLimit() const
{
	quint32 maxPop = WorldPopulationMax();
	return AuCityReport::WorldBuildingLimit(maxPop, false);
}

bool DaWorld::CityHereNamed(const QString& cityName) const
{
	bool isHere = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if((*it)->IsDying())
			continue;				// Bd is about to decay from existence.
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(bd)
		{
			if(bd->IndustryId() == IID_Capitol && bd->CityName() == cityName)
			{
				isHere = true;
				break;
			}
		}
	}
	return isHere;
}

bool DaWorld::HQHereNamed(const QString& cityName) const
{
	bool isHere = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if((*it)->IsDying())
			continue;				// Bd is about to decay from existence.
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(bd)
		{
			if(bd->IndustryId() == IID_BaseHeadquarters && bd->CityName() == cityName)
			{
				isHere = true;
				break;
			}
		}
	}
	return isHere;
}

//static
float DaWorld::AirDensityToBar(float airDensity)
{
	// .15 to .85 air density is breathable.
	// Humans can survive .47 to 2.5 bar without special gases|prep.
	// At sea level, Earth atmosphere is 1.01325 bars.
	return airDensity * airDensity * 4.053;
}

// We consider Earth to be about 50% atmosphere density.
// Earth standard atmospheric pressure at sea leval is one bar.
float DaWorld::SeaLevelPressure() const
{
	return AirDensityToBar(MSLAtmosphereDensity());
}

DaWorld* DaWorld::FindParentPlanetInEnv()
{
	return this;
}

const DaWorld* DaWorld::FindParentPlanetInEnv() const
{
	return this;
}

void DaWorld::FindPlanetsRecursive(DaAtomList& planets) const
{
	//if(!IsNormal())
		//return;
	planets.append((DaWorld*)this);	// Cast away constness.
	DaAtom::FindPlanetsRecursive(planets);
}

bool DaWorld::FindNaturalResource(COMMODITYID resCID, quint8 minQ) const
{
	int zones = ResourceZones();
	for(int rz=0; rz<zones; rz++)
	{
		AuZoneResource res;
		QueryNaturalResources(res, rz);
		if(res.Quality(resCID) >= minQ && res.FetchProbability(resCID) > 0)
			return true;
	}
	return false;
}

void DaWorld::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << (quint8)m_OrbitZone;
	out << m_InDesigner;

	m_WorldLock.lockForRead();
	out << m_GenSeed;
	AuTime lastAirTravellerArr;
	out << lastAirTravellerArr;
	AuTime lastAirTravellerDep;
	out << lastAirTravellerDep;
	AuTime lastIndigenousArr;
	out << lastIndigenousArr;
	AuTime lastIndigenousDep;
	out << lastIndigenousDep;
	AuTime lastNewborn;
	out << lastNewborn;
	out << m_OrbitRadius;
	out << AuTime();	//m_OrbitChanged;
	out << m_sName;
	m_WorldLock.unlock();
}

void DaWorld::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	quint8 ui8;
	in >> ui8;
	m_OrbitZone = (ORBITZONE)ui8;
	in >> m_InDesigner;

	m_WorldLock.lockForWrite();
	in >> m_GenSeed;
	AuTime lastAirTravellerArr;
	in >> lastAirTravellerArr;
	AuTime lastAirTravellerDep;
	in >> lastAirTravellerDep;
	AuTime lastIndigenousArr;
	in >> lastIndigenousArr;
	AuTime lastIndigenousDep;
	in >> lastIndigenousDep;
	AuTime lastNewborn;
	in >> lastNewborn;
	in >> m_OrbitRadius;
	AuTime _OrbitChanged;
	in >> _OrbitChanged;
	in >> m_sName;
	m_WorldLock.unlock();
}

QString DaWorld::WorkerSpeciesDesc() const
{
	AuSpecies species = WorkerSpecies();
	if(species.IsNull())
		return "";
	return species.Description();
}

QString DaWorld::WorkerSpeciesName() const
{
	AuSpecies species = WorkerSpecies();
	if(species.IsNull())
		return "";
	return species.NameOrDesc();
}

bool DaWorld::IsAlienSpecies(const AuSpecies& species) const
{
	AuSpecies wspecies = WorkerSpecies();
	if(wspecies.IsNull())
		return false;	// Not alien if not indigenous.
	return !wspecies.IsSexuallyCompatible(species);
}

void DaWorld::QueryTerrainAtLoc(AuTerrainInfo& ter, const AuPoint& fSurfLoc) const
{
	Q_UNUSED(fSurfLoc);
	ter = AuTerrainInfo();
}

QString DaWorld::CatalogName(const DaSolarSystem* system) const
{
	return DefaultName(system->CatalogName());
}

