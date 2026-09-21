//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtccpacket.h,v 1.70 2022/03/10 19:47:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtCCPacket_H
#define DtCCPacket_H

namespace DtChatChannelPacket
{
	// Atoms are only sent to the client from the server, never back the other way.
	// Other packets vary in usage: client-to-server or server-to-client or both.
	enum PACKET
	{
		P_Invalid,

		// Building construction related packets
		PB_BdBuild,
		PB_ChangeRoadConstruction,
		PB_ClearLand,					// Clear land info sent to server.
		PB_Demolish,
		PB_PlanFence,					// Plan a fence segment.
		PB_PlanRoadGlobe,				// Plan a road segment.
		PB_RaiseFlag_Obsolete,			// Raise flag info sent to server.

		// Chat
		PC_ChannelOpened,				// Chat channel opened.
		PC_ChannelClosed,				// Chat channel closed.
		PC_Chat,						// Chat message.
		PC_Connect,						// Initial connection packet, includes list of channels and unit id info.
		PC_OpenChannelTo,				// Open channel to specific atom id.
		PC_PostBounty,					// Post a bounty on a DaUnit
		PC_SquelchAdd,					// Add a squelch id to the list
		PC_SquelchDelete,				// Remove a squelch id from the list
		PC_SquelchUpdate,				// Sending list of squelch ids

		// Spacecraft design
		PD_BdDesign,					// Request for an AuBdDesign from client. Design rec from server.
		PD_BdDesignHere,				// Notification from client that it has loaded an AuBdDesign from cache.
		PD_BdErase,						// Design erase request from server. Precedes design.
		PD_BdExchange,
		PD_BdLibrary,
		PD_BdLoaded,					// AuDesign record sent from client to server when loaded from a file.
		PD_Exit,						// Exit design environment from client.
		PD_Gravity,						// Toggle gravity from client.
		PD_New,							// Start a nu design from client.
		PD_Open,						// Open a design from client.
		PD_Properties,					// Update design properties from client.
		PD_Redo,
		PD_ScDesign,					// Request for an AuScDesign from client. Design rec from server.
		PD_ScDesignHere,				// Notification from client that it has loaded an AuScDesign from cache.
		PD_ScErase,						// Design erase request from server. Precedes design.
		PD_ScExchange,
		PD_ScLibrary,
		PD_ScLoaded,					// AuDesign record sent from client to server when loaded from a file.
		PD_ScPart,
		PD_Undo,

		PE_ChangeFlag,
		PE_GABalance,					// Request balance of government account.
		PE_GADeposit,					// Deposit funds into government account.
		PE_GAWithdraw,					// Withdraw funds from government account.
		PE_LeaveEmpire,					// Remove user from empire sent to server.
		PE_Rename,						// Rename city, planet, system, sector
		PE_Report,						// Request report to server, report data to client.

		// DpStation::STATION_TYPE overlaps here for client restore of station settings to server.
		PF_Station0,					// STATION_Command
		PF_Station1,
		PF_Station2,
		PF_Station3,
		PF_Station4,
		PF_Station5,
		PF_Station6,
		PF_Station7,
		PF_Station8,
		PF_Station9,
		PF_Station10,
		PF_Station11,
		PF_Station12,
		PF_Station13,
		PF_Station14,
		PF_Station15,
		PF_Station16,
		PF_Station17,
		PF_Station18,
		PF_Station19,					// STATION_Count

		// Personal gear and control.
		PI_AvatarSetting,
		PI_BankBorrow,
		PI_BankPay,
		PI_Drop,						// Drop gear item(s)
		PI_GearItem,					// Does various things to a selected gear item.
		PI_GetFromCargo,				// Try to get an item from the ship's cargo.
		PI_Inscribe,					// Inscribe an item in my gear.
		PI_Manipulate,					// Manipulates a static display item in the environment.
		PI_Message,						// Sets message text on an item in my gear.
		PI_Ordnance,					// Used for requesting, transferring ordnance.
		PI_PutIntoCargo,				// Try to put an item into the ship's cargo.
		PI_Selfie,
		PI_SendFunds,					// transfer funds to another atom, could be a player, city, or spacecraft.
		PI_SetFlowerPot,				// Set settings of a flower pot item in my gear.
		PI_SetPictureFrame,				// Set settings of a picture frame item in my gear.
		PI_TransferCargo,				// Performs all the various cargo transfer operations on the cargo transfer ui.
		PI_TurboLift,
		PI_Use,							// Use gear item

		PJ_Accept,						// Accept job sent to server.
		PJ_Query,						// Query for jobs that can be started. Available AuJobOffer back to client.
		PJ_QueryFilter,					// Query for jobs that can be started, with list of types. AuJobOffer back to client.

		// Manufacture/Construction/Work/Labor UI
		PM_ArenaExit,
		PM_ArenaRecall,
		PM_ArenaStatus,
		PM_CitizenEnabled,				// citizen enabled state changed
		PM_CreateArena,
		PM_EnterStudio,					// enter a design studio
		PM_Fetch,						// fetch manufacturing materials
		PM_Go,							// run process
		PM_HandCraftGo,					// run handcrafting process
		PM_Hangar,						// Performs various actions at a hangar.
		PM_JoinArena,
		PM_ProcessSelected,				// select a manufacturing process
		PM_ProcessSetMinQ,				// sets the minimum Q for a manufacturing process
		PM_QueryArenas,
		PM_ScFactory,					// perform spacecraft factory functions, and other things done at buildings

		PN_Mission,						// Mission information packet for modifying the current ship's mission.

		// Story line
		PS_ChoiceMade,					// Choice picked sent to server. Used to ack NarrateChoice or ChatChoice comm messages.
		PS_CloseStory,					// End story request to server.
		PS_StoryHint,					// client request for the hint associated with the current story block
		PS_StoryResendBlock,			// client request for repeat of story block info

		PT_GameOver,					// Notification to client that a game has ended, with a blob of results.
		PT_GameQuit,					// Client has quit the game, to server.
		PT_GameSelect,					// Selects a nu game for play at a game table, to server. Game select result to client.
		PT_GameStart,					// Contains a blob of settings for the current game, to server.
		PT_GameTurn,					// Sent to client periodically with person id of whose turn it is now. Sent to server when
										// client completes their turn.

		// PTR_ Transporter Room. See AuBeamOptions.
		PTR_Place,						// Place destination identified using TRANSPORTER_TARGET.
		PTR_InputCoord,					// Cartesian coordinate entered at transporter console.
		PTR_InputLongLat,				// Longitude latitude coordinate entered at transporter console.
		PTR_Ship,						// Ship destination.
		PTR_Transporter,				// Transporter room destination.

		// Unit gear Control
		PU_SetAmmo,
		PU_SetAmmoCount,
		PU_SetArmor,
		PU_SetClothing,
		PU_SetHelmet,
		PU_SetWeapon,

		// Governance
		PV_AcceptCrew,					// Change auto-accept policy on a spaceship.
		PV_AcceptPassengers,			// Change auto-accept policy on a spaceship.
		PV_AcceptTroops,				// Change auto-accept policy on a spaceship.
		PV_AllowPassengerSeaChests,		// Change passengers sea chest access policy on a spaceship.
		PV_AssignOffice,				// Assign office to player.
		PV_ChangeTitle,					// Changes title to a privately owned ship.
		PV_Christen,					// Christen ship person is aboard.
		PV_ClaimReward,					// Client req to claim rewards from empire
		PV_ContrailColor,
		PV_Dossier,						// Various commands to manage the dossier maintained on empires that have been encountered.
		PV_KickBerthOccupant,			// Kicks a specific occupant from their berth. Kicks stowaways when berth occupant id is 0.
		PV_ReceiveShipFunds,
		PV_ReconcileRoster,				// Remove unwanted crew, troops and passengers from roster.
		PV_ReleaseBerth,
		PV_ResignOffice,
		PV_RevokeOffice,				// Revokes a single atom from a person's domain.
		PV_SendShipFunds,
		PV_SetAnnouncements,
		PV_SetBoss,
		PV_SetCapital,					// Set current city to be the capital
		PV_SetCommerce,					// Set commodity commerce policies
		PV_SetDefStance,				// Set default political stance
		PV_SetPilotPolicy,
		PV_SetPoliticalStance,			// Set political stance on empire
		PV_SetShieldPolicy,				// Set shield 'fuel' or 'up' policy.
		PV_SetTax,						// Set taxation levels
		PV_SetThemeColor,
		PV_Specie,						// To server, names a specie. To client, contains a specie record.
		PV_SpeciesRequest,				// To server, requests a species record.
		PV_WarCriteria,

		// Admin
		PZ_AgentBits,					// Includes current desired state of agent bits.
		PZ_Capacitor,					// Fill or drain the ship's capacitor.
		PZ_ChangeShip,					// changes the ship the admin is aboard
		PZ_EmpireChgEmperor,
		PZ_EmpireChgFlag,
		PZ_EmpireChgName,
		PZ_EmpireChgRole,
		PZ_Encounter,
		PZ_EnvironmentChange,
		PZ_GoToAtom,
		PZ_GoToLongLat,
		PZ_KickStowaways,
		PZ_KillAnimals,					// kills all wild or domestic animals in the scene
		PZ_MakeCommodity,				// Creates an item or vehicle at the agent's location.
		PZ_QueryAvatars,				// Query for current list of avatar ids and names from login server.
		PZ_Refuel,
		PZ_ResetStory,
		PZ_SetDNA,						// Change DNA of a creature.
		PZ_SummonPerson,
		PZ_Trade,						// Manage trade routes.

		PC_HailStore,					// Triggers a building to open a private channel to me.
		PS_StorySubmission,				// client submits a script
		PD_Position,					// Move avatar in design environment.
		PZ_EmpireChg,
		PI_Insurer,
		PZ_GoToGalaxyLoc,
		PD_ItLibrary,
		PD_ItDesign,					// Request for an AuItDesign from client. Design rec from server.
		PD_ItDesignHere,				// Notification from client that it has loaded an AuItDesign from cache.
		PD_ItLoaded,					// AuDesign record sent from client to server when loaded from a file.
		PD_ItErase,						// Design erase request from server. Precedes design.
		PC_Emoji,
		PV_SetJoinPass,
		PZ_TickTock,
	};

	inline const char* PacketName(PACKET p)
	{
		const char* packetNames[] =
		{
		"P_Invalid",

		// Building construction related packets
		"PB_BdBuild",
		"PB_ChangeRoadConstruction",
		"PB_ClearLand",
		"PB_Demolish",
		"PB_PlanFence",
		"PB_PlanRoadGlobe",
		"PB_RaiseFlag_Obsolete",

		// Chat
		"PC_ChannelOpened",
		"PC_ChannelClosed",
		"PC_Chat",
		"PC_Connect",
		"PC_OpenChannelTo",
		"PC_PostBounty",
		"PC_SquelchAdd",
		"PC_SquelchDelete",
		"PC_SquelchUpdate",

		// Spacecraft design
		"PD_BdDesign",
		"PD_BdDesignHere",
		"PD_BdErase",
		"PD_BdExchange",
		"PD_BdLibrary",
		"PD_BdLoaded",
		"PD_Exit",
		"PD_Gravity",
		"PD_New",
		"PD_Open",
		"PD_Properties",
		"PD_Redo",
		"PD_ScDesign",
		"PD_ScDesignHere",
		"PD_ScErase",
		"PD_ScExchange",
		"PD_ScLibrary",
		"PD_ScLoaded",
		"PD_ScPart",
		"PD_Undo",

		"PE_ChangeFlag",
		"PE_GABalance",
		"PE_GADeposit",
		"PE_GAWithdraw",
		"PE_LeaveEmpire",
		"PE_Rename",
		"PE_Report",

		// DpStation::STATION_TYPE overlaps here.
		"PF_Station0",
		"PF_Station1",
		"PF_Station2",
		"PF_Station3",
		"PF_Station4",
		"PF_Station5",
		"PF_Station6",
		"PF_Station7",
		"PF_Station8",
		"PF_Station9",
		"PF_Station10",
		"PF_Station11",
		"PF_Station12",
		"PF_Station13",
		"PF_Station14",
		"PF_Station15",
		"PF_Station16",
		"PF_Station17",
		"PF_Station18",
		"PF_Station19",

		// Personal gear and control.
		"PI_AvatarSetting",
		"PI_BankBorrow",
		"PI_BankPay",
		"PI_Drop",
		"PI_GearItem",
		"PI_GetFromCargo",
		"PI_Inscribe",
		"PI_Manipulate",
		"PI_Message",
		"PI_Ordnance",
		"PI_PutIntoCargo",
		"PI_Selfie",
		"PI_SendFunds",
		"PI_SetFlowerPot",
		"PI_SetPictureFrame",
		"PI_TransferCargo",
		"PI_TurboLift",
		"PI_Use",

		"PJ_Accept",
		"PJ_Query",
		"PJ_QueryFilter",

		// Manufacture/Construction/Work/Labor UI
		"PM_ArenaExit",
		"PM_ArenaRecall",
		"PM_ArenaStatus",
		"PM_CitizenEnabled",
		"PM_CreateArena",
		"PM_EnterStudio",
		"PM_Fetch",
		"PM_Go",
		"PM_HandCraftGo",
		"PM_Hangar",
		"PM_JoinArena",
		"PM_ProcessSelected",
		"PM_ProcessSetMinQ",
		"PM_QueryArenas",
		"PM_ScFactory",

		"PN_Mission",

		// Story line
		"PS_ChoiceMade",
		"PS_CloseStory",
		"PS_StoryHint",
		"PS_StoryResendBlock",

		"PT_GameOver",
		"PT_GameQuit",
		"PT_GameSelect",
		"PT_GameStart",
		"PT_GameTurn",
								// completes their turn.

		// PTR_ Transporter Room. See AuBeamOptions.
		"PTR_Place",
		"PTR_InputCoord",
		"PTR_InputLongLat",
		"PTR_Ship",
		"PTR_Transporter",

		// Unit gear Control
		"PU_SetAmmo",
		"PU_SetAmmoCount",
		"PU_SetArmor",
		"PU_SetClothing",
		"PU_SetHelmet",
		"PU_SetWeapon",

		// Governance
		"PV_AcceptCrew",
		"PV_AcceptPassengers",
		"PV_AcceptTroops",
		"PV_AllowPassengerSeaChests",
		"PV_AssignOffice",
		"PV_ChangeTitle",
		"PV_Christen",
		"PV_ClaimReward",
		"PV_ContrailColor",
		"PV_Dossier",
		"PV_KickBerthOccupant",
		"PV_ReceiveShipFunds",
		"PV_ReconcileRoster",
		"PV_ReleaseBerth",
		"PV_ResignOffice",
		"PV_RevokeOffice",
		"PV_SendShipFunds",
		"PV_SetAnnouncements",
		"PV_SetBoss",
		"PV_SetCapital",
		"PV_SetCommerce",
		"PV_SetDefStance",
		"PV_SetPilotPolicy",
		"PV_SetPoliticalStance",
		"PV_SetShieldPolicy",
		"PV_SetTax",
		"PV_SetThemeColor",
		"PV_Specie",
		"PV_SpeciesRequest",
		"PV_WarCriteria",

		// Admin
		"PZ_AgentBits",
		"PZ_Capacitor",
		"PZ_ChangeShip",
		"PZ_EmpireChgEmperor",
		"PZ_EmpireChgFlag",
		"PZ_EmpireChgName",
		"PZ_EmpireChgRole",
		"PZ_Encounter",
		"PZ_EnvironmentChange",
		"PZ_GoToAtom",
		"PZ_GoToLongLat",
		"PZ_KickStowaways",
		"PZ_KillAnimals",
		"PZ_MakeCommodity",
		"PZ_QueryAvatars",
		"PZ_Refuel",
		"PZ_ResetStory",
		"PZ_SetDNA",
		"PZ_SummonPerson",
		"PZ_Trade",

		"PC_HailStore",
		"PS_StorySubmission",
		"PD_Position",
		"PZ_EmpireChg",
		"PI_Insurer",
		"PZ_GoToGalaxyLoc",
		"PD_ItLibrary",
		"PD_ItDesign",
		"PD_ItDesignHere",
		"PD_ItLoaded",
		"PD_ItErase",
		"PC_Emoji",
		"PV_SetJoinPass",
		};
		return packetNames[p];
	}
}

#endif // DtCCPacket_H
