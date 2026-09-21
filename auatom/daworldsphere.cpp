//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldsphere.cpp,v 1.51 2022/03/10 16:12:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daworldsphere.h"
//
#include "auglobal.h"
#include "aunoise.h"
#include "auParticleSystem.h"
#include "auscdesign.h"
#include "ausphere.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacitizen.h"
#include "dacreature.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "dgdata.h"
#include "dprock.h"

DaWorldSphere::DaWorldSphere()
{
	m_AxialTilt = 0;
	m_RotationAngleOffset = 0;
}

DaWorldSphere::~DaWorldSphere()
{
}

DaWorldSphere::DaWorldSphere(const DaWorldSphere& c) : DaWorldLL(c)
{
	m_AxialTilt = c.m_AxialTilt;
	m_RotationAngleOffset = c.m_RotationAngleOffset;
}

void DaWorldSphere::ActivateHengeContaining(const AuPoint& loc)
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
	quint8 shadowPanels = 0;

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
				shadowPanels = henge->DestShadowPanels();
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
		quint8 fromArcSegment = 0;

		emit HengeActivatedSignal(fromSysId, fromSun, shadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
	}
}

// Checks the area of a mine or well to see if it contains any rocks that can produce 'idRes'.
bool DaWorldSphere::TestRockResource(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
		COMMODITYID idRes) const
{
	quint32 worldGenSeed = GenSeed();
	AuPoint vx = mBld.ProjectVector(AuPoint(1,0));
	AuPoint vy = mBld.ProjectVector(AuPoint(0,1));

	// The rest of this was cobbled from CreateRocks
	const double radius = GlobeRadius();
	int eqDivs = (int)(radius*2*PI);							// Circumference of globe in whole feet.
	eqDivs /= 20;												// 20' spacing for rocks at highest density.

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
			TerrainAltitudeMSL(rockLL, hideRocks, hideGrass, hideShrubs, hideTrees, pavementType, farmShrub, orchardTree);
			if(hideRocks)
				continue;
				*/

			return true;				// Stop as soon as we find the rock we are looking for.
		}
	}
	return false;
}

AuGlobeLL DaWorldSphere::RoundedGlobeLoc(const AuGlobeLL& locLL, int eqdivs) const
{
	AuGlobeLL rndLL;											// This will be our return value.

	// Determine closest latitude.
	int latdivshemi = eqdivs/4;									// Latitude divisions per hemisphere.
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

AuGlobeLLF DaWorldSphere::RoundedGlobeLoc(const AuGlobeLLF& locLL, int eqdivs) const
{
	AuGlobeLLF rndLL;											// This will be our return value.

	// Determine closest latitude.
	int latdivshemi = eqdivs/4;									// Latitude divisions per hemisphere.
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

AuGlobeLLF DaWorldSphere::RandomLL(const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	float lon = dice.Roll(1,361,-181) * DEG2RADf;				// Linear longitude probability.
	float lat = dice.Roll(2,91,-92) * DEG2RADf;					// Bell curve accounts for polar pinching.
	return AuGlobeLLF(lon, lat);
}

// Returns the altitude of the surface at ll relative to MSL.
// Checks for permanent terrain-like features such as the control center dais on a ringworld.
double DaWorldSphere::SurfaceAltitudeMSL(const AuGlobeLL& ll) const
{
	return TerrainAltitudeMSL(ll);
}

// Spawns spacecraft in orbit.
// Returns:
//   0 = ok
//   1 = blocked by unanalyzed ship
//   2 = Cannot spawn another station in orbit or replace an existing station.
//   3 = Cannot spawn another nu ship in orbit or replace an existing nu ship.
//   4 = Orbit spawn location could not be found.
//   5 = No station at which to spawn.
int DaWorldSphere::SpawnSc(const AuScReport& shipDesign, quint32 bdEmpireId, quint8 quality,
		const AuId& ownerId, const QString& ownerName, quint32 ownerEmpire, const AuId& cityOrBdId)
{
	AuEther* ether = Ether();
	if(!ether)
		return 4;
	//qDebug("DaWorldSphere::SpawnSpacecraft %d %s design %d %s", Id().GetID(), (const char*)Name().toAscii(),
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
			//qDebug("DaWorldSphere::SpawnSpacecraft %d %s cannot spawn design %d in orbit. "
			//"Ship %d %s design %d already in orbit has invalid design cache.",
			//Id().GetID(), (const char*)Name().toAscii(), shipDesign.DesignId(),
			//ship->Id().GetID(), (const char*)ship->Name().toAscii(), ship->DesignId());
			//qDebug("DaWorldSphere::SpawnSpacecraft %d %s not yet", Id().GetID(), (const char*)Name().toAscii());
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
		if(anyStation)
		{
			// When there is any other station, spawn on the opposite side of the planet.
			spawnAt = -anyStation->Location();
		}
		else
		{
			// No other station so spawn right above the city since orbits are geosyncronous.
			if(builder)
				spawnAt = builder->Location();	// FindOrbitSpawnLoc calculates a location at the proper altitude.
			else
			{
				// Calculate random rotation around planet.
				float rot = g_Au.Dice().Roll(1,360,-1);
				rot *= DEG2RADf;
				spawnAt = AuPoint(orbitAlt,0,0).RotateZ(rot);
			}
		}
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
		//qDebug("DaWorldSphere::SpawnSpacecraft %d %s ok station", Id().GetID(), (const char*)Name().toAscii());

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

	//qDebug("DaWorldSphere::SpawnSpacecraft %d %s station needed", Id().GetID(), (const char*)Name().toAscii());
	return 5;
}

// NOTE: This code is identical to DaStar::CalcAngleOfRotation.
// This calculates the angle around this planets axis based on real time.
// The time scale is modified such that four days in real time are equal to an earth 
// year in the universe.
// This results in a standard earth day of about 15.77 minutes in real time.
// All planets rotate at the same rate. Mass is not used in the calculation.
double DaWorldSphere::CalcAngleOfRotation() const
{
	return CalcAngleOfRotation(g_Au.CoordinatedTime().MSecsSinceEpoch());
}

double DaWorldSphere::CalcAngleOfRotation(qint64 msecsSinceEpoch) const
{
	double rRealDays = (double)msecsSinceEpoch / (double)MillisecondsPerDay();
	double rUYears = rRealDays / RealDaysPerUYear();			// This is how many years have passed in the game universe.
	double rUDays = rUYears * DaysPerYear();					// How many days have passed.

	// Modify the number of days by the rotation speed factor.
	rUDays *= RotationSpeedFactor();

	//rUDays *= 180;	// Really fast day/night cycles for debugging.

	// Calculate the orbit angle from the fractional day that we're in.
	double rFracDays = rUDays - (int)rUDays;
	double rAngle = rFracDays * PI*2.0 * DirectionOfRotation();	// Current rotation angle of this body.
	rAngle += RotationAngleOffset();	// In design environment preview, day angle can be offset to adjust lighting.
	return rAngle;
}

// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
// 'sightedLoc' and 'sightedPart' are not changed if false is returned.
bool DaWorldSphere::SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
		unsigned int& sightedPart, bool& sightedShield) const
{
	// Test against a sphere with my atmosphere radius.
	//  -1 = no intersect, segment is inside sphere
	//   0 = no intersect, segment is outside sphere
	//   1 = 1 intersection, result in 'intersect0'
	//   2 = 2 intersections, results in 'intersect0' and 'intersect1' where 'intersect0' is the closest intersection to 'seg0'.
	double atmoRadius = AtmosphereRadius();
	AuSphere sphere(tModel.Project(AuPoint(0,0)), atmoRadius);
	AuPoint hit1, hit2;
	int intResult = sphere.Intersect(eyeLoc, sightedLoc, hit1, hit2);
	if(!intResult)
		return false;											// Segment is entirely outside my atmosphere sphere.

	// At least a portion of the test segment enters my atmosphere.
	AuPoint seg0, seg1;											// End points of the segment that enters my atmosphere.
	if(intResult == 2)
	{
		// Two intersections with the atmosphere.
		seg0 = hit1;
		seg1 = hit2;
	}
	else if(intResult == 1)
	{
		// One intersection with the atmosphere.
		if(eyeLoc.Distance2(sphere.Center()) < atmoRadius*atmoRadius)
		{
			// The eyeLoc is inside the atmosphere so we will check the first portion of the segment for intersections.
			seg0 = eyeLoc;
			seg1 = hit1;
		}
		else
		{
			// The eyeLoc is outside the atmosphere so we will check the last portion of the segment for intersections.
			seg0 = hit1;
			seg1 = sightedLoc;
		}
	}
	else
	{
		seg0 = eyeLoc;
		seg1 = sightedLoc;
	}

	// It's not a hit if the test segment is null.
	if(seg0.FuzzyEquals(seg1))
		return false;

	// Project the sight segment into the space of this atom.
	seg0 = tModel.UnProject(seg0);
	seg1 = tModel.UnProject(seg1);
	AuPoint segVec = seg1 - seg0;
	double segLength = segVec.Length();
	segVec /= segLength;				// Unit vector.

	// Integrate the sight segment in coarse 5' increments for speed, looking for the segment to go under ground.
	double radius = SeaLevelRadius();
	AuPoint prevTestLoc = seg0;
	bool ug = false;					// True if the segment goes under ground.
	AuPoint ugLoc;						// Underground location, if we find one.
	for(double testDist = 0; testDist <= segLength; testDist += 6)
	{
		AuPoint testLoc = seg0 + segVec*testDist;
		AuGlobeLL testLL;
		LLOfLoc(testLL, testLoc);
		double testAlt = radius + TerrainAltitudeMSL(testLL);
		double testAlt2 = testAlt*testAlt;
		double alt2 = testLoc.Length2();
		if(alt2 == testAlt2)
		{
			// testLoc is exactly at the terrain altitude.
			sightedLoc = tModel.Project(testLoc);
			sightedPart = 0xffff;								// Dirt was hit
			sightedShield = false;
			return true;
		}
		else if(alt2 < testAlt2)
		{
			// testLoc is below the terrain.
			ug = true;
			ugLoc = testLoc;
			break;
		}
		prevTestLoc = testLoc;
	}

	if(ug)
	{
		// The coarse segment test went under ground.
		// Integrate the final segment in finer increments, looking for terrain intersections. The first point we find under ground
		// is the terrain hit location we will return.
		segLength = prevTestLoc.Distance(ugLoc) + .5;
		for(double testDist = .5; testDist <= segLength; testDist += .5)	// We already know testDist 0 is not under ground.
		{
			AuPoint testLoc = prevTestLoc + segVec*testDist;
			AuGlobeLL testLL;
			LLOfLoc(testLL, testLoc);
			double testAlt = radius + TerrainAltitudeMSL(testLL);
			if(testLoc.Length2() <= testAlt*testAlt)
			{
				// testLoc is at or below the terrain.
				testLoc.Unit(testAlt);								// Move testLoc to be exactly at the terrain surface.
				sightedLoc = tModel.Project(testLoc);
				sightedPart = 0xffff;								// Dirt was hit
				sightedShield = false;
				return true;
			}
		}
	}

	// No point along the test segment was beneath the ground.
	return false;
}

quint64 DaWorldSphere::MemoryUsage() const
{
	quint64 mem = DaWorldLL::MemoryUsage();
	mem += sizeof(m_SphereLock);
	mem += sizeof(m_AxialTilt);
	mem += sizeof(m_RotationAngleOffset);
	return mem;
}

quint32 DaWorldSphere::WorldPopulationMax() const
{
	double r = GlobeRadius() / FeetPerMeter();					// Radius of the globe in meters.
	double area = 4 * PI * r * r;								// Area of the globe in square meters.
	double maxPop = area / 1000;   								// Maximum population, at 1000 square meters per person.
	return (quint32)round(maxPop);								// Rounded to whole people.
}

DaWorldRing* DaWorldSphere::FindRing(const AuPoint& searchLoc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldRing* ring = dynamic_cast<DaWorldRing*>(*it);
		if(!ring)
			continue;
		if(!ring->IsLocInRing(searchLoc))
			continue;
		return ring;
	}
	return 0;
}

bool DaWorldSphere::SightSeaSurface(const AuPoint& loc1, const AuMatrix4x4& model, AuPoint& hitLoc) const
{
	// Test against a sphere with my sea level radius.
	//  -1 = no intersect, segment is inside sphere
	//   0 = no intersect, segment is outside sphere
	//   1 = 1 intersection, result in 'intersect0'
	//   2 = 2 intersections, results in 'intersect0' and 'intersect1' where 'intersect0' is the closest intersection to 'seg0'.
	AuPoint eyeVec(hitLoc - loc1);
	eyeVec.Unit(AtmosphereRadius());
	AuPoint sightLoc(loc1 + eyeVec);
	double mslRadius = SeaLevelRadius();
	AuSphere sphere(model.Project(AuPoint(0,0)), mslRadius);
	AuPoint hit1, hit2;
	int intResult = sphere.Intersect(loc1, sightLoc, hit1, hit2);
	if(!intResult)
		return false;											// Segment is entirely outside my sea sphere.
	hitLoc = hit1;
	return true;
}

void DaWorldSphere::operator>>(QDataStream& out) const
{
	DaWorldLL::operator>>(out);
	m_SphereLock.lockForRead();
	out << m_AxialTilt;
	out << m_RotationAngleOffset;
	m_SphereLock.unlock();
}

void DaWorldSphere::operator<<(QDataStream& in)
{
	DaWorldLL::operator<<(in);
	m_SphereLock.lockForWrite();
	in >> m_AxialTilt;
	in >> m_RotationAngleOffset;
	m_SphereLock.unlock();
}

double DaWorldSphere::AxialTilt() const
{
	m_SphereLock.lockForRead();
	double v = m_AxialTilt;
	m_SphereLock.unlock();
	if(v == 0)
	{
		// Calculate and cache my axial tilt.
		AuDice dice(GenSeed());
		if(IsHabitable(HABITABLE_HomeWorld))
			v = dice.RandomGaussianDistribution(0, (PI/3.)/3.);
		else
			v = dice.RandomGaussianDistribution(0, (PI/2.)/3.);
		if(v == 0)
			v = .00001;	// Close enough to prevent recaching.
		m_SphereLock.lockForWrite();
		m_AxialTilt = v;
		m_SphereLock.unlock();
		//qDebug() << "DaWorldSphere::AxialTilt" << *this << "tilt" << (v*RAD2DEG);
	}
	return v;
}

void DaWorldSphere::CreateMoons(int moons, double firstOrbit, bool hasTitan, const AuDice& Cdice, HABITABILITY hab,
		ORBITZONE orbitZone)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	if(moons <= 0)
		return;
	AuDice dice(Cdice);

	// Create moon(s).
	// Determine the maximum number of moons. We may not have room for all of them.
	// Make an array of the available orbit radii based on typical moons.
	// Planets around gas giants will reserve what they need.
	QVector<double> aMoonOrbits;	// Distances where we can place moons.
	double radmax = (FeetPerOrbit() - FeetPerMoonOrbit())/2.0;	// Maximum available moon orbit radius.
	for(double radm=firstOrbit; radm <= radmax; radm += FeetPerMoonOrbit())
		aMoonOrbits.append(radm);
	Q_ASSERT(aMoonOrbits.count() > 0);

	// Create moons and rings where we can, consuming orbits as we go from our array of available orbits.
	for(int imoon=0; imoon<moons && !aMoonOrbits.isEmpty(); imoon++)
	{
		// A gas giant in the habitable zone may have a habitable moon.
		if(imoon == 0 && hasTitan)
		{
			// Determine which orbit the titan is in (max orbit zone width required is 16).
			Q_ASSERT(aMoonOrbits.count() >= 3);
			int iorb = dice.Roll(1, aMoonOrbits.count()-2, 0); // Index to orbit radius.
			Q_ASSERT(iorb >= 1);
			double radOrbitAu = aMoonOrbits[iorb] / FeetPerAu();	// Need this radius value in au.
			aMoonOrbits.erase(aMoonOrbits.begin()+iorb-1, aMoonOrbits.begin()+iorb+2);	// Remove three available orbits.

			// Create the titan.
			DaWorldGlobe* m = new DaWorldGlobe;
			m->ConnectSignals();
			m->Create(BODY_Titan, hab, dice, radOrbitAu, orbitZone, false);
			m->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(m);

			m->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();

			// AtomAdded is not called because all atoms added as a result of Create are reported at the end.
			// See DaSolarSystem::FinalizeContents
			//g_Au.AtomAdded(pMoon->Id());
		}
		else
		{
			// Determine which orbit the moon or ring is in.
			int iorb = 0;
			if(aMoonOrbits.count() > 1)
				iorb = dice.Roll(1, aMoonOrbits.count(), -1); // Index to orbit radius
			double radOrbitAu = aMoonOrbits[iorb] / FeetPerAu();	// Need this radius value in au.
			aMoonOrbits.remove(iorb);	// Remove this available orbit from the array.

			if(dice.Roll(2,6) <= 3)
			{
				// Create a ring.
				DaWorldRing* r = new DaWorldRing;
				r->ConnectSignals();
				r->Create(radOrbitAu, orbitZone);
				r->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
				ether->AddAtom(r);

				r->SetNormal();	// New atom is ready for normal thread processing.
			}
			else
			{
				// Create a moon.
				DaWorldGlobe* m = new DaWorldGlobe;
				m->ConnectSignals();
				m->Create(BODY_Moon, hab, dice, radOrbitAu, orbitZone, false);	// Could create a small titan.
				m->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
				ether->AddAtom(m);

				m->SetNormal();	// New atom is ready for normal thread processing.
				dice.Random();
			}

			// AtomAdded is not called because all atoms added as a result of Create are reported at the end.
			// See DaSolarSystem::FinalizeContents
			//g_Au.AtomAdded(pMoon->Id());
		}
	}
}

void DaWorldSphere::CreateMoonsForScan(int moons, double firstOrbit, bool hasTitan, const AuDice& Cdice, HABITABILITY hab,
		ORBITZONE orbitZone)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	if(moons <= 0)
		return;
	AuDice dice(Cdice);

	// Create moon(s).
	// Determine the maximum number of moons. We may not have room for all of them.
	// Make an array of the available orbit radii based on typical moons.
	// Planets around gas giants will reserve what they need.
	QVector<double> aMoonOrbits;	// Distances where we can place moons.
	double radmax = (FeetPerOrbit() - FeetPerMoonOrbit())/2.0;	// Maximum available moon orbit radius.
	for(double radm=firstOrbit; radm <= radmax; radm += FeetPerMoonOrbit())
		aMoonOrbits.append(radm);
	Q_ASSERT(aMoonOrbits.count() > 0);

	// Create moons and rings where we can, consuming orbits as we go from our array of available orbits.
	for(int imoon=0; imoon<moons && !aMoonOrbits.isEmpty(); imoon++)
	{
		// A gas giant in the habitable zone may have a habitable moon.
		if(imoon == 0 && hasTitan)
		{
			// Determine which orbit the titan is in (max orbit zone width required is 16).
			Q_ASSERT(aMoonOrbits.count() >= 3);
			int iorb = dice.Roll(1, aMoonOrbits.count()-2, 0); // Index to orbit radius.
			Q_ASSERT(iorb >= 1);
			double radOrbitAu = aMoonOrbits[iorb] / FeetPerAu();	// Need this radius value in au.
			aMoonOrbits.erase(aMoonOrbits.begin()+iorb-1, aMoonOrbits.begin()+iorb+2);	// Remove three available orbits.

			// Create the titan.
			DaWorldGlobe* m = new DaWorldGlobe;
			m->ConnectSignals();
			m->CreateForScan(BODY_Titan, hab, dice, radOrbitAu, orbitZone, false);
			m->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(m);

			m->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();

			// AtomAdded is not called because all atoms added as a result of Create are reported at the end.
			// See DaSolarSystem::FinalizeContents
			//g_Au.AtomAdded(pMoon->Id());
		}
		else
		{
			// Determine which orbit the moon or ring is in.
			int iorb = 0;
			if(aMoonOrbits.count() > 1)
				iorb = dice.Roll(1, aMoonOrbits.count(), -1); // Index to orbit radius
			double radOrbitAu = aMoonOrbits[iorb] / FeetPerAu();	// Need this radius value in au.
			aMoonOrbits.remove(iorb);	// Remove this available orbit from the array.

			if(dice.Roll(2,6) <= 3)
			{
				// Create a ring.
				DaWorldRing* r = new DaWorldRing;
				r->ConnectSignals();
				r->CreateForScan(radOrbitAu, orbitZone);
				r->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
				ether->AddAtom(r);

				r->SetNormal();	// New atom is ready for normal thread processing.
			}
			else
			{
				// Create a moon.
				DaWorldGlobe* m = new DaWorldGlobe;
				m->ConnectSignals();
				m->CreateForScan(BODY_Moon, hab, dice, radOrbitAu, orbitZone, false);	// Could create a titan.
				m->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
				ether->AddAtom(m);

				m->SetNormal();	// New atom is ready for normal thread processing.
				dice.Random();
			}

			// AtomAdded is not called because all atoms added as a result of Create are reported at the end.
			// See DaSolarSystem::FinalizeContents
			//g_Au.AtomAdded(pMoon->Id());
		}
	}
}

