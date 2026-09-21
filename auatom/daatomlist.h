//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daatomlist.h,v 1.171 2022/04/15 00:35:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaAtomList_H
#define DaAtomList_H

#include "auatom.h"
#include "audef.h"
#include <QList>

class AuAtomSet;
class AuDomain;
class AuElapsedTimer;
class AuId;
class AuIdList;
class AuPoint;
class DaAsteroid;
class DaAtom;
class DaBd;
class DaCapsule;
class DaCreature;
class DaNpc;
class DaPerson;
class DaVessel;
class QDataStream;

class AUATOM_DLL DaAtomList : public QList<DaAtom*>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	// Adds an atom to the list if it's not already here.
	// Test compares pointer value, not Id.
	inline bool AddIfUnique(DaAtom* pAdd)
	{
		if(contains(pAdd))
			return false;
		append(pAdd);
		return true;
	}

	// Adds an atom to the list if it's not already here.
	// Test compares Id, not pointer value.
	bool AddIfUniqueId(DaAtom* pAdd);

	// Adjusts 'navLoc' so it is not inside the atmosphere/photosphere of any planets/stars or other objects a navigator would
	// perceive.
	// Result will be a close orbit radius outside of those.
	// 'shipLoc' is the reference loc for deciding where to move 'navLoc'. It is probably the location of a navigator's ship.
	// 'shipLoc' and 'navLoc' are in solar system coordinates.
	// Recursive.
	void AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const;

	// Appends all the atoms in 'atoms' to this list. Does not make clones. Copies the pointers.
	void Append(const DaAtomList& atoms);
	void Append(const AuAtomSet& atoms);

	// Appends the normal atoms in 'atoms' to this list. Does not make clones. Copies the pointers.
	void AppendNormal(const DaAtomList& atoms);
	void AppendNormal(const AuAtomSet& atoms);

	float AsteroidBeltDensityRecursive(const AuPoint& envLoc) const;

	void AvatarAccountActiveRecursive(const AuIdList& land) const;

	void ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId, const QString& ownerName,
			quint32 ownerEmpire) const;

	// Clears this list and performs a simple 'delete' on every atom. Does not involve the ether.
	void DeleteContents();

	void DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc) const;

	// Recursively invalidates the system transform the atoms in this list.
	void InvalidateEnvTransform() const;

	// Remove the atom with this id, if there is one. (not a recursive search)
	// Returns the atom pointer, if it was found.
	DaAtom* RemoveAtomId(const AuId &id);

	// Returns true if the atom is removed.
	bool RemoveAtom(DaAtom* atom);

	// Removes a random atom and returns it. Returns 0 if empty on entry.
	DaAtom* RemoveRandom();

	void FindAIPeopleRecursive(const AuId& shipAssigned, DaAtomList& aipeople) const;

	const DaPerson* FindAIPersonRecursive() const;

	void FindAnimalsRecursive(DaAtomList& animals) const;

	const DaAsteroid* FindAsteroidRecursive(const AuId& targetId) const;

	void FindAsteroidsRecursive(DaAtomList& asteroids) const;

	// Recursively finds all Bd structures.
	void FindBdRecursive(DaAtomList& bds) const;

	// Recursively finds all Bd structures that belong to 'empireId'.
	void FindBdRecursive(DaAtomList& bds, quint32 empireId) const;

	void FindBeaconsRecursive(DaAtomList& beacons) const;

	void FindCapsulesRecursive(DaAtomList& capsules) const;

	DaNpc* FindCharacterRecursive() const;

	DaNpc* FindCharacterRecursive(const QString& charName, quint32 storyInstance) const;

	// Recursively finds all Bd capitols and Bd HQs.
	void FindCitiesBdRecursive(DaAtomList& cities) const;

	// Recursively finds all Bd capitols and Bd HQs that belong to a particular empire.
	void FindCitiesBdRecursive(DaAtomList& cities, quint32 empireId) const;

	// Recursively finds cities and bds that belong to empireId.
	// 'supplyCities' is filled with cities and bds that could supply a spacecraft.
	// 'repairCities' is filled with cities and bds that could supply and repair a spacecraft.
	// 'troopCities' is filled with cities and bds that could deploy troops to a spacecraft.
	void FindCitiesBdRecursive(DaAtomList& supplyCities, DaAtomList& repairCities, DaAtomList& troopCities, quint32 empireId) const;

	void FindCitizensRecursive(DaAtomList& citizens) const;

	// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	void FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// 'harvesterModule' is a COMMODITYID
	void FindClosestHarvesterTargetRecursive(int harvesterModule, const AuPoint& shipSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange) const;

	void FindClosestShipLockOnRecursive(int scannerDomain, const AuId& scannerWorldId, quint32 scannerEmpire,
			const AuId& scannerCapsuleId, const AuPoint& scannerSysLoc, float sensorRange,
			const DaVessel*& enemyVessel, double& vesselRange,
			const DaVessel*& nonEnemyStation, double& stationRange,
			const DaBd*& enemyMilBd, double& milRange,	// Enemy military building
			const DaBd*& anyBd, double& bdRange) const;

	void FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
			SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
			double& targetRange) const;

	// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	void FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, const DaAtom*& targetAtom,
			double& targetRange, AuPoint& targetSysLoc) const;

	// Finds creature atoms that are !IsDead. This means that people who are Normal or Travelling will be found.
	DaCreature* FindCreatureRecursive() const;

	void FindCrewRecursive(const AuId& shipAssigned, DaAtomList& crewMembers) const;

	// Searches for any atom with the 'id'. Not recursive.
	DaAtom* FindId(const AuId& id) const;

	// Searches recursively for a normal atom with id.
	DaAtom* FindIdRecursive(const AuId &id) const;

	void FindItemsRecursive(DaAtomList& items, bool includeStaticDisplayedItems) const;

	// Searches recursively for a normal atom with id.
	DaAtom* FindLiveIdRecursive(const AuId &id) const;

	// Finds !IsDead people, which gets IsNormal and IsTraveling people.
	void FindLivePeopleRecursive(DaAtomList& people) const;

	void FindOfficersRecursive(const AuId& shipAssigned, DaAtomList& officers) const;

	const DaNpc* FindOfficerRecursive() const;

	void FindPassengersRecursive(const AuId& shipAssigned, DaAtomList& passengers) const;

	void FindPeopleAwakeRecursive(DaAtomList& people) const;

	void FindPeopleOnLine(DaAtomList& people) const;

	void FindPeopleOnLineRecursive(DaAtomList& people) const;

	void FindPeopleOnLineRecursiveInStory(DaAtomList& people, quint32 storyInstance) const;

	// Finds IsNormal people.
	void FindPeopleRecursive(DaAtomList& people) const;

	const DaPerson* FindPersonOnLineRecursive() const;

	// Finds person atoms that are !IsDead. This means that people who are Normal or Travelling will be found.
	DaPerson* FindPersonRecursive() const;

	void FindPirateStationsRecursive(DaAtomList& stations) const;

	void FindPlanetsRecursive(DaAtomList& planets) const;

	void FindShipMatesRecursive(const AuId& shipAssigned, DaAtomList& mates) const;

	void FindTeleportersRecursive(DaAtomList& teleporters) const;

	// Called by stations and cities.
	// Searches recursively for any trespassing cities or stations.
	const DaAtom* FindTrespasserRecursive(quint32 empireId, const AuPoint& systemLoc, float sensorRange) const;

	void FindTroopsRecursive(const AuId& shipAssigned, DaAtomList& troops) const;

	void FindTroopsRecursive(quint32 empireId, DaAtomList& troops) const;

	void FindVehiclesRecursive(DaAtomList& vehicles) const;

	void FindVehiclesRecursive(DaAtomList& vehicles, const AuId& motherShipId) const;

	void FindVesselsRecursive(DaAtomList& spaceships) const;

	void InvalidateEnvironmentRecursive() const;

	// Kills an atom and its subatoms recursively.
	// The corresponding atom record is deleted from the database.
	void KillAtom(const AuId &id) const;

	// Kills all atoms and their subatoms recursively.
	// The corresponding atom records are deleted from the database.
	void KillAtoms() const;

	// List all atom pointers recursively into one linear list.
	void ListAtomsRecursive(DaAtomList &aList) const;

	void ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const;

	// List IsNormal atom pointers recursively into one linear list.
	void ListNormalAtomsRecursive(DaAtomList &aList) const;

	void ListIds(AuIdList& ids) const;

	void ListNormalIds(AuIdList& ids) const;

	void ListIdsNoInstance(AuIdList& ids) const;

	void ListNormalIdsNoInstance(AuIdList& ids) const;

	// Returns count of atoms that are not IsDead.
	int LiveCount() const;

	quint64 MemoryUsage() const;

	void StartVoiceConversationRecursive(const DaPerson& talker) const;

	void TickOnClient(const AuElapsedTimer& last) const;

	void TickTock() const;
};

inline QDataStream& operator<<(QDataStream& out, const DaAtomList& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DaAtomList& o)
{
	o << in;
	return in;
}

#endif // DaAtomList_H
