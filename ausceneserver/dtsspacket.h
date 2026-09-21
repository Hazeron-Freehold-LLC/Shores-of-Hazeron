//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtsspacket.h,v 1.38 2022/02/19 18:39:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtSSPacket_H
#define DtSSPacket_H

namespace DtSceneServerPacket
{
	// Packet types used when communicating between the scene server and the scene client.
	enum PACKET
	{
		// DaAtom::ATOM_TYPE overlaps here 1:1
		P_Invalid,				// ATOM_Invalid
		P_SyncComplete = 50,	// ATOM_Count

		// Game start
		PE_DeletePerson,
		PE_EstDeferred,			// Deferred Establish request to another server.
		PE_EstError,			// Error trying to EstablishFirstEmpire sent to client.
		PE_SceneDeferred,		// Deferred scene sync to another server.
		PE_StartNewAvatar,		// Create first empire data sent to master server.
		PE_Success,				// Successful Est/Join result.

		// Game data
		PG_Commodity,			// Commodity rec sent to client from server.
		PG_Data,				// Client req for basic game data from server. Server sends at end of game data reply.
		PG_Empire,				// Empire rec sent to client. Request for rec from client.
		PG_EmpiresAll,			// Request from client for all empire records.
		PG_EmpiresJoinable,		// Request from client for joinable empire records.
		PG_Industry,			// Industry rec sent to client from server
		PG_ManufacturingComp,	// Manufacturing process component sent to client.
		PG_ManufacturingProc,	// Manufacturing process sent to client.
		PG_StarMap,				// Request/reply when requesting galaxy viewer data.

		// Personal control
		PI_Collision,			// Client movement resulted in a collision.
		PI_Control,				// Client control input sent as a UDP Packet.
		PI_Deadhead,			// Client movement resulted in deadhead departure from solar system.
		PI_EnteredWormhole,		// Client entered a wormhole while under client-side control.
		PI_Fell,				// Client movement resulted in a fall.
		PI_Position,			// Client position update

		// Login/logout
		PL_KeepAlive,			// Client sends a keep alive packet.
		PL_Login,				// Login sent to server. Good reply to client.
		PL_LoginFailure,		// Login failure response sent to client.
		PL_Logout,				// Logout notification sent to client when logged out by server. Client sends when logging out
		PL_Setting,
		PL_SwitchAvatar,		// Request from player to switch to another character id.

		// Admin
		PZ_PlayersQuery,		// Sent by admin to get a list of players who are currently online.
		PZ_StatusQuery,			// Sent by AuServerStatus to check my current status.
	};

	inline const char* PacketName(PACKET p)
	{
		const char* packetNames[] =
		{
			"ATOM_Invalid",
			"ATOM_Atom",
			"ATOM_Animal",
			"ATOM_Arena",
			"ATOM_Asteroid",
			"ATOM_Bd",
			"ATOM_Capsule",
			"ATOM_CitCharacter",
			"ATOM_CitCrew",
			"ATOM_CitIndigenous",
			"ATOM_Citizen",
			"ATOM_CitOfficer",
			"ATOM_CitPassenger",
			"ATOM_CitTroop",
			"ATOM_CitWorker",
			"ATOM_Creature",
			"ATOM_DesignBase",
			"ATOM_Designer",
			"ATOM_DesignPlane",
			"ATOM_Effect",
			"ATOM_Galaxy",
			"ATOM_Item",
			"ATOM_Missile",
			"ATOM_Person",
			"ATOM_Root",
			"ATOM_Sc",
			"ATOM_Sector",
			"ATOM_Sentient",
			"ATOM_SolarSystem",
			"ATOM_Star",
			"ATOM_Teleporter",
			"ATOM_Unit",
			"ATOM_Universe",
			"ATOM_Vehicle",
			"ATOM_Vessel",
			"ATOM_WorldGasGiant",
			"ATOM_WorldGlobe",
			"ATOM_World",
			"ATOM_WorldLL",
			"ATOM_WorldRing",
			"ATOM_WorldRingSection",
			"ATOM_WorldSphere",
			"ATOM_42",
			"ATOM_43",
			"ATOM_44",
			"ATOM_45",
			"ATOM_46",
			"ATOM_47",
			"ATOM_48",
			"ATOM_49",
			"P_SyncComplete",	// ATOM_Count

			// Game start
			"PE_DeletePerson",
			"PE_EstDeferred",
			"PE_EstError",
			"PE_SceneDeferred",
			"PE_StartNewAvatar",
			"PE_Success",

			// Game data
			"PG_Commodity",
			"PG_Data",
			"PG_Empire",
			"PG_EmpiresAll",
			"PG_EmpiresJoinable",
			"PG_Industry",
			"PG_ManufacturingComp",
			"PG_ManufacturingProc",
			"PG_StarMap",

			// Personal control
			"PI_Collision",
			"PI_Control",
			"PI_Deadhead",
			"PI_EnteredWormhole",
			"PI_Fell",
			"PI_Position",

			// Login/logout
			"PL_KeepAlive",
			"PL_Login",
			"PL_LoginFailure",
			"PL_Logout",
			"PL_Setting",
			"PL_SwitchAvatar",

			// Admin
			"PZ_PlayersQuery",
			"PZ_StatusQuery",
		};
		return packetNames[p];
	}
}

#endif // DtSSPacket_H
