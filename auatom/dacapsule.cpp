//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dacapsule.cpp,v 1.38 2022/03/09 19:59:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dacapsule.h"
//
#include "aucommpacket.h"
#include "auglobal.h"
#include "auterraininfo.h"
#include "daatomlist.h"
#include "dabd.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasentient.h"
#include "dasolarsystem.h"
#include "dateleporter.h"
#include "davehicle.h"
#include "daworldll.h"
#include "dgdata.h"

DaCapsule::DaCapsule(const QString& name, int idEmpire) : DaUnit(name, idEmpire)
{
}

DaCapsule::DaCapsule()
{
}

DaCapsule::~DaCapsule()
{
}

DaCapsule::DaCapsule(const DaCapsule& c) : DaUnit(c)
{
}

void DaCapsule::UnlockLiftContents(const DaAtomList& lockedAtoms)
{
	for(DaAtomList::const_iterator it = lockedAtoms.cbegin(); it != lockedAtoms.cend(); it++)
	{
		(*it)->MovedByTurboLift();		// Queries environment. Kicks AI in case this is their stop.
		(*it)->SetModified();
	}
}

DaCapsule* DaCapsule::FindParentCapsuleInEnv()
{
	return this;
}

const DaCapsule* DaCapsule::FindParentCapsuleInEnv() const
{
	return this;
}

int DaCapsule::BeamUp(DaPerson* person, int countRequested)
{
	// Locate a transporter room aboard this ship. It doesn't have to be on; it will turn on when it is activated.
	DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
	quint16 whichT;
	if(vehicle)
	{
		whichT = VehicleTransporterSystemId(vehicle->ParkingSpotType(), false);
		if(whichT == 0xffff)
			return -14;
	}
	else
	{
		whichT = TransporterSystemId(countRequested, false);
		if(whichT == 0xffff)
			return -1;
	}

	DaCreature* beamWho = dynamic_cast<DaCreature*>(person->Parent());
	if(!beamWho)
		beamWho = person;

	// We have functioning pads.
	// Set up the transfer.
	DaCapsule* targCap = 0;
	if(beamWho->IsInsideCapsule())
		targCap = beamWho->Parent()->FindParentCapsuleInEnv();

	if(targCap)
	{
		// Prepare to beam the subject from aboard a spacecraft to this ship.
		AuPointArray targPads;
		quint16 targWhich = 0;
		if(vehicle)
		{
			// Determine the 'which' id of the transporter room in which the subject is standing.
			targWhich = targCap->VehicleTransporterSystemId(vehicle->ParkingSpotType(), false);
			if(targWhich == 0xffff)
				return -16;

			// Vehicles can only be transported from vehicle transporter parking spots.
			AuMatrix4x4 parkPosM;
			VEHICLE_SPOT_TYPE spotType;
			if(!targCap->ClosestVehicleParkingSpot(vehicle, parkPosM, spotType))
				return -11;

			// The transporter room must be on and inactive. The transfer will activate it.
			int troomIndex = targCap->TransporterSystemIndex(targWhich);
			if(!targCap->IsTransporterSystemOn(troomIndex))
			{
				// The subject ship's transporter is not on. Any non-enemy ship permits people to beam away.
				POLITICAL_STANCE stanceToSubject = targCap->PoliticalStanceMakeContact(beamWho);
				if(stanceToSubject != POL_Enemy)
				{
					// The subject is not an enemy of the ship.
					// The transporter console in the room is manned by AI; turn it on.
					targCap->TurnTransporterSystemOn(troomIndex);
				}
			}

			// Locate vacant vehicle parking spots of the correct type at the destination.
			QList<AuMatrix4x4> targParkingSpots;
			targParkingSpots.append(parkPosM);

			// targLoc is only needed when targParkingSpots is empty.
			AuPoint targLoc;
			AuPoint targZ = targParkingSpots.first().ProjectVector(0,0,1);

			// In recognition of Star Trek, energize...
			return EnergizeVehicleTransporter(whichT, vehicle, false, targCap, targLoc, targZ, targWhich, targParkingSpots, false,
					false);
		}
		else
		{
			// If the subject is in a transporter room, attempt to initiate a transporter-to-transporter transfer from beam-up ship.
			if(targCap->IsTransporterRoom(beamWho->CapsuleRoomId()))
			{
				// Determine the 'which' id of the transporter room in which the subject is standing.
				targWhich = beamWho->AtTransporterWhich();
				if(targCap == this && targWhich == whichT)
					return -17;
				int troomIndex = targCap->TransporterSystemIndex(targWhich);
				if(!targCap->IsTransporterSystemOn(troomIndex))
				{
					// The subject ship's transporter is not on. Any non-enemy ship permits people to beam away.
					POLITICAL_STANCE stanceToSubject = targCap->PoliticalStanceMakeContact(beamWho);
					if(stanceToSubject != POL_Enemy)
					{
						// The subject is not an enemy of the ship.
						// If the transporter console in the room is manned by AI, turn it on.
						DaAtomList subs;
						targCap->ListNormalSubatoms(subs);
						for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
						{
							DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
							if(!pAtom)
								continue;
							if(!pAtom->IsAIControlled())
								continue;
							if(pAtom->IsTransporterStationPose() && pAtom->StationId() == targWhich)
							{
								targCap->TurnTransporterSystemOn(troomIndex);
								break;
							}
						}
					}
				}

				// The transporter room must be on and inactive. The transfer will activate it.
				if(targCap->IsTransporterSystemOn(troomIndex) && !targCap->IsTransporterSystemActive(troomIndex))
					targCap->TransporterPadLocations(targWhich, targPads);
			}

			AuPoint targLoc;
			if(targPads.isEmpty())
			{
				// Calculate location subject is standing relative to the ship, which is the target.
				// targLoc is only needed when targPads is empty.
				targLoc = beamWho->Location() - AuPoint(0,0,beamWho->GroundHeightAboveCG());

				// Offset randomly from the target location.
				targLoc += AuPoint(g_Au.Dice().Roll(1,17,-9), g_Au.Dice().Roll(1,17,-9));
			}

			// In recognition of Star Trek, energize...
			return EnergizeTransporter(whichT, false, countRequested, targCap, targLoc, AuPoint(0,0,1), targWhich, targPads, false,
					false);
		}
	}
	else
	{
		if(beamWho->IsRider())
			return -12;	// Shouldn't be possible but what the heck.
		else if(vehicle)
		{
			// Beam target from wherever they are, probably standing on a world or floating in space.
			// They could also be standing on the outside hull of a spacecraft.
			// Calculate location subject is standing, and up vector, relative to their parent atom, which is the target.
			DaAtom* targAtom = vehicle->Parent();
			AuPoint targLoc = AuPoint(0, 0, -vehicle->GroundHeightAboveCG());	// Location at vehicle's feet.

			// Offset randomly from the target location.
			targLoc += AuPoint(g_Au.Dice().Roll(1,17,-9), g_Au.Dice().Roll(1,17,-9));

			// Calculate targLoc and targZ relative to the parent of vehicle.
			AuMatrix4x4 t(vehicle->Transform());
			targLoc = t.Project(targLoc);										// vehicle's location, relative to their parent.
			AuPoint targZ = t.ProjectVector(vehicle->GravityNormal());			// vehicle's up vector, relative to their parent.

			// As in Star Trek, energize...
			QList<AuMatrix4x4> targParkingSpots;
			return EnergizeVehicleTransporter(whichT, vehicle, false, targAtom, targLoc, targZ, 0, targParkingSpots, false,
					person->IsAtBd());
		}
		else
		{
			// Beam target from wherever they are, probably standing on a world or floating in space.
			// They could also be standing on the outside hull of a spacecraft.
			// Calculate location subject is standing, and up vector, relative to their parent atom, which is the target.
			DaAtom* targAtom = beamWho->Parent();
			AuPoint targLoc = AuPoint(0, 0, -beamWho->GroundHeightAboveCG());	// Location at beamWho's feet.

			// Offset randomly from the target location.
			targLoc += AuPoint(g_Au.Dice().Roll(1,17,-9), g_Au.Dice().Roll(1,17,-9));

			// Calculate targLoc and targZ relative to the parent of beamWho.
			AuMatrix4x4 t(beamWho->Transform());
			targLoc = t.Project(targLoc);										// beamWho's location, relative to their parent.
			AuPoint targZ = t.ProjectVector(beamWho->GravityNormal());			// beamWho's up vector, relative to their parent.

			// As in Star Trek, energize...
			AuPointArray targPads;
			return EnergizeTransporter(whichT, false, countRequested, targAtom, targLoc, targZ, 0, targPads, false,
					person->IsAtBd());
		}
	}
}

int DaCapsule::EnergizeTransporter(quint16 shipWhich, bool isFrom, int padLimit, DaAtom* targAtom, const AuPoint& targLoc,
		const AuPoint& targZ, quint16 targWhich, AuPointArray& targPads, bool ignoreDestShields, bool targAtBd)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	// Verify that we have a target atom.
	if(!targAtom)
		return -3;

	int troomIndex = TransporterSystemIndex(shipWhich);
	if(troomIndex < 0)
		return -1;
	if(IsTransporterSystemActive(troomIndex))
		return -5;		// Transporter room is still energized in its cycle. This will complete shortly.

	DaWorldLL* targWorld = dynamic_cast<DaWorldLL*>(targAtom);

	DaCapsule* targCap = dynamic_cast<DaCapsule*>(targAtom);
	if(targCap)
	{
		if(targCap->IsNew())
		{
			// Prevent beaming aboard a nu ship that does not belong to my empire or a friend's empire.
			// The transporter console has shown us blips based on our stance toward them.
			// The stance of the target ship toward me is what matters here, though.
			POLITICAL_STANCE stance = PoliticalStance(targCap->EmpireId(), EmpireId());
			if(stance == POL_Enemy || stance == POL_Neutral)
				return -3;	// New ship's stance toward my empire prevents me from beaming aboard.
		}
		else if(!ignoreDestShields && !targCap->Is(Id()) && targPads.isEmpty() && targCap->ShieldsBlockTransportAboard())
		{
			// Prevent beaming aboard a ship with enough shields to block transporters.
			// The target ship must have its shields down when it is not the beam-up ship and the destination is not an array of
			// transporter room pads.
			return -10;	// Shields are up on the target ship.
		}
	}
	else if(!ignoreDestShields && targPads.isEmpty())
	{
		// Prevent beaming into the shields of a city, except when beaming to a transporter room.
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(targAtom->FindParentPlanetInEnv());
		if(llworld)
		{
			AuMatrix4x4 tm;
			targAtom->GetTransformRelativeToParent(tm, llworld);
			AuPoint worldLoc = tm.Project(targLoc);
			if(llworld->SubatomShieldsBlockTransporters(EmpireId(), worldLoc))
				return -13;	// Shields of a city block transporters.
		}
	}

	// Calculate X and Y vectors to lay out teleporters appropriately at transport location.
	AuPoint targX, targY;	// Only needed when targAtom is terrain.
	if(targZ != AuPoint(0,0,1))
	{
		targX = AuPoint(0,0,1).Cross(targZ);
		targX.Unit();
		targY = targZ.Cross(targX);
		targY.Unit();
	}
	else
	{
		targX = AuPoint(1,0);
		targY = AuPoint(0,1);
	}

	// Create an origin teleporter on every active pad and a corresponding destination teleporter at the destination.
	AuPointArray pads;
	TransporterPadLocations(shipWhich, pads);		// Locations of pads on the floor.
	if(padLimit > pads.count())
		padLimit = pads.count();	// Limited by number of physical pads.
	int padsWorking = TransporterPadsWorking(shipWhich);
	if(padLimit > padsWorking)
		padLimit = padsWorking;		// Limited by number of functioning pads.
	bool transporterMode = targPads.isEmpty() ? false:true;	// pads means transporter mode
	AuGlobeLLF targDevLL;				// Only if destination is transporter pads at a city.
	if(transporterMode)
	{
		// Target pads limit only applies in transporter mode.
		// Target pads don't have to be functioning. This ship does all the work. The pads just provide locations.
		if(padLimit > targPads.count())
			padLimit = targPads.count();	// Limited by number of pads at target.
	}
	if(!padLimit)
		return -1;	// No pads to transport, either here or there.

	// We know how many pads can be fired. Limit it further by the amount of capacitor power available.
	if(padLimit > (int)CapacitorLevel())
		padLimit = (int)CapacitorLevel();
	if(!padLimit)
		return -4;	// No power to transport. This ship is in trouble.

	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return -3;	// Not likely.

	DaTeleporter* telDest = 0;
	DaTeleporter* telOrigin = 0;
	const float maxRange = TransporterRange(shipWhich);
	const float centerHeight = DeckHeight()/2;	// Height above ground|floor for center placement of teleporters.
	int pad = 0;
	int teleportersCreated = 0;
	for(AuPointArray::const_iterator pl = pads.cbegin(); pl != pads.cend() && pad < padLimit; pl++, pad++)
	{
		// Determine position of the teleporter at the target.
		AuPoint destLoc;
		AuMatrix4x4 tTarget;
		if(!targPads.isEmpty())
		{
			// Destination is an array of transporter pad locations.
			// Take the next destination pad location from the array.
			destLoc = targPads.first();
			targPads.remove(0);

			if(targCap)
			{
				// Target is not terrain. Upright orienting not needed. Already upright relative to all other targets.
				tTarget.Translate(destLoc + AuPoint(0,0,centerHeight));
			}
			else
			{
				// Orient it upright to the terrain.
				AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(targZ);
				tTarget.Translate(destLoc + targZ*centerHeight);
				tTarget *= trot;
			}
		}
		else if(targWorld)
		{
			// Target location atom is a globe.
			// Array the destination teleporters to match the pad layout in the transporter room.
			if(pl == pads.cbegin())
			{
				destLoc = targLoc;
			}
			else
			{
				AuPoint pv = *pl - pads[0];
				destLoc = targLoc + targX * pv.x + targY * pv.y;
			}

			AuGlobeLL destLL;
			if(!targWorld->LLOfLoc(destLL, destLoc))
				continue;										// Skip pads that are not in the terrain space.

			AuTerrainInfo ter;
			targWorld->QueryTerrain(ter, destLL);
			double destAlt = targWorld->CGAltitude(destLoc);
			double locAlt = destAlt - ter.m_rSurfaceRadius;		// AGL altitude
			if(locAlt < 100*FeetPerMeter())
			{
				// Calculate location at ground level, even on ringworld dais.
				if(targAtBd)
				{
					// At a bd site but not in the building. Adjust up to ground level location if dest loc is under ground.
					// Otherwise we leave it so we can beam people from rooftops.
					if(ter.m_rSurfaceRadius > destAlt)
						destLoc = targWorld->LocOfLL(destLL, ter.m_rSurfaceRadius - ter.m_rSeaLevelRadius);
				}
				else
					destLoc = targWorld->LocOfLL(destLL, ter.m_rSurfaceRadius - ter.m_rSeaLevelRadius);
			}

			// Orient it upright to the terrain.
			AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(targZ);
			tTarget.Translate(destLoc + targZ*centerHeight);
			tTarget *= trot;
		}
		else if(targCap)
		{
			// Target location atom is in a capsule.
			// Array the destination teleporters to match the pad layout in the transporter room.
			if(pl == pads.cbegin())
			{
				// targLoc has already been tested for validity/unblocked/void.
				destLoc = targLoc;
			}
			else
			{
				AuPoint pv = *pl - pads[0];
				destLoc = targLoc + AuPoint(pv.x, pv.y);
			}

			// Skip any pads that are blocked in the destination ship.
			if(!targCap->TestTransportLocation(destLoc))
				continue;

			// Target is not terrain. Upright orienting not needed. Already upright relative to all other targets.
			tTarget.Translate(destLoc + AuPoint(0,0,centerHeight));
		}
		else
		{
			// Target location atom is not terrain or a transporter room or a spacecraft.
			// Array the destination teleporters to match the pad layout in the transporter room.
			if(pl == pads.cbegin())
			{
				// targLoc has already been tested for validity/unblocked/void.
				destLoc = targLoc;
			}
			else
			{
				AuPoint pv = *pl - pads[0];
				destLoc = targLoc + targX * pv.x + targY * pv.y;
			}

			// Orient it upright to the destination up vector.
			AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(targZ);
			tTarget.Translate(destLoc + targZ*centerHeight);
			tTarget *= trot;
		}

		// Destination and origin teleporter locations are in the coordinate space of their respective teleporter atoms.
		// The first location is always (0,0), which is the location of the teleporter atom itself.
		AuPoint destTelLoc;
		AuPoint orgTelLoc;
		if(!telOrigin || !telDest)
		{
			// First teleporter creates the teleporter atom instances.

			// Before placing the first teleporter, check to see if the target location is in range.
			// Perform a range test on the first teleporter to be created.
			// The ship location is a coarse enough test, relative to the solar system.

			// Calculate destination location relative to the solar system.
			const AuMatrix4x4& tdsys = targAtom->EnvTransform();
			AuPoint destSysLoc = tdsys.Project(destLoc);

			// Compare the distance between the locations.
			if(EnvLocation().Distance2(destSysLoc) > maxRange*maxRange)
				return -2;	// out of range

			// Determine position of the teleporter on the pad.
			AuMatrix4x4 tPad;
			tPad.Translate(*pl + AuPoint(0,0,centerHeight));

			// Create a destination teleporter atom that will die in a few seconds.
			// Must create destination first because the origin needs the id of the destination.
			telDest = new DaTeleporter;	// destination teleporter need no parameters
			telDest->ConnectSignals();
			telDest->Create();
			if(isFrom)
				telDest->SetPosAndOrient(AuAtomPos(targAtom->Id(), tTarget));	// Position the dest teleporter at the target.
			else
				telDest->SetPosAndOrient(AuAtomPos(Id(), tPad));				// Position the dest teleporter on the pad.
			ether->AddAtom(telDest);
			telDest->SetNormal();	// New atom is ready for normal thread processing.

			// Create an origin teleporter atom that will die in a few seconds. Origin needs the id of the destination.
			telOrigin = new DaTeleporter(telDest->Id(), maxRange);	// Origin needs destination id and max range.
			telOrigin->ConnectSignals();
			telOrigin->Create();
			if(isFrom)
				telOrigin->SetPosAndOrient(AuAtomPos(Id(), tPad));				// Position the origin teleporter on the pad.
			else
				telOrigin->SetPosAndOrient(AuAtomPos(targAtom->Id(), tTarget));	// Position the origin teleporter at the target.
			ether->AddAtom(telOrigin);
			telOrigin->SetNormal();	// New atom is ready for normal thread processing.
		}
		else
		{
			// Subsequent teleporters simply calculate locations, in the space of their respective teleporters.
			AuPoint padLoc = *pl + AuPoint(0,0,centerHeight);	// Location relative to the ship.
			AuPoint targLoc = tTarget.Translation();			// Location relative to the target atom.
			if(isFrom)
			{
				orgTelLoc = telOrigin->Transform().UnProject(padLoc);
				destTelLoc = telDest->Transform().UnProject(targLoc);
			}
			else
			{
				orgTelLoc = telOrigin->Transform().UnProject(targLoc);
				destTelLoc = telDest->Transform().UnProject(padLoc);
			}
		}

		// Add teleporter location to destination and origin teleporter atoms.
		telOrigin->AddTeleporterLoc(orgTelLoc);
		telDest->AddTeleporterLoc(destTelLoc);

		teleportersCreated++;
	}

	if(teleportersCreated)
	{
		// Signal addition of the nu teleporter atoms that we are done constructing them.
		g_Au.AtomAdded(telDest->Id());
		g_Au.AtomAdded(telOrigin->Id());

		// Deduct the power from the ship's capacitor and 'activate' the transporter room.
		// It will be deactivated when no more teleporters remain in the room.
		DischargeCapacitor(teleportersCreated);
		ActivateTransporterSystem(TransporterSystemIndex(shipWhich));
		if(transporterMode)
		{
			if(targCap)
				targCap->ActivateTransporterSystem(targCap->TransporterSystemIndex(targWhich));
		}

		telDest->TickTock();
		telOrigin->TickTock();
	}

	return teleportersCreated;
}

int DaCapsule::EnergizeVehicleTransporter(quint16 shipWhich, DaVehicle* vehicle, bool isFrom, DaAtom* targAtom,
		const AuPoint& targLoc, const AuPoint& targZ, quint16 targWhich, QList<AuMatrix4x4>& targParkingSpots,
		bool ignoreDestShields, bool targAtBd)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	// Verify that we have a target atom.
	if(!targAtom || !vehicle)
		return -3;
	if(!vehicle->CanSetInMotion())
		return 0;

	int troomIndex = TransporterSystemIndex(shipWhich);
	if(troomIndex < 0)
		return -1;
	if(IsTransporterSystemActive(troomIndex))
		return -5;		// Transporter room is still energized in its cycle. This will complete shortly.

	DaWorldLL* targWorld = dynamic_cast<DaWorldLL*>(targAtom);

	DaCapsule* targCap = dynamic_cast<DaCapsule*>(targAtom);
	if(targCap)
	{
		if(targCap->IsNew())
		{
			// Prevent beaming aboard a nu ship that does not belong to my empire or a friend's empire.
			// The transporter console has shown us blips based on our stance toward them.
			// The stance of the target ship toward me is what matters here, though.
			POLITICAL_STANCE stance = PoliticalStance(targCap->EmpireId(), EmpireId());
			if(stance == POL_Enemy || stance == POL_Neutral)
				return -3;	// New ship's stance toward my empire prevents me from beaming aboard.
		}
		else if(!ignoreDestShields && !targCap->Is(Id()) && targParkingSpots.isEmpty() && targCap->ShieldsBlockTransportAboard())
		{
			// Prevent beaming aboard a ship with enough shields to block transporters.
			// The target ship must have its shields down when it is not the beam-up ship and the destination is not an array of
			// transporter room pads.
			return -10;	// Shields are up on the target ship.
		}
	}
	else if(!ignoreDestShields && targParkingSpots.isEmpty())
	{
		// Prevent beaming into the shields of a city, except when beaming to a transporter room.
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(targAtom->FindParentPlanetInEnv());
		if(llworld)
		{
			AuMatrix4x4 tm;
			targAtom->GetTransformRelativeToParent(tm, llworld);
			AuPoint worldLoc = tm.Project(targLoc);
			if(llworld->SubatomShieldsBlockTransporters(EmpireId(), worldLoc))
				return -13;	// Shields of a city block transporters.
		}
	}

	// Calculate X and Y vectors to lay out teleporters appropriately at transport location.
	AuPoint targX, targY;	// Only needed when targAtom is terrain.
	if(targZ != AuPoint(0,0,1))
	{
		targX = AuPoint(0,0,1).Cross(targZ);
		targX.Unit();
		targY = targZ.Cross(targX);
		targY.Unit();
	}
	else
	{
		targX = AuPoint(1,0);
		targY = AuPoint(0,1);
	}

	// Create an origin teleporter on the vehicle and a corresponding destination teleporter at the destination.
	QList<AuMatrix4x4> pads;
	DaVehicle* parked = 0;
	if(isFrom)
	{
		// Beaming away. Find the closest parking spot to the vehicle.
		AuMatrix4x4 parkPosM;
		VEHICLE_SPOT_TYPE spotType;
		if(ClosestVehicleParkingSpot(vehicle, parkPosM, spotType))
			pads.append(parkPosM);
	}
	else
	{
		// Beaming aboard. Find appropriate parking spots that are vacant.
		AuMatrix4x4 spotM;
		if(!VehicleTransporterSpot(vehicle->ParkingSpotType(), spotM, parked))
			return -6;	// No spots available for the type of vehicle.
		pads.append(spotM);
	}
	int padsWorking = TransporterPadsWorking(shipWhich);
	if(padsWorking > pads.size())
		padsWorking = pads.size();
	bool transporterMode = targParkingSpots.isEmpty() ? false:true;	// pads means transporter mode
	AuGlobeLLF targDevLL;				// Only if destination is transporter pads at a city.
	if(transporterMode)
	{
		// Target pads don't have to be functioning. This ship does all the work. The pads just provide locations.
		if(padsWorking > targParkingSpots.count())
			padsWorking = targParkingSpots.count();	// Limited by number of pads at target.
	}
	if(!padsWorking)
		return -1;	// No pads to transport, either here or there.

	// We know how many pads can be fired. Limit it further by the amount of capacitor power available.
	if(CapacitorLevel() < 10)
		return -4;	// No power to transport. This ship is in trouble.

	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return -3;	// Not likely.

	DaTeleporter* telDest = 0;
	DaTeleporter* telOrigin = 0;
	const float maxRange = TransporterRange(shipWhich);
	const float centerHeight = DeckHeight()/2;	// Height above ground|floor for center placement of teleporters.

	// Determine position of the teleporter at the target.
	AuPoint destLoc;
	AuMatrix4x4 tTarget;
	if(!targParkingSpots.isEmpty())
	{
		// Destination is an array of parking spot matrices.
		// Use the first destination matrix in the array.
		tTarget = targParkingSpots.first();
		destLoc = tTarget.Translation();
		tTarget.Translate(0,0,centerHeight);
	}
	else if(targWorld)
	{
		// Target location atom is a globe.
		destLoc = targLoc;

		AuGlobeLL destLL;
		if(!targWorld->LLOfLoc(destLL, destLoc))
			return -3;										// Skip pads that are not in the terrain space.

		AuTerrainInfo ter;
		targWorld->QueryTerrain(ter, destLL);
		double destAlt = targWorld->CGAltitude(destLoc);
		double locAlt = destAlt - ter.m_rSurfaceRadius;		// AGL altitude
		if(locAlt < 100*FeetPerMeter())
		{
			// Calculate location at ground level, even on ringworld dais.
			if(targAtBd)
			{
				// At a bd site but not in the building. Adjust up to ground level location if dest loc is under ground.
				// Otherwise we leave it so we can beam people from rooftops.
				if(ter.m_rSurfaceRadius > destAlt)
					destLoc = targWorld->LocOfLL(destLL, ter.m_rSurfaceRadius - ter.m_rSeaLevelRadius);
			}
			else
				destLoc = targWorld->LocOfLL(destLL, ter.m_rSurfaceRadius - ter.m_rSeaLevelRadius);
		}

		// Orient it upright to the terrain.
		AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(targZ);
		tTarget.Translate(destLoc + targZ*centerHeight);
		tTarget *= trot;
	}
	else if(targCap)
	{
		// Target location atom is in a spacecraft.
		destLoc = targLoc;

		// Skip any pads that are blocked in the destination ship.
		if(parked && parked->IsNormal())
		{
			// Hide the parked vehicle so it doesn't block our transport.
			parked->SetDormant();
			if(!targCap->TestTransportLocation(destLoc))
			{
				parked->SetNormal();
				return 0;
			}
			parked->SetNormal();
		}
		else
		{
			if(!targCap->TestTransportLocation(destLoc))
				return 0;
		}

		// Target is not terrain. Upright orienting not needed. Already upright relative to all other targets.
		AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(targZ);
		tTarget.Translate(destLoc + targZ*centerHeight);
		tTarget *= trot;
	}
	else
	{
		// Target location atom is not terrain or a transporter room or a spacecraft.
		destLoc = targLoc;

		// Orient it upright to the destination up vector.
		AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(targZ);
		tTarget.Translate(destLoc + targZ*centerHeight);
		tTarget *= trot;
	}

	// Destination and origin teleporter locations are in the coordinate space of their respective teleporter atoms.
	// The first location is always (0,0), which is the location of the teleporter atom itself.
	AuPoint destTelLoc;
	AuPoint orgTelLoc;

	// Before placing the first teleporter, check to see if the target location is in range.
	// Perform a range test on the first teleporter to be created.
	// The ship location is a coarse enough test, relative to the solar system.

	// Calculate destination location relative to the solar system.
	const AuMatrix4x4& tdsys = targAtom->EnvTransform();
	AuPoint destSysLoc = tdsys.Project(destLoc);

	// Compare the distance between the locations.
	if(EnvLocation().Distance2(destSysLoc) > maxRange*maxRange)
		return -2;	// out of range

	// We are committed now. The parked vehicle can be destroyed.
	if(parked && !parked->HasSubatoms())
		parked->Kill();

	// Determine position of the teleporter on the pad.
	AuMatrix4x4 tPad(pads.first());
	tPad.Translate(AuPoint(0,0,centerHeight));

	// Create a destination teleporter atom that will die in a few seconds.
	// Must create destination first because the origin needs the id of the destination.
	telDest = new DaTeleporter;	// destination teleporter need no parameters
	telDest->ConnectSignals();
	telDest->Create();
	if(isFrom)
		telDest->SetPosAndOrient(AuAtomPos(targAtom->Id(), tTarget));	// Position the dest teleporter at the target.
	else
		telDest->SetPosAndOrient(AuAtomPos(Id(), tPad));				// Position the dest teleporter on the pad.
	ether->AddAtom(telDest);
	telDest->SetVehicleSpotType(vehicle->ParkingSpotType());
	telDest->SetNormal();	// New atom is ready for normal thread processing.

	// Create an origin teleporter atom that will die in a few seconds. Origin needs the id of the destination.
	telOrigin = new DaTeleporter(telDest->Id(), maxRange);	// Origin needs destination id and max range.
	telOrigin->ConnectSignals();
	telOrigin->Create();
	if(isFrom)
		telOrigin->SetPosAndOrient(AuAtomPos(Id(), tPad));				// Position the origin teleporter on the pad.
	else
		telOrigin->SetPosAndOrient(AuAtomPos(targAtom->Id(), tTarget));	// Position the origin teleporter at the target.
	ether->AddAtom(telOrigin);
	telOrigin->SetVehicleSpotType(vehicle->ParkingSpotType());
	telOrigin->SetNormal();	// New atom is ready for normal thread processing.

	// Add teleporter location to destination and origin teleporter atoms.
	telOrigin->AddTeleporterLoc(orgTelLoc);
	telDest->AddTeleporterLoc(destTelLoc);

	// Signal addition of the nu teleporter atoms that we are done constructing them.
	g_Au.AtomAdded(telDest->Id());
	g_Au.AtomAdded(telOrigin->Id());

	telDest->TickTock();
	telOrigin->TickTock();

	// Deduct the power from the ship's capacitor and 'activate' the transporter room.
	// It will be deactivated when no more teleporters remain in the room.
	DischargeCapacitor(10);
	ActivateTransporterSystem(TransporterSystemIndex(shipWhich));
	if(transporterMode)
	{
		if(targCap)
			targCap->ActivateTransporterSystem(targCap->TransporterSystemIndex(targWhich));
	}

	// Kick the vehicle so it gets sucked into the portal.
	vehicle->SetInMotion();
	vehicle->TickTock();
	return 1;
}

bool DaCapsule::EnergizeVehicleTransporter(const AuCommPacket* msg, bool isFrom, DaAtom* targAtom, const AuPoint& targLoc,
		const AuPoint& targZ, quint16 targWhich, QList<AuMatrix4x4>& targParkingSpots, AuCommStdChannel::RANGE limitedRange)
{
	// Locate the sender. They must be aboard this ship and in a vehicle.
	const DaPerson* person = dynamic_cast<const DaPerson*>(FindIdRecursive(msg->FromId()));
	if(!person)
		return false;	// Person was not found aboard.
	VEHICLE_SPOT_TYPE spotType = person->InVehicleTransporter();
	if(spotType == SPOT_Invalid)
		return false;
	DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
	if(!vehicle)
		return false;

	// Insure my transporter has a vehicle transporter module.
	quint16 trWhich = VehicleTransporterSystemId(spotType, false);
	if(!trWhich)
		return false;	// My transporter system is not equipped with a vehicle transporter.

	// Caller has already verified shields state at destination is ok.
	int result = EnergizeVehicleTransporter(trWhich, vehicle, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots, true,
			vehicle->IsAtBd());
	QString text;
	if(result == 1)
		text = QObject::tr("Vehicle transporter energized.");
	else if(result == 0)
		text = QObject::tr("Vehicle transporter beam was blocked by obstacles near target; try again.");
	else if(result == -1)
		text = QObject::tr("Vehicle transporter is not ready.");
	else if(result == -2)
		text = QObject::tr("Target is beyond vehicle transporter range.");
	else if(result == -3)
		text = QObject::tr("Vehicle transporter cannot lock on to target.");
	else if(result == -4)
		text = QObject::tr("Insufficient power to energize vehicle transporter.");
	else if(result == -5)
		text = QObject::tr("Vehicle transporter has not finished its activation cycle.");
	else if(result == -6)
		text = QObject::tr("Vehicle transporter has no empty parking spots for the vehicle.");
	else if(result == -10)
		text = QObject::tr("Vehicle transporter blocked by shields on target vessel.");
	else if(result == -13)
		text = QObject::tr("Vehicle transporter blocked by shield generator on the ground.");
	else
		text = QObject::tr("Unable to energize vehicle transporter.");
	TxReplyChat(text, *msg, limitedRange);
	return result == 1;
}

void DaCapsule::FindCapsulesRecursive(DaAtomList& capsules) const
{
	//if(!IsNormal())
		//return;
	DaUnit::FindCapsulesRecursive(capsules);
	capsules.append((DaCapsule*)this);	// Cast away constness.
}

void DaCapsule::WorldCaptured(quint32 byEmpire)
{
	// Notify my subatoms.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ci = subs.cbegin(); ci != subs.cend(); ci++)
		(*ci)->WorldCaptured(byEmpire);
}

DaSentient* DaCapsule::FindDoctor(const DaCreature& patient) const
{
	quint16 stationId = patient.StationId();
	if(stationId)
	{
		// Patient is in a surgery unit. Find the doctor manning the corresponding station.
		if(patient.ConsoleSeatIndex() != 1)
			return 0;
	}
	else
	{
		// Patient is not in a surgery unit. Find the closest station and try to put them into it.
		AuStation station;
		quint16 which;
		if(!ClosestUnoccupiedSurgeryUnit(patient.Location(), stationId, station, which, false))
			return 0;
		// Check proximity to the surgery unit.
		AuPoint sloc = station.OpM().UnProject(patient.Location());	// Patient location in the space of the surgery unit.
		if(sloc.Length2() > 5*5)
			return 0;
	}

	// Find the doctor that is controlling stationId.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* sen = dynamic_cast<DaSentient*>(*it);
		if(!sen)
			continue;
		if(sen->StationId() != stationId)
			continue;
		if(sen->ConsoleSeatIndex() != 0)
			continue;	// Doctor's seat is always index 0. Old style designs?
		return sen;
	}
	return 0;
}

DaSentient* DaCapsule::FindTransporterOperator(quint16 roomId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// A transporter operator will be the only crewman in the room seated at a transporter station. There can be only one.
		DaSentient* sen = dynamic_cast<DaSentient*>(*it);
		if(!sen)
			continue;
		if(sen->CapsuleRoomId() != roomId)
			continue;
		if(!sen->IsAIControlled())
			continue;
		if(!sen->IsTransporterStationPose())
			continue;
		return sen;	// Someone was found in the room that is sitting at a transporter console.
	}
	return 0;
}

bool DaCapsule::IsConsoleOccupiedExcept(quint16 stationId, const AuId& exceptId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		if((*ai)->Is(exceptId))
			continue;
		const DaSentient* sent = dynamic_cast<const DaSentient*>(*ai);
		if(!sent)
			continue;
		if(sent->StationId() != stationId)
			continue;
		return true;
	}
	return false;
}

bool DaCapsule::IsConsoleSeatOccupiedExcept(quint16 stationId, unsigned int seatIndex, const AuId& exceptId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		if((*ai)->Is(exceptId))
			continue;
		const DaSentient* sent = dynamic_cast<const DaSentient*>(*ai);
		if(!sent)
			continue;
		if(sent->StationId() != stationId)
			continue;
		if(sent->ConsoleSeatIndex() != seatIndex)
			continue;
		return true;
	}
	return false;
}

DaSentient* DaCapsule::SentientGoingToStation(quint16 stationId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		DaSentient* sent = dynamic_cast<DaSentient*>(*ai);
		if(!sent)
			continue;
		if(sent->OrderStationId() != stationId &&
				sent->StationId() != stationId)
			continue;	// Not going to that station or seated at that station.
		return sent;
	}
	return 0;
}

// Test for medical unit occupied by anyone, including people and ai controlled citizens.
bool DaCapsule::IsPatientStationOccupied(quint16 stationId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		DaCreature* pAtom = dynamic_cast<DaCreature*>(*ai);
		if(!pAtom)
			continue;
		if(pAtom->StationId() != stationId)
			continue;
		if(!pAtom->IsPatientStationPose())
			continue;
		return true;
	}
	return false;
}

QString DaCapsule::DamageReportHtml() const
{
	// Lock the root and find the scene atom.
	AuDamage damage;
	DamageReportPercent(damage, 3);	// damage will be expressed in aggregate percent for each system, with no parts.

	// Get the names of those systems.
	QSet<AuSysId> sysIds;
	damage.SystemIds(sysIds);
	QHash<AuSysId, QString> names;
	for(QSet<AuSysId>::const_iterator di = sysIds.cbegin(); di != sysIds.cend(); di++)
	{
		QString name = SystemName(*di);
		names.insert(*di, name);
	}

	QString icoRed(":data/AlertRedBright.png");
	QString icoYellow(":data/AlertYellowBright.png");
	QString icoGreen(":data/AlertGreen.png");

	QString rpt = QObject::tr("<b>DAMAGE REPORT</b><div style=\"margin-left: 10px;\">");
	rpt += tr("<table border=\"0\" cellpadding=\"2\" "
		"cellspacing=\"0\" style=\"background-color: rgb(90, 20, 20);\"><tbody>");
	for(QSet<AuSysId>::const_iterator di = sysIds.cbegin(); di != sysIds.cend(); di++)
	{
		QString name = names[*di];
		QString icoSys(SystemIcon((*di).System()));
		quint32 percDmg = damage.HitsDamageNoParts(*di);
		QString perc = QString("%1%").arg(100-percDmg);
		QString icoPerc;
		if(percDmg >= 90)
			icoPerc = icoRed;
		else if(percDmg >= 50)
			icoPerc = icoYellow;
		else
			icoPerc = icoGreen;

		rpt += tr("<tr><td><img height=\"16\" src=\"qrc%1\"> %2</td><td><img height=\"16\" src=\"qrc%3\"> %4</td></tr>")
			.arg(icoSys)
			.arg(name)
			.arg(icoPerc)
			.arg(perc);
	}
	rpt += "</tbody></table>";
	rpt += "</div><br>";
	return rpt;
}

int DaCapsule::TestMissileHit(const AuPoint& missileLoc, float proximity) const
{
	Q_UNUSED(proximity);
	AuPoint loc = Transform().UnProject(missileLoc);
	if(ShieldsBlockWeapons())
	{
		// Shields are effective if the explosion occurs inside them or in the zone of the shell surface.
		if(ShieldSurfaceNear(loc) || ShieldContains(loc))
			return 2;	// Shields neutralize missiles everywhere inside the bubble.
	}

	// Test for actual hit on my solid hull.
	return IsInOrOnHull(loc) ? 1 : 0;
}

bool DaCapsule::VehicleTransporterSpot(VEHICLE_SPOT_TYPE spotType, AuMatrix4x4& parkingSpot, DaVehicle*& parked) const
{
	QList<AuMatrix4x4> parkingSpots;
	VehicleParkingSpots(spotType, parkingSpots);
	if(parkingSpots.empty())
		return false;		// No spots at all.

	// Remember spots with new and used replaceable vehicles.
	DaVehicle* repNew = 0;
	AuMatrix4x4 repNewM;
	DaVehicle* repUsed = 0;
	AuMatrix4x4 repUsedM;

	// Check vehicle parking spots for vacancy.
	DaAtomList vehicles;
	FindVehiclesRecursive(vehicles);
	for(DaAtomList::const_iterator it = vehicles.begin(); it != vehicles.end(); it++)
	{
		DaVehicle* vehicle = (DaVehicle*)*it;
		if(!vehicle->IsNormal())
			continue;
		AuPoint shipLoc;
		if(!vehicle->GetLocationRelativeToParent(shipLoc, this))
			continue;
		// Check proximity to parking spots. Remove any that are too close.
		double vht = vehicle->GroundHeightAboveCG() + 2;
		QList<AuMatrix4x4>::iterator si = parkingSpots.begin();
		while(si != parkingSpots.end())
		{
			AuPoint spotLoc = (*si).UnProject(shipLoc);		// Vehicle location relative to the parking spot.
			if(spotLoc.z > 0 && spotLoc.z < vht && spotLoc.Length_2D() < 4)
			{
				if(vehicle->IsNew())
				{
					// New vehicle can be replaced.
					repNew = vehicle;
					repNewM = *si;
				}
				else if(!vehicle->HasSubatoms())
				{
					// Empty vehicle can be replaced.
					repUsed = vehicle;
					repUsedM = *si;
				}
				si = parkingSpots.erase(si);				// Spot is occupied by the vehicle.
			}
			else
				si++;										// Spot is not occupied by the vehicle.
		}
	}

	// Return the result.
	if(!parkingSpots.empty())
	{
		// We found an empty parking spot.
		parked = 0;
		parkingSpot = parkingSpots.first();
		return true;
	}
	else if(repNew)
	{
		// No empty spot was found but we found a replaceable nu vehicle.
		parked = repNew;
		parkingSpot = repNewM;
		return true;
	}
	else if(repUsed)
	{
		// No empty spot was found but we found a replaceable used vehicle.
		parked = repUsed;
		parkingSpot = repUsedM;
		return true;
	}

	// No useable parking spot was found.
	return false;
}

DaSentient* DaCapsule::FindAI(NPC_ROLE npcrole) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* sent = dynamic_cast<DaSentient*>(*it);
		if(!sent)
			continue;
		if(sent->NpcRole() != npcrole)
			continue;	// Demiavatars who are currently under avatar control will return ROLE_Count.
		if(sent->IsLockedPose())
			continue;	// Skip sentients at a console or post.
		if(sent->IsAIControlled())
			return sent;
	}
	return 0;
}

DaSentient* DaCapsule::FindAIInRoom(NPC_ROLE npcrole, const QString& roomName) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* sent = dynamic_cast<DaSentient*>(*it);
		if(!sent)
			continue;
		if(sent->NpcRole() != npcrole)
			continue;	// Demiavatars who are currently under avatar control will return ROLE_Count.
		if(sent->IsLockedPose())
			continue;	// Skip sentients at a console or post.
		if(sent->CapsuleRoomName() != roomName)
			continue;
		if(sent->IsAIControlled())
			return sent;
	}
	return 0;
}

DaNpc* DaCapsule::FindNpc(NPC_ROLE npcrole) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaNpc* sent = dynamic_cast<DaNpc*>(*it);
		if(!sent)
			continue;
		if(sent->NpcRole() != npcrole)
			continue;	// Demiavatars who are currently under avatar control will return ROLE_Count.
		if(sent->IsLockedPose())
			continue;	// Skip npcs at a console or post.
		return sent;
	}
	return 0;
}

DaNpc* DaCapsule::FindNpcInRoom(NPC_ROLE npcrole, const QString& roomName) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaNpc* sent = dynamic_cast<DaNpc*>(*it);
		if(!sent)
			continue;
		if(sent->NpcRole() != npcrole)
			continue;	// Demiavatars who are currently under avatar control will return ROLE_Count.
		if(sent->IsLockedPose())
			continue;	// Skip npcs at a console or post.
		if(sent->CapsuleRoomName() != roomName)
			continue;
		return sent;
	}
	return 0;
}

