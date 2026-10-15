//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasentient.cpp,v 1.646 2022/05/19 16:18:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dasentient.h"
//
#include "aubank.h"
#include "aubddesign.h"
#include "aubox.h"
#include "aucity.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "auscroute.h"
#include "auskill.h"
#include "austory.h"
#include "ausynccontroller.h"
#include "daanimal.h"
#include "daarena.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dpconsolesurgery.h"
#include "dpconsoleweapon.h"

// This id is traced by various pathing functions. It can be used to follow an individual citizen's pathing decisions.
static AuId 							s_PathTrace(1);

#define MOVE_NEAR					12	// Feet distance from target to stop within.
#define MOVE_CLOSE					7	// Feet distance from target to stop within that is closer than MOVE_NEAR.
//#define MOVE_EXACT					2.5	// Feet distance from target to stop within for precise movements.
#define MOVE_EXACT					1.5	// Feet from target to stop within for precise moves.
#define MOVE_STATION				2.5	// Feet from target to stop within for moves to a control station.
#define NEAR_DOOR_THINK_FAST		6	// Feet distance from door or hatch to think more often about getting through it.
#define RUN_FAR_ENOUGH				15	// Feet distance from target to run instead of walk.
#define ANIMAL_RANGE				100	// Feet distance at which wild animals are considered to be a threat.
#define DEFEND_RANGE				50

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAIOrder::operator>>(QDataStream& out) const
{
	quint8 hasWhat = m_Type;	// Bottom 6 bits is order type.
	if(m_StationId)
		hasWhat |= 0x80;
	if(!m_Text.isEmpty())
		hasWhat |= 0x40;

	out << hasWhat;
	out << m_Loc;
	out << m_Target;
	out << m_Time.UTC();

	if(hasWhat & 0x80)
		out << m_StationId;
	if(hasWhat & 0x40)
		out << m_Text;
}

void AuAIOrder::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;

	m_Type = (CITIZEN_ORDER)(hasWhat & 0x3f);	// Bottom 6 bits.
	in >> m_Loc;
	in >> m_Target;

	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_Time = tm;

	if(hasWhat & 0x80)
		in >> m_StationId;
	else
		m_StationId = 0;

	if(hasWhat & 0x40)
		in >> m_Text;
	else
		m_Text = "";
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAIOrderArray::operator>>(QDataStream& out) const
{
	quint8 countToSend;
	if(count() < 0xff)
		countToSend = count();
	else
		countToSend = 0xff;
	out << countToSend;
	for(const_iterator it = cbegin(); it != cend() && countToSend; it++, countToSend--)
		(*it) >> out;
}

void AuAIOrderArray::operator<<(QDataStream& in)
{
	clear();
	quint8 orders;
	in >> orders;
	AuAIOrder aio;
	for(; orders; orders--)
	{
		aio << in;
		append(aio);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
AuSentientDecision* AuSentientDecision::Create(DECISION type)
{
	switch(type)
	{
	case D_Invalid:
	case D_COUNT:
		break;
	case D_PickUp:
		return new AuSDPickUp;
	case D_ReloadWeapon:
		return new AuSDReloadWeapon;
	case D_ShipAnnounceOrder:
		return new AuSDShipAnnounceOrder;
	case D_ShipBoardCrew:
		return new AuSDShipBoardCrew;
	case D_ShipBoardCryo:
		return new AuSDShipBoardCryo;
	case D_ShipBoardPassengers:
		return new AuSDShipBoardPassengers;
	case D_ShipBoardTroops:
		return new AuSDShipBoardTroops;
	case D_ShipBuyCargoCategory:
		return new AuSDShipBuyCargoCategory;
	case D_ShipBuyCargoCommodity:
		return new AuSDShipBuyCargoCommodity;
	case D_ShipCloseAllHullDoors:
		return new AuSDShipCloseAllHullDoors;
	case D_ShipCrewPosture:
		return new AuSDShipCrewPosture;
	case D_ShipDebarkCryo:
		return new AuSDShipDebarkCryo;
	case D_ShipDebarkPassengers:
		return new AuSDShipDebarkPassengers;
	case D_ShipDoorClose:
		return new AuSDShipDoorClose;
	case D_ShipDoorOpen:
		return new AuSDShipDoorOpen;
	case D_ShipExterminateRats:
		return new AuSDShipExterminateRats;
	case D_ShipLootShip:
		return new AuSDShipLootShip;
	case D_ShipMissionAttack:
		return new AuSDShipMissionAttack;
	case D_ShipMissionOrderComplete:
		return new AuSDShipMissionOrderComplete;
	case D_ShipMissionState:
		return new AuSDShipMissionState;
	case D_ShipPatientDefib:
		return new AuSDShipPatientDefib;
	case D_ShipPatientHeal:
		return new AuSDShipPatientHeal;
	case D_ShipPatientTreat:
		return new AuSDShipPatientTreat;
	case D_ShipPilotPolicy:
		return new AuSDShipPilotPolicy;
	case D_ShipPilotReportContacts:
		return new AuSDShipPilotReportContacts;
	case D_ShipReconcileRoster:
		return new AuSDShipReconcileRoster;
	case D_ShipScuttle:
		return new AuSDShipScuttle;
	case D_ShipSellCargoCategory:
		return new AuSDShipSellCargoCategory;
	case D_ShipSellCargoCommodity:
		return new AuSDShipSellCargoCommodity;
	case D_ShipSensorsReportContacts:
		return new AuSDShipSensorsReportContacts;
	case D_ShipShieldsPolicy:
		return new AuSDShipShieldsPolicy;
	case D_ShipTransporterState:
		return new AuSDShipTransporterState;
	case D_ShipWeaponFire:
		return new AuSDShipWeaponFire;
	case D_ShipWeaponSetPower:
		return new AuSDShipWeaponSetPower;
	}

	qWarning() << "AuSentientDecision::Create error: bad decision type" << type;
	return 0;
}

void AuSDPickUp::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaAtom* env = sent->FindEnvironmentAtom();
	DaAtom* orderTarget = env->FindIdRecursive(m_PickUpId);
	DaItem* pItem = dynamic_cast<DaItem*>(orderTarget);
	if(pItem)
	{
		// Found an item to pick up.
		// Add the item to this atom's inventory if it is useful.
		// The item will replace an existing item and cause it to be dropped if it is used for the same purpose. i.e. a
		// weapon will cause me to drop my current weapon and use the nu weapon instead. Ammo will not replace existing
		// ammo and will instead be ignored if I don't have room.
		const AuItem* item = pItem->LockItem();
		if(!item || sent->AddGearItemUseDrop(*item, 2, true))
		{
			// The item fit into my inventory somewhere.
			pItem->UnlockItem();
			pItem->Kill();					// Delete the item atom from the environment.
		}
		else
			pItem->UnlockItem();
	}
	else
	{
		DaCreature* creature = dynamic_cast<DaCreature*>(orderTarget);
		bool isWorkerOrCharacter = false;
		DaCitizen* citiz = dynamic_cast<DaCitizen*>(orderTarget);
		if(citiz)
			isWorkerOrCharacter = citiz->IsNpcWorker() || citiz->IsStoryCharacter();

		if(creature &&										// Target is a creature.
				creature->IsBodyDyingOrDead() &&			// Creature is dying.
				!creature->IsRider() &&						// Creature is not riding another creature.
				!isWorkerOrCharacter)						// Creature is not a worker or a character.
		{
			// Attempt to pick up the dying body if it is small enough and if I am not already carrying a body.
			if(!sent->IsMounted() && (!dynamic_cast<DaAnimal*>(creature) || sent->Volume() >= creature->Volume() * .9f))
			{
				// Pick up the dying body. This will stabilize it and stop it from dying until it is dropped.
				creature->ForceMountCreature(sent);
			}
		}
	}
}

void AuSDReloadWeapon::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	// Reload my weapon.
	AuGear* gear = sent->LockGearForWrite();
	if(gear)
	{
		if(gear->ReloadItem(m_UsePos, m_UseSub))
		{
			sent->UpdateGearWeight();
			sent->SetModified();
		}
		else
		{
			// My weapon could not be reloaded. Find ammo.
			DaItem* ammoItem;
			AuPoint ammoDir;
			float ammoRange;
			sent->LookForAmmoType(m_AmmoType, ammoItem, ammoDir, ammoRange);
			if(ammoItem)
			{
				// Found ammo. Insert a temporary order to get the ammo.
				sent->TakeOrderAmendment(ORDER_GetItem, ammoItem->Id(), AuPoint(0,0));
			}
		}
	}
	sent->UnlockGear();
}

void AuSDShipDoorClose::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaAtom* env = sent->FindEnvironmentAtom();
	DaCapsule* ship = dynamic_cast<DaCapsule*>(env->FindIdRecursive(m_ShipId));
	if(ship)
		ship->CloseDoor(m_DoorIndex);	// Unjams the door if it's jammed.
}

void AuSDShipDoorOpen::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaAtom* env = sent->FindEnvironmentAtom();
	DaCapsule* ship = dynamic_cast<DaCapsule*>(env->FindIdRecursive(m_ShipId));
	if(ship && ship->CanOpenDoor(*sent, m_DoorIndex))
		ship->OpenDoor(m_DoorIndex);	// Unjams the door if it's jammed.
}

void AuSDShipCloseAllHullDoors::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaCapsule* ship = dynamic_cast<DaCapsule*>(sent->FindParentCapsuleInEnv());
	if(ship)
		ship->CloseAllHullDoors();
}

void AuSDShipBoardTroops::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Pre-boarding announcement period has passed or all troop berths have been taken.
	// Reconcile the roster to clear the troops who are not aboard.
	// That way we can load the maximum number of troops.
	ship->ReconcileTroops();
	int troopBerths = ship->TroopBerthsAvailable();
	if(!troopBerths)
		return;

	// Limit the number of troop to load if the mission provides a count.
	if(m_MissionHeadCount && troopBerths > m_MissionHeadCount)
		troopBerths = m_MissionHeadCount;

	// One or more troop berths are available.
	DaAtomList troopBds;				// List of buildings with which we can trade.
	ship->FindTroopBds(troopBds);
	if(!troopBds.empty())
	{
		// Load troops into the ship.
		for(DaAtomList::const_iterator bi = troopBds.begin(); bi != troopBds.end(); bi++)
		{
			DaBd* troopBd = (DaBd*)*bi;
			// Take troops from the building to fill my empty berths.
			int troops = troopBd->RemoveTravellers(troopBerths);
			if(!troops)
				continue;
			troopBerths -= troops;		// Reduce the number of empty berths.
			QList<AuSpecies> species;
			species.append(troopBd->ResidentSpecies());
			quint32 empireId = troopBd->EmpireId();
			AuItemHash inventory;
			troopBd->CommunityInventoryAvailable(inventory, empireId);
			quint8 bodyQ;
			const DaWorldLL* world = troopBd->FindParentWorldInEnv();
			if(world)
			{
				int resourceZone = world->ResourceZone(troopBd->GlobeLL().Longitude());
				bodyQ = world->NaturalResourceQuality(CID_Animal, resourceZone);
			}
			else
				bodyQ = 128;
			for(int icol = 0; icol < troops; icol++)
			{
				DaNpc* troop = ship->BoardTroop(empireId, species, inventory, bodyQ);
				if(troop)
					troop->SetPosture(sent->Posture());	// Match posture to mine when I load them.
			}

			// Decrease the number of troop berths available on the ship.
			troopBerths -= troops;
			if(!troopBerths)
				break;
		}
	}
}

void AuSDShipSellCargoCategory::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);
	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// I can get this list once and use it over and over to save a lot of time.
	DaAtomList reachableBds;
	ship->FindReachableBds(reachableBds);
	if(reachableBds.empty())
		return;

	for(QList<COMMODITYID>::const_iterator ci = m_Commodities.cbegin(); ci != m_Commodities.cend(); ci++)
	{
		// Determine quantity of that cargo. Fuel and electricity get stored in a different place than the hold.
		int cargoInHold;
		if(*ci == CID_Hydrogen)
			cargoInHold = (int)ship->FuelQuantity();								// Total currently in fuel cells.
		else if(*ci == CID_Electricity)
			cargoInHold = (int)ship->CapacitorLevel();								// Total currently in capacitor.
		else
		{
			const AuItemHash& sinv = ship->LockCargoForRead();
			cargoInHold = sinv.RawCount(*ci);								// Total currently in hold.
			ship->UnlockCargo();
		}
		if(cargoInHold)
		{
			// Ship has cargo.
			int cargoToUnload;
			if(m_MissionAmountType == 0)
			{
				Q_ASSERT(m_MissionAmount <= 100);
				if(m_MissionAmount == 100)
					cargoToUnload = cargoInHold;
				else if(cargoInHold >= 0x7fffffff/100)
					cargoToUnload = cargoInHold / 100 * m_MissionAmount;	// Avoid integer rollover.
				else
					cargoToUnload = cargoInHold * m_MissionAmount / 100;
			}
			else
			{
				cargoToUnload = m_MissionAmount;	// Amount is a unit count
				if(cargoToUnload > cargoInHold)
					cargoToUnload = cargoInHold;
			}

			if(!cargoToUnload)
				cargoToUnload = 1;					// minimum 1
			if(cargoToUnload)
			{
				// The mission demands that I sell cargo.
				DaAtomList cargoCities;				// List of cities with which we can trade.
				ship->FindTradeBds(cargoCities, *ci, reachableBds);	// Finds airports, air bases and brokers. Checks export policy.

				// Unload cargo from the ship.
				sent->SellCargo(ship, *ci, cargoToUnload, cargoCities);
			}
		}
	}
}

void AuSDShipMissionAttack::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Insert a do-once attack order to go after the enemy ship.
	if(ship->IsMissionActive())
		ship->MissionInsertOrder(0, AuMissionOrderAttack(m_TargetId, m_TargetName, AuMissionOrder::DoOnce));
	else
	{
		// Ship mission is currently on hold. Set this up to go back to hold after attacking.
		ship->MissionInsertOrder(0, AuMissionOrderHoldMission(AuMissionOrder::DoOnce));
		ship->MissionInsertOrder(0, AuMissionOrderAttack(m_TargetId, m_TargetName, AuMissionOrder::DoOnce));
		ship->MissionSetActive(true);						// Activate my mission.
	}
}

void AuSDShipMissionOrderComplete::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	ship->MissionOrderComplete(sent, m_CompletionCode);
}

void AuSDShipSellCargoCommodity::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);
	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// The mission demands that I sell cargo.
	DaAtomList cargoCities;				// List of cities with which we can trade.
	ship->FindTradeBds(cargoCities, m_CargoCID);	// Finds airports, air bases and brokers. Checks export policy.

	// Unload cargo from the ship.
	sent->SellCargo(ship, m_CargoCID, m_CargoToUnload, cargoCities);
}

void AuSDShipBuyCargoCommodity::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);
	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// The mission demands that I buy cargo.
	DaAtomList cargoCities;		// List of cities with which we can trade.
	ship->FindTradeBds(cargoCities, m_CargoCID);	// Finds airports, air bases and brokers. Checks export policy.

	// Load cargo into the ship.
	sent->BuyCargo(ship, m_CargoCID, m_CargoToLoad, cargoCities);
}

void AuSDShipBuyCargoCategory::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);
	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// I can get this list once and use it over and over to save a lot of time.
	DaAtomList reachableBds;
	ship->FindReachableBds(reachableBds);
	if(reachableBds.empty())
		return;

	for(QList<COMMODITYID>::const_iterator ci = m_Commodities.cbegin(); ci != m_Commodities.cend(); ci++)
	{
		// Don't buy fuel or electricity because it gets stored in a different place.
		if(*ci == CID_Hydrogen || *ci == CID_Electricity)
			continue;

		// Test for desired amount of each commodity.
		const AuItemHash& sinv = ship->LockCargoForRead();
		int unitsInHold = sinv.RawCount(*ci);
		ship->UnlockCargo();
		if(unitsInHold < m_EachWanted)
		{
			// Hold does not contain enough of the commodity.
			DaAtomList cargoCities;									// List of cities with which we can trade.
			ship->FindTradeBds(cargoCities, *ci, reachableBds);		// Finds airports, air bases and brokers. Checks export policy.

			// Load cargo into the ship.
			int cargoToLoad = m_EachWanted - unitsInHold;			// Net amount wanted to load.
			if(cargoToLoad > m_SpaceAvail)
				cargoToLoad = m_SpaceAvail;
			int cargoLoaded = sent->BuyCargo(ship, *ci, cargoToLoad, cargoCities);
			m_CargoWanted -= cargoLoaded;
			m_SpaceAvail -= cargoLoaded;
			if(m_CargoWanted <= 0 || m_SpaceAvail <= 0)
				break;												// Stop loading when our hold is full.
		}
	}
}

void AuSDShipBoardCrew::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Pre-boarding announcement period has passed or all crew berths have been taken.
	// Reconcile the roster to clear the crew who are not aboard.
	// That way we can load the maximum number of crew members.
	ship->ReconcileCrew();
	int crewBerths = ship->CrewBerthsAvailable();
	if(!crewBerths)
		return;

	// Limit the number of crew to load if the mission provides a count.
	if(m_MissionHeadCount && crewBerths > m_MissionHeadCount)
		crewBerths = m_MissionHeadCount;

	// A crew berth is available.
	DaAtomList crewBds;			// List of cities with which we can trade.
	ship->FindCrewBds(crewBds);
	if(!crewBds.empty())
	{
		// Load crew into the ship.
		for(DaAtomList::const_iterator bi = crewBds.begin(); bi != crewBds.end(); bi++)
		{
			DaBd* crewBd = (DaBd*)*bi;
			// Take crew from the building to fill my empty berths.
			int crew = crewBd->RemoveTravellers(crewBerths);
			if(!crew)
				continue;
			crewBerths -= crew;		// Reduce the number of empty berths.
			QList<AuSpecies> species;
			species.append(crewBd->ResidentSpecies());
			quint32 empireId = crewBd->EmpireId();
			AuItemHash inventory;
			crewBd->CommunityInventoryAvailable(inventory, empireId);
			quint8 bodyQ;
			const DaWorldLL* world = crewBd->FindParentWorldInEnv();
			if(world)
			{
				int resourceZone = world->ResourceZone(crewBd->GlobeLL().Longitude());
				bodyQ = world->NaturalResourceQuality(CID_Animal, resourceZone);
			}
			else
				bodyQ = 128;
			for(int icol = 0; icol < crew; icol++)
			{
				DaNpc* crew = ship->BoardCrew(empireId, species, inventory, bodyQ);
				if(crew)
					crew->SetPosture(sent->Posture());	// Match posture to mine when I load them.
			}

			// Decrease the number of crew berths available on the ship.
			crewBerths -= crew;
			if(!crewBerths)
				break;
		}
	}
}

void AuSDShipBoardCryo::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);
	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	int cryoBerths = ship->CryoBerthsAvailable();
	if(!cryoBerths)
		return;

	// Limit the number of passengers to load if the mission provides a count.
	if(m_MissionHeadCount && cryoBerths > m_MissionHeadCount)
		cryoBerths = m_MissionHeadCount;

	// A passenger berth is available.
	DaAtomList passBds;		// List of cities with which we can trade.
	ship->FindPassengerLoadBds(passBds);
	if(!passBds.empty())
	{
		// Load passengers into the ship.
		for(DaAtomList::const_iterator bi = passBds.begin(); bi != passBds.end(); bi++)
		{
			DaBd* passBd = (DaBd*)*bi;

			// Determine the number of colonists ready to leave the city.
			int colonists = passBd->ResidentsReadyToDepart(false);
			if(colonists <= 0)
				continue;	// City has insufficient population.
			if(colonists > cryoBerths)
				colonists = cryoBerths;

			// Transfer population from the city to the ship's roster.
			// Remove citizens from the city.
			passBd->RemoveTravellers(colonists);

			// Put them in cryo berths on the ship.
			AuSpecies species = passBd->ResidentSpecies();
			for(int icol = 0; icol < colonists; icol++)
				ship->BoardCryoTraveller(species);

			// Decrease the number of cryo berths available on the ship.
			cryoBerths -= colonists;
			if(!cryoBerths)
				return;
		}
	}
}

void AuSDShipBoardPassengers::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);
	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Preboarding time has finished.
	// Reconcile the roster to clear the passengers who are not aboard.
	// That way we can load the maximum number of paying passengers.
	ship->ReconcilePassengers();
	int passBerths = ship->PassengerBerthsAvailable();
	if(!passBerths)
		return;

	// Limit the number of passengers to load if the mission provides a count.
	if(m_MissionHeadCount && passBerths > m_MissionHeadCount)
		passBerths = m_MissionHeadCount;

	// A passenger berth is available.
	DaAtomList passBds;		// List of cities with which we can trade.
	ship->FindPassengerLoadBds(passBds);
	if(!passBds.empty())
	{
		// Load passengers into the ship.
		for(DaAtomList::const_iterator bi = passBds.begin(); bi != passBds.end(); bi++)
		{
			DaBd* passBd = (DaBd*)*bi;
			if(passBd->IsMilitary())
				continue;	// Cannot load passengers from military buildings.

			// Determine the number of colonists ready to leave the city.
			int colonists = passBd->ResidentsReadyToDepart(false);
			if(colonists <= 0)
				continue;	// City has insufficient population.
			if(colonists > passBerths)
				colonists = passBerths;

			// Transfer population from the city to the ship's roster.
			// Remove citizens from the city.
			QList<AuSpecies> species;
			species.append(passBd->ResidentSpecies());
			passBd->RemoveTravellers(colonists);

			// Put them in cryo berths on the ship.
			quint32 empireId = passBd->EmpireId();
			AuItemHash inventory;
			passBd->CommunityInventoryAvailable(inventory, empireId);
			quint8 bodyQ;
			const DaWorldLL* world = passBd->FindParentWorldInEnv();
			if(world)
			{
				int resourceZone = world->ResourceZone(passBd->GlobeLL().Longitude());
				bodyQ = world->NaturalResourceQuality(CID_Animal, resourceZone);
			}
			else
				bodyQ = 128;
			for(int icol = 0; icol < colonists; icol++)
				ship->BoardColonist(passBd->Id(), world, empireId, species, inventory, bodyQ);

			// Decrease the number of cryo berths available on the ship.
			passBerths -= colonists;
			if(!passBerths)
				return;
		}
	}
}

void AuSDShipDebarkCryo::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	DaAtomList passBds;		// List of cities with which we can trade.
	ship->FindPassengerUnloadBds(passBds);
	if(!passBds.empty())
	{
		// Unload cryo travellers from the ship.
		for(DaAtomList::const_iterator bi = passBds.begin(); bi != passBds.end(); bi++)
		{
			DaBd* passBd = (DaBd*)*bi;
			DaWorld* planet = passBd->FindParentPlanetInEnv();
			if(!planet)
				continue;

			quint16 emptyHomes = passBd->EmptyHomes();
			if(!emptyHomes)
				continue;
			if(emptyHomes > m_CryoBerths)
				emptyHomes = m_CryoBerths;

			// Take them from cryo berths on the ship.
			unsigned int debarked = 0;
			for(int icol = 0; icol < emptyHomes; icol++)
			{
				AuSpecies species = ship->RemoveAnyCryoBerthOccupant();
				if(species.IsNull())
					continue;
				passBd->TravellerMovedIn(species);	// Transfer species,population to this city.
				debarked++;
			}

			m_CryoBerths -= debarked;
			if(!m_CryoBerths)
				break;
		}
	}
}

void AuSDShipDebarkPassengers::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Determine transport rates based on trip length.
	int longTripRate = 300;						// Travel to another solar system.
	int medTripRate = 30;						// Travel to another world.
	int shortTripRate = 3;						// Travel to another city.
	if(ship->IsFirstClass())
	{
		// First class accomodations earn triple fees.
		longTripRate *= 3;
		medTripRate *= 3;
		shortTripRate *= 3;
	}
	else if(ship->IsSecondClass())
	{
		// Second class accomodations earn double fees.
		longTripRate *= 2;
		medTripRate *= 2;
		shortTripRate *= 2;
	}

	DaAtomList passBds;		// List of cities with which we can trade.
	ship->FindPassengerUnloadBds(passBds);
	if(!passBds.empty())
	{
		// Unload passengers from the ship.
		DaAtom* system = ship->FindEnvironmentAtom();
		for(DaAtomList::const_iterator bi = passBds.begin(); bi != passBds.end(); bi++)
		{
			DaBd* passBd = (DaBd*)*bi;
			DaWorld* planet = passBd->FindParentPlanetInEnv();
			if(!planet)
				continue;

			quint16 emptyHomes = passBd->EmptyHomes();
			if(!emptyHomes)
				continue;
			if(emptyHomes > m_Passengers)
				emptyHomes = m_Passengers;

			// Take them from passenger berths on the ship.
			// Calculate payment due for delivering the colonists.
			QString sourceName = ship->NameWithEmpire();	// For disease outbreak event message.
			unsigned int transportFee = 0;	// accumulated
			unsigned int debarked = 0;
			for(int icol = 0; icol < emptyHomes; icol++)
			{
				DaNpc* npc = ship->FindColonist();
				if(!npc)
					break;
				AuId originCity(npc->PortBd());			// Get the passenger's bd of origin.
				ship->RemoveBerthOccupant(npc->Id());	// Release the colonist's berth.
				passBd->TravellerMovedIn(npc, sourceName);	// Transfer disease,dna,population to this city.
				ship->KillColonist();					// Remove a colonist atom from the ship.
				debarked++;

				if(!passBd->Is(originCity))
				{
					// Colonist was dropped off in a city that is not their city of origin. A fee is due.
					if(planet->FindSubatomId(originCity))
					{
						// The city of origin is on this planet.
						transportFee += shortTripRate;

						// Passengers take 1000c with them when they leave their city of origin.
						// Put their remaining money into their nu city's citizen fund.
						passBd->CreditAccount(1000 - shortTripRate, AuBank::Civ_CreditImmigration);
					}
					else if(system->FindIdRecursive(originCity))
					{
						// The city of origin is in this solar system.
						// City could be up to five levels below the
						// solar system.
						// 0 - Solar System
						//   1 - Primary Star
						//     2 - Secondary or Tertiary Star
						//       3 - Planet
						//         4 - Moon
						//           5 - City
						transportFee += medTripRate;

						// Passengers take 1000c with them when they leave their city of origin.
						// Put their remaining money into their nu city's citizen fund.
						passBd->CreditAccount(1000 - medTripRate, AuBank::Civ_CreditImmigration);
					}
					else
					{
						// The city of origin is NOT in this system.
						transportFee += longTripRate;

						// Passengers take 1000c with them when they leave their city of origin.
						// Put their remaining money into their nu city's citizen fund.
						passBd->CreditAccount(1000 - longTripRate, AuBank::Civ_CreditImmigration);
					}
				}
				else
				{
					// Put their money back into their origin city's citizen fund.
					passBd->CreditAccount(1000, AuBank::Civ_CreditImmigration);
				}
			}

			// Pay the ship for debarking the passengers.
			ship->CreditAccountSales(transportFee);

			m_Passengers -= debarked;
			if(!m_Passengers)
				break;
		}
	}
}

void AuSDShipReconcileRoster::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Reconcile the roster.
	ship->ReconcileRoster();

	/* This makes it too hard to stow away anywhere.
	// Kick stowaways and log the names of those that were kicked.
	QStringList stowaways;
	ship->KickStowaways(stowaways, true);
	for(QStringList::const_iterator si = stowaways.cbegin(); si != stowaways.cend(); si++)
		ship->LogEvent(AuEventLogEntry::KickedStowaway, *si);
		*/
}

void AuSDShipCrewPosture::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// All officers (me), crew and troops aboard.
	ship->SetCrewPosture(m_Posture);
}

void AuSDShipPilotPolicy::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	ship->SetPilotUpPolicy(m_UpPolicy);
	ship->SetModified();
}

void AuSDShipShieldsPolicy::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	ship->SetShieldFuelPolicy(m_FuelPolicy);
	ship->SetShieldUpPolicy(m_UpPolicy);
	ship->SetModified();
}

void AuSDShipMissionState::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	ship->MissionSetActive(m_Active);
	ship->SetModified();
}

void AuSDShipAnnounceOrder::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
		sent->AnnounceMissionOrder(ship);
}

void AuSDShipScuttle::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
		ship->MissionScuttle(3 * 60);
}

/*
void AuSDShipAddWaypoint::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
	{
		ship->AddWaypoint(m_DestSystem, m_DestAtom, m_DestOffset);
		
		switch(m_Announcement)
		{
		default:
		case 0:
			break;
		case 1:
			{
				AuCommChat msg(QObject::tr("Target spacecraft wormhole drive is hot. Navigating through wormhole to pursue."),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Grim);
			}
			break;
		case 2:
			{
				AuCommChat msg(QObject::tr("Plotting course to next wormhole on patrol."),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 3:
			{
				const DaSolarSystem* solar = sent->FindParentSolarSystem();
				if(solar)
				{
					AuCommChat msg(QObject::tr("Plotting course to approach the primary star of %1.").arg(solar->Name()),
							AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
							sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
					sent->Say(msg, AuVisage::Closed);
				}
			}
			break;
		case 4:
			{
				AuCommChat msg(QObject::tr("Plotting course direct to coordinates."),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close,
						AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 5:
			{
				const DaGalaxy* galaxy = sent->FindParentGalaxy();
				if(galaxy)
				{
					const DaSolarSystem* dest = dynamic_cast<const DaSolarSystem*>(galaxy->FindIdRecursive(m_DestAtom));
					if(dest)
					{
						AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
									"This ship does not have a wormhole FTL drive. "
									"Course laid direct to %1 instead.").arg(dest->NameKnownTo(ship->EmpireId())),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Perplexed);
					}
				}
			}
			break;
		case 6:
			{
				const DaGalaxy* galaxy = sent->FindParentGalaxy();
				if(galaxy)
				{
					const DaSolarSystem* dest = dynamic_cast<const DaSolarSystem*>(galaxy->FindIdRecursive(m_DestAtom));
					if(dest)
					{
						AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
									"The ship's capacitor is inadequate to engage wormhole FTL drive. "
									"Course laid direct to %1 instead.").arg(dest->NameKnownTo(ship->EmpireId())),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Perplexed);
					}
				}
			}
			break;
		case 7:
			{
				const DaGalaxy* galaxy = sent->FindParentGalaxy();
				if(galaxy)
				{
					const DaSolarSystem* dest = dynamic_cast<const DaSolarSystem*>(galaxy->FindIdRecursive(m_DestAtom));
					if(dest)
					{
						AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
									"The ship's wormhole FTL drive is too badly damaged to engage. "
									"Course laid direct to %1 instead.").arg(dest->NameKnownTo(ship->EmpireId())),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Perplexed);
					}
				}
			}
			break;
		case 8:
			{
				const DaGalaxy* galaxy = sent->FindParentGalaxy();
				if(galaxy)
				{
					const DaSolarSystem* dest = dynamic_cast<const DaSolarSystem*>(galaxy->FindIdRecursive(m_DestAtom));
					if(dest)
					{
						AuCommChat msg(QObject::tr("Course laid direct to %1.").arg(dest->NameKnownTo(ship->EmpireId())),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
				}
			}
			break;
		case 9:
			{
				const DaSolarSystem* solar = sent->FindParentSolarSystem();
				if(solar)
				{
					const DaAtom* destPlanet = solar->FindIdRecursive(m_DestAtom);
					if(destPlanet)
					{
						AuCommChat msg(QObject::tr("Plotting course to planet %1.").arg(destPlanet->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
				}
			}
			break;
		case 10:
			{
				const DaSolarSystem* solar = sent->FindParentSolarSystem();
				if(solar)
				{
					const DaAtom* destPlanet = solar->FindIdRecursive(m_DestAtom);
					const DaAtom* destAtom = solar->FindIdRecursive(m_AnnounceAtom);
					if(destPlanet && destAtom)
					{
						AuCommChat msg(QObject::tr("Plotting course to %1 at %2.").arg(destAtom->Name()).arg(destPlanet->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
					else if(destPlanet)
					{
						AuCommChat msg(QObject::tr("Plotting course to %1.").arg(destPlanet->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
				}
			}
			break;
		case 11:
			{
				const DaSolarSystem* solar = sent->FindParentSolarSystem();
				if(solar)
				{
					const DaAtom* destAtom = solar->FindIdRecursive(m_DestAtom);
					if(destAtom)
					{
						AuCommChat msg(QObject::tr("Plotting course to approach star %1.").arg(destAtom->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close,
								AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
				}
			}
			break;
		case 12:
			{
				const DaSolarSystem* solar = sent->FindParentSolarSystem();
				if(solar)
				{
					const DaAtom* destAtom = solar->FindIdRecursive(m_DestAtom);
					if(destAtom)
					{
						AuCommChat msg(QObject::tr("Plotting course to %1.").arg(destAtom->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
				}
			}
			break;
		case 13:
			{
				const DaSolarSystem* solar = sent->FindParentSolarSystem();
				if(solar)
				{
					const DaAtom* destAtom = solar->FindIdRecursive(m_DestAtom);
					if(destAtom)
					{
						AuCommChat msg(QObject::tr("Plotting course to star %1.").arg(destAtom->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close,
								AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
				}
			}
			break;
		case 14:
			{
				const DaSolarSystem* solar = sent->FindParentSolarSystem();
				if(solar)
				{
					const DaAtom* destStar = solar->FindIdRecursive(m_DestAtom);
					const DaAtom* destAtom = solar->FindIdRecursive(m_AnnounceAtom);
					if(destStar && destAtom)
					{
						AuCommChat msg(QObject::tr("Plotting course to %1 at star %2.").arg(destAtom->Name()).arg(destStar->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
					else if(destStar)
					{
						AuCommChat msg(QObject::tr("Plotting course to %1.").arg(destStar->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Closed);
					}
				}
			}
			break;
		case 15:
			{
				const DaGalaxy* galaxy = sent->FindParentGalaxy();
				if(galaxy)
				{
					const DaAtom* dest = galaxy->FindIdRecursive(m_DestAtom);
					if(dest)
					{
						AuCommChat msg(QObject::tr("Course laid to %1 in pursuit of target.").arg(dest->Name()),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Grim);
					}
				}
			}
			break;
		}
	}
}

void AuSDShipAddWaypointRoute::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
	{
		ship->AddWaypointRouteTo(m_DestAtom, m_DestOffset);
		if(m_AnnounceResult)
		{
			const DaGalaxy* galaxy = sent->FindParentGalaxy();
			if(galaxy)
			{
				const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
				const DaSolarSystem* dest = dynamic_cast<const DaSolarSystem*>(galaxy->FindIdRecursive(m_DestAtom));
				if(system && dest)
				{
					if(ship->HasWaypoint())
					{
						// The first waypoint is at the wormhole.
						// Insure the wormhole is not blocked.
						const DeWaypoint* wpnext = ship->LockWaypoint(0);
						AuId wpSystem(wpnext->m_idSystem);
						AuPoint wpDestOff(wpnext->DestinationOffset());
						ship->UnlockWaypoint();
						const DaAtom* sunOrPlanet = system->WormholeBlocked(wpDestOff);
						if(sunOrPlanet)
						{
							// Wormhole is blocked. Remove the waypoint and the next/only waypoint is the destination system.
							ship->RemoveWaypoint(0);

							AuCommChat msg(QObject::tr("Wormhole is blocked by %2. Course laid direct to wormhole destination "
										"system instead.")
									.arg(sunOrPlanet->Name()),
									AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
									galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
							sent->Say(msg, AuVisage::Perplexed);
						}
						else
						{
							AuCommChat msg(QObject::tr("Course laid to %1 via wormholes.").arg(dest->NameKnownTo(ship->EmpireId())),
									AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
									galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
							sent->Say(msg, AuVisage::Closed);
						}
					}
					else
					{
						ship->RemoveAllWaypoints();
						ship->AddWaypoint(dest->Id(), dest->Id());
						AuCommChat msg(QObject::tr("Unable to plot a course to %1 via wormholes. "
									"Course laid direct to destination system instead. ")
								.arg(dest->NameKnownTo(ship->EmpireId())),
								AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
								galaxy->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
						sent->Say(msg, AuVisage::Perplexed);
					}
				}
			}
		}
	}
}

void AuSDShipRemoveAllWaypoints::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
	{
		ship->RemoveAllWaypoints();
		
		switch(m_Announce)
		{
		case 0:
		default:
			break;
		case 1:
			{
				AuCommChat msg(QObject::tr("We are in orbital proximity of %1.").arg(m_AnnounceName),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 2:
			{
				AuCommChat msg(QObject::tr("We are in orbit at %1.").arg(m_AnnounceName),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 3:
			{
				AuCommChat msg(QObject::tr("We are orbiting %1.").arg(m_AnnounceName),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 4:
			{
				AuCommChat msg(QObject::tr("We are in orbital proximity of %1, the ring's parent planet.").arg(m_AnnounceName),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 5:
			{
				AuCommChat msg(QObject::tr("We are in close proximity to %1.").arg(m_AnnounceName),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 6:
			{
				AuCommChat msg(QObject::tr("We are orbiting the photon sphere of the %1.").arg(m_AnnounceName),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Grim);
			}
			break;
		case 7:
			{
				AuCommChat msg(QObject::tr("We are orbiting the photosphere of %1.").arg(m_AnnounceName),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Grim);
			}
			break;
		}
	}
}

void AuSDShipRemoveWaypoint::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
	{
		ship->RemoveWaypoint(0);

		switch(m_Announce)
		{
		case 0:
		default:
			break;
		case 1:
			{
				AuCommChat msg(QObject::tr("We are within 500m of nav coordinates."),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 2:
			{
				AuCommChat msg(QObject::tr("We are approaching the outer reaches of the destination system."),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		case 3:
			{
				AuCommChat msg(QObject::tr("We have arrived in the destination system."),
						AuCommStdChannel::CrewChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
						sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				sent->Say(msg, AuVisage::Closed);
			}
			break;
		}
	}
}
*/

void AuSDShipExterminateRats::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	DaAtom* tradeCity = 0;
	DaAtomList tradeCities;								// List of cities that can exterminate rats.
	ship->FindTradeBds(tradeCities, CID_Sunlight);		// Finds airports and air bases. Checks export policy.
	if(!tradeCities.empty())
		tradeCity = tradeCities.first();
	if(!tradeCity)
	{
		QString text = QObject::tr("No building or city is in range that can provide spacecraft services.");
		AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
				sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		sent->Say(msg, AuVisage::Perplexed);
		return;
	}

	if(ship->IsPrivatelyOwned())
	{
		// Private ship must pay for exterminator.
		if(ship->AccountBalance() < 100)
			return;											// Ship cannot afford the exterminator or cleanup crew.
		else
		{
			ship->DebitAccount(100);
			if(tradeCity)
				tradeCity->CommunityCreditAccount(100, AuBank::Civ_CreditSales);		// Collects sales tax appropriately.
		}
	}

	// Exterminate rats. Report the result.
	int result = ship->ExterminateRats();
	QString text = QObject::tr("The exterminator is done. ");

	if((result & 0x03) == 0x03)
		text += QObject::tr("They killed the rats and removed a nest. ");
	else if(result & 0x01)
		text += QObject::tr("A nest was removed but they didn't find any rats. ");
	else if(result & 0x02)
		text += QObject::tr("They killed some rats but no nest was found. ");

	bool doclean = true;
	if(ship->IsPrivatelyOwned())
	{
		// Private ship must pay for house keepers.
		if(ship->AccountBalance() >= 100)
		{
			doclean = true;
			ship->DebitAccount(100);
			if(tradeCity)
				tradeCity->CommunityCreditAccount(100, AuBank::Civ_CreditSales);		// Collects sales tax appropriately.
		}
	}
	if(doclean)
	{
		if(ship->CleanHouse())
			text += QObject::tr("The cleanup crew found some food that was attracting the critters; I think they ate it.");
		else
			text += QObject::tr("The cleanup crew came through like a white tornado. "
					"There is nothing left around that a rat might eat.");
	}

	AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
			sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
	sent->Say(msg, AuVisage::Closed);
}

void AuSDShipTransporterState::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaCapsule* capsule = dynamic_cast<DaCapsule*>(sent->FindParentCapsuleInEnv());
	if(capsule)
	{
		if(m_On)
			capsule->TurnTransporterSystemOn(m_RoomIndex);
		else
			capsule->TurnTransporterSystemOff(m_RoomIndex);
	}
}

void AuSDShipPatientDefib::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaCapsule* capsule = dynamic_cast<DaCapsule*>(sent->FindParentCapsuleInEnv());
	if(capsule)
	{
		DaCreature* patient = dynamic_cast<DaCreature*>(capsule->FindSubatomId(m_PatientId));
		if(patient)
		{
			QString text = QObject::tr("%1 is in cardiac arrest. Defibrillator ready! CLEAR!").arg(patient->Name());
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
					sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			sent->Say(msg, AuVisage::Grim);

			DpStationSurgery::DoDefibrillate(sent, patient, *capsule);
		}
	}
}

void AuSDShipPatientHeal::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaCapsule* capsule = dynamic_cast<DaCapsule*>(sent->FindParentCapsuleInEnv());
	if(capsule)
	{
		DaCreature* patient = dynamic_cast<DaCreature*>(capsule->FindSubatomId(m_PatientId));
		if(patient)
		{
			QString text = QObject::tr("%1, hold still while I bandage your wounds.").arg(patient->Name());
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), sent->Id(), sent->Name(), sent->FindParentSector()->Id(),
					sent->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			sent->Say(msg, AuVisage::Grim);

			const AuItemHash& sinv = capsule->LockCargoForRead();
			AuItemHash::const_iterator ci = sinv.FindBest(CID_MedicalKit);
			if(ci != sinv.end())
			{
				AuItem* medKit = (*ci).second->Clone();
				capsule->UnlockCargo();
				DpStationSurgery::DoHeal(sent, patient, *capsule, *medKit);
				delete medKit;
			}
			else
				capsule->UnlockCargo();
		}
	}
}

void AuSDShipPatientTreat::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaCapsule* capsule = dynamic_cast<DaCapsule*>(sent->FindParentCapsuleInEnv());
	if(capsule)
	{
		DaCreature* patient = dynamic_cast<DaCreature*>(capsule->FindSubatomId(m_PatientId));
		if(patient)
			sent->TreatPatientConditions(patient, capsule);
	}
}

void AuSDShipPilotReportContacts::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
		sent->ReportEnemyWeaponContact(ship);
}

void AuSDShipSensorsReportContacts::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
		sent->ReportEnemySensorContact(ship, m_SensorBlips, m_SensorRange);
}

void AuSDShipWeaponFire::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	// We have found a target in range to shoot at.
	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Insure weapon bay has valid ammo selected.
	if(!m_IsHarvester && ship->SelectBayWeapon(m_Which))
	{
		// New ammo was selected into my weapon bay. It takes me a moment to react.
		sent->StayIdle();
		return;
	}

	// Weapon has valid ammo selected or it doesn't need ammo.
	DaAtom* env = sent->FindEnvironmentAtom();
	const DaAtom* targetAtom = env->FindIdRecursive(m_TargetId);
	if(!targetAtom)
		return;
	if(targetAtom->IsNoncombatant())
		return;		// Cannot fire at noncombatants.

	ship->FireBayWeapon(*sent, m_Which, m_UnitsFired, m_HarvestIndex, targetAtom, m_TargetSysLoc);

	// Cycle to the next bay when firing a weapon bay from the bridge. There may only be a bridge fire control console.
	// Don't cycle to a harvester. Loop until we find a non-harvester. We might end up back on the same one; that's ok.
	if(!m_IsHarvester && sent->IsInsideSpacecraft() && sent->ShipRoomType() == ROOM_Bridge)
	{
		DpStationWeapon* console = dynamic_cast<DpStationWeapon*>(sent->LockConsoleForRead());
		if(console)
		{
			bool isHarvester;
			do
			{
				console->SelectNextBay(*ship);
				COMMODITYID module = ship->SystemModule(SYS_Weapon, console->SystemWhich());
				isHarvester = DbCommodity::IsHarvesterModule(module);
			}
			while(isHarvester);
			if(sent->ControlId() != console->SystemWhich())
			{
				// The system monitored/controlled by my console changed.
				sent->m_ControlId = console->SystemWhich();
				sent->SetModified();
			}
		}
		sent->UnlockConsole();
	}
}

void AuSDShipWeaponSetPower::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(ship)
	{
		// Fire at 100% power.
		ship->SetBayWeaponPowerSetting(m_Which, 40);
		ship->SetModified();
	}
}

void AuSDShipLootShip::Execute(DaSentient* sent, int msElapsed)
{
	Q_UNUSED(msElapsed);

	DaVessel* ship = dynamic_cast<DaVessel*>(sent->FindParentVesselInEnv());
	if(!ship)
		return;

	// Weapon has valid ammo selected or it doesn't need ammo.
	DaAtom* env = sent->FindEnvironmentAtom();
	DaVessel* victimShip = dynamic_cast<DaVessel*>(env->FindIdRecursive(m_VictimShipId));
	if(!victimShip)
		return;

	const DaSector* sector = ship->FindParentSector();
	if(!sector)
		return;

	int cargoSpace = ship->CargoCapacityVolume() - ship->CargoTotalVolume();
	int passBerths = ship->PassengerBerths();

	QString text;
	if(passBerths && cargoSpace)
		text = QObject::tr("%1, we only want cash, cargo and passengers. "
				"Obey orders and your crew will be spared.").arg(victimShip->NameWithEmpire());
	else if(passBerths)
		text = QObject::tr("%1, we only want cash and passengers. "
				"Obey orders and your crew will be spared.").arg(victimShip->NameWithEmpire());
	else if(cargoSpace)
		text = QObject::tr("%1, we only want cash and cargo. "
				"Obey orders and your crew will be spared.").arg(victimShip->NameWithEmpire());
	else
		text = QObject::tr("%1, we only want cash. "
				"Obey orders and your crew will be spared.").arg(victimShip->NameWithEmpire());
	//qDebug() << text;
	AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
			sector->Id(), sent->Galaxy(), AuCommStdChannel::SolarSystem,
			AuCommStdChannel::Everyone);
	sent->Say(msg, AuVisage::Grim);

	// Transfer loot from the target.
	{
		double money = victimShip->AccountBalance();
		victimShip->DebitAccount(money);
		ship->CreditAccountSales(money);	// Can anyone say "privateer"?

		// Announce what we take.
		QString text;
		if(money < 10000)
			text = QObject::tr("%1, that's all you've got? Poor sap.")
				.arg(QString("%L1%2").arg(money, 0,'f',0).arg(g_Au.CurrencySymbol()));
		else if(money < 100000)
			text = QObject::tr("%1, not bad.")
				.arg(QString("%L1%2").arg(money, 0,'f',0).arg(g_Au.CurrencySymbol()));
		else if(money < 1000000)
			text = QObject::tr("%1 we're talking.")
				.arg(QString("%L1%2").arg(money, 0,'f',0).arg(g_Au.CurrencySymbol()));
		else if(money < 10000000)
			text = QObject::tr("%1, call me megabucks!")
				.arg(QString("%L1%2").arg(money, 0,'f',0).arg(g_Au.CurrencySymbol()));
		else
			text = QObject::tr("WHOA! %1, lets party!")
				.arg(QString("%L1%2").arg(money, 0,'f',0).arg(g_Au.CurrencySymbol()));
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
				sector->Id(), sent->Galaxy(), AuCommStdChannel::SolarSystem,
				AuCommStdChannel::Everyone);
		sent->Say(msg, AuVisage::Grim);
	}

	// Transfer passengers from the target ship.
	for(int passIdx=0; passIdx < passBerths; passIdx++)
	{
		DaNpc* npc = victimShip->FindColonist();
		if(!npc)
			break;						// Target ship has no more passengers.
		// Change the npc's berth assignment from the old ship to the nu ship.
		victimShip->RemoveBerthOccupant(npc->Id());
		ship->OccupyBerth(AuRosterEntry::PassengerAI, npc->Id(), npc->Name());
		npc->AssignToShip(ship->Id(), AuRosterEntry::PassengerAI);

		// Physically move the npc to their berth on the nu ship.
		AuMatrix4x4 t;
		t.Translate(ship->BerthLocation(npc->Id()) + AuPoint(0,0,npc->CGHeightStanding()));
		npc->ChangePosAndOrient(AuAtomPos(ship->Id(), t));
		npc->InvalidateEnvironment();
		npc->Stand();
		npc->SetModified();

		// Each npc cries out when taken.
		QString outcry = npc->OutcryCapturedByPirates();
		AuCommChat msg(outcry, AuCommStdChannel::HailChannelName(), npc->Id(), npc->Name(),
				sector->Id(), sent->Galaxy(), AuCommStdChannel::SolarSystem,
				AuCommStdChannel::Everyone);
		sent->Say(msg, AuVisage::Sad);
	}

	// Transfer cryo passengers.
	quint32 cryoBerths = ship->CryoBerthsAvailable();
	quint32 cryoPass = ship->CryoBerthsOccupied();
	if(cryoBerths > cryoPass)
		cryoBerths = cryoPass;	// We can't take more than they have.
	if(cryoBerths)
	{
		QString sayChoices[] =
		{
			QObject::tr("We took %1 cryo passengers. A nearby cantina pays cash for them, extra if they can be revived. "),
			QObject::tr("We took %1 cryo passengers. The university research laboratory buys them. "),
			QObject::tr("We took %1 cryo passengers. Tell the chef to thaw some of that frozen meat for dinner. "),
			QObject::tr("We took %1 cryo passengers. Revive a female and send her to my cabin. "),
			QObject::tr("We took %1 cryo passengers. Revive a male and send him to my cabin. "),
			QObject::tr("We took %1 cryo passengers. Revive one to clean my cabin. No slugs this time. That was a mess. "),
			QObject::tr("We took %1 cryo passengers. They make great jerky, with a little teriyaki sauce. "),
			QObject::tr("We took %1 cryo passengers. Revive one and make it clean up this place. "
					"The bathroom needs some serious attention. "),
			QObject::tr("We took %1 cryo passengers. It's our lucky day. We just ran out of them. "),
		};
		int choiceCount = sizeof(sayChoices) / sizeof(QString);
		int sayChoice = g_Au.Dice().Roll(1,choiceCount,-1);
		QString text = sayChoices[sayChoice].arg(cryoBerths);
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(), sector->Id(), sent->Galaxy(),
				AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
		sent->Say(msg, AuVisage::Grim);
		while(cryoBerths--)
		{
			AuSpecies species = victimShip->RemoveAnyCryoBerthOccupant();
			if(species.IsNull())
				break;
			ship->BoardCryoTraveller(species);
		}
	}

	// Transfer cargo from the target ship.
	// Make a list of the items we might take.
	int take = 0;
	AuItemHash cargoTake;
	const AuItemHash& vsinv = victimShip->LockCargoForRead();
	for(AuItemHash::const_iterator ci = vsinv.begin(); ci != vsinv.end(); ci++)
	{
		if((int)(take + (*ci).second->Count()) < cargoSpace)
		{
			cargoTake.insert(pair<COMMODITYID, AuItem*>((*ci).first, (*ci).second->Clone()));
			take += (*ci).second->Count();
		}
	}
	victimShip->UnlockCargo();

	// Take as many of those cargo items as possible.
	for(AuItemHash::const_iterator iii = cargoTake.begin(); iii != cargoTake.end() && cargoSpace > 0; iii++)
	{
		// Create an object to represent the cargo we are transferring.
		AuItem* cargo = (*iii).second->Clone();
		if((int)cargo->Count() > cargoSpace)
			cargo->SetCount(cargoSpace);

		// If my ship can accept the cargo, attempt to remove it from the other ship.
		if(ship->CanAddCargo(*cargo) && victimShip->RemoveCargo(*cargo, cargo->Count()))
		{
			ship->AddCargo(*cargo);	// Add the cargo to my ship.
			cargoSpace -= cargo->Count();

			// Announce what we take.
			QString text = QObject::tr("Thanks for the %1.").arg(cargo->SayName());
			AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
					sector->Id(), sent->Galaxy(), AuCommStdChannel::SolarSystem,
					AuCommStdChannel::Everyone);
			sent->Say(msg, AuVisage::Grim);
		}

		delete cargo;
		cargo = 0;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaSentient::DaSentient(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, quint32 empire,
		GROWTHSTAGE stage) :
	DaCreature(dna, chroma, hand, name, empire, stage)
{
	m_AIIsAlert = false;
	m_AIPosture = POSTURE_DefendSelf;
	m_AIScRoute = 0;
	m_AIStrategy = 0;
	m_AIThinkRate = 250;
	m_DecisionCount = 0;
	m_DecisionTable = 0;
	m_EnemyBlips = 0;
	m_InEnemyRange = false;
	m_MoveDistLimit = -1;
	m_RecentMemory = MEM_None;
	m_TurnZ = 0;
}

DaSentient::DaSentient(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, GROWTHSTAGE stage) :
	DaCreature(dna, chroma, hand, name, stage)
{
	m_AIIsAlert = false;
	m_AIPosture = POSTURE_DefendSelf;
	m_AIScRoute = 0;
	m_AIStrategy = 0;
	m_AIThinkRate = 250;
	m_DecisionCount = 0;
	m_DecisionTable = 0;
	m_EnemyBlips = 0;
	m_InEnemyRange = false;
	m_MoveDistLimit = -1;
	m_RecentMemory = MEM_None;
	m_TurnZ = 0;
}

DaSentient::DaSentient(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
		GROWTHSTAGE stage) : DaCreature(species, chroma, hand, name, stage)
{
	m_AIIsAlert = false;
	m_AIPosture = POSTURE_DefendSelf;
	m_AIScRoute = 0;
	m_AIStrategy = 0;
	m_AIThinkRate = 250;
	m_DecisionCount = 0;
	m_DecisionTable = 0;
	m_EnemyBlips = 0;
	m_InEnemyRange = false;
	m_MoveDistLimit = -1;
	m_RecentMemory = MEM_None;
	m_TurnZ = 0;
}

DaSentient::DaSentient()
{
	m_AIIsAlert = false;
	m_AIPosture = POSTURE_DefendSelf;
	m_AIScRoute = 0;
	m_AIStrategy = 0;
	m_AIThinkRate = 250;
	m_DecisionCount = 0;
	m_DecisionTable = 0;
	m_EnemyBlips = 0;
	m_InEnemyRange = false;
	m_MoveDistLimit = -1;
	m_RecentMemory = MEM_None;
	m_TurnZ = 0;
}

DaSentient::~DaSentient()
{
	if(m_DecisionTable)
	{
		for(int dt=0; dt<AuSentientDecision::D_COUNT; dt++)
			if(m_DecisionTable[dt])
				delete m_DecisionTable[dt];
		delete [] m_DecisionTable;
	}
	if(m_AIScRoute)
		delete m_AIScRoute;
}

DaSentient::DaSentient(const DaSentient& c) : DaCreature(c)
{
	m_RecentMemory = c.m_RecentMemory;

	c.m_SentientLock.lockForRead();
	m_AIRecentCollide = c.m_AIRecentCollide;
	m_AIRecentParent = c.m_AIRecentParent;
	m_AIRecentPath = c.m_AIRecentPath;
	m_AIScRoute = 0;
	m_AIStrategy = c.m_AIStrategy;
	m_AIStrategyTime = c.m_AIStrategyTime;
	m_EnemyBlips = c.m_EnemyBlips;
	m_InEnemyRange = c.m_InEnemyRange;
	m_LastPlunder = c.m_LastPlunder;
	m_LastShipName = c.m_LastShipName;
	m_MoveDistLimit = c.m_MoveDistLimit;
	m_tmLastLook = c.m_tmLastLook;
	m_tmLastScan = c.m_tmLastScan;
	m_tmLastThought = c.m_tmLastThought;
	c.m_SentientLock.unlock();

	c.m_SkillsLock.lockForRead();
	m_Skills = c.m_Skills;
	m_SkillsChg = c.m_SkillsChg;
	c.m_SkillsLock.unlock();

	c.m_TalkLock.lockForRead();
	m_Interlocutor = c.m_Interlocutor;
	m_JobOffer = c.m_JobOffer;
	m_LastSaid = c.m_LastSaid;
	m_tmLastSpoke = c.m_tmLastSpoke;
	c.m_TalkLock.unlock();

	c.m_ThinkLock.lockForRead();
	m_AIAttackTime = c.m_AIAttackTime;
	m_AIThinkTime = c.m_AIThinkTime;
	m_AIIsAlert = c.m_AIIsAlert;
	m_AIOrders = c.m_AIOrders;
	m_AIPosture = c.m_AIPosture;
	m_AIThinkRate = c.m_AIThinkRate;
	m_DecisionCount = 0;
	m_TurnZ = c.m_TurnZ;
	c.m_ThinkLock.unlock();

	// Do no read decision table from rhs.
	m_DecisionTable = 0;
}

void DaSentient::operator>>(QDataStream& out) const
{
	DaCreature::operator>>(out);
	m_ThinkLock.lockForRead();
	out << QTime();
	out << m_AIIsAlert;
	out << m_AIOrders;
	out << (quint8)m_AIPosture;
	out << m_AIThinkRate;
	out << QTime();
	out << m_TurnZ;
	m_ThinkLock.unlock();

	m_SentientLock.lockForRead();
	out << m_AIRecentCollide;
	out << m_AIRecentParent;
	out << m_AIRecentPath;
	if(m_AIScRoute)
	{
		out << (quint8)1;
		out << *m_AIScRoute;
	}
	else
		out << (quint8)0;
	out << m_AIStrategy;
	QTime aiStrategyTime;
	out << aiStrategyTime;
	out << m_EnemyBlips;
	out << m_InEnemyRange;
	out << m_LastPlunder;
	out << m_LastShipName;
	out << m_MoveDistLimit;
	AuTime tmLastLook;
	out << tmLastLook;
	AuTime tmLastScan;
	out << tmLastScan;
	AuTime tmLastThought;
	out << tmLastThought;
	m_SentientLock.unlock();

	m_SkillsLock.lockForRead();
	out << AuTime();	//m_SkillsChg;
	out << m_Skills;
	m_SkillsLock.unlock();

	m_TalkLock.lockForRead();
	out << m_Interlocutor;
	out << m_JobOffer;
	out << m_LastSaid;
	AuTime tmLastSpoke;
	out << tmLastSpoke;
	m_TalkLock.unlock();
}

void DaSentient::operator<<(QDataStream& in)
{
	DaCreature::operator<<(in);
	m_ThinkLock.lockForWrite();
	QTime aiAttackTime;
	in >> aiAttackTime;
	in >> m_AIIsAlert;
	in >> m_AIOrders;
	quint8 ui8;
	in >> ui8;
	m_AIPosture = (CITIZEN_POSTURE)ui8;
	in >> m_AIThinkRate;
	QTime aiThinkTime;
	in >> aiThinkTime;
	in >> m_TurnZ;
	m_ThinkLock.unlock();

	m_SentientLock.lockForWrite();
	in >> m_AIRecentCollide;
	in >> m_AIRecentParent;
	in >> m_AIRecentPath;
	if(m_AIScRoute)
		delete m_AIScRoute;
	in >> ui8;
	if(ui8)
	{
		m_AIScRoute = new AuScRoute;
		in >> *m_AIScRoute;
	}
	else
		m_AIScRoute = 0;
	in >> m_AIStrategy;
	QTime aiStrategyTime;
	in >> aiStrategyTime;
	in >> m_EnemyBlips;
	in >> m_InEnemyRange;
	in >> m_LastPlunder;
	in >> m_LastShipName;
	in >> m_MoveDistLimit;
	AuTime tmLastLook;
	in >> tmLastLook;
	AuTime tmLastScan;
	in >> tmLastScan;
	AuTime tmLastThought;
	in >> tmLastThought;
	m_SentientLock.unlock();

	m_SkillsLock.lockForWrite();
	AuTime _SkillsChg;
	in >> _SkillsChg;
	in >> m_Skills;
	m_SkillsLock.unlock();

	m_TalkLock.lockForWrite();
	in >> m_Interlocutor;
	in >> m_JobOffer;
	in >> m_LastSaid;
	AuTime tmLastSpoke;
	in >> tmLastSpoke;
	m_TalkLock.unlock();
}

void DaSentient::Create()
{
	DaCreature::Create();

	SetOrder(ORDER_Stay, AuId(0), AuPoint());	// Set default Stay order and start thinking.

	StartThinking();
}

bool DaSentient::IsTicker() const
{
	if(DaCreature::IsTicker())
		return true;

	if(IsBodyDyingOrDead())
		return false;

	if(IsAIControlled() && TurnZ() != 0 && CanTurn())
		return true;

	return false;
}

void DaSentient::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	DaCreature::Tick(last, now);
	if(IsDead())
		return;

	if(IsBodyDyingOrDead())
		return;

	if(IsAIControlled())
	{
		// Calculate number of milliseconds elapsed since last tick.
		int msElapsed = last.elapsed();
		if(msElapsed > 0)
		{
			// Turn toward desired direction.
			if(TurnZ() != 0 && CanTurn())
				AnimateTurn(msElapsed);
		}
	}
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaSentient::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaCreature::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;

	float turnZ = TurnZ();
	if(turnZ != 0 && !Is(sceneAtom.Id()))
		hasWhat |= 0x01;

	if(IsAIControlled())
		hasWhat |= 0x02;	// Orders

	m_SkillsLock.lockForRead();
	if(lastTx.IsNull() || m_SkillsChg >= lastTx)
		hasWhat |= 0x04;
	m_SkillsLock.unlock();

	toClient << hasWhat;

	if(hasWhat & 0x01)
		toClient << turnZ;	// Non-scene atom needs turnZ for turn animation.

	if(hasWhat & 0x02)
	{
		quint8 ui8 = Order();
		toClient << ui8;
		toClient << OrderTarget();
		if(ui8 == ORDER_GoToRoom)
			toClient << OrderTargetText();
		toClient << (quint8)Posture();
	}

	if(hasWhat & 0x04)
	{
		m_SkillsLock.lockForRead();
		toClient << m_Skills;
		m_SkillsLock.unlock();
	}
}

// Receive updates to this atom from a server.
void DaSentient::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaCreature::RxUpdate(fromServer, isFirstUpdate);
	quint8 hasWhat;
	fromServer >> hasWhat;

	if(hasWhat & 0x01)
		fromServer >> m_TurnZ;
	else
		m_TurnZ = 0;

	if(hasWhat & 0x02)
	{
		// Client only gets part of the current order.
		if(m_AIOrders.count() != 1)
		{
			m_AIOrders.clear();
			m_AIOrders.append(AuAIOrder(ORDER_Stay, AuId(0), Location(), 0, ""));
		}
		quint8 ui8;
		fromServer >> ui8;
		m_AIOrders[0].m_Type = (CITIZEN_ORDER)ui8;
		m_AIOrders[0].m_Target << fromServer;
		if(ui8 == ORDER_GoToRoom)
			fromServer >> m_AIOrders[0].m_Text;

		fromServer >> ui8;
		m_AIPosture = (CITIZEN_POSTURE)ui8;
	}
	else
	{
		// This sentient being is not AI controlled. The client signifies this with an empty orders list.
		m_AIOrders.clear();
	}

	if(hasWhat & 0x04)
		fromServer >> m_Skills;
}

// Refresh data from valid counterpart.
bool DaSentient::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaCreature::Refresh(pAtom, ari);

	const DaSentient* pA = (const DaSentient*)pAtom;

	// Scene atom sends skills so they must be refreshed here. They are not transmitted in TxUpdate because most sentients do not
	// need them on the client and they change a lot.
	pA->m_SkillsLock.lockForRead();
	m_SkillsLock.lockForWrite();
	m_Skills = pA->m_Skills;
	m_SkillsLock.unlock();
	pA->m_SkillsLock.unlock();

	pA->m_ThinkLock.lockForRead();
	m_ThinkLock.lockForWrite();
	m_AIOrders = pA->m_AIOrders;
	m_AIPosture = pA->m_AIPosture;
	m_TurnZ = pA->m_TurnZ;
	m_ThinkLock.unlock();
	pA->m_ThinkLock.unlock();

	return false;
}

void DaSentient::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	//qDebug("DaSentient::AnimateDeviceObjects - %s torso verts %d arms %d "
			//"legs %d upper arm %d fore arm %d hand %d size %d",
			//(const char*)Description(), DNA().TorsoVertebrae,
			//DNA().ArmPairs*2, DNA().LegPairs*2,
			//DNA().UpperArmLength, DNA().ForeArmLength,
			//DNA().HandLength, DNA().OverallSize);
	DaCreature::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

	// Scene atom handles there own turning.
	if(sceneAtom != this && m_TurnZ != 0 && CanTurn())
	{
		// Turn toward desired direction.
		AnimateTurn(lastAnim.elapsed());
	}
}

bool DaSentient::SayNoRepeat(AuCommPacket& msg, const AuVisage& visage, const AuId& toWho)
{
	QString text = msg.Text();
	m_TalkLock.lockForWrite();
	if(toWho == m_Interlocutor && !m_LastSaid.empty() && m_LastSaid.last() == text)
	{
		m_TalkLock.unlock();
		return true;
	}
	m_LastSaid.append(text);
	while(m_LastSaid.size() > 100)
		m_LastSaid.takeFirst();		// Remember the last 100 things I said.
	m_TalkLock.unlock();

	Say(msg, visage, toWho);
	return false;
}

void DaSentient::Say(AuCommPacket& msg, const AuId& toWho)
{
	m_TalkLock.lockForWrite();
	bool setmod;
	if(m_Interlocutor != toWho)
	{
		setmod = true;
		m_Interlocutor = toWho;
	}
	else
		setmod = false;
	m_tmLastSpoke.restart();
	m_TalkLock.unlock();

	if(setmod)
		SetModified(NOTIFY_Database);	// Passengers store the interlocutor so they're not obnoxious when server restarts.

	if(msg.Scope() == AuCommStdChannel::CrewMates)
		msg.SetScopeData(AssignedShip().GetID());				// Required for CrewMates message to be valid.
	else
		msg.SetScopeData(EmpireId());							// All other scopes that care want empire id.
	g_Au.SendAtomChat(msg);
	PoseMouthForSaying(msg, 0);
}

void DaSentient::Say(AuCommPacket& msg, const AuVisage& visage, const AuId& toWho)
{
	if(!visage.IsValid())
	{
		Say(msg, toWho);
		return;
	}
	m_TalkLock.lockForWrite();
	bool setmod;
	if(m_Interlocutor != toWho)
	{
		setmod = true;
		m_Interlocutor = toWho;
	}
	else
		setmod = false;
	m_tmLastSpoke.restart();
	m_TalkLock.unlock();

	if(setmod)
		SetModified(NOTIFY_Database);	// Passengers store the interlocutor so they're not obnoxious when server restarts.

	if(msg.Scope() == AuCommStdChannel::CrewMates)
		msg.SetScopeData(AssignedShip().GetID());				// Required for CrewMates message to be valid.
	else
		msg.SetScopeData(EmpireId());							// All other scopes that care want empire id.
	g_Au.SendAtomChat(msg);
	PoseMouthForSaying(msg, &visage);
}

void DaSentient::DoAI()
{
	// Do the AI needed to carry out my current order.
	switch(Order())
	{
	case ORDER_Stay:
		DoStayHere();
		break;
	case ORDER_AttackTarget:
		DoAttackTarget();
		break;
	case ORDER_BattleStation:
		DoBattleStation();
		break;
	case ORDER_ComeHere:
		DoComeHere();
		break;
	case ORDER_ComeHereExactly:
		DoComeHereExactly();
		break;
	case ORDER_FollowMe:
		DoFollowMe();
		break;
	case ORDER_FollowTarget:
		DoFollowTarget();
		break;
	case ORDER_Garrison:
		DoGarrison();
		break;
	case ORDER_GetIn:
		DoGetIn();
		break;
	case ORDER_GetOut:
		DoGetOut();
		break;
	case ORDER_GoToRoom:
		DoGoToRoom();
		break;
	case ORDER_GoToTarget:
		DoGoToTarget();
		break;
	case ORDER_GoToTargetExactly:
		DoGoToTargetExactly();
		break;
	case ORDER_HengeCeremony:
		DoHengeCeremony();
		break;
	case ORDER_CloseDoor:
		DoCloseDoor();
		break;
	case ORDER_OpenDoor:
		DoOpenDoor();
		break;
	case ORDER_GetItem:
		DoGetItem();
		break;
	case ORDER_Sleep:
	case ORDER_SleepDayDream:
	case ORDER_SleepEat:
	case ORDER_SleepExercise:
	case ORDER_SleepGame:
	case ORDER_SleepLetter:
	case ORDER_SleepMovie:
	case ORDER_SleepMusic:
	case ORDER_SleepPine:
	case ORDER_SleepPhone:
	case ORDER_SleepRead:
	case ORDER_SleepTalk:
	case ORDER_SleepText:
	case ORDER_SleepTV:
	case ORDER_SleepWeb:
		DoSleep();
		break;
	case ORDER_DamageReport:
		DoDamageReport();
		break;
	case ORDER_SurveyScanPlanet:
	case ORDER_SurveyScanSystem:
	case ORDER_TacticalScan:
		DoScan();
		break;
	case ORDER_EnterSurgery:
		DoEnterSurgery();
		break;
	case ORDER_TakeCommand:
		DoTakeCommand();
		break;
	case ORDER_ReleaseCommand:
		DoReleaseCommand();
		break;
	case ORDER_DropRider:
		DoDropRider();
		break;
	case ORDER_MountMe:
		DoMountMe();
		break;
	case ORDER_MountTarget:
		DoMountTarget();
		break;
	}

	m_SentientLock.lockForWrite();
	m_tmLastThought.restart();
	m_SentientLock.unlock();
}

// Turn toward desired direction, as determined by m_TurnZ.
// Returns 'true' when I finished turning to the desired direction.
// Returns 'false' when not turning or not done turning.
bool DaSentient::AnimateTurn(unsigned int msElapsed)
{
	//Q_ASSERT(TurnZ() != 0 && CanTurn());						// Check these before calling.

	bool finishedTurn = false;

	// Calculate turn amounts based on turn rate and time span.
	// The most I will turn in this time segment is influenced by the urgency of my current order.
	const float rsec = (float)msElapsed/1000.f;		// Real world elapsed secs.
	float rMaxTurn;
	if(IsTurretPose())
		rMaxTurn = TurretTurnRate() * rsec;			// Max turn in this time period based on turret turn rate.
	else
		rMaxTurn = TurnRate() * rsec;				// Max turn in this time period based on physiology.
	if(!IsInSpacecraft() && !AIIsAlert())
	{
		// Turn rate outside spacecraft can be lazy.
		switch(Order())
		{
		case ORDER_BattleStation:
		case ORDER_CloseDoor:
		case ORDER_DamageReport:
		case ORDER_DropRider:
		case ORDER_EnterSurgery:
		case ORDER_Garrison:
		case ORDER_OpenDoor:
		case ORDER_Sleep:
		case ORDER_SleepDayDream:
		case ORDER_SleepEat:
		case ORDER_SleepExercise:
		case ORDER_SleepGame:
		case ORDER_SleepLetter:
		case ORDER_SleepMovie:
		case ORDER_SleepMusic:
		case ORDER_SleepPine:
		case ORDER_SleepPhone:
		case ORDER_SleepRead:
		case ORDER_SleepTalk:
		case ORDER_SleepText:
		case ORDER_SleepTV:
		case ORDER_SleepWeb:
		case ORDER_Stay:
		case ORDER_SurveyScanPlanet:
		case ORDER_SurveyScanSystem:
		case ORDER_TacticalScan:
		case ORDER_TakeCommand:
		case ORDER_ReleaseCommand:
			rMaxTurn *= .5f;
			break;
		case ORDER_AttackTarget:
			// Maximum turn rate.
			break;
		case ORDER_ComeHere:
			rMaxTurn *= .85f;
			break;
		case ORDER_ComeHereExactly:
			// Maximum turn rate.
			break;
		case ORDER_FollowMe:
			rMaxTurn *= .9f;
			break;
		case ORDER_FollowTarget:
			rMaxTurn *= .8f;
			break;
		case ORDER_GetIn:
			rMaxTurn *= .6f;
			break;
		case ORDER_GetOut:
			rMaxTurn *= .7f;
			break;
		case ORDER_GoToRoom:
		case ORDER_GoToTarget:
		case ORDER_HengeCeremony:
			rMaxTurn *= .75f;
			break;
		case ORDER_GoToTargetExactly:
		case ORDER_GetItem:
			// Maximum turn rate.
			break;
		case ORDER_MountMe:
			rMaxTurn *= .9f;
			break;
		case ORDER_MountTarget:
			rMaxTurn *= .8f;
			break;
		}
	}

	m_ThinkLock.lockForWrite();
	float turnZNow;									// Amount to turn in this time period.
	if(m_TurnZ > rMaxTurn)
	{
		turnZNow = rMaxTurn;						// Amount to turn in this time period.
		m_TurnZ -= rMaxTurn;						// Update the amount of turn remaining.
	}
	else if(m_TurnZ < -rMaxTurn)
	{
		turnZNow = -rMaxTurn;						// Amount to turn in this time period.
		m_TurnZ += rMaxTurn;						// Update the amount of turn remaining.
	}
	else
	{
		turnZNow = m_TurnZ;							// Amount to turn in this time period.
		m_TurnZ = 0;								// Update the amount of turn remaining.
		finishedTurn = true;						// Finished turning to my desired heading.
	}
	m_ThinkLock.unlock();

	// Constrain turn angle when engaged with a wall turret.
	double constrainTurn;
	double gunDir;
	switch(CurrentGeneralPose().Pose())
	{
	//DHH Sc Console constrain turn for turrets.
	//case AuPose::Turret_Sit:
	//case AuPose::Turret_SitStationary:
	//case AuPose::Turret_SitSwivel3D:
	//case AuPose::Turret_Stand:
	//case AuPose::Turret_StandStationary:
	case AuPose::ConsoleTurret0:
		constrainTurn = PI/2;
		gunDir = 270 * DEG2RAD;
		break;
	case AuPose::ConsoleTurret45:
		constrainTurn = PI/2;
		gunDir = 315 * DEG2RAD;
		break;
	case AuPose::ConsoleTurret90:
		constrainTurn = PI/2;
		gunDir = 0 * DEG2RAD;
		break;
	case AuPose::ConsoleTurret135:
		constrainTurn = PI/2;
		gunDir = 45 * DEG2RAD;
		break;
	case AuPose::ConsoleTurret180:
		constrainTurn = PI/2;
		gunDir = 90 * DEG2RAD;
		break;
	case AuPose::ConsoleTurret225:
		constrainTurn = PI/2;
		gunDir = 135 * DEG2RAD;
		break;
	case AuPose::ConsoleTurret270:
		constrainTurn = PI/2;
		gunDir = 180 * DEG2RAD;
		break;
	case AuPose::ConsoleTurret315:
		constrainTurn = PI/2;
		gunDir = 225 * DEG2RAD;
		break;
	default:
		constrainTurn = 0;
		gunDir = 0;
		break;
	}
	if(constrainTurn != 0)
	{
		// Constrain turn to the limits of my turret arc.
		double zrot = Rotation().z;		// My current Z rotation.
		double nuz = zrot + turnZNow;	// My proposed Z rotation.
		double deltaz = nuz - gunDir;	// Delta from turret direction.
		while(deltaz < -PI)
			deltaz += PI*2;
		while(deltaz > PI)
			deltaz -= PI*2;
		// Prevent turning beyond arc of gun.
		if(deltaz < -constrainTurn)
		{
			double diff = -constrainTurn - deltaz;
			turnZNow += diff;
		}
		else if(deltaz > constrainTurn)
		{
			double diff = constrainTurn - deltaz;
			turnZNow += diff;
		}
	}

	// The client is not updated with this transformation change because it calls this function to animate locally. The
	// matrix change causes unnecessary network traffic if I am just turning on my axis.
	AuAtomPos tMe(Pos());
	tMe.RotateZ(turnZNow);
	ChangePosAndOrientNotParent(tMe);
	return finishedTurn;
}

// Citizens have a mind of their own. They're a bit dim-witted though otherwise they would put a huge drain on the server CPU.
// The time between thoughts is calculated using this citizen's id, causing some citizens to be faster or slower thinkers than
// others. The smarter citizens think more often. The urgency of their current order influences the speed of their thinking.
void DaSentient::UpdateThinkRate()
{
	quint16 thinkRate;
	CITIZEN_ORDER order = Order();
	switch(order)
	{
#if defined(QT_NO_DEBUG)
	default:
		// Defined so I get a warning message in the debug build if a CITIZEN_ORDER is not handled here.
		thinkRate = 250;
		break;
#endif
	case ORDER_AttackTarget:
	case ORDER_CloseDoor:
	case ORDER_ComeHere:
	case ORDER_ComeHereExactly:
	case ORDER_DropRider:
	case ORDER_Garrison:
	case ORDER_GetIn:
	case ORDER_GetItem:
	case ORDER_GetOut:
	case ORDER_GoToRoom:
	case ORDER_GoToTarget:
	case ORDER_GoToTargetExactly:
	case ORDER_HengeCeremony:
	case ORDER_OpenDoor:
	case ORDER_Sleep:
	case ORDER_Stay:
	case ORDER_SleepDayDream:
	case ORDER_SleepEat:
	case ORDER_SleepExercise:
	case ORDER_SleepGame:
	case ORDER_SleepLetter:
	case ORDER_SleepMovie:
	case ORDER_SleepMusic:
	case ORDER_SleepPine:
	case ORDER_SleepPhone:
	case ORDER_SleepRead:
	case ORDER_SleepTalk:
	case ORDER_SleepText:
	case ORDER_SleepTV:
	case ORDER_SleepWeb:
	case ORDER_DamageReport:
	case ORDER_SurveyScanPlanet:
	case ORDER_SurveyScanSystem:
	case ORDER_TacticalScan:
	case ORDER_TakeCommand:
	case ORDER_ReleaseCommand:
		// These orders have a think rate that does not vary based on conditions.
		thinkRate = ThinkRateForPose(order);
		break;

	case ORDER_BattleStation:
		if(IsLockedPose())
			thinkRate = ThinkRateForPose(ORDER_BattleStation);	// I am manning a console or turret.
		else
			thinkRate = ThinkRateForPose(ORDER_Stay);			// Wait patiently for a station to become available.
		break;

	case ORDER_FollowMe:
		if(!IsMovingPose() && Vector().IsFuzzyZero() && TurnZ() == 0 && IsRouteEmpty())
			thinkRate = ThinkRateForPose(ORDER_Garrison);
		else
			thinkRate = ThinkRateForPose(ORDER_FollowMe);
		break;

	case ORDER_FollowTarget:
		if(!IsMovingPose() && Vector().IsFuzzyZero() && TurnZ() == 0 && IsRouteEmpty())
			thinkRate = ThinkRateForPose(ORDER_Garrison);
		else
			thinkRate = ThinkRateForPose(ORDER_FollowTarget);
		break;

	case ORDER_EnterSurgery:
		if(IsPatientStationPose())
		{
			// This time interval MUST exceed the think rate for an AI doctor sitting at the medical console.
			// Here's how this works.
			// 1. I decide to enter a surgery unit because I need treatment.
			// 2. The doc eventually notices me and applies some treatment, which restarts my think time.
			// 3. The doc waits for his next time to think, which could be up to 4500ms for a low ranking dude. The doc needs to
			//    think again before my think time arrives so he can apply more treatments if needed.
			// 4. After the doc has stopped treating me, my think time arrives without being reset, which is why the
			//    interval must be longer than the think rate of the AI doctor.
			// 5. When I call DoEnterSurgery and I am already in a surgery unit of my own accord, I get out and do my next order.
			thinkRate = ThinkRateForPose(ORDER_Sleep) * 2;		// I am in a surgery unit. Wait for the doc to heal me.
		}
		else
			thinkRate = ThinkRateForPose(ORDER_EnterSurgery);	// Wait patiently for a surgery unit to become available.
		break;

	case ORDER_MountMe:
		if(!IsMovingPose() && Vector().IsFuzzyZero() && TurnZ() == 0 && IsRouteEmpty())
			thinkRate = ThinkRateForPose(ORDER_Garrison);
		else
			thinkRate = ThinkRateForPose(ORDER_MountMe);
		break;

	case ORDER_MountTarget:
		if(!IsMovingPose() && Vector().IsFuzzyZero() && TurnZ() == 0 && IsRouteEmpty())
			thinkRate = ThinkRateForPose(ORDER_Garrison);
		else
			thinkRate = ThinkRateForPose(ORDER_MountTarget);
		break;
	}

	// When not alert, let more time pass between thoughts.
	//if(!IsAlert())
		//m_AIThinkRate = m_AIThinkRate * 3 / 2;	// 1/2 again as much time between thoughts when not alert

	m_ThinkLock.lockForWrite();
	m_AIThinkRate = thinkRate;
	m_ThinkLock.unlock();
}

void DaSentient::StayIdle()
{
	// Update movement path parameters.
	ResetStrategy(true);

	// Don't do idle animation when not in anyone's scene.
	if(!IsInAnyScene())
		return;

	// Do nothing when riding a mount or mounted.
	if(IsRider() || IsMounted())
		return;

	AuId intid(Interlocutor());
	if(!intid.IsNull())
	{
		// Turn to face the person I am conversing with.
		const DaAtom* env = FindEnvironmentAtom();
		const DaSentient* sent = env ? dynamic_cast<const DaSentient*>(env->FindIdRecursive(intid)) : 0;
		const DaPerson* person = dynamic_cast<const DaPerson*>(sent);
		if(person)
		{
			if(!person->IsEthereal())
			{
				// Found the person on line. Turn to face them.
				// Calculate the person's location relative to me.
				AuPoint personLoc = EnvTransform().Inverse().Project(person->EnvLocation());
				if(personLoc.Length2() < 20*FeetPerMeter()*20*FeetPerMeter())
				{
					// Remain engaged with the conversation as long as they are in a short range.
					// Otherwise I will say hello repeatedly.
					if(SpokeRecently(30))
					{
						// I have spoken recently so turn to face the interlocutor.
						double turnZ = personLoc.ArcTangent_2D() - PI/2;
						if(turnZ < -PI)
							turnZ += PI*2;
						if(turnZ != TurnZ())
							SetTurnZ(turnZ);

						// Tilt head to look at the person.
						if(CurrentGeneralPose() == AuPose::ReceptiveMate)
							TiltHeadTo(personLoc.PolarIncline() - 10*DEG2RADf);
						else
							TiltHeadTo(personLoc.PolarIncline());
					}
				}
				else
					ClearInterlocutor();	// Person has gotton too far away to continue the conversation.
			}
			else
				ClearInterlocutor();	// Person is off line or invisible.
		}
		else if(sent)
		{
			// Found the sentient. Turn to face them.
			// Calculate the sentient's location relative to me.
			AuPoint sentLoc = EnvTransform().Inverse().Project(sent->EnvLocation());
			if(sentLoc.Length2() < 20*20)
			{
				// Remain engaged with the conversation as long as they are in a short range.
				// Otherwise I will say hello repeatedly.
				if(SpokeRecently(30))
				{
					// I have spoken recently so turn to face the interlocutor.
					double turnZ = sentLoc.ArcTangent_2D() - PI/2;
					if(turnZ < -PI)
						turnZ += PI*2;
					if(turnZ != TurnZ())
						SetTurnZ(turnZ);

					// Tilt head to look at the sentient.
					TiltHeadTo(sentLoc.PolarIncline());
				}
			}
			else
				ClearInterlocutor();	// Person has gotton too far away to continue the conversation.
		}
		else
			ClearInterlocutor();	// Person is gone or invisible.
	}
	else
	{
		// Tilt my head.
		float headTiltChange = g_Au.Dice().Roll(1,11,-6) * DEG2RADf;	// Change head tilt by -5 degrees down to 5 degrees up
		if(headTiltChange != 0)
		{
			float headTilt = CurrentHeadTilt() + headTiltChange;
			if(headTilt < -40 * DEG2RADf)
				headTilt = 0;	// Look straight ahead.
			else if(headTilt > 40 * DEG2RADf)
				headTilt = 0;	// Look straight ahead.
			// TiltHeadTo will constrain to turret angle, if necessary.
			if(TiltHeadTo(headTilt))
				SetModified(NOTIFY_Scene);			// Head tilt not stored in db but client likes to know.
		}

		// Turn to face a different direction.
		if(IsLadderPose())
			SetTurnZ(0);
		else if(g_Au.Dice().Roll(1,3) == 1)
		{
			if(IsHelmStationPose() && !IsFuzzyZero(Rotation().z))
			{
				// Pilot is not looking forward; look forward.
				// This way pilot alternates between random turn away from forward, then back to facing forward.
				SetTurnZ(-Rotation().z);
			}
			else
			{
				// Turn to a random facing.
				SetTurnZ((float)g_Au.Dice().Roll(1,361,-181) * DEG2RADf);
			}
		}
	}

	// Stop moving.
	// Don't do this in a vehicle or at a console or when riding a mount because it messes up my pose.
	if(!IsAboardVehicle() && !IsLockedPose() && !CurrentGeneralPose().IsMatingOrWaiting())
	{
		// Select an appropriate waiting pose.
		AuPose pose;
		if(IsInsideSpacecraft())
			pose = AuPose::Stand;
		else
		{
			float cght = CGHeightStanding();
			if(WaterDepth() >= cght)
				pose = AuPose::TreadWater;
			else if(CanFly() && AltitudeAGL() > cght*2 + CameraOffset().z)
			{
				// Flyers will hover or circle.
				if(CanHover())
					pose = AuPose::Hover;
				else
					pose = AuPose::Glide;
			}
			else
			{
				if(CurrentGeneralPose() == AuPose::SitOnGround)
					pose = AuPose::SitOnGround;	// Remain sitting.
				else
					pose = AuPose::Stand;
			}
		}

		// Pose my body.
		if(CurrentGeneralPose() != pose)
			DoPose(pose);
	}
}

// Returns 0 when I am still moving to the target.
// Returns 1 when I have reached the target. 'orderTarget' will be set as an output parameter in this case.
// Returns 2 when no path to the target exists.
int DaSentient::MoveToOrderLoc(double closeEnough, bool lookAtTarget, DaAtom*& orderTarget)
{
	if(IsAboardVehicle())
	{
		if(ParentId() == OrderTarget())
		{
			// I'm in the vehicle that is my movement target.
			orderTarget = Parent();
			return 1;
		}
		else
		{
			// I'm not supposed to be in the vehicle.
			DoGetOut();
			return 0;
		}
	}

	// Reset movement strategy when my parent atom changes.
	m_SentientLock.lockForWrite();
	if(m_AIRecentParent != ParentId())
	{
		// My parent atom has changed. Reset my path strategy parameters.
		m_AIRecentParent = ParentId();
		ResetStrategyNoLock(true);
	}

	// Perform actions based on my current movement strategy.
	m_MoveDistLimit = -1;
	int strategy = m_AIStrategy;
	int selapsed = m_AIStrategyTime.elapsed();
	m_SentientLock.unlock();

	if(strategy == 0)
	{
		// Primary movement strategy tries to be smart about where it's going.
		if(IsInCapsule())
		{
			// I'm in or on a spacecraft and not in a vehicle.
			DaCapsule* ship = FindParentCapsuleInEnv();
			if(!ship || !ship->IsDesignDataCacheValid())
				return 0;
			if(ship->Is(OrderTarget()))
			{
				// I'm supposed to move to a location relative to the spacecraft.
				orderTarget = ship;
				quint16 toRoomId;
				AuPoint orderLoc(OrderLoc());
				ship->InsideRoom(orderLoc, toRoomId);							// The room I am going to, could be 0 if outside.
				AuPoint moveVec = Transform().UnProject(orderLoc);				// Destination vector relative to me.
				if(lookAtTarget)
					moveVec -= CameraOffset();				// Relative to my eye is better for targeting.
				int result;
				if(toRoomId == CapsuleRoomId() && moveVec.Length_2D() < closeEnough)
				{
					result = 1;		// Close enough and in the same room as the target; skip the hard routing work.
					if(lookAtTarget)
						MoveInDirection(moveVec, 0, AuPose::Stand);	// Look at the target.
				}
				else
					result = MoveToCapsuleLoc(ship, orderLoc);	// Move toward the room.

				if(result == 2)
					return 2;
				else if(result == 1)
				{
					// I'm in the destination floor cell or I'm in the same room as the target and within the required distance.
#if !defined(QT_NO_DEBUG)
		if(false && Is(s_PathTrace))
			qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "ship" << ship->Id() << ship->Name() << "close enough aboard ship";
#endif
					ClearRoute();
					return 1;	// Close enough to the target to ask for my next order.
				}
			}
			else
			{
				// The spacecraft I'm aboard is not the target of my order.
				// Is my target aboard the spacecraft?
				orderTarget = ship->FindNormalSubatomId(OrderTarget());
				if(orderTarget)
				{
					// My order target is aboard.
					// Move to its location aboard this ship.
					quint16 toRoomId = orderTarget->CapsuleRoomId();				// The room I am going to.
					AuPoint targLoc = orderTarget->Transform().Project(orderTarget->TargetLocation());
					AuPoint moveVec = Transform().UnProject(targLoc);			// Destination vector rel to my CG.
					if(lookAtTarget)
						moveVec -= CameraOffset();				// Relative to my eye is better for targeting.
					int result;
					if(toRoomId == CapsuleRoomId() && moveVec.Length_2D() < closeEnough)
					{
						result = 1;		// Close enough and in the same room as the target; skip the hard routing work.
						if(lookAtTarget)
						{
							// Calculate the moveVec from my camera location, not my body location, for proper aiming.
							if(Is(s_PathTrace))
								qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "targLoc" << targLoc
									<< "moveVec" << moveVec << "incline" << (moveVec.PolarIncline()*RAD2DEG);
							MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Stand);	// Look at the target.
						}
					}
					else
						result = MoveToCapsuleLoc(ship, orderTarget->Location());	// Move toward the target location.

					if(result == 2)
						return 2;	// Return failure result.
					else if(result == 1 || (IsMovingPose() && orderTarget->Is(AIRecentCollide())))
					{
						// I'm in the destination floor cell or I'm in the same room as the target and within the required distance.
						// OR I've bumped into the target.
#if !defined(QT_NO_DEBUG)
		if(false && Is(s_PathTrace))
			qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "ship" << orderTarget->Id() << orderTarget->Name() << "close enough to order target";
#endif
						ClearRoute();
						return 1;	// Close enough to the target.
					}
				}
				else
				{
					// My order target could be inside a vehicle or other spacecraft aboard this ship.
					orderTarget = ship->FindIdRecursive(OrderTarget());
					if(orderTarget)
					{
						// The order target is aboard the ship somewhere.
						if(orderTarget->Parent()->Parent() == ship)
						{
							// The order target is subordinate to an object whose parent is the ship.
							DaCapsule* orderToShip = dynamic_cast<DaCapsule*>(orderTarget->Parent());
							if(orderToShip)
							{
								// Order target is aboard a spacecraft.
								// Find a door to enter. I need the location relative to the ship I'm aboard.
								// Find the door that is most convenient to my location, projected into the space of the ship.
								AuPoint myShipLoc = orderToShip->Transform().UnProject(Location());
								bool isCrew = orderToShip->IsCrewOrTroop(Id());
								int doorIndex = -1;
								int holeIndex = -1;
								AuPoint entrance = orderToShip->Transform().Project(orderToShip->EntranceLocation(myShipLoc,
											doorIndex, holeIndex, orderToShip->IsOnGround(),
											isCrew || orderToShip->AnyHullDoorUnlocked()));
								if(doorIndex < 0 && holeIndex < 0)
								{
									// When on the ground, try again to see if I can get to an entrance above the ground.
									//if(orderToShip->IsOnGround())
										//entrance = orderToShip->Transform().Project(orderToShip->EntranceLocation(myShipLoc,
													//doorIndex, holeIndex, false, isCrew || orderToShip->AnyHullDoorUnlocked()));
									//if(doorIndex < 0 && holeIndex < 0)
										return 2;	// I can't find an entrance so I can't get into the ship.
								}
								int result = MoveToCapsuleLoc(ship, entrance);	// Move toward the room.
								if(result == 2)
									return 2;	// Return failure result.
								else if(result == 1)
								{
									// I'm in the destination floor cell or I'm within reach of the destination door.
									// Open the hull door because they don't open automatically.
									if(doorIndex >= 0 && !orderToShip->IsDoorOpen(doorIndex))
									{
										// Hull doors can be locked from the outside to non-crew members.
										if(isCrew || !orderToShip->HullDoorLocked(doorIndex))
											orderToShip->OpenDoor(doorIndex);
										else
											return 2;	// I can't open the door so I can't get into the ship.
									}
								}
							}
							else
							{
								// Order target is probably subordinate to a vehicle or riding an animal.
								// Move to the target's parent location aboard this ship.
								int result = MoveToCapsuleLoc(ship, orderTarget->Parent()->Location());
								if(result == 2)
									return 2;	// Return failure result.
								else if(result == 1 ||
										(IsMovingPose() && orderTarget->Parent()->Is(AIRecentCollide())) ||
										IsRouteDistanceLessThan(closeEnough))
								{
									// I'm in the destination floor cell
									//   or
									// I've bumped into the target's parent.
									//   or
									// I'm within the required distance of the destination.
#if !defined(QT_NO_DEBUG)
		if(false && Is(s_PathTrace))
			qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "ship" << orderTarget->Id() << orderTarget->Name() << "close enough to order target sub";
#endif
									ClearRoute();
									if(lookAtTarget)
									{
										AuPoint moveVec = Transform().UnProject(orderTarget->Parent()->Location());
										moveVec -= CameraOffset();				// Relative to my eye is better for targeting.
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Stand);
									}
									return 1;	// Close enough to the target.
								}
							}
						}
						else
						{
							// Order target is too many levels subordinate to a sub object for me to figure out how to get there.
							return 2;
						}
					}
					else
					{
						// My order target is not aboard the spacecraft. Get off the ship.
						//qDebug("%s Order target not found aboard my spacecraft", (const char*)Name().toAscii());
						bool movingToTeleporter = false;
						if(ship->IsTransporterRoom(CapsuleRoomId()))
						{
							// Move into transporter if there is an active teleporter in the room.
							quint16 whichTrans = AtTransporterWhich();
							AuPoint padLoc;
							if(ship->ActiveTransporterPadLocation(whichTrans, padLoc))
							{
								// Convert the pad location to a vector relative to me.
								//qDebug("%s moving to transporter.", (const char*)Name().toAscii());
								AuPoint moveVec = Transform().UnProject(padLoc);	// Make the target loc a vector relative to me.
								MoveInDirection(moveVec, 0, AuPose::StandWalk);
								movingToTeleporter = true;
							}
						}
						else
						{
							// Target is not aboard my spacecraft. Did he beam away? Look for teleporters in my area.
							double telDistance = 0;
							AuPoint telLoc;
							if(ship->FindClosestOriginTeleporter(Location(), telLoc, telDistance))
							{
								// Found an active origin teleporter in my terrain patch.
								if(telDistance < RUN_FAR_ENOUGH*4)
								{
									// The origin teleporter is close enough to go for it.
									// Make the teleporter location into a vector relative to me.
									AuPoint moveVec = Transform().UnProject(telLoc);
									if(telDistance > RUN_FAR_ENOUGH)
										MoveInDirection(moveVec, 0, AuPose::StandRun);
									else
										MoveInDirection(moveVec, 0, AuPose::StandWalk);
									movingToTeleporter = true;
								}
							}
						}
						if(!movingToTeleporter)
						{
							// I did not find a teleporter to move toward.
							if(ship->IsOnGround())
							{
								// The ship is on the ground. Is my order target outside?
								DaWorld* planet = (DaWorld*)FindParentPlanetInEnv();
								if(planet)
								{
									orderTarget = planet->FindIdRecursive(OrderTarget());
									if(orderTarget)
									{
										// My order target is at this planet.
										AuPoint myLoc;
										GetLocationRelativeToParent(myLoc, planet);
										AuPoint targetLoc;
										AuMatrix4x4 tt;
										orderTarget->GetTransformRelativeToParent(tt, planet);
										targetLoc = tt.Project(OrderLoc());
										//qDebug("%s target outside of ship is %g feet away.",
													//(const char*)Name().toAscii(), myLoc.Distance(targetLoc));
										if(myLoc.Distance(targetLoc) < 1000)
										{
											// Convert targetLoc to be relative to the ship.
											AuMatrix4x4 ts;
											ship->GetTransformRelativeToParent(ts, planet);
											AuPoint shipLoc = ts.UnProject(targetLoc);

											//if(Is(s_PathTrace))
												//qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name()
													//<< "moving to target location outside ship on ground";
											int result = MoveToCapsuleLoc(ship, shipLoc);	// Move toward the best exit.
											if(result)
												return result;	// Could not move outside.
										}
										else
											return 2;	// My target is too far away.
									}
									else
										return 2;	// My target is not at this planet.
								}
								else
									return 2;	// Not really possible.
							}
							else
							{
								// No transporter to target and ship is not on the ground.
								DaWorld* planet = (DaWorld*)FindParentPlanetInEnv();
								if(planet)
								{
									orderTarget = planet->FindIdRecursive(OrderTarget());
									if(orderTarget)
									{
										// My order target is at this planet.
										AuPoint myLoc;
										GetLocationRelativeToParent(myLoc, planet);
										AuPoint targetLoc;
										AuMatrix4x4 tt;
										orderTarget->GetTransformRelativeToParent(tt, planet);
										targetLoc = tt.Project(OrderLoc());
										//qDebug("%s target outside of ship is %g feet away.",
													//(const char*)Name().toAscii(), myLoc.Distance(targetLoc));
										if(myLoc.Distance(targetLoc) < 10000)
										{
											// Convert targetLoc to be relative to the ship.
											AuMatrix4x4 ts;
											ship->GetTransformRelativeToParent(ts, planet);
											AuPoint shipLoc = ts.UnProject(targetLoc);

											//if(Is(s_PathTrace))
												//qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name()
													//<< "moving to target location outside ship that is not on ground";
											int result = MoveToCapsuleLoc(ship, shipLoc);	// Move toward the best exit.
											if(result)
												return result;	// Could not move outside.
										}
										else
											return 2;	// My target is too far away.
									}
									else
										return 2;	// My target is not at this planet.
								}
								else
									return 2;	// Not at a planet.
							}
						}
					}
				}
			}
		}
		/*
		else if(IsInSpacecraft())
		{
			// I am in a spacecraft but I am not inside the spacecraft. I must be hanging from a ladder or standing on the hull.
			// Am I trying to get aboard the spacecraft or get off of the spacecraft?
			DaVessel* ship = FindParentVesselInEnv();
			if(!ship || !ship->IsDesignDataCacheValid())
				return 0;
			if(ship->Is(OrderTarget()) || ship->FindNormalSubatomId(OrderTarget()))
			{
				// I am trying to get aboard the ship.
				// Find the hull door that is most convenient to my location, projected into the space of the ship.
				AuPoint myShipLoc = Location();
				int openingIndex = -1;
				bool isCrew = ship->IsCrewOrTroop(Id());
				AuPoint entrance = ship->EntranceLocation(myShipLoc, openingIndex, ship->IsOnGround(),
						isCrew || ship->AnyHullDoorUnlocked()) - myShipLoc;	// vec in ship space
				if(openingIndex < 0)
					return 2;	// I can't find an entrance so I can't get into the ship.
				AuPoint moveVec = Transform().UnProjectVector(entrance);		// Convert ship vec to a vector relative to me.
				double distToMove = moveVec.Length_2D();
				float headTilt = 0;
				bool climbLadder = false;
				if(distToMove < 4)	// Far enough to open a hatch above my head.
				{
					// Open the hull door because they don't open automatically.
					const AuOpening& opening = ship->Opening(openingIndex);
					int doorIndex = ship->DoorIndex(opening.m_Detail);
					if(!ship->IsDoorOpen(doorIndex))
					{
						// Hull doors can be locked from the outside to non-crew members.
						if(isCrew || !ship->HullDoorLocked(doorIndex))
							ship->OpenDoor(doorIndex);
						else
							return 2;	// I can't open the door so I can't get into the ship.
					}

					// Look toward hatches to help go up/down through them.
					if(!opening.IsOnWall())
					{
						headTilt = moveVec.PolarIncline();	// Look up/down to go through hatch.

						// Grab the ladder.
						if(!IsLadderPose())
						{
							// Not on the ladder yet.
							// Calculate movement vector to snap me to the ladder.
							AuMatrix4x4 tmv;
							tmv.Translate(moveVec.x, moveVec.y, 0);
							TransformBy(tmv);
							InvalidateEnvironment();
						}
						climbLadder = true;
					}
				}

				if(climbLadder)
				{
					// Climb up/down the ladder according to head tilt.
					MoveInDirection(moveVec, headTilt, AuPose::LadderClimb);
				}
				else
				{
					if(distToMove > RUN_FAR_ENOUGH)
						MoveInDirection(moveVec, headTilt, AuPose::StandRun);
					else
						MoveInDirection(moveVec, headTilt, AuPose::StandWalk);

					if(headTilt != 0 && distToMove < NEAR_DOOR_THINK_FAST)
					{
						// I must think often to position myself above or below a hatch accurately.
						ThinkSoon();
						m_SentientLock.lockForWrite();
						m_MoveDistLimit = distToMove;
						m_SentientLock.unlock();
					}
				}
			}
			else
			{
				// I am trying to get off the ship.
				//DHH This should get me off the hull until I can be smarter about this.
				MoveInDirection(AuPoint(0,1), 0, AuPose::StandRun);
			}
		}
		*/
		else
		{
			// I am not in a spacecraft or vehicle.
			ClearRoute();	// Insure that I think about my path when I enter the next spacecraft.
			if(IsInAtmosphere())
			{
				// I am in atmosphere range of a planet, which means my parent atom is terrain until I learn how to ride animals.
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
				if(llworld)
				{
					// World is nu style globe or ring section. No terrain patches.
					if(llworld->Is(OrderTarget()))
					{
						// My current world is the target of my order.
						//qDebug("%s My current world is the target of my order.", (const char*)Name().toAscii());
						// The location is relative to the world.
						orderTarget = llworld;
						AuPoint orderLoc(OrderLoc());
						AuPoint moveVec = Transform().UnProject(orderLoc);	// Make the target location a vector relative to me.
						if(lookAtTarget)
							moveVec -= CameraOffset();				// Relative to my eye is better for targeting.

						// Determine my room id if I'm in a building. 0 indicates I am not inside a building.
						quint16 myRoomId = 0;

						// Determine room id of destination if it is in the same building site in which I am standing.
						quint16 toRoomId = 0;
						if(InIndustry() != IID_None && InIndustry() != IID_Road)
						{
							// I am in a building site. Check to see if the destination is in a room of a building on this site.
							/*DHH DaCity
							DaCity* city;
							FINDDEV filter = (FINDDEV)(FIND_FinishedLevel|FIND_BuildingsOnly);
							const DpBuilding* building =
								dynamic_cast<const DpBuilding*>(llworld->LockDevelopmentContainingLoc(orderLoc, &city, filter));
							if(building && building->DevLL() == InIndustryLL() && !building->IsDestroyed())
								building->InsideRoom(building->WorldTransform(*llworld).UnProject(orderLoc),
										false, toRoomId);		// The room I am going to, 0 if outside.
							llworld->UnlockDevelopment(city);
							*/
						}

						// Determine if I am close enough to my target location.
						double moveLen = moveVec.Length_2D();
						if(myRoomId == toRoomId)
						{
							m_SentientLock.lockForWrite();
							m_MoveDistLimit = moveLen;			// Limit my movement to stop right at the target.
							m_SentientLock.unlock();
						}
						//if(Is(s_PathTrace))
							//qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "moveLen" << moveLen
								//<< "myRoomId" << myRoomId << "toRoomId" << toRoomId;
						if(myRoomId == toRoomId && moveLen < closeEnough)
						{
							if(lookAtTarget)
								MoveInDirection(moveVec, 0, AuPose::Stand);	// Look at the target.
							return 1;	// Close enough to my destination world.
						}
						else
						{
							// Move toward the target location.
							MoveToGlobeLoc(llworld, orderLoc, moveVec, MoveDistLimit(), 0);
						}
						/*
						else if(MoveDistLimit() > RUN_FAR_ENOUGH)
						{
							if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
								MoveInDirection(moveVec, 0, AuPose::SwimFast);
							else if(CanFly())
								MoveInDirection(moveVec, 0, AuPose::FlyFast);
							else if(IsInWater())
								MoveInDirection(moveVec, 0, AuPose::SwimFast);
							else
								MoveInDirection(moveVec, 0, AuPose::StandRun);
						}
						else
						{
							if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
								MoveInDirection(moveVec, 0, AuPose::Swim);
							else if(CanFly())
								MoveInDirection(moveVec, 0, AuPose::Fly);
							else if(IsInWater())
								MoveInDirection(moveVec, 0, AuPose::Swim);
							else
								MoveInDirection(moveVec, 0, AuPose::StandWalk);
						}
						*/
					}
					else
					{
						// My current globe is not the target of the order.
						orderTarget = llworld->FindNormalSubatomId(OrderTarget());
						if(orderTarget)
						{
							// The order target is on my current globe.
							AuPoint moveVec;
							DaCapsule* orderToShip = dynamic_cast<DaCapsule*>(orderTarget);
							if(orderToShip)
							{
								// The target of the order is a spacecraft. Find a door to enter. I need the location relative to
								// the globe.
								// Find the door that is most convenient to my location, projected into the space of the ship.
								AuPoint myShipLoc = orderToShip->Transform().UnProject(Location());
								bool isCrew = orderToShip->IsCrewOrTroop(Id());
								int doorIndex = -1;
								int holeIndex = -1;
								AuPoint entrance = orderToShip->Transform().Project(orderToShip->EntranceLocation(myShipLoc,
											doorIndex, holeIndex, orderToShip->IsOnGround(),
											isCrew || orderToShip->AnyHullDoorUnlocked()));
								if(doorIndex < 0 && holeIndex < 0)
								{
									// When on the ground, try again to see if I can get to an entrance above the ground.
									//if(orderToShip->IsOnGround())
										//entrance = orderToShip->Transform().Project(orderToShip->EntranceLocation(myShipLoc,
													//doorIndex, holeIndex, false, isCrew || orderToShip->AnyHullDoorUnlocked()));
									//if(doorIndex < 0 && holeIndex < 0)
										return 2;	// I can't find an entrance so I can't get into the ship.
								}
								moveVec = Transform().UnProject(entrance);		// Convert globe loc to a vector relative to me.
								double distToMove = moveVec.Length_2D();
								//qDebug("%s orderToShip %d %s moveVec %g,%g,%g dist %g",
										//(const char*)Name().toAscii(), orderToShip->Id().GetID(),
										//(const char*)orderToShip->Name().toAscii(), moveVec.x, moveVec.y, moveVec.z, distToMove);
								float headTilt = 0;
								bool climbLadder = false;
								if(distToMove < 4)	// Far enough to open a hatch above my head.
								{
									// Open the hull door because they don't open automatically.
									bool isOnWall;
									if(doorIndex >= 0)
									{
										if(!orderToShip->IsDoorOpen(doorIndex))
										{
											// Hull doors can be locked from the outside to non-crew members.
											if(isCrew || !orderToShip->HullDoorLocked(doorIndex))
												orderToShip->OpenDoor(doorIndex);
											else
												return 2;	// I can't open the door so I can't get into the ship.
										}

										isOnWall = orderToShip->IsDoorOnWall(doorIndex);
									}
									else
										isOnWall = orderToShip->IsHoleOnWall(holeIndex);

									// Look toward hatches to help go up/down through them.
									if(!isOnWall)
									{
										headTilt = moveVec.PolarIncline();	// Look up/down to go through hatch.

										// Grab the ladder if I can.
										if(!IsLadderPose() &&					// Not on a ladder.
												(fabsf(headTilt) > PIf/4 || !IsInGravity() || distToMove < 1) &&	// Looking up/down or in zero G or very close.
												GrabLadderOnCapsule(orderToShip))	// Attempt to grab any ladder.
											climbLadder = true;
									}
								}

								if(climbLadder)
								{
									// Climb up/down the ladder according to head tilt.
									MoveInDirection(moveVec, headTilt, AuPose::LadderClimb);
								}
								else
								{
									// Move toward the entrance door.
									MoveToGlobeLoc(llworld, entrance, moveVec, distToMove, headTilt);
									if(headTilt != 0 && distToMove < NEAR_DOOR_THINK_FAST)
									{
										// I must think often to position myself above or below a hatch accurately.
										ThinkSoon();
										m_SentientLock.lockForWrite();
										m_MoveDistLimit = distToMove;
										m_SentientLock.unlock();
									}
								}
								/*
								else
								{
									// Move toward the door.
									if(distToMove > RUN_FAR_ENOUGH)
									{
										if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
											MoveInDirection(moveVec, headTilt, AuPose::SwimFast);
										else if(CanFly())
											MoveInDirection(moveVec, headTilt, AuPose::FlyFast);
										else if(IsInWater())
											MoveInDirection(moveVec, headTilt, AuPose::SwimFast);
										else
											MoveInDirection(moveVec, headTilt, AuPose::StandRun);
									}
									else
									{
										if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
											MoveInDirection(moveVec, headTilt, AuPose::Swim);
										//else if(CanFly())	// Don't try to fly into the entrance of the ship.
											//MoveInDirection(moveVec, headTilt, AuPose::Fly);
										else if(IsInWater())
											MoveInDirection(moveVec, headTilt, AuPose::Swim);
										else
											MoveInDirection(moveVec, headTilt, AuPose::StandWalk);
									}

									if(headTilt != 0 && distToMove < NEAR_DOOR_THINK_FAST)
									{
										// I must think often to position myself above or below a hatch accurately.
										ThinkSoon();
										m_SentientLock.lockForWrite();
										m_MoveDistLimit = distToMove;
										m_SentientLock.unlock();
									}
								}
								*/
							}
							else
							{
								// Order target is not a spacecraft. Move to the target location.
								// Get my transform and location relative to the globe.
								AuMatrix4x4 tp;
								GetTransformRelativeToParent(tp, llworld);
								AuPoint myTerLoc = tp.Project(AuPoint(0,0));

								// Get the target location relative to the terrain.
								// Target location is relative to the target atom.
								AuMatrix4x4 tt;
								orderTarget->GetTransformRelativeToParent(tt, llworld);
								AuPoint orderTerLoc;
								AuPoint orderLoc(OrderLoc());
								if(orderLoc == AuPoint(0,0))
									orderTerLoc = tt.Project(orderTarget->TargetLocation());	// Target loc relative to the globe.
								else
									orderTerLoc = tt.Project(orderLoc);	// Target loc relative to the globe.

								// Make the target loc a vector relative to me.
								AuPoint moveVec = tp.UnProjectVector(orderTerLoc - myTerLoc);
								if(lookAtTarget)
									moveVec -= CameraOffset();				// Relative to my eye is better for targeting.
								/*
								if(Is(AuId(241530682)))
									qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "orderLoc" << orderLoc
										<< "orderTarget" << orderTarget->Id() << orderTarget->Name()
										<< "targetLoc" << orderTarget->TargetLocation()
										<< "moveVec" << moveVec
										<< "camOffset" << CameraOffset() << "lookAt" << lookAtTarget
										<< "moveLimit" << (moveVec + CameraOffset()).Length_2D() << "closeEnough" << closeEnough;
										*/

								m_SentientLock.lockForWrite();
								m_MoveDistLimit = moveVec.Length_2D();
								float mdl = m_MoveDistLimit;
								m_SentientLock.unlock();

								if(mdl < closeEnough)
								{
									if(lookAtTarget)
									{
										float cght = CGHeightStanding();
										if(WaterDepth() >= cght)
											MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::TreadWater);
										else if(CanFly() && AltitudeAGL() > cght*2 + CameraOffset().z)
										{
											// Flyers will hover or circle.
											if(CanHover())
												MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Hover);
											else
												MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Glide);
										}
										else
											MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Stand);
									}
									return 1;	// Close enough to my destination.
								}
								else
								{
									// Move toward the order target.
									MoveToGlobeLoc(llworld, orderTerLoc, moveVec, mdl, moveVec.PolarIncline());
								}
								/*
								else if(mdl > RUN_FAR_ENOUGH)
								{
									if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::SwimFast);
									else if(CanFly())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::FlyFast);
									else if(IsInWater())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::SwimFast);
									else
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::StandRun);
								}
								else
								{
									if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Swim);
									else if(CanFly())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Fly);
									else if(IsInWater())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Swim);
									else
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::StandWalk);
								}
								*/
							}
						}
						else
						{
							// The order target is not on my current globe.
							//qDebug("%s The order target is not in my current globe.", (const char*)Name().toAscii());
							orderTarget = llworld->FindIdRecursive(OrderTarget());
							if(!orderTarget)
								return 2;	// My target atom is not at this planet.
							bool movingToTeleporter = false;
							if(orderTarget->DoesQueryEnvironment() && orderTarget->IsInsideSpacecraft())
							{
								// Target is aboard a spacecraft. Did he beam there? Look for teleporters in my area.
								// The spacecraft could be anywhere subordinate to the planet, in orbit or on terrain.
								double telDistance = 0;
								AuPoint telLoc;
								if(llworld->FindClosestOriginTeleporter(Location(), telLoc, telDistance))
								{
									// Found an active origin teleporter in my terrain patch.
									if(telDistance < RUN_FAR_ENOUGH*4)
									{
										// The origin teleporter is close enough to go for it.
										// Make the teleporter location into a vector relative to me.
										AuPoint moveVec = Transform().UnProject(telLoc);
										// Move toward the teleporter.
										MoveToGlobeLoc(llworld, AuPoint(0,0), moveVec, telDistance, 0);
										/*
										if(telDistance > RUN_FAR_ENOUGH)
											MoveInDirection(moveVec, 0, AuPose::StandRun);
										else
											MoveInDirection(moveVec, 0, AuPose::StandWalk);
											*/
										movingToTeleporter = true;
									}
								}
							}
							if(!movingToTeleporter)
							{
								// I didn't find a teleporter to move to.
								if(!orderTarget->IsInAtmosphere())
									return 2;	// My target is outside the atmosphere. I don't know how to get there from here.

								//DHH Work needed here when target is aboard a vehicle or spacecraft, etc.

								// Get my transform and location relative to the planet.
								AuMatrix4x4 tp;
								GetTransformRelativeToParent(tp, llworld);
								AuPoint myPlanetLoc = tp.Project(AuPoint(0,0));

								// Get the target location relative to the planet.
								// Target location is relative to the target atom.
								AuMatrix4x4 tt;
								orderTarget->GetTransformRelativeToParent(tt, llworld);
								AuPoint orderPlanetLoc;
								AuPoint orderLoc(OrderLoc());
								if(orderLoc == AuPoint(0,0))
									orderPlanetLoc = tt.Project(orderTarget->TargetLocation());	// Target loc relative to the planet
								else
									orderPlanetLoc = tt.Project(orderLoc);	// Target loc relative to the planet.

								// Make the target loc a vector relative to me.
								AuPoint moveVec = tp.UnProjectVector(orderPlanetLoc - myPlanetLoc);
								if(lookAtTarget)
									moveVec -= CameraOffset();				// Relative to my eye is better for targeting.

								m_SentientLock.lockForWrite();
								m_MoveDistLimit = moveVec.Length_2D();
								float mdl = m_MoveDistLimit;
								m_SentientLock.unlock();

								if(mdl < closeEnough)
								{
									// Make the target loc a vector relative to me.
									if(lookAtTarget)
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Stand);
									return 1;	// Close enough to my destination.
								}
								else
								{
									// Move toward the target loc.
									MoveToGlobeLoc(llworld, AuPoint(0,0), moveVec, mdl, moveVec.PolarIncline());
								}
								/*
								else if(mdl > RUN_FAR_ENOUGH)
								{
									if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::SwimFast);
									else if(CanFly())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::FlyFast);
									else if(IsInWater())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::SwimFast);
									else
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::StandRun);
								}
								else
								{
									if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Swim);
									else if(CanFly())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Fly);
									else if(IsInWater())
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Swim);
									else
										MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::StandWalk);
								}
								*/
							}
						}
					}
				}
			}
			else
			{
				// I am not in a spacecraft or atmosphere range of a planet.
				//qDebug("%s I am not in a spacecraft or atmosphere range of a planet.", (const char*)Name().toAscii());
				// Try to go directly to my target location.
				DaWorld* planet = FindParentPlanetInEnv();
				if(!planet)
					return 2;	// No parent planet is a problem I can't deal with.

				orderTarget = planet->FindIdRecursive(OrderTarget());
				if(orderTarget)
				{
					// My order target is at this planet.
					AuMatrix4x4 tt;
					orderTarget->GetTransformRelativeToParent(tt, planet);
					AuPoint orderPlanetLoc;
					AuPoint orderLoc(OrderLoc());
					if(orderLoc == AuPoint(0,0))
						orderPlanetLoc = tt.Project(orderTarget->TargetLocation());	// Target loc relative to the planet.
					else
						orderPlanetLoc = tt.Project(orderLoc);	// Target loc relative to the planet.

					// Convert target planet loc to be relative to me.
					AuMatrix4x4 tme;
					GetTransformRelativeToParent(tme, planet);
					AuPoint moveVec = tme.UnProject(orderPlanetLoc);
					if(lookAtTarget)
						moveVec -= CameraOffset();				// Relative to my eye is better for targeting.

					m_SentientLock.lockForWrite();
					m_MoveDistLimit = moveVec.Length();
					float mdl = m_MoveDistLimit;
					m_SentientLock.unlock();

					//qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "moveVec" << moveVec << "dist" << mdl
						//<< "closeEnough" << closeEnough;
					if(mdl < closeEnough)
					{
						// Make the target loc a vector relative to me.
						if(lookAtTarget)
							MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::Stand);
						return 1;	// Close enough to my destination.
					}

					// Move toward the target location.
					MoveInDirection(moveVec, moveVec.PolarIncline(), AuPose::StandWalk);
					return 0;
				}
				else
					return 2;	// My target is not at this planet.
			}
		}

		// Update movement strategy.
		// Accumulate recent path location information relative to the m_AIRecentParent.
		m_SentientLock.lockForWrite();
		m_AIRecentPath.append(Location());
		if(m_AIRecentPath.count() > 30)
			m_AIRecentPath.remove(0);
		bool touchingWall = MoveResult() == CT_HitParent;
		if(touchingWall)
		{
			// I am touching a wall.
			if(m_AIRecentPath.size() > 2)
			{
				// I have accumulated memory of my recent movement.
				int ckprev = m_AIRecentPath.size() - 1 - 2;		// Check two path locations back.
				if(m_AIRecentPath.last().Distance(m_AIRecentPath[ckprev]) < 1)
				{
					// I am touching a wall and I haven't moved much.
					ClearRouteSentientLocked();
					m_AIStrategy = 1;	// Pick a random facing.
					strategy = m_AIStrategy;
					m_AIStrategyTime.restart();
				}
			}
		}
		/*
		if(selapsed > 15000)
		{
			// I've been using this strategy for a few seconds. Am I stuck?
			AuPoint min, max;
			m_AIRecentPath.Extents(min, max);
			double rangeDiag = min.Distance(max);
			if(rangeDiag < 20)
			{
				// Movement strategy doesn't appear to be working. Try a different strategy.
#if !defined(QT_NO_DEBUG)
				if(false && Is(s_PathTrace))
					qDebug() << "DaSentient::MoveToOrderLoc" << Id() << Name() << "stuck, picking a random facing";
#endif
				ClearRouteSentientLocked();
				m_AIStrategy = 1;	// Pick a random facing.
				strategy = m_AIStrategy;
				m_AIStrategyTime.restart();
			}
		}
		*/
		m_SentientLock.unlock();

		// When I must turn significantly while moving normally, stop to complete the turn.
		if(strategy == 0 && fabs(TurnZ()) > PIf*3.f/8.f)
		{
			m_SentientLock.lockForWrite();
			m_AIStrategy = 4;					// Finish turning before continuing forward.
			m_SentientLock.unlock();

			if(!IsLadderPose())
			{
				// Not on a ladder.
				// Select an appropriate turning pose.
				AuPose pose;
				if(IsInsideSpacecraft())
					pose = AuPose::Stand;
				else
				{
					float cght = CGHeightStanding();
					if(WaterDepth() >= cght)
						pose = AuPose::TreadWater;
					else if(CanFly() && AltitudeAGL() > cght*2 + CameraOffset().z)
					{
						// Flyers will hover or circle.
						if(CanHover())
							pose = AuPose::Hover;
						else
							pose = AuPose::Glide;
					}
					else
						pose = AuPose::Stand;
				}

				// Pose my body.
				DoPose(pose);
			}
		}
	}
	else if(strategy == 1)
	{
		// Pick a random facing.
		SetTurnZ((float)g_Au.Dice().Roll(1,361,-181) * DEG2RADf);
		if(TiltHeadTo(0))
			SetModified(NOTIFY_Scene);			// Head tilt not stored in db but client likes to know.

		// Select a stationary pose while turning.
		float cght = CGHeightStanding();
		if(WaterDepth() >= cght)
			DoPose(AuPose::TreadWater);
		else if(CanFly() && AltitudeAGL() > cght*2 + CameraOffset().z)
		{
			// Flyers will hover or circle.
			if(CanHover())
				DoPose(AuPose::Hover);
			else
				DoPose(AuPose::Glide);
		}
		else
			DoPose(AuPose::Stand);

		// Update movement strategy.
		m_SentientLock.lockForWrite();
		m_AIStrategy = 2;				// Wait for facing change to complete.
		m_AIStrategyTime.restart();
		m_SentientLock.unlock();
	}
	else if(strategy == 2)
	{
		// Waiting for facing change to complete.
		if(TurnZ() == 0 || selapsed > 2000)
		{
			// I should be done turning by now.
			// Decide to run or walk.
			if(g_Au.Dice().Roll(1,3) == 1)
			{
				// Move fast
				if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::SwimFast);
				else if(CanFly())
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::FlyFast);
				else if(WaterDepth() >= CGHeightStanding())
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::SwimFast);
				else
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::StandRun);
			}
			else
			{
				// Move normal speed
				if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::Swim);
				else if(CanFly())
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::Fly);
				else if(WaterDepth() >= CGHeightStanding())
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::Swim);
				else
					MoveInDirection(AuPoint(0,1,0), 0, AuPose::StandWalk);
			}

			// Update movement strategy.
			m_SentientLock.lockForWrite();
			m_AIStrategy = 3;	// Run/walk straight ahead for a bit.
			m_AIStrategyTime.restart();
			m_SentientLock.unlock();
		}
	}
	else if(strategy == 3)
	{
		// Run/walk for a bit.
		if(selapsed > 1000)
			ResetStrategy(false);	// Revert to primary movement strategy.
	}
	else if(strategy == 4)
	{
		// Waiting for facing change to complete.
		if(TurnZ() == 0 || selapsed > 2000)
		{
			// I should be done turning by now.
			m_SentientLock.lockForWrite();
			m_AIStrategy = 0;									// Return to default movement strategy.
			m_SentientLock.unlock();
		}
	}
	else if(strategy == 5)
	{
		// Waiting for facing change to complete after hitting something.
		if(fabs(TurnZ()) < DEG2RAD)
		{
			// I am done turning, close enough.
			if(Is(s_PathTrace))
				qDebug() << "DaSentient::MoveToOrderLoc done turning away from obstruction";
			// Move normal speed
			if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
				MoveInDirection(AuPoint(0,1,0), 0, AuPose::Swim);
			else if(CanFly())
				MoveInDirection(AuPoint(0,1,0), 0, AuPose::Fly);
			else if(WaterDepth() >= CGHeightStanding())
				MoveInDirection(AuPoint(0,1,0), 0, AuPose::Swim);
			else
				MoveInDirection(AuPoint(0,1,0), 0, AuPose::StandWalk);

			// Set a small move distance limit so I don't bounce around too bad. This should make me move really slowly.
			m_SentientLock.lockForWrite();
			m_MoveDistLimit = 1.5;

			// Update movement strategy.
			m_AIStrategy = 6;	// Run/walk for a bit.
			//m_AIStrategyTime.restart();
			m_SentientLock.unlock();
		}
	}
	else if(strategy == 6)
	{
		// Run/walk for a bit.
		m_SentientLock.lockForWrite();
		m_AIRecentPath.append(Location());
		if(m_AIRecentPath.count() > 30)
			m_AIRecentPath.remove(0);

		// Set a small move distance limit so I don't bounce around too bad. This should make me move really slowly.
		m_MoveDistLimit = 1.5;
		if(selapsed > 1000)
		{
			if(Is(s_PathTrace))
				qDebug() << "DaSentient::MoveToOrderLoc done moving away from obstruction";
			// Update movement strategy.
			m_AIStrategy = 0;									// Revert to primary movement strategy.
			//m_AIStrategyTime.restart();
		}
		m_SentientLock.unlock();
	}
	else
	{
		qWarning("DaSentient::MoveToOrderTarget %d %s invalid movement strategy %d. Reverting to primary strategy.",
				Id().GetID(), (const char*)Name().toLatin1(), strategy);
		ResetStrategy(true);
	}

	// Moving to the target.
	return 0;
}

// Returns 0 when I am still moving to the target.
// Returns 1 when I have reached the target location.
// Returns 2 when no path to the target room exists.
// 'ship' is my parent spacecraft.
// 'toRoomId' is zero to move outside via the shortest path.
int DaSentient::MoveToCapsuleLoc(DaCapsule* ship, const AuPoint& toLoc)
{
	if(!IsInCapsule())
		return 0;	// I moved out of the capsule.
	if(!g_Au.m_EnableCitizenPathing)
		return 0;

	// Find the best path to get to the destination location.
	bool isCrew = ship->IsCrewOrTroop(Id());
	if(IsRouteEmpty())
	{
#if !defined(QT_NO_DEBUG)
		if(Is(s_PathTrace))
		{
			//qDebug() << "DaSentient::MoveToCapsuleLoc" << Id() << Name() << "ship" << ship->Id() << ship->Name() << "loc" << toLoc;
			g_Au.m_LogMessages |= LOG_Path;
		}
#endif
		m_SentientLock.lockForWrite();
		ClearRouteSentientLocked();

		DaSc* sc = dynamic_cast<DaSc*>(ship);
		if(sc)
		{
			if(!m_AIScRoute)
				m_AIScRoute = new AuScRoute;

			if(!sc->Route(*m_AIScRoute, Location(), toLoc, this, isCrew))
			{
				// No route to the destination.
#if !defined(QT_NO_DEBUG)
				if(Is(s_PathTrace))
				{
					qDebug() << "DaSentient::MoveToCapsuleLoc" << Id() << Name() << "no sc route to loc";
					g_Au.m_LogMessages &= ~LOG_Path;
				}
#endif
				m_AIScRoute->clear();
				m_SentientLock.unlock();
				return 2;
			}
			// Successful path so restart my movement strategy.
			m_AIStrategyTime.restart();

			// If the first route location is not a ladder, I must let go or I'll be stuck here.
			if(!m_AIScRoute->FirstIsLadder() && IsLadderPose())
				Stand();

#if !defined(QT_NO_DEBUG)
			if(Is(s_PathTrace))
			{
				qDebug() << *m_AIScRoute;
				g_Au.m_LogMessages &= ~LOG_Path;
			}
#endif
		}

		bool hasRoute = false;
		if(m_AIScRoute && !m_AIScRoute->empty())
			hasRoute = true;
		m_SentientLock.unlock();
		if(!hasRoute)
			return 1;											// Route function is happy with empty route. I must be there.
	}

	// Follow the route I have established.
	MoveInsideCapsule(ship, isCrew);
	return 0;													// I am still moving toward my destination.
}

void DaSentient::MoveInsideCapsule(DaCapsule* ship, bool isCrew)
{
	if(!IsInSpacecraft())
		return;													// I moved outside the spaceship?

	bool hasScRoute = false;
	m_SentientLock.lockForRead();
	if(m_AIScRoute && !m_AIScRoute->empty())
		hasScRoute = true;
	m_SentientLock.unlock();

	if(hasScRoute)
	{
		// Location hash value is based on coordinates in the space of the design.
		// When my location hash value matches the next route location, it's time to update the route.
		AuPoint loc = Location();
		double cgHt = GroundHeightAboveCG();
		AuPoint upVec = Transform().ProjectVector(0,0,1);
		quint16 roomId = CapsuleRoomId();
		QString atStop;

		m_SentientLock.lockForWrite();
		m_MoveDistLimit = -1;									// Cancel move distance limit.

		DaSc* sc = dynamic_cast<DaSc*>(ship);
		if(sc)
		{
			atStop = sc->TurboLiftStop(roomId);
			bool wasLadderBeforeReach = m_AIScRoute->FirstIsLadder();
			if(wasLadderBeforeReach && IsLadderPose())
			{
				// First route location is a ladder. I am on a ladder. Does my ladder go to the first route location?
				if(!sc->IsOnLadderTo(loc, m_AIScRoute->first()))
				{
					// I must get off the first ladder, so I can get onto the next ladder.
					//qDebug() << *this << "releasing ladder to move to next ladder";
					wasLadderBeforeReach = false;
					Stand();
				}
			}
			//qDebug() << "DaSentient::MoveInsideCapsule" << *this << "route update, going to ladder" << wasLadderBeforeReach
				//<< "isLadderPose" << IsLadderPose();
			bool touchingDirt = OnGround() != OG_None && !IsInsideSpacecraft() && IsInAtmosphere();	// Dirt, not a ship deck.
			bool reached = m_AIScRoute->Update(loc, cgHt, upVec, roomId, atStop, IsLadderPose(), touchingDirt,
					sc->LockDoorStates());
			sc->UnlockDoorStates();
			if(reached)
			{
				//qDebug() << "DaSentient::MoveInsideCapsule reached route point" << Id() << *m_AIScRoute << "wasLadder"
					//<< wasLadderBeforeReach << "isLadderPose" << IsLadderPose();
				// I have successfully reached a waypoint along my route.
				m_AIStrategyTime.restart();						// Move strategy is working; reset time to avoid getting frustrated.
				if(m_AIScRoute->isEmpty())
				{
					// Route complete, at the destination or an intermediate stop.
					m_SentientLock.unlock();
					ThinkSoon();
					return;
				}

				if(IsLadderPose() && !wasLadderBeforeReach)
				{
					// I was in a ladder pose and switched to a segment that is not a ladder.
					m_SentientLock.unlock();
					ThinkSoon();
					//qDebug() << *this << "reached end of ladder, standing";
					Stand();
					return;
				}
			}
		}

		/*
		switch(MoveResult())
		{
		case CT_Miss:
			break;
		case CT_HitParent:
			qDebug() << "DaSentient::MoveInsideCapsule" << Id() << Name() << "Hit Parent";
			break;
		case CT_HitPeer:
			qDebug() << "DaSentient::MoveInsideCapsule" << Id() << Name() << "Hit Peer";
			break;
		case CT_JoinedPeer:
			qDebug() << "DaSentient::MoveInsideCapsule" << Id() << Name() << "Joined Peer";
			break;
		}
		*/
		if(m_AIScRoute->empty())
		{
			m_SentientLock.unlock();
			return;
		}

		// Calculate movement vector to next path location relative to me.
		AuPoint routeLoc(m_AIScRoute->FirstLoc());
		AuPoint moveVec = Transform().UnProject(routeLoc);
		quint16 doorId = m_AIScRoute->FirstDoorId();
		bool isLadder = m_AIScRoute->FirstIsLadder();
		quint16 liftId = m_AIScRoute->FirstLiftId();
		QString liftStop = m_AIScRoute->FirstLiftStop();

		m_SentientLock.unlock();

		// Select the appropriate pose and head tilt.
		AuPose pose;
		double headTilt = 0;									// Head looks straight and level, unless moving up|down.
		//if(IsLadderPose())
			//qDebug() << "IsLadderPose moveVec" << Id() << moveVec << moveVec.Length() << "touchingGnd" << IsTouchingGround();
		if(IsLadderPose() && moveVec.Length() > .5 && (!IsTouchingGround() || moveVec.z > 0))
		{
			// I am climbing a ladder and I am not at my destination yet.
			//qDebug() << *this << "climbing ladder" << moveVec;
			pose = AuPose::LadderClimb;
			SetTurnZ(0);

			// Look in the movement direction to follow the ladder path.
			headTilt = moveVec.PolarIncline();

			// Limit my movement so I don't go too fast on the ladder.
			m_SentientLock.lockForWrite();
			m_MoveDistLimit = moveVec.Length();					// distance to the next waypoint in my route
			if(m_MoveDistLimit > FeetPerMeter())
				m_MoveDistLimit = (float)FeetPerMeter();
			m_SentientLock.unlock();

			// Open the door.
			if(doorId && !ship->IsDoorOpen(doorId))
			{
				// Crew hatches can be locked. Hull hatches are never locked from the inside. Other hatches can't be locked.
				if(isCrew || !ship->CrewDoorLocked(doorId))
					ship->OpenDoor(doorId);
				else
				{
					// Hatch won't open so give up.
					pose = AuPose::Stand;							// Get off the ladder.
					headTilt = 0;
					m_SentientLock.lockForWrite();
					m_AIScRoute->clear();						// Door won't open so force recheck of path next time around.
					m_SentientLock.unlock();
				}
			}

			// Move the lift to the next stop.
			if(sc && liftId && CapsuleRoomId() == liftId && !liftStop.isEmpty())
			{
				//qDebug() << "DaSentient::MoveInsideCapsule" << Id() << Name() << "on ladder, calling lift" << liftId << liftStop;
				pose = AuPose::Stand;								// Wait for the lift to move.
				headTilt = 0;
				if(atStop == liftStop)
				{
					// Lift is at my desired stop.
					m_SentientLock.lockForWrite();
					m_AIScRoute->removeFirst();
					m_SentientLock.unlock();
				}
				else
				{
					int result  = sc->CallTurboLift(liftId, liftStop);
					switch(result)
					{
					case 0:	//  0=lift already at stop, door already open
					case 1:	//  1=no door at turbo lift stop
					case 4:	//  4=broken turbo lift data
					case 6: //  6=lift already at stop, door opened
						// wut?
						ResetStrategy(true);
						break;
					case 2:	//  2=not ready to move
					case 3:	//  3=a door is stuck open,
					case 5:	//  5=turbo lift moved to stop and door opened
					case 7:	//  7=a door is blocked open
						// Wait. My MovedByTurboLift function handles this.
						break;
					}
				}
			}
		}
		else
		{
			// I am not climbing a ladder, or at least I no longer want to be climbing a ladder.
			// Limit my movement to end at the next route waypoint.
			m_SentientLock.lockForWrite();
			m_MoveDistLimit = moveVec.Length();					// distance to the next waypoint in my route
			float mdl = m_MoveDistLimit;
			m_SentientLock.unlock();

			// Run|walk depending on the distance.
			if(mdl < RUN_FAR_ENOUGH)
			{
				// I must think often to position myself at the destination accurately.
				ThinkSoon();
				pose = AuPose::StandWalk;
			}
			else
			{
				// Not changing decks and not close to the end of my path.
				pose = AuPose::StandRun;							// Run to cover long distances quickly.
			}

			// Determine if I am planning to change decks at the next waypoint.
			if(!IsInSpacecraft())
				return;											// I moved outside the spaceship?

			if(mdl < FeetPerMeter())
			{
				// Almost at my destination.
				// Open the door that is there, if any.
				if(doorId && !ship->IsDoorOpen(doorId))
				{
					// Crew hatches can be locked. Hull hatches are never locked from the inside. Other hatches can't be locked.
					if(isCrew || !ship->CrewDoorLocked(doorId))
						ship->OpenDoor(doorId);
					else
					{
						// Hatch won't open so give up.
						pose = AuPose::Stand;						// Get off the ladder.
						headTilt = 0;
						m_SentientLock.lockForWrite();
						m_AIScRoute->clear();					// Door won't open so force recheck of path next time around.
						m_SentientLock.unlock();
					}
				}

				// Move the lift to the next stop.
				if(sc && liftId && CapsuleRoomId() == liftId && !liftStop.isEmpty())
				{
					pose = AuPose::Stand;							// Wait for the lift to move.
					headTilt = 0;
					if(atStop == liftStop)
					{
						// Lift is at my desired stop.
						m_SentientLock.lockForWrite();
						m_AIScRoute->removeFirst();
						m_SentientLock.unlock();
					}
					else
					{
						int result  = sc->CallTurboLift(liftId, liftStop);
						switch(result)
						{
						case 0:	//  0=lift already at stop, door already open
						case 1:	//  1=no door at turbo lift stop
						case 4:	//  4=broken turbo lift data
						case 6: //  6=lift already at stop, door opened
							// wut?
							ResetStrategy(true);
							break;
						case 2:	//  2=not ready to move
						case 3:	//  3=a door is stuck open,
						case 5:	//  5=turbo lift moved to stop and door opened
						case 7:	//  7=a door is blocked open
							// Wait. My MovedByTurboLift function handles this.
							break;
						}
					}
				}
			}

			// Grab the ladder that is there, if any.
			if(isLadder && !IsLadderPose() && GrabLadderOnMyShipAt(routeLoc))
				pose = CurrentGeneralPose();	// Close enough to grab the ladder.
		}

#if !defined(QT_NO_DEBUG)
		if(false && Is(s_PathTrace))
		{
			m_SentientLock.lockForRead();
			qDebug() << "DaSentient::MoveInsideCapsule" << Id() << Name() << "onLadder" << IsLadderPose() << "move" << moveVec
				<< "headTilt" << headTilt*RAD2DEGf << "pose" << pose << "moveLimit" << m_MoveDistLimit
				<< "sc route" << m_AIScRoute->count() << m_AIRecentCollide;
			m_SentientLock.unlock();
		}
#endif

		MoveInDirection(moveVec, headTilt, pose);
	}
}

void DaSentient::MoveInDirection(const AuPoint& moveVec, float headTilt, const AuPose& pose)
{
	float turnZ;
	AuPose nuPose(pose);
	if(nuPose == AuPose::LadderClimb || nuPose == AuPose::Ladder)
	{
		// Keep facing the ladder.
		turnZ = 0;		// Do not turn. I already faced the appropriate ladder direction when I grabbed the ladder.
	}
	else
	{
		// Turn toward the destination location.
		turnZ = moveVec.ArcTangent_2D() - PIf/2;
		if(IsMovingPose() && !IsLadderPose() && Order() != ORDER_AttackTarget && !IsInSpacecraft())
		{
			// Too much precision while moving and they end up on top of each other.
			// A few degrees variance will induce a little wander to their walk.
			// Totally random wander turns a crowd of them into a wiggling gaggle.
			// This logic causes them to lean consistently to the left or right by a consistent varying amount based on their id.
			// This variance must not be applied to stationary poses or while attacking because it affects their aim.
			if(Id().GetID()&1)
				turnZ -= (DEG2RADf * g_Au.Dice().Roll(1,Id().GetID()&7));	// Leans to the left.
			else
				turnZ += (DEG2RADf * g_Au.Dice().Roll(1,Id().GetID()&7));	// Leans to the right.
		}
		if(turnZ < -PIf)
			turnZ += PIf*2;
	}
	if(turnZ != TurnZ())
	{
		// Don't run in a big circle.
		if(fabsf(turnZ) > 10*DEG2RADf)
			nuPose = nuPose.Stationary();
		SetTurnZ(turnZ);
	}

	// Look straight ahead.
	if(TiltHeadTo(headTilt))
		SetModified(NOTIFY_Scene);			// Head tilt not stored in db but client likes to know.

	// Move toward the destination location.
	DoPose(nuPose);
}

// 'hit' is the atom that was hit.
// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaSentient::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	if(hitNormal.IsZero())
	{
		SetVector(AuPoint(0,0));
		return AuPoint(0,0);
	}
	AuPoint bounceVec = DaCreature::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
	if(hitShield)
		return bounceVec;	// Going this fast just bounce.

	/*
#if !defined(QT_NO_DEBUG)
	if(Is(s_PathTrace))
	{
		int steps = 0;
		m_SentientLock.lockForRead();
		if(m_AIScRoute)
			steps += m_AIScRoute->size();
		m_SentientLock.unlock();

		qDebug() << "DaSentient::MovementBlocked" << Id() << Name() << "hit" << hit->Id() << hit->Name()
			<< "hitNormal" << hitNormal << hitNormal.ZRotation()*RAD2DEGf << "route steps" << steps;
	}
#endif

	// Avoid the object that was hit if I am not already avoiding the object.
	m_SentientLock.lockForRead();
	int strategy = m_AIStrategy;
	m_SentientLock.unlock();
	if(strategy != 5 && strategy != 6)
	{
		// Turn away from the object.
		// Calculate the turn angle to align with the hit normal.
		// Positive angle is a left turn; negative angle is a right turn.
		float turnZ = hitNormal.ZRotation() - PIf/2;
		if(turnZ < -PIf)
			turnZ += PIf*2;										// Final range is [-PI..PI]

		//turnZ /= 2;											// Turn half-way toward the surface normal to get away.
		if(turnZ != TurnZ())
		{
			// Decide how to react to the collision since I am not already reacting to a collision.
#if !defined(QT_NO_DEBUG)
			if(Is(s_PathTrace))
			{
				const DaCapsule* ship = dynamic_cast<const DaCapsule*>(Parent());
				if(ship)
				{
					quint32 myHash = AuPath::MakeHashKey(Location() + ship->DesignCG());
					int cellX = (myHash & 0x0000ff00) >> 8;
					int cellY = myHash & 0x000000ff;
					qint8 cellDeck = (myHash & 0x00ff0000) >> 16;
					qDebug() << "DaSentient::MovementBlocked" << Id() << Name() << "hitNormal" << hitNormal
						<< (turnZ<0?"turn right":"turn left") << turnZ*RAD2DEGf
						<< "path cell" << QString("0x%1").arg(myHash,0,16) << "(" << cellX << cellY << cellDeck << ")"
						<< "roomId" << CapsuleRoomId() << "sub" << CapsuleRoomNumber() << "type" << CapsuleRoomType() << strategy;
				}
				else
				{
					qDebug() << "DaSentient::MovementBlocked" << Id() << Name() << "hitNormal" << hitNormal
						<< (turnZ<0?"turn right":"turn left") << turnZ*RAD2DEGf;
				}
			}
#endif
			// Turn parallel to the obstruction if I'm heading into it. I am heading into it if turnZ is more than 90 degrees left
			// or right.
			bool turnAway;
			if(turnZ > 90.*DEG2RADf)
			{
				SetTurnZ(turnZ - 88.*DEG2RADf);
				turnAway = true;
			}
			else if(turnZ < -90.*DEG2RADf)
			{
				SetTurnZ(turnZ + 88.*DEG2RADf);
				turnAway = true;
			}
			else
			{
				SetTurnZ(turnZ);
				turnAway = false;
			}

			bool poseChanged;
			if(turnAway)
			{
				// A lot of turning is needed to get away from the obstruction.
				// Stop moving while turning.
				poseChanged = SetGeneralPose(StationaryPose());
			}
			else
			{
				// A little turning is needed to get away from the obstruction.
				// Stop moving fast while turning.
				if(WaterDepth() >= CGHeightStanding())
					poseChanged = SetGeneralPose(AuPose::Swim);
				else if(CanFly() && IsFlying())
					poseChanged = SetGeneralPose(AuPose::Fly);
				else
					poseChanged = SetGeneralPose(AuPose::StandWalk);
			}
			if(poseChanged)
			{
				// Kick loose from at-rest if my nu pose is a moving pose.
				if(IsMovingPose())
					SetInMotion();
				SetModified(NOTIFY_Scene);
			}

			// Update movement strategy.
			m_SentientLock.lockForWrite();
			m_AIStrategy = 5;			// Wait for turn to complete then go forward a few steps.
			//m_AIStrategyTime.restart();
			m_SentientLock.unlock();
		}
	}
	*/

	// Remember the last atom I bumped into. It might be my target atom.
	m_SentientLock.lockForWrite();
	m_AIRecentCollide = hit->Id();
	ResetStrategyNoLock(true);			// Reset my strategy and routing.
	m_SentientLock.unlock();

	// Stop trying to move.
	SetGeneralPose(StationaryPose());

	return bounceVec;
}

// Turrets are operated by troops, not crew. Troops check for threats in DoBattleStation.
void DaSentient::OperateTurret(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	//if(Is(AuId(241530682)))
		//qDebug() << "DaSentient::OperateTurret" << Id() << Name() << "thinkRate" << AIThinkRate()
			//<< "forPose" << ThinkRateForPose(ORDER_BattleStation) << "turnZ" << TurnZ()
			//<< "attackTime" << AIAttackTime().elapsed();
	// While operating a turret, my think rate changes after a few seconds of not shooting any targets.
	UpdateThinkRate();

	// When rotating in a turret, let it finish turning to the direction I selected before thinking again.
	if(TurnZ() != 0)
		return;

	AuElapsedTimer tm(AIAttackTime());
	if(tm.isValid())
	{
		// Impose personal rate-of-fire limit based on rank, regardless of weapon capability.
		if(tm.elapsed() < (int)TurretFireMs())
			return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	// Turrets cannot rotate when there is no power to the ship or the turret is broken.
	if(ship->CapacitorLevel() <= 0)
		return;
	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Ensure my turret has ammo.
	if(!ship->TurretRoundsCount(StationId()))
	{
		DoShipReloadTurret(ship, StationId());
		StayIdle();
		return;
	}

	if(Posture() == POSTURE_HoldFire || ship->MissionOrderType() == AuMissionOrder::StandDown || ship->RepairSecondsRemaining())
	{
		// I'm ordered to hold my fire or the ship is getting repairs.
		// Look alive but don't actually shoot the weapon.
		StayIdle();
		return;
	}

	if(!ship->IsSpaceStation() && !ship->HasFirstOfficer() && !ship->FindPersonOnLineRecursive())
	{
		// Don't fire when this ship is not a station and it doesn't have anyone online or any AI officers.
		StayIdle();
		return;
	}

	if(!ship->TurretCanFire(StationId()))
	{
		// Turret not ready to fire. Either no ammo or rate of fire limit exceeded.
		if(!ship->TurretRoundsCount(StationId()))
			StayIdle();	// No ammo so it's ok to look around. Don't look around when waiting to pull the trigger again.
		return;
	}

	AuPoint emitLoc = WeaponMuzzleLoc();						// Relative to my body.
	AuPoint turretEmitLoc = EnvTransform().Project(emitLoc);	// Relative to the environment.
	AuPoint turretHemiVec = ship->TurretHemiVector(CurrentGeneralPose(), StationId());
	turretHemiVec = ship->EnvTransform().ProjectVector(turretHemiVec);	// Relative to the environment.

	// Fire up to the turret gun's maximum range since there is no cost for plinking.
	COMMODITYID module = ship->SystemModule(StationSysId());
	float fireRange;
	if(module == CID_EnergyWeaponTurretModule)
		fireRange = ship->TurretRange(StationId());
	else if(module == CID_WeaponLauncherTurretModule)
		fireRange = AttackRange();
	else// if(module == CID_BallisticWeaponTurretModule)
	{
		if(ship->IsInAtmosphere())
			fireRange = 5 * 500;	// Range the shell will travel in its 5 sec lifetime at 500 fps.
		else
			fireRange = AttackRange();
	}

	DaAtom* system = ship->FindEnvironmentAtom();
	if(!system)
		return;	// D:

	AuId gunnerWorldId;
	int gunnerDomain = ship->GunnerDomain(gunnerWorldId);

	// Look for a target in the system.
	// The current mission order may specify a target atom to attack or plunder.
	const DaAtom* targetAtom = 0;
	AuPoint targetSysLoc;
	const DaAtom* standDownAtom = 0;							// Target to ignore because they are standing down.
	AuMissionOrder::TYPE t = ship->MissionOrderType();
	AuId tid = ship->MissionOrderTarget();

	if(t == AuMissionOrder::Attack)
	{
		// When ordered, I will attack friendlies when in range.
		targetAtom = system->FindIdRecursive(tid);
		if(targetAtom && !targetAtom->IsShipTurretTarget(ship->Id(), gunnerDomain, gunnerWorldId, 0, Posture(), turretEmitLoc,
					fireRange, turretHemiVec, targetSysLoc))
			targetAtom = 0;	// Target atom not found or it is obstructed/out of my field of fire.
	}
	else if(t == AuMissionOrder::Plunder)
	{
		// When ordered, I will plunder friendlies when in range.
		targetAtom = system->FindIdRecursive(tid);
		if(targetAtom)
		{
			if(targetAtom->IsShipTurretTarget(ship->Id(), gunnerDomain, gunnerWorldId, 0, Posture(), turretEmitLoc, fireRange,
						turretHemiVec, targetSysLoc))
			{
				// Mission order target is something I can shoot with a turret.
				// Plunder order needs to know if target is standing down.
				// Plunder order does not attack targets who are standing down.
				const DaVessel* target = dynamic_cast<const DaVessel*>(targetAtom);
				bool targetStandingDown;
				if(target)
					targetStandingDown = target->IsStandingDown();	// I will care if the target is standing down.
				else
					targetStandingDown = false;					// Target is not a ship or not in the system so it can't stand down.

				if(targetStandingDown)
				{
					// Plunder target is standing down.
					standDownAtom = targetAtom;			// Do not target this atom.
					targetAtom = 0;
				}
			}
			else
				targetAtom = 0;
		}
	}

	if(!targetAtom && !ship->LockedBlip().IsNull() && ship->LockedBlip() != tid)
	{
		// Did not find a specific target to attack.
		// Ship is locked on to a target, try it first.
		targetAtom = system->FindIdRecursive(ship->LockedBlip());
		if(targetAtom == standDownAtom)
			targetAtom = 0;
		if(targetAtom)
		{
			if(targetAtom->IsShipTurretTarget(ship->Id(), gunnerDomain, gunnerWorldId, EmpireId(), Posture(), turretEmitLoc,
						fireRange, turretHemiVec, targetSysLoc))
			{
				// The blip is a fair target unless I'm in DefendSelf posture and it hasn't done me any damage.
				if(Posture() == POSTURE_DefendSelf && !ship->IsDamagedBy(targetAtom->Id()) &&
						targetAtom->AtomType() != ATOM_Asteroid)
					targetAtom = 0;	// Target atom has not hurt me and its not an asteroid.
			}
			else
				targetAtom = 0;	// Target atom not found or it is obstructed/out of my field of fire.
		}
	}

	if(!targetAtom)
	{
		// Not attacking a specific target or locked-on target so check all sensor blips.
		double targetRange = fireRange;
		system->FindClosestShipTurretTargetRecursive(ship->Id(), gunnerDomain, gunnerWorldId, EmpireId(), Posture(), turretEmitLoc,
				fireRange, turretHemiVec, targetAtom, targetRange, targetSysLoc);
		if(targetAtom == standDownAtom)
			targetAtom = 0;
		if(targetAtom)
		{
			// The blip is a fair target unless I'm in DefendSelf posture and it hasn't done me any damage.
			if(Posture() == POSTURE_DefendSelf && !ship->IsDamagedBy(targetAtom->Id()) &&
					targetAtom->AtomType() != ATOM_Asteroid)
				targetAtom = 0;	// Target atom has not hurt me and its not an asteroid.
		}
	}

	if(targetAtom)
	{
		// We found a target in range to shoot at.
		// I've already insured the target vec is within my field of fire so turn to face it.
		// Calculate target vector in my space.
		AuPoint targetVec;
		DaSc* sc = dynamic_cast<DaSc*>(ship);
		if(sc)
		{
			// I am operating a remote turret gun in an Sc design ship.
			float baseRot = Rotation().z;
			AuMatrix4x4 mAim;
			sc->TurretAimMatrix(StationId(), baseRot, mAim);
			AuPoint shipTargLoc = sc->EnvTransform().UnProject(targetSysLoc);	// Target location relative to the ship.
			AuPoint shipEmitLoc = mAim.Translation();							// Turret gun pivot at its current position.
			AuPoint shipTargetVec = shipTargLoc - shipEmitLoc;					// Vec to target from muzzle, relative to the ship.

			// Calculate rotation angles to aim at target.
			AuPoint mtvec = mAim.UnProjectVector(shipTargetVec);				// Vec to target, in the space of the gun barrel.
			// Base can rotate all around in either direction.
			double baseRotAim = mtvec.YRotation() + PI/2;						// Base rotation angle needed to aim at the target.
			if(baseRotAim > PI)
				baseRotAim -= PI*2;
			if(baseRotAim < -PI)
				baseRotAim += PI*2;
			// Turret gun can only go 90 degrees up or down.
			double gunRotAim = mtvec.XRotation() + PI/2;						// Gun elevation angle needed to aim at the target.
			if(gunRotAim > PI)
				gunRotAim -= PI*2;
			if(gunRotAim < -PI)
				gunRotAim += PI*2;
			if(gunRotAim > PI/2)
				gunRotAim = PI/2;
			else if(gunRotAim < -PI/2)
				gunRotAim = -PI/2;

			// Construct movement vector relative to me that will aim the gun.
			targetVec = AuPoint(0,1,0)
				.RotateX(gunRotAim)
				.RotateZ(baseRotAim);
			//qDebug() << "DaSentient::OperateTurret" << Id() << Name() << "target" << targetAtom->Id() << targetAtom->Name()
				//<< "range" << mtvec.Length()
				//<< "baseRot" << baseRot*RAD2DEG
				//<< "mtvec" << mtvec
				//<< "baseRotAim" << baseRotAim*RAD2DEG << "gunRotAim" << gunRotAim*RAD2DEG
				//<< "targetVec" << targetVec << targetVec.Length();
		}

		double headTilt = targetVec.PolarIncline();
		MoveInDirection(targetVec, headTilt, CurrentGeneralPose());	// Point gun at the target.
		double angTol = DEG2RAD*(5 - Rank());
		if(fabs(TurnZ()) <= angTol && fabs(CurrentHeadTilt() - headTilt) <= angTol)
		{
			//qDebug() << "DaSentient::OperateTurret" << Id() << Name() << "close enough to shoot" << TurnZ()*RAD2DEG;
			// The turret angle is within my tolerance for closeness.
			// If we have a target, check sight path before firing.
			AuMatrix4x4 tModel = EnvTransform().Inverse();
			unsigned int sightedPart = 0;
			bool sightedShield = false;
			AuPoint sightedLoc(tModel.Project(targetSysLoc));
			AuPoint emitDir = WeaponMuzzleVec();
			AuPoint muzzleLoc = emitLoc + emitDir*3;	// move farther out for sighting target
			DaAtom* sightedAtom = system->SightTarget(this, muzzleLoc, tModel, 0, sightedLoc, sightedPart, sightedShield, 0);
			if(sightedAtom == targetAtom)
			{
				//qDebug() << "DaSentient::OperateTurret" << Id() << Name() << "target" << targetAtom->Id() << targetAtom->Name()
					//<< "not blocked, firing";
				DoShipFireTurret(ship, targetAtom->Id());
			}
		}
		else
		{
			// Do random turning and looking behavior.
			StayIdle();
		}
	}
	else
	{
		// Do random turning and looking behavior.
		StayIdle();
	}
}

bool DaSentient::IsReadyToShootAimedWeapon() const
{
	switch(Order())
	{
	case ORDER_Stay:
	case ORDER_Sleep:
	case ORDER_EnterSurgery:
	case ORDER_FollowMe:
	case ORDER_FollowTarget:
	case ORDER_GoToRoom:
	case ORDER_GoToTarget:
	case ORDER_GoToTargetExactly:
	case ORDER_ComeHere:
	case ORDER_ComeHereExactly:
	case ORDER_GetIn:
	case ORDER_GetOut:
	case ORDER_Garrison:
	case ORDER_CloseDoor:
	case ORDER_OpenDoor:
	case ORDER_GetItem:
	case ORDER_SleepDayDream:
	case ORDER_SleepEat:
	case ORDER_SleepExercise:
	case ORDER_SleepGame:
	case ORDER_SleepLetter:
	case ORDER_SleepMovie:
	case ORDER_SleepMusic:
	case ORDER_SleepPine:
	case ORDER_SleepPhone:
	case ORDER_SleepRead:
	case ORDER_SleepTalk:
	case ORDER_SleepText:
	case ORDER_SleepTV:
	case ORDER_SleepWeb:
	case ORDER_DamageReport:
	case ORDER_SurveyScanPlanet:
	case ORDER_SurveyScanSystem:
	case ORDER_TacticalScan:
	case ORDER_TakeCommand:
	case ORDER_ReleaseCommand:
	case ORDER_DropRider:
	case ORDER_MountMe:
	case ORDER_MountTarget:
	case ORDER_HengeCeremony:
		// These are not attacking poses.
		break;
	case ORDER_AttackTarget:
		// I am physically attacking, with my body or my firearm.
		// The ORDER_BattleStation think rate allows time to aim, which is better than just using the weapon's rate of fire.
		// If I have attacked within a short time, longer than the rate of fire of any of the guns, I can keep attacking repeatedly.
		// This gives the effect of taking time to aim but then opening up on the target.
		if(IsInAnyScene())
		{
			int elapsed = AIAttackTime().elapsed();
			if(elapsed < 2000+Rank()*500 || elapsed > ThinkRateForPose(ORDER_BattleStation))
			{
				// A time period long enough to aim has passed since I last attacked.
				// All weapons have a non-zero rate of fire.
				AuDNABase::WEAPONLOC wLoc;
				AuCombatWeapon weapon(WM_Not);	// CID_Invalid if wpn is part of my body
				int ammoCur, ammoFull;
				BestWeaponAtRange(0, wLoc, weapon, ammoCur, ammoFull);

				if(weapon.m_CID < CID_Invalid)
				{
					// I can attack with my body once per positive bite cycle.
					//if(Is(AuId(241530682)))
						//qDebug() << "DaSentient::IsReadyToShootAimedWeapon" << Id() << Name()
							//<< "attackTime" << elapsed << "cycleSpeed" << CycleSpeed();
					if(elapsed > (int)(CycleSpeed()*1000))
						return true;
				}
				else
				{
					// NOTE: These rate-of-fire values were copied from DaPerson.
					switch(weapon.m_CID)
					{
					case CID_AK47:
					case CID_Rifle:
						if(elapsed > 180)
							return true;
						break;
					case CID_LaserRifle:
						if(elapsed > 750)
							return true;
						break;
					case CID_LaserPistol:
						if(elapsed > 650)
							return true;
						break;
					case CID_Pistol:
						if(elapsed > 400)
							return true;
						break;
					case CID_Shotgun:
						if(elapsed > 600)
							return true;
						break;
					default:
						// How about 1 hit per second with everything else.
						if(elapsed > 1000)
							return true;
						break;
					}
				}
			}
		}
		break;
	case ORDER_BattleStation:
		if(IsInAnyScene() && IsInSpacecraft() && IsTurretPose())
		{
			// I am manning a turret on a spacecraft, which is an aimed weapon.
			// Guard tower turrets are not aimed; the guard tower gun is omnidirectional.
			// I only fire my turret when I am in a scene and my turret is not turning.
			if(TurnZ() != 0)
				break;											// Do not fire turret while it is turning.

			// My rate of fire is based on my rank and smarts.
			// Turret ordnance has a maximum rate-of-fire limit of once per 333ms, per AuOrdnance::CanFire.
			int elapsed = AIAttackTime().elapsed();
			if(elapsed < 333)
				break;											// Restricted to maximum rate-of-fire of turret.

			// If I have attacked within a short time, longer than the rate of fire of any of the turrets, I can keep attacking
			// repeatedly. This gives the effect of taking time to aim but then opening up on the target.
			if(elapsed < 2000+Rank()*500 || elapsed > ThinkRateForPose(ORDER_BattleStation))
				return true;
		}
		break;
	}
	return false;
}

// 'destLoc' is relative to my current globe world. IsNull if the destination is not in my current globe world.
// 'moveVec' is the movement vector relative to me to get to the destination loc.
// 'distToMove' is the distance (usually 2D) to move to the target loc from my current loc.
// 'headTilt' is my head tilt to the target loc.
// 'now' is the current date/time
void DaSentient::MoveToGlobeLoc(DaWorldLL* world, const AuPoint& destLoc, const AuPoint& moveVec, double distToMove, float headTilt)
{
	if(Is(s_PathTrace))
		qDebug() << "DaSentient::MoveToGlobeLoc" << Id() << Name() << "loc" << destLoc << "vec" << moveVec << "dist" << distToMove
			<< "headTilt" << (headTilt*RAD2DEGf) << "pathable" << InPathableSpace() << "industry" << InIndustry();

	//DHH Do not attempt to move into a building.
	/*
	if(IsInsideBuilding() || (InPathableSpace() && !IsInSpacecraft()))
	{
		// I am in a building or in the pathable space of a building. Pathable space of a building includes roof areas and the
		// TerrainLevelRadius at ground level. (It's actually slightly larger than the TerrainLevelRadius to insure a margin.)
		DpBuilding* building = world->BuildingAt(InIndustryLL());
		if(building)
			MoveToBuildingLoc(world, building, destLoc);
	}
	else
	*/
	{
		// Check for building obstructions.
		// Test path for obstructions.
		AuPoint moveTo = world->NextPathLocation(Location(), destLoc);
		if(!moveTo.IsZero() && moveTo != destLoc)
		{
			// An alternate path was found.
			m_SentientLock.lockForWrite();
			m_AIStrategyTime.restart();	// A good path was found. Let the dude walk it without getting frustrated and bolting.
			if(m_MoveDistLimit == 0 || m_MoveDistLimit > 5)
				m_MoveDistLimit = 5;	// Move slowly in and around buildings.
			m_SentientLock.unlock();

			AuPoint nuMoveVec = Transform().UnProject(moveTo);	// Make the nu location a vector relative to me.
			if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
				MoveInDirection(nuMoveVec, headTilt, AuPose::Swim);
			else if(CanFly() && distToMove > RUN_FAR_ENOUGH)
				MoveInDirection(nuMoveVec, headTilt, AuPose::Fly);
			else if(WaterDepth() >= CGHeightStanding())
				MoveInDirection(nuMoveVec, headTilt, AuPose::Swim);
			else
				MoveInDirection(nuMoveVec, headTilt, AuPose::StandWalk);
			return;
		}

		// There are no buildings to obstruct me in my current terrain tile or no path was found.
		// Move directly toward the destination location.
		if(distToMove > RUN_FAR_ENOUGH)
		{
			if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
				MoveInDirection(moveVec, headTilt, AuPose::SwimFast);
			else if(CanFly())
			{
				if(distToMove > RUN_FAR_ENOUGH*RUN_FAR_ENOUGH)
					MoveInDirection(moveVec, headTilt, AuPose::FlyFast);
				else
					MoveInDirection(moveVec, headTilt, AuPose::Fly);
			}
			else if(WaterDepth() >= CGHeightStanding())
				MoveInDirection(moveVec, headTilt, AuPose::SwimFast);
			else
				MoveInDirection(moveVec, headTilt, AuPose::StandRun);
		}
		else
		{
			if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
				MoveInDirection(moveVec, headTilt, AuPose::Swim);
			else
				MoveInDirection(moveVec, headTilt, AuPose::StandWalk);
		}
	}
}

void DaSentient::PilotAttackUnit(DaVessel* ship, const DaAtom* unit)
{
	// Init clearance based on my posture.
	double clearance;
	if(Posture() == POSTURE_Berserk)
		clearance = 89*FeetPerMeter() + ship->RoughRadius();
	else
		clearance = 160*FeetPerMeter() + ship->RoughRadius();

	// Add rough radius of target including shields, to avoid a collision.
	clearance += unit->PilotAvoidRadius();

	// Move near the unit.
	PilotToUnit(ship, unit, clearance, false);
}

void DaSentient::PilotFollowUnit(DaVessel* ship, const DaAtom* unit)
{
	// Init clearance based on my posture.
	double clearance;
	if(Posture() == POSTURE_Berserk)
		clearance = 100*FeetPerMeter() + ship->RoughRadius();
	else
		clearance = 200*FeetPerMeter() + ship->RoughRadius();

	// Add rough radius of target including shields, to avoid a collision.
	clearance += unit->PilotAvoidRadius();

	// Move near the unit.
	PilotToUnit(ship, unit, -clearance, true);				// Negative clearance for positioning behind the unit.
}

void DaSentient::PilotPlunderUnit(DaVessel* ship, const DaUnit* unit)
{
	// Init clearance based on my posture to be close enough for cargo transfer, which is 100m between ships.
	double clearance;
	if(Posture() == POSTURE_Berserk)
		clearance = 60*FeetPerMeter() + ship->RoughRadius();
	else
		clearance = 120*FeetPerMeter() + ship->RoughRadius();

	// Add rough radius of target including shields, to avoid a collision.
	clearance += unit->PilotAvoidRadius();

	// Move near the unit.
	PilotToUnit(ship, unit, clearance, false);
}

// 'clearance' is negative for positioning behind rather than in front of the unit
// When 'okToLand' is true, land near the target unit if the unit is on the ground.
// When 'okToLand' is false, do not land.
void DaSentient::PilotToUnit(DaVessel* ship, const DaAtom* unit, double clearance, bool okToLand)
{
	const DaWorld* planet = unit->FindParentPlanetInEnv();
	if(planet && planet == ship->FindParentPlanetInEnv())
	{
		// Unit and ship are near the same planet.
		if(unit->IsInAtmosphere())
		{
			if(okToLand)
			{
				// When ok to land, we will land once the target unit is on the ground. Otherwise, we'll hover near the target.
				if(unit->IsOnGround())
				{
					// Maneuver in front of unit on the ground, behind when clearance is negative.
					AuPoint posVec(0,clearance,0);			// Vector relative to target unit, initially out in front.
					float zrot = 60 - Id().GetID()%121;			// -60 to 60 degrees based on my id so I'm consistent
					posVec = posVec.RotateZ(zrot*DEG2RADf);

					AuMatrix4x4 unitT;
					unit->GetTransformRelativeToParent(unitT, planet);
					AuPoint unitLoc = unitT.Project(posVec);
					PilotToLandingSite(ship, planet, unitLoc);	// Land behind the unit.
				}
				else
				{
					// Maneuver above the unit in the atmosphere.
					AuPoint posVec(0,0,fabs(clearance));	// Vector relative to target unit, initially above.

					AuMatrix4x4 unitT;
					unit->GetTransformRelativeToParent(unitT, planet);
					AuPoint unitLoc = unitT.Project(posVec);
					PilotToHoverSite(ship, planet, unitLoc);	// Hover above the unit.
				}
			}
			else
			{
				// Maneuver above the unit in the atmosphere.
				AuPoint posVec(0,0,fabs(clearance));		// Vector relative to target unit, initially above.

				AuMatrix4x4 shipT;
				ship->GetTransformRelativeToParent(shipT, planet);
				AuMatrix4x4 unitT;
				unit->GetTransformRelativeToParent(unitT, planet);
				AuPoint shipLoc = shipT.Translation();			// Ship location relative to the solar system.
				AuPoint unitSysLoc = unitT.Translation();		// Unit location relative to the solar system.
				if(shipLoc.Distance2(unitSysLoc) > clearance*clearance)
				{
					// Ship is too far from the target unit; move closer.
					AuPoint unitLoc = unitT.Project(posVec);	// Location near unit relative to the solar system.
					PilotToHoverSite(ship, planet, unitLoc);	// Hover over the unit.
				}
				else
					PilotHoldPosition(ship);				// Close enough to the unit.
			}
		}
		else
		{
			// Unit is not in atmosphere range of the planet.
			if(ship->IsInAtmosphere())
				PilotVerticalTakeOff(ship);				// The ship is in atmosphere range of a planet. Take-off.
			else
			{
				// Maneuver in near unit in orbit.
				AuMatrix4x4 shipT;
				ship->GetTransformRelativeToParent(shipT, planet);
				AuPoint shipLoc = shipT.Translation();			// Ship location relative to planet.

				AuMatrix4x4 unitT;
				unit->GetTransformRelativeToParent(unitT, planet);
				AuPoint unitLoc = unitT.Translation();			// Unit location relative to the planet.

				AuPoint orbitLoc;
				AuPoint vV(unit->Vector());
				if(!vV.IsFuzzyZero())
				{
					// Approaching a moving target, either a unit or asteroid. Calculate a safe approach that won't crash into it.
					vV = unitT.ProjectVector(vV);				// Relative to the planet.
					vV.Unit();
					AuPoint vT(shipLoc - unitLoc);				// Vector to ship, relative to the planet. Not likely 0,0.
					vT.Unit();
					AuPoint vN = vT.Cross(vV);					// Perpendicular to the target vector and the ship vec.
					if(vN.IsFuzzyZero())
						vV.ArbitraryAxis(vN);					// Heading along (away or toward) the target's path.
					else
						vN.Unit();
					AuPoint vC = vV.Cross(vN);
					vC.Unit(fabs(clearance));
					orbitLoc = unitLoc + vC;					// Location relative to planet.
					//qDebug() << "DaSentient::PilotToUnit" << *this << "unit" << *unit
						//<< "planetDist" << (shipLoc.Distance(unitLoc)) << "clearance" << clearance;
				}
				else
				{
					// Maneuver in front of the unit in orbit, behind when clearance is negative.
					AuPoint posVec(0,clearance,0);
					orbitLoc = unitT.Project(posVec);
				}

				// Calculate appropriate speed limit.
				float speedLimit;
				const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
				if(rsworld)
					speedLimit = SpeedOfLight();				// No speed limit catching up to the target.
				else
				{
					// Limit speed if we clip close to globe.
					if(AuPoint(0,0).DistToSegment(shipLoc, orbitLoc) < planet->AtmosphereRadius() + OrbitAltAboveAtm())
					{
						const DaWorldSphere* glworld = dynamic_cast<const DaWorldSphere*>(planet);
						if(glworld)
							speedLimit = sqrt((float)glworld->Radius100km()/10) * 200*FeetPerMeter();
						else
							speedLimit = SpeedOfLight();
					}
					else
						speedLimit = SpeedOfLight();			// No speed limit catching up to the target.
				}

				if(orbitLoc.Length2() < clearance*clearance && ship->Speed() < ShieldCollisionSpeed())
				{
					// We are close enough to the target; point the ship at the target if the engines are still on.
					if(ship->GravityDriveState() == DaVessel::GD_On && ship->GravityDriveWorking())
					{
						// Gravity drive is full-on (not hold) and it is working.
						// Calculate pitch and yaw needed to point the ship at the target.
						AuPoint targLoc = shipT.UnProject(unitLoc);		// Unit loc in the space of my ship.
						double pitch = targLoc.PolarIncline();
						double yaw = targLoc.ArcTangent_2D() - PI/2;
						if(yaw < -PI)
							yaw += PI*2;

						// More than 5 degrees of either pitch or yaw is enough to turn the ship.
						if(fabs(pitch) > 5*DEG2RAD || fabs(yaw) > 5*DEG2RAD)
						{
							// Pitch or yaw is needed to align with the target.
							DoShipRollPitchYaw(ship, 0, pitch, yaw);

							// Stop at current location.
							if(!ship->IsPowerPlantOn())
								DoShipPowerPlantState(ship, true);
							else
							{
								DoShipKillThrottles(ship);
								if(ship->InBlackHoleGravity())
									DoShipBrakeState(ship, true, true);
								else
									DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
							}
						}
						else
							PilotHoldPosition(ship);			// Close enough to the unit and pointing toward it.
					}
					else
						PilotHoldPosition(ship);			// Close enough to the unit and pointing toward it.
				}
				else
				{
					// Orbit the planet to reach the target's location.
					PilotToPlanetLocation(ship, shipT, shipLoc, planet, orbitLoc, speedLimit, 5*FeetPerMeter(), 0, false);
				}
			}
		}
	}
	else
	{
		// Unit is not near a planet.
		if(ship->IsInAtmosphere())
			PilotVerticalTakeOff(ship);					// The ship is in atmosphere range of a planet. Take-off.
		else
		{
			const AuMatrix4x4& shipT = ship->EnvTransform();
			const AuMatrix4x4& unitT = unit->EnvTransform();
			AuPoint shipLoc = shipT.Translation();
			AuPoint unitSysLoc = unitT.Translation();			// Unit location relative to the solar system.
			if(shipLoc.Distance2(unitSysLoc) > clearance*clearance || ship->Speed() > ShieldCollisionSpeed())
			{
				// Ship is too far from the target unit or moving too fast.
				AuPoint sysLoc;
				AuPoint vV(unit->Vector());
				if(!vV.IsFuzzyZero())
				{
					// Approaching a moving target, either a unit or asteroid. Calculate a safe approach that won't crash into it.
					vV = unitT.ProjectVector(vV);				// Relative to the solar system.
					vV.Unit();
					AuPoint vT(shipLoc - unitSysLoc);			// Vector to ship, relative to the system. Not likely 0,0.
					vT.Unit();
					AuPoint vN = vT.Cross(vV);					// Perpendicular to the target vector and the ship vec.
					if(vN.IsFuzzyZero())
						vV.ArbitraryAxis(vN);					// Heading along (away or toward) the target's path.
					else
						vN.Unit();
					AuPoint vC = vV.Cross(vN);
					vC.Unit(fabs(clearance));
					sysLoc = unitSysLoc + vC;					// Location relative to solar system.
					//qDebug() << "DaSentient::PilotToUnit" << *this << "unit" << *unit
						//<< "sysDist" << (shipLoc.Distance(unitSysLoc)) << "clearance" << clearance;
				}
				else
				{
					// Maneuver in front of the unit in the solar system, behind when clearance is negative.
					AuPoint posVec(0,clearance,0);
					sysLoc = unitT.Project(posVec);
				}
				PilotToSystemLoc(ship, shipT, ship->FindEnvironmentAtom(), shipT.UnProject(sysLoc), AuWormhole::Invalid,false,"",0);
			}
			else
			{
				// We are close enough to the target and slow enough to enter shields; point the ship at the target if the engines
				// are still on.
				if(ship->GravityDriveState() == DaVessel::GD_On && ship->GravityDriveWorking())
				{
					// Gravity drive is full-on (not hold) and it is working.
					AuPoint targLoc = shipT.UnProject(unitSysLoc);	// Target loc in the space of my ship.

					// Calculate pitch and yaw needed to point the ship at the target.
					double pitch = targLoc.PolarIncline();
					double yaw = targLoc.ArcTangent_2D() - PI/2;
					if(yaw < -PI)
						yaw += PI*2;

					// More than 5 degrees of either pitch or yaw is enough to turn the ship.
					if(fabs(pitch) > 5*DEG2RAD || fabs(yaw) > 5*DEG2RAD)
					{
						// Pitch or yaw is needed to align with the target.
						DoShipRollPitchYaw(ship, 0, pitch, yaw);

						// Stop at current location.
						if(!ship->IsPowerPlantOn())
							DoShipPowerPlantState(ship, true);
						else
						{
							DoShipKillThrottles(ship);
							if(ship->InBlackHoleGravity())
								DoShipBrakeState(ship, true, true);
							else
								DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
						}
					}
					else
						PilotHoldPosition(ship);				// Close enough to the unit and pointing toward it.
				}
				else
					PilotHoldPosition(ship);				// Close enough to the unit and pointing toward it.
			}
		}
	}
}

// Doesn't mess with the throttle unnecessarily in case a human has set the throttle on the helm.
// In fact, assumes that if a throttle is on the ship is trying to go somewhere. Sets gravity drive accordingly.
// If you are trying to stop, turn off the throttle before calling this function.
// NOTE: Assumes we would not be here if a human was operating a helm console.
void DaSentient::PilotHoldPosition(DaVessel* ship)
{
	//if(ship->Id() == AuId(108424718))
		//qDebug() << "DaSentient::PilotHoldPosition" << ship->Id() << ship->Name();
	ResetOnApproach();										// Reset for atmospheric approach.

	// When holding our position and the ship is on a mission, we kill the throttle.
	// Without a mission, a person might have set the throttle and left the console.
	if(ship->IsOnAMission() || ship->IsCommandedByAI())
		DoShipKillThrottles(ship);

	// We are stopping so disengage the warp drive.
	if(ship->HasWarpFTLDrive())
	{
		quint16 whichFTL = ship->FTLWarpDrive();
		if(ship->HasHumanOfficer())
			DoShipDisengageFTLDrive(ship, 2, whichFTL, QString());
		else
			DoShipDisengageFTLDrive(ship, 0, whichFTL, QString());
	}

	if(ship->IsInAtmosphere())
	{
		// The ship is in atmosphere range of a planet or in the gravity influence of a black hole.
		if(ship->IsOnGround())
		{
			// Ship is on the ground. It will right itself due to ground contact.
			// Insure the wheels are down. Does nothing if they are already down or if the ship does not have retractable gear.
			DoShipLandingGear(ship, true);
			if(!ship->IsMoving())
			{
				// Very low speed so kill the engine. The ship will drop to the ground and grind to a halt.
				if(ship->GravityDriveState() != DaVessel::GD_Off)
				{
					DoShipGravityDriveState(ship, DaVessel::GD_Off);
					DoShipKillThrottles(ship);
				}
				else if(ship->IsAtRest() &&
						(ship->MissionOrderType() == AuMissionOrder::Repair ||
						 ship->MissionOrderType() == AuMissionOrder::Hide))
					DoShipPowerPlantState(ship, false);	// Ship order is to hide in ground clutter.
			}
			else
			{
				// Ship is moving more than just a little bit while touching the ground.
				// Since I know no human is manning the helm, I think the ship should stop.
				if(ship->GravityDriveState() != DaVessel::GD_On)
				{
					DoShipGravityDriveState(ship, DaVessel::GD_On);
					DoShipKillThrottles(ship);
				}
				else
				{
					if(ship->InBlackHoleGravity())
						DoShipBrakeState(ship, true, true);
					else
						DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
				}
			}
		}
		else
		{
			// Ship is not on the ground.
			// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
			DoShipLandingGear(ship, false);
			if(!ship->IsPowerPlantOn())
			{
				// Turn on the power plant if it's not already on.
				DoShipPowerPlantState(ship, true);
			}
			else if(ship->GravityDriveState() != DaVessel::GD_Hold)
			{
				// Position the ship to hold.
				if(ship->GravityDriveState() != DaVessel::GD_On)
				{
					DoShipGravityDriveState(ship, DaVessel::GD_On);
					DoShipKillThrottles(ship);
				}
				else
				{
					// No throttles are trying to move the ship or the ship is AI commanded. Gravity is going to make us fall.
					// Perform maneuvers relative to the surface of the planet.
					double speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude;
					ship->FlightInstruments(ship->DesignCenterOffset(), 0, speed, speedF, speedL, speedV, altitude,
							pitch, roll, heading, longitude, latitude);

					// Roll according to flight attitude.
					FLIGHT_ATTITUDE fatt = FlightAttitude(ship);	// Returns upright in atmosphere.
					if(fatt == FLY_Inverted)
					{
						roll += PI;
						pitch = -pitch;
					}
					else if(fatt == FLY_LeftSide)
						roll -= PI/2;
					else if(fatt == FLY_RightSide)
						roll += PI/2;
					if(roll < -PI)
						roll += PI*2;
					else if(roll > PI)
						roll -= PI*2;

					if(fabs(roll) < DEG2RAD)
						roll = 0;

					if(fabs(pitch) < DEG2RAD)
						pitch = 0;

					DoShipRollPitchYaw(ship, roll, pitch, 0);

					// Use brakes to hold hover position until we stop.
					DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());

					// Set hold state when ship is level.
					if(fabs(pitch) <= DEG2RAD && fabs(roll) <= DEG2RAD)
					{
						DoShipGravityDriveState(ship, DaVessel::GD_Hold);
						DoShipKillThrottles(ship);
					}
				}
			}
		}
	}
	else
	{
		// The ship is in space.
		// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
		DoShipLandingGear(ship, false);
		if(!ship->IsPowerPlantOn())
		{
			// Turn on the power plant if it's not already on.
			DoShipPowerPlantState(ship, true);
		}
		else if(!ship->IsMoving())
		{
			// Ship is not moving or it is moving very slowly.
			DoShipGravityDriveState(ship, DaVessel::GD_Hold);
			DoShipKillThrottles(ship);
		}
		else
		{
			// Ship is moving.
			if(ship->GravityDriveState() != DaVessel::GD_On)
			{
				DoShipGravityDriveState(ship, DaVessel::GD_On);
				DoShipKillThrottles(ship);
			}
			else
			{
				// No throttle controls are trying to move the ship or the ship is AI commanded.
				// Do not pitch and roll in deep space to hold position. This helps to stay on course during server handoffs
				// when deadheading.
				DoShipRollPitchYaw(ship, 0, 0, 0);

				// Use brakes to stop moving and hold my current position.
				if(ship->InBlackHoleGravity())
					DoShipBrakeState(ship, true, true);
				else
					DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
			}
		}
	}
}

void DaSentient::PilotHoldPositionClearWormhole(DaVessel* ship)
{
	ResetOnApproach();										// Reset for atmospheric approach.

	if(ship->IsInAtmosphere())
	{
		// Don't do this work when the ship is in an atmosphere.
		PilotHoldPosition(ship);
		return;
	}

	// Don't loiter too close to a wormhole.
	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
	if(!system)
	{
		// Not likely. Are we in wormhole space? Arena?
		PilotHoldPosition(ship);
		return;
	}

	const AuMatrix4x4& shipT = ship->EnvTransform();
	AuPoint shipLoc = shipT.Translation();
	AuWormhole wh = system->FindClosestWormhole(shipLoc);
	if(!wh.IsValid())
	{
		// Not likely since all solar systems have at least one wormhole.
		PilotHoldPosition(ship);
		return;
	}

	AuPoint whSysLoc = wh.Location(system);
	AuPoint whvec = shipLoc - whSysLoc;							// Vector from wormhole to ship.
	double whdist2 = whvec.Length2();							// squared distance to the closest wormhole
	if(whdist2 > 2000*FeetPerMeter()*2000*FeetPerMeter())
	{
		// Ship is beyond 2000m of the nearest wormhole.
		PilotHoldPosition(ship);
		return;
	}

	// Move away from the wormhole.
	AuPoint moveLoc;
	if(whdist2 < 200*FeetPerMeter()*200*FeetPerMeter())
	{
		// Ship is close to the wormhole.
		AuPoint whoutvec(whSysLoc);
		whoutvec.Unit(3000*FeetPerMeter());			// Vector out of wormhole, well outside 2000m.
		float zrot = 30 - (Id().GetID()%61);		// -30 to 30 degrees based on my id so I'm consistent
		whoutvec = whoutvec.RotateZ(zrot*DEG2RADf);
		float xrot = 20 - (Id().GetID()%41);		// -20 to 20 degrees based on my id so I'm consistent
		whoutvec = whoutvec.RotateX(xrot*DEG2RADf);
		moveLoc = whSysLoc + whoutvec;
	}
	else
	{
		// Ship is near the wormhole.
		whvec.Unit(3000*FeetPerMeter());				// Vector away from wormhole, well outside 2000m.
		moveLoc = whSysLoc + whvec;
	}
	moveLoc = shipT.UnProject(moveLoc);		// Convert location to be relative to the ship.
	PilotToSystemLoc(ship, shipT, system, moveLoc, AuWormhole::Invalid, false, "", 0);
}

void DaSentient::PilotToWaypoint(DaVessel* ship)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	ResetOnApproach();										// Reset for atmospheric approach.

	if(ship->IsInAtmosphere())
	{
		PilotVerticalTakeOff(ship);	// The ship is in atmosphere range of a planet. Take-off.
		return;
	}

	// Perform maneuvers relative to the plane of the ecliptic of the solar system.
	// It would be easier to fly above/below the plane of the ecliptic but that's not how players tend to fly.
	// Calculate a vector directly to the next waypoint.
	DaAtom* system = ship->FindEnvironmentAtom();
	Q_ASSERT(system);
	const AuMatrix4x4& tship = ship->EnvTransform();

	const DeWaypoint* wp = ship->LockWaypoint(0);
	if(!wp)
	{
		ship->UnlockWaypoint();
		return;
	}

	QString destName;
	double destRad = 0;
	AuPoint destLoc;
	const DaAtom* destAtom = wp->Location(system, destLoc);	// Returns 0 if the waypoint destination is in another solar system.
	AuId wpSystem(wp->m_idSystem);
	AuId wpDest(wp->Destination());
	AuPoint wpDestOff(wp->DestinationOffset());
	ship->UnlockWaypoint();

	AuWormhole::POLARITY wormholePolarity = AuWormhole::Invalid;
	bool doFAF = false;	// true if we are going to a final approach fix before entry into a wormhole
	if(destAtom)
	{
		// The atom at the waypoint is in this solar system.
		// Calculate destination location relative to the ship. 'destLoc' is currently relative to the solar sytem.
		destLoc = tship.UnProject(destLoc);

		DaSolarSystem* solar = dynamic_cast<DaSolarSystem*>(system);
		AuWormhole wh;
		if(!ship->MissionOrderIsPatrol() && solar && solar->Is(wpSystem) && solar->Is(wpDest))
			wh = solar->FindWormhole(wpDestOff);

		if(wh.IsValid())
		{
			// Destination is a wormhole and we're not patrolling so we must be going through it.
			if(!IsOkToLeaveSolarSystem(ship))
			{
				// A ship with no officers online must not leave the solar system.
				PilotHoldPosition(ship);
				return;
			}

			if(ship->IsReadyToWormhole())
			{
				// Wormhole drive is ready so it's dangerous to go through the wormhole the wrong way.
				if(wh.DrivePolarity() != ship->FTLDriveSetting(ship->FTLWormholeDrive())+1)
				{
					// Drive polarity does not match wormhole polarity.
					// Wait for the wormhole drive to discharge before approaching the wormhole.
					PilotHoldPosition(ship);
					return;
				}
			}

			// Waypoint wormhole is valid.
			destName = wh.Name();
			wormholePolarity = wh.Polarity();

			DaStar* primary;
			if(wh.IsIntergalactic())
				primary = solar->FindPrimaryStar();
			else
				primary = solar->FindPrimaryStarNotBlackHole();
			AuPoint whSysLoc = wh.Location(solar);
			if(primary)
			{
				// Due to a bug, check to see if the wormhole is inside the radius of the sun.
				// Squared distance to wormhole from center of sun.
				double rangeFromSun2 = whSysLoc.Distance2(primary->EnvLocation());
				double sunRad2 = primary->Radius();
				sunRad2 *= sunRad2;								// Squared radius of primary sun.
				if(rangeFromSun2 <= sunRad2)
				{
					// We have detected a problem with the position of the wormhole.
					// Clear the current route. Don't fly into sun.
					ship->RemoveAllWaypoints();

					// Report this in the log.
					qWarning() << "ERROR! Pilot" << Id() << Name() << "noticed"
						<< "wormhole polarity" << wh.Polarity()
						<< "position wrong in system" << solar->Name()
						<< "star radius" << primary->Radius()
						<< "wormhole distance from star center" << sqrt(rangeFromSun2)
						<< "Route cleared.";
					return;
				}
			}

			// Calculate wormhole entry waypoint. If I don't set doFAF true, then I will pilot directly to the destLoc.
			AuPoint tovec(destLoc);							// Vec to wormhole entry point from ship.
			double destDist = tovec.Length();
			if(destDist < FeetPerOrbit())
			{
				// The ship is close enough to the wormhole to maneuver for proper approach to the wormhole.
				if(destDist != 0)
					tovec /= destDist;

				// If the ship is not positioned and ready to enter the wormhole, aim for a turn point clear of the wormhole.
				// FAC is Final Approach Course in these comments.
				AuPoint facVec(primary->Location() - whSysLoc);					// FAC vector is relative to the primary.
				facVec = tship.UnProjectVector(facVec);							// Project FAC vector into space of ship.
				facVec.Unit();
				double facDot = tovec.Dot(facVec);
				//qDebug("%d %s wormhole entry dot %g angle is %g", Id().GetID(), (const char*)Name().toAscii(),
						//facDot, acos(facDot)*RAD2DEG);
				if(facDot < .8660)
				{
					// Ship is not positioned within a 30 degree cone of entry.
					AuPoint vec0 = tovec.Cross(facVec);			// Perpendicular vector to both my path and the FAC.
					vec0.Unit();
					AuPoint perpVec = vec0.Cross(facVec);		// Perpendicular to FAC.
					perpVec.Unit();
					if(facDot < 0)
					{
						// Ship is on the wrong side of the plane of the wormhole.
						// We might need to aim for a turn point that will clear the disk of the wormhole entry plane.
						double facDist = fabs(destLoc.Dot(perpVec));	// Lateral distance from FAC.
						if(wh.Polarity() == AuWormhole::Positive)
						{
							if(facDist < 3000)
							{
								// Ship is too close to the FAC.
								destLoc += perpVec * 3000;		// Aim for a point beyond the radius of the wormhole.
								doFAF = true;					// Go slow but don't stop at the turn.
							}
						}
						else if(wh.Polarity() == AuWormhole::Negative)
						{
							if(facDist < 1500)
							{
								// Ship is too close to the FAC.
								destLoc += perpVec * 1500;		// Aim for a point beyond the radius of the wormhole.
								doFAF = true;					// Go slow but don't stop at the turn.
							}
						}
						else
						{
							if(facDist < 6000)
							{
								// Ship is too close to the FAC.
								destLoc += perpVec * 6000;		// Aim for a point beyond the radius of the wormhole.
								doFAF = true;					// Go slow but don't stop at the turn.
							}
						}
					}

					if(!doFAF)
					{
						// Aim for a final approach fix some distance out from the wormhole entrance.
						// The location we calculate puts the entrance location 1/2 way between the positive and negative
						// wormholes if wh happens to be positive.
						//qDebug("%d %s aiming for FAF", Id().GetID(), (const char*)Name().toAscii());
						// Final approach fix is 1/2 way between + and - wormholes.
						AuPoint fafLoc = primary->Location() + (whSysLoc - primary->Location()) * 1.05;
						destLoc = tship.UnProject(fafLoc);		// Project location into space of ship.
						if(wh.Polarity() == AuWormhole::Positive)
							destLoc += perpVec * 3000;
						else if(wh.Polarity() == AuWormhole::Negative)
							destLoc += perpVec * 1500;
						else
							destLoc += perpVec * 6000;
						doFAF = true;								// Go slow but don't stop at the turn.
					}
				}
			}
			else
				doFAF = true;	// So we know we are heading for a wormhole. Affects target speed limit at destLoc.
		}
		else
		{
			// Destination waypoint is not at a wormhole.
			const DaWorld* planet = dynamic_cast<const DaWorld*>(destAtom);
			if(planet)
			{
				// Destination atom is a planet. Don't fly to the center of the planet.
				destName = planet->Name();
				// Go for a point that is off to the side of the planet at the standard orbit altitude for spacecraft so we won't
				// plunge into the atmosphere when going too fast.
				if(destLoc.IsZero() || ship->IsAnyFTLDriveHot())
				{
					if(dynamic_cast<const DaWorldSphere*>(planet))
					{
						//qDebug() << "approaching planet FTL drive hot" << ship->IsAnyFTLDriveHot() << "destLoc" << destLoc
							//<< "radius" << planet->AtmosphereRadius();
						// Planet is a globe and no destination point was specified.
						// Destination is center of planet or the FTL drive is hot; aim for a point that will just miss the planet.
						// Calculate perpendicular vector to the vector pointing toward the planet.
						AuPoint perpVec(-destLoc.x, -destLoc.z);

						// Using perpendicular vector, calculate a point in orbit to head toward that misses the planet.
						if(ship->IsAnyFTLDriveHot())
							perpVec.Unit(planet->AtmosphereRadius() * 2);
						else
							perpVec.Unit(planet->AtmosphereRadius() + OrbitAltAboveAtm());
						destLoc += perpVec;
					}
				}
			}
			else
			{
				const DaStar* star = dynamic_cast<const DaStar*>(destAtom);
				if(star)
				{
					// Destination atom is a star. Don't fly to the center of the star.
					destName = star->Name();
					// Go for a point that misses the star by a safe margin.
					if(destLoc.IsZero() || ship->IsAnyFTLDriveHot())
					{
						//qDebug() << "approaching star FTL drive hot" << ship->IsAnyFTLDriveHot() << "destLoc" << destLoc
							//<< "radius" << star->Radius();
						// Destination is center of star or the FTL drive is hot; aim for a point that will just miss the star.
						// Calculate perpendicular vector to the vector pointing toward the star.
						AuPoint perpVec(-destLoc.x, -destLoc.z);

						// Using perpendicular vector, calculate a point in orbit to head toward that misses the star.
						perpVec.Unit(star->Radius() * 2);
						destLoc += perpVec;
					}
				}
				else
				{
					const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(destAtom);
					if(system)
					{
						// Destination atom is the solar system.
						destName = system->NameKnownTo(ship->EmpireId());
						// Check to see if destination waypoint is inside the primary star radius.
						bool insideStar = false;
						const DaStar* star = system->FindPrimaryStar();
						if(star)
						{
							double starRad = star->Radius();
							double destoff2 = wpDestOff.Length2();
							if(destoff2 <= starRad*starRad)
							{
								//qDebug() << "approaching primary star FTL drive hot" << ship->IsAnyFTLDriveHot()
									//<< "destLoc" << destLoc << "radius" << star->Radius();
								// Destination location is within the primary star.
								insideStar = true;

								// Go for a point that misses the star by a safe margin.
								if(destLoc.IsZero() || ship->IsAnyFTLDriveHot())
								{
									// Destination is center of star or the FTL drive is hot; aim for a point that will just miss.
									// Calculate perpendicular vector to the vector pointing toward the star.
									AuPoint perpVec(-destLoc.x, -destLoc.z);

									// Using perpendicular vector, calculate a point in orbit to head toward that misses the star.
									perpVec.Unit(starRad * 2);
									destLoc += perpVec;
								}
								else
								{
									// Waypoint is inside the primary star. Certain special distances might indicate a bug with 
									// the position of wormholes or waypoints at unpositioned wormholes.
									double destoff = sqrt(destoff2);
									if(FuzzyEquals(destoff, 1) || FuzzyEquals(destoff, 2))
									{
										// Destination waypoint was probably placed at a wormhole that was not positioned yet. We
										// get here if that wormhole is no longer at that location, otherwise FindWormhole would
										// have located it. Report the problem and clear the waypoint route so the navigator can
										// try again.
										// Clear the current route. Don't fly into sun.
										ship->RemoveAllWaypoints();
										qWarning() << "ERROR! Pilot" << Id() << Name()
											<< "noticed next waypoint is at special wormhole distance"
											<< destoff << "from primary star. Route cleared.";
									}
									else
									{
										// Waypoint is inside sun radius but it is not at a special wormhole distance.
										// This typically happens when following a waypoint to deadhead to a system, right when we
										// cross the system boundary.
										// Aim for a point that will just miss.
										// Calculate perpendicular vector to the vector pointing toward the star.
										AuPoint perpVec(-destLoc.x, -destLoc.z);

										// Using perpendicular vector, calculate a point in orbit to head toward that misses the
										// star.
										perpVec.Unit(starRad * 2);
										destLoc += perpVec;
									}
								}
							}
						}
						if(!insideStar)
						{
							// Destination is a point relative to the solar system that is not inside the primary star.
							if(!IsOkToLeaveSolarSystem(ship))
							{
								// Ship is not ok to leave the solar system. Insure we are not going to depart the system.
								if(!system->IsInSystem(wpDestOff))
								{
									// Destination is more than a sector from my current solar system
									//  or
									// Destination departs the current solar system.
									PilotHoldPosition(ship);			// Do not leave the solar system.
								}
							}
						}
					}
					else
					{
						const DaArena* system = dynamic_cast<const DaArena*>(destAtom);
						if(system)
						{
							// Destination atom is the arena.
							double radius = wpDestOff.Length();

							if(radius > c_ArenaBoundsRadius)
								PilotHoldPosition(ship);			// Do not leave the arena.
						}
					}
				}
			}
		}
	}
	else
	{
		// Waypoint is in another solar system.
		if(!IsOkToLeaveSolarSystem(ship) || InArena() || InDesignPreview())
		{
			// A ship with no officers online must not leave the solar system.
			PilotHoldPosition(ship);
			return;
		}

		// Calculate destination solar system location relative to the ship.
		const DaSector* sector = (DaSector*)system->Parent();
		Q_ASSERT(sector);
		const DaSolarSystem* pdss = dynamic_cast<const DaSolarSystem*>(ether->FindNormalAtom(wpSystem));
		if(pdss)
		{
			// Determine waypoint location in star field.
			destName = pdss->NameKnownTo(ship->EmpireId());
			destRad = pdss->ActualRadius();

			// Multiply by inverse of subatom transformation
			// (Translation, rotation, and scale.)
			AuMatrix4x4 sysRot;
			sysRot.Rotate(system->Rotation());
			sysRot = sysRot * tship;
			AuPoint floc = sysRot.Project(AuPoint(0,0));	// vector from system in coordinates relative to the sector

			AuMatrix4x4 tstar;
			tstar.Translate(-floc);
			tstar = tstar * sysRot;
			tstar = tstar.Inverse();

			// Determine the center point, in parsecs, about which the star field is offset.
			AuPoint fSceneLoc;
			ship->GetLocationRelativeToParent(fSceneLoc, sector);

			// Find the destination system in the list.
			DqAtomLocArray aStars;
			sector->ListSolarSystemLocations(aStars);
			for(int il=0; il<aStars.count(); il++)
			{
				DqAtomLoc &al = aStars[il];
				if(al.m_pA == (const DaAtom*)pdss)
				{
					// Found the destination star.
					AuPoint fstarloc = al.m_fLocation - fSceneLoc;	// fstarloc is in parsecs (up to about 25 away)
					fstarloc *= FeetPerParsec();			// This will make a good enough vector to get us there.
					destLoc = tstar.Project(fstarloc);
					break;
				}
			}
			if(destLoc.IsZero())
			{
				// Destination solar system not found in starfield.
				DoShipKillThrottles(ship);
				if(ship->InBlackHoleGravity())
					DoShipBrakeState(ship, true, true);
				else
					DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
			}
		}
		else
		{
			// Destination solar system not found in atom data.
			DoShipKillThrottles(ship);
			if(ship->InBlackHoleGravity())
				DoShipBrakeState(ship, true, true);
			else
				DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
		}
	}

	// Travel to coordinates order is confusing when it announces the current system as the destination, except in rare cases.
	if(ship->MissionOrderType() == AuMissionOrder::GoToCoordinate)
		destName = QString();

	PilotToSystemLoc(ship, tship, system, destLoc, wormholePolarity, doFAF, destName, destRad);
}

// 'sysLoc' is relative to the ship.
void DaSentient::PilotToSystemLoc(DaVessel* ship, const AuMatrix4x4& shipT, const DaAtom* system, const AuPoint& sysLoc,
		AuWormhole::POLARITY wormholePolarity, bool doFAF, const QString& destName, double destRadius)
{
	if(ship->IsInAtmosphere())
		return;
	//if(Name() == "SN Ocxutejti")
		//qDebug() << "DaSentient::PilotToSystemLoc" << Id() << Name() << "ship" << ship->Id() << ship->Name()
			//<< "sysLoc" << sysLoc << "wormholePolarity" << wormholePolarity << "doFAF" << doFAF;

	// The ship is in space.
	ResetOnApproach();										// Reset for atmospheric approach.

	if(!ship->IsPowerPlantOn())
	{
		// Turn on the power plant if it's not already on.
		DoShipPowerPlantState(ship, true);
		return;
	}

	if(ship->GravityDriveState() != DaVessel::GD_On)
	{
		// Turn on the gravity drive if it's not already on.
		DoShipGravityDriveState(ship, DaVessel::GD_On);
		DoShipKillThrottles(ship);
		return;
	}

	// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
	DoShipLandingGear(ship, false);

	// Get current flight instrument readings.
	double speed, speedF, speedL, speedV, altitude, curPitch, roll, heading, longitude, latitude;
	int altMode = ship->FlightInstruments(ship->DesignCenterOffset(), 0, speed, speedF, speedL, speedV, altitude, curPitch, roll,
			heading, longitude, latitude);

	const double accel = ship->ManeuverAcceleration();			// ft/sec/sec adjusted for damage

	// Modify the destination location if I am going way too fast to stop by the time I get there.
	AuPoint courseLoc(sysLoc);
	double speedPlanDist = courseLoc.Length();					// Distance for planning speed changes.
	if(accel > 0 && !ship->IsAnyFTLDriveHot() && !IsFuzzyZero(speed))
	{
		double ete = speedPlanDist / speed;						// sec to arrive at destination
		double stopTime = speed / accel;						// sec to stop at destination
		if(stopTime > ete*2.)
		{
			// Going way too fast. Aim for a safe location above or below the plane of the ecliptic from the destination.
			AuPoint shipSysVec = shipT.ProjectVector(ship->Vector());	// Current vector relative to the solar system.
			AuPoint sysCourseLoc = shipT.Project(courseLoc);	// Course loc relative to the solar system.
			if(shipSysVec.z >= 0)
				sysCourseLoc.z += FeetPerSolarSystem()/2;		// Already pitching up so aim above the destination.
			else
				sysCourseLoc.z -= FeetPerSolarSystem()/2;		// Already pitching down so aim below the destination.
			courseLoc = shipT.UnProject(sysCourseLoc);
		}
	}

	// Modify the destination location based on any obstacles along the path.
	// Sensor data provides the information we need for this.
	double speedLimit = -1;				// No speed limit unless destination is close to a planet.
	QMap<DaAtom*, int> obstacles;		// Count of each obstacle encountered as we do multipasses.
	//QMap<AuWormholeArray::const_iterator, int> obstacleWorms;   // Count of each obstacle wormhole as we do multipasses.
	bool anyObstacle;					// Set true if I change courseLoc due to an obstacle, requiring re-eval of all blips.
	bool amConfused = false;			// Set true if I can't figure out how to get around an obstacle.
	double shipRad = ship->RoughRadius();
	int tries = 0;	// Don't do this forever.
	QSet<const DaWorld*> checkedRWs;	// Ringworlds trip me up. Let's hope I don't start veering into them.
	DaAtomList blips;
	system->ListNormalSubatomsRecursive(blips);
	do
	{
		anyObstacle = false;	// Will be set true if we find any obstacle in the course direction.

		// Calculate unit vector pointing toward destination. This is my path vector.
		double destDist = courseLoc.Length();	// Distance to course location.
		if(destDist == 0)
			break;
		AuPoint pathVec(courseLoc);
		pathVec /= destDist;					// Make path vector into a unit vector.

		// Check for any blips that obstruct my path.
		for(DaAtomList::const_iterator bi = blips.begin(); bi != blips.end(); bi++)
		{
			if(ship->Is((*bi)->Id()))
				continue;

			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(*bi);
			if(rsworld)
			{
				// Blip is a ringworld section. These take extra work to avoid.
				// This code treats the ring section like a complete ring so we only have to do this work on ring section 0.
				if(rsworld->SectionIndex())
					continue;
				if(checkedRWs.contains(rsworld))
					continue;
				checkedRWs.insert(rsworld);

				const AuMatrix4x4& ringT = rsworld->EnvTransform();

				// Calculate the radii of the ringworld section.
				double ringRadius = rsworld->OrbitRadius();
				const double wallThickness = 100*FeetPerMeter();
				double ringWidth = rsworld->TerrainYSize();
				double ringWidth2 = ringWidth/2;
				double radRingMax = ringRadius - rsworld->MinRadius();		// Radius at back wall.
				double radRingMin = ringRadius - (rsworld->AtmosphereRadius() + wallThickness);	// Radius at wall tops.
				double radRingWid = ringWidth2 + wallThickness;			// 1/2 ring width at the outside of the wall.

				// Calculate my avoidance zone around the ring cylinder.
				double radRingMaxZone = radRingMax + FeetPerMoonOrbit();
				double radRingMinZone = radRingMin - FeetPerMoonOrbit();
				double radRingWidZone = FeetPerOrbit()/5. + shipRad;	// Width based on DaWorld::CheckChildExit

				// Convert my current location and course destination into the space of the ringworld section.
				AuMatrix4x4 shipToRingT = ringT.Inverse() * shipT;	// To project locs from ship space into ring space.
				AuPoint pathOrig = shipToRingT.Project(AuPoint(0,0));	// Location of ship relative to ringworld section.
				AuPoint pathDest = shipToRingT.Project(courseLoc);	// Course destination relative to ringworld section.
				double radOrig = ringRadius - rsworld->CGAltitude(pathOrig);
				double radDest = ringRadius - rsworld->CGAltitude(pathDest);
				//qDebug("radRingMax %g radRingMin %g radRingWid %g radOrig %g radDest %g pathOrig.y %g pathDest.y %g",
						//radRingMax, radRingMin, radRingWid, radOrig, radDest, pathOrig.y, pathDest.y);

				// NOTE: Must use fuzzy logic here due to results of projecting large numbers.
				if(FuzzyLessThan(radOrig, radRingMinZone) && FuzzyLessThan(radDest, radRingMinZone))
				{
					//qDebug("path inside ring");
					continue;	// Both path locations are inside the ringworld so it's not an obstacle.
				}
				else if(FuzzyGreaterThan(pathOrig.y, radRingWid) && FuzzyGreaterThan(pathDest.y, radRingWidZone))
				{
					//qDebug("path above ring");
					continue;	// Both path locations are above the ringworld so it's not an obstacle.
				}
				else if(FuzzyGreaterThan(-pathOrig.y, radRingWid) && FuzzyGreaterThan(-pathDest.y, radRingWidZone))
				{
					//qDebug("path below ring");
					continue;	// Both path locations are below the ringworld so it's not an obstacle.
				}
				else if(FuzzyGreaterThan(radOrig, radRingMax) && FuzzyGreaterThan(radDest, radRingMax))
				{
					// Both path locations are outside the ringworld and not both above or both below.
					// Determine closest pass to ringworld.
					double pathDist = AuPoint(0,0).DistToSegment(pathOrig, pathDest);
					if(FuzzyGreaterThan(pathDist, radRingMaxZone))
					{
						//qDebug("path outside ring, pathDist %g", pathDist);
						continue;	// Closest distance to movement path is outside max radius of ringworld.
					}

					// Must fly above or below the ringworld to avoid it.
					// Fly straight up/down to clear the ring.
					// NOTE: Can't est for pathDest.y >= 0 because it fluctuates +/- really close to zero due to
					//       matrix projections.
					if(GreaterThanOrFuzzyEquals(pathDest.y, 0))
					{
						//qDebug("path intersects ring outside, fly above ring");
						if(FuzzyGreaterThan(pathOrig.y, radRingWid))
							pathDest.y = radRingWidZone+1;									// Fly to point above dest.
						else
							pathDest = AuPoint(pathOrig.x, radRingWidZone+1, pathOrig.z);	// Fly straight up.
					}
					else
					{
						//qDebug("path intersects ring outside, fly below ring");
						if(FuzzyGreaterThan(-pathOrig.y, radRingWid))
							pathDest.y = -(radRingWidZone+1);								// Fly to point below dest.
						else
							pathDest = AuPoint(pathOrig.x, -(radRingWidZone+1), pathOrig.z);	// Fly straight down.
					}
					courseLoc = shipToRingT.UnProject(pathDest);
					anyObstacle = true;						// Force re-eval of nuly calculated courseLoc.
					break;
				}
				else if((FuzzyLessThan(radOrig, radRingMax) && FuzzyGreaterThan(radDest, radRingMax)) ||
						(FuzzyGreaterThan(radOrig, radRingMax) && FuzzyLessThan(radDest, radRingMax)))
				{
					// One location is outside and one location is inside the ring but not both above or both below.
					// Must fly above or below the ringworld to avoid it.
					if(FuzzyGreaterThan(pathOrig.y, radRingWid))
					{
						// I'm well above the ring. Move the destination up.
						//qDebug("path crosses ring, fly above dest");
						pathDest.y = radRingWidZone+1;								// Fly to point above dest.
						courseLoc = shipToRingT.UnProject(pathDest);
					}
					else if(FuzzyGreaterThan(-pathOrig.y, radRingWid))
					{
						// I'm well below the ring. Move the destination down.
						//qDebug("path crosses ring, fly below dest");
						pathDest.y = -(radRingWidZone+1);							// Fly to point below dest.
						courseLoc = shipToRingT.UnProject(pathDest);
					}
					else
					{
						// I'm not above or below the ring.
						// Fly straight up/down to clear the ring.
						if(GreaterThanOrFuzzyEquals(pathDest.y, 0))
						{
							//qDebug("path crosses ring, fly above ring");
							pathOrig.y = radRingWidZone+1;
						}
						else
						{
							//qDebug("path crosses ring, fly below ring");
							pathOrig.y = -(radRingWidZone+1);
						}
						courseLoc = shipToRingT.UnProject(pathOrig);
					}
					anyObstacle = true;						// Force re-eval of nuly calculated courseLoc.
					break;
				}
				else if(FuzzyLessThan(radDest, radOrig))
				{
					//qDebug("path moves away from ring");
					continue;					// Moving away from the ring is not an obstacle.
				}
				else
				{
					// All that's left is both locations are between the outer walls of the ringworld. At least
					// one of them is too close to the ring and the path is moving toward the ring.
					// Pull the destination just inside the minimum radius of the ring.
					//qDebug("path moves too close to ring, fly inside of dest");
					pathDest = rsworld->Transform().Project(pathDest);		// pathDest relative to parent sun.
					pathDest.Unit(radRingMinZone-1);						// Scale pathDest relative to sun.
					pathDest = rsworld->Transform().UnProject(pathDest);	// pathDest relative to ringworld section again.

					courseLoc = shipToRingT.UnProject(pathDest);			// courseLoc relative to ship.
					anyObstacle = true;						// Force re-eval of nuly calculated courseLoc.

					// Clear the waypoint when we are close enough to the shortened location.
					// Otherwise we'll be stuck here.
					speedPlanDist = courseLoc.Length();
					if(speedPlanDist < 1000.*FeetPerMeter())
					{
						//qDebug() << Id() << Name() << "calling a waypoint good" << ship->WaypointCount();
						// Call that waypoint good.
						ship->RemoveWaypoint(0);
					}
					break;
				}
			}
			else if((*bi)->IsObstacle(this))
			{
				double avoidRad = (*bi)->PilotAvoidRadius();
				if(avoidRad == 0)
					continue;	// Object is not an obstacle to spacecraft.

				double spacingWanted = avoidRad + shipRad;
				double obstacleRad = (*bi)->ObstacleRadius();
				double spacingNeeded = obstacleRad + shipRad;
				//if(Name() == "SN Ocxutejti" && (*bi)->AtomId() == AuId(205515800))
				//qDebug() << "DaSentient::PilotToSystemLoc" << Id() << Name() << "blip" << (*bi)->AtomId() << (*bi).ScreenName()
						//<< "avoidRad" << avoidRad << "spacingWanted" << spacingWanted
						//<< "obstacleRad" << obstacleRad << "spacingNeeded" << spacingNeeded;

				// Calculate location of blip relative to the ship.
				AuPoint blipShipLoc = ship->EnvTransform().UnProject((*bi)->EnvLocation());

				// Determine if the blip is behind me if I go in the direction of the projected course.
				double pathDist = blipShipLoc.Dot(pathVec);	// Distance along pathVec is calculated.
				if(pathDist <= 0)
					continue;	// Ignore obstacles that are behind me or adjacent to me.

				// Test raw distance to object.
				double obDist = blipShipLoc.Length();	// Distance to the object from the ship.
				if(obDist >= destDist + spacingNeeded)
					continue;	// Ignore obstacles that are too far away.

				// Calculate distance from blip to the course destination location.
				double courseDist = blipShipLoc.Distance(courseLoc);

				// If both the ship and the destination are inside the zone of a planet, impose a speed limit.
				float speedLimitRange = (*bi)->PilotSpeedLimitRange();
				if(speedLimitRange > 0)
				{
					// Obstacle is never ignored, even if too close.
					if(obDist < speedLimitRange && courseDist < speedLimitRange)
						speedLimit = 1000*FeetPerMeter();
				}
				else
				{
					// Ignore obstacles that we are already too close to, except stars and planets.
					if(obDist < spacingNeeded)
						continue;	// Already too close to this obstacle; ignore it.
				}

				//if(Name() == "SN Ocxutejti" && (*bi)->AtomId() == AuId(205515800))
					//qDebug() << "blip" << (*bi).m_eType
						//<< " obstacleRad" << obstacleRad/FeetPerMeter()
						//<< "+ shipRad" << shipRad/FeetPerMeter()
						//<< "= spacingNeeded" << spacingNeeded/FeetPerMeter()
						//<< " avoidRad" << avoidRad/FeetPerMeter()
						//<< "+ shipRad" << shipRad/FeetPerMeter()
						//<< "= spacingWanted" << spacingWanted/FeetPerMeter()
						//<< "pathDist" << pathDist/FeetPerMeter()
						//<< "obDist" << obDist/FeetPerMeter()
						//<< "courseDist" << courseDist/FeetPerMeter()
						//<< "destDist" << destDist/FeetPerMeter()
						//<< "speedLimit" << speedLimit/FeetPerMeter();

				// Test for our destination location being inside an obstruction.
				if(courseDist < spacingNeeded)
				{
					// Destination location is inside an obstacle.
					// Shorten the destination location to the intersection of the path with the spherical obstacle.
					AuSphere sphere(blipShipLoc, spacingNeeded);
					AuPoint isect1, isect2;
					int isect = sphere.Intersect(AuPoint(0,0), courseLoc, isect1, isect2);
					if(isect > 0)
					{
						// Sphere intersection was found. isect1 is the closest point.
						courseLoc = isect1;
						destDist = courseLoc.Length();
					}

					// Clear the waypoint when we are close enough to the shortened location.
					// Otherwise we'll be stuck here.
					speedPlanDist = destDist;
					if(speedPlanDist < 1000*FeetPerMeter())
					{
						//qDebug() << Id() << Name() << "calling another waypoint good" << ship->WaypointCount()
							//<< speedPlanDist/FeetPerMeter();
						// Call that waypoint good.
						ship->RemoveWaypoint(0);
					}

					// 'anyObstacle' is not set true here because this doesn't require a re-eval of previous obstructions.
					// The destination location has been shortened along the same path; the course has not been deflected.
					continue;
				}

				// Calculate perpendicular distance of obstacle off of my path.
				AuPoint dropLoc = pathVec * pathDist;			// Perpendicular drop location from ob loc to my path.
				AuPoint dropVec = dropLoc - blipShipLoc;	// Vector from object to its location dropped onto my path.
				const double dropDist = dropVec.Length();
				if(dropDist >= spacingWanted)
				{
					//if(Name() == "SN Ocxutejti" && (*bi)->AtomId() == AuId(205515800))
						//qDebug() << "dropDist" << dropDist << "> spacingWanted" << spacingWanted;
					continue;	// Ignore obstacles that are far enough to the side of my path.
				}

				// The sensor blip object is an obstacle.
				if(obstacles.contains(*bi))
				{
					obstacles[*bi]++;			// Increment number of times we've tried to avoid this obstacle.
					if(obstacles[*bi] > 100)
					{
						qWarning("DaSentient::PilotToSystemLoc %d %s piloting %d %s can't resolve obstacle %s, confused",
								Id().GetID(), (const char*)Name().toLatin1(),
								ship->Id().GetID(), (const char*)ship->Name().toLatin1(),
								(const char*)(*bi)->Name().toLatin1());
						amConfused = true;
						break;
					}
				}
				else
					obstacles.insert(*bi,1);		// First time we've tried to avoid this obstacle.

				// Calculate a vector to take the ship around it with a clearance that grows as we repeatedly try to
				// avoid an obstacle. That way we won't get stuck oscillating between two obstacles. We'll consistently
				// decide on the same path around it every time.
				const double clearance = spacingWanted + 1000*FeetPerMeter() * obstacles[*bi];
				//if(Name() == "SN Ocxutejti")
					//qDebug() << "Obstacle" << (*bi)->ScreenName()
						//<< "loc" << blipShipLoc/FeetPerMeter()
						//<< "hits" << obstacles[*bi]
						//<< "avoidRad" << (*bi)->PilotAvoidRadius()/FeetPerMeter()
						//<< "obsRad" << (*bi)->ObstacleRadius()/FeetPerMeter()
						//<< "shipRad" << shipRad/FeetPerMeter()
						//<< "spacingNeeded" << spacingNeeded/FeetPerMeter()
						//<< "spacingWanted" << spacingWanted/FeetPerMeter()
						//<< "clearance" << clearance/FeetPerMeter()
						//<< "dropDist" << dropDist/FeetPerMeter()
						//<< "dropVec" << dropVec/FeetPerMeter()
						//<< "obDist" << obDist/FeetPerMeter();

				if(dropDist > 0)
				{
					// Pass the object.
					dropVec /= dropDist;
					dropVec *= clearance;
					courseLoc = blipShipLoc + dropVec;
				}
				else
				{
					// Obstacle is exactly on my path.
					courseLoc = blipShipLoc;
					if(IsLeftHanded())
						courseLoc.x -= clearance;		// Turn left to avoid the obstacle.
					else
						courseLoc.x += clearance;		// Turn right to avoid the obstacle.
				}
				anyObstacle = true;						// Force re-eval of nuly calculated courseLoc.
				break;
			}
		}

		/*
		if(!anyObstacle)
		{
			// Check for any wormholes that obstruct my path.
			// Wormhole is an obstacle to this spacecraft.
			const AuWormholeArray& worms = system->Wormholes();
			for(AuWormholeArray::const_iterator wi = worms.cbegin(); wi != worms.cend(); wi++)
			{
				double avoidRad = (*wi).PilotAvoidRadius();
				double spacingWanted = avoidRad + shipRad;
				double obstacleRad = (*wi).ObstacleRadius();
				double spacingNeeded = obstacleRad + shipRad;

				// Ignore the wormhole I am trying to get to.
				AuPoint whSysLoc = (*wi).Location(system);		// System location of wormhole.
				if(isWormhole && sysLoc.Distance(whSysLoc) < obstacleRad)
					continue;

				// Calculate location of blip relative to the ship.
				AuPoint blipShipLoc = EnvTransform().UnProject(whSysLoc);

				// Determine if the blip is behind me if I go in the direction of the projected course.
				double pathDist = blipShipLoc.Dot(pathVec);	// Distance along pathVec is calculated.
				if(pathDist <= 0)
					continue;	// Ignore obstacles that are behind me or adjacent to me.

				// Test raw distance to object.
				double obDist = blipShipLoc.Length();	// Distance to the object from the ship.
				if(obDist >= destDist + spacingNeeded)
					continue;	// Ignore obstacles that are too far away.

				// Calculate distance from blip to the course destination location.
				double courseDist = blipShipLoc.Distance(courseLoc);

				// Test for our destination location being inside an obstruction.
				if(courseDist < spacingNeeded)
				{
					// Destination location is inside an obstacle.
					// Shorten the destination location to the intersection of the path with the spherical obstacle.
					AuSphere sphere(blipShipLoc, spacingNeeded);
					AuPoint isect1, isect2;
					int isect = sphere.Intersect(AuPoint(0,0), courseLoc, isect1, isect2);
					if(isect > 0)
					{
						// Sphere intersection was found. isect1 is the closest point.
						courseLoc = isect1;
						destDist = courseLoc.Length();
					}

					// Clear the waypoint when we are close enough to the shortened location.
					// Otherwise we'll be stuck here.
					speedPlanDist = destDist;
					if(!isWormhole && speedPlanDist < 50*FeetPerMeter())
						ship->RemoveWaypoint(0);	// Call that waypoint good.

					// 'anyObstacle' is not set true here because this doesn't require a re-eval of previous obstructions.
					// The destination location has been shortened along the same path; the course has not been deflected.
					continue;
				}

				// Calculate perpendicular distance of obstacle off of my path.
				AuPoint dropLoc = pathVec * pathDist;			// Perpendicular drop location from ob loc to my path.
				AuPoint dropVec = dropLoc - blipShipLoc;	// Vector from object to its location dropped onto my path.
				const double dropDist = dropVec.Length();
				if(dropDist >= spacingWanted)
				{
					//if(Name() == "SN Ocxutejti" && (*bi)->AtomId() == AuId(205515800))
						//qDebug() << "dropDist" << dropDist << "> spacingWanted" << spacingWanted;
					continue;	// Ignore obstacles that are far enough to the side of my path.
				}

				// The sensor blip object is an obstacle.
				if(obstacleWorms.contains(wi))
				{
					obstacleWorms[wi]++;			// Increment number of times we've tried to avoid this obstacle.
					if(obstacleWorms[wi] > 100)
					{
						qWarning("DaSentient::PilotToSystemLoc %d %s can't resolve wormhole obstacle, confused",
								Id().GetID(), (const char*)Name().toLatin1());
						amConfused = true;
						break;
					}
				}
				else
					obstacleWorms.insert(wi,1);		// First time we've tried to avoid this obstacle.

				// Calculate a vector to take the ship around it with a clearance that grows as we repeatedly try to
				// avoid an obstacle. That way we won't get stuck oscillating between two obstacles. We'll consistently
				// decide on the same path around it every time.
				const double clearance = spacingWanted + 1000*FeetPerMeter() * obstacleWorms[wi];
				if(dropDist > 0)
				{
					// Pass the object.
					dropVec /= dropDist;
					dropVec *= clearance;
					courseLoc = blipShipLoc + dropVec;
				}
				else
				{
					// Obstacle is exactly on my path.
					courseLoc = blipShipLoc;
					if(IsLeftHanded())
						courseLoc.x -= clearance;		// Turn left to avoid the obstacle.
					else
						courseLoc.x += clearance;		// Turn right to avoid the obstacle.
				}
				anyObstacle = true;						// Force re-eval of nuly calculated courseLoc.
				break;
			}
		}
		*/
	}
	while(anyObstacle && !amConfused && tries++ < 100);
	if(tries >= 100)
		amConfused = true;	// I can't find a route past the obstacle.

	double pitch, yaw;
	if(amConfused)
	{
		// There is an obstacle I can't figure out how to deal with.
		if(IsLeftHanded())
			yaw = 10*DEG2RAD;	// Turn to the left slowly.
		else
			yaw = -10*DEG2RAD;	// Turn to the right slowly.
		pitch = 0;

		// Impose a speed limit when things have gotten out of hand.
		speedLimit = 100*FeetPerMeter();
		//qDebug("confused, imposing %gm/s speed limit", speedLimit/FeetPerMeter());
	}
	else
	{
		// Calculate pitch and yaw needed to get on course to the destination.
		pitch = courseLoc.PolarIncline();
		yaw = courseLoc.ArcTangent_2D() - PI/2;
		if(yaw < -PI)
			yaw += PI*2;

		// Roll according to flight attitude.
		FLIGHT_ATTITUDE fatt = FlightAttitude(ship);	// Returns upright in atmosphere.
		if(fatt == FLY_Inverted)
			roll += PI;
		else if(fatt == FLY_LeftSide)
			roll -= PI/2;
		else if(fatt == FLY_RightSide)
			roll += PI/2;
		if(roll < -PI)
			roll += PI*2;
		else if(roll > PI)
			roll -= PI*2;
	}

	// Halt lateral movement.
	bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
	DoShipBrakeState(ship, false, brakeLV);

	// Plan speed changes.
	// NOTE: The 'skew flip' mentioned in the comments is a reference from Robert Heinlein's "Have Spacesuit Will 
	//       Travel". This was the point at which a spacecraft would stop accelerating toward the destination and
	//       turn around to decelerate the rest of the way to the destination, about the half-way point.
	if(speed > speedLimit && speedLimit > 0)
	{
		// We have exceeded our speed limit, probably approaching the zone of a planet or confused.
		//qDebug("speed %gm/s exceeds limit %gm/s, braking", speed/FeetPerMeter(), speedLimit/FeetPerMeter());
		if(ship->HasWarpFTLDrive())
		{
			quint16 whichFTL = ship->FTLWarpDrive();
			if(ship->HasHumanOfficer())
				DoShipDisengageFTLDrive(ship, 3, whichFTL, destName);
			else
				DoShipDisengageFTLDrive(ship, 0, whichFTL, destName);
		}

		DoShipKillThrottles(ship);
		// Brake to slow down.
		DoShipBrakeState(ship, true, brakeLV);
	}
	else if(wormholePolarity != AuWormhole::Invalid)
	{
		// Next waypoint is a wormhole. Plan for low speed entry.
		if(IsFuzzyZero(speed))
		{
			// Ship is not moving.
			DoShipThrottleState(ship, 1); // Start moving.
		}
		else
		{
			// Ship is moving.
			double targetSpeed;
			if(doFAF)
				targetSpeed = 200 * FeetPerMeter();			// Low speed for turn at FAF, confusion.
			else if(!ship->IsReadyToWormhole())
				targetSpeed = 100 * FeetPerMeter();			// Low speed for wormhole when FTL drive not ready.
			else if(speedLimit > 0)
				targetSpeed = speedLimit;					// Make straight-in entry at maximum speed.
			else
				targetSpeed = SpeedOfLight();

			const double ete = speedPlanDist / speed;		// sec to arrive at destination

			// If the ETE to the wormhole is less than 15 seconds, activate the wormhole FTL drive.
			if(ete < 15)
			{
				// Set polarity of FTL drive appropriately.
				quint16 whichFTL = ship->FTLWormholeDrive();
				switch(wormholePolarity)
				{
				case AuWormhole::Positive:
				case AuWormhole::StargatePositive:
					if(ship->FTLDriveSetting(whichFTL) != 0)
						DoShipFTLDriveSetting(ship, 0, whichFTL);	// Set positive polarity.
					break;
				case AuWormhole::Negative:
				case AuWormhole::StargateNegative:
					if(ship->FTLDriveSetting(whichFTL) != 1)
						DoShipFTLDriveSetting(ship, 1, whichFTL);	// Set negative polarity.
					break;
				case AuWormhole::Intergalactic:
				case AuWormhole::IntergalacticOneWay:
					if(ship->FTLDriveSetting(whichFTL) != 2)
						DoShipFTLDriveSetting(ship, 2, whichFTL);	// Set neutral polarity.
					break;
				case AuWormhole::Invalid:
				case AuWormhole::POLARITY_Count:
					break;
				}

				int announce;
				if(ship->HasHumanOfficer())
					announce = 1;
				else
					announce = 0;

				// Activate the wormhole FTL drive, if it can be.
				DoShipEngageFTLDrive(ship, announce, destName, whichFTL);
			}

			//qDebug("dist %gm speed %gm/s targetSpeed %gm/s pitch %gdeg yaw %gdeg accel %gm/s/s ete %gsec slow %gsec",
					//dist/FeetPerMeter(), speed/FeetPerMeter(), targetSpeed/FeetPerMeter(), pitch*RAD2DEG, yaw*RAD2DEG,
					//ship->ManeuverAcceleration()/FeetPerMeter(), dist/speed,
					//(speed - targetSpeed)/ship->ManeuverAcceleration());
			if(ete < 3 || (!ship->IsReadyToWormhole() && ete < 10))
			{
				// Only seconds to go so just coast.
				// Ten seconds at wormhole allows time for a slow engineer to fire the FTL drive.
				DoShipKillThrottles(ship);
				// Brake if necessary to slow down or turn.
				if(speed > targetSpeed || fabs(pitch) > 15.*DEG2RAD || fabs(yaw) > 15.*DEG2RAD)
					DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// More than a few seconds to go.
				if(accel > 0)
				{
					// Gravity drive is working.
					double slowTime = (speed - targetSpeed) / accel;	// sec to slow to target speed
					if(slowTime >= ete)
					{
						// Ship is moving too fast.
						// Reduce speed to arrive at destination going the target speed.
						DoShipKillThrottles(ship);
						// Hold the brakes.
						DoShipBrakeState(ship, true, brakeLV);
					}
					else
					{
						// Accelerate toward the skew flip.
						if(fabs(pitch) < 15.*DEG2RAD && fabs(yaw) < 15.*DEG2RAD)
						{
							if(amConfused)
								DoShipThrottleState(ship, 1);	// Accelerate slowly when confused.
							else
								DoShipThrottleState(ship, 10);	// Accelerate to max speed before the skew flip.
						}
						else
						{
							// Ship is turning significantly.
							// Coast until the turn is complete.
							DoShipKillThrottles(ship);
							// Hold the brakes to keep from overshooting the final approach course.
							if(speedPlanDist < FeetPerOrbit())
								DoShipBrakeState(ship, true, brakeLV);
						}
					}
				}
			}
		}
	}
	else
	{
		// Waypoint is in this solar system. How far away is it?
		// Operate the FTL drive.
		if(ship->HasWarpFTLDrive())
		{
			// Check on course. Very hard to turn at warp speed.
			quint16 whichFTL = ship->FTLWarpDrive();
			bool onCourseForWarp = false;
			bool isStarTraveller = ship->MissionOrderIsStarTravelOrder();
			if(isStarTraveller)
			{
				AuPoint courseVec(courseLoc);
				double distp = courseVec.Length();
				if(distp != 0)
					courseVec /= distp;
				double angDot = courseVec.Dot(AuPoint(0,1,0));
				//onCourseForWarp = angDot > .984807753;	// 10 degrees
				//onCourseForWarp = angDot > 0.996194698;	// 5 degrees
				onCourseForWarp = angDot > 0.999048222;		// 2.5 degrees
			}

			if(isStarTraveller && speedPlanDist < FeetPerSolarSystem()/2)
			{
				// Disengage at solar system perimeter. This lets the system transition happen. Then I can plan my approach.
				// The ship is within a solar system radius of the destination star.
				//qDebug() << "DaSentient::PilotToSystemLoc" << *this << "destName" << destName << "dist" << speedPlanDist
					//<< "destRad" << destRadius;
				if(destName.isEmpty())
					DoShipDisengageFTLDrive(ship, ship->HasHumanOfficer() ? 1 : 0, whichFTL, destName);	// Close enough for empty
				else if(destRadius == 0  && speedPlanDist < FeetPerSolarSystem()/3)
					DoShipDisengageFTLDrive(ship, ship->HasHumanOfficer() ? 1 : 0, whichFTL, destName);	// in system
				else if(speedPlanDist < destRadius)
					DoShipDisengageFTLDrive(ship, ship->HasHumanOfficer() ? 1 : 0, whichFTL, destName);	// Within oort radius.
				else if(speedPlanDist < destRadius + FeetPerOrbit()*2)
					DoShipDisengageFTLDrive(ship, ship->HasHumanOfficer() ? 4 : 0, whichFTL, destName);	// In the oord cloud.
			}
			else if(!isStarTraveller && speedPlanDist < FeetPerSolarSystem()/4)
			{
				// Disengage near target. This lets the system transition happen. Then I can plan my approach.
				DoShipDisengageFTLDrive(ship, ship->HasHumanOfficer() ? 5 : 0, whichFTL, destName);
			}
			else
			{
				// Engage warp drive when far enough away and pointing in the right direction.
				// Check against the distance the ship travels at warp 1 (C x 10) in 10 seconds.
				// Farther than that is safe to engage, at least at warp 1.
				if(speedPlanDist >= SpeedOfWarp() * 10.)
				{
					// Far enough away to use warp drive to get there. Turn on the FTL drive.
					if(!ship->IsFTLDriveOn(whichFTL))
						DoShipFTLDriveState(ship, true, whichFTL);

					// Validation at warp factor 0.
					bool canCommit;
					DaWorldLL* world;
					if(altMode == 0)
						world = ship->FindParentWorldInEnv();
					else
						world = 0;
					if(world && altitude < world->AtmosphereRadius() + OrbitAltAboveAtm()*2)
						canCommit = false;								// Not while ship is near a world.
					else if(onCourseForWarp)
						canCommit = ship->CanEngageFTLAt0(whichFTL);	// Ship is on course. Is warp drive ready?
					else
						canCommit = false;								// Waiting to be on course.

					// If we know we can commit at least at warp factor 1, proceed.
					if(canCommit)
					{
						int maxWarpFactor = ship->MaxCapacitorWarpFactor();
						if(maxWarpFactor >= 0)
						{
							// Calculate warp factor needed. Denominator is distance traveled at warp 1 in 30 seconds.
							int ftlDriveSetting = (int)(speedPlanDist / (SpeedOfWarp() * 30.));
							if(ftlDriveSetting > maxWarpFactor)
								ftlDriveSetting = maxWarpFactor;
							DoShipFTLDriveSetting(ship, ftlDriveSetting, whichFTL);

							int announce;
							if(ship->HasHumanOfficer())
								announce = 1;
							else
								announce = 0;

							// Activate the FTL drive.
							DoShipEngageFTLDrive(ship, announce, destName, whichFTL);
						}
					}
				}
			}
		}

		if(ship->WaypointCount() <= 1)
		{
			// Next waypoint is the last waypoint. Plan to stop at the destination.
			if(IsFuzzyZero(speed))
			{
				// Ship is not moving.
				DoShipThrottleState(ship, 1);	// Start moving.
			}
			else
			{
				// Ship is moving.
				if(accel > 0)
				{
					// Gravity drive is working.
					double ete = speedPlanDist / speed;				// sec to arrive at destination
					double stopTime = speed / accel;				// sec to stop at destination
					//qDebug("DaSentient::PilotToSystemLoc stopTime %gsec ete %gsec", stopTime, ete);
					if(stopTime > ete)
					{
						// Ship is moving too fast. Reduce speed to stop right at the destination.
						DoShipKillThrottles(ship);
						DoShipBrakeState(ship, true, brakeLV);

						// Avoid the target while going too fast.
						/*
						if(stopTime > ete*1.05 && !ship->IsAnyFTLDriveHot())
						{
							// Turn 90 degrees away from our course.
							pitch = courseLoc.XRotation();			// Pitch directly toward the target.
							if(pitch > 0)
							{
								pitch -= PIf/2;						// Pitch down away from the target.
								roll = courseLoc.YRotation();		// Roll the top toward the target.
							}
							else
							{
								pitch += PIf/2;						// Pitch up away from the target.
								roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
								if(roll > PI)
									roll -= PI*2;
							}
							yaw = 0;
						}
						*/
					}
					else
					{
						// Accelerate toward the skew flip.
						if(ete > 3 && fabs(pitch) < 5.*DEG2RAD && fabs(yaw) < 5.*DEG2RAD)
						{
							if(amConfused)
								DoShipThrottleState(ship, 1);	// Accelerate slowly when confused.
							else
								DoShipThrottleState(ship, 10);	// Accelerate to max speed before the skew flip.
						}
						else
							DoShipKillThrottles(ship);
					}
				}
			}
		}
		else
		{
			// Next waypoint is not a wormhole and it's not my last waypoint.
			if(amConfused)
				DoShipThrottleState(ship, 1);	// Accelerate slowly when confused.
			else
				DoShipThrottleState(ship, 10);	// Accelerate to max speed before the skew flip.
		}
	}

	if(fabs(pitch) < DEG2RAD)
		pitch = 0;

	if(fabs(yaw) < DEG2RAD)
		yaw = 0;

	// We must roll when pitch is nonzero because pitch is based on the up vector. Yaw is not affected by the up vector.
	if(fabs(roll) < DEG2RAD || pitch == 0)
		roll = 0;

	DoShipRollPitchYaw(ship, roll, pitch, yaw);
}

// 'planet' is the parent planet of the 'ship'.
void DaSentient::PilotToBd(DaVessel* ship, const DaBd* bd, const DaWorld* planet)
{
	if(ship->MissionOrderType() == AuMissionOrder::PatrolCities)
	{
		// Hover above the building.
		AuPoint hoverLoc = bd->Location();
		if(hoverLoc.IsZero())
		{
			// No hover loc was found.
			PilotHoldPosition(ship);

			// We're patrolling cities. Call this one good so we can move on to the next city.
			DoShipArrivedAtDestination(ship, bd->Id());
		}
		else
		{
			const float dist = planet->CGAltitude(hoverLoc) + 100*FeetPerMeter() + ship->GroundHeightAboveCG();
			planet->ElevateLoc(hoverLoc, dist);
			PilotToHoverSite(ship, planet, hoverLoc);
		}
	}
	else if(ship->MissionOrderType() == AuMissionOrder::GoToCityAnywhere)
	{
		// Going to land near a bd.
		if(ship->IsOnGround())
		{
			if(bd->Is(ship->BdId()))
			{
				// On the ground at the destination bd site.
				PilotHoldPosition(ship);
				if(!ship->IsMoving())
					DoShipArrivedAtDestination(ship, bd->Id());
				return;
			}
			else
			{
				AuPoint shipLoc;
				ship->GetLocationRelativeToParent(shipLoc, planet);
				AuPoint bdLoc = bd->Location();
				if(shipLoc.Distance(bdLoc) < 5000)
				{
					// On the ground near the destination bd.
					PilotHoldPosition(ship);
					if(!ship->IsMoving())
						DoShipArrivedAtDestination(ship, bd->Id());
					return;
				}
			}
		}

		// Locate a good landing site near the bd. It's ok to prefer the airport.
		AuPoint landLoc = FindAirportLandingSite(ship, planet, bd);
		if(landLoc.IsZero())
			PilotHoldPosition(ship);		// No landing site was found. Wait. Maybe a spot will clear.
		else
			PilotToLandingSite(ship, planet, landLoc);
	}
	else
	{
		// Going to land at an airport in the city.
		INDUSTRYID iid = ship->InIndustry();
		if(ship->IsOnGround() && (iid == IID_AirportTerminal || iid == IID_BaseAirTerminal) && bd->CapitolId() == ship->InBdCity())
		{
			// On a road slab in the destination city.
			PilotHoldPosition(ship);
			if(!ship->IsMoving())
				DoShipArrivedAtDestination(ship, bd->CapitolId());
			return;
		}

		// Locate a good landing site at an airport in the city.
		AuPoint landLoc = FindAirportLandingSite(ship, planet, bd);
		if(landLoc.IsZero())
			PilotHoldPosition(ship);		// No landing site was found. Wait. Maybe a spot will clear.
		else
			PilotToLandingSite(ship, planet, landLoc);
	}
}

// 'planet' is the parent planet of the 'ship'.
void DaSentient::PilotToHoverSite(DaVessel* ship, const DaWorld* planet, const AuPoint& hoverLoc)
{
	if(ship->IsInAtmosphere())
	{
		// In the atmosphere.
		ResetOnApproach();									// Reset to be ready for next approach into atmosphere.

		// Calculate ship's location relative to the planet.
		AuMatrix4x4 shipT;
		ship->GetTransformRelativeToParent(shipT, planet);
		AuPoint shipLoc = shipT.Project(AuPoint(0,0));
		double dist = shipLoc.Distance(hoverLoc);
		if(dist < 50*FeetPerMeter())
		{
			PilotHoldPosition(ship);		// Close enough to the hover location.

			// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
			if(!ship->IsOnGround())
				DoShipLandingGear(ship, false);

			// Call the destination arrived if we are patrolling cities.
			if(ship->MissionOrderType() == AuMissionOrder::PatrolCities)
				DoShipArrivedAtDestination(ship, ship->InBdCity());
		}
		else if(ship->IsOnGround())
			PilotVerticalTakeOff(ship);	// Get off the ground.
		else
		{
			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
			if(rsworld)
			{
				// Ringworld
				if(dist < 2000*FeetPerMeter())
				{
					// Impose a speed limit so we don't overstress the ship.
					float speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
					double terClearance = ship->GroundHeightAboveCG() + 50*FeetPerMeter();
					PilotToPlanetLocation(ship, shipT, shipLoc, planet, hoverLoc, speedLimit, 20*FeetPerMeter(), terClearance,
							false);
				}
				else
				{
					// Too far from hover location. Take off into space and fly around the planet. Less chance of hitting terrain.
					PilotVerticalTakeOff(ship);
				}
			}
			else
			{
				// Planet
				// Watch for angle from destination point. Then we are clear of planet sphere and can go direct.
				// This angle is intentionally wider of the decision angle to enter the atmosphere.
				// NOTE: If an angle other than 90 is tested, the (shipLoc - hoverLoc) vector must be a unit vector.
				AuPoint destUpVec(hoverLoc);
				destUpVec.Unit();								// Unit vector point up toward destination.
				double dot = (shipLoc - hoverLoc).Dot(destUpVec);
				if(dot < 0)
				{
					// Wrong hemisphere. Take off into space and fly around the planet. Less chance of hitting terrain.
					PilotVerticalTakeOff(ship);
				}
				else
				{
					// Impose a speed limit so we don't overstress the ship.
					float speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
					double terClearance = ship->GroundHeightAboveCG() + 50*FeetPerMeter();
					PilotToPlanetLocation(ship, shipT, shipLoc, planet, hoverLoc, speedLimit, 20*FeetPerMeter(), terClearance,
							false);
				}
			}
		}
	}
	else
	{
		// Not in the atmosphere yet. Fly to the point near the landing segment and descend into the atmosphere.
		// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
		DoShipLandingGear(ship, false);
		// Calculate a point in orbit directly above the hover site.
		// The altitude of this point is the distance above the atmosphere we will fly.
		AuPoint orbitLoc(hoverLoc);
		const float dist = planet->AtmosphereRadius() + OrbitAltAboveAtm();	// Atmosphere radius plus a safety margin.

		// Note the distance at which we decide to dive in must be well within the distance that we decide is too far to navigate
		// and requires a vertical takeoff, which is 1000 miles.
		// Once we start heading in, fast rotation of the planet must not deter us.
		const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
		if(rsworld)
		{
			// Ringworld
			planet->ElevateLoc(orbitLoc, dist);

			ResetOnApproach();								// Reset for next planetary approach. N/a for ringworld approaches.
			double segDist = ship->Location().DistToSegment(orbitLoc, hoverLoc);	// Distance from landing segment.
			if(segDist < 1000*FeetPerMeter())
			{
				// Send the ship toward the actual hover location. Other logic will take over once we enter the atmosphere.
				// Impose a speed limit so we don't overstress the ship when we hit the atmosphere.
				float speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, hoverLoc, speedLimit, 1, 0, false);
			}
			else
			{
				// Fly directly to the orbit location across the ring.
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, orbitLoc, SpeedOfLight(),
						10*FeetPerMeter(), 0, false);
			}
		}
		else
		{
			// Planet
			orbitLoc.Unit(dist);

			// Calculate ship's location relative to the planet.
			AuMatrix4x4 shipT;
			ship->GetTransformRelativeToParent(shipT, planet);
			AuPoint shipLoc = shipT.Project(AuPoint(0,0));

			// Watch for angle from destination point. Then we are clear of planet sphere and can go direct.
			AuPoint destUpVec(hoverLoc);
			destUpVec.Unit();									// Unit vector point up toward destination.
			AuPoint vecToShip = shipLoc - hoverLoc;
			vecToShip.Unit();
			double dot = vecToShip.Dot(destUpVec);
			if(dot > 0.342020143 || IsOnApproach())				// sin(20 degrees)
			{
				// Send the ship toward the actual hover location. Other logic will take over once we enter the atmosphere.
				SetOnApproach();							// Pilot is now committed to making the approach.

				// Impose a speed limit so we don't overstress the ship when we hit the atmosphere.
				float speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, hoverLoc, speedLimit, 1, 0, false);
			}
			else
			{
				// Fly around the planet to orbitLoc, at the altitude of orbitLoc, which has a built-in safety margin.
				ResetOnApproach();							// Pilot is not on the entry approach segment.

				// Calculate appropriate speed limit.
				float speedLimit = SpeedOfLight();				// No speed limit.
				const DaWorldSphere* glworld = dynamic_cast<const DaWorldSphere*>(planet);
				if(glworld)
				{
					if(AuPoint(0,0).DistToSegment(ship->Location(), orbitLoc) < planet->AtmosphereRadius() + OrbitAltAboveAtm())
						speedLimit = sqrt((float)glworld->Radius100km()/10) * 200*FeetPerMeter();	// Speed limit close to globe.
				}
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, orbitLoc, speedLimit,
						10*FeetPerMeter(), 0, false);
			}
		}
	}
}

// 'planet' is the parent planet of the 'ship'.
void DaSentient::PilotToLandingSite(DaVessel* ship, const DaWorld* planet, const AuPoint& landLoc)
{
	// A landing site was found.
	// Calculate a point in orbit directly above the landing site.
	// The altitude of this point is the distance above the atmosphere we will fly.
	AuPoint orbitLoc(landLoc);
	const double dist = planet->AtmosphereRadius() + OrbitAltAboveAtm();	// Atmosphere radius plus a safety margin.
	planet->ElevateLoc(orbitLoc, dist);
	const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);

	if(ship->IsInAtmosphere())
	{
		// In the atmosphere.
		ResetOnApproach();									// Reset to be ready for next approach into atmosphere.

		// Calculate ship's location relative to the planet.
		AuMatrix4x4 shipT;
		ship->GetTransformRelativeToParent(shipT, planet);
		AuPoint shipBaseLoc = shipT.Project(0,0,-ship->GroundHeightAboveCG());	// Allow for height of ship above insertion pt.
		double segDist = shipBaseLoc.DistToSegment(orbitLoc, landLoc);			// Distance of ship base from landing segment.
		//if(ship->Name() == "Soxmer")
			//qDebug() << "DaSentient::PilotToLandingSite" << Id() << Name() << "ship" << ship->Id() << ship->Name()
				//<< "segDist" << segDist / FeetPerMeter() << ((segDist < 10*FeetPerMeter()) ? "Vert" : "App");
		if(segDist < 10*FeetPerMeter())
		{
			// Close enough to the landing segment.
			if(ship->IsOnGround())
				PilotHoldPosition(ship);		// We're on the ground safe and sound.
			else
				PilotVerticalLanding(ship);	// Not on the ground yet.
		}
		else if(ship->IsOnGround())
			PilotVerticalTakeOff(ship);
		else if(rsworld)
		{
			// Ringworld
			if(segDist < 2000*FeetPerMeter())
			{
				// Fly to the landing segment.
				double terClearance = ship->GroundHeightAboveCG() + 100*FeetPerMeter();
				AuPoint atmLoc(landLoc);
				const float dist = planet->CGAltitude(landLoc) + terClearance;
				planet->ElevateLoc(atmLoc, dist);

				//qDebug("%s PilotToCity fly to landing segment", (const char*)Name().toAscii());
				// Impose a speed limit so we don't overstress the ship.
				float speedLimit;
				if(segDist < 250*(float)FeetPerMeter())
					speedLimit = 50*(float)FeetPerMeter();	// fast enough when lining up to land.
				else if(segDist < 150*(float)FeetPerMeter())
					speedLimit = 25*(float)FeetPerMeter();	// fast enough when lining up to land.
				else if(segDist < 50*(float)FeetPerMeter())
					speedLimit = 10*(float)FeetPerMeter();	// fast enough when lining up to land.
				else
					speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
				AuPoint shipLoc = shipT.Translation();
				PilotToPlanetLocation(ship, shipT, shipLoc, planet, atmLoc, speedLimit, 5*FeetPerMeter(), terClearance, true);
			}
			else
			{
				// Too far from landing segment. Take off into space and fly around the planet. Less chance of hitting terrain.
				PilotVerticalTakeOff(ship);
			}
		}
		else
		{
			// Planet
			// Watch for angle from destination point. Then we are clear of planet sphere and can go direct.
			// This angle is intentionally wider of the decision angle to enter the atmosphere.
			// NOTE: If an angle other than 90 is tested, the (shipBaseLoc - landLoc) vector must be a unit vector.
			AuPoint destUpVec(orbitLoc);
			destUpVec.Unit();									// Unit vector point up toward destination.
			double dot = (shipBaseLoc - landLoc).Dot(destUpVec);
			if(dot < 0)
			{
				// Wrong hemisphere. Take off into space and fly around the planet. Less chance of hitting terrain.
				PilotVerticalTakeOff(ship);
			}
			else
			{
				// Fly to the landing segment inside the atmosphere.
				double terClearance = ship->GroundHeightAboveCG() + 100*FeetPerMeter();
				AuPoint atmLoc(landLoc);
				const float dist = planet->CGAltitude(landLoc) + terClearance;
				atmLoc.Unit(dist);

				/*
				if(ship->Name() == "Soxmer")
				{
					double segDist = atmLoc.DistToSegment(orbitLoc, landLoc);
					qDebug() << "DaSentient::PilotToLandingSite" << Id() << Name() << "ship" << ship->Id() << ship->Name()
						<< "landSegDist" << segDist / FeetPerMeter();
				}
				*/

				//qDebug("%s PilotToCity fly to landing segment", (const char*)Name().toAscii());
				// Impose a speed limit so we don't overstress the ship.
				float speedLimit;
				if(segDist < 250*(float)FeetPerMeter())
					speedLimit = 50*(float)FeetPerMeter();	// fast enough when lining up to land.
				else if(segDist < 150*(float)FeetPerMeter())
					speedLimit = 25*(float)FeetPerMeter();	// fast enough when lining up to land.
				else if(segDist < 50*(float)FeetPerMeter())
					speedLimit = 10*(float)FeetPerMeter();	// fast enough when lining up to land.
				else
					speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
				AuPoint shipLoc = shipT.Translation();
				PilotToPlanetLocation(ship, shipT, shipLoc, planet, atmLoc, speedLimit, 5*FeetPerMeter(), terClearance, true);
			}
		}
	}
	else
	{
		// Not in the atmosphere yet. Fly to the point near the landing segment and descend into the atmosphere.
		// Note the distance at which we decide to dive in must be well within the distance that we decide is too far to navigate
		// and requires a vertical takeoff, which is 1000 miles.
		// Once we start heading in, fast rotation of the planet must not deter us.
		if(rsworld)
		{
			// Ringworld
			ResetOnApproach();								// Reset for next planetary approach. N/a for ringworld approaches.
			double segDist = ship->Location().DistToSegment(orbitLoc, landLoc);	// Distance from landing segment.
			if(segDist < 1000*FeetPerMeter())
			{
				// Send the ship toward the actual landing site. Other logic will take over once we enter the atmosphere.
				// Impose a speed limit so we don't overstress the ship when we hit the atmosphere.
				float speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, landLoc, speedLimit, 1, 0, false);
			}
			else
			{
				// Fly directly to the orbit location across the ring.
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, orbitLoc, SpeedOfLight(),
						10*FeetPerMeter(), 0, false);
			}
		}
		else
		{
			// Planet
			// Calculate ship's location relative to the planet.
			AuMatrix4x4 shipT;
			ship->GetTransformRelativeToParent(shipT, planet);
			AuPoint shipLoc = shipT.Project(AuPoint(0,0));

			// Watch for angle from destination point. Then we are clear of planet sphere and can go direct.
			AuPoint destUpVec = orbitLoc/dist;					// Unit vector point up toward destination.
			AuPoint vecToShip = shipLoc - landLoc;
			vecToShip.Unit();
			double dot = vecToShip.Dot(destUpVec);
			if(dot > 0.342020143 || IsOnApproach())				// sin(20 degrees)
			{
				// Send the ship toward the actual landing site. Other logic will take over once we enter the atmosphere.
				SetOnApproach();							// Pilot is now committed to making the approach.

				// Impose a speed limit so we don't overstress the ship when we hit the atmosphere.
				float speedLimit = ship->MaxAtmosphereSpeed(planet) * .9f;
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, landLoc, speedLimit, 1, 0, false);
			}
			else
			{
				// Fly around the planet to orbitLoc, at the altitude of orbitLoc, which has a built-in safety margin.
				ResetOnApproach();							// Pilot is not on the entry approach segment.

				// Calculate appropriate speed limit.
				float speedLimit = SpeedOfLight();				// No speed limit.
				const DaWorldSphere* glworld = dynamic_cast<const DaWorldSphere*>(planet);
				if(glworld)
				{
					if(AuPoint(0,0).DistToSegment(ship->Location(), orbitLoc) < planet->AtmosphereRadius() + OrbitAltAboveAtm())
						speedLimit = sqrt((float)glworld->Radius100km()/10) * 200*FeetPerMeter();	// Speed limit close to globe.
				}
				PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, orbitLoc, speedLimit,
						10*FeetPerMeter(), 0, false);
			}
		}
	}
}

// Return value is relative to the planet atom.
AuPoint DaSentient::FindAirportLandingSite(const DaVessel* ship, const DaWorld* planet, const DaBd* bd) const
{
	const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
	if(!llworld)
		return AuPoint(0,0);

	// The city or base may have one or more airport terminals. Look for landing spots next to them.
	DaAtomList terminals;
	bd->CommunityAirTerminals(terminals);
	if(!terminals.empty())
	{
		// The community has one or more air terminals.
		const double clearance = ship->RoughRadius() + 5*FeetPerMeter();
		for(DaAtomList::const_iterator ti = terminals.cbegin(); ti != terminals.cend(); ti++)
		{
			DaBd* cbd = (DaBd*)*ti;
			QList<AuMatrix4x4> parkMs;
			cbd->SpacecraftParkingSpots(parkMs);
			for(QList<AuMatrix4x4>::const_iterator pi = parkMs.cbegin(); pi != parkMs.cend(); pi++)
			{
				AuPoint parkLoc = (*pi).Translation();			// Parking spot location relative to the design.
				parkLoc = cbd->Transform().Project(parkLoc);		// Parking spot location relative to the world.
				if(!llworld->IsLandingAreaObstructed(parkLoc, clearance, ship))
					return parkLoc;
			}
		}
	}

	// We did not find an unobstructed landing spot next to an airport terminal.
	// Find any decent landing site in the community.
	return FindLandingSite(ship, planet, bd);
}

// Return value is relative to the planet atom.
AuPoint DaSentient::FindLandingSite(const DaVessel* ship, const DaWorld* planet, const DaBd* bd) const
{
	const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
	if(!llworld)
		return AuPoint(0,0);

	AuPoint cityLoc;
	bd->GetLocationRelativeToParent(cityLoc, llworld);

	const double clearance = ship->RoughRadius() + 5*FeetPerMeter();

	DaAtomList bds;
	if(bd->IsMilitary())
		llworld->FindBdBase(bds);
	else
		llworld->FindBdCity(bds, bd->CapitolId());

	if(!bds.empty())
	{
		// The community has one or more buildings.
		const double clearance = ship->RoughRadius() + 5*FeetPerMeter();
		for(DaAtomList::const_iterator ti = bds.cbegin(); ti != bds.cend(); ti++)
		{
			DaBd* cbd = (DaBd*)*ti;
			QList<AuMatrix4x4> parkMs;
			cbd->SpacecraftParkingSpots(parkMs);
			for(QList<AuMatrix4x4>::const_iterator pi = parkMs.cbegin(); pi != parkMs.cend(); pi++)
			{
				AuPoint parkLoc = (*pi).Translation();			// Parking spot location relative to the design.
				parkLoc = cbd->Transform().Project(parkLoc);		// Parking spot location relative to the world.
				if(!llworld->IsLandingAreaObstructed(parkLoc, clearance, ship))
					return parkLoc;
			}
		}
	}

	// No spacecraft parking spots found.
	// Land on open road within city limits if all else fails. Ship can still trade there.
	AuGeologicalFeatures roads;	// Make a copy of the roads to avoid possible deadlocks. We're going to call a lot of stuff.
	const AuGeologicalFeatures& geo = llworld->LockGeologicalFeatures();
	for(AuGeologicalFeatures::const_iterator gi = geo.cbegin(); gi != geo.cend(); gi++)
		if((*gi)->IsRoad())
			roads.append((*gi)->Clone());
	llworld->UnlockGeologicalFeatures();
	if(roads.empty())
		return AuPoint(0,0);	// No roads on which to land.

	double asphSlabWid = 0;
	AuPoint asphSlabLoc;
	double asphRoadWid = 0;
	AuPoint asphRoadLoc;
	double concSlabWid = 0;
	AuPoint concSlabLoc;
	double concRoadWid = 0;
	AuPoint concRoadLoc;
	double dirtSlabWid = 0;
	AuPoint dirtSlabLoc;
	double dirtRoadWid = 0;
	AuPoint dirtRoadLoc;
	quint32 cityEmpireId;
	bool hasWater = llworld->Hydrographics();
	for(AuGeologicalFeatures::const_iterator gi = roads.cbegin(); gi != roads.cend(); gi++)
	{
		// Check for paved roads first because they are derived from dirt roads.
		const AuGeoRoadAsphalt* asphRoad = dynamic_cast<const AuGeoRoadAsphalt*>(*gi);
		if(asphRoad)
		{
			bool isSlab = asphRoad->IsSlab();
			if((isSlab && asphSlabWid < asphRoad->Width()) ||
					(!isSlab && asphRoadWid < asphRoad->Width()))
			{
				// Widest road found so far.
				AuPoint roadLoc = asphRoad->Center(*llworld);
				double roadMSL = llworld->AltitudeMSL(roadLoc);
				if(!hasWater || roadMSL > 0)
				{
					// Road location is not under water.
					AuGlobeLL roadLL;
					llworld->LLOfLoc(roadLL, roadLoc);
					if(!llworld->InEnclosure(roadLL, roadMSL) &&
							(bd->IsMilitary() || bd->CapitolId() == llworld->JurisdictionAt(roadLoc, cityEmpireId)) &&
							!llworld->IsLandingAreaObstructed(roadLoc, clearance, ship))
					{
						// Road location found, unobstructed, not in an enclosure, and part of the city or base of the building.
						if(isSlab)
						{
							asphSlabWid = asphRoad->Width();
							asphSlabLoc = roadLoc;
						}
						else
						{
							asphRoadWid = asphRoad->Width();
							asphRoadLoc = roadLoc;
						}
					}
				}
			}
			continue;
		}

		const AuGeoRoadConcrete* concRoad = dynamic_cast<const AuGeoRoadConcrete*>(*gi);
		if(concRoad)
		{
			bool isSlab = concRoad->IsSlab();
			if((isSlab && concSlabWid < concRoad->Width()) ||
					(!isSlab && concRoadWid < concRoad->Width()))
			{
				// Widest road found so far.
				AuPoint roadLoc = concRoad->Center(*llworld);
				double roadMSL = llworld->AltitudeMSL(roadLoc);
				if(!hasWater || roadMSL > 0)
				{
					// Road location is not under water.
					AuGlobeLL roadLL;
					llworld->LLOfLoc(roadLL, roadLoc);
					if(!llworld->InEnclosure(roadLL, roadMSL) &&
							(bd->IsMilitary() || bd->CapitolId() == llworld->JurisdictionAt(roadLoc, cityEmpireId)) &&
							!llworld->IsLandingAreaObstructed(roadLoc, clearance, ship))
					{
						// Road location found, unobstructed, not in an enclosure, and part of the city or base of the building.
						if(isSlab)
						{
							concSlabWid = concRoad->Width();
							concSlabLoc = roadLoc;
						}
						else
						{
							concRoadWid = concRoad->Width();
							concRoadLoc = roadLoc;
						}
					}
				}
			}
			continue;
		}

		const AuGeoRoadDirt* dirtRoad = dynamic_cast<const AuGeoRoadDirt*>(*gi);
		if(dirtRoad)
		{
			bool isSlab = dirtRoad->IsSlab();
			if((isSlab && dirtSlabWid < dirtRoad->Width()) ||
					(!isSlab && dirtRoadWid < dirtRoad->Width()))
			{
				// Widest road found so far.
				AuPoint roadLoc = dirtRoad->Center(*llworld);
				double roadMSL = llworld->AltitudeMSL(roadLoc);
				if(!hasWater || roadMSL > 0)
				{
					// Road location is not under water.
					AuGlobeLL roadLL;
					llworld->LLOfLoc(roadLL, roadLoc);
					if(!llworld->InEnclosure(roadLL, roadMSL) &&
							(bd->IsMilitary() || bd->CapitolId() == llworld->JurisdictionAt(roadLoc, cityEmpireId)) &&
							!llworld->IsLandingAreaObstructed(roadLoc, clearance, ship))
					{
						// Road location found, unobstructed, not in an enclosure, and part of the city or base of the building.
						if(isSlab)
						{
							dirtSlabWid = dirtRoad->Width();
							dirtSlabLoc = roadLoc;
						}
						else
						{
							dirtRoadWid = dirtRoad->Width();
							dirtRoadLoc = roadLoc;
						}
					}
				}
			}
			continue;
		}
	}
	roads.Clear();

	// Take the best one that fits me.
	double widWanted = clearance*2;
	if(concSlabWid >= widWanted)
		return concSlabLoc;
	if(asphSlabWid >= widWanted)
		return asphSlabLoc;
	if(dirtSlabWid >= widWanted)
		return dirtSlabLoc;
	if(concRoadWid >= widWanted)
		return concRoadLoc;
	if(asphRoadWid >= widWanted)
		return asphRoadLoc;
	if(dirtRoadWid >= widWanted)
		return dirtRoadLoc;

	// Take any.
	if(concSlabWid > 0)
		return concSlabLoc;
	if(asphSlabWid > 0)
		return asphSlabLoc;
	if(dirtSlabWid > 0)
		return dirtSlabLoc;
	if(concRoadWid > 0)
		return concRoadLoc;
	if(asphRoadWid > 0)
		return asphRoadLoc;
	if(dirtRoadWid > 0)
		return dirtRoadLoc;

	// Found nothing.
	return AuPoint(0,0);
}

// 'star' is the destination star. It is known to be the parent star of 'ship' when this is called.
void DaSentient::PilotToStar(DaVessel* ship, const DaStar* star, const AuPoint& destOffset)
{
	ResetOnApproach();										// Reset for atmospheric approach.

	Q_ASSERT(!destOffset.IsZero());
	if(ship->MissionOrderType() == AuMissionOrder::GoToOrbit ||
			ship->MissionOrderType() == AuMissionOrder::GoToOrbitCoords)
	{
		// Fly to coordinate location in orbit above the star.
		if(ship->IsInAtmosphere())
			PilotVerticalTakeOff(ship);				// The ship is in atmosphere range of a planet. Take-off.
		else
		{
			// Ship is in space. Move to the specified orbit location.
			// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
			if(!ship->IsOnGround())
				DoShipLandingGear(ship, false);

			// Check ship proximity to orbit location.
			AuMatrix4x4 shipT;
			ship->GetTransformRelativeToParent(shipT, star);
			AuPoint shipLoc = shipT.Project(AuPoint(0,0));
			if(shipLoc.Distance2(destOffset) > (100*FeetPerMeter())*(100*FeetPerMeter()))
			{
				// Move to within 10m of the target location. This will push the ship into the 100m radius zone we want to
				// keep so it will shuffle along a little at a time.
				PilotToStarLocation(ship, shipT, shipLoc, star, destOffset, SpeedOfLight(), 10*FeetPerMeter());
			}
			else
			{
				// Orbiting within 100m of target. That's good enough.
				PilotHoldPosition(ship);					// Hold position in standard orbit zone.
			}
		}
	}
	else
		PilotHoldPosition(ship);							// Garbled destination, hold position.
}

// 'planet' is the destination planet. It is known to be the parent planet of 'ship' when this is called.
void DaSentient::PilotToPlanet(DaVessel* ship, const DaWorld* planet, const AuPoint& destOffset)
{
	Q_ASSERT(!destOffset.IsZero());
	AuMissionOrder::TYPE t = ship->MissionOrderType();
	if(t == AuMissionOrder::GoToOrbit ||
			t == AuMissionOrder::GoToOrbitCity ||
			t == AuMissionOrder::GoToOrbitBestPlanet ||
			t == AuMissionOrder::GoToOrbitCoords)
	{
		// Fly to coordinate location in orbit above the planet.
		if(ship->IsInAtmosphere())
			PilotVerticalTakeOff(ship);				// The ship is in atmosphere range of a planet. Take-off.
		else
		{
			// Ship is in space. Move to the specified orbit location.
			// Check ship proximity to orbit location. This test works for rings and planets.
			AuMatrix4x4 shipT;
			ship->GetTransformRelativeToParent(shipT, planet);
			AuPoint shipLoc = shipT.Project(AuPoint(0,0));
			double dist2 = shipLoc.Distance2(destOffset);
			if(dist2 > (100*FeetPerMeter())*(100*FeetPerMeter()))
			{
				// Move to within 100m of the target location.
				PilotToPlanetLocation(ship, shipT, shipLoc, planet, destOffset, SpeedOfLight(), 10*FeetPerMeter(), 0, false);
			}
			else
			{
				// Orbiting within 100m of target. That's good enough.
				PilotHoldPosition(ship);					// Hold position in standard orbit zone.
			}
		}
	}
	else if(t == AuMissionOrder::GoToLand)
	{
		if(ship->IsInVisibleWater())
			PilotVerticalTakeOff(ship);	// Don't remain in water/lava/supercooled liquid.
		else if(ship->IsOnGround())
			PilotHoldPosition(ship);		// Hold where we are at on the ground.
		else
			PilotToLandingSite(ship, planet, destOffset);
	}
	else if(t == AuMissionOrder::GoToLandingSite)
	{
		// Land at a specified longitude, latitude on the planet.
		if(!destOffset.IsZero())
			PilotToLandingSite(ship, planet, destOffset);
		else
			PilotHoldPosition(ship);	// Garbled destination, hold position.
	}
	else
	{
		// Hover at a specified longitude, latitude on the planet.
		if(!destOffset.IsZero())
			PilotToHoverSite(ship, planet, destOffset);
		else
			PilotHoldPosition(ship);	// Garbled destination, hold position.
	}
}

// 'planet' is the parent planet of 'ship' and 'station'.
void DaSentient::PilotToStation(DaVessel* ship, const DaVessel* station, const DaWorld* planet)
{
	if(ship->IsInAtmosphere())
	{
		PilotVerticalTakeOff(ship);	// The ship is in atmosphere range of a planet. Take-off.
		return;
	}

	// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
	if(!ship->IsOnGround())
		DoShipLandingGear(ship, false);

	// Check proximity to the destination station.
	AuPoint vecToDest = station->Location() - ship->Location();
	double tradeRange = station->StationParkingRange(ship);
	double dist2 = vecToDest.Length2();							// square of distance to station
	if(dist2 < tradeRange*tradeRange)
	{
		// We are close enough to the station; turn the ship side toward the station if the engines are still on.
		if(ship->GravityDriveState() == DaVessel::GD_On && ship->GravityDriveWorking())
		{
			// Gravity drive is full-on (not hold) and it is working.
			AuPoint stationLoc = ship->Transform().UnProject(station->Location());	// Station location relative to the ship.
			AuPoint stationUp = ship->Transform().UnProjectVector(station->Transform().ProjectVector(AuPoint(0,0,1)));	// Up vec
			if(ship->PilotUpPolicy() == UpTowardGravity)
				stationUp = -stationUp;

			// Pitch the ship level with the station's up vector.
			double pitch = stationUp.XRotation() - PI/2;		// pitch needed to align with station up vector.
			if(pitch < -PI)
				pitch += PI*2;

			// Roll and yaw the ship so the side faces the station. I will choose the side that I sit closest to.
			double roll;
			double yaw = stationLoc.ArcTangent_2D() - PI/2;
			if(Location().x <= 0)
			{
				roll = stationLoc.PolarIncline();
				yaw -= PI/2;			// I am sitting left of center so turn the port side to the station.
			}
			else
			{
				roll = -stationLoc.PolarIncline();
				yaw += PI/2;			// I am sitting right of center so turn the starboard side to the station.
			}
			if(yaw < -PI)
				yaw += PI*2;
			else if(yaw > PI)
				yaw -= PI*2;

			// More than 5 degrees of either pitch or roll is enough to turn the ship.
			if(fabs(pitch) > 5*DEG2RAD || fabs(roll) > 5*DEG2RAD || fabs(yaw) > 5*DEG2RAD)
			{
				// Pitch or roll is needed to align with the station.
				DoShipRollPitchYaw(ship, roll, pitch, yaw);

				// Stop at current location.
				if(!ship->IsPowerPlantOn())
					DoShipPowerPlantState(ship, true);
				else
				{
					DoShipKillThrottles(ship);
					if(ship->InBlackHoleGravity())
						DoShipBrakeState(ship, true, true);
					else
						DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
				}
			}
			else
				PilotHoldPosition(ship);	// Hold position in trade range of the station.
		}
		else
		{
			// Gravity drive is no longer On, hopefully it is on Hold and not dead.
			PilotHoldPosition(ship);	// Hold position in trade range of the station.
			if(!ship->IsMoving())
				DoShipArrivedAtDestination(ship, station->Id());
		}
	}
	else
	{
		// Orbit the planet to reach the station.
		//qDebug() << "DaSentient::PilotToStation" << Id() << Name() << "approaching station";
		// Station is close enough that we will aim for a point near the station to park.
		AuPoint parkLoc;
		if(station->ParkingSpotInRange(ship, parkLoc))
		{
			// parkLoc is relative to the station.
			parkLoc = station->EnvTransform().Project(parkLoc);
			parkLoc = planet->EnvTransform().UnProject(parkLoc);	// Now it's in the space of the planet.

			// Calculate appropriate speed limit for maneuvering near the atmosphere.
			float speedLimit;
			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
			if(rsworld)
				speedLimit = SpeedOfLight();					// No speed limit catching up to the target.
			else
			{
				// Speed limit if we clip close to globe.
				if(AuPoint(0,0).DistToSegment(ship->Location(), parkLoc) < planet->AtmosphereRadius() + OrbitAltAboveAtm())
				{
					const DaWorldSphere* glworld = dynamic_cast<const DaWorldSphere*>(planet);
					if(glworld)
						speedLimit = sqrt((float)glworld->Radius100km()/10) * 200*FeetPerMeter();
					else
						speedLimit = SpeedOfLight();
				}
				else
					speedLimit = SpeedOfLight();				// No speed limit catching up to the target.
			}

			PilotToPlanetLocation(ship, ship->Transform(), ship->Location(), planet, parkLoc, speedLimit, 1, 0, false);
		}
		else
		{
			// A parking location was not found within trade range of the station.
			PilotHoldPosition(ship);
		}
	}
}

// Simple obstacle avoidance that ignores other planets and stars.
// 'destLoc' is in the space of the ship. It is modified to avoid obstacles. Result is in the space of the ship.
// 'shipLoc' is in the space of the planet.
// 'shipT' is matrix from planet to ship. When ship's parent is planet, expecting ship->Transform().
// returns true if I can't figure out a path.
bool DaSentient::PilotObstacleAvoid(AuPoint& courseLoc, const DaVessel* ship) const
{
	const DaAtom* system = ship->FindEnvironmentAtom();
	if(!system)
		return true;

	// Modify the destination location based on any obstacles along the path.
	// Sensor data provides the information we need for this.
	QMap<DaAtom*, int> obstacles;	// Count of each obstacle encountered as we do multipasses.
	bool anyObstacle;			// Set true if I change courseLoc due to an obstacle, requiring re-eval of all blips.
	bool amConfused = false;	// Set true if I can't figure out how to get around an obstacle.
	double shipRad = ship->RoughRadius();
	int tries = 0;	// Don't do this forever.
	DaAtomList blips;
	system->ListNormalSubatomsRecursive(blips);
	do
	{
		anyObstacle = false;	// Will be set true if we find any obstacle in the course direction.

		// Calculate unit vector pointing toward destination. This is my path vector.
		double destDist = courseLoc.Length();	// Distance to course location.
		if(destDist == 0)
			break;
		AuPoint pathVec(courseLoc);
		pathVec /= destDist;					// Make path vector into a unit vector.

		// Check for any atoms that obstruct my path.
		for(DaAtomList::const_iterator bi = blips.begin(); bi != blips.end(); bi++)
		{
			if(ship->Is((*bi)->Id()))
				continue;

			// We're not worried about other stars or planets this close to our parent planet.
			if(dynamic_cast<DaStar*>(*bi) || dynamic_cast<DaWorld*>(*bi))
				continue;

			if(!(*bi)->IsObstacle(this))
				continue;

			// Object is an obstacle to this spacecraft.
			double avoidRad = (*bi)->PilotAvoidRadius();
			if(avoidRad == 0)
				continue;	// Object is not an obstacle to spacecraft.
			double spacingWanted = avoidRad + shipRad;
			double obstacleRad = (*bi)->ObstacleRadius();
			double spacingNeeded = obstacleRad + shipRad;

			// Calculate location of blip relative to the ship.
			AuPoint blipShipLoc = ship->EnvTransform().UnProject((*bi)->EnvLocation());

			// Determine if the blip is behind me if I go in the direction of the projected course.
			double pathDist = blipShipLoc.Dot(pathVec);	// Distance along pathVec is calculated.
			if(pathDist <= 0)
				continue;	// Ignore obstacles that are behind me or adjacent to me.

			// Test raw distance to object.
			double obDist = blipShipLoc.Length();	// Distance to the object from the ship.
			if(obDist >= destDist + spacingNeeded)
				continue;	// Ignore obstacles that are too far away.

			// Ignore obstacles that we are already too close to.
			if(obDist < spacingNeeded)
				continue;	// Already too close to this obstacle; ignore it.

			// Calculate distance from blip to the course destination location.
			double courseDist = blipShipLoc.Distance(courseLoc);

			// Test for our destination location being inside an obstruction.
			if(courseDist < spacingNeeded)
			{
				// Destination location is inside an obstacle.
				// Shorten the destination location to the intersection of the path with the spherical obstacle.
				AuSphere sphere(blipShipLoc, spacingNeeded);
				AuPoint isect1, isect2;
				int isect = sphere.Intersect(AuPoint(0,0), courseLoc, isect1, isect2);
				if(isect > 0)
					courseLoc = isect1;							// Sphere intersection was found. isect1 is the closest point.

				// 'anyObstacle' is not set true here because this doesn't require a re-eval of previous obstructions.
				// The destination location has been shortened along the same path; the course has not been deflected.
				continue;
			}

			// Calculate perpendicular distance of obstacle off of my path.
			AuPoint dropLoc = pathVec * pathDist;			// Perpendicular drop location from ob loc to my path.
			AuPoint dropVec = dropLoc - blipShipLoc;	// Vector from object to its location dropped onto my path.
			const double dropDist = dropVec.Length();
			if(dropDist >= spacingWanted)
				continue;	// Ignore obstacles that are far enough to the side of my path.

			// The sensor blip object is an obstacle.
			if(obstacles.contains(*bi))
			{
				obstacles[*bi]++;			// Increment number of times we've tried to avoid this obstacle.
				if(obstacles[*bi] > 100)
				{
					qWarning("DaSentient::PilotObstacleAvoid %d %s piloting %d %s can't resolve orbital obstacle %s, confused",
							Id().GetID(), (const char*)Name().toLatin1(),
							ship->Id().GetID(), (const char*)ship->Name().toLatin1(),
							(const char*)(*bi)->Name().toLatin1());
					amConfused = true;
					break;
				}
			}
			else
				obstacles.insert(*bi,1);		// First time we've tried to avoid this obstacle.

			// Calculate a vector to take the ship around it with a clearance that grows as we repeatedly try to
			// avoid an obstacle. That way we won't get stuck oscillating between two obstacles. We'll consistently
			// decide on the same path around it every time.
			const double clearance = spacingWanted + 100*FeetPerMeter() * obstacles[*bi];
			//qDebug("obstacle %s dropDist %gm spacingNeeded %g blipRad %gm shipRad %gm hits %d clearance %gm "
					//"obDist %gm speed %gm/s",
					//(const char*)(*bi)->ScreenName().toAscii(), dropDist/FeetPerMeter(), spacingNeeded/FeetPerMeter(),
					//blipRad/FeetPerMeter(), shipRad/FeetPerMeter(),
					//obstacles[bi], clearance/FeetPerMeter(), obDist/FeetPerMeter(), ship->Speed()/FeetPerMeter());
			if(dropDist > 0)
			{
				// Pass the object.
				dropVec /= dropDist;
				dropVec *= clearance;
				courseLoc = blipShipLoc + dropVec;
			}
			else
			{
				// Obstacle is exactly on my path.
				courseLoc = blipShipLoc;
				if(IsLeftHanded())
					courseLoc.x -= clearance;		// Turn left to avoid the obstacle.
				else
					courseLoc.x += clearance;		// Turn right to avoid the obstacle.
			}
			anyObstacle = true;						// Force re-eval of nuly calculated courseLoc.
			break;
		}
	}
	while(anyObstacle && !amConfused && tries++ < 100);
	if(tries >= 100)
		amConfused = true;	// I can't find a route past the obstacle.
	return amConfused;
}

// 'shipT' is the transform acquired by calling ship->GetTransformRelativeToParent(shipT, star). When the ship's parent atom is
// the star, the expected value is ship->Transform().
// 'star' is the parent star of 'ship', as returned by FindParentStarInEnv.
// 'shipLoc' and 'destLoc' are relative to the star.
void DaSentient::PilotToStarLocation(DaVessel* ship, const AuMatrix4x4& shipT, const AuPoint& shipLoc, const DaStar* star,
		const AuPoint& destLoc, double speedLimit, double closeness)
{
	ResetOnApproach();										// Reset for atmospheric approach.

	if(!ship->IsPowerPlantOn())
	{
		// Turn on the power plant if it's not already on.
		DoShipPowerPlantState(ship, true);
		return;
	}

	if(ship->GravityDriveState() != DaVessel::GD_On)
	{
		// Turn on the gravity drive if it's not already on.
		DoShipGravityDriveState(ship, DaVessel::GD_On);
		DoShipKillThrottles(ship);
		return;
	}

	const double accel = ship->ManeuverAcceleration();		// ft/sec/sec adjusted for damage
	if(accel == 0)
		return;

	// Modify the destination location based on any obstacles along the path.
	AuPoint courseLoc = shipT.UnProjectVector(destLoc - shipLoc);	// Vector to destLoc in space of ship.
	if(PilotObstacleAvoid(courseLoc, ship))
	{
		// I can't figure out how to avoid an obstacle.
		DoShipKillThrottles(ship);
		if(ship->InBlackHoleGravity())
			DoShipBrakeState(ship, true, true);
		else
			DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
		return;
	}

	// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
	if(!ship->IsOnGround())
		DoShipLandingGear(ship, false);

	// Perform maneuvers relative to the surface of the star.
	double speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude;
	ship->FlightInstruments(ship->DesignCenterOffset(), 4, speed, speedF, speedL, speedV, altitude, pitch, roll, heading,
			longitude, latitude);

	AuPoint starLoc = shipT.Project(courseLoc);					// Convert courseLoc to be in the space of the star.
	double targetAlt = starLoc.Length() - star->Radius();		// MSL altitude at destination location.

	double yaw;
	FLIGHT_ATTITUDE fatt = FlightAttitude(ship);	// Returns upright in atmosphere.
	switch(fatt)
	{
	default:
	case FLY_Upright:
		{
			// Turn toward destination location.
			double coursePitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			if(coursePitch < -89*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No yaw when more than 80 degrees of pitch down required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				yaw = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				yaw = courseLoc.ArcTangent_2D() - PI/2;
				if(yaw < -PI)
					yaw += PI*2;
			}

			// Adjust pitch.
			bool pitchIsAway = coursePitch > pitch;

			if(altitude > targetAlt + closeness || pitchIsAway)
			{
				// Current altitude is above the max or the desired pitch is a pitch up.
				pitch = coursePitch;		// in space we can take any pitch change
			}
			else if(altitude > targetAlt - closeness)
			{
				// Altitude is above the minimum.
				// Slight descent is needed to maintain altitude around ball; level causes climb.
				float distTraveled = speedF / ThoughtSpeed();	// Distance I will probably travel in the next think cycle.
				float orbitDia = ship->CGAltitude()*2;				// Diameter of circle at my altitude from center.
				float orbitCirc = PIf * orbitDia;					// Circumference of orbit at my altitude from center.
				float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Angle around planet ship will travel in one think cycle.
				float baseAngle = (PIf - orbitAngle)/2;				// Base angle of isosceles tri of center, cur & next locs.
				float pitchAdj = PIf/2 - baseAngle;					// Pitch adjustment to end at same altitude as start.
				pitch -= pitchAdj;
			}

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(yaw) > 3*DEG2RAD || fabs(pitch) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				// Brake to slow down.
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix. Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch > 0)
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								int maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when pitch is nonzero because pitch is based on the up vector. Yaw is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || pitch == 0)
				roll = 0;
		}
		break;

	case FLY_Inverted:
		{
			// Turn toward destination location.
			// Fly inverted.
			roll += PI;
			if(roll > PI)
				roll -= PI*2;
			pitch = -pitch;

			// If we're too close, we don't turn in the atmosphere. Instead, we move away, turn, then come back. This strategy works
			// better when we're trying to gain a little altitude.
			double coursePitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			if(coursePitch > 89*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No yaw when more than 80 degrees of pitch up required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				yaw = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				yaw = courseLoc.ArcTangent_2D() - PI/2;
				if(yaw < -PI)
					yaw += PI*2;
			}

			// Adjust pitch.
			bool pitchIsAway = coursePitch < pitch;

			if(altitude > targetAlt + closeness || pitchIsAway)
			{
				// Current altitude is above the max or the desired pitch is a pitch up.
				pitch = coursePitch;		// in space we can take any pitch change
			}
			else if(altitude > targetAlt - closeness)
			{
				// Altitude is above the minimum.
				// Slight descent is needed to maintain altitude around ball; level causes climb.
				float distTraveled = speedF / ThoughtSpeed();	// Distance I will probably travel in the next think cycle.
				float orbitDia = ship->CGAltitude()*2;				// Diameter of circle at my altitude from center.
				float orbitCirc = PIf * orbitDia;					// Circumference of orbit at my altitude from center.
				float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Angle around planet ship will travel in one think cycle.
				float baseAngle = (PIf - orbitAngle)/2;				// Base angle of isosceles tri of center, cur & next locs.
				float pitchAdj = PIf/2 - baseAngle;					// Pitch adjustment to end at same altitude as start.
				pitch += pitchAdj;
			}

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(yaw) > 3*DEG2RAD || fabs(pitch) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix. Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch < 0)
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								int maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when pitch is nonzero because pitch is based on the up vector. Yaw is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || pitch == 0)
				roll = 0;
		}
		break;

	case FLY_LeftSide:
		{
			// Fly with left side toward the gravity well.
			// Turn toward destination location.
			yaw = -pitch;	// Yaw angle to be level, once flying side-on.
			roll -= PI/2;

			double courseYaw = courseLoc.ArcTangent_2D() - PI/2;	// Yaw needed to point directly at target. D:
			if(courseYaw < -PI)
				courseYaw += PI*2;
			if(courseYaw > 89*DEG2RAD && courseYaw < 91*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No pitch when more than 80 degrees of yaw down required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				pitch = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				pitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			}

			// Adjust yaw.
			bool yawIsAway = courseYaw < yaw;	// Less yaw is a right turn away from the planet on the left.

			if(altitude > targetAlt + closeness || yawIsAway)
			{
				// Current altitude is above the max or the desired yaw is a yaw away from the sun.
				yaw = courseYaw;		// in space we can take any yaw change
			}
			else if(altitude > targetAlt - closeness)
			{
				// Altitude is above the minimum.
				// Slight descent is needed to maintain altitude around ball; level causes climb.
				float distTraveled = speedF / ThoughtSpeed();	// Distance I will probably travel in the next think cycle.
				float orbitDia = ship->CGAltitude()*2;				// Diameter of circle at my altitude from center.
				float orbitCirc = PIf * orbitDia;					// Circumference of orbit at my altitude from center.
				float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Angle around planet ship will travel in one think cycle.
				float baseAngle = (PIf - orbitAngle)/2;				// Base angle of isosceles tri of center, cur & next locs.
				float yawAdj = PIf/2 - baseAngle;					// Pitch adjustment to end at same altitude as start.
				yaw += yawAdj;
			}

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(pitch) > 3*DEG2RAD || fabs(yaw) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				// Brake to slow down.
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix.
				// Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch > 0)
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								int maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when yaw is nonzero because yaw is based on the up vector. Pitch is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || yaw == 0)
				roll = 0;
		}
		break;

	case FLY_RightSide:
		{
			// Fly with right side toward the gravity well.
			// Turn toward destination location.
			yaw = pitch;	// Yaw angle to be level, once flying side-on.
			roll += PI/2;

			double courseYaw = courseLoc.ArcTangent_2D() - PI/2;	// Yaw needed to point directly at target. D:
			if(courseYaw < -PI)
				courseYaw += PI*2;
			if(courseYaw < -89*DEG2RAD && courseYaw > -91*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No pitch when more than 80 degrees of yaw down required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				pitch = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				pitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			}

			// Adjust yaw.
			bool yawIsAway = courseYaw > yaw;	// More yaw is a left turn away from the planet on the right.

			if(altitude > targetAlt + closeness || yawIsAway)
			{
				// Current altitude is above the max or the desired yaw is a yaw away from the sun.
				yaw = courseYaw;		// in space we can take any yaw change
			}
			else if(altitude > targetAlt - closeness)
			{
				// Altitude is above the minimum.
				// Slight descent is needed to maintain altitude around ball; level causes climb.
				float distTraveled = speedF / ThoughtSpeed();	// Distance I will probably travel in the next think cycle.
				float orbitDia = ship->CGAltitude()*2;				// Diameter of circle at my altitude from center.
				float orbitCirc = PIf * orbitDia;					// Circumference of orbit at my altitude from center.
				float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Angle around planet ship will travel in one think cycle.
				float baseAngle = (PIf - orbitAngle)/2;				// Base angle of isosceles tri of center, cur & next locs.
				float yawAdj = PIf/2 - baseAngle;					// Pitch adjustment to end at same altitude as start.
				yaw -= yawAdj;
			}

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(pitch) > 3*DEG2RAD || fabs(yaw) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				// Brake to slow down.
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix.
				// Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch > 0)
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								int maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when yaw is nonzero because yaw is based on the up vector. Pitch is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || yaw == 0)
				roll = 0;
		}
		break;
	}

	DoShipRollPitchYaw(ship, roll, pitch, yaw);
}

// 'shipT' is the transform acquired by calling ship->GetTransformRelativeToParent(shipT, planet). When the ship's parent atom is
// the planet, the expected value is ship->Transform().
// 'planet' is the parent planet of 'ship', as returned by FindParentPlanetInEnv.
// 'shipLoc' and 'destLoc' are relative to the planet.
// 'terrainClearance' is only relevant if 'ship' is in atmosphere range of 'planet'
void DaSentient::PilotToPlanetLocation(DaVessel* ship, const AuMatrix4x4& shipT, const AuPoint& shipLoc,
		const DaWorld* planet, const AuPoint& destLoc, double speedLimit, double closeness, double terrainClearance, bool isLanding)
{
	if(!ship->IsPowerPlantOn())
	{
		// Turn on the power plant if it's not already on.
		DoShipPowerPlantState(ship, true);
		return;
	}

	if(ship->GravityDriveState() != DaVessel::GD_On)
	{
		// Turn on the gravity drive if it's not already on.
		DoShipGravityDriveState(ship, DaVessel::GD_On);
		DoShipKillThrottles(ship);
		return;
	}

	const double accel = ship->ManeuverAcceleration();		// ft/sec/sec adjusted for damage
	if(accel == 0)
		return;

	// Modify the destination location based on any obstacles along the path.
	AuPoint courseLoc = shipT.UnProjectVector(destLoc - shipLoc);	// Vector to destLoc in space of ship.
	if(PilotObstacleAvoid(courseLoc, ship))
	{
		// I can't figure out how to avoid an obstacle.
		DoShipKillThrottles(ship);
		if(ship->InBlackHoleGravity())
			DoShipBrakeState(ship, true, true);
		else
			DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
		return;
	}

	// Perform maneuvers relative to the surface of the planet.
	double speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude;
	ship->FlightInstruments(ship->DesignCenterOffset(), 1, speed, speedF, speedL, speedV, altitude, pitch, roll, heading,
			longitude, latitude);

	// Adjust target altitude based on proximity to the destination.
	AuPoint planetLoc = shipT.Project(courseLoc);				// Convert courseLoc to be in the space of the planet.
	double segDist2D = courseLoc.Length_2D();					// 2D distance to course location.
	double distToDest = courseLoc.Length();

	const DaWorldSphere* glworld = dynamic_cast<const DaWorldSphere*>(planet);

	// MSL altitude at destination location.
	double targetAlt = planet->AltitudeMSL(planetLoc);
	if(!isLanding)
	{
		// Not landing.
		// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
		if(!ship->IsOnGround())
			DoShipLandingGear(ship, false);
		if(ship->IsInAtmosphere())
		{
			// Ship is in atmosphere range of the planet.
			// Don't fly under water.
			if(targetAlt < ship->GroundHeightAboveCG())
				targetAlt = ship->GroundHeightAboveCG();			// Don't let targetAlt be under water.

			// Try VTOL.
			if(segDist2D < closeness && speed < accel*2)
			{
				// Not landing, ship is in atmosphere, and the 2D segment distance is close enough to stop within a second.
				// Check to see if we can use VTOL to achieve our desired altitude.
				if(altitude > targetAlt + closeness/2)
				{
					PilotVerticalLanding(ship);	// Descend VTOL.
					return;
				}
				else if(altitude < targetAlt - closeness/2)
				{
					PilotVerticalTakeOff(ship);	// Climb VTOL.
					return;
				}
			}
		}
		else
		{
			if(glworld)
			{
				// Ship is in orbit over a spherical planet.
				if(targetAlt > planet->AtmosphereHeightMSL())
				{
					// Target is also in orbit.
					// Fly at standard altitude while a long distance from the target location.
					if(distToDest > 2000*FeetPerMeter())
						targetAlt = planet->AtmosphereHeightMSL() + OrbitAltAboveAtm();	// Standard MSL altitude
				}
				else
				{
					// Target is inside the atmosphere.
					// Fly at standard altitude when a long distance from the landing segment.
					AuPoint plu(planetLoc);
					plu.Unit();
					double dot = shipLoc.Dot(plu);				// Negative sign says we're in opposite hemispheres.
					if(dot < 0)
						targetAlt = planet->AtmosphereHeightMSL() + OrbitAltAboveAtm();	// Standard MSL altitude
				}
			}
		}
	}

	double yaw;
	FLIGHT_ATTITUDE fatt = FlightAttitude(ship);	// Returns upright in atmosphere.
	switch(fatt)
	{
	default:
	case FLY_Upright:
		{
			// Turn toward destination location.
			// If we're too close, we don't turn in the atmosphere. Instead, we move away, turn, then come back. This strategy works
			// better when we're trying to gain a little altitude.
			double coursePitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			if(coursePitch < -89*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No yaw when more than 80 degrees of pitch down required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				yaw = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				yaw = courseLoc.ArcTangent_2D() - PI/2;
				if(yaw < -PI)
					yaw += PI*2;

				// When ship is in space we can yaw all we want.
				// We are allowed minor adjustments to yaw any time.
				// Make sharp turns at a distance away from the landing segment that grows with alititude AGL.
				//if(ship->IsInAtmosphere() && fabs(yaw) > 10*DEG2RAD && segDist2D < ship->AltitudeAGL())
					//yaw = 0;
			}

			// Don't make pitch changes too close to the landing segment or it messes with our vertical landing logic.
			if(!ship->IsInAtmosphere() || segDist2D > closeness)
			{
				// When ship is in space we can pitch all we want.
				// Make pitch changes a little ways away from the landing segment.
				// Pitch the ship to the destination location, or climb if we're too low.
				// Current calculated 'pitch' value would level the ship to the ground.
				// Note: Pitching level to the ground results in a climb on spherical bodies (non ringworlds).
				//       On ringworlds, we're not worried about clipping the atmosphere since it curves away from almost every path
				//       we take.
				if(ship->IsInAtmosphere() && (ship->AltitudeAGL() < terrainClearance || ship->AltitudeMSL() < terrainClearance))
					pitch += 10*DEG2RAD;	// Too close to the ground or sea level to use this code. Do you need VTOL?
				else
				{
					// Adjust pitch.
					bool pitchIsAway = coursePitch > pitch;

					if(altitude > targetAlt + closeness || pitchIsAway)
					{
						// Current altitude is above the max or the desired pitch is a pitch up.
						// Any pitch is ok when the ship is high enough or climbing in space.
						if(ship->IsInAtmosphere())
						{
							// In atmosphere, we restrict pitch-up or we'll fall backwards on our tail when going slowly.
							// Note: We take off VTOL. This code is for altitude adjustments to the final landing fix.
							double finalPitch = coursePitch - pitch;	// Pitch off of horizon
							if(finalPitch > 10*DEG2RAD)
								pitch += 10*DEG2RAD;	// Limit pitch-up.
							else
								pitch = coursePitch;	// No limit to pitch-down.
						}
						else
							pitch = coursePitch;		// in space we can take any pitch change
					}
					else if(altitude > targetAlt - closeness)
					{
						if(glworld)
						{
							// Not a ringworld and altitude is above the minimum.
							// Slight descent is needed to maintain altitude around globe; level causes climb.
							float distTraveled = speedF / ThoughtSpeed();	// Dist I will probably travel in the next think cycle.
							float orbitDia = ship->CGAltitude()*2;			// Diameter of circle at my altitude from center.
							float orbitCirc = PIf * orbitDia;				// Circumference of orbit at my altitude from center.
							float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Ang around planet ship will travel in one cycle.
							float baseAngle = (PIf - orbitAngle)/2;		// Base angle of isosceles tri of center, cur & next locs.
							float pitchAdj = PIf/2 - baseAngle;			// Pitch adjustment to end at same altitude as start.
							pitch -= pitchAdj;
						}
					}
				}
			}

			// Reduce speed limit when near the landing/hovering segment.
			// This makes final alignment more precise.
			if(ship->IsInAtmosphere() && segDist2D < 100*FeetPerMeter() && altitude < targetAlt + closeness)
				speedLimit /= 2;

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(yaw) > 3*DEG2RAD || fabs(pitch) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix. Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch > 0 && !ship->IsInAtmosphere())
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								// Max throttle is determined by hull streamlining and current maneuver acceleration.
								// Max throttle is also imposed when current planet is the destination planet.
								int maxThrottle;
								if(planet->HasAtmosphere() && (ship->IsInAtmosphere() || ship->Parent() == planet))
									maxThrottle = ship->MaxAtmosphereThrottle();	// Max throttle for atmosphere movement/entry.
								else
									maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when pitch is nonzero because pitch is based on the up vector. Yaw is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || (pitch == 0 && !ship->IsInAtmosphere()))
				roll = 0;
		}
		break;

	case FLY_Inverted:
		{
			// Turn toward destination location.
			// Fly inverted.
			roll += PI;
			if(roll > PI)
				roll -= PI*2;
			pitch = -pitch;

			// If we're too close, we don't turn in the atmosphere. Instead, we move away, turn, then come back. This strategy works
			// better when we're trying to gain a little altitude.
			double coursePitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			if(coursePitch > 89*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No yaw when more than 80 degrees of pitch up required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				yaw = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				yaw = courseLoc.ArcTangent_2D() - PI/2;
				if(yaw < -PI)
					yaw += PI*2;
			}

			// Don't make pitch changes too close to the landing segment or it messes with our vertical landing logic.
			if(!ship->IsInAtmosphere() || segDist2D > closeness)
			{
				// When ship is in space we can pitch all we want.
				// Make pitch changes a little ways away from the landing segment.
				// Pitch the ship to the destination location, or climb if we're too low.
				// Current calculated 'pitch' value would level the ship to the ground.
				// Note: Pitching level to the ground results in a climb on spherical bodies (non ringworlds).
				//       On ringworlds, we're not worried about clipping the atmosphere since it curves away from almost every path
				//       we take.
				if(ship->IsInAtmosphere() && (ship->AltitudeAGL() < terrainClearance || ship->AltitudeMSL() < terrainClearance))
					pitch += 10*DEG2RAD;	// Too close to the ground or sea level to use this code. Do you need VTOL?
				else
				{
					// Adjust pitch.
					bool pitchIsAway = coursePitch < pitch;

					if(altitude > targetAlt + closeness || pitchIsAway)
					{
						// Current altitude is above the max or the desired pitch is a pitch up.
						// Any pitch is ok when the ship is high enough or climbing in space.
						if(ship->IsInAtmosphere())
						{
							// In atmosphere, we restrict pitch-up or we'll fall backwards on our tail when going slowly.
							// Note: We take off VTOL. This code is for altitude adjustments to the final landing fix.
							double finalPitch = coursePitch - pitch;	// Pitch off of horizon
							if(finalPitch > 10*DEG2RAD)
								pitch += 10*DEG2RAD;	// Limit pitch-up.
							else
								pitch = coursePitch;	// No limit to pitch-down.
						}
						else
							pitch = coursePitch;		// in space we can take any pitch change
					}
					else if(altitude > targetAlt - closeness)
					{
						if(glworld)
						{
							// Not a ringworld and altitude is above the minimum.
							// Slight descent is needed to maintain altitude around globe; level causes climb.
							float distTraveled = speedF / ThoughtSpeed();	// Dist I will probably travel in the next think cycle.
							float orbitDia = ship->CGAltitude()*2;			// Diameter of circle at my altitude from center.
							float orbitCirc = PIf * orbitDia;				// Circumference of orbit at my altitude from center.
							float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Ang around planet ship will travel in one cycle.
							float baseAngle = (PIf - orbitAngle)/2;		// Base angle of isosceles tri of center, cur & next locs.
							float pitchAdj = PIf/2 - baseAngle;			// Pitch adjustment to end at same altitude as start.
							pitch += pitchAdj;
						}
					}
				}
			}

			// Reduce speed limit when near the landing/hovering segment.
			// This makes final alignment more precise.
			if(ship->IsInAtmosphere() && segDist2D < 100*FeetPerMeter() && altitude < targetAlt + closeness)
				speedLimit /= 2;

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(yaw) > 3*DEG2RAD || fabs(pitch) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix. Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch < 0)
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								// Max throttle is determined by hull streamlining and current maneuver acceleration.
								// Max throttle is also imposed when current planet is the destination planet.
								int maxThrottle;
								if(planet->HasAtmosphere() && (ship->IsInAtmosphere() || ship->Parent() == planet))
									maxThrottle = ship->MaxAtmosphereThrottle();	// Max throttle for atmosphere movement/entry.
								else
									maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when pitch is nonzero because pitch is based on the up vector. Yaw is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || (pitch == 0 && !ship->IsInAtmosphere()))
				roll = 0;
		}
		break;

	case FLY_LeftSide:
		{
			// Fly with left side toward the gravity well.
			// Turn toward destination location.
			yaw = -pitch;	// Yaw angle to be level, once flying side-on.
			roll -= PI/2;

			// If we're too close, we don't turn in the atmosphere. Instead, we move away, turn, then come back. This strategy works
			// better when we're trying to gain a little altitude.
			double courseYaw = courseLoc.ArcTangent_2D() - PI/2;	// Yaw needed to point directly at target. D:
			if(courseYaw < -PI)
				courseYaw += PI*2;
			if(courseYaw > 89*DEG2RAD && courseYaw < 91*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No pitch when more than 80 degrees of yaw down required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				pitch = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				pitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			}

			// Don't make pitch changes too close to the landing segment or it messes with our vertical landing logic.
			if(!ship->IsInAtmosphere() || segDist2D > closeness)
			{
				// When ship is in space we can yaw all we want.
				// Make yaw changes a little ways away from the landing segment.
				// Yaw the ship to the destination location, or climb if we're too low.
				// Current calculated 'yaw' value would level the ship to the ground.
				// Note: Yawing level to the ground results in a climb on spherical bodies (non ringworlds).
				//       On ringworlds, we're not worried about clipping the atmosphere since it curves away from almost every path
				//       we take.
				if(ship->IsInAtmosphere() && (ship->AltitudeAGL() < terrainClearance || ship->AltitudeMSL() < terrainClearance))
					yaw += -10*DEG2RAD;	// Too close to the ground or sea level to use this code for maneuvering. Do you need VTOL?
				else
				{
					// Adjust yaw.
					bool yawIsAway = courseYaw < yaw;	// Less yaw is a right turn away from the planet on the left.

					if(altitude > targetAlt + closeness || yawIsAway)
					{
						// Current altitude is above the max or the desired yaw is a yaw away from the planet.
						// Any yaw is ok when the ship is high enough or climbing in space.
						/*
						if(ship->IsInAtmosphere())
						{
							// In atmosphere, we restrict yaw-away or we'll fall backwards on our tail when going slowly.
							// Note: We take off VTOL. This code is for altitude adjustments to the final landing fix.
							double finalYaw = courseYaw + yaw;	// Yaw off of horizon
							if(finalYaw < -10*DEG2RAD)
								yaw += -10*DEG2RAD;	// Limit yaw-away.
							else
								yaw = courseYaw;	// No limit to yaw-to.
						}
						else
							*/
							yaw = courseYaw;		// in space we can take any yaw change
					}
					else if(altitude > targetAlt - closeness)
					{
						if(glworld)
						{
							// Not a ringworld and altitude is above the minimum.
							// Slight descent is needed to maintain altitude around globe; level causes climb.
							float distTraveled = speedF / ThoughtSpeed();	// Dist I will probably travel in the next think cycle.
							float orbitDia = ship->CGAltitude()*2;			// Diameter of circle at my altitude from center.
							float orbitCirc = PIf * orbitDia;				// Circumference of orbit at my altitude from center.
							float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Ang around planet ship will travel in one cycle.
							float baseAngle = (PIf - orbitAngle)/2;		// Base angle of isosceles tri of center, cur & next locs.
							float yawAdj = PIf/2 - baseAngle;				// Yaw adjustment to end at same altitude as start.
							yaw += yawAdj;
						}
					}
				}
			}

			// Reduce speed limit when near the landing/hovering segment.
			// This makes final alignment more precise.
			if(ship->IsInAtmosphere() && segDist2D < 100*FeetPerMeter() && altitude < targetAlt + closeness)
				speedLimit /= 2;

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(pitch) > 3*DEG2RAD || fabs(yaw) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix. Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch > 0 && !ship->IsInAtmosphere())
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								// Max throttle is determined by hull streamlining and current maneuver acceleration.
								// Max throttle is also imposed when current planet is the destination planet.
								int maxThrottle;
								if(planet->HasAtmosphere() && (ship->IsInAtmosphere() || ship->Parent() == planet))
									maxThrottle = ship->MaxAtmosphereThrottle();	// Max throttle for atmosphere movement/entry.
								else
									maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when yaw is nonzero because yaw is based on the up vector. Pitch is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || (yaw == 0 && !ship->IsInAtmosphere()))
				roll = 0;
		}
		break;

	case FLY_RightSide:
		{
			// Fly with right side toward the gravity well.
			// Turn toward destination location.
			yaw = pitch;	// Yaw angle to be level, once flying side-on.
			roll += PI/2;

			// If we're too close, we don't turn in the atmosphere. Instead, we move away, turn, then come back. This strategy works
			// better when we're trying to gain a little altitude.
			double courseYaw = courseLoc.ArcTangent_2D() - PI/2;	// Yaw needed to point directly at target. D:
			if(courseYaw < -PI)
				courseYaw += PI*2;
			if(courseYaw < -89*DEG2RAD && courseYaw > -91*DEG2RAD && fabs(roll) < 3*DEG2RAD)
			{
				// No pitch when more than 80 degrees of yaw down required.
				// This keeps me from spinning in place in orbit trying to go to a target exactly on the opposite side.
				pitch = 0;
			}
			else
			{
				// Pitch requirement is not great so it's ok to yaw.
				pitch = courseLoc.PolarIncline();				// Pitch angle to target unit.
			}

			// Don't make pitch changes too close to the landing segment or it messes with our vertical landing logic.
			if(!ship->IsInAtmosphere() || segDist2D > closeness)
			{
				// When ship is in space we can yaw all we want.
				// Make yaw changes a little ways away from the landing segment.
				// Yaw the ship to the destination location, or climb if we're too low.
				// Current calculated 'yaw' value would level the ship to the ground.
				// Note: Yawing level to the ground results in a climb on spherical bodies (non ringworlds).
				//       On ringworlds, we're not worried about clipping the atmosphere since it curves away from almost every path
				//       we take.
				if(ship->IsInAtmosphere() && (ship->AltitudeAGL() < terrainClearance || ship->AltitudeMSL() < terrainClearance))
					yaw += 10*DEG2RAD;	// Too close to the ground or sea level to use this code for maneuvering. Do you need VTOL?
				else
				{
					// Adjust yaw.
					bool yawIsAway = courseYaw > yaw;	// More yaw is a left turn away from the planet on the right.

					if(altitude > targetAlt + closeness || yawIsAway)
					{
						// Current altitude is above the max or the desired yaw is a yaw away from the planet.
						// Any yaw is ok when the ship is high enough or climbing in space.
						/*
						if(ship->IsInAtmosphere())
						{
							// In atmosphere, we restrict yaw-away or we'll fall backwards on our tail when going slowly.
							// Note: We take off VTOL. This code is for altitude adjustments to the final landing fix.
							double finalYaw = courseYaw + yaw;	// Yaw off of horizon
							if(finalYaw > 10*DEG2RAD)
								yaw += 10*DEG2RAD;	// Limit yaw-away.
							else
								yaw = courseYaw;	// No limit to yaw-to.
						}
						else
						*/
							yaw = courseYaw;		// in space we can take any yaw change
					}
					else if(altitude > targetAlt - closeness)
					{
						if(glworld)
						{
							// Not a ringworld and altitude is above the minimum.
							// Slight descent is needed to maintain altitude around globe; level causes climb.
							float distTraveled = speedF / ThoughtSpeed();	// Dist I will probably travel in the next think cycle.
							float orbitDia = ship->CGAltitude()*2;			// Diameter of circle at my altitude from center.
							float orbitCirc = PIf * orbitDia;				// Circumference of orbit at my altitude from center.
							float orbitAngle = distTraveled/orbitCirc * PIf*2;	// Ang around planet ship will travel in one cycle.
							float baseAngle = (PIf - orbitAngle)/2;		// Base angle of isosceles tri of center, cur & next locs.
							float yawAdj = PIf/2 - baseAngle;				// Yaw adjustment to end at same altitude as start.
							yaw -= yawAdj;
						}
					}
				}
			}

			// Reduce speed limit when near the landing/hovering segment.
			// This makes final alignment more precise.
			if(ship->IsInAtmosphere() && segDist2D < 100*FeetPerMeter() && altitude < targetAlt + closeness)
				speedLimit /= 2;

			// Prevent unwanted lateral or vertical movement.
			bool brakeLV = speedL || speedV || ship->InBlackHoleGravity();
			DoShipBrakeState(ship, false, brakeLV);

			// Change speed.
			// Observe speed limit so we don't overstress the ship.
			// Stop when we need to turn a lot so we don't circle the target.
			if(speed > speedLimit || fabs(pitch) > 3*DEG2RAD || fabs(yaw) > 20*DEG2RAD)
			{
				DoShipKillThrottles(ship);
				DoShipBrakeState(ship, true, brakeLV);
			}
			else
			{
				// Not going too fast.
				// When trying to align with the landing segment, measure distance to the segment, not exactly to the final approach
				// fix. Otherwise, our speed calc overshoots the line a lot when our altitude isn't quite right.
				double distToDest = courseLoc.Length();
				if(distToDest < closeness)
				{
					// Close enough to the destination.
					DoShipKillThrottles(ship);
					DoShipBrakeState(ship, true, brakeLV);
				}
				else
				{
					// Not close enough to the location yet.
					if(speedF == 0 || IsFuzzyZero(speed))
					{
						// Start moving.
						DoShipThrottleState(ship, 1);
					}
					else
					{
						double ete = (distToDest - closeness) / speed;	// sec to arrive at dest, a bit short to allow coasting
						double stopTime = speed / accel;				// sec to stop at destination
						if(stopTime > ete)
						{
							// Ship is moving too fast. Reduce speed to stop right at the destination.
							DoShipKillThrottles(ship);
							DoShipBrakeState(ship, true, brakeLV);

							// Avoid the target while going too fast.
							if(stopTime > ete*2. && !ship->IsAnyFTLDriveHot())
							{
								// Turn 90 degrees away from our course.
								pitch = courseLoc.XRotation();			// Pitch directly toward the target.
								if(pitch > 0 && !ship->IsInAtmosphere())
								{
									pitch -= PIf/2;						// Pitch down away from the target.
									roll = courseLoc.YRotation();		// Roll the top toward the target.
								}
								else
								{
									pitch += PIf/2;						// Pitch up away from the target.
									roll = courseLoc.YRotation() + PI;	// Roll the belly toward the target.
									if(roll > PI)
										roll -= PI*2;
								}
								yaw = 0;
							}
						}
						else
						{
							// Accelerate toward the skew flip.
							if(ete > 3)
							{
								// Select a throttle setting that won't accelerate past the limit in less than one thought cycle.
								// Max throttle is determined by hull streamlining and current maneuver acceleration.
								// Max throttle is also imposed when current planet is the destination planet.
								int maxThrottle;
								if(planet->HasAtmosphere() && (ship->IsInAtmosphere() || ship->Parent() == planet))
									maxThrottle = ship->MaxAtmosphereThrottle();	// Max throttle for atmosphere movement/entry.
								else
									maxThrottle = 10;

								if(speed + accel < speedLimit)
								{
									// Maximum acceleration won't exceed the speed limit in one second.
									// Accelerate to maximum speed before the skew flip.
									DoShipThrottleState(ship, maxThrottle);
								}
								else
								{
									// Max acceleration for one second will exceed the speed limit. Select a lower throttle setting.
									const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};
									int throttle = maxThrottle;		// Start at max safe throttle setting.
									for(; throttle > 1; throttle--)
										if(speed + accel * engineThrust[throttle] <= speedLimit)
											break;
									DoShipThrottleState(ship, throttle);
								}
							}
							else
								DoShipKillThrottles(ship);
						}
					}
				}
			}

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			if(fabs(yaw) < DEG2RAD)
				yaw = 0;

			// We must roll when yaw is nonzero because yaw is based on the up vector. Pitch is not affected by the up vector.
			if(fabs(roll) < DEG2RAD || (yaw == 0 && !ship->IsInAtmosphere()))
				roll = 0;
		}
		break;
	}

	DoShipRollPitchYaw(ship, roll, pitch, yaw);
}

void DaSentient::PilotVerticalLanding(DaVessel* ship)
{
	ResetOnApproach();										// Reset for atmospheric approach.

	Q_ASSERT(ship->IsInAtmosphere());

	if(!ship->IsPowerPlantOn())
	{
		// Turn on the power plant if it's not already on.
		DoShipPowerPlantState(ship, true);
		return;
	}

	if(ship->GravityDriveState() != DaVessel::GD_On)
	{
		// Turn on the gravity drive if it's not already on.
		DoShipGravityDriveState(ship, DaVessel::GD_On);
		DoShipKillThrottles(ship);
		return;
	}

	// Insure the wheels are down. Does nothing if they are already down or if the ship does not have retractable gear.
	DoShipLandingGear(ship, true);

	// Perform maneuvers relative to the surface of the planet.
	double speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude;
	ship->FlightInstruments(ship->DesignCenterOffset(), 1, speed, speedF, speedL, speedV, altitude, pitch, roll, heading,
			longitude, latitude);

	if(ship->LandingOrientation() == LORI_Tail)
	{
		if(fabs(roll) < DEG2RAD)
			roll = 0;

		pitch += PI/2;	// We want a resulting pitch of 90 degrees.
		if(fabs(pitch) < DEG2RAD)
			pitch = 0;

		DoShipRollPitchYaw(ship, roll, pitch, 0);

		double accelF = ship->ManeuverAcceleration();
		if(accelF <= 0)
			return;

		// Stop lateral movement during VTOL.
		bool brake = fabs(speedL) > 1 || fabs(speedV) > 1;
		DoShipBrakeState(ship, false, brake);

		// Use thrust to land gently in the atmosphere.
		DoShipThrottleState(ship, 0);
		int thrustF = ship->ThrustLongitudinal();
		int maxThrottle = ship->MaxAtmosphereThrottle();
		if(IsFuzzyZero(speedF))
		{
			// Start moving downward.
			// Do this in two increments if possible due to control decay.
			// Accelerate downwards.
			thrustF -= 2;
			if(thrustF <= -maxThrottle)
				thrustF = -maxThrottle;
			DoShipThrusterLateralState(ship, 0);
			DoShipThrusterLongitudinalState(ship, thrustF);
			DoShipThrusterVerticalState(ship, 0);
		}
		else
		{
			DoShipThrusterLateralState(ship, 0);
			DoShipThrusterVerticalState(ship, 0);

			// Impose a speed limit so we don't overstress the ship.
			double altBelly = ship->AltitudeAGL() - ship->GroundHeightAboveCG();
			if(altBelly < 100*FeetPerMeter() && speedF < -7.5*FeetPerMeter())
				DoShipBrakeState(ship, true, brake);	// Ship is descending too fast.
			else if(ship->ThrustLongitudinal() > -maxThrottle)
			{
				// Ship is not using too much negative vertical throttle.
				// Calculate descent to stop at ground level.
				double ete = altBelly / -speedF;	// sec to arrive at ground level
				double stopTime = -speedF / accelF;				// sec to stop at altitude
				if(stopTime > ete)
					DoShipBrakeState(ship, brake, true);	// Hold the lateral/vertical brakes.
				// Ship is descending too fast.
				else
				{
					// Do this in two increments if possible due to control decay.
					// Accelerate downwards.
					thrustF -= 2;
					if(thrustF < -maxThrottle)
						thrustF = -maxThrottle;
					DoShipThrusterLongitudinalState(ship, thrustF);
				}
			}
			else if(ship->ThrustLongitudinal() < -maxThrottle ||
					speed > ship->MaxAtmosphereSpeed(ship->FindParentPlanetInEnv()) * .9f)
				DoShipBrakeState(ship, true, brake);	// Ship is using too much throttle.
		}
	}
	else
	{
		if(fabs(roll) < DEG2RAD)
			roll = 0;

		if(fabs(pitch) < DEG2RAD)
			pitch = 0;

		DoShipRollPitchYaw(ship, roll, pitch, 0);

		double accelV = ship->ManeuverAccelerationV();
		if(accelV <= 0)
			return;

		// Stop forward/backward movement during VTOL.
		bool brake = fabs(speedF) > 1;
		DoShipBrakeState(ship, brake, false);

		// Use vertical thrust to land gently in the atmosphere.
		DoShipThrottleState(ship, 0);
		int thrustV = ship->ThrustVertical();
		int maxThrottle = ship->MaxAtmosphereThrottle();
		if(IsFuzzyZero(speedV))
		{
			// Start moving downward.
			// Do this in two increments if possible due to control decay.
			// Accelerate downwards.
			thrustV -= 2;
			if(thrustV <= -maxThrottle)
				thrustV = -maxThrottle;
			DoShipThrusterLateralState(ship, 0);
			DoShipThrusterLongitudinalState(ship, 0);
			DoShipThrusterVerticalState(ship, thrustV);
		}
		else
		{
			DoShipThrusterLateralState(ship, 0);
			DoShipThrusterLongitudinalState(ship, 0);

			// Impose a speed limit so we don't overstress the ship.
			double altBelly = ship->AltitudeAGL() - ship->GroundHeightAboveCG();
			if(altBelly < 100*FeetPerMeter() && speedV < -7.5*FeetPerMeter())
				DoShipBrakeState(ship, brake, true);	// Ship is descending too fast.
			else if(ship->ThrustVertical() > -maxThrottle)
			{
				// Ship is not using too much negative vertical throttle.
				// Calculate descent to stop at ground level.
				double ete = altBelly / -speedV;	// sec to arrive at ground level
				double stopTime = -speedV / accelV;				// sec to stop at altitude
				if(stopTime > ete)
					DoShipBrakeState(ship, brake, true);	// Hold the lateral/vertical brakes.
				// Ship is descending too fast.
				else
				{
					// Do this in two increments if possible due to control decay.
					// Accelerate downwards.
					thrustV -= 2;
					if(thrustV < -maxThrottle)
						thrustV = -maxThrottle;
					DoShipThrusterVerticalState(ship, thrustV);
				}
			}
			else if(ship->ThrustVertical() < -maxThrottle ||
					speed > ship->MaxAtmosphereSpeed(ship->FindParentPlanetInEnv()) * .9f)
				DoShipBrakeState(ship, brake, true);	// Ship is using too much vertical throttle.
		}
	}
}

void DaSentient::PilotVerticalTakeOff(DaVessel* ship)
{
	ResetOnApproach();										// Reset for atmospheric approach.
	if(!ship->IsInAtmosphere())
		return;

	if(!ship->IsPowerPlantOn())
	{
		// Turn on the power plant if it's not already on.
		DoShipPowerPlantState(ship, true);
		return;
	}

	if(ship->GravityDriveState() != DaVessel::GD_On)
	{
		// Turn on the gravity drive if it's not already on.
		DoShipGravityDriveState(ship, DaVessel::GD_On);
		DoShipKillThrottles(ship);
		return;
	}

	// Insure the wheels are up. Does nothing if they are already up or if the ship does not have retractable gear.
	if(!ship->IsOnGround())
		DoShipLandingGear(ship, false);

	// Perform maneuvers relative to the surface of the planet.
	double speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude;
	ship->FlightInstruments(ship->DesignCenterOffset(), 1, speed, speedF, speedL, speedV, altitude, pitch, roll, heading,
			longitude, latitude);

	if(ship->LandingOrientation() == LORI_Tail)
	{
		double yaw;
		if(ship->AltitudeAGL() < 1)
		{
			pitch = 0;
			yaw = 0;
		}
		else
		{
			// Calculate pitch needed based on gravity normal.
			AuPoint gN = ship->GravityNormal();
			pitch = gN.XRotation();
			if(pitch < -PI)
				pitch += PI*2;
			if(fabs(pitch) < DEG2RAD)
				pitch = 0;

			// Calculate yaw needed based on gravity normal.
			yaw = gN.ArcTangent_2D() - PI/2;
			if(yaw < -PI)
				yaw += PI*2;
			if(fabs(yaw) < DEG2RAD)
				yaw = 0;
			//qDebug() << "DaSentient::PilotVerticalTakeOff" << *this << "gN" << gN << "pitch" << (pitch*RAD2DEG)
				//<< "yaw" << (yaw*RAD2DEG);
		}

		DoShipRollPitchYaw(ship, 0, pitch, yaw);

		// Stop forward/backward movement during VTOL.
		DoShipThrottleState(ship, 0);
		DoShipThrusterLateralState(ship, 0);
		DoShipThrusterVerticalState(ship, 0);
		bool brake = false;
		bool brakeLV = fabs(speedL) > 1 || fabs(speedV) > 1;

		// Use thrust to leave the atmosphere.
		// Impose a speed limit so we don't overstress the ship.
		int maxThrottle = ship->MaxAtmosphereThrottle();
		if(ship->ThrustLongitudinal() < maxThrottle)
		{
			// Do this in two increments if possible due to control decay. Otherwise a dim-witted pilot can barely take off.
			// Accelerate upwards.
			int thrustF = ship->ThrustLongitudinal();
			thrustF += 2;
			if(thrustF > maxThrottle)
				thrustF = maxThrottle;
			DoShipThrusterLongitudinalState(ship, thrustF);
		}
		else if(ship->ThrustLongitudinal() > maxThrottle || speed > ship->MaxAtmosphereSpeed(ship->FindParentPlanetInEnv()) * .9f)
		{
			DoShipThrusterLongitudinalState(ship, 0);
			brake = true;	// Don't go too fast in atmosphere.
		}

		DoShipBrakeState(ship, brake, brakeLV);
	}
	else
	{
		if(ship->AltitudeAGL() < 1)
		{
			pitch = 0;
			roll = 0;
		}
		else
		{
			if(fabs(roll) < DEG2RAD)
				roll = 0;

			if(fabs(pitch) < DEG2RAD)
				pitch = 0;
		}

		DoShipRollPitchYaw(ship, roll, pitch, 0);

		// Stop forward/backward movement during VTOL.
		DoShipThrottleState(ship, 0);
		DoShipThrusterLongitudinalState(ship, 0);
		DoShipThrusterLateralState(ship, 0);
		bool brake = fabs(speedF) > 1;
		bool brakeLV = false;

		// Use vertical thrust to leave the atmosphere.
		// Impose a speed limit so we don't overstress the ship.
		int maxThrottle = ship->MaxAtmosphereThrottle();
		if(ship->ThrustVertical() < maxThrottle)
		{
			// Do this in two increments if possible due to control decay. Otherwise a dim-witted pilot can barely take off.
			// Accelerate upwards.
			int thrustV = ship->ThrustVertical();
			thrustV += 2;
			if(thrustV > maxThrottle)
				thrustV = maxThrottle;
			DoShipThrusterVerticalState(ship, thrustV);
		}
		else if(ship->ThrustVertical() > maxThrottle || speed > ship->MaxAtmosphereSpeed(ship->FindParentPlanetInEnv()) * .9f)
		{
			brakeLV = true;	// Don't go too fast in atmosphere.
			DoShipThrusterVerticalState(ship, 0);
		}

		DoShipBrakeState(ship, brake, brakeLV);
	}
}

void DaSentient::ReportEnemySensorContact(DaVessel* ship, const AuSensorBlipList& sensorBlips, float sensorRange)
{
	if(!ship->IsSensorOn())
		return;

	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
	if(!system)
		return;

	// Report enemy sensor contacts to the first officer for logging, if there is one.
	DaSentient* officer = ship->FindOfficer();
	bool humanOfficer = ship->HasHumanOfficer();
	if(!officer && !humanOfficer)
		return;				// Don't bother doing this work if there is nobody to notify.

	bool anomaly = false;
	bool asteroid = false;
	bool beacon = false;
	QString beaconMsg;
	int enemyBlips = 0;		// Count of all enemy blips on sensors, including cities and capitols.
	int enemyCities = 0;	// Count of enemy cities on sensors.
	QString enemyCityName;	// Name of first enemy city spotted.
	int enemyUnits = 0;		// Count of enemy units, not including cities or capitols.
	QString enemyUnitName;	// Name of first enemy unit spotted.
	AuPoint shipSysLoc = ship->EnvLocation();
	float sensorRange2 = sensorRange*sensorRange;
	float closestRange2 = sensorRange2;
	AuPoint closestSysVec;
	for(AuSensorBlipList::const_iterator bi = sensorBlips.begin(); bi != sensorBlips.end(); bi++)
	{
		// These are sorted so AI targets are ahead of all other blips.
		if(!(*bi)->IsAITarget())
			break;	// Stop searching when there are no more potential targets in the sensor blips array.
		if(!(*bi)->IsSensorConsoleBlip())
			continue;
		double range2 = (*bi)->Range2(shipSysLoc);	// Squared range to the blip.
		if(range2 <= sensorRange2)
		{
			AuEventLogEntry::TYPE eventType = (*bi)->OfficerEventLogType();
			switch(eventType)
			{
			case AuEventLogEntry::Anomaly:
				if(officer)
					officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

				anomaly = true;
				break;

			case AuEventLogEntry::AsteroidDetected:
				if(officer)
					officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true), (*bi)->AtomId());

				asteroid = true;
				break;

			case AuEventLogEntry::Beacon:
				if(officer)
					officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

				beacon = true;
				{
					const AuBlipBeacon* beac = dynamic_cast<const AuBlipBeacon*>(*bi);
					if(beac)
						beaconMsg = beac->Message();
				}
				break;

			case AuEventLogEntry::EnemyCapital:
				if(officer)
				{
					officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

					/*
					// Send an Alert message.
					if(ship->LastAlert().ElapsedSec() > 5*60)
					{
						QString text = tr("Enemy capital city detected by %1. Tracking %2.")
							.arg(ship->NameWithEmpire())
							.arg((*bi)->HtmlDesc(system, false));
						ship->SendAlert(text);
					}
					*/
				}
				if(enemyCityName.isEmpty())
					enemyCityName = (*bi)->ScreenName();
				enemyCities++;
				enemyBlips++;

				if(range2 < closestRange2)
				{
					closestRange2 = range2;
					closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
				}
				break;
			case AuEventLogEntry::EnemyCity:
				if(officer)
				{
					officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

					/*
					// Send an Alert message.
					if(ship->LastAlert().ElapsedSec() > 5*60)
					{
						QString text = tr("Enemy city detected by %1. Tracking %2.")
							.arg(ship->NameWithEmpire())
							.arg((*bi)->HtmlDesc(system, false));
						ship->SendAlert(text);
					}
					*/
				}
				if(enemyCityName.isEmpty())
					enemyCityName = (*bi)->ScreenName();
				enemyCities++;
				enemyBlips++;

				if(range2 < closestRange2)
				{
					closestRange2 = range2;
					closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
				}
				break;
			case AuEventLogEntry::EnemySpaceship:
			case AuEventLogEntry::EnemySpaceshipS:
				// Ignore "nu" enemy spacecraft.
				if(!(*bi)->IsNewSpacecraft())
				{
					if(officer)
					{
						officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

						// Send an Alert message.
						if(ship->LastAlert().ElapsedSec() > 5*60)
						{
							QString text = tr("Enemy spaceship detected by %1. Tracking %2.")
								.arg(ship->NameWithEmpire())
								.arg((*bi)->HtmlDesc(system, false));
							ship->SendAlert(text);
						}
					}
					if(enemyUnitName.isEmpty())
						enemyUnitName = (*bi)->ScreenName();
					enemyUnits++;
					enemyBlips++;

					if(range2 < closestRange2)
					{
						closestRange2 = range2;
						closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
					}
				}
				break;
			case AuEventLogEntry::EnemyStarship:
			case AuEventLogEntry::EnemyStarshipS:
				// Ignore "nu" enemy spacecraft.
				if(!(*bi)->IsNewSpacecraft())
				{
					if(officer)
					{
						officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

						// Send an Alert message.
						if(ship->LastAlert().ElapsedSec() > 5*60)
						{
							QString text = tr("Enemy starship detected by %1. Tracking %2.")
								.arg(ship->NameWithEmpire())
								.arg((*bi)->HtmlDesc(system, false));
							ship->SendAlert(text);
						}
					}
					if(enemyUnitName.isEmpty())
						enemyUnitName = (*bi)->ScreenName();
					enemyUnits++;
					enemyBlips++;

					if(range2 < closestRange2)
					{
						closestRange2 = range2;
						closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
					}
				}
				break;
			case AuEventLogEntry::EnemyStation:
			case AuEventLogEntry::EnemyStationS:
				// Ignore "nu" enemy spacecraft.
				if(!(*bi)->IsNewSpacecraft())
				{
					if(officer)
					{
						officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

						// Send an Alert message.
						if(ship->LastAlert().ElapsedSec() > 5*60)
						{
							QString text = tr("Enemy station detected by %1. Tracking %2.")
								.arg(ship->NameWithEmpire())
								.arg((*bi)->HtmlDesc(system, false));
							ship->SendAlert(text);
						}
					}
					if(enemyUnitName.isEmpty())
						enemyUnitName = (*bi)->ScreenName();
					enemyUnits++;
					enemyBlips++;

					if(range2 < closestRange2)
					{
						closestRange2 = range2;
						closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
					}
				}
				break;
			case AuEventLogEntry::EnemyAircraft:
				// Ignore "nu" enemy vehicles.
				if(!(*bi)->IsNewVehicle())
				{
					if(officer)
					{
						officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

						/*
						// Send an Alert message.
						if(ship->LastAlert().ElapsedSec() > 5*60)
						{
							QString text = tr("Enemy aircraft detected by %1. Tracking %2.")
								.arg(ship->NameWithEmpire())
								.arg((*bi)->HtmlDesc(system, false));
							ship->SendAlert(text);
						}
						*/
					}
					if(enemyUnitName.isEmpty())
						enemyUnitName = (*bi)->ScreenName();
					enemyUnits++;
					enemyBlips++;

					if(range2 < closestRange2)
					{
						closestRange2 = range2;
						closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
					}
				}
				break;
			case AuEventLogEntry::EnemyBoat:
				// Ignore "nu" enemy vehicles.
				if(!(*bi)->IsNewVehicle())
				{
					if(officer)
					{
						officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

						/*
						// Send an Alert message.
						if(ship->LastAlert().ElapsedSec() > 5*60)
						{
							QString text = tr("Enemy watercraft detected by %1. Tracking %2.")
								.arg(ship->NameWithEmpire())
								.arg((*bi)->HtmlDesc(system, false));
							ship->SendAlert(text);
						}
						*/
					}
					if(enemyUnitName.isEmpty())
						enemyUnitName = (*bi)->ScreenName();
					enemyUnits++;
					enemyBlips++;

					if(range2 < closestRange2)
					{
						closestRange2 = range2;
						closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
					}
				}
				break;
			case AuEventLogEntry::EnemyVehicle:
				// Ignore "nu" enemy vehicles.
				if(!(*bi)->IsNewVehicle())
				{
					if(officer)
					{
						officer->LogEvent(eventType, (*bi)->HtmlDesc(system, true));

						/*
						// Send an Alert message.
						if(ship->LastAlert().ElapsedSec() > 5*60)
						{
							QString text = tr("Enemy ground vehicle detected by %1. Tracking %2.")
								.arg(ship->NameWithEmpire())
								.arg((*bi)->HtmlDesc(system, false));
							ship->SendAlert(text);
						}
						*/
					}
					if(enemyUnitName.isEmpty())
						enemyUnitName = (*bi)->ScreenName();
					enemyUnits++;
					enemyBlips++;

					if(range2 < closestRange2)
					{
						closestRange2 = range2;
						closestSysVec = (*bi)->EnvLocation() - shipSysLoc;
					}
				}
				break;
			default:
				break;
			}
		}
	}

	m_SentientLock.lockForWrite();
	int lastBlips = m_EnemyBlips;
	m_EnemyBlips = enemyBlips;
	m_SentientLock.unlock();
	if(humanOfficer && (enemyBlips != lastBlips || anomaly || beacon || asteroid))
	{
		if(enemyBlips != lastBlips)
		{
			double rangeM = round(sqrt(closestRange2) / FeetPerMeter());
			AuPoint vecToTarget = ship->EnvTransform().UnProjectVector(closestSysVec);
			double angXY = (vecToTarget.ZRotation() - PI/2)*RAD2DEG;	// Angle around Z, with 0 up +Y.
			if(angXY > 0)
				angXY -= 360;			// Clockwise around Z is a negative angle.
			angXY = fabs(angXY);		// Now positive angle are clockwise.
			angXY /= 30;
			int hourDir = (int)round(angXY);	// Round to nearest whole hour, 0 to 12.
			if(hourDir == 0)
				hourDir = 12;
			double inc = vecToTarget.PolarIncline();
			QString clockDir;
			if(inc > 30*DEG2RAD)
				clockDir = tr("%1 o'clock high").arg(hourDir);
			else if(inc < -30*DEG2RAD)
				clockDir = tr("%1 o'clock low").arg(hourDir);
			else
				clockDir = tr("%1 o'clock").arg(hourDir);

			// Announce change of enemy contact status on the crew channel.
			QString tracking;
			if(enemyCities == 1 && enemyUnits == 1)
				tracking = tr("Tracking enemy city %1 and enemy unit %2. Closest contact %L3m %4.")
					.arg(enemyCityName).arg(enemyUnitName).arg(rangeM, 0,'f',0).arg(clockDir);
			else if(enemyCities == 1 && enemyUnits)
				tracking = tr("Tracking enemy city %1 and %2 enemy units. Closest contact %L3m %4.")
					.arg(enemyCityName).arg(enemyUnits).arg(rangeM, 0,'f',0).arg(clockDir);
			else if(enemyCities && enemyUnits == 1)
				tracking = tr("Tracking %1 enemy cities and enemy unit %2. Closest contact %L3m %4.")
					.arg(enemyCities).arg(enemyUnitName).arg(rangeM, 0,'f',0).arg(clockDir);
			else if(enemyCities && enemyUnits)
				tracking = tr("Tracking %1 enemy cities and %2 enemy units. Closest contact %L3m %4.")
					.arg(enemyCities).arg(enemyUnits).arg(rangeM, 0,'f',0).arg(clockDir);
			else if(enemyCities == 1)
				tracking = tr("Tracking enemy city %1. Range %L3m %4.").arg(enemyCityName)
					.arg(rangeM, 0,'f',0).arg(clockDir);
			else if(enemyCities)
				tracking = tr("Tracking %1 enemy cities. Closest city %L3m %4.").arg(enemyCities)
					.arg(rangeM, 0,'f',0).arg(clockDir);
			else if(enemyUnits == 1)
				tracking = tr("Tracking enemy unit %1. Range is %L3m %4.").arg(enemyUnitName)
					.arg(rangeM, 0,'f',0).arg(clockDir);
			else if(enemyUnits)
				tracking = tr("Tracking %1 enemy units. Closest contact %L3m %4.").arg(enemyUnits)
					.arg(rangeM, 0,'f',0).arg(clockDir);
			else
				tracking = tr("Scope is clear.");

			QString text;
			if(enemyBlips > lastBlips)
			{
				// Number of enemy contacts increase.
				if(lastBlips > 0)
				{
					// We were already tracking blips.
					int increase = enemyBlips - lastBlips;
					if(increase == 1)
						text = tr("Another sensor contact!") + " " + tracking;
					else
						text = tr("%1 more sensor contacts!").arg(increase) + " " + tracking;
				}
				else
				{
					// This is initial contact.
					text = tr("Sensor contact!") + " " + tracking;
				}
			}
			else if(enemyBlips < lastBlips)
			{
				// Number of enemy contacts decreased.
				if(enemyBlips == 0)
				{
					if(lastBlips == 1)
						text = tr("Sensor contact lost.") + " " + tracking;
					else
						text = tr("%1 sensor contacts lost.").arg(lastBlips) + " " + tracking;
				}
				else
				{
					int decrease = lastBlips - enemyBlips;
					if(decrease == 1)
						text = tr("Sensor contact lost.") + " " + tracking;
					else
						text = tr("%1 sensor contacts lost.").arg(decrease) + " " + tracking;
				}
			}

			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
			SayNoRepeat(msg, AuVisage::Closed);
		}

		if(asteroid)
		{
			QString text = tr("Sir, sensors have detected a large asteroid. "
					"It is hazardous to space traffic and world ecosystems.");
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
			SayNoRepeat(msg, AuVisage::Grim);
		}

		if(beacon)
		{
			QString text;
			if(beaconMsg.isEmpty())
				text = tr("Sir, I am picking up a beacon on sensors.");
			else
				text = tr("Sir, I am picking up a beacon on sensors. Its automated broadcast says, '%1'").arg(beaconMsg);
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
			SayNoRepeat(msg, AuVisage::Closed);
		}

		if(anomaly)
		{
			QString anoms[] =
			{
				"",
				tr("I don't know what to make of it."),
				tr("It could be anything."),
				tr("It is probably nothing."),
				tr("I think the sensor array needs to be calibrated."),
				tr("You may want to take a look at this."),
				tr("I've had it! I didn't sign up for this crap."),
				tr("This is not something they teach you at the academy."),
				tr("What the heck am I supposed to do now?"),
				tr("The pilot is a butt head; I think he's messing with me."),
				tr("Oh wait, no, I lost it."),
				tr("Blast you deviant phosphors!"),
				tr("Wait, never mind. I must be seeing things; it's been a long shift."),
			};
			int anomcnt = sizeof(anoms)/sizeof(QString);

			QString text = tr("Sir, sensors are showing an anomaly. %1").arg(anoms[g_Au.Dice().Roll(1,anomcnt,-1)]);
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
			Say(msg, AuVisage::Grim);
		}
	}
}

void DaSentient::ReportEnemyWeaponContact(DaVessel* ship)
{
	// Report enemy weapon contacts to the first officer for logging, if there is one.
	DaSentient* officer = ship->FindOfficer();
	bool humanOfficer = ship->HasHumanOfficer();
	if(!officer && !humanOfficer)
		return;				// Don't bother doing this work if there is nobody to notify.

	const DaAtom* system = ship->FindEnvironmentAtom();
	if(!system)
		return;

	bool inEnemyRange = ship->IsInEnemyRange();
	if(inEnemyRange != InEnemyRange())
	{
		QString text;
		if(inEnemyRange)
		{
			// Number of enemy contacts increase.
			// This is initial contact because we only track one.
			text = tr("Enemy weapon lock-on detected!");
			if(officer)
				officer->LogEvent(AuEventLogEntry::EnemyWeaponRangeIn, system->Name() + ", " + text);
		}
		else
		{
			// Number of enemy contacts decreased.
			// This is lost contact because we only track one.
			text = tr("Enemy weapon lock signal lost.");
			if(officer)
				officer->LogEvent(AuEventLogEntry::EnemyWeaponRangeOut, system->Name() + ", " + text);
		}
		m_SentientLock.lockForWrite();
		m_InEnemyRange = inEnemyRange;
		m_SentientLock.unlock();

		if(humanOfficer)
		{
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
			Say(msg, AuVisage::Grim);
		}
	}
}

void DaSentient::AmendOrder(CITIZEN_ORDER order, const AuId& atom, const AuPoint& loc, quint16 stationId, const QString& orderText)
{
	// Append my nu current order.
	AuAIOrder aio(order, atom, loc, stationId, orderText);;
	m_ThinkLock.lockForWrite();
	m_AIOrders.append(aio);
	m_ThinkLock.unlock();

	// Reset path finding logic parameters.
	m_SentientLock.lockForWrite();
	m_AIRecentCollide = AuId(0);
	m_AIRecentParent = ParentId();
	ResetStrategyNoLock(true);
	m_SentientLock.unlock();

	// This person needs some time to think about their nu order.
	RestartThinkTime();
	UpdateThinkRate();			// Rate of thinking is based on current order.
}

void DaSentient::SetOrder(CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc, const QString& orderText)
{
	// Discard all current orders.
	m_ThinkLock.lockForWrite();
	m_AIOrders.clear();

	// Index 0 is always my fallback Stay order.
	m_AIOrders.append(AuAIOrder(ORDER_Stay, ParentId(), Location(), 0, ""));

	// Index 1 is my nu current order.
	if(order != ORDER_Stay)
		m_AIOrders.append(AuAIOrder(order, orderTarget, orderLoc, 0, orderText));
	m_ThinkLock.unlock();

	// Reset path finding logic parameters.
	m_SentientLock.lockForWrite();
	m_AIRecentCollide = AuId(0);
	m_AIRecentParent = ParentId();
	ResetStrategyNoLock(true);
	m_SentientLock.unlock();

	// This person needs some time to think about their nu order.
	RestartThinkTime();
	UpdateThinkRate();			// Rate of thinking is based on current order.
}

void DaSentient::TakeOrder(const AuCommPacket& msg, CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc,
		bool firstInGroup, const QString& orderText)
{
	Q_UNUSED(msg);
	Q_UNUSED(firstInGroup);
	// Set this as my current order. It will be processed during Tick from now on.
	SetOrder(order, orderTarget, orderLoc, orderText);

	// A stay order requires the dude to stop moving right away, otherwise it might take a while.
	if(Order() == ORDER_Stay)
	{
		// Make the dude's body stop moving right now.
		ResetStrategy(true);
		SetGeneralPose(StationaryPose());
	}
	else
		SetModified();
}

void DaSentient::TakeOrderAmendment(CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc, const QString& orderText)
{
	// Set this as my current order. It will be processed during Tick from now on.
	AmendOrder(order, orderTarget, orderLoc, 0, orderText);

	// Taking a stay order requires the dude to stop moving right away, otherwise it might take a while.
	if(Order() == ORDER_Stay)
	{
		// Make the dude's body stop moving right now.
		ResetStrategy(true);
		SetGeneralPose(StationaryPose());
	}
	else
		SetModified();
}

void DaSentient::TakePosture(const AuCommPacket& /*msg*/, CITIZEN_POSTURE posture, bool /*firstInGroup*/)
{
	// Set this as my current posture. It will be processed during Tick from now on.
	SetPosture(posture);

	// Changing to a defensive posture requires the dude to stop attacking right away, otherwise it might take a while.
	if(Order() == ORDER_AttackTarget &&
			(posture == POSTURE_DefendSelf || posture == POSTURE_DefendArea || posture == POSTURE_HoldFire))
		RequestOrder();	// So the dude will stop attacking.

	SetModified();
}

void DaSentient::RequestOrder()
{
	// Select my next order.
	m_ThinkLock.lockForWrite();
	if(m_AIOrders.count() > 1)
	{
		m_AIOrders.pop_back();			// Erase the last order so the previous-to-last order will become current.
		m_AIOrders.last().m_Time.Set();	// Restart order time when an old order becomes active again.
	}
	m_ThinkLock.unlock();

	// A stay order requires the dude to stop moving right away, otherwise it might take a while.
	if(Order() == ORDER_Stay)
		StayIdle();						// So the dude's body will stop moving right now.

	ClearRoute();						// Clear any existing route.
	UpdateThinkRate();					// Rate of thinking is based on current order.
	SetModified();
}

// Sets the think rate that corresponds to a certain order, unconditionally, as if carrying out the order, not idling or moving
// to where the order can be carried out.
// Citizens have a mind of their own. They're a bit dim-witted though otherwise they would put a huge drain on the server CPU.
// The time between thoughts is calculated using this citizen's id, causing some citizens to be faster or slower thinkers than
// others. The smarter citizens think more often. The urgency of their current order influences the speed of their thinking.
quint16 DaSentient::ThinkRateForPose(CITIZEN_ORDER forOrder) const
{
	if(forOrder == ORDER_BattleStation)
	{
		if(IsHelmStationPose())
		{
			// Helm requires special attention.
			const DaVessel* ship = FindParentVesselInEnv();
			if(ship && !ship->IsAtRest() && ship->CapacitorLevel() > 0)
			{
				// I need to think often enough to fly safely when the ship is moving.
				// Control decay on spacecraft is 175ms. I must think more often than that or brakes and turns will stutter.
				return Acuity()*10 + 80 - Rank()*10;	// Think again in 80-150ms, shorter with rank.
			}
		}
		else if(IsNavStationPose())
		{
			// Navigator requires extra attention when navigating to wormholes in a black hole system.
			const DaVessel* ship = FindParentVesselInEnv();
			if(ship && !ship->IsAtRest() && ship->CapacitorLevel() > 0 &&
					ship->MissionOrderType() == AuMissionOrder::GoToSystemViaWormholes)
			{
				const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
				if(system && system->StrongGravity())
					return Acuity()*30 + 550 - Rank()*50;		// Think again in 550-760ms.
			}
		}
		else if(IsTurretPose() && IsInSpacecraft() && AIAttackTime().elapsed() < 2000+Rank()*500)
		{
			// If I have attacked within a short time, longer than the rate of fire of any of the turrets, I can keep attacking
			// repeatedly. This gives the effect of taking time to aim but then opening up on the target.
			return Acuity()*25 + 300 - Rank()*20;	// Think again in 300-475ms, shorter with rank.
		}
	}

	switch(forOrder)
	{
	case ORDER_Sleep:
	case ORDER_SleepDayDream:
	case ORDER_SleepEat:
	case ORDER_SleepExercise:
	case ORDER_SleepGame:
	case ORDER_SleepLetter:
	case ORDER_SleepMovie:
	case ORDER_SleepMusic:
	case ORDER_SleepPine:
	case ORDER_SleepPhone:
	case ORDER_SleepRead:
	case ORDER_SleepTalk:
	case ORDER_SleepText:
	case ORDER_SleepTV:
	case ORDER_SleepWeb:
		return 10000;								// Think again in 10 seconds.
	case ORDER_Stay:
	case ORDER_DamageReport:
	case ORDER_SurveyScanPlanet:
	case ORDER_SurveyScanSystem:
	case ORDER_TacticalScan:
		return Acuity()*300 + 3000;	// Think again in 3000-5100ms.
	case ORDER_AttackTarget:
		return Acuity()*30 + 550 - Rank()*50;		// Think again in 550-760ms.
	case ORDER_ComeHereExactly:
	case ORDER_GoToTargetExactly:
		return Acuity()*30 + 150;		// Think again in 150-360ms.
	case ORDER_BattleStation:
	case ORDER_EnterSurgery:
	case ORDER_TakeCommand:
	case ORDER_ReleaseCommand:
		return Acuity()*100 + 3800 - Rank()*500;		// Think again in 3800-4500ms.
	case ORDER_Garrison:
		return Acuity()*200 + 1500;	// Think again in 1500-2900ms.
	case ORDER_CloseDoor:
	case ORDER_OpenDoor:
	case ORDER_GetIn:
		return Acuity()*50 + 300;		// Think again in 300-650ms.
	case ORDER_ComeHere:
	case ORDER_FollowMe:
	case ORDER_FollowTarget:
	case ORDER_GetOut:
	case ORDER_GoToRoom:
	case ORDER_GoToTarget:
	case ORDER_HengeCeremony:
	case ORDER_GetItem:
	case ORDER_DropRider:
	case ORDER_MountMe:
	case ORDER_MountTarget:
		return Acuity()*50 + 250;		// Think again in 250-600ms.
	}
	return 1000;
}

void DaSentient::DoAttackTarget()
{
	// This is a lot of work for the server. Don't do it if nobody is around to watch. No commander, no battle. Sorry.
	if(!IsInAnyScene())
		return;

	// Locate the target I am ordered to attack.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;		// I'm not in a solar system?

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	int elapsed = AIAttackTime().elapsed();
	DaAtom* target = system->FindIdRecursive(OrderTarget());
	if(!target || !target->IsSightTarget(this))
	{
		// My target is not in this solar system or I cannot sight on it.
		// AI orders in excess of 2 mean that I have more to do besides attack this target.
		// If I don't have anything else to to, I can wait a while before giving up.
		// Give up immediately if my target isn't found and I just attacked something within the last second.
		int orderCount = OrderCount();
		if(!orderCount || orderCount > 2 || OrderTime().ElapsedSec() > 60 || elapsed < 2000)
			RequestOrder();									// Give up and request my next order.
		else
			StayIdle();										// Wait for a way to get to my target.
		return;
	}

	float moveWithinRange = MOVE_NEAR;
	if(elapsed < 2000+Rank()*500)
	{
		// I have attacked a target within the past few seconds. Can I stay at this range?
		// What is the range of my weapon?
		AuDNABase::WEAPONLOC wLoc;
		AuCombatWeapon weapon(WM_Not);	// CID_Invalid if wpn is part of my body
		int ammoCur, ammoFull;
		BestWeaponAtRange(0, wLoc, weapon, ammoCur, ammoFull);
		if(Id().GetID()&1)
		{
			// 50% of citizens will move just within range of their weapon.
			moveWithinRange = weapon.m_Range;
		}
		else if(Id().GetID()&2)
		{
			// 25% will move to 1/2 the range of their weapon.
			moveWithinRange = weapon.m_Range / 2;
		}
		else
		{
			// The remainder move close to the target.
			if(weapon.m_Range < moveWithinRange)
				moveWithinRange = weapon.m_Range;
		}
		//if(Is(AuId(241530682)))
			//qDebug() << "DaSentient::DoAttackTarget" << Id() << Name() << "weapon.m_Range" << weapon.m_Range
				//<< "moveWithinRange" << moveWithinRange;
	}

	// Move closer to the designated target.
	//qDebug("%s moving within %g", (const char*)Name().toAscii(), moveWithinRange);
	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(moveWithinRange, true, orderTarget);
	if(result == 2)
	{
		// There is no path to the order target.
		//if(Is(AuId(241530682)))
			//qDebug() << "DaSentient::DoAttackTarget" << Id() << Name() << "no path to target, orders" << m_AIOrders.count();
		// AI orders in excess of 2 mean that I have more to do besides attack this target.
		// If I don't have anything else to to, I can wait a while before giving up.
		int orderCount = OrderCount();
		if(!orderCount || orderCount > 2 || OrderTime().ElapsedSec() > 60)
			RequestOrder();	// Give up and request my next order.
		else
			StayIdle();		// Wait for a way to get to my target.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		//g_Au.Log(QString("DaSentient::DoAttackTarget %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
		TakeANap();
	}
	else if(!result && OrderCount() > 2 && OrderTime().ElapsedSec() > 30)
	{
		// It's taking too long to catch a secondary target.
		//if(Is(AuId(241530682)))
			//qDebug() << "DaSentient::DoAttackTarget" << Id() << Name() << "giving up on secondary target" << m_AIOrders.count();
		RequestOrder();	// Give up and request my next order.
	}
	else if(IsReadyToShootAimedWeapon())
	{
		//if(Is(AuId(241530682)))
			//qDebug() << "DaSentient::DoAttackTarget" << Id() << Name() << "ready to shoot aimed weapon" << m_AIOrders.count();
		// I am moving or facing toward my target.
		// I am ready to shoot so enough time has passed since my last attack.
		bool didAttack = false;
		AuPoint sightedLoc;
		unsigned int sightedPart = 0;
		bool sightedShield = false;
		DaAtom* sightedTarget = SightTargetInSight(sightedLoc, sightedPart, sightedShield);
		if(sightedTarget && !sightedTarget->IsNoncombatant())
		{
			// There is a target in my line-of-sight.
			bool attackTarget = false;	// set true if I actually attack a target
			bool targetIsFriendlyOrDying = false;
			// Should I attack it?
			if(sightedTarget->Is(OrderTarget()))
			{
				// I am sighted on the atom I was ordered to attack.
				//if(Is(AuId(241530682)))
					//qDebug() << "DaSentient::DoAttackTarget" << Id() << Name() << "looking at my target";
				attackTarget = true;
				DaCreature* creature = dynamic_cast<DaCreature*>(sightedTarget);
				if(creature && creature->CurrentHitPoints() <= 0)
					targetIsFriendlyOrDying = true;		// Target is a dying creature.
				else
				{
					// Target is not a dying creature.
					DaUnit* unit = dynamic_cast<DaUnit*>(sightedTarget);
					if(unit)
					{
						// Target is a unit.
						POLITICAL_STANCE stance = PoliticalStanceMakeContact(unit);
						if(stance == POL_Empire || stance == POL_Friend || stance == POL_Vassal)
							targetIsFriendlyOrDying = true;		// Target is a friendly unit.
					}
				}
			}
			else
			{
				DaAnimal* animal = dynamic_cast<DaAnimal*>(sightedTarget);
				if(animal)
				{
					// I am looking at an animal.
					if(animal->IsBodyDyingOrDead())
						targetIsFriendlyOrDying = true;		// Target is a dying creature.
					else
					{
						if(animal->IsWild())
						{
							// Wild animals are threats when they do certain things.
							//if(Is(AuId(241530682)))
								//qDebug() << "DaSentient::DoAttackTarget" << Id() << Name() << "target under ground"
									//<< animal->IsUnderGround() << "AGL" << animal->AltitudeAGL();
							//if(animal->IsActivityThreatening())
							if(!animal->IsUnderGround())
								attackTarget = true;
						}
						else if(animal->EmpireId() != EmpireId() && !animal->IsCivilian())
						{
							// Animal is mounted and it belongs to a different empire.
							POLITICAL_STANCE stance = PoliticalStanceMakeContact(animal);
							if(stance == POL_Enemy)
								attackTarget = true;	// Mounted animal that belongs to the enemy.
						}
					}
				}
				else
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(sightedTarget);
					if(vehicle)
					{
						// Is the vehicle a threat.
						if(vehicle->EmpireId() != EmpireId() && vehicle->IsOccupied())
						{
							// Vehicle is occupied and it belongs to a different empire.
							POLITICAL_STANCE stance = PoliticalStanceMakeContact(vehicle);
							if(stance == POL_Enemy)
								attackTarget = true;	// Occupied vehicle that belongs to the enemy.
						}
					}
					else
					{
						DaVessel* ship = dynamic_cast<DaVessel*>(sightedTarget);
						if(ship)
						{
							if(!ship->IsNew() && ship->EmpireId() != EmpireId())
							{
								// Spacecraft is not nu and it belongs to a different empire.
								POLITICAL_STANCE stance = PoliticalStanceMakeContact(ship);
								if(stance == POL_Enemy)
									attackTarget = true;	// Spacecraft that belongs to the enemy.
							}
						}
						else
						{
							DaUnit* unit = dynamic_cast<DaUnit*>(sightedTarget);
							if(unit)
							{
								if(unit->EmpireId() != EmpireId())
								{
									// Unit belongs to a different empire.
									POLITICAL_STANCE stance = PoliticalStanceMakeContact(unit);
									if(stance == POL_Enemy)
										attackTarget = true;	// Unit that belongs to the enemy.
								}
							}
						}
					}
				}
			}

			if(attackTarget)
			{
				// It's ok to attack the target.
				// What is the range of my weapon?
				double targetRange = sightedLoc.Length();	// Range to target.
				AuDNABase::WEAPONLOC wLoc;
				AuCombatWeapon weapon(WM_Not);	// CID_Invalid if wpn is part of my body
				int ammoCur, ammoFull;
				bool useSpecWpn = BestWeaponAtRange(targetRange, wLoc, weapon, ammoCur, ammoFull);

				AuGearPos gearPos(CurrentItemPos());
				INVENTORYSUB useSub = IPS_Item;
				AMMOTYPE ammoType = AMMO_None;
				if(weapon.m_CID > CID_Invalid)
				{
					// Weapon is not part of my physical body.
					if(gearPos.IsGear())
					{
						const AuGear* gear = LockGearForRead();
						if(gear)
						{
							const AuItem* gearItem = gear->ItemAt(gearPos);
							if(gearItem)
							{
								if(gearItem->IsWeapon())
								{
									useSub = IPS_Weapon1;
									const AuItemWeaponAmmo* wammo = dynamic_cast<const AuItemWeaponAmmo*>(gearItem);
									if(wammo)
										ammoType = wammo->Ammo1Type();
								}
								else
									useSub = IPS_Item;
							}
						}
						UnlockGear();
					}
				}

				if(weapon.m_w1.m_Effect == WE_None)
				{
					// Weapon needs ammo or target is out of range of physical body.
					if(weapon.m_CID > CID_Invalid && ammoCur == 0 && ammoFull != 0)
					{
						// My weapon is not part of my physical body so it must be out of ammo.
						DecideReloadWeapon(ammoType, gearPos, useSub);

						// Reloading my weapon used up my attack cycle.
						// Now I can't attack again until the next positive bite cycle.
						RestartAIAttackTime();
					}
					else
					{
						// When my weapon is my physical body, the target must be out of range to produce an effect of WE_None.
						//qDebug() << "target out of range";
					}
				}
				else if(!(Id().GetID()&1) && (Id().GetID()&2) &&
						!weapon.IsOptimumRange(targetRange))
				{
					// The Id() masks match those used above to determine the moveWithinRange value.
					// Wait for optimum weapon range to attack target.
					//qDebug("%s not optimum range at %g. Moving within %g range", (const char*)Name().toAscii(),
							//targetRange, moveWithinRange);
				}
				else
				{
					// Use the weapon on the target.
					DoAttackTarget(sightedTarget, sightedLoc, sightedShield, targetIsFriendlyOrDying, weapon, useSpecWpn, gearPos,
							useSub);

					// Now I can't attack again until the next attack cycle.
					RestartAIAttackTime();
					didAttack = true;
				}
			}
			//else
			//{
				//qDebug("%s not attacking target %s", (const char*)Name().toAscii(),
						//(const char*)sightedTarget->Name().toAscii());
			//}
		}

		if(!result && !didAttack && orderTarget && orderTarget->DoesQueryEnvironment())
		{
			// I am pursuing a target and I did not attack it.
			if(!orderTarget->IsOnGround() && !CanFly())
			{
				// The target is flying away. Stop chasing it if I can't fly.
				RequestOrder();	// Give up and request my next order.
			}
			else if(orderTarget->IsInWater() && !IsInWater())
			{
				// The target is in water and I am not in water.
				RequestOrder();	// Give up and request my next order.
			}
			else if(orderTarget->IsEyeSubmerged() && !IsEyeSubmerged())
			{
				// The target is under water and I am not under water.
				RequestOrder();	// Give up and request my next order.
			}
		}
	}
}

void DaSentient::DoBattleStation()
{
	//g_Au.Log(QString("DaSentient::DoBattleStation %1 %2").arg(Id().GetID()).arg(Name()));
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoBattleStation %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());

	// Default citizen doesn't man battle stations.
	RequestOrder();	// Ask for my next order.
}

void DaSentient::DoEnterSurgery()
{
	//if(Is(49385478))
		//qDebug() << "DaSentient::DoEnterSurgery" << Id() << Name() << "pose" << CurrentGeneralPose();
	if(IsAboardVehicle())
	{
		// I am in a vehicle.
		DaAtom* parent = Parent();
		if(!parent)
			return;
		if(parent->IsInSpacecraft())
			DoGetOut();		// Vehicle is inside a spacecraft. Maybe there is a sick bay aboard.
		else
			RequestOrder();	// Ask for my next order.
		return;
	}

	// Dismount to carry out this order.
	if(IsRider())
		DismountCreature();

	if(!IsInSpacecraft())
	{
		// I'm not in a vehicle or a spacecraft. Cancel this order and move on to the next order.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// I am aboard a spacecraft.
	DaCapsule* cap = dynamic_cast<DaCapsule*>(Parent());
	if(!cap)
	{
		// Parent atom is not a spacecraft. Cancel this order and move on to the next order.
		RequestOrder();										// Ask for my next order.
		return;
	}
	if(!cap->IsDesignDataCacheValid())
		return;													// Wait for capsule data cache to catch up.
	else if(!cap->IsNormal())
		return;													// Wait for capsule to be normal. It could be waiting to wormhole.

	if(IsPatientStationPose())
	{
		// I am in a surgery unit.
		if(PrimaryOrder() == ORDER_EnterSurgery)
		{
			// My primary order is to stay in this surgery unit. I hope the doc doesn't give me some ugly disease.
			StayIdle();										// Wait for a different order.
		}
		else
		{
			// My primary order is not to stay in this surgery unit.
			// Go back to work after the doctor can do nothing else for me.
			// Each time the console is used to apply a treatment, my think time is restarted. That way I won't ever get here
			// until the doctor has stopped treating me.
			ExitConsole();								// Leave surgery unit when it can't help me.
			RequestOrder();									// Ask for my next order.
		}
	}
	else if(IsLockedPose())
	{
		// I am manning some other console.
		//if(Is(49385478))
			//qDebug() << Id() << Name() << "exiting some other console, primary order" << PrimaryOrder()
				//<< ",next order" << NextOrder() << ",orders" << m_AIOrders.count();
		ExitConsole();									// Exit this console so I can go find a surgery unit.
	}
	else
	{
		// I am not in a surgery unit.
		// Locate the closest unoccupied surgery unit to my location.
		// If all surgery units are occupied, I'll go to my bunk and wait.
		// If I don't have a bunk, I'll wait where I am standing.
		quint16 stationId;
		AuStation station;
		quint16 which;
		if(cap->ClosestUnoccupiedSurgeryUnit(Location(), stationId, station, which, cap->IsCrewOrTroop(Id())))
		{
			//if(Is(49385478))
				//qDebug() << Id() << Name() << "found unoccupied surgery unit, primary order" << PrimaryOrder()
					//<< ",next order" << NextOrder() << ",orders" << m_AIOrders.count();
			// Found an unoccupied surgery unit that is not broken and has a doctor. A doctor means the med station is working.
			// Try to enter the unit if it is close enough.
			// Update calculated dest location relative to the cap, from the design coordinates in the detail.
			AuPoint destLoc = station.OpM().Translation();
			AuPoint conLoc = station.OpM().UnProject(Location());	// My location in the space of the console.
			if(conLoc.Length2_2D() < 4*4 || cap->OkToTeleportToStation(this))
			{
				// I am close enough to the unit or it's ok to just teleport this dude to the unit.
				// Adjust for pose.
				// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
				station.OffsetOpM(ConsolePoseOffset(station.Type(), station.OccPos(), CGHeightStanding(), TorsoThickness(),
							station.SeatIndex()));
				DoSitAtConsole(stationId, station, which);

				// NOTE: I do not have to remove my clothing. My clothing functions return nakedness when I am in a surgery unit.
				cap->WakeDoctor(*this);	// Wake up the doctor; he might be sleeping.
			}
			else
			{
				// Move close to the unoccupied surgery unit.
				// Amend my orders with a movement order that will get me there.
				AmendOrder(ORDER_GoToTargetExactly, cap->Id(), destLoc, stationId);
			}
		}
		else if(cap->IsBerthedHere(Id()))
		{
			//if(Is(49385478))
				//qDebug() << Id() << Name() << "no unoccupied surgery unit, going to berth, primary order" << PrimaryOrder()
					//<< ",next order" << NextOrder() << ",orders" << m_AIOrders.count();
			// No unoccupied surgery unit and I have a berth aboard this capsule.
			quint16 roomId;
			AuPoint berthLoc = cap->BerthLocation(Id(), roomId);

			// When going to my berth, don't be too precise about it or my bunkmate and I stand on top of each other.
			if(roomId == CapsuleRoomId())
			{
				// I am in my bunk room. That's close enough.
				StayIdle();			// Wait for an empty surgery unit.

				// Amend my orders with a sleep order so I won't be a load on the server looking for an unoccupied surgery unit.
				TakeANap();			// Sleep when I am not on duty.
			}
			else
			{
				// Move to my bunk room.
				// Adjust the destination location to be in the room and not exactly on the floor, ceiling, or wall line.
				AuPoint destLoc = berthLoc - cap->DesignCG();
				destLoc.z += CGHeightStanding();
				if(cap->OkToTeleportToStation(this))
				{
					// It's ok to just teleport this dude to his bunk.
					DoTeleport(destLoc);
				}
				else
				{
					// Amend my orders with a movement order that will get me there.
					AmendOrder(ORDER_GoToTarget, cap->Id(), destLoc, 0);
				}
			}
		}
		else
		{
			//if(Is(49385478))
				//qDebug() << Id() << Name() << "no unoccupied surgery unit, staying here, primary order" << PrimaryOrder()
					//<< ",next order" << NextOrder() << ",orders" << m_AIOrders.count();
			// No unoccupied surgery unit and I don't have a berth aboard the capsule.
			StayIdle();							// Wait for an empty surgery unit.

			// Amend my orders with a sleep order so I won't be a load on the server looking for an unoccupied surgery unit.
			TakeANap();							// Sleep when I am not on duty.
		}
	}
}

void DaSentient::DoCloseDoor()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoCloseDoor %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());
	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	if(IsAboardVehicle())
	{
		RequestOrder();	// Ask for my next order.
		return;
	}

	if(IsInCapsule())
	{
		// Look for the closest operable door.
		DaCapsule* ship = FindParentCapsuleInEnv();
		if(ship)
		{
			// Must exit my console or turret.
			if(IsLockedPose())
			{
				ExitConsole();
				return;
			}

			double doorDist = 0;
			int doorIndex = ship->ClosestOpenDoor(this, doorDist);
			if(doorIndex >= 0 && ship->IsDoorOpen(doorIndex))
			{
				// Unjams the door if its jammed.
				DecideShipDoorClose(ship->Id(), doorIndex);
			}
		}
	}
	else if(IsInAtmosphere())
	{
		// Look for the closest hull door.
		DaWorld* planet = FindParentPlanetInEnv();
		if(planet)
		{
			DaAtomList spaceships;
			planet->FindCapsulesRecursive(spaceships);
			DaCapsule* closestShip = 0;
			int closestDoorIndex = -1;
			double closestDoor = 0;
			for(DaAtomList::const_iterator ssi = spaceships.begin(); ssi != spaceships.end(); ssi++)
			{
				DaCapsule* ship = dynamic_cast<DaCapsule*>(*ssi);
				if(!ship)
					continue;
				double doorDist = 0;
				int doorIndex = ship->ClosestOpenDoor(this, doorDist);
				if(doorIndex >= 0)
				{
					// Found an operable door within 6' on the ship.
					if(closestDoor == 0 || doorDist < closestDoor)
					{
						closestDoor = doorDist;
						closestDoorIndex = doorIndex;
						closestShip = ship;
					}
				}
			}
			if(closestDoorIndex >= 0 && closestShip->IsDoorOpen(closestDoorIndex))
				DecideShipDoorClose(closestShip->Id(), closestDoorIndex);	// Unjams the door if its jammed.
		}
	}
	RequestOrder();	// Ask for my next order.
}

void DaSentient::DoComeHere()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoComeHere %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 120)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Carry out my current order.
	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(IsInSpacecraft()?MOVE_CLOSE:MOVE_NEAR, true, orderTarget);
	if(result)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		if(result == 2)
		{
			//g_Au.Log(QString("DaSentient::DoComeHere %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
			TakeANap();
		}
	}
}

void DaSentient::DoComeHereExactly()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoComeHereExactly %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 60)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.
		return;
	}

	DaAtom* orderTarget = 0;
	if(MoveToOrderLoc(MOVE_EXACT, true, orderTarget))
		RequestOrder();	// Ask for my next order.
}

void DaSentient::DoFollowMe()
{
	DoFollowTarget();
}

void DaSentient::DoFollowTarget()
{
	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Be on the look-out for threats.
	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 2)
		LookForThreats(false);

	// Carry out my current order.
	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(IsInSpacecraft()?MOVE_CLOSE:MOVE_NEAR, false, orderTarget);
	if(result == 2)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		//g_Au.Log(QString("DaSentient::DoFollowTarget %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
		TakeANap();
	}
	else if(result == 1)
		StayIdle();		// I am close to my target.
}

void DaSentient::DoGarrison()
{
	// Must exit my console or turret.
	if(IsLockedPose())
		ExitConsole();

	// Dismount to carry out this order.
	if(IsRider())
		DismountCreature();

	if(!AssignedShip().IsNull())
		ReleaseBerth();

	SetGarrisonPosture();										// Switch to default garrision posture for this type of dude.
}

void DaSentient::DoGetIn()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoGetIn %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());
	if(OrderTarget() == ParentId())
	{
		// I was ordered to get in to my current parent atom so my order is complete.
		// This could include mounting a creature, entering a vehicle, or entering a spacecraft.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 60)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.
		return;
	}

	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(IsInSpacecraft()?MOVE_CLOSE:MOVE_NEAR, true, orderTarget);
	if(result == 2)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		//g_Au.Log(QString("DaSentient::DoGetIn %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
		TakeANap();
	}
	else if(result == 1)
	{
		// I am close to my target.
		// If it is a vehicle, get in.
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(orderTarget);
		if(vehicle)
		{
			// Found a vehicle.
			if(!IsMounted())
			{
				// I am not carrying a rider.
				DoEnterVehicle(vehicle);
			}
			else
			{
				// Cannot enter a vehicle while I am carrying a rider.
				RequestOrder();	// Ask for my next order.
			}
		}
		else
		{
			// If it is a creature, mount up.
			DaCreature* creature = dynamic_cast<DaCreature*>(orderTarget);
			if(creature)
				MountCreatureIfPossible(creature);
			RequestOrder();
		}
	}
}

void DaSentient::DoGetItem()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoGetItem %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 120)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Carry out my current order.
	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(MOVE_CLOSE, true, orderTarget);
	if(result == 1 && orderTarget)
	{
		// Close enough to pick up the item.
		DecidePickUp(orderTarget->Id());
	}
	if(result)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		if(result == 2)
		{
			//g_Au.Log(QString("DaSentient::DoGetItem %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
			TakeANap();
		}
	}
}

void DaSentient::DoGetOut()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoGetOut %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());
	if(OrderTarget() != ParentId())
	{
		// I was ordered to get out of the orderTarget atom so my order is complete.
		RequestOrder();	// Ask for my next order.
		return;
	}

	if(IsRider())
	{
		// I am riding a creature. Dismount it.
		DismountCreature();
		RequestOrder();									// Ask for my next order.
	}
	else if(IsAboardVehicle())
	{
		// I am in the vehicle that I am supposed to exit.
		DismountVehicle();
		RequestOrder();									// Ask for my next order.
	}
	else if(IsInCapsule())
	{
		// Must exit my console or turret.
		if(IsLockedPose())
		{
			ExitConsole();
			return;
		}

		// Give up on the current order when it has taken too long to complete.
		if(!HasOrder() || OrderTime().ElapsedSec() > 120)
		{
			// It has taken too long for me to complete the order.
			// Give up rather than load the server forever.
			RequestOrder();	// Ask for my next order.
			return;
		}

		// I am in a spacecraft that I am supposed to exit.
		DaCapsule* ship = FindParentCapsuleInEnv();
		if(ship)
		{
			bool movingToTeleporter = false;
			if(ship->IsTransporterRoom(CapsuleRoomId()))
			{
				// I am standing in a transporter room.
				// Move into transporter if there is an active teleporter in the room.
				quint16 whichTrans = AtTransporterWhich();
				AuPoint padLoc;
				if(ship->ActiveTransporterPadLocation(whichTrans, padLoc))
				{
					// Convert the pad location to a vector relative to me.
					//qDebug("%s moving to transporter.", (const char*)Name().toAscii());
					AuPoint moveVec = Transform().UnProject(padLoc);	// Make the target location a vector relative to me.
					MoveInDirection(moveVec, 0, AuPose::StandWalk);
					movingToTeleporter = true;
				}
			}
			else
			{
				// I am not standing in a transporter room. Look for teleporters in my area.
				double telDistance = 0;
				AuPoint telLoc;
				if(ship->FindClosestOriginTeleporter(Location(), telLoc, telDistance))
				{
					// Found an active origin teleporter aboard this ship.
					if(telDistance < RUN_FAR_ENOUGH*2)
					{
						// The origin teleporter is close enough to go for it.
						// Make the teleporter location into a vector relative to me.
						AuPoint moveVec = Transform().UnProject(telLoc);
						if(telDistance > RUN_FAR_ENOUGH)
							MoveInDirection(moveVec, 0, AuPose::StandRun);
						else
							MoveInDirection(moveVec, 0, AuPose::StandWalk);
						movingToTeleporter = true;
					}
				}
			}
			if(!movingToTeleporter)
			{
				// I did not find a teleporter to move toward.
				// Move toward the nearest exit.
				//qDebug("%s Moving to exit ship", (const char*)Name().toAscii());
				int doorIndex, holeIndex;								// Output value from EntranceLocation. -1 if no exit found.
				if(Is(s_PathTrace))
				{
					g_Au.m_LogMessages |= LOG_Path;
					qDebug() << "DaSentient::DoGetOut" << Id() << Name() << "ship on ground" << ship->IsOnGround();
				}
				AuPoint exitLoc = ship->ExitLocation(Location(), doorIndex, holeIndex, ship->IsOnGround());
				if(Is(s_PathTrace))
					g_Au.m_LogMessages &= ~LOG_Path;
				if(doorIndex >= 0)
				{
					if(Is(s_PathTrace))
						qDebug() << "DaSentient::DoGetOut" << Id() << Name() << "moving to door location of ship on ground"
							<< doorIndex;

					// Adjust the exit location to get off dangling ladders.
					if(dynamic_cast<const DaSc*>(ship))
					{
						// Move away from the ship.
						exitLoc.Unit(ship->RoughRadius());
						if(ship->IsOnGround())
						{
							AuBox extBox;
							ship->RoughExtents(extBox);
							exitLoc.z = extBox.m_fCorners[0].z;	// Min z of extents, so we find low doors instead of on top.
						}
					}
					MoveToCapsuleLoc(ship, exitLoc);			// Move toward the exit to the outside.
				}
				else if(holeIndex >= 0)
				{
					if(Is(s_PathTrace))
						qDebug() << "DaSentient::DoGetOut" << Id() << Name() << "moving to hole location of ship on ground";
					if(dynamic_cast<const DaSc*>(ship))
					{
						// Move away from the ship.
						exitLoc.Unit(ship->RoughRadius());
						if(ship->IsOnGround())
						{
							AuBox extBox;
							ship->RoughExtents(extBox);
							exitLoc.z = extBox.m_fCorners[0].z;	// Min z of extents, so we find low doors instead of on top.
						}
					}
					MoveToCapsuleLoc(ship, exitLoc);			// Move toward the exit to the outside.
				}
				else
					RequestOrder();							// No exit found.
			}
		}
	}
	else
	{
		// I don't know how to get out of whatever I'm in.
		RequestOrder();
	}
}

void DaSentient::DoHengeCeremony()
{
	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	if(!IsInAtmosphere())
	{
		// I am not at a planet?
		RequestOrder();
		return;
	}

	// The ceremony does not last forever.
	if(!HasOrder() || OrderTime().ElapsedMin() > 30)
	{
		// It has been long enough.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Give up on the current order when it has taken long enough.
	DaWorldLL* world = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
	if(!world)
	{
		// I am not at a planet?
		RequestOrder();
		return;
	}

	AuGlobeLLF hengeLL;
	AuPoint orderLoc(OrderLoc());
	if(!world->IsHengeNear(orderLoc, hengeLL))
	{
		// I am not at a planet with a henge?
		RequestOrder();
		return;
	}

	// There is a henge near the destination of my order.
	double innerRadius, outerRadius;
	world->HengeRadii(hengeLL, innerRadius, outerRadius);
	AuPoint myGndLoc = world->LocOfLL(GlobeLL(), world->TerrainAltitudeMSL(GlobeLL()));	// My location on the ground.
	double myGndDist = myGndLoc.Distance(orderLoc);

	// Dance around the henge when I am in the zone between the inner and outer stones.
	if(myGndDist < outerRadius)
	{
		if(AuTime::Now().SecsSinceEpoch() & 1)
		{
			// Dance around the ring.
			double danceR = (innerRadius + outerRadius) / 2;	// Radius at which to dance.
			double c = PI * danceR * 2;							// circumference of dance ring.
			double step = 1;									// Always aim for a spot slightly ahead of me around the circle.
			double steps = c / step;							// Number of 1m steps around.
			double angPerStep = (PI*2) / steps;					// Angle per step.

			// Create transform of henge.
			AuMatrix4x4 t;
			t.Translate(orderLoc);
			t *= AuPoint(0,0,1).GetAlignmentTransform(-world->GravityDir(orderLoc));

			AuPoint myhloc = t.UnProject(myGndLoc);	// My location in the space of the henge.
			myhloc = myhloc.RotateZ(angPerStep);
			myhloc.Unit(danceR);					// Aim for the dance radius.
			myhloc = t.Project(myhloc);				// Dance destination location in world space.

			AuPoint moveVec = Transform().UnProject(myhloc);	// Make the target location a vector relative to me.
			MoveToGlobeLoc(world, myhloc, moveVec, -1, 0);
		}
		else
		{
			StayIdle();
		}
	}
	else
	{
		// Move toward the henge.
		DaAtom* orderTarget = 0;
		int result = MoveToOrderLoc(MOVE_NEAR, true, orderTarget);
		if(result)
		{
			RequestOrder();	// Ask for my next order.

			// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
			if(result == 2)
			{
				// Each sleep order is 30 seconds.
				TakeANap();
				TakeANap();
				TakeANap();
				TakeANap();
			}
		}
	}
}

void DaSentient::DoGoToRoom()
{
	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 60*4)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Be on the look-out for threats.
	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 2)
		LookForThreats(false);

	// I can only do this if I am on the roster of the spacecraft I am aboard.
	bool canFindIt = false;
	if(IsInSpacecraft())
	{
		const DaVessel* v = dynamic_cast<const DaVessel*>(Parent());
		if(v && v->Is(AssignedShip()))
			canFindIt = true;
	}

	// Carry out my current order.
	DaAtom* orderTarget = 0;
	int result;
	if(canFindIt)
		result = MoveToOrderLoc(IsInSpacecraft()?MOVE_CLOSE:MOVE_NEAR, true, orderTarget);
	else
		result = 2;	// I don't know how to find the room.
	if(result)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		if(result == 2)
		{
			//g_Au.Log(QString("DaSentient::DoGoToTarget %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
			// Each sleep order is 30 seconds.
			TakeANap();
			TakeANap();
			TakeANap();
			TakeANap();
		}
	}
}

void DaSentient::DoGoToTarget()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoGoToTarget %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 60*4)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.

		if(Order() == ORDER_BattleStation)
		{
			//g_Au.Log(QString("DaSentient::DoGoToTarget %1 %2 timed out, napping").arg(Id().GetID()).arg(Name()));
			if(!AssignedShip().IsNull())
			{
				// If I have taken too long to get to my battle station, sleep before trying again.
				// Each sleep order is 30 seconds.
				TakeANap();
				TakeANap();
				TakeANap();
				TakeANap();
			}
			else
			{
				// I don't have an assigned ship so I must be manning a base.
				// Self destruct if I'm stuck. A nu guy will spawn to replace me; maybe he won't be stuck.
				int orderTime = OrderTime().ElapsedSec();
				qDebug("DaSentient::DoGoToTarget %d %s Killed. Failed to get to station at defense base after %d sec.",
						Id().GetID(), (const char*)Name().toLatin1(), orderTime);
				Kill();
			}
		}
		return;
	}

	// Be on the look-out for threats.
	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 2)
		LookForThreats(false);

	// Carry out my current order.
	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(IsInSpacecraft()?MOVE_CLOSE:MOVE_NEAR, true, orderTarget);
	if(result)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		if(result == 2)
		{
			//g_Au.Log(QString("DaSentient::DoGoToTarget %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
			// Each sleep order is 30 seconds.
			TakeANap();
			TakeANap();
			TakeANap();
			TakeANap();
		}
	}
}

void DaSentient::DoGoToTargetExactly()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoGoToTargetExactly %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 120)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.

		if(Order() == ORDER_BattleStation)
		{
			//g_Au.Log(QString("DaSentient::DoGoToTargetExactly %1 %2 timed out, napping").arg(Id().GetID()).arg(Name()));
			if(!AssignedShip().IsNull())
			{
				// If I have taken too long to get to my battle station, sleep before trying again.
				// Each sleep order is 30 seconds.
				TakeANap();
			}
			else
			{
				// I don't have an assigned ship so I must be manning a base.
				// Self destruct if I'm stuck. A nu guy will spawn to replace me; maybe he won't be stuck.
				int orderTime = OrderTime().ElapsedSec();
				qDebug("DaSentient::DoGoToTargetExactly %d %s Killed. Failed to get to station at defense base after %d sec.",
						Id().GetID(), (const char*)Name().toLatin1(), orderTime);
				Kill();
			}
		}
		return;
	}

	// Check to see if I am going to a station.
	CITIZEN_ORDER nextOrder = NextOrder();
	double closeEnough;
	if(nextOrder == ORDER_BattleStation)
	{
		// I am going to a control station.
		int orderTime = OrderTime().ElapsedSec();
		if(orderTime%3 == 0)
		{
			// Check every few seconds to insure my target console has not been occupied.
			const DaCapsule* ship = FindParentCapsuleInEnv();
			if(ship)
			{
				if(ship->Is(OrderTarget()) && ship->IsConsoleSeatOccupied(OrderStationId(), 0))
				{
					// The ship I am aboard is the target of my current move order and the console at the target loc is occupied.
					RequestOrder();	// Ask for my next order, which is the battle station order. That will decide what to do.
					return;
				}
			}
		}
		closeEnough = MOVE_STATION;
	}
	else if(nextOrder == ORDER_EnterSurgery)
	{
		// I am going to a surgery unit.
		int orderTime = OrderTime().ElapsedSec();
		if(orderTime%3 == 0)
		{
			// Check every few seconds to insure my target surgery unit has not been occupied.
			const DaCapsule* ship = FindParentCapsuleInEnv();
			if(ship && ship->Is(OrderTarget()) && ship->IsPatientStationOccupied(OrderStationId()))
			{
				// The ship I am aboard is the target of my current move order and the unit at the target location is occupied.
				RequestOrder();	// Ask for my next order, which is the EnterSurgery order. That will decide what to do next.
				return;
			}
		}
		closeEnough = MOVE_STATION;
	}
	else
		closeEnough = MOVE_EXACT;

	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(closeEnough, true, orderTarget);
	if(result)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		if(result == 2)
		{
			//g_Au.Log(QString("DaSentient::DoGoToTargetExactly %1 %2 no path, napping").arg(Id().GetID()).arg(Name()));
			// Each sleep order is 30 seconds.
			TakeANap();
			TakeANap();
			TakeANap();
			TakeANap();
		}
	}
}

void DaSentient::DoOpenDoor()
{
	//if(!IsInAnyScene())
		//qDebug("DaSentient::DoOpenDoor %d %s not in any scene", Id().GetID(), (const char*)Name().toAscii());
	if(IsRider())
	{
		// I am on a mount.
		RequestOrder();
		return;
	}

	if(IsAboardVehicle())
	{
		RequestOrder();	// Ask for my next order.
		return;
	}

	if(IsInCapsule())
	{
		// Look for the closest operable door.
		DaCapsule* ship = FindParentCapsuleInEnv();
		if(ship)
		{
			// Must exit my console or turret.
			if(IsLockedPose())
			{
				ExitConsole();
				return;
			}

			double doorDist = 0;
			int doorIndex = ship->ClosestClosedDoor(this, doorDist);
			if(doorIndex >= 0 && ship->CanOpenDoor(*this, doorIndex))
				DecideShipDoorOpen(ship->Id(), doorIndex);
		}
	}
	else if(IsInAtmosphere())
	{
		// Look for the closest hull door.
		DaWorld* planet = FindParentPlanetInEnv();
		if(planet)
		{
			DaAtomList spaceships;
			planet->FindCapsulesRecursive(spaceships);
			DaCapsule* closestShip = 0;
			int closestDoorIndex = -1;
			double closestDoor = 0;
			for(DaAtomList::const_iterator ssi = spaceships.begin(); ssi != spaceships.end(); ssi++)
			{
				DaCapsule* ship = dynamic_cast<DaCapsule*>(*ssi);
				if(!ship)
					continue;
				double doorDist = 0;
				int doorIndex = ship->ClosestClosedDoor(this, doorDist);
				if(doorIndex >= 0)
				{
					// Found an operable door within 6' on the ship.
					if(closestDoor == 0 || doorDist < closestDoor)
					{
						closestDoor = doorDist;
						closestDoorIndex = doorIndex;
						closestShip = ship;
					}
				}
			}
			if(closestDoorIndex >= 0 && !closestShip->IsDoorOpen(closestDoorIndex) &&
					closestShip->CanOpenDoor(*this, closestDoorIndex))
				DecideShipDoorOpen(closestShip->Id(), closestDoorIndex);
		}
	}
	RequestOrder();	// Ask for my next order.
}

void DaSentient::DoStayHere()
{
	if(IsRider())
		return;													// Stay mounted.

	if(IsAboardVehicle())
		return;													// Stay in the vehicle.

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	// Check to see if I need to run to the sick bay for surgery.
	if(GetACheckup())
		return;													// I have decided to get medical attention.

	// Don't do idle animation when not in anyone's scene.
	if(!IsInAnyScene())
		return;

	// Be on the look-out for threats.
	if(!IsInAnyScene() || LastLookForThreatsTime().ElapsedSec() < 5 || !LookForThreats(true))
	{
		// I am executing a stay order and I'm not on alert for threats. Take a little nap.
		TakeANap();

		// Carry out my current order.
		StayIdle();
	}
}

void DaSentient::DoSleep()
{
	// Wake up after 30 seconds.
	if(!HasOrder() || OrderTime().ElapsedSec() >= 30)
	{
		//g_Au.Log(QString("DaSentient::DoSleep %1 %2 woke up").arg(Id().GetID()).arg(Name()));
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Carry out my current order.
	StayIdle();
}

// Look around and perceive things that matter to this citizen's life strategy
// using eyes, ears, and nose.
// 'item' is nearest item of interest, could be a vehicle.
// 'target' is the target of this citizen's AI order, OrderTarget().
// 'threat' is nearest threat
// 'comrade' is nearest comrade who is alive and kicking
// 'fallen' is nearest fallen comrade who is not dead.
// Distances will be 0 for things that are not seen. i.e. itemRange = 0 when no item is in sight.
// Vectors returned are relative to this atom.
// Vehicles may be identified as threats or targets or items.
void DaSentient::Look(DaAtom*& itemAtom, AuPoint& itemDir, float& itemRange,
		DaAtom*& targetAtom, AuPoint& targetDir, float& targetRange,
		DaAtom*& threatAtom, AuPoint& threatDir, float& threatRange,
		DaAtom*& comradeAtom, AuPoint& comradeDir, float& comradeRange,
		DaAtom*& fallenAtom, AuPoint& fallenDir, float& fallenRange)
{
	// What is the range of my weapon?
	AuDNABase::WEAPONLOC wLoc;
	AuCombatWeapon weapon(WM_Not);	// CID_Invalid if wpn is part of my body
	int ammoCur, ammoFull;
	BestWeaponAtRange(0, wLoc, weapon, ammoCur, ammoFull);

	//qDebug("DaSentient::Look %d %s looking", Id().GetID(), (const char*)Name().toAscii());
	// Init return values.
	comradeAtom = 0;
	comradeRange = 0;
	fallenAtom = 0;
	fallenRange = 0;
	itemAtom = 0;
	itemRange = 0;
	targetAtom = 0;
	targetRange = 0;
	threatAtom = 0;
	threatRange = 0;

	bool saySomething = false;
	if(IsInCapsule())
	{
		//qDebug("DaSentient::Look - %d Aboard ship. Can't see.", Id().GetID());
		// Citizen vision on board ship.
		const DaCapsule* ship = FindParentCapsuleInEnv();
		if(ship)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, ship);
			AuPoint eyeLoc = CameraOffset();
			AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			// Based on eye size, scan the spacecraft in this citizen's field of vision.
			float earRange = HearingRange();	// Range of ears.
			if(IsEyeSubmerged())
				earRange *= 2.f;
			else if(!IsUnderGround() && !AirDensity())
				earRange = 0;
			float noseRange = SmellRange();	// Range of nose.
			if(IsEyeSubmerged())
				noseRange /= 2.f;
			else if(IsUnderGround())
				noseRange = 0;
			else if(!AirDensity())
				noseRange = 0;
			float eyeRange = SightRange();	// Range of eyes.
			if(IsUnderGround())
				eyeRange = 0;
			else if(IsEyeSubmerged())
				eyeRange /= 3.f;
			if(AIIsAlert())
			{
				// Increase range of senses when alert.
				eyeRange *= 1.2f;
				earRange *= 1.15f;
				noseRange *= 1.1f;
			}
			float eyeAngle = FieldOfViewAngle();	// Field of view angle is 1/2 field of view.

			// Scan the subatoms of the spacecraft.
			LookAtSubatoms(ship, ship, myT, eyeLoc, eyeVec, eyeAngle, eyeRange, noseRange, earRange, weapon.m_Range,
					itemAtom, itemDir, itemRange,
					targetAtom, targetDir, targetRange,
					threatAtom, threatDir, threatRange,
					comradeAtom, comradeDir, comradeRange,
					fallenAtom, fallenDir, fallenRange);

			if((targetAtom || threatAtom || fallenAtom) && !IsAlert())
			{
				// I saw something to become alert about.
				saySomething = true;
			}
		}
	}
	else if(IsInAtmosphere())
	{
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, llworld);
			AuPoint eyeLoc = CameraOffset();
			AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			// Based on eye size, scan the terrain in this animal's field of vision.
			float earRange = HearingRange();	// Range of ears.
			if(IsEyeSubmerged())
				earRange *= 2.f;
			else if(!IsUnderGround() && !AtmosphereDensity())
				earRange = 0;
			float noseRange = SmellRange();	// Range of nose.
			if(IsEyeSubmerged())
				noseRange /= 2.f;
			else if(IsUnderGround())
				noseRange = 0;
			else if(!AtmosphereDensity())
				noseRange = 0;
			float eyeRange = SightRange();	// Range of eyes.
			if(IsUnderGround())
				eyeRange = 0;
			else if(IsEyeSubmerged())
				eyeRange /= 3.f;
			if(AIIsAlert())
			{
				// Increase range of senses when alert.
				eyeRange *= 1.2f;
				earRange *= 1.15f;
				noseRange *= 1.1f;
			}
			float eyeAngle = FieldOfViewAngle();	// Field of view angle is 1/2 field of view.

			//qDebug("DaSentient::Look %d %s Ranges ear %g eye %g nose %g", Id().GetID(), (const char*)Name().toAscii(),
					//earRange, eyeRange, noseRange);
			// Scan the subatoms of the globe.
			LookAtSubatoms(llworld, llworld, myT, eyeLoc, eyeVec, eyeAngle, eyeRange, noseRange, earRange, weapon.m_Range,
					itemAtom, itemDir, itemRange,
					targetAtom, targetDir, targetRange,
					threatAtom, threatDir, threatRange,
					comradeAtom, comradeDir, comradeRange,
					fallenAtom, fallenDir, fallenRange);

			if((targetAtom || threatAtom || fallenAtom) && !IsAlert())
			{
				// I saw something to become alert about.
				saySomething = true;
			}
		}
	}
	else
	{
		// Not in spacecraft and not in atmosphere. I am EVA.
		const DaAtom* commonParent = FindParentPlanetInEnv();
		if(!commonParent)
			commonParent = FindParentStarInEnv();
		if(!commonParent)
			commonParent = FindEnvironmentAtom();
		if(commonParent)
		{
			AuMatrix4x4 myT;
			if(dynamic_cast<const DaSolarSystem*>(commonParent))
				myT = EnvTransform();
			else
				GetTransformRelativeToParent(myT, commonParent);
			AuPoint eyeLoc = CameraOffset();
			AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			// Based on eye size, scan the spacecraft in this citizen's field of vision.
			float earRange = 0;
			float noseRange = 0;
			float eyeRange = SightRange() * 2;	// Range of eyes is greater in space.
			if(AIIsAlert())
				eyeRange *= 1.2f;						// Increase range of senses when alert.
			float eyeAngle = FieldOfViewAngle();	// Field of view angle is 1/2 field of view.

			// Scan the subatoms of the commonParent. This will only see other atoms that are outside the atmosphere.
			LookAtSubatoms(commonParent, commonParent, myT, eyeLoc, eyeVec, eyeAngle, eyeRange, noseRange, earRange, weapon.m_Range,
					itemAtom, itemDir, itemRange,
					targetAtom, targetDir, targetRange,
					threatAtom, threatDir, threatRange,
					comradeAtom, comradeDir, comradeRange,
					fallenAtom, fallenDir, fallenRange);

			if((targetAtom || threatAtom || fallenAtom) && !IsAlert())
			{
				// I saw something to become alert about.
				saySomething = true;
			}
		}
		else
			qDebug("DaSentient::Look - %d Not subordinate to any useful atoms. Can't see.", Id().GetID());
	}
	//qDebug("DaSentient::Look - foodRange %g preyRange %g threatRange %g",
			//foodRange, preyRange, threatRange);

	if(targetAtom || threatAtom || fallenAtom)
	{
		// Become alert when I perceive a threat or my target or a fallen comrade.
		// Alertness increases my ability to perceive threats and helps me stay focused on threats once they have been acquired.
		if(!IsAlert())
		{
			// I am just now becoming alert from complacent state.
			if(saySomething)
			{
				// Announce the reason I am becoming alert.
				QString text;
				if(threatAtom)
				{
					const DaCreature* creature = dynamic_cast<const DaCreature*>(threatAtom);
					if(!creature || !creature->LooksLikeVermin())
					{
						if(!SpokeRecently(60))
						{
							DaVessel* ves = FindParentVesselInEnv();
							if(ves)
							{
								QString origin;
								switch(ves->MonsterOrigin())
								{
								case DaVessel::MONSTER_Unknown:
									break;
								case DaVessel::MONSTER_Officer:
									origin = tr("We tracked it back through the ventilation system, "
											"to some egg shells in the officer's berth. ");
									break;
								case DaVessel::MONSTER_Crew:
									origin = tr("A crewman barfed up some kind of organism that grew quickly. ");
									break;
								case DaVessel::MONSTER_Troop:
									origin = tr("One of the troops found a nest in their gear. ");
									break;
								case DaVessel::MONSTER_Passenger:
									origin = tr("It looks like it climbed out of the passenger's latrine. ");
									break;
								case DaVessel::MONSTER_Cargo:
									origin = tr("I found a cargo container broken open with what appears to be a den inside. ");
									break;
								}
								ves->ResetMonsterOrigin();
								if(origin.isEmpty())
									text = QObject::tr("Sir. Space monster spotted! %1")
										.arg(threatAtom->SpottedLocation());
								else
									text = QObject::tr("Sir! We've got a space monster aboard. %1 %2")
										.arg(threatAtom->SpottedLocation())
										.arg(origin);
							}
							else
								text = QObject::tr("Sir! I spotted a wild animal. %1")
									.arg(threatAtom->SpottedLocation());
						}
					}
				}
				else if(targetAtom)
				{
					const DaCreature* creature = dynamic_cast<const DaCreature*>(targetAtom);
					if(!creature || !creature->LooksLikeVermin())
					{
						if(Order() == ORDER_AttackTarget && !SpokeRecently(60))
							text = QObject::tr("Sir, I am attacking %1. %2")
								.arg(targetAtom->Name())
								.arg(targetAtom->SpottedLocation());
						//else
							//text = QObject::tr("Sir, %1 located. I am proceeding.").arg(targetAtom->Name());
					}
				}
				else if(fallenAtom)
				{
					const DaCreature* creature = dynamic_cast<const DaCreature*>(fallenAtom);
					if(creature)
						text = QObject::tr("Sir, I am going to pick up %1; %2 has fallen. %3")
							.arg(creature->Name())
							.arg(creature->SheHe(false))
							.arg(creature->SpottedLocation());
					else
						text = QObject::tr("Sir, I am going to pick up %1; it has fallen. %2")
							.arg(fallenAtom->Name())
							.arg(fallenAtom->SpottedLocation());
				}

				BecomeAlert(text);	// Wakes nearby troops.
			}
			else
				BecomeAlert();		// Become alert without saying anything.
		}
	}
	else
		SetAIIsAlert(false);	// Return to being complacent when no threats are detected.
}

void DaSentient::LookAtSubatoms(const DaAtom* parent, const DaAtom* commonParent, const AuMatrix4x4& myT,
		const AuPoint& eyeLoc, const AuPoint& eyeVec, float eyeAngle, float eyeRange, float noseRange, float earRange,
		float weaponRange,
		DaAtom*& itemAtom, AuPoint& itemDir, float& itemRange,
		DaAtom*& targetAtom, AuPoint& targetDir, float& targetRange,
		DaAtom*& threatAtom, AuPoint& threatDir, float& threatRange,
		DaAtom*& comradeAtom, AuPoint& comradeDir, float& comradeRange,
		DaAtom*& fallenAtom, AuPoint& fallenDir, float& fallenRange) const
{
	DaAtomList subs;
	parent->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom == this)
			continue;	// skip myself
		if(!pAtom->DoesQueryEnvironment())
			continue;
		if(pAtom->IsInAtmosphere() != IsInAtmosphere())
			continue;	// skip atoms that are not on the same side of the atmosphere envelope

		DaCreature* creature = dynamic_cast<DaCreature*>(pAtom);
		if(creature)
		{
			// A creature is present; could be a citizen, person, or animal.
			if(IsInAtmosphere())
			{
				// Make sure I can enter the creature's environment.
				// Some creatures can be reached by my weapon anyway so they get tested in a bit.
				if(creature->IsUnderGround())
				{
					if(!LivesUnderGround() || IsFlying())
						continue;	// I can't go underground.
				}
				//DHH This logic isn't right for citizens. There are no aquatic citizens yet.
				/*
				else
				{
					// Other creature is on the ground or in the water.
					if(!LivesInAir() && !LivesOnLand())
						continue;	// I can't go on land.
				}
				*/
			}

			AuMatrix4x4 targT;
			if(dynamic_cast<const DaSolarSystem*>(commonParent))
				targT = pAtom->EnvTransform();
			else
				pAtom->GetTransformRelativeToParent(targT, commonParent);
			AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to commonParent.
			loc = myT.UnProject(loc);	// Relative to me
			AuPoint sightVec = loc - eyeLoc;
			float sightDist = sightVec.Length();
			if(sightDist == 0)
				sightVec = AuPoint(0,1,0);	// In my eye?
			else
				sightVec /= sightDist;		// Make unit vector.
			bool smellIt = false;
			if(sightDist < noseRange)
			{
				if(creature->IsUnderGround())
					smellIt = false;	// under ground.
				else if(creature->IsCGSubmerged() && IsEyeSubmerged())
					smellIt = true;		// Both under water.
				else if(!creature->IsEyeSubmerged() && !IsEyeSubmerged())
					smellIt = true;		// Both not under water.
			}
			bool seeIt = false;
			if(sightDist < eyeRange)
			{
				// Restrict sight test to field of vision only when not alert.
				// Otherwise this creature will stop noticing a threat after it turns to run away.
				if(creature->IsUnderGround())
					seeIt = false;	// It is under ground.
				else
				{
					if(AIIsAlert())
						seeIt = true;
					else
					{
						float sightAngle = eyeVec.AngleTo(sightVec);
						if(sightAngle <= eyeAngle)
							seeIt = true;	// Inside F.O.V.
					}
				}
			}

			// Noise level of creature affects perception range with ears.
			bool hearIt = false;
			float noise = creature->NoiseLevel();
			if(sightDist < earRange * noise)
				hearIt = true;
			if(!seeIt && !smellIt && !hearIt)
				continue;	// Out of range to see, smell or hear.

			// This citizen "senses" the other creature.
			if(IsInAtmosphere())
			{
				// The range to the other creature is known.
				// Make sure I can reach the creature if it is not in my native environment.
				if(creature->IsInVisibleWater() && !LivesInWater() && sightDist > weaponRange)
					continue;									// I can't reach it in the water.
				else if(creature->IsFlying() && !IsFlyer() && sightDist > weaponRange)
					continue;									// I can't reach it in the air.
			}

			// Evaluate the other creature to determine if it is a threat or a comrade.
			bool isOtherDying = (creature->CurrentHitPoints() <= 0) ? true : false;
			bool isComrade = false;	// true if the other creature is a comrade, could by dying
			bool isThreat = false;	// true if the creature is a threat
			DaCitizen* citizen = dynamic_cast<DaCitizen*>(creature);
			if(citizen)
			{
				// The creature is a citizen.
				if(citizen->EmpireId() == EmpireId())
					isComrade = true;	// Citizen is a member of my empire.
				else if(!AssignedShip().IsNull() && citizen->AssignedShip() == AssignedShip())
					isComrade = true;	// Citizen is assigned to the same ship as I am.
				else if(!isOtherDying)
				{
					// Not a comrade.
					// Everyone is a threat when I'm berserk.
					// Only non-civilians are threats when I am not berserk.
					if(Posture() == POSTURE_Berserk || !citizen->IsCivilian())
					{
						// Citizen is a threat.
						POLITICAL_STANCE stance = PoliticalStanceMakeContact(citizen);
						if(stance == POL_Enemy)
							isThreat = true;
					}
				}
			}
			else
			{
				DaPerson* person = dynamic_cast<DaPerson*>(creature);
				if(person)
				{
					// The creature is a player character.
					// Ignore people that are not sightable as a target. i.e. waiting to spawn elsewhere or not on line
					if(!person->IsSightTarget(this))
						continue;

					if(person->EmpireId() == EmpireId())
						isComrade = true;
					else if(!AssignedShip().IsNull() && person->SpawnShip() == AssignedShip())
						isComrade = true;
					else if(!isOtherDying)
					{
						// Not a comrade. Is the person a threat?
						if(person->EmpireId())
						{
							POLITICAL_STANCE stance = PoliticalStanceMakeContact(person);
							if(stance == POL_Enemy)
								isThreat = true;
						}
						else if(EmpireId())
							isThreat = true;	// Free people are a threat to civilized people.
					}
				}
				else if(!isOtherDying)
				{
					DaAnimal* animal = dynamic_cast<DaAnimal*>(creature);
					if(animal)
					{
						// The creature is an animal.
						if(animal->IsWild())
						{
							// Wild animals are threats when they do certain things.
							//if(animal->IsActivityThreatening())
							if(!animal->IsUnderGround())
							{
								// The animal is doing threatening things.
								if(sightDist < ANIMAL_RANGE)
									isThreat = true;
							}
						}
						else if(animal->EmpireId() != EmpireId() && (!animal->IsCivilian() || Posture() == POSTURE_Berserk))
						{
							// Animal belongs to a different empire and it is mounted or I am berserk.
							POLITICAL_STANCE stance = PoliticalStanceMakeContact(animal);
							if(stance == POL_Enemy)
								isThreat = true;
						}
					}
				}
			}

			// The atom might be a comrade.
			if(isComrade)
			{
				// I might be able to help nearby fallen comrades.
				if(isOtherDying)
				{
					// The creature is a fallen comrade.
					if(fallenRange == 0 || sightDist < fallenRange)
					{
						// The creature is the nearest fallen comrade.
						fallenAtom = pAtom;
						fallenRange = loc.Length();
						if(fallenRange)
							fallenDir = loc/fallenRange;
						else
							fallenDir = AuPoint(0,1,0);
					}
				}
				else
				{
					// The creature is a comrade who is alive and kicking.
					if(comradeRange == 0 || sightDist < comradeRange)
					{
						// The creature is the nearest comrade.
						comradeAtom = pAtom;
						comradeRange = loc.Length();
						if(comradeRange)
							comradeDir = loc/comradeRange;
						else
							comradeDir = AuPoint(0,1,0);
					}
				}
			}

			// The atom might be a threat.
			if(isThreat)
			{
				if(threatRange == 0 || sightDist < threatRange)
				{
					// The creature is the nearest threat.
					threatAtom = pAtom;
					threatRange = loc.Length();
					if(threatRange)
						threatDir = loc/threatRange;
					else
						threatDir = AuPoint(0,1,0);
				}
			}

			// The atom might be the target of my orders.
			if(pAtom->Is(OrderTarget()))
			{
				// The creature is the target atom.
				targetAtom = pAtom;
				targetRange = loc.Length();
				if(targetRange)
					targetDir = loc/targetRange;
				else
					targetDir = AuPoint(0,1,0);
			}
		}
		else
		{
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(pAtom);
			if(vehicle)
			{
				// A vehicle is present; could be considered a threat or my target, or just an interesting item.
				if(IsInAtmosphere())
				{
					// Make sure I can enter the vehicle's environment.
					if(vehicle->IsUnderGround())
					{
						if(!LivesUnderGround() || IsFlying())
							continue;	// I can't go underground.
					}
					else if(vehicle->IsInVisibleWater())
					{
						if(!LivesInWater())
							continue;	// I can't go in the water.
					}
					else if(vehicle->IsFlying())
					{
						if(!IsFlyer())
							continue;	// I can't fly.
					}
					else
					{
						// Vehicle is on the ground.
						if(!LivesInAir() && !LivesOnLand())
							continue;	// I can't go on land.
					}
				}

				AuMatrix4x4 targT;
				if(dynamic_cast<const DaSolarSystem*>(commonParent))
					targT = vehicle->EnvTransform();
				else
					vehicle->GetTransformRelativeToParent(targT, commonParent);
				AuPoint loc = targT.Project(vehicle->TargetLocation());	// Relative to commonParent.
				loc = myT.UnProject(loc);								// Relative to me
				AuPoint sightVec = loc - eyeLoc;
				float sightDist = sightVec.Length();
				if(sightDist == 0)
					sightVec = AuPoint(0,1,0);	// In my eye?
				else
					sightVec /= sightDist;	// Make unit vector.
				bool smellIt = false;
				if(sightDist < noseRange)
				{
					if(vehicle->IsUnderGround())
						smellIt = false;	// under ground
					else if(vehicle->IsCGSubmerged() && IsEyeSubmerged())
						smellIt = true;		// Both under water.
					else if(!vehicle->IsEyeSubmerged() && !IsEyeSubmerged())
						smellIt = true;	// Both not under water.
				}
				bool seeIt = false;
				if(sightDist < eyeRange)
				{
					// Restrict sight test to field of vision
					// only when not alert.
					// Otherwise this creature will stop
					// noticing a threat after it turns to run
					// away.
					if(vehicle->IsUnderGround())
						seeIt = false;	// It is under ground.
					else
					{
						if(AIIsAlert())
							seeIt = true;
						else
						{
							float sightAngle = eyeVec.AngleTo(sightVec);
							if(sightAngle <= eyeAngle)
								seeIt = true;	// In F.O.V.
						}
					}
				}

				// Noise level of vehicle affects perception range with ears.
				bool hearIt = false;
				float noise = vehicle->NoiseLevel();
				if(sightDist < earRange * noise)
					hearIt = true;
				if(!seeIt && !smellIt && !hearIt)
					continue;	// Out of range to see, smell or hear.

				// Is the vehicle a threat.
				if(vehicle->EmpireId() != EmpireId() && vehicle->IsOccupied())
				{
					// An occupied vehicle of a different empire. Is the vehicle a threat?
					POLITICAL_STANCE stance = PoliticalStanceMakeContact(vehicle);
					if(stance == POL_Enemy)
					{
						if(threatRange == 0 || sightDist < threatRange)
						{
							// The vehicle is the nearest threat.
							threatAtom = pAtom;
							threatRange = loc.Length();
							if(threatRange)
								threatDir = loc/threatRange;
							else
								threatDir = AuPoint(0,1,0);
						}
					}
				}

				// The vehicle might be the target of my orders.
				if(pAtom->Is(OrderTarget()))
				{
					// The creature is the target atom.
					targetAtom = pAtom;
					targetRange = loc.Length();
					if(targetRange)
						targetDir = loc/targetRange;
					else
						targetDir = AuPoint(0,1,0);
				}
			}
			else
			{
				const DaItem* item = dynamic_cast<const DaItem*>(pAtom);
				if(item)
				{
					// An item is present. This citizen "sees" the item.
					// Notice ammo in case I run out.
					bool isUseful = false;
					const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(item->LockItem());
					if(ammo && !DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
					{
						// Check to see if the ammo fits my current weapon.
						// Citizens always carry their current weapon in their dominant hand.
						AuGearPos gearPos(CurrentItemPos());
						if(gearPos.IsGear())
						{
							const AuGear* gear = LockGearForRead();
							if(gear)
							{
								const AuItem* gearItem = gear->ItemAt(gearPos);
								if(gearItem)
								{
									const AuItemWeaponAmmo* weapon = dynamic_cast<const AuItemWeaponAmmo*>(gearItem);
									if(weapon && weapon->Ammo1Type() == ammo->AmmoType())
										isUseful = true;	// The item is ammo for my weapon.
								}
							}
							UnlockGear();
						}
					}
					item->UnlockItem();

					if(isUseful)
					{
						AuMatrix4x4 targT;
						if(dynamic_cast<const DaSolarSystem*>(commonParent))
							targT = pAtom->EnvTransform();
						else
							pAtom->GetTransformRelativeToParent(targT, commonParent);
						AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to commonParent.
						loc = myT.UnProject(loc);								// Relative to me
						AuPoint sightVec = loc - eyeLoc;
						float sightDist = sightVec.Length();
						if(sightDist == 0)
							sightVec = AuPoint(0,1,0);	// In my eye?
						else
							sightVec /= sightDist;	// Make a unit vector.
						bool smellIt = false;
						if(sightDist < noseRange)
						{
							// Can only smell inside atmosphere.
							if(IsInAtmosphere())
								smellIt = true;
						}
						bool seeIt = false;
						if(sightDist < eyeRange)
						{
							// Restrict sight test to field of vision only when not alert.
							// Otherwise this creature might stop noticing items.
							if(AIIsAlert())
								seeIt = true;
							else
							{
								float sightAngle = eyeVec.AngleTo(sightVec);
								if(sightAngle <= eyeAngle)
									seeIt = true;	// Inside field of view.
							}
						}
						if(!seeIt && !smellIt)
							continue;	// Out of range to see or smell.
						if(itemRange == 0. || sightDist < itemRange)
						{
							// The item is the nearest item.
							itemAtom = pAtom;
							itemRange = loc.Length();
							if(itemRange)
								itemDir = loc/itemRange;
							else
								itemDir = AuPoint(0,1,0);
						}
					}
				}
			}
		}
	}
}

bool DaSentient::LookForThreats(bool returnToPost)
{
	m_SentientLock.lockForWrite();
	m_tmLastLook.restart();
	m_SentientLock.unlock();

	// Be on the look-out for threats.
	if(Posture() == POSTURE_Offensive || Posture() == POSTURE_Berserk)
	{
		DaAtom* itemAtom;
		AuPoint itemDir;
		float itemRange;
		DaAtom* targetAtom;
		AuPoint targetDir;
		float targetRange;
		DaAtom* threatAtom;
		AuPoint threatDir;
		float threatRange;
		DaAtom* comradeAtom;
		AuPoint comradeDir;
		float comradeRange;
		DaAtom* fallenAtom;
		AuPoint fallenDir;
		float fallenRange;
		Look(itemAtom, itemDir, itemRange,
			targetAtom, targetDir, targetRange,
			threatAtom, threatDir, threatRange,
			comradeAtom, comradeDir, comradeRange,
			fallenAtom, fallenDir, fallenRange);
		if(threatAtom)
		{
			if(returnToPost)
			{
				// Add a goto order to my list of orders. Orders are executed from last to first.
				// When finished attacking, the goto order will return me back to my current location.
				// After returning to my current location, I'll revert back to my default stay order.
				AmendOrder(ORDER_GoToTarget, ParentId(), Location(), 0);					// Goto where I am now.
			}

			// Add an attack order to my list of orders. Orders are executed from last to first.
			TakeOrderAmendment(ORDER_AttackTarget, threatAtom->Id(), AuPoint(0,0));	// Attack the target, with radio call.
			return true;
		}
	}
	else if(Posture() == POSTURE_DefendArea)
	{
		DaAtom* itemAtom;
		AuPoint itemDir;
		float itemRange;
		DaAtom* targetAtom;
		AuPoint targetDir;
		float targetRange;
		DaAtom* threatAtom;
		AuPoint threatDir;
		float threatRange;
		DaAtom* comradeAtom;
		AuPoint comradeDir;
		float comradeRange;
		DaAtom* fallenAtom;
		AuPoint fallenDir;
		float fallenRange;
		Look(itemAtom, itemDir, itemRange,
			targetAtom, targetDir, targetRange,
			threatAtom, threatDir, threatRange,
			comradeAtom, comradeDir, comradeRange,
			fallenAtom, fallenDir, fallenRange);
		if(threatAtom && threatRange < DEFEND_RANGE)
		{
			if(returnToPost)
			{
				// Add a goto order to my list of orders. Orders are executed from last to first.
				// When finished attacking, the goto order will return me back to my current location.
				// After returning to my current location, I'll revert back to my default stay order.
				AmendOrder(ORDER_GoToTarget, ParentId(), Location(), 0);					// Goto where I am now.
			}

			// Add an attack order to my list of orders.
			TakeOrderAmendment(ORDER_AttackTarget, threatAtom->Id(), AuPoint(0,0));
			return true;
		}
	}
	return false;
}

void DaSentient::LookForAmmoType(AMMOTYPE ammoType, DaItem*& itemAtom, AuPoint& itemDir, float& itemRange) const
{
	//qDebug("DaSentient::LookForAmmoType %d %s looking", Id().GetID(), (const char*)Name().toAscii());
	// Init return values.
	itemAtom = 0;
	itemRange = 0;

	if(IsInCapsule())
	{
		//qDebug("DaSentient::LookForAmmoType - %d Aboard ship. Can't see.", Id().GetID());
		// Citizen vision on board ship.
		const DaCapsule* ship = FindParentCapsuleInEnv();
		if(ship)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, ship);
			AuPoint eyeLoc = CameraOffset();
			//AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			// Scan the interior of the spacecraft.
			DaAtomList subs;
			ship->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(pAtom == this)
					continue;	// skip myself

				DaItem* pItem = dynamic_cast<DaItem*>(pAtom);
				if(!pItem)
					continue;
				const AuItemAmmo* item = dynamic_cast<const AuItemAmmo*>(pItem->LockItem());
				if(!item)
				{
					pItem->UnlockItem();
					continue;	// Item is not ammo.
				}
				if(item->AmmoType() == ammoType && !DbCommodity::IsEnergyWeapon(item->CommodityId()))
				{
					// An ammo item is present of the appropriate ammo type.
					AuMatrix4x4 targT;
					pAtom->GetTransformRelativeToParent(targT, ship);
					AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to ship.
					loc = myT.UnProject(loc);								// Relative to me
					AuPoint sightVec = loc - eyeLoc;
					float sightDist = sightVec.Length();
					if(sightDist > 100)
					{
						pItem->UnlockItem();
						continue;	// Out of pick-up range.
					}
					if(itemRange == 0. || sightDist < itemRange)
					{
						// The item is the nearest item.
						itemAtom = pItem;
						itemRange = loc.Length();
						if(itemRange)
							itemDir = loc/itemRange;
						else
							itemDir = AuPoint(0,1,0);
					}
				}
				pItem->UnlockItem();
			}
		}
	}
	else if(IsInAtmosphere())
	{
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, llworld);
			AuPoint eyeLoc = CameraOffset();
			//AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			DaAtomList subs;
			llworld->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(pAtom == this)
					continue;	// skip myself
				if(!pAtom->DoesQueryEnvironment())
					continue;

				DaItem* pItem = dynamic_cast<DaItem*>(pAtom);
				if(!pItem)
					continue;
				const AuItemAmmo* item = dynamic_cast<const AuItemAmmo*>(pItem->LockItem());
				if(!item)
				{
					pItem->UnlockItem();
					continue;	// Item is not ammo.
				}
				if(item->AmmoType() == ammoType && !DbCommodity::IsEnergyWeapon(item->CommodityId()))
				{
					// An ammo item is present of the appropriate ammo type.
					AuMatrix4x4 targT;
					pAtom->GetTransformRelativeToParent(targT, llworld);
					AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to planet.
					loc = myT.UnProject(loc);								// Relative to me
					AuPoint sightVec = loc - eyeLoc;
					float sightDist = sightVec.Length();
					if(sightDist > 100)
					{
						pItem->UnlockItem();
						continue;	// Out of pick-up range.
					}
					if(itemRange == 0. || sightDist < itemRange)
					{
						// The item is the nearest item.
						itemAtom = pItem;
						itemRange = loc.Length();
						if(itemRange)
							itemDir = loc/itemRange;
						else
							itemDir = AuPoint(0,1,0);
					}
				}
				pItem->UnlockItem();
			}
		}
	}
	else
	{
		qDebug("DaSentient::LookForAmmoType - %d Not in terrain or a spacecraft. Can't see.", Id().GetID());
	}
}

void DaSentient::LookForAmmoCID(COMMODITYID ammoCID, DaItem*& itemAtom, AuPoint& itemDir, float& itemRange) const
{
	//qDebug("DaSentient::LookForAmmoCID %d %s looking", Id().GetID(), (const char*)Name().toAscii());
	// Init return values.
	itemAtom = 0;
	itemRange = 0;

	if(IsInCapsule())
	{
		//qDebug("DaSentient::LookForAmmoCID - %d Aboard ship. Can't see.", Id().GetID());
		// Citizen vision on board ship.
		const DaCapsule* ship = FindParentCapsuleInEnv();
		if(ship)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, ship);
			AuPoint eyeLoc = CameraOffset();
			//AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			// Scan the interior of the spacecraft.
			DaAtomList subs;
			ship->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(pAtom == this)
					continue;	// skip myself

				DaItem* item = dynamic_cast<DaItem*>(pAtom);
				if(item && item->CommodityId() == ammoCID)
				{
					// An ammo item is present.
					AuMatrix4x4 targT;
					pAtom->GetTransformRelativeToParent(targT, ship);
					AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to planet.
					loc = myT.UnProject(loc);								// Relative to me
					AuPoint sightVec = loc - eyeLoc;
					float sightDist = sightVec.Length();
					if(sightDist > 100)
						continue;	// Out of pick-up range.
					if(itemRange == 0. || sightDist < itemRange)
					{
						// The item is the nearest item.
						itemAtom = item;
						itemRange = loc.Length();
						if(itemRange)
							itemDir = loc/itemRange;
						else
							itemDir = AuPoint(0,1,0);
					}
				}
			}
		}
	}
	else if(IsInAtmosphere())
	{
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, llworld);
			AuPoint eyeLoc = CameraOffset();
			//AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			DaAtomList subs;
			llworld->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(!pAtom->DoesQueryEnvironment())
					continue;

				DaItem* item = dynamic_cast<DaItem*>(pAtom);
				if(item && item->CommodityId() == ammoCID)
				{
					// An ammo item is present.
					AuMatrix4x4 targT;
					pAtom->GetTransformRelativeToParent(targT, llworld);
					AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to planet.
					loc = myT.UnProject(loc);								// Relative to me
					AuPoint sightVec = loc - eyeLoc;
					float sightDist = sightVec.Length();
					if(sightDist > 100)
						continue;	// Out of pick-up range.
					if(itemRange == 0. || sightDist < itemRange)
					{
						// The item is the nearest item.
						itemAtom = item;
						itemRange = loc.Length();
						if(itemRange)
							itemDir = loc/itemRange;
						else
							itemDir = AuPoint(0,1,0);
					}
				}
			}
		}
	}
	else
	{
		qDebug("DaSentient::LookForAmmoCID - %d Not in terrain or a spacecraft. Can't see.", Id().GetID());
	}
}

// ::OperateWeapon
void DaSentient::OperateFireControlConsole(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
	{
		// Crew at non-critical station watches for threats.
		if(LookForThreats(true))
			return;						// Threat spotted; pursue and destroy; return to my station.
	}

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	DpStationWeapon* console = dynamic_cast<DpStationWeapon*>(LockConsoleForRead());
	if(!console)
	{
		// My console hasn't been created yet.
		UnlockConsole();
		return;	// shouldn't happen
	}

	// Consoles cannot rotate when there is no power to the ship.
	if(ship->CapacitorLevel() <= 0)
	{
		UnlockConsole();
		TakeANap();
		return;
	}
	if(!ship->IsConsoleOperational(StationSysId()))
	{
		UnlockConsole();
		ExitConsole();
		return;
	}

	// Insure the weapon bay is ready to fire again. It may still be waiting to fire since it takes a couple of seconds to rearm.
	// No need to do lots of AI work while waiting.
	if(!ship->WeaponBayCanFire(console->SystemWhich()))
	{
		UnlockConsole();
		return;	// Just wait, no need to turn and bob rapidly while waiting for the weapon to cycle.
	}

	if(Posture() == POSTURE_HoldFire || ship->MissionOrderType() == AuMissionOrder::StandDown || !ship->SensorsWorking() ||
			ship->RepairSecondsRemaining() || ship->IsInSpacecraft())
	{
		// I'm ordered to hold my fire or the ship's sensors are not functioning or the ship is getting repairs.
		// Or the ship is aboard another ship.
		// Look alive but don't actually shoot the weapon.
		UnlockConsole();
		StayIdle();
		TakeANap();
		return;
	}

	if(ship->IsSpaceStation())
	{
		// Privately owned stations don't fire bay weapons unless a person is aboard online or there is an officer in command.
		if(ship->IsPrivatelyOwned() && !ship->HasFirstOfficer() && !ship->FindPersonOnLineRecursive())
		{
			// Don't fire.
			UnlockConsole();
			StayIdle();
			//g_Au.Log(QString("DaSentient::OperateFireControlConsole %1 %2 station can't fire, napping")
					//.arg(Id().GetID()).arg(Name()));
			TakeANap();
			return;
		}
	}
	else
	{
		// Spaceships don't fire bay weapons unless a person is aboard online or there is an officer in command.
		if(!ship->HasFirstOfficer() && !ship->FindPersonOnLineRecursive())
		{
			// Don't fire.
			UnlockConsole();
			StayIdle();
			//g_Au.Log(QString("DaSentient::OperateFireControlConsole %1 %2 ship can't fire, napping")
					//.arg(Id().GetID()).arg(Name()));
			TakeANap();
			return;
		}
	}

	// Don't do anything if a person is manning one of these consoles. This also notices command console in this mode.
	// This is only done if there is only one weapon bay. With more than one bay the decision is ambiguous.
	if(ship->WeaponBays() == 1 && ship->IsWeaponMannedByPerson())
	{
		UnlockConsole();
		StayIdle();
		return;
	}

	unsigned int unitsThatCanFire = ship->GetPowerConsumption(SYS_Weapon, console->SystemWhich());
	if(!unitsThatCanFire)
	{
		// Do random turning and looking behavior.
		UnlockConsole();
		StayIdle();
		TakeANap();
		return;
	}

	DaAtom* system = FindEnvironmentAtom();
	if(!system)
	{
		UnlockConsole();
		return;	// D:
	}

	// I may be operating a harvester.
	const DaAtom* targetAtom = 0;							// Target in range to shoot at, if we find one.
	const DaAtom* standDownAtom = 0;						// Target to ignore because they are standing down.
	bool blocked = false;
	AuPoint targetSysLoc;
	unsigned int unitsFired = unitsThatCanFire;				// Bay units fired. On harvesters, fire all available units.
	quint8 harvestIndex = 0;								// See global Harvest functions in audef.h
	COMMODITYID module = ship->SystemModule(SYS_Weapon, console->SystemWhich());
	bool isHarvester = DbCommodity::IsHarvesterModule(module);;
	if(isHarvester)
	{
		// Harvester is only operated while ship is executing a Harvest order.
		bool doHarvestOp = false;
		if(ship->MissionOrderType() == AuMissionOrder::HarvestOperationByBay ||
				ship->MissionOrderType() == AuMissionOrder::HarvestOperationByModule)
		{
			// Configure my bay for what the current order wants it to harvest.
			bool hasOp;
			AuHarvestOp op;
			if(ship->MissionOrderType() == AuMissionOrder::HarvestOperationByBay)
				hasOp = ship->MissionHarvestOperationByBay(console->SystemWhich(), op);
			else
				hasOp = ship->MissionHarvestOperationByModule(module, op);
			if(hasOp && op.AmountType() != 2)
			{
				// My bay has a harvest operation that is not idle/disabled. Is it satisfied?
				if(op.CommodityId() == CID_Hydrogen)
				{
					// Hydrogen needs fuel cell space.
					quint32 fuelQty = ship->FuelQuantity();
					quint32 fuelVolume = ship->FuelCapacity();
					if(fuelQty < fuelVolume)
					{
						// Fuel cells are not full.
						// How much does the operation want?
						quint32 amountWanted;
						if(op.AmountType() == 0)
							amountWanted = fuelVolume * op.Amount() / 100;
						else
							amountWanted = op.Amount();
						if(fuelQty < amountWanted)
						{
							// Found an unsatisfied harvest process.
							doHarvestOp = true;
						}
					}
				}
				else
				{
					// Harvested materials need hold space.
					// holdFill is 0 if this is the first materials harvest op in the list.
					quint32 holdQty = ship->CargoTotalVolume();
					quint32 holdVolume = ship->CargoCapacityVolume();
					if(holdQty < holdVolume)
					{
						// Hold is not full.
						// How much does the operation want?
						quint32 amountWanted;
						if(op.AmountType() == 0)
							amountWanted = holdVolume * op.Amount() / 100;
						else
							amountWanted = op.Amount();
						if(ship->CargoRawCount(op.CommodityId()) < amountWanted)
						{
							// Found an unsatisfied harvest process.
							doHarvestOp = true;
						}
					}
				}

				if(doHarvestOp)
				{
					// Get the harvestIndex value for the commodity the harvester is supposed to collect.
					if(module == CID_AtmosphereCondenserBayModule)
						harvestIndex = HarvestAtmosphereIndex(op.CommodityId());
					else if(module == CID_ProspectorBayModule)
						harvestIndex = HarvestProspectorIndex(op.CommodityId());
					else if(module == CID_SiphonBayModule)
						harvestIndex = HarvestSiphonIndex(op.CommodityId());
					else if(module == CID_WildcatterBayModule)
						harvestIndex = HarvestWildcatterIndex(op.CommodityId());
					else if(module == CID_ParticleCollectorBayModule)
						harvestIndex = HarvestParticleIndex(op.CommodityId());
					else
						doHarvestOp = false;

					if(doHarvestOp)
					{
						if(ship->BayWeaponPowerSetting(console->SystemWhich()) != harvestIndex)
							ship->SetBayWeaponPowerSetting(console->SystemWhich(), harvestIndex);

						// Could be null so gunner must choose.
						ship->SetBayWeaponTarget(console->SystemWhich(), ship->MissionOrderTarget());
						ship->SetModified();
					}
				}
			}
		}
		else if(ship->MissionOrderType() == AuMissionOrder::Harvest)
		{
			// Harvester power setting is the index of the commodity that is to be harvested.
			harvestIndex = ship->BayWeaponPowerSetting(console->SystemWhich());
			doHarvestOp = true;
		}

		if(doHarvestOp)
		{
			// Check for target that is locked on fire control station.
			AuId targetId = ship->BayWeaponTarget(console->SystemWhich());
			if(!targetId.IsNull())
			{
				// Search for the target blip.
				targetAtom = system->FindIdRecursive(targetId);
				if(targetAtom && !targetAtom->CanBeHarvested(module))
					targetAtom = 0;					// The target atom cannot be harvested.
			}

			if(!targetAtom)
			{
				// Console is not targetted on anything viable.
				float fireRange = ship->BayWeaponRange(console->SystemWhich());
				fireRange *= .75f;

				// Find the closest blip I can harvest.
				double targetDist = 0;
				system->FindClosestHarvesterTargetRecursive(module, ship->EnvLocation(), fireRange, targetAtom, targetDist);
			}

			if(targetAtom)
				targetSysLoc = targetAtom->WeaponLockLocation(ship->EnvLocation(), module);
		}
	}
	else if(module == CID_TractorBeamBayModule)
	{
		//DHH AI cannot operate tractor beam.
	}
	else if(module == CID_WeaponDeploymentBayModule)
	{
		//DHH AI cannot operate bomb bay.
	}
	else
	{
		// I don't like attacking targets at minimum power, which is the default for a weapon bay console.
		if(ship->BayWeaponPowerSetting(console->SystemWhich()) == 0)
		{
			// Fire at 100% power.
			DecideShipWeaponSetPower(console->SystemWhich());
			UnlockConsole();
			return;
		}

		float fireRange;
		if(module == CID_EnergyWeaponBayModule)
		{
			unitsFired = BayWeaponProjectors(unitsThatCanFire) *
				(ship->BayWeaponPowerSetting(console->SystemWhich()) + 1)/41;
			fireRange = ship->BayWeaponRange(console->SystemWhich());
		}
		else if(module == CID_WeaponDeploymentBayModule)
		{
			unitsFired = BayWeaponEjectorRacks(unitsThatCanFire) *
				(ship->BayWeaponPowerSetting(console->SystemWhich()) + 1)/41;
			fireRange = AttackRange();
		}
		else if(module == CID_WeaponLauncherBayModule)
		{
			unitsFired = BayWeaponLaunchers(unitsThatCanFire) *
				(ship->BayWeaponPowerSetting(console->SystemWhich()) + 1)/41;
			fireRange = AttackRange();
		}
		else// if(module == CID_BallisticWeaponBayModule)
		{
			unitsFired = BayWeaponCannons(unitsThatCanFire) *
				(ship->BayWeaponPowerSetting(console->SystemWhich()) + 1)/41;
			if(ship->IsInAtmosphere())
				fireRange = 5 * 500;	// Range the shell will travel in its 5 sec lifetime at 500 fps.
			else
				fireRange = AttackRange();
		}
		if(!unitsFired)
			unitsFired = 1;

		// Limit bay weapon firing range to the functioning sensor range.
		const float sensorRange = ship->SensorRange();
		fireRange = fireRange * .9f;
		if(fireRange > sensorRange)
			fireRange = sensorRange;

		//if(ship->IsPirate())
			//qDebug() << "DaSentient::OperateFireControlConsole" << Id() << Name() << "ship" << ship->Id() << ship->Name()
				//<< "lock" << ship->LockedBlip() << "missionTarget" << ship->MissionOrderTargetId()
				//<< "mission" << ship->Mission() << now;

		AuId gunnerWorldId;
		int gunnerDomain = ship->GunnerDomain(gunnerWorldId);

		// The current mission order may specify a target atom to attack or plunder.
		AuMissionOrder::TYPE t = ship->MissionOrderType();
		if(t == AuMissionOrder::Attack)
		{
			// When ordered, I will attack friendlies when in range.
			targetAtom = system->FindIdRecursive(ship->MissionOrderTarget());
			if(targetAtom && !targetAtom->IsShipWeaponTarget(ship->Id(), gunnerDomain, gunnerWorldId, 0, Posture(),
						ship->EnvLocation(), fireRange, targetSysLoc))
				targetAtom = 0;	// Target atom not found or it is obstructed/out of my field of fire.
			else if(targetAtom && IsNoncombatant())
			{
				// I can only attack the target if we are at the same world.
				if(FindParentWorldInEnv() != targetAtom->FindParentWorldInEnv())
					targetAtom = 0;	// Target atom is not at my world.
			}
		}
		else if(t == AuMissionOrder::Plunder)
		{
			// When ordered, I will plunder friendlies when in range.
			targetAtom = system->FindIdRecursive(ship->MissionOrderTarget());
			if(targetAtom)
			{
				if(targetAtom->IsShipWeaponTarget(ship->Id(), gunnerDomain, gunnerWorldId, 0, Posture(), ship->EnvLocation(),
							fireRange, targetSysLoc))
				{
					// Mission order target is something I can shoot with a bay weapon.
					// Plunder order needs to know if target is standing down.
					// Plunder order does not attack targets who are standing down.
					const DaVessel* target = dynamic_cast<const DaVessel*>(targetAtom);
					bool targetStandingDown;
					if(target)
						targetStandingDown = target->IsStandingDown();	// I will care if the target is standing down.
					else
						targetStandingDown = false;				// Target is not a ship or not in the system so it can't stand down.

					if(targetStandingDown)
					{
						// Plunder target is standing down.
						standDownAtom = targetAtom;			// Do not target this atom.
						targetAtom = 0;
					}
					else if(targetAtom && IsNoncombatant())
					{
						// I can only attack the target if we are at the same world.
						if(FindParentWorldInEnv() != targetAtom->FindParentWorldInEnv())
							targetAtom = 0;	// Target atom is not at my world.
					}
				}
				else
					targetAtom = 0;
			}
		}

		if(!targetAtom && !ship->LockedBlip().IsNull() && ship->LockedBlip() != ship->MissionOrderTarget())
		{
			// Did not find a specific target to attack.
			// Ship is locked on to a target that is not the mission order target, try it first.
			targetAtom = system->FindIdRecursive(ship->LockedBlip());
			if(targetAtom == standDownAtom)
				targetAtom = 0;
			if(targetAtom)
			{
				if(ship->IsDamagedBy(targetAtom->Id()))
				{
					if(!targetAtom->IsShipWeaponTarget(ship->Id(), gunnerDomain, gunnerWorldId, EmpireId(), Posture(),
								ship->EnvLocation(), fireRange, targetSysLoc))
						targetAtom = 0;	// Target atom not found or it is obstructed/out of my field of fire.
				}
				else
				{
					// Target atom has not hurt me.
					if(targetAtom->IsShipWeaponTarget(ship->Id(), gunnerDomain, gunnerWorldId, EmpireId(), Posture(),
								ship->EnvLocation(), fireRange, targetSysLoc))
					{
						// The blip is a fair target unless I'm in DefendSelf posture and it hasn't done me any damage.
						if(Posture() == POSTURE_DefendSelf && targetAtom->AtomType() != ATOM_Asteroid)
							targetAtom = 0;	// Target atom has not hurt me and its not an asteroid.
						else if(targetAtom && IsNoncombatant())
						{
							// I can only attack the target if we are at the same world.
							if(FindParentWorldInEnv() != targetAtom->FindParentWorldInEnv())
								targetAtom = 0;	// Target atom is not at my world.
						}
					}
					else
						targetAtom = 0;	// Target atom not found or it is obstructed/out of my field of fire.
				}
			}
		}

		if(!targetAtom)
		{
			// Not attacking a specific target or locked-on target so check all sensor blips.
			double targetRange = 0;
			system->FindClosestShipWeaponTargetRecursive(ship->Id(), gunnerDomain, gunnerWorldId, EmpireId(), Posture(),
					ship->EnvLocation(), fireRange, targetAtom, targetRange, targetSysLoc);
			if(targetAtom == standDownAtom)
				targetAtom = 0;
			if(targetAtom)
			{
				// The blip is a fair target unless I'm in DefendSelf posture and it hasn't done me any damage.
				if(Posture() == POSTURE_DefendSelf && !ship->IsDamagedBy(targetAtom->Id()) &&
						targetAtom->AtomType() != ATOM_Asteroid)
					targetAtom = 0;	// Target atom has not hurt me and its not an asteroid.
				else if(targetAtom && IsNoncombatant())
				{
					// I can only attack the target if we are at the same world.
					if(FindParentWorldInEnv() != targetAtom->FindParentWorldInEnv())
						targetAtom = 0;	// Target atom is not at my world.
				}
			}
		}

		// If we have a target, check sight path before firing.
		if(targetAtom)
		{
			const AuMatrix4x4& shipsys = ship->EnvTransform();
			AuMatrix4x4 tModel = shipsys.Inverse();
			unsigned int sightedPart = 0;
			bool sightedShield = false;
			AuPoint sightedLoc(shipsys.UnProject(targetSysLoc));
			DaAtom* sightedAtom = system->SightTarget(ship, Location(), tModel, 0, sightedLoc, sightedPart, sightedShield, 0);
			//if(sightedAtom)
				//qDebug() << "DaSentient::OperateFireControlConsole" << Id() << Name() << "ship" << ship->Id() << ship->Name()
					//<< "sightedAtom" << sightedAtom->Id() << sightedAtom->Name()
					//<< "target" << targetAtom->Id() << targetAtom->Name();
			if(sightedAtom != targetAtom)
			{
				blocked = true;	// Don't take a nap.
				targetAtom = 0;
			}
		}
	}

	if(targetAtom)
	{
		// We have found a target in range to shoot at.
		// Insure weapon bay has valid ammo selected.
		DecideShipWeaponFire(console->SystemWhich(), unitsFired, targetAtom->Id(), targetSysLoc, isHarvester, harvestIndex);
		UnlockConsole();
		RestartAIAttackTime();
	}
	else
	{
		// Do random turning and looking behavior.
		UnlockConsole();
		StayIdle();
		if(!standDownAtom && !blocked)
			TakeANap();
	}
}

void DaSentient::OperateSensorConsole(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
	{
		// Crew at non-critical station watches for threats.
		if(LookForThreats(true))
			return;						// Threat spotted; pursue and destroy; return to my station.
	}

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate when there is no power to the ship.
	if(ship->CapacitorLevel() <= 0)
		return;
	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Don't do anything if a person is manning one of these consoles. This also notices command console in this mode.
	// Don't do anything if the ship is getting repairs.
	if(ship->IsSensorMannedByPerson() || ship->RepairSecondsRemaining())
	{
		StayIdle();
		return;
	}

	// Turn on the sensors, if the ship has any and if they are not already on.
	AuMissionOrder::TYPE t = ship->MissionOrderType();
	if(ship->IsMoored())
		DoShipSensorsState(ship, false);	// Turn off sensor when moored.
	else if(t == AuMissionOrder::Hide && ship->IsAtRest() && ship->IsOnGround())
		DoShipSensorsState(ship, false);	// Turn off sensor when hiding.
	else
		DoShipSensorsState(ship, true);

	// Pirate stations never need to lock on to targets.
	if(ship->IsPirateWithFreeWill() && ship->IsSpaceStation())
	{
		StayIdle();
		return;
	}

	if(!ship->SensorsWorking())
	{
		// The ship's sensors are not functioning.
		// Look alive but don't actually lock onto targets.
		StayIdle();
		return;
	}

	// Determine the functioning sensor range.
	const float sensorRange = ship->SensorRange();
	if(sensorRange <= 0)
	{
		StayIdle();
		return;
	}

	// Report changes to enemy sensor contact status.
	// Pirates do not report sensor contacts.
	if(!ship->IsPirateWithFreeWill())
	{
		// Every few seconds is often enough to do this.
		m_SentientLock.lockForRead();
		int lastScanSecs = m_tmLastScan.ElapsedSec();
		m_SentientLock.unlock();
		if(lastScanSecs >= 3)
		{
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
			if(!system ||	// Not in system implies I am in an arena.
					system->IsEnemyEmpirePresent(EmpireId()) ||	// Not in arena and enemy empire present.
					(lastScanSecs > 60 && ship->FindOfficerOnLineRecursive()))	// Check for anomalies.
			{
				// Enemy units are in the system or it has been a while since I did a scan and an officer is aboard.
				// Do the hard work of a scan so enemy units and anomalies can be reported, if we scan them.
				AuSensorBlipList sensorBlips;
				ship->PerformSensorConsoleScan(sensorBlips);
				DecideShipSensorsReportContacts(sensorBlips, sensorRange);

				m_SentientLock.lockForWrite();
				m_tmLastScan.restart();
				m_SentientLock.unlock();
			}
		}
	}

	// Don't bother locking on to targets when not in any scene.
	if(!IsInAnyScene())
	{
		// Empires that are not pirates do not need sensor lock-on when not in any scene.
		if(!ship->IsPirateWithFreeWill())
			return;

		// This is a pirate ship that is not in any scene.
		// Pirate captain depends on me to lock-on to a target to hunt at key times.
		// Pirate captain only looks for targets to hunt at the conclusion of certain orders.
		if(!(t == AuMissionOrder::Attack ||
				t == AuMissionOrder::GoToOrbitCity ||
				t == AuMissionOrder::GoToSystemDirect ||
				t == AuMissionOrder::GoToSystemDirect_Deprecated ||
				t == AuMissionOrder::GoToSystemViaWormholes ||
				t == AuMissionOrder::PatrolSystem ||
				t == AuMissionOrder::Repair))
			return;

		// Pirate captain does not care to chase targets unless ship is in good condition. Ignore fire.
		if(ship->AlertConditionOfStructure() == ConditionRed || ship->AlertConditionOfSystems() == ConditionRed)
			return;
	}

	// Don't change the sensor lock when it was acquired by a human.
	// The ship will clear the 'human-locked' flag when the blip goes out of range.
	if(ship->IsSensorLockedByHuman() && !ship->IsPirateWithFreeWill())
	{
		// Must not be locked on to a target when standing down. Sorry, this overrides human lockon.
		//if(t == AuMissionOrder::StandDown)
			//ship->LockOn(AuId(0), false);
		StayIdle();
		return;
	}

	// Don't lock-on to targets when holding fire. This may be detected as a provocative action.
	if(Posture() == POSTURE_HoldFire)// || t == AuMissionOrder::StandDown)
	{
		DoShipSensorsLock(ship, AuId(0));
		StayIdle();
		return;
	}

	const DaAtom* system = ship->FindEnvironmentAtom();
	if(!system)
		return;

	// Look for the attack/follow target in the sensor data.
	AuPoint shipSysLoc = ship->EnvLocation();
	if(t == AuMissionOrder::Attack ||
			t == AuMissionOrder::Plunder ||
			t == AuMissionOrder::Follow)
	{
		const DaAtom* targetAtom = system->FindIdRecursive(ship->MissionOrderTarget());
		if(targetAtom)
		{
			if(targetAtom->EnvLocation().Distance2(shipSysLoc) <= sensorRange*sensorRange)
			{
				// The target blip is in sensor range.
				// Lock-on to the target blip.
				DoShipSensorsLock(ship, targetAtom->Id());

				// Do random turning and looking behavior.
				StayIdle();
				return;
			}
		}
	}

	AuId gunnerWorldId;
	int gunnerDomain = GunnerDomain(gunnerWorldId);

	// Select a target to lock on to.
	const DaAtom* targetAtom = 0;
	if(!ship->IsPirateWithFreeWill())
	{
		// Priorities after attack/follow target: Enemy Ship or Station, Non-enemy Station, Any City.
		const DaVessel* enemySpacecraft = 0;
		double enemyRange = 0;
		const DaVessel* nonEnemyStation = 0;
		double stationRange = 0;
		const DaBd* enemyMilBd = 0;
		double milRange = 0;
		const DaBd* anyBd = 0;
		double bdRange = 0;
		system->FindClosestShipLockOnRecursive(gunnerDomain, gunnerWorldId, EmpireId(), ship->Id(), ship->EnvLocation(),
				sensorRange, enemySpacecraft, enemyRange, nonEnemyStation, stationRange, enemyMilBd, milRange, anyBd, bdRange);

		// Choose the best blip to lock-on to.
		// Noncombatants do not lock onto units that are not at the same world as them.
		if(IsNoncombatant())
		{
			const DaWorldLL* myWorld = FindParentWorldInEnv();
			if(enemySpacecraft && enemySpacecraft->FindParentWorldInEnv() == myWorld)
				targetAtom = enemySpacecraft;
			else if(enemyMilBd && enemyMilBd->FindParentWorldInEnv() == myWorld)
				targetAtom = enemyMilBd;
			else if(nonEnemyStation && nonEnemyStation->FindParentWorldInEnv() == myWorld)
				targetAtom = nonEnemyStation;
			else if(anyBd && anyBd->FindParentWorldInEnv() == myWorld)
				targetAtom = anyBd;
		}
		else
		{
			if(enemySpacecraft)
				targetAtom = enemySpacecraft;
			else if(enemyMilBd)
				targetAtom = enemyMilBd;
			else if(nonEnemyStation)
				targetAtom = nonEnemyStation;
			else if(anyBd)
				targetAtom = anyBd;
		}
	}
	else
	{
		// Pirates don't care about stations or cities.
		// Priorities after attack/follow target: Enemy Ship or Station.
		const DaVessel* enemySpacecraft = 0;
		double enemyRange = 0;
		const DaVessel* nonEnemyStation = 0;
		double stationRange = 0;
		const DaBd* enemyMilBd = 0;
		double milRange = 0;
		const DaBd* anyBd = 0;
		double bdRange = 0;
		system->FindClosestShipLockOnRecursive(gunnerDomain, gunnerWorldId, EmpireId(), ship->Id(), ship->EnvLocation(),
				sensorRange, enemySpacecraft, enemyRange, nonEnemyStation, stationRange, enemyMilBd, milRange, anyBd, bdRange);

		// Choose the best blip to lock-on to.
		// Pirates are never noncombatants.
		if(enemySpacecraft)
			targetAtom = enemySpacecraft;
	}
	if(targetAtom)
	{
		// Found a target to lock-on to.
		DoShipSensorsLock(ship, targetAtom->Id());
	}

	// Do random turning and looking behavior.
	StayIdle();
}

void DaSentient::OperateShieldConsole(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
	{
		// Crew at non-critical station watches for threats.
		if(LookForThreats(true))
			return;						// Threat spotted; pursue and destroy; return to my station.
	}

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate when there is no power to the ship.
	if(ship->CapacitorLevel() <= 0)
		return;
	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Don't do anything if a person is manning one of these consoles. This also notices command console in this mode.
	// Don't do anything if the ship is getting repairs.
	if(ship->IsShieldMannedByPerson() || ship->RepairSecondsRemaining())
	{
		StayIdle();
		return;
	}

	AuMissionOrder::TYPE t = ship->MissionOrderType();
	if(t == AuMissionOrder::StandDown)
	{
		// Shields down when standing down.
		if(ship->ShieldState() == DaVessel::SS_Up)
			DoShipShieldsState(ship, DaVessel::SS_Down, false, QObject::tr("Standing down."));
	}
	else if(t == AuMissionOrder::ShieldsDown)
	{
		// Shields down when ordered to lower them.
		if(ship->ShieldState() == DaVessel::SS_Up)
			DoShipShieldsState(ship, DaVessel::SS_Down, false, QObject::tr("As ordered."));
	}
	else if(ship->IsMoored())
	{
		// Shields off when moored.
		if(ship->ShieldState() != DaVessel::SS_Off)
			DoShipShieldsState(ship, DaVessel::SS_Off, false, QObject::tr("Mooring."));
	}
	else if(t == AuMissionOrder::Hide && ship->IsAtRest() && ship->IsOnGround())
	{
		// Shields off when hiding in ground clutter.
		if(ship->ShieldState() != DaVessel::SS_Off)
			DoShipShieldsState(ship, DaVessel::SS_Off, false, QObject::tr("Hiding in ground clutter."));
	}
	else
	{
		// Ship fuel status, shield policy and sensor scan information are used to determine whether the shields should be up.
		int why;
		QString threatName;
		DaVessel::SHIELD_STATE shieldState = ship->ShieldStateDesired(why, threatName);
		QString sayWhy;
		if(why >= 0 && why <= 5)
		{
			QString reasonsWhy[] =
			{
				QObject::tr("Ship is low on fuel."),
				QObject::tr("Shields are inoperative."),
				QObject::tr("Captain ordered shields up at all times."),
				QObject::tr("Captain ordered shields down at all times."),
				QObject::tr("The universe is broken."),
				QObject::tr("No threats detected."),
			};
			sayWhy = reasonsWhy[why];
		}
		else if(why == 6)
		{
			if(!threatName.isEmpty())
				sayWhy = QObject::tr("%1 detected.").arg(threatName);
			else
				sayWhy = QObject::tr("Threat detected.");
		}
		else
			sayWhy = QObject::tr("I had a whim.");

		if(shieldState != ship->ShieldState())
			DoShipShieldsState(ship, shieldState, true, sayWhy);	// Switch to new shield state.
		else if(shieldState == DaVessel::SS_Up)
			DoShipShieldsBalance(ship);								// Balance shields to even power distribution.
	}
	StayIdle();
}

void DaSentient::OperateSurgeryConsole(const DaCapsule* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleSeatOccupiedExcept(StationId(), 0, Id()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate when there is no power to the ship.
	//DHH if(ship->CapacitorLevel() <= 0)
		//DHH return;
	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	DaCreature* patient = ship->FindPatient(*this);
	if(patient)
	{
		// Found a patient.
		bool treatConditions = patient->HasTreatableConditions();	// true if we will treat conditions that exist
		bool isBandaged = patient->HasCondition(AuConBandage::ConditionType());
		if(patient->IsBodyDyingOrDead())
		{
			// Defibrillate the patient, STAT!
			DecideShipPatientDefib(patient->Id());

			treatConditions = false;	// Until next time through.
		}
		else if(patient->CurrentHitPoints() < patient->MaxHitPoints() && !isBandaged)
		{
			// Bandage the patient before performing any other procedures.
			// Locate a medical kit in the ship's inventory.
			const AuItemHash& sinv = ship->LockCargoForRead();
			AuItemHash::const_iterator ci = sinv.FindBest(CID_MedicalKit);
			if(ci != sinv.end())
			{
				ship->UnlockCargo();

				// Heal wounds.
				DecideShipPatientHeal(patient->Id());

				treatConditions = false;	// Until next time through.
			}
			else
			{
				// No medical kit.
				ship->UnlockCargo();

				// Give the patient a diagnosis.
				if(treatConditions)
				{
					// Patient has one or more conditions that need treatment.
					QStringList conditions;
					patient->ListTreatableConditions(conditions);
					QString conList = conditions[0];
					for(QStringList::const_iterator cni = conditions.cbegin(); cni != conditions.cend(); cni++)
					{
						if(cni == conditions.cend() - 1)
							conList += " and " + *cni;
						else
							conList += ", " + *cni;
					}
					QString text = QObject::tr("%1, I can't bandage your wounds; the pharmacy is out of medical kits. "
							"Treatment of your %2 is going to be risky.").arg(patient->Name()).arg(conList);
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(msg, AuVisage::Frown);
				}
				else
				{
					// Patient has no conditions that need treatment.
					QString text = tr("%1, I can't bandage your wounds; the pharmacy is out of medical kits. ")
						.arg(patient->Name());

					// Tell them about their pregnancy.
					const AuConPregnant* preg =
						dynamic_cast<const AuConPregnant*>(patient->FindConditionLock(AuConPregnant::ConditionType()));
					if(preg)
						text += preg->SayDescription(patient, false);
					else
						text += tr("You're healthy as a horse, otherwise.");
					patient->UnlockConditions();

					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(msg, AuVisage::Frown);

					// Take a little nap if I can't do anything for them, so the person doesn't make me waste a bunch of time.
					TakeANap();
				}
			}

			// Kick wild animal out of surgery unit when I can no longer help it.
			DaAnimal* animal = dynamic_cast<DaAnimal*>(patient);
			if(animal)
				animal->Stand();
		}
		else if(!treatConditions)
		{
			// Patient is unhurt and clean of all conditions.
			// Tell them about their pregnancy.
			QString text;
			const AuConPregnant* preg =
				dynamic_cast<const AuConPregnant*>(patient->FindConditionLock(AuConPregnant::ConditionType()));
			if(preg)
				text = preg->SayDescription(patient, true);
			else if(isBandaged)
				text = QObject::tr("%1, those bandages will dissolve when you are fully healed. "
						"Come back for a checkup any time.").arg(patient->Name());
			else
				text = QObject::tr("%1, you appear healthy. Come back for a checkup any time.").arg(patient->Name());
			patient->UnlockConditions();
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(msg, AuVisage::Closed);

			// Take a little nap if I can't do anything for them, so the person doesn't make me waste a bunch of time.
			TakeANap();

			// Kick wild animal out of surgery unit when I can no longer help it.
			DaAnimal* animal = dynamic_cast<DaAnimal*>(patient);
			if(animal)
				animal->Stand();
		}

		if(treatConditions)
			DecideShipPatientTreat(patient->Id());
	}
	else
	{
		// No patient. Take a short nap before looking again.
		TakeANap();
	}

	StayIdle();
}

void DaSentient::TreatPatientConditions(DaCreature* patient, DaCapsule* capsule)
{
	if(!HasConsole())
		return;

	// Must clone conditions to avoid deadlock when treatment removes them.
	AuConditionList conditions;
	const AuConditionList& pcond = patient->LockConditions();
	for(AuConditionList::const_iterator ci = pcond.cbegin(); ci != pcond.cend(); ci++)
		conditions.append((*ci)->Clone());
	patient->UnlockConditions();

	// Check for a condition that I can treat.
	// Up to this point, pregnancy has not been mentioned for a patient who has any conditions.
	bool anyTreated = false;	// true if I treat any conditions.
	for(AuConditionList::const_iterator ci = conditions.cbegin(); ci != conditions.cend() && !anyTreated; ci++)
	{
		switch((*ci)->CType())
		{
		case AuCondition::Poison:
			{
				// Locate an antidote in the capsule's inventory.
				const AuItemHash& sinv = capsule->LockCargoForRead();
				AuItemHash::const_iterator ai = sinv.FindBest(CID_Antidote);
				if(ai == sinv.end())
				{
					capsule->UnlockCargo();

					QString text = QObject::tr("%1, I can't treat your %2; the pharmacy is out of antidotes.")
						.arg(patient->Name()).arg((*ci)->Name());
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(msg, AuVisage::Frown);

					// Failure is reported to the captain for annotation in the log.
					text = QObject::tr("%1 needs treatment for %2 but the pharmacy is out of antidotes.")
						.arg(patient->Name()).arg((*ci)->Name());
					capsule->LogEvent(AuEventLogEntry::Poison, text);
					continue;	// No antidote found.
				}
				AuItem* antidote = (*ai).second->Clone();
				capsule->UnlockCargo();

				// Treat the poison.
				QString text = QObject::tr("%1, this antidote will treat your %2.").arg(patient->Name())
					.arg((*ci)->Name());
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Closed);
				DpStationSurgery::DoAntidote(this, patient, *capsule, *ci, *antidote);
				delete antidote;
				anyTreated = true;

				// Successful poisoning treatment is not reported to the captain.
			}
			break;
		case AuCondition::Disease:
			{
				// Locate an antitoxin in the capsule's inventory.
				const AuItemHash& sinv = capsule->LockCargoForRead();
				AuItemHash::const_iterator ai = sinv.FindBest(CID_Antitoxin);
				if(ai == sinv.end())
				{
					capsule->UnlockCargo();

					QString text = QObject::tr("%1, I can't treat your %2; the pharmacy is out of antitoxins.")
						.arg(patient->Name()).arg((*ci)->Name());
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(msg, AuVisage::Frown);

					// Failure is reported to the captain for annotation in the log.
					text = QObject::tr("%1 needs treatment for %2 but the pharmacy is out of antitoxins.")
						.arg(patient->Name()).arg((*ci)->Name());
					capsule->LogEvent(AuEventLogEntry::Disease, text);
					continue;	// No antitoxin found.
				}
				AuItem* antitoxin = (*ai).second->Clone();
				capsule->UnlockCargo();

				// Disease is reported to the captain for annotation in the log.
				QString text = QObject::tr("%1 was treated for %2.").arg(patient->Name()).arg((*ci)->Name());
				capsule->LogEvent(AuEventLogEntry::Disease, text);

				// Treat the disease.
				text = QObject::tr("%1, this antitoxin will treat your %2.").arg(patient->Name())
					.arg((*ci)->Name());
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Closed);
				DpStationSurgery::DoAntitoxin(this, patient, *capsule, *ci, *antitoxin);
				delete antitoxin;
				anyTreated = true;
			}
			break;
		case AuCondition::Paralysis:
			{
				// Treat the paralysis.
				QString text = QObject::tr("%1, rigorous physical therapy will treat your %2.")
					.arg(patient->Name()).arg((*ci)->Name());
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Closed);
				DpStationSurgery::DoPhysicalTherapy(this, patient, *ci);
				anyTreated = true;

				// Successful paralysis treatment is not reported to the captain.
			}
			break;
		case AuCondition::ImplantDNA:
		case AuCondition::ImplantSpecies:
			{
				// Remove the parasite implant.
				QString text = QObject::tr("%1, surgery is needed to remove your %2.")
					.arg(patient->Name()).arg((*ci)->Name());
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Grim);
				DpStationSurgery::DoSurgery(this, patient, *ci);
				anyTreated = true;

				// Successful parasite implant treatment is not reported to the captain.
			}
			break;
		case AuCondition::Acid:
			{
				// Locate water in the capsule's inventory.
				const AuItemHash& sinv = capsule->LockCargoForRead();
				AuItemHash::const_iterator ai = sinv.FindBest(CID_Water);
				if(ai == sinv.end())
				{
					capsule->UnlockCargo();

					QString text = QObject::tr("%1, I can't treat your %2; the pharmacy is out of water.")
						.arg(patient->Name()).arg((*ci)->Name());
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(msg, AuVisage::Frown);

					// Failure is reported to the captain for annotation in the log.
					text = QObject::tr("%1 needs treatment for %2 but the pharmacy is out of water.")
						.arg(patient->Name()).arg((*ci)->Name());
					capsule->LogEvent(AuEventLogEntry::Medical, text);
					continue;	// No water found.
				}
				AuItem* water = (*ai).second->Clone();
				capsule->UnlockCargo();

				// Treat the acid.
				QString text = QObject::tr("%1, this water will decontaminate your %2.").arg(patient->Name())
					.arg((*ci)->Name());
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Closed);
				DpStationSurgery::DoDecontaminate(this, patient, *capsule, *ci, *water);
				delete water;
				anyTreated = true;

				// Successful acid treatment is not reported to the captain.
			}
			break;
		case AuCondition::Fire:
			{
				// Locate water in the capsule's inventory.
				const AuItemHash& sinv = capsule->LockCargoForRead();
				AuItemHash::const_iterator ai = sinv.FindBest(CID_Water);
				if(ai == sinv.end())
				{
					capsule->UnlockCargo();

					QString text = QObject::tr("%1, I can't douse your %2; the pharmacy is out of water. "
							"You should stop, drop and roll.")
						.arg(patient->Name()).arg((*ci)->Name());
					AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(msg, AuVisage::Frown);

					// Failure is reported to the captain for annotation in the log.
					text = QObject::tr("%1 is on %2 but the pharmacy is out of water.")
						.arg(patient->Name()).arg((*ci)->Name());
					capsule->LogEvent(AuEventLogEntry::Medical, text);
					continue;	// No water found.
				}
				AuItem* water = (*ai).second->Clone();
				capsule->UnlockCargo();

				// Extinguish the fire.
				QString text = QObject::tr("%1, this water will put out that %2.").arg(patient->Name())
					.arg((*ci)->Name());
				AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Closed);
				DpStationSurgery::DoExtinguish(this, patient, *capsule, *ci, *water);
				delete water;
				anyTreated = true;

				// Successful fire treatment is not reported to the captain.
			}
			break;
		case AuCondition::Antitoxin:
		case AuCondition::Bandage:
		case AuCondition::Bounty:
		case AuCondition::UNUSED:
		case AuCondition::Invalid:
		case AuCondition::Pregnant:
		case AuCondition::Thought:
			break;
		}
	}

	// Take a little nap if I can't do anything for them, so the person doesn't make me waste a bunch of time.
	if(!anyTreated)
	{
		// Tell them about their pregnancy.
		const AuConPregnant* preg = dynamic_cast<const AuConPregnant*>(conditions.Condition(AuConPregnant::ConditionType()));
		if(preg)
		{
			QString text = preg->SayDescription(patient, true);
			AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(msg, AuVisage::Smile);
		}

		TakeANap();
	}

	// Clean up our cloned conditions list.
	conditions.Clear();
}

// This one engineer is expected to operate both FTL drives, if so equipped.
void DaSentient::OperateEngineerConsole(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
	{
		// Crew at non-critical station watches for threats.
		if(LookForThreats(true))
			return;						// Threat spotted; pursue and destroy; return to my station.
	}

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate when there is no power to the ship.
	bool canTurn = (ship->CapacitorLevel() > 0) ? true : false;

	// Don't do anything if a person is manning one of these consoles. This also notices command console in this mode.
	// Don't do anything if the ship is getting repairs.
	if(ship->IsEngineerMannedByPerson() || ship->RepairSecondsRemaining())
	{
		if(canTurn)
			StayIdle();
		return;
	}

	// Operate power plant and gravity drive.
	// The power plant and gravity drive controls duplicate helm controls.
	// Don't override the pilot, whether it is human or AI.
	bool hasPilot = ship->IsPilotManned();
	AuMissionOrder::TYPE t = ship->MissionOrderType();
	if(!hasPilot)
	{
		// Helm is not manned by human or AI.
		if(ship->IsMoored())
		{
			if(ship->IsAtRest())
			{
				// Moored so turn off power plant and gravity drive.
				if(ship->GravityDriveState() != DaVessel::GD_Off)
				{
					// Turn off the gravity drive if it's on.
					DoShipGravityDriveState(ship, DaVessel::GD_Off);
				}
				else if(ship->IsPowerPlantOn())
				{
					// Turn off the power plant if it's on.
					DoShipPowerPlantState(ship, false);
				}
			}
			else if(!ship->IsOnGround())
			{
				// Still moving so insure power plant and gravity drive are on.
				if(!ship->IsPowerPlantOn() && ship->FuelQuantity() > 0)
				{
					// Turn on the power plant if it's not already on.
					DoShipPowerPlantState(ship, true);
				}
				else if(canTurn)
				{
					// Turn on the gravity drive if it's not already on.
					DoShipGravityDriveState(ship, DaVessel::GD_On);
					// Engineer doesn't really have throttle control but let's not blow up the ship.
					DoShipKillThrottles(ship);
				}
			}
		}
		else if(ship->IsSpaceStation())
		{
			// I'm at the engineer console of a station.
			// Stations must have their power plant and gravity drive on at all times.
			if(!ship->IsPowerPlantOn() && ship->FuelQuantity() > 0)
			{
				// Turn on the power plant if it's not already on.
				DoShipPowerPlantState(ship, true);
			}
			else if(canTurn)
			{
				// Turn on the gravity drive if it's not already on.
				DoShipGravityDriveState(ship, DaVessel::GD_On);
				// Engineer doesn't really have throttle control but let's not blow up the ship.
				DoShipKillThrottles(ship);
			}
		}
		else if(!ship->IsInAtmosphere())
		{
			// I'm at the engineer console of a spacecraft that is not in atmosphere range of a planet.
			if(!ship->IsPowerPlantOn() && ship->FuelQuantity() > 0)
			{
				// Turn on the power plant if it's not already on.
				DoShipPowerPlantState(ship, true);
			}
		}
		else
		{
			// I'm at the engineer console of a spacecraft that is in atmosphere range of a planet.
			if(t == AuMissionOrder::Hide && ship->IsAtRest() && ship->IsOnGround())
			{
				// Ship order is to hide in ground clutter.
				DoShipPowerPlantState(ship, false);
			}
			else if(!ship->IsOnGround() && ship->FuelQuantity() > 0)
			{
				// Ship is not hiding in ground clutter and not on the ground.
				DoShipPowerPlantState(ship, true);
			}
		}
	}

	// Operate FTL drive.
	if(ship->IsMoored())
		DoShipFTLDriveState(ship, false, ControlId());	// Turn off FTL drive when moored.
	else if(ship->IsInAtmosphere())
		DoShipFTLDriveState(ship, false, ControlId());	// Turn off FTL within atmo range of a planet. Don't operate it.
	else if(ship->IsFTLDriveHot(ControlId()))
	{
		// FTL station's drive is hot.
		if(DbCommodity::IsWarpFTLDriveModule(ship->SystemModule(SYS_FTLDrive, ControlId())) &&
				t != AuMissionOrder::Follow &&
				!ship->HasWaypoint() &&
				!ship->IsPilotMannedByPerson())
		{
			// Warp FTL drive is hot, we're not following somebody, no waypoint is set, and a human is not piloting the ship.
			// Turn off the warp drive to terminate the warp speed effect.
			DoShipFTLDriveState(ship, false, ControlId());
		}
	}
	else if(hasPilot && !ship->MissionOrderIsPatrol() && !ship->IsOnGround())
	{
		// The ship has a pilot; I am not patrolling and my FTL drive is not cycling. Should I engage it?
		// Engineer no longer engages the FTL drive, just insures it is on.
		const DeWaypoint* wp = ship->LockWaypoint(0);
		if(wp)
		{
			AuId wpSystem(wp->m_idSystem);
			AuId wpDest(wp->Destination());
			AuPoint wpDestOff(wp->DestinationOffset());
			ship->UnlockWaypoint();

			// The ship has a waypoint destination to go to. Can I use the FTL drive to get there?
			if(DbCommodity::IsWormholeFTLDriveModule(ship->SystemModule(SYS_FTLDrive, ControlId())))
			{
				// Turn on the wormhole FTL drive if the next waypoint is a wormhole.
				// Is my waypoint at a wormhole in this system?
				const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
				if(system && system->Is(wpSystem) && system->Is(wpDest))
				{
					AuWormhole wh = system->FindWormhole(wpDestOff);
					if(wh.IsValid())
					{
						// The next waypoint is a wormhole in this system. Turn on the FTL drive.
						if(!ship->IsFTLDriveOn(ControlId()))
							DoShipFTLDriveState(ship, true, ControlId());

						/*
						// If the ETE to the wormhole is less than 15 seconds, activate the FTL drive.
						if(ship->CanEngageFTLNow(ControlId()))
						{
							// Calculate ete to wormhole in seconds.
							AuPoint sysLoc = ship->EnvLocation();
							double rdist = sysLoc.Distance(wpDestOff);
							if(rdist > 0)
							{
								float speed = fabs(ship->Speed());	// Can be negative when in reverse.
								if(speed != 0.)
								{
									float etesecs = (float)rdist/speed;
									if(etesecs < 15)
									{
										// Set polarity of FTL drive appropriately.
										if(wh.Polarity() == AuWormhole::Positive)
											DoShipFTLDriveSetting(ship, 0, ControlId());	// Set positive polarity.
										else if(wh.Polarity() == AuWormhole::Negative)
											DoShipFTLDriveSetting(ship, 1, ControlId());	// Set negative polarity.
										else
											DoShipFTLDriveSetting(ship, 2, ControlId());	// Set neutral polarity.

										int announce;
										if(ship->HasHumanOfficer())
											announce = 1;
										else
											announce = 0;

										// Activate the FTL drive.
										DoShipEngageFTLDrive(ship, announce, "", ControlId());
									}
								}
							}
						}
						*/
					}
				}
			}
			/*
			else if(DbCommodity::IsWarpFTLDriveModule(ship->SystemModule(SYS_FTLDrive, ControlId())) &&
					!dynamic_cast<const DaWorld*>(ship->Parent()))
			{
				// If the next waypoint is in another solar system, use warp drive to get there.
				// Do not operate the warp drive when near a planet.
				// Warp factor setting is estimated based on parsecs distance to destination.
				const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
				if(system && system->Is(wpSystem))
				{
					// Waypoint is in this solar system. How far away is it?
					// If the destination is close to the primary star, use max warp speed and depend on warp braking at the end.
					bool useMaxWarp = false;
					double distp = 0;								// Distance to waypoint in feet.
					AuPoint mySysLoc = ship->EnvLocation();
					QString destName;
					const DaStar* star = system->FindPrimaryStar();
					Q_ASSERT(star);
					double orbitAlt = star->AtmosphereRadius() + ParkAltAboveSun()*2;	// This should make the passengers nervous.
					double starDist2 = orbitAlt * orbitAlt;		// squared for faster compare
					bool onCourseForWarp;
					if(system->Is(wpDest))
					{
						// Destination is this system. Waypoint offset is relative to the solar system.
						// Check on course. Very hard to turn at warp speed.
						AuPoint courseVec = ship->EnvTransform().UnProject(wpDestOff);	// Vec to dest rel to ship.
						courseVec.Unit();
						double angDot = courseVec.Dot(AuPoint(0,1,0));
						onCourseForWarp = angDot > .999;	// About 2.5 degrees
						if(onCourseForWarp)
						{
							distp = mySysLoc.Distance(wp->DestinationOffset());
							//if(ship->Name() == "Atlatl")
								//qDebug() << Id() << Name() << "sysDest" << system->Is(wpDest)
									//<< "destOffLen" << wpDestOff.Length() << "starDist" << orbitAlt;
							if(wpDestOff.Length2() < starDist2)
								useMaxWarp = true;	// Max warp when destination is close to the sun for warp braking.
						}
					}
					else
					{
						// Destination is in this system. Waypoint offset is relative to the destination atom.
						const DaAtom* target = system->FindIdRecursive(wpDest);
						if(target)
						{
							// Check on course. Very hard to turn at warp speed.
							AuPoint destSysLoc = target->EnvTransform().Project(wpDestOff);
							AuPoint courseVec = ship->EnvTransform().UnProject(destSysLoc);	// Vec to dest rel to ship.
							courseVec.Unit();
							double angDot = courseVec.Dot(AuPoint(0,1,0));
							onCourseForWarp = angDot > .999;	// About 2.5 degrees
							if(onCourseForWarp)
							{
								distp = mySysLoc.Distance(destSysLoc);
								destName = target->Name();
								//if(ship->Name() == "Atlatl")
									//qDebug() << Id() << Name() << "destSysLocLen" << destSysLoc.Length()
										//<< "starDist" << orbitAlt;
								if(destSysLoc.Length2() < starDist2)
									useMaxWarp = true;	// Max warp when destination is close to the sun for warp braking.
							}
						}
					}

					//qDebug() << "DaSentient::OperateEngineerConsole" << Id() << Name()
						//<< "aboard" << ship->Id() << ship->Name()
						//<< "distance to waypoint in this system in parsecs is" << distp;
					bool farEnoughToWarp = false;
					//if(distp > .9*FeetPerParsec())
					// Check against the distance the ship travels at warp 1 (C x 10) in a full 50 second cycle.
					// Farther than this and it's safe to engage, at least at warp 1.
					if(distp >= SpeedOfWarp() * 50.)
						farEnoughToWarp = true;
					else if(useMaxWarp &&
							ship->MissionOrderIsStarTravelOrder() &&
							distp > FeetPerSolarSystem()/2)
					{
						// Destination is close to the sun for warp braking.
						// Warp in-system to get there if it's not too close to stop.
						const double accel = ship->ManeuverAcceleration();		// ft/sec/sec adjusted for damage
						if(accel > 0)
						{
							// Maneuver drive is working to use for braking. Calculate time to stop from 1/2 light speed.
							float stopTime = SpeedOfLight()/2 / accel;			// stop time from 1/2 light speed
							float stopDist = stopTime * SpeedOfLight()/2;		// worst case stop dist, without integrating
							if(distp > stopDist)
								farEnoughToWarp = true;							// Far enough to use warp drive.
						}
					}

					if(farEnoughToWarp)
					{
						// Far enough away in this system to use warp drive to get there. Turn on the FTL drive.
						if(!ship->IsFTLDriveOn(ControlId()))
							DoShipFTLDriveState(ship, true, ControlId());

						// Validation at warp factor 0.
						bool canCommit;
						if(onCourseForWarp)
							canCommit = ship->CanEngageFTLAt0(ControlId());
						else
							canCommit = false;

						// If we know we can commit at least at warp factor 0, proceed.
						if(canCommit)
						{
							int maxWarpFactor = ship->MaxCapacitorWarpFactor();
							if(maxWarpFactor >= 0)
							{
								if(useMaxWarp)
								{
									// Max warp. We'll depend on sun braking when we get there.
									//if(ship->Name() == "Atlatl")
										//qDebug() << Id() << Name() << "useMaxWarp" << maxWarpFactor;
									DoShipFTLDriveSetting(ship, maxWarpFactor, ControlId());
								}
								else
								{
									// Calculate warp factor needed. Denominator is distance traveled at warp 1 in a 50 sec. cycle.
									int ftlDriveSetting = (int)(distp / (SpeedOfWarp() * 50.));
									if(ftlDriveSetting > maxWarpFactor)
										ftlDriveSetting = maxWarpFactor;
									DoShipFTLDriveSetting(ship, ftlDriveSetting, ControlId());
								}

								int announce;
								if(ship->HasHumanOfficer())
									announce = 1;
								else
									announce = 0;

								// Activate the FTL drive.
								DoShipEngageFTLDrive(ship, announce, destName, ControlId());
							}
						}
					}
				}
				else if(system)
				{
					// Waypoint is in a different solar system. Turn on the FTL drive.
					if(!ship->IsFTLDriveOn(ControlId()))
						DoShipFTLDriveState(ship, true, ControlId());

					// Validation at warp factor 0.
					bool canCommit = ship->CanEngageFTLAt0(ControlId());

					// Don't engage warp drive too close to the sun. It's better to accelerate using maneuver drive.
					if(canCommit && ship->EnvLocation().Length2() < FeetPerSolarSystem()/4 * FeetPerSolarSystem()/4)
						canCommit = false;

					// If we know we can commit at least at warp factor 0, proceed.
					if(canCommit)
					{
						const DaGalaxy* galaxy = system->FindParentGalaxy();
						Q_ASSERT(galaxy);
						const DaSolarSystem* destSystem = dynamic_cast<const DaSolarSystem*>(galaxy->FindIdRecursive(wpSystem));
						if(destSystem)
						{
							// Destination solar system was found.
							AuMatrix4x4 mgShip;
							ship->GetTransformRelativeToParent(mgShip, galaxy);
							AuPoint mySysLoc = mgShip.Project(AuPoint(0,0));				// Result should be in parsecs.
							AuPoint destSysLoc;
							destSystem->GetLocationRelativeToParent(destSysLoc, galaxy);	// Result should be in parsecs.

							// Check on course. Very hard to turn at warp speed.
							AuPoint courseVec = mgShip.UnProject(destSysLoc);
							courseVec.Unit();
							double angDot = courseVec.Dot(AuPoint(0,1,0));
							bool onCourseForWarp = angDot > .999;	// About 2.5 degrees
							if(onCourseForWarp)
							{
								double distp2 = mySysLoc.Distance2(destSysLoc);		// Squared distance.
								//qDebug() << "DaSentient::OperateEngineerConsole" << Id() << Name()
									//<< "aboard" << ship->Id() << ship->Name()
									//<< "distance to waypoint in another system in parsecs is" << distp2;

								if(distp2 > .9*.9)
								{
									// Far enough away to use warp drive to get there.
									int maxWarpFactor = ship->MaxCapacitorWarpFactor();
									if(maxWarpFactor >= 0)
									{
										// Max warp. We'll depend on sun braking when we get there.
										//if(ship->Name() == "Atlatl")
											//qDebug() << Id() << Name() << "set maxWarp" << maxWarpFactor;
										DoShipFTLDriveSetting(ship, maxWarpFactor, ControlId());

										int announce;
										if(ship->HasHumanOfficer())
											announce = 1;
										else
											announce = 0;

										// Activate the FTL drive.
										DoShipEngageFTLDrive(ship, announce, destSystem->NameKnownTo(ship->EmpireId()),
												ControlId());
									}
								}
							}
						}
					}
				}
			}
			*/
		}
		else
		{
			ship->UnlockWaypoint();
			/*
			if(t == AuMissionOrder::Follow && DbCommodity::IsWarpFTLDriveModule(ship->SystemModule(SYS_FTLDrive, ControlId())))
			{
				// We are following a unit in space and we have a warp drive.
				// Engage the warp drive if the follow target has engaged their warp drive.
				// Determine the maximum FTL drive setting based on our capacitor level.
				int maxWarpFactor = ship->MaxCapacitorWarpFactor();
				if(maxWarpFactor >= 0 && ship->CanEngageFTLNow(ControlId()))
				{
					const DaAtom* system = dynamic_cast<const DaAtom*>(FindEnvironmentAtom());
					const DaVessel* target =
						dynamic_cast<const DaVessel*>(system->FindIdRecursive(ship->MissionOrderTarget()));
					if(target)
					{
						// Follow target is a spacecraft in this system. Has it engaged its warp drive?
						if(DbCommodity::IsWarpFTLDriveModule(target->SystemModule(SYS_FTLDrive, ControlId())) &&
								target->FTLDriveWorking(ControlId()) &&
								target->FTLDriveCommitState(ControlId()) > FTL_COMMIT_Dormant &&
								target->FTLDriveCommitState(ControlId()) < FTL_COMMIT_ChargedBegin+3)
						{
							// The target has engaged its warp drive and it has not gotten fully charged yet. Turn on the FTL drive.
							if(!ship->IsFTLDriveOn(ControlId()))
								DoShipFTLDriveState(ship, true, ControlId());

							// Set my warp factor to the highest setting that does not exceed the warp factor of the target ship.
							int ftlDriveSetting = target->FTLDriveSetting(ControlId());
							if(ftlDriveSetting > maxWarpFactor)
								ftlDriveSetting = maxWarpFactor;
							DoShipFTLDriveSetting(ship, ftlDriveSetting, ControlId());

							// Activate the FTL drive.
							int announce;
							if(ship->HasHumanOfficer())
								announce = 2;
							else
								announce = 0;
							DoShipEngageFTLDrive(ship, announce, target->NameWithEmpire(), ControlId());
						}
						else
						{
							// Engage warp drive if target is a great distance away.
							AuPoint courseVec = ship->EnvTransform().UnProject(target->EnvLocation());	// Vec to dest rel to ship.
							double dist = courseVec.Length();
							if(dist > FeetPerSolarSystem())
							{
								courseVec /= dist;
								double angDot = courseVec.Dot(AuPoint(0,1,0));
								bool onCourseForWarp = angDot > .999;	// About 2.5 degrees
								if(onCourseForWarp)
								{
									// Calculate warp factor needed. Denominator is distance traveled at warp 1 in a 50 sec. cycle.
									int ftlDriveSetting = (int)(dist / (SpeedOfWarp() * 50.));
									if(ftlDriveSetting > maxWarpFactor)
										ftlDriveSetting = maxWarpFactor;
									DoShipFTLDriveSetting(ship, ftlDriveSetting, ControlId());

									// Activate the FTL drive.
									int announce;
									if(ship->HasHumanOfficer())
										announce = 2;
									else
										announce = 0;
									DoShipEngageFTLDrive(ship, announce, target->NameWithEmpire(), ControlId());
								}
							}
						}
					}
				}
			}
			*/
		}
	}

	// Adjust the skew factors.
	DoShipEngineTune(ship);

	if(canTurn)
		StayIdle();
}

void DaSentient::OperateNavigatorConsole(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
	{
		// Crew at non-critical station watches for threats.
		if(LookForThreats(true))
			return;						// Threat spotted; pursue and destroy; return to my station.
	}

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate when there is no power to the ship.
	if(ship->CapacitorLevel() <= 0)
		return;
	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Don't do anything if the ship has no active mission or I'll end up messing with someone's navigation route.
	// Don't do anything if a person is manning one of these consoles. This also notices command console in this mode.
	// Don't do anything if the ship is getting repairs.
	if(!ship->IsOnAMission() || ship->IsNavigatorMannedByPerson())
	{
		StayIdle();
		return;
	}

	// Clear route if ship is getting repairs.
	if(ship->RepairSecondsRemaining())
	{
		//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "ship repairing" << ship->RepairSecondsRemaining();
		if(ship->HasWaypoint())
			ship->RemoveAllWaypoints();
		StayIdle();
		return;
	}

	// Rate of thinking when operating nav is dependent on changing ship factors so we must update it every time we think.
	UpdateThinkRate();

	// Check the current mission order. Does it require me to navigate somewhere?
	AuPoint destOffset;				// Used for going to wormholes and navigating near ringworlds.
	AuMissionOrder::TYPE destOrder;
	const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
	if(destAtom)
	{
		// A destination atom was found.
		const DaAtom* system = ship->FindEnvironmentAtom();
		//qDebug("%d %s destination is %d %s",
				//Id().GetID(), (const char*)Name().toAscii(),
				//destAtom->Id().GetID(), (const char*)destAtom->Name().toAscii());

		// When following a unit, the pilot does most of the work.
		if(destOrder == AuMissionOrder::Follow)
		{
			// I'll watch for the target moving toward a wormhole.
			const DaVessel* destShip = dynamic_cast<const DaVessel*>(destAtom);
			const DaSolarSystem* solar = dynamic_cast<const DaSolarSystem*>(system);
			if(destShip)
			{
				// Target is a spacecraft.
				if(solar && destShip->IsReadyToWormhole() && IsOkToLeaveSolarSystem(ship) &&
						DbCommodity::IsWormholeFTLDriveModule(ship->SystemModule(SYS_FTLDrive, ControlId())))
				{
					// Target ship is ready to wormhole, I am cleared to leave the system, and my ship also has a wormhole drive.
					// Try to determine what wormhole it will go through.
					AuWormhole wh;
					const DeWaypoint* wp = ship->LockWaypoint(0);
					if(wp && solar->Is(wp->m_idSystem) && solar->Is(wp->Destination()))
						wh = solar->FindWormhole(wp->DestinationOffset());		// The target ship has a waypoint set.
					ship->UnlockWaypoint();

					if(!wh.IsValid())
					{
						// Target's next waypoint is not a wormhole. Find the closest wormhole to the target ship.
						AuPoint targetLoc = destShip->EnvLocation();
						wh = solar->FindClosestWormhole(targetLoc);
					}

					if(wh.IsValid())
					{
						// Found a valid wormhole.
						AuPoint whSysLoc = wh.Location(solar);
						if(ship->HasWaypoint())
						{
							// My ship has a route set. Does it take me to that wormhole? I may have already done this.
							//qDebug("%d %s route goes to wrong wormhole", Id().GetID(), (const char*)Name().toAscii());
							if(ship->WaypointCount() != 1)
								ship->RemoveAllWaypoints();
							else
							{
								const DeWaypoint* wp = ship->LockWaypoint(0);
								bool v = !solar->Is(wp->Destination()) ||
									whSysLoc.Distance2(wp->DestinationOffset()) > 10000;	// Fuzzy iffy, 100' is close enough
								ship->UnlockWaypoint();
								if(v)
									ship->RemoveAllWaypoints();
							}
						}
						if(!ship->HasWaypoint())
						{
							// Add waypoint at the wormhole.
							ship->AddWaypoint(solar->Id(), solar->Id(), whSysLoc);

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Target spacecraft wormhole drive is hot. "
											"Navigating through wormhole to pursue."),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Grim);
							}
						}
					}
					else
					{
						// No valid wormhole was found. Possible because some solar systems may have no wormholes.
						if(ship->HasWaypoint())
						{
							//qDebug("%d %s no valid wormhole found", Id().GetID(), (const char*)Name().toAscii());
							ship->RemoveAllWaypoints();
						}
					}
				}
				else
				{
					// Target ship is not ready to wormhole or I am not permitted to leave the solar system or I don't have
					// a wormhole drive.
					if(ship->HasWaypoint())
					{
						//qDebug("%d %s target ship not ready to wormhole", Id().GetID(), (const char*)Name().toAscii());
						ship->RemoveAllWaypoints();
					}
				}
			}
			else if(destAtom->AtomType() == ATOM_Asteroid)
			{
				// Target is an asteroid. The pilot can handle it.
				if(ship->HasWaypoint())
					ship->RemoveAllWaypoints();
			}
			else if(ship->HasWaypoint())
			{
				// I am following a target and I have a route set.
				// Target was not found but we have a waypoint set.
				// Continue following through a wormhole if that is our next waypoint.
				const DeWaypoint* wp = ship->LockWaypoint(0);
				Q_ASSERT(wp);
				AuWormhole wh;
				if(solar && solar->Is(wp->m_idSystem) && solar->Is(wp->Destination()))
					wh = solar->FindWormhole(wp->DestinationOffset());
				bool v = !wh.IsValid();
				ship->UnlockWaypoint();
				if(v)
				{
					// My next waypoint is not a wormhole. Clear the route.
					//qDebug("%d %s target lost, not going to wormhole, clearing route", Id().GetID(),
							//(const char*)Name().toAscii());
					ship->RemoveAllWaypoints();
				}
			}
		}
		else
		{
			// Our mission is not to follow the destAtom.
			const DaSolarSystem* destSystem = dynamic_cast<const DaSolarSystem*>(destAtom);
			const DaGalaxy* destGal = dynamic_cast<const DaGalaxy*>(destAtom);
			if(destSystem)
			{
				// The destination is a solar system.
				const DaSolarSystem* solar = dynamic_cast<const DaSolarSystem*>(system);
				if(destSystem == solar)
				{
					// Destination IS my current solar system.
					// I am either patroling wormholes or I have deadheaded to this sytem or I am going directly to a coordinate
					// that is relative to the current solar system.
					if(destOrder == AuMissionOrder::PatrolWormholes)
					{
						// Current mission order requires me to patrol wormholes.
						if(ship->HasWaypoint())
						{
							// My ship has a route set. Does it take me to that wormhole? I may have already done this.
							if(ship->WaypointCount() != 1)
								ship->RemoveAllWaypoints();
							else
							{
								const DeWaypoint* wp = ship->LockWaypoint(0);
								bool v = !solar->Is(wp->Destination()) ||
									destOffset.Distance2(wp->DestinationOffset()) > 10000;	// Fuzzy iffy, 100' is close enough
								//qDebug() << Id() << Name()
									//<< "clearing waypoint at wrong wormhole" << solar->Id() << solar->Name()
									//<< destOffset << "wp" << wp->Destination() << wp->DestinationOffset();
								ship->UnlockWaypoint();
								if(v)
									ship->RemoveAllWaypoints();
							}
						}
						else
						{
							// No route so add a waypoint at the wormhole.
							//qDebug() << Id() << Name()
								//<< "setting course for wormhole" << solar->Id() << solar->Name() << destOffset;
							// Add waypoint at dest wormhole.
							ship->AddWaypoint(solar->Id(), solar->Id(), destOffset);

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Plotting course to next wormhole on patrol."),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Closed);
							}
						}
					}
					else
					{
						// When traveling directly to a solar system, decide we're there when we get within the system radius.
						bool isTravelToSystemOrder;
						if(destOrder == AuMissionOrder::GoToSystemDirect ||
								destOrder == AuMissionOrder::GoToSystemDirect_Deprecated)
							isTravelToSystemOrder = true;		// Destination is the system.
						else
						{
							// Check for destination inside the primary star.
							const DaStar* destStar = solar->FindPrimaryStar();
							if(destStar)
							{
								double starRad = destStar->Radius();
								isTravelToSystemOrder = destOffset.Length2() <= starRad*starRad;
							}
							else
								isTravelToSystemOrder = false;
						}

						if(isTravelToSystemOrder)
						{
							// Check to see if we are within a solar system radius of the center of the system.
							AuPoint shipSysLoc = ship->EnvLocation();
							const double distFromSun2 = shipSysLoc.Length2();	// Squared distance to the sun.
							double navDist = FeetPerSolarSystem()/2;			// Navigate to within 1/2 solar system diameter.
							if(distFromSun2 < navDist*navDist)
							{
								// The ship is within a solar system radius of the destination star.
								// Take some precious CPU time to be more precise so we get to where the planets are.
								navDist = destSystem->ActualRadius() + FeetPerOrbit();
								if(distFromSun2 < navDist*navDist)
								{
									// Ship is in the system; that's close enough.
									if(ship->HasWaypoint())
									{
										// Clear the waypoint.
										ship->RemoveWaypoint(0);

										// Announce my action.
										if(ship->HasHumanOfficer())
										{
											QString text;
											if(destOrder == AuMissionOrder::GoToSystemViaWormholes)
												text = QObject::tr("We have arrived in the destination system.");
											else
												text = QObject::tr("We are approaching the outer reaches of the destination "
														"system.");
											AuCommChat msg(text,
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Closed);
										}
									}
								}
							}
						}
						else if(destOrder == AuMissionOrder::GoToCoordinate)
						{
							// Destination is a specific coordinate. The order may specify the destination coordinate relative to
							// the galaxy, sector or solar system. The DaVessel::MissionDestination function always returns a
							// destination that is relative to the current solar system.
							AuPoint shipSysLoc = ship->EnvLocation();

							// Adjust destOffset so it does not land inside a planet or star.
							solar->AdjustNavLocation(shipSysLoc, destOffset);
							AuPoint vecAhead = destOffset - shipSysLoc;
							double distToDest2 = vecAhead.Length2();	// Squared distance to the destination location.

							//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name()
								//<< "ship" << ship->Id() << ship->Name()
								//<< "go to system coordinate" << destOffset << "ship system loc" << shipSysLoc;
							if(distToDest2 > 500*FeetPerMeter() * 500*FeetPerMeter())
							{
								// The ship is beyond 500m of the target location.
								bool okToGo = true;					// false if it is not ok to depart the solar system
								if(!ship->FindPersonOnLineRecursive() && !solar->IsInSystem(destOffset))
								{
									// No online person is aboard and the destination departs the current solar system.
									// Insure the destination does not take us into unexplored space.
									// destOffset is in feet relative to my current solar system.
									if(distToDest2 > FeetPerParsec()*FeetPerParsec())
									{
										// Destination is more than 1pc away.
										// Scale the look-ahead vector to be no more than 1pc.
										vecAhead /= sqrt(distToDest2);	// Unit vector
										vecAhead *= FeetPerParsec();	// 1pc vector length
									}

									AuPoint lookAhead = shipSysLoc + vecAhead;	// Look ahead location relative to current system.
									const DaSector* ps = (const DaSector*)solar->Parent();
									AuMatrix4x4 msys;
									solar->GetTransformRelativeToParent(msys, ps);
									AuPoint sectorLoc = msys.Project(lookAhead);	// Result in pc relative to sector.

									// FindClosestSolarSystemInRegion will find the closest system, even if the search point is
									// far outside the box. If that system is not explored, we will not proceed.
									DaSolarSystem* nextSystem = ps->FindClosestSolarSystemInRegion(sectorLoc);
									if(!nextSystem->IsExploredBy(ship->EmpireId()))
									{
										//if(ship->Name() == "Atlatl")
											//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "nextSystem"
												//<< nextSystem->Id() << nextSystem->Name()
												//<< "is unexplored. I WON'T GO, currently in" << system->Id() << system->Name();
										okToGo = false;				// Next system is not explored by my empire. Don't go there.
									}
									//else if(ship->Name() == "Atlatl")
											//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "nextSystem"
												//<< nextSystem->Id() << nextSystem->Name() << "is explored, currently in"
												//<< system->Id() << system->Name();
								}

								if(okToGo)
								{
									// Fly toward the target location.
									// Does my current route take me to the destOffset location.
									if(ship->HasWaypoint())
									{
										const DeWaypoint* wp = ship->LockWaypoint(0);
										//qDebug() << "waypoints" << ship->WaypointCount()
											//<< "is system" << solar->Is(wp->Destination())
											//<< "distance" << destOffset.Distance(wp->DestinationOffset());
										bool v = !solar->Is(wp->Destination()) ||
											destOffset.Distance2(wp->DestinationOffset()) > 500*FeetPerMeter() * 500*FeetPerMeter();
										ship->UnlockWaypoint();
										if(v)
											ship->RemoveAllWaypoints();
									}

									// Establish a route; we're not close enough.
									if(!ship->HasWaypoint())
									{
										// Add wp at dest in system.
										ship->AddWaypoint(solar->Id(), solar->Id(), destOffset);

										// Announce my action.
										if(ship->HasHumanOfficer())
										{
											AuCommChat msg(QObject::tr("Plotting course direct to coordinates."),
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Closed);
										}
									}
								}
								else
								{
									// It's not ok to go to that destination. It must be an unexplored system.
									ship->RemoveAllWaypoints();
								}
							}
							else
							{
								// Destination location is within 500m of the ship; that's close enough.
								if(ship->HasWaypoint())
								{
									// Clear the waypoint.
									//qDebug() << Id() << Name() << "clearing a waypoint" << ship->WaypointCount();
									ship->RemoveWaypoint(0);

									// Announce my action.
									if(ship->HasHumanOfficer())
									{
										AuCommChat msg(QObject::tr("We are within 500m of nav coordinates."),
												AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
												Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
										Say(msg, AuVisage::Closed);
									}
								}
							}
						}
					}
				}
				else if(!destSystem->IsExploredBy(ship->EmpireId()) && !ship->IsPirateWithFreeWill() &&
						!ship->FindPersonOnLineRecursive())
				{
					// Do not navigate a ship to an unexplored solar system unless there is a person aboard who is on line.
					// Pirates are allowed to do this to flee. They are destroyed when they cross any system boundary, for cleanup.
					if(ship->HasWaypoint())
					{
						//qDebug() << Id() << Name() << "cannot navigate to unexplored system with AI commander";
						ship->RemoveAllWaypoints();
					}
				}
				else if(ship->HasWaypoint())
				{
					// The ship already has a course set. Does it take me to the destination system?
					const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
					bool destOk = destAtom->Is(wp->m_idSystem);
					ship->UnlockWaypoint();
					if(!destOk)
					{
						// The final waypoint is not in the destination system.
						//qDebug("%d %s changing course for system %d %s", Id().GetID(), (const char*)Name().toAscii(),
								//destAtom->Id().GetID(), (const char*)destAtom->Name().toAscii());
						ship->RemoveAllWaypoints();
					}
					else
					{
						// The final waypoint is in the destination system.
						// The first waypoint is at the wormhole.
						const DeWaypoint* wpnext = ship->LockWaypoint(0);
						AuId wpSystem(wpnext->m_idSystem);
						AuId wpDest(wpnext->Destination());
						AuPoint wpDestOff(wpnext->DestinationOffset());
						ship->UnlockWaypoint();

						// Am I on course to get there as ordered?
						if(destOrder == AuMissionOrder::GoToSystemViaWormholes)
						{
							// We are traveling via wormholes and the route has at least two waypoints.
							// We may have already decided to skip the wormhole due to blockage and deadhead to the next system.
							if(system->Is(wpSystem))
							{
								// The next waypoint should be at a wormhole in my current system.
								AuWormhole wh;
								if(solar->Is(wpSystem) && solar->Is(wpDest))
									wh = solar->FindWormhole(wpDestOff);

								// When wormhole travel is not possible, we deadhead instead.
								// - Wormhole FTL drive must be installed.
								// - Capacitor must be capable of delivering the power, in its current condition.
								// - FTL drive must be functioning well enough to engage.
								quint16 whichFTL = ship->FTLWormholeDrive();
								if(ship->HasWormholeFTLDrive() &&
										ship->CapacitorMax() >= ship->PowerNeededToEngageFTLDrive(whichFTL, 0) &&
										ship->GetPowerConsumption(SYS_FTLDrive, whichFTL) >= ship->FTLDriveUnitsToEngage())
								{
									// We are supposed to travel using wormholes and the wormhole drive is operational.
									// Next waypoint had better be a wormhole.
									const DaAtom* sunOrPlanet;
									if(!wh.IsValid())
									{
										//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "wh not valid"
											//<< "count" << ship->WaypointCount();
										// Next waypoint is not a wormhole. Clear the current route.
										ship->RemoveAllWaypoints();

										// Replace the waypoint immediately so we don't have jerky behavior in black hole systems
										// where the wormholes are constantly in motion.
										// Add waypoint route to dest sys.
										ship->AddWaypointRouteTo(destAtom->Id(), AuPoint(0,0));
									}
									else if(!wh.IsIntergalactic())
									{
										// Waypoint wormhole is valid and it is not intergalactic.
										//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "wh valid"
											//<< "count" << ship->WaypointCount();
										if(solar)
											sunOrPlanet = solar->WormholeBlocked(wpDestOff);
										else
											sunOrPlanet = 0;

										if(sunOrPlanet)
										{
											// Remove waypoint at blocked wormhole. Remaining waypoint is at the destination system.
											ship->RemoveWaypoint(0);

											// Announce my action.
											if(ship->HasHumanOfficer())
											{
												AuCommChat msg(QObject::tr("Wormhole is blocked by %1. "
															"Changing course direct to wormhole destination system.")
														.arg(sunOrPlanet->Name()),
														AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
														Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
												Say(msg, AuVisage::Perplexed);
											}
										}

										// Due to a bug, check to see if the wormhole is inside the radius of the sun.
										DaStar* primary = solar->FindPrimaryStarNotBlackHole();
										if(primary)
										{
											// Squared distance to wormhole from center of sun.
											AuPoint whSysLoc = wh.Location(solar);
											double rangeFromSun2 = whSysLoc.Distance2(primary->EnvLocation());
											double sunRad2 = primary->Radius();
											sunRad2 *= sunRad2;								// Squared radius of primary sun.
											if(rangeFromSun2 <= sunRad2)
											{
												// We have detected a problem with the position of the wormhole.
												// Clear the current route. Don't navigate into sun.
												ship->RemoveAllWaypoints();

												// Report this in the log.
												const float rWormOrgDist = primary->Radius() * 2.f;
												const float rWormDestDist = rWormOrgDist * 1.1f;
												float rdist;
												if(wh.Polarity() == AuWormhole::Positive)
													rdist = rWormOrgDist;
												else
													rdist = rWormDestDist;
												qWarning() << "ERROR! Navigator" << Id() << Name() << "noticed"
													<< ((wh.Polarity() == AuWormhole::Positive)?"positive":"negative")
													<< "wormhole position wrong in system" << solar->Name()
													<< "star radius" << primary->Radius()
													<< "wormhole distance from star center" << sqrt(rangeFromSun2)
													<< "should be" << rdist
													<< "Route cleared.";
											}
										}
									}
								}
								else
								{
									// Ship cannot wormhole so remove the waypoint at the wormhole. The remaining waypoint is the
									// destination system.
									if(wh.IsValid() && !wh.IsIntergalactic())
										ship->RemoveWaypoint(0);
									else
										ship->RemoveAllWaypoints();

									// Announce my action.
									if(ship->HasHumanOfficer())
									{
										if(!ship->HasWormholeFTLDrive())
										{
											AuCommChat msg(QObject::tr("Spacecraft cannot jump without a wormhole drive. "
														"Changing course direct to wormhole destination system."),
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Perplexed);
										}
										else
										{
											quint16 whichFTL = ship->FTLWormholeDrive();
											if(ship->CapacitorMax() < ship->PowerNeededToEngageFTLDrive(whichFTL, 0))
											{
												AuCommChat msg(QObject::tr("Capacitor is inadequate to engage wormhole drive. "
															"Changing course direct to wormhole destination system."),
														AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
														Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
												Say(msg, AuVisage::Perplexed);
											}
											else if(ship->GetPowerConsumption(SYS_FTLDrive, whichFTL) <
													ship->FTLDriveUnitsToEngage())
											{
												AuCommChat msg(QObject::tr("Wormhole drive is inadequate to engage. "
															"Changing course direct to wormhole destination system."),
														AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
														Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
												Say(msg, AuVisage::Perplexed);
											}
											else
											{
												AuCommChat msg(QObject::tr("Spacecraft cannot wormhole. "
															"Changing course direct to wormhole destination system."),
														AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
														Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
												Say(msg, AuVisage::Perplexed);
											}
										}
									}
								}
							}
						}
						else
						{
							// The next and only waypoint had better be the destination solar system.
							if(ship->WaypointCount() != 1 || !destAtom->Is(wpDest))
							{
								//qDebug("%d %s waypoint count %d, not destination solar system",
										//Id().GetID(), (const char*)Name().toAscii(), ship->WaypointCount());
								// Next wp is not direct to the dest. Clear the current route.
								ship->RemoveAllWaypoints();
							}
						}
					}
				}
				else
				{
					//qDebug() << "setting course for system" << destAtom->Id() << destAtom->Name();
					if(destOrder == AuMissionOrder::GoToSystemViaWormholes)
					{
						// When traveling via wormholes, deadhead instead if the wormhole drive cannot be operated.
						if(!ship->HasWormholeFTLDrive())
						{
							// The ship does not have a wormhole drive.
							// Add waypoint directly to dest system.
							ship->AddWaypoint(destAtom->Id(), destAtom->Id(), AuPoint());

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
											"This ship does not have a wormhole FTL drive. "
											"Course laid direct to %1 instead.").arg(destSystem->NameKnownTo(ship->EmpireId())),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Perplexed);
							}
						}
						else
						{
							quint16 whichFTL = ship->FTLWormholeDrive();
							if(ship->CapacitorMax() < ship->PowerNeededToEngageFTLDrive(whichFTL, 0))
							{
								// The capacitor is inadequate to engage the wormhole drive.
								// Add waypoint directly to dest system.
								ship->AddWaypoint(destAtom->Id(), destAtom->Id(), AuPoint());

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
												"The ship's capacitor is inadequate to engage wormhole FTL drive. "
												"Course laid direct to %1 instead.").arg(destSystem->NameKnownTo(ship->EmpireId())),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Perplexed);
								}
							}
							else if(ship->GetPowerConsumption(SYS_FTLDrive, whichFTL) < ship->FTLDriveUnitsToEngage())
							{
								// The wormhole drive is too badly damaged to engage.
								// Add waypoint directly to dest system.
								ship->AddWaypoint(destAtom->Id(), destAtom->Id(), AuPoint());

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
												"The ship's wormhole FTL drive is too badly damaged to engage. "
												"Course laid direct to %1 instead.").arg(destSystem->NameKnownTo(ship->EmpireId())),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Perplexed);
								}
							}
							else
							{
								// All systems are go for wormhole routing.
								// Add a waypoint route to the dest sys.
								ship->AddWaypointRouteTo(destAtom->Id(), AuPoint());
								if(ship->HasWaypoint())
								{
									// The first waypoint is at the wormhole.
									const DaSolarSystem* mysys = dynamic_cast<const DaSolarSystem*>(system);
									// Insure the wormhole is not blocked.
									const DeWaypoint* wpnext = ship->LockWaypoint(0);
									AuId wpSystem(wpnext->m_idSystem);
									AuPoint wpDestOff(wpnext->DestinationOffset());
									ship->UnlockWaypoint();
									const DaAtom* sunOrPlanet = mysys ? mysys->WormholeBlocked(wpDestOff) : 0;
									if(sunOrPlanet)
									{
										// Wormhole is blocked. Remove the waypoint and the next/only waypoint is the dest system.
										ship->RemoveWaypoint(0);
										AuCommChat msg(QObject::tr("Wormhole is blocked by %2. Course laid direct to wormhole "
													"destination system instead.")
												.arg(sunOrPlanet->Name()),
												AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
												Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
										Say(msg, AuVisage::Perplexed);
									}
									else
									{
										AuCommChat msg(QObject::tr("Course laid to %1 via wormholes.")
												.arg(destSystem->NameKnownTo(ship->EmpireId())),
												AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
												Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
										Say(msg, AuVisage::Closed);
									}
								}
								else
								{
									ship->RemoveAllWaypoints();
									ship->AddWaypoint(destAtom->Id(), destAtom->Id());
									AuCommChat msg(QObject::tr("Unable to plot a course to %1 via wormholes. "
												"Course laid direct to destination system instead. ")
											.arg(destSystem->NameKnownTo(ship->EmpireId())),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Perplexed);
								}
							}
						}
					}
					else
					{
						// Add waypoint directly to dest system.
						ship->AddWaypoint(destAtom->Id(), destAtom->Id(), AuPoint());

						// Announce my action.
						if(ship->HasHumanOfficer())
						{
							AuCommChat msg(QObject::tr("Course laid direct to %1.").arg(destSystem->NameKnownTo(ship->EmpireId())),
									AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
									Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
							Say(msg, AuVisage::Closed);
						}
					}
				}
			}
			else if(destGal)
			{
				// The destination is a galaxy.
				const DaSolarSystem* solar = dynamic_cast<const DaSolarSystem*>(system);
				if(destGal->Galaxy() == solar->Galaxy())
				{
					// Destination galaxy IS my current galaxy.
					if(ship->HasWaypoint())
					{
						ship->RemoveAllWaypoints();

						// Announce my action.
						if(ship->HasHumanOfficer())
						{
							QString text = QObject::tr("We have arrived in %1.").arg(destGal->Name());
							AuCommChat msg(text,
									AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
									Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
							Say(msg, AuVisage::Closed);
						}
					}
				}
				else if(ship->HasWaypoint())
				{
					// The ship already has a course set. Does it take me to the destination system?
					const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
					bool destOk = destAtom->Is(wp->m_idSystem);
					ship->UnlockWaypoint();
					if(!destOk)
					{
						// The final waypoint is not the destination galaxy.
						ship->RemoveAllWaypoints();
					}
					else
					{
						// The final waypoint is in the destination system.
						// The first waypoint is at the wormhole.
						const DeWaypoint* wpnext = ship->LockWaypoint(0);
						AuId wpSystem(wpnext->m_idSystem);
						AuId wpDest(wpnext->Destination());
						AuPoint wpDestOff(wpnext->DestinationOffset());
						ship->UnlockWaypoint();

						// We are traveling via wormholes and the route has at least two waypoints.
						// We may have already decided to skip the wormhole due to blockage and deadhead to the next system.
						if(system->Is(wpSystem))
						{
							// The next waypoint should be at a wormhole in my current system.
							AuWormhole wh;
							if(solar->Is(wpSystem) && solar->Is(wpDest))
								wh = solar->FindWormhole(wpDestOff);

							// When wormhole travel is not possible, we deadhead instead.
							// - Wormhole FTL drive must be installed.
							// - Capacitor must be capable of delivering the power, in its current condition.
							// - FTL drive must be functioning well enough to engage.
							quint16 whichFTL = ship->FTLWormholeDrive();
							if(ship->HasWormholeFTLDrive() &&
									ship->CapacitorMax() >= ship->PowerNeededToEngageFTLDrive(whichFTL, 2) &&
									ship->GetPowerConsumption(SYS_FTLDrive, whichFTL) >= ship->FTLDriveUnitsToEngage())
							{
								// We are supposed to travel using wormholes and the wormhole drive is operational.
								// Next waypoint had better be a wormhole.
								const DaAtom* sunOrPlanet;
								if(!wh.IsValid())
								{
									//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "wh not valid"
										//<< "count" << ship->WaypointCount();
									// Next waypoint is not a wormhole. Clear the current route.
									ship->RemoveAllWaypoints();

									// Replace the waypoint immediately so we don't have jerky behavior in black hole systems
									// where the wormholes are constantly in motion.
									// Add waypoint route to dest sys.
									ship->AddWaypointRouteTo(destAtom->Id(), AuPoint(0,0));
								}
								else if(wh.IsIntergalactic())
								{
									// Waypoint wormhole is valid and it is intergalactic.
									//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "wh valid"
										//<< "count" << ship->WaypointCount();
									if(solar)
										sunOrPlanet = solar->WormholeBlocked(wpDestOff);
									else
										sunOrPlanet = 0;

									if(sunOrPlanet)
									{
										// Remove all waypoints due to blocked wormhole.
										ship->RemoveAllWaypoints();

										// Announce my action.
										if(ship->HasHumanOfficer())
										{
											AuCommChat msg(QObject::tr("Intergalactic wormhole is blocked by %1. ")
													.arg(sunOrPlanet->Name()),
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Perplexed);
										}
									}

									// Due to a bug, check to see if the wormhole is inside the radius of the sun.
									DaStar* primary = solar->FindPrimaryStar();
									if(primary)
									{
										// Squared distance to wormhole from center of sun.
										AuPoint whSysLoc = wh.Location(solar);
										double rangeFromSun2 = whSysLoc.Distance2(primary->EnvLocation());
										double sunRad2 = primary->Radius();
										sunRad2 *= sunRad2;								// Squared radius of primary sun.
										if(rangeFromSun2 <= sunRad2)
										{
											// We have detected a problem with the position of the wormhole.
											// Clear the current route. Don't navigate into sun.
											ship->RemoveAllWaypoints();

											// Report this in the log.
											float rdist = primary->AtmosphereRadius();
											qWarning() << "ERROR! Navigator" << Id() << Name() << "noticed"
												<< "intergalactic wormhole position wrong in system" << solar->Name()
												<< "star radius" << primary->Radius()
												<< "wormhole distance from star center" << sqrt(rangeFromSun2)
												<< "should be" << rdist
												<< "Route cleared.";
										}
									}
								}
							}
							else
							{
								// Ship cannot wormhole so remove all the waypoints.
								ship->RemoveAllWaypoints();

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									if(!ship->HasWormholeFTLDrive())
									{
										AuCommChat msg(QObject::tr("Spacecraft cannot jump without a wormhole drive."),
												AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
												Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
										Say(msg, AuVisage::Perplexed);
									}
									else
									{
										quint16 whichFTL = ship->FTLWormholeDrive();
										if(ship->CapacitorMax() < ship->PowerNeededToEngageFTLDrive(whichFTL, 2))
										{
											AuCommChat msg(QObject::tr("Capacitor is inadequate to engage wormhole drive at neutral polarity."),
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Perplexed);
										}
										else if(ship->GetPowerConsumption(SYS_FTLDrive, whichFTL) <
												ship->FTLDriveUnitsToEngage())
										{
											AuCommChat msg(QObject::tr("Wormhole drive is inadequate to engage."),
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Perplexed);
										}
										else
										{
											AuCommChat msg(QObject::tr("Spacecraft cannot wormhole."),
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Perplexed);
										}
									}
								}
							}
						}
					}
				}
				else
				{
					//qDebug() << "setting course for galaxy" << destAtom->Id() << destAtom->Name();
					// When traveling via wormholes, deadhead instead if the wormhole drive cannot be operated.
					if(!ship->HasWormholeFTLDrive())
					{
						// The ship does not have a wormhole drive.
						ship->RemoveAllWaypoints();

						// Announce my action.
						if(ship->HasHumanOfficer())
						{
							AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
										"This ship does not have a wormhole FTL drive. "),
									AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
									Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
							Say(msg, AuVisage::Perplexed);
						}
					}
					else
					{
						quint16 whichFTL = ship->FTLWormholeDrive();
						if(ship->CapacitorMax() < ship->PowerNeededToEngageFTLDrive(whichFTL, 2))
						{
							// The capacitor is inadequate to engage the wormhole drive.
							ship->RemoveAllWaypoints();

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
											"The ship's capacitor is inadequate to engage wormhole FTL drive at neutral polarity. "),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Perplexed);
							}
						}
						else if(ship->GetPowerConsumption(SYS_FTLDrive, whichFTL) < ship->FTLDriveUnitsToEngage())
						{
							// The wormhole drive is too badly damaged to engage.
							ship->RemoveAllWaypoints();

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Unable to plot wormhole course. "
											"The ship's wormhole FTL drive is too badly damaged to engage. "),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Perplexed);
							}
						}
						else
						{
							// All systems are go for wormhole routing.
							// Add a waypoint route to the dest sys.
							ship->AddWaypointRouteTo(destAtom->Id(), AuPoint());
							if(ship->HasWaypoint())
							{
								// The first waypoint is at the wormhole.
								const DaSolarSystem* mysys = dynamic_cast<const DaSolarSystem*>(system);
								// Insure the wormhole is not blocked.
								const DeWaypoint* wpnext = ship->LockWaypoint(0);
								AuId wpSystem(wpnext->m_idSystem);
								AuPoint wpDestOff(wpnext->DestinationOffset());
								ship->UnlockWaypoint();
								const DaAtom* sunOrPlanet = mysys ? mysys->WormholeBlocked(wpDestOff) : 0;
								if(sunOrPlanet)
								{
									// Wormhole is blocked. Remove the waypoint and the next/only waypoint is the dest system.
									ship->RemoveAllWaypoints();
									AuCommChat msg(QObject::tr("Intergalactic wormhole is blocked by %2.")
											.arg(sunOrPlanet->Name()),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Perplexed);
								}
								else
								{
									AuCommChat msg(QObject::tr("Course laid to %1 via intergalactic wormhole.")
											.arg(destGal->Name()),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Closed);
								}
							}
							else
							{
								ship->RemoveAllWaypoints();
								AuCommChat msg(QObject::tr("Unable to plot a course to %1 via intergalactic wormhole.")
										.arg(destGal->Name()),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Perplexed);
							}
						}
					}
				}
			}
			else if(dynamic_cast<const DaArena*>(destAtom) && destAtom == system)
			{
				// The destination is my current arena.
				// I am going directly to a coordinate that is relative to the current system/arena
				if(destOrder == AuMissionOrder::GoToCoordinate)
				{
					// Destination is a specific coordinate. The order may specify the destination coordinate relative to
					// the galaxy, sector or solar system. The DaVessel::MissionDestination function always returns a
					// destination that is relative to the current solar system.
					AuPoint shipSysLoc = ship->EnvLocation();

					// Adjust destOffset so it does not land inside a planet or star.
					AuPoint vecAhead = destOffset - shipSysLoc;
					double distToDest2 = vecAhead.Length2();	// Squared distance to the destination location.

					//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name()
						//<< "ship" << ship->Id() << ship->Name()
						//<< "go to system coordinate" << destOffset << "ship system loc" << shipSysLoc;
					if(distToDest2 > 500*FeetPerMeter() * 500*FeetPerMeter())
					{
						// The ship is beyond 500m of the target location.
						// Fly toward the target location.
						// Does my current route take me to the destOffset location.
						if(ship->HasWaypoint())
						{
							const DeWaypoint* wp = ship->LockWaypoint(0);
							//qDebug() << "waypoints" << ship->WaypointCount()
								//<< "is system" << solar->Is(wp->Destination())
								//<< "distance" << destOffset.Distance(wp->DestinationOffset());
							bool v = !system->Is(wp->Destination()) ||
								destOffset.Distance2(wp->DestinationOffset()) > 500*FeetPerMeter() * 500*FeetPerMeter();
							ship->UnlockWaypoint();
							if(v)
								ship->RemoveAllWaypoints();
						}

						// Establish a route; we're not close enough.
						if(!ship->HasWaypoint())
						{
							// Add wp at dest in system.
							ship->AddWaypoint(system->Id(), system->Id(), destOffset);

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Plotting course direct to coordinates."),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Closed);
							}
						}
					}
					else
					{
						// Destination location is within 500m of the ship; that's close enough.
						if(ship->HasWaypoint())
						{
							// Clear the waypoint.
							ship->RemoveWaypoint(0);

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("We are within 500m of nav coordinates."),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Closed);
							}
						}
					}
				}
			}
			else
			{
				// The destination is in my current solar system.
				const DaWorld* destPlanet = destAtom->FindParentPlanetInEnv();	// Returns destAtom if it is a planet.
				if(destPlanet)
				{
					// Destination is at a planet or destination is a planet.
					const DaWorldRingSection* destRingSection = dynamic_cast<const DaWorldRingSection*>(destPlanet);
					if(destPlanet == ship->FindParentPlanetInEnv())
					{
						// The ship is at the destination atom's planet. Dest atom could be that planet.
						//qDebug("at dest planet %d %s cgAlt %g orbitRange %g waypoints %d", destAtom->Id().GetID(),
								//(const char*)destAtom->Name().toAscii(), ship->CGAltitude(),
								//destPlanet->AtmosphereRadius() + ParkAltAboveAtm()*1.5, ship->WaypointCount());
						if(ship->HasWaypoint())
						{
							// Decide if the nav route can be cleared.
							if(ship->IsInAtmosphere())
							{
								//qDebug("%d %s clear route in atmosphere", Id().GetID(), (const char*)Name().toAscii());
								// ship is inside the atmosphere
								ship->RemoveAllWaypoints();
							}
							else if(destOrder != AuMissionOrder::GoToOrbit &&
									destOrder != AuMissionOrder::GoToOrbitCity &&
									destOrder != AuMissionOrder::GoToOrbitCoords &&
									destOrder != AuMissionOrder::GoToOrbitBestPlanet)
							{
								//qDebug() << "clear route near dest planet" << Id() << Name();
								// same planet is close enough to clear the route unless orbiting
								ship->RemoveAllWaypoints();	// This really can't wait or the ship bobs around like mad.

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									AuCommChat msg(QObject::tr("We are in orbital proximity of %1.").arg(destPlanet->Name()),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Closed);
								}
							}
							else if(ship->CGAltitude() < destPlanet->AtmosphereRadius() + OrbitAltAboveAtm() + 1000*FeetPerMeter())
							{
								// Within CG altitude range of surface.
								if(destRingSection)
								{
									// Ringworld takes a little extra checking to see if we're close enough to "orbit".
									if(ship->CGAltitude() > destPlanet->AtmosphereRadius())
									{
										// Ship is above the top of the atmosphere.
										// Be sure the ship is between the walls of the ringworld section.
										// Note: Current ship coords are relative to the ringworld section.
										AuGlobeLL ll;
										if(destRingSection->LLOfLoc(ll, ship->Location()))
										{
											// LLOfLoc only returns true if my coordinate is above the terrain area of the world.
											//qDebug() << "clear route at dest ringworld" << Id() << Name();
											// ship is between the walls of the ringworld.
											ship->RemoveAllWaypoints();

											// Announce my action.
											if(ship->HasHumanOfficer())
											{
												AuCommChat msg(QObject::tr("We are in orbit at %1.").arg(destPlanet->Name()),
														AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
														Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
												Say(msg, AuVisage::Closed);
											}
										}
									}
								}
								else
								{
									//qDebug("%d %s clear route at dest planet", Id().GetID(), (const char*)Name().toAscii());
									// ship is in orbit range of a spherical planet.
									ship->RemoveAllWaypoints();

									// Announce my action.
									if(ship->HasHumanOfficer())
									{
										AuCommChat msg(QObject::tr("We are orbiting %1.").arg(destPlanet->Name()),
												AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
												Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
										Say(msg, AuVisage::Closed);
									}
								}
							}
							else if((destOrder == AuMissionOrder::GoToOrbit ||
										destOrder == AuMissionOrder::GoToOrbitCoords) &&
									destPlanet != system->FindDestinationWorld(ship->MissionOrderDestinationId(),
										ship->MissionOrderDestinationName()))
							{
								// Mission order is to orbit and it's not the dest planet. This condition indicates we are ordered
								// to orbit a ring. The destAtom is the parent planet of the ring.
								// ship is in the zone of the ring's parent planet
								ship->RemoveAllWaypoints();

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									AuCommChat msg(QObject::tr("We are in orbital proximity of %1, the ring's parent planet.")
											.arg(destPlanet->Name()),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Closed);
								}
							}
							else if(destPlanet->IsInAsteroidBelt())
							{
								// Destination is an asteroid belt object and it is my parent. I am there.
								// ship is in the zone of the asteroid belt
								ship->RemoveAllWaypoints();

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									AuCommChat msg(QObject::tr("We are in orbital proximity of %1.").arg(destPlanet->Name()),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Closed);
								}
							}
						}
					}
					else if(destPlanet == destAtom)
					{
						// The destination atom IS a planet but not the one the ship is currently at.
						//qDebug() << "Destination atom IS a planet but not the one the ship is currently at."
							//<< ship->WaypointCount();
						if(ship->HasWaypoint())
						{
							// The ship already has a course set. Does it take me to the destination atom's planet?
							const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
							AuId wpDest(wp->Destination());
							AuPoint wpDestOff(wp->DestinationOffset());
							ship->UnlockWaypoint();

							if(!destAtom->Is(wpDest))
							{
								// The final waypoint is not to the destination atom's planet.
								//qDebug() << "changing course for planet" << destAtom->Id() << destAtom->Name() <<
									//"offset" << destOffset << "wp id" << wp->Destination() << "offset" << wp->DestinationOffset();
								ship->RemoveAllWaypoints();
							}
							else if(destOffset.Distance2(wpDestOff) > 10000)	// Fuzzy doesn't work; 100' close enough
							{
								// The final waypoint is to the destination atom's planet but the position has changed.
								// When the planet doesn't change, we remove and replace the waypoint in one shot because the
								// destination offset may creep. If I just remove it and wait to add it again, the pilot
								// stops-and-goes in an obnoxious way.
								//qDebug() << "changing course for planet loc" << destAtom->Id() << destAtom->Name() <<
									//"offset" << destOffset << "wp id" << wp->Destination() << "offset" << wp->DestinationOffset();
								ship->RemoveAllWaypoints();

								// Add waypoint at dest planet.
								ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);
							}
							//else
								//qDebug("already going to planet");
						}
						else
						{
							//qDebug("setting course for planet %d %s",
									//destAtom->Id().GetID(), (const char*)destAtom->Name().toAscii());
							// Add waypoint at dest planet.
							ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Plotting course to planet %1.").arg(destPlanet->Name()),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Closed);
							}
						}
					}
					else
					{
						// The ship is not at the destination atom's planet and the destination atom is not a planet.
						//qDebug() << "The ship is not at the destination atom's planet and the destination atom is not a planet.";
						// Nav destination is the planet that hosts the destAtom.
						AuPoint planetLoc;						// Destination atom location relative to destination atom's planet.
						destAtom->GetLocationRelativeToParent(planetLoc, destPlanet);
						if(ship->HasWaypoint())
						{
							// The ship already has a course set. Does it take me to the destination atom's planet?
							const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
							AuId wpDest(wp->Destination());
							AuPoint wpDestOff(wp->DestinationOffset());
							ship->UnlockWaypoint();

							if(!destPlanet->Is(wpDest))
							{
								// The final waypoint is not to the destination atom's planet.
								//qDebug() << "changing course for planet" << destPlanet->Id() << destPlanet->Name() <<
									//"offset" << destOffset << "wp id" << wp->Destination() << "offset" << wp->DestinationOffset();
								ship->RemoveAllWaypoints();
							}
							else if(planetLoc.Distance2(wpDestOff) > 10000)	// Fuzzy doesn't work; 100' close enough
							{
								// The final waypoint is to the destination atom's planet but the position has changed.
								// When the planet doesn't change, we remove and replace the waypoint in one shot because the
								// destination offset may creep. If I just remove it and wait to add it again, the pilot
								// stops-and-goes in an obnoxious way.
								//qDebug() << "changing course for planet loc" << destPlanet->Id() << destPlanet->Name() <<
									//"offset" << destOffset << "wp id" << wp->Destination() << "offset" << wp->DestinationOffset();
								ship->RemoveAllWaypoints();
								// Add waypoint at dest planet.
								ship->AddWaypoint(system->Id(), destPlanet->Id(), planetLoc);
							}
							//else
								//qDebug("already going to planet");
						}
						else
						{
							//qDebug("setting course for planet %d %s",
									//destPlanet->Id().GetID(), (const char*)destPlanet->Name().toAscii());
							// Add waypoint at dest planet.
							ship->AddWaypoint(system->Id(), destPlanet->Id(), planetLoc);

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Plotting course to %1 at %2.")
										.arg(destAtom->Name())
										.arg(destPlanet->Name()),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Closed);
							}
						}
					}
				}
				else
				{
					// Destination is not a planet and destination is not at a planet.
					const DaStar* destStar = destAtom->FindParentStarInEnv();	// Returns destAtom if it is a star.
					if(destStar)
					{
						// Destination is at a star or destination is a star.
						if(destStar == ship->FindParentStarInEnv())
						{
							// The ship is at the destination atom's star. Dest atom could be that star.
							if(destStar == destAtom)
							{
								// The ship's parent star is the destination of the mission.
								// When the star is a great distance away, we must navigate to the star with waypoints.
								AuPoint starLoc;
								ship->GetLocationRelativeToParent(starLoc, destStar);
								double farDist = destStar->AtmosphereRadius() + FeetPerOrbit()/2;	// Dist when waypoints needed.
								if(starLoc.Length2() > farDist*farDist)
								{
									// Ship is a long distance from the star.
									// Set a waypoint at the star.
									if(ship->HasWaypoint())
									{
										// The ship already has a course set. Does it take me to the destination atom's star?
										const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
										AuId wpDest(wp->Destination());
										AuPoint wpDestOff(wp->DestinationOffset());
										ship->UnlockWaypoint();

										if(!destAtom->Is(wpDest))
										{
											// The final waypoint is not to the destination atom's star.
											ship->RemoveAllWaypoints();
										}
										else if(destOffset.Distance2(wpDestOff) > 10000)	// 100' close enough
										{
											// The final waypoint takes me to the destination atom's star but the position has
											// changed. When the star doesn't change, we remove and replace the waypoint in one
											// shot because the destination offset may creep. If I just remove it and wait to add
											// it again, the pilot stops-and-goes in an obnoxious way.
											ship->RemoveAllWaypoints();
											// Add wp at star
											ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);
										}
										//else
											//qDebug("already going to star");
									}
									else
									{
										//qDebug("setting course for star %d %s",
												//destAtom->Id().GetID(), (const char*)destAtom->Name().toAscii());
										// Add wp at star
										ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);

										// Announce my action.
										if(ship->HasHumanOfficer())
										{
											AuCommChat msg(QObject::tr("Plotting course to approach star %1.")
													.arg(destAtom->Name()),
													AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
													Galaxy(), AuCommStdChannel::Close,
													AuCommStdChannel::CrewMates);
											Say(msg, AuVisage::Closed);
										}
									}
								}
								else
								{
									// Ship is in close range of the star.
									if(ship->HasWaypoint())
									{
										// Decide if the nav route can be cleared.
										if(destOrder != AuMissionOrder::GoToOrbit &&
												destOrder != AuMissionOrder::GoToOrbitCoords)
										{
											//qDebug() << "clear route near dest star" << Id() << Name();
											// same star close enough to clear route unless orbiting
											ship->RemoveAllWaypoints();

											// Announce my action.
											if(ship->HasHumanOfficer())
											{
												AuCommChat msg(QObject::tr("We are in close proximity to %1.")
														.arg(destStar->Name()),
														AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
														Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
												Say(msg, AuVisage::Closed);
											}
										}
										else if(ship->CGAltitude() < destStar->AtmosphereRadius() + OrbitAltAboveSun() +
												100*FeetPerMeter())
										{
											// Within CG altitude range of photosphere.
											// ship is in orbit range of a spherical star.
											ship->RemoveAllWaypoints();

											// Announce my action.
											if(ship->HasHumanOfficer())
											{
												QString text;
												if(destStar->IsAnyBlackHole())
													text = QObject::tr("We are orbiting the photon sphere of the %1.")
														.arg(destStar->Name());
												else
													text = QObject::tr("We are orbiting the photosphere of %1.")
														.arg(destStar->Name());
												AuCommChat msg(text,
														AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
														Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
												Say(msg, AuVisage::Grim);
											}
										}
									}
								}
							}
							else
							{
								// The destination atom is subordinate to the same star as the ship.
								// The destination atom is not the star itself.
								// Our route should take us directly to the destination atom then.
								if(dynamic_cast<const DaUnit*>(destAtom) || destAtom->AtomType() == ATOM_Asteroid)
								{
									// The destination atom is a unit or an asteroid.
									// Clear the route so the pilot can take over the pursuit.
									if(ship->HasWaypoint())
										ship->RemoveAllWaypoints();
								}
								else if(ship->HasWaypoint())
								{
									// The destination atom is not a unit.
									// The ship already has a course set. Does it take me to the dest atom?
									const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
									AuId wpDest(wp->Destination());
									AuPoint wpDestOff(wp->DestinationOffset());
									ship->UnlockWaypoint();

									if(!destAtom->Is(wpDest) || !destOffset.FuzzyEquals(wpDestOff))
									{
										// The final waypoint is not to the dest atom.
										//qDebug("changing course for %d %s", destAtom->Id().GetID(),
												//(const char*)destAtom->Name().toAscii());
										ship->RemoveAllWaypoints();
									}
									//else
										//qDebug("already going to dest");
								}
								else
								{
									// The destination atom is not a unit and the ship has no waypoint route set.
									// Add wp at dest atom.
									ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);

									// Announce my action.
									if(ship->HasHumanOfficer())
									{
										AuCommChat msg(QObject::tr("Plotting course to %1.").arg(destAtom->Name()),
												AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
												Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
										Say(msg, AuVisage::Closed);
									}
								}
							}
						}
						else if(destStar == destAtom)
						{
							// The destination atom IS a star but not the one the ship is currently at.
							if(ship->HasWaypoint())
							{
								// The ship already has a course set. Does it take me to the destination atom's star?
								const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
								AuId wpDest(wp->Destination());
								AuPoint wpDestOff(wp->DestinationOffset());
								ship->UnlockWaypoint();

								if(!destAtom->Is(wpDest))
								{
									// The final waypoint is not to the destination atom's star.
									ship->RemoveAllWaypoints();
								}
								else if(destOffset.Distance2(wpDestOff) > 10000)	// 100' close enough
								{
									// The final waypoint is to the destination atom's star but the position has changed.
									// When the star doesn't change, we remove and replace the waypoint in one shot because the
									// destination offset may creep. If I just remove it and wait to add it again, the pilot
									// stops-and-goes in an obnoxious way.
									ship->RemoveAllWaypoints();
									// Add waypoint at dest star
									ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);
								}
								//else
									//qDebug("already going to star");
							}
							else
							{
								//qDebug("setting course for star %d %s",
										//destAtom->Id().GetID(), (const char*)destAtom->Name().toAscii());
								// Add waypoint at dest star
								ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									AuCommChat msg(QObject::tr("Plotting course to star %1.").arg(destAtom->Name()),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close,
											AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Closed);
								}
							}
						}
						else
						{
							// The ship is not at the destination atom's star and the destination atom is not a star.
							// Nav destination is the star that hosts the destAtom.
							AuPoint starLoc;					// Destination atom location relative to destination atom's star.
							destAtom->GetLocationRelativeToParent(starLoc, destStar);
							if(ship->HasWaypoint())
							{
								// The ship already has a course set. Does it take me to the destination atom's star?
								const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
								AuId wpDest(wp->Destination());
								AuPoint wpDestOff(wp->DestinationOffset());
								ship->UnlockWaypoint();

								if(!destStar->Is(wpDest))
								{
									// The final waypoint is not to the destination atom's star.
									ship->RemoveAllWaypoints();
								}
								else if(starLoc.Distance2(wpDestOff) > 10000)	// Fuzzy doesn't work; 100' close enough
								{
									// The final waypoint is to the destination atom's planet but the position has changed.
									// When the planet doesn't change, we remove and replace the waypoint in one shot because the
									// destination offset may creep. If I just remove it and wait to add it again, the pilot
									// stops-and-goes in an obnoxious way.
									ship->RemoveAllWaypoints();
									// Add waypoint at dest star.
									ship->AddWaypoint(system->Id(), destStar->Id(), starLoc);
								}
								//else
									//qDebug("already going to star");
							}
							else
							{
								// Add waypoint at dest star.
								ship->AddWaypoint(system->Id(), destStar->Id(), starLoc);

								// Announce my action.
								if(ship->HasHumanOfficer())
								{
									AuCommChat msg(QObject::tr("Plotting course to %1 at star %2.")
											.arg(destAtom->Name())
											.arg(destStar->Name()),
											AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
											Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
									Say(msg, AuVisage::Closed);
								}
							}
						}
					}
					else
					{
						// The destination atom is not at a planet or star and it is not a planet or star itself.
						// Our route should take us directly to the destination atom then.
						if(dynamic_cast<const DaUnit*>(destAtom) || destAtom->AtomType() == ATOM_Asteroid)
						{
							// The destination atom is a unit or an asteroid.
							// Clear the route so the pilot can take over the pursuit.
							if(ship->HasWaypoint())
								ship->RemoveAllWaypoints();
						}
						else if(ship->HasWaypoint())
						{
							// The destination atom is not a unit.
							// The ship already has a course set. Does it take me to the dest atom?
							const DeWaypoint* wp = ship->LockWaypoint(ship->WaypointCount()-1);
							AuId wpDest(wp->Destination());
							AuPoint wpDestOff(wp->DestinationOffset());
							ship->UnlockWaypoint();

							if(!destAtom->Is(wpDest) || !destOffset.FuzzyEquals(wpDestOff))
							{
								// The final waypoint is not to the dest atom.
								//qDebug("changing course for %d %s", destAtom->Id().GetID(),
										//(const char*)destAtom->Name().toAscii());
								ship->RemoveAllWaypoints();
							}
							//else
								//qDebug("already going to dest");
						}
						else
						{
							// The destination atom is not a unit and the ship has no waypoint route set.
							//qDebug("setting course for %d %s", destAtom->Id().GetID(), (const char*)destAtom->Name().toAscii());
							// Add a waypoint at the dest atom.
							ship->AddWaypoint(system->Id(), destAtom->Id(), destOffset);

							// Announce my action.
							if(ship->HasHumanOfficer())
							{
								AuCommChat msg(QObject::tr("Plotting course to %1.").arg(destAtom->Name()),
										AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
										Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
								Say(msg, AuVisage::Closed);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// Destination atom was not found in this solar system.
		//if(destOrder == AuMissionOrder::Follow)
			//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "aboard" << ship->Id() << ship->Name()
				//<< "follow target not found in system, route waypoints" << ship->WaypointCount();
		if(ship->HasWaypoint())
		{
			// Clear the route and figure it out again.
			if(destOrder == AuMissionOrder::Follow && ship->WaypointCount() == 1)
			{
				// When following a unit, do not clear the route if the only waypoint is at a wormhole in this system.
				// It is probably there to pursue the target through the wormhole.
				const DaAtom* system = ship->FindEnvironmentAtom();
				Q_ASSERT(system);
				const DeWaypoint* wp = ship->LockWaypoint(0);
				AuId wpSystem(wp->m_idSystem);
				AuId wpDest(wp->Destination());
				AuPoint wpDestOff(wp->DestinationOffset());
				ship->UnlockWaypoint();

				if(wp && system->Is(wpSystem))
				{
					// Waypoint was found to a destination in this system.
					AuWormhole wh;
					const DaSolarSystem* solar = dynamic_cast<const DaSolarSystem*>(system);
					if(solar && solar->Is(wpDest))
						wh = solar->FindWormhole(wpDestOff);
					if(!wh.IsValid())
					{
						// Waypoint to destination in this system does not go to a wormhole.
						//qDebug("DaSentient::OperateNavigatorConsole %d %s aboard %d %s no dest or follow target, clearing route",
								//Id().GetID(), (const char*)Name().toAscii(),
								//ship->Id().GetID(), (const char*)ship->Name().toAscii());
						ship->RemoveAllWaypoints();
					}
				}
			}
			else
			{
				//qDebug("DaSentient::OperateNavigatorConsole %d %s aboard %d %s no target, clearing route",
						//Id().GetID(), (const char*)Name().toAscii(),
						//ship->Id().GetID(), (const char*)ship->Name().toAscii());
				ship->RemoveAllWaypoints();
			}
		}
		else if(destOrder == AuMissionOrder::Follow && IsOkToLeaveSolarSystem(ship))
		{
			// The ship is following someone and we have no waypoints. The target was not found in this system and it is ok to
			// leave the solar system.
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->Parent());
			if(system && ship->Vector().Length2() > 1)
			{
				// This ship is moving in deep space. The target was destroyed, departed this solar system, or I have followed it
				// into another solar system and the target is not here.
				// If my ship is moving and I am heading away from the solar system, set a waypoint for the solar system that is
				// in front of me.
				AuPoint mvec = ship->Transform().ProjectVector(ship->Vector());	// Project movement vector into space of system.
				mvec.Unit();
				AuPoint locvec = ship->Location();
				locvec.Unit();									// Vector toward the ship in the space of the solar system.
				//qDebug() << "DaSentient::OperateNavigatorConsole" << Id() << Name() << "aboard" << ship->Id() << ship->Name()
					//<< "movement dot" << mvec.Dot(locvec) << "mvec" << mvec << "locvec" << locvec;
				if(mvec.Dot(locvec) > .9848)
				{
					// Current vector is taking us away from the solar system, within 10 degrees of the vector straight away from
					// the system. Do some work to figure out where we are probably going and set a waypoint there.
					const DaSector* sector = system->FindParentSector();
					Q_ASSERT(sector);

					// Project my location and movement vector into the space of the sector.
					AuPoint secloc;
					ship->GetLocationRelativeToParent(secloc, sector);
					AuPoint secvec = system->Transform().ProjectVector(mvec);	// Avoid scaling by GetTransformRelativeToParent.
					secvec.Unit();

					// Get locations of all stars in sector and its neighboring sectors, in coordinates relative to sector.
					DqAtomLocArray aStars;
					sector->ListSolarSystemLocations(aStars);

					// Determine which star we are heading toward.
					// Select the closest star that is within 10 degrees of our current movement vector.
					double bestDist2 = 0;						// Squared distance to the best system found so far.
					const DaSolarSystem* bestSystem = 0;
					for(DqAtomLocArray::const_iterator ali = aStars.cbegin(); ali != aStars.cend(); ali++)
					{
						if((*ali).m_pA == this)
							continue;							// Skip current system.

						// Calculate sector-relative vector from the ship to the other solar system.
						AuPoint vecToSys = (*ali).m_fLocation - secloc;
						double dist2 = vecToSys.Length2();		// Squared length of the vector.
						Q_ASSERT(dist2 != 0);
						if(bestSystem && dist2 > bestDist2)
							continue;							// Skip if we have already found one that is closer.

						vecToSys /= sqrt(dist2);				// Calculate unit vector to system from ship.
						double vecdot = secvec.Dot(vecToSys);
						if(vecdot > .984807753)					// cosine of 10 degrees
						{
							// Vector to the system from ship location is within 10 degrees of ship's current movement vector.
							bestSystem = dynamic_cast<const DaSolarSystem*>((*ali).m_pA);
							Q_ASSERT(bestSystem);
							bestDist2 = dist2;
						}
					}

					if(bestSystem)
					{
						// We found the system closest to being in front of our path.
						// Add a waypoint directly to the best system.
						ship->AddWaypoint(bestSystem->Id(), bestSystem->Id(), AuPoint());

						// Announce my action.
						if(ship->HasHumanOfficer())
						{
							AuCommChat msg(QObject::tr("Course laid to %1 in pursuit of target.").arg(bestSystem->Name()),
									AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
									Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
							Say(msg, AuVisage::Grim);
						}
					}
				}
			}
		}
	}

	StayIdle();
}

// DaSentient::OperateHelmConsole(DaVessel* ship)
void DaSentient::OperatePilotConsole(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate when there is no power to the ship.
	bool canTurn = (ship->CapacitorLevel() > 0) ? true : false;

	// Don't do anything if a person is manning one of these consoles. This also notices command console in this mode.
	// Don't do anything if the ship is getting repairs.
	if(ship->IsPilotMannedByPerson() || ship->RepairSecondsRemaining())
	{
		// Rate of thinking when operating helm is dependent on changing ship factors so we must update it every time we think.
		UpdateThinkRate();

		if(canTurn)
		{
			if(AIThinkRate() < 200)
			{
				// I think fast when running the helm. This test keeps me from being too jittery. I'll turn once every odd second.
				m_SentientLock.lockForRead();
				bool v = m_tmLastThought.SecIsOdd() && m_tmLastThought.ElapsedSecTicks() > 0;
				m_SentientLock.unlock();
				if(v)
					StayIdle();
			}
			else
				StayIdle();
		}
		return;
	}
	else if(ship->IsNew())
	{
		// I am at the helm of a nu ship so turn the systems on if there is fuel.
		if(!ship->IsPowerPlantOn() && ship->FuelQuantity() > 0)
		{
			// This is the only action I will perform in this thought cycle.
			// Turn power plant on if ship has fuel.
			DoShipPowerPlantState(ship, true);
		}
		else if(canTurn)
		{
			// Ship has power.
			if(ship->InteriorLight(ShipRoomName()) != INTLIGHT_Bright)
			{
				// This is the only action I will perform in this thought cycle.
				// Turn interior lights on.
				DoShipInteriorLightState(ship, INTLIGHT_Bright, ShipRoomName());
			}
			else if(ship->GravityDriveState() != DaVessel::GD_On)
			{
				// This is the only action I will perform in this thought cycle.
				// Turn on the gravity drive if it's not already on.
				// This clears the NEW state of the ship.
				DoShipGravityDriveState(ship, DaVessel::GD_On);
				DoShipKillThrottles(ship);
			}
		}

		// Rate of thinking when operating helm is dependent on changing ship factors so we must update it every time we think.
		UpdateThinkRate();
		return;
	}

	// Close hull doors when inside air pressure is about to become unbreathable.
	float shipOutsideAir = ship->AtmosphereDensity();
	if(shipOutsideAir < .15f)
	{
		// Outside air pressure is too thin to breath.
		if(AirDensity() < .2f)
		{
			// Interior air pressure in my room is almost too low.
			// Close the hull doors if doing so will restore a breathable environment.
			// Don't imprison people inside a ship that will kill them.
			if(ship->AnyHullDoorOpen() && ship->LifeSupportWorking())
				DecideShipCloseAllHullDoors();
		}
	}
	else if(shipOutsideAir > .85f)
	{
		// Outside air pressure is too thick to breath.
		if(AirDensity() > .8f)
		{
			// Interior air pressure in my room is almost too high.
			// Close the hull doors if doing so will restore a breathable environment.
			// Don't imprison people inside a ship that will kill them.
			if(ship->AnyHullDoorOpen() && ship->LifeSupportWorking())
				DecideShipCloseAllHullDoors();
		}
	}

	// Call out when we enter or leave the range of an enemy bay weapon.
	if(!ship->IsPirateWithFreeWill())
	{
		// Every other second is often enough to do this.
		m_SentientLock.lockForRead();
		bool v = m_tmLastThought.SecIsOdd() && m_tmLastThought.ElapsedSecTicks() > 0;
		m_SentientLock.unlock();
		if(v)
			DecideShipPilotReportContacts();
	}

	AuMissionOrder::TYPE t = ship->MissionOrderType();
	if(t == AuMissionOrder::LightState)
	{
		// Configure the lights.
		bool headlight = ship->MissionOrderHeadlightOn();
		bool navLight = ship->MissionOrderNavLightOn();
		bool beacon = ship->MissionOrderRotatingBeaconOn();
		bool strobe = ship->MissionOrderStrobeOn();
		DoShipLightState(ship, headlight, navLight, beacon, strobe);
	}

	// Moor or unmoor the ship.
	if(t == AuMissionOrder::Repair)
	{
		// Repairing ship. Makes the appropriate tests, like am I already moored, is a station in range, etc.
		if(!ship->IsMoored())
			DoShipMoorState(ship, true);
	}
	else
	{
		// Not repairing ship and I'm in control. Release moorings if I am going somewhere.
		if(ship->IsMoored() && (ship->MissionOrderIsTravelOrder() || ship->HasWaypoint()))
			DoShipMoorState(ship, false);
	}

	if(ship->IsMoored())
	{
		// The ship is moored.
		if(ship->IsAtRest())
		{
			// This ship is at rest.
			if(ship->GravityDriveState() != DaVessel::GD_Off)
			{
				// Turn off the gravity drive if it's on.
				DoShipGravityDriveState(ship, DaVessel::GD_Off);
				DoShipKillThrottles(ship);
			}
			else if(ship->IsPowerPlantOn())
			{
				// Turn off power plant if it's on.
				DoShipPowerPlantState(ship, false);
			}
		}
		else if(!ship->IsOnGround())
		{
			// Moored but still moving and not on the ground; insure power plant and gravity drive are on.
			if(!ship->IsPowerPlantOn() && ship->FuelQuantity() > 0)
			{
				// Turn on the power plant if it's not already on.
				DoShipPowerPlantState(ship, true);
			}
			else if(canTurn && ship->GravityDriveState() != DaVessel::GD_On)
			{
				// Turn on the gravity drive if it's not already on.
				DoShipGravityDriveState(ship, DaVessel::GD_On);
				DoShipKillThrottles(ship);
			}
		}
	}
	else if(ship->IsSpaceStation())
	{
		// I'm at the helm of a station.
		// Stations must have their power plant and gravity drive on at all times.
		if(!ship->IsPowerPlantOn() && ship->FuelQuantity() > 0)
		{
			// Turn on the power plant if it's not already on.
			DoShipPowerPlantState(ship, true);
		}
		else if(canTurn)
		{
			// Station has power.
			if(ship->GravityDriveState() != DaVessel::GD_On)
			{
				// Turn on the gravity drive if it's not already on.
				DoShipGravityDriveState(ship, DaVessel::GD_On);
				DoShipKillThrottles(ship);
			}
			else if(!ship->IsAtRest())
			{
				// Put on the brakes if the station starts moving somehow.
				DoShipKillThrottles(ship);
				if(ship->InBlackHoleGravity())
					DoShipBrakeState(ship, true, true);
				else
					DoShipBrakeState(ship, ship->ForwardSpeed(), ship->LateralSpeed() || ship->VerticalSpeed());
			}
		}
	}
	else
	{
		// I'm at the helm of a spacecraft.
		// If I have a waypoint I must be going somewhere.
		if(ship->HasWaypoint())
			PilotToWaypoint(ship);		// Fly the ship to the waypoint.
		else
		{
			// The ship has no waypoints.
			// Does the ship's mission require me to drive somewhere?
			AuPoint destLoc;
			AuMissionOrder::TYPE destOrder;
			const DaAtom* destAtom = ship->MissionDestination(destOrder, destLoc);
			if(!destAtom || destAtom == ship->FindEnvironmentAtom())
			{
				// Destination atom was not found or it is the solar system, in which case the navigator will put a waypoint there.
				// We could be following a target that left the solar system or was destroyed.
				if(!destAtom &&													// Target atom not found in this system.
						t == AuMissionOrder::Follow &&	// Current order is follow.
						dynamic_cast<const DaSolarSystem*>(ship->Parent()) &&	// Ship is in deep space.
						IsOkToLeaveSolarSystem(ship))							// It's ok to leave the solar system.
				{
					// Lost the target we were following in deep space.
					// Insure power plant and gravity drive are on.
					// Allow warp drive to remain on.
					if(!ship->IsPowerPlantOn() && ship->FuelQuantity() > 0)
					{
						// Turn on the power plant if it's not already on.
						DoShipPowerPlantState(ship, true);
					}
					else if(canTurn)
					{
						// Ship has power.
						if(ship->GravityDriveState() != DaVessel::GD_On)
						{
							// Turn on the gravity drive if it's not already on.
							DoShipGravityDriveState(ship, DaVessel::GD_On);
							DoShipKillThrottles(ship);
						}
						else
						{
							// Pour on the coal on our current heading.
							DoShipThrottleState(ship, 10);
						}
					}
				}
				else
					PilotHoldPositionClearWormhole(ship);
			}
			else
			{
				// Remaining destinations do not need FTL drive to get there.
				if(ship->HasWarpFTLDrive())
				{
					quint16 whichFTL = ship->FTLWarpDrive();
					if(ship->HasHumanOfficer())
						DoShipDisengageFTLDrive(ship, 1, whichFTL, QString());
					else
						DoShipDisengageFTLDrive(ship, 0, whichFTL, QString());
				}

				if(t == AuMissionOrder::Attack)
				{
					// We are ordered to attack a unit and we have found that atom.
					PilotAttackUnit(ship, destAtom);
				}
				else if(t == AuMissionOrder::Plunder)
				{
					// We are ordered to plunder a unit and we have found that atom.
					const DaUnit* destUnit = dynamic_cast<const DaUnit*>(destAtom);
					if(destUnit)
						PilotPlunderUnit(ship, destUnit);
					else
						RequestOrder();
				}
				else if(t == AuMissionOrder::Follow)
				{
					// We are ordered to follow a unit and we have found that atom.
					PilotFollowUnit(ship, destAtom);
				}
				else
				{
					// The destination atom was found.
					const DaStar* star = ship->FindParentStarInEnv();
					if(destAtom == star)
					{
						// Destination atom is the star I am at.
						// Orbit the star.
						PilotToStar(ship, star, destLoc);
					}
					else
					{
						const DaWorld* planet = ship->FindParentPlanetInEnv();
						if(destAtom == planet)
						{
							// Destination atom is the planet I am at.
							// Orbit the planet or land at the planet somewhere.
							PilotToPlanet(ship, planet, destLoc);
						}
						else if(planet && destAtom->FindParentPlanetInEnv() == planet)
						{
							// I am at the same planet as the dest atom and the dest atom is not specifically the planet.
							// The only types of destinations that need careful maneuvering are stations and cities.
							// The navigator gets me close enough to everything else using waypoints.
							const DaVessel* destShip = dynamic_cast<const DaVessel*>(destAtom);
							if(destShip)
								PilotToStation(ship, destShip, planet);
							else
							{
								const DaBd* destBd = dynamic_cast<const DaBd*>(destAtom);
								if(destBd)
									PilotToBd(ship, destBd, planet);
								else
									PilotHoldPosition(ship);
							}
						}
						else
						{
							// I'm not at the same planet as the destination atom.
							// Wait for the navigator to set up the course.
							PilotHoldPosition(ship);
						}
					}
				}
			}
		}
	}

	// Rate of thinking when operating helm is dependent on changing ship factors so we must update it every time we think.
	UpdateThinkRate();

	if(canTurn)
	{
		if(AIThinkRate() < 200)
		{
			// I think fast when running the helm. This test keeps me from being too jittery. I'll turn once every odd second.
			m_SentientLock.lockForRead();
			bool v = m_tmLastThought.SecIsOdd() && m_tmLastThought.ElapsedSecTicks() > 0;
			m_SentientLock.unlock();
			if(v)
				StayIdle();
		}
		else
			StayIdle();
	}
}

void DaSentient::OperatePowerRelayConsole(DaVessel* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
	{
		// Crew at non-critical station watches for threats.
		if(LookForThreats(true))
			return;						// Threat spotted; pursue and destroy; return to my station.
	}

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate when there is no power to the ship.
	bool canTurn = (ship->CapacitorLevel() > 0) ? true : false;

	// Don't do anything if a person is manning one of these consoles. This also notices command console in this mode.
	// Don't do anything if the ship is getting repairs.
	if(ship->IsPowerRelayMannedByPerson() || ship->RepairSecondsRemaining())
	{
		if(canTurn)
			StayIdle();
		//g_Au.Log(QString("DaSentient::OperatePowerRelayConsole %1 %2 preempted, napping").arg(Id().GetID()).arg(Name()));
		TakeANap();
		return;
	}

	// Adjust one thing with the power relay console.
	// Start by trying to turn off a power plant shunt when the power plant is not damaged.
	// If no shunts to turn off, try reconnecting a system that is disconnected.
	// If no disconnected systems, try turning off overclocking on undamaged system.
	DoShipPowerRelayTune(ship);

	if(canTurn)
	{
		StayIdle();
		//g_Au.Log(QString("DaSentient::OperatePowerRelayConsole %1 %2 idle, napping").arg(Id().GetID()).arg(Name()));
		TakeANap();
	}
}

void DaSentient::OperateTransporterConsole(const DaCapsule* ship)
{
	if(!ship->IsDesignDataCacheValid())
		return;

	if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
	{
		// Crew at non-critical station watches for threats.
		if(LookForThreats(true))
			return;						// Threat spotted; pursue and destroy; return to my station.
	}

	// Close jammed doors if the atmosphere inside is bad.
	AuPoint doorLoc;
	if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
	{
		doorLoc = ship->FloorLocInsideDoor(doorLoc);
		AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		return;
	}

	// Exit the station if I somehow ended up in the same seat with somebody else.
	if(ship->IsConsoleOccupiedExcept(StationId(), Id()))
	{
		ExitConsole();
		return;
	}

	// Consoles cannot rotate or operate when there is no power to the ship.
	//DHH if(ship->CapacitorLevel() <= 0)
		//DHH return;

	if(HasConsole())
	{
		// I am manning a transporter console. If the transporter room is on and inactive, turn it off.
		int troomIndex = ship->TransporterSystemIndex(StationId());
		if(ship->IsTransporterSystemOn(troomIndex))
		{
			if(!ship->IsTransporterSystemActive(troomIndex))
				DecideShipTransporterState(troomIndex, false);
		}
		else
		{
			// Transporter room is not on. Take a nap for a while.
			TakeANap();
			TakeANap();
			TakeANap();
			TakeANap();
		}
	}

	StayIdle();
}

void DaSentient::TakeANap()
{
	CITIZEN_ORDER sleepOrders[] =
	{
		ORDER_SleepDayDream,
		ORDER_SleepEat,
		ORDER_SleepExercise,
		ORDER_SleepGame,
		ORDER_SleepLetter,
		ORDER_SleepMovie,
		ORDER_SleepMusic,
		ORDER_SleepPine,
		ORDER_SleepPhone,
		ORDER_SleepRead,
		ORDER_SleepTalk,
		ORDER_SleepText,
		ORDER_SleepTV,
		ORDER_SleepWeb,
	};
	int sleepOrderCount = sizeof(sleepOrders) / sizeof(CITIZEN_ORDER);
	CITIZEN_ORDER sleepOrder = sleepOrders[g_Au.Dice().Roll(1, sleepOrderCount, -1)];
	AmendOrder(sleepOrder, ParentId(), Location(), 0);
}

// Wakes this citizen up if it is napping.
void DaSentient::WakeUp()
{
	QList<CITIZEN_ORDER> sleepOrders;
	sleepOrders << ORDER_SleepDayDream;
	sleepOrders << ORDER_SleepEat;
	sleepOrders << ORDER_SleepExercise;
	sleepOrders << ORDER_SleepGame;
	sleepOrders << ORDER_SleepLetter;
	sleepOrders << ORDER_SleepMovie;
	sleepOrders << ORDER_SleepMusic;
	sleepOrders << ORDER_SleepPine;
	sleepOrders << ORDER_SleepPhone;
	sleepOrders << ORDER_SleepRead;
	sleepOrders << ORDER_SleepTalk;
	sleepOrders << ORDER_SleepText;
	sleepOrders << ORDER_SleepTV;
	sleepOrders << ORDER_SleepWeb;

	while(sleepOrders.contains(Order()))
		RequestOrder();

	// Think ASAP.
	ThinkSoon();
	TickTock();
}

void DaSentient::DoDamageReport()
{
	// Do not send final mail while in arena.
	if(InArena() || InDesignPreview())
		return;

	// Anybody aboard a ship can get a damage report.
	DaVessel* ship = dynamic_cast<DaVessel*>(Parent());
	if(!ship)
	{
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Prepare a report.
	QString title = tr("Damage Report");
	QString rpt = MailHeader(ship, title, "");

	// Damage Report
	rpt += ship->DamageReportHtml();

	// Roster
	rpt += "<br>";
	rpt += "<b>PERSONNEL ROSTER</b><div style=\"margin-left: 10px;\">";
	{
		if(ship->CaptainsBerthType() == AuRosterEntry::CaptainSlot)
			rpt += QString("No Berths, Captain Slot Only<br>");
		else
		{
			if(ship->OfficerBerths())
				rpt += QString("%1 Officer Berths<br>").arg(ship->OfficerBerths());
			if(ship->CrewBerths())
				rpt += QString("%1 Crew Berths<br>").arg(ship->CrewBerths());
			if(ship->TroopBerths())
				rpt += QString("%1 Troop Berths<br>").arg(ship->TroopBerths());
			if(ship->PassengerBerths())
			{
				if(ship->IsFirstClass())
					rpt += QString("%1 1st Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else if(ship->IsSecondClass())
					rpt += QString("%1 2nd Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else
					rpt += QString("%1 3rd Class Passenger Berths<br>").arg(ship->PassengerBerths());
			}
		}

		// Create a map of the roster so it will be alphabetized.
		QMap<QString, const char*> occs;		// Key is occupant name, value is icon resource name.
		const AuRoster& roster = ship->LockRoster();
		for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
			occs.insert((*ri).OccupantName(), (*ri).OccupantIcon());
		ship->UnlockRoster();

		rpt += tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(32, 32, 32);\"><tbody>");
		for(QMap<QString, const char*>::const_iterator oi = occs.cbegin(); oi != occs.cend(); oi++)
			rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td></tr>") .arg(*oi) .arg(oi.key());
		rpt += "</tbody></table><br>";
	}
	rpt += "</div>";

	// Send the sensor scan report by mail to everyone in the chain of command of the officer.
	AuIdList recipients;
	AuIdList places;
	const DgData& gd = g_Au.LockDataForRead();
	if(ship->MissionOrderReportToCommand())
	{
		places.append(AuId::Universe());
		const DaNpc* officer = dynamic_cast<const DaNpc*>(ship->FindOfficer());
		if(officer)
		{
			places.append(officer->HomeGalaxyId());
			places.append(officer->HomeSectorId());
			places.append(officer->HomeSolarSystemId());
			places.append(officer->HomeWorldId());
		}
		gd.EmpireReceivesFleetMessages(ship->EmpireId(), places, recipients);
	}
	unsigned int idPlayer = gd.PlayerId();				// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	// List captain and human officers as recipients.
	const AuRoster& roster = ship->LockRoster();
	for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
		if((*ri).IsHumanOfficer())	// Also includes captain.
			recipients.AddIfUnique((*ri).m_OccId);
	ship->UnlockRoster();

	if(!recipients.isEmpty())
		g_Au.SendBulkMail(idPlayer, confirmNum, ship->Id(), ship->Name(), recipients, AuMailMsg::MSG_ShipReport, title, rpt,
				AuId(0), "", AuId(0), "", GALAXY_Count, AuPoint(0,0,0));

	// We only get one shot at doing this.
	RequestOrder();	// Ask for my next order.
}

// Mail header when not aboard a ship.
QString DaSentient::MailHeader(const QString& subject, const QString& salutation) const
{
	if(InArena() || InDesignPreview())
		return "";

	// If I am aboard a ship, the other MailHeader function can be used.
	const DaVessel* ship = FindParentVesselInEnv();
	if(ship)
		return MailHeader(ship, subject, salutation);
	// I am not aboard a spacecraft.

	QString rpt;
	rpt = g_Au.EncodeUTCString() + "<p>";
	rpt += HtmlName();

	// Create a block of location information.
	rpt += "<div style=\"margin-left: 10px;\">";
	QString locText;
	const DaWorld* planet = FindParentPlanetInEnv();
	if(planet)
	{
		AuPoint planetLoc;
		GetLocationRelativeToParent(planetLoc, planet);
		Q_ASSERT(!planetLoc.IsNaN());

		// Determine my longitude/latitude string on this world.
		QString strLongLat;
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
		if(llworld)
		{
			AuGlobeLLF planetLL;
			if(llworld->LLOfLoc(planetLL, planetLoc))
				strLongLat = planetLL.ToString();
		}

		if(IsOnGround())
		{
			const DaAtom* atom = planet->FindSubatomId(BdId());
			const DaBd* bd = dynamic_cast<const DaBd*>(atom);
			if(bd)
				locText = tr("In %2 at %3.").arg(HtmlName(bd->CityBuildingName())).arg(strLongLat);
			else
				locText = tr("On %2 at %3.").arg(planet->HtmlName()).arg(strLongLat);
		}
		else if(IsInAtmosphere())
			locText = tr("On %2 at %3.").arg(planet->HtmlName()).arg(strLongLat);
		else
			locText = tr("EVA near %2 at %3.").arg(planet->HtmlName()).arg(strLongLat);
	}
	else if(FindParentStarInEnv())
		locText = tr("EVA in space.");
	else
		locText = tr("EVA in deep space.");
	rpt += locText;

	if(planet)
		rpt += "<br>" + planet->HtmlName() + ", " + planet->Description();

	const DaSolarSystem* system = FindParentSolarSystem();
	if(system && !system->IsEmptySystem())
		rpt += "<br>" + system->HtmlNameWithSystem() + ' ' + system->Description();

	const DaSector* sector = FindParentSector();
	if(sector)
		rpt += "<br>" + sector->HtmlName() + ' ' + sector->Description();

	const DaGalaxy* galaxy = FindParentGalaxy();
	if(galaxy)
		rpt += "<br>" + galaxy->HtmlName();

	rpt += "</div><br>";

	// Subject and salutation are the last lines before the body content.
	if(!subject.isEmpty())
		rpt += tr("%1<p>").arg(subject);
	if(!salutation.isEmpty())
		rpt += tr("%1<p>").arg(salutation);
	return rpt;
}

QString DaSentient::MailHeader(const DaVessel* ship, const QString& subject, const QString salutation) const
{
	if(InArena() || InDesignPreview())
		return "";

	QString rpt;
	rpt = g_Au.EncodeUTCString() + "<p>";
	rpt += HtmlName();

	// Create a block of location information.
	rpt += "<div style=\"margin-left: 10px;\">";
	QString locText;
	const DaWorld* planet = ship->FindParentPlanetInEnv();
	if(planet)
	{
		AuPoint planetLoc;
		ship->GetLocationRelativeToParent(planetLoc, planet);
		Q_ASSERT(!planetLoc.IsNaN());

		// Determine my longitude/latitude string on this world.
		QString strLongLat;
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
		if(llworld)
		{
			AuGlobeLLF planetLL;
			if(llworld->LLOfLoc(planetLL, planetLoc))
				strLongLat = planetLL.ToString();
		}

		if(ship->IsOnGround())
		{
			const DaAtom* atom = planet->FindSubatomId(BdId());
			const DaBd* bd = dynamic_cast<const DaBd*>(atom);
			if(bd)
				locText = tr("On the ground in %2 at %3.").arg(HtmlName(bd->CityBuildingName())).arg(strLongLat);
			else
				locText = tr("On the ground on %2 at %3.").arg(planet->HtmlName()).arg(strLongLat);
		}
		else if(ship->IsInAtmosphere())
			locText = tr("Flying over %2 at %3.").arg(planet->HtmlName()).arg(strLongLat);
		else if(!ship->IsMoving())
			locText = tr("Orbiting %2 at %3.").arg(planet->HtmlName()).arg(strLongLat);
		else
			locText = tr("Maneuvering near %2 at %3.").arg(planet->HtmlName()).arg(strLongLat);
	}
	else if(ship->FindParentStarInEnv())
	{
		if(!ship->IsMoving())
			locText = tr("Holding our position in space.");
		else
			locText = tr("Traveling through space.");
	}
	else if(!ship->IsMoving())
		locText = tr("Holding our position in deep space.");
	else
		locText = tr("Traveling through deep space.");
	rpt += locText;

	rpt += "<br>" + ship->SpotDescription(ship->EmpireId());

	if(planet)
		rpt += "<br>" + planet->HtmlName() + ", " + planet->Description();

	const DaSolarSystem* system = ship->FindParentSolarSystem();
	if(system && !system->IsEmptySystem())
		rpt += "<br>" + system->HtmlNameWithSystem() + ' ' + system->Description();

	const DaSector* sector = ship->FindParentSector();
	if(sector)
		rpt += "<br>" + sector->HtmlName() + ' ' + sector->Description();

	rpt += "</div><br>";

	// Subject and salutation are the last lines before the body content.
	if(!subject.isEmpty())
		rpt += tr("%1<p>").arg(subject);
	if(!salutation.isEmpty())
		rpt += tr("%1<p>").arg(salutation);
	return rpt;
}

void DaSentient::DoScan()
{
	// Average citizen doesn't know how to do a sensor scan.
	RequestOrder();	// Ask for my next order.
}

void DaSentient::DoTakeCommand()
{
	// Locate my parent ship atom.
	DaVessel* ship = FindParentVesselInEnv();
	if(!ship)
	{
		RequestOrder();										// Ask for my next order.
		return;
	}

	// Am I aboard my assigned spacecraft?
	if(!ship->Is(AssignedShip()))
	{
		RequestOrder();										// Ask for my next order.
		return;
	}

	// Take the con.
	TakeTheCon();

	// Average citizen doesn't know how to take command.
	RequestOrder();	// Ask for my next order.
}

void DaSentient::DoReleaseCommand()
{
	// Release the con, if I have it.
	ReleaseTheCon();
	RequestOrder();	// Ask for my next order.
}

void DaSentient::DoDropRider()
{
	DismountAllRiders();
	RequestOrder();
}

void DaSentient::DoMountMe()
{
	//qDebug() << "DaSentient::DoMountMe" << Id() << Name();
	DoMountTarget();
}

void DaSentient::DoMountTarget()
{
	//qDebug() << "DaSentient::DoMountTarget" << Id() << Name();
	if(OrderTarget() == ParentId())
	{
		// I was ordered to mount my current parent atom so my order is complete.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Must exit my console or turret.
	if(IsLockedPose())
	{
		ExitConsole();
		return;
	}

	if(IsRider())
	{
		// I am on a mount.
		DismountCreature();
		return;
	}

	// Give up on the current order when it has taken too long to complete.
	if(!HasOrder() || OrderTime().ElapsedSec() > 60)
	{
		// It has taken too long for me to complete the order.
		// Give up rather than load the server forever.
		RequestOrder();	// Ask for my next order.
		return;
	}

	DaAtom* orderTarget = 0;
	int result = MoveToOrderLoc(IsInSpacecraft()?MOVE_CLOSE:MOVE_NEAR, true, orderTarget);
	if(result == 2)
	{
		RequestOrder();	// Ask for my next order.

		// When there is no path, sleep for a while. Otherwise I can use up a lot of CPU time on the server.
		TakeANap();
	}
	else if(result == 1)
	{
		// I am close to my target.
		// If it is a creature, mount up.
		DaCreature* creature = dynamic_cast<DaCreature*>(orderTarget);
		if(creature)
			MountCreatureIfPossible(creature);
		RequestOrder();
	}
}

bool DaSentient::MountCreatureIfPossible(DaCreature* creature)
{
	if(IsMounted())
		return false;	// I am mounted.
	if(Is(creature->Id()))
		return false;	// I am the creature to be mounted.
	if(creature->IsBodyDyingOrDead())
		return false;	// Creature body is dying or dead
	if(creature->IsLockedPose())
		return false;	// Creature is sitting at a console or turret.
	if(creature->IsMounted())
		return false;	// Creature is mounted.
	if(creature->IsRider())
		return false;	// Creature is riding a mount.

	POLITICAL_STANCE stance = PoliticalStanceMakeContact(creature);
	DaAnimal* animal = dynamic_cast<DaAnimal*>(creature);
	if(animal)												// Target is an animal.
	{
		/*
		if(animal->Volume() >= Volume() * .9f)				// Animal is big enough.
		{
			// AI dudes do not know how to ride animals.
			if(!animal->IsWild())								// Animal is not wild.
			{
				// Animal meets all criteria for mounting.
				// Capture animals that belong to the enemy.
				if(stance == POL_Enemy)
				{
					animal->SetEmpireId(EmpireId());
					animal->SetModified();
				}
				MountCreature(creature);					// Animal is ok to mount.

				// Look straight ahead.
				SetTurnZ(0);
				if(TiltHeadTo(0))
					SetModified(NOTIFY_Scene);			// Head tilt not stored in db but client likes to know.
				return true;
			}
		}
		*/
	}
	else
	{
		DaPerson* person = dynamic_cast<DaPerson*>(creature);
		if(person)												// Target is a person.
		{
			if(!person->IsEthereal())
			{
				// Person meets all criteria for mounting except possibly their pose.
				// We will return success here so the caller will let me keep trying until the target crouches.
				AuPose curPose = person->CurrentGeneralPose();
				if(curPose == AuPose::Crouch || curPose == AuPose::CrouchCreep)	// Person is crouching.
				{
					if(stance == POL_Enemy)
						return false;
					MountCreature(creature);					// Person is ok to mount piggyback.
				}
				else if(!IsMarried() && TryMating(*person))
					MateWithCreature(person);					// Person IsReadyToMate, properly posed, and IsSexuallyCompatible.
				if(!IsRider())
					return false;
				// I am aboard. Look straight ahead.
				SetTurnZ(0);
				if(TiltHeadTo(0))
					SetModified(NOTIFY_Scene);					// Head tilt not stored in db but client likes to know.
				return true;
			}
		}
		else
		{
			DaCitizen* citizen = dynamic_cast<DaCitizen*>(creature);
			if(citizen)											// Target is a citizen
			{
				if(stance != POL_Enemy && !AssignedShip().IsNull() && citizen->AssignedShip() == AssignedShip())
				{
					// Citizen is not an enemy and we are assigned to the same ship.
					if(!IsMarried() && !citizen->IsMarried() && citizen->TryBecomingReceptiveToMating(*this))
					{
						// Unmarried citizen will have sex with me.
						MateWithCreature(citizen);
						if(citizen->IsMating())
							citizen->PoseVisage(AuVisage::Grin, AuVisage::Smile);
					}
					else
						MountCreature(citizen);					// Citizen is ok to mount.

					// Look straight ahead.
					SetTurnZ(0);
					if(TiltHeadTo(0))
						SetModified(NOTIFY_Scene);				// Head tilt not stored in db but client likes to know.
					return true;
				}
			}
		}
	}
	return false;
}

// Sets m_AIIsAlert and notifies comrades in the area.
void DaSentient::BecomeAlert()
{
	if(!AIIsAlert())
	{
		WakeUp();						// Awaken from my nap, if sleeping.
		SetAIIsAlert(true);				// Become alert. This helps my fellow comrades to notice there is a problem.
	}
}

// Sets m_AIIsAlert and notifies comrades in the area.
void DaSentient::BecomeAlert(const QString& sayWhat)
{
	BecomeAlert();

	if(sayWhat.isEmpty())
		return;

	if(AssignedShip().IsNull())
	{
		// Announce on voice channel.
		AuCommChat msg(sayWhat, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		SayNoRepeat(msg, AuVisage::Closed);
	}
	else
	{
		// Announce on crew channel.
		AuCommChat msg(sayWhat, AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, AuCommStdChannel::CrewMates);
		SayNoRepeat(msg, AuVisage::Closed);
	}

	// Alert troops in my area who would respond to my call.
	DaAtom* parent = Parent();
	DaAtomList subs;
	parent->ListNormalSubatoms(subs);
	if(subs.empty())
		return;

	if(IsInCapsule())
	{
		// Awaken crew from their stations if the ship is not moving too fast.
		bool awakenCrew = false;
		bool awakenEngineer = false;
		bool awakenPilot = false;
		const DaVessel* ship = dynamic_cast<const DaVessel*>(parent);
		if(ship)
		{
			double speed = ship->Speed();
			awakenCrew = true;
			awakenEngineer = !ship->IsAnyFTLDriveHot();
			awakenPilot = speed < 2;
		}

		// Awaken all empire troops aboard the ship who are not operating turrets.
		for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
		{
			DaSentient* sent = dynamic_cast<DaSentient*>(*ai);
			if(!sent)
				continue;
			if(this == sent)
				continue;

			// People only awaken if they are AI controlled.
			DaPerson* demi = dynamic_cast<DaPerson*>(sent);
			if(demi)
			{
				if(!demi->IsAIControlled())
					continue;
			}
			else
			{
				DaCitizen* citiz = dynamic_cast<DaCitizen*>(*ai);
				if(!citiz)
					continue;
				switch(citiz->NpcRole())
				{
				case ROLE_Citizen:
				case ROLE_IndigenousPerson:
				case ROLE_IndigenousElder:
				case ROLE_IndigenousHealer:
				case ROLE_IndigenousStargazer:
				case ROLE_Worker:
				case ROLE_Passenger:
				case ROLE_Officer:
				case ROLE_Count:
					continue;
				case ROLE_Crew:
					// Crew only awakens in certain circumstances.
					if(citiz->IsHelmStationPose())
					{
						if(!awakenPilot)
							continue;
					}
					else if(citiz->IsEngineStationPose())
					{
						if(!awakenEngineer)
							continue;
					}
					else
					{
						if(!awakenCrew)
							continue;
					}
					break;
				case ROLE_Troop:
					// Troops only awaken if they are in the same empire and not manning a turret.
					if(citiz->EmpireId() != EmpireId())
						continue;
					if(citiz->IsTurretPose())
						continue;
					break;
				}
			}

			// We get here if the sentient is to become alert.
			sent->BecomeAlert();
		}
	}
	else
	{
		// Awaken all empire troops within 150 meters who are not operating a turret or console.
		for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
		{
			if(this == *ai)
				continue;
			DaCitizen* troop = dynamic_cast<DaCitizen*>(*ai);
			if(!troop || !troop->IsNpcTroop())
				continue;
			if(troop->EmpireId() != EmpireId())
				continue;
			if(troop->IsTurretPose() || troop->IsConsolePose())
				continue;
			if(troop->EnvLocation().Distance2(EnvLocation()) > 150*FeetPerMeter()*150*FeetPerMeter())
				continue;
			troop->BecomeAlert();
		}
	}
}

// Check to see if I need to run to the sick bay for surgery.
// Insures the ship I am aboard has a functioning surgery unit and medical kits in the hold.
// Returns true if I decide to go get medical attention.
bool DaSentient::GetACheckup()
{
	if(IsPatientStationPose())
		return false;
	if(IsBodyDyingOrDead())
		return false;

	// Berserk dudes do not seek medical attention.
	if(Posture() == POSTURE_Berserk)
		return false;											// Never stop to get healed when berserk.

	// I only seek hit points. Other conditions that cause health problems might get healed while I am there if I'm lucky.
	if(CurrentHitPoints() >= MaxHitPoints())
		return false;											// I am not hurt at all.

	// Don't really do this work every time.
	// It gets more likely that I'll decide to do it as I get more injured.
	if(g_Au.Dice().Roll(1, MaxHitPoints()) <= CurrentHitPoints())
		return false;

	// I currently only know how to get medical attention if I am aboard a spacecraft.
	if(!IsInSpacecraft())
		return false;											// I am not in a spacecraft at any level.
	const DaVessel* ship = dynamic_cast<const DaVessel*>(Parent());
	if(!ship || !ship->IsDesignDataCacheValid())
		return false;											// Parent atom is not a spaceship.

	// Dudes already planning to get surgery do not need to keep adding more orders to do so.
	if(HasOrder(ORDER_EnterSurgery))
		return false;											// Don't keep adding these on.

	bool isBandaged = HasCondition(AuConBandage::ConditionType());
	if(isBandaged)
		return false;											// I already have a bandage for my wound.

	// Insure the ship has a medical kit in the hold.
	if(!ship->CargoHas(CID_MedicalKit))
		return false;											// The ship does not have any medical kits in the hold.

	// Insure the ship has a working surgery unit.
	if(!ship->HasWorkingSurgeryUnit())
		return false;											// The ship does not have a functioning surgery unit.

	// Certain roles aboard a spacecraft must not leave their station under certain conditions.
	if(IsHelmStationPose())
	{
		// I am a pilot. I must not leave my station if I am the pilot in control of a ship that is in motion.
		if(!ship->IsAtRest())
		{
			// The ship is in motion.
			if(Is(ship->AIPilot()))
				return false;									// I am the pilot in control of the ship.
		}
	}
	else if(IsMedicStationPose())
	{
		// I am a medic. I must not leave my station if I have a patient.
		if(ship->FindPatient(*this))
			return false;										// I have a patient.
	}

	// It's ok for me to decide to get medical attention.
	// Don't let the decision to get surgery become my primary order or I will stay in the unit forever.
	if(OrderCount() < 2)
		AmendOrder(ORDER_GoToTargetExactly, ship->Id(), Location(), 0);	// Return to this spot when done with surgery.
	AmendOrder(ORDER_EnterSurgery, ship->Id(), AuPoint(0,0), 0);
	//qDebug() << "DaSentient::GetACheckup" << Id() << Name() << "aboard" << ship->Id() << ship->Name()
		//<< "decided to see a doctor.";
	return true;
}

// returns true if this citizen is going to man a station, such as a control console or turret or surgery unit.
bool DaSentient::IsGoingToConsole() const
{
	// I am going to a console if I am going to an exact location and my next order is battle station or enter surgery.
	if(Order() != ORDER_GoToTargetExactly || ParentId() != OrderTarget())
		return false;											// I always move to consoles using a goto target exactly order.

	// I am moving exactly to a target location. Then what do I intend to do?
	CITIZEN_ORDER nextOrder = NextOrder();
	if(nextOrder == ORDER_BattleStation || nextOrder == ORDER_EnterSurgery)
		return true;											// I am going to battle station or surgery.
	return false;
}

void DaSentient::ReleaseBerth()
{
	if(!AssignedShip().IsNull())
	{
		// I am assigned to a ship.
		// Locate my ship in the solar system. It's ok if I don't find it.
		DaVessel* ship;
		DaAtom* system = FindEnvironmentAtom();
		if(system)
			ship = (DaVessel*)system->FindIdRecursive(AssignedShip());
		else
			ship = 0;

		// Release my berth.
		ReleaseBerth(ship);
	}
}

void DaSentient::ReleaseBerth(DaVessel* ship)
{
	// Release my berth on the ship.
	if(ship)
		ship->RemoveBerthOccupant(Id());

	// Clear assignment to any ship.
	ClearShipAssignment();
}

bool DaSentient::IsOkToLeaveSolarSystem(const DaVessel* ship) const
{
	if(InDesignPreview())
		return false;

	// Ship may leave the solar system if it has an AI officer aboard.
	if(ship->HasFirstOfficer())
		return true;											// Pirates get destroyed when they leave the solar system.

	// Ship may leave solar system if there is a person on board who is online.
	if(ship->FindPersonOnLineRecursive())
		return true;											// Person aboard who is online is ok to leave the system.

	// Some story ships flee the scene. So far, they are always pirates.
	if(ship->IsPirate() && ship->FindNpcRecursive())
		return true;											// Pirates get destroyed when they leave the solar system.

	// A ship that is human-commanded (vs AI-commanded) must not leave the solar system with no players aboard who are online.
	return false;												// Must not leave the solar system.
}

QString DaSentient::OrderTargetName() const
{
	if(OrderTarget().IsNull())
		return "";						// No order target.

	const DaAtom* system = FindEnvironmentAtom();
	if(system)
	{
		const DaAtom* following = system->FindIdRecursive(OrderTarget());
		if(following)
			return following->Name();	// Return Name of target atom.
	}

	return tr("Target");				// Target atom was not found.
}

QString DaSentient::OrderTargetText() const
{
	QString text;
	m_ThinkLock.lockForRead();
	if(!m_AIOrders.isEmpty())
	{
		AuAIOrderArray::const_iterator oi = m_AIOrders.cend()-1;
		if((*oi).m_Type == ORDER_GoToRoom)
			text = (*oi).m_Text;		// Room name.
	}
	m_ThinkLock.unlock();
	return text;
}

void DaSentient::BodyHasDied(COMMODITYID /*weaponCID*/, quint8 /*weaponQ*/, WEAPONEFFECT /*effect1*/, WEAPONEFFECT /*effect2*/,
		const QString& /*killerName*/)
{
	//qDebug() << "DaSentient::BodyHasDied" << Id() << Name();
	if(IsInAnyScene() && !InDesignPreview())
	{
		// Drop a random item from the citizen's inventory.
		AuGear* gear = LockGearForWrite();
		if(gear)
			DropRandomItem(*gear, false);
		UnlockGear();

		AuId killerId = KillerId();
		QString killerName;
		if(!killerId.IsNull())
		{
			DaAtom* env = FindEnvironmentAtom();
			if(env)
			{
				DaAtom* killer = env->FindIdRecursive(killerId);
				if(killer)
					killerName = killer->Name();
			}
		}
		QString inscription;
		if(!killerName.isEmpty())
			inscription = QString("%1 %2 killed by %3 %4")
				.arg(Name()).arg(Id().StringId())
				.arg(killerName).arg(killerId.StringId());
		else
			inscription = QString("%1 %2")
				.arg(Name()).arg(Id().StringId());
		AuDNABase* dna = DNAnew();
		if(dna)
		{
			AuItemDNA head(dna, Chromatid(), true, true, Handedness(), GrowthStage(), Haircut(), Shave(), CID_Head, BodyQ(),
					EmpireId(), 1);
			head.Inscribe(inscription);
			DropItem(head, CameraOffset());

			// Don't let too many identical meat piles appear in the same spot.
			DaAtom* parent = Parent();
			if(parent && parent->OkToDropMeat(Location(), CID_Animal, dna))
			{
				// Create an item to encapsulate the CID_Animal carcass left behind.
				AuItemDNA carcass(dna, Chromatid(), true, true, Handedness(), GrowthStage(), Haircut(), Shave(), CID_Animal,
						BodyQ(), EmpireId(), 1);
				DropItem(carcass);
			}
			delete dna;
		}
	}

	// Remove me from my ship's roster. It's not a big deal if I can't be removed.
	ReleaseBerth();

	Kill();		// Remove the atom from the universe.
}

// Returns the amount actually loaded.
int DaSentient::BuyCargo(DaVessel* ship, COMMODITYID cargoCID, int cargoCount, DaAtomList& tradeCities)
{
	//if(Is(1066326070))
		//qDebug() << "DaSentient::BuyCargo" << Id() << Name() << "cid" << cargoCID << "count" << cargoCount;
	int cargoLoaded = 0;
	for(DaAtomList::const_iterator ci = tradeCities.begin(); ci != tradeCities.end(); ci++)
	{
		DaAtom* tradeCity = *ci;
		if(!tradeCity)
			continue;

		// Grab a copy of the city's inventory.
		AuItemHash cinv;
		tradeCity->CommunityInventoryAvailable(cinv, EmpireId());

		AuItemHash::const_iterator bestI = cinv.FindBest(cargoCID);
		if(bestI == cinv.end())
			continue;		// The city does not have the commodity.

		int price = -1;
		if(!ship->IsPrivatelyOwned() && (*bestI).second->IsFreeToFriendlyGovShip())
		{
			// Subsidized ship gets some things free from Friend and Empire cities.
			POLITICAL_STANCE stanceToShip = tradeCity->PoliticalStanceMakeContact(tradeCity->EmpireId(), ship, ship->EmpireId());
			if(stanceToShip == POL_Friend || stanceToShip == POL_Empire || stanceToShip == POL_Vassal)
				price = 0;
		}
		if(price < 0)
		{
			// Didn't get it for free so calculate the city's price.
			// A price margin of zero means we're willing to pay any price.
			price = tradeCity->SellCost(ship->Id(), *(*bestI).second);
			if(ship->MissionOrderPriceMarginType() == 0)
			{
				// Price margin is a percentage.
				// Skip if the price is higher than the max acceptable cost of the commodity.
				int maxPrice = (int)(g_Au.CommodityQualityCost(cargoCID, (*bestI).second->Quality()) *
						ship->MissionOrderPriceMarginValue() / 100);
				if(price > maxPrice)
					continue;	// The city's price is too high.
			}
			else if(ship->MissionOrderPriceMarginType() == 1)
			{
				// Price margin is a cronodollar value.
				// Skip if the price is higher than the max acceptable cost of the commodity.
				int maxPrice = ship->MissionOrderPriceMarginValue();
				if(price > maxPrice)
					continue;	// The city's price is too high.
			}
		}

		// Price is acceptable, might be zero.
		// How many can we afford.
		int cargoToBuy = cargoCount;
		if(cargoToBuy > (int)(*bestI).second->Count())
			cargoToBuy = (int)(*bestI).second->Count();					// Limit to quantity in city's inventory.
		if(price > 0 && price * cargoToBuy > ship->AccountBalance())
			cargoToBuy = (int)ship->AccountBalance() / price;	// Limit to what the ship's account can afford.
		if(!cargoToBuy)
		{
			//qDebug("%s BuyCargo can't afford city %s price %dc, my account balance %gc", (const char*)Name().toAscii(),
					//(const char*)tradeCity->Name().toAscii(), price, ship->AccountBalance());
			continue;		// Can't afford any.
		}

		if(price)
		{
			// Exchange the money.
			double totalPrice = cargoToBuy * price;
			if(totalPrice)
			{
				ship->DebitAccount(totalPrice);			// Ship's account pays the full price.
				tradeCity->CommunityCreditAccount(totalPrice, AuBank::Civ_CreditSales);	// Collects sales tax off the 'price' appropriately.
			}
		}

		// Transfer the cargo.
		AuItem* cargoItem = (*bestI).second->Clone();

		cargoItem->SetCount(cargoToBuy);
		if(ship->CanAddCargo(*cargoItem))
		{
			ship->AddCargo(*cargoItem);						// Hydrogen is put into fuel cells.
			tradeCity->CommunityRemoveInventory(*cargoItem);
		}
		else
		{
			// This is an error condition. It should not happen.
			int fuelNeeded = ship->FuelWanted();
			int powerNeeded = (int)ceil(ship->CapacitorMax() - ship->CapacitorLevel());
			const AuItemHash& sinv = ship->LockCargoForRead();
			int cargoRoom = ship->CargoCapacityVolume() - sinv.TotalUnits();
			ship->UnlockCargo();
			qWarning() << "DaSentient::BuyCargo" << Id() << Name() << "CanAddCargo failed" << cargoItem->ListDesc()
				<< "fuelNeeded" << fuelNeeded << "powerNeeded" << powerNeeded << "cargoRoom" << cargoRoom;
		}
		delete cargoItem;
		cargoItem = 0;

		// Accumulate totals.
		cargoLoaded += cargoToBuy;
		cargoCount -= cargoToBuy;
		if(cargoCount <= 0)
			break;											// We're done loading so stop looping on cities.
	}
	//qDebug("%s BuyCargo bought %d units total", (const char*)Name().toAscii(), cargoLoaded);
	return cargoLoaded;
}

void DaSentient::SellCargo(DaVessel* ship, COMMODITYID cargoCID, int cargoCount, DaAtomList& tradeCities)
{
	//qDebug("%s SellCargo cid %d %s cargoCount %d cities %d", (const char*)Name().toAscii(), cargoCID,
			//(const char*)g_Au.CommodityName(cargoCID).toAscii(), cargoCount, tradeCities.count());
	if(cargoCID == CID_Hydrogen)
	{
		// Subsidized ships cannot sell things that they get for free.
		if(!ship->IsPrivatelyOwned())
		{
			//qDebug("%s SellCargo hydrogen cannot be sold by subsidized ship", (const char*)Name().toAscii());
			return;
		}

		AuItem* offer = AuItem::Create(cargoCID, ship->FuelQuality(), ship->EmpireId());
		if(!offer)
			return;

		// Check each city to see if they want the cargo.
		for(DaAtomList::const_iterator ci = tradeCities.begin(); ci != tradeCities.end(); ci++)
		{
			DaAtom* tradeCity = *ci;
			// Determine the price the city is willing to pay.
			// A price margin of zero means we're willing to accept any offer.
			int price = tradeCity->BuyCost(ship->Id(), *offer);
			if(ship->MissionOrderPriceMarginType() == 0)
			{
				// Price margin is a percentage.
				// Skip if the price is lower than the min acceptable offer for the commodity.
				int minPrice = (int)(g_Au.CommodityQualityCost(cargoCID, offer->Quality()) *
						ship->MissionOrderPriceMarginValue() / 100);
				if(price < minPrice)
					continue;	// The city's price is too low.
			}
			else if(ship->MissionOrderPriceMarginType() == 1)
			{
				// Price margin is a cronodollar value.
				// Skip if the price is lower than the min acceptable offer for the commodity.
				int minPrice = ship->MissionOrderPriceMarginValue();
				if(price < minPrice)
					continue;	// The city's price is too low.
			}

			// Limit the purchase quantity to what the city can afford.
			// Mission order PriceMarginType of 2 means accept ANY price.
			double totalPrice = cargoCount * (double)price;
			if(ship->MissionOrderPriceMarginType() != 2 && totalPrice > tradeCity->CommunityAccountBalance())
			{
				// Citizen account can't afford all of them. Calculate the max they can afford.
				int countToBuy = (int)tradeCity->CommunityAccountBalance() / price;
				if(countToBuy <= 0)
					continue;
				offer->SetCount(countToBuy);
			}
			else
				offer->SetCount(cargoCount);

			// How many of those items does the city want?
			quint32 canFit = tradeCity->CommunityTestFitCommodity(*offer);
			if(!canFit)
				continue;	// The city has no room for those.

			// The city wants to buy some of those items and the price is acceptable.
			// Modify the offer to fit into the city's inventory.
			offer->SetCount(canFit);

			// Calculate the final price.
			totalPrice = offer->Count() * (double)price;
			if(totalPrice > tradeCity->CommunityAccountBalance())
				totalPrice = tradeCity->CommunityAccountBalance();	// At this point, take all the money they have.

			// Transfer the money.
			ship->CreditAccountSales(totalPrice);			// Ship's account receives the full price.
			tradeCity->CommunityDebitAccount(totalPrice, AuBank::Civ_DebitPurchases);

			// Transfer the cargo.
			ship->RemoveCargo(*offer, offer->Count());						// Hydrogen is taken from fuel cells.
			tradeCity->CommunityAddCommodity(*offer);

			// Accumulate totals.
			cargoCount -= offer->Count();
			if(cargoCount <= 0)
				break;											// We're done unloading so stop looping.
		}

		delete offer;
		offer = 0;
	}
	else if(cargoCID == CID_Electricity)
	{
		AuItem* offer = AuItem::Create(cargoCID, 255, 0);
		if(!offer)
			return;

		// Check each city to see if they want the cargo.
		for(DaAtomList::const_iterator ci = tradeCities.begin(); ci != tradeCities.end(); ci++)
		{
			DaAtom* tradeCity = *ci;
			// Determine the price the city is willing to pay.
			int price = tradeCity->BuyCost(ship->Id(), *offer);
			if(ship->MissionOrderPriceMarginType() == 0)
			{
				// Price margin is a percentage.
				// Skip if the price is lower than the min acceptable offer for the commodity.
				int minPrice = (int)(g_Au.CommodityQualityCost(cargoCID, offer->Quality()) *
						ship->MissionOrderPriceMarginValue() / 100);
				if(price < minPrice)
					continue;	// The city's price is too low.
			}
			else if(ship->MissionOrderPriceMarginType() == 1)
			{
				// Price margin is a cronodollar value.
				// Skip if the price is lower than the min acceptable offer for the commodity.
				int minPrice = ship->MissionOrderPriceMarginValue();
				if(price < minPrice)
					continue;	// The city's price is too low.
			}

			// Limit the purchase quantity to what the city can afford.
			// Mission order PriceMarginType of 2 means accept ANY price.
			double totalPrice = cargoCount * (double)price;
			if(ship->MissionOrderPriceMarginType() != 2 && totalPrice > tradeCity->CommunityAccountBalance())
			{
				// Citizen account can't afford all of them. Calculate the max they can afford.
				int countToBuy = (int)tradeCity->CommunityAccountBalance() / price;
				if(countToBuy <= 0)
					continue;
				offer->SetCount(countToBuy);
			}
			else
				offer->SetCount(cargoCount);

			// How many of those items does the city want?
			quint32 canFit = tradeCity->CommunityTestFitCommodity(*offer);
			if(!canFit)
				continue;	// The city has no room for those.

			// The city wants to buy some of those items and the price is acceptable.
			// Modify the offer to fit into the city's inventory.
			offer->SetCount(canFit);

			// Calculate the final price.
			totalPrice = offer->Count() * (double)price;
			if(totalPrice > tradeCity->CommunityAccountBalance())
				totalPrice = tradeCity->CommunityAccountBalance();	// At this point, take all the money they have.

			// Transfer the money.
			ship->CreditAccountSales(totalPrice);			// Ship's account receives the full price.
			tradeCity->CommunityDebitAccount(totalPrice, AuBank::Civ_DebitPurchases);

			// Transfer the cargo.
			ship->RemoveCargo(*offer, offer->Count());						// Hydrogen is taken from fuel cells.
			tradeCity->CommunityAddCommodity(*offer);

			// Accumulate totals.
			cargoCount -= offer->Count();
			if(cargoCount <= 0)
				break;											// We're done unloading so stop looping.
		}

		delete offer;
		offer = 0;
	}
	else
	{
		const AuItemHash& sinv = ship->LockCargoForRead();
		for(AuItemHash::const_iterator it = sinv.find(cargoCID); it != sinv.cend() && (*it).first == cargoCID; it++)
		{
			// Prevent any ships from selling "freeStuff" that is issued to govt ships of my empire.
			if((*it).second->IsFreeToFriendlyGovShip())
			{
				//qDebug("%s SellCargo cid %d cannot be sold by subsidized ship", (const char*)Name().toAscii(), cargoCID);
				ship->UnlockCargo();
				return;
			}

			// Check each city to see if they want the cargo.
			for(DaAtomList::const_iterator ci = tradeCities.begin(); ci != tradeCities.end(); ci++)
			{
				DaAtom* tradeCity = *ci;
				// Determine the price the city is willing to pay.
				int price = tradeCity->BuyCost(ship->Id(), *(*it).second);
				if(ship->MissionOrderPriceMarginType() == 0)
				{
					// Price margin is a percentage.
					// Skip if the price is lower than the nominal cost of the commodity.
					int minPrice = (int)(g_Au.CommodityQualityCost(cargoCID, (*it).second->Quality()) *
							ship->MissionOrderPriceMarginValue() / 100);
					if(price < minPrice)
						continue;	// The city's price is too low.
				}
				else if(ship->MissionOrderPriceMarginType() == 1)
				{
					// Price margin is a cronodollar value.
					// Skip if the price is lower than the nominal cost of the commodity.
					int minPrice = ship->MissionOrderPriceMarginValue();
					if(price < minPrice)
						continue;	// The city's price is too low.
				}

				// How many of those items does the city want?
				AuItem* offer = (*it).second->Clone();

				// Limit the purchase quantity to what the city can afford.
				// Mission order PriceMarginType of 2 means accept ANY price.
				double totalPrice = cargoCount * (double)price;
				if(ship->MissionOrderPriceMarginType() != 2 && totalPrice > tradeCity->CommunityAccountBalance())
				{
					// Citizen account can't afford all of them. Calculate the max they can afford.
					int countToBuy = (int)tradeCity->CommunityAccountBalance() / price;
					if(countToBuy <= 0)
					{
						delete offer;
						offer = 0;
						continue;
					}
					offer->SetCount(countToBuy);
				}
				else
					offer->SetCount(cargoCount);

				// How many of those items does the city want?
				quint32 canFit = tradeCity->CommunityTestFitCommodity(*offer);
				if(!canFit)
				{
					delete offer;
					offer = 0;
					continue;	// The city has no room for those.
				}

				// The city wants to buy some of those items and the price is acceptable.
				// Modify the offer to fit into the city's inventory.
				offer->SetCount(canFit);

				// Calculate the final price.
				totalPrice = offer->Count() * (double)price;
				if(totalPrice > tradeCity->CommunityAccountBalance())
					totalPrice = tradeCity->CommunityAccountBalance();	// At this point, take all the money they have.

				// Transfer the money.
				ship->CreditAccountSales(totalPrice);		// Ship's account receives the full price.
				tradeCity->CommunityDebitAccount(totalPrice, AuBank::Civ_DebitPurchases);

				// Transfer the cargo.
				ship->RemoveCargoNoLock(*offer, offer->Count());					// Hydrogen is taken from fuel cells.
				tradeCity->CommunityAddCommodity(*offer);

				// Accumulate totals.
				cargoCount -= offer->Count();
				delete offer;
				offer = 0;
				if(cargoCount <= 0)
				{
					ship->UnlockCargo();
					return;										// We're done unloading so stop looping.
				}

				// Update ship cargo iterator because the removed cargo may have invalidated the current item.
				it = sinv.find(cargoCID);
				if(it == sinv.end())
				{
					ship->UnlockCargo();
					return;										// No more cargoCID in the ship's hold.
				}
			}
		}
		ship->UnlockCargo();
	}
}

void DaSentient::CommandShip(DaVessel* ship)
{
	// Impose a time delay based on rank for getting started with each nu order.
	if(!IsReadyToStartMissionOrder(ship))
		return;

	AuMissionOrder::TYPE destOrder = ship->MissionOrderType();
	if(IsPirate() && ship->IsPirateWithFreeWill() && !IsInAnyScene() && destOrder != AuMissionOrder::Scuttle)
	{
		// Pirate commander scuttles the ship for cleanup when not in any scene, when certain conditions are true.
		if(ship->AlertConditionOfStructure() == ConditionRed || ship->AlertConditionOfSystems() == ConditionRed)
		{
			if(InArena())
			{
				AuCommHailRequest msg(EmpireId(), tr("You win. My ship %1 is beaten; I am scuttling her.")
						.arg(ship->NameWithEmpire()),
						AuCommStdChannel::HailChannelName(),
						ship->Id(), ship->Name(), ship->FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Sad);
			}
			else
				qDebug() << "DaSentient::CommandShip" << Id() << Name() << "scuttling" << ship->Id() << ship->Name()
					<< "Ship is severely damaged.";
			DecideShipScuttle();
			return;
		}
		else if(ship->IsInAtmosphere() && ship->CreatedTime().ElapsedSec() > 180)
		{
			// In an atmosphere and ship is more than 3 minutes old.
			// We sometimes spawn in an atmosphere. This allows enough time to take off. After that, atmosphere is forbidden.
			qDebug() << "DaSentient::CommandShip" << Id() << Name() << "scuttling" << ship->Id() << ship->Name()
				<< "Ship in atmosphere, grounded?" << ship->IsOnGround();
			DecideShipScuttle();
			return;
		}
		else if(ship->FuelQuantity() < 1)
		{
			// Ship needs fuel.
			if(InArena())
			{
				AuCommHailRequest msg(EmpireId(), tr("My ship %1 has no fuel; I am scuttling her.")
						.arg(ship->NameWithEmpire()),
						AuCommStdChannel::HailChannelName(),
						ship->Id(), ship->Name(), ship->FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Sad);
			}
			else
				qDebug() << "DaSentient::CommandShip" << Id() << Name() << "scuttling" << ship->Id() << ship->Name()
					<< "Ship is out of fuel.";
			DecideShipScuttle();
			return;
		}
		else if(!InArena())
		{
			if(!ship->IsSpaceStation() && ship->ManeuverAcceleration() < 100*FeetPerMeter())
			{
				// Ship needs at least 100m/s/s acceleration.
				qDebug() << "DaSentient::CommandShip" << Id() << Name() << "scuttling" << ship->Id() << ship->Name()
					<< "Ship acceleration only" << ship->ManeuverAcceleration()/FeetPerMeter() << "m/s/s";
				DecideShipScuttle();
				return;
			}
			else if(!ship->HasWeaponBay() && (!ship->Turrets() || ship->TroopBerths() == 0))
			{
				// Ship needs a weapon bay or a turret with a troop to operate it.
				qDebug() << "DaSentient::CommandShip" << Id() << Name() << "scuttling" << ship->Id() << ship->Name()
					<< "Ship is unarmed.";
				DecideShipScuttle();
				return;
			}
		}
	}

	// Decide whether to attack enemy spacecraft that show up on my sensors.
	// We will not end up doing this if the current order is complete this time through, i.e. if orderComplete gets set to true.
	bool huntEnemy = false;										// true if it's ok to go after enemy spacecraft detected on sensors.
	bool huntAsteroids = false;
	if((ship->IsMissionActive() || ship->MissionIsEmpty()) && !InArenaCannotAccelerate())
	{
		// Ship's mission is active or it has no mission orders. It's ok to go hunting if the ship is in good shape.
		// When the ship has a non-empty mission that is deactivated, we don't go hunting because we'll end up activating the
		// mission. That is probably not wanted by the player who owns the ship.
		// Stations always hunt for enemy to concentrate fire on a target; they can't actually pursue.
		// The InArenaCannotAccelerate check insure we don't go hunting until the match has started.
		if(ship->IsSpaceStation())
		{
			huntEnemy = true;
			huntAsteroids = true;
		}
		else if(Posture() == POSTURE_Berserk)
		{
			// Do not go hunting when on red alert. Ignore fire and acid.
			if(ship->AlertConditionOfStructure() != ConditionRed && ship->AlertConditionOfSystems() != ConditionRed)
			{
				huntEnemy = true;									// Berserk officers hunt until condition red.
				huntAsteroids = true;
			}
		}
		else if(Posture() == POSTURE_Offensive)
		{
			// Do not go hunting unless condition is green.
			if(ship->AlertCondition() == ConditionGreen)
			{
				huntEnemy = true;									// Non-berserk officers hunt until while condition is green.
				huntAsteroids = true;
			}
		}
		else if(Posture() == POSTURE_DefendSelf)
		{
			// Do not go hunting asteroids unless condition is not red.
			if(ship->AlertCondition() != ConditionRed)
				huntAsteroids = true;
		}

		// Noncombatant units do not pursue enemies.
		// They ARE permitted to pursue the asteroid. They are supposed to return to base afterwards and it is unlikely an enemy is
		// present when this happens.
		if(huntEnemy && IsNoncombatant())
			huntEnemy = false;
	}

	// Verbally issue the current order on the crew channel if I have not already done so.
	if(!IsOrderAnnounced())
	{
		DecideShipAnnounceOrder();
		SetOrderAnnounced();
	}

	bool orderComplete = false;
	int completionCode = 0;	// 0="Done, sir." See AnnounceMissionOrderComplete for other options.
	switch(destOrder)
	{
	case AuMissionOrder::Invalid:
	case AuMissionOrder::OrderTypes:
		orderComplete = true;
		completionCode = 99;
		break;

	case AuMissionOrder::Report:
		{
			DoOrderReport(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferCargoFrom:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferCargoFrom(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferCargoTo:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferCargoTo(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferCargoCategoryFrom:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferCargoCategoryFrom(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferCargoCategoryTo:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferCargoCategoryTo(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferMoneyFrom:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferMoneyFrom(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferMoneyTo:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferMoneyTo(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferPeopleFrom:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferPeopleFrom(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TransferPeopleTo:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DoOrderTransferPeopleTo(ship);

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::ExterminateRats:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DecideShipExterminateRats();

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::StandDown:
		// Unlock crew and hull doors during stand down order.
		huntEnemy = false;
		huntAsteroids = false;

		// Order complete when the prescribed time has passed.
		// Time of 0 results in endless order.
		if(ship->MissionOrderIsExpired(ship->MissionOrderStart()))
		{
			orderComplete = true;
			completionCode = 3;
		}
		break;

	case AuMissionOrder::Harvest:
	case AuMissionOrder::Hide:
	case AuMissionOrder::PatrolCities:
	case AuMissionOrder::PatrolStations:
	case AuMissionOrder::PatrolSystem:
	case AuMissionOrder::PatrolWormholes:
	case AuMissionOrder::Stay:									// Default Stay order has 0 time so it is never complete.
		// Order complete when the prescribed time has passed, in minutes.
		// Time of 0 results in endless order.
		if(ship->MissionOrderIsExpired(ship->MissionOrderStart()))
		{
			orderComplete = true;
			completionCode = 3;
		}
		break;

	case AuMissionOrder::HarvestOperationByBay:
	case AuMissionOrder::HarvestOperationByModule:
		// Order completes based on several criteria.
		huntEnemy = false;
		huntAsteroids = false;
		switch(ship->MissionIsHarvestOperationComplete())
		{
		case 0:	// All harvest operations are satisfied.
			orderComplete = true;
			completionCode = 17;
			break;
		case 1:	// Hold is full.
			orderComplete = true;
			completionCode = 18;
			break;
		case 2:	// Current order is not a harvest operation.
			orderComplete = true;
			completionCode = 0;
			break;
		case 3:	// Fuel is at or below the threshold.
			orderComplete = true;
			completionCode = 19;
			break;
		case 4:	// Unsatisfied harvester operation exists.
			break;
		}
		break;

	case AuMissionOrder::ShieldsDown:
		// Order complete when the prescribed time has passed, in seconds.
		// Time of 0 results in endless order.
		if(ship->MissionOrderIsExpired(ship->MissionOrderStart()))
		{
			orderComplete = true;
			completionCode = 3;
		}
		break;

	case AuMissionOrder::Christen:
		// Order complete as soon as I christen the ship.
		if(ship->Name() != ship->MissionOrderShipName())
		{
			ship->SetName(ship->MissionOrderShipName());
			ship->SetModified();
		}
		orderComplete = true;
		break;

	case AuMissionOrder::HoldMission:
		// Order complete as soon as I deactivate the mission.
		DecideShipMissionState(false);
		orderComplete = true;
		completionCode = 13;
		break;

	case AuMissionOrder::LightState:
		{
			// By the time I get to here, the pilot may have already switched the lights.
			bool headlight = ship->MissionOrderHeadlightOn();
			bool navLight = ship->MissionOrderNavLightOn();
			bool beacon = ship->MissionOrderRotatingBeaconOn();
			bool strobe = ship->MissionOrderStrobeOn();
			DoShipLightState(ship, headlight, navLight, beacon, strobe);
			orderComplete = true;
		}
		break;

	case AuMissionOrder::ShieldPolicy:
		// This simply sets the policy for operation of the shields. The shield operator makes the decisions.
		DecideShipShieldsPolicy(ship->MissionOrderShipShieldFuelPolicy(), ship->MissionOrderShipShieldPolicy());
		orderComplete = true;
		break;

	case AuMissionOrder::PilotPolicy:
		// This simply sets the policy. The pilot makes the decisions.
		DecideShipPilotPolicy(ship->MissionOrderShipPilotUpPolicy());
		orderComplete = true;
		break;

	case AuMissionOrder::Posture:
		// Order complete as soon as I set the posture of me and the crew.
		// All officers (me), crew and troops aboard.
		DecideShipCrewPosture(ship->MissionOrderCitizenPosture());
		orderComplete = true;
		break;

	case AuMissionOrder::GoToLand:
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			// Land on the ground on the destination planet.
			AuPoint destOffset;
			DaWorld* destPlanet = (DaWorld*)(ship->MissionDestination(destOrder, destOffset));
			if(destPlanet)
			{
				// Destination planet was found in this solar system.
				if(ship->IsOnGround() && !ship->IsInVisibleWater() && destPlanet == ship->FindParentPlanetInEnv())
					orderComplete = true;	// Ship is on the ground of the destination planet. That's good enough.
			}
			else
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 4;
			}
		}
		break;

	case AuMissionOrder::GoToCityAnywhere:
		// Order complete when the ship is at rest near the destination city.
		if(!ship->IsMoving() && !ship->HasWaypoint() && ship->IsOnGround())
		{
			// Ship is at rest and no nav route is set.
			AuPoint destOffset;
			const DaAtom* destCity = ship->MissionDestination(destOrder, destOffset);
			if(destCity)
			{
				// Destination city was found in the solar system.
				if(destCity->Is(ship->InBdCity()))
					orderComplete = true;	// Ship is sitting on a road slab in the destination city.
				else
				{
					const DaWorld* planet = ship->FindParentPlanetInEnv();
					if(planet == destCity->FindParentPlanetInEnv())
					{
						AuPoint shipLoc;
						ship->GetLocationRelativeToParent(shipLoc, planet);
						AuPoint cityLoc = destCity->Location();
						if(shipLoc.Distance(cityLoc) < 5000)
							orderComplete = true;	// Ship is sitting on the ground near the destination city.
					}
				}
			}
			else
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 5;
			}
		}
		break;
	case AuMissionOrder::GoToCity:		// Land at airport order.
	case AuMissionOrder::GoToBestCity:
	case AuMissionOrder::GoToClosestCity:
		// Order complete when the ship is at rest on a road slab in the destination city.
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			AuPoint destOffset;
			const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
			if(destAtom)
			{
				// Our destination is an airport.
				if(ship->IsOnGround())
				{
					const DaBd* bd = dynamic_cast<const DaBd*>(destAtom);
					if(bd)
					{
						if(bd->CapitolId() == ship->InBdCity())
							orderComplete = true;	// Ship is in the destination city.
					}
				}
			}
			else
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 5;
			}
		}
		break;
	case AuMissionOrder::GoToStation:
	case AuMissionOrder::GoToBestStation:
	case AuMissionOrder::GoToClosestStation:
		// Order complete when the ship is within trade range of the destination station.
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			AuPoint destOffset;
			const DaVessel* destStation = dynamic_cast<const DaVessel*>(ship->MissionDestination(destOrder, destOffset));
			if(destStation)
			{
				// Destination station was found in this system.
				if(!ship->IsInAtmosphere())
				{
					// Ship is in space.
					// Check to see if we're close enough to the destination station.
					AuPoint shipLoc = ship->EnvLocation();
					AuPoint statLoc = destStation->EnvLocation();
					if(statLoc.Distance(shipLoc) < destStation->StationTradeRange(ship))
						orderComplete = true;
				}
			}
			else
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 6;
			}
		}
		break;
	case AuMissionOrder::GoToBestStationOrCity:
	case AuMissionOrder::GoToClosestStationOrCity:
		// Order complete when the ship is at rest on a road slab in the destination city
		//  or
		// the ship is within trade range of the destination station.
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			AuPoint destOffset;
			const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
			if(destAtom)
			{
				// Destination atom was found. We expect it to be a city or a station.
				const DaVessel* destStation = dynamic_cast<const DaVessel*>(destAtom);
				if(destStation)
				{
					// Our destination is a station.
					// Check to see if we're close enough to the destination station.
					AuPoint shipLoc = ship->EnvLocation();
					AuPoint statLoc = destStation->EnvLocation();
					if(statLoc.Distance(shipLoc) < destStation->StationTradeRange(ship))
						orderComplete = true;
				}
				else
				{
					// Our destination is an airport.
					if(ship->IsOnGround())
					{
						const DaBd* bd = dynamic_cast<const DaBd*>(destAtom);
						if(bd && bd->CapitolId() == ship->InBdCity())
							orderComplete = true;	// Ship is in the destination city.
					}
				}
			}
			else
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 7;
			}
		}
		break;
	case AuMissionOrder::GoToHoverSite:
		// Order complete when the ship is near the hover locatio nat the destination planet.
		if(!ship->HasWaypoint())
		{
			// No nav route is set.
			// Hover at a specified longitude, latitude on the destination planet.
			AuPoint destOffset;
			const DaWorld* destPlanet = dynamic_cast<const DaWorld*>(ship->MissionDestination(destOrder, destOffset));
			if(destPlanet)
			{
				// Destination planet was found in this solar system.
				if(ship->IsInAtmosphere() && destPlanet == ship->FindParentPlanetInEnv())
				{
					// Ship is in the atmosphere of the destination planet.
					// Check proximity to hover location.
					AuPoint shipLoc;
					ship->GetLocationRelativeToParent(shipLoc, destPlanet);
					if(shipLoc.Distance(destOffset) < 50*FeetPerMeter())
						orderComplete = true;	// Hovering within 10m of target. That's good enough.
				}
			}
			else
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 4;
			}
		}
		break;
	case AuMissionOrder::GoToLandingSite:
		// Order complete when the ship is at rest near the landing site at the destination planet.
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			// Land at a specified longitude, latitude on the destination planet.
			AuPoint destOffset;
			const DaWorld* destPlanet = dynamic_cast<const DaWorld*>(ship->MissionDestination(destOrder, destOffset));
			if(destPlanet)
			{
				// Destination planet was found in this solar system.
				if(ship->IsOnGround() && destPlanet == ship->FindParentPlanetInEnv())
				{
					// Ship is on the ground of the destination planet.
					// Check proximity to landing location.
					double prox = 10*FeetPerMeter() + ship->ObstacleRadius();
					AuMatrix4x4 shipT;
					ship->GetTransformRelativeToParent(shipT, destPlanet);
					AuPoint shipBaseLoc = shipT.Project(0,0,-ship->GroundHeightAboveCG());
					if(shipBaseLoc.Distance(destOffset) < prox)
						orderComplete = true;	// Landed within 10m of target. That's good enough.
				}
			}
			else
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 4;
			}
		}
		break;

	case AuMissionOrder::GoToOrbitCity:							// Destination is a planet that hosts a city.
		// Order complete when the ship is at rest in orbit at the destination planet.
		if(!ship->HasWaypoint())
		{
			// Ship has no nav route set.
			AuPoint destOffset;
			const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
			if(!destAtom)
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 4;
				break;
			}

			const DaWorld* destPlanet = dynamic_cast<const DaWorld*>(destAtom);
			if(destPlanet)
			{
				// Destination planet was found in this solar system.
				if(!ship->IsInAtmosphere() && destPlanet == ship->FindParentPlanetInEnv())
				{
					// Ship is in space at the destination planet.
					const double orbitAlt = destPlanet->AtmosphereRadius() + ParkAltAboveAtm();
					if(ship->CGAltitude() > orbitAlt - 100*FeetPerMeter() &&
							ship->CGAltitude() < orbitAlt + 100*FeetPerMeter())
					{
						// Ship is at the proper altitude.
						// Order requires orbit at specific long/lat coordinates.
						// Check proximity to orbit location above target city.
						AuPoint shipLoc;
						ship->GetLocationRelativeToParent(shipLoc, destPlanet);
						if(shipLoc.Distance(destOffset) < 100*FeetPerMeter())
							orderComplete = true;	// Orbiting within 100m of target. That's good enough.
					}
				}
			}
		}
		break;

	case AuMissionOrder::GoToOrbit:
	case AuMissionOrder::GoToOrbitBestPlanet:
	case AuMissionOrder::GoToOrbitCoords:
		// Order complete when the ship is at rest in orbit at the destination planet.
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			AuPoint destOffset;
			const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
			if(!destAtom)
			{
				orderComplete = true;	// Don't get stuck here. Move on to the next order.
				completionCode = 4;
				break;
			}

			const DaWorld* destPlanet = dynamic_cast<const DaWorld*>(destAtom);
			if(destPlanet)
			{
				// Destination planet was found in this solar system.
				const DaSolarSystem* destSystem = destPlanet->FindParentSolarSystemInEnv();
				if(!destSystem)
				{
					// wut?
					orderComplete = true;
					completionCode = 99;
				}
				else if(!ship->IsInAtmosphere() && destPlanet == ship->FindParentPlanetInEnv())
				{
					// Ship is in space at the destination planet. Watch for the special case of going to orbit a ring. In that
					// case, the mission order Id is the ring but the destPlanet is the parent planet of the ring.
					if(ship->MissionOrderDestinationId().IsNull() ||
							destPlanet == destSystem->FindDestinationWorld(ship->MissionOrderDestinationId(),
								ship->MissionOrderDestinationName()))
					{
						// The destination planet is my true destination.
						const double orbitAlt = destPlanet->AtmosphereRadius() + ParkAltAboveAtm();
						if(ship->CGAltitude() > orbitAlt - 100*FeetPerMeter() &&
								ship->CGAltitude() < orbitAlt + 100*FeetPerMeter())
						{
							// Ship is at the proper altitude.
							if(destOrder == AuMissionOrder::GoToOrbitCoords)
							{
								// Order requires orbit at specific long/lat coordinates.
								// Check proximity to orbit location.
								AuPoint shipLoc;
								ship->GetLocationRelativeToParent(shipLoc, destPlanet);
								if(shipLoc.Distance(destOffset) < 100*FeetPerMeter())
									orderComplete = true;	// Orbiting within 100m of target. That's good enough.
							}
							else
								orderComplete = true;	// Close enough for these orbit commands.
						}
					}
					else
					{
						// The destination planet is not the actual target of my mission order.
						// This indicates I am going to orbit a ring that is subordinate to my destination planet.
						const DaWorld* ringPlanet =
							dynamic_cast<const DaWorld*>(destSystem->FindDestinationWorld(ship->MissionOrderDestinationId(),
										ship->MissionOrderDestinationName()));
						if(ringPlanet)
						{
							const double orbitAlt = ringPlanet->Location().Length();
							if(ship->CGAltitude() > orbitAlt - 100*FeetPerMeter() &&
									ship->CGAltitude() < orbitAlt + 100*FeetPerMeter() &&
									fabs(ship->Location().z) < 100*FeetPerMeter())				// Close to the plane of the ring.
							{
								// Ship is at the proper altitude.
								if(destOrder == AuMissionOrder::GoToOrbitCoords)
								{
									// Order requires orbit at specific long/lat coordinates.
									// Check proximity to orbit location.
									AuPoint shipLoc;
									ship->GetLocationRelativeToParent(shipLoc, destPlanet);
									if(shipLoc.Distance(destOffset) < 100*FeetPerMeter())
										orderComplete = true;	// Orbiting within 100m of target. That's good enough.
								}
								else
									orderComplete = true;	// Close enough for these orbit commands.
							}
						}
						else
						{
							// Something is wrong if we get here.
							orderComplete = true;
							completionCode = 99;
						}
					}
				}
			}
			else
			{
				const DaStar* destStar = dynamic_cast<const DaStar*>(destAtom);
				if(destStar)
				{
					// Destination star was found in this solar system.
					if(!ship->IsInAtmosphere() && destStar == ship->FindParentStarInEnv())
					{
						// Ship is in space in the zone of the destination star, which could still be a great distance away.
						const double orbitAlt = destStar->AtmosphereRadius() + ParkAltAboveSun();
						if(ship->CGAltitude() > orbitAlt - 100*FeetPerMeter() &&
								ship->CGAltitude() < orbitAlt + 100*FeetPerMeter())
						{
							// Ship is at the proper altitude.
							if(destOrder == AuMissionOrder::GoToOrbitCoords)
							{
								// Order requires orbit at specific long/lat coordinates.
								// Check proximity to orbit location.
								AuPoint shipLoc;
								ship->GetLocationRelativeToParent(shipLoc, destStar);
								if(shipLoc.Distance(destOffset) < 100*FeetPerMeter())
									orderComplete = true;	// Orbiting within 100m of target. That's good enough.
							}
							else
								orderComplete = true;	// Close enough for these orbit commands.
						}
					}
				}
				else
				{
					orderComplete = true;	// Don't get stuck here. Move on to the next order.
					completionCode = 99;
				}
			}
		}
		break;

	case AuMissionOrder::GoToCoordinate:
		// Order complete when we are near the destination coordinate.
		{
			AuPoint destOffset;
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->MissionDestination(destOrder, destOffset));
			if(system)
			{
				// DaVessel::MissionDestination returns this solar system and destOffset in feet relative to this system.
				// Check to see if we are close enough to the destination location.
				const DaStar* star = system->FindPrimaryStar();
				if(star && destOffset.Length2() <= star->Radius()*star->Radius())
				{
					// Destination is inside the primary star.
					// Call it good when rhe navigator has cleared the course.
					if(!ship->HasWaypoint())
					{
						AuPoint shipLoc = ship->EnvLocation();
						const double distFromSun2 = shipLoc.Length2();			// Squared distance to the sun.
						double navDist = FeetPerSolarSystem()/2;				// Navigate to within 1/2 solar system diameter.
						if(distFromSun2 < navDist*navDist)
						{
							// The ship is within a solar system radius of the destination star.
							// Take some precious CPU time to be more precise so we get to where the planets are.
							navDist = system->ActualRadius() + FeetPerOrbit();
							if(distFromSun2 < navDist*navDist)
								orderComplete = true;
						}
					}
				}
				else
				{
					// Destination is not inside the primary star.
					// Ship does not have to be at rest.
					AuPoint shipLoc = ship->EnvLocation();
					double distToDest2 = shipLoc.Distance2(destOffset);			// Squared distance to the destination location.
					if(distToDest2 < 600*FeetPerMeter() * 600*FeetPerMeter())
						orderComplete = true;					// 600m is close enough to the coordinate. 100m sloppier than pilot.
				}
			}
			else
			{
				orderComplete = true;
				completionCode = 99;
			}

			if(orderComplete)
				DecideShipReconcileRoster();	// This is a good time to reconcile the roster.
		}
		break;

	case AuMissionOrder::GoToSystemViaWormholes:
		{
			// Check intergalactic wormhole.
			GALAXY destGal = ship->MissionOrderDestinationId().Galaxy();
			if(destGal != GALAXY_Count)
			{
				if(ship->Galaxy() == destGal && !ship->IsAnyFTLDriveHot())
				{
					// Clear the order and reconcile the roster.
					orderComplete = true;
					DecideShipReconcileRoster();	// This is a good time to reconcile the roster.
				}
				break;
			}
		}
		// Check regular solar system wormhole.
		[[fallthrough]];

	case AuMissionOrder::GoToSystemDirect:
	case AuMissionOrder::GoToSystemDirect_Deprecated:
		// Order complete when we are in the destination solar system.
		// Ship does not have to be at rest.
		{
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
			if(system && system->Is(ship->MissionOrderDestinationId()))
			{
				// Check to see if we are within a solar system radius of the center of the system.
				AuPoint shipSysLoc = ship->EnvLocation();
				const double distFromSun2 = shipSysLoc.Length2();				// Squared distance to the sun.
				double navDist = FeetPerSolarSystem()/2;						// Navigate to within 1/2 solar system diameter.
				if(distFromSun2 < navDist*navDist)
				{
					// The ship is within a solar system radius of the destination star.
					// Call it good if the FTL drive is not hot.
					if(!ship->IsAnyFTLDriveHot())
					{
						// Clear the order and reconcile the roster.
						orderComplete = true;
						DecideShipReconcileRoster();	// This is a good time to reconcile the roster.
					}
					else
					{
						// Take some precious CPU time to be more precise so we get to where the planets are.
						navDist = system->ActualRadius() + FeetPerOrbit();
						if(distFromSun2 < navDist*navDist)
						{
							// Clear the order and reconcile the roster.
							orderComplete = true;
							DecideShipReconcileRoster();	// This is a good time to reconcile the roster.
						}
					}
				}
			}
		}
		break;

	case AuMissionOrder::Follow:
		// Order complete when the atom is no longer in the system and we have no waypoint route set.
		// When following a target that leaves the solar system through a wormhole, the navigator sets a waypoint at that
		// wormhole, which keeps the "follow" going through the wormhole.
		/*
		if(!ship->HasWaypoint())
		{
			// No nav route is set. Insure target atom is in the solar system.
			AuPoint destOffset;
			const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
			if(!destAtom)
				orderComplete = true;	// The target is no longer in the system.
		}
		*/
		break;
	case AuMissionOrder::Attack:
		// Order complete when the atom is no longer in the system.
		huntEnemy = false;
		huntAsteroids = false;
		{
			AuPoint destOffset;
			const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
			const DaVessel* destVessel = dynamic_cast<const DaVessel*>(destAtom);
			if(!destAtom)
			{
				orderComplete = true;							// The target is no longer in the system.
				completionCode = 8;
			}
			else if(IsPirate() && ship->IsPirateWithFreeWill() && destAtom->IsInAtmosphere())
			{
				orderComplete = true;							// Don't pursue prey into an atmosphere.
				completionCode = 9;

				QString text = tr("%1, blast you! Come up from that dirt ball so we can face each other as warriors.")
					.arg(destAtom->Name());
				const DaSector* sector = ship->FindParentSector();
				AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
						sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Frown, destAtom->Id());
			}
			else if(Posture() == POSTURE_Berserk)
			{
				// I'm berserk. Break off the attack when my ship's status goes to red alert, ignoring fire and acid.
				if(ship->AlertConditionOfStructure() == ConditionRed || ship->AlertConditionOfSystems() == ConditionRed)
				{
					orderComplete = true;
					completionCode = 11;
				}
			}
			else if(IsPirate() && ship->IsPirateWithFreeWill() && destVessel && destVessel->IsCoveredByATC())
			{
				orderComplete = true;							// Don't pursue prey into an ADIZ.
				completionCode = 20;

				QString text = tr("%1, you have escaped. Luck is on your side today. Perhaps we will meet again.")
					.arg(destAtom->Name());
				const DaSector* sector = ship->FindParentSector();
				AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
						sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Frown, destAtom->Id());
			}
			else
			{
				// I'm not berserk. Break off the attack when my ship's status goes to red or yellow alert.
				if(ship->AlertCondition() != ConditionGreen)
				{
					orderComplete = true;
					if(ship->AlertCondition() == ConditionRed)
						completionCode = 11;
					else
						completionCode = 10;
				}
			}
		}
		break;
	case AuMissionOrder::Plunder:
		// Order complete when the atom gets away.
		huntEnemy = false;
		huntAsteroids = false;
		{
			// I only know how to plunder spacecraft.
			AuPoint destOffset;
			const DaVessel* destAtom = dynamic_cast<const DaVessel*>(ship->MissionDestination(destOrder, destOffset));
			if(!destAtom)
			{
				orderComplete = true;							// The target is no longer in the system or not a vessel.
				completionCode = 8;
			}
			else if(IsPirate() && ship->IsPirateWithFreeWill() && destAtom->IsInAtmosphere())
			{
				orderComplete = true;							// Don't pursue prey into an atmosphere.
				completionCode = 9;

				QString text = tr("%1, you earthworms can't stay down there forever. We'll be waiting for you.")
					.arg(destAtom->Name());
				const DaSector* sector = ship->FindParentSector();
				AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
						sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Frown, destAtom->Id());
			}
			else if(Posture() == POSTURE_Berserk)
			{
				// I'm berserk. Break off the attack when my ship's status goes to red alert, ignoring fire and acid.
				if(ship->AlertConditionOfStructure() == ConditionRed || ship->AlertConditionOfSystems() == ConditionRed)
				{
					orderComplete = true;
					completionCode = 11;
				}
			}
			else if(IsPirate() && ship->IsPirateWithFreeWill() && destAtom->IsCoveredByATC())
			{
				orderComplete = true;							// Don't pursue prey into an ADIZ.
				completionCode = 20;

				QString text = tr("%1, cowards! You won't get away next time.").arg(destAtom->Name());
				const DaSector* sector = ship->FindParentSector();
				AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
						sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Frown, destAtom->Id());
			}
			else
			{
				// I'm not berserk. Break off the attack when my ship's status goes to red or yellow alert.
				if(ship->AlertCondition() != ConditionGreen)
				{
					orderComplete = true;
					if(ship->AlertCondition() == ConditionRed)
						completionCode = 11;
					else
						completionCode = 10;
				}
			}
			if(orderComplete)
				break;

			// Bark orders at the target ship periodically.
			if(!SpokeRecently(15) && g_Au.Dice().Roll(1,15) == 1)
			{
				AuPoint shipLoc = ship->EnvLocation();
				AuPoint targetLoc = destAtom->EnvLocation();
				double distToShip2 = shipLoc.Distance2(targetLoc);

				const DaSector* sector = ship->FindParentSector();
				quint16 standDown = destAtom->ReasonNotStandingDown();	// Returns a bit mask of reasons.
				if(standDown == 0)
				{
					// Target ship is standing down.
					// Determine if our ships are close enough together to transfer cargo.
					float tradeRange = ship->TransferRange(destAtom);
					if(distToShip2 > tradeRange*tradeRange)
					{
						QString text = tr("%1, hold your position; we will be there shortly.").arg(destAtom->Name());
						//qDebug() << text;
						AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
								sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
						SayNoRepeat(msg, AuVisage::Frown, destAtom->Id());
					}
					else
					{
						// We are going to loot the target.
						m_SentientLock.lockForWrite();
						m_LastPlunder = destAtom->Id();
						m_SentientLock.unlock();
						DecideShipLootShip(destAtom->Id());

						// Clear history of damage to my ship by the target. Otherwise the crew will blast them when we finish
						// looting them and switch to DefendSelf posture.
						ship->ClearUnitDamage(destAtom->Id());

						//qDebug() << "We are done looting the target";
						orderComplete = true;
					}
				}
				else
				{
					float sensorRange2 = ship->SensorRange();
					sensorRange2 *= sensorRange2;	// squared
					if(distToShip2 < ship->SightRange2() || distToShip2 < sensorRange2)
					{
						// The target ship is in sight or sensor range.
						// Order the target ship to stand down.
						QString text = tr("%1, stand down at once or be destroyed.").arg(destAtom->Name());
						if(standDown & 0x0080)
							text += tr(" Hold your fire!");
						if(standDown & 0x0001)
							text += tr(" Order your ship to stand down.");
						if(standDown & 0x0004)
							text += tr(" Kill your throttles.");
						if(standDown & 0x0008)
							text += tr(" Stop maneuvering your vessel.");
						if(standDown & 0x0010)
							text += tr(" Shut down your FTL drive.");
						if(standDown & 0x0020)
							text += tr(" Lower your shields.");
						if(standDown & 0x0040)
							text += tr(" Unlock your sensors.");
						if(standDown & 0x0002)
							text += tr(" Unlock your doors.");

						//qDebug() << text;
						AuCommChat msg(text, AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(),
								sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
						SayNoRepeat(msg, AuVisage::Frown, destAtom->Id());
					}
				}
			}
		}
		break;
	case AuMissionOrder::Repair:
		// Crew will be preparing to moor the ship when Repair order is current.
		// We start out waiting for the crew to prepare the ship for repairs.
		// Check to see if the ship needs repairs.
		// This logic avoids mooring and system shutdown when no repairs are needed.
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsDamaged())
		{
			// Ship is not damaged.
			orderComplete = true;
			completionCode = 2;		// No repairs needed.
		}
		else if(ship->IsReadyForRepairs())
		{
			// Ship is at rest and systems are off so repairs can commence.
			if(IsPirate() && ship->IsPirateWithFreeWill())
			{
				// Ship is a pirate. I don't have any cities. :(
				RepairPirate(ship);
				orderComplete = true;
				completionCode = 0;		// Done
			}
			else if(!ship->IsPrivatelyOwned() || ship->AccountBalance() >= 1)
			{
				// Ship is not privately owned or there is money in its account.
				int result = RepairShip(ship);
				orderComplete = true;
				if(result)
					completionCode = 20 + result;
				else
					completionCode = 0;	// Done.
			}
			else
			{
				// Private ship with no money.
				orderComplete = true;
				completionCode = 16;	// No money.
			}
		}
		break;

	case AuMissionOrder::PassengersLoad:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			int passBerths = ship->PassengerBerthsAvailable();

			// Pre-boarding announcement has already been made.
			// Wait the prescribed amount of time before boarding colonists.
			if(!passBerths || ship->MissionOrderIsExpired(ship->MissionOrderStart()))
			{
				// Pre-boarding announcement period has passed or all passenger berths have been taken.
				DecideShipBoardPassengers(ship->MissionOrderHeadCount());

				// Send passenger boarding completed message.
				const DaSector* sector = ship->FindParentSector();
				if(sector)
				{
					AuCommStdChannel::RANGE range;
					if(ship->IsOnGround() && !ship->BdId().IsNull())
						range = AuCommStdChannel::Close;
					else
						range = AuCommStdChannel::SolarSystem;
					AuCommPassengerBoardingComplete msg(AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(),
							sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
					SayNoRepeat(msg, AuVisage::Closed);
				}

				// Stop monitoring the Trade channel for acceptance of the boarding announcement.
				g_Au.StopMonitoringChannel(AuCommStdChannel::TradeChannelName(), ship->Id());

				// We only get one shot at doing this.
				orderComplete = true;
			}
		}
		break;

	case AuMissionOrder::PassengersUnload:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			unsigned int passengers = ship->PassengerBerthsOccupied();
			if(passengers)
			{
				// Limit the number of passengers to unload if the mission provides a count.
				if(ship->MissionOrderHeadCount() && passengers > ship->MissionOrderHeadCount())
					passengers = ship->MissionOrderHeadCount();

				DecideShipDebarkPassengers(passengers);
			}

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::CryoBerthsUnload:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			unsigned int cryoPass = ship->CryoBerthsOccupied();
			if(cryoPass)
			{
				// Limit the number of cryo travellers to unload if the mission provides a count.
				if(ship->MissionOrderHeadCount() && cryoPass > ship->MissionOrderHeadCount())
					cryoPass = ship->MissionOrderHeadCount();

				DecideShipDebarkCryo(cryoPass);
			}

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::TroopsLoad:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			// Troops can operate hull doors even if they are locked.
			int troopBerths = ship->TroopBerthsAvailable();

			// Pre-boarding announcement has already been made.
			// Wait the prescribed amount of time before boarding troops.
			if(!troopBerths || ship->MissionOrderIsExpired(ship->MissionOrderStart()))
			{
				// Pre-boarding announcement period has passed or all troop berths have been taken.
				DecideShipBoardTroops(ship->MissionOrderHeadCount());

				// Send troop boarding completed message.
				const DaSector* sector = ship->FindParentSector();
				if(sector)
				{
					AuCommStdChannel::RANGE range;
					if(ship->IsOnGround() && !ship->BdId().IsNull())
						range = AuCommStdChannel::Close;
					else
						range = AuCommStdChannel::SolarSystem;
					AuCommTroopBoardingComplete msg(AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(),
							sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
					SayNoRepeat(msg, AuVisage::Closed);
				}

				// Stop monitoring the Trade channel for acceptance of the boarding announcement.
				g_Au.StopMonitoringChannel(AuCommStdChannel::TradeChannelName(), ship->Id());

				// We only get one shot at doing this.
				orderComplete = true;
			}
		}
		break;

	case AuMissionOrder::CrewLoad:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			// Crew can operate hull doors even if they are locked.
			int crewBerths = ship->CrewBerthsAvailable();

			// Pre-boarding announcement has already been made.
			// Wait the prescribed amount of time before boarding crew members.
			if(!crewBerths || ship->MissionOrderIsExpired(ship->MissionOrderStart()))
			{
				// Pre-boarding announcement period has passed or all crew berths have been taken.
				DecideShipBoardCrew(ship->MissionOrderHeadCount());

				// Send crew boarding completed message.
				const DaSector* sector = ship->FindParentSector();
				if(sector)
				{
					AuCommStdChannel::RANGE range;
					if(ship->IsOnGround() && !ship->BdId().IsNull())
						range = AuCommStdChannel::Close;
					else
						range = AuCommStdChannel::SolarSystem;
					AuCommCrewBoardingComplete msg(AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(),
							sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
					SayNoRepeat(msg, AuVisage::Closed);
				}

				// Stop monitoring the Trade channel for acceptance of the boarding announcement.
				g_Au.StopMonitoringChannel(AuCommStdChannel::TradeChannelName(), ship->Id());

				// We only get one shot at doing this.
				orderComplete = true;
			}
		}
		break;

	case AuMissionOrder::CryoBerthsLoad:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			DecideShipBoardCryo(ship->MissionOrderHeadCount());

			// Send cryo traveller boarding completed message.
			const DaSector* sector = ship->FindParentSector();
			if(sector)
			{
				AuCommStdChannel::RANGE range;
				if(ship->IsOnGround() && !ship->BdId().IsNull())
					range = AuCommStdChannel::Close;
				else
					range = AuCommStdChannel::SolarSystem;
				AuCommCryoBoardingComplete msg(AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(),
						sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Closed);
			}

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::CargoBuyCategory:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			int holdSpace = ship->CargoCapacityVolume();
			int spaceAvail = holdSpace - ship->CargoTotalVolume();
			if(holdSpace && spaceAvail > 0)
			{
				// The ship has hold space available.
				COMMODITYFILTER cargoFilter = ship->MissionOrderCommodityFilter();
				QList<COMMODITYID> commodities;
				g_Au.m_CommodityArray.ListFilterCommodities(commodities, cargoFilter);
				if(!commodities.isEmpty())
				{
					// Count total items in hold that are in the category.
					int cargoInHold = 0;
					int cargoTypes = commodities.count();
					const AuItemHash& sinv = ship->LockCargoForRead();
					for(QList<COMMODITYID>::const_iterator ci = commodities.cbegin(); ci != commodities.cend(); ci++)
					{
						// Don't count fuel or electricity because it gets stored in a different place.
						if(*ci == CID_Hydrogen || *ci == CID_Electricity)
							cargoTypes--;
						else
							cargoInHold += sinv.RawCount(*ci);
					}
					ship->UnlockCargo();
					
					int cargoWanted;
					if(ship->MissionOrderAmountType() == 0)
					{
						// Total wanted. Count is a percentage.
						Q_ASSERT(ship->MissionOrderAmount() <= 100);
						if(ship->MissionOrderAmount() == 100)
							cargoWanted = holdSpace;
						else if(holdSpace >= 0x7fffffff/100)
							cargoWanted = holdSpace / 100 * ship->MissionOrderAmount();	// Avoid integer rollover.
						else
							cargoWanted = holdSpace * ship->MissionOrderAmount() / 100;
					}
					else if(ship->MissionOrderAmountType() == 1)
					{
						cargoWanted = ship->MissionOrderAmount();	// Amount is a unit count
						if(cargoWanted > holdSpace)
							cargoWanted = holdSpace;
					}
					else
						cargoWanted = holdSpace;

					if(!cargoWanted)
						cargoWanted = 1;
					if(cargoInHold < cargoWanted && cargoTypes)
					{
						// The mission demands that I buy cargo.
						int eachWanted;
						if(ship->MissionOrderAmountType() == 2)
							eachWanted = ship->MissionOrderAmount();			// Order specifies the count of each.
						else
							eachWanted = cargoWanted / cargoTypes;				// Split by number of cargo types in the category.
						if(!eachWanted)
							eachWanted = 1;

						DecideShipBuyCargoCategory(commodities, eachWanted, spaceAvail, cargoWanted);
					}
				}
			}

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::CargoSellCategory:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			COMMODITYFILTER cargoFilter = ship->MissionOrderCommodityFilter();
			QList<COMMODITYID> commodities;
			g_Au.m_CommodityArray.ListFilterCommodities(commodities, cargoFilter);
			if(cargoFilter == FILTER_All)
			{
				commodities.removeOne(CID_Electricity);	// Don't sell my power during a 'sell all' order.
				commodities.removeOne(CID_Hydrogen);	// Don't sell my fuel during a 'sell all' order.
			}
			if(!commodities.isEmpty())
				DecideShipSellCargoCategory(commodities, ship->MissionOrderAmountType(), ship->MissionOrderAmount());

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::CargoBuyCommodity:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			COMMODITYID cargoCID = ship->MissionOrderCommodityId();
			int cargoInHold;
			int spaceAvail;		// Space available for loading.
			int holdSpace;		// Total gross space.
			if(cargoCID == CID_Hydrogen)
			{
				cargoInHold = (int)ship->FuelQuantity();								// Total currently in fuel cells.
				holdSpace = ship->FuelCapacity();
				spaceAvail = ship->FuelWanted();
			}
			else if(cargoCID == CID_Electricity)
			{
				cargoInHold = (int)ship->CapacitorLevel();								// Total currently in capacitor.
				holdSpace = (int)ship->CapacitorMax();
				spaceAvail = (int)ceil(ship->CapacitorMax() - ship->CapacitorLevel());
			}
			else if(cargoCID == CID_Invalid)
			{
				// Do not try to buy the invalid commodity.
				cargoInHold = 0;
				holdSpace = 0;
				spaceAvail = 0;
			}
			else
			{
				const AuItemHash& sinv = ship->LockCargoForRead();
				cargoInHold = sinv.RawCount(cargoCID);							// Total currently in hold.
				ship->UnlockCargo();

				holdSpace = ship->CargoCapacityVolume();
				spaceAvail = holdSpace - ship->CargoTotalVolume();
			}
			//qDebug("%s buy cid %d cargoInHold %d holdSpace %d", (const char*)Name().toAscii(), cargoCID, cargoInHold, holdSpace);
			if(holdSpace && spaceAvail > 0)
			{
				// The ship has hold space and space available to load.
				int cargoWanted;
				if(ship->MissionOrderAmountType() == 0)
				{
					// Total wanted. Count is a percentage.
					Q_ASSERT(ship->MissionOrderAmount() <= 100);
					if(ship->MissionOrderAmount() == 100)
						cargoWanted = holdSpace;
					else if(holdSpace >= 0x7fffffff/100)
						cargoWanted = holdSpace / 100 * ship->MissionOrderAmount();	// Avoid integer rollover.
					else
						cargoWanted = holdSpace * ship->MissionOrderAmount() / 100;
				}
				else
				{
					cargoWanted = ship->MissionOrderAmount();	// Amount is a unit count
					if(cargoWanted > holdSpace)
						cargoWanted = holdSpace;
				}

				if(!cargoWanted)
					cargoWanted = 1;

				//qDebug("%s cargoWanted %d cargoToLoad %d", (const char*)Name().toAscii(), cargoWanted, cargoWanted-cargoInHold);
				if(cargoInHold < cargoWanted)
				{
					// The mission demands that I buy cargo.
					int cargoToLoad = cargoWanted - cargoInHold;						// Net amount wanted to load.
					if(cargoToLoad > spaceAvail)
						cargoToLoad = spaceAvail;

					//if(Is(1066326070))
						//qDebug() << "DaSentient::CommandShip" << Id() << Name() << "cid" << cargoCID
							//<< "cargoToLoad" << cargoToLoad << "= cargoWanted" << cargoWanted << "- cargoInHold" << cargoInHold
							//<< "spaceAvail" << spaceAvail;
					DecideShipBuyCargoCommodity(cargoCID, cargoToLoad);
				}
			}

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::CargoSellCommodity:
		huntEnemy = false;
		huntAsteroids = false;
		if(!ship->IsMoving() && !ship->HasWaypoint())
		{
			// Ship is at rest and no nav route is set.
			COMMODITYID cargoCID = ship->MissionOrderCommodityId();
			// Determine quantity of that cargo. Fuel and electricity get stored in a different place than the hold.
			int cargoInHold;
			if(cargoCID == CID_Hydrogen)
				cargoInHold = (int)ship->FuelQuantity();								// Total currently in fuel cells.
			else if(cargoCID == CID_Electricity)
				cargoInHold = (int)ship->CapacitorLevel();								// Total currently in capacitor.
			else
			{
				const AuItemHash& sinv = ship->LockCargoForRead();
				cargoInHold = sinv.RawCount(cargoCID);							// Total currently in hold.
				ship->UnlockCargo();
			}
			if(cargoInHold)
			{
				// Ship has cargo.
				int cargoToUnload;
				if(ship->MissionOrderAmountType() == 0)
				{
					Q_ASSERT(ship->MissionOrderAmount() <= 100);
					if(ship->MissionOrderAmount() == 100)
						cargoToUnload = cargoInHold;
					else if(cargoInHold >= 0x7fffffff/100)
						cargoToUnload = cargoInHold / 100 * ship->MissionOrderAmount();	// Avoid integer rollover.
					else
						cargoToUnload = cargoInHold * ship->MissionOrderAmount() / 100;
				}
				else
				{
					cargoToUnload = ship->MissionOrderAmount();	// Amount is a unit count
					if(cargoToUnload > cargoInHold)
						cargoToUnload = cargoInHold;
				}

				if(!cargoToUnload)
					cargoToUnload = 1;					// minimum 1
				if(cargoToUnload)
					DecideShipSellCargoCommodity(cargoCID, cargoToUnload);
			}

			// We only get one shot at doing this.
			orderComplete = true;
		}
		break;

	case AuMissionOrder::ScanSurveySystem:
		if(ship->SensorRange() <= 0)
		{
			orderComplete = true;
			completionCode = 14;
		}
		else
		{
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
			if(system && system->IsScannedBy(ship->EmpireId()))
			{
				orderComplete = true;
				completionCode = 12;
			}
		}
		break;

	case AuMissionOrder::DamageReport:
	case AuMissionOrder::ScanSurveyPlanet:
	case AuMissionOrder::ScanTactical:
		// A crewman or I was given a unit order to perform the report when the ship mission order became current.
		// I don't have to wait for it to be completed.
		orderComplete = true;
		completionCode = 12;
		break;

	case AuMissionOrder::Scuttle:
	case AuMissionOrder::HarvestAtmosphere:
	case AuMissionOrder::HarvestMinerals:
	case AuMissionOrder::HarvestParticles:
	case AuMissionOrder::HarvestSea:
	case AuMissionOrder::HarvestWell:
		// The ship carries out this order.
		// Wait patiently for the ship to complete the order.
		huntEnemy = false;
		huntAsteroids = false;
		break;
	}

	// Perform actions that can only be done while seated in a captain's chair.
	if(IsCommandStationPose())
	{
		// Sitting in captain's chair.
		// Close hull doors when inside air pressure is about to become unbreathable.
		float shipOutsideAir = ship->AtmosphereDensity();
		if(shipOutsideAir < .15f)
		{
			// Outside air pressure is too thin to breath.
			if(AirDensity() < .2f)
			{
				// Interior air pressure of ship is almost too low.
				// Close the hull doors if doing so will restore a breathable environment.
				// Don't imprison people inside a ship that will kill them.
				if(ship->AnyHullDoorOpen() && ship->LifeSupportWorking())
					DecideShipCloseAllHullDoors();
			}
		}
		else if(shipOutsideAir > .85f)
		{
			// Outside air pressure is too thick to breath.
			if(AirDensity() > .8f)
			{
				// Interior air pressure of ship is almost too high.
				// Close the hull doors if doing so will restore a breathable environment.
				// Don't imprison people inside a ship that will kill them.
				if(ship->AnyHullDoorOpen() && ship->LifeSupportWorking())
					DecideShipCloseAllHullDoors();
			}
		}
	}

	if(orderComplete)
	{
		// Current order is complete.
		DecideShipMissionOrderComplete(completionCode);
	}
	else if(huntEnemy || huntAsteroids)
	{
		// Current order is not complete so hunt enemy spacecraft that show up on my sensors.
		float sensorRange = ship->SensorRange();
		if(sensorRange < FeetPerMoonOrbit())
			sensorRange = FeetPerMoonOrbit();					// Optimistic range for "eyesight".

		AuId gunnerWorldId;
		int gunnerDomain = ship->GunnerDomain(gunnerWorldId);

		// Look for an enemy spacecraft blip that is in range.
		const DaAtom* targetAtom = 0;
		double targetRange = 0;
		AuPoint targetSysLoc;
		ship->FindEnvironmentAtom()->FindClosestShipWeaponTargetRecursive(ship->Id(), gunnerDomain, gunnerWorldId, EmpireId(),
				Posture(), ship->EnvLocation(), sensorRange, targetAtom, targetRange, targetSysLoc);
		if(targetAtom)
		{
			if(huntEnemy)
			{
				// Hunt enemy and asteroids.
				if(dynamic_cast<const DaVessel*>(targetAtom) || targetAtom->AtomType() == ATOM_Asteroid)
				{
					// Detected an enemy spacecraft blip that is in range.
					// Insert a do-once attack order to go after the enemy ship.
					// It will be removed after the enemy is gone, even if my current mission is cyclic.
					DecideShipMissionAttack(targetAtom->Id(), targetAtom->Name());
				}
			}
			else
			{
				// Hunt only asteroids.
				if(targetAtom->AtomType() == ATOM_Asteroid)
				{
					// Detected an asteroid blip that is in range.
					// Insert a do-once attack order to go after the asteroid.
					// It will be removed after the asteroid is gone, even if my current mission is cyclic.
					DecideShipMissionAttack(targetAtom->Id(), targetAtom->Name());
				}
			}
		}
	}
}

// Induces some overhead time based on the rank of this officer.
// Low ranking officers take longer to reply true.
bool DaSentient::IsReadyToStartMissionOrder(const DaVessel* ship) const
{
	if(ship->MissionIsEmpty())
		return true;

	int waitSecs = ship->MissionOrderOrderTime() * (6 - Rank());				// My rank value range is 0-5.

	// We're ready if the wait time has passed.
	if(ship->MissionOrderStart().IsNull())
	{
		qDebug() << "DaSentient::IsReadyToStartMissionOrder" << Id() << Name() << "aboard" << ship->Id() << ship->Name()
			<< "Mission OrderStart not valid";
		return true;
	}
	return (ship->MissionOrderStart().ElapsedSec() < waitSecs) ? false : true;
}

void DaSentient::AnnounceMissionOrder(DaVessel* ship)
{
	// Remember when I have verbally issued the current order.
	SetOrderAnnounced();

	// The AI first officer announces this information for the benefit of human crew aboard the ship; that includes officers.
	// The AI first officer does not announce this information if there is no human crew to hear it.
	DaAtomList subs;
	ship->ListNormalSubatoms(subs);
	if(subs.empty())
		return;

	bool humanCrewAboard = false;		// true if any human player on-line is aboard who is a crew member.
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		const DaPerson* person = dynamic_cast<const DaPerson*>(*ai);
		if(!person)
			continue;
		if(!person->IsOnLine())
			continue;
		if(!ship->IsCrewOrTroop(person->Id()))
			continue;
		humanCrewAboard = true;
		break;
	}

	if(!humanCrewAboard)
		return;							// No human aboard to hear the message.

	QString pilotN;
	const DaCreature* pilot = ship->Pilot();
	if(pilot)
		pilotN = pilot->CrewName();
	else
		pilotN = tr("Helmsman");

	QString navigatorN;
	const DaCreature* navigator = ship->Navigator();
	if(navigator)
		navigatorN = navigator->CrewName();
	else
		navigatorN = tr("Navigator");

	QString engineerN;
	const DaCreature* engineer = ship->Engineer();
	if(engineer)
		engineerN = engineer->CrewName();

	QString powerN;
	const DaCreature* power = ship->PowerOp();
	if(power)
		powerN = power->CrewName();

	QString sensorN;
	const DaCreature* sensor = ship->SensorOp();
	if(sensor)
		sensorN = sensor->CrewName();

	QString shieldsN;
	const DaCreature* shields = ship->ShieldOp();
	if(shields)
		shieldsN = shields->CrewName();

	QString text = ship->MissionOrderVerbalization(pilotN, navigatorN, engineerN, powerN, sensorN, shieldsN);
	if(!text.isEmpty())
	{
		AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
		SayNoRepeat(msg, AuVisage::Closed);
	}
}

void DaSentient::AnnounceMissionOrderComplete(DaVessel* ship, int completionCode)
{
	// The AI first officer announces this information for the benefit of human officers of the ship.
	// The AI first officer does not announce this information if there is no human officer to hear it.
	// Human players could be on the crew or passenger roster. The first officer does not report to them.
	// Determine the presense of human officers.
	bool humanOfficer = false;									// true if any human officer is on the roster
	bool humanOfficerNotInSystem = false;						// true if any human officer is not found in the system at all
	bool humanOfficerOnLineInSystem = false;					// true if any human officer on line is found in the system
	DaAtom* system = ship->FindEnvironmentAtom();
	const AuRoster& roster = ship->LockRoster();
	for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
	{
		if((*ri).IsHumanOfficer())
		{
			// Found a roster entry occupied by a human officer.
			humanOfficer = true;

			// We would like to find one officer on line in the system.
			if(!humanOfficerOnLineInSystem || !humanOfficerNotInSystem)
			{
				const DaPerson* person = dynamic_cast<const DaPerson*>(system->FindIdRecursive((*ri).m_OccId));
				if(person)
				{
					// Human officer was found in the system.
					if(person->IsOnLine())
					{
						humanOfficerOnLineInSystem = true;		// Human officer found who can receive the message at system range.
						break;									// No need to continue searching for officers at this point.
					}
				}
				else
				{
					// Human officer was not found in the system at all. They may be able to receive the message at galaxy range.
					humanOfficerNotInSystem = true;
				}
			}
		}
	}
	ship->UnlockRoster();

	// Select range at which to broadcast the message based on what human officer can receive it.
	if(!humanOfficer)
		return;													// Do not broadcast if no human will receive the message.
	AuCommStdChannel::RANGE range;
	if(humanOfficerOnLineInSystem)
		range = AuCommStdChannel::SolarSystem;					// Human officer on line in the system can receive at system range.
	else if(humanOfficerNotInSystem)
		range = AuCommStdChannel::Universe;						// Human officer not in system can receive at galaxy range.
	else
		return;													// The human officer(s) must be off line in the system.

	// Send an announcement on the crew channel.
	QString text;
	switch(completionCode)
	{
	case 0:
	default:
		if(ship->MissionOrderIsTravelOrder())
			text = ship->MissionOrderListDescription() + " " + tr("Arrived, sir.");
		else
			text = ship->MissionOrderListDescription() + " " + tr("Done, sir.");
		break;
	case 1:
		text = ship->MissionOrderListDescription() + " " + tr("Target destroyed, sir.");
		break;
	case 2:
		text = ship->MissionOrderListDescription() + " " + tr("No repairs needed, sir.");
		break;
	case 3:
		text = ship->MissionOrderListDescription() + " " + tr("Time's up, sir.");
		break;
	case 4:
		text = ship->MissionOrderListDescription() + " " + tr("Destination world not here, sir.");
		break;
	case 5:
		text = ship->MissionOrderListDescription() + " " + tr("No good city here, sir.");
		break;
	case 6:
		text = ship->MissionOrderListDescription() + " " + tr("No good station here, sir.");
		break;
	case 7:
		text = ship->MissionOrderListDescription() + " " + tr("No good station or city here, sir.");
		break;
	case 8:
		text = ship->MissionOrderListDescription() + " " + tr("Target not here. Breaking off, sir.");
		break;
	case 9:
		text = ship->MissionOrderListDescription() + " " + tr("Target entered atmosphere. Breaking off, sir.");
		break;
	case 10:
		text = ship->MissionOrderListDescription() + " " + tr("Yellow alert! Breaking off the attack, sir.");
		break;
	case 11:
		text = ship->MissionOrderListDescription() + " " + tr("Red alert! Breaking off the attack, sir.");
		break;
	case 12:
		text = ship->MissionOrderListDescription() + " " + tr("Report initiated; it will be delivered ASAP, sir.");
		break;
	case 13:
		text = tr("Mission deactivated; holding position, sir.");
		break;
	case 14:
		text = tr("Unable to complete the scan due to scanner equipment failure, sir.");
		break;
	case 15:
		text = tr("This system has already been scanned by our empire, sir. "
				"Check your star map; you may need fresh map data.");
		break;
	case 16:
		text = ship->MissionOrderListDescription() + " " + tr("No money to purchase repairs, sir.");
		break;
	case 17:
		text = tr("All harvester operations are complete as ordered. Super job crew.");
		break;
	case 18:
		text = tr("Stopping harvester operations. The hold is full. Great job crew.");
		break;
	case 19:
		text = tr("Stopping harvester operations. Our fuel is at the threshold.");
		break;
	case 20:
		text = ship->MissionOrderListDescription() + " " + tr("Target entered ADIZ. Breaking off, sir.");
		break;
	case 21:
		text = ship->MissionOrderListDescription() + " " + tr("No repair shops to perform repairs, sir.");
		break;
	case 22:
		text = ship->MissionOrderListDescription() + " " + tr("Ship account has insufficient funds to complete the repairs, sir.");
		break;
	case 23:
		text = ship->MissionOrderListDescription() + " " + tr("Repair shop does not have parts to complete the repairs, sir.");
		break;
	case 99:
		text = ship->MissionOrderListDescription() + " " + tr("Madness, I say! I can't do it, sir.");
		break;
	}
	AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
			FindParentSector()->Id(), Galaxy(), range, AuCommStdChannel::CrewMates);
	SayNoRepeat(msg, AuVisage::Closed);
}

bool DaSentient::MissionOrderChanged(DaVessel* ship, const AuMissionOrder& prevOrder)
{
	// Some orders require work as soon as they become the current order, such as pre-boarding announcements.
	ResetOrderAnnounced();
	switch(ship->MissionOrderType())
	{
	case AuMissionOrder::Repair:
		// Avoid mooring and system shutdown when no repairs are needed, even for just a few seconds.
		if(!ship->IsDamaged())
			return false;	// Reject repair order when the ship is not damaged. It may become the current order anyway.
		// Fall through to hail for repairs.
		[[fallthrough]];
	case AuMissionOrder::CargoBuyCategory:
	case AuMissionOrder::CargoBuyCommodity:
	case AuMissionOrder::CargoSellCategory:
	case AuMissionOrder::CargoSellCommodity:
	case AuMissionOrder::PassengersUnload:
	case AuMissionOrder::CryoBerthsLoad:
	case AuMissionOrder::CryoBerthsUnload:
	case AuMissionOrder::TransferCargoCategoryFrom:
	case AuMissionOrder::TransferCargoCategoryTo:
	case AuMissionOrder::TransferCargoFrom:
	case AuMissionOrder::TransferCargoTo:
	case AuMissionOrder::TransferMoneyFrom:
	case AuMissionOrder::TransferMoneyTo:
	case AuMissionOrder::TransferPeopleFrom:
	case AuMissionOrder::TransferPeopleTo:
		// Hail on the trade channel, just like players do.
		{
			const DaSector* sector = ship->FindParentSector();
			if(!sector)
				break;

			AuCommStdChannel::RANGE range;
			if(ship->IsOnGround() && !ship->BdId().IsNull())
				range = AuCommStdChannel::Close;
			else
				range = AuCommStdChannel::SolarSystem;
			AuCommHailRequest msg(EmpireId(), ship->MissionOrderHailIntentions(), AuCommStdChannel::TradeChannelName(),
					ship->Id(), ship->Name(), sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
			SayNoRepeat(msg, AuVisage::Closed);

			if(!ship->IsPrivatelyOwned())
			{
				// My ship is subsidized.
				// My hail message does not get received by any non-person atoms. Normally a city would grab my subsidy tax account
				// and pay back pay to my crew when my hail was received. I must do that work myself, here.
				if(ship->AccountBalanceSubsidyTax() > 0)
				{
					// There is money in the subsidy tax account or the crew is owed back pay.
					// Find an empire city in the system that has a bank.
					DaAtom* system = ship->FindEnvironmentAtom();
					if(!system)
						break;

					// Get a list of empire cities here. 
					DaAtomList cities;
					system->FindCitiesBdRecursive(cities, ship->EmpireId());
					for(DaAtomList::const_iterator ci = cities.begin(); ci != cities.end(); ci++)
					{
						DaBd* bd = dynamic_cast<DaBd*>(*ci);
						if(bd)
						{
							if(!bd->LastReportedLevels(IID_Bank))
								continue;

							// Transfer my subsidy tax to the city's government account.
							bd->CreditAccountGovtSubsidyTax(ship->AccountBalanceSubsidyTax());
							ship->DebitAccountSubsidyTax(ship->AccountBalanceSubsidyTax());
							break;
						}
					}
				}
			}
		}
		break;

	case AuMissionOrder::PassengersLoad:
		// If PreboardingAnnouncementMinutes() == 0, no pre-boarding announcement is made.
		ship->ReconcilePassengers();
		if(ship->MissionOrderPreboardingAnnouncementMinutes())
		{
			// Make the pre-boarding announcement now.
			unsigned int passBerths = ship->PassengerBerthsAvailable();
			if(passBerths)
			{
				// If a count was specified, limit the number to board to that count.
				if(ship->MissionOrderHeadCount() && passBerths > ship->MissionOrderHeadCount())
					passBerths = ship->MissionOrderHeadCount();

				// Broadcast a pre-boarding announcement on the Hail channel.
				const DaSector* sector = ship->FindParentSector();
				if(!sector)
					break;

				int locType = -1;
				QString locName;
				QString locLongLat;
				if(ship->IsInAtmosphere())
				{
					// Ship is in an atmosphere. Locate the city the ship is in.
					if(ship->IsOnGround() && !ship->InBdCity().IsNull())
					{
						DaWorld* planet = ship->FindParentPlanetInEnv();
						DaBd* bd = planet ? planet->FindBd(ship->InBdCity()) : 0;
						if(bd)
						{
							locType = 0;
							locName = bd->CityOrBuildingName();

							AuPoint shipLoc;
							ship->GetLocationRelativeToParent(shipLoc, planet);
							Q_ASSERT(!shipLoc.IsNaN());
							const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
							if(llworld)
							{
								AuGlobeLLF shipLL;
								if(llworld->LLOfLoc(shipLL, shipLoc))
									locLongLat = shipLL.ToString();
							}
						}
					}
				}
				else
				{
					// Ship is in space.
					// Locate station within trade range of the ship.
					bool hasStation;
					DaVessel* station = ship->StationInTradeRange(ship->RoughRadius(), hasStation);
					if(station)
					{
						// Ship is in trade range of the station.
						DaWorld* planet = ship->FindParentPlanetInEnv();
						if(planet)
						{
							locType = 1;
							locName = station->Name();

							AuPoint shipLoc;
							ship->GetLocationRelativeToParent(shipLoc, planet);
							Q_ASSERT(!shipLoc.IsNaN());
							const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
							if(llworld)
							{
								AuGlobeLLF shipLL;
								if(llworld->LLOfLoc(shipLL, shipLoc))
									locLongLat = shipLL.ToString();
							}
						}
						else
						{
							locType = 2;
							locName = station->Name();
						}
					}
				}

				if(locType >= 0)
				{
					// Monitor the Trade channel for acceptance of the boarding announcement.
					g_Au.MonitorChannel(AuCommStdChannel::TradeChannelName(), ship->Id());

					AuCommStdChannel::RANGE range;
					if(ship->IsOnGround() && !ship->BdId().IsNull())
						range = AuCommStdChannel::Close;
					else
						range = AuCommStdChannel::SolarSystem;
					AuCommPreBoard msg(ship->Id(), ship->NameWithEmpire(), ship->MissionOrderPreboardingAnnouncementMinutes(),
							passBerths, ship->IsFirstClass()?1:ship->IsSecondClass()?2:3, locType, locName, locLongLat,
							ship->MissionNextSystem(),
							AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(),
							sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
					SayNoRepeat(msg, AuVisage::Closed);
				}
			}
		}
		else
		{
			// Hail on the trade channel, just like players do.
			int passBerths = ship->PassengerBerthsAvailable();
			if(passBerths)
			{
				const DaSector* sector = ship->FindParentSector();
				if(!sector)
					break;

				AuCommStdChannel::RANGE range;
				if(ship->IsOnGround() && !ship->BdId().IsNull())
					range = AuCommStdChannel::Close;
				else
					range = AuCommStdChannel::SolarSystem;
				AuCommHailRequest msg(EmpireId(), ship->MissionOrderHailIntentions(),
						AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(), sector->Id(), Galaxy(), range,
						AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Closed);
			}
		}
		break;

	case AuMissionOrder::TroopsLoad:
		// If PreboardingAnnouncementMinutes() == 0, no pre-boarding announcement is made.
		ship->ReconcileTroops();
		if(ship->MissionOrderPreboardingAnnouncementMinutes())
		{
			// Make the pre-boarding announcement now.
			unsigned int troopBerths = ship->TroopBerthsAvailable();
			if(troopBerths)
			{
				// If a count was specified, limit the number to board to that count.
				if(ship->MissionOrderHeadCount() && troopBerths > ship->MissionOrderHeadCount())
					troopBerths = ship->MissionOrderHeadCount();

				// Broadcast a pre-boarding announcement on the Hail channel.
				const DaSector* sector = ship->FindParentSector();
				if(!sector)
					break;

				int locType = -1;
				QString locName;
				QString locLongLat;
				if(ship->IsInAtmosphere())
				{
					// Ship is in an atmosphere. Locate the city the ship is in.
					if(ship->IsOnGround() && !ship->InBdCity().IsNull())
					{
						DaWorld* planet = ship->FindParentPlanetInEnv();
						DaBd* bd = planet ? planet->FindBd(ship->InBdCity()) : 0;
						if(bd)
						{
							locType = 0;
							locName = bd->CityOrBuildingName();

							AuPoint shipLoc;
							ship->GetLocationRelativeToParent(shipLoc, planet);
							Q_ASSERT(!shipLoc.IsNaN());
							const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
							if(llworld)
							{
								AuGlobeLLF shipLL;
								if(llworld->LLOfLoc(shipLL, shipLoc))
									locLongLat = shipLL.ToString();
							}
						}
					}
				}
				else
				{
					// Ship is in space.
					// Locate station within trade range of the ship.
					bool hasStation;
					DaVessel* station = ship->StationInTradeRange(ship->RoughRadius(), hasStation);
					if(station)
					{
						// Ship is in trade range of the station.
						DaWorld* planet = ship->FindParentPlanetInEnv();
						if(planet)
						{
							locType = 1;
							locName = station->Name();

							AuPoint shipLoc;
							ship->GetLocationRelativeToParent(shipLoc, planet);
							Q_ASSERT(!shipLoc.IsNaN());
							const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
							if(llworld)
							{
								AuGlobeLLF shipLL;
								if(llworld->LLOfLoc(shipLL, shipLoc))
									locLongLat = shipLL.ToString();
							}
						}
						else
						{
							locType = 2;
							locName = station->Name();
						}
					}
				}

				if(locType >= 0)
				{
					// Monitor the Trade channel for acceptance of the boarding announcement.
					g_Au.MonitorChannel(AuCommStdChannel::TradeChannelName(), ship->Id());

					AuCommStdChannel::RANGE range;
					if(ship->IsOnGround() && !ship->BdId().IsNull())
						range = AuCommStdChannel::Close;
					else
						range = AuCommStdChannel::SolarSystem;
					AuCommPreBoard msg(ship->Id(), ship->NameWithEmpire(), ship->MissionOrderPreboardingAnnouncementMinutes(),
							troopBerths, 4, locType, locName, locLongLat, ship->MissionNextSystem(),
							AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(),
							sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
					SayNoRepeat(msg, AuVisage::Closed);
				}
			}
		}
		else
		{
			// Hail on the trade channel, just like players do.
			int troopBerths = ship->TroopBerthsAvailable();
			if(troopBerths)
			{
				const DaSector* sector = ship->FindParentSector();
				if(!sector)
					break;

				AuCommStdChannel::RANGE range;
				if(ship->IsOnGround() && !ship->BdId().IsNull())
					range = AuCommStdChannel::Close;
				else
					range = AuCommStdChannel::SolarSystem;
				AuCommHailRequest msg(EmpireId(), ship->MissionOrderHailIntentions(),
						AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(), sector->Id(), Galaxy(), range,
						AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Closed);
			}
		}
		break;

	case AuMissionOrder::CrewLoad:
		// If PreboardingAnnouncementMinutes() == 0, no pre-boarding announcement is made.
		ship->ReconcileCrew();
		if(ship->MissionOrderPreboardingAnnouncementMinutes())
		{
			// Make the pre-boarding announcement now.
			unsigned int crewBerths = ship->CrewBerthsAvailable();
			if(crewBerths)
			{
				// If a count was specified, limit the number to board to that count.
				if(ship->MissionOrderHeadCount() && crewBerths > ship->MissionOrderHeadCount())
					crewBerths = ship->MissionOrderHeadCount();

				// Broadcast a pre-boarding announcement on the Hail channel.
				const DaSector* sector = ship->FindParentSector();
				if(!sector)
					break;

				int locType = -1;
				QString locName;
				QString locLongLat;
				if(ship->IsInAtmosphere())
				{
					// Ship is in an atmosphere. Locate the city the ship is in.
					if(ship->IsOnGround() && !ship->InBdCity().IsNull())
					{
						DaWorld* planet = ship->FindParentPlanetInEnv();
						DaBd* bd = planet ? planet->FindBd(ship->InBdCity()) : 0;
						if(bd)
						{
							locType = 0;
							locName = bd->CityOrBuildingName();

							AuPoint shipLoc;
							ship->GetLocationRelativeToParent(shipLoc, planet);
							Q_ASSERT(!shipLoc.IsNaN());
							const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
							if(llworld)
							{
								AuGlobeLLF shipLL;
								if(llworld->LLOfLoc(shipLL, shipLoc))
									locLongLat = shipLL.ToString();
							}
						}
					}
				}
				else
				{
					// Ship is in space.
					// Locate station within trade range of the ship.
					bool hasStation;
					DaVessel* station = ship->StationInTradeRange(ship->RoughRadius(), hasStation);
					if(station)
					{
						// Ship is in trade range of the station.
						DaWorld* planet = ship->FindParentPlanetInEnv();
						if(planet)
						{
							locType = 1;
							locName = station->Name();

							AuPoint shipLoc;
							ship->GetLocationRelativeToParent(shipLoc, planet);
							Q_ASSERT(!shipLoc.IsNaN());
							const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
							if(llworld)
							{
								AuGlobeLLF shipLL;
								if(llworld->LLOfLoc(shipLL, shipLoc))
									locLongLat = shipLL.ToString();
							}
						}
						else
						{
							locType = 2;
							locName = station->Name();
						}
					}
				}

				if(locType >= 0)
				{
					// Monitor the Trade channel for acceptance of the boarding announcement.
					g_Au.MonitorChannel(AuCommStdChannel::TradeChannelName(), ship->Id());

					AuCommStdChannel::RANGE range;
					if(ship->IsOnGround() && !ship->BdId().IsNull())
						range = AuCommStdChannel::Close;
					else
						range = AuCommStdChannel::SolarSystem;
					AuCommPreBoard msg(ship->Id(), ship->NameWithEmpire(), ship->MissionOrderPreboardingAnnouncementMinutes(),
							crewBerths, 5, locType, locName, locLongLat, ship->MissionNextSystem(),
							AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(),
							sector->Id(), Galaxy(), range, AuCommStdChannel::Everyone);
					SayNoRepeat(msg, AuVisage::Closed);
				}
			}
		}
		else
		{
			int crewBerths = ship->CrewBerthsAvailable();
			if(crewBerths)
			{
				// Hail on the trade channel, just like players do.
				const DaSector* sector = ship->FindParentSector();
				if(!sector)
					break;

				AuCommStdChannel::RANGE range;
				if(ship->IsOnGround() && !ship->BdId().IsNull())
					range = AuCommStdChannel::Close;
				else
					range = AuCommStdChannel::SolarSystem;
				AuCommHailRequest msg(EmpireId(), ship->MissionOrderHailIntentions(),
						AuCommStdChannel::TradeChannelName(), ship->Id(), ship->Name(), sector->Id(), Galaxy(), range,
						AuCommStdChannel::Everyone);
				SayNoRepeat(msg, AuVisage::Closed);
			}
		}
		break;

	case AuMissionOrder::ScanSurveySystem:
		/*
		{
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ship->FindEnvironmentAtom());
			if(system && system->IsScannedBy(ship->EmpireId()))
				return false;
		}
		*/
		break;

	case AuMissionOrder::Attack:
	case AuMissionOrder::Christen:
	case AuMissionOrder::DamageReport:
	case AuMissionOrder::ExterminateRats:
	case AuMissionOrder::Follow:
	case AuMissionOrder::GoToBestCity:
	case AuMissionOrder::GoToBestStation:
	case AuMissionOrder::GoToBestStationOrCity:
	case AuMissionOrder::GoToCity:			// Land at airport order.
	case AuMissionOrder::GoToCityAnywhere:	// Land at city order.
	case AuMissionOrder::GoToClosestCity:
	case AuMissionOrder::GoToClosestStation:
	case AuMissionOrder::GoToClosestStationOrCity:
	case AuMissionOrder::GoToCoordinate:
	case AuMissionOrder::GoToHoverSite:
	case AuMissionOrder::GoToLand:
	case AuMissionOrder::GoToLandingSite:
	case AuMissionOrder::GoToOrbit:
	case AuMissionOrder::GoToOrbitBestPlanet:
	case AuMissionOrder::GoToOrbitCity:
	case AuMissionOrder::GoToOrbitCoords:
	case AuMissionOrder::GoToStation:
	case AuMissionOrder::GoToSystemDirect:
	case AuMissionOrder::GoToSystemDirect_Deprecated:
	case AuMissionOrder::GoToSystemViaWormholes:
	case AuMissionOrder::Harvest:
	case AuMissionOrder::HarvestAtmosphere:
	case AuMissionOrder::HarvestMinerals:
	case AuMissionOrder::HarvestOperationByBay:
	case AuMissionOrder::HarvestOperationByModule:
	case AuMissionOrder::HarvestParticles:
	case AuMissionOrder::HarvestSea:
	case AuMissionOrder::HarvestWell:
	case AuMissionOrder::Hide:
	case AuMissionOrder::HoldMission:
	case AuMissionOrder::LightState:
	case AuMissionOrder::PatrolCities:
	case AuMissionOrder::PatrolStations:
	case AuMissionOrder::PatrolSystem:
	case AuMissionOrder::PatrolWormholes:
	case AuMissionOrder::PilotPolicy:
	case AuMissionOrder::Plunder:
	case AuMissionOrder::Posture:
	case AuMissionOrder::Report:
	case AuMissionOrder::ScanSurveyPlanet:
	case AuMissionOrder::ScanTactical:
	case AuMissionOrder::Scuttle:
	case AuMissionOrder::ShieldPolicy:
	case AuMissionOrder::ShieldsDown:
	case AuMissionOrder::StandDown:
	case AuMissionOrder::Stay:
	case AuMissionOrder::Invalid:
	case AuMissionOrder::OrderTypes:
		break;
	}

	// Pirate ship captain may need to decide what to do now.
	// Pirate station caption does nothing.
	if(IsPirate() && ship->IsPirateWithFreeWill() && ship->MissionIsEmpty() && !ship->IsSpaceStation())
	{
		// I have no mission orders. What shall I do?
		// My previous order gives me a hint as to what I was doing.
		if(InArena())
		{
			// I am a pirate in an area.
			const DaArena* arena = FindParentArena();
			//qDebug() << "DaSentient::MissionOrderChanged" << Id() << Name() << "inArena state" << arena->MatchState()
				//<< "hasHailed" << arena->HasHailed(ship->Id()) << "pilot" << ship->IsPilotManned();
			if(!arena || arena->MatchState() != DaArena::Play)
			{
				// Hail when I am ready to start.
				if(arena && arena->MatchState() == DaArena::Join && ship->IsPilotManned() && !arena->HasHailed(ship->Id()))
				{
					AuCommHailRequest msg(EmpireId(), tr("Ready to begin the match."),
							AuCommStdChannel::HailChannelName(),
							ship->Id(), ship->Name(), ship->FindParentSector()->Id(), Galaxy(),
							AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					Say(msg, AuVisage::Closed);
				}

				// Wait patiently.
				ship->MissionAppendOrder(AuMissionOrderStay(1));
			}
			else if(!ship->LockedBlip().IsNull())
			{
				// Hunt enemy units.
				// Sensor operator will lock on to enemy units for me.
				const DaAtom* system = ship->FindEnvironmentAtom();
				const DaAtom* target = system->FindIdRecursive(ship->LockedBlip());
				if(dynamic_cast<const DaVessel*>(target) && target->EmpireId() != ship->EmpireId())
				{
					// We have a target. I am a pirate so all ships from other empires are fair game. Pirates ignore asteroids.
					//qDebug() << "DaSentient::MissionOrderChanged" << Id() << Name()
						//<< "aboard" << ship->Id() << ship->Name()
						//<< "prev Attack, I'm berserk so now Attack" << target->Id()
						//<< target->Name();
					ship->MissionAppendOrder(AuMissionOrderAttack(target->Id(), target->Name()));
				}
				else	// No enemy target so move to the center of the ring.
				{
					//qDebug() << "DaSentient::MissionOrderChanged" << Id() << Name()
						//<< "aboard" << ship->Id() << ship->Name()
						//<< "prev Attack, no enemy so now GoToBestStation";
					ship->MissionAppendOrder(AuMissionOrderGoToCoordinate(AuMissionOrder::SolarSystem, 0, 0, 0));
					ship->MissionAppendOrder(AuMissionOrderStay(1));
				}
			}
			else
			{
				// Nothing on the sensors so move to the center of the ring.
				//qDebug() << "DaSentient::MissionOrderChanged" << Id() << Name()
					//<< "aboard" << ship->Id() << ship->Name()
					//<< "prev Attack, no targets so now GoToBestStation";
				ship->MissionAppendOrder(AuMissionOrderGoToCoordinate(AuMissionOrder::SolarSystem, 0, 0, 0));
				ship->MissionAppendOrder(AuMissionOrderStay(1));
			}
		}
		else
		{
			// I am a pirate in the wild.
			PirateMissionOrderChanged(ship, prevOrder);
		}
	}

	// Announce the nu mission order as soon as it becomes active.
	if(!ship->IsMissionOrder(prevOrder))
		AnnounceMissionOrder(ship);

	// The nu mission order is accepted.
	return true;
}

bool DaSentient::DecideWhetherToTravel(DaVessel* ship) const
{
	if(ship->MissionOrderType() == AuMissionOrder::Follow)
		return true;

	if(ship->FindPersonOnLineRecursive())
		return true;					// Person on line so we can go anywhere.

	AuPoint destOffset;					// Used for going to wormholes and navigating near ringworlds.
	AuMissionOrder::TYPE destOrder;
	const DaAtom* destAtom = ship->MissionDestination(destOrder, destOffset);
	const DaSolarSystem* destSystem = dynamic_cast<const DaSolarSystem*>(destAtom);
	if(!destSystem)
		return true;					// Destination is not a solar system. It's ok to go there.

	// The destination is a solar system.
	const DaAtom* system = ship->FindEnvironmentAtom();
	if(destSystem != system)
	{
		if(InArena() || InDesignPreview())
			return false;
		return destSystem->IsExploredBy(ship->EmpireId());	// Ok to travel to explored solar system.
	}

	// Destination IS my current solar system.
	if(destOrder != AuMissionOrder::GoToCoordinate)
		return true;

	const DaSolarSystem* solar = dynamic_cast<const DaSolarSystem*>(system);
	if(!solar)
		return true;	// In an area it's ok to travel anywhere. Arena rules will take over when I go out of bounds.

	const DaStar* destStar = solar->FindPrimaryStar();
	if(destStar && destOffset.Length2() < destStar->Radius()*destStar->Radius())
		return true;	// Destination is inside the primary star.

	// Destination is a specific coordinate. The order may specify the destination coordinate relative to
	// the galaxy, sector or solar system. The DaVessel::MissionDestination function always returns a
	// destination that is relative to the current solar system.
	AuPoint shipSysLoc = ship->EnvLocation();

	// Adjust destOffset so it does not land inside a planet or star.
	solar->AdjustNavLocation(shipSysLoc, destOffset);
	AuPoint vecAhead = destOffset - shipSysLoc;
	double distToDest2 = vecAhead.Length2();	// Squared distance to the destination location.
	if(distToDest2 < 500*FeetPerMeter() * 500*FeetPerMeter())
		return true;

	// The ship is beyond 500m of the target location.
	if(solar->IsInSystem(destOffset))
		return true;

	// No online person is aboard and the destination departs the current solar system.
	// Insure the destination does not take us into unexplored space.
	// destOffset is in feet relative to my current solar system.
	if(distToDest2 > FeetPerParsec()*FeetPerParsec())
	{
		// Destination is more than 1pc away.
		// Scale the look-ahead vector to be no more than 1pc.
		vecAhead /= sqrt(distToDest2);	// Unit vector
		vecAhead *= FeetPerParsec();	// 1pc vector length
	}

	AuPoint lookAhead = shipSysLoc + vecAhead;	// Look ahead location relative to current system.
	const DaSector* ps = (const DaSector*)solar->Parent();
	AuMatrix4x4 msys;
	solar->GetTransformRelativeToParent(msys, ps);
	AuPoint sectorLoc = msys.Project(lookAhead);	// Result in pc relative to sector.

	// FindClosestSolarSystemInRegion will find the closest system, even if the search point is
	// far outside the box. If that system is not explored, we will not proceed.
	const DaSolarSystem* nextSystem = ps->FindClosestSolarSystemInRegion(sectorLoc);
	return nextSystem->IsExploredBy(ship->EmpireId());	// Ok to go if next system is explored by my empire.
}

// Troops monitor the Crew channel when they are assigned to a spacecraft.
// Troops monitor the Intercom channel when they are aboard a vehicle.
// AI controlled citizens monitor the Voice channel when they are spoken to.
void DaSentient::RxChatMessage(const AuCommPacket* msg, quint32& /*countSupplied*/, bool firstInGroup)
{
	if(!IsAIControlled())
		return;

	switch(msg->Type())
	{
	case AuCommPacket::CitizenOrder:
		RxCitizenOrder(dynamic_cast<const AuCommCitizenOrder*>(msg), firstInGroup);
		break;

	case AuCommPacket::CitizenPosture:
		RxCitizenPosture(dynamic_cast<const AuCommCitizenPosture*>(msg), firstInGroup);
		break;

	case AuCommPacket::BeamAtom:
		RxBeamAtom(dynamic_cast<const AuCommBeamAtom*>(msg));
		break;

	case AuCommPacket::BeamCoordinate:
		RxBeamCoordinate(dynamic_cast<const AuCommBeamCoordinate*>(msg));
		break;

	case AuCommPacket::BeamPlanet:
		RxBeamPlanet(dynamic_cast<const AuCommBeamPlanet*>(msg));
		break;

	case AuCommPacket::BeamShip:
		RxBeamShip(dynamic_cast<const AuCommBeamShip*>(msg));
		break;

	case AuCommPacket::BeamTransporter:
		RxBeamTransporter(dynamic_cast<const AuCommBeamTransporter*>(msg));
		break;

		/* These are handled by the ship.
	case AuCommPacket::BeamVehicleAtom:
		RxBeamVehicleAtom(dynamic_cast<const AuCommBeamVehicleAtom*>(msg));
		break;

	case AuCommPacket::BeamVehicleCoordinate:
		RxBeamVehicleCoordinate(dynamic_cast<const AuCommBeamVehicleCoordinate*>(msg));
		break;

	case AuCommPacket::BeamVehiclePlanet:
		RxBeamVehiclePlanet(dynamic_cast<const AuCommBeamVehiclePlanet*>(msg));
		break;

	case AuCommPacket::BeamVehicleShip:
		RxBeamVehicleShip(dynamic_cast<const AuCommBeamVehicleShip*>(msg));
		break;

	case AuCommPacket::BeamVehicleTransporter:
		RxBeamVehicleTransporter(dynamic_cast<const AuCommBeamVehicleTransporter*>(msg));
		break;
		*/

	case AuCommPacket::HailReply:
		RxHailReply(msg);
		break;

	case AuCommPacket::HailRequest:
		RxHailRequest(msg);
		break;

	case AuCommPacket::Chat:
		RxChat(dynamic_cast<const AuCommChat*>(msg));
		break;

					/*
	case AuCommPacket::BoardTroops:
		{
			if(m_Role != Troop)
				break;	// This citizen is not a troop.
			if(msg.Scope() != AuCommPacket::CrewMates)
				break;	// Only process this message on the Crew channel.

			// The sender is a fellow crew member.
			DaSolarSystem* system = FindParentSolarSystem();
			if(!system)
				break;
			DaPerson* person = dynamic_cast<DaPerson*>(system->FindIdRecursive(msg.FromId()));
			if(!person)
				break;	// Ignore message; sender is not in this system or not a person. Only respond to people.

			if(person->IsAboardVehicle())
			{
				// Get into the vehicle with the person if the vehicle is near enough.
				DaVehicle* vehicle = (DaVehicle*)person->Parent();
				if(IsAboardVehicle() && Parent() == vehicle)
					break;	// I'm already aboard the person's vehicle.
				bool closeEnough = false;
				if(person->IsInSpacecraft() && IsInSpacecraft() && person->FindParentVesselInEnv() == FindParentVesselInEnv())
				{
					// We're aboard the same spacecraft. That's good enough because troops get placed all over the ship.
					closeEnough = true;
				}
				else
				{
					// We're not aboard the same spacecraft so check physical distance to the vehicle.
					AuPoint vehicleLoc;
					vehicle->GetLocationRelativeToParent(vehicleLoc, system);
					AuPoint myLoc;
					GetLocationRelativeToParent(myLoc, system);
					if(vehicleLoc.Distance(myLoc) < 100)
						closeEnough = true;
				}
				if(closeEnough)
				{
					// Get into the vehicle.
					if(vehicle->BoardTroop(this))
					{
						QString text = tr("I'm in.");
						AuCommMessage myreply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
								msg.ReceptionRange(), text, -1, "", AuCommPacket::Chat);
						Say(msg);
					}
					else
					{
						QString text = tr("There are no empty seats in your vehicle.");
						AuCommMessage myreply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
								msg.ReceptionRange(), text, -1, "", AuCommPacket::Chat);
						Say(msg);
					}
				}
				else
				{
					QString text = tr("Your vehicle is too far away to board.");
					AuCommMessage myreply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
							msg.ReceptionRange(), text, -1, "", AuCommPacket::Chat);
					Say(msg);
				}
			}
			else if(person->IsInSpacecraft())
			{
				// Get into the spacecraft with the person if the spacecraft is near enough.
				DaVessel* ship = person->FindParentVesselInEnv();
				if(IsInSpacecraft() && FindParentVesselInEnv() == ship)
					break;	// I'm already aboard the spacecraft.
				AuBox fExt;
				ship->RoughExtents(fExt);
				AuPoint fmin, fmax;
				fExt.GetMinMax(fmin, fmax);
				double spotRad = fmin.Distance_2D(fmax) / 2 + 50;	// Radius to load troops around the ship.
				AuPoint shipLoc;
				ship->GetLocationRelativeToParent(shipLoc, system);
				AuPoint myLoc;
				GetLocationRelativeToParent(myLoc, system);
				if(shipLoc.Distance(myLoc) < spotRad)
				{
					// Get into the spacecraft.
					if(ship->BoardTroop(this))
					{
						QString text = tr("I'm aboard.");
						AuCommMessage myreply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
								msg.ReceptionRange(), text, -1, "", AuCommPacket::Chat);
						Say(msg);
					}
					else
					{
						QString text = tr("There is no room to board your spacecraft.");
						AuCommMessage myreply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
								msg.ReceptionRange(), text, -1, "", AuCommPacket::Chat);
						Say(msg);
					}
				}
				else
				{
					QString text = tr("Your spacecraft is too far away to board.");
					AuCommMessage myreply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
							msg.ReceptionRange(), text, -1, "", AuCommPacket::Chat);
					Say(msg);
				}
			}
		}
		break;
		*/

	default:
		break;
	}
}

void DaSentient::RxCitizenOrder(const AuCommCitizenOrder* msg, bool firstInGroup)
{
	if(!msg)
		return;

	// Troop ids are in the binary data.
	if(!msg->IsRecipient(Id()))
		return;	// The order is not directed to me.

	// Locate the sender.
	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;	// Not likely.
	const DaPerson* person = dynamic_cast<const DaPerson*>(env->FindIdRecursive(msg->FromId()));
	if(!person)
		return;	// Person was not found in this system?

	// Should I accept orders from the person?
	if(!UnitCanBeSelected(person, false))
		return;	// If they cannot select me, they cannot give me orders.

	// Extract order details.
	CITIZEN_ORDER order = msg->Order();
	AuId orderTarget;
	AuPoint orderLoc;
	if(order == ORDER_Stay ||
			order == ORDER_Sleep ||
			order == ORDER_SleepDayDream ||
			order == ORDER_SleepEat ||
			order == ORDER_SleepExercise ||
			order == ORDER_SleepGame ||
			order == ORDER_SleepLetter ||
			order == ORDER_SleepMovie ||
			order == ORDER_SleepMusic ||
			order == ORDER_SleepPine ||
			order == ORDER_SleepPhone ||
			order == ORDER_SleepRead ||
			order == ORDER_SleepTalk ||
			order == ORDER_SleepText ||
			order == ORDER_SleepTV ||
			order == ORDER_SleepWeb ||
			order == ORDER_DamageReport ||
			order == ORDER_SurveyScanPlanet ||
			order == ORDER_SurveyScanSystem ||
			order == ORDER_TacticalScan ||
			order == ORDER_BattleStation ||
			order == ORDER_CloseDoor ||
			order == ORDER_DropRider ||
			order == ORDER_GetOut ||
			order == ORDER_OpenDoor ||
			order == ORDER_EnterSurgery ||
			order == ORDER_TakeCommand ||
			order == ORDER_ReleaseCommand ||
			order == ORDER_GoToRoom ||
			order == ORDER_Garrison)
	{
		// With Stay and some other orders, the target and location are my parent atom and my location, respectively.
		orderTarget = ParentId();
		orderLoc = Location();
	}
	else
	{
		// All other orders pass the order target and location with the message.
		orderTarget = msg->OrderTarget();
		orderLoc = AuPoint(msg->OrderX(), msg->OrderY(), msg->OrderZ());

		// If the target location is close to where I am, switch to the Exact versions of these commands.
		if(order == ORDER_ComeHere || order == ORDER_GoToTarget)
		{
			DaAtom* system = FindEnvironmentAtom();
			if(system)
			{
				DaAtom* target = system->FindIdRecursive(orderTarget);
				if(target)
				{
					// Get my location relative to the solar system.
					AuPoint mySystemLoc = EnvLocation();

					// Get the target location relative to the solar system.
					// Target location is relative to the target atom.
					const AuMatrix4x4& tt = target->EnvTransform();
					AuPoint orderSystemLoc = tt.Project(orderLoc);
					if(mySystemLoc.Distance(orderSystemLoc) < MOVE_NEAR*3/2)
					{
						if(order == ORDER_ComeHere)
							order = ORDER_ComeHereExactly;
						else
							order = ORDER_GoToTargetExactly;
					}
				}
			}
		}
	}

	// Respond to the order.
	TakeOrder(*msg, order, orderTarget, orderLoc, firstInGroup, msg->OrderText());
}

void DaSentient::RxCitizenPosture(const AuCommCitizenPosture* msg, bool firstInGroup)
{
	if(!msg)
		return;

	// Troop ids are in the binary data.
	if(!msg->IsRecipient(Id()))
		return;	// The order is not directed to me.

	// Locate the sender.
	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;	// Not likely.
	const DaPerson* person = dynamic_cast<const DaPerson*>(env->FindIdRecursive(msg->FromId()));
	if(!person)
		return;	// Person was not found in this system?

	// Should I accept orders from the person?
	if(!UnitCanBeSelected(person, false))
		return;	// If they cannot select me, they cannot give me orders.

	// Extract posture details.
	CITIZEN_POSTURE posture = msg->Posture();

	// Respond to the posture change.
	TakePosture(*msg, posture, firstInGroup);
}

void DaSentient::RxBeamAtom(const AuCommBeamAtom* msg)
{
	//qDebug() << "DaSentient::RxBeamAtom" << Id() << Name();
	DaAtom* targAtom = 0;
	AuPoint targLoc, targZ;
	int resultCode = FindTransporterTarget(msg->CityId(), msg->Target(), targAtom, targLoc, targZ);
	if(resultCode || !targAtom)
	{
		// An error occurred.
		if(resultCode == 2)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Transport to enemy building blocked by shield generator on the ground.");
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
		}
		else if(resultCode == 3)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Transport to enemy city blocked by shield generator on the ground.");
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
		}
		else if(resultCode == 4)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Transport blocked by shields on the target vessel.");
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
		}
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	AuPointArray targPads;
	EnergizeTransporter(msg, isFrom, msg->Count(), targAtom, targLoc, targZ, targWhich, targPads);
}

void DaSentient::RxBeamVehicleAtom(const AuCommBeamVehicleAtom* msg)
{
	//qDebug() << "DaSentient::RxBeamVehicleAtom" << Id() << Name();
	DaAtom* targAtom = 0;
	AuPoint targLoc, targZ;
	int resultCode = FindTransporterTarget(msg->CityId(), msg->Target(), targAtom, targLoc, targZ);
	if(resultCode || !targAtom)
	{
		// An error occurred.
		if(resultCode == 2)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Vehicle transport to enemy building blocked by shield generator on the ground.");
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
		}
		else if(resultCode == 3)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Vehicle transport to enemy city blocked by shield generator on the ground.");
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
		}
		else if(resultCode == 4)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Vehicle transport blocked by shields on the target vessel.");
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
		}
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots);
}

void DaSentient::RxBeamCoordinate(const AuCommBeamCoordinate* msg)
{
	//qDebug() << "DaSentient::RxBeamCoordinate" << Id() << Name();
	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return;
	// Calculate location relative to the parent of the ship.
	DaAtom* targAtom = ship->Parent();
	AuPoint targLoc = ship->Transform().Project(msg->Coordinate());
	AuPoint targZ = ship->Transform().ProjectVector(AuPoint(0,0,1));
	targZ.Unit();

	// Also calculate X and Y vectors to find a random location nearby.
	AuPoint targX = AuPoint(0,0,1).Cross(targZ);
	targX.Unit();
	AuPoint targY = targZ.Cross(targX);
	targY.Unit();

	// Offset randomly from the target location.
	targLoc += targX * g_Au.Dice().Roll(1,41,-21) + targY * g_Au.Dice().Roll(1,41,-21);

	// Insure the target location is not inside the shield of a city.
	DaWorldLL* targWorld = dynamic_cast<DaWorldLL*>(targAtom);
	if(targWorld && targWorld->SubatomShieldsBlockTransporters(ship->EmpireId(), targLoc))
	{
		QString text = QObject::tr("Transport to coordinates blocked by shield generator on the ground.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	// Insure target location is not inside the hull of any spacecraft with shields up.
	AuPoint sysTargLoc = targAtom->EnvTransform().Project(targLoc);	// Calculate targLoc relative to solar system.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.
	DaAtomList ships;
	system->FindCapsulesRecursive(ships);
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaCapsule* otherShip = (DaCapsule*)(*si);
		if(otherShip->Is(ship->Id()))
			continue;					// Ignore the ship we are aboard. We can beam aboard it using coordinates.
		if(!otherShip->ShieldsWorkingAndUp())
			continue;					// Ignore ships without shields. We are not blocked from beaming aboard them.

		// Calculate the target location in the space of the other ship.
		AuPoint otherShipLoc = otherShip->EnvTransform().UnProject(sysTargLoc);
		if(otherShip->IsInOrOnHull(otherShipLoc) != 1)
			continue;					// Ignore ship if location is not inside its hull.

		// The other ship blocks the transporter destination because its shields are up.
		QString text = QObject::tr("Transporter blocked by spacecraft shields at destination.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	AuPointArray targPads;
	EnergizeTransporter(msg, isFrom, msg->Count(), targAtom, targLoc, targZ, targWhich, targPads);
}

void DaSentient::RxBeamPlanet(const AuCommBeamPlanet* msg)
{
	//qDebug() << "DaSentient::RxBeamPlanet" << Id() << Name();
	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return;
	// Locate the target planet.
	DaWorld* planet = FindParentPlanetInEnv();
	if(!planet)
	{
		// We are not orbiting a planet.
		QString text = QObject::tr("Unable to transport to planet coordinates. We are not orbiting a planet.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
	if(!llworld)
		return;

	// Insure the terrain patches exist at the planet location.
	DaAtom* targAtom = llworld;
	AuPoint targLoc = llworld->GetSurfaceLocation(AuGlobeLL(msg->Longitude(), msg->Latitude()));
	AuPoint targZ = -llworld->GravityDir(targLoc);

	if(llworld->SubatomShieldsBlockTransporters(ship->EmpireId(), targLoc))
	{
		QString text = QObject::tr("Transporter blocked by shield generator on the ground.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	AuPointArray targPads;
	EnergizeTransporter(msg, isFrom, msg->Count(), targAtom, targLoc, targZ, targWhich, targPads);
}

void DaSentient::RxBeamShip(const AuCommBeamShip* msg)
{
	//qDebug() << "DaSentient::RxBeamShip" << Id() << Name();
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.

	// Locate the ship that is the target of the order.
	DaCapsule* targAtom = dynamic_cast<DaCapsule*>(system->FindIdRecursive(msg->ShipId()));
	if(!targAtom)
		return;

	// Insure the person is permitted to beam aboard the other ship.
	bool okToBeam = false;
	{
		// Locate my ship.
		DaCapsule* ship = FindParentCapsuleInEnv();
		if(!ship)
			return;	// Not likely.
		// Locate the sender.
		const DaPerson* person = dynamic_cast<const DaPerson*>(ship->FindIdRecursive(msg->FromId()));
		if(!person)
			return;	// Person was not found in this system?
		if(targAtom->CheckAccess(person))
			okToBeam = true;	// Person is crew member, in chain of command, or an agent with powers active.
		else if(targAtom->IsSpaceStation())
		{
			// Stations will beam any non-enemy aboard if their doors are not locked.
			if(!targAtom->AnyHullDoorUnlocked() && PoliticalStance(targAtom->EmpireId(), ship->EmpireId()) != POL_Enemy)
				okToBeam = true;
		}
		else if(targAtom->IsCommandedByAI())
		{
			// Ship is not a station and the requestor is not a crew member or in CoC.
			// AI commander must decide whether to beam them aboard.
			// The sender is neutral, friend, or empire; we've already skipped messages from enemy senders.
			AuMissionOrder::TYPE t = targAtom->MissionOrderType();
			if(t == AuMissionOrder::CrewLoad ||
					t == AuMissionOrder::PassengersLoad)
				okToBeam = true;		// Ok when the mission order is to board passengers or crew.
			else if(t == AuMissionOrder::TroopsLoad)
			{
				// Determine stance toward message sender.
				POLITICAL_STANCE stanceToSender = targAtom->PoliticalStanceMakeContact(person);
				if(stanceToSender == POL_Empire || stanceToSender == POL_Vassal)
					okToBeam = true;	// Ok when the mission order is to board troops and the sender is my empire or overlord.
			}
		}

		if(!okToBeam && targAtom->ShieldsBlockTransportAboard())
		{
			// Shields will block the beam attempt when not permitted to make the transport.
			QString text = QObject::tr("Transporter blocked by shields on target vessel.");

			// Transporter operator announces the result at close range.
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
			return;
		}
	}

	AuPoint targLoc;
	if(!targAtom->TransporterVoidLocation(targLoc))
	{
		QString text = QObject::tr("Unable to lock onto a safe void inside the hull.");

		// Transporter operator announces the result at close range. This message is only processed for a person in the same room.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	AuPoint targZ(0,0,1);
	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	AuPointArray targPads;
	EnergizeTransporter(msg, isFrom, msg->Count(), targAtom, targLoc, targZ, targWhich, targPads);
}

void DaSentient::RxBeamTransporter(const AuCommBeamTransporter* msg)
{
	//qDebug() << "DaSentient::RxBeamTransporter";
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.

	// Locate the ship that is the target of the order.
	DaCapsule* targAtom = dynamic_cast<DaCapsule*>(system->FindIdRecursive(msg->ShipId()));
	if(!targAtom)
		return;

	// Insure the person is permitted to beam aboard the other ship.
	bool okToBeam = false;
	{
		// Locate my ship.
		DaCapsule* ship = FindParentCapsuleInEnv();
		if(!ship)
			return;	// Not likely.
		// Locate the sender.
		const DaPerson* person = dynamic_cast<const DaPerson*>(ship->FindIdRecursive(msg->FromId()));
		if(!person)
			return;	// Person was not found in this system?
		if(targAtom->CheckAccess(person))
			okToBeam = true;	// Person is crew member, in chain of command, or an agent with powers active.
		else if(targAtom->IsSpaceStation())
		{
			// Stations will beam any non-enemy aboard if their doors are not locked.
			if(!targAtom->AnyHullDoorUnlocked() && PoliticalStance(targAtom->EmpireId(), ship->EmpireId()) != POL_Enemy)
				okToBeam = true;
		}
		else if(targAtom->IsCommandedByAI())
		{
			// Ship is not a station and the requestor is not a crew member or in CoC.
			// AI commander must decide whether to beam them aboard.
			// The sender is neutral, friend, or empire; we've already skipped messages from enemy senders.
			AuMissionOrder::TYPE t = targAtom->MissionOrderType();
			if(t == AuMissionOrder::CrewLoad ||
					t == AuMissionOrder::PassengersLoad)
				okToBeam = true;		// Ok when the mission order is to board passengers or crew.
			else if(t == AuMissionOrder::TroopsLoad)
			{
				// Determine stance toward message sender.
				POLITICAL_STANCE stanceToSender = targAtom->PoliticalStanceMakeContact(person);
				if(stanceToSender == POL_Empire || stanceToSender == POL_Vassal)
					okToBeam = true;	// Ok when the mission order is to board troops and the sender is my empire or overlord.
			}
		}

		if(!okToBeam && targAtom->ShieldsBlockTransportAboard())
		{
			// Shields will block the beam attempt when not permitted to make the transport.
			QString text = QObject::tr("Transporter blocked by shields on target vessel.");

			// Transporter operator announces the result at close range.
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
			return;
		}
	}

	// When permitted to beam the person, the destination transporter room need not already be on.
	// When not permitted to beam the person, only look for transporter rooms that are on and inactive.
	int padLimit = msg->Count();
	quint16 targWhich = msg->SystemWhich();	//targAtom->TransporterSystemId(padLimit, !okToBeam);
	if(targWhich == 0xffff)
	{
		QString text = QObject::tr("Target vessel has no transporter available.");

		// Transporter operator announces the result at close range. This message is only processed for a person in the same room.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	AuPointArray targPads;
	targAtom->TransporterPadLocations(targWhich, targPads);
	if(padLimit > targPads.count())
		padLimit = targPads.count();
	if(!padLimit)
	{
		QString text = QObject::tr("Target transporter has no functioning pads.");

		// Transporter operator announces the result at close range. This message is only processed for a person in the same room.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	AuPoint targLoc;
	AuPoint targZ(0,0,1);
	bool isFrom = msg->BeamDirection() == 0;
	EnergizeTransporter(msg, isFrom, padLimit, targAtom, targLoc, targZ, targWhich, targPads);
}

void DaSentient::RxBeamVehicleCoordinate(const AuCommBeamVehicleCoordinate* msg)
{
	//qDebug() << "DaSentient::RxBeamVehicleCoordinate" << Id() << Name();
	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return;
	// Calculate location relative to the parent of the ship.
	DaAtom* targAtom = ship->Parent();
	AuPoint targLoc = ship->Transform().Project(msg->Coordinate());
	AuPoint targZ = ship->Transform().ProjectVector(AuPoint(0,0,1));
	targZ.Unit();

	// Also calculate X and Y vectors to find a random location nearby.
	AuPoint targX = AuPoint(0,0,1).Cross(targZ);
	targX.Unit();
	AuPoint targY = targZ.Cross(targX);
	targY.Unit();

	// Offset randomly from the target location.
	targLoc += targX * g_Au.Dice().Roll(1,41,-21) + targY * g_Au.Dice().Roll(1,41,-21);

	// Insure the target location is not inside the shield of a city.
	DaWorldLL* targWorld = dynamic_cast<DaWorldLL*>(targAtom);
	if(targWorld && targWorld->SubatomShieldsBlockTransporters(ship->EmpireId(), targLoc))
	{
		QString text = QObject::tr("Vehicle transport to coordinates blocked by shield generator on the ground.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	// Insure target location is not inside the hull of any spacecraft with shields up.
	AuPoint sysTargLoc = targAtom->EnvTransform().Project(targLoc);	// Calculate targLoc relative to solar system.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.
	DaAtomList ships;
	system->FindCapsulesRecursive(ships);
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaCapsule* otherShip = (DaCapsule*)(*si);
		if(otherShip->Is(ship->Id()))
			continue;					// Ignore the ship we are aboard. We can beam aboard it using coordinates.
		if(!otherShip->ShieldsWorkingAndUp())
			continue;					// Ignore ships without shields. We are not blocked from beaming aboard them.

		// Calculate the target location in the space of the other ship.
		AuPoint otherShipLoc = otherShip->EnvTransform().UnProject(sysTargLoc);
		if(otherShip->IsInOrOnHull(otherShipLoc) != 1)
			continue;					// Ignore ship if location is not inside its hull.

		// The other ship blocks the transporter destination because its shields are up.
		QString text = QObject::tr("Vehicle transporter blocked by spacecraft shields at destination.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots);
}

void DaSentient::RxBeamVehiclePlanet(const AuCommBeamVehiclePlanet* msg)
{
	//qDebug() << "DaSentient::RxBeamVehiclePlanet" << Id() << Name();
	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return;
	// Locate the target planet.
	DaWorld* planet = FindParentPlanetInEnv();
	if(!planet)
	{
		// We are not orbiting a planet.
		QString text = QObject::tr("Unable to transport vehicle to planet coordinates. We are not orbiting a planet.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
	if(!llworld)
		return;

	// Insure the terrain patches exist at the planet location.
	DaAtom* targAtom = llworld;
	AuPoint targLoc = llworld->GetSurfaceLocation(AuGlobeLL(msg->Longitude(), msg->Latitude()));
	AuPoint targZ = -llworld->GravityDir(targLoc);

	if(llworld->SubatomShieldsBlockTransporters(ship->EmpireId(), targLoc))
	{
		QString text = QObject::tr("Vehicle transporter blocked by shield generator on the ground.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots);
}

void DaSentient::RxBeamVehicleShip(const AuCommBeamVehicleShip* msg)
{
	//qDebug() << "DaSentient::RxBeamVehicleShip" << Id() << Name();
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.

	// Locate the ship that is the target of the order.
	DaCapsule* targAtom = dynamic_cast<DaCapsule*>(system->FindIdRecursive(msg->ShipId()));
	if(!targAtom)
		return;

	// Insure the person is permitted to beam aboard the other ship.
	bool okToBeam = false;
	{
		// Locate my ship.
		DaCapsule* ship = FindParentCapsuleInEnv();
		if(!ship)
			return;	// Not likely.
		// Locate the sender.
		const DaPerson* person = dynamic_cast<const DaPerson*>(ship->FindIdRecursive(msg->FromId()));
		if(!person)
			return;	// Person was not found in this system?
		if(targAtom->CheckAccess(person))
			okToBeam = true;	// Person is crew member, in chain of command, or an agent with powers active.
		else if(targAtom->IsSpaceStation())
		{
			// Stations will beam any non-enemy aboard if their doors are not locked.
			if(!targAtom->AnyHullDoorUnlocked() && PoliticalStance(targAtom->EmpireId(), ship->EmpireId()) != POL_Enemy)
				okToBeam = true;
		}
		else if(targAtom->IsCommandedByAI())
		{
			// Ship is not a station and the requestor is not a crew member or in CoC.
			// AI commander must decide whether to beam them aboard.
			// The sender is neutral, friend, or empire; we've already skipped messages from enemy senders.
			AuMissionOrder::TYPE t = targAtom->MissionOrderType();
			if(t == AuMissionOrder::CrewLoad ||
					t == AuMissionOrder::PassengersLoad)
				okToBeam = true;		// Ok when the mission order is to board passengers or crew.
			else if(t == AuMissionOrder::TroopsLoad)
			{
				// Determine stance toward message sender.
				POLITICAL_STANCE stanceToSender = targAtom->PoliticalStanceMakeContact(person);
				if(stanceToSender == POL_Empire || stanceToSender == POL_Vassal)
					okToBeam = true;	// Ok when the mission order is to board troops and the sender is my empire or overlord.
			}
		}

		if(!okToBeam && targAtom->ShieldsBlockTransportAboard())
		{
			// Shields will block the beam attempt when not permitted to make the transport.
			QString text = QObject::tr("Vehicle transporter blocked by shields on target vessel.");

			// Transporter operator announces the result at close range.
			AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Hail, msg->Scope());
			Say(msgResult, AuVisage::Perplexed);
			return;
		}
	}

	AuPoint targLoc;
	if(!targAtom->TransporterVoidLocation(targLoc))
	{
		QString text = QObject::tr("Unable to lock onto a safe void inside the hull.");

		// Transporter operator announces the result at close range. This message is only processed for a person in the same room.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	AuPoint targZ(0,0,1);
	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots);
}

void DaSentient::RxBeamVehicleTransporter(const AuCommBeamVehicleTransporter* msg)
{
	//qDebug() << "DaSentient::RxBeamVehicleTransporter";
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.

	// Locate the ship that is the target of the order.
	DaCapsule* targAtom = dynamic_cast<DaCapsule*>(system->FindIdRecursive(msg->ShipId()));
	if(!targAtom)
		return;

	// Insure the person is permitted to beam aboard the other ship.
	bool okToBeam = false;
	// Locate my ship.
	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return;	// Not likely.
	// Locate the sender.
	const DaPerson* person = dynamic_cast<const DaPerson*>(ship->FindIdRecursive(msg->FromId()));
	if(!person)
		return;	// Person was not found in this system?
	const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person->Parent());
	if(!vehicle)
		return;
	VEHICLE_SPOT_TYPE spotType = vehicle->ParkingSpotType();
	if(targAtom->CheckAccess(person))
		okToBeam = true;	// Person is crew member, in chain of command, or an agent with powers active.
	else if(targAtom->IsSpaceStation())
	{
		// Stations will beam any non-enemy aboard if their doors are not locked.
		if(!targAtom->AnyHullDoorUnlocked() && PoliticalStance(targAtom->EmpireId(), ship->EmpireId()) != POL_Enemy)
			okToBeam = true;
	}
	else if(targAtom->IsCommandedByAI())
	{
		// Ship is not a station and the requestor is not a crew member or in CoC.
		// AI commander must decide whether to beam them aboard.
		// The sender is neutral, friend, or empire; we've already skipped messages from enemy senders.
		AuMissionOrder::TYPE t = targAtom->MissionOrderType();
		if(t == AuMissionOrder::CrewLoad ||
				t == AuMissionOrder::PassengersLoad)
			okToBeam = true;		// Ok when the mission order is to board passengers or crew.
		else if(t == AuMissionOrder::TroopsLoad)
		{
			// Determine stance toward message sender.
			POLITICAL_STANCE stanceToSender = targAtom->PoliticalStanceMakeContact(person);
			if(stanceToSender == POL_Empire || stanceToSender == POL_Vassal)
				okToBeam = true;	// Ok when the mission order is to board troops and the sender is my empire or overlord.
		}
	}

	if(!okToBeam && targAtom->ShieldsBlockTransportAboard())
	{
		// Shields will block the beam attempt when not permitted to make the transport.
		QString text = QObject::tr("Vehicle transporter blocked by shields on target vessel.");

		// Transporter operator announces the result at close range.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	// When permitted to beam the person, the destination transporter room need not already be on.
	// When not permitted to beam the person, only look for transporter rooms that are on and inactive.
	quint16 targWhich = msg->SystemWhich();	//targAtom->VehicleTransporterSystemId(spotType, !okToBeam);
	if(targWhich == 0xffff)
	{
		QString text = QObject::tr("Target vessel has no vehicle transporter available.");

		// Transporter operator announces the result at close range. This message is only processed for a person in the same room.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}

	AuMatrix4x4 spotM;
	DaVehicle* parked = 0;
	if(!targAtom->VehicleTransporterSpot(spotType, spotM, parked))
	{
		QString text = QObject::tr("Target vehicle transporter has no vacant parking spots.");

		// Transporter operator announces the result at close range. This message is only processed for a person in the same room.
		AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Hail, msg->Scope());
		Say(msgResult, AuVisage::Perplexed);
		return;
	}
	QList<AuMatrix4x4> targParkingSpots;
	targParkingSpots.append(spotM);

	AuPoint targLoc;
	AuPoint targZ(0,0,1);
	bool isFrom = msg->BeamDirection() == 0;
	if(EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots))
	{
		// Transport was successful.
		// Eliminate the parked vehicle.
		if(parked && parked->IsNormal() && !parked->HasSubatoms())
			parked->Kill();
	}
}

void DaSentient::RxChat(const AuCommChat* msg)
{
	if(msg->Text().toUpper().left(5) != "HELLO" && !ContainsEuphemismForSex(msg->Text()))
		return;	// They didn't say "Hello" and they don't want to have sex. What else is there?
	if(msg->FromId() != Interlocutor())
		RxHailRequest(msg);
	else
		RxHailReply(msg);
}

//static
bool DaSentient::ContainsEuphemismForSex(const QString& text)
{
	// Nerf solo sex.
	if(!g_Au.IsMatingOk())
		return false;

	// Look for certain key phrases to indicate a desire for sex.
	// Phrases are not case sensitive.
	QString euphemisms[] =
	{
		QString("Fuck"),
		QString("Afternoon Delight"),
		QString("Amorous Congress"),
		QString("Have Sex"),
		QString("Make Love"),
		QString("Make a Baby"),
		QString("Make whoopee"),
		QString("Marital Congress"),
		QString("Sexual Congress"),
		QString("boink"),
		QString("bump uglies"),
		QString("bury the weasel"),
		QString("butter the biscuit"),
		QString("do the dirty deed"),
		QString("doink"),
		QString("forbidden polka"),
		QString("get laid"),
		QString("go all the way"),
		QString("hanky panky"),
		QString("shagg"),
		QString("shtupp"),
		QString("stuff the muffin"),
		QString("stuff the taco"),
		QString("tube snake boogie"),
		QString("tube-snake boogie"),
		QString("tubesnake boogie"),
		QString("wet the willy"),
		QString("yentzing"),
		QString("yiffing"),
	};
	int eucount = sizeof(euphemisms)/sizeof(QString);
	for(int eu=0; eu<eucount; eu++)
		if(text.indexOf(euphemisms[eu], 0, Qt::CaseInsensitive) >= 0)
			return true;
	return false;
}

void DaSentient::RxHailRequest(const AuCommPacket* msg)
{
	// Do not talk when I am unconscious.
	if(IsBodyDyingOrDead())
		return;

	if(!IsAIControlled())
		return;

	if(!AuCommStdChannel::IsVoiceChannel(msg->ChannelName()))
		return;

	DaAtom* env = FindEnvironmentAtom();
	DaPerson* person = env ? dynamic_cast<DaPerson*>(env->FindIdRecursive(msg->FromId())) : 0;
	if(!person || !InConeOfAddress(*person))
		return;

	// Wake up from my nap, if I am sleeping.
	WakeUp();

	if(PoliticalStance(EmpireId(), msg->FromEmpireId()) == POL_Enemy)
		return;

	// Select a job offer for the person before I say anything.
	SelectJobOffer(*person);

	// Say hello, if this is the beginning of our conversation.
	if(msg->FromId() != Interlocutor())
	{
		QString text;
		if(msg->FromId() == Father())
			text = tr("Hello Dad.");
		else if(msg->FromId() == Mother())
			text = tr("Hello Mom.");
		else
			text = tr("Hello %1.").arg(msg->FromName());

		QDateTime dt = g_Au.CurrentUTC();
		if(dt.date().month() == 12 && (dt.date().day() == 24 || dt.date().day() == 25))
			text += tr(" Merry Christmas.");
		else if(dt.date().month() == 1 && dt.date().day() == 1)
			text += tr(" Happy New Year.");

		AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
				msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Smile, msg->FromId());

		if(!IsMarried() && TryMating(*person))
		{
			// Person IsReadyToMate, properly posed, and IsSexuallyCompatible.
			QString text = tr("I will be a good %2, %1.")
				.arg(person->Name())
				.arg(MotherFather(false));
			AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), msg->FromSectorId(), Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Grin, person->Id());
			DaSentient::TakeOrder(rmsg, ORDER_MountTarget, person->Id(), AuPoint(0,0), true,
					tr("Have sex with %1.").arg(person->Name()));
			return;
		}
	}

	// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
	SayJobOffer(*person);
}

void DaSentient::RxHailReply(const AuCommPacket* msg)
{
	if(!IsAIControlled())
		return;

	// Do not talk when I am unconscious.
	if(IsBodyDyingOrDead())
		return;

	if(!AuCommStdChannel::IsVoiceChannel(msg->ChannelName()))
		return;

	if(msg->FromId() != Interlocutor())
		return;

	DaAtom* env = FindEnvironmentAtom();
	DaPerson* person = env ? dynamic_cast<DaPerson*>(env->FindIdRecursive(msg->FromId())) : 0;
	if(!person || !InConeOfAddress(*person))
		return;

	// Wake up from my nap, if I am sleeping.
	WakeUp();
	if(msg->FromEmpireId() != 0 && PoliticalStance(EmpireId(), msg->FromEmpireId()) == POL_Enemy)
		return;

	// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
	SayJobOffer(*person);
}

void DaSentient::SayJobOffer(DaPerson& person)
{
	if(SelectJobOffer(person))
	{
		// I have a job offer for the person.
		m_TalkLock.lockForRead();
		QString text = m_JobOffer.Teaser();
		m_TalkLock.unlock();

		if(!text.isEmpty())
		{
			AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, person.Id());

			AuCommChatChoice nomsg(person.Id(), tr("I'm not interested."), 1, 2, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(nomsg, person.Id());

			AuCommChatChoice continuemsg(person.Id(), tr("What else is new?"), 1, 4, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(continuemsg, person.Id());

			AuCommChatChoice okmsg(person.Id(), tr("Tell me more."), 1, 0, AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(okmsg, AuVisage::Closed, person.Id());
		}
	}
	else
	{
		// I do not have a job offer for the person.
		SayPleasantry(person);
	}
}

// static
QString DaSentient::QuotableQuote()
{
	// Spock quotes
	QString spockQs[] =
	{
		tr("Spock said, 'May I say that I have not thoroughly enjoyed serving with humans? I find their illogic and "
				"foolish emotions a constant irritant.'"),
		tr("Spock said, 'Computers make excellent and efficient servants, but I have no wish to serve under them.'"),
		tr("Spock said, 'Insufficient facts always invite danger.'"),
		tr("Spock said, 'In critical moments, men sometimes see exactly what they wish to see.'"),
		tr("Spock said, 'After a time, you may find that having is not so pleasing a thing after all as wanting. It is "
				"not logical, but is often true.'"),
		tr("Spock said, 'Without followers, evil cannot spread.'"),
		tr("Spock said, 'When you eliminate the impossible, whatever remains, however improbable, must be the truth.'"),
		tr("Spock said, 'I could not deprive you of the revelation of all that you could accomplish together, of a "
				"friendship that will define you both in ways you cannot yet realize.'"),
		tr("Spock said, 'Live long and prosper.'"),
		tr("Spock said, 'The needs of the many outweigh the needs of the few.'"),
	};
	int spockQcount = sizeof(spockQs)/sizeof(QString);

	// Yoda quotes.
	QString yodaQs[] =
	{
		tr("Yoda said, 'Train yourself to let go of everything you fear to lose.'"),
		tr("Yoda said, 'Fear is the path to the dark side. Fear leads to anger. Anger leads to hate. "
				"Hate leads to suffering.'"),
		tr("Yoda said, 'Death is a natural part of life. Rejoice for those around you who transform into the Force. "
				"Mourn them do not. Miss them do not. Attachment leads to jealosy. The shadow of greed, that is.'"),
		tr("Yoda said, 'Always pass on what you have learned.'"),
		tr("Yoda said, 'You will know (the good from the bad) when you are calm, at peace. Passive. "
				"A Jedi uses the Force for knowledge and defense, never for attack.'"),
		tr("Yoda said, 'Yes, a Jedi's strength flows from the Force. But beware of the dark side. Anger, fear, aggression; "
				"the dark side of the Force are they. Easily they flow, quick to join you in a fight. If once you start down "
				"the dark path, forever will it dominate your destiny, consume you it will, as it did Obi-Wan's "
				"apprentice.'"),
		tr("Yoda said, 'Powerful you have become, the dark side I sense in you.'"),
		tr("Yoda said, 'If you end your training now--if you choose the quick and easy path as Vader did--you will "
				"become an agent of evil.'"),
		tr("Yoda said, 'Patience you must have my young padawan.'"),
	};
	int yodaQcount = sizeof(yodaQs)/sizeof(QString);

	// Bob Stedman quotes.
	QString bobQs[] =
	{
		tr("When the houses get bigger, pull back on the stick."),
		tr("A good landing is any landing where everyone walks away. It's a great landing when you can reuse the spacecraft."),
		tr("When flying small aircraft, if you have to go down at night, turn the landing light on. If you don't like what you "
				"see, turn it off."),
		tr("When you lose an engine while flying a twin engine aircraft, the other engine is guaranteed to get you to the crash "
				"site."),
		tr("You know you've landed with the landing gear up when it takes full power to taxi."),
	};
	int bobQcount = sizeof(bobQs)/sizeof(QString);

	// Lazarus Long quotes.
	QString lazarusQs[] =
	{
		tr("Lazarus Long said, 'It is a truism that almost any sect, cult, or religion will legislate its creed into law "
				"if it acquires the political power to do so, and will follow it by suppressing opposition, subverting all "
				"education to seize early the minds of the young, and by killing, locking up, or driving underground all "
				"heretics.'"),
		tr("Lazarus Long said, 'Always store beer in a dark place.'"),
		tr("Lazarus Long said, 'By the data to date, there is only one animal in the Galaxy dangerous to man--man "
				"himself. So he must supply his own indispensable competition. He has no enemy to help him.'"),
		tr("Lazarus Long said, 'Men are more sentimental than women. It blurs their thinking.'"),
		tr("Lazarus Long said, 'Certainly the game is rigged. Don’t let that stop you; if you don’t bet, you can’t win.'"),
		tr("Lazarus Long said, 'Any priest or shaman must be presumed guilty until proved innocent.'"),
		tr("Lazarus Long said, 'Always listen to experts. They’ll tell you what can’t be done and why. Then do it!'"),
		tr("Lazarus Long said, 'Get a shot off fast. This upsets him long enough to let you make your second shot perfect.'"),
		tr("Lazarus Long said, 'There is no conclusive evidence of life after death. But there is no evidence of any sort against it. Soon enough you will know. So why fret about it?'"),
		tr("Lazarus Long said, 'If it can’t be expressed in figures, it is not science; it is opinion.'"),
		tr("Lazarus Long said, 'It has long been known that one horse can run faster than another--but which one? Differences are crucial.'"),
		tr("Lazarus Long said, 'A fake fortune teller can be tolerated. But an authentic soothsayer should be shot on sight. Cassandra did not get half the kicking around she deserved.'"),
		tr("Lazarus Long said, 'Delusions are often functional. A mother’s opinions about her children’s beauty, intelligence, goodness, et cetera ad nauseam, keep her from drowning them at birth.'"),
		tr("Lazarus Long said, 'Most “scientists” are bottle washers and button sorters.'"),
		tr("Lazarus Long said, 'A “pacifist male” is a contradiction in terms. Most self-described “pacifists” are not pacific; they simply assume false colors. When the wind changes, they hoist the Jolly Roger.'"),
		tr("Lazarus Long said, 'Nursing does not diminish the beauty of a woman’s breasts; it enhances their charm by making them look lived in and happy.'"),
		tr("Lazarus Long said, 'A generation which ignores history has no past—and no future.'"),
		tr("Lazarus Long said, 'A poet who reads his verse in public may have other nasty habits.'"),
		tr("Lazarus Long said, 'What a wonderful world it is that has girls in!'"),
		tr("Lazarus Long said, 'Small change can often be found under seat cushions.'"),
		tr("Lazarus Long said, 'History does not record anywhere at any time a religion that has any rational basis. Religion is a crutch for people not strong enough to stand up to the unknown without help. But, like dandruff, most people do have a religion and spend time and money on it and seem to derive considerable pleasure from fiddling with it.'"),
		tr("Lazarus Long said, 'It’s amazing how much “mature wisdom” resembles being too tired.'"),
		tr("Lazarus Long said, 'If you don’t like yourself, you can’t like other people.'"),
		tr("Lazarus Long said, 'Your enemy is never a villain in his own eyes. Keep this in mind; it may offer a way to make him your friend. If not, you can kill him without hate--and quickly.'"),
		tr("Lazarus Long said, 'A motion to adjourn is always in order.'"),
		tr("Lazarus Long said, 'No state has an inherent right to survive through conscript troops and, in the long run, no state ever has. Roman matrons used to say to their sons: “Come back with your shield, or on it.” Later on this custom declined. So did Rome.'"),
		tr("Lazarus Long said, 'Of all the strange “crimes that human beings have legislated out of nothing, “blasphemy” is the most amazing--with “obscenity” and “indecent exposure” fighting it out for second and third place.'"),
		tr("Lazarus Long said, 'Cheops’ Law: Nothing is ever built on schedule or within budget.'"),
		tr("Lazarus Long said, 'It is better to copulate than never.'"),
		tr("Lazarus Long said, 'All societies are based on rules to protect pregnant women and young children. All else is surplus age, excrescence, adornment, luxury or folly which can--and must--be dumped in emergency to preserve this prime function. As racial survival is the only universal morality, no other basic is possible. Attempts to formulate a “perfect society” on any foundation other than “women and children first!” is not only witless, it is automatically genocidal. Nevertheless, starry-eyed idealists (all of them male) have tried endlessly--and no doubt will keep on trying.'"),
		tr("Lazarus Long said, 'All men are created unequal.'"),
		tr("Lazarus Long said, 'Money is a powerful aphrodisiac. But flowers work almost as well.'"),
		tr("Lazarus Long said, 'A brute kills for pleasure. A fool kills from hate.'"),
		tr("Lazarus Long said, 'There is only one way to console a widow. But remember the risk.'"),
		tr("Lazarus Long said, 'When the need arises--and it does--you must be able to shoot your own dog. Don’t farm it out--that doesn’t make it nicer, it makes it worse.'"),
		tr("Lazarus Long said, 'Everything in excess! To enjoy the flavor of life, take big bites. Moderation is for monks.'"),
		tr("Lazarus Long said, 'It may be better to be a live jackal than a dead lion, but it is better still to be a live lion. And usually easier.'"),
		tr("Lazarus Long said, 'One man’s theology is another man’s belly laugh.'"),
		tr("Lazarus Long said, 'Sex should be friendly. Otherwise stick to mechanical toys; it’s more sanitary.'"),
		tr("Lazarus Long said, 'Men rarely (if ever) manage to dream up a God superior to themselves. Most Gods have the manners and morals of a spoiled child.'"),
		tr("Lazarus Long said, 'Never appeal to a man’s “better nature.” He may not have one. Invoking his “self—interest” gives you more leverage.'"),
		tr("Lazarus Long said, 'Little girls, like butterflies, need no excuse.'"),
		tr("Lazarus Long said, 'You can have peace. Or you can have freedom. Don’t ever count on having both at once.'"),
		tr("Lazarus Long said, 'Avoid making irrevocable decisions while tired or hungry. N.B.: Circumstances can force your hand. So think ahead!'"),
		tr("Lazarus Long said, 'Place your clothes and weapons where you can find them in the dark.'"),
		tr("Lazarus Long said, 'An elephant: A mouse built to government specifications.'"),
		tr("Lazarus Long said, 'Throughout history, poverty is the normal condition of man. Advances which permit this norm to be exceeded--here and there, now and then--are the work of an extremely small minority, frequently despised, often condemned, and almost always opposed by all right-thinking people. Whenever this tiny minority is kept from creating, or (as sometimes happens) is driven out of a society, the people then slip back into abject poverty. This is known as “bad luck.”'"),
		tr("Lazarus Long said, 'In a mature society, “civil servant” is semantically equal to “civil master.”'"),
		tr("Lazarus Long said, 'When a place gets crowded enough to require ID’s, social collapse is not far away. It is time to go elsewhere. The best thing about space travel is that it made it possible to go elsewhere.'"),
		tr("Lazarus Long said, 'A woman is not property, and husbands who think otherwise are living in a dreamworld.'"),
		tr("Lazarus Long said, 'The second best thing about space travel is that the distances involved make war very difficult, usually impractical, and almost always unnecessary. This is probably a loss for most people, since war is our races most popular diversion, one which gives purpose and color to dull and stupid lives. But it is a great boon to the intelligent man who fights only when he must--never for sport.'"),
		tr("Lazarus Long said, 'A zygote is a gamete’s way of producing more gamete’s. This may be the purpose of the universe.'"),
		tr("Lazarus Long said, 'There are hidden contradictions within the minds of people who “love nature” while deploring the “artificialities” with which “Man has spoiled ‘Nature.” The obvious contradiction lies in their choice of words, which imply that Man and his artifacts are not part of “Nature”--but beavers and their damns are. But the contradictions go deeper than this prima-facie absurdity. In declaring his love for a beaver damn (erected by beavers for beaver’s purposes) and his hatred for dams erected by men (for the purpose of men) the “Naturist” reveals his hatred for his own race--i.e., his own self-hatred. In the case of “Naturists” such self—hatred is understandable; they are such a sorry lot. But hatred is too strong an emotion to feel toward them; pity and contempt are the most they rate. As for me, willy-nilly I am a man, not a beaver, and H. Sapiens is the only race I have or can have. Fortunately for me, I like being part of a race made up of men and women-- it strikes me as a fine arrangement and perfectly “natural.” Believe it or not, there were “Naturists” who opposed the first flight to old Earth’s Moon as being “unnatural” and a “despoiling of nature.”'"),
		tr("Lazarus Long said, 'No man is an island--” Much as we may feel and act as individuals, our race is a single organism, always growing and branching--which must be pruned regularly to be healthy. This necessity need not be argued; anyone with eyes can see that any organism which grows without limit always dies within •its own poisons. The only rational question is whether pruning is best done before or after birth. Being an incurable sentimentalist I favor the former of these methods--killing makes me queasy, even when it’s a case of “He’s dead and I’m alive and that’s the way I wanted it to be.” But this may be a matter of taste. Some shamans think that it is better to be killed in a war, or to die in childbirth, or to starve in misery, than never to have lived at all. They may be right. But I don’t have to like it--and I don’t.'"),
		tr("Lazarus Long said, 'Democracy is based on the assumption that a million men are wiser than one man. How’s that again? I missed something.'"),
		tr("Lazarus Long said, 'Autocracy is based on the assumption that one man is wiser than a million men. Let’s play that over again, too. Who decides?'"),
		tr("Lazarus Long said, 'Any government will work if authority and responsibility are equal and coordinate. This does not insure “good” government; it simply insures that it will work. But such governments are rare--most people want to run things but want no part of the blame. This used to be called the “backseat-driver syndrome.”'"),
		tr("Lazarus Long said, 'What are the facts? Again and again and again-what are the facts? Shun wishful thinking, ignore divine revelation, forget what “the stars foretell,” avoid opinion, care not what the neighbors think, never mind the unguessable “verdict of history”--what are the facts, and to how many decimal places? You pilot always into an unknown future; facts are your single clue. Get the facts!'"),
		tr("Lazarus Long said, 'God is omnipotent, omniscient, and omnibenevolent-it says so right here on the label. If you have a mind capable of believing all three of these divine attributes simultaneously, I have a wonderful bargain for you. No checks, please. Cash and in small bills.'"),
		tr("Lazarus Long said, 'Courage is the compliment of fear. A man who is fearless cannot be courageous. (He is also a fool.)'"),
		tr("Lazarus Long said, 'The two highest achievements of the human mind are the twin concepts of “loyalty” and “duty.” Whenever these twin concepts fall into disrepute--get out of there fast. You may possibly save yourself, but it is too late to save that society. It is doomed.'"),
		tr("Lazarus Long said, 'People who go broke in a big way never miss any meals. It is the poor jerk who is shy half a slug who must tighten his belt.'"),
		tr("Lazarus Long said, 'The truth of a proposition has nothing to do with it’s credibility. And vice versa.'"),
		tr("Lazarus Long said, 'Anyone who cannot cope with mathematics is not fully human. At best he is a tolerable sub-human who has learned to wear shoes, bathe, and not make messes in the house.'"),
		tr("Lazarus Long said, 'Moving parts in rubbing contact require lubrication to avoid excessive wear. Honorifics and formal politeness provide lubrication where people rub together. Often the very young, the untraveled, the naive, the unsophisticated deplore these formalities as “empty,” “meaningless,” or “dishonest,” and scorn to use them. No matter how “pure” their motives, they thereby throw sand into machinery that does not work too well at best.'"),
		tr("Lazarus Long said, 'A human being should be able to change a diaper, plan an invasion, butcher a hog, conn a ship, design a building, write a sonnet, balance accounts, build a wall, set a bone, comfort the dying, take orders, give orders, cooperate, act alone, solve equations, analyze a new problem, pitch manure, program a computer, cook a tasty meal, fight efficiently, die gallantly. Specialization is for insects!'"),
		tr("Lazarus Long said, 'The more you love, the more you can love--and the more intensely you love. Nor is there any limit on how many you can love. If a person had Time Enough, he could Love all of the majority who are decent and just.'"),
		//tr("Lazarus Long said, 'Masturbation is cheap, clean, convenient, and free of any possibility of wrong-doing--and you don’t have to go home in the cold. But it’s lonely.'"),
		tr("Lazarus Long said, 'Beware of altrusim. It is based on self-deception, the root of all evil.'"),
		tr("Lazarus Long said, 'If tempted by something that feels “altruistic” examine your motives and root out that self-deception. Then, if you still want to do it, wallow in it!'"),
		tr("Lazarus Long said, 'The most preposterous notion that H. Sapiens has ever dreamed up is that the Lord God of Creation, Shaper and Ruler of all the Universes, wants the saccharine adoration of His creatures, can be swayed by their prayers, and becomes petulant if He does not receive this flattery. Yet this absurd fantasy, without a shred of evidence to bolster it, pays all the expenses of the oldest, largest, and least productive industry in all of history.'"),
		tr("Lazarus Long said, 'The second most preposterous notion is that copulation is inherently sinful.'"),
		tr("Lazarus Long said, 'Writing is not necessarily something to be ashamed of--but do it in private and wash your hands afterwards.'"),
		tr("Lazarus Long said, '$100 placed at 7% interest compounded quarterly for 200 years will increase to more ‘than $100,000,000--by which time it will be worth nothing.'"),
		tr("Lazarus Long said, 'Dear, don’t bore him with trivia or burden him with your past mistakes. The happiest way to deal with a man is never to tell him anything he does not need to know.'"),
		tr("Lazarus Long said, 'Darling, a true lady takes off her dignity with her clothes and does her whorish best. At other times you can be as modest and dignified as your persona requires.'"),
		tr("Lazarus Long said, 'Everybody lies about sex.'"),
		tr("Lazarus Long said, 'If men were the automatons that behaviorists claim they are, the behaviorist psychologists could not have invented the amazing nonsense called “behaviorist psychology.” So they are wrong from scratch--as clever and as wrong as phlogiston chemists.'"),
		tr("Lazarus Long said, 'The shamans are forever yacking about their snake-oil “miracles.” I prefer the real McCoy--a pregnant woman.'"),
		tr("Lazarus Long said, 'If the universe has any purpose more important than topping the woman you love and making a baby with her hearty help, I’ve never heard of it.'"),
		tr("Lazarus Long said, 'Thou shalt remember the 11th commandment and keep it Wholly.'"),
		tr("Lazarus Long said, 'A touchstone to determine the actual worth of an “intellectual”--find out how he feels about astrology.'"),
		tr("Lazarus Long said, 'Taxes are not levied for the benefit of the taxed.'"),
		tr("Lazarus Long said, 'There is no such thing as ‘‘Social Gambling. Either you are there to cut the other blokes heart out and eat it--or you’re a sucker. If you don’t like this choice--don’t gamble.'"),
		tr("Lazarus Long said, 'When a ship lifts, all bills are paid. No regrets.'"),
		tr("Lazarus Long said, 'The first time I was a drill instructor I was too inexperienced for the job--the things I taught those lads must have got some of them killed. War is too serious a matter to be taught by the inexperienced.'"),
		tr("Lazarus Long said, 'A competent and self confident person is incapable of jealousy in anything. Jealousy is invariably a symptom of neurotic insecurity.'"),
		tr("Lazarus Long said, 'Money is the sincerest of all flattery. Women love to be flattered. So do men.'"),
		tr("Lazarus Long said, 'You live and learn or you don’t live long.'"),
		tr("Lazarus Long said, 'Whenever women have insisted on absolute equality with men, they have invariably wound up on the dirty end of the stick. What they are and what they can do makes them superior to men, and their proper tactic is to demand special privileges, all the traffic will bear. They should never settle merely for equality. For women, “equality” is a disaster.'"),
		tr("Lazarus Long said, 'Peace is an extension of war by political means. Plenty of elbowroom is pleasanter--and much safer.'"),
		tr("Lazarus Long said, 'One man’s “magic” is another man’s engineering. “Supernatural” is a null word.'"),
		tr("Lazarus Long said, 'The phrase “we (I) (you) simply must--”designates something that need not be done. “That goes without saying” is a red warning. “Of course” means you had best check it yourself. These small--change cliches and others like them, when read correctly, are reliable channel markers.'"),
		tr("Lazarus Long said, 'Do not handicap your children by making their lives easy.'"),
		tr("Lazarus Long said, 'Rub her feet.'"),
		tr("Lazarus Long said, 'If you happen to be one of the fretful who can do creative work, never force an idea; you’ll abort it if you do. Be patient and you’ll give birth to it when the time is ripe. Learn to wait.'"),
		tr("Lazarus Long said, 'Never crowd youngsters about their private affairs--sex especially. When they are growing up they are nerve ends all over, and resent (quite properly) any invasions of their privacy. Oh sure, they’ll make mistakes--but that’s their business, not yours. (You made your own mistakes, did you not?)'"),
		tr("Lazarus Long said, 'Never underestimate the power of human stupidity!'"),
	};
	int lazarusQcount = sizeof(lazarusQs)/sizeof(QString);

	int quoteWho = g_Au.Dice().Roll(1,4);
	switch(quoteWho)
	{
	case 1:
		return spockQs[g_Au.Dice().Roll(1, spockQcount, -1)];
	case 2:
		return yodaQs[g_Au.Dice().Roll(1, yodaQcount, -1)];
	case 3:
		return bobQs[g_Au.Dice().Roll(1, bobQcount, -1)];
	default:
		return lazarusQs[g_Au.Dice().Roll(1, lazarusQcount, -1)];
	}
}

QString DaSentient::ThinkOfSomethingToSay(const DaPerson& person) const
{
	// This table contains common statements that don't imply an environment or spacecraft.
	QString commonPleasantries[] =
	{
		tr("How old are you? It's such an odd question when you don't spend most of your life doing laps around the same sun."),
		tr("Have you ever noticed how many cultures celebrate, as a national holiday, "
				"the day they threw out their last government."),
		//tr("The next time your Bic runs out of gas, do this. Take the roller off the top. Remove the flint and spring. "
				//"Stretch out the spring and wrap one end of it around the flint. "
				//"Holding the spring, heat the flint in a flame until it glows "
				//"then throw it against a hard surface, like a slab of concrete."),
		tr("Gold is the money of kings; silver is the money of gentlemen; barter is the money of the common man; "
				"and debt is the money of slaves."),
		tr("Pigs eat. Hogs get slaughtered."),
		tr("Governments like to enact laws but they don't understand law. A law is like: rocks fall down in gravity. "
				"A government would try to change that by decree, arrest the rock, and throw it in prison until it submits."),
		tr("A brother may not be a friend, but a friend will always be a brother."),
		tr("An armed society is a polite society."),
		tr("I want to find a ringworld and meet a girl like Teela Brown, just like in Larry Niven's book."),
		tr("Is rishathra something you engage in? I've tried it a couple of times. It's weird."),
		tr("Some day I want to own my own star ship. Hey diddle dee dee, the starman's life for me."),
		tr("A robot asked me out on a date. I said no. Am I old fashioned or what?"),
		tr("I once knew a Syndicate spy. That got intense sometimes, pretending we were married and all. "
				"I'm glad my spouse never found out."),
		tr("Space travel makes me queasy."),
		tr("Space travel cannot solve the problem of overpopulation. You couldn't manufacture enough spacecraft per day to keep "
				"up with the birth rate. Even if you could, you would be challenged with finding enough brave pioneers every day "
				"willing to fill them up."),
		tr("Spaceships always feel cold and drafty."),
		tr("One of the biggest obstacles to the development of space travel is the government's paranoia that one of its "
				"citizens might escape. That prevents most species from ever reaching the stars."),
		tr("I don't like aliens."),
		tr("Logan ran. Look where it got him."),
		tr("'To Protect And To Serve' It's funny how many people think that means them."),
		tr("Just because you've taken three steps down the wrong road, doesn't mean you have to take two more."),
		tr("I met a guy who got partially spaghettified in a black hole. He was messed up."),
		tr("I'm the only hell my momma ever raised."),
		tr("Aliens give me the creeps."),
		tr("Hiring a stock broker is like hiring a professional gambler. They might win a lot but it's never your money that got "
				"lucky."),
		//tr("I like hunting, just to kill things. It's best if they scream."),
		tr("I don't feel very good."),
		//tr("People like Marvin Heemeyer must never be forgotten, lest their death be in vain."),
		//tr("Have you ever heard the legend of Joe Stack?"),
		//tr("LaVoy Finicum was murdered in plain sight, in front of many witnesses and cameras. Where is the justice?"),
		tr("It is strange that I have absolutely no memory of my childhood."),
		tr("I'm glad I don't have the free will of an avatar. It's a lot easier to just stand here."),
		tr("I was accepted into star fleet academy with a full ride scholarship."),
		tr("I flunked out of star fleet academy. Squeamish leaf eaters! "
				"I guess practical jokes with meat weren't as funny there as back home."),
		tr("Avatars are overrated."),
		tr("I know Haxus. Really, he's a friend of mine. We play golf."),
		tr("Do you have any naked videos of your mate? Wanna buy some?"),
		tr("I met Haxus once."),
		tr("A recent newsfeed said the government of some planet criminalized the possession or consumption of a specie "
				"of plant that grows there. It's intoxicating to the natives and they use it for herbal remedies. "
				"The government plans to enforce this law galaxy-wide. "
				"They figure the plant could grow somewhere else and it might intoxicate other people."),
		tr("In a recent newsfeed, I saw that some scientist has developed a new theory about stone henges. "
				"I really like watching that kind of stuff."),
		tr("Capt. Kirk is soo cool. I want to travel the galaxy and dork green women, too."),
		tr("Barney Rubble; he's an actor."),
		tr("An owl ate my ducks! They're federally protected birds. What am I supposed to do now? "
				"Get it a plate and silverware?"),
		tr("Have you ever seen a badger? Those things stink and they're mean. "
				"Their skull has special channels to protect the jaw muscles, "
				"so you can't pry them loose without removing its head."),
		tr("Somebody stretched plastic wrap across the toilet bowl the other day. "
				"It was a real mess when my sister went to pee."),
		tr("I got expelled from school because I put those little packs of ketchup under the pads on the toilet seats in the "
				"girls' bathroom."),
		tr("Highborns give me the willies. "
				"There's something strange about them, like they have a different electrical charge than everybody else. "
				"I can feel them when they're near."),
		tr("I got the strangest feeling of déjà vu while playing %1 the other day.").arg(g_Au.GameName()),
		tr("I pray every night the Lord keep my soul, lest I despawn before I wake."),
		tr("Say 'hi' to Deantwo for me."),
		tr("I'm excellent at guessing weight. Without your gear, you look to weigh about %1kg.")
			.arg((person.Mass()*(person.Gender()==Female?.96f:1.04f))*KilogramsPerPound(), 0,'f',1),
		tr("You look familiar. Have we met?"),
		tr("It wasn't me; it must have been somebody that looked just like me."),
		tr("You're not a bill collector are you?"),
		tr("I have a really hot race car parked in my yard back home. You can't come see it though."),
		tr("I've had a lot of great cats. The last cat was a demon from hades named Nala. "
				"She outlived all the great cats combined. I still haven't recovered."),
		tr("I knew somebody named %1 once.").arg(person.Name()),
		tr("Have you ever noticed that if you enter 99 into the microwave timer, you get a minute and thirty nine seconds? "
				"If you enter 100, you only get a minute."),
		tr("Have you ever toasted a CD in a microwave? Try it for three seconds and watch what happens. Don't do it with a CD "
				"you ever want to use again. :D"),
		tr("Just because you took three steps down the wrong road, doesn't mean you have to take two more."),
		tr("One does evil enough when one does no good."),
		tr("A cat that's jumped on a hot stove isn't too quick to jump on a cold one after that either."),

		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
		"Q",	// Random selection of this entry triggers subsequent random selection from quotes table.
	};
	int cpcount = sizeof(commonPleasantries)/sizeof(QString);

	QString text;
	if(IsInSpacecraft())
	{
		QString pleasantries[] =
		{
			tr("Han Solo shot Greedo first. I was there."),
			tr("This is a great ship."),
			tr("It must have taken a long time to design this ship."),
			tr("I picked up some cool souvenirs at the last world."),
			tr("I've ridden on ships way better than this piece of crap."),
			tr("Does the galley always smell like that?"),
			tr("I brought some yummy cheese sandwiches along. I stored them safely under the bed in my cabin."),
			tr("I would like to report a rat. It scurried into the ventilator in my cabin. "
					"It was munching on some cheese sandwiches it dragged out from under a bed."),
			tr("I mated with a crewman last night."),
			tr("I met a crewman I want to mate with."),
			tr("I mated with a troop last night."),
			tr("I met a troop I want to mate with."),
			tr("I want a new cabin so I can get some sleep. Someone spent all night mating loudly in the cabin next to mine."),
			tr("I hope the captain knows what he's doing."),
			tr("I heard pirates eat passengers, or worse."),
			tr("I heard the vacuum of space feels like your skin is burning in acid, then your eyes pop out. D:"),
			tr("Pirates haven't been spotted near here, have they?"),
			tr("What was that meat they served in the galley today?"),
			tr("That food we had for lunch today, was it really a vegetable? I thought it was a sea slug with antlers. "
					"I swear it looked at me."),
			tr("My cabin is nice."),
			tr("I caught some fuzzy little creatures on that last world. I know we aren't supposed to bring stuff like that "
					"aboard, but they were soo cute. Only, uh, they chewed a hole in the ParcX box I was keeping them in."),
			tr("I found some nifty rocks on shore leave. When I went to my cabin to look at them later, one had cracked open and "
					"it was hollow inside. Do you suppose there was something in there?"),
			tr("I went swimming on shore leave. Something bit me. The bite has swollen considerably. It feels weird, like there's "
					"something's moving around in there."),
			tr("I went swimming on shore leave. Something bit me. I haven't felt well since then, some diarrhea, a little "
					"vomiting, nothing too severe."),
			tr("Wow those native mushrooms are fantastic. Waitress! I'd like another order...of... Wait, how did you get here? "
					"When...how...did I get back to the ship? Um. Wait a minute."),
			tr("I ate some of the local food on shore leave. It didn't set well. After I got sick, it climbed out of the toilet in "
					"my cabin. I didn't see where it went."),
			tr("I ate some of the local food on shore leave. It was delicious."),
			tr("I ate some native food on shore leave. Now I'm breaking out with a rash everywhere. It's very uncomfortable."),
			tr("I got some really great pictures of that last world."),
			tr("Space travel is so awesome. I could never get tired of seeing all the different worlds, nebulae, "
					"and the milky way of the galaxy."),
			tr("I've heard of some ships carrying out breeding experiments. Do they do that here?"),
			tr("It would be fun to travel to all twenty galaxies in the Universe."),
			tr("Is there a doctor on this ship? I have a boil that needs lancing."),
			tr("I don't get the blue shift. The pilot says it's a kind of Doppler effect, "
					"like the sound changing as a train passes, only with light instead of sound. "
					"I can't seem to get my head around that; it just doesn't make sense to me."),
			tr("My family is proud of me traveling in a spaceship. It's almost like I'm an avatar."),
			tr("The bed in my cabin could use a new mattress."),
			tr("The interior decorator did a nice job in here."),
			tr("The air in this ship smells like it's hauling old cheese or dirty laundry. I can't tell which."),
			tr("The architect of this ship was a true visionary."),
			tr("Ringworlds are a myth. They don't really exist. I read it on the Internet."),
			tr("I'm afraid of despawning when I leave the ship."),
			tr("Is despawning the same as dying? Do you suppose it hurts much?"),
			tr("I hope I get to colonize a perfect Garden of Eden world."),
			tr("After I get established, my kids are going to join me."),
			tr("Some people call the habitable zone the 'Goldilocks Zone'. It's not too this or that; it's just right."),
			tr("The gut wrenching twist when you enter a wormhole almost makes me puke."),
			tr("Until I traveled space much, I never realized the sun was down. It was up all my life; that was so backward. "
					"Those Earthworms probably never get it."),
			tr("I have a theory that those glowing entities are related to wormholes. I think they can probably travel through "
					"them at will, as easily as a fish swims a stream."),
		};
		int pcount = sizeof(pleasantries)/sizeof(QString);
		int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
		if(pi < cpcount)
		{
			text = commonPleasantries[pi];
			if(text == "Q")
				text = QuotableQuote();
		}
		else
			text = pleasantries[pi - cpcount];
	}
	else if(IsInAtmosphere())
	{
		const DaWorld* world = FindParentPlanetInEnv();
		if(world && world->Hydrographics() > 0)
		{
			// World has water.
			if(world->HasBreathableAtmosphere())
			{
				// World has water and breathable atmosphere.
				switch(world->OrbitZone())
				{
				case OZONE_Inferno:
				case OZONE_Frigid:
				case OZONE_DeepSpace:
					// These won't have breathable atmospheres.
					break;
				case OZONE_Inner:
					{
						QString pleasantries[] =
						{
							tr("I just love it here."),
							tr("The fishing is great here."),
							tr("Hunting isn't bad here if you stay near the coast."),
							tr("The water is a like swimming in a bath tub."),
							tr("Be careful of the wildlife."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Habitable:
					{
						QString pleasantries[] =
						{
							tr("I just love it here."),
							tr("The fishing is excellent here."),
							tr("Hunting is excellent here."),
							tr("The water is great for swimming."),
							tr("Be careful of the wildlife."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Outer:
					{
						QString pleasantries[] =
						{
							tr("I just love it here."),
							tr("The fishing is great here."),
							tr("Hunting isn't bad here if you stay near the coast."),
							tr("The water is too cold for swimming."),
							tr("Be careful of the wildlife."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				}
			}
			else
			{
				// World has water and non-breathable atmosphere.
				switch(world->OrbitZone())
				{
				case OZONE_Inferno:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("The acrid smell of the lava gets into everything."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("My air conditioner is broken; it smells like fuel oil."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Inner:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("The fishing is good here."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Habitable:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("The fishing is good here."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Outer:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("The fishing is good here."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Frigid:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("It's dark and cold here all the time."),
							tr("My heater is broken; there's ice around my windows."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_DeepSpace:
					break;
				}
			}
		}
		else if(world)
		{
			// World has no water.
			if(world->HasBreathableAtmosphere())
			{
				// World has no water and breathable atmosphere.
				switch(world->OrbitZone())
				{
				case OZONE_Inferno:
				case OZONE_Frigid:
				case OZONE_DeepSpace:
					// These won't have breathable atmospheres.
					break;
				case OZONE_Inner:
					{
						QString pleasantries[] =
						{
							tr("I love the hot desert here."),
							tr("The fishing is good here."),
							tr("Hunting isn't bad here."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Habitable:
					{
						QString pleasantries[] =
						{
							tr("I just love the desert here."),
							tr("The fishing is good here."),
							tr("Hunting is ok here."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Outer:
					{
						QString pleasantries[] =
						{
							tr("I just love the cold desert here."),
							tr("The fishing is good here."),
							tr("Hunting isn't bad here."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				}
			}
			else
			{
				// World has no water and non-breathable atmosphere.
				switch(world->OrbitZone())
				{
				case OZONE_Inferno:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("My air conditioner is broken; it smells like burning cheese."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Inner:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Habitable:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Outer:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_Frigid:
					{
						QString pleasantries[] =
						{
							tr("I hate it here."),
							tr("It's dark and cold here all the time."),
							tr("My heater is broken; there's ice around my windows."),
							tr("I like to hunt bunnies in the biodome. Shh, don't tell; it's not allowed."),
							tr("I get tired of filtered air."),
							tr("I miss my home world."),
						};
						int pcount = sizeof(pleasantries)/sizeof(QString);
						int pi = g_Au.Dice().Roll(1, cpcount + pcount, -1);
						if(pi < cpcount)
						{
							text = commonPleasantries[pi];
							if(text == "Q")
								text = QuotableQuote();
						}
						else
							text = pleasantries[pi - cpcount];
					}
					break;
				case OZONE_DeepSpace:
					break;
				}
			}
		}
	}
	else
		text = tr("Help! I don't know what to do.");
	return text;
}

void DaSentient::SayPleasantry(const DaPerson& person)
{
	// Decide what to say.
	int tries = 0;
	QString text, saidKey;
	m_TalkLock.lockForRead();
	do
	{
		text = ThinkOfSomethingToSay(person);
		saidKey = text.left(10);
		tries++;
	}
	while(m_LastSaid.contains(saidKey) && tries < 20);	// Try up to 20 times to come up with something new to say.
	m_TalkLock.unlock();

	if(tries >= 20 || text.isEmpty())
	{
		AuCommChat rmsg(tr("I have nothing more to say."), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Closed, person.Id());
		return;
	}

	// Remember what I decided to say so I can avoid repeating myself.
	if(!saidKey.isEmpty())
	{
		m_TalkLock.lockForWrite();
		m_LastSaid.append(saidKey);
		while(m_LastSaid.size() > 100)
			m_LastSaid.takeFirst();		// Remember the last 100 things I said.
		m_TalkLock.unlock();
	}

	// Say something.
	if(text.length() < AuCommChat::MaxChatText())
	{
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Closed, person.Id());
	}
	else
	{
		// Cheat and use this one so I can say more than what a normal chat message allows.
		AuCommChatContinued rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Closed, person.Id());
	}

	QString continuations[] =
	{
		tr("What else is new?"),
		tr("That is so interesting."),
		tr("You don't say."),
		tr("Hmm."),
		tr("Uh huh."),
		tr("Oh."),
		tr("Golly."),
		tr("Go on."),
		tr("Ooh."),
		tr("Gosh."),
		tr("Groovy."),
		tr("Cool."),
		tr("Awesome."),
		tr("Astounding."),
		tr("Excellent."),
		tr("I wondered."),
		tr("That's worth thinking about."),
		tr("It happens."),
		tr("It's been known to happen."),
		tr("Really?"),
		tr("That is so profound."),
		tr("Pure wisdom."),
		tr("That sounds familiar."),
		tr("I heard that."),
		tr("Then what?"),
		tr("Do you always feel this way?"),
	};
	int ccount = sizeof(continuations)/sizeof(QString);
	int pi = g_Au.Dice().Roll(1, ccount, -1);
	text = continuations[pi];

	AuCommChatChoice nomsg(person.Id(), text, 1, 5, AuCommStdChannel::VoiceChannelName(),
			Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
	Say(nomsg, person.Id());
}

void DaSentient::EnergizeTransporter(const AuCommPacket* msg, bool isFrom, int padLimit, DaAtom* targAtom, const AuPoint& targLoc,
		const AuPoint& targZ, quint16 targWhich, AuPointArray& targPads)
{
	// I must be operating a transporter console to process this message.
	if(!IsTransporterStationPose() || !HasConsole())
		return;

	// Locate my ship.
	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return;	// Not likely.

	// Locate the sender. They must be aboard this ship and in my room.
	const DaPerson* person = dynamic_cast<const DaPerson*>(ship->FindIdRecursive(msg->FromId()));
	if(!person)
		return;	// Person was not found aboard my ship.
	Q_ASSERT(IsInCapsule());
	if(person->CapsuleRoomId() != CapsuleRoomId())
		return;	// Person is not in my room.

	// Determine stance toward message sender.
	POLITICAL_STANCE stanceToSender = PoliticalStanceMakeContact(person);
	if(stanceToSender == POL_Enemy)
		return;	// Do not communicate with enemies.

	// Citizen has already verified shields state at destination is ok.
	int result = ship->EnergizeTransporter(AtTransporterWhich(), isFrom, padLimit, targAtom, targLoc, targZ, targWhich, targPads,
				true, false);

	QString text;
	if(result == padLimit && padLimit == 1)
		text = QObject::tr("Transporter energized.");
	else if(result == padLimit)
		text = QObject::tr("Transporter energized with %1 portals.").arg(result);
	else if(result > 0)
		text = QObject::tr("Transporter energized with only %1 portals. "
				"Other portals were blocked by obstacles near target.").arg(result);
	else if(result == 0)
		text = QObject::tr("Transporter beam was blocked by obstacles near target; try again.");
	else if(result == -1)
		text = QObject::tr("Transporter is not ready.");
	else if(result == -2)
		text = QObject::tr("Target is beyond transporter range.");
	else if(result == -3)
		text = QObject::tr("Transporter cannot lock on to target.");
	else if(result == -4)
		text = QObject::tr("Insufficient power to energize transporter.");
	else if(result == -5)
		text = QObject::tr("Transporter has not finished its activation cycle.");
	else if(result == -6)
		text = QObject::tr("Vehicle transporter has no empty parking spots for the vehicle.");
	else if(result == -10)
		text = QObject::tr("Transporter blocked by shields on target vessel.");
	else if(result == -13)
		text = QObject::tr("Transporter blocked by shield generator on the ground.");
	else
		text = QObject::tr("Unable to energize transporter.");

	// Transporter operator announces the result at close range. This message is processed for a person in the same room.
	AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
			AuCommStdChannel::Close, msg->Scope());
	if(result >= 0)
		Say(msgResult, AuVisage::Closed);
	else
		Say(msgResult, AuVisage::Perplexed);
}

bool DaSentient::EnergizeVehicleTransporter(const AuCommPacket* msg, bool isFrom, DaAtom* targAtom, const AuPoint& targLoc,
		const AuPoint& targZ, quint16 targWhich, QList<AuMatrix4x4>& targParkingSpots)
{
	// I must be operating a transporter console to process this message.
	if(!IsTransporterStationPose() || !HasConsole())
		return false;

	// Locate my ship.
	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return false;	// Not likely.

	// Locate the sender. They must be aboard this ship and in a vehicle.
	const DaPerson* person = dynamic_cast<const DaPerson*>(ship->FindIdRecursive(msg->FromId()));
	if(!person)
		return false;	// Person was not found aboard my ship.
	if(person->InVehicleTransporter() == SPOT_Invalid)
		return false;
	DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
	if(!vehicle)
		return false;

	// Insure my transporter has a vehicle transporter module.
	COMMODITYID transModule = ship->TransporterModule(AtTransporterWhich());
	if(transModule != CID_VehicleTransporterModule)
		return false;	// My transporter is not equipped with a vehicle transporter.

	// Determine stance toward message sender.
	POLITICAL_STANCE stanceToSender = PoliticalStanceMakeContact(person);
	if(stanceToSender == POL_Enemy)
		return false;	// Do not communicate with enemies.

	// Citizen has already verified shields state at destination is ok.
	int result = ship->EnergizeVehicleTransporter(AtTransporterWhich(), vehicle, isFrom, targAtom, targLoc, targZ, targWhich,
			targParkingSpots, true, false);

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

	// Transporter operator announces the result at hail range. This message is processed for a person in transporter range.
	AuCommChat msgResult(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
			AuCommStdChannel::Hail, msg->Scope());
	if(result >= 0)
		Say(msgResult, AuVisage::Closed);
	else
		Say(msgResult, AuVisage::Perplexed);
	return result == 1;
}

QString DaSentient::DescribeActivity() const
{
	AuPose pose(CurrentGeneralPose());
	switch(pose.Pose())
	{
	case AuPose::ReceptiveMate:
		return tr("Waiting to Have Sex");
	case AuPose::MateOnBottom:
		{
			// I am in the mating on bottom pose. Find the f'er riding me.
			const DaCreature* rider = dynamic_cast<const DaCreature*>(Rider());
			if(rider && rider->CurrentGeneralPose() == AuPose::MateOnTop)
				return tr("Having Sex with %1").arg(rider->Name());
		}
		break;
	case AuPose::MateOnTop:
		{
			// I am on top.
			const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
			if(mount && mount->CurrentGeneralPose() == AuPose::MateOnBottom)
				return tr("Having Sex with %1").arg(mount->Name());
		}
		break;
	default:
		break;
	}

	QString order;
	switch(Order())
	{
#if defined(QT_NO_DEBUG)										// I want the compiler warning if I forget to handle an order here.
	default:
		order = tr("Doing Something");
		break;
#endif
	case ORDER_HengeCeremony:
		order = tr("Performing the Henge Ceremony.");
		break;
	case ORDER_Stay:
		order = tr("Standing");
		break;
	case ORDER_AttackTarget:
		order = tr("Attacking %1").arg(OrderTargetName());
		break;
	case ORDER_BattleStation:
		switch(CurrentGeneralPose().Pose())
		{
		case AuPose::Init:
		case AuPose::Beckon:
		case AuPose::Bite:
		case AuPose::Block:
		case AuPose::Eat:
		case AuPose::Hug:
		case AuPose::Kick:
		case AuPose::Lash:
		case AuPose::Point:
		case AuPose::Psionic:
		case AuPose::Stomp:
		case AuPose::Strike:
		case AuPose::Throw:
		case AuPose::Upright:
		case AuPose::Wag:
		case AuPose::Wave:
		case AuPose::MouthPursed:
		case AuPose::MouthClosed:
		case AuPose::MouthOpen:
		case AuPose::MouthAgape:
		case AuPose::MouthOpenWide:
		case AuPose::MouthYawn:
		case AuPose::MouthBite:
		case AuPose::MouthLaugh:
		case AuPose::MouthYap:
		case AuPose::MouthWhisper:
		case AuPose::MouthTalk:
		case AuPose::MouthJabber:
		case AuPose::MouthYell:
		case AuPose::MouthSmile:
		case AuPose::MouthGrin:
		case AuPose::MouthSad:
		case AuPose::MouthFrown:
		case AuPose::MouthCry:
		case AuPose::MouthPerplexed:
		case AuPose::MouthGrim:
		case AuPose::MouthSardonic:
			// These are not "general" poses.
			break;
		case AuPose::EyesOnly:
		case AuPose::Spawn:
		case AuPose::Straddle:
			// These poses don't occur or already handled above.
			break;
		case AuPose::GameTableSeat0_0:
		case AuPose::GameTableSeat1_0:
		case AuPose::GameTableSeat2_0:
		case AuPose::GameTableSeat3_0:
		case AuPose::GameTableSeat0_45:
		case AuPose::GameTableSeat1_45:
		case AuPose::GameTableSeat2_45:
		case AuPose::GameTableSeat3_45:
			order = tr("Playing a Game");
			break;
		case AuPose::ConsoleCommand:
		case AuPose::Command_Sit:
		case AuPose::Command_SitStationary:
			order = tr("Sitting");
			break;
		case AuPose::Command_Stand:
		case AuPose::Command_StandStationary:
			order = tr("Standing");
			break;
		case AuPose::ConsoleNav:
		case AuPose::Nav_Sit:
		case AuPose::Nav_SitStationary:
		case AuPose::Nav_Stand:
		case AuPose::Nav_StandStationary:
			order = tr("Navigating");
			break;
		case AuPose::ConsoleEngine:
		case AuPose::Engine_Sit:
		case AuPose::Engine_SitStationary:
		case AuPose::Engine_Stand:
		case AuPose::Engine_StandStationary:
			order = tr("Tuning Engines");
			break;
		case AuPose::ConsoleFireControl:
		case AuPose::FireControl_Sit:
		case AuPose::FireControl_SitStationary:
		case AuPose::FireControl_Stand:
		case AuPose::FireControl_StandStationary:
			order = tr("Targeting");
			break;
		case AuPose::ConsoleSurgery:
		case AuPose::Medic_Sit:
		case AuPose::Medic_SitStationary:
		case AuPose::Medic_Stand:
		case AuPose::Medic_StandStationary:
			order = tr("Tending Patients");
			break;
		case AuPose::ConsolePilot:
		case AuPose::Helm_Sit:
		case AuPose::Helm_SitStationary:
		case AuPose::Helm_Stand:
		case AuPose::Helm_StandStationary:
			order = tr("Piloting");
			break;
		case AuPose::ConsolePower:
		case AuPose::Power_Sit:
		case AuPose::Power_SitStationary:
		case AuPose::Power_Stand:
		case AuPose::Power_StandStationary:
			order = tr("Routing Power");
			break;
		case AuPose::ConsoleSensor:
		case AuPose::Sensor_Sit:
		case AuPose::Sensor_SitStationary:
		case AuPose::Sensor_Stand:
		case AuPose::Sensor_StandStationary:
			order = tr("Scanning");
			break;
		case AuPose::ConsoleShield:
		case AuPose::Shield_Sit:
		case AuPose::Shield_SitStationary:
		case AuPose::Shield_Stand:
		case AuPose::Shield_StandStationary:
			order = tr("Balancing Shields");
			break;
		case AuPose::ConsoleSurgeryUnit:
		case AuPose::Patient_Sit:
		case AuPose::Patient_SitStationary:
		case AuPose::Patient_Stand:
		case AuPose::Patient_StandStationary:
		case AuPose::Patient_Prone:
			order = tr("Receiving Medical Treatment");
			break;
		case AuPose::ConsoleTransporter:
		case AuPose::Transporter_Sit:
		case AuPose::Transporter_SitStationary:
		case AuPose::Transporter_Stand:
		case AuPose::Transporter_StandStationary:
			order = tr("Scanning");
			break;
		case AuPose::ConsoleTurret0:
		case AuPose::ConsoleTurret45:
		case AuPose::ConsoleTurret90:
		case AuPose::ConsoleTurret135:
		case AuPose::ConsoleTurret180:
		case AuPose::ConsoleTurret225:
		case AuPose::ConsoleTurret270:
		case AuPose::ConsoleTurret315:
		case AuPose::ConsoleTurretBottom:
		case AuPose::ConsoleTurretTop:
		case AuPose::Turret_Sit:
		case AuPose::Turret_SitStationary:
		case AuPose::Turret_SitSwivel3D:
		case AuPose::Turret_Stand:
		case AuPose::Turret_StandStationary:
			order = tr("Aiming");
			break;
		case AuPose::Crouch:
		case AuPose::Prone:
		case AuPose::SitOnGround:
		case AuPose::Stand:
		case AuPose::TreadWater:
		case AuPose::Hover:
		case AuPose::Ladder:
			order = tr("Waiting for Station");
			break;
		case AuPose::CrouchCreep:
		case AuPose::SitOnGroundCreep:
		case AuPose::Dig:
		case AuPose::Fly:
		case AuPose::FlyFast:
		case AuPose::Glide:
		case AuPose::Jump:
		case AuPose::LadderClimb:
		case AuPose::ProneCreep:
		case AuPose::StandRun:
		case AuPose::StandWalk:
		case AuPose::Swim:
		case AuPose::SwimFast:
			order = tr("Going to Station");
			break;
		case AuPose::DeadFaceDown:
		case AuPose::DeadFaceLeft:
		case AuPose::DeadFaceRight:
		case AuPose::DeadFaceUp:
			order = tr("Dead");
			break;
		case AuPose::WritheFaceDown:
		case AuPose::WritheFaceLeft:
		case AuPose::WritheFaceRight:
		case AuPose::WritheFaceUp:
			order = tr("Dying");
			break;
		case AuPose::ReceptiveMate:
			order = tr("Waiting to Have Sex");
			break;
		case AuPose::MateOnTop:
			order = tr("Having Sex");
			break;
		case AuPose::MateOnBottom:
			order = tr("Having Sex");
			break;
		case AuPose::BerthBottomFaceDown:
		case AuPose::BerthBottomFaceLeft:
		case AuPose::BerthBottomFaceRight:
		case AuPose::BerthBottomFaceUp:
		case AuPose::BerthTopFaceDown:
		case AuPose::BerthTopFaceLeft:
		case AuPose::BerthTopFaceRight:
		case AuPose::BerthTopFaceUp:
			order = tr("Sleeping");
			break;
		case AuPose::Post_Sit:
		case AuPose::Post_Stand:
			order = tr("Guarding");
			break;
		}
		break;
	case ORDER_ComeHere:
		order = tr("Moving");
		break;
	case ORDER_ComeHereExactly:
		order = tr("Positioning");
		break;
	case ORDER_FollowMe:
	case ORDER_FollowTarget:
		order = tr("Following %1").arg(OrderTargetName());
		break;
	case ORDER_Garrison:
		order = tr("Occupying");
		break;
	case ORDER_GetIn:
		order = tr("Getting In");
		break;
	case ORDER_GetOut:
		order = tr("Getting Out");
		break;
	case ORDER_GoToRoom:
		order = tr("Going to %2 aboard %1").arg(OrderTargetName()).arg(OrderTargetText());
		break;
	case ORDER_GoToTarget:
		order = tr("Going to %1").arg(OrderTargetName());
		break;
	case ORDER_GoToTargetExactly:
		order = tr("Going to %1").arg(OrderTargetName());
		break;
	case ORDER_CloseDoor:
		order = tr("Closing Door");
		break;
	case ORDER_OpenDoor:
		order = tr("Opening Door");
		break;
	case ORDER_GetItem:
		order = tr("Getting %1").arg(OrderTargetName());
		break;
	case ORDER_Sleep:
		order = tr("Sleeping");
		break;
	case ORDER_SleepDayDream:
		order = tr("Day Dreaming");
		break;
	case ORDER_SleepEat:
		order = tr("Eating");
		break;
	case ORDER_SleepExercise:
		order = tr("Exercising");
		break;
	case ORDER_SleepGame:
		order = tr("Playing %1").arg(g_Au.GameName());
		break;
	case ORDER_SleepLetter:
		order = tr("Writing A Letter");
		break;
	case ORDER_SleepMovie:
		order = tr("Watching A Movie");
		break;
	case ORDER_SleepMusic:
		order = tr("Listening To Music");
		break;
	case ORDER_SleepPine:
		order = tr("Feeling Homesick");
		break;
	case ORDER_SleepPhone:
		order = tr("Talking On The Phone");
		break;
	case ORDER_SleepRead:
		order = tr("Reading");
		break;
	case ORDER_SleepTalk:
		order = tr("Talking");
		break;
	case ORDER_SleepText:
		order = tr("Texting");
		break;
	case ORDER_SleepTV:
		order = tr("Watching TV");
		break;
	case ORDER_SleepWeb:
		order = tr("Surfing The Web");
		break;
	case ORDER_DamageReport:
		order = tr("Reporting Damage");
		break;
	case ORDER_SurveyScanPlanet:
		order = tr("Survey Scanning Planet");
		break;
	case ORDER_SurveyScanSystem:
		order = tr("Survey Scanning System");
		break;
	case ORDER_TacticalScan:
		order = tr("Tactical Scanning");
		break;
	case ORDER_EnterSurgery:
		if(IsPatientStationPose())
			order = tr("Undergoing Surgery");
		else
			order = tr("Waiting for Surgery Unit");
		break;
	case ORDER_TakeCommand:
		order = tr("Taking Command");
		break;
	case ORDER_ReleaseCommand:
		order = tr("Relinquishing Command");
		break;
	case ORDER_DropRider:
		order = tr("Dropping Rider");
		break;
	case ORDER_MountMe:
	case ORDER_MountTarget:
		order = tr("Mounting %1").arg(OrderTargetName());
		break;
	}
	return order;
}

unsigned int DaSentient::IsTocker() const
{
	unsigned int ms = DaCreature::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is not zero.

	if(IsBodyDyingOrDead())
		return ms;
	if(!IsAIControlled())
		return ms;

	unsigned int thinkRate = AIThinkRate();
	if(thinkRate && ms > thinkRate)
		ms = thinkRate;
	return ms;
}

// Decide what to do.
void DaSentient::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaCreature::Tock(last, now, scripts, tockTrans);
	if(IsBodyDyingOrDead())
		return;
	GainExperienceForWorking();	// People online and AI controlled gain experience.
	if(!IsAIControlled())
		return;

	// Don't do any AI until it's time to think again.
	if(!IsTimeToThink())
		return;

	int msElapsed = last.MSecsTo(now);
	if(msElapsed <= 0)
		return;

	int minCycles = last.ElapsedMinTicks(now);
	if(minCycles)
	{
		if(CurrentGeneralPose() == AuPose::ReceptiveMate)
		{
			// Don't lay around waiting to get screwed forever.
			if(PoseTime().ElapsedMinTicks())
				SetGeneralPose(AuPose::SitOnGround);
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	DoAI();
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	ExecuteDecision(msElapsed);
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
}

// Do what I decided to do.
void DaSentient::ExecuteDecision(int msElapsed)
{
	m_DecisionTableLock.lock();

	m_ThinkLock.lockForWrite();

	if(!m_DecisionCount)
	{
		m_ThinkLock.unlock();
		m_DecisionTableLock.unlock();
		return;
	}

	// Make a temporary list of the decisions I will execute.
	QVector<AuSentientDecision::DECISION> decisions(m_Decisions);	// The queue of my decisions. Indices into m_DecisionTable.
	unsigned int decount = m_DecisionCount;
	m_DecisionCount = 0;
	m_ThinkLock.unlock();

	if(decount > 10)
		qDebug() << "DaSentient::ExecuteDecision" << Id() << Name() << "high decision count" << decount;

	// Execute my decisions.
	for(unsigned int dec=0; dec<decount; dec++)
	{
		AuSentientDecision::DECISION decision = decisions[dec];
		if(decount > 10)
			qDebug() << "DaSentient::ExecuteDecision" << Id() << Name() << "executing decision" << decision;
		//Q_ASSERT(decision > AuSentientDecision::D_Invalid);
		//Q_ASSERT(decision < AuSentientDecision::D_COUNT);
		//Q_ASSERT(decisions[decision]);
		Q_ASSERT(m_DecisionTable);
		m_DecisionTable[decision]->Execute(this, msElapsed);
	}

	m_DecisionTableLock.unlock();
}

void DaSentient::DoAttackTarget(DaAtom* targetAtom, const AuPoint& sightedLoc, bool sightedShield, bool targetIsFriendlyOrDying,
			const AuCombatWeapon& weapon, bool useSpecWpn, const AuGearPos& usePos, INVENTORYSUB useSub)
{
	// Use the weapon on the target.
	bool posechanged = false;
	if(weapon.m_CID < CID_Invalid)
	{
		// I am attacking with a physical part of my body.
		if(IsUnderGround())
			posechanged = PoseAttackUnderGround();
		else if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
			posechanged = PoseAttackUnderWater();
		else if(CanHover())
			posechanged = PoseAttackHovering();
		else if(CanFly())
			posechanged = PoseAttackFlying();
		else
			posechanged = PoseAttackStanding();
	}
	else if(weapon.m_CID > CID_Invalid)
	{
		// I am attacking with a weapon.
		posechanged = false;
	}
	if(posechanged)
		SetModified();		// Pose not stored in db.

	// Use weapon on the target.
	// The citizen "uses" the weapon in hand.
	// This will consume ammo if applicable.
	UseItemEffect(weapon.m_CID, weapon.m_Mode, weapon.m_w1.m_Effect, weapon.MaxRadius(), useSub, sightedLoc);
	if(weapon.m_CID < CID_Invalid || UseGearItem(usePos, useSub))
	{
		if(weapon.m_Mode == WM_Detonator)
		{
			// Drop an item and configure it to be detonated by this person.
			//DHH AI doesn't know how to use detonator weapons safely.
		}
		else if(weapon.m_Mode == WM_Fuse)
		{
			// Drop an item and configure it to blow up soon.
			//DHH AI doesn't know how to use fuse weapons safely.
		}
		else if(weapon.m_Mode == WM_Area)
		{
			// Perform area attack that excludes me and others of my own kind.
			AuCombatAttacker attacker(Id(), Name(), EmpireId(), IsPirate());
			if(weapon.m_CID < CID_Invalid)
			{
				const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
				if(mount)
					attacker.SetSpeciesExempt(Species(), mount->Species());	// Mount species is exempt. Could be its weapon.
				else
					attacker.SetSpeciesExempt(Species());	// My species is exempt from my own area attack.
			}

			AuCombatDefender defender;

			AuCombatResult result;

			AttackTarget(attacker, defender, weapon, true, result);

			// Check to see if the atom was destroyed or incapacitated.
			if(targetAtom)
			{
				if(targetAtom->IsDead())
					targetIsFriendlyOrDying = true;	// Atom killed
				else
				{
					DaCreature* creature = dynamic_cast<DaCreature*>(targetAtom);
					if(creature && creature->IsBodyDyingOrDead())
						targetIsFriendlyOrDying = true;	// Body killed, or at least dying.
				}
			}
		}
		else
		{
			// Attack the target atom.
			//qDebug("%s attacking target %g range", (const char*)Name().toAscii(), targetRange);
			AuCombatAttacker attacker(Id(), Name(), EmpireId(), IsPirate());
			if(IsInVehicle())
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
				if(vehicle)
					attacker.m_Name = QObject::tr("%1 in %2").arg(Name()).arg(vehicle->Name());
			}
			else if(IsOnVehicle())
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
				if(vehicle)
					attacker.m_Name = QObject::tr("%1 on %2").arg(Name()).arg(vehicle->Name());
			}
			else if(IsInSpacecraft())
			{
				const DaVessel* ship = FindParentVesselInEnv();
				if(ship)
					attacker.m_Name = QObject::tr("%1 aboard %2").arg(Name()).arg(ship->NameWithEmpire());
			}
			else if(IsInBd())
			{
				const DaBd* bd = FindParentBdInEnv();
				if(bd)
					attacker.m_Name = tr("%1 in %2").arg(Name()).arg(bd->CityBuildingName());
			}

			double targetRange = sightedLoc.Length();	// Range to target.
			AuCombatDefender defender(targetAtom, sightedLoc, sightedShield, targetRange);

			AuCombatResult result;

			AttackTarget(attacker, defender, weapon, true, result);

			// Check to see if the atom was destroyed or incapacitated.
			if(targetAtom)
			{
				if(targetAtom->IsDead())
					targetIsFriendlyOrDying = true;	// Atom killed
				else
				{
					DaCreature* creature = dynamic_cast<DaCreature*>(targetAtom);
					if(creature && creature->IsBodyDyingOrDead())
						targetIsFriendlyOrDying = true;	// Body killed, or at least dying.
				}
			}
		}

		// Notify body the special weapon was used so we can't use it again until its ready.
		if(useSpecWpn && weapon.m_CID < CID_Invalid)
			UseSpecialWeapon();

		// If the target is friendly or dying, don't attack it again.
		// This allows me to strike back when a friendly hits me but then I stop after one blow.
		// This also allows me to move to my next target when the current target is incapacitated.
		if(targetIsFriendlyOrDying && OrderCount() > 2)
			RequestOrder();	// Stop attacking and request my next order.
	}
}

void DaSentient::DoPose(const AuPose& pose)
{
	if(SetGeneralPose(pose, StationId()))
	{
		UpdateThinkRate();
		// Kick loose from at-rest if my nu pose is a moving pose.
		if(IsMovingPose())
		{
			SetInMotion();
			TickTock();
		}
	}
}

void DaSentient::DoEnterVehicle(DaVehicle* vehicle)
{
	if(vehicle)
	{
		int seatIndex = MountVehicle(vehicle);
		if(seatIndex == 0)
		{
			// I am sitting in seat zero. Capture the vehicle if it belongs to the enemy.
			POLITICAL_STANCE stance = PoliticalStanceMakeContact(vehicle);
			if(stance == POL_Enemy)
				vehicle->SetEmpireId(EmpireId());
		}
	}
	RequestOrder();	// Ask for my next order.
}

void DaSentient::DoSitAtConsole(quint16 stationId, const AuStation& station, quint16 which)
{
	// Insure the console is not already occupied.
	DaCapsule* ship = dynamic_cast<DaCapsule*>(Parent());
	if(!ship)
		return;
	if(ship->IsConsoleSeatOccupied(stationId, station.SeatIndex()))
		return;

	ForcePosStop(AuAtomPos(ship->Id(), station.OpM()));

	// Must call SitAtConsole after atom is moved to console location.
	AuPose pose = SitAtConsole(stationId, station.Type(), station.OccPos(), station.SeatIndex(), which);
	SyncController(AuSyncPose(pose, 0, stationId, station.SeatIndex()));
	InvalidateEnvironment();
	SetModified();
	UpdateThinkRate();			// Think rate changes now that I am seated at a console.
}

void DaSentient::DoTeleport(const AuPoint& loc)
{
	// Calculate movement vector to destination location, e.g. my bunk.
	AuPoint mvec = loc - Location();
	AuAtomPos pos(Pos());
	mvec = pos.UnProjectVector(mvec);
	AuMatrix4x4 tmv;
	tmv.Translate(mvec);
	pos.TransformBy(tmv);
	ChangePosNotParent(pos);
	InvalidateEnvironment();
	SetAtRest();
	SetModified();
	UpdateThinkRate();					// Think rate changes now that I am seated at a console.
}

void DaSentient::DoMoveInCrew()
{
	// I am moving in to the ship I am aboard or the city I am in.
	if(IsInsideSpacecraft())
	{
		// I am aboard a spacecraft.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			// The ship can accept a crew berth request from me.
			// There is an empty Crew berth so take it.
			OccupyBerth(ship);
		}
	}
	else if(!BdId().IsNull())
	{
		// I am in a city and I am not inside a spacecraft so join the local population.
		// Add myself to the population of the city and then disappear.
		DaAtom* env = FindEnvironmentAtom();
		DaAtom* atom = env->FindSubatomId(BdId());
		DaBd* bd = dynamic_cast<DaBd*>(atom);
		if(bd)
		{
			if(LastShipName().isEmpty())
				bd->CommunityIncreaseCrew(this, QObject::tr("Dismissed from crew"));
			else
				bd->CommunityIncreaseCrew(this, QObject::tr("Dismissed from crew of %1").arg(LastShipName()));
		}
		Kill();
	}
}

void DaSentient::DoMoveInOfficer()
{
	// I am moving in to the ship I am aboard or the city I am in.
	if(IsInsideSpacecraft())
	{
		// I am aboard a spacecraft.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship && ship->EmpireId() == EmpireId() && ship->OfficerBerthsAvailable())
			OccupyBerth(ship);								// There is an empty Officer berth so take it.
		else
			ReleaseTheCon();
	}
	else if(!BdId().IsNull())
	{
		// I am in a city and I am not inside a spacecraft so join the local population.
		// Add myself to the population of the city and then disappear.
		DaAtom* env = FindEnvironmentAtom();
		DaAtom* atom = env->FindSubatomId(BdId());
		DaBd* bd = dynamic_cast<DaBd*>(atom);
		if(bd)
		{
			if(LastShipName().isEmpty())
				bd->CommunityIncreaseCrew(this, QObject::tr("Dismissed from command"));
			else
				bd->CommunityIncreaseCrew(this, QObject::tr("Dismissed from command of %1").arg(LastShipName()));
		}
		Kill();
	}
}

void DaSentient::DoMoveInPassenger()
{
	// I am moving in to the ship I am aboard or the city I am in.
	if(IsInsideSpacecraft())
	{
		// I am aboard a spacecraft.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship && ship->CanAcceptBerthType(AuRosterEntry::PassengerAI) && ship->PassengerBerthsAvailable())
		{
			// The ship can accept a passenger berth request from me.
			// There is an empty passenger berth so take it.
			OccupyBerth(ship);
		}
	}
	else if(!BdId().IsNull())
	{
		// I am in a city so join the local population.
		// Add myself to the population of the city and then disappear.
		DaAtom* env = FindEnvironmentAtom();
		DaAtom* atom = env->FindSubatomId(BdId());
		DaBd* bd = dynamic_cast<DaBd*>(atom);
		if(bd)
		{
			if(LastShipName().isEmpty())
				bd->CommunityIncreasePopulation(this, QObject::tr("Marooned by ship"));
			else
				bd->CommunityIncreasePopulation(this, QObject::tr("Marooned by %1").arg(LastShipName()));
		}
		Kill();
	}
}

void DaSentient::DoMoveInTroop()
{
	if(IsInsideSpacecraft())
	{
		// I am aboard a spacecraft and I have no ship assignment.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship && ship->EmpireId() == EmpireId() && ship->CanAcceptBerthType(AuRosterEntry::Troop) &&
				ship->TroopBerthsAvailable())
		{
			// The ship can accept a troop berth request from me.
			// There is an empty troop berth so take it.
			OccupyBerth(ship);
		}
	}
	else if(!BdId().IsNull() && InCityEmpire() == EmpireId())
	{
		// I am in a city and I am not assigned to a ship and I am not inside a ship.
		// Attempt to garrison this city.
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld)
		{
			DaAtom* atom = llworld->FindSubatomId(BdId());
			DaBd* bd = dynamic_cast<DaBd*>(atom);
			if(bd)
			{
				if(LastShipName().isEmpty())
					bd->CommunityIncreaseTroops(this, QObject::tr("Discharged by ship"));
				else
					bd->CommunityIncreaseTroops(this, QObject::tr("Discharged by %1").arg(LastShipName()));
			}
		}
	}
}

void DaSentient::DoShipPowerPlantState(DaVessel* ship, bool on)
{
	if(on)
		ship->TurnPowerPlantOn();
	else
		ship->TurnPowerPlantOff();
}

void DaSentient::DoShipLandingGear(DaVessel* ship, bool extended)
{
	if(!ship->HasRetractableLandingGear())
		return;
	if(ship->IsLandingGearExtended() == extended)
		return;
	ship->LandingGearToggle(extended);
	ship->SetModified();
}

void DaSentient::DoShipRollPitchYaw(DaVessel* ship, float roll, float pitch, float yaw)
{
	ship->RollPitchYaw(roll, pitch, yaw);
}

void DaSentient::DoShipBrakeState(DaVessel* ship, bool brakeF, bool brakeLV)
{
	ship->BrakeStop();
	if(brakeF)
		ship->Brake();
	if(brakeLV)
		ship->BrakeLV();
}

void DaSentient::DoShipGravityDriveState(DaVessel* ship, DaVessel::GRAVITY_DRIVE_STATE state)
{
	ship->SetGravityDriveState(state);
}

void DaSentient::DoShipThrottleState(DaVessel* ship, qint8 state)
{
	ship->SetThrottle(state);
}

void DaSentient::DoShipThrusterLateralState(DaVessel* ship, qint8 thrust)
{
	ship->SetThrustLateral(thrust);
}

void DaSentient::DoShipThrusterLongitudinalState(DaVessel* ship, qint8 thrust)
{
	ship->SetThrustLongitudinal(thrust);
}

void DaSentient::DoShipThrusterVerticalState(DaVessel* ship, qint8 thrust)
{
	ship->SetThrustVertical(thrust);
}

void DaSentient::DoShipFTLDriveState(DaVessel* ship, bool on, quint16 whichFTL)
{
	if(on)
		ship->TurnFTLDriveOn(whichFTL);
	else
		ship->TurnFTLDriveOff(whichFTL);
}

void DaSentient::DoShipFTLDriveSetting(DaVessel* ship, quint8 setting, quint16 whichFTL)
{
	ship->SetFTLDriveSetting(whichFTL, setting);
}

void DaSentient::DoShipEngageFTLDrive(DaVessel* ship, int announce, const QString& destName, quint16 whichFTL)
{
	if(ship->CanEngageFTLNow(whichFTL))
	{
		ship->EngageFTLDrive(whichFTL);

		if(announce == 1)
		{
			// Announce engaging the FTL drive.
			COMMODITYID ftlModule = ship->SystemModule(SYS_FTLDrive, whichFTL);
			if(DbCommodity::IsWormholeFTLDriveModule(ftlModule))
			{
				QString polarityName;
				switch(ship->FTLDriveSetting(whichFTL))
				{
				case 0:
					polarityName = QObject::tr("positive");
					break;
				case 1:
					polarityName = QObject::tr("negative");
					break;
				case 2:
					polarityName = QObject::tr("neutral");
					break;
				}

				QString text = QObject::tr("Wormhole drive engaged, %1 polarity.").arg(polarityName);
				AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
						FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
				Say(msg, AuVisage::Closed);
			}
			else if(DbCommodity::IsWarpFTLDriveModule(ftlModule))
			{
				QString text;
				if(destName.isEmpty())
					text = QObject::tr("Warp drive engaged, warp factor %1.").arg(ship->FTLDriveSetting(whichFTL)+1);
				else
					text = QObject::tr("Warp drive engaged, warp factor %1, destination %2.")
						.arg(ship->FTLDriveSetting(whichFTL)+1).arg(destName);
				AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
						FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
						AuCommStdChannel::CrewMates);
				Say(msg, AuVisage::Closed);
			}
		}
		else if(announce == 2)
		{
			QString text = QObject::tr("Warp drive engaged, warp factor %1, following %2.")
				.arg(ship->FTLDriveSetting(whichFTL)+1).arg(destName);
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
					AuCommStdChannel::CrewMates);
			Say(msg, AuVisage::Closed);
		}
	}
}

void DaSentient::DoShipDisengageFTLDrive(DaVessel* ship, int announce, quint16 whichFTL, const QString& destName)
{
	if(!ship->CanDisengageFTLNow(whichFTL))
		return;

	ship->DischargeFTLDrive(whichFTL);

	// Announce disengaging the FTL drive.
	if(announce == 1)
	{
		QString text;
		if(destName.isEmpty())
			text = QObject::tr("Warp drive disengaged. Switching to manoeuvre drive.");
		else
			text = QObject::tr("Warp drive disengaged in %1. Switching to manoeuvre drive.").arg(destName);
		AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
				AuCommStdChannel::CrewMates);
		Say(msg, AuVisage::Closed);
	}
	else if(announce == 2)
	{
		QString text;
		if(destName.isEmpty())
			text = QObject::tr("Warp drive disengaged. Make ready for full stop.");
		else
			text = QObject::tr("Warp drive disengaged in %1. Make ready for full stop.").arg(destName);
		AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
				AuCommStdChannel::CrewMates);
		Say(msg, AuVisage::Closed);
	}
	else if(announce == 3)
	{
		QString text;
		if(destName.isEmpty())
			text = QObject::tr("Warp drive disengaged. This could get bumpy.");
		else
			text = QObject::tr("Warp drive disengaged in %1. This could get bumpy.").arg(destName);
		AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
				AuCommStdChannel::CrewMates);
		Say(msg, AuVisage::Grim);
	}
	else if(announce == 4)
	{
		QString text;
		if(destName.isEmpty())
			text = QObject::tr("Warp drive disengaged. Make ready for full stop.");
		else
			text = QObject::tr("Warp drive disengaged in %1. Make ready for full stop.").arg(destName);
		AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
				AuCommStdChannel::CrewMates);
		Say(msg, AuVisage::Closed);
	}
	else if(announce == 5)
	{
		QString text;
		if(destName.isEmpty())
			text = QObject::tr("Warp drive disengaged in proximity to destination. Plotting approach.");
		else
			text = QObject::tr("Warp drive disengaged in proximity to destination in %1. Plotting approach.").arg(destName);
		AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
				AuCommStdChannel::CrewMates);
		Say(msg, AuVisage::Closed);
	}
}

void DaSentient::DoShipEngineTune(DaVessel* ship)
{
	if(ship->GravityDriveState() != DaVessel::GD_Off)
	{
		if(ship->TurnRateSkewX() > 0)
			ship->TurnRateSkewChange(-1,0,5);
		else if(ship->TurnRateSkewX() < 0)
			ship->TurnRateSkewChange(1,0,5);

		if(ship->TurnRateSkewY() > 0)
			ship->TurnRateSkewChange(0,-1,5);
		else if(ship->TurnRateSkewY() < 0)
			ship->TurnRateSkewChange(0,1,5);

		if(ship->AccelSkew() > 0)
			ship->AccelSkewChange(-1,5);
		else if(ship->AccelSkew() < 0)
			ship->AccelSkewChange(1,5);
	}

	if(ship->IsPowerPlantOn())
	{
		if(ship->PowerPlantSkew() > 0)
			ship->PowerPlantSkewChange(-1,5);
		else if(ship->PowerPlantSkew() < 0)
			ship->PowerPlantSkewChange(1,5);
	}
}

void DaSentient::DoShipPowerRelayTune(DaVessel* ship)
{
	// Do just one thing per tune.
	if(ship->TurnOffOneShunt() || ship->ConnectOneSystem() || ship->TurnOffOneOverclock())	
		return;
}

void DaSentient::DoShipKillThrottles(DaVessel* ship)
{
	ship->KillThrottles();
}

void DaSentient::DoShipReloadTurret(DaVessel* ship, quint16 which)
{
	ship->ReloadTurret(which);
}

void DaSentient::DoShipFireTurret(DaVessel* ship, const AuId& target)
{
	if(ship->FireTurret(*this, true, target))
	{
		IncreaseSkill(AuSkill::Turret, 1);
		RestartAIAttackTime();
		UpdateThinkRate();
	}
}

void DaSentient::DoShipInteriorLightState(DaVessel* ship, INTERIOR_LIGHT state, const QString& roomName)
{
	if(ship->InteriorLightToggle(roomName, state))
		ship->SetModified();
}

void DaSentient::DoShipLightState(DaVessel* ship, bool headlight, bool navLight, bool beacon, bool strobe)
{
	bool anyChange = false;
	if(ship->IsHeadlightOn() != headlight)
	{
		ship->HeadlightToggle(headlight);
		anyChange = true;
	}

	if(ship->IsNavLightOn() != navLight)
	{
		ship->NavLightToggle(navLight);
		anyChange = true;
	}

	if(ship->IsRotatingBeaconOn() != beacon)
	{
		ship->RotatingBeaconToggle(beacon);
		anyChange = true;
	}

	if(ship->IsStrobeOn() != strobe)
	{
		ship->StrobeToggle(strobe);
		anyChange = true;
	}

	if(anyChange)
		ship->SetModified();
}

void DaSentient::DoShipArrivedAtDestination(DaVessel* ship, const AuId& destId)
{
	ship->MissionDestinationArrived(destId);
}

void DaSentient::DoShipArrivedAtDestinationStar(DaVessel* ship, const AuId& destId, bool announce, const DaSolarSystem* system,
		const DaStar* primaryStar)
{
	ship->MissionDestinationArrived(destId);

	if(announce)
	{
		AuCommChat msg(QObject::tr("We are near the primary star of %1; type and size is '%2'.")
				.arg(system->NameKnownTo(ship->EmpireId()))
				.arg(primaryStar->Name(primaryStar->StarType(), primaryStar->Size())),
				AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::CrewMates);
		Say(msg, AuVisage::Closed);
	}
}

void DaSentient::DoShipMoorState(DaVessel* ship, bool moor)
{
	if(moor)
		ship->Moor();
	else
		ship->Unmoor();
}

void DaSentient::DoShipSensorsState(DaVessel* ship, bool on)
{
	if(on)
		ship->TurnSensorsOn();
	else
		ship->TurnSensorsOff();
}

void DaSentient::DoShipSensorsLock(DaVessel* ship, const AuId& lockId)
{
	ship->LockOn(lockId, false);
}

void DaSentient::DoShipShieldsState(DaVessel* ship, DaVessel::SHIELD_STATE state, bool announce, const QString& sayWhy)
{
	switch(state)
	{
	case DaVessel::SS_Up:
		ship->SetShieldState(state);
		if(announce)
		{
			// Report changing the shield state.
			QString text = QObject::tr("%1 Raising the shields, sir. I'll do my best to keep them balanced.").arg(sayWhy);
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem,
					AuCommStdChannel::CrewMates);
			Say(msg, AuVisage::Closed);
		}
		break;

	case DaVessel::SS_Down:
		ship->SetShieldState(state);
		if(announce)
		{
			// Report changing the shield state.
			QString text = QObject::tr("%1 Lowering the shields, sir.").arg(sayWhy);
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem,
					AuCommStdChannel::CrewMates);
			Say(msg, AuVisage::Closed);
		}
		break;

	case DaVessel::SS_Off:
		ship->SetShieldState(state);
		if(announce)
		{
			// Report changing the shield state.
			QString text = QObject::tr("%1 Switching shields off, sir.").arg(sayWhy);
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
					FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem,
					AuCommStdChannel::CrewMates);
			Say(msg, AuVisage::Closed);
		}
		break;

	case DaVessel::SS_Invalid:
		break;
	}
}

void DaSentient::DoShipShieldsBalance(DaVessel* ship)
{
	ship->ShieldBalance();
}

bool DaSentient::InConeOfAddress(const DaPerson& talker) const
{
	// Calculate my eye location relative to the talker's eye location.
	AuPoint myEye = EnvTransform().Project(CameraOffset());			// My eye location relative to the environment.
	AuPoint myTalkerEye = talker.EnvTransform().UnProject(myEye);	// My eye location relative to the talker.
	if((talker.CurrentGeneralPose() == AuPose::ReceptiveMate || talker.CurrentGeneralPose() == AuPose::MateOnBottom) &&
			talker.HasDNA3())
		myTalkerEye -= talker.CameraOffset().RotateY(PI);			// My eye location relative to the prone talker's eye.
	else
		myTalkerEye -= talker.CameraOffset();						// My eye location relative to the talker's eye.
	double dist2 = myTalkerEye.Length2();							// Squared distance from talker's eye to my eye.
	if(dist2 > 10*10)
		return false;	// I am farther than 10' from the talker.
	else if(dist2 == 0)
		return true;	// My eye is at the same location as the talker's eye.

	// Check field of view of talker.
	AuPoint lookVec = AuPoint(0,1,0).RotateX(talker.CurrentHeadTilt());
	if(talker.CurrentGeneralPose() == AuPose::ReceptiveMate || talker.CurrentGeneralPose() == AuPose::MateOnBottom)
	{
		if(talker.HasDNA3())
			lookVec = lookVec.RotateY(PI);
		lookVec = lookVec.RotateX(PI/2);
	}
	myTalkerEye /= sqrt(dist2);	// Make it a unit vector.
	double vdot = myTalkerEye.Dot(lookVec);
	if(vdot <= .7071)
		return false;	// I am not in the talker's field of view.

	return true;
}

void DaSentient::StartVoiceConversationRecursive(const DaPerson& talker) const
{
	if(!InConeOfAddress(talker))
		return;

	// I should start listening on the voice channel.
	g_Au.MonitorChannel(AuCommStdChannel::VoiceChannelName(), Id());
	//qDebug() << "DaSentient::StartVoiceConversationRecursive" << Id() << Name() << "distance" << myTalkerLoc.Length()
		//<< "dot" << vdot << "listening";
}

bool DaSentient::HasJobToOffer(DaPerson& person) const
{
	// Do not offer jobs when I am unconscious.
	if(IsBodyDyingOrDead())
		return false;

	// Do not offer jobs when I am a baby.
	if(!IsFullGrown())
		return false;

	// Do not offer jobs to enemy.
	if(PoliticalStance(EmpireId(), person.EmpireId()) == POL_Enemy)
		return false;

	if(!IsJobOfferValid())
		return false;

	// Check to see if the person is already doing the job I am offering.
	m_TalkLock.lockForRead();
	bool hasJobForPerson = !person.HasJob(m_JobOffer);
	if(hasJobForPerson)
	{
		// The person is not already doing the job I am offering.
		const AuJobStory* storyJob = dynamic_cast<const AuJobStory*>(m_JobOffer.Job());
		if(storyJob)
		{
			// It is a story job. Can the person start a nu story?
			hasJobForPerson = person.CanStartStory();
		}
		else
		{
			// It is not a story job. Insure the job offer still exists in the jobs offers queue.
			hasJobForPerson = false;
			const AuJobOffers* jobOffers = g_Au.LockJobOffersForRead();
			for(AuJobOffers::const_iterator joi = jobOffers->cbegin(); joi != jobOffers->cend(); joi++)
			{
				if(m_JobOffer.IsLike(*joi))
				{
					// Found the job in the job offers queue.
					hasJobForPerson = true;
					break;
				}
			}
			g_Au.UnlockJobOffers();
		}
	}
	m_TalkLock.unlock();
	return hasJobForPerson;
}

bool DaSentient::IsJobOfferValid() const
{
	bool jobOfferValid = false;
	m_TalkLock.lockForRead();
	if(!m_JobOffer.IsExpired(AuTime::Now()))
	{
		// Job is not expired.
		// Insure the job is still available in the jobs list.
		const AuJobOffers* jobOffers = g_Au.LockJobOffersForRead();
		for(AuJobOffers::const_iterator joi = jobOffers->cbegin(); joi != jobOffers->cend(); joi++)
		{
			if(m_JobOffer == *joi)
			{
				// The job exists in the job offers list.
				jobOfferValid = true;
				break;
			}
		}
		g_Au.UnlockJobOffers();
	}
	m_TalkLock.unlock();
	return jobOfferValid;
}

// Does nothing if my current job offer has not expired yet.
// Configures my current job offer for a 'person'.
// Returns true if I have a job offer that can be offered to the person.
// Returns false if I have a valid job offer that cannot be offered to the person.
// Returns false if I do not have a job offer.
bool DaSentient::SelectJobOffer(DaPerson& person)
{
	if(InDesignPreview())
		return false;
	// Check to see if my current job offer can be accepted by the person.
	if(HasJobToOffer(person))
		return true;

	// Check to see if my current job offer is valid at all.
	if(IsJobOfferValid())
		return false;	// My valid job offer cannot be offered to the person.

	// My current job offer is not valid. I can get a nu job every few minutes.
	m_TalkLock.lockForRead();
	bool canOfferNewJob = !m_tmLastSpoke.isValid() || m_tmLastSpoke.elapsed()/d_MSecPerMin >= 3;
	m_TalkLock.unlock();
	if(!canOfferNewJob)
		return false;			// Current job offer is invalid and I am not ready to offer a nu job yet.

	// Get a nu job offer.
	const DaSector* sector = FindParentSector();
	if(!sector)
		return false;	// wut?

	// Get a list of my AI controlled peers, so I can choose a job they don't already have.
	DaAtomList sentientPeers;
	DaAtom* parent = Parent();
	DaAtomList subs;
	parent->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if(*it == this)
			continue;	// skip myself
		if(!(*it)->DoesQueryEnvironment())
			continue;
		if((*it)->IsInAtmosphere() != IsInAtmosphere())
			continue;	// skip atoms that are not on the same side of the atmosphere envelope
		if((*it)->EmpireId() != EmpireId())
			continue;	// Skip atoms from other empires.

		DaSentient* peer = dynamic_cast<DaSentient*>(*it);
		if(!peer)
			continue;	// Skip non-sentients.
		if(!peer->IsAIControlled())
			continue;	// Skip people and active demis.
		sentientPeers.append(peer);
	}

	// Create a job offer.
	AuJobOffer jobOffer;

	// Make a list of the story scripts that the player is eligible to start.
	const AuScripts* scripts = g_Au.LockScripts();
	const AuStories* stories = g_Au.LockStoriesForRead();
	QMap<QString, QVariant> vars;
	for(AuScripts::const_iterator it = scripts->begin(); it != scripts->end(); it++)
	{
		// Citizens do not offer jobs that start automatically. They have their own catalyst for starting.
		if((*it).second.AnyAutoStart())
			continue;

		// Insure the player is eligible to start the story.
		vars.clear();
		if(!person.TestStoryCatalyst((*it).second, *stories, 0, vars))
			continue;

		// Create a story job that will be included with the job offer.
		// When AuJobStory is used as an offer, the story script id is substituted for the story instance id.
		AuJobStory story((*it).second.Id(), (*it).second.Title(), (*it).second.Author(), (*it).second.Brief());

		// A story job offer passes the author of the story as the from name. That way story offers sorted by origin get sorted
		// by author name.
		AuJobOffer sinfo((*it).second.Author(), 0, sector->Location(), Galaxy(), &story);

		// Check to see if any of my peers is already offering that job.
		bool peerHasJob = false;
		for(DaAtomList::const_iterator pi = sentientPeers.begin(); pi != sentientPeers.end(); pi++)
		{
			const DaSentient* peer = (const DaSentient*)(*pi);
			if(!peer->IsNormal())
				continue;	// Something happened to the peer.
			if(peer->IsOfferingJob(sinfo))
			{
				peerHasJob = true;
				break;
			}
		}
		if(peerHasJob)
			continue;

		jobOffer = sinfo;
	}
	g_Au.UnlockStories();
	g_Au.UnlockScripts();

	// List job offers that have been posted publicly and that are ok to offer to the person.
	if(!jobOffer.Job())
	{
		// We will search for jobs from closest to farthest, as measured in whole sectors.
		// We want the closest critical job. We will keep the closest non-critical job we find and we will keep searching until we
		// find a critical job or run out of jobs.
		AuTime expiryTest = AuTime::Now().AddMins(3);					// Test against expiration 3 minutes from now.
		const AuJobOffers* jobOffers = g_Au.LockJobOffersForRead();
		for(int searchRange = 0; searchRange < MaxJobSectorRange(); searchRange++)
		{
			for(AuJobOffers::const_iterator ji = jobOffers->cbegin(); ji != jobOffers->cend(); ji++)
			{
				if((*ji).IsExpired(expiryTest))
					continue;											// Job will expire too soon.

				if(Galaxy() != (*ji).FromGalaxy())
					continue;

				int range = (int)ceil(sector->Location().Distance((*ji).FromSectorLoc()) / 10);             // result is in sectors
				if(range != searchRange)
					continue;											// Not the right distance away.

				if(!(*ji).IsOkToOffer(person, range))
					continue;											// Person is not eligible to take the job.

				// Check to see if any of my peers is already offering that job.
				bool peerHasJob = false;
				for(DaAtomList::const_iterator pi = sentientPeers.begin(); pi != sentientPeers.end(); pi++)
				{
					const DaSentient* peer = (const DaSentient*)(*pi);
					if(!peer->IsNormal())
						continue;	// Something happened to the peer.
					if(peer->IsOfferingJob(*ji))
					{
						peerHasJob = true;
						break;
					}
				}
				if(peerHasJob)
					continue;

				if((*ji).IsCritical(searchRange))
				{
					// Found a job that is critical at this range.
					jobOffer = *ji;
					break;	// Stop searching.
				}
				else if(!jobOffer.Job())
					jobOffer = *ji;	// Remember the first/closest non-critical job and keep searching for critical jobs.
			}

			// Stop searching when we find a critical job.
			if(jobOffer.IsCritical(searchRange))
				break;	// A critical job was found.
		}
		g_Au.UnlockJobOffers();
	}

	// Did we find a job to offer?
	if(!jobOffer.Job())
		return false;

	// Choose one of the available job offers.
	m_TalkLock.lockForWrite();
	m_JobOffer = jobOffer;
	m_TalkLock.unlock();

	return HasJobToOffer(person);
}

void DaSentient::ConversationChoicePicked(DaPerson& person, qint8 pickValue)
{
	if(!IsAIControlled())
		return;

	//qDebug() << "DaSentient::ConversationChoicePicked" << Id() << Name() << "pick" << pickValue << "by" << person.Id()
		//<< person.Name();
	if(!IsMarried() && TryMating(person))
	{
		// Person IsReadyToMate, properly posed, and IsSexuallyCompatible.
		QString text = tr("I will be a good %2, %1.")
			.arg(person.Name())
			.arg(MotherFather(false));
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Grin, person.Id());
		DaSentient::TakeOrder(rmsg, ORDER_MountTarget, person.Id(), AuPoint(0,0), true, tr("Have sex with %1.").arg(person.Name()));
		return;
	}

	switch(pickValue)
	{
	case 0:
		// Tell me more, after hearing the teaser.
		if(HasJobToOffer(person))
		{
			m_TalkLock.lockForRead();
			QString text = m_JobOffer.Proposal();
			m_TalkLock.unlock();

			if(text.length() < AuCommChat::MaxChatText())
			{
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(),
						Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Closed, person.Id());
			}
			else
			{
				AuCommChatContinued rmsg(text, AuCommStdChannel::VoiceChannelName(),
						Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Closed, person.Id());
			}

			AuCommChatChoice nomsg(person.Id(), tr("No, thanks."), 1, 3, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(nomsg, person.Id());

			AuCommChatChoice okmsg(person.Id(), tr("I'll do it."), 1, 1, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(okmsg, person.Id());

			AuCommChatChoice continuemsg(person.Id(), tr("What else do you know?"), 1, 4, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(continuemsg, person.Id());
		}
		else
			SayPleasantry(person);
		break;

	case 1:
		// I'll do it, after hearing the proposal.
		if(HasJobToOffer(person))
		{
			m_TalkLock.lockForRead();
			QString text = m_JobOffer.AcceptReply();
			m_TalkLock.unlock();

			AuCommChat pmsg(text, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(pmsg, AuVisage::Closed, person.Id());

			person.JobAccepted(m_JobOffer);
		}
		else
			SayPleasantry(person);
		break;

	case 2:
		// Not interested by the teaser.
		if(HasJobToOffer(person))
		{
			m_TalkLock.lockForRead();
			QString text = m_JobOffer.RejectTeaserReply();
			m_TalkLock.unlock();

			AuCommChat pmsg(text, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(pmsg, AuVisage::Closed, person.Id());
		}
		else
			SayPleasantry(person);
		break;

	case 3:
		// No thanks, after hearing the proposal.
		if(HasJobToOffer(person))
		{
			m_TalkLock.lockForRead();
			QString text = m_JobOffer.RejectProposalReply();
			m_TalkLock.unlock();

			AuCommChat pmsg(text, AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(pmsg, AuVisage::Closed, person.Id());
		}
		else
			SayPleasantry(person);
		break;

	case 4:
		// What else is nu?
		SayPleasantry(person);
		break;

	case 5:
		// Responded to a pleasantry.
		SayJobOffer(person);
		break;

	default:
		{
			AuCommChat rmsg(tr("What?"), AuCommStdChannel::VoiceChannelName(),
					Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Closed, person.Id());
		}
		break;
	}
}

// The item will replace an existing item and cause it to be dropped if it is used for the same purpose. i.e. a
// weapon will cause me to drop my current weapon and use the nu weapon instead.
bool DaSentient::AddGearItemAt(AuGear& gear, AuGearSlot::SLOTTYPE slotType, const AuItem& cItem, bool dropIt)
{
	// Some items are too big to carry.
	if(!cItem.CanBeCarried())
		return false;

	// Insure I am permitted to use that item in that position.
	if(slotType != AuGearSlot::Invalid && !CanUseGear(cItem, slotType))
		slotType = AuGearSlot::Invalid;

	if(slotType == AuGearSlot::Invalid)
	{
		// The item was not placed into a use/wear position.
		AuItem* item = gear.ItemInSlot(AuGearSlot::Body);
		if(item && item->FitItemIntoContainer(cItem))
		{
			UpdateGearWeight();
			return true;
		}

		if(cItem.IsAmmo())
		{
			// Ammo can be stored in one of five possible locations.
			// The most I could possibly carry would be five clips since I will keep my hands free for my weapon and a misc. item.
			// These are the inventory positions that can hold a clip of ammo.
			quint8 waistItems = gear.ItemsInSlot(AuGearSlot::Waist, 0);
			if(waistItems < 4)
				slotType = AuGearSlot::Waist;
			else if(!gear.ItemsInSlot(AuGearSlot::Body, 0))
				slotType = AuGearSlot::Body;
		}
		else
		{
			// Misc. items are stored in the hand.
			slotType = AuGearSlot::Hand;
		}
	}
	else if(slotType == AuGearSlot::Head)
	{
		// Head can wear two items but one must be a helmet and the other must be a hat.
		if(cItem.IsHelmet())
		{
			// Item is a helmet. Cannot wear two helmets.
			if(gear.HelmetItemWorn())
				return false;
		}
		else if(cItem.IsHat())
		{
			// Item is a hat. Cannot wear two hats.
			if(gear.HatItemWorn())
				return false;
		}
		else
			return false;
	}
	else if(slotType == AuGearSlot::BodyWear)
	{
		// Body can wear two items but one must be armor and the other must be clothing.
		if(cItem.IsBodyArmor())
		{
			// Item is body armor. Cannot wear two body armors.
			if(gear.ArmorItemWorn())
				return false;
		}
		else if(cItem.IsClothing())
		{
			// Item is clothing. Cannot wear two clothings.
			if(gear.ClothingItemWorn())
				return false;
		}
		else
			return false;
	}

	// Determine the maximum number of items that can be carried in the slot type.
	int slotMaxItems = 0;
	switch(slotType)
	{
	case AuGearSlot::Invalid:
	case AuGearSlot::Type_Count:
		break;	// No slot was found to carry the item.

	case AuGearSlot::Hand:
		slotMaxItems = 2;
		break;

	case AuGearSlot::Head:
	case AuGearSlot::Face:
	case AuGearSlot::Neck:
	case AuGearSlot::Body:
		slotMaxItems = 1;
		break;

	case AuGearSlot::BodyWear:
		slotMaxItems = 2;
		break;

	case AuGearSlot::Limb:
	case AuGearSlot::Digit:
	case AuGearSlot::Waist:
		slotMaxItems = 4;
		break;
	}

	if(!slotMaxItems)
		return false;

	// Drop current item to free up a slot of that type.
	quint8 curItems = gear.ItemsInSlot(slotType, 0);	// The number of items in the destination slot type.
	while(curItems >= slotMaxItems)
	{
		AuGearPos pos = gear.ItemSlotPos(slotType);
		if(dropIt)
		{
			const AuItem* dropItem = gear.ItemAt(pos);
			if(dropItem)
				DropItem(*dropItem);
		}
		gear.Erase(pos);
		curItems--;
	}

	// Store the nu item in the slot type.
	gear.Insert(slotType, gear.NextWhich(slotType), cItem);
	UpdateGearWeight();
	return true;
}

DaItem* DaSentient::DropGearItem(COMMODITYID cid)
{
	// Can't drop invalid items.
	if(cid <= CID_Invalid)
		return 0;

	AuGearPos pos;
	AuGear* gear = LockGearForWrite();
	if(gear)
	{
		qint8 index = 0;
		for(AuGear::iterator ii = gear->begin(); ii != gear->end(); ii++, index++)
		{
			if((*ii).Item().CommodityId() == cid)
			{
				pos = AuGearPos(*ii);
				break;
			}
		}
	}
	UnlockGear();

	return DropGearAt(pos);
}

DaItem* DaSentient::DropGearAt(const AuGearPos& pos)
{
	// Avoid negative inventory pos values; they can't hold an item anyway.
	if(!pos.IsGear())
		return 0;												// 'pos' cannot be dropped.

	// Locate the item to drop in my gear.
	AuGear* gear = LockGearForWrite();
	if(!gear)
	{
		UnlockGear();
		return 0;
	}

	const AuItem* gearItem = gear->ItemAt(pos);
	if(!gearItem)
	{
		UnlockGear();
		return 0;
	}

	// Create an item atom to encapsulate the gear item.
	DaItem* itemAtom = DropItem(*gearItem);

	// Remove the item from my gear.
	gear->Erase(pos);

	// Update my gear weight.
	UpdateGearWeight();
	UnlockGear();

	return itemAtom;
}

DaItem* DaSentient::DropGearSlot(AuGearSlot::SLOTTYPE slotType)
{
	if(slotType == AuGearSlot::Invalid)
		return 0;

	// Locate the item to drop in my gear.
	AuGear* gear = LockGearForWrite();
	if(!gear)
	{
		UnlockGear();
		return 0;
	}

	AuGearPos pos = gear->ItemSlotPos(slotType);
	const AuItem* gearItem = gear->ItemAt(pos);
	if(!gearItem)
	{
		UnlockGear();
		return 0;
	}

	// Create an item atom to encapsulate the gear item.
	DaItem* itemAtom = DropItem(*gearItem);

	// Remove the item from my gear.
	gear->Erase(pos);

	// Update my gear weight.
	UpdateGearWeight();
	UnlockGear();

	return itemAtom;
}

bool DaSentient::HasItemSpecific(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		const QString& inscription) const
{
	bool foundItem = false;

	const AuGear* gear = LockGearForRead();
	if(gear)
	{
		if(inscription.isEmpty())
			foundItem = gear->HasItem(cid, quality, empireOfOrigin);
		else
			foundItem = gear->HasItem(cid, quality, empireOfOrigin, inscription);
	}
	UnlockGear();

	return foundItem;
}

AuGearPos DaSentient::CurrentItemPos() const
{
	const AuGear* gear = LockGearForRead();
	if(!gear)
	{
		UnlockGear();
		return AuGearPos();
	}

	// AI dudes prefer to hold their weapon as current, if they have one in hand.
	AuGearPos pos = gear->WeaponSlotPos(AuGearSlot::Hand);
	if(!pos.IsGear())
		pos = gear->ItemSlotPos(AuGearSlot::Hand);
	UnlockGear();
	return pos;
}

INVENTORYSUB DaSentient::CurrentItemSub() const
{
	return IPS_Weapon1;
}

void DaSentient::FireWeapon()
{
	AuCombatWeapon weapon(WM_Not);
	int ammoCur, ammoFull;
	AuGearPos gearPos(CurrentItemPos());
	WeaponAtUsePos(gearPos, IPS_Weapon1, weapon, ammoCur, ammoFull);
	//qDebug() << "DaSentient::FireWeapon" << Id() << Name() << "weapon" << weapon.m_w1.m_Effect << "ammo" << ammoCur << ammoFull;

	if(weapon.m_w1.m_Effect == WE_None)
	{
		if(weapon.m_CID > CID_Invalid && ammoCur == 0 && ammoFull != 0)
		{
			// Reload my weapon.
			//qDebug() << "DaSentient::FireWeapon" << Id() << Name() << "reloading weapon";
			AuGear* gear = LockGearForWrite();
			if(gear)
			{
				const AuItem* gearItem = gear->ItemAt(gearPos);
				if(gearItem)
				{
					if(!gearItem->CanFireWeapon(IPS_Weapon1))
					{
						if(gear->ReloadItem(gearPos, IPS_Weapon1))
						{
							UpdateGearWeight();
							SetModified();
						}
					}
				}
			}
			UnlockGear();
			WeaponAtUsePos(gearPos, IPS_Weapon1, weapon, ammoCur, ammoFull);
		}
	}

	if(weapon.m_w1.m_Effect != WE_None)
	{
		//qDebug() << "DaSentient::FireWeapon" << Id() << Name() << "firing weapon" << weapon.m_CID << weapon.m_Mode
			//<< weapon.m_w1.m_Effect;
		AuPoint targetLoc = WeaponMuzzleLoc() + WeaponMuzzleVec();
		UseItemEffect(weapon.m_CID, weapon.m_Mode, weapon.m_w1.m_Effect, weapon.MaxRadius(), IPS_Weapon1, targetLoc);
	}
}

// Looks for a specific item in my inventory associated with a story instance and makes it permanent.
// 'inscription' is only compared if 'inscription' is not blank.
void DaSentient::StoryItemMakePermanent(quint32 storyInstance, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		const QString& inscription)
{
	if(!storyInstance)
		return;

	AuGear* gear = LockGearForWrite();
	if(gear)
	{
		if(inscription.isEmpty())
			gear->StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin);
		else
			gear->StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin, inscription);
	}
	UnlockGear();
}

void DaSentient::StoryItemSetDropOnDeath(int drop, quint32 storyInstance, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		const QString& inscription)
{
	if(!storyInstance)
		return;

	AuGear* gear = LockGearForWrite();
	if(gear)
	{
		if(inscription.isEmpty())
			gear->StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin);
		else
			gear->StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin, inscription);
		UpdateGearWeight();
	}
	UnlockGear();
}

// Used when conducting barter transactions. Removes cItem.Count() items from my inventory.
void DaSentient::RemoveGearItems(const AuItem& cItem)
{
	// Remove the items from my inventory, one by one.
	AuGear* gear = LockGearForWrite();
	if(gear)
	{
		gear->RemoveItems(cItem);
		UpdateGearWeight();
	}
	UnlockGear();
}

bool DaSentient::OkToOrient() const
{
	if(TurnZ() != 0 && CanTurn())
		return false;
	return DaCreature::OkToOrient();
}

int DaSentient::UseItem(AuItem& item, INVENTORYSUB sub)
{
	if(!item.IsValid())
		return -1;

	// If the item in hand is a weapon that needs ammo, expend ammo.
	if(item.IsWeaponThatUsesAmmo(sub) && !item.ExpendAmmo(sub, 1))
		return -2;												// Ammo consuming weapon is out of ammo.

	// At this point, the item has functioned successfully.
	// Remaining tests may consume or wear down the item.

	// Detonator and fuse weapons are dropped, in an armed state, when used.
	if(item.IsDetonatorWeapon() || item.IsFuseWeapon())
		return 0;												// Call it good. Don't use it up; it will be dropped.

	// Some items are consumed whenever they are used.
	if(item.CanBeEaten(EcoRole()) ||							// Food and liquor is consumed when used/eaten.
			item.CommodityId() == CID_Flag ||					// Flags are consumed to when a world is claimed.
			item.CommodityId() == CID_Seed)						// Seeds are consumed when planted in pots.

	{
		// Item is consumed.
		item.Consume(1);										// Consume one of the components.
		return 1;
	}

	// Test the item for being used up due to wear.
	quint8 condition = item.HitPoints();
	if(!item.UseItem())											// Item is consumed when usage test fails, all used up.
	{
		// Item usage test failed. Use up one of the items, possibly the only one. Assume it worked before it failed.
		item.Consume(1);										// Consume one of the components.
		return 1;
	}

	// Caller may need to react when the condition of the item changes.
	if(item.HitPoints() != condition)
		return 2;												// Item condition degraded.

	// The item functioned successfully with no complications.
	return 0;
}

void DaSentient::DoOrderTransferCargoFrom(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	COMMODITYID cid = ship->MissionOrderCommodityId();

	if(cid == CID_Hydrogen)
	{
		// Fuel is stored in the ship's fuel cells.
		// Determine the amount of fuel to transfer.
		quint32 amt;
		if(ship->MissionOrderAmountType())
			amt = ship->MissionOrderAmount();
		else
			amt = ship->MissionOrderAmount() * (otherShip->FuelQuantity()-1) / 100;
		if(amt > otherShip->FuelQuantity()-1)
			amt = otherShip->FuelQuantity()-1;
		if(amt > ship->FuelWanted())
			amt = ship->FuelWanted();
		if(!amt)
			return;

		// Transfer the fuel.
		AuItem* fuelItem = AuItem::Create(CID_Hydrogen, otherShip->FuelQuality(), otherShip->EmpireId(), amt);

		if(otherShip->RemoveCargo(*fuelItem, amt))
			ship->AddCargo(*fuelItem);

		delete fuelItem;
	}
	else if(cid == CID_Electricity)
	{
		// Electricity is stored in the ship's capacitor.
		// Determine the amount of electricity to transfer.
		quint32 amt;
		if(ship->MissionOrderAmountType())
			amt = ship->MissionOrderAmount();
		else
			amt = ship->MissionOrderAmount() * (otherShip->CapacitorLevel()-1) / 100;
		if(amt > otherShip->CapacitorLevel()-1)
			amt = otherShip->CapacitorLevel()-1;
		float capWanted = ship->CapacitorMax() - ship->CapacitorLevel();
		if(amt > capWanted)
			amt = capWanted;
		if(!amt)
			return;

		// Transfer the electricity.
		AuItem* fuelItem = AuItem::Create(CID_Electricity, 255, otherShip->EmpireId(), amt);

		if(otherShip->RemoveCargo(*fuelItem, amt))
			ship->AddCargo(*fuelItem);

		delete fuelItem;
	}
	else
	{
		// Cargo is stored in the ship's hold.
		// Determine the amount of cargo to transfer.
		quint32 amt;
		quint32 cnt = otherShip->CargoRawCount(cid);
		if(ship->MissionOrderAmountType())
			amt = ship->MissionOrderAmount();
		else
			amt = ship->MissionOrderAmount() * cnt / 100;
		if(amt > cnt)
			amt = cnt;
		quint32 spc = ship->CargoCapacityVolume() - ship->CargoTotalVolume();
		if(amt > spc)
			amt = spc;
		if(!amt)
			return;

		// Transfer the cargo.
		AuItemHash cargo = otherShip->LockCargoForRead();
		otherShip->UnlockCargo();
		bool xfer;
		do
		{
			xfer = false;
			AuItemHash::iterator ci = cargo.FindBest(cid);
			if(ci != cargo.end())
			{
				if((*ci).second->Count() > amt)
					(*ci).second->SetCount(amt);

				if(otherShip->RemoveCargo(*(*ci).second, (*ci).second->Count()))
					ship->AddCargo(*(*ci).second);	// Add the cargo to the other ship.

				amt -= (*ci).second->Count();
				cargo.erase(ci);

				xfer = true;
			}
		}
		while(amt && xfer);
	}
}

void DaSentient::DoOrderTransferCargoTo(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	COMMODITYID cid = ship->MissionOrderCommodityId();

	if(cid == CID_Hydrogen)
	{
		// Fuel is stored in the ship's fuel cells.
		// Determine the amount of fuel to transfer.
		quint32 amt;
		if(ship->MissionOrderAmountType())
			amt = ship->MissionOrderAmount();
		else
			amt = ship->MissionOrderAmount() * (ship->FuelQuantity()-1) / 100;
		if(amt > ship->FuelQuantity()-1)
			amt = ship->FuelQuantity()-1;
		if(amt > otherShip->FuelWanted())
			amt = otherShip->FuelWanted();
		if(!amt)
			return;

		// Transfer the fuel.
		AuItem* fuelItem = AuItem::Create(CID_Hydrogen, ship->FuelQuality(), ship->EmpireId(), amt);

		if(ship->RemoveCargo(*fuelItem, amt))
			otherShip->AddCargo(*fuelItem);

		delete fuelItem;
	}
	else if(cid == CID_Electricity)
	{
		// Electricity is stored in the ship's capacitor.
		// Determine the amount of electricity to transfer.
		quint32 amt;
		if(ship->MissionOrderAmountType())
			amt = ship->MissionOrderAmount();
		else
			amt = ship->MissionOrderAmount() * (ship->CapacitorLevel()-1) / 100;
		if(amt > ship->CapacitorLevel()-1)
			amt = ship->CapacitorLevel()-1;
		float capWanted = otherShip->CapacitorMax() - otherShip->CapacitorLevel();
		if(amt > capWanted)
			amt = capWanted;
		if(!amt)
			return;

		// Transfer the electricity.
		AuItem* fuelItem = AuItem::Create(CID_Electricity, 255, ship->EmpireId(), amt);

		if(ship->RemoveCargo(*fuelItem, amt))
			otherShip->AddCargo(*fuelItem);

		delete fuelItem;
	}
	else
	{
		// Cargo is stored in the ship's hold.
		// Determine the amount of cargo to transfer.
		quint32 amt;
		quint32 cnt = ship->CargoRawCount(cid);
		if(ship->MissionOrderAmountType())
			amt = ship->MissionOrderAmount();
		else
			amt = ship->MissionOrderAmount() * cnt / 100;
		if(amt > cnt)
			amt = cnt;
		quint32 spc = otherShip->CargoCapacityVolume() - otherShip->CargoTotalVolume();
		if(amt > spc)
			amt = spc;
		if(!amt)
			return;

		// Transfer the cargo.
		AuItemHash cargo = ship->LockCargoForRead();
		ship->UnlockCargo();
		bool xfer;
		do
		{
			xfer = false;
			AuItemHash::iterator ci = cargo.FindBest(cid);
			if(ci != cargo.end())
			{
				if((*ci).second->Count() > amt)
					(*ci).second->SetCount(amt);

				if(ship->RemoveCargo(*(*ci).second, (*ci).second->Count()))
					otherShip->AddCargo(*(*ci).second);	// Add the cargo to the other ship.

				amt -= (*ci).second->Count();
				cargo.erase(ci);

				xfer = true;
			}
		}
		while(amt && xfer);
	}
}

void DaSentient::DoOrderTransferCargoCategoryFrom(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	COMMODITYFILTER cargoFilter = ship->MissionOrderCommodityFilter();
	QList<COMMODITYID> commodities;
	g_Au.m_CommodityArray.ListFilterCommodities(commodities, cargoFilter);
	if(commodities.isEmpty())
		return;

	quint32 totake = ship->MissionOrderAmount();	// Could be a literal quantity or a % (1-100).
	for(QList<COMMODITYID>::const_iterator ci = commodities.cbegin(); ci != commodities.cend() && totake; ci++)
	{
		COMMODITYID cid = *ci;
		if(cid == CID_Hydrogen)
		{
			// Fuel is stored in the ship's fuel cells.
			// Determine the amount of fuel to transfer.
			quint32 amt;
			if(ship->MissionOrderAmountType())
				amt = totake;
			else
				amt = totake * (otherShip->FuelQuantity()-1) / 100;
			if(amt > otherShip->FuelQuantity()-1)
				amt = otherShip->FuelQuantity()-1;
			if(amt > ship->FuelWanted())
				amt = ship->FuelWanted();
			if(!amt)
				return;

			// Transfer the fuel.
			AuItem* fuelItem = AuItem::Create(CID_Hydrogen, otherShip->FuelQuality(), otherShip->EmpireId(), amt);

			if(otherShip->RemoveCargo(*fuelItem, amt))
			{
				ship->AddCargo(*fuelItem);

				// When taking a literal quantity, reduce the amount remaining to take.
				if(ship->MissionOrderAmountType() == 1)
					totake -= amt;
			}

			delete fuelItem;
		}
		else if(cid == CID_Electricity)
		{
			// Electricity is stored in the ship's capacitor.
			// Determine the amount of electricity to transfer.
			quint32 amt;
			if(ship->MissionOrderAmountType())
				amt = totake;
			else
				amt = totake * (otherShip->CapacitorLevel()-1) / 100;
			if(amt > otherShip->CapacitorLevel()-1)
				amt = otherShip->CapacitorLevel()-1;
			float capWanted = ship->CapacitorMax() - ship->CapacitorLevel();
			if(amt > capWanted)
				amt = capWanted;
			if(!amt)
				return;

			// Transfer the electricity.
			AuItem* fuelItem = AuItem::Create(CID_Electricity, 255, otherShip->EmpireId(), amt);

			if(otherShip->RemoveCargo(*fuelItem, amt))
			{
				ship->AddCargo(*fuelItem);

				// When taking a literal quantity, reduce the amount remaining to take.
				if(ship->MissionOrderAmountType() == 1)
					totake -= amt;
			}

			delete fuelItem;
		}
		else
		{
			// Cargo is stored in the ship's hold.
			// Determine the amount of cargo to transfer.
			quint32 amt;
			quint32 cnt = otherShip->CargoRawCount(cid);
			if(ship->MissionOrderAmountType())
				amt = totake;
			else
				amt = totake * cnt / 100;
			if(amt > cnt)
				amt = cnt;
			quint32 spc = ship->CargoCapacityVolume() - ship->CargoTotalVolume();
			if(amt > spc)
				amt = spc;
			if(!amt)
				return;

			// Transfer the cargo.
			AuItemHash cargo = otherShip->LockCargoForRead();
			otherShip->UnlockCargo();
			bool xfer;
			do
			{
				xfer = false;
				AuItemHash::iterator ci = cargo.FindBest(cid);
				if(ci != cargo.end())
				{
					if((*ci).second->Count() > amt)
						(*ci).second->SetCount(amt);

					if(otherShip->RemoveCargo(*(*ci).second, (*ci).second->Count()))
						ship->AddCargo(*(*ci).second);	// Add the cargo to the other ship.

					amt -= (*ci).second->Count();
					if(ship->MissionOrderAmountType() == 1)
						totake -= (*ci).second->Count();
					cargo.erase(ci);

					xfer = true;
				}
			}
			while(amt && xfer);
		}
	}
}

void DaSentient::DoOrderTransferCargoCategoryTo(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	COMMODITYFILTER cargoFilter = ship->MissionOrderCommodityFilter();
	QList<COMMODITYID> commodities;
	g_Au.m_CommodityArray.ListFilterCommodities(commodities, cargoFilter);
	if(commodities.isEmpty())
		return;

	quint32 tosend = ship->MissionOrderAmount();	// Could be a literal quantity or a % (1-100).
	for(QList<COMMODITYID>::const_iterator ci = commodities.cbegin(); ci != commodities.cend() && tosend; ci++)
	{
		COMMODITYID cid = *ci;
		if(cid == CID_Hydrogen)
		{
			// Fuel is stored in the ship's fuel cells.
			// Determine the amount of fuel to transfer.
			quint32 amt;
			if(ship->MissionOrderAmountType())
				amt = tosend;
			else
				amt = tosend * (ship->FuelQuantity()-1) / 100;
			if(amt > ship->FuelQuantity()-1)
				amt = ship->FuelQuantity()-1;
			if(amt > otherShip->FuelWanted())
				amt = otherShip->FuelWanted();
			if(!amt)
				return;

			// Transfer the fuel.
			AuItem* fuelItem = AuItem::Create(CID_Hydrogen, ship->FuelQuality(), ship->EmpireId(), amt);

			if(ship->RemoveCargo(*fuelItem, amt))
			{
				otherShip->AddCargo(*fuelItem);

				// When sending a literal quantity, reduce the amount remaining to send.
				if(ship->MissionOrderAmountType() == 1)
					tosend -= amt;
			}

			delete fuelItem;
		}
		else if(cid == CID_Electricity)
		{
			// Electricity is stored in the ship's capacitor.
			// Determine the amount of electricity to transfer.
			quint32 amt;
			if(ship->MissionOrderAmountType())
				amt = tosend;
			else
				amt = tosend * (ship->CapacitorLevel()-1) / 100;
			if(amt > ship->CapacitorLevel()-1)
				amt = ship->CapacitorLevel()-1;
			float capWanted = otherShip->CapacitorMax() - otherShip->CapacitorLevel();
			if(amt > capWanted)
				amt = capWanted;
			if(!amt)
				return;

			// Transfer the electricity.
			AuItem* fuelItem = AuItem::Create(CID_Electricity, 255, ship->EmpireId(), amt);

			if(ship->RemoveCargo(*fuelItem, amt))
			{
				otherShip->AddCargo(*fuelItem);

				// When sending a literal quantity, reduce the amount remaining to send.
				if(ship->MissionOrderAmountType() == 1)
					tosend -= amt;
			}

			delete fuelItem;
		}
		else
		{
			// Cargo is stored in the ship's hold.
			// Determine the amount of cargo to transfer.
			quint32 amt;
			quint32 cnt = ship->CargoRawCount(cid);
			if(ship->MissionOrderAmountType())
				amt = tosend;
			else
				amt = tosend * cnt / 100;
			if(amt > cnt)
				amt = cnt;
			quint32 spc = otherShip->CargoCapacityVolume() - otherShip->CargoTotalVolume();
			if(amt > spc)
				amt = spc;
			if(!amt)
				return;

			// Transfer the cargo.
			AuItemHash cargo = ship->LockCargoForRead();
			ship->UnlockCargo();
			bool xfer;
			do
			{
				xfer = false;
				AuItemHash::iterator ci = cargo.FindBest(cid);
				if(ci != cargo.end())
				{
					if((*ci).second->Count() > amt)
						(*ci).second->SetCount(amt);

					if(ship->RemoveCargo(*(*ci).second, (*ci).second->Count()))
						otherShip->AddCargo(*(*ci).second);	// Add the cargo to the other ship.

					amt -= (*ci).second->Count();
					if(ship->MissionOrderAmountType() == 1)
						tosend -= (*ci).second->Count();
					cargo.erase(ci);

					xfer = true;
				}
			}
			while(amt && xfer);
		}
	}
}

void DaSentient::DoOrderTransferMoneyFrom(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	// Determine the amount of money to transfer.
	double money;
	if(ship->MissionOrderAmountType())
		money = ship->MissionOrderAmountOfMoney();
	else
		money = ship->MissionOrderAmount() * otherShip->AccountBalance() / 100.;
	if(money > otherShip->AccountBalance())
		money = otherShip->AccountBalance();
	if(money <= 0)
		return;

	// Transfer the money.
	otherShip->DebitAccount(money);
	ship->CreditAccount(money);
}

void DaSentient::DoOrderTransferMoneyTo(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	// Determine the amount of money to transfer.
	double money;
	if(ship->MissionOrderAmountType())
		money = ship->MissionOrderAmountOfMoney();
	else
		money = ship->MissionOrderAmount() * ship->AccountBalance() / 100.;
	if(money > ship->AccountBalance())
		money = ship->AccountBalance();
	if(money <= 0)
		return;

	// Transfer the money.
	ship->DebitAccount(money);
	otherShip->CreditAccount(money);
}

void DaSentient::DoOrderTransferPeopleFrom(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	// Locate the people available for transfer.
	DaAtomList people;
	quint8 peopleType = ship->MissionOrderPeopleType();
	quint32 berthsAvailable;
	AuRosterEntry::TYPE berthType;
	switch(peopleType)
	{
	case 0:	// officers
		berthsAvailable = ship->OfficerBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::OfficerAI;
		otherShip->FindOfficersRecursive(otherShip->Id(), people);
		break;
	case 1:	// crew
		berthsAvailable = ship->CrewBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::Crew;
		otherShip->FindCrewRecursive(otherShip->Id(), people);
		break;
	case 2:	// troops
		berthsAvailable = ship->TroopBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::Troop;
		otherShip->FindTroopsRecursive(otherShip->Id(), people);
		break;
	case 3:	// passengers
		berthsAvailable = ship->PassengerBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::PassengerAI;
		otherShip->FindPassengersRecursive(otherShip->Id(), people);
		break;
	default:
		return;
	}
	if(people.isEmpty())
		return;

	// Determine the number of people to transfer.
	quint32 amt;
	if(ship->MissionOrderAmountType())
		amt = ship->MissionOrderAmount();
	else
		amt = ship->MissionOrderAmount() * (quint32)people.size() / 100;
	if(amt > (quint32)people.size())
		amt = (quint32)people.size();
	if(amt > berthsAvailable)
		amt = berthsAvailable;
	if(!amt)
		return;

	// Transfer the people.
	for(DaAtomList::const_iterator pi = people.cbegin(); pi != people.cend() && amt; pi++, amt--)
	{
		DaCitizen* citizen = dynamic_cast<DaCitizen*>(*pi);
		if(!citizen)
			continue;

		// Change the citizen's berth assignment from the old ship to the nu ship.
		otherShip->RemoveBerthOccupant(citizen->Id());
		ship->OccupyBerth(berthType, citizen->Id(), citizen->Name());
		citizen->AssignToShip(ship->Id(), berthType);
		citizen->ReleaseTheCon();

		// Physically move the citizen to their berth on the nu ship.
		AuMatrix4x4 t;
		t.Translate(ship->BerthLocation(citizen->Id()) + AuPoint(0,0,citizen->CGHeightStanding()));
		citizen->ChangePosAndOrient(AuAtomPos(ship->Id(), t));
		citizen->InvalidateEnvironment();
		citizen->Stand();
		citizen->GoToWork();
		citizen->SetModified();
	}
}

void DaSentient::DoOrderTransferPeopleTo(DaVessel* ship)
{
	// Locate the target of the transfer order.
	DaVessel* otherShip = ship->MissionOrderTransferTarget(Id());
	if(!otherShip)
		return;

	// Locate the people available for transfer.
	DaAtomList people;
	quint8 peopleType = ship->MissionOrderPeopleType();
	quint32 berthsAvailable;
	AuRosterEntry::TYPE berthType;
	switch(peopleType)
	{
	case 0:	// officers
		berthsAvailable = otherShip->OfficerBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::OfficerAI;
		ship->FindOfficersRecursive(ship->Id(), people);
		break;
	case 1:	// crew
		berthsAvailable = otherShip->CrewBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::Crew;
		ship->FindCrewRecursive(ship->Id(), people);
		break;
	case 2:	// troops
		berthsAvailable = otherShip->TroopBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::Troop;
		ship->FindTroopsRecursive(ship->Id(), people);
		break;
	case 3:	// passengers
		berthsAvailable = otherShip->PassengerBerthsAvailable();
		if(!berthsAvailable)
			return;
		berthType = AuRosterEntry::PassengerAI;
		ship->FindPassengersRecursive(ship->Id(), people);
		break;
	default:
		return;
	}
	if(people.isEmpty())
		return;

	// Determine the number of people to transfer.
	quint32 amt;
	if(ship->MissionOrderAmountType())
		amt = ship->MissionOrderAmount();
	else
		amt = ship->MissionOrderAmount() * (quint32)people.size() / 100;
	if(amt > (quint32)people.size())
		amt = (quint32)people.size();
	if(amt > berthsAvailable)
		amt = berthsAvailable;
	if(!amt)
		return;

	// Transfer the people.
	for(DaAtomList::const_iterator pi = people.cbegin(); pi != people.cend() && amt; pi++, amt--)
	{
		DaCitizen* citizen = dynamic_cast<DaCitizen*>(*pi);
		if(!citizen)
			continue;

		// Change the citizen's berth assignment from the old ship to the nu ship.
		ship->RemoveBerthOccupant(citizen->Id());
		otherShip->OccupyBerth(berthType, citizen->Id(), citizen->Name());
		citizen->AssignToShip(otherShip->Id(), berthType);
		citizen->ReleaseTheCon();

		// Physically move the citizen to their berth on the nu ship.
		AuMatrix4x4 t;
		t.Translate(otherShip->BerthLocation(citizen->Id()) + AuPoint(0,0,citizen->CGHeightStanding()));
		citizen->ChangePosAndOrient(AuAtomPos(otherShip->Id(), t));
		citizen->InvalidateEnvironment();
		citizen->Stand();
		citizen->GoToWork();
		citizen->SetModified();
	}
}

void DaSentient::ClearRouteSentientLocked()
{
	if(m_AIScRoute)
		m_AIScRoute->clear();
}

bool DaSentient::IsRouteEmpty() const
{
	bool v = true;
	m_SentientLock.lockForRead();
	if(m_AIScRoute && !m_AIScRoute->empty())
		v = false;
	m_SentientLock.unlock();
	return v;
}

bool DaSentient::IsRouteDistanceLessThan(double closeEnough) const
{
	bool v = false;
	m_SentientLock.lockForRead();
	if(m_AIScRoute && m_AIScRoute->DistanceLessThan(Location(), closeEnough))
		v = true;
	m_SentientLock.unlock();
	return v;
}

FLIGHT_ATTITUDE DaSentient::FlightAttitude(const DaVessel* ship) const
{
	// Returns upright in atmosphere.
	if(ship->IsInAtmosphere())
		return FLY_Upright;

	switch(ship->PilotUpPolicy())
	{
	default:
	case UpAwayFromGravity:
		return FLY_Upright;

	case UpTowardGravity:
		return FLY_Inverted;

	case PortTowardGravity:
		return FLY_LeftSide;

	case StarboardTowardGravity:
		return FLY_RightSide;
	}
}

void DaSentient::DoOrderReport(DaVessel* ship)
{
	const DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;	// wut?
	const DaSector* sector = env->FindParentSector();
	if(!sector)
		return;	// wut?

	const AuMissionOrder& order = ship->LockMissionOrder();
	QString channel(order.CommChannel());
	AuCommStdChannel::RANGE range = order.CommRange();
	quint16 reportWhat = order.ReportWhat();
	QString sayWhat = order.SayText();
	ship->UnlockMissionOrder();

	QString text;
	if(!sayWhat.isEmpty())
		text = sayWhat;
	else
		text = QObject::tr("%1 reports as ordered.").arg(ship->NameWithEmpire());

	if(reportWhat & 0x0001)
		text += " " + ship->CommMessageLocation();

	if(reportWhat & 0x0002)
	{
		// Take roll call and check everybody's health.
		DaAtomList crewMembers;
		ship->FindCrewRecursive(ship->Id(), crewMembers);
		unsigned int crewSize = (unsigned int)crewMembers.size();
		ship->FindTroopsRecursive(ship->Id(), crewMembers);
		unsigned int troopSize = (unsigned int)crewMembers.size() - crewSize;

		// Report staffing.
		if(crewSize < ship->CrewBerths() && troopSize < ship->TroopBerths())
			text += QObject::tr(" We are short %1 of our %2 crew and %3 of our %4 troops.")
				.arg(ship->CrewBerths() - crewSize)
				.arg(ship->CrewBerths())
				.arg(ship->TroopBerths() - troopSize)
				.arg(ship->TroopBerths());
		else if(crewSize < ship->CrewBerths() && troopSize)
			text += QObject::tr(" We are short %1 of our %2 crew but we've got a full complement of %3 troops.")
				.arg(ship->CrewBerths() - crewSize)
				.arg(ship->CrewBerths())
				.arg(troopSize);
		else if(crewSize < ship->CrewBerths())
			text += QObject::tr(" We are short %1 of our %2 crew.")
				.arg(ship->CrewBerths() - crewSize)
				.arg(ship->CrewBerths());
		else if(troopSize < ship->TroopBerths())
			text += QObject::tr(" We have a full complement of %1 crew but we are short %2 of our %3 troops.")
				.arg(ship->CrewBerths())
				.arg(ship->TroopBerths() - troopSize)
				.arg(ship->TroopBerths());
		else if(troopSize)
			text += QObject::tr(" We have a full complement of %1 crew and %2 troops.")
				.arg(ship->CrewBerths())
				.arg(ship->TroopBerths());
		else
			text += QObject::tr(" We have a full complement of %1 crew.")
				.arg(ship->CrewBerths());

		// Report health.
		if(ship->HasWorkingSurgeryUnit())
		{
			// The ship has a sick bay.
			ship->FindOfficersRecursive(ship->Id(), crewMembers);
			int worstHealth = 100;
			bool anySick = false;
			for(DaAtomList::const_iterator ci = crewMembers.cbegin(); ci != crewMembers.cend(); ci++)
			{
				const DaCreature* crew = dynamic_cast<const DaCreature*>(*ci);
				if(!crew)
					continue;	// wut?
				int health = Percent(crew->CurrentHitPoints(), crew->MaxHitPoints());
				if(health < worstHealth)
					worstHealth = health;
				if(crew->HasDisease())
					anySick = true;
			}

			if(anySick && worstHealth < 90)
				text += QObject::tr(" Some kind of space germ hit the crew hard. The doc looks worried. ");
			else if(anySick)
				text += QObject::tr(" A minor illness is spreading among the crew. Doc thinks he can handle it. ");
			else if(worstHealth < 90)
				text += QObject::tr(" Doc says there are injuries among the crew, just cuts and bruises; they can still work. ");
			else
				text += QObject::tr(" The doc says everybody is healthy. ");
		}
	}

	if(reportWhat & 0x0004)
		text += QObject::tr(" Fuel level is at %1%.").arg(ship->FuelPercent());

	if(reportWhat & 0x0008)
	{
		unsigned int cur = ship->CargoTotalVolume();
		unsigned int cap = ship->CargoCapacityVolume();
		quint8 cargoPerc = Percent(cur, cap);
		text += QObject::tr(" Hold is at %3%, with %L1 units of cargo and room for a total of %L2.")
			.arg(cur)
			.arg(cap)
			.arg(cargoPerc);
	}

	if(reportWhat & 0x0010)
	{
		unsigned int maxHits = ship->DesignHitsStructural();
		unsigned int dmgHits = ship->HitsDamageStructural();
		unsigned int curHits;
		if(dmgHits < maxHits)
			curHits = maxHits - dmgHits;
		else
			curHits = 0;
		QString worst = ship->WorstDamagedSystem();
		if(curHits < maxHits && !worst.isEmpty())
		{
			quint8 healthPerc = Percent(curHits, maxHits);
			unsigned int worstPerc = 100 - ship->WorstSystemDamage();
			text += QObject::tr(" Structure is %1%; %2 is at %3%.")
				.arg(healthPerc)
				.arg(worst)
				.arg(worstPerc);
		}
		else if(curHits < maxHits)
		{
			quint8 healthPerc = Percent(curHits, maxHits);
			text += QObject::tr(" Structure is %1%; all systems are nominal.")
				.arg(healthPerc);
		}
		else if(!worst.isEmpty())
		{
			unsigned int worstPerc = 100 - ship->WorstSystemDamage();
			text += QObject::tr(" Structure is 100%; %2 is at %3%.")
				.arg(worst)
				.arg(worstPerc);
		}
		else
			text += QObject::tr(" Structure is 100%; all systems are nominal.");
	}

	if(reportWhat & 0x0020)
	{
		// Take a quick sensor scan and tell them what we see.
		AuSensorBlipList sensorBlips;
		ship->PerformSensorConsoleScan(sensorBlips);
		int enemyCities = 0;
		int enemyShips = 0;
		int enemyStations = 0;
		int neutralCities = 0;
		int neutralShips = 0;
		int neutralStations = 0;
		int friendlyCities = 0;
		int friendlyShips = 0;
		int friendlyStations = 0;
		for(AuSensorBlipList::const_iterator bi = sensorBlips.cbegin(); bi != sensorBlips.cend(); bi++)
		{
			if(ship->Is((*bi)->AtomId()))
				continue;	// Don't include myself.

			if((*bi)->IsCity())
			{
				if((*bi)->PoliticalStance() == POL_Enemy)
					enemyCities++;
				else if((*bi)->PoliticalStance() == POL_Neutral)
					neutralCities++;
				else
					friendlyCities++;
			}
			else if((*bi)->IsSpaceStation())
			{
				if((*bi)->PoliticalStance() == POL_Enemy)
					enemyStations++;
				else if((*bi)->PoliticalStance() == POL_Neutral)
					neutralStations++;
				else
					friendlyStations++;
			}
			else if((*bi)->IsSpacecraft())
			{
				if((*bi)->PoliticalStance() == POL_Enemy)
					enemyShips++;
				else if((*bi)->PoliticalStance() == POL_Neutral)
					neutralShips++;
				else
					friendlyShips++;
			}
		}

		if(enemyCities || enemyShips || enemyStations ||
				neutralCities || neutralShips || neutralStations ||
				friendlyCities || friendlyShips || friendlyStations)
		{
			// Make a list of the things we are tracking.
			QStringList what;
			if(enemyStations == 1)
				what << QObject::tr("one enemy station");
			else if(enemyStations)
				what << QObject::tr("%1 enemy stations").arg(enemyStations);

			if(enemyShips == 1)
				what << QObject::tr("one enemy spacecraft");
			else if(enemyShips)
				what << QObject::tr("%1 enemy spacecraft").arg(enemyShips);

			if(enemyCities == 1)
				what << QObject::tr("one enemy city");
			else if(enemyCities)
				what << QObject::tr("%1 enemy cities").arg(enemyCities);

			if(friendlyStations == 1)
				what << QObject::tr("one friendly station");
			else if(friendlyStations)
				what << QObject::tr("%1 friendly stations").arg(friendlyStations);

			if(friendlyShips == 1)
				what << QObject::tr("one friendly spacecraft");
			else if(friendlyShips)
				what << QObject::tr("%1 friendly spacecraft").arg(friendlyShips);

			if(friendlyCities == 1)
				what << QObject::tr("one friendly city");
			else if(friendlyCities)
				what << QObject::tr("%1 friendly cities").arg(friendlyCities);

			if(neutralStations == 1)
				what << QObject::tr("one neutral station");
			else if(neutralStations)
				what << QObject::tr("%1 neutral stations").arg(neutralStations);

			if(neutralShips == 1)
				what << QObject::tr("one neutral spacecraft");
			else if(neutralShips)
				what << QObject::tr("%1 neutral spacecraft").arg(neutralShips);

			if(neutralCities == 1)
				what << QObject::tr("one neutral city");
			else if(neutralCities)
				what << QObject::tr("%1 neutral cities").arg(neutralCities);

			// Build the message.
			if(what.size() == 1)
				text += QObject::tr(" Sensors are tracking %1.").arg(what.first());
			else if(what.size() == 2)
				text += QObject::tr(" Sensors are tracking %1 and %2.").arg(what.first()).arg(what.last());
			else
			{
				text += QObject::tr(" Sensors are tracking ");
				for(QStringList::const_iterator wi = what.cbegin(); wi != what.cend(); wi++)
				{
					if(wi == what.cbegin())
						text += *wi;
					else if(wi < what.cend() - 1)
						text += QObject::tr(", %1").arg(*wi);
					else
						text += QObject::tr(", and %1.").arg(*wi);
				}
			}
		}
		else
			text += QObject::tr(" Sensors are clear of cities and spacecraft.");
	}

	AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(channel);
	if(channel == ship->Name())
		scope = AuCommStdChannel::Fleet;
	AuCommHailReply msg(text, env->Id(), channel, Id(), Name(), sector->Id(), Galaxy(), range, scope);
	if(scope == AuCommStdChannel::CrewMates)
		msg.SetScopeData(ship->Id().GetID());					// Required for CrewMates message to be valid.
	else
		msg.SetScopeData(ship->EmpireId());						// All other scopes that care want empire id.
	g_Au.SendAtomChat(msg);
}

bool DaSentient::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	if(IsTurretPose() && m_Effects)
	{
		// My effects are needed to see gun fire from my turret.
		m_EffectsLock.lockForRead();
		bool e = m_Effects && !m_Effects->m_Effects.empty();
		m_EffectsLock.unlock();
		const DaAtom* parent = Parent();
		if(e && parent && parent->IsInScene(sceneAtom, cameraEnvLoc))
			return true;	// I am a gunner with effects and my parent atom is in the scene.
	}

	// I only need to be available for receiving orders when AI controlled.
	if(!IsAIControlled())
		return DaCreature::IsInScene(sceneAtom, cameraEnvLoc);

	// Check to see if we are assigned to the same ship.
	if(AssignedShip().IsNull() || AssignedShip() != sceneAtom->AssignedShip())
		return DaCreature::IsInScene(sceneAtom, cameraEnvLoc);

	// We are assigned to the same ship. Are we both aboard it?
	const DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return DaCreature::IsInScene(sceneAtom, cameraEnvLoc);	// wut?

	// Is our ship subordinate to the same environment?
	const DaAtom* vessel = env->FindIdRecursive(AssignedShip());
	if(!vessel)
		return DaCreature::IsInScene(sceneAtom, cameraEnvLoc);	// Ship is not in the environment.

	// Am I aboard our ship?
	if(!vessel->FindIdRecursive(Id()))
		return DaCreature::IsInScene(sceneAtom, cameraEnvLoc);	// I am not aboard our ship.

	// Is the person aboard our ship?
	if(!vessel->FindIdRecursive(sceneAtom->Id()))
		return DaCreature::IsInScene(sceneAtom, cameraEnvLoc);	// The person is not aboard our ship.

	// I am aboard the same ship as the scene atom and we are both assigned to that ship.
	return true;
}

void DaSentient::MovedByTurboLift()
{
	DaCreature::MovedByTurboLift();
	quint16 roomId = CapsuleRoomId();
	if(!roomId)
		qWarning() << "error: DaSentient::MovedByTurboLift" << Id() << Name() << "ended up in no room";
	if(!IsAIControlled())
		return;
	ThinkSoon();
	Stand();
	DaCapsule* capsule = FindParentCapsuleInEnv();
	if(!capsule)
	{
		ResetStrategy(true);
		return;
	}
	AuPoint loc = Location();
	double cgHt = GroundHeightAboveCG();
	AuPoint upVec = Transform().ProjectVector(0,0,1);
	bool touchingDirt = OnGround() != OG_None && !IsInsideSpacecraft() && IsInAtmosphere();	// Dirt, not a ship deck.
	DaSc* sc = dynamic_cast<DaSc*>(capsule);
	if(sc)
	{
		// Check to see if we reached our route destination by moving in the lift.
		QString atStop = sc->TurboLiftStop(roomId);
		m_SentientLock.lockForWrite();
		if(!m_AIScRoute)
			m_AIScRoute = new AuScRoute;
		m_AIRecentPath.clear();
		if(!m_AIScRoute->Update(loc, cgHt, upVec, roomId, atStop, IsLadderPose(), touchingDirt, sc->LockDoorStates()))
			m_AIScRoute->clear();	// The move was not to my destination. Reevaluate the situation.
		sc->UnlockDoorStates();
		m_AIStrategy = 0;
		m_AIStrategyTime.restart();
		m_SentientLock.unlock();
	}
	else
	{
		DaBd* bd = dynamic_cast<DaBd*>(capsule);
		if(bd)
		{
			// Check to see if we reached our route destination by moving in the lift.
			QString atStop = bd->TurboLiftStop(roomId);
			m_SentientLock.lockForWrite();
			if(!m_AIScRoute)
				m_AIScRoute = new AuScRoute;
			m_AIRecentPath.clear();
			if(!m_AIScRoute->Update(loc, cgHt, upVec, roomId, atStop, IsLadderPose(), touchingDirt, bd->LockDoorStates()))
				m_AIScRoute->clear();	// The move was not to my destination. Reevaluate the situation.
			bd->UnlockDoorStates();
			m_AIStrategy = 0;
			m_AIStrategyTime.restart();
			m_SentientLock.unlock();
		}
	}
}

void DaSentient::Defibrillated(quint32 byEmpireId)
{
	SetOrder(ORDER_Stay, AuId(0), AuPoint());
	PoseVisage(AuVisage::Closed, AuVisage::Closed);
	// Don't let the decision to get surgery become my primary order or I will stay in the unit forever.
	AmendOrder(ORDER_GoToTargetExactly, ParentId(), Location(), 0);
	AmendOrder(ORDER_EnterSurgery, ParentId(), Location(), 0);
	DaCreature::Defibrillated(byEmpireId);
}

AuPoint DaSentient::OrderLoc() const
{
	AuPoint orderLoc;
	QString orderText;
	m_ThinkLock.lockForRead();
	if(!m_AIOrders.isEmpty())
	{
		AuAIOrderArray::const_iterator oi = m_AIOrders.cend()-1;
		if((*oi).m_Type == ORDER_GoToRoom)
			orderText = (*oi).m_Text;		// Room name.
		else
			orderLoc = (*oi).m_Loc;
	}
	m_ThinkLock.unlock();
	if(!orderText.isEmpty())
	{
		// Ask my parent vessel for the location of the room.
		const DaCapsule* vessel = dynamic_cast<const DaCapsule*>(Parent());
		if(vessel)
			vessel->RoomOrderLoc(orderText, orderLoc);
	}
	return orderLoc;
}

quint64 DaSentient::MemoryUsage() const
{
	quint64 mem = DaCreature::MemoryUsage();
	mem += sizeof(DaSentient) - sizeof(DaCreature);
	mem += sizeof(AuAIOrder) * m_AIOrders.size();
	mem += sizeof(AuSentientDecision::DECISION)	* m_Decisions.capacity();
	if(m_DecisionTable)
		mem += sizeof(AuSentientDecision) * AuSentientDecision::D_COUNT;
	mem += m_AIRecentPath.MemoryUsage();
	if(m_AIScRoute)
		mem += sizeof(AuScRouteLoc) * m_AIScRoute->size();
	return mem;
}

void DaSentient::BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire)
{
	if(EmpireId() != surrenderingEmpire)
		return;
	SetEmpireId(toEmpire);
}

void DaSentient::RepairPirate(DaVessel* ship)
{
	quint16 repairSecs = ship->RepairAll();
	ship->AddRepairTime(repairSecs);

	// If the ship got any repairs, configure the ship to undergo those repairs.
	if(ship->RepairSecondsRemaining())
	{
		// Insure all systems are off since broken consoles might have prevented AI crew from turning them off.
		ship->TurnPowerPlantOff();
		ship->SetGravityDriveState(DaVessel::GD_Off);
		ship->TurnAllFTLDrivesOff();
		ship->TurnSensorsOff();
		ship->SetShieldState(DaVessel::SS_Off);
	}
	ship->SetModified();
}

int DaSentient::RepairShip(DaVessel* ship)
{
	bool completedRepairs = false;
	bool tooExpensive = false;			// true if any building or city is too expensive

	DaAtomList repairBds;				// List of bds that can repair my ship.
	ship->FindRepairBds(repairBds);
	if(!repairBds.empty())
	{
		// Repair the ship.
		for(DaAtomList::const_iterator bi = repairBds.begin(); bi != repairBds.end() && !completedRepairs; bi++)
		{
			DaBd* repairBd = dynamic_cast<DaBd*>(*bi);
			if(!repairBd)
				continue;

			// There is a building that can repair this ship.
			// Determine what is needed to repair as much of the ship as possible.
			int applyMinQ = ship->MissionOrderApplyMinQ();
			if(applyMinQ == 0)
			{
				// Repair systems one by one to maintain system module quality.
				for(int system=0; system<SYS_Count && !completedRepairs; system++)
				{
					QSet<quint16> whiches;
					if(!ship->SystemWhiches((SYSTEM)system, whiches))
						continue;	// No system.
					if(whiches.empty())
					{
						// System is not divided into subsystems.
						quint8 minPartQ = ship->SystemQuality((SYSTEM)system);
						quint16 repairSecs = 0;
						unsigned int repairCost = repairBd->EstimateRepairCostPossible(ship, (SYSTEM)system, 0, repairSecs,
								minPartQ);
						if(!repairCost)
							continue;	// System is not damaged.
						if(ship->IsPrivatelyOwned())
						{
							// Privately owned ship must pay for the repairs out of its account.
							if(repairCost > ship->AccountBalance())
							{
								tooExpensive = true;
								continue;	// Privately owned ship can't afford repairs at the city.
							}
							ship->DebitAccount(repairCost);
							repairBd->CreditAccountCitizenSales(repairCost);	// Collects sales tax off the price.
						}

						ship->AddRepairTime(repairSecs);
						ship->RepairAllPossible(repairBd, (SYSTEM)system, minPartQ);

						if(ship->IsPrivatelyOwned() && ship->AccountBalance() < 1)
						{
							if(ship->IsDamaged())
								tooExpensive = true;
							completedRepairs = true;
							break;	// Private ship ran out of money so stop buying repairs.
						}
						if(!ship->IsDamaged())
						{
							completedRepairs = true;
							break;	// No more damage so stop checking repair cities.
						}
					}
					else
					{
						// System is divided into subsystems.
						for(QSet<quint16>::const_iterator wi = whiches.cbegin(); wi != whiches.cend(); wi++)
						{
							quint8 minPartQ = ship->SystemQuality((SYSTEM)system, *wi);
							quint16 repairSecs = 0;
							unsigned int repairCost = repairBd->EstimateRepairCostPossible(ship, (SYSTEM)system, *wi, repairSecs,
									minPartQ);
							if(!repairCost)
								continue;	// System is not damaged.
							if(ship->IsPrivatelyOwned())
							{
								// Privately owned ship must pay for the repairs out of its account.
								if(repairCost > ship->AccountBalance())
								{
									tooExpensive = true;
									continue;	// Privately owned ship can't afford repairs at the city.
								}
								ship->DebitAccount(repairCost);
								repairBd->CreditAccountCitizenSales(repairCost);	// Collects sales tax off the price.
							}

							ship->AddRepairTime(repairSecs);
							ship->RepairAllPossible(repairBd, AuSysId((SYSTEM)system, *wi), minPartQ);

							if(ship->IsPrivatelyOwned() && ship->AccountBalance() < 1)
							{
								if(ship->IsDamaged())
									tooExpensive = true;
								completedRepairs = true;
								break;	// Private ship ran out of money so stop buying repairs.
							}
							if(!ship->IsDamaged())
							{
								completedRepairs = true;
								break;	// No more damage so stop checking repair cities.
							}
						}
					}
				}
			}
			else
			{
				// Can repair all systems at once using the same minimum part quality for all of them.
				quint8 minPartQ;
				if(applyMinQ == 1)
					minPartQ = ship->QualityMade();
				else
					minPartQ = ship->MissionOrderMinQ();
				quint16 repairSecs = 0;
				unsigned int repairCost = repairBd->EstimateRepairCostPossible(ship, repairSecs, minPartQ);
				if(ship->IsPrivatelyOwned())
				{
					// Privately owned ship must pay for the repairs out of its account.
					if(repairCost > ship->AccountBalance())
					{
						tooExpensive = true;
						continue;	// Privately owned ship can't afford repairs at the city.
					}
					ship->DebitAccount(repairCost);
					repairBd->CreditAccountCitizenSales(repairCost);	// Collects sales tax off the price.
				}

				ship->AddRepairTime(repairSecs);
				ship->RepairAllPossible(repairBd, minPartQ);

				if(ship->IsPrivatelyOwned() && ship->AccountBalance() < 1)
				{
					if(ship->IsDamaged())
						tooExpensive = true;
					completedRepairs = true;
					break;	// Private ship ran out of money so stop buying repairs.
				}
				if(!ship->IsDamaged())
				{
					completedRepairs = true;
					break;	// No more damage so stop checking repair cities.
				}
			}
		}
	}

	// If the ship got any repairs, configure the ship to undergo those repairs.
	if(ship->RepairSecondsRemaining())
	{
		// Insure all systems are off since broken consoles might have prevented AI crew from turning them off.
		ship->TurnPowerPlantOff();
		ship->SetGravityDriveState(DaVessel::GD_Off);
		ship->TurnAllFTLDrivesOff();
		ship->TurnSensorsOff();
		ship->SetShieldState(DaVessel::SS_Off);
	}
	ship->SetModified();

	// Return a useful result code.
	if(ship->IsDamaged())
	{
		// Repairs were not completed.
		if(tooExpensive)
			return 2;	// Company ship had insufficient money to complete repairs.
		return 3;	// Insufficient parts.
	}
	else
		return 0;	// Repairs were complete.
}

// Returns false if unable to add the item.
bool DaSentient::AddGearItem(const AuItem& cItem)
{
	AuGear* gear = LockGearForWrite();
	bool v = AddGearItemNoLock(*gear, cItem);
	UnlockGear();
	return v;
}

// Returns false if unable to add the item.
bool DaSentient::AddGearItemNoLock(AuGear& gear, const AuItem& cItem)
{
	//qDebug("DaCitizen::AddGearItem");
	// Some items are too big to carry.
	if(!cItem.CanBeCarried())
		return false;

	// Attempt to store the item in the person's container(s).
	// Priority: Back, Hand, Waist containers.
	for(AuGear::iterator gi = gear.begin(); gi != gear.end(); gi++)
	{
		// Only test body slot(s).
		if((*gi).SlotType() != AuGearSlot::Body)
			continue;

		if((*gi).Item().FitItemIntoContainer(cItem))
		{
			UpdateGearWeight();
			return true;	// Container on back accepted the item.
		}
	}

	for(AuGear::iterator gi = gear.begin(); gi != gear.end(); gi++)
	{
		// Only test hand slot(s).
		if((*gi).SlotType() != AuGearSlot::Hand)
			continue;

		if((*gi).Item().FitItemIntoContainer(cItem))
		{
			UpdateGearWeight();
			return true;	// Container on back accepted the item.
		}
	}

	for(AuGear::iterator gi = gear.begin(); gi != gear.end(); gi++)
	{
		// Only test hand slot(s).
		if((*gi).SlotType() != AuGearSlot::Waist)
			continue;

		if((*gi).Item().FitItemIntoContainer(cItem))
		{
			UpdateGearWeight();
			return true;	// Container on back accepted the item.
		}
	}

	// Failed to store the item in a container so try to add it as a nu item.
	AuGearSlot::SLOTTYPE slotType;
	quint8 slotWhich;
	if(!gear.ItemsInSlot(AuGearSlot::Body, &slotWhich))
		slotType = AuGearSlot::Body;
	else if(gear.ItemsInSlot(AuGearSlot::Hand, &slotWhich) < 2)
		slotType = AuGearSlot::Hand;
	else
	{
		quint8 packUnits = cItem.PackSize().width() * cItem.PackSize().height();
		if(packUnits <= 15 && gear.ItemsInSlot(AuGearSlot::Waist, &slotWhich) < 4)
			slotType = AuGearSlot::Waist;	// Small enough to fit waist position.
		else
			slotType = AuGearSlot::Invalid;
	}

	if(slotType != AuGearSlot::Invalid)
	{
		gear.Insert(slotType, slotWhich, cItem);
		UpdateGearWeight();
		return true;
	}

	// Backpack, hands, back, and waist are not available to accept the nu item.
	return false;
}

quint32 DaSentient::ConsumeItem(const AuItem& itemToConsume)
{
	AuGearPos gearPos(CurrentItemPos());
	INVENTORYSUB isub = CurrentItemSub();

	AuGear* gear = LockGearForWrite();
	quint32 v = ConsumeItemNoLock(*gear, gearPos, isub, itemToConsume);
	UnlockGear();
	return v;
}

quint32 DaSentient::ConsumeItemNoLock(AuGear& gear, const AuGearPos& gearPos, INVENTORYSUB curSub, const AuItem& itemToConsume)
{
	// How many are we supposed to consume?
	if(!itemToConsume.Count())
		return 0;												// None?

	// First try to consume the current item in hand.
	quint32 countConsumed = 0;
	if(curSub == IPS_Item)
	{
		const AuItem* gearItem = gear.ItemAt(gearPos);
		if(gearItem)
		{
			// This person has a "current" item selected.
			if(gearItem->IsItem(itemToConsume))
			{
				// The current item is the desired item.
				// Consume one of those items and update the weight of my gear.
				quint32 consumeItem = gearItem->Count();					// Amount of the item to consume.
				if(consumeItem >= itemToConsume.Count())
				{
					// The current item can supply all that needs to be consumed. It might be used up in the process.
					ConsumeGearNoLock(gear, gearPos, itemToConsume.Count());
					return itemToConsume.Count();
				}

				// The current item is less than all that needs to be consumed. Use it all up.
				ConsumeGearNoLock(gear, gearPos, consumeItem);
				countConsumed += consumeItem;
			}
		}
	}

	// Look recursively in entire inventory for the item(s) to consume.
	int gidx=0;
	while(gidx<gear.size() && countConsumed < itemToConsume.Count())
	{
		AuGear::iterator gi = gear.begin() + gidx;
		AuItem& item = (*gi).Item();
		countConsumed += ConsumeItemRecursiveNoLock(item, itemToConsume, countConsumed);
		if(item.Count() == 0)
			gear.Erase(AuGearPos(*gi));	// Remove top level gear item when its count reaches zero.
		else
			gidx++;
	}
	UpdateGearWeight();
	return countConsumed;
}

quint32 DaSentient::ConsumeItemRecursiveNoLock(AuItem& item, const AuItem& itemToConsume, quint32 countConsumed)
{
	if(!item.Content() || item.Content()->empty())
	{
		// The item is not a container or it is an empty container.
		if(item.IsItem(itemToConsume))
			return item.Consume(itemToConsume.Count() - countConsumed);	// Consume as much as we can.
		return 0;
	}

	// Look inside the container.
	quint32 nuConsumed = countConsumed;	// Track a nu consumed count so we can return the net difference.
	AuItemContent::iterator ci = item.Content()->begin();
	while(ci != item.Content()->end() && nuConsumed < itemToConsume.Count())
	{
		if((*ci).second->IsItem(itemToConsume) && (!(*ci).second->Content() || (*ci).second->Content()->empty()))
		{
			// Found the desired item. It is not a non-empty container so it can be consumed.
			nuConsumed += (*ci).second->Consume(itemToConsume.Count() - nuConsumed);	// Consume as much as we can.

			// Items inside containers are removed when the quantity reaches 0.
			if((*ci).second->Count() == 0)
			{
				ci = item.Content()->Erase(ci);
				continue;	// skip ci++
			}
		}
		else if((*ci).second->Content() && !(*ci).second->Content()->empty())
			nuConsumed += ConsumeItemRecursiveNoLock(*(*ci).second, itemToConsume, nuConsumed);	// Recurse into non-empty container.
		ci++;
	}
	return nuConsumed - countConsumed;
}

// 'subItemLoc' is the location of items within containers, when the item at 'pos' is a container.
void DaSentient::ConsumeGear(const AuGearPos& pos, quint32 qty)
{
	AuGear* gear = LockGearForWrite();
	ConsumeGearNoLock(*gear, pos, qty);
	UnlockGear();
}

void DaSentient::ConsumeGearNoLock(AuGear& gear, const AuGearPos& pos, quint32 qty)
{
	AuItem* gearItem = gear.ItemAt(pos);
	if(!gearItem)
		return;

	gearItem->Consume(qty);
	if(gearItem->Count() == 0)
		gear.Erase(pos);

	UpdateGearWeight();
}

void DaSentient::FindShipMatesRecursive(const AuId& shipAssigned, DaAtomList& mates) const
{
	//if(!IsNormal())
		//return;
	DaCreature::FindShipMatesRecursive(shipAssigned, mates);
	if(AssignedShip() == shipAssigned)
		mates.append((DaAtom*)this);	// Cast away constness.
}

bool DaSentient::PirateMissionGoHome(DaVessel* ship)
{
	const DaSolarSystem* system = ship->FindParentSolarSystem();
	if(!system)
		return false;
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return false;
	quint32 myEmpireId = EmpireId();
	const DgData& gd = g_Au.LockDataForRead();
	AuId baseSystemId = gd.PirateBaseSystemInSector(sector->Id(), myEmpireId);
	AuId baseId;
	AuCity base;
	if(!baseSystemId.IsNull())
	{
		quint32 baseEmpireId;
		baseId = gd.PirateBaseInSystem(baseSystemId, baseEmpireId);
		if(!baseId.IsNull())
			gd.EmpireCity(baseEmpireId, baseId, base);
	}
	g_Au.UnlockData();
	if(base.CityId().IsNull())
		return false;

	// Do I need to travel far to get there?
	if(!system->Is(base.SystemId()))
	{
		if(ship->HasWormholeFTLDrive())
			ship->MissionAppendOrder(AuMissionOrderGoToSystemViaWormholes(base.SystemId(), base.SystemName()));
		else
			ship->MissionAppendOrder(AuMissionOrderGoToSystemDirect(base.SystemId(), base.SystemName(), base.SystemLoc()));
	}

	// Is there a station? Can I get repairs at the base?
	ship->MissionAppendOrder(AuMissionOrderGoToOrbitCity(base.CityId(), base.CityName()));
	return true;
}

bool DaSentient::PirateMissionStartHunting(DaVessel* ship)
{
	const DaSolarSystem* system = ship->FindParentSolarSystemInEnv();
	if(!system)
		return false;
	quint32 baseEmpireId;
	bool hasBase = system->HasPirateBase(baseEmpireId);
	if(!hasBase || baseEmpireId != ship->EmpireId())
	{
		// No base is present in this system, or it is not my base, so we will stop hunting when the ship's condition is red.
		if(ship->AlertCondition() == ConditionRed)
			return false;	// I don't want to hunt with a broken or burning ship.

		// Ship is in good shape. Hunt enemy units. Don't plunder the same ship repeatedly.
		ship->SetCrewPosture(POSTURE_Offensive);	// Assume offensive posture so I will hunt while waiting.
		if(!ship->LockedBlip().IsNull())
		{
			if(ship->LockedBlip() != LastPlunder())
			{
				// Sensor operator will lock on to enemy units for me.
				const DaAtom* system = ship->FindEnvironmentAtom();
				const DaVessel* target = dynamic_cast<const DaVessel*>(system->FindIdRecursive(ship->LockedBlip()));
				if(target &&
						target->EmpireId() != ship->EmpireId() &&
						!target->IsInAtmosphere() &&
						!target->IsCoveredByATC())
				{
					// We have a target. I am a pirate so all ships from other empires are fair game.
					ship->MissionAppendOrder(AuMissionOrderPlunder(target->Id(), target->Name()));
				}
				else	// No enemy target so wait a while.
					ship->MissionAppendOrder(AuMissionOrderStay(2));
			}
			else	// Leave the area of our last plundered vessel.
				ship->MissionAppendOrder(AuMissionOrderPatrolSystem(3));
		}
		else	// Nothing on the sensors so wait a while.
			ship->MissionAppendOrder(AuMissionOrderStay(2));
	}
	else
	{
		// A base of our empire is present so we fight to the death.
		ship->SetCrewPosture(POSTURE_Berserk);		// Assume berserk posture so I will fight to the death to defend the base.
		if(!ship->LockedBlip().IsNull())
		{
			// Sensor operator will lock on to enemy units for me.
			const DaAtom* system = ship->FindEnvironmentAtom();
			const DaAtom* target = system->FindIdRecursive(ship->LockedBlip());
			if(dynamic_cast<const DaVessel*>(target) && target->EmpireId() != ship->EmpireId())
			{
				// We have a target. I am a pirate so all ships from other empires are fair game.
				ship->MissionAppendOrder(AuMissionOrderAttack(target->Id(), target->Name()));
			}
			else	// No enemy target so wait a while.
				ship->MissionAppendOrder(AuMissionOrderStay(2));
		}
		else	// Nothing on the sensors so wait a while.
			ship->MissionAppendOrder(AuMissionOrderStay(2));
	}
	return true;
}

void DaSentient::PirateMissionOrderChanged(DaVessel* ship, const AuMissionOrder& prevOrder)
{
	// Do nothing special if I have not completed my mission.
	if(ship->IsOnAMission())
		return;

	// I am a pirate in the wild.
	bool hasStation;
	switch(prevOrder.Type())
	{
	default:
		break;
	case AuMissionOrder::Plunder:
		// Finished plundering. Run home, just to show them where we live.
		ship->SetCrewPosture(POSTURE_DefendSelf);				// Don't hunt on the way home. Let our prey get clear.
		if(!PirateMissionGoHome(ship))
			ship->Retreat();									// No home? Just run. Posture is set to DefendSelf.
		break;

	case AuMissionOrder::Attack:
		// Finished attack order. The target must be dead or gone?
		if(!PirateMissionGoHome(ship))
			if(!PirateMissionStartHunting(ship))				// No home to go to?
				ship->Retreat();								// Ship is in bad shape; run away.
		break;

	case AuMissionOrder::Stay:
		// Finished waiting.
		ship->SetCrewPosture(POSTURE_Offensive);				// Attack anything we detect.
		if(!PirateMissionGoHome(ship))
		{
			// We have no home to return to.
			ship->MissionAppendOrder(AuMissionOrderStay(2));	// Wait for passers by.
		}
		break;

	case AuMissionOrder::Repair:
		// I finished getting repairs.
		if(PirateMissionStartHunting(ship))
			break;
		// I did not get enough repairs to return to condition green.
		[[fallthrough]];

	case AuMissionOrder::GoToOrbitCity:
		// Arrived at orbit position over the city.
		// Or finished repairing and the ship is still broken.
		if(ship->StationInTradeRange(ship->RoughRadius(), hasStation))
		{
			ship->SetCrewPosture(POSTURE_DefendArea);
			ship->MissionAppendOrder(AuMissionOrderRepair());	// Get repairs at the station.
		}
		else if(!PirateMissionStartHunting(ship))
		{
			ship->SetCrewPosture(POSTURE_Offensive);			// Attack anything we detect.
			ship->MissionAppendOrder(AuMissionOrderStay(2));	// Ship is in bad shape; we're home; stay put.
		}
		break;

	case AuMissionOrder::PatrolSystem:
		// I finished patroling.
		ship->SetCrewPosture(POSTURE_Offensive);				// Attack anything we detect.
		if(!PirateMissionStartHunting(ship))
			if(!PirateMissionGoHome(ship))						// Ship is broken so I won't hunt.
				ship->Retreat();								// No home? Run away.
		break;

	case AuMissionOrder::GoToSystemDirect:
	case AuMissionOrder::GoToSystemDirect_Deprecated:
	case AuMissionOrder::GoToSystemViaWormholes:
		// We arrived in our destination system. That was the end of the mission.
		ship->SetCrewPosture(POSTURE_Offensive);				// Attack anything we detect.
		PirateMissionStartHunting(ship);						// Start hunting or just sit here until I self destruct.
		break;
	}
	QString nuOrder = ship->LockMissionOrder().ListName();
	ship->UnlockMissionOrder();
	//qDebug() << "DaSentient::PirateMissionOrderChanged" << *this << "ship" << *ship << "prevOrder" << prevOrder.ListName()
		//<< "new order" << nuOrder;
}

COMMODITYID DaSentient::WornArmor() const
{
	if(CurrentGeneralPose().IsMatingOrWaiting() && IsAIControlled())
		return CID_Invalid;
	const AuGear* gear = LockGearForRead();
	COMMODITYID c = gear->ArmorWorn();
	UnlockGear();
	return c;
}

COMMODITYID DaSentient::WornClothing() const
{
	if(CurrentGeneralPose().IsMatingOrWaiting() && g_Au.IsMatingOk())
		return CID_Invalid;		// Mating implies nudity, even if nudity is not enabled.
	const AuGear* gear = LockGearForRead();
	COMMODITYID c = gear->ClothingWorn();
	UnlockGear();

	// When nudity is not generally ok, wear minimal clothing when not having sex.
	if(c == CID_Invalid && !g_Au.IsNudityOk())
		c = CID_Underwear;
	return c;
}

COMMODITYID DaSentient::WornHat() const
{
	if(CurrentGeneralPose().IsMatingOrWaiting() && IsAIControlled())
		return CID_Invalid;
	const AuGear* gear = LockGearForRead();
	COMMODITYID c = gear->HelmetOrHatWorn();
	UnlockGear();
	return c;
}

bool DaSentient::InterpretVisage(const AuEmojiList& emojis, const QString& saidWord, AuVisage& visage) const
{
	return emojis.Visage(saidWord, visage);
}

void DaSentient::PoseMouthForSaying(const AuCommPacket& msg, const AuVisage* visage)
{
	PoseMouthForSaying(msg, visage, g_Au.Emojis());
}

void DaSentient::PoseMouthForSaying(const AuCommPacket& msg, const AuVisage* visage, const AuEmojiList& emojis)
{
	QStringList words = emojis.SplitWords(msg.Text());
	unsigned int wordCount = words.size();
	if(!wordCount)
	{
		AuPose stationary(MouthStationaryPose());
		PoseVisage(AuVisage(stationary, 0), AuVisage(stationary, 0));
	}
	else
	{
		AuVisage talkVisage(AuVisage::Closed);
		bool startPose = emojis.Visage(words.first(), talkVisage);	// True if first word triggers visage.

		// See if we can pick out a pose to change our mouth to when the first animated pose is complete.
		// Note that this finish pose is only selected when the first pose is animated and expires.
		AuVisage finishVisage(AuVisage::Closed);
		bool finPose;
		if(wordCount > 1)
			finPose = emojis.Visage(words.last(), finishVisage);
		else
			finPose = false;

		// AI sometimes specifies the finish pose so they don't have to use emotes. Story engine uses this.
		if(!finPose && visage)
		{
			finishVisage = *visage;
			finPose = true;
		}

		if(startPose && finPose)
		{
			// Start with the pose triggered by the first word.
			// When it is finished, switch to the finish pose, if any.
			// This will eventually end with the last stationary finish pose if this finish pose is animated.
			PoseVisage(talkVisage, finishVisage);
		}
		else if(startPose)
		{
			// Start with the pose triggered by the first word.
			if(wordCount > 1)
			{
				// When it is finished, switch to a finish talking pose.
				if(msg.Text().toUpper() == msg.Text())
					finishVisage = AuVisage(AuPose::MouthYell, wordCount * 400);
				else if(msg.Text().toLower() == msg.Text())
				{
					if(wordCount > 10)
						finishVisage = AuVisage(AuPose::MouthWhisper, wordCount * 250);
					else
						finishVisage = AuVisage(AuPose::MouthWhisper, wordCount * 350);
				}
				else if(wordCount > 10)
					finishVisage = AuVisage(AuPose::MouthJabber, wordCount * 200);
				else
					finishVisage = AuVisage(AuPose::MouthTalk, wordCount * 300);
			}
			else
			{
				// No talking to finish the pose. Return to last stationary pose.
				if(talkVisage.MouthPose().IsMouthStationary())
					finishVisage = AuVisage(talkVisage.MouthPose(), 0);
				else
					finishVisage = AuVisage(MouthStationaryPose(), 0);
			}

			// This will eventually end with MouthStationaryPose if the finish pose is animated.
			PoseVisage(talkVisage, finishVisage);
		}
		else
		{
			// Start with a talking pose and end with the specified finish pose, if any.
			if(!finPose)
			{
				// No specific finish pose. Return to last stationary pose.
				finishVisage = AuVisage(MouthStationaryPose(), 0);
			}

			// This will eventually end with the last stationary finish pose if this finish pose is animated.
			if(msg.Text().toUpper() == msg.Text())
				PoseVisage(AuVisage(AuPose::MouthYell, wordCount * 400), finishVisage);		// Yell, All CAPS
			else if(msg.Text().toLower() == msg.Text())
			{
				if(wordCount > 10)
					PoseVisage(AuVisage(AuPose::MouthWhisper, wordCount * 250), finishVisage);	// Whisper
				else
					PoseVisage(AuVisage(AuPose::MouthWhisper, wordCount * 350), finishVisage);	// Whisper
			}
			else if(wordCount > 10)
				PoseVisage(AuVisage(AuPose::MouthJabber, wordCount * 200), finishVisage);	// Jabber, lot of words.
			else
				PoseVisage(AuVisage(AuPose::MouthTalk, wordCount * 300), finishVisage);		// Talk
		}

		// Start ticking immediately, to shut off the mouth pose when appropriate. Sometimes they are brief.
		TickTock();
	}
	SetModified(NOTIFY_Scene);
}

void DaSentient::IncreaseSkill(AuSkill::SKILL skill, quint32 xp)
{
	if(!xp)
		return;
	m_SkillsLock.lockForWrite();
	m_Skills.IncreaseSkill(skill, xp);
	m_SkillsChg.Set();
	m_SkillsLock.unlock();
	SetModified(NOTIFY_Database);
}

AuSkill::SKILL DaSentient::BestCrewStationSkill() const
{
	m_SkillsLock.lockForRead();
	AuSkill::SKILL skill = m_Skills.BestCrewStationSkill();
	m_SkillsLock.unlock();
	return skill;
}

quint32 DaSentient::CrewExperiencePoints() const
{
	m_SkillsLock.lockForRead();
	quint32 v = m_Skills.CrewExperience();
	m_SkillsLock.unlock();
	return v;
}

quint32 DaSentient::CommandExperiencePoints() const
{
	m_SkillsLock.lockForRead();
	quint32 v = m_Skills.CommandExperience();
	m_SkillsLock.unlock();
	return v;
}

quint32 DaSentient::TroopExperiencePoints() const
{
	m_SkillsLock.lockForRead();
	quint32 v = m_Skills.TroopExperience();
	m_SkillsLock.unlock();
	return v;
}

quint32 DaSentient::TotalExperiencePoints() const
{
	m_SkillsLock.lockForRead();
	quint32 v = m_Skills.TotalExperience();
	m_SkillsLock.unlock();
	return v;
}

quint32 DaSentient::ExperiencePointsForKilling() const
{
	quint32 xp = DaCreature::ExperiencePointsForKilling();
	xp += TotalExperiencePoints()/250;
	return xp;
}

bool DaSentient::GainExperienceForWorking()
{
	// Check time since last skills change.
	quint32 min = SkillsChangeTime().ElapsedMin();
	if(min < 2)
		return false;	// Not enough time has passed.
	// Update time since last skills change.
	m_SkillsLock.lockForWrite();
	m_SkillsChg.Set();
	m_SkillsLock.unlock();

	if(!IsInAtmosphere() && !IsInSpacecraft() && !IsAboardVehicle() && !InDesignEnvironment())
		IncreaseSkill(AuSkill::EVSuit, 1);
	return true;		// It is time.
}

bool DaSentient::IsProtectedByEnvironmentSuit() const
{
	// Check for leaky environment suit.
	const AuGear* gear = LockGearForRead();
	const AuItem* armor = gear->ArmorItemWorn();
	if(!armor || armor->HitPoints() <= 1)
	{
		// No suit or suit seal is broken.
		UnlockGear();
		return false;
	}

	// Check for leaky environment helmet.
	const AuItem* helmet = gear->HelmetItemWorn();
	if(!helmet || !helmet->IsEnvironmentHelmet() || helmet->HitPoints() <= 1)
	{
		// No helmet, helmet is not env, or seal is broken.
		UnlockGear();
		return false;
	}

	// Check for helmet visor open.
	/* Npcs do not manage their visor state.
	if(helmet->State() != 1)
	{
		// Helmet visor is open.
		UnlockGear();
		return 1;
	}
	*/
	UnlockGear();
	return true;	// ok
}

bool DaSentient::BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
		int& ammoFull) const
{
	// Get info about physical body weapon; there is always a physical body weapon even if its my teeth.
	// This provides the base range provided by the physical body.
	// They physical body range is added to the weapon range.
	bool useSpecWpn = DaCreature::BestWeaponAtRange(targetRange, loc, weapon, ammoCur, ammoFull);
	if(weapon.m_Mode == WM_Psionic)
	{
		// Check for psionic weapon enhancing ring.
		quint8 highest_RingQ = 0;
		AuGearPos ringPos;

		const AuGear* gear = LockGearForRead();
		ringPos = gear->FindBestTop(CID_PreonRingO);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingB);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingA);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingF);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingG);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingK);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingM);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}
		UnlockGear();

		// Use the highest Q, if any?
		if(highest_RingQ)
		{
			float qboost = QualityBoostF(highest_RingQ) + 1;
			weapon.Magnify(qboost);
		}
	}
	return useSpecWpn;
}

void DaSentient::WeaponAtUsePos(const AuGearPos& pos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const
{
	DaCreature::WeaponAtUsePos(pos, sub, weapon, ammoCur, ammoFull);
	if(weapon.m_Mode == WM_Psionic)
	{
		// Check for psionic weapon enhancing ring.
		quint8 highest_RingQ = 0;
		AuGearPos ringPos;

		const AuGear* gear = LockGearForRead();
		ringPos = gear->FindBestTop(CID_PreonRingO);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingB);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingA);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingF);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingG);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingK);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}

		ringPos = gear->FindBestTop(CID_PreonRingM);
		if(ringPos != AuGearPos())
		{
			const AuItem* ring = gear->ItemAt(ringPos);
			if(ring && ring->Quality() > highest_RingQ)
				highest_RingQ = ring->Quality();
		}
		UnlockGear();

		// Use the highest Q, if any?
		if(highest_RingQ)
		{
			float qboost = QualityBoostF(highest_RingQ) + 1;
			weapon.Magnify(qboost);
		}
	}
}

void DaSentient::ChooseHairStyle()
{
	AuHairStyle::STYLE style;
	AuHairStyle::SHAVE shave;
	if(Gender() == Male)
	{
		// Choose from among certain masculine hair and shave styles.
		if(GrowthStage() == Juvenile)
		{
			AuHairStyle::STYLE styles[] =
			{
				//AuHairStyle::Bald,							// All follicles removed.
				AuHairStyle::Buzz,							// Every follicle as a stub.
				AuHairStyle::Close,							// Hair patch only at every follicle.
				//AuHairStyle::Straight,						// Straight out of every follicle.
				//AuHairStyle::Unkempt,						// Straight out, wavy, falling with gravity, to maximum hair length.
				//AuHairStyle::CousinIt,
				AuHairStyle::Short,
				AuHairStyle::Baltar,
				//AuHairStyle::Eowyn,
				//AuHairStyle::Arwen,
				//AuHairStyle::Elrond,
				//AuHairStyle::Mandinka,						// Mr. T
				AuHairStyle::Mohawk,
				//AuHairStyle::PonyBangs,
				AuHairStyle::PonyTail,
				AuHairStyle::Regulation,						// Parted on the right
				AuHairStyle::Waterfall,
				AuHairStyle::HighAndTight,
				AuHairStyle::Crop,
				//AuHairStyle::Manager,
				//AuHairStyle::FrTuck,
				//AuHairStyle::Egghead,
				//AuHairStyle::Spacely,
				AuHairStyle::Carrot,
				//AuHairStyle::Crown,
				//AuHairStyle::Mentat,
				AuHairStyle::Flattop,
				AuHairStyle::Bun,
				//AuHairStyle::BunBangs,
				AuHairStyle::Bullet,
				//AuHairStyle::Fallout,
				//AuHairStyle::Curtain,
				AuHairStyle::Manchu,
				AuHairStyle::MohawkPony,
				//AuHairStyle::STYLE_Count
			};
			unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
			style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];
		}
		else
		{
			AuHairStyle::STYLE styles[] =
			{
				AuHairStyle::Bald,							// All follicles removed.
				AuHairStyle::Buzz,							// Every follicle as a stub.
				AuHairStyle::Close,							// Hair patch only at every follicle.
				//AuHairStyle::Straight,						// Straight out of every follicle.
				//AuHairStyle::Unkempt,						// Straight out, wavy, falling with gravity, to maximum hair length.
				//AuHairStyle::CousinIt,
				AuHairStyle::Short,
				AuHairStyle::Baltar,
				//AuHairStyle::Eowyn,
				//AuHairStyle::Arwen,
				AuHairStyle::Elrond,
				AuHairStyle::Mandinka,						// Mr. T
				AuHairStyle::Mohawk,
				//AuHairStyle::PonyBangs,
				AuHairStyle::PonyTail,
				AuHairStyle::Regulation,						// Parted on the right
				AuHairStyle::Waterfall,
				AuHairStyle::HighAndTight,
				AuHairStyle::Crop,
				AuHairStyle::Manager,
				AuHairStyle::FrTuck,
				AuHairStyle::Egghead,
				AuHairStyle::Spacely,
				AuHairStyle::Carrot,
				AuHairStyle::Crown,
				AuHairStyle::Mentat,
				AuHairStyle::Flattop,
				AuHairStyle::Bun,
				//AuHairStyle::BunBangs,
				AuHairStyle::Bullet,
				//AuHairStyle::Fallout,
				AuHairStyle::Curtain,
				AuHairStyle::Manchu,
				AuHairStyle::MohawkPony,
				//AuHairStyle::STYLE_Count
			};
			unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
			style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];
		}

		// Juveniles do not grow facial hair but pick a style for them when they grow up.
		AuHairStyle::SHAVE shaves[] =
		{
			AuHairStyle::Clean,							// All follicles removed.
			AuHairStyle::Stubble,						// Every follicle as a stub.
			AuHairStyle::CloseWhiskers,					// Hair patch only at every follicle.
			//AuHairStyle::StraightWhiskers,				// Straight out of every follicle.
			//AuHairStyle::Unshaven,						// Straight out, wavy, falling with gravity, to maximum hair length.
			AuHairStyle::ShapedStubble,
			AuHairStyle::Hollywoodian,
			AuHairStyle::MuttonChops,
			AuHairStyle::FrenchFork,
			AuHairStyle::FuManchu,
			AuHairStyle::OldDutch,
			AuHairStyle::VanDyke,
			AuHairStyle::ExtendedVanDyke,
			AuHairStyle::FriendlyMuttonChops,
			AuHairStyle::Balbo,
			AuHairStyle::Klingon,
			AuHairStyle::ShortBoxedBeard,
			AuHairStyle::FullBeard,
			AuHairStyle::Goatee,
			AuHairStyle::ExtendedGoatee,
			AuHairStyle::ChinStrap,
			AuHairStyle::ChinCurtain,
			AuHairStyle::Zappa,
			AuHairStyle::SoulPatch,
			AuHairStyle::Winnfield,
			AuHairStyle::PetitGoatee,
			AuHairStyle::FranzJosef,
			AuHairStyle::Anchor,
			AuHairStyle::Napoleon,
			AuHairStyle::Copstash,
			AuHairStyle::Gandalf,
			AuHairStyle::Confucius,
			AuHairStyle::Boris,
			AuHairStyle::Toothbrush,
			AuHairStyle::ChinPuff,
			AuHairStyle::Stripe,
			AuHairStyle::RapStandard,
			AuHairStyle::Pencil,
			AuHairStyle::Handlebar,
			AuHairStyle::HandlebarAndChinPuff,
			AuHairStyle::HandlebarAndGoatee,
			AuHairStyle::Wally,
			AuHairStyle::PencilPatch,
			//AuHairStyle::SHAVE_Count
		};
		unsigned int shavesCount = sizeof(shaves)/sizeof(AuHairStyle::SHAVE);
		shave = shaves[g_Au.Dice().Roll(1, shavesCount, -1)];
	}
	else if(Gender() == Female)
	{
		// Choose from among certain feminine hair and shave styles.
		if(GrowthStage() == Juvenile)
		{
			AuHairStyle::STYLE styles[] =
			{
				//AuHairStyle::Bald,							// All follicles removed.
				//AuHairStyle::Buzz,							// Every follicle as a stub.
				AuHairStyle::Close,							// Hair patch only at every follicle.
				//AuHairStyle::Straight,						// Straight out of every follicle.
				//AuHairStyle::Unkempt,						// Straight out, wavy, falling with gravity, to maximum hair length.
				//AuHairStyle::CousinIt,
				AuHairStyle::Short,
				AuHairStyle::Baltar,
				AuHairStyle::Eowyn,
				AuHairStyle::Arwen,
				//AuHairStyle::Elrond,
				//AuHairStyle::Mandinka,						// Mr. T
				AuHairStyle::Mohawk,
				AuHairStyle::PonyBangs,
				AuHairStyle::PonyTail,
				AuHairStyle::Regulation,						// Parted on the right
				AuHairStyle::Waterfall,
				//AuHairStyle::HighAndTight,
				AuHairStyle::Crop,
				//AuHairStyle::Manager,
				//AuHairStyle::FrTuck,
				//AuHairStyle::Egghead,
				//AuHairStyle::Spacely,
				AuHairStyle::Carrot,
				//AuHairStyle::Crown,
				//AuHairStyle::Mentat,
				AuHairStyle::Flattop,
				AuHairStyle::Bun,
				AuHairStyle::BunBangs,
				AuHairStyle::Bullet,
				//AuHairStyle::Fallout,
				//AuHairStyle::Curtain,
				AuHairStyle::Manchu,
				AuHairStyle::MohawkPony,
				//AuHairStyle::STYLE_Count
			};
			unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
			style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];
		}
		else
		{
			AuHairStyle::STYLE styles[] =
			{
				//AuHairStyle::Bald,							// All follicles removed.
				//AuHairStyle::Buzz,							// Every follicle as a stub.
				AuHairStyle::Close,							// Hair patch only at every follicle.
				//AuHairStyle::Straight,						// Straight out of every follicle.
				//AuHairStyle::Unkempt,						// Straight out, wavy, falling with gravity, to maximum hair length.
				//AuHairStyle::CousinIt,
				AuHairStyle::Short,
				AuHairStyle::Baltar,
				AuHairStyle::Eowyn,
				AuHairStyle::Arwen,
				AuHairStyle::Elrond,
				//AuHairStyle::Mandinka,						// Mr. T
				AuHairStyle::Mohawk,
				AuHairStyle::PonyBangs,
				AuHairStyle::PonyTail,
				AuHairStyle::Regulation,						// Parted on the right
				AuHairStyle::Waterfall,
				//AuHairStyle::HighAndTight,
				AuHairStyle::Crop,
				//AuHairStyle::Manager,
				//AuHairStyle::FrTuck,
				//AuHairStyle::Egghead,
				//AuHairStyle::Spacely,
				AuHairStyle::Carrot,
				//AuHairStyle::Crown,
				//AuHairStyle::Mentat,
				AuHairStyle::Flattop,
				AuHairStyle::Bun,
				AuHairStyle::BunBangs,
				AuHairStyle::Bullet,
				//AuHairStyle::Fallout,
				AuHairStyle::Curtain,
				AuHairStyle::Manchu,
				AuHairStyle::MohawkPony,
				//AuHairStyle::STYLE_Count
			};
			unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
			style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];
		}

		shave = AuHairStyle::Clean;
	}
	else
	{
		// Choose from among certain hair and shave styles.
		if(GrowthStage() == Juvenile)
		{
			AuHairStyle::STYLE styles[] =
			{
				//AuHairStyle::Bald,							// All follicles removed.
				AuHairStyle::Buzz,							// Every follicle as a stub.
				AuHairStyle::Close,							// Hair patch only at every follicle.
				//AuHairStyle::Straight,						// Straight out of every follicle.
				//AuHairStyle::Unkempt,						// Straight out, wavy, falling with gravity, to maximum hair length.
				//AuHairStyle::CousinIt,
				AuHairStyle::Short,
				AuHairStyle::Baltar,
				AuHairStyle::Eowyn,
				AuHairStyle::Arwen,
				//AuHairStyle::Elrond,
				//AuHairStyle::Mandinka,						// Mr. T
				AuHairStyle::Mohawk,
				AuHairStyle::PonyBangs,
				AuHairStyle::PonyTail,
				AuHairStyle::Regulation,						// Parted on the right
				AuHairStyle::Waterfall,
				AuHairStyle::HighAndTight,
				AuHairStyle::Crop,
				//AuHairStyle::Manager,
				//AuHairStyle::FrTuck,
				//AuHairStyle::Egghead,
				//AuHairStyle::Spacely,
				AuHairStyle::Carrot,
				//AuHairStyle::Crown,
				//AuHairStyle::Mentat,
				AuHairStyle::Flattop,
				AuHairStyle::Bun,
				AuHairStyle::BunBangs,
				AuHairStyle::Bullet,
				//AuHairStyle::Fallout,
				//AuHairStyle::Curtain,
				AuHairStyle::Manchu,
				AuHairStyle::MohawkPony,
				//AuHairStyle::STYLE_Count
			};
			unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
			style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];
		}
		else
		{
			AuHairStyle::STYLE styles[] =
			{
				AuHairStyle::Bald,							// All follicles removed.
				AuHairStyle::Buzz,							// Every follicle as a stub.
				AuHairStyle::Close,							// Hair patch only at every follicle.
				//AuHairStyle::Straight,						// Straight out of every follicle.
				//AuHairStyle::Unkempt,						// Straight out, wavy, falling with gravity, to maximum hair length.
				//AuHairStyle::CousinIt,
				AuHairStyle::Short,
				AuHairStyle::Baltar,
				AuHairStyle::Eowyn,
				AuHairStyle::Arwen,
				AuHairStyle::Elrond,
				AuHairStyle::Mandinka,						// Mr. T
				AuHairStyle::Mohawk,
				AuHairStyle::PonyBangs,
				AuHairStyle::PonyTail,
				AuHairStyle::Regulation,						// Parted on the right
				AuHairStyle::Waterfall,
				AuHairStyle::HighAndTight,
				AuHairStyle::Crop,
				AuHairStyle::Manager,
				AuHairStyle::FrTuck,
				AuHairStyle::Egghead,
				AuHairStyle::Spacely,
				AuHairStyle::Carrot,
				AuHairStyle::Crown,
				AuHairStyle::Mentat,
				AuHairStyle::Flattop,
				AuHairStyle::Bun,
				AuHairStyle::BunBangs,
				AuHairStyle::Bullet,
				//AuHairStyle::Fallout,
				AuHairStyle::Curtain,
				AuHairStyle::Manchu,
				AuHairStyle::MohawkPony,
				//AuHairStyle::STYLE_Count
			};
			unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
			style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];
		}

		// Juveniles do not grow facial hair but pick a style for them when they grow up.
		AuHairStyle::SHAVE shaves[] =
		{
			AuHairStyle::Clean,							// All follicles removed.
			AuHairStyle::Stubble,						// Every follicle as a stub.
			AuHairStyle::CloseWhiskers,					// Hair patch only at every follicle.
			//AuHairStyle::StraightWhiskers,				// Straight out of every follicle.
			//AuHairStyle::Unshaven,						// Straight out, wavy, falling with gravity, to maximum hair length.
			AuHairStyle::ShapedStubble,
			AuHairStyle::Hollywoodian,
			AuHairStyle::MuttonChops,
			AuHairStyle::FrenchFork,
			AuHairStyle::FuManchu,
			AuHairStyle::OldDutch,
			AuHairStyle::VanDyke,
			AuHairStyle::ExtendedVanDyke,
			AuHairStyle::FriendlyMuttonChops,
			AuHairStyle::Balbo,
			AuHairStyle::Klingon,
			AuHairStyle::ShortBoxedBeard,
			AuHairStyle::FullBeard,
			AuHairStyle::Goatee,
			AuHairStyle::ExtendedGoatee,
			AuHairStyle::ChinStrap,
			AuHairStyle::ChinCurtain,
			AuHairStyle::Zappa,
			AuHairStyle::SoulPatch,
			AuHairStyle::Winnfield,
			AuHairStyle::PetitGoatee,
			AuHairStyle::FranzJosef,
			AuHairStyle::Anchor,
			AuHairStyle::Napoleon,
			AuHairStyle::Copstash,
			AuHairStyle::Gandalf,
			AuHairStyle::Confucius,
			AuHairStyle::Boris,
			AuHairStyle::Toothbrush,
			AuHairStyle::ChinPuff,
			AuHairStyle::Stripe,
			AuHairStyle::RapStandard,
			AuHairStyle::Pencil,
			AuHairStyle::Handlebar,
			AuHairStyle::HandlebarAndChinPuff,
			AuHairStyle::HandlebarAndGoatee,
			AuHairStyle::Wally,
			AuHairStyle::PencilPatch,
			//AuHairStyle::SHAVE_Count
		};
		unsigned int shavesCount = sizeof(shaves)/sizeof(AuHairStyle::SHAVE);
		shave = shaves[g_Au.Dice().Roll(1, shavesCount, -1)];
	}

	SetHairStyle(style, shave);
}

quint32 DaSentient::TurretFireMs() const
{
	// 1xp is gained for every 2 minutes of sitting in a turret, plus 1xp every time they fire the turret. That means an npc will
	// gain 720xp per real day if there's no action.
	m_SkillsLock.lockForRead();
	quint32 xp = m_Skills.Experience(AuSkill::Turret);
	m_SkillsLock.unlock();
	if(xp == 0)
        return 5000;
    float x = (float)xp / 10000.f;
    float fx = 1500.f / (5.f * x);
    quint32 ms = (quint32)ceilf(fx) + 500;
    if(ms > 5000)
        ms = 5000;
    return ms;
}

