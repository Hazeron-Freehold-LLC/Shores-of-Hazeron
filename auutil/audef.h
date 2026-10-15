//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audef.h,v 1.140 2022/03/04 21:47:46 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDef_H
#define AuDef_H

#include <math.h>
#include <QColor>
#include <QDataStream>
#include <QObject>
#include <QString>
#include <QUrl>

#define d_LogByteRange					63						// NEVER CHANGE THIS VALUE! The highest possible value is 255, which
																// results in tiny increments between species. That doesn't allow
																// much variance within a species. Some experimenting got to this
																// value. Results generated using this value are stored in the
																// AuSpecies.m_Key field. Changes would affect all stored AuSpecies.

#define Free2Play												// Disables requirement to pay money to play.

namespace AuDef
{
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Enumerated Types
	// Access policy determines who may request landing and remotely open hull doors.
	enum ACCESS_POLICY
	{
		ACCESS_Command,
		ACCESS_Crew,
		ACCESS_CrewAndPassengers,
		ACCESS_Empire,
		ACCESS_Friend,
		ACCESS_Neutral,
		ACCESS_Enemy,
	};

	// This is the g_Player.status field value in the login database.
	enum ACCOUNT
	{
		ACCT_Active			= 0,
		ACCT_Expired		= 1,
		ACCT_Server			= 2,
		ACCT_Agent			= 3,
		ACCT_Programmer		= 4,
		ACCT_Architect		= 5,
		ACCT_Minion			= 6,
		ACCT_Trial			= 7,
		ACCT_LockedOut		= 8,
		ACCT_Count
	};

	enum AGENTBITS
	{
		AGENT_Animal		= 0x01,		// Agent is identified as an animal when spotted. Does not show up in players online list.
		AGENT_Entity		= 0x02,		// Agent is rendered as disembodied entity and cannot be spotted. Does not show as online.
		AGENT_Invisible		= 0x04,		// Agent is invisible and cannot be spotted.
		AGENT_Enable		= 0x08,		// Agent power is enabled.
		AGENT_Incognito		= 0x10,		// Agent is not highlighted in player on-line queries. Haxus doesn't show up at all.
		AGENT_Invincible	= 0x20,		// Agent cannot be harmed.
	};

	enum ALERT_CONDITION
	{
		ConditionGreen,
		ConditionYellow,
		ConditionRed
	};

	enum ATMTYPE
	{
		ATMTYPE_Standard,		// Breathable without equipment if density is 15-85.
		ATMTYPE_Tainted,		// Breathable with respirator if density is 15-85.
		ATMTYPE_Exotic,			// Supports plant life. Environment suit needed.
		ATMTYPE_Corrosive,
		ATMTYPE_Insidious,
		ATMTYPE_Count
	};

	// Types of bodies in a solar system.
	enum BODY_TYPE
	{
		BODY_GasGiant,
		BODY_Titan,						// Titans are large globe satellites with an atmosphere.
		BODY_Moon,						// Moons are small globe satellites with no atmosphere.
		BODY_Planet,					// Planets are the primary worlds in a system, apart from gas giants.
		BODY_Ringworld,					// A ringworld.
		BODY_Ring,						// A ring of rocks and ice around a planet.
		BODY_Planetoid,					// Planetoid in an asteroid belt.
		BODY_TypeOStar,
		BODY_TypeBStar,
		BODY_TypeAStar,
		BODY_TypeFStar,
		BODY_TypeGStar,
		BODY_TypeKStar,
		BODY_TypeMStar,
		BODY_SupermassiveBlackHole,
		BODY_StellarBlackHole,
		BODY_NeutronStar,
		BODY_SolarSystemSpace,
		BODY_SectorSpace,
		BODY_TYPE_Count						// LAST ITEM
	};

	// Update DpBuilding::operator<< and DpBuilding::operator>> if the value of this enum exceeds 3 bits (when rotated to the
	// right 20 bits).
	enum BUILDMATERIAL
	{
		BUILDMAT_RoughWood		= 0x00000000,
		BUILDMAT_MilledLumber	= 0x00100000,
		BUILDMAT_Stone			= 0x00200000,
		BUILDMAT_Metal			= 0x00300000,
		BUILDMAT_Magmium		= 0x00400000,
		BUILDMAT_Vulcium		= 0x00500000,
		BUILDMAT_Adamantine		= 0x00600000
	};

	// Bits indicate what environments are ok to build a blueprint.
	// Environment bits indicate if a blueprint is structurally capable and has adequate life support for a given environment.
	// Environment bits cannot consider a building's purpose (its INDUSTRYID).
	enum BUILDING_ENVIRONMENT
	{
		BENV_HabitableLand		= 0x01,		// Habitable environment land.
		BENV_UnbreathableLand	= 0x02,		// Uninhabitable environment land, inner through outer zones.
		BENV_FrigidLand			= 0x04,		// Land on frigid world.
		BENV_InfernoLand		= 0x08,		// Land on inferno world.
		BENV_SeaBottom			= 0x10,		// Bottom of water sea.
		BENV_FrigidSeaBottom	= 0x20,		// Bottom of supercooled liquid sea on frigid world.
		BENV_GasGiantCore		= 0x40,		// Bottom of supercooled liquid at core of gas giant.
		BENV_All				= 0x7f,		// All building environment bits on.
	};

	enum BUILDING_STRUCTURE
	{
		BD_Invalid = 0,					// Invalid building structure value.
		BD_Biodome = 1,					// Biodome enclosure.
		BD_Bridge = 2,					// Bridge across terrain.
		BD_Building = 3,				// Typical building.
		BD_Fence = 4,					// Fence or wall.
		BD_SeaPlatform = 5,				// Floating platform for other buildings to be constructed upon.
		BD_AerialPlatform = 6,			// Flying platform for other buildings to be constructed upon.
		BD_GroundPlatform = 7,			// Ground platform for other buildings to be constructed upon.
		BD_RoadEnclosure = 8,			// Enclosure for roads.
		BD_SeaSurfaceBuilding = 9,		// Oil rig.
		BD_SpaceElevator = 10,			// Ground to orbit.
		BD_Statue = 11,					// For parks.
		BD_Tunnel = 12,					// Tunnel through terrain.
		BD_StreetLight = 13,			// Street light.
		BD_Count						// LAST ITEM.
	};

	enum GROWTHSTAGE
	{
		Juvenile,
		Mature
	};

	// Orders that can be issued to troops and other citizens.
	enum CITIZEN_ORDER
	{
		ORDER_Stay = 0,					// Target atom is the parent of the atom who receives the order.
		ORDER_AttackTarget,				// Target atom is the atom to attack.
		ORDER_ComeHere,					// Target atom is the parent atom of the atom who issued the order.
		ORDER_ComeHereExactly,			// Target atom is the parent atom of the atom who issued the order.
		ORDER_FollowMe,					// Target atom is the atom who issued the order.
		ORDER_FollowTarget,				// Target atom is the atom to follow.
		ORDER_GetIn,					// Target atom is the parent atom of the atom who issued the order.
		ORDER_GetOut,					// Target atom is the parent of the atom who receives the order.
		ORDER_GoToTarget,				// Target atom is the atom to move close to.
		ORDER_GoToTargetExactly,		// Target atom is the atom to move close to.
		ORDER_Garrison,					// Target atom is the parent of the atom who receives the order.
		ORDER_CloseDoor,				// Target atom is the parent of the atom who receives the order.
		ORDER_OpenDoor,					// Target atom is the parent of the atom who receives the order.
		ORDER_GetItem,					// Target atom is the item or creature atom to get. We only get item and creature atoms.
		ORDER_BattleStation,			// Target atom is the parent of the atom who receives the order.
		ORDER_Sleep,					// Target atom is the parent of the atom who receives the order.
		ORDER_SleepDayDream,			// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepEat,					// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepExercise,			// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepGame,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepLetter,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepMovie,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepMusic,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepPine,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepPhone,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepRead,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepTalk,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepText,				// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepTV,					// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_SleepWeb,					// Target atom is the parent of the atom who receives the order. A type of sleep, for fun.
		ORDER_DamageReport,				// Target atom is the parent of the atom who receives the order.
		ORDER_SurveyScanPlanet,			// Target atom is the parent of the atom who receives the order.
		ORDER_SurveyScanSystem,			// Target atom is the parent of the atom who receives the order.
		ORDER_TacticalScan,				// Target atom is the parent of the atom who receives the order.
		ORDER_EnterSurgery,				// Target atom is the parent of the atom who receives the order.
		ORDER_TakeCommand,				// Target atom is the parent of the atom who receives the order.
		ORDER_ReleaseCommand,			// Target atom is the parent of the atom who receives the order.
		ORDER_DropRider,				// Target atom is the parent of the atom who receives the order.
		ORDER_MountMe,					// Target atom is the atom who issued the order.
		ORDER_MountTarget,				// Target atom is the atom to follow.
		ORDER_HengeCeremony,			// Target atom is the parent of the atom who receives the order, loc is center of henge.
		ORDER_GoToRoom,					// Target atom is the atom who has the room.
		//Highest value must not exceed 6 bits (63).
	};

	// Threat reaction of citizens.
	enum CITIZEN_POSTURE
	{
		POSTURE_Offensive = 0,			// Will initiate attack on threats.
		POSTURE_DefendSelf,				// Will not attack threats unless attacked.
		POSTURE_DefendArea,				// Will initiate attack on threats that get close.
		POSTURE_Berserk,				// Will initiate attack on all enemy units, including civilians and farm animals.
		POSTURE_HoldFire,				// Will not attack for any reason.
	};

	// MUST FIT INTO 2 BITS
	enum CITYTYPE
	{
		CITY_City_Obsolete,				// DaCity
		CITY_Capitol,					// DaBd IID_Capitol
		CITY_MilitaryBase,				// DaBd IID_BaseHeadquarters
	};

	// Used to form bitmask of environments.
	enum CONSTRUCTIONENV
	{
		CENV_Habitable = 0x01,
		CENV_Vacuum = 0x02,
		CENV_SeaBottom = 0x04,
	};

	// Context of control processing for ProcessControlInput. 
	enum CONTROL_CONTEXT
	{
		CONTROL_Server,					// Server is processing controls for client.
		CONTROL_Client,					// Client is processing controls for smooth response; they will also be sent to the server.
		CONTROL_Solo,					// Solo mode control processing.
	};

	enum ECOROLE
	{
		ECO_Carnivore,					// Creature only eats meat that it kills.
		ECO_Herbivore,					// Creature only eats plants.
		ECO_Omnivore,					// Creature eats plants or meat that it kills.
		ECO_Scavenger,					// Creature only eats meat killed by others.
	};

	enum EMPIRE_ROLE
	{
		EMPIRE_Player,
		EMPIRE_Pirate,
		EMPIRE_Agent,
		EMPIRE_ArenaTeam,
		EMPIRE_ArenaObserver
	};

	// Values used to identify the export policy of a commodity.
	enum EXPORT_POLICY
	{
		EXPOL_ToNobody	= 0,			// Cannot be exported to any other empires.
		EXPOL_ToEnemy	= 1,			// Can be exported to any empire.
		EXPOL_ToFriend	= 2,			// Can be exported only to friend empires.
		EXPOL_ToNeutral	= 3,			// Can be exported to friend or neutral empires.
	};

	enum FINDDEV
	{
		FIND_All						= 0x0000,				// No filter. Find all developments.
		FIND_Finished					= 0x0001,				// Find developments that are not under construction at all.
		FIND_FinishedLevel				= 0x0002,				// Find developments that have at least one finish level.
		FIND_BuildingsOnly				= 0x0004,				// Buildings only, do not search roads.
		FIND_RoadsOnly					= 0x0008,				// Roads only, do not search buildings.
		FIND_SkipLandscapers			= 0x0010,				// Skip all landscapers.
		FIND_SkipFinishedLandscapers	= 0x0020,				// Skip landscapers that are not under construction.
		FIND_Enclosed					= 0x0040,				// Enclosed developments only.
		FIND_SkipBiodomes				= 0x0080,				// Skip all biodomes.
		FIND_SkipPlatforms				= 0x0100,				// Skip all platforms.
	};

	enum FLIGHT_ATTITUDE
	{
		FLY_Upright,
		FLY_Inverted,
		FLY_LeftSide,
		FLY_RightSide,
	};

	enum FOUNDER_STATUS
	{
		FOUNDER_Unknown,
		FOUNDER_Empire,					// Citizen of my empire.
		FOUNDER_Friend,					// Citizen of friend empire, toward my empire.
		FOUNDER_Neutral,				// Citizen of neutral empire, toward my empire.
		FOUNDER_Enemy,					// Citizen of enemy empire, toward my empire.
		FOUNDER_Vassal,					// Citizen of vassal empire, toward my empire.
		FOUNDER_FreeOrDead,				// Citizen of no empire, could be dead.
		FOUNDER_NoEmpire,				// Status requested for nonexistent empire.
		FOUNDER_NoCity,					// Status requested but city is not part of requested empire.
		FOUNDER_NoFounder,				// Requested city has null founder.
		FOUNDER_BadStance,				// Invalid political stance for founder's empire to city's empire.
	};

	enum FTL_COMMIT
	{
		// This enum is used when animating the FTL drive commit process.
		// Each state other than Dormant lasts for one second.
		FTL_COMMIT_Invalid = -1,
		FTL_COMMIT_Dormant = 0,
		FTL_COMMIT_Precharge1,
		FTL_COMMIT_Precharge2,
		FTL_COMMIT_Precharge3,
		FTL_COMMIT_Precharge4,
		FTL_COMMIT_Precharge5,
		FTL_COMMIT_Precharge6,
		FTL_COMMIT_ChargedBegin = 7,	// Charged state lasts for 50 seconds.
		FTL_COMMIT_Warmed = 30,		// Used when drive is in warming state.
		FTL_COMMIT_ChargedEnd = 56,
		FTL_COMMIT_Discharge1 = 57,
		FTL_COMMIT_Discharge2,
		FTL_COMMIT_Discharge3,
		FTL_COMMIT_Discharge4,
		FTL_COMMIT_Discharge5,
		FTL_COMMIT_Discharge6,
		FTL_COMMIT_Discharged = 63,	// Final value must not exceed 63. See DpConsoleEngineer::Rx/Tx
	};

	enum FTL_DRIVE_STATE
	{
		FTL_DRIVE_Invalid = -1,
		FTL_DRIVE_Off,
		FTL_DRIVE_Warming,
		FTL_DRIVE_On,
		FTL_DRIVE_New,			// Used to identify a "nu" vessel.
	};

	enum GALAXY
	{
		AndromedaRising,
		BlackHole,
		Core,
		CrownOfOthon,
		DyrathonsRetreat,
		EdgeOfTheRift,
		FallasEmbrace,
		FallenLegionsOfMuturon,
		HeartOfVictorus,
		HouseZanathar,
		IndigoSea,
		InkarBorderRegion,
		MuturonEncounter,
		RansuulsFlamingSword,
		SevenTen,
		ShoresOfHazeron,				// Default
		ThustrasEye,
		VeilOfTargoss,					// No PvP
		VreenoxEclipse,
		VulcansForge,
		MilkyWay,
		GALAXY_Count					// LAST ITEM
	};

	enum GENDER
	{
		Female,
		Male,
		Asexual,
		Hermaphrodite,
	};

	enum GRAVITY
	{
		G_Natural,						// Gravity is based on the environment of the ship.
		G_Artificial,					// Gravity has specified up vector while engine power on or life support w/gravity.
		G_Zero,							// Gravity is zero when life support w/gravity is functioning, else environmental gravity.
		G_Count							// LAST ITEM. Used for invalid value.
	};

	enum GROUND_ORIENT
	{
		GORI_ZPosUp,					// Lie on bottom, with postive Z up.
		GORI_ZNegUp,					// Lie on top, with negative Z up.
		GORI_YPosUp,					// Lie on back, with positive Y up.
		GORI_YNegUp,					// Lie on front, with negative Y up.
		GORI_XPosUp,					// Lie on left side, with positive X up.
		GORI_XNegUp,					// Lie on right side, with negative X up.
		GORI_Count
	};

	// HABITABILITY is used by stars and planets during creation of a solar system
	// to select rules for creation of worlds in habitable orbits.
	// NOTE: These are in escalating order on purpose!
	enum HABITABILITY
	{
		HABITABLE_Unspecified,			// No rules for habitability.
		HABITABLE_HomeWorld,			// Very stringent rules for habitability.
		HABITABLE_Ringworld,			// Generate a ringworld.
		HABITABLE_System,				// Less stringent rules for habitability.
		HABITABLE_EmptySystem,			// Used when creating empty solar system place holder at center of empty sector.
	};

	enum HANDEDNESS
	{
		LeftHanded,
		RightHanded,
	};

	enum HOLD_ACCESS
	{
		HOLD_Inaccessible,
		HOLD_Armory,
		HOLD_Bar,
		HOLD_Feeder,
		HOLD_FuelCoupling,
		HOLD_VehicleBay,
		HOLD_Hold,
		HOLD_Pantry,
		HOLD_Pharmacy,
		HOLD_Wardrobe,
		HOLD_Count
	};

	enum INTERIOR_LIGHT
	{
		INTLIGHT_Dark,
		INTLIGHT_Bright,
		INTLIGHT_Dim,
		INTLIGHT_Invalid
	};

	enum LINEAGE_TYPE
	{
		LINEAGE_None,						// No parents, spawned into universe nu.
		LINEAGE_Citizen,					// Both parents were citizens, regardless if either citizen parent was demiavatar.
		LINEAGE_Demiavatar,					// Lineage is mixed avatar and citizen.
		LINEAGE_Avatar,						// Lineage is pure avatar.
	};

	enum LOG_MESSAGE_BITS
	{
		LOG_Log								= 0x00000001,			// Routine events.
		LOG_Debug							= 0x00000002,			// Debug messages.
		LOG_Warning							= 0x00000004,			// Messages resulting from hackers or cheaters.
		LOG_Error							= 0x00000008,			// Messages resulting from program errors.
		LOG_Admin							= 0x00000010,
		LOG_AnimalEncounter					= 0x00000020,
		LOG_AtomCreate						= 0x00000040,
		LOG_AtomKillAndDestruct				= 0x00000080,
		LOG_AtomModify						= 0x00000100,
		LOG_AtomQuery						= 0x00000200,
		LOG_Comm							= 0x00000400,
		LOG_ConnectDisconnect				= 0x00000800,			// NOTE: Servers only. Client connections go to connect.log.
		LOG_Construction					= 0x00001000,
		LOG_Damage							= 0x00002000,
		LOG_Database						= 0x00004000,
		LOG_Governance						= 0x00008000,
		LOG_LoadBalance						= 0x00010000,
		LOG_Manufacture						= 0x00020000,
		LOG_Path							= 0x00040000,
		LOG_Physics							= 0x00080000,
		LOG_SpacecraftDesign				= 0x00100000,
		LOG_Spawn							= 0x00200000,
		LOG_Status							= 0x00400000,
		LOG_Story							= 0x00800000,
		LOG_Thread							= 0x01000000,
		LOG_TickTock						= 0x02000000,
		LOG_Trade							= 0x04000000,
		LOG_SlowThinkers					= 0x08000000,
		LOG_Travel							= 0x10000000,			// Wormhole and deadhead travel, including server handoffs.
		LOG_Etherium						= 0x20000000,
		LOG_Chat							= 0x40000000,			// chat.serverName.log
		LOG_Shaders							= 0x80000000,			// GLSL shader debugs
	};

	enum LANDING_ORIENT
	{
		LORI_Belly,						// Land on belly, with positive Z up. For spacecraft and items.
		LORI_Tail,						// Land on tail, with positive Y up. For spacecraft and items.
		LORI_Count
	};

	enum MANUFACTURINGPROCESSID
	{
		MPID_Flag = 25,
		MPID_WaterUsingEnvironment = 170,		// at treatment plant in water
		MPID_WaterUsingIce = 171,				// at treatment plant
		MPID_Magmex = 252,						// at treatment plant in lava
		MPID_Myrathane = 253,					// at treatment plant in supercooled liquid
		MPID_Flomentum = 1078,					// at treatment plant in supercooled liquid
	};

	// Never change the value of these enum's. Only add new ones at the end.
	enum NPC_ROLE
	{
		ROLE_Citizen,
		ROLE_IndigenousPerson,
		ROLE_IndigenousElder,
		ROLE_IndigenousHealer,
		ROLE_IndigenousStargazer,
		ROLE_Worker,					// Building inhabitant.
		ROLE_Passenger,
		ROLE_Crew,
		ROLE_Troop,
		ROLE_Officer,
		ROLE_Count
	};

	// NOTE: There is an order dependency between this enum and the SOUND_EFFECT enum in augleffects.h
	enum ON_GROUND
	{
		OG_None,						// no ground contact
		OG_Forest,
		OG_HardFloor,
		OG_Metal,
		OG_Pavement,
		OG_Rock,
		OG_Sand,
		OG_Savanna,
		OG_Snow,
		OG_WaterSoundOnly,
		OG_WoodFloor,
	};

	enum ORBITZONE
	{
		OZONE_Inferno,
		OZONE_Inner,
		OZONE_Habitable,
		OZONE_Outer,
		OZONE_Frigid,
		OZONE_DeepSpace,				// Not actually an orbit zone. Beyond the farthest orbit zone.
	};

	// Access is granted to specific sets of personnel by ORing these bits together.
	enum PERSONNEL_ACCESS
	{
		PERSONNEL_Command				= 0x01,					// Access is restricted to captain and officers.
		PERSONNEL_Crew					= 0x02,					// Access is restricted to crew.
		PERSONNEL_Passengers			= 0x04,					// Access is restricted to passengers.
		PERSONNEL_Troops				= 0x08,					// Access is restricted to troops.
		PERSONNEL_Everyone				= 0x0f,					// Access is granted to everyone.
	};

	// AuMissionOrder PilotUpPolicy order has a good description of how these are treated.
	// NOTE: If any policies are added, aumission.cpp requires changes to describe the pilot up policy.
	// Pilot up policy applies when flying outside the atmosphere of a world.
	enum PILOT_UP_POLICY
	{
		UpAwayFromGravity,					// Down is toward the nearest gravity well, like inside the atmosphere.
		UpTowardGravity,					// Down is away from the nearest gravity well, so you look up to see it.
		PortTowardGravity,					// Port side is toward the nearest gravity well, so you look left to see it.
		StarboardTowardGravity,				// Starboard side is toward the nearest gravity well, so you look right to see it.
		PILOT_UP_POLICY_Count				// LAST ITEM
	};

	enum POLITICAL_STANCE
	{
		POL_Friend,
		POL_Neutral,
		POL_Enemy,
		POL_Empire,	// Result of stance comparison when they are the same empire. Cannot be set as a stance toward another empire.
		POL_Vassal,
	};

	enum REWARDTYPE
	{
		REW_None,
		REW_CaptureBase,
		REW_CaptureCapitalEmpire,
		REW_CaptureCapitalSector,
		REW_CaptureCity,
		REW_CaptureStation,
		REW_CaptureSpaceship,
		REW_CaptureStarship,
		REW_DamageStation,
		REW_DamageSpaceship,
		REW_DamageStarship,
		REW_ColonizeHabitableWorld,
		REW_ExploreWormhole,
		REW_KillUnit,
		REW_Salary,
		REW_SpotCity,
		REW_SpotEnemy,
		REW_OrderComplete,
		REW_CaptureCapitalGalaxy,
		REW_DeliverTEU,
		REW_ColonizeUninhabitableWorld,
	};

	enum ROADMAT
	{
		ROADMAT_Dirt			= 0,
		ROADMAT_Asphalt			= 1,
		ROADMAT_Concrete		= 2,
		ROADMAT_Pressurized		= 3,	// harsh environment construction
		ROADMAT_GradedArea		= 4,	// Grades terrain like a road. Not really a road.
		ROADMAT_Irrigation		= 5,	// Moderate trees and shrubs, mowed grass. Does not grade terrain.
		ROADMAT_Defoliant		= 6,	// Removes all plants.
		ROADMAT_Clearing		= 7,	// Removes trees and shrubs. Does not grade terrain.
	};

	// This was scavenged from dnroomtype.h.
	enum ROOMTYPE
	{
		ROOM_None,
		ROOM_Barracks,
		ROOM_Bridge,
		ROOM_Brig,
		ROOM_CaptainsCabin,
		ROOM_Hold,
		ROOM_Corridor,
		ROOM_CrewCabin,
		ROOM_EngineRoom,
		ROOM_Fuel,
		ROOM_Galley,
		ROOM_GravityWell,
		ROOM_Hull,			// Solid or dense equipment. NOTE: Update DnRoom::IsValid if this enum value gets used for DnSpaceShip
		ROOM_VehicleBay,
		ROOM_Lounge,
		ROOM_OpenToDeckBelow,
		ROOM_SensorRoom,
		ROOM_ShieldRoom,
		ROOM_SickBay,
		ROOM_Stateroom,
		ROOM_TransporterRoom,
		ROOM_WeaponBay,
		ROOM_RoofGable,
		ROOM_RoofHip,
		ROOM_RoofRound,				// NOTE: Update DnRoom::IsValid if this is no longer the last valid room type.
		ROOM_HullLargeDoor = 28,	// Used by hull wall skinner.
		ROOM_HullOpening = 29,		// Used by hull wall skinner.
		ROOM_HullWallDoor = 30,		// Used by hull wall skinner.
		ROOM_HullWallTurret = 31	// Used by hull wall skinner.
	};

	// Startup scenarios for new avatars.
	// DO NOT CHANGE THE VALUE OF THESE ENUMS, insert new ones before the SCENARIO_Count enum.
	enum SCENARIO
	{
		ReachForTheStars,
		CitizenOfTheGalaxy,
		SavageState,
		LostAtlantis,
		SCENARIO_Count
	};

	inline QString ScenarioName(SCENARIO scenario)
	{
		if(scenario < ReachForTheStars || scenario > SCENARIO_Count)
			scenario = SCENARIO_Count;
		QString names[SCENARIO_Count+1] =
		{
			QObject::tr("Reach for the Stars"),						// ReachForTheStars
			QObject::tr("Citizen of the Galaxy"),					// CitizenOfTheGalaxy
			QObject::tr("The Savage State"),						// SavageState
			QObject::tr("Lost Atlantis"),							// LostAtlantis
			QObject::tr("Invalid Scenario")							// SCENARIO_Count
		};
		return names[scenario];
	}

	// Returns true if a scenario is ok for solo play.
	inline bool ScenarioOkMultiplayer(SCENARIO scenario)
	{
		switch(scenario)
		{
		case ReachForTheStars:
		case CitizenOfTheGalaxy:
			return true;
		case SavageState:
		case LostAtlantis:
		case SCENARIO_Count:
			break;
		}
		return false;
	}

	// Returns true if a scenario is ok for solo play.
	inline bool ScenarioOkSolo(SCENARIO scenario)
	{
		switch(scenario)
		{
		case ReachForTheStars:
		case CitizenOfTheGalaxy:
		case SavageState:
		case LostAtlantis:
			return true;
		case SCENARIO_Count:
			break;
		}
		return false;
	}

	// AuMissionOrder ShieldPolicy order has a good description of how these are treated.
	// NOTE: If any policies are added, aumission.cpp requires changes to describe the shield policy.
	enum SHIELD_POLICY
	{
		ShieldUp,							// Up all the time
		ShieldDown,							// Down all the time
		ShieldUpIfAnyone,					// Up if any contact
		ShieldUpIfForeign,					// Up if any non-empire contact
		ShieldUpIfNeutral,					// Up if any neutral or enemy contact
		ShieldUpIfEnemy,					// Up if any enemy contact
	};

	// NOTE: If any policies are added, aumission.cpp requires changes to describe the shield policy.
	enum SHIELDFUEL_POLICY
	{
		ShieldUpUnlessNoFuel,				// Shields up unless the ship has no fuel.
		ShieldUpIgnoreFuel,					// Shields up regardless of fuel status of the ship.
	};

	enum STREAMLINE
	{
		SLINE_None,						// Not streamlined.
		SLINE_Atmospheric,				// Streamlined and hardened for atmospheric entry.
		SLINE_SeaSurface,				// Streamlined and hardened for sea surface operations.
		SLINE_Submersible,				// Streamlined and hardened for under water operations.
		SLINE_Subterranean,				// Streamlined and hardened for under ground operations.
		SLINE_Count
	};

	enum SPACECRAFT_STRUCTURE
	{
		SC_Hull = 0,					// Powerless hull
		SC_Starship,					// Spacecraft hull with maneuver and FTL drives.
		SC_Spaceship,					// Spacecraft hull with maneuver drives.
		SC_Deprecated,
		SC_Station,						// Spacecraft hull with minimal drives.
		SC_Count
	};

	// Types of surface liquid that may exist on worlds. Hydrographics percentage indicates amount of surface coverage.
	enum HYDROGRAPHIC_LIQUID
	{
		HYDRO_None,						// Any zone, when hydrographics is 0.
		HYDRO_Lava,						// Inferno zone except gas giant.
		HYDRO_Water,					// Inner, habitable and outer zones, except gas giant.
		HYDRO_SupercooledLiquid,		// Frigid zone and all gas giants.
	};

	// Used to generalize the strata of terrain.
	enum TER_ELEV_TYPE
	{
		TER_Peak=0,						// Top terrain level. Only if the world has snow-capped peaks.
		TER_Alpine,						// Terrain level below peak. Only if the world has snow-capped peaks.
		TER_Highlands,					// Terrain level below alpine. Top level if the world does not have alpine peaks.
		TER_Midlands,					// Terrain is between TER_Highlands and TER_Lowlands.
		TER_Lowlands,					// Terrain patch is terrain level above TER_Coastal.
		TER_Coastal,					// Terrain patch has at least one vertex above sea level and one below.
		TER_ContinentalShelf,			// Terrain patch is terrain level below TER_Coastal.
		TER_SeaFloor,					// Terrain is between TER_ContinentalShelf and TER_SeaTrench. Bottom in shallow sea.
		TER_SeaTrench,					// Bottom terrain level if sea is deep enough.
		TER_Types						// Number of TER_ types. NOT ACTUALLY A TERRAIN TYPE.
	};

	enum TERRAIN
	{
		HighlandForest,
		HighlandSavanna,
		HighlandDesert,
		LowlandForest,
		LowlandSavanna,
		LowlandDesert,
		SeaLevelForest,
		SeaLevelSavanna,
		SeaLevelDesert,
		Underwater,
		TerrainTypes	// MUST BE LAST ENTRY
	};

	enum TOD							// Time of day animals are active.
	{
		TOD_Nocturnal,					// Creature is normally active only at night.
		TOD_Crepuscular,				// Creature is normally active only at dawn and dusk.
		TOD_Diurnal,					// Creature is normally active only during daylight.
		TOD_Any,						// Creature may be active at any time.
	};

	// If you add a TRANSPORTER_TARGET, be sure to add to TTIcon.
	enum TRANSPORTER_TARGET
	{
		TT_Invalid,
		TT_Bd,
		TT_Beacon,
		TT_ClaimFlag,
		TT_Henge,
		TT_RingworldControlCenter,
		TT_Village,
		TT_Avatar,
		TT_Captain,
		TT_Officer,
		TT_Crew,
		TT_Troop,
		TT_Passenger,
		TT_Spaceship,
		TT_Starship,
		TT_SpaceStation,
		TT_Transporter,					// Target is a transporter system that is on.
		TT_GeologicalFeatureFirst,		// This must be the last item because geo features use this value, on up to 255.
		TT_GeologicalFeatureLast = 255
	};

	enum UNIT_OF_MEASURE
	{
		UOM_Meters,
		UOM_Centimeters,
		UOM_Millimeters,
		UOM_Yards,
		UOM_Feet,
		UOM_Inches,
	};

	enum VEHICLE_SPOT_TYPE
	{
		SPOT_Invalid,
		SPOT_GroundVehicle,
		SPOT_Rocket,
		SPOT_SpaceVehicle,
		SPOT_WaterVehicle,
		SPOT_SpecificVehicle,
		SPOT_Count
	};

	// Index into terrain tile palette. When there is a conflict between textures on a polygon, the lowest numbered WORLDTEX_ value
	//   takes precedence.
	// Names:
	//   Brightness - Light, Medium, Dark
	//   Bumpiness - Diffuse, Smooth, Moderate, Heavy, Coarse
	// NOTE: Changes here must be mirrored in DaWorldLL::CreateTerrainTexture.
	// Trailing letter indicates how the texture is known to be used: C=City ambient light, T=Terrain, P=Pavement, S=Shader
	enum WORLDTEX_TYPE
	{
		// Pavements (Pavements through land and site developments start at 0 for shaders)
		WORLDTEX_ConstructionZone,								// Construction zone	(P)
		WORLDTEX_ConcreteHeavy,									// Concrete road/Pressurized road/Magmium building pavement	// (T,P)
		WORLDTEX_Tarmac,										// Asphalt road/Vulcium building pavement (P)
		WORLDTEX_DirtSmooth,									// Dirt road/Log building pavement	(P)
		WORLDTEX_CobblestoneLight,								// Stone/Lumber building pavement (this P and below hide snow) (P)

		// End of Pavements that automatically raise ambient light level.

		// Land at site developments
		WORLDTEX_DirtCrackedSmooth,								// Farm/Orchard/Logging Camp/House/Park/Zoo land (P)
		WORLDTEX_RockTan,										// Mine/Well/Wind Power Plant/Solar Power Plant land	(P)

		// End of the Pavement textures

		// Ice and Snow
		WORLDTEX_SnowModerate,									// (S)
		WORLDTEX_IceModerate,									// (T)

		// Ground surface
		WORLDTEX_ConcreteDarkModerate,							// (T)
		WORLDTEX_DirtCoarse,									// (T)
		WORLDTEX_DirtGray,										// (T)
		WORLDTEX_DirtHeavy,										// (T)
		WORLDTEX_DirtModerate,									// (T)
		WORLDTEX_Pebbles,										// (T)
		WORLDTEX_RockBrown,										// (T)
		WORLDTEX_RockBrownModerate,								// (T)
		WORLDTEX_RockGrayCoarse,								// (T)
		WORLDTEX_RockGrayHeavy,									// (T)
		WORLDTEX_RockGrayModerate,								// (T)
		WORLDTEX_RockGraySmooth,								// (T)
		WORLDTEX_RockRedHeavy,									// (T)
		WORLDTEX_RockRedModerate,								// (T)
		WORLDTEX_RockRedSmooth,									// (T)
		WORLDTEX_SandSmooth,									// (T)

		// Sea surfaces
		WORLDTEX_Magma,											// (T)
		WORLDTEX_SeaWater,										// (T)
		WORLDTEX_SupercooledLiquid,								// (T)

		WORLDTEX_Count					// LAST ITEM (C)
	};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Units-of-Measure
	inline double CubicFeetPerCubicMeter()
	{
		return 35.314666212661322;
	}

	inline double CubicMetersPerCubicFoot()
	{
		return 0.0283168466;
	}

	inline double FeetPerMeter()
	{
		return 3.28083989501312;
	}

	inline double FeetPerMM()
	{
		return FeetPerMeter() / 1000;
	}

	inline double KilogramsPerPound()
	{
		return 0.45359237;
	}

	inline double KmPerMile()
	{
		return 1.609344;
	}

	// Returns n/d * 100
	inline int Percent(quint32 n, quint32 d)
	{
		float f = (float)n/(float)d;
		return (int)roundf(f * 100);
	}

	inline double PoundsPerKilogram()
	{
		return 2.20462262;
	}

	inline double SquareFeetPerSquareMeter()
	{
		return 10.7639;
	}

	inline double SquareMetersPerSquareFoot()
	{
		return 0.092903;
	}

	// Return the factor that is multiplied by values in a specific 'uom', to convert them to our internal unit of feet.
	inline double UnitScaleToFeet(UNIT_OF_MEASURE uom)
	{
		switch(uom)
		{
		case UOM_Meters:
			return FeetPerMeter();
		case UOM_Centimeters:
			return FeetPerMeter() / 100;
		case UOM_Millimeters:
			return FeetPerMeter() / 1000;
		case UOM_Yards:
			return 3.;
		case UOM_Feet:
			break;
		case UOM_Inches:
			return 1./12.;
		}
		return 1.;
	}

	inline QString UnitSymbol(UNIT_OF_MEASURE uom)
	{
		switch(uom)
		{
		case UOM_Meters:
			return QString("m");
		case UOM_Centimeters:
			return QString("cm");
		case UOM_Millimeters:
			return QString("mm");
		case UOM_Yards:
			return QString("yd");
		case UOM_Feet:
			return QString("\'");
		case UOM_Inches:
			return QString("\"");
		}
		return QString();
	}

	// Returns 'number' (from 1 to 100) expressed using Roman numerals.
	inline QString RomanNumber(int number)
	{
		// Our range is 1 to 100.
		if(number < 1 || number > 100)
			return QString("%L1").arg(number);
		QString romans[] =
		{
			"",
			"I",
			"II",
			"III",
			"IV",
			"V",
			"VI",
			"VII",
			"VIII",
			"IX",
			"X",
			"XI",
			"XII",
			"XIII",
			"XIV",
			"XV",
			"XVI",
			"XVII",
			"XVIII",
			"XIX",
			"XX",
			"XXI",
			"XXII",
			"XXIII",
			"XXIV",
			"XXV",
			"XXVI",
			"XXVII",
			"XXVIII",
			"XXIX",
			"XXX",
			"XXXI",
			"XXXII",
			"XXXIII",
			"XXXIV",
			"XXXV",
			"XXXVI",
			"XXXVII",
			"XXXVIII",
			"XXXIX",
			"XXXX",
			"XXXXI",
			"XXXXII",
			"XXXXIII",
			"XXXXIV",
			"XXXXV",
			"XXXXVI",
			"XXXXVII",
			"XXXXVIII",
			"XXXXIX",
			"L",
			"LI",
			"LII",
			"LIII",
			"LIV",
			"LV",
			"LVI",
			"LVII",
			"LVIII",
			"LIX",
			"LX",
			"LXI",
			"LXII",
			"LXIII",
			"LXIV",
			"LXV",
			"LXVI",
			"LXVII",
			"LXVIII",
			"LXIX",
			"LXX",
			"LXXI",
			"LXXII",
			"LXXIII",
			"LXXIV",
			"LXXV",
			"LXXVI",
			"LXXVII",
			"LXXVIII",
			"LXXIX",
			"LXXX",
			"LXXXI",
			"LXXXII",
			"LXXXIII",
			"LXXXIV",
			"LXXXV",
			"LXXXVI",
			"LXXXVII",
			"LXXXVIII",
			"LXXXIX",
			"LXXXX",
			"LXXXXI",
			"LXXXXII",
			"LXXXXIII",
			"LXXXXIV",
			"LXXXXV",
			"LXXXXVI",
			"LXXXXVII",
			"LXXXXVIII",
			"LXXXXIX",
			"C",
		};
		return romans[number];
	}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Universal Constants
	// The base density was determined by estimating that one design grid square (32' sq x 8' ht) of empty unarmored hull
	// might weigh about 200 lbs. That's close enough to call it one pound per cubic foot.
	inline double DensityOfMetalHull()
	{
		return 200. / (32. * 8.);
	}

	// The base density was determined from empirical analysis of the space fighter model and some guessing.
	// The space fighter is estimated to be 10000kg at 27.6 cubic meters.
	inline double DensityOfVehicle()
	{
		return (10000. * PoundsPerKilogram()) / (27.6 * CubicFeetPerCubicMeter());
	}

	//  Water ranges from 999.84kg/m^3 at 0C to 998.21 at 20C to 958.37 at 100C
	inline double DensityOfWater()
	{
		return 998.2071 * CubicMetersPerCubicFoot() * PoundsPerKilogram();
	}

	// Force exerted by one gravity drive unit, in lbs.
	inline int GravityDriveForce()
	{
		// Calculate force so a 2250kg gravity drive unit will have a maximum acceleration of 500m/s/s.
		//   F = ma
		// Where:
		//   m = 2250kg
		//   a = 500m/s/s
		return (int)((500 * FeetPerMeter()) * (2250 * PoundsPerKilogram()));
	}

	// Acceleration due to gravity on an Earth-sized world at sea level, in ft/sec/sec.
	inline double GravityStandard()
	{
		return 32;
	}

	inline int MillisecondsPerDay()
	{
		return 60 * 60 * 24 * 1000;
	}

	inline int SecondsPerDay()
	{
		return 60 * 60 * 24;
	}

	inline int SecondsPerYear()
	{
		return 365*86400 + 5*3600 + 48*60 + 46;	// From Webster's New Universal Unabridged Dictionary
	}

	inline int DaysPerYear()
	{
		return (float)SecondsPerYear() / (float)SecondsPerDay();
	}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Utilities

	// Test for bare text free of non-printable characters and html codes.
	inline bool IsTextValid(const QString& text, int sizeLimit)
	{
		if(text.length() > sizeLimit)
			return false;
		if(text.contains('<') || text.contains('>') || text.contains('&'))
			return false;				// Prevent embedding html codes in text.
		if(text.simplified() != text)
			return false;				// Leading/trailing/excess whitespace and control characters are not allowed.
		// Check for non-printable unicode characters. Not sure if the simplified test would catch these.
		for(QString::const_iterator si = text.constBegin(); si != text.constEnd(); si++)
			if(!(*si).isPrint())
				return false;
		return true;
	}

	// Max length of names. e.g. people, cities, empires, ships, etc.
	inline int MaxNameLength()
	{
		return 30;
	}

	inline bool IsNameValid(const QString& name)
	{
		if(name.isEmpty())
			return false;				// Names need to be at least 1 character long.
		if(name.contains('/') || name.contains('\\') || name.contains(':') || name.contains('!'))
			return false;				// Text cannot contain slash or : because it could end up as part of a path name.
		if(name.contains('|'))
			return false;				// Some old code uses | as a field delimiter when concatenating strings.
		if(name.contains('%'))
			return false;				// Percent signs are used for formatting. They give us grief if someone puts one in a string, especially if its followed by a number. e.g. Athena%1
		if(!IsTextValid(name, MaxNameLength()))
			return false;
		return true;
	}

	// Titles may be two valid names separated by a /, or one valid name.
	inline bool IsTitleValid(const QString& title)
	{
		int slashIdx = title.indexOf('/');
		if(slashIdx >= 0)
		{
			// Split the title into male and female variations.
			QString maleTitle = title.left(slashIdx);
			QString femaleTitle = title.mid(slashIdx+1);
			return IsNameValid(maleTitle) && IsNameValid(femaleTitle);
		}
		return IsNameValid(title);
	}

	// Returns the phonetic word corresponding to a letter from A to Z.
	// Upper/lower case of 'letter' determines case of phonetic.
	// Returns blank if 'letter' is not in the range A-Z or a-z.
	inline const char* Phonetic(char letter)
	{
		const char* phonetic[26*2] =
		{
			"Alpha",
			"Bravo",
			"Charlie",
			"Delta",
			"Echo",
			"Foxtrot",
			"Golf",
			"Hotel",
			"India",
			"Juliet",
			"Kilo",
			"Lima",
			"Mike",
			"November",
			"Oscar",
			"Papa",
			"Quebec",
			"Romeo",
			"Sierra",
			"Tango",
			"Uniform",
			"Victor",
			"Whiskey",
			"Xray",
			"Yankee",
			"Zulu",
			"alpha",
			"bravo",
			"charlie",
			"delta",
			"echo",
			"foxtrot",
			"golf",
			"hotel",
			"india",
			"juliet",
			"kilo",
			"lima",
			"mike",
			"november",
			"oscar",
			"papa",
			"quebec",
			"romeo",
			"sierra",
			"tango",
			"uniform",
			"victor",
			"whiskey",
			"xray",
			"yankee",
			"zulu",
		};
		if(letter >= 'A' && letter <= 'Z')
			return phonetic[letter - 'A'];
		else if(letter >= 'a' && letter <= 'z')
			return phonetic[letter - 'a' + 26];
		else
			return "";
	};

	inline QUrl UrlFromString(const QString& string)
	{
		QStringList fields = string.split(' ');
		for(QStringList::const_iterator fi = fields.cbegin(); fi != fields.cend(); fi++)
		{
			QUrl url(*fi);
			QString scheme(url.scheme());
			if(scheme == "http" || scheme == "https" || scheme == "ts3server")
				return url;
		}
		return QUrl();
	}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Game Constants
	inline double AtmosphereTopMSL()
	{
		return 1500. * FeetPerMeter();				// Altitude of atmosphere limit above sea level.
	}

	inline double AtmosphereTopGasGiantMSL()
	{
		return 3000. * FeetPerMeter();				// Altitude of atmosphere limit above sea level.
	}

	inline double OrbitAltAboveAtm()
	{
		return 2000. * FeetPerMeter();				// Altitude at which spacecraft circle a globe.
	}

	inline double ParkAltAboveAtm()
	{
		return 1500. * FeetPerMeter();				// Altitude at which stations are parked.
	}

	// Altitude at which spacecraft circle a sun.
	// Distances are measured from the surface of the photosphere.
	inline double OrbitAltAboveSun()
	{
		return 50000 * FeetPerMeter();
	}

	inline double ParkAltAboveSun()
	{
		return 3000. * FeetPerMeter();		// Altitude at which pilots park next to a sun.
	}

	// The average distance to the sun is 92,900,000 miles (149,476,000 kilometers).
	// That means in the real world MilesPerAU would be 92,900,000. That spreads things out a bit too far.
	// By reducing this constant, the star sizes and orbit radii shrink together, making for a more playable space.
	// Reducing it too far will not leave enough room for moons to orbit between planets.
	// There are four planetary orbits in 1 au so one fourth of an au is the number of miles between orbit zones.
	// Largest planet (gas giant) radius = 40000m MSL
	//  Atmosphere = 2000m above sea.
	//  Orbit alt = 2000m above atm.
	//  Gap = 2000m
	//  Largest planet with a gap is 46,000m radius. 92000m diameter.
	// Largest moon radius = 2900m
	//  Atmosphere = 2000m above sfc.
	//  Orbit alt = 2000m above atm.
	//  Gap = 1000m
	//  Each moon increases the radius of the parent world by (7900*2) = 15,800m.
	// Largest titan radius = 9,900m
	//  Atmosphere = 2000m above sfc.
	//  Orbit alt = 2000m above atm.
	//  Gap = 1000m
	//  Each titan increases the radius of the parent world by (14,900 * 2) = 29,800m
	// Orbit Spacing
	//  To allow largest gas giant with largest titan and two moons: (46,000 + (15,800*2) + 29,800) * 2 = 214,800m
	// Earth-size planet radius = 6400m MSL
	//  Atmosphere = 1000m above sea.
	//  Orbit alt = 2000m above atm.
	//  Gap = 2000m
	//  Earth-size planet with a gap is 11,400m radius. 22,800m diameter.
	//  How many moons could it have? (214,800 - 22,800) / (15,800*2) = 6

	inline double FeetPerMoonOrbit()
	{
		return 15800. * FeetPerMeter();		// The number of feet allotted to each moon orbit zone
	}

	inline double FeetPerOrbit()
	{
		return (214800. * FeetPerMeter());   // The number of feet allotted to each orbit zone. 2/20/2020 704,724.41'
	}

	inline double FeetPerAu()
	{
		return FeetPerOrbit() * 4.;			// Conversion from au to feet. Four orbits per AU. 2/20/2020 2,818,897.64'
	}

	inline double FirstMoonOrbitAboveAtm()
	{
		return (3000 + 7900) * FeetPerMeter();	// (planet orbit alt + gap) + (largest moon radius)
	}

	// Returns sea-level radius in feet. Scaling is applied of 1:1000.
	// 'radius100km' is in 100s of km.
	inline double Radius100km(quint16 radius100km)
	{
		return radius100km * 100. * FeetPerMeter();
	}

	// Earth radius in 100s of km.
	inline quint16 RadiusStandard100km()
	{
		return 64;									// In-game radius is 6,400m. Radius of Earth is 6,371km.
	}

	// Radius of a standard Earth-sized world in universe feet. Scaling is applied of 1:1000.
	inline double RadiusStandard()
	{
		return Radius100km(RadiusStandard100km());	// In game radius is 6,400m. Radius of Earth is 6,371km.
	}

	// Returns the center to center orbit radius in au that results in a 28 day orbital period.
	inline double StandardMoonOrbitRadiusAu()
	{
		// Let's say the 4th lunar orbit matches our moon, of a possible 6 lunar orbits at an Earth sized world.
		return (RadiusStandard() + AtmosphereTopMSL() + FirstMoonOrbitAboveAtm() + FeetPerMoonOrbit() * 4) / FeetPerAu();
	}

	inline double FeetPerSolarSystem()
	{
		return (16*2+1) * FeetPerOrbit() * 1.5;	// 2/20/2020 34,883,858'
	}

	inline double FeetPerParsec()
	{
		return FeetPerSolarSystem() * 10.;
	}

	inline double FeetPerSector()
	{
		return FeetPerParsec() * 10.;
	}

	inline double MaxRoadWidthMeters()
	{
		return 80;
	}

	inline double MaxRoadWidthFeet()
	{
		return MaxRoadWidthMeters() * FeetPerMeter();
	}

	inline double MinRoadWidthMeters()
	{
		return 8;
	}

	inline double MinRoadWidthFeet()
	{
		return MinRoadWidthMeters() * FeetPerMeter();
	}

	// Returns the box radius of the largest spacecraft hull that can spawn on the ground.
	static double GroundSpawnSizeLimit()
	{
		return 119;
	}

	inline double MinSpacecraftSlabWidthFeet()
	{
		return GroundSpawnSizeLimit() / 0.707106781f;
	}

	inline double MinSpacecraftSlabWidthMeters()
	{
		return MinSpacecraftSlabWidthFeet() / FeetPerMeter();
	}

	inline double MinVehicleSlabWidthMeters()
	{
		return 40;
	}

	inline double MinVehicleSlabWidthFeet()
	{
		return (MinVehicleSlabWidthMeters() * FeetPerMeter());
	}

	// Nominal TL1 range of most weapons.
	inline double AttackRange()
	{
		return FeetPerMoonOrbit()*2;
	}

	// On Earth the upper 200 meters of the ocean is called the euphotic, or "sunlight", zone. Only a small amount of sunlight
	// penetrates beyond this depth.
	inline double SeaEuphoticDepth()
	{
		return 200*FeetPerMeter();
	}

	// The dysphotic zone continues down from the euphotic zone to 1000m depth. Light level drops off quickly in the dysphotic zone.
	inline double SeaDysphoticDepth()
	{
		return 1000*FeetPerMeter();
	}

	// Returns the velocity at which objects collide with an energy shield. Below this speed and objects go through the shield.
	inline double ShieldCollisionSpeed()
	{
		return 150*FeetPerMeter();
	}

	// Spacecraft launch and recovery systems.
	inline double SpacecraftLaunchLength()
	{
		return 500*FeetPerMeter();	// Length of spacecraft launch system beam.
	}

	inline double SpacecraftRecoveryLength()
	{
		return 500*FeetPerMeter();	// Length of spacecraft recovery system runway.
	}

	inline double SpacecraftRecoveryWidth()
	{
		return 20*FeetPerMeter();		// Width of spacecraft recovery system runway.
	}

	// Speed of light in universe feet per real second so that it takes
	// 10 real minutes to travel one parsec (not including acceleration/deceleration).
	// 2/20/2020 581,397.6 ft/sec (177,210 m/sec)
	inline double SpeedOfLight()
	{
		return FeetPerParsec() / (10 * 60);
	}

	// Speed of light squared
	inline double SpeedOfLight2()
	{
		return SpeedOfLight() * SpeedOfLight();
	}

	inline double SpeedOfWarp()
	{
		return SpeedOfLight() * 5;	// Speed of each warp increment.
	}

	inline double SpeedOfFreight()
	{
		return SpeedOfLight() * 5;	// Speed of broker shipped freight at TL1.
	}

	// Volume of a standard Earth-sized world in cubic universe feet.
	inline double VolumeStandard()
	{
		return (4./3. * 3.1415 * RadiusStandard() * RadiusStandard() * RadiusStandard());
	}

	// Mass constant. Produces standard acceleration at standard radius when multiplied by the standard planetary volume.
	inline double MassK()
	{
		return GravityStandard() * RadiusStandard() * RadiusStandard() / VolumeStandard();
	}

	// These mass constants are used for calculating gravity from stars and black holes. They are a guess based on what "feels ok".
	inline double MassKBlackHole()
	{
		return MassK() * 4;
	}

	inline double MassKNeutronStar()
	{
		return MassK() * 2;
	}

	inline double MassKStar()
	{
		return MassK();
	}

	inline int UDaysPerRealDay()
	{
		return 28;		// Days that pass in the Universe during each day in the real world.
	}

	inline double RealDaysPerUYear()
	{
		return 365./UDaysPerRealDay();	// Real days that pass during one Universe year
	}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

	// Cronodollars earned by each working citizen in one city report. This is loosely based on them getting paid 1c to push Run
	// about once every 30 seconds. This is also the basis for paying players 1c each time they push Run. See DpBuilding::MakeOne.
	inline int CitizenSalary()
	{
		return 25;
	}

	// Active accounts are defined as all account types except ACCT_Expired, ACCT_LockedOut and ACCT_Count.
	inline bool AcctIsActive(ACCOUNT acct)
	{
		return acct != ACCT_Expired && acct != ACCT_LockedOut && acct >= ACCT_Active && acct < ACCT_Count;
	}

	inline bool AcctIsArchitect(ACCOUNT acct)
	{
		return acct == ACCT_Architect;
	}

	inline bool AcctIsExpired(ACCOUNT acct)
	{
		return acct == ACCT_Expired;
	}

	inline bool AcctIsProgrammer(ACCOUNT acct)
	{
		return acct == ACCT_Programmer || AcctIsArchitect(acct);
	}

	inline bool AcctIsAgent(ACCOUNT acct)
	{
		return acct == ACCT_Agent || AcctIsProgrammer(acct);
	}

	inline bool AcctIsLockedOut(ACCOUNT acct)
	{
		return acct == ACCT_LockedOut;
	}

	inline bool AcctIsMinion(ACCOUNT acct)
	{
		return acct == ACCT_Minion;
	}

	inline bool AcctIsTrial(ACCOUNT acct)
	{
		return acct == ACCT_Trial;
	}

	inline QString AcctTypeIcon(ACCOUNT acct)
	{
		if(acct < ACCT_Active || acct > ACCT_Count)
			acct = ACCT_Count;
		QString names[ACCT_Count+1] =
		{
			QObject::tr(":data/c_VacSuitHelmet.png"),	// ACCT_Active
			QObject::tr(":data/Locker.png"),			// ACCT_Inactive
			QObject::tr(":data/Server.png"),
			QObject::tr(":data/Agent.png"),
			QObject::tr(":data/Programmer.png"),
			QObject::tr(":data/Architect.png"),
			QObject::tr(":data/Minion.png"),
			QObject::tr(":data/FreeTrial.png"),			// ACCT_Trial
			QObject::tr(":data/ScLock.png"),			// ACCT_LockedOut
			""	// ACCT_Count
		};
		return names[acct];
	}

	inline QString AcctTypeName(ACCOUNT acct)
	{
		if(acct < ACCT_Active || acct > ACCT_Count)
			acct = ACCT_Count;
		QString names[ACCT_Count+1] =
		{
			QObject::tr("Active"),						// ACCT_Active
			QObject::tr("Expired"),						// ACCT_Expired
			QObject::tr("Server"),
			QObject::tr("Agent"),
			QObject::tr("Programmer"),
			QObject::tr("Architect"),
			QObject::tr("Minion"),
			QObject::tr("Free Trial"),					// ACCT_Trial
			QObject::tr("Locked Out"),					// ACCT_LockedOut
			QObject::tr("Invalid Account Type"),
		};
		return names[acct];
	}

	inline QString AtmoTypeName(ATMTYPE atm)
	{
		if(atm < ATMTYPE_Standard || atm > ATMTYPE_Count)
			atm = ATMTYPE_Count;
		QString names[ATMTYPE_Count+1] =
		{
			QObject::tr("Standard"),
			QObject::tr("Tainted"),
			QObject::tr("Exotic"),
			QObject::tr("Corrosive"),
			QObject::tr("Insidious"),
			QObject::tr("Invalid Atmosphere Type"),
		};
		return names[atm];
	}

	inline QString BodyTypeIcon(BODY_TYPE wt)
	{
		if(wt < BODY_GasGiant || wt >= BODY_TYPE_Count)
			return "";
		QString names[BODY_TYPE_Count] =
		{
			":data/WorldGasGiant.png",
			":data/WorldTitan.png",
			":data/WorldMoon.png",
			":data/WorldPlanet.png",
			":data/WorldRingworld.png",
			":data/WorldRing.png",
			":data/WorldAsteroid.png",
			":data/c_TypeOPreons.png",
			":data/c_TypeBPreons.png",
			":data/c_TypeAPreons.png",
			":data/c_TypeFPreons.png",
			":data/c_TypeGPreons.png",
			":data/c_TypeKPreons.png",
			":data/c_TypeMPreons.png",
			":data/StarBlackHole.png",
			":data/StarBlackHole.png",
			":data/StarNeutron.png",
			":data/EmptySpace.png",
			":data/EmptySpace.png",
		};
		return names[wt];
	}

	inline QString BodyTypeName(BODY_TYPE wt)
	{
		if(wt < BODY_GasGiant || wt >= BODY_TYPE_Count)
			return "";
		QString names[BODY_TYPE_Count] =
		{
			QObject::tr("Gas Giant"),
			QObject::tr("Titan"),
			QObject::tr("Moon"),
			QObject::tr("Planet"),
			QObject::tr("Ringworld"),
			QObject::tr("Ring"),
			QObject::tr("Planetoid"),
			QObject::tr("Type O Star"),
			QObject::tr("Type B Star"),
			QObject::tr("Type A Star"),
			QObject::tr("Type F Star"),
			QObject::tr("Type G Star"),
			QObject::tr("Type K Star"),
			QObject::tr("Type M Star"),
			QObject::tr("Supermassive Black Hole"),
			QObject::tr("Stellar Black Hole"),
			QObject::tr("Neutron Star"),
			QObject::tr("Deep Space"),							// BODY_SolarSystemSpace
			QObject::tr("Lost in Space"),						// BODY_SectorSpace
		};
		return names[wt];
	}

	inline QString BuildingEnvironmentName(BUILDING_ENVIRONMENT envBit)
	{
		switch(envBit)
		{
		case BENV_HabitableLand:
			return QObject::tr("Land and Atmosphere with Breathable Air");
		case BENV_UnbreathableLand:
			return QObject::tr("Land and Atmosphere with Unbreathable Air");
		case BENV_FrigidLand:
			return QObject::tr("Land and Atmosphere in Frigid Zone, Gas Giant Sea and Atmosphere");
		case BENV_InfernoLand:
			return QObject::tr("Land and Atmosphere in Inferno Zone");
		case BENV_SeaBottom:
			return QObject::tr("Sea Bottom");
		case BENV_FrigidSeaBottom:
			return QObject::tr("Sea Bottom in Frigid Zone");
		case BENV_GasGiantCore:
			return QObject::tr("Gas Giant Core");
		case BENV_All:
			return QObject::tr("All Environments");
		}
		return QObject::tr("Invalid");
	}

	inline QString BuildingStructureName(BUILDING_STRUCTURE structure)
	{
		if(structure < BD_Invalid || structure >= BD_Count)
			structure = BD_Invalid;
		QString structureNames[BD_Count] =
		{
			QObject::tr("Invalid"),
			QObject::tr("Biodome"),
			QObject::tr("Bridge"),
			QObject::tr("Building"),
			QObject::tr("Fence or Wall"),
			QObject::tr("Sea Platform"),
			QObject::tr("Aerial Platform"),
			QObject::tr("Ground Platform"),
			QObject::tr("Road Enclosure"),
			QObject::tr("Sea Surface Building"),
			QObject::tr("Space Elevator"),
			QObject::tr("Statue"),
			QObject::tr("Tunnel"),
			QObject::tr("Street Light"),
		};
		return structureNames[structure];
	}

	inline QString NpcRoleName(NPC_ROLE role)
	{
		if(role < ROLE_Citizen || role > ROLE_Count)
			return QString("Invalid NPC_ROLE %1").arg(role);

		QString names[ROLE_Count+1] =
		{
			QObject::tr("Citizen"),
			QObject::tr("Indigenous Person"),
			QObject::tr("Indigenous Elder"),
			QObject::tr("Indigenous Healer"),
			QObject::tr("Indigenous Stargazer"),
			QObject::tr("City Worker"),
			QObject::tr("Passenger"),
			QObject::tr("Crew"),
			QObject::tr("Troop"),
			QObject::tr("Officer"),
			QObject::tr("ROLE_Count"),
		};
		return names[role];
	}

	// Scavenged from dnspaceship.h. Used to be the standard deck height of old style DnSpaceShip models.
	inline double DeckHeight()
	{
		return 8.0;
	}

	inline const char* GalaxyCatalog(GALAXY galaxy)
	{
		const char* galaxyCat[GALAXY_Count+1] =
		{
			"ARi",						// Andromeda Rising
			"BHo",						// Black Hole
			"Cor",						// Core
			"CoO",						// Crown of Othon
			"DRe",						// Dyrathon's Retreat
			"EoR",						// Edge of the Rift
			"FEm",						// Falla's Embrace
			"FLM",						// Fallen Legions of Muturon
			"HoV",						// House of Victorus
			"HZa",						// House Zanathar
			"ISe",						// Indigo Sea
			"IBR",						// In'kar Border Region
			"MEn",						// Muturon Encounter
			"RFS",						// Ransuul's Flaming Sword
			"STe",						// Seven Ten
			"SoH",						// Shores of Hazeron
			"TEy",						// Thustra's Eye
			"VoT",						// Veil of Targoss
			"VEc",						// Vreenox Eclipse
			"VFo",						// Vulcan's Forge
			"MWy",						// Milky Way
			"XXX"						// GALAXY_Count
		};
		return galaxyCat[galaxy];
	}

	inline const char* GalaxyName(GALAXY galaxy)
	{
		const char* galaxyName[GALAXY_Count+1] =
		{
			"Andromeda Rising",
			"Black Hole",
			"Core",
			"Crown of Othon",
			"Dyrathon's Retreat",
			"Edge of the Rift",
			"Falla's Embrace",
			"Fallen Legions of Muturon",
			"Heart of Victorus",
			"House Zanathar",
			"Indigo Sea",
			"In'kar Border Region",
			"Muturon Encounter",
			"Ransuul's Flaming Sword",
			"Seven Ten",
			"Shores of Hazeron",
			"Thustra's Eye",
			"Veil of Targoss",
			"Vreenox Eclipse",
			"Vulcan's Forge",
			"Milky Way",
			"X",							// GALAXY_Count
		};
		return galaxyName[galaxy];
	}

	inline GALAXY GalaxyEnum(const QString& galaxyName)
	{
		for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
			if(QString(GalaxyName((GALAXY)galaxy)) == galaxyName)
				return (GALAXY)galaxy;
		return GALAXY_Count;
	}

	inline QString GenderName(GENDER g)
	{
		if(g < Female || g > Hermaphrodite)
			return "";
		QString gtypes[Hermaphrodite+1] =
		{
			QObject::tr("Female"),
			QObject::tr("Male"),
			QObject::tr("Asexual"),
			QObject::tr("Hermaphrodite"),
		};
		return gtypes[g];
	}

	inline QString GravityName(GRAVITY g)
	{
		if(g < G_Natural || g >= G_Count)
			return "";
		QString gtypes[G_Count] =
		{
			QObject::tr("Natural"),
			QObject::tr("Artificial"),
			QObject::tr("Zero"),
		};
		return gtypes[g];
	}

	inline static QString GroundOrientationName(GROUND_ORIENT orient)
	{
		if(orient < 0 || orient > GORI_Count)
			orient = GORI_Count;
		QString s[GORI_Count+1] =
		{
			QObject::tr("Lie on Bottom, with +Z Up"),			// GORI_ZPosUp
			QObject::tr("Lie on Top, with -Z Up"),				// GORI_ZNegUp
			QObject::tr("Lie on Back, with +Y Up"),				// GORI_YPosUp
			QObject::tr("Lie on Front, with -Y Up"),			// GORI_YNegUp
			QObject::tr("Lie on Left Side, with +X Up"),		// GORI_XPosUp
			QObject::tr("Lie on Right Side, with -X Up"),		// GORI_XNegUp
			QObject::tr("Invalid Ground Orientation")			// GORI_Count
		};
		return s[orient];
	}

	inline QString HabitabilityName(HABITABILITY hab)
	{
		if(hab < HABITABLE_Unspecified || hab > HABITABLE_EmptySystem)
			return "";
		QString habn[HABITABLE_EmptySystem+1] =
		{
			QObject::tr("Unspecified"),
			QObject::tr("Home World"),
			QObject::tr("Ringworld"),
			QObject::tr("System"),
			QObject::tr("Empty System"),
		};
		return habn[hab];
	}

	// Returns the number of commodity units that may be harvested from each energy unit that hits the target.
	inline int HarvestUnitsPerEnergyUnit()
	{
		return 10;
	}

	inline QString HoldAccessName(HOLD_ACCESS holdAccess)
	{
		if(holdAccess < HOLD_Inaccessible || holdAccess >= HOLD_Count)
			return "";
		QString hold[HOLD_Count] =
		{
			QObject::tr("Inaccessible"),
			QObject::tr("Armory"),
			QObject::tr("Bar"),
			QObject::tr("Feeder"),
			QObject::tr("Fuel Coupling"),
			QObject::tr("Garage"),
			QObject::tr("Hold"),
			QObject::tr("Pantry"),
			QObject::tr("Pharmacy"),
			QObject::tr("Wardrobe"),
		};
		return hold[holdAccess];
	}

	// AuId::InsurerId() provides an atom id.
	inline QString InsurerName()
	{
		return QString("Planetary Indemnification Guild");	// Do not tr() this string.
	}

	// Kinetic damage is applied to objects as hit points measured in foot pounds per second.
	// Wikipedia says one mechanical horse power is 550 ft lbs per second.
	inline int KineticDamageFtLbsPerSec()
	{
		return 11;	// Ft Lbs per Second in each hit point of damage.
	}

	inline static QString LandingOrientationName(LANDING_ORIENT orient)
	{
		if(orient < 0 || orient > LORI_Count)
			orient = LORI_Count;
		QString s[LORI_Count+1] =
		{
			QObject::tr("Land on Belly, with +Z Up"),			// LORI_Belly
			QObject::tr("Land on Tail, with +Y Up"),			// LORI_Tail
			QObject::tr("Invalid Landing Orientation")			// LORI_Count
		};
		return s[orient];
	}

	inline const char* LogMessageTypeName(int bitIndex)
	{
		const char* logMsgNames[] =
		{
			"Log",
			"Debug",
			"Warning",
			"Error",
			"Admin",
			"Animal Encounter",
			"Atom Create",
			"Atom Kill and Destruct",
			"Atom Modify",
			"Atom Query",
			"Comm",
			"Connect/Disconnect",
			"Construction",
			"Damage",
			"Database",
			"Governance",
			"Load Balance",
			"Manufacture",
			"Path",
			"Physics",
			"Spacecraft Design",
			"Spawn",
			"Status",
			"Story",
			"Thread",
			"Tick/Tock",
			"Trade",
			"Slow Thinkers",
			"Travel",
			"Etherium",
			"Chat",
			"Shaders",
		};
		const int logMsgCount = sizeof(logMsgNames) / sizeof(const char*);
		if(bitIndex >= 0 && bitIndex < logMsgCount)
			return logMsgNames[bitIndex];
		return 0;
	}

	// Maximum range in sectors at which jobs are available to players.
	inline int MaxJobSectorRange()
	{
		return 10;
	}

	// Icons that correspond to each CITIZEN_ORDER.
	inline const char* OrderIcon(CITIZEN_ORDER order)
	{
		const char* orderIcon[] =
		{
			":data/OrderStay.png",				// ORDER_Stay
			":data/OrderAttackTarget.png",		// ORDER_AttackTarget
			":data/OrderComeHere.png",			// ORDER_ComeHere
			":data/OrderComeHereExactly.png",	// ORDER_ComeHereExactly
			":data/OrderFollowMe.png",			// ORDER_FollowMe
			":data/OrderFollowTarget.png",		// ORDER_FollowTarget
			":data/OrderGetIn.png",				// ORDER_GetIn
			":data/OrderGetOut.png",			// ORDER_GetOut
			":data/OrderGoToTarget.png",		// ORDER_GoToTarget
			":data/OrderGoToTargetExactly.png",	// ORDER_GoToTargetExactly
			":data/OrderGarrison.png",			// ORDER_Garrison
			":data/OrderCloseDoor.png",			// ORDER_CloseDoor
			":data/OrderOpenDoor.png",			// ORDER_OpenDoor
			":data/OrderGetItem.png",			// ORDER_GetItem
			":data/OrderBattleStation.png",		// ORDER_BattleStation
			":data/OrderSleep.png",				// ORDER_Sleep
			":data/OrderDaydream.png",			// ORDER_SleepDayDream
			":data/OrderEat.png",				// ORDER_SleepEat
			":data/OrderExercise.png",			// ORDER_SleepExercise
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderLetter.png",			// ORDER_SleepLetter
			":data/OrderMovie.png",				// ORDER_SleepMovie
			":data/OrderMusic.png",				// ORDER_SleepMusic
			":data/OrderPine.png",				// ORDER_SleepPine
			":data/OrderPhone.png",				// ORDER_SleepPhone
			":data/OrderRead.png",				// ORDER_SleepRead
			":data/OrderTalk.png",				// ORDER_SleepTalk
			":data/OrderText.png",				// ORDER_SleepText
			":data/OrderTV.png",				// ORDER_SleepTV
			":data/OrderWeb.png",				// ORDER_SleepWeb
			":data/OrderDamageReport.png",		// ORDER_DamageReport
			":data/OrderSurveyScanPlanet.png",	// ORDER_SurveyScanPlanet
			":data/OrderSurveyScanSystem.png",	// ORDER_SurveyScanSystem
			":data/OrderTacticalScan.png",		// ORDER_TacticalScan
			":data/OrderEnterSurgery.png",		// ORDER_EnterSurgery
			":data/OrderTakeCommand.png",		// ORDER_TakeCommand
			":data/OrderReleaseCommand.png",	// ORDER_ReleaseCommand
			":data/OrderDropRider.png",			// ORDER_DropRider
			":data/OrderMountMe.png",			// ORDER_MountMe
			":data/OrderMountTarget.png",		// ORDER_MountTarget
			":data/Ringworld.png",				// ORDER_HengeCeremony
			":data/OrderGoToRoom.png",			// ORDER_GoToRoom

			// Insert nu entries above this line.
			// Here are a few extra harmless entries in case I add order types on the server without building a nu client.
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
			":data/OrderGame.png",				// ORDER_SleepGame
		};
		return orderIcon[order];
	}

	// Icons that correspond to each CITIZEN_POSTURE.
	inline const char* PostureIcon(CITIZEN_POSTURE posture)
	{
		const char* postureIcon[] =
		{
			":data/PostureOffensive.png",
			":data/PostureDefendSelf.png",
			":data/PostureDefendArea.png",
			":data/PostureBerserk.png",
			":data/PostureHoldFire.png",
		};
		return postureIcon[posture];
	}

	inline QString SpacecraftStructureIcon(SPACECRAFT_STRUCTURE structure)
	{
		if(structure < SC_Hull || structure > SC_Count)
			structure = SC_Count;
		QString structureNames[SC_Count+1] =
		{
			QObject::tr(":data/ScHull.png"),
			QObject::tr(":data/Starship.png"),
			QObject::tr(":data/Spacecraft.png"),
			QObject::tr(""),			// DEPRECATED
			QObject::tr(":data/SpaceStation.png"),
			QObject::tr("")
		};
		return structureNames[structure];
	}

	inline QString SpacecraftStructureName(SPACECRAFT_STRUCTURE structure)
	{
		if(structure < SC_Hull || structure > SC_Count)
			structure = SC_Count;
		QString structureNames[SC_Count+1] =
		{
			QObject::tr("Hull"),
			QObject::tr("Starship"),
			QObject::tr("Spaceship"),
			QObject::tr(""),			// DEPRECATED
			QObject::tr("Station"),
			QObject::tr("Invalid Structure")
		};
		return structureNames[structure];
	}

	inline QString StreamlineName(STREAMLINE streamLine)
	{
		if(streamLine < SLINE_None || streamLine > SLINE_Count)
			streamLine = SLINE_Count;
		QString s[SLINE_Count+1] =
		{
			QObject::tr("Unstreamlined"),						// SLINE_None
			QObject::tr("Atmospheric"),							// SLINE_Atmospheric
			QObject::tr("Sea Surface"),							// SLINE_SeaSurface
			QObject::tr("Submersible"),							// SLINE_Submersible
			QObject::tr("Subterranean"),						// SLINE_Subterranean
			QObject::tr("Invalid Streamlining")					// SLINE_Count
		};
		return s[streamLine];
	}

	// Icons that correspond to each TRANSPORTER_TARGET.
	inline const char* TTIcon(TRANSPORTER_TARGET target)
	{
		if(target >= TT_GeologicalFeatureFirst)
		{
			if(target <= TT_GeologicalFeatureLast)
				return ":data/c_Camera.png";
			else
				return "";
		}
		const char* targetIcon[] =
		{
			"",													// TT_Invalid,
			":data/City.png",									// TT_Bd,
			":data/c_Beacon.png",								// TT_Beacon,
			":data/SurrenderFlag.png",							// TT_ClaimFlag,
			":data/MsgTransport.png",							// TT_Henge,
			":data/WorldRingworld.png",							// TT_RingworldControlCenter,
			":data/c_Tepee.png",								// TT_Village,
			":data/c_VacSuitHelmet.png",						// TT_Avatar,
			":data/Captain.png",								// TT_Captain,
			":data/Officer.png",								// TT_Officer,
			":data/Crewman.png",								// TT_Crew,
			":data/Troop.png",									// TT_Troop,
			":data/Passenger.png",								// TT_Passenger,
			":data/Spacecraft.png",								// TT_Spaceship,
			":data/Starship.png",								// TT_Starship,
			":data/SpaceStation.png",							// TT_SpaceStation,
			":data/MsgTransport.png",							// TT_Transporter,
		};
		return targetIcon[target];
	}

	// This is the quality at which the boost is 1x.
	inline quint8 QualityBaseline()
	{
		return 87;						// This is the threshold for the lowest Poor quality.
	}

	// Returns the multiplication factor to modify something based on quality.
	inline float QualityBoostF(quint8 quality)
	{
		// Table generated by auempirereport -qgen
		float qBoostF[256] =
		{
			0.f , // Quality zero is invalid.
			0.9f,
			0.915205f,
			0.921441f,
			0.926182f,
			0.930143f,
			0.9336f,
			0.936697f,
			0.939518f,
			0.942118f,
			0.944537f,
			0.946801f,
			0.948934f,
			0.950951f,
			0.952865f,
			0.954688f,
			0.956428f,
			0.958093f,
			0.959689f,
			0.961221f,
			0.962694f,
			0.964112f,
			0.965479f,
			0.966797f,
			0.96807f,
			0.969301f,
			0.97049f,
			0.971641f,
			0.972756f,
			0.973835f,
			0.97488f,
			0.975894f,
			0.976876f,
			0.977829f,
			0.978753f,
			0.979649f,
			0.980519f,
			0.981362f,
			0.982181f,
			0.982975f,
			0.983745f,
			0.984493f,
			0.985218f,
			0.985921f,
			0.986603f,
			0.987264f,
			0.987904f,
			0.988525f,
			0.989126f,
			0.989708f,
			0.990272f,
			0.990817f,
			0.991344f,
			0.991853f,
			0.992345f,
			0.99282f,
			0.993277f,
			0.993718f,
			0.994143f,
			0.994551f,
			0.994944f,
			0.99532f,
			0.995681f,
			0.996027f,
			0.996357f,
			0.996673f,
			0.996973f,
			0.997258f,
			0.997529f,
			0.997785f,
			0.998027f,
			0.998254f,
			0.998467f,
			0.998666f,
			0.998851f,
			0.999022f,
			0.999179f,
			0.999322f,
			0.999451f,
			0.999566f,
			0.999668f,
			0.999756f,
			0.999831f,
			0.999892f,
			0.999939f,
			0.999973f,
			0.999993f,
			1.f,
			1.00003f,
			1.00011f,
			1.00024f,
			1.00043f,
			1.00066f,
			1.00096f,
			1.0013f,
			1.0017f,
			1.00215f,
			1.00266f,
			1.00322f,
			1.00383f,
			1.0045f,
			1.00522f,
			1.00599f,
			1.00682f,
			1.0077f,
			1.00863f,
			1.00962f,
			1.01067f,
			1.01176f,
			1.01292f,
			1.01412f,
			1.01539f,
			1.0167f,
			1.01807f,
			1.0195f,
			1.02098f,
			1.02252f,
			1.02411f,
			1.02576f,
			1.02746f,
			1.02922f,
			1.03104f,
			1.03291f,
			1.03484f,
			1.03683f,
			1.03888f,
			1.04098f,
			1.04314f,
			1.04536f,
			1.04763f,
			1.04997f,
			1.05236f,
			1.05481f,
			1.05732f,
			1.0599f,
			1.06253f,
			1.06522f,
			1.06797f,
			1.07079f,
			1.07366f,
			1.0766f,
			1.0796f,
			1.08266f,
			1.08579f,
			1.08897f,
			1.09223f,
			1.09554f,
			1.09893f,
			1.10237f,
			1.10588f,
			1.10946f,
			1.11311f,
			1.11682f,
			1.1206f,
			1.12445f,
			1.12837f,
			1.13235f,
			1.13641f,
			1.14054f,
			1.14474f,
			1.14901f,
			1.15335f,
			1.15777f,
			1.16226f,
			1.16683f,
			1.17147f,
			1.17619f,
			1.18099f,
			1.18586f,
			1.19081f,
			1.19585f,
			1.20096f,
			1.20616f,
			1.21144f,
			1.2168f,
			1.22225f,
			1.22778f,
			1.2334f,
			1.23911f,
			1.24491f,
			1.2508f,
			1.25678f,
			1.26285f,
			1.26902f,
			1.27529f,
			1.28165f,
			1.28811f,
			1.29468f,
			1.30134f,
			1.30811f,
			1.31499f,
			1.32197f,
			1.32906f,
			1.33627f,
			1.34359f,
			1.35102f,
			1.35857f,
			1.36625f,
			1.37404f,
			1.38197f,
			1.39002f,
			1.3982f,
			1.40651f,
			1.41497f,
			1.42356f,
			1.4323f,
			1.44118f,
			1.45022f,
			1.45941f,
			1.46876f,
			1.47827f,
			1.48796f,
			1.49781f,
			1.50784f,
			1.51806f,
			1.52847f,
			1.53907f,
			1.54987f,
			1.56089f,
			1.57212f,
			1.58357f,
			1.59526f,
			1.60719f,
			1.61937f,
			1.63181f,
			1.64453f,
			1.65754f,
			1.67084f,
			1.68446f,
			1.69842f,
			1.71271f,
			1.72738f,
			1.74244f,
			1.75791f,
			1.77382f,
			1.79019f,
			1.80707f,
			1.82449f,
			1.84249f,
			1.86112f,
			1.88044f,
			1.90052f,
			1.92143f,
			1.94327f,
			1.96615f,
			1.99021f,
			2.01563f,
			2.04263f,
			2.07152f,
			2.1027f,
			2.13677f,
			2.17463f,
			2.21779f,
			2.26923f,
			2.33658f,
			2.5f,
		};
		return qBoostF[quality];
	}

	// Returns the value, modified based on quality.
	inline float QualityBoostedF(float val, quint8 quality)
	{
		return val * QualityBoostF(quality);
	}

	// Returns the value, modified based on quality.
	inline int QualityBoosted(int val, quint8 quality)
	{
		return (int)roundf(QualityBoostedF(val, quality));
	}

	inline float TurretRange(quint8 turretQ)
	{
		const float tlBoost = QualityBoostF(turretQ);
		return 1000.f * FeetPerMeter() * tlBoost;
	}

	// Range calculated so first transporter on an equatorial station can reach the poles of an Earth-sized planet.
	// The result needs to reach at least 7200mi, roughly the distance to the pole from a station at 2000mi MSL above a world that
	// is 4000mi in radius (Earth-sized).
	inline double NominalTransporterRange()
	{
		return 10000 * FeetPerMeter();
	}

	inline QString OrbitZoneName(ORBITZONE ozone)
	{
		if(ozone < OZONE_Inferno || ozone > OZONE_DeepSpace)
			return "";
		QString names[OZONE_DeepSpace+1] =
		{
			QObject::tr("Inferno"),
			QObject::tr("Inner"),
			QObject::tr("Habitable"),
			QObject::tr("Outer"),
			QObject::tr("Frigid"),
			QObject::tr("Deep Space"),
		};
		return names[ozone];
	}

	// Formats 'ival' into a little endian base 26 (A-Z) number. 'A' = 0.
	inline QString StringLE26(quint64 ival)
	{
		if(!ival)
			return QString("A");	// Special 0 case.
		QString encoded;
		quint64 ich;
		while(ival)
		{
			if(ival > 25)
			{
				ich = ival%26;
				ival /= 26;
			}
			else
			{
				ich = ival;
				ival = 0;
			}
			encoded += QChar((char)('A' + ich));
		}
		return encoded;
	}

	// Formats 'ival' into a big endian base 26 (A-Z) number. 'A' = 0.
	inline QString StringBE26(quint64 ival)
	{
		QString le26(StringLE26(ival));
		QString be26;
		for(QString::const_reverse_iterator li = le26.crbegin(); li != le26.crend(); li++)
			be26 += *li;
		return be26;
	}

	inline quint64 StringMemoryUsage(const QString& s)
	{
		return sizeof(QChar) * s.capacity();
	}

	inline QString TimeActiveName(TOD tod)
	{
		Q_ASSERT(tod >= 0 && tod <= TOD_Any);
		QString tods[4] =
		{
			QObject::tr("Nocturnal"),
			QObject::tr("Crepuscular"),
			QObject::tr("Diurnal"),
			QObject::tr("Always"),
		};
		return tods[tod];
	}

	// Decodes 'logByte' from a byte range of (0 to d_LogByteRange) to the arbitrary range (0 to 'maxPossibleValue') using a
	// logarithmic scale.
	// This is the converse of ToLogByte.
	// 'maxPossibleValue' is the maximum possible value of the returned value.
	inline quint16 FromLogByte(quint8 logByte, quint16 maxPossibleValue)
	{
		if(logByte == 0 || maxPossibleValue == 0)
			return 0;
		// 255 could be used instead of m_LogByteRange, to scale the value to fit exactly inside a byte.
		double scale = (double)d_LogByteRange / log(maxPossibleValue);
		double exponent = logByte / scale;
		quint16 val = (quint16)round(exp(exponent));
		if(val > maxPossibleValue)
			val = maxPossibleValue;	// In case of rounding.
		return val;
	}

	inline quint16 FromLogByteD(double logByte, quint16 maxPossibleValue)
	{
		if(logByte == 0 || maxPossibleValue == 0)
			return 0;
		// 255 could be used instead of m_LogByteRange, to scale the value to fit exactly inside a byte.
		double scale = (double)d_LogByteRange / log(maxPossibleValue);
		double exponent = logByte / scale;
		quint16 val = (quint16)round(exp(exponent));
		if(val > maxPossibleValue)
			val = maxPossibleValue;	// In case of rounding.
		return val;
	}

	inline quint16 ReadLogByte(QDataStream& in, quint16 maxPossibleValue)
	{
		quint8 logByte;
		in >> logByte;
		return FromLogByte(logByte, maxPossibleValue);
	}

	// Encodes 'value' from an arbitrary range of (0 to 'maxPossibleValue') to a byte range of (0 to d_LogByteRange) using a
	// logarithmic scale.
	// This is the converse of FromLogByte.
	// 'maxPossibleValue' is the maximum possible value of 'value'.
	inline quint8 ToLogByte(quint16 value, quint16 maxPossibleValue)
	{
		if(value == 0 || maxPossibleValue == 0)
			return 0;
		Q_ASSERT(value <= maxPossibleValue);
		// 255 could be used instead of m_LogByteRange, to scale the value to fit exactly inside a byte.
		double scale = (double)d_LogByteRange / log(maxPossibleValue);
		quint8 lb = (quint8)round(log(value) * scale);
#if !defined(QT_NO_DEBUG)
		{
			// Insure decoding then recoding the value gives the same result.
			quint16 l = FromLogByte(lb, maxPossibleValue);
			Q_ASSERT((quint8)round(log(l) * scale) == lb);
		}
#endif
		return lb;
	}

	// Decodes 'angleByte' from a range of -127 to 127 to a range of -180 to 180 degrees.
	inline qint16 FromAngleByte(qint8 angleByte)
	{
		double scale = 127. / 180.;			// Scale needed to fit 180 degrees into 127.
		qint16 val = (qint16)round(angleByte / scale);
		if(val < -180)
			val = -180;
		else if(val > 180)
			val = 180;
		return val;
	}

	inline qint16 ReadAngleByte(QDataStream& in)
	{
		qint8 angleByte;
		in >> angleByte;
		return FromAngleByte(angleByte);
	}

	// Encodes an angle value in the range -180 to 180 degrees into a single byte range of -127 to 127.
	inline qint8 ToAngleByte(qint16 angle)
	{
		Q_ASSERT(angle >= -180 && angle <= 180);
		double scale = 127. / 180.;		// Scale needed to fit 180 degrees into 127.
		double ang127 = angle * scale;	// Scale angle to fit into the range -127 to +127.
		qint8 ab = (qint8)round(ang127);
#if !defined(QT_NO_DEBUG)
		{
			// Insure decoding then recoding the value gives the same result.
			qint16 a = FromAngleByte(ab);
			double a127 = a * scale;	// Scale angle to fit into the range -127 to +127.
			Q_ASSERT(ab == (qint8)round(a127));
		}
#endif
		return ab;
	}

	inline QColor FromColor32(quint32 color32)
	{
		return QColor(QRgb(color32));
	}

	inline QColor ReadColor32(QDataStream& in)
	{
		quint32 color32;
		in >> color32;
		return FromColor32(color32);
	}

	inline QString RoomTypeNameInSpacecraft(ROOMTYPE type)
	{
		if(type < ROOM_None || type > ROOM_RoofRound)
			return QString("");
		QString typeNames[ROOM_RoofRound+1] =
		{
			QString(""),						// ROOM_None
			QObject::tr("Barracks"),			// ROOM_Barracks
			QObject::tr("Bridge"),				// ROOM_Bridge
			QObject::tr("Brig"),				// ROOM_Brig
			QObject::tr("Officer Cabin"),		// ROOM_CaptainsCabin
			QObject::tr("Hold"),				// ROOM_Hold
			QObject::tr("Hall"),				// ROOM_Corridor
			QObject::tr("Crew Cabin"),			// ROOM_CrewCabin
			QObject::tr("Engine Room"),			// ROOM_EngineRoom
			QObject::tr("Fuel Cell"),			// ROOM_Fuel
			QObject::tr("Galley"),				// ROOM_Galley
			QObject::tr("Gravity Well"),		// ROOM_GravityWell
			QObject::tr("Embedded in Hull"),	// ROOM_Hull
			QObject::tr("Vehicle Bay"),			// ROOM_VehicleBay
			QObject::tr("Lounge"),				// ROOM_Lounge
			QObject::tr("Open to Deck Below"),	// ROOM_OpenToDeckBelow
			QObject::tr("Sensor Room"),			// ROOM_SensorRoom
			QObject::tr("Shield Room"),			// ROOM_ShieldRoom
			QObject::tr("Sick Bay"),			// ROOM_SickBay
			QObject::tr("Stateroom"),			// ROOM_Stateroom
			QObject::tr("Transporter Room"),	// ROOM_TransporterRoom
			QObject::tr("Weapon Bay"),			// ROOM_WeaponBay
			QObject::tr("Roof, Gable"),			// ROOM_RoofGable
			QObject::tr("Roof, Hip"),			// ROOM_RoofHip
			QObject::tr("Roof, Round"),			// ROOM_RoofRound
		};
		return typeNames[type];
	}

	inline quint32 ToColor32(const QColor& color)
	{
		quint32 c32 = color.rgba();
#if !defined(QT_NO_DEBUG)
		{
			// Insure decoding then recoding the value gives the same result.
			QColor c = FromColor32(c32);
			Q_ASSERT(c.rgba() == c32);
		}
#endif
		return c32;
	}

	// Returns the trade range in meters (hull to hull) for spacecraft, mainly space stations.
	inline double TradeRangeM(quint8 cncQ)
	{
		Q_ASSERT(cncQ);
		return 50 + 4 * cncQ;
	}

	// Returns the trade range in feet.
	inline double TradeRange(quint8 cncQ)
	{
		Q_ASSERT(cncQ);
		return TradeRangeM(cncQ) * FeetPerMeter();
	}

	inline QString VehicleSpotTypeName(VEHICLE_SPOT_TYPE spotType)
	{
		if(spotType < SPOT_Invalid || spotType >= SPOT_Count)
			return "";
		QString spotTypes[SPOT_Count] =
		{
			QString(""),
			QObject::tr("ground vehicle", "Vehicle parking spot type name"),
			QObject::tr("space rocket", "Vehicle parking spot type name"),
			QObject::tr("space vehicle", "Vehicle parking spot type name"),
			QObject::tr("water vehicle", "Vehicle parking spot type name"),
			QObject::tr("specific vehicle", "Vehicle parking spot type name"),
		};
		return spotTypes[spotType];
	}

	// Cannons determines the number of heavy weapon rounds that can be fired simultaneously from a ballistic weapon bay.
	inline quint32 BayWeaponCannons(quint32 weaponUnits)
	{
		if(!weaponUnits)
			return 0;
		return ((quint32)round((float)weaponUnits/10.f)) + 1;
	}

	// Ejector racks determines the number of bombs that can be dropped simultaneously from a weapon deployment bay.
	inline quint32 BayWeaponEjectorRacks(quint32 weaponUnits)
	{
		if(!weaponUnits)
			return 0;
		return ((quint32)round((float)weaponUnits/100.f)) + 1;
	}

	// Projectors determines the number of energy weapons that can be fired simultaneously from an energy weapon bay.
	inline quint32 BayWeaponProjectors(quint32 weaponUnits)
	{
		if(!weaponUnits)
			return 0;
		return ((quint32)round((float)weaponUnits/25.f)) + 1;
	}

	// Launchers determines the number of missiles or rockets that can be launched simultaneously from a weapon launcher bay.
	inline quint32 BayWeaponLaunchers(quint32 weaponUnits)
	{
		if(!weaponUnits)
			return 0;
		return ((quint32)round((float)weaponUnits/90.f)) + 1;
	}
}	// End of AuDef namespace

using namespace AuDef;

#endif // AuDef_H
