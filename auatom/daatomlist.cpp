//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daatomlist.cpp,v 1.246 2022/04/14 14:48:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daatomlist.h"
//
#include "auatomset.h"
#include "auglobal.h"
#include "daperson.h"

// Returns true if the atom is removed.
bool DaAtomList::RemoveAtom(DaAtom* atom)
{
	for(iterator it = begin(); it != end(); it++)
	{
		if((*it) == atom)
		{
			erase(it);
			return true;
		}
	}
	return false;
}

// Remove the atom with this id, if there is one. (not recursive)
// Returns the atom pointer, if it was found.
DaAtom *DaAtomList::RemoveAtomId(const AuId& id)
{
	for(iterator it = begin(); it != end(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom->Is(id))
		{
			erase(it);
			return pAtom;
		}
	}
	return 0;
}

// Kills the atom with this id, if there is one.
// The dead atom remains in the list.
void DaAtomList::KillAtom(const AuId &id) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom->IsDead())
			continue;
		if(pAtom->Is(id))
		{
			//qDebug("DaAtomList::KillAtom - %d killed.", pAtom->Id().GetID());
			pAtom->Kill();		// Atom is killed.
		}
	}
}

// Kills all live atoms, if there are any.
// The dead atoms remain in the list.
void DaAtomList::KillAtoms() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom->IsDead())
			continue;
		//qDebug("DaAtomList::KillAtoms - %d killed.", pAtom->Id().GetID());
		pAtom->Kill();		// Atom is killed.
	}
}

void DaAtomList::TickOnClient(const AuElapsedTimer& last) const
{
	int atomIdx = 0;
	while(atomIdx < count())
	{
		// Tick the atom on the client.
		// Dormant atoms are not ticked at all.
		DaAtom* pAtom = operator[](atomIdx);
		if(pAtom && !pAtom->IsDormant())
			pAtom->TickOnClient(last);
		atomIdx++;
	}
}

void DaAtomList::TickTock() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		pAtom->TickTock();
	}
}

void DaAtomList::ListAtomsRecursive(DaAtomList &aList) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		aList.append(pAtom);
		pAtom->ListSubatomsRecursive(aList);
	}
}

void DaAtomList::ListNormalAtomsRecursive(DaAtomList &aList) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;
		aList.append(pAtom);
		pAtom->ListNormalSubatomsRecursive(aList);
	}
}

DaNpc* DaAtomList::FindNpcRecursive() const
{
    for(const_iterator it = cbegin(); it != cend(); it++)
    {
        DaAtom* pAtom = *it;
        if(!pAtom->IsNormal())
            continue;
        DaNpc* character = pAtom->FindNpcRecursive();
        if(character)
            return(character);
    }
    return 0;
}

DaNpc* DaAtomList::FindNpcRecursive(const QString& charName, quint32 storyInstance) const
{
    for(const_iterator it = cbegin(); it != cend(); it++)
    {
        DaAtom* pAtom = *it;
        if(!pAtom->IsNormal())
            continue;
        DaNpc* character = pAtom->FindNpcRecursive(charName, storyInstance);
        if(character)
            return(character);
    }
    return 0;
}

const DaAsteroid* DaAtomList::FindAsteroidRecursive(const AuId& targetId) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAsteroid* pFound = (*it)->FindAsteroidRecursive(targetId);
		if(pFound)
			return pFound;
	}
	return 0;
}

void DaAtomList::FindAsteroidsRecursive(DaAtomList& asteroids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;
		(*it)->FindAsteroidsRecursive(asteroids);
	}
}

const DaPerson* DaAtomList::FindAIPersonRecursive() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;
		const DaPerson* pFound = pAtom->FindAIPersonRecursive();
		if(pFound)
			return(pFound);
	}
	return 0;
}

const DaNpc* DaAtomList::FindOfficerRecursive() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;
		const DaNpc* pFound = pAtom->FindOfficerRecursive();
		if(pFound)
			return(pFound);
	}
	return 0;
}

// Finds creature atoms that are !IsDead. This means that people who are Normal or Travelling will be found.
DaCreature* DaAtomList::FindCreatureRecursive() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom->IsDead())
			continue;
		DaCreature *pFound = pAtom->FindCreatureRecursive();
		if(pFound)
			return(pFound);
	}
	return 0;
}

const DaPerson* DaAtomList::FindPersonOnLineRecursive() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;
		const DaPerson *pFound = pAtom->FindPersonOnLineRecursive();
		if(pFound)
			return(pFound);
	}
	return 0;
}

DaPerson* DaAtomList::FindPersonRecursive() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom->IsDead())
			continue;
		DaPerson *pFound = pAtom->FindPersonRecursive();
		if(pFound)
			return(pFound);
	}
	return 0;
}

void DaAtomList::FindPeopleOnLine(DaAtomList& people) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;
		DaPerson* person = dynamic_cast<DaPerson*>(*it);
		if(!person)
			continue;
		if(!person->IsOnLine())
			continue;
		people.append(person);
	}
}

void DaAtomList::FindPeopleOnLineRecursive(DaAtomList& people) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindPeopleOnLineRecursive(people);
}

void DaAtomList::FindPeopleAwakeRecursive(DaAtomList& people) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindPeopleAwakeRecursive(people);
}

void DaAtomList::FindPeopleOnLineRecursiveInStory(DaAtomList& people, quint32 storyInstance) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindPeopleOnLineRecursiveInStory(people, storyInstance);
}

void DaAtomList::FindBdRecursive(DaAtomList& bds) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindBdRecursive(bds);
}

void DaAtomList::FindBdRecursive(DaAtomList& bds, quint32 empireId) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindBdRecursive(bds, empireId);
}

void DaAtomList::FindBeaconsRecursive(DaAtomList& cities) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindBeaconsRecursive(cities);
}

void DaAtomList::FindCitiesBdRecursive(DaAtomList& cities) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindCitiesBdRecursive(cities);
}

void DaAtomList::FindCitiesBdRecursive(DaAtomList& cities, quint32 empireId) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindCitiesBdRecursive(cities, empireId);
}

void DaAtomList::FindCitiesBdRecursive(DaAtomList& supplyCities, DaAtomList& repairCities, DaAtomList& troopCities,
		quint32 empireId) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindCitiesBdRecursive(supplyCities, repairCities, troopCities, empireId);
}

void DaAtomList::FindNpcsRecursive(DaAtomList& npcs) const
{
    for(const_iterator it = cbegin(); it != cend(); it++)
        if((*it)->IsNormal())
            (*it)->FindNpcsRecursive(npcs);
}

void DaAtomList::FindPlanetsRecursive(DaAtomList& planets) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindPlanetsRecursive(planets);
}

void DaAtomList::FindTeleportersRecursive(DaAtomList& teleporters) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindTeleportersRecursive(teleporters);
}

void DaAtomList::FindLivePeopleRecursive(DaAtomList& people) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if(!(*it)->IsDead())
			(*it)->FindLivePeopleRecursive(people);
}

void DaAtomList::FindPeopleRecursive(DaAtomList& people) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindPeopleRecursive(people);
}

void DaAtomList::FindPirateStationsRecursive(DaAtomList& stations) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->FindPirateStationsRecursive(stations);
}

void DaAtomList::FindCapsulesRecursive(DaAtomList& capsules) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;
		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;
		(*it)->FindCapsulesRecursive(capsules);
	}
}

void DaAtomList::FindVesselsRecursive(DaAtomList& spaceships) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;
		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;
		(*it)->FindVesselsRecursive(spaceships);
	}
}

void DaAtomList::FindCitizensRecursive(DaAtomList& citizens) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindCitizensRecursive(citizens);
	}
}

void DaAtomList::FindVehiclesRecursive(DaAtomList& vehicles) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindVehiclesRecursive(vehicles);
	}
}

void DaAtomList::FindVehiclesRecursive(DaAtomList& vehicles, const AuId& motherShipId) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindVehiclesRecursive(vehicles, motherShipId);
	}
}

void DaAtomList::FindAnimalsRecursive(DaAtomList& animals) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindAnimalsRecursive(animals);
	}
}

void DaAtomList::FindCrewRecursive(const AuId& shipAssigned, DaAtomList& crewMembers) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindCrewRecursive(shipAssigned, crewMembers);
	}
}

void DaAtomList::FindShipMatesRecursive(const AuId& shipAssigned, DaAtomList& mates) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindShipMatesRecursive(shipAssigned, mates);
	}
}

void DaAtomList::FindItemsRecursive(DaAtomList& items, bool includeStaticDisplayedItems) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindItemsRecursive(items, includeStaticDisplayedItems);
	}
}

void DaAtomList::FindAIPeopleRecursive(const AuId& shipAssigned, DaAtomList& aipeople) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindAIPeopleRecursive(shipAssigned, aipeople);
	}
}

void DaAtomList::FindOfficersRecursive(const AuId& shipAssigned, DaAtomList& officers) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindOfficersRecursive(shipAssigned, officers);
	}
}

void DaAtomList::FindPassengersRecursive(const AuId& shipAssigned, DaAtomList& passengers) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindPassengersRecursive(shipAssigned, passengers);
	}
}

void DaAtomList::FindTroopsRecursive(const AuId& shipAssigned, DaAtomList& troops) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindTroopsRecursive(shipAssigned, troops);
	}
}

void DaAtomList::FindTroopsRecursive(quint32 empireId, DaAtomList& troops) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->FindTroopsRecursive(empireId, troops);
	}
}

// Adds an atom to the list if it's not already here.
// Test compares Id, not pointer value.
bool DaAtomList::AddIfUniqueId(DaAtom* pAdd)
{
	if(FindId(pAdd->Id()))
		return false;
	append(pAdd);
	return true;
}

// Returns count of live atoms in list.
int DaAtomList::LiveCount() const
{
	int liveAtoms = 0;
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom->IsDead())
			continue;
		liveAtoms++;
	}
	return liveAtoms;
}

void DaAtomList::InvalidateEnvironmentRecursive() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it)->InvalidateEnvironmentRecursive();
}

void DaAtomList::operator<<(QDataStream& in)
{
	clear();
	quint16 cnt;
	in >> cnt;
	while(cnt--)
	{
		quint8 atomType;
		in >> atomType;
		DaAtom* atom = DaAtom::Create((DaAtom::ATOM_TYPE)atomType);
		if(atom)
		{
			*atom << in;
			append(atom);
		}
	}
}

void DaAtomList::operator>>(QDataStream& out) const
{
	int liveAtoms = LiveCount();
	Q_ASSERT(liveAtoms <= 0xffff);	// We don't expect more than 65k atoms subordinate to any given atom.
	out << (quint16)liveAtoms;
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* atom = *it;
		if(atom->IsDead())
			continue;
		out << (quint8)atom->AtomType();
		*atom >> out;
	}
}

void DaAtomList::InvalidateEnvTransform() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->InvalidateEnvTransform();
	}
}

void DaAtomList::DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->DetectWarpSignatureRecursive(systemId, warpFactors, galaxyLoc);
	}
}

void DaAtomList::AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			(*it)->AdjustNavLocation(shipLoc, navLoc);
}

// Finds creature atoms that are !IsDead. This means that people who are Normal or Travelling will be found.
const DaAtom* DaAtomList::FindTrespasserRecursive(quint32 empireId, const AuPoint& systemLoc, float sensorRange) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		const DaAtom* trespasser = pAtom->FindTrespasserRecursive(empireId, systemLoc, sensorRange);
		if(trespasser)
			return(trespasser);
	}
	return 0;
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaAtomList::FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		pAtom->FindClosestDefenseBaseTargetRecursive(gunnerDomain, gunnerWorldId, gunnerEmpire, gunSysLoc, gunRange, targetAtom,
				targetRange, targetSysLoc);
	}
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaAtomList::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		pAtom->FindClosestShipTurretTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc,
				gunRange, gunSysVec, targetAtom, targetRange, targetSysLoc);
	}
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaAtomList::FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, const DaAtom*& targetAtom,
		double& targetRange, AuPoint& targetSysLoc) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		pAtom->FindClosestShipWeaponTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, shipSysLoc,
				gunRange, targetAtom, targetRange, targetSysLoc);
	}
}

void DaAtomList::FindClosestHarvesterTargetRecursive(int harvesterModule, const AuPoint& shipSysLoc, float gunRange,
		const DaAtom*& targetAtom, double& targetRange) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		pAtom->FindClosestHarvesterTargetRecursive((COMMODITYID)harvesterModule, shipSysLoc, gunRange, targetAtom, targetRange);
	}
}

void DaAtomList::FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
		SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
		double& targetRange) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		pAtom->FindClosestShipThreatRecursive(shipId, shipDomain, shipWorldId, shipEmpire, shieldPolicy, shipSysLoc, sensorRange,
				targetAtom, targetRange);
	}
}

void DaAtomList::FindClosestShipLockOnRecursive(int scannerDomain, const AuId& scannerWorldId, quint32 scannerEmpire,
		const AuId& scannerCapsuleId, const AuPoint& scannerSysLoc, float sensorRange,
		const DaVessel*& enemyVessel, double& vesselRange,
		const DaVessel*& nonEnemyStation, double& stationRange,
		const DaBd*& enemyMilBd, double& milRange,
		const DaBd*& anyBd, double& bdRange) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		const DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		pAtom->FindClosestShipLockOnRecursive(scannerDomain, scannerWorldId, scannerEmpire, scannerCapsuleId, scannerSysLoc,
				sensorRange, enemyVessel, vesselRange, nonEnemyStation, stationRange, enemyMilBd, milRange, anyBd, bdRange);
	}
}

void DaAtomList::StartVoiceConversationRecursive(const DaPerson& talker) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		if(!(*it)->IsNormal())
			continue;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		(*it)->StartVoiceConversationRecursive(talker);
	}
}

void DaAtomList::ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it)->ListDesignsRecursive(bdDesigns, scDesigns);
}

quint64 DaAtomList::MemoryUsage() const
{
	quint64 mem = sizeof(DaAtomList);
	for(const_iterator it = cbegin(); it != cend(); it++)
		mem += (*it)->MemoryUsage();
	return mem;
}

DaAtom* DaAtomList::RemoveRandom()
{
	if(empty())
		return 0;
	iterator ri;
	if(size() == 1)
		ri = begin();
	else
		ri = begin() + g_Au.Dice().Roll(1, (unsigned int)size(), -1);
	DaAtom* a = *ri;
	erase(ri);
	return a;
}

void DaAtomList::AvatarAccountActiveRecursive(const AuIdList& land) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it)->AvatarAccountActiveRecursive(land);
}

void DaAtomList::ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
		const QString& ownerName, quint32 ownerEmpire) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it)->ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
}

DaAtom* DaAtomList::FindId(const AuId& id) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->Is(id))
			return *it;
	return 0;
}

DaAtom* DaAtomList::FindIdRecursive(const AuId& id) const
{
	DaAtom* atom = FindId(id);
	if(atom)
		return atom;

	// recurse
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* atom = (*it)->FindIdRecursive(id);
		if(atom)
			return atom;
	}
	return 0;
}

DaAtom* DaAtomList::FindLiveIdRecursive(const AuId& id) const
{
	DaAtom* atom = FindId(id);
	if(atom)
	{
		if(atom->IsDead())
			return 0;
		return atom;
	}

	// recurse
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		DaAtom* atom = (*it)->FindLiveIdRecursive(id);
		if(atom)
			return atom;
	}
	return 0;
}

float DaAtomList::AsteroidBeltDensityRecursive(const AuPoint& envLoc) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		float abd = (*it)->AsteroidBeltDensityRecursive(envLoc);
		if(abd > 0)
			return abd;
	}
	return 0;
}

void DaAtomList::Append(const DaAtomList& atoms)
{
	for(const_iterator it = atoms.cbegin(); it != atoms.cend(); it++)
		append(*it);
}

void DaAtomList::AppendNormal(const DaAtomList& atoms)
{
	for(const_iterator it = atoms.cbegin(); it != atoms.cend(); it++)
		if((*it)->IsNormal())
			append(*it);
}

void DaAtomList::Append(const AuAtomSet& atoms)
{
	for(AuAtomSet::const_iterator it = atoms.cbegin(); it != atoms.cend(); it++)
		append(*it);
}

void DaAtomList::AppendNormal(const AuAtomSet& atoms)
{
	for(AuAtomSet::const_iterator it = atoms.cbegin(); it != atoms.cend(); it++)
		if((*it)->IsNormal())
			append(*it);
}

void DaAtomList::ListIds(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		ids.append((*it)->Id());
}

void DaAtomList::ListNormalIds(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			ids.append((*it)->Id());
}

void DaAtomList::ListIdsNoInstance(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if(!(*it)->IsInstanceUniverse())
			ids.append((*it)->Id());
}

void DaAtomList::ListNormalIdsNoInstance(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal() && !(*it)->IsInstanceUniverse())
			ids.append((*it)->Id());
}

void DaAtomList::DeleteContents()
{
	for(iterator it = begin(); it != end(); it++)
		delete *it;
	clear();
}

