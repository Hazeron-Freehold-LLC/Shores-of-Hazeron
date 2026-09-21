//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmcpacket.h,v 1.4 2022/02/19 18:39:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtMCPacket_H
#define DtMCPacket_H

namespace DtMgrChannelPacket
{
	// Packet types used when communicating between the scene server and the scene client.
	enum PACKET
	{
		P_Invalid,
		P_Connect,						// Connection information. This must be the first packet received.
		P_Status,						// Request for status from client. Status info from server.
		P_KickUnit,						// Command to kick unit from client.
		P_Debug,						// Command to manipulate debug settings.
		P_Resources,					// Request for resource usage from client. Resource usage info from server.
		P_ReloadData,					// Reloads static data. Ends up calling DgData::Load.
		P_LimboRescue,
		P_Shutdown,						// Shuts down a server and all subordinate servers.
		P_Revoke,						// Revokes assignment of an atom from a subserver of this server.
		P_Setting,						// Changes a global setting
		P_StatusQuery,					// Sent by AuServerStatus to get info to display on the server status html page.
		P_FinalizeSystem,	// Sent to manager to finalize and assign a solar system. Ignored if the system is already finalized.
		P_SetModified,					// Calls SetModified on an atom.
		P_CreateSectorAt,
		P_Reassign,
		P_DebugMessages,				// Command to manipulate debug message bits.
		P_MemoryUsage,
		P_RestoreEmpire,
		P_ReloadScript,					// Reloads a story script, or all.
		P_Announcement
	};

	inline const char* PacketName(PACKET p)
	{
		const char* packetNames[] =
		{
			"P_Invalid",
			"P_Connect",
			"P_Status",
			"P_KickUnit",
			"P_Debug",
			"P_Resources",
			"P_ReloadData",
			"P_LimboRescue",
			"P_Shutdown",
			"P_Revoke",
			"P_Setting",
			"P_StatusQuery",
			"P_FinalizeSystem",
			"P_SetModified",
			"P_CreateSectorAt",
			"P_Reassign",
			"P_DebugMessages",
			"P_MemoryUsage",
			"P_RestoreEmpire",
			"P_ReloadScripts",
		};
		return packetNames[p];
	}
}

#endif // DtMCPacket_H
