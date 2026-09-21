//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dacapsule.h,v 1.54 2022/02/19 17:33:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaCapsule_H
#define DaCapsule_H

#include "aumission.h"
#include "auroster.h"
#include "daunit.h"

#define REPAIR_PER_SEC					100						// Hit points repaired per worker per second at a repair shop.

class AuDamage;
class AuDialog;
class AuGame;
class AuQuad;
class AuScPartSet;
class AuSensorBlip;
class AuStation;
class DaCreature;
class DaNpc;
class DaSentient;

// A capsule is a unit that other units can be subordinate to and move around inside of. Not like a vehicle where people are in
// designated spots. DaCapsule came into existence to provide common elements of DaBd and DaVessel; it cannot be instantiated.
class AUATOM_DLL DaCapsule : public DaUnit
{
public:
	DaCapsule(const QString& name, int idEmpire);
	DaCapsule();
	virtual ~DaCapsule();

protected:
	DaCapsule(const DaCapsule& c);

public:
	// 'troomIndex' is the index of the transporter room in the design analysis map of rooms.
	virtual void ActivateTransporterSystem(int troomIndex) = 0;

	// Be sure to call CanAddCargo first. This function doesn't check.
	virtual void AddCargo(const AuItem& iNew) = 0;

	// Locates an active transporter pad location in a room. Only searches for outgoing pads.
	// Returns false if none are active.
	virtual bool ActiveTransporterPadLocation(quint16 which, AuPoint& padLoc) const = 0;

	// Returns inside air pressure, scaled to a range of 0..1
	virtual float AirPressure(const QString& roomName) const = 0;

	// Returns true if any jammed door is found.
	// 'where' is an output parameter set to the location of the jammed door.
	virtual bool AnyDoorJammed(AuPoint& where) const = 0;

	virtual bool AnyHullDoorOpen() const = 0;

	// Called on client only for room air pressure on DaSc.
	virtual bool AnyHullDoorOpenInRoom(const QString& roomName) const = 0;

	virtual bool AnyHullDoorUnlocked() const = 0;

	// A person has requested beam-up.
	// returns
	// >0 number of teleporters created
	//  0 if 'targLoc' is blocked. Only occurs aboard a ship.
	// -1 if no transporters available
	// -2 if out of range
	// -3 location prevents transport
	// -4 no power for transport
	// -5 transporter cycle already active
	// -6 no empty vehicle parking spots
	// -10 blocked by ship shields at target
	// -11 subject vehicle is not in an appropriate parking spot
	// -12 subject is riding a creature that is riding a creature?
	// -13 blocked by city shields at target
	// -14 not equipped with a vehicle transporter
	// -15 no appropriate parking spots available at destination
	// -16 person's ship is not equipped with a vehicle transporter
	// -17 already in my transporter room
	int BeamUp(DaPerson* person, int countRequested);

	// Returns the berth detail location occupied by 'id'.
	// Returns a random location if the person is occupying the CaptianSlot.
	// Returns 0,0 if id is not in the roster.
	virtual AuPoint BerthLocation(const AuId& id) const = 0;

	// Returns the location of someone's berth. 'roomNumber' is an output parameter.
	virtual AuPoint BerthLocation(const AuId& id, quint16& roomNumber) const = 0;

	virtual AuRosterEntry::TYPE BerthTypeOccupied(const AuId& id) const = 0;

	// Returns:
	//  0=lift already at stop, door already open
	//  1=no door at turbo lift stop
	//  2=not ready to move
	//  3=a door is stuck open,
	//  4=broken turbo lift data
	//  5=turbo lift moved to stop and door opened
	//  6=lift already at stop, door opened
	//  7=a door is blocked open
	virtual int CallTurboLift(quint16 liftId, const QString& liftStop)
	{
		Q_UNUSED(liftId);
		Q_UNUSED(liftStop);
		return 4;
	}

	virtual bool CanAddCargo(const AuItem& iNew) const = 0;

	// 'stationId' is set to the part id in an AuScDesign.
	// 'emptySeatIndex' is set to the empty seat, if any, for multi-seat consoles like game tables. Otherwise 0.
	virtual bool CanEnterConsole(const DaPerson& who, quint16& stationId, AuStation& station, quint16& which) const = 0;

	// 'mLadder' is set to matrix needed to position the body on the ladder.
	virtual bool CanGrabLadder(const DaAtom& who, const AuPoint& loc, AuMatrix4x4& mLadder) = 0;

	// Returns true if 'who' is close enough to grab a ladder path at 'ladderPathLoc', which is 'who's next route location, known
	// to be the end of a ladder segment.
	virtual bool CanGrabLadderAt(const DaAtom& who, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder) = 0;

	virtual bool CanOpenDoor(const DaSentient& who, unsigned int doorIndex) const = 0;

	virtual float CapacitorLevel() const = 0;

	virtual bool CargoHas(COMMODITYID cid) const = 0;

	// Returns the height of the next solid ceiling above 'loc', considering open floors and stairs.
	// 'loc' is relative to this capsule.
	// 'isLocInside' is true if 'loc' is inside the hull, not on top or dangling from a ladder (old style designs need this).
	virtual double CeilingHt(const AuPoint& loc, const AuPoint& gravityN, bool isLocInside) const = 0;

	// Checks to see if 'unit' is granted access when calling for a beam up, recovery system, opening a hull door, etc.
	// Access does not override door locks.
	virtual bool CheckAccess(const DaUnit* unit) const = 0;

	virtual void CitizenPosts(AuScPartSet& posts) const
	{
		Q_UNUSED(posts);
	}

	virtual void CloseAllHullDoors() = 0;

	virtual void CloseDoor(unsigned int doorIndex) = 0;

	virtual void ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestGravityDrive, AuPoint& closestPowerPlant,
			AuPoint& closestSensor, AuPoint& closestFTLDrive, AuPoint& closestSiren) const = 0;

	// Returns door index to be used for subsequent call to OpenDoor, etc.
	// Returns -1 if none is found within 6'.
	// 'doorDistance' is an output parameter that holds the distance to the closest door that is closed.
	virtual int ClosestClosedDoor(const DaAtom* toWho, double& doorDistance) const = 0;

	// Returns door index to be used for subsequent call to OpenDoor, etc.
	// Returns -1 if none is found within 6'.
	// 'doorDistance' is an output parameter that holds the distance to the closest door.
	virtual int ClosestDoor(const AuPoint& toLoc, double& doorDistance) const = 0;

	// Returns door index to be used for subsequent call to CloseDoor, etc.
	// Returns -1 if none is found within 6'.
	// 'doorDistance' is an output parameter that holds the distance to the closest door that is open.
	virtual int ClosestOpenDoor(const DaAtom* toWho, double& doorDistance) const = 0;

	// Locates the closest unoccupied surgery unit to 'toLoc'.
	// 'stationId' returns the part id in an AuScDesign.
	// Ignores stations that have no path from toLoc to the station, accessible to 'isCrew'.
	// Ignores stations that already have an AI dude to them, unless toLoc is in the same room as the station.
	virtual bool ClosestUnoccupiedSurgeryUnit(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
			bool isCrew) const = 0;

	// Locates the closest empty vehicle parking spot in the same VehicleBay room as 'vehicle'. The empty test ignores the presense
	// of 'vehicle'.
	// Returns true if a spot is found.
	// 'parkPosM' is set on true return, undefined on false return.
	// 'spotType' is set on true return.
	virtual bool ClosestVehicleParkingSpot(const DaVehicle* vehicle, AuMatrix4x4& parkPosM, VEHICLE_SPOT_TYPE& spotType) const = 0;

	virtual bool CrewDoorLocked(unsigned int doorIndex) const = 0;

	// Builds 'damage' with entries for all systems.
	// 'damage' is expressed in hit points and it may contain parts.
	virtual void DamageReportHp(AuDamage& damage) const = 0;

	// Builds 'damage' with entries for equipment/nonstructural systems. Value in 'damage' is the system damage percent from 0 to
	// 100; 100 = destroyed. Parts are not added but their effect is included in the percentage calculation.
	// 'damage' will not contain parts.
	// 'structEquip' is a bitmask. b0 = structural systems, b1 = equipment systems, b2 = exclude undamaged systems.
	virtual void DamageReportPercent(AuDamage& damage, int structEquip) const = 0;

	QString DamageReportHtml() const;

	// Design CG offset from world CG, for old style designs that allowed the architect to move the CG.
	virtual AuPoint DesignCG() const
	{
		return AuPoint(0,0);
	}

	virtual void DischargeCapacitor(float units) = 0;

	// Perform transporter activation.
	// 'shipWhich' is this ship's transporter room which identifier
	// 'isFrom' is true when transporting from the ship, false when tranporting to the ship
	// 'pads' are the available pads in the transporter room on the ship
	// 'padLimit' is the number of pads to activate
	// 'targAtom' is parent atom of transport location. Typically a spaceship or terrain patch. Sometimes otherwise for deep space.
	// 'targLoc' relative to 'targAtom'. N/A when 'targPads' are supplied
	// 'targZ' is a unit up vector at 'targLoc'.
	// 'targWhich' set if target is a transporter room aboard a spacecraft or in a city.
	// 'targPads' filled if target is a transporter room aboard a spacecraft. Pad locs are relative to spacecraft 'targAtom'.
	// 'now' is the now time.
	// returns
	// >0 number of teleporters created
	//  0 if 'targLoc' is blocked. Only occurs aboard a ship.
	// -1 if no transporters available
	// -2 if out of range
	// -3 location prevents transport
	// -4 no power
	// -5 transporter is still active; wait for cycle to complete.
	// -6 no empty vehicle parking spots
	// -10 blocked by capsule shields at target
	// -13 blocked by city shields at target
	int EnergizeTransporter(quint16 shipWhich, bool isFrom, int padLimit, DaAtom* targAtom, const AuPoint& targLoc,
			const AuPoint& targZ, quint16 targWhich, AuPointArray& targPads, bool ignoreDestShields, bool targAtBd);

	int EnergizeVehicleTransporter(quint16 shipWhich, DaVehicle* vehicle, bool isFrom, DaAtom* targAtom,
			const AuPoint& targLoc, const AuPoint& targZ, quint16 targWhich, QList<AuMatrix4x4>& targParkingSpots,
			bool ignoreDestShields, bool targAtBd);

	// 'person' is the sender of 'msg'.
	bool EnergizeVehicleTransporter(const AuCommPacket* msg, bool isFrom, DaAtom* targAtom, const AuPoint& targLoc,
			const AuPoint& targZ, quint16 targWhich, QList<AuMatrix4x4>& targParkingSpots, AuCommStdChannel::RANGE limitedRange);

	// Returns a location for entry from the outside.
	// 'loc' is a starting location, relative to this ship.
	// 'doorIndex' and 'holeIndex' are output parameters, only one may be >= 0, both set to -1 if no door is found.
	// 'onGround' looks only for ground level doors and down-hatches on the second deck. Set false if pathing for a flyer.
	// 'onGround' looks for doors and openings that are at ground level or that have a ladder to reach ground level.
	// 'hullDoorsUsable' is true if hull doors can be opened readily by the caller.
	virtual AuPoint EntranceLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround,
			bool hullDoorsUsable) const = 0;

	// Returns a location for exit from the inside.
	// 'loc' is a starting location, relative to this ship.
	// 'doorIndex' and 'holeIndex' are output parameters, only one may be >= 0, both set to -1 if no door is found.
	// 'onGround' looks only for ground level doors and down-hatches on the second deck. Set false if pathing for a flyer.
	// 'onGround' looks for doors and openings that are at ground level or that have a ladder to reach ground level.
	virtual AuPoint ExitLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround) const = 0;

	// Locates an AI controlled citizen or demiavatar who is not at a station or post.
	DaSentient* FindAI(NPC_ROLE npcrole) const;

	// Locates an AI controlled citizen or demiavatar who is not at a station or post.
	DaSentient* FindAIInRoom(NPC_ROLE npcrole, const QString& roomName) const;

	virtual void FindCapsulesRecursive(DaAtomList& capsules) const;

	virtual DaSentient* FindDoctor(const DaCreature& patient) const;

	// Locates an npc who is not at a station or post.
	DaNpc* FindNpc(NPC_ROLE npcrole) const;

	// Locates an npc who is not at a station or post.
	DaNpc* FindNpcInRoom(NPC_ROLE npcrole, const QString& roomName) const;

	virtual DaCapsule* FindParentCapsuleInEnv();

	virtual const DaCapsule* FindParentCapsuleInEnv() const;

	// Locates a patient for 'doctor'.
	virtual DaCreature* FindPatient(const DaSentient& doctor) const = 0;

	// Locates an AI controlled sentient operating a transporter console in 'roomId'.
	DaSentient* FindTransporterOperator(quint16 roomId) const;

	// 'loc' is relative to this capsule.
	// 'htAbvFloor' is a return value, set to the height of the next solid floor below 'loc'.
	// 'inGravityWell', 'onStairs', and 'stairNormal' are return values, in the space of this capsule.
	// 'forWho' is the atom looking for the floor.
	// Returns true if 'loc' is above any solid floor formed by this capsule.
	// Returns false if 'loc' is not above any solid floor formed by this capsule.
	virtual bool FloorHt(const AuPoint& loc, const AuPoint& gravityN, double& htAbvFloor, bool& inGravityWell, bool& onStairs,
			AuPoint& stairNormal, bool isLocInside, const DaAtom* forWho) const = 0;

	// Returns a pathable floor cell location that is near a door location but not on the door.
	virtual AuPoint FloorLocInsideDoor(const AuPoint& doorLoc) const = 0;

	// Returns a user interface created by the current game, enabling the person to configure and submit their next turn when
	// it is their turn.
	virtual AuDialog* GameCreateTurnUI(quint16 whichTable, const AuId& personsTurn, QWidget* parent) = 0;

	// Exits a player from any game they're involved in. Terminates the game if it's the last person to leave.
	virtual void GameExitPlayer(quint16 whichTable, const AuId& playerId) = 0;

	// Selects a nu game for a game table, if there is no game currently active at that table.
	virtual bool GameSelected(quint16 whichTable, quint8 gameType, const AuId& dealerId) = 0;

	// Sets the players in a game according to who is at the same table as the dealer and their seating order.
	// Can be called on the client.
	virtual void GameSetPlayers(quint16 whichTable, AuGame* game, const AuId& dealerId) const = 0;

	// Starts the game at a table. Game settings are sent in a byte array so they are generic between all game types.
	virtual void GameStart(DaPerson* dealer, quint8 gameType, const QByteArray& gameSettings) = 0;

	// 'stationId' is the id of a game table. 'tableM' is filled with the position matrix for the table.
	// false is returned if the game table is not found.
	virtual bool GameTableLocation(quint16 stationId, AuMatrix4x4& tableM) const = 0;

	// Sends the game turn info when a player executes their turn.
	// Game turn data is sent in a byte array so it is generic between all game types.
	virtual void GameTurn(DaPerson* person, quint8 gameType, const QByteArray& gameTurn) = 0;

	// Checks to see if the game turn UI is still appropriate for the current game state.
	// Returns false if the turnUI should be deleted.
	virtual bool GameTurnUIValid(quint16 whichTable, const AuId& personsTurn, AuDialog* turnUI) = 0;

	virtual bool HasSpaceVehicleRecoverySystem() const
	{
		return false;
	}

	virtual bool HasTransporter() const = 0;

	virtual bool HasVehicleTransporter() const = 0;

	// Returns true if this ship has a working surgery unit aboard.
	virtual bool HasWorkingSurgeryUnit() const = 0;

	virtual bool HullDoorLocked(unsigned int doorIndex) const = 0;

	// Returns volume of the hull in cubic meters.
	virtual quint32 HullVolume() const = 0;

	// Returns true if 'atom' is in the chain of command of this capsule.
	// Returns true if 'atom' has agent power enabled.
	virtual bool InChainOfCommand(const DaAtom* atom) const = 0;

	// Returns true if the person is in the chain of command of this capsule.
	virtual bool InChainOfCommand(const AuId& personId, quint32 personEmpire) const = 0;

	// 'roomId' is set to the id of room if loc is a routable walking space inside the hull. 0 is outside space.
	// 'loc' is relative to this atom.
	// Returns true if 'loc' is in the routable path space of this capsule, false if not.
	virtual bool InsideRoom(const AuPoint& loc, quint16& roomId) const = 0;

	// Returns id of room if loc is a routable walking space inside the hull.
	// 'loc' is relative to this atom.
	// Returns 0 if not inside a room.
	// Sets 'roomType' if 'loc' is inside a room of this capsule.
	// On AuScDesign based ships, roomNum will be the part id.
	virtual quint16 InsideRoomType(const AuPoint& loc, ROOMTYPE& roomType, QString& roomName) const = 0;

	// Same as previous InsideRoomType except...
	// 'getRoomUp' is the caller wants 'gravityN' set to the void's gravity up vector, regardless of its gravity settings.
	//   regardless of external gravity conditions or void gravity settings.
	// 'roomId' is set if 'loc' is inside a pathable cell of this building. 'roomId' will be 0 if that
	//   pathable cell is not inside a room, i.e. it's on the roof or grounds that is not in a room of the design.
	// 'htAbvFloor' is set to the height of 'loc' above the floor, stairs or roof. This may be non-zero even though 'loc' is not
	//   inside any room. Height is above the floor of the current deck. Does not consider open floors. Use FloorHt for that.
	//   'htAbvFloor' may be 0 and this calculation is not performed.
	// 'gravityN' is the up direction at 'loc', with respect to gravity.
	// Returns true if 'loc' is in a pathable cell of this capsule.
	virtual bool InsideRoomType(const AuPoint& loc, bool getRoomUp, quint16& roomId, ROOMTYPE& roomType, double& htAbvFloor,
			QString& roomName, AuPoint& gravityN) const = 0;

	// Returns ambient light values for a room.
	// 'sunsBlocked' is set true in rooms that are not normally lit by sunlight.
	virtual void InteriorAmbientLight(const QString& roomName, float& ambientR, float& ambientG, float& ambientB,
			bool& sunsBlocked) const = 0;

	// This returns the current state of the light switch in a room. Check CapacitorLevel to insure lights can operate.
	virtual INTERIOR_LIGHT InteriorLight(const QString& roomName) const = 0;

	// Sets the state of all interior lights.
	// Returns true if any light states change.
	virtual bool InteriorLightToggle(INTERIOR_LIGHT state) = 0;

	// Sets the state of an interior light.
	// Returns true if light state changes.
	virtual bool InteriorLightToggle(const QString& roomName, INTERIOR_LIGHT state) = 0;

	// Return true if 'id' occupies any berth aboard.
	virtual bool IsBerthedHere(const AuId& id) const
	{
		Q_UNUSED(id);
		return false;
	}

	// Returns true if this capsule is currently under the command of an Npc officer.
	// The capsule is commanded by AI when an officer has the con.
	virtual bool IsCommandedByAI() const
	{
		return false;
	}

	// Test for a creature occupying a console or turret seat at loc.
	// The 'exceptId' atom is ignored.
	// This is used by AI dudes to see if anyone else is at the same console as them.
	bool IsConsoleOccupiedExcept(quint16 stationId, const AuId& exceptId) const;
	bool IsConsoleSeatOccupiedExcept(quint16 stationId, unsigned int seatIndex, const AuId& exceptId) const;

	virtual bool IsConsoleOperational(const AuSysId& sysId) const = 0;

	virtual bool IsCrewOrTroop(const AuId& id) const
	{
		Q_UNUSED(id);
		return false;
	}

	virtual bool IsDamaged() const = 0;

	virtual bool IsDesignDataCacheValid() const = 0;

	virtual bool IsDoorIndexValid(unsigned int doorIndex) const = 0;

	virtual bool IsDoorOnWall(unsigned int doorIndex) const = 0;

	virtual bool IsDoorOpen(unsigned int doorIndex) const = 0;

	virtual bool IsHidingInGroundClutter() const = 0;

	virtual bool IsHoleOnWall(unsigned int holeIndex) const = 0;

	// 'doorIndex' is as returned by ClosestDoor.
	virtual bool IsHullDoor(int doorIndex) const = 0;

	// Returns true if loc is in or on the hull.
	// 'on' the hull is defined as above a room within one deck height of the room.
	// 'loc' is relative to this atom.
	// returns 0=no, 1=in hull, 2=on hull, 3=under hull hatch
	virtual int IsInOrOnHull(const AuPoint& loc) const = 0;

	// Returns true if 'fromLoc' is on a ladder path to 'ladderPathLoc', which is a route location known to be the end of a ladder
	// segment.
	virtual bool IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc) const = 0;

	virtual bool IsNew() const
	{
		return false;
	}

	// Test for medical unit occupied by anyone, including people and ai controlled citizens.
	bool IsPatientStationOccupied(quint16 stationId) const;

	// True if the sensor power switch is on, regardless of whether it is functional.
	virtual bool IsSensorOn() const
	{
		return true;
	}

	virtual bool IsSpaceStation() const
	{
		return false;
	}

	// Returns true if 'roomId' is a transporter room.
	virtual bool IsTransporterRoom(quint16 roomId) const = 0;

	// Actively energized for transport. Always temporary.
	virtual bool IsTransporterSystemActive(int troomIndex) const = 0;

	// Merely powered on.
	virtual bool IsTransporterSystemOn(int troomIndex) const = 0;

	virtual void JamDoorClosed(unsigned int doorIndex) = 0;

	virtual void JamDoorOpen(unsigned int doorIndex) = 0;

	// Kicks AI controlled operator of a console, if there is one.
	virtual void KickAIStationOccupant(quint16 stationId, unsigned int seatIndex)
	{
		Q_UNUSED(stationId);
		Q_UNUSED(seatIndex);
	}

	// Test for life support functioning.
	virtual bool LifeSupportWorking() const = 0;

	// Returns a list of room names suitable for a go to room order.
	// Each room should return a unique name.
	// Only called on the client to offer order UI choices.
	virtual void ListRoomNames(QStringList& roomNames) const = 0;

	virtual void LivestockPosts(AuScPartSet& posts) const
	{
		Q_UNUSED(posts);
	}

	virtual AuItemHash& LockCargoForWrite() = 0;

	virtual const AuItemHash& LockCargoForRead() const = 0;

	virtual AuId LockedBlip() const
	{
		return AuId();
	}

	// NOTE: Only valid on the client.
	virtual bool LockedIsCivilian() const
	{
		return false;
	}

	// NOTE: Only valid on the client.
	virtual POLITICAL_STANCE LockedStance() const
	{
		return POL_Neutral;
	}

	// NOTE: Only valid on the client.
	virtual AuPoint LockedSysLoc() const
	{
		return AuPoint();
	}

	virtual void LockOn(const AuId& lockOnId, bool isLockedByHuman)
	{
		Q_UNUSED(lockOnId);
		Q_UNUSED(isLockedByHuman);
	}

	// Calls LockEnviron on all atoms in the lift 'liftId'.
	virtual void LockLiftContents(quint16 liftId, DaAtomList& atomsInLift) = 0;

	// Notifies a commanding AI officer of an event worth noting in the ship's log.
	// Nothing happens if there is no AI officer aboard to log the event.
	virtual void LogEvent(AuEventLogEntry::TYPE type, const QString& desc, const AuId& atomId=AuId()) = 0;

	// Return the maximum range of the largest bay weapon, whether or not it's operational.
	virtual float MaxBayWeaponRange() const = 0;

	virtual AuMissionOrder::TYPE MissionOrderType() const
	{
		return AuMissionOrder::Invalid;
	}

	virtual bool MoonOrbitRingsVisible(const QString& roomName) const
	{
		Q_UNUSED(roomName);
		return false;
	}

	// If any AI aboard is enemy, the crewman cannot teleport to his station.
	// If any person aboard who is online is neutral or enemy, the crewman cannot teleport to his station.
	// If any wild animal is aboard, the crewman cannot teleport to his station.
	virtual bool OkToTeleportToStation(const DaSentient* crewman) const
	{
		Q_UNUSED(crewman);
		return false;
	}

	// Returns false if the door could not be opened. e.g. turbo lift at another stop and can't be moved.
	virtual bool OpenDoor(unsigned int doorIndex) = 0;

	virtual bool OrbitRingsVisible(const QString& roomName) const
	{
		Q_UNUSED(roomName);
		return false;
	}

	// Locates the patient station associated with 'medicStationId'.
	// Sets 'surgeryId' to the part id of the surgery unit in an AuScDesign.
	// Sets 'station' to the patient position of the surgery unit, relative to this ship.
	// In an AuScDesign, the surgery unit id is the same as the 'medicStationId'.
	// Does not test for the surgery being occupied or operational.
	virtual void PatientStation(quint16 medicStationId, quint16& surgeryId, AuStation& station) const = 0;

	virtual void PerformSensorConsoleScan(AuSensorBlipList& sensorBlips) const = 0;

	virtual void PerformTransporterConsoleScan(AuSensorBlipList& sensorBlips, bool showAboard) const = 0;

	virtual void PositionTurret(const DaCreature& gunner)
	{
		Q_UNUSED(gunner);
	}

	// Reloads turret with current ordnance. Picks nu ordnance at random if current ordnance is gone.
	// Does nothing if current ordnance is full of ammo.
	// Returns true if the turret is loaded.
	virtual bool ReloadTurret(quint16 which) = 0;

	// Returns the location of 'cameraId'.
	virtual AuPoint RemoteCameraLocation(quint16 cameraId) const
	{
		Q_UNUSED(cameraId);
		return AuPoint();
	}

	// Returns the next camera id after 'currentCamera'. 'currentCamera' may be 0 to start the sequence.
	// Returns 0 if there are no cameras.
	virtual quint16 RemoteCameraNext(quint16 currentCamera) const
	{
		Q_UNUSED(currentCamera);
		return 0;
	}

	// Returns the previous camera id before 'currentCamera'. 'currentCamera' may be 0 to start the sequence.
	// Returns 0 if there are no cameras.
	virtual quint16 RemoteCameraPrevious(quint16 currentCamera) const
	{
		Q_UNUSED(currentCamera);
		return 0;
	}

	// Returns true if 'cameraId' identifies a valid remote camera.
	virtual bool RemoteCameraVerify(quint16 cameraId) const
	{
		Q_UNUSED(cameraId);
		return false;
	}

	// Looks for a remotely operable door.
	// 'forVehicle' is true for a vehicle, false for a spacecraft.
	// Returns the door index of the remotely operable door closest to 'loc', or -1 if no door is near.
	// Use this one when looking for a door from the outside.
	virtual int RemoteDoorIndex(bool forVehicle, const AuPoint& loc) const = 0;

	// Looks for a remotely operable door.
	// 'forVehicle' is true for a vehicle, false for a spacecraft.
	// Returns the door index of the remotely operable door closest to 'loc' that goes into 'roomId', or -1 if no door is near.
	// Use this one when looking for a door from the inside.
	virtual int RemoteDoorIndex(bool forVehicle, const AuPoint& loc, quint16 roomId) const = 0;

	virtual bool RemoveCargo(const AuItem& iTake, quint32 howMany) = 0;

	// Removes a unit of fuel. Used when servicing vehicles.
	virtual bool RemoveFuel(COMMODITYID fuelCID) = 0;

	// A pilot in a vehicle aboard this spacecraft is requesting launch.
	virtual void RequestLaunch(DaCreature* pilot, DaVehicle* vehicle)
	{
		Q_UNUSED(pilot);
		Q_UNUSED(vehicle);
	}

	// A pilot in a vehicle that is not aboard this spacecraft is requesting recovery. 'vehLoc' is their vehicle's location
	// relative to this capsule.
	virtual void RequestRecovery(DaCreature* pilot, DaVehicle* vehicle, const AuPoint& vehLoc)
	{
		Q_UNUSED(pilot);
		Q_UNUSED(vehicle);
		Q_UNUSED(vehLoc);
	}

	virtual bool RoomHasNavHUD(const QString& roomName) const
	{
		Q_UNUSED(roomName);
		return false;
	}

	// Sets 'orderLoc' to the path destination location for an Npc who is ordered to go to a room named 'roomName'. The name may
	// be one of those returned by the ListRoomNames function, which tends to be 'room name N' where N is the room id. It could
	// also be a fragment, like 'bridge'. Capitalization is ignored.
	// Called on the server when AI wants to carry out its go to room order.
	// Returns true if a location in the room is found, false if not.
	virtual bool RoomOrderLoc(const QString& roomName, AuPoint& orderLoc) const = 0;

	virtual double RoughRadius() const = 0;

	// Selects weapon to fire from bay. Picks nu ordnance at random if current ordnance is gone.
	// Does nothing if current ordnance is available.
	// Returns true if the weapon bay selection is changed.
	// Returns false if 'which' bay is not configured with a weapon bay module.
	virtual bool SelectBayWeapon(quint16 which) = 0;

	virtual bool SensorsWorking() const = 0;

	// Returns a sentient subordinate to me who is currently going to (or manning) 'stationId'.
	DaSentient* SentientGoingToStation(quint16 stationId) const;

	// 'byWho' just entered a console.
	virtual void SetConsoleEntered(const DaCreature* byWho)
	{
		Q_UNUSED(byWho);
	}

	virtual bool SetMoonOrbitRingsVisible(const QString& roomName, bool visible)
	{
		Q_UNUSED(roomName);
		Q_UNUSED(visible);
		return false;
	}

	virtual bool SetOrbitRingsVisible(const QString& roomName, bool visible)
	{
		Q_UNUSED(roomName);
		Q_UNUSED(visible);
		return false;
	}

	virtual bool SetSystemNamesVisible(const QString& roomName, int visible)
	{
		Q_UNUSED(roomName);
		Q_UNUSED(visible);
		return false;
	}

	virtual bool SetWormholeLinesVisible(const QString& roomName, bool visible)
	{
		Q_UNUSED(roomName);
		Q_UNUSED(visible);
		return false;
	}

	virtual bool ShieldsBlockTransportAboard() const = 0;

	virtual bool ShieldsBlockWeapons() const = 0;

	virtual bool ShieldContains(const AuPoint& loc) const = 0;

	// Returns true if 'loc' is on or near (within 5m of?) the shield bubble surface.
	virtual bool ShieldSurfaceNear(const AuPoint& loc) const = 0;

	// Test for shields up|down and functioning.
	virtual bool ShieldsWorkingAndOn() const = 0;

	// Test for shields up and functioning.
	virtual bool ShieldsWorkingAndUp() const = 0;

	// Returns the system id that corresponds to a 'sightedPart'.
	// Returns default constructed AuSysId if 'sightedPart' is invalid.
	virtual AuSysId SightedSystem(quint16 sightedPart) const = 0;

	// Gets station for 'partId'.
	virtual bool Station(quint16 partId, AuStation& station) const
	{
		Q_UNUSED(partId);
		Q_UNUSED(station);
		return false;
	}

	// Gets the 'station' at which 'who' is sitting/standing, if any.
	// Returns true on success, false if 'who' is not at a station.
	virtual bool Station(const DaCreature& who, AuStation& station) const
	{
		Q_UNUSED(who);
		Q_UNUSED(station);
		return false;
	}

	// Retrieves the screen quad for 'stationId'.
	// Returns true on success, false if design is not loaded/ready.
	virtual bool StationScreen(quint16 stationId, AuQuad& screen) const
	{
		Q_UNUSED(stationId);
		Q_UNUSED(screen);
		return false;
	}

	// Returns the number of hit points of 'sysId' when it is at maximum health.
	virtual quint32 SystemHits(const AuSysId& sysId) const = 0;

	// 'which' identifies a particular subsystem
	virtual QString SystemName(const AuSysId& sysId) const = 0;

	virtual int SystemNamesVisible(const QString& roomName) const
	{
		Q_UNUSED(roomName);
		return 0;
	}

	// Current quality of a system.
	// Initially quality at which the ship was manufactured. This may be greater than the quality of the design.
	virtual quint8 SystemQuality(SYSTEM system) const = 0;

	// Current quality of a system.
	// Initially quality at which the ship was manufactured. This may be greater than the quality of the design.
	virtual quint8 SystemQuality(SYSTEM system, quint16 which) const = 0;

	virtual int TestMissileHit(const AuPoint& missileLoc, float proximity) const;

	// Test a location to see if it is clear as a transporter destination.
	virtual bool TestTransportLocation(const AuPoint& loc) const = 0;

	// Gives the person a thought message of the result.
	virtual void ToggleDoorLock(const DaPerson& person, unsigned int targetPart) = 0;

	virtual void ToggleDoorState(const DaSentient& who, unsigned int doorIndex) = 0;

	// Toggle state of door.
	virtual void ToggleHullDoor(int doorIndex) = 0;

	virtual COMMODITYID TransporterModule(quint16 which) const = 0;

	// List all transporter pad locations in a given 'which' transporter room.
	virtual void TransporterPadLocations(quint16 which, AuPointArray& pads) const = 0;

	virtual int TransporterPadsWorking(quint16 which) const = 0;

	virtual float TransporterRange(quint16 which) const = 0;

	// Returns the 'which' id of a transporter system, preferably with at least padsDesired functioning pads.
	// Returns 0xffff on failure.
	virtual quint16 TransporterSystemId(int padsDesired, bool mustBeOn) const = 0;

	virtual int TransporterSystemIndex(quint16 which) const = 0;

	// Returns the number of different transporter systems.
	virtual int TransporterSystems() const = 0;

	// Returns the which identifiers for all of my transporter systems.
	virtual QList<quint16> TransporterSystemWhiches() const = 0;

	// Randomly locates a point inside the hull to which to transport.
	// Returns false if it doesn't find a clear location within a reasonable amount of searching. Another try might succeed.
	// 'voidLoc' is an output parameter, only valid if return result is true.
	virtual bool TransporterVoidLocation(AuPoint& voidLoc) const = 0;

	// Return the 'which' identifier for the transporter room that produced the 'blip'. It will be a transporter room blip.
	virtual quint16 TransporterWhich(const AuSensorBlip& blip) const = 0;

	// Returns the transporter system which identifier for 'atom'.
	virtual quint16 TransporterWhich(const DaAtom& atom) const = 0;

	virtual void TroopPosts(AuScPartSet& posts) const
	{
		Q_UNUSED(posts);
	}

	// 'liftStops' is filled with the lift stops available to 'liftId'.
	virtual QString TurboLiftStop(quint16 liftId, QStringList& liftStops) const
	{
		Q_UNUSED(liftId);
		Q_UNUSED(liftStops);
		return "";
	}

	virtual void TurnSensorsOff()
	{
	}

	virtual void TurnSensorsOn()
	{
	}

	virtual void TurnTransporterSystemOff(int troomIndex) = 0;

	virtual void TurnTransporterSystemOn(int troomIndex) = 0;

	virtual AuItem* TurretAmmunitionClone(quint16 which) const = 0;

	virtual void TxReplyChat(const QString& replyText, const AuCommPacket& msg, AuCommStdChannel::RANGE range) = 0;

	virtual void UnlockCargo() const = 0;

	void UnlockLiftContents(const DaAtomList& lockedAtoms);

	// 'parkingSpots' is filled with appropriate parking spots.
	// Parking spots may be not be empty. Does not check for existing vehicles.
	virtual void VehicleParkingSpots(VEHICLE_SPOT_TYPE spotType, QList<AuMatrix4x4>& parkingSpots) const = 0;

	// 'parkingSpot' is filled with an appropriate parking spot.
	// 'parked' is set to the vehicle that is already parked there, if it can be replaced. Prefers to find a spot with no vehicle.
	// Returns true if a spot is found, with or without a 'parked' vehicle.
	bool VehicleTransporterSpot(VEHICLE_SPOT_TYPE spotType, AuMatrix4x4& parkingSpot, DaVehicle*& parked) const;

	// Returns the 'which' id of a vehicle transporter system.
	// Returns 0xffff on failure.
	virtual quint16 VehicleTransporterSystemId(VEHICLE_SPOT_TYPE spotType, bool mustBeOn) const = 0;

	virtual void WakeDoctor(const DaCreature& patient) = 0;

	virtual void WorldCaptured(quint32 byEmpire);

	virtual bool WormholeLinesVisible(const QString& roomName) const
	{
		Q_UNUSED(roomName);
		return false;
	}
};

#endif // DaCapsule_H
