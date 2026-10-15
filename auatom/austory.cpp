//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austory.cpp,v 1.322 2022/05/16 23:02:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austory.h"
//
#include "aucommpacket.h"
#include "audnabase.h"
#include "aufilever.h"
#include "auglobal.h"
#include "auscreport.h"
#include "auspecies.h"
#include "austoryres.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacapsule.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldll.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "hashpack.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTextStream>
#include <QVariant>

static int s_AuScriptLineMetaType = AuScriptLine::RegisterMetaType();

//static
int AuScriptLine::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections and QVariants.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuScriptLine>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuScriptLine>();

	return metaType;
}

QDataStream& operator<<(QDataStream& out, const AuScriptLine& o)
{
	o >> out;
	return out;
}

QDataStream& operator>>(QDataStream& in, AuScriptLine& o)
{
	o << in;
	return in;
}

enum UNIVERSE_ACCESS
{
	UA_Const,							// Function does not modify the universe, including does not communicate.
	UA_ConstOnce,						// Function does not modify the universe. Must not be executed twice if block is resent.
	UA_Dynamic,							// Function may modify the universe.
	UA_DynamicOnce,						// Function may modify the universe. Must not be executed twice if block is resent.
};

class StoryFunc
{
public:
	bool								m_CatalystOnly;			// Function can only appear in the catalyst. e.g. story definitions.
	QString								m_Desc;					// a brief description
	SCRIPT_FUNC							m_Func;					// function opcode
	QString								m_Name;					// function name
	QString								m_Parms;				// parameter types: see AuScript::ListParameterTypes.
	QString								m_ReturnParm;			// return parameter type. # is doc section
	UNIVERSE_ACCESS						m_UniverseAccess;

	StoryFunc(SCRIPT_FUNC func, const QString& returnParm, const QString& name, UNIVERSE_ACCESS ua, const QString& parms,
			const QString& desc, bool catalystOnly=false)
	{
		m_CatalystOnly = catalystOnly;
		m_Desc = desc;
		m_Func = func;
		m_Name = name;
		m_Parms = parms;
		m_ReturnParm = returnParm;
		m_UniverseAccess = ua;
	}

	// Body functions could modify the universe.
	inline bool OkInBody() const
	{
		return !m_CatalystOnly;
	}

	// Catalyst functions are not expected to modify the universe.
	inline bool OkInCatalyst() const
	{
		return UniverseConst() || UniverseConstOnce();
	}

	inline bool UniverseConst() const
	{
		return m_UniverseAccess == UA_Const;
	}

	inline bool UniverseConstOnce() const
	{
		return m_UniverseAccess == UA_ConstOnce;
	}

	inline bool UniverseDynamic() const
	{
		return m_UniverseAccess == UA_Dynamic;
	}

	inline bool UniverseDynamicOnce() const
	{
		return m_UniverseAccess == UA_DynamicOnce;
	}
};

// These are functions that can be used in a story segment.
// NOTE: These are expected to be in the order of the SCRIPT_FUNC enumeration so it can be indexed directly using the enum value.
StoryFunc s_StoryFunc[F_Count] = 
{
	StoryFunc(F_MiscDocTitle, "#", "Miscellaneous Functions", UA_Const, "", ""),
	StoryFunc(F_NOP, "", "NOP", UA_Const, "", ""),
	StoryFunc(F_CONST, "", "CONST", UA_Const, "", ""),
	StoryFunc(F_VAR, "", "VAR", UA_Const, "", ""),
	StoryFunc(F_AND, "", "&", UA_Const, "", ""),
	StoryFunc(F_OR, "", "|", UA_Const, "", ""),
	StoryFunc(F_EQ, "", "==", UA_Const, "", ""),
	StoryFunc(F_LE, "", "<=", UA_Const, "", ""),
	StoryFunc(F_LT, "", "<", UA_Const, "", ""),
	StoryFunc(F_GE, "", ">=", UA_Const, "", ""),
	StoryFunc(F_GT, "", ">", UA_Const, "", ""),
	StoryFunc(F_GoTo, "void", "goto", UA_Const, "",
			"Unconditional branch to a block of the story."),
	StoryFunc(F_If, "void", "if", UA_Const, "bool",
			"In the catalyst block, ''if'' tests determine when the story can start for an avatar and "
			"which block will begin the story."
			"\n\n"
			"In body blocks, ''if'' tests determine when to branch to another block of the story."
			"\n\n"
			"Each block of the story, including the catalyst, may "
			"contain more than one ''if'' to test and branch based on different conditions. "
			"The story engine checks the ''if'' conditions of the current block every 10 seconds. "
			"Each ''if'' is tested in the order it appears in the script. "
			"The first ''if'' to return true results in a branch to its block of the story."),
	StoryFunc(F_ChoiceMade, "choiceValue", "choiceMade", UA_Const, "",
			"Test for selection of a specific choice that was offered using a choice command on a comm channel."),
	StoryFunc(F_DateIsChristmas, "bool", "dateIsChristmas", UA_Const, "",
			"Test if the current date is Christmas eve or Christmas day. Returns true on 12/24 and 12/25."),
	StoryFunc(F_SecondsElapsed, "int", "secondsElapsed", UA_Const, "",
			"Returns the number of seconds that have passed since the current story block was started."),
	StoryFunc(F_Wait, "bool", "wait", UA_Const, "int",
			"Returns true if ''int'' seconds or more have passed since the current story block was started."),

	StoryFunc(F_StoryDocTitle, "#", "Story Functions", UA_Const, "", "Story functions configure the current story. "
			"Most story functions can only be used in the catalyst block. "
			"The only exception is the ''storyDroppable'' function."),
	StoryFunc(F_StoryAuthor, "void", "storyAuthor", UA_Const, "text",
			"Identifies the author of the story. Required in catalyst block.", true),
	StoryFunc(F_StoryAutoRepeat, "void", "storyAutoRepeat", UA_Const, "",
			"Enables the story to automatically start repeatedly, based on the catalyst."
			"\n\n"
			"The story can restart within 10 seconds of being terminated if an interval is not specified."
			"\n\n"
			"Only one auto started story can be active at a time.", true),
	StoryFunc(F_StoryAutoRepeatUntilComplete, "void", "storyAutoRepeatUntilComplete", UA_Const, "",
			"Enables the story to automatically start repeatedly, based on the catalyst. "
			"\n\n"
			"The story can restart within 10 seconds of being terminated if an interval is not specified."
			"The story stops autostarting once it has been completed. "
			"\n\n"
			"A story that is dropped is not considered to be completed. "
			"\n\n"
			"Only one auto started story can be active at a time.", true),
	StoryFunc(F_StoryAutoStart, "void", "storyAutoStart", UA_Const, "",
			"Enables the story to automatically start once, based on the catalyst. "
			"\n\n"
			"Only one auto started story can be active at a time.", true),
	StoryFunc(F_StoryBrief, "void", "storyBrief", UA_Const, "text",
			"Gives a brief description of the story. Required in catalyst block.", true),
	StoryFunc(F_StoryDroppable, "void", "storyDroppable", UA_Dynamic, "bool",
			"Sets the current story's droppable state. "
			"The droppable state of a story can be changed at any time in the script. "
			"\n\n"
			"A story that is not droppable cannot be canceled by the avatar using the Jobs window. "
			"The avatar gets a thought message if they try dropping an undroppable story. "
			"\n\n"
			"Usually, avatars must be able to drop a story to enable it to restart. "
			"Disable drop only when dropping would break the story in some way. "),
	StoryFunc(F_StoryInterval, "void", "storyInterval", UA_Const, "int",
			"Specifies the time interval in minutes before the story can start again. "
			"\n\n"
			"The time interval starts when the story terminates, whether by finishing or by dropping. ", true),
	StoryFunc(F_StoryTitle, "void", "storyTitle", UA_Const, "text",
			"Declares the title of the story. Required in catalyst block.", true),

	// These person functions implicitly operate on the person running the story.
	StoryFunc(F_AvatarDocTitle, "#", "Avatar Functions", UA_Const, "", "Avatar functions operate on the avatar atom for "
			"whom the story is running. "),
	StoryFunc(F_Avatar, "avatar", "avatar", UA_Const, "",
			"Returns the person executing the story."),
	StoryFunc(F_AvatarConstructedBuildingHabitable, "bool", "avatarConstructedBuildingHabitable", UA_Const, "building",
			"Test if the person has ever constructed a particular kind of building on a habitable world. "
			"True is returned if they ever planned one, regardless of whether it was completed."),
	StoryFunc(F_AvatarConstructedBuildingHarsh, "bool", "avatarConstructedBuildingHarsh", UA_Const, "building",
			"Test if the person has ever constructed a particular kind of building on a world with a harsh environment. "
			"True is returned if they ever planned one, regardless of whether it was completed."),
	StoryFunc(F_AvatarCreditGovtDebt, "void", "avatarCreditGovtDebt", UA_DynamicOnce, "real",
			"Credits the person's government debt account with a number of cronodollars."),
	StoryFunc(F_AvatarCurrentItem, "bool", "avatarCurrentItem", UA_Const, "commodity",
			"Test if the person's current item is a commodity."),
	StoryFunc(F_AvatarEnvironment, "atom", "avatarEnvironment", UA_Const, "",
			"Returns the environment atom that contains the person. "
			"This is usually a solar system but it can be an arena or a design studio. "
			"The environment containing the person is the only environment a story can access."),
	StoryFunc(F_AvatarEVAMinutes, "int", "avatarEVAMinutes", UA_Const, "",
			"Returns the number of minutes of EVA experience."),
	StoryFunc(F_AvatarHadItem, "bool", "avatarHadItem", UA_Const, "commodity",
			"Test if the person's gear has ever contained a commodity."),
	StoryFunc(F_AvatarHasFood, "bool", "avatarHasFood", UA_Const, "",
			"Test if the person's gear contains any food."),
	StoryFunc(F_AvatarHasItem, "bool", "avatarHasItem", UA_Const, "commodity",
			"Test if the person's gear contains a commodity."),
	StoryFunc(F_AvatarHasItemInHand, "bool", "avatarHasItemInHand", UA_Const, "commodity",
			"Test if the person is holding a commodity in their hand."),
	StoryFunc(F_AvatarHasItemInHandSpecific, "bool", "avatarHasItemInHandSpecific", UA_Const,
			"commodity,quality,empire,text",
			"Test if the person is holding a specific commodity in their hand. "
			"''text'' is compared against the inscription only if ''text'' is not blank."),
	StoryFunc(F_AvatarHasItemSpecific, "bool", "avatarHasItemSpecific", UA_Const, "commodity,quality,empire,text",
			"Test if the person's gear contains a very specific commodity. "
			"''text'' is compared against the inscription only if ''text'' is not blank."),
	StoryFunc(F_AvatarInCity, "bool", "avatarInCity", UA_Const, "",
			"Test if the person is in a city. The person must be standing in a developed terrain tile."),
	StoryFunc(F_AvatarInitScenario, "bool", "avatarInitScenario", UA_Const, "scenario",
			"Test if the person started the game using a specified ''scenario''."),
	StoryFunc(F_AvatarInOrbit, "bool", "avatarInOrbit", UA_Const, "",
			"Test if the person is in orbit at a planet, outside the atmosphere."),
	StoryFunc(F_AvatarInStudio, "bool", "avatarInStudio", UA_Const, "",
			"Test if the person is inside a design studio instance."),
	StoryFunc(F_AvatarInVehicle, "bool", "avatarInVehicle", UA_Const, "commodity",
			"Test if the person is inside a certain type of vehicle."),
	StoryFunc(F_AvatarInVehicleMoving, "bool", "avatarInVehicleMoving", UA_Const, "commodity",
			"Test if the person is inside a certain type of vehicle that is moving."),
	StoryFunc(F_AvatarInVehicleOnGround, "bool", "avatarInVehicleOnGround", UA_Const, "commodity",
			"Test if the person is inside a certain type of vehicle that is touching the ground."),
	StoryFunc(F_AvatarItemCount, "int", "avatarItemCount", UA_Const, "commodity",
			"Returns the amount of a commodity possessed by the avatar."),
	StoryFunc(F_AvatarItemMakePermanent, "void", "avatarItemMakePermanent", UA_DynamicOnce,
			"commodity,quality,empire,text",
			"Makes permanent a specific item in the person's gear. "
			"Permanent story items persist in the game after the story ends. "
			"Nonpermanent story items are removed from the game when the story ends. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' is compared against the inscription only if ''text'' is not blank."),
	StoryFunc(F_AvatarItemSetDropOnDeath, "void", "avatarItemSetDropOnDeath", UA_DynamicOnce,
			"int,commodity,quality,empire,text",
			"Sets the ''drop'' behavior of an item: 0=default random drop on death, 1=no drop, 2=always drop. "
			"''bool'' is the new state."),
	StoryFunc(F_AvatarLineageAvatar, "bool", "avatarLineageAvatar", UA_Const, "",
			"Returns true if the avatar's lineage is a regular avatar."),
	StoryFunc(F_AvatarLineageDemiavatar, "bool", "avatarLineageDemiavatar", UA_Const, "",
			"Returns true if the avatar's lineage is demiavatar."),
	StoryFunc(F_AvatarLineageHighborn, "bool", "avatarLineageHighborn", UA_Const, "",
			"Returns true if the avatar's lineage is highborn."),
	StoryFunc(F_AvatarOnLineMinutes, "int", "avatarOnLineMinutes", UA_Const, "",
			"Returns number of minutes the person has been on line."),
	StoryFunc(F_AvatarStoryCompleted, "bool", "avatarStoryCompleted", UA_Const, "int",
			"Test if the person has completed a particular story script id."),
	StoryFunc(F_AvatarStoryLastResult, "int", "avatarStoryLastResult", UA_Const, "int",
			"Returns a result code associated with a particular story script id. "
			"0 is returned if the story has never been started."),
	StoryFunc(F_AvatarStoryResult, "void", "avatarStoryResult", UA_DynamicOnce, "int",
			"Stores the result for the current story in an avatar's memory. "
			"''int'' is result, in the range 0 to 255. "),
	StoryFunc(F_AvatarVehicleMinutes, "int", "avatarVehicleMinutes", UA_Const, "commodity",
			"Returns number of minutes the person has experience in a vehicle."),

	// From here on, we reference story objects using variables. The first parameter of every function, if there is one, tends to
	// be the object on which the function operates, similar to the 'this' pointer. When the object is not valid, all functions
	// return quietly.
	StoryFunc(F_AtomDocTitle, "#", "Atom Functions", UA_Const, "", "Atom functions operate on a specific atom, which is the first "
			"argument of most atom functions. "
			"When the ''atom'' argument is not valid, all functions return quietly. "),
	StoryFunc(F_Atom, "atom", "atom", UA_Const, "int",
			"Casts ''int'' to be an atom identifier."),
	StoryFunc(F_AtomCity, "city", "atomCity", UA_Const, "atom",
			"Returns the city capitol building atom whose jurisdiction controls the building site that contains ''atom''. "
			"\n\n"
			"The result is 0 if the atom's current building is not part of a city. "
			"\n\n"
			"If ''atom'' is on a road, the capitol building whose jurisdiction contains ''atom'' is returned. "
			"\n\n"
			"The result is 0 if ''atom'' is not at a building site or on a road in a city. "),
	StoryFunc(F_AtomComm, "void", "atomComm", UA_Dynamic, "atom,channel,range,text",
			"Atom sends a message on a comm channel."),
	StoryFunc(F_AtomCommChannelToAtom, "void", "atomCommChannelToAtom", UA_Dynamic, "atom,atom,channel",
			"Atom opens a channel to the other atom."),
	StoryFunc(F_AtomCommChoice, "void", "atomCommChoice", UA_Dynamic, "atom,channel,range,choiceValue,text",
			"Atom says a choice on a comm channel. "
			"A choice message appears with a bright green asterisk. "
			"\n\n"
			"Several choices may be stated if each is given a different choiceValue. "
			"\n\n"
			"Result of player's choice is tested using the choiceMade function."),
	StoryFunc(F_AtomCreateCharacter, "character", "atomCreateCharacter", UA_DynamicOnce,
			"atom,angle,real,text,gender,hand,empire,dna,stage",
			"Creates a character near ''atom'', subordinate to the same parent as ''atom''. "
			"Will not create the charater if ''atom'' is in a vehicle. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''angle'' is relative to the atom's facing: 0 is forward, 90 is left, -90 is right and 180 is behind. "
			"\n\t"
			"''real'' is a distance in meters. "
			"\n\t\t"
			"The new character's location will be in the domain of ''atom''s parent. i.e. on the same ship or world. "
			"The path from ''atom'' to the new character's location will be unobstructed. "
			"The final starting location will be no farther than ''real'' meters from ''atom'', "
			"even if it means starting on top of ''atom''. "
			"It will be closer than ''real'' if the path to the starting location from the ''atom'' is obstructed. "
			"\n\t"
			"''text'' is the name of the new character. "
			"\n\t\t"
			"If a character with the same name and story instance exists in the environment, "
			"it is moved rather than create a new character. "
			"\n\t"
			"''dna'' specifies the DNA of the character. "
			"However, if the avatar of the story has ever met a story character with the same name, in any prior encounter, "
			"then the ''dna'' argument is overridden by the DNA of the original character met by the avatar. "),
	StoryFunc(F_AtomDistanceToAtom, "real", "atomDistanceToAtom", UA_Const, "atom,atom",
			"Returns the distance between two atoms, in meters. "),
	StoryFunc(F_AtomEmpire, "int", "atomEmpire", UA_Const, "atom",
			"Returns id value of an atom's empire. 0 if none."),
	StoryFunc(F_AtomFindBuilding, "atom", "atomFindBuilding", UA_ConstOnce, "atom,building,int,stance,int",
			"Locates a building of a specific ''building'' type, subordinate to ''atom''. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is under construction requirement 0=yes, 1=no, 2=ok, "
			"\n\t"
			"''stance'' and ''int'' are used to check the atom empire's stance toward the building's empire. "
			"''int'' 0=stance must match, 1=stance must match or be better, 2=stance must match or be worse. "),
	StoryFunc(F_AtomFindVehicle, "vehicle", "atomFindVehicle", UA_ConstOnce, "atom,commodity,int",
			"Locates a vehicle of a specific ''commodity'' type, subordinate to ''atom''. "
			"''int'' is 1 if the vehicle must be new, 2 if the vehicle must not be new, and 0 if you don't care. "),
	StoryFunc(F_AtomHasAtom, "bool", "atomHasAtom", UA_Const, "atom,atom",
			"Searches the atom recursively for an atom. "
			"Returns true if the atom is found."),
	StoryFunc(F_AtomHasBuilding, "bool", "atomHasBuilding", UA_Const, "atom,building,int,stance,int",
			"Test recursively if an atom has a building type. "
			"The atom might typically be a system, world or city. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is under construction requirement 0=yes, 1=no, 2=ok, "
			"\n\t"
			"''stance'' and ''int'' are used to check the atom empire's stance toward the building's empire. "
			"''int'' 0=stance must match, 1=stance must match or be better, 2=stance must match or be worse. "),
	StoryFunc(F_AtomInSystem, "bool", "atomInSystem", UA_Const, "atom,system",
			"Returns true if atom is in a solar system."),
	StoryFunc(F_AtomIsAtom, "bool", "atomIsAtom", UA_Const, "atom,atom",
			"Returns true if atom == atom and atom is valid."),
	StoryFunc(F_AtomSector, "sector", "atomSector", UA_Const, "atom",
			"Returns the sector atom that contains ''atom''."),
	StoryFunc(F_AtomValid, "bool", "atomValid", UA_Const, "atom",
			"Returns true if the atom exists and looks valid. "
			"The atom must be in the same solar system, design studio, or arena as the avatar."),
	StoryFunc(F_AtomWorld, "world", "atomWorld", UA_Const, "atom",
			"Returns the world atom that contains ''atom''. "
			"''atom'' may be subordinate to the world at any subatom level or distance, possibly in space. "
			"\n\n"
			"Returns ''atom'' if atom is a world."),
	StoryFunc(F_AtomWorldLL, "ll", "atomWorldLL", UA_Const, "atom",
			"Returns the longitude latitude of an atom, if it is on a world."),

	StoryFunc(F_BlueprintDocTitle, "#", "Blueprint Functions", UA_Const, "", "Blueprint functions operate on the designs in the "
			"blueprint library. "
			"The library can contain many blueprints. "
			"Only the blueprints in use by game objects are loaded at any given time. "
			"\n\n"
			"Atoms may reference blueprints but blueprints are not atoms by themselves. "
			"i.e. a building atom references a building blueprint to know about its geometry and other design features."),
	StoryFunc(F_BlueprintSpacecraftLoad, "bool", "blueprintSpacecraftLoad", UA_Const, "int",
			"Test if the spacecraft design is loaded. "
			"Signals the need for it to be loaded, if it is not already loaded."),

	StoryFunc(F_CharacterDocTitle, "#", "Character Functions", UA_Const, "", "Character functions operate on a character atom that "
			"was created by this story. "
			"\n\n"
			"Characters are associated with a specific instance of a story when they are created. "
			"This enables multiple instances of the same story to exist in an environment simultaneously. "),
	StoryFunc(F_CharacterDepart, "void", "characterDepart", UA_DynamicOnce, "character",
			"Causes a character to depart the scene. "
			"The character disappears instantly with no visual effect. "),
	StoryFunc(F_CharacterDie, "void", "characterDie", UA_DynamicOnce, "character",
			"Kills a character. "
			"They fall to the ground and die like other sentient creatures. "
			"Their body dies of massive internal injuries with no bleed out time. "),
	StoryFunc(F_CharacterEquipFromCity, "void", "characterEquipFromCity", UA_DynamicOnce, "character,gearslot,commodity,text",
			"Transfer an item to a character's gear from the inventory of the current city. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' is an optional inscription to place on the item."
			"\n\n"
			"Quietly does nothing if the character already has an item in that slot."),
	StoryFunc(F_CharacterSetIndestructible, "void", "characterSetIndestructible", UA_Dynamic, "character,bool",
			"Sets indestructible state of a character. "),
	StoryFunc(F_CharacterWantItem, "void", "characterWantItem", UA_Dynamic, "character,commodity",
			"Causes the character to want a specific commodity. "
			"\n\n"
			"When a character wants an item, they will make a bid if it is offered on the voice channel. "),
	StoryFunc(F_CharacterWantItemSpecific, "void", "characterWantItemSpecific", UA_Dynamic,
			"character,commodity,quality,empire,text",
			"Causes the character to want a very specific commodity. "
			"\n\n"
			"When a character wants an item, they will make a bid if it is offered on the voice channel. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' is compared against the inscription only if ''text'' is not blank. "),

	StoryFunc(F_CityDocTitle, "#", "City Functions", UA_Const, "", "City functions operate on a capitol building atom. "),
	StoryFunc(F_CityFindNewVehicle, "vehicle", "cityFindNewVehicle", UA_Const, "city,commodity",
			"Returns a new vehicle in a city, at a factory that makes them."),
	StoryFunc(F_CityHasBuilding, "bool", "cityHasBuilding", UA_Const, "city,building,int",
			"Test if a city has a number of completed levels of a building type, as shown on the city report. "
			"Levels are shops, fields, generators, etc. as appropriate to the type of building. "),
	StoryFunc(F_CityHasFood, "bool", "cityHasFood", UA_Const, "city",
			"Test if a city has any food in any of its buildings."),
	StoryFunc(F_CityHasItem, "bool", "cityHasItem", UA_Const, "city,commodity",
			"Test if a city has a commodity in any of its buildings. "
			"\n\n"
			"Vehicle commodities return true if a new vehicle is found at a factory that makes them."),
	StoryFunc(F_CityHasPower, "bool", "cityHasPower", UA_Const, "city",
			"Test if a city has any electricity. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''city'' can be any building. "),
	StoryFunc(F_CityItemCount, "int", "cityItemCount", UA_Const, "city,commodity",
			"Returns the total amount of a commodity in a city's inventory. "
			"Does not count vehicles."),
	StoryFunc(F_CityPopulation, "int", "cityPopulation", UA_Const, "city",
			"Returns the population of a city."),

	StoryFunc(F_CreatureDocTitle, "#", "Creature Functions", UA_Const, "", "Creature functions operate on a specific creature, "
			"which is the first argument of all creature functions. "),
	StoryFunc(F_CreatureDyingOrDead, "bool", "creatureIsDyingOrDead", UA_Const, "creature",
			"Test if the creature's body is dying or dead."),
	StoryFunc(F_CreatureSetHealth, "void", "creatureSetHealth", UA_DynamicOnce, "creature,int",
			"Sets health level of a creature. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is the health level as a percentage, 0 to 100."),
	StoryFunc(F_CreatureSetMouth, "void", "creatureSetMouth", UA_DynamicOnce, "creature,visage,visage",
			"Sets ''visage'' pose of a creature. "
			"\n\n"
			"When the first ''visage'' pose is an animated pose, the second ''visage'' pose is set when the animation completes. "),
	StoryFunc(F_CreatureSetVisage, "void", "creatureSetVisage", UA_DynamicOnce, "creature,visage",
			"Sets ''visage'' of a creature. "),

	StoryFunc(F_FindDocTitle, "#", "Find Functions", UA_Const, "", "Find functions operate in the avatar's current environment. "
			"The environment is the solar system, design studio instance, or arena instance that contains the avatar. "),
	StoryFunc(F_FindGasGiant, "world", "findGasGiant", UA_ConstOnce, "orbit",
			"Locates a gas giant in the current environment. "
			"Avatar must not be in deep space. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''orbit'' may include the also types."),
	StoryFunc(F_FindMoon, "world", "findMoon", UA_ConstOnce, "orbit",
			"Locates a moon in the current environment. "
			"Avatar must not be in deep space. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''orbit'' may include the also types."),
	StoryFunc(F_FindPlanet, "world", "findPlanet", UA_ConstOnce, "orbit,int",
			"Locates a planet in the current environment. "
			"Avatar must not be in deep space. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''orbit'' may include the also types. "
			"\n\t"
			"''int'' is maximum hydrographics percentage in the range 0 to 100."),
	StoryFunc(F_FindRing, "world", "findRing", UA_ConstOnce, "orbit",
			"Locates a ring around a world in the current environment. "
			"Avatar must not be in deep space. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''orbit'' may include the also types."),
	StoryFunc(F_FindRingWorld, "world", "findRingWorld", UA_ConstOnce, "",
			"Locates a ringworld arc section in the current environment. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"Avatar must not be in deep space."),

	StoryFunc(F_HengeDocTitle, "#", "Henge Functions", UA_Const, "", "Henge functions operate on the henges that interconnect "
			"ringworld arc sections with other worlds in the ringworld's sector. "),
	StoryFunc(F_HengeSystemName, "text", "hengeSystemName", UA_Const, "system",
			"Returns the name of a solar system that is known to contain a henge, as returned by the ''sectorHengeSystem'' "
			"function. "),
	StoryFunc(F_HengeSystemWorld, "worldcat", "hengeSystemWorld", UA_Const, "system,int",
			"Returns the catalog name of a habitable world that contains a henge portal, if any. "
			"The henge portal will be more than 200m from any city. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is henge type: 0=any, 1=on globe, 2=on ringworld"),
	StoryFunc(F_HengeWorld, "world", "hengeWorld", UA_Const, "worldcat",
			"Returns the world id of a henge world, given its catalog name. "
			"Story avatar must be in the solar system with the world. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''worldcat'' is a catalog name of a henge world. "),
	StoryFunc(F_HengeWorldLL, "ll", "hengeWorldLL", UA_Const, "worldcat",
			"Returns the longitude latitude of a henge if there is one. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''worldcat'' is a catalog name of a henge world. "),
	StoryFunc(F_HengeWorldName, "text", "hengeWorldName", UA_Const, "worldcat",
			"Returns the name assigned to a henge world, given its catalog name."),
	StoryFunc(F_HengeWorldPortalActive, "bool", "hengeWorldPortalActive", UA_Const, "worldcat,ll",
			"Test if the henge portal on a world is active. "
			"An active portal is indicated when the henge fire turns blue. "
			"That is when it operates as a portal. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''worldcat'' is a catalog name of a henge world. "),

	StoryFunc(F_ItemDocTitle, "#", "Item Functions", UA_Const, "", "Item functions operate on a specific item, which is the first "
            "argument of all item functions. "
			"\n\n"
			"An item atom represents a commodity in the environment. "
			"Once it is picked up by anybody, the item atom ceases to exist and the item's data is added to their gear. "
			"The item atom can no longer be referenced after that. "
			"\n\n"
			"A new atom is created when that item, or any item, is dropped in the environment. "
			"It will not be the same atom as it was before being picked up. "
			"The item atom identifier must be reacquired to operate on the new item atom. "),
	StoryFunc(F_ItemArmWeapon, "void", "itemArmWeapon", UA_DynamicOnce, "item,unit,empire",
			"Arms a weapon, if the item is a weapon that can be armed. e.g. Plastic Explosive. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''unit'' is the unit who armed the weapon. "
			"\n\t"
			"''empire'' is the empire who armed the weapon. "),
	StoryFunc(F_ItemCreateContent, "void", "itemCreateContent", UA_DynamicOnce,
			"item,bool,int,commodity,quality,empire,text",
			"Creates a new item inside ''item'', which is expected to be a container. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''bool'' is true to make the item permanent. "
			"Permanent story items persist in the game after the story ends. "
			"Nonpermanent story items are removed from the game when the story ends. "
			"\n\t"
			"''empire'' is the empire of origin of the item. "
			"\n\t"
			"''int'' is condition percent, 1 to 100. "
			"\n\t"
			"If ''text'' is not blank, it is inscribed onto the item."),
	StoryFunc(F_ItemDetonate, "void", "itemDetonate", UA_DynamicOnce, "item",
			"Detonates an armed weapon, if the item is a weapon that is armed. e.g. Plastic Explosive. "
			"Other items disappear quietly."),
	StoryFunc(F_ItemMakePermanent, "void", "itemMakePermanent", UA_DynamicOnce, "item",
			"Changes a story item into a permanent item. "
			"\n\n"
			"Permanent story items persist in the game after the story ends. "
			"Nonpermanent story items are removed from the game when the story ends. "),
	StoryFunc(F_ItemOnGround, "item", "itemOnGround", UA_Const, "commodity,real",
			"Locates an item on the ground nearby. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''real'' is the farthest distance from the story avatar to find the item, in meters."),
	StoryFunc(F_ItemOnGroundSpecific, "item", "itemOnGroundSpecific", UA_Const, "int,commodity,quality,empire,text",
			"Locates a specific item on the ground nearby. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is the farthest distance from the story avatar to find the item, in meters. "
			"\n\t"
			"''text'' is compared against the inscription only if ''text'' is not blank."),
	StoryFunc(F_ItemSetDropOnDeath, "void", "itemSetDropOnDeath", UA_DynamicOnce, "item,int",
			"Sets the drop behavior of an item: 0=default random drop on death, 1=no drop, 2=always drop. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is the new state."),
	StoryFunc(F_ItemSetMessage, "void", "itemSetMessage", UA_DynamicOnce, "item,text",
			"Sets the message of an item, such as a beacon. This is not the inscription. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' is the new message."),
	StoryFunc(F_ItemSetState, "void", "itemSetState", UA_DynamicOnce, "item,int",
			"Sets the state of an item, such as the on/off state of a light or musical instrument. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is the new state."),

	// Narrator functions.
	StoryFunc(F_NarrDocTitle, "#", "Narrator Functions", UA_Const, "", "Narrator functions provide a means for communicating with "
			"the story avatar. Narrator communications are associated with the story itself and they do not emanate from any in-"
			"game source. "),
	StoryFunc(F_NarrHint, "void", "narratorHint", UA_Dynamic, "text",
			"Sends a text message on the narrator comm channel when requested by the person. "
			"Hints are requested using the ? button on the Jobs window. "),
	StoryFunc(F_NarrImage, "void", "narratorImage", UA_Dynamic, "file,text,text",
			"Sends a picture, a title, and an optional message on the narrator comm channel. "
			"The image appears automatically in a popup window. "),
	StoryFunc(F_NarrSay, "void", "narratorSay", UA_Dynamic, "text",
			"Sends a text message on the narrator comm channel. "),
	StoryFunc(F_NarrSayAudio, "void", "narratorSayAudio", UA_Dynamic, "file,text",
			"Sends audio and an optional text message on the narrator comm channel."),
	StoryFunc(F_NarrSayChoice, "void", "narratorSayChoice", UA_Dynamic, "choiceValue,text",
			"Say a choice on the narrator comm channel. "
			"A choice message appears with a bright green asterisk. "
			"\n\n"
			"Several choices may be stated if each is given a different choiceValue. "
			"\n\n"
			"Result of player's choice is tested using the choiceMade function."),
	StoryFunc(F_NarrTitle, "void", "narratorTitle", UA_Dynamic, "text",
			"Sends a text message on the narrator comm channel formatted as a title. "),
	StoryFunc(F_NarrVideo, "void", "narratorVideo", UA_Dynamic, "file,text",
			"Sends video and an optional text message on the narrator comm channel. "),

	StoryFunc(F_NpcDocTitle, "#", "Npc Functions", UA_Const, "", "Npc functions operate on a specific npc, which is "
			"the first argument of all npc functions. "
			"Npc's are AI controlled citizens not including demiavatars who are AI controlled. "),
	StoryFunc(F_NpcSetRole, "void", "npcSetRole", UA_DynamicOnce, "npcrole",
			"Changes the role of an npc. "
			"\n\n"
			"The role of an npc cannot be changed while they hold a berth. "),

	StoryFunc(F_SectorDocTitle, "#", "Sector Functions", UA_Const, "", "Sector functions operate on a specific sector, which is "
			"the first argument of all sector functions. "),
	StoryFunc(F_SectorHasHengeSystem, "bool", "sectorHasHengeSystem", UA_Const, "sector,int",
			"Test if the sector contains a habitable world with a henge portal. "
			"The henge portal will be more than 200m from any city. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is henge type: 0=any, 1=on globe, 2=on ringworld"),
	StoryFunc(F_SectorHengeSystem, "system", "sectorHengeSystem", UA_Const, "sector,int",
			"Returns a solar system that contains a habitable world with a henge portal, if any. "
			"The henge portal will be more than 200m from any city. "
			"\n\n"
			"If more than one of them exists, one is chosen randomly. "
			"Ringworld systems are not returned even though they always contain henge portals."
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is henge type: 0=any, 1=on globe, 2=on ringworld"),

	StoryFunc(F_SentientDocTitle, "#", "Sentient Functions", UA_Const, "", "Sentient functions operate on a specific sentient "
			"creature, which is the first argument of all sentient functions. "),
	StoryFunc(F_SentientAttackUnit, "void", "sentientAttackUnit", UA_DynamicOnce, "sentient,unit",
			"Order a sentient to attack a unit."),
	StoryFunc(F_SentientCreateGear, "void", "sentientCreateGear", UA_DynamicOnce,
			"sentient,gearslot,bool,commodity,quality,empire,text",
			"Create an item in a sentient's gear. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''bool'' is true to make the item permanent. "
			"Permanent story items persist in the game after the story ends. "
			"Nonpermanent story items are removed from the game when the story ends. "
			"\n\t"
			"''empire'' is the empire of origin of the item. "
			"\n\t"
			"''text'' is inscribed on the item if not blank."),
	StoryFunc(F_SentientDropGear, "item", "sentientDropGear", UA_DynamicOnce, "sentient,gearslot",
			"Drops the item at a sentient's gear slot."),
	StoryFunc(F_SentientDropGearItem, "item", "sentientDropGearItem", UA_DynamicOnce, "sentient,commodity",
			"Drops an item from a sentient's gear."),
	StoryFunc(F_SentientFireWeapon, "void", "sentientFireWeapon", UA_DynamicOnce, "sentient",
			"Order a sentient to fire their weapon in hand at nothing in particular."),
	StoryFunc(F_SentientFollowAvatar, "void", "sentientFollowAvatar", UA_DynamicOnce, "sentient",
			"Order a sentient to follow the person."),
	StoryFunc(F_SentientFollowSentient, "void", "sentientFollowSentient", UA_DynamicOnce, "sentient,sentient",
			"Order a sentient to follow another sentient."),
	StoryFunc(F_SentientGetItem, "void", "sentientGetItem", UA_DynamicOnce, "sentient,item",
			"Order a sentient to pick an item up off the ground."),
	StoryFunc(F_SentientGoToAvatar, "void", "sentientGoToAvatar", UA_DynamicOnce, "sentient",
			"Order a sentient to go to the avatar."),
	StoryFunc(F_SentientGoToLL, "void", "sentientGoToLL", UA_DynamicOnce, "sentient,ll",
			"Order a sentient to go to a longitude latitude."),
	StoryFunc(F_SentientGoToLLExactly, "void", "sentientGoToLLExactly", UA_DynamicOnce, "sentient,ll",
			"Order a sentient to go to a longitude latitude as precisely as possible."),
	StoryFunc(F_SentientGoToRoom, "void", "sentientGoToRoom", UA_DynamicOnce, "sentient,text",
			"Order a sentient to go to a room, by room name. "
			"The sentient needs to be aboard a spacecraft."),
	StoryFunc(F_SentientGoToUnit, "void", "sentientGoToUnit", UA_DynamicOnce, "sentient,unit",
			"Order a sentient to go to a unit."),
	StoryFunc(F_SentientHasItem, "bool", "sentientHasItem", UA_Const, "sentient,commodity",
			"Test if the sentient possesses a specific commodity."),
	StoryFunc(F_SentientHasItemSpecific, "bool", "sentientHasItemSpecific", UA_Const,
			"sentient,commodity,quality,empire,text",
			"Test if the sentient's gear contains a very specific commodity. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' is compared against the inscription only if ''text'' is not blank."),
	StoryFunc(F_SentientHengeCeremony, "void", "sentientHengeCeremony", UA_DynamicOnce, "sentient,ll",
			"Order a sentient to perform the henge ceremony, if a henge is found near ''ll''."),
	StoryFunc(F_SentientItemMakePermanent, "void", "sentientItemMakePermanent", UA_DynamicOnce,
			"sentient,commodity,quality,empire,text",
			"Makes permanent a specific item in the sentient's gear. "
			"Permanent story items persist in the game after the story ends. "
			"Nonpermanent story items are removed from the game when the story ends. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' is compared against the inscription only if ''text'' is not blank."),
	StoryFunc(F_SentientItemSetDropOnDeath, "void", "sentientItemSetDropOnDeath", UA_DynamicOnce,
			"sentient,int,commodity,quality,empire,text",
			"Sets the drop behavior of an item: 0=default random drop on death, 1=no drop, 2=always drop. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''bool'' is the new state."),
	StoryFunc(F_SentientMoveBack, "void", "sentientMoveBack", UA_DynamicOnce, "sentient,real",
			"Order a sentient to move backward a distance specified in meters."),
	StoryFunc(F_SentientMoveForward, "void", "sentientMoveForward", UA_DynamicOnce, "sentient,real",
			"Order a sentient to move forward a distance specified in meters."),
	StoryFunc(F_SentientSay, "void", "sentientSay", UA_Dynamic, "sentient,text,visage",
			"Order a sentient being to say a message on the Voice channel."
			"\n\n"
			"Arguments:"
			"\n\t"
			"''visage'' is the mouth pose after speaking, may be None to leave it as is."),
	StoryFunc(F_SentientSayAudio, "void", "sentientSayAudio", UA_Dynamic, "sentient,file,text,visage",
			"Order a sentient to say a message on the Voice channel. The audio emits from the sentient."
			"\n\n"
			"Arguments:"
			"\n\t"
			"''visage'' is the mouth pose after speaking, may be None to leave it as is."),
	StoryFunc(F_SentientSayChoice, "void", "sentientSayChoice", UA_Dynamic, "sentient,choiceValue,text,visage",
			"Order a sentient being to say a choice on the Voice channel. "
			"A choice message appears with a bright green asterisk. "
			"\n\n"
			"Several choices may be stated if each is given a different choiceValue. "
			"\n\n"
			"Result of player's choice is tested using the choiceMade function. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''visage'' is the mouth pose after speaking, may be None to leave it as is."),
	StoryFunc(F_SentientSayImage, "void", "sentientSayImage", UA_Dynamic, "sentient,file,text,text,visage",
			"Order a sentient to say a message on the Voice channel. "
			"The image appears automatically."
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' args are image title and text to say. "
			"\n\t"
			"''visage'' is the mouth pose after speaking, may be None to leave it as is."),
	StoryFunc(F_SentientSayTo, "void", "sentientSayTo", UA_Dynamic, "sentient,atom,text,visage",
			"Order a sentient being to say a message on the Voice channel to someone. "
			"They will turn to face them."
			"\n\n"
			"Arguments:"
			"\n\t"
			"''visage'' is the mouth pose after speaking, may be None to leave it as is."),
	StoryFunc(F_SentientSellGearItem, "void", "sentientSellGearItem", UA_DynamicOnce, "sentient,atom,commodity,int",
			"Offers an item from a sentient's gear on the Voice channel. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''atom'' is the target of the sale, causing sentient to turn to face them; it can be null. "
			"\n\t"
			"The best Q ''commodity'' possessed by ''sentient'' is offered for sale. "
			"\n\t"
			"''int'' is the price, which can be 0 to offer it for free. "),
	StoryFunc(F_SentientStay, "void", "sentientStay", UA_DynamicOnce, "sentient",
			"Order a sentient to stay at their current location."),

	StoryFunc(F_ShipDocTitle, "#", "Ship Functions", UA_Const, "", "Ship functions operate on a specific spacecraft, which is the "
			"first argument of all ship functions. "),
	StoryFunc(F_ShipChargeCapacitor, "void", "shipChargeCapacitor", UA_DynamicOnce, "ship",
			"Fills the capacitor to 100% on a ship."),
	StoryFunc(F_ShipChargeShields, "void", "shipChargeShields", UA_DynamicOnce, "ship",
			"Fills the shields to 100% on a ship."),
	StoryFunc(F_ShipCreateCharacter, "character", "shipCreateCharacter", UA_DynamicOnce, "ship,int,text,gender,hand,empire,dna,stage",
			"Creates a character aboard ''ship''. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is placement strategy: 0=transporter void location, 1=crew or troop void location, 2=colonist void location. "
			"\n\t"
			"''text'' is the name of the new character. "
			"\n\t\t"
			"If a character with the same name and story instance exists in the environment, "
			"it is moved rather than create a new character. "
			"\n\t"
			"''dna'' specifies the DNA of the character. "
			"However, if the avatar of the story has ever met a story character with the same name, in any prior encounter, "
			"then the ''dna'' argument is overridden by the DNA of the original character met by the avatar. "),
	StoryFunc(F_ShipCreateCrew, "void", "shipCreateCrew", UA_DynamicOnce, "ship,dna",
			"Fills all empty crew positions on a ship. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''dna'' is used to create the new crew members."),
	StoryFunc(F_ShipCreateTroops, "void", "shipCreateTroops", UA_DynamicOnce, "ship,dna",
			"Fills all empty troop positions on a ship. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''dna'' is used to create the new troops."),
	StoryFunc(F_ShipFindAI, "sentient", "shipFindAI", UA_Const, "ship,npcrole",
			"Locates an AI controlled sentient aboard a ship. "
			"\n\n"
			"The returned sentient will be an NPC or a demiavatar who is offline and AI controlled. "
			"The sentient will not be at a station or post; use shipStationOccupant to find them. "),
	StoryFunc(F_ShipFindAIInRoom, "sentient", "shipFindAIInRoom", UA_Const, "ship,npcrole,text",
			"Locates an AI controlled sentient aboard a ship, in a specific room. "
			"\n\n"
			"The returned sentient will be an NPC or a demiavatar who is offline and AI controlled. "
			"The sentient will not be at a station or post; use shipStationOccupant to find them. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"text - Room name to find station occupant, as returned by unitRoomName. "),
	StoryFunc(F_ShipFindNpc, "npc", "shipFindNpc", UA_Const, "ship,npcrole",
			"Locates an npc aboard a ship. "
			"\n\n"
			"Does not find demiavatars. "),
	StoryFunc(F_ShipFindNpcInRoom, "npc", "shipFindNpcInRoom", UA_Const, "ship,npcrole,text",
			"Locates an npc aboard a ship, in a specific room. "
			"\n\n"
			"Does not find demiavatars. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"text - Room name to find station occupant, as returned by unitRoomName. "),
	StoryFunc(F_ShipInteriorLight, "void", "shipInteriorLight", UA_DynamicOnce, "ship,int",
			"Sets a ship's interior light state. 0=off, 1=on, 2=dim."),
	StoryFunc(F_ShipManeuverDrive, "void", "shipManeuverDrive", UA_DynamicOnce, "ship,int",
			"Sets a ship's maneuver drive state. 0=off, 1=on, 2=hold."),
	StoryFunc(F_ShipPowerPlant, "void", "shipPowerPlant", UA_DynamicOnce, "ship,int",
			"Sets a ship's power plant state. 0=off, 1=on."),
	StoryFunc(F_ShipRefuel, "void", "shipRefuel", UA_DynamicOnce, "ship,int",
			"Fills the fuel cells to 100% on a ship with ''int'' quality fuel."),
	StoryFunc(F_ShipRetreat, "void", "shipRetreat", UA_DynamicOnce, "ship",
			"Ship selects a random solar system and goes directly there at maximum speed, without using wormholes. "
			"This is an exit-the-scene maneuver."),
	StoryFunc(F_ShipShields, "void", "shipShields", UA_DynamicOnce, "ship,int",
			"Sets a ship's shield state. 0=off, 1=on."),
	StoryFunc(F_ShipStationOccupant, "creature", "shipStationOccupant", UA_Const, "ship,station",
			"Returns a creature who is controlling or occupying a station on the ship. "
			"\n\n"
			"When multiple redundant stations exist, the sentient currently controlling the underlying system is returned. e.g. "
			"when two helms exist the active pilot is returned. "
			"\n\n"
			"Unless otherwise noted here, the returned creature will be an NPC or a demiavatar who is offline and AI controlled. "
			"\n\n"
			"Command station prefers to return the AI officer with the con. If none is found, any AI officer is returned. "
			"\n\n"
			"Designer returns an avatar in a designer instance. "
			"\n\n"
			"Livestock returns an animal. "),
	StoryFunc(F_ShipStationOccupantInRoom, "creature", "shipStationOccupantInRoom", UA_Const, "ship,station,text",
			"Returns a creature who is controlling or occupying a station on the ship. "
			"\n\n"
			"When multiple redundant stations exist, the sentient currently controlling the underlying system is returned. e.g. "
			"when two helms exist the active pilot is returned. "
			"\n\n"
			"Unless otherwise noted here, the returned creature will be an NPC or a demiavatar who is offline and AI controlled. "
			"\n\n"
			"Command station prefers to return the AI officer with the con. If none is found, any AI officer is returned. "
			"\n\n"
			"Designer returns an avatar in a designer instance. "
			"\n\n"
			"Livestock returns an animal. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"text - Room name to find station occupant, as returned by unitRoomName. "),

	StoryFunc(F_UnitDocTitle, "#", "Unit Functions", UA_Const, "", "Unit functions operate on a specific unit, which is the first "
            "argument of all unit functions. A unit is any creature, vehicle, or spacecraft. "),
	StoryFunc(F_UnitAboardShip, "ship", "unitAboardShip", UA_Const, "unit",
			"Returns the ship that the unit is aboard, could be on the hull or hanging from a ladder."),
	StoryFunc(F_UnitAtBuilding, "bool", "unitAtBuilding", UA_Const, "unit,building",
			"Test for presence of a unit at a particular building site. Does not have to be inside the building."),
	StoryFunc(F_UnitAtWorld, "world", "unitAtWorld", UA_Const, "unit",
			"Returns the world that the unit is at. They could be inside or outside the atmosphere."),
	StoryFunc(F_UnitInOrbit, "bool", "unitInOrbit", UA_Const, "unit",
			"Test if the unit is in orbit at a planet, outside the atmosphere."),
	StoryFunc(F_UnitInsideShip, "ship", "unitInsideShip", UA_Const, "unit",
			"Returns the ship that the unit is inside. The unit must be in a room, not on the hull or hanging from a ladder."),
	StoryFunc(F_UnitRoomName, "text", "unitRoomName", UA_Const, "unit",
			"Returns the name of the room that the unit is inside. "
			"The room may be in a ship or in a building. "
			"Blank is returned when the unit is not inside a room. "),
	StoryFunc(F_UnitStationInTradeRange, "ship", "unitStationInTradeRange", UA_Const, "unit,stance,int",
			"Returns station that is within trade range of the unit. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''stance'' and ''int'' are used to check the station empire's stance toward the unit's empire. "
			"''int'' 0=stance must match, 1=stance must match or be better, 2=stance must match or be worse. "),

	StoryFunc(F_VehicleDocTitle, "#", "Vehicle Functions", UA_Const, "", "Vehicle functions operate on a specific vehicle, which "
			"is the first argument of all vehicle functions. "),
	StoryFunc(F_VehicleRefuel, "void", "vehicleRefuel", UA_DynamicOnce, "vehicle",
			"Fills the fuel cells to 100% on a vehicle. "),

	StoryFunc(F_WorldDocTitle, "#", "World Functions", UA_Const, "", "World functions operate on a specific world, which is the "
			"first argument of all world functions. "),
	StoryFunc(F_WorldAtmosphereBreathable, "bool", "worldAtmosphereBreathable", UA_Const, "world",
			"Test if the world atmosphere is breathable."),
	StoryFunc(F_WorldClaimed, "bool", "worldClaimed", UA_Const, "world",
			"Test if the world is claimed by any empire."),
	StoryFunc(F_WorldClaimedBy, "bool", "worldClaimedBy", UA_Const, "world,empire",
			"Test if the world is claimed by ''empire''."),
	StoryFunc(F_WorldCreateCharacter, "character", "worldCreateCharacter", UA_DynamicOnce, "world,ll,text,gender,hand,empire,dna,stage",
			"Creates a character on ''world'' at ''ll''. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''text'' is the name of the new character. "
			"\n\t"
			"If a character with the same name and story instance exists in the environment, "
			"it is moved rather than create a new character. "
			"\n\t"
			"''dna'' specifies the DNA of the character. "
			"However, if the avatar of the story has ever met a story character with the same name, in any prior encounter, "
			"then the ''dna'' argument is overridden by the DNA of the original character met by the avatar. "),
	StoryFunc(F_WorldCreateIndigenous, "void", "worldCreateIndigenous", UA_DynamicOnce, "world,ll,dna",
			"Create an indigenous village on a world. "
			"Establishes the indigenous DNA using ''dna'' if it is not already set."),
	StoryFunc(F_WorldCreateItem, "item", "worldCreateItem", UA_DynamicOnce,
			"world,ll,bool,commodity,quality,empire,text",
			"Create an item on a world. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''ll'' is the location to create the item. "
			"\n\t"
			"''bool'' is true to make the item permanent. "
			"Permanent story items persist in the game after the story ends. "
			"Nonpermanent story items are removed from the game when the story ends. "
			"\n\t"
			"''empire'' is the empire of origin of the item. "
			"\n\t"
			"''text'' is inscribed on the item if not blank."),
	StoryFunc(F_WorldCreateShip, "ship", "worldCreateShip", UA_DynamicOnce, "world,ll,real,sc,quality,empire,text",
			"Create a spaceship at a world. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''ll'' is the location to create the spaceship. When altitude AGL is 0, a 50m radius about ll must be clear of "
			"obstructions to succeed. "
			"\n\t"
			"''real'' is the altitude in meters AGL if alt<500; alt>500 puts the ship at atmosphere radius + alt. "
			"Standard orbit is about 2000m above the atmosphere."
			"Parking altitude is about 1500m above the atmosphere."
			"\n\t"
			"''sc'' is the blueprint id of the ship. "
			"\n\t"
			"''empire'' is the empire who owns the spaceship. "
			"\n\t"
			"''text'' is the name of the ship, set randomly if this is blank."),
	StoryFunc(F_WorldCreateVehicle, "vehicle", "worldCreateVehicle", UA_DynamicOnce,
			"world,ll,commodity,quality,empire",
			"Create a vehicle on a world. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''ll'' is the location to create the vehicle. A 5m radius about ll must be clear of obstructions to succeed. "
			"\n\t"
			"''empire'' is the empire who owns the vehicle."),
	StoryFunc(F_WorldHasCity, "bool", "worldHasCity", UA_Const, "world",
			"Test if the world has any cities at all."),
	StoryFunc(F_WorldHengeCeremony, "void", "worldHengeCeremony", UA_DynamicOnce, "world,ll,int",
			"Sets the state of the henge ceremony, if there is a henge. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''int'' is ceremony state: 1=Active, 0=inactive."),
	StoryFunc(F_WorldHydrographics, "int", "worldHydrographics", UA_Const, "world",
			"Returns the world's water percentage in the range 0 to 100."),
	StoryFunc(F_WorldIsGasGiant, "bool", "worldIsGasGiant", UA_Const, "world",
			"Test if the world is a gas giant."),
	StoryFunc(F_WorldIsMoon, "bool", "worldIsMoon", UA_Const, "world",
			"Test if the world is a moon. "
			"Moons are small globe satellites with no atmosphere."),
	StoryFunc(F_WorldIsPlanet, "bool", "worldIsPlanet", UA_Const, "world",
			"Test if the world is a planet. "
			"Planets are the primary worlds in a solar system, apart from gas giants."),
	StoryFunc(F_WorldIsPlanetoid, "bool", "worldIsPlanetoid", UA_Const, "world",
			"Test if the world is a planetoid. "
			"Planetoid is a tiny globe in an asteroid belt, orbiting a sun."),
	StoryFunc(F_WorldIsRing, "bool", "worldIsRing", UA_Const, "world",
			"Test if the world is a ring. "
			"A ring is a band of small rocks and ice, orbiting a planet."),
	StoryFunc(F_WorldIsRingworld, "bool", "worldIsRingworld", UA_Const, "world",
			"Test if the world is a ringworld arc section."),
	StoryFunc(F_WorldIsTitan, "bool", "worldIsTitan", UA_Const, "world",
			"Test if the world is a titan. "
			"Titans are large satellites with an atmosphere."),
	StoryFunc(F_WorldLandLocation, "ll", "worldLandLocation", UA_ConstOnce, "world,real,real",
			"Returns a location that is on land. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"First ''real'' is minimum altitude MSL, in meters. "
			"\n\t"
			"Second ''real'' is minimum radius from forbidden zones, in meters; 0 does not avoid forbidden zones."),
	StoryFunc(F_WorldLandLocationNear, "ll", "worldLandLocationNear", UA_ConstOnce, "world,ll,real,real,real",
			"Returns a location that is on land somewhere near ''ll''. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"First ''real'' is minimum altitude MSL, in meters. "
			"\n\t"
			"Second ''real'' is minimum distance from ''ll'', in meters. "
			"\n\t"
			"Third ''real'' is maximum distance from ''ll'', in meters."),
	StoryFunc(F_WorldOrbitFrigid, "bool", "worldOrbitFrigid", UA_Const, "world",
			"Test if the world is in the frigid orbit zone. Also true if the world is a satellite of a world in that zone."),
	StoryFunc(F_WorldOrbitHabitable, "bool", "worldOrbitHabitable", UA_Const, "world",
			"Test if the world is in the habitable orbit zone. Also true if the world is a satellite of a world in that zone."),
	StoryFunc(F_WorldOrbitInferno, "bool", "worldOrbitInferno", UA_Const, "world",
			"Test if the world is in the inferno orbit zone. Also true if the world is a satellite of a world in that zone."),
	StoryFunc(F_WorldOrbitInner, "bool", "worldOrbitInner", UA_Const, "world",
			"Test if the world is in the inner orbit zone. Also true if the world is a satellite of a world in that zone."),
	StoryFunc(F_WorldOrbitOuter, "bool", "worldOrbitOuter", UA_Const, "world",
			"Test if the world is in the outer orbit zone. Also true if the world is a satellite of a world in that zone."),
	StoryFunc(F_WorldRelativeLL, "ll", "worldRelativeLL", UA_Const, "world,ll,real,real",
			"Returns a location that is offset from another location. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''ll'' is the reference location. "
			"\n\t"
			"''real,real'' is horizontal, vertical offset, in meters."
			"\n\t\t"
			"Horizontal + is easterly and - is westerly on a globe. On a ringworld, + is east and - is west. "
			"\n\t\t"
			"Vertical + is north and - is south. "),
	StoryFunc(F_WorldScheduleAsteroid, "void", "worldScheduleAsteroid", UA_DynamicOnce, "world,int",
			"Schedules an asteroid to destroy the world in ''int'' minutes. "
			"\n\n"
			"Arguments:"
			"\n\t"
			"''world'' is the world that will be the target of the asteroid. "
			"\n\t"
			"''int'' is the number of minutes before impact. Must be at least 1."),
	StoryFunc(F_WorldSupportsAnimals, "bool", "worldSupportsAnimals", UA_Const, "world",
			"Test if the world can support animal life. "
			"World could be in the inner, habitable or outer orbit zone to support animals. "
			"\n\n"
			"All worlds that support animals also support plants. "),
	StoryFunc(F_WorldSupportsPlants, "bool", "worldSupportsPlants", UA_Const, "world",
			"Test if the world can support plant life. "
			"World could be in the inner, habitable or outer orbit zone to support plants. "
			"\n\n"
			"Worlds that support plants do not necessarily support animals. "),
};
unsigned int s_StoryFuncs = sizeof(s_StoryFunc)/sizeof(StoryFunc);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// 'lineText' is expected to be trimmed and have trailing semicolon removed.
AuScriptLine::AuScriptLine(const QString& blockName, const QString& lineText, bool isEval, const QString& scriptFile,
		int scriptLine, AuScript& script, QStringList& varDefs, QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns,
		QMap<int, AuScriptBranch>& branches)
{
	if(lineText.left(3) == "if(")
	{
		m_Func = F_If;
		m_Negate = false;

		// Tokenize each evaluation.
		int cpar = lineText.lastIndexOf(')');
		if(cpar < 0)
		{
			script.CompilerAppendOutput(QString("%1:line %2: error: expected ')' in '%3'").arg(scriptFile).arg(scriptLine).arg(lineText));
			return;
		}
		QString eval = lineText.mid(3,cpar-3).trimmed();
		if(eval.isEmpty())
		{
			script.CompilerAppendOutput(QString("%1:line %2: error: empty 'if' in '%3'").arg(scriptFile).arg(scriptLine).arg(lineText));
			return;
		}
		QString toLabel = lineText.mid(cpar+1).trimmed();
		if(toLabel.isEmpty())
		{
			script.CompilerAppendOutput(QString("%1:line %2: error: expected block label to follow branch condition '%3'")
					.arg(scriptFile).arg(scriptLine).arg(lineText));
			return;
		}
		else if(toLabel[0] == ':')
		{
			script.CompilerAppendOutput(QString("%1:line %2: error: unexpected ':' in branch condition block label '%3'")
					.arg(scriptFile).arg(scriptLine).arg(lineText));
			return;
		}
		while(!eval.isEmpty())
		{
			// Look for separating logical operator.
			SCRIPT_FUNC logop;
			QString subeval;
			int lop;
			if((lop = eval.indexOf(" &")) >= 0)
			{
				logop = F_AND;
				subeval = eval.left(lop).trimmed();
				eval = eval.mid(lop+2).trimmed();
			}
			else if((lop = eval.indexOf(" |")) >= 0)
			{
				logop = F_OR;
				subeval = eval.left(lop).trimmed();
				eval = eval.mid(lop+2).trimmed();
			}
			else if((lop = eval.indexOf(" ==")) >= 0)
			{
				logop = F_EQ;
				subeval = eval.left(lop).trimmed();
				eval = eval.mid(lop+3).trimmed();
			}
			else if((lop = eval.indexOf(" <=")) >= 0)
			{
				logop = F_LE;
				subeval = eval.left(lop).trimmed();
				eval = eval.mid(lop+3).trimmed();
			}
			else if((lop = eval.indexOf(" >=")) >= 0)
			{
				logop = F_GE;
				subeval = eval.left(lop).trimmed();
				eval = eval.mid(lop+3).trimmed();
			}
			else if((lop = eval.indexOf(" <")) >= 0)
			{
				logop = F_LT;
				subeval = eval.left(lop).trimmed();
				eval = eval.mid(lop+2).trimmed();
			}
			else if((lop = eval.indexOf(" >")) >= 0)
			{
				logop = F_GT;
				subeval = eval.left(lop).trimmed();
				eval = eval.mid(lop+2).trimmed();
			}
			else
				logop = F_NOP;

			if(logop != F_NOP)
			{
				// Found a logical operator.
				if(eval.isEmpty())
				{
					// Bad syntax, logical operator followed by nothing.
					script.CompilerAppendOutput(QString("%1:line %2: error: trailing logical operator in '%3'").arg(scriptFile).arg(scriptLine)
						.arg(lineText));
					break;
				}

				// Tokenize the evaluator we have extracted.
				AuScriptLine subLine(blockName, subeval, true, scriptFile, scriptLine, script, varDefs, bdDesigns, scDesigns,
						branches);
				if(subLine.Function() == F_NOP)
				{
					// Bad evaluator.
					m_Func = F_NOP;
					script.CompilerAppendOutput(QString("%1:line %2: error: bad evaluator '%3' in '%4'").arg(scriptFile).arg(scriptLine).arg(subeval)
						.arg(lineText));
					break;
				}

				// Append the tokenized evaluator and the logical op.
				m_Parms.append(QVariant::fromValue(subLine));
				m_Parms.append(QVariant::fromValue(AuScriptLine(logop)));
			}
			else
			{
				// No logical operator found.
				// Tokenize the trailing or perhaps only evaluator.
				AuScriptLine subLine(blockName, eval, true, scriptFile, scriptLine, script, varDefs, bdDesigns, scDesigns,
						branches);
				if(subLine.Function() == F_NOP)
				{
					// Bad evaluator.
					m_Func = F_NOP;
					script.CompilerAppendOutput(QString("%1:line %2: error: bad evaluator '%3' in '%4'")
							.arg(scriptFile).arg(scriptLine).arg(eval).arg(lineText));
					break;
				}
				eval.clear();

				// Append the tokenized evaluator.
				m_Parms.append(QVariant::fromValue(subLine));
			}
		}
		if(m_Func == F_NOP)
			return;	// Error message already output.

		// Append the branch goto label.
		m_Parms.append(QVariant::fromValue(toLabel));

		// Remember branches.
		// We'll build the error message while we have all the info. It may not be needed.
		AuScriptBranch branch(blockName, scriptLine, lineText, toLabel);
		branches.insert(scriptLine, branch);
	}
	else if(lineText.left(4) == "goto")
	{
		m_Func = F_GoTo;
		m_Negate = false;

		QString toLabel = lineText.mid(4).trimmed();
		if(toLabel.isEmpty())
		{
			script.CompilerAppendOutput(QString("%1:line %2: error: expected block label to follow goto '%3'").arg(scriptFile).arg(scriptLine)
				.arg(lineText));
			return;
		}
		else if(toLabel[0] == ':')
		{
			script.CompilerAppendOutput(QString("%1:line %2: error: unexpected ':' in goto block label '%3'").arg(scriptFile).arg(scriptLine)
				.arg(lineText));
			return;
		}

		// Append the branch goto label.
		m_Parms.append(QVariant::fromValue(toLabel));

		// Remember branches.
		// We'll build the error message while we have all the info. It may not be needed.
		AuScriptBranch branch(blockName, scriptLine, lineText, toLabel);
		branches.insert(scriptLine, branch);
	}
	else
	{
		Compile(lineText, isEval, scriptFile, scriptLine, script, varDefs, bdDesigns, scDesigns);
	}
}

AuScriptLine::AuScriptLine(SCRIPT_FUNC func)
{
	m_Negate = false;
	m_Func = func;
}

AuScriptLine::AuScriptLine()
{
	m_Negate = false;
	m_Func = F_NOP;
}

AuScriptLine::~AuScriptLine()
{
}

void AuScriptLine::Compile(const QString& lineText, bool isEval, const QString& scriptFile, int scriptLine, AuScript& script,
		QStringList& varDefs, QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns)
{
	QString funcCall(lineText);

	// Look for an assignment operator that is not inside of quotes.
	bool inQuotes = false;
	for(int idx = 0; idx < funcCall.length(); idx++)
	{
		if(funcCall[idx] == '"')
			inQuotes = !inQuotes;
		else if(!inQuotes && funcCall[idx] == '=')
		{
			// Found an assignment.
			m_Assignee = funcCall.left(idx).trimmed();
			if(m_Assignee.isEmpty())
			{
				QString sError = QString("%1:line %2: error: bad '=' syntax in '%3'")
					.arg(scriptFile).arg(scriptLine).arg(funcCall);
				script.CompilerAppendOutput(sError);
				m_Func = F_NOP;
				return;	// Bad syntax
			}
			funcCall = funcCall.mid(idx+1).trimmed();
			if(!varDefs.contains(m_Assignee))
				varDefs.append(m_Assignee);	// A new variable was defined.
			break;
		}
	}

	// Strip leading NOT '!' unary operators.
	m_Negate = false;
	while(funcCall[0] == '!')
	{
		m_Negate = !m_Negate;
		funcCall = funcCall.mid(1).trimmed();
	}

	// NOT operator doesn't make sense with assignee.
	if(m_Negate && !m_Assignee.isEmpty())
	{
		QString sError = QString("%1:line %2: error: unexpected '!' operator in '%3'").arg(scriptFile).arg(scriptLine).arg(funcCall);
		script.CompilerAppendOutput(sError);
		m_Func = F_NOP;
		return;	// Bad NOT operator
	}

	// Look for opening and closing parentheses.
	int opar = funcCall.indexOf('(');
	int cpar = funcCall.indexOf(')');
	if(opar <= 0 && cpar <= 0)
	{
		// Check for a numeric constant.
		bool ok;
		funcCall.toDouble(&ok);
		if(ok)
		{
			m_Func = F_CONST;							// This looks like a numeric constant.
			m_Parms.append(QVariant::fromValue(funcCall));
			return;
		}
		if(varDefs.contains(funcCall))
		{
			m_Func = F_VAR;								// This looks like a numeric constant.
			m_Parms.append(funcCall);					// The parm is the variable name.
			return;
		}
	}

	if(opar <= 0 || cpar <= 0 || cpar <= opar)
	{
		QString sError = QString("%1:line %2: error: bad syntax in '%3'").arg(scriptFile).arg(scriptLine).arg(funcCall);
		script.CompilerAppendOutput(sError);
		m_Func = F_NOP;
		return;	// Bad format
	}
	QString func = funcCall.left(opar);	// Function name
	cpar = funcCall.lastIndexOf(')');
	int plen = cpar - opar - 1;		// Length of parameters string.
	QString parms;					// Parameters string
	if(plen)
		parms = funcCall.mid(opar+1, plen);

	// Split parameters in script into strings.
	QStringList splitParms = SplitFields(parms);

	// Tokenize the function name and its parameters.
	m_Func = F_NOP;
	int parmi = -1;	// Index of script parameter being parsed.
	for(unsigned int fidx = 0; fidx<s_StoryFuncs; fidx++)
	{
		if(s_StoryFunc[fidx].m_Name != func)
			continue;

		// Found the function in the function table.
		if(s_StoryFunc[fidx].m_ReturnParm == "void")
		{
			// Eval requires every function to return something.
			if(isEval)
			{
				QString sError = QString("%1:line %2: error: void function cannot be used in 'if' evaluation: '%3'")
					.arg(scriptFile).arg(scriptLine).arg(funcCall);
				script.CompilerAppendOutput(sError);
				m_Func = F_NOP;
				return;
			}

			// Void result cannot be assigned. It must be ignored.
			if(!m_Assignee.isEmpty())
			{
				QString sError = QString("%1:line %2: error: void return not ignored like it should be: '%3'")
						.arg(scriptFile).arg(scriptLine).arg(funcCall);
				script.CompilerAppendOutput(sError);
				m_Func = F_NOP;
				return;
			}
		}
		else if(s_StoryFunc[fidx].m_ReturnParm == "choiceValue")
		{
			if(!m_Assignee.isEmpty())
			{
				QString sError = QString("%1:line %2: error: 'choiceValue' return cannot be assigned: '%3'")
						.arg(scriptFile).arg(scriptLine).arg(funcCall);
				script.CompilerAppendOutput(sError);
				m_Func = F_NOP;
				return;
			}
		}

		// Validate the parameters.
		QStringList splitFuncParms;
		if(!s_StoryFunc[fidx].m_Parms.isEmpty())
			splitFuncParms = s_StoryFunc[fidx].m_Parms.split(",");
		if(splitParms.count() != splitFuncParms.count())
		{
			QString sError = QString("%1:line %2: error: wrong number of parameters, found %3 expected %4: '%5'")
				.arg(scriptFile).arg(scriptLine).arg(splitParms.count()).arg(splitFuncParms.count()).arg(funcCall);
			script.CompilerAppendOutput(sError);
			m_Func = F_NOP;
			return;	// Wrong number of parameters.
		}

		// Get function opcode from table.
		m_Func = s_StoryFunc[fidx].m_Func;

		// Insure the s_StoryFunc table of data is indexable by func enum value.
		if(m_Func != (SCRIPT_FUNC)fidx)
		{
			QString sError = QString("%1:line %2: error: internal error, function: '%3' index: %4 token: %5. Compiler is broken!")
				.arg(scriptFile).arg(scriptLine).arg(s_StoryFunc[fidx].m_Name).arg(fidx).arg(m_Func);
			script.CompilerAppendOutput(sError);
			m_Func = F_NOP;
			return;
		}

		parmi = 0;
		QStringList::const_iterator fp = splitFuncParms.cbegin();
		QStringList::const_iterator pp = splitParms.cbegin();
		for(;fp != splitFuncParms.cend() && pp != splitParms.constEnd(); fp++, pp++, parmi++)
		{
			if((*fp) == "bool")
			{
				if((*pp).compare("true", Qt::CaseInsensitive) == 0)
					m_Parms.append(QVariant(true));	// The parameter value is the bool.
				else if((*pp).compare("false", Qt::CaseInsensitive) == 0)
					m_Parms.append(QVariant(false));	// The parameter value is the bool.
				else
				{
					QString sError = QString("%1:line %2: error: invalid boolean value in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
			}
			else if((*fp) == "animal" ||	// expecting a DaAnimal id variable
					(*fp) == "atom" ||		// expecting a DaAtom id variable
					(*fp) == "avatar" ||	// expecting a DaPerson id variable
					(*fp) == "character" ||	// expecting a DaNpc id variable
					(*fp) == "city" ||		// expecting a DaBd id variable
					(*fp) == "creature" ||	// expecting a DaCreature id variable
					(*fp) == "item" ||		// expecting a DaItem id variable
					(*fp) == "ll" ||		// expecting a AuGlobeLLF variable
					(*fp) == "npc" ||		// expecting a DaNpc id variable
					(*fp) == "sector" ||	// expecting a DaSector id variable
					(*fp) == "sentient" ||	// expecting a DaSentient id variable
					(*fp) == "ship" ||		// expecting a DaVessel id variable
					(*fp) == "system" ||	// expecting a DaSolarSystem variable
					(*fp) == "unit" ||		// expecting a DaUnit id variable
					(*fp) == "vehicle" ||	// expecting a DaVehicle id variable
					(*fp) == "world" ||		// expecting a DaWorld id variable
					(*fp) == "worldcat")	// expecting a DaWorld CatalogName variable
			{
				// Check to see if the variable has been defined.
				if(!varDefs.contains(*pp))
				{
					QString sError = QString("%1:line %2: error: undefined variable '%5' in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall).arg(*pp);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(*pp));		// The parameter value is the variable name. It does not need interpreting.
			}
			else if((*fp) == "stance")
			{
				// Stance name, not case sensitive.
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in stance name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				POLITICAL_STANCE stance;
				if((*pp).compare("Friend", Qt::CaseInsensitive) == 0)
					stance = POL_Friend;
				else if((*pp).compare("Neutral", Qt::CaseInsensitive) == 0)
					stance = POL_Neutral;
				else if((*pp).compare("Enemy", Qt::CaseInsensitive) == 0)
					stance = POL_Enemy;
				else if((*pp).compare("Empire", Qt::CaseInsensitive) == 0)
					stance = POL_Empire;
				else if((*pp).compare("Vassal", Qt::CaseInsensitive) == 0)
					stance = POL_Vassal;
				else
				{
					QString sError = QString("%1:line %2: error: invalid stance name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(stance));	// The parameter value is the stance enum.
			}
			else if((*fp) == "station")
			{
				// Station name, not case sensitive.
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in station name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				AuStation::TYPE station;
				if((*pp).compare("Command", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Command;
				else if((*pp).compare("Designer", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Designer;
				else if((*pp).compare("Engine", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Engine;
				else if((*pp).compare("FireControl", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_FireControl;
				else if((*pp).compare("Helm", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Helm;
				else if((*pp).compare("Medic", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Medic;
				else if((*pp).compare("Navigation", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Navigation;
				else if((*pp).compare("Patient", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Patient;
				else if((*pp).compare("Power", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Power;
				else if((*pp).compare("Sensor", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Sensor;
				else if((*pp).compare("Shield", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Shield;
				else if((*pp).compare("Transporter", Qt::CaseInsensitive) == 0)
					station = AuStation::Integrated_Transporter;
				else if((*pp).compare("Turret", Qt::CaseInsensitive) == 0)
					station = AuStation::Turret_Integrated;
				else if((*pp).compare("Game", Qt::CaseInsensitive) == 0)
					station = AuStation::Table_Game;
				else if((*pp).compare("Troop", Qt::CaseInsensitive) == 0)
					station = AuStation::Troop;
				else if((*pp).compare("Citizen", Qt::CaseInsensitive) == 0)
					station = AuStation::Citizen;
				else if((*pp).compare("Livestock", Qt::CaseInsensitive) == 0)
					station = AuStation::Livestock;
				else
				{
					QString sError = QString("%1:line %2: error: invalid station name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(station));	// The parameter value is the station enum.
			}
			else if((*fp) == "angle")
			{
				// Angle value.
				int angle = QVariant(*pp).toInt();
				if(angle < -360 || angle > 360)
				{
					QString sError = QString("%1:line %2: error: invalid angle value in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(angle));		// The parameter value is the angle.
			}
			else if((*fp) == "building")
			{
				// Building name, not case sensitive.
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in building name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				const DgData& gd = g_Au.LockDataForRead();
				INDUSTRYID iid = gd.IndustryId(*pp);
				g_Au.UnlockData();
				if(iid == IID_None)
				{
					QString sError = QString("%1:line %2: error: invalid industry name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(iid));	// The parameter value is the industry id
			}
			else if((*fp) == "commodity")
			{
				// Commodity name, not case sensitive.
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in commodity name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				COMMODITYID cid = g_Au.CommodityId(*pp);
				if(cid == CID_Invalid)
				{
					QString sError = QString("%1:line %2: error: invalid commodity name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(cid));	// The parameter value is the commodity id
			}
			else if((*fp) == "dna")
			{
				// A DNA specification.
				QString dnaspec(*pp);
				AuSpecies species;
				if((*pp).compare("Avatar", Qt::CaseInsensitive) != 0 &&
						(*pp).compare("Indigenous", Qt::CaseInsensitive) != 0 &&
						(*pp).compare("Worker", Qt::CaseInsensitive) != 0)
				{
					// Not a "avatar" or "worker" dna.
					// Validate custom DNA string.
					if((*pp).right(4).compare(".chr", Qt::CaseInsensitive) == 0)
					{
						// A file name in the story's data directory.
						QFileInfo fileInfo(scriptFile);
						QString fname = fileInfo.path() + "/";
						fname += (*pp);
						DhDNA dna;
						if(!DNALoadFile(fname, dna))
						{
							QString sError = QString("%1:line %2: error: DNA file could not be loaded in parameter %3: '%4'")
								.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
							script.CompilerAppendOutput(sError);
							m_Func = F_NOP;
							return;
						}

						species = dna.Species();
					}
					else if((*pp).right(4).compare(".dna", Qt::CaseInsensitive) == 0)
					{
						// A file name in the story's data directory.
						QFileInfo fileInfo(scriptFile);
						QString fname = fileInfo.path() + "/";
						fname += (*pp);
						AuDNA dna;
						if(!DNALoadFile(fname, dna))
						{
							QString sError = QString("%1:line %2: error: DNA file could not be loaded in parameter %3: '%4'")
								.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
							script.CompilerAppendOutput(sError);
							m_Func = F_NOP;
							return;
						}

						species = dna.Species();
					}
				}
				if(!species.IsNull())
					m_Parms.append(QVariant::fromValue<AuSpecies>(species));	// The parameter value is the species specification.
				else
					m_Parms.append(QVariant(dnaspec));	// The parameter value is the dna specification.
			}
			else if((*fp) == "file")
			{
				// A file name in the story's data directory.
				QFileInfo fileInfo(scriptFile);
				QString fname = fileInfo.path() + "/";
				fname += (*pp);

				QFileInfo finfo(fname);
				if(!finfo.exists())
				{
					QString sError = QString("%1:line %2: error: file '%3' not found: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(fname).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				QByteArray fileData;
				QFile file(fname);
				if(file.open(QIODevice::ReadOnly))
				{
					// Successfully opened the file. Read the audio data.
					fileData = file.readAll();
					file.close();
				}
				m_Parms.append(QVariant(fileData));	// The parameter value is the file data.
			}
			else if((*fp) == "hand")
			{
				// A dominant hand specification.
				int handedness;
				if((*pp).compare("Left Handed", Qt::CaseInsensitive) == 0)
					handedness = LeftHanded;
				else if((*pp).compare("Right Handed", Qt::CaseInsensitive) == 0)
					handedness = RightHanded;
				else
				{
					QString sError = QString("%1:line %2: error: invalid handedness specification in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;

				}
				m_Parms.append(QVariant(handedness));	// The parameter value is the handedness value.
			}
			else if((*fp) == "gender")
			{
				// A gender specification.
				int gender;
				if((*pp).compare("Male", Qt::CaseInsensitive) == 0)
					gender = Male;
				else if((*pp).compare("Female", Qt::CaseInsensitive) == 0)
					gender = Female;
				else if((*pp).compare("Asexual", Qt::CaseInsensitive) == 0)
					gender = Asexual;
				else if((*pp).compare("Hermaphrodite", Qt::CaseInsensitive) == 0)
					gender = Hermaphrodite;
				else
				{
					QString sError = QString("%1:line %2: error: invalid gender specification in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;

				}
				m_Parms.append(QVariant(gender));	// The parameter value is the gender value.
			}
			else if((*fp) == "npcrole")
			{
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in npcrole name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				int npcrole;
				if((*pp).compare("Citizen", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_Citizen;
				else if((*pp).compare("IndigenousPerson", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_IndigenousPerson;
				else if((*pp).compare("IndigenousElder", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_IndigenousElder;
				else if((*pp).compare("IndigenousHealer", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_IndigenousHealer;
				else if((*pp).compare("IndigenousStargazer", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_IndigenousStargazer;
				else if((*pp).compare("Worker", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_Worker;
				else if((*pp).compare("Passenger", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_Passenger;
				else if((*pp).compare("Crew", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_Crew;
				else if((*pp).compare("Troop", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_Troop;
				else if((*pp).compare("Officer", Qt::CaseInsensitive) == 0)
					npcrole = ROLE_Officer;
				else
				{
					QString sError = QString("%1:line %2: error: invalid npcrole specification in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;

				}
				m_Parms.append(QVariant(npcrole));	// The parameter value is the npcrole value.
			}
			else if((*fp) == "stage")
			{
				// A growth stage specification.
				int stage;
				if((*pp).compare("Mature", Qt::CaseInsensitive) == 0)
					stage = Mature;
				else if((*pp).compare("Juvenile", Qt::CaseInsensitive) == 0)
					stage = Juvenile;
				else
				{
					QString sError = QString("%1:line %2: error: invalid growth stage specification in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;

				}
				m_Parms.append(QVariant(stage));	// The parameter value is the stage value.
			}
			else if((*fp) == "channel")
			{
				// Communication channel name
				// Drop quotes, if any.
				QString s;
				int slen = (*pp).length();
				if(slen > 2 && (*pp)[0] == '"' && (*pp)[slen-1] == '"')
					s = (*pp).mid(1,slen-2);
				else
					s = *pp;

				if(s.contains('"'))
				{
					QString sError =
						QString("%1:line %2: error: unexpected '\"' in communications channel name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				// Check for field variables. Error on undefined variables.
				if(!ValidateStringParameter(s, varDefs))
				{
					QString sError = QString("%1:line %2: error: undefined field variable in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				m_Parms.append(QVariant(s));		// The parameter value is the channel name.
			}
			else if((*fp) == "visage")
			{
				int visage;
				if((*pp).compare("None", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Invalid;
				else if((*pp).compare("Agape", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Agape;
				else if((*pp).compare("Closed", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Closed;
				else if((*pp).compare("Cry2Sec", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Cry2Sec;
				else if((*pp).compare("Frown", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Frown;
				else if((*pp).compare("Grim", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Grim;
				else if((*pp).compare("Grin", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Grin;
				else if((*pp).compare("Laugh1Sec", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Laugh1Sec;
				else if((*pp).compare("Laugh2Sec", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Laugh2Sec;
				else if((*pp).compare("Laugh4Sec", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Laugh4Sec;
				else if((*pp).compare("Open", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Open;
				else if((*pp).compare("OpenWide", Qt::CaseInsensitive) == 0)
					visage = AuVisage::OpenWide;
				else if((*pp).compare("Perplexed", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Perplexed;
				else if((*pp).compare("Pursed", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Pursed;
				else if((*pp).compare("Sad", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Sad;
				else if((*pp).compare("Sardonic", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Sardonic;
				else if((*pp).compare("Smile", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Smile;
				else if((*pp).compare("Yawn", Qt::CaseInsensitive) == 0)
					visage = AuVisage::Yawn;
				else
				{
					QString sError = QString("%1:line %2: error: invalid visage specification in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;

				}
				m_Parms.append(QVariant(visage));		// The parameter value is the AuVisage::VISAGE enum
			}
			else if((*fp) == "scenario")
			{
				// Find the scenario by name, not case sensitive.
				int scenario = -1;
				for(int s=0; s<SCENARIO_Count; s++)
				{
					if((*pp).compare(ScenarioName((SCENARIO)s), Qt::CaseInsensitive) == 0)
					{
						scenario = (SCENARIO)s;
						break;
					}
				}

				if(scenario < 0)
				{
					QString sError = QString("%1:line %2: error: invalid orbit specification in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;

				}
				m_Parms.append(QVariant(scenario));		// The parameter value is the SCENARIO enum.
			}
			else if((*fp) == "orbit")
			{
				int orbit;
				if((*pp).compare("Inferno", Qt::CaseInsensitive) == 0)
					orbit = OZONE_Inferno;
				else if((*pp).compare("Inner", Qt::CaseInsensitive) == 0)
					orbit = OZONE_Inner;
				else if((*pp).compare("Habitable", Qt::CaseInsensitive) == 0)
					orbit = OZONE_Habitable;
				else if((*pp).compare("Outer", Qt::CaseInsensitive) == 0)
					orbit = OZONE_Outer;
				else if((*pp).compare("Frigid", Qt::CaseInsensitive) == 0)
					orbit = OZONE_Frigid;
				else if((*pp).compare("Any", Qt::CaseInsensitive) == 0)
					orbit = 10;
				else if((*pp).compare("Harsh", Qt::CaseInsensitive) == 0)
					orbit = 11;
				else if((*pp).compare("Mild", Qt::CaseInsensitive) == 0)
					orbit = 12;
				else if((*pp).compare("Uninhabitable", Qt::CaseInsensitive) == 0)
					orbit = 13;
				else
				{
					QString sError = QString("%1:line %2: error: invalid orbit specification in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;

				}
				m_Parms.append(QVariant(orbit));		// The parameter value is the OZONE_ enum or 10-13.
			}
			else if((*fp) == "empire")
			{
				// Empire name or stance specification.
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in empire name in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				int empire;
				if((*pp).compare("Empire", Qt::CaseInsensitive) == 0)
					empire = 0;
				else if((*pp).compare("Vassal", Qt::CaseInsensitive) == 0)
					empire = -1;
				else if((*pp).compare("Friend", Qt::CaseInsensitive) == 0)
					empire = -2;
				else if((*pp).compare("Neutral", Qt::CaseInsensitive) == 0)
					empire = -3;
				else if((*pp).compare("Enemy", Qt::CaseInsensitive) == 0)
					empire = -4;
				else if((*pp).compare("None", Qt::CaseInsensitive) == 0)
					empire = -5;
				else
				{
					const DgData& gd = g_Au.LockDataForRead();
					empire = gd.EmpireId(*pp);
					g_Au.UnlockData();
					if(!empire)
					{
						QString sError = QString("%1:line %2: error: invalid empire name in parameter %3: '%4'")
							.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
						script.CompilerAppendOutput(sError);
						m_Func = F_NOP;
						return;
					}
				}
				m_Parms.append(QVariant(empire));		// The parameter value is the empire id or (-4 to 0)
			}
			else if((*fp) == "bd")
			{
				// Bd blueprint id.
				quint32 id = QVariant(*pp).toUInt();
				if(!id)
				{
					QString sError = QString("%1:line %2: error: invalid bd blueprint id in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				bdDesigns.insert(id);
				m_Parms.append(QVariant(*pp));		// The parameter value is the integer number.
			}
			else if((*fp) == "sc")
			{
				// Sc blueprint id.
				quint32 id = QVariant(*pp).toUInt();
				if(!id)
				{
					QString sError = QString("%1:line %2: error: invalid sc blueprint id in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				scDesigns.insert(id);
				m_Parms.append(QVariant(*pp));		// The parameter value is the integer number.
			}
			else if((*fp) == "int" ||
				(*fp) == "real")
			{
				// Integer value.
				m_Parms.append(QVariant(*pp));		// The parameter value is the integer number.
			}
			else if((*fp) == "choiceValue")
			{
				// Integer value.
				int choiceValue = QVariant(*pp).toInt();
				if(choiceValue < -127 || choiceValue > 127)
				{
					QString sError = QString("%1:line %2: error: invalid choice value in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(choiceValue));		// The parameter value is the choice value.
			}
			else if((*fp) == "quality")
			{
				// Integer value.
				int quality = QVariant(*pp).toInt();
				if(quality < 1 || quality > 255)
				{
					QString sError = QString("%1:line %2: error: invalid quality value in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(quality));		// The parameter value is the quality value.
			}
			else if((*fp) == "range")
			{
				// Communication range.
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in communication range in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				AuCommStdChannel::RANGE range = AuCommStdChannel::InvalidRange;
				if((*pp).compare("Close", Qt::CaseInsensitive) == 0)
				{
					range = AuCommStdChannel::Close;
				}
				else if((*pp).compare("Hail", Qt::CaseInsensitive) == 0)
				{
					range = AuCommStdChannel::Hail;
				}
				else if((*pp).compare("System", Qt::CaseInsensitive) == 0)
				{
					range = AuCommStdChannel::SolarSystem;
				}
				else if((*pp).compare("Sector", Qt::CaseInsensitive) == 0)
				{
					range = AuCommStdChannel::Sector;
				}
				else if((*pp).compare("Galaxy", Qt::CaseInsensitive) == 0)
				{
					range = AuCommStdChannel::Galaxy;
				}
				else if((*pp).compare("Universe", Qt::CaseInsensitive) == 0)
				{
					range = AuCommStdChannel::Universe;
				}
				else
				{
					QString sError = QString("%1:line %2: error: invalid comm range in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				m_Parms.append(QVariant(range));	// The parameter value is the range enum value.
			}
			else if((*fp) == "text")
			{
				// Text string delimited with " quotes.
				int slen = (*pp).length();
				if(slen < 2 || (*pp)[0] != '"' || (*pp)[slen-1] != '"')
				{
					QString sError = QString("%1:line %2: error: badly formed string in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				// Check for field variables. Error on undefined variables.
				if(!ValidateStringParameter(*pp, varDefs))
				{
					QString sError = QString("%1:line %2: error: undefined field variable in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}

				// Drop the quote marks and convert nulines to real nulines.
				QString s = (*pp).mid(1,slen-2);
				s.replace(QString("\\n"), QChar('\n'));	// \n becomes a real newline.
				s.replace(QString("\\\""), QChar('"'));	// \" becomes a "
				m_Parms.append(QVariant(s));	// The parameter value is the string.
			}
			else if((*fp) == "gearslot")
			{
				// Gear slot
				if((*pp).contains('"'))
				{
					QString sError = QString("%1:line %2: error: unexpected '\"' in gear slot in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
				AuGearSlot::SLOTTYPE slotType;
				if((*pp).compare("Hand", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Hand;
				else if((*pp).compare("Head", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Head;
				else if((*pp).compare("Face", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Face;
				else if((*pp).compare("Neck", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Neck;
				else if((*pp).compare("Body", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Body;
				else if((*pp).compare("Wear", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::BodyWear;
				else if((*pp).compare("Limb", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Limb;
				else if((*pp).compare("Digit", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Digit;
				else if((*pp).compare("Waist", Qt::CaseInsensitive) == 0)
					slotType = AuGearSlot::Waist;
				else
					slotType = AuGearSlot::Invalid;

				if(slotType != AuGearSlot::Invalid)
					m_Parms.append(QVariant(slotType));	// The parameter value is the SLOTTYPE enum value.
				else
				{
					QString sError = QString("%1:line %2: error: invalid gear slot in parameter %3: '%4'")
						.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
					script.CompilerAppendOutput(sError);
					m_Func = F_NOP;
					return;
				}
			}
			else
			{
				QString sError = QString("%1:line %2: error: invalid type specified for parameter %3: '%4'")
					.arg(scriptFile).arg(scriptLine).arg(parmi+1).arg(funcCall);
				script.CompilerAppendOutput(sError);
				//Q_ASSERT(false);	// This is a programming error.
				m_Func = F_NOP;
				return;
			}
		}

		// No need to keep looking. Pass or fail, this was the function.
		break;
	}

	if(m_Func == F_NOP)
	{
		// Did not get as far as parsing any parameters.
		QString sError = QString("%1:line %2: error: unknown function: '%3'").arg(scriptFile).arg(scriptLine).arg(funcCall);
		script.CompilerAppendOutput(sError);
	}
}

QStringList AuScriptLine::SplitFields(const QString& parms) const
{
	QStringList fields;
	if(!parms.isEmpty())
	{
		int fldstart = 0;
		bool inQuote = false;
		for(int ic=0; ic<parms.length(); ic++)
		{
			// Ignore field delimiter when we're inside a quoted string.
			if(parms[ic] == ',' && !inQuote)
			{
				// End of field found.
				if(fldstart == ic)
				{
					// Empty field.
					fields.append("");
				}
				else
				{
					// Field contains data.
					QString fld = parms.mid(fldstart, ic-fldstart).trimmed();
					fields.append(fld);
				}
				fldstart = ic+1;	// Next field starts after the comma.
			}
			else if(parms[ic] == '"')
			{
				// Enter/exit quoted string.
				inQuote = !inQuote;
			}
		}
		if(fldstart == parms.length())
		{
			// Last field is empty.
			fields.append("");
		}
		else
		{
			// Last field is not empty.
			QString fld = parms.mid(fldstart).trimmed();
			fields.append(fld);
		}
	}
	return fields;
}

// Send a comm message from an atom.
void AuScriptLine::AtomSendComm(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, const QString& channel,
		AuCommStdChannel::RANGE range, const AuId& fromId) const
{
	// Locate the character who sent the message.
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(!env)
	{
		qWarning() << "error: AuScriptLine::AtomSendComm person" << sceneAtom.Id() << sceneAtom.Name() << "msg" << messageText
			<< "no environment";
		return;
	}

	DaAtom* atom = env->FindIdRecursive(fromId);
	if(!atom)
	{
		qWarning() << "error: AuScriptLine::AtomSendComm person" << sceneAtom.Id() << sceneAtom.Name() << "msg" << messageText
			<< "from atom" << fromId << "not found";
		return;
	}

	// Found the atom in the scene atom's environment.
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	const DaSector* sector = env->FindParentSector();
	if(sector)
	{
		sectorId = sector->Id();
		galaxy = sector->Galaxy();
	}

	// Use narrator scope so everyone else does not hear the dialog, only the story participant(s).
	AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(channel);
	if(scope == AuCommStdChannel::Everyone || scope == AuCommStdChannel::Audible)
		scope = AuCommStdChannel::Narrator;

	AuCommChatStory msg(messageText, storyInstance, channel, fromId, atom->Name(), sectorId, galaxy, range, scope);

	if(scope == AuCommStdChannel::CrewMates)
	{
		const DaNpc* sent = dynamic_cast<const DaNpc*>(atom);
		if(sent)
			msg.SetScopeData(sent->AssignedShip().GetID());
	}
	else
		msg.SetScopeData(atom->EmpireId());

	g_Au.SendAtomChat(msg);

	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScriptLine::AtomSendComm %1 atom %2 %3 msg '%4' range %5 scope %6")
			.arg(storyInstance)
			.arg(atom->Id().GetID())
			.arg(atom->Name())
			.arg(messageText)
			.arg(AuCommStdChannel::RangeName(range))
			.arg(AuCommStdChannel::ScopeName(scope));
		g_Au.Log(logmsg);
	}
}

void AuScriptLine::AtomSendCommChoice(DaPerson& person, quint32 storyInstance, const QString& messageText, int choiceValue,
		const QString& channel, AuCommStdChannel::RANGE range, const AuId& fromId) const
{
	// Locate the character who sent the message.
	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return;

	DaAtom* atom = env->FindIdRecursive(fromId);
	if(!atom)
	{
		qWarning() << "error: AuScriptLine::AtomSendCommChoice person" << person.Id() << person.Name() << "msg" << messageText
			<< "from atom" << fromId << "not found";
		return;
	}

	// Found the atom in the scene atom's environment.
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	const DaSector* sector = env->FindParentSector();
	if(sector)
	{
		sectorId = sector->Id();
		galaxy = sector->Galaxy();
	}

	// Use narrator scope so everyone else does not hear the dialog, only the story participant(s).
	AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(channel);
	if(scope == AuCommStdChannel::Everyone || scope == AuCommStdChannel::Audible)
		scope = AuCommStdChannel::Narrator;

	AuCommChatChoice msg(person.Id(), messageText, storyInstance, choiceValue, channel, fromId, atom->Name(), sectorId, galaxy,
			range, scope);

	if(scope == AuCommStdChannel::CrewMates)
	{
		const DaNpc* sent = dynamic_cast<const DaNpc*>(atom);
		if(sent)
			msg.SetScopeData(sent->AssignedShip().GetID());
	}
	else
		msg.SetScopeData(atom->EmpireId());

	g_Au.SendAtomChat(msg);

	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScriptLine::AtomSendCommChoice %1 atom %2 %3 msg '%4' range %5 scope %6 choiceVal %7")
			.arg(storyInstance)
			.arg(atom->Id().GetID())
			.arg(atom->Name())
			.arg(messageText)
			.arg(AuCommStdChannel::RangeName(range))
			.arg(AuCommStdChannel::ScopeName(scope))
			.arg(choiceValue);
		g_Au.Log(logmsg);
	}
}

// Send a comm message from a unit.
void AuScriptLine::SentientSay(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, const QString& channel,
		AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho, const AuVisage& visage) const
{
	// Locate the character who sent the message.
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(!env)
		return;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	const DaSector* sector = env->FindParentSector();
	if(sector)
	{
		sectorId = sector->Id();
		galaxy = sector->Galaxy();
	}

	AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(channel);
	AuCommChatStory msg(messageText, storyInstance, channel, sent.Id(), sent.Name(), sectorId, galaxy, range, scope);
	sent.Say(msg, visage, toWho);	// This causes me to turn to face the person I am talking to.
}

void AuScriptLine::SentientSayAudio(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, const QString& channel,
		AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho, const QByteArray& soundData,
		const AuVisage& visage) const
{
	// Locate the character who sent the message.
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(!env)
		return;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	const DaSector* sector = env->FindParentSector();
	if(sector)
	{
		sectorId = sector->Id();
		galaxy = sector->Galaxy();
	}

	AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(channel);
	AuCommChatAudio msg(messageText, storyInstance, soundData, channel, sent.Id(), sent.Name(), sectorId, galaxy, range, scope);
	sent.Say(msg, visage, toWho);	// This causes me to turn to face the person I am talking to.
}

void AuScriptLine::SentientSayImage(DaPerson& sceneAtom, quint32 storyInstance, const QString& imageTitle,
		const QString& messageText, const QString& channel, AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho,
		const QImage& image, const AuVisage& visage) const
{
	// Locate the character who sent the message.
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(!env)
		return;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	const DaSector* sector = env->FindParentSector();
	if(sector)
	{
		sectorId = sector->Id();
		galaxy = sector->Galaxy();
	}

	AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(channel);
	AuCommChatImage msg(imageTitle, messageText, storyInstance, image, channel, sent.Id(), sent.Name(), sectorId, galaxy, range,
			scope);
	sent.Say(msg, visage, toWho);	// This causes me to turn to face the person I am talking to.
}

void AuScriptLine::SentientSayChoice(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, int choiceValue,
		const QString& channel, AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho,
		const AuVisage& visage) const
{
	// Locate the character who sent the message.
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(!env)
		return;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	const DaSector* sector = env->FindParentSector();
	if(sector)
	{
		sectorId = sector->Id();
		galaxy = sector->Galaxy();
	}

	AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(channel);
	AuCommChatChoice msg(toWho, messageText, storyInstance, choiceValue, channel, sent.Id(), sent.Name(), sectorId, galaxy, range,
			scope);
	sent.Say(msg, visage, toWho);	// This causes me to turn to face the person I am talking to.
}

// Send a comm message from the narrator.
void AuScriptLine::NarratorSendComm(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText,
									const QString& channel) const
{
	//qDebug("AuScriptLine::NarratorSendComm %d %s story %d channel %s text %s",
	//sceneAtom.Id().GetID(), (const char*)sceneAtom.Name().toLatin1(), storyInstance,
	//(const char*)channel.toLatin1(), (const char*)messageText.toLatin1());
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(env)
	{
		AuId sectorId;
		GALAXY galaxy = GALAXY_Count;
		const DaSector* sector = env->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}

		// narrator message
		AuCommNarrate msg(messageText, storyInstance, channel, sectorId, galaxy);
		g_Au.SendAtomChat(msg);
	}
}

void AuScriptLine::NarratorSendCommAudio(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText,
		const QString& channel, const QByteArray& soundData) const
{
	//qDebug("AuScriptLine::NarratorSendCommAudio %d %s story %d channel %s text %s data %d bytes",
	//sceneAtom.Id().GetID(), (const char*)sceneAtom.Name().toLatin1(), storyInstance,
	//(const char*)channel.toAscii(), (const char*)messageText.toLatin1(), soundData.count());
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(env)
	{
		AuId sectorId;
		GALAXY galaxy = GALAXY_Count;
		const DaSector* sector = env->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}

		// narrator message
		AuCommNarrateAudio msg(messageText, storyInstance, soundData, channel, sectorId, galaxy);
		g_Au.SendAtomChat(msg);
	}
}

void AuScriptLine::NarratorSendCommImage(DaPerson& sceneAtom, quint32 storyInstance, const QString& imageTitle,
		const QString& messageText, const QString& channel, const QImage& image) const
{
	//qDebug("AuScriptLine::NarratorSendCommAudio %d %s story %d channel %s text %s data %d bytes",
	//sceneAtom.Id().GetID(), (const char*)sceneAtom.Name().toLatin1(), storyInstance,
	//(const char*)channel.toAscii(), (const char*)messageText.toLatin1(), soundData.count());
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(env)
	{
		AuId sectorId;
		GALAXY galaxy = GALAXY_Count;
		const DaSector* sector = env->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}

		// narrator message
		AuCommNarrateImage msg(imageTitle, messageText, storyInstance, image, channel, sectorId, galaxy);
		g_Au.SendAtomChat(msg);
	}
}

// Send a title by comm message from the narrator.
void AuScriptLine::NarratorSendCommTitle(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText,
		const QString& channel) const
{
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(env)
	{
		AuId sectorId;
		GALAXY galaxy = GALAXY_Count;
		const DaSector* sector = env->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}

		// narrator message
		AuCommNarrateTitle msg(messageText, storyInstance, channel, sectorId, galaxy);
		g_Au.SendAtomChat(msg);
	}
}

// Send a choice by comm message from the narrator.
void AuScriptLine::NarratorSendCommChoice(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, int choiceValue,
		const QString& channel) const
{
	DaAtom* env = sceneAtom.FindEnvironmentAtom();
	if(env)
	{
		AuId sectorId;
		GALAXY galaxy = GALAXY_Count;
		const DaSector* sector = env->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}

		// Send choices to the specific person so they can't be clicked by other people.
		AuCommNarrateChoice msg(messageText, storyInstance, choiceValue, channel, sectorId, galaxy);
		g_Au.SendAtomChat(msg);
	}
}

void AuScriptLine::GoToBranch(QString& branchToBlock) const
{
	Q_ASSERT(m_Func == F_GoTo);
	Q_ASSERT(m_Parms.count() >= 1);
	branchToBlock = m_Parms.last().toString();

	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScriptLine::GoToBranch %1")
			.arg(branchToBlock);
		g_Au.Log(logmsg);
	}
}

bool AuScriptLine::TestBranch(DaPerson& person, quint32 storyInstance, const AuTime& blockStart, QString& branchToBlock,
		QMap<QString, QVariant>& vars, int choicePicked) const
{
	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScriptLine::TestBranch %1 person %3 %4")
			.arg(storyInstance ? QString("%1").arg(storyInstance) : "catalyst")
			.arg(person.Id().GetID())
			.arg(person.Name());
		g_Au.Log(logmsg);
	}

	Q_ASSERT(m_Func == F_If);
	Q_ASSERT(m_Parms.count() >= 2);

	// This 'if' was constructed with parameters that are AuScriptLine objects.
	// Every other script line is a logical operator, i.e. AND or OR, except the
	// last parameter, which is the branch to block label.
	int parms = m_Parms.count();
	int evalparms = parms-1;
	Q_ASSERT((evalparms&1));	// MUST be an odd number of eval parameters

	QList<QVariant>::const_iterator it = m_Parms.cbegin();
	QVariant lastResult;
	SCRIPT_FUNC lop = F_NOP;
	for(int parm=0; parm<evalparms; parm+=2)
	{
		const AuScriptLine& eval = (*it).value<AuScriptLine>();
		QVariant evalResult = eval.Eval(person, storyInstance, blockStart, vars, choicePicked);
		if(parm < evalparms-1)
		{
			// Bump iterator to the logical operator.
			it++;

			// Test the logical operator.
			const AuScriptLine& logop = (*it).value<AuScriptLine>();
			lop = logop.Function();
			switch(lop)
			{
			case F_OR:
				// First true result satisfies an OR.
				if(evalResult.toBool())
				{
					branchToBlock = m_Parms.last().toString();
					return true;
				}
				break;
			case F_AND:
				// First false result satisfies an AND.
				if(!evalResult.toBool())
					return false;
				break;
			case F_EQ:
			case F_LE:
			case F_LT:
			case F_GE:
			case F_GT:
				// Cannot evalulate until we see the next parameter.
				break;
			default:
				qWarning() << "AuScriptLine::TestBranch expected logical operator" << lop;
				break;
			}

			// Bump iterator to next evaluator.
			it++;
		}
		else
		{
			// This is the last parameter.
			bool result;
			switch(lop)
			{
			case F_OR:
			case F_AND:
			default:
				// Pending result of last logical operator, AND or OR. Could be F_NOP if it was the only parameter. e.g. if(a)
				result = evalResult.toBool();
				break;
			case F_EQ:
				// Evalulate value comparison operator.
				result = (lastResult == evalResult);
				//qDebug() << "AuScriptLine::TestBranch" << lastResult << "F_EQ" << evalResult << result;
				break;
			case F_LE:
				// Evalulate value comparison operator.
				result = (lastResult.toInt() <= evalResult.toInt());
				//qDebug() << "AuScriptLine::TestBranch" << lastResult << "F_LE" << evalResult << result;
				break;
			case F_LT:
				// Evalulate value comparison operator.
				result = (lastResult.toInt() < evalResult.toInt());
				//qDebug() << "AuScriptLine::TestBranch" << lastResult << "F_LT" << evalResult << result;
				break;
			case F_GE:
				// Evalulate value comparison operator.
				result = (lastResult.toInt() >= evalResult.toInt());
				//qDebug() << "AuScriptLine::TestBranch" << lastResult << "F_GE" << evalResult << result;
				break;
			case F_GT:
				// Evalulate value comparison operator.
				result = (lastResult.toInt() > evalResult.toInt());
				//qDebug() << "AuScriptLine::TestBranch" << lastResult << "F_GT" << evalResult << result;
				break;
			}

			if(result)
				branchToBlock = m_Parms.last().toString();
			return result;
		}
		lastResult = evalResult;
	}

	// No eval parameters?
	qWarning("AuScriptLine::TestBranch no eval parameters?");
	return false;
}

//static
void AuScriptLine::DumpVars(const QMap<QString, QVariant>& vars)
{
	qDebug() << "AuScriptLine::DumpVars";
	for(QMap<QString, QVariant>::const_iterator ui = vars.cbegin(); ui != vars.cend(); ui++)
	{
		if((*ui).canConvert<AuGlobeLLF>())
		{
			AuGlobeLLF ll((*ui).value<AuGlobeLLF>());
			qDebug() << " var" << ui.key() << "ll val" << ll;
		}
		else if((*ui).canConvert<AuId>())
		{
			AuId id((*ui).value<AuId>());
			qDebug() << " var" << ui.key() << "id val" << id;
		}
		else
			qDebug() << " var" << ui.key() << "val" << *ui;
	}
}

QVariant AuScriptLine::Eval(DaPerson& person, quint32 storyInstance, const AuTime& blockStart, QMap<QString, QVariant>& vars,
		int choicePicked) const
{
	//qDebug("AuScriptLine::Eval person %d func %d negate %c", person.Id().GetID(), m_Func, m_Negate?'y':'n');
	Q_UNUSED(storyInstance);
	QVariant evalResult;
	switch(m_Func)
	{
	case F_CONST:
		evalResult = m_Parms[0];
		break;

	case F_VAR:
		{
			QMap<QString, QVariant>::const_iterator vi = vars.find(m_Parms[0].toString());
			if(vi != vars.cend())
				evalResult = *vi;
		}
		break;

	case F_SecondsElapsed:
		evalResult = QVariant(blockStart.ElapsedSec());
		break;

	case F_Wait:
		evalResult = blockStart.ElapsedSec() >= m_Parms[0].toInt();
		break;

	case F_ChoiceMade:
		evalResult = QVariant(choicePicked);
		break;

	case F_DateIsChristmas:
		{
			QDateTime dt = g_Au.CurrentUTC();
			if(dt.date().month() == 12 && (dt.date().day() == 24 || dt.date().day() == 25))
				evalResult = true;
		}
		break;

	case F_Avatar:
		evalResult = QVariant(person.Id().GetID());
		break;

	case F_AvatarConstructedBuildingHabitable:
		evalResult = person.HasConstructedBuildingHabitable((INDUSTRYID)m_Parms[0].toInt());
		break;

	case F_AvatarConstructedBuildingHarsh:
		evalResult = person.HasConstructedBuildingHarsh((INDUSTRYID)m_Parms[0].toInt());
		break;

	case F_AvatarCurrentItem:
		evalResult = QVariant(person.CurrentItemCID() == (COMMODITYID)m_Parms[0].toInt());
		break;

	case F_AvatarEnvironment:
		{
			const DaAtom* env = person.FindEnvironmentAtom();
			if(env)
				evalResult = QVariant::fromValue(env->Id().GetID());
		}
		break;

	case F_AvatarEVAMinutes:
		evalResult = QVariant(person.EVAMinutes());
		break;

	case F_AvatarHadItem:
		evalResult = person.HadItem((COMMODITYID)m_Parms[0].toInt());
		break;

	case F_AvatarHasFood:
		// Check gear, including inside containers.
		{
			const AuGear* gear = person.LockGearForRead();
			evalResult = QVariant(gear->HasFood(person.EcoRole()));
			person.UnlockGear();
		}
		break;

	case F_AvatarHasItem:
		// Check gear, including inside containers.
		evalResult = QVariant(person.HasItem((COMMODITYID)m_Parms[0].toInt()));
		break;

	case F_AvatarHasItemInHand:
		{
			COMMODITYID cid = (COMMODITYID)m_Parms[0].toInt();
			evalResult = person.HasItemInHand(cid);
		}
		break;

	case F_AvatarHasItemInHandSpecific:
		{
			int pidx=0;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			evalResult = person.HasItemInHandSpecific(cid, quality, empireOfOrigin, inscription);
		}
		break;

	case F_AvatarHasItemSpecific:
		// Check gear, including inside containers.
		{
			int pidx=0;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			evalResult = person.HasItemSpecific(cid, quality, empireOfOrigin, inscription);
		}
		break;

	case F_AvatarInCity:
		evalResult = !person.InBdCity().IsNull();
		break;

	case F_AvatarInitScenario:
		{
			int pidx=0;
			SCENARIO scenario = (SCENARIO)m_Parms[pidx++].toInt();

			evalResult = person.InitScenario() == scenario;
		}
		break;

	case F_AvatarInOrbit:
		evalResult = (!person.IsInAtmosphere() && person.FindParentPlanetInEnv());
		break;

	case F_AvatarInStudio:
		evalResult = person.InDesignEnvironment();
		break;

	case F_AvatarInVehicle:
		if(person.IsAboardVehicle())
		{
			const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person.Parent());
			if(vehicle && vehicle->CommodityId() == (COMMODITYID)m_Parms[0].toInt())
				evalResult = true;
		}
		break;

	case F_AvatarInVehicleMoving:
		if(person.IsAboardVehicle())
		{
			const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person.Parent());
			if(vehicle && vehicle->IsMoving() && vehicle->CommodityId() == (COMMODITYID)m_Parms[0].toInt() &&
					vehicle->Vector().Length2() >= 1)
				evalResult = true;
		}
		break;

	case F_AvatarInVehicleOnGround:
		if(person.IsAboardVehicle())
		{
			const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person.Parent());
			if(vehicle && vehicle->IsOnGround() && vehicle->CommodityId() == (COMMODITYID)m_Parms[0].toInt())
				evalResult = true;
		}
		break;

	case F_AvatarItemCount:
		// Check gear, including inside containers.
		{
			const AuGear* gear = person.LockGearForRead();
			evalResult = QVariant(gear->HasItemCount((COMMODITYID)m_Parms[0].toInt()));
			person.UnlockGear();
		}
		break;

	case F_AvatarLineageAvatar:
		evalResult = QVariant(person.Lineage() == LINEAGE_None);
		break;

	case F_AvatarLineageDemiavatar:
		evalResult = QVariant(person.Lineage() == LINEAGE_Demiavatar);
		break;

	case F_AvatarLineageHighborn:
		evalResult = QVariant(person.Lineage() == LINEAGE_Avatar);
		break;

	case F_AvatarOnLineMinutes:
		evalResult = QVariant(person.OnLineMinutes());
		break;

	case F_AvatarStoryCompleted:
		evalResult = person.HasFinishedStory(m_Parms[0].toInt());
		break;

	case F_AvatarStoryLastResult:
		evalResult = person.StoryLastResult(m_Parms[0].toInt());
		break;

	case F_AvatarVehicleMinutes:
		evalResult = QVariant(person.VehicleMinutes((COMMODITYID)m_Parms[0].toInt()));
		break;

	case F_Atom:
		{
			quint32 u0 = m_Parms[0].toUInt();
			evalResult = QVariant::fromValue(AuId(u0).GetID());
		}
		break;

	case F_AtomCity:
		evalResult = AtomCity(person, vars);
		break;

	case F_AtomDistanceToAtom:
		evalResult = AtomDistanceToAtom(person, vars);
		break;

	case F_AtomEmpire:
		evalResult = AtomEmpire(person, vars);
		break;

	case F_AtomFindBuilding:
		evalResult = AtomFindBuilding(person, vars);
		break;

	case F_AtomFindVehicle:
		evalResult = AtomFindVehicle(person, vars);
		break;

	case F_AtomHasAtom:
		evalResult = AtomHasAtom(person, vars);
		break;

	case F_AtomHasBuilding:
		evalResult = AtomHasBuilding(person, vars);
		break;

	case F_AtomInSystem:
		{
			QString u0 = m_Parms[0].toString();	// atom variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[1].toString();	// system variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = env->Is(u1id) && dynamic_cast<const DaSolarSystem*>(env);
		}
		break;

	case F_AtomIsAtom:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[1].toString();	// unit variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;
			if(u0id == u1id)
				evalResult = true;
		}
		break;

	case F_AtomSector:
		evalResult = AtomSector(person, vars);
		break;

	case F_AtomValid:
		{
			QString u0 = m_Parms[0].toString();	// atom variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->IsNormal();
		}
		break;

	case F_AtomWorld:
		evalResult = AtomWorld(person, vars);
		break;

	case F_AtomWorldLL:
		{
			QString u0 = m_Parms[0].toString();	// atom variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			if(!atom->DoesQueryEnvironment())
				break;

			evalResult = QVariant::fromValue(atom->GlobeLL());
		}
		break;

	case F_BlueprintSpacecraftLoad:
		{
			int pidx=0;
			quint32 designId = m_Parms[pidx++].toUInt();
			if(!designId)
				break;

			// Signal need for the design to the database thread.
			AuScReport r;
			const DgData& sdd = g_Au.LockScDesignsForRead();
			evalResult = sdd.ScDesignReport(designId, r);
			g_Au.UnlockScDesigns();
		}
		break;

	case F_CityFindNewVehicle:
		evalResult = CityFindNewVehicle(person, vars);
		break;

	case F_CityHasBuilding:
		evalResult = CityHasBuilding(person, vars);
		break;

	case F_CityHasFood:
		evalResult = CityHasFood(person, vars);
		break;

	case F_CityHasItem:
		evalResult = CityHasItem(person, vars);
		break;

	case F_CityHasPower:
		evalResult = CityHasPower(person, vars);
		break;

	case F_CityItemCount:
		evalResult = CityItemCount(person, vars);
		break;

	case F_CityPopulation:
		evalResult = CityPopulation(person, vars);
		break;

	case F_CreatureDyingOrDead:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// creature variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaCreature* c = dynamic_cast<DaCreature*>(env->FindIdRecursive(u0id));
			if(!c)
				break;

			evalResult = c->IsBodyDyingOrDead();
		}
		break;

	case F_FindGasGiant:
		evalResult = FindGasGiant(person, vars);
		break;

	case F_FindMoon:
		evalResult = FindMoon(person, vars);
		break;

	case F_FindPlanet:
		evalResult = FindPlanet(person, vars);
		break;

	case F_FindRing:
		evalResult = FindRing(person, vars);
		break;

	case F_FindRingWorld:
		evalResult = FindRingWorld(person, vars);
		break;

	case F_HengeSystemName:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// system variable name
			AuId u0id = VariableAtomId(u0, vars);

			QString systemName;
			if(!u0id.IsNull())
			{
				// Look for a henge system in the sector.
				QStringList hdsystems;
				const AuHengeDestinations& hds = g_Au.LockHengeDestinations();
				for(AuHengeDestinations::const_iterator hdi = hds.cbegin(); hdi != hds.cend(); hdi++)
					if((*hdi).m_SystemId == u0id)
						systemName = (*hdi).m_SystemName;
				g_Au.UnlockHengeDestinations();
			}

			evalResult = QVariant::fromValue(systemName);
		}
		break;

	case F_HengeSystemWorld:
		evalResult = SystemHengeWorldCatName(person, vars);
		break;

	case F_HengeWorld:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// worldcat variable name
			QString u0catname = VariableString(u0, vars);
			if(u0catname.isEmpty())
				break;

			DaSolarSystem* system = person.FindParentSolarSystemInEnv();
			if(!system)
				break;

			DaAtomList worlds;
			system->FindPlanetsRecursive(worlds);
			for(DaAtomList::const_iterator ai = worlds.cbegin(); ai != worlds.cend(); ai++)
			{
				DaWorld* world = (DaWorld*)*ai;
				if(world->CatalogName(system) == u0catname)
				{
					evalResult = QVariant::fromValue(world->Id().GetID());
					break;
				}
			}
		}
		break;

	case F_HengeWorldLL:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world catalog name variable name
			QString u0catname = VariableString(u0, vars);
			if(u0catname.isEmpty())
				break;

			AuGlobeLLF hengeLL;
			const AuHengeDestinations& hds = g_Au.LockHengeDestinations();
			for(AuHengeDestinations::const_iterator hdi = hds.cbegin(); hdi != hds.cend(); hdi++)
				if((*hdi).m_WorldCatName == u0catname)
					hengeLL = (*hdi).m_HengeLL;
			g_Au.UnlockHengeDestinations();
			evalResult = QVariant::fromValue(hengeLL);
		}
		break;

	case F_HengeWorldName:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world catalog name
			QString u0catname = VariableString(u0, vars);
			if(u0catname.isEmpty())
				break;

			QString worldName;
			const AuHengeDestinations& hds = g_Au.LockHengeDestinations();
			for(AuHengeDestinations::const_iterator hdi = hds.cbegin(); hdi != hds.cend(); hdi++)
				if((*hdi).m_WorldCatName == u0catname)
					worldName = (*hdi).m_WorldName;
			g_Au.UnlockHengeDestinations();
			evalResult = QVariant::fromValue(worldName);
		}
		break;

	case F_HengeWorldPortalActive:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			QString u0catname = VariableString(u0, vars);
			if(u0catname.isEmpty())
				break;
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);

			DaSolarSystem* system = person.FindParentSolarSystemInEnv();
			if(!system)
				break;

			// Look for a world in the system with that catalog name.
			DaAtomList worlds;
			system->FindPlanetsRecursive(worlds);
			for(DaAtomList::const_iterator ai = worlds.cbegin(); ai != worlds.cend(); ai++)
			{
				DaWorldLL* world = dynamic_cast<DaWorldLL*>(*ai);
				if(!world)
					continue;
				if(world->CatalogName(system) == u0catname)
				{
					evalResult = world->IsHengeActive(ll);
					break;
				}
			}
		}
		break;

	case F_ItemOnGround:
		evalResult = ItemOnGround(person, vars);
		break;

	case F_ItemOnGroundSpecific:
		evalResult = ItemOnGroundSpecific(person, vars);
		break;

	case F_SectorHasHengeSystem:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// sector variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			int hengeWorldType = m_Parms[pidx++].toInt();	// 0=any, 1=on globe, 2=on ringworld

			// Look for a henge system in the sector.
			const AuHengeDestinations& hds = g_Au.LockHengeDestinations();
			//qDebug() << "AuScriptLine::Eval henge dest count" << hds.size();
			for(AuHengeDestinations::const_iterator hdi = hds.cbegin(); hdi != hds.cend(); hdi++)
			{
				// Currently we only look for henges on habitable worlds that are outside a city.
				if(!(*hdi).IsHabitable() || !(*hdi).IsOutsideTown())
					continue;

				// Sometimes they care what kind of world the henge is on.
				if(hengeWorldType == 1 && (*hdi).m_WorldType != DaAtom::ATOM_WorldGlobe)
					continue;
				else if(hengeWorldType == 2 && (*hdi).m_WorldType != DaAtom::ATOM_WorldRingSection)
					continue;

				if((*hdi).m_SectorId == u0id)
					evalResult = true;
			}
			g_Au.UnlockHengeDestinations();
		}
		break;

	case F_SectorHengeSystem:
		evalResult = SectorHengeSystem(person, vars);
		break;

	case F_SentientHasItem:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			COMMODITYID cid = (COMMODITYID)m_Parms[1].toInt();

			// Locate the sentient in the person's environment.
			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			evalResult = sent->HasItem(cid);
		}
		break;

	case F_SentientHasItemSpecific:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			quint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			// Locate the sentient in the person's environment.
			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			evalResult = sent->HasItemSpecific(cid, quality, empireOfOrigin, inscription);
		}
		break;

	case F_ShipStationOccupant:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			AuStation::TYPE stationType = (AuStation::TYPE)m_Parms[pidx++].toUInt();

			DaVessel* ves = FindShip(person, u0id);
			if(!ves)
				break;

			AuId opId = ves->FindStationOccupant(stationType);
			evalResult = QVariant::fromValue(opId.GetID());
		}
		break;

	case F_ShipStationOccupantInRoom:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			AuStation::TYPE stationType = (AuStation::TYPE)m_Parms[pidx++].toUInt();
			QString room = m_Parms[pidx++].toString();

			DaVessel* ves = FindShip(person, u0id);
			if(!ves)
				break;

			AuId opId = ves->FindStationOccupantInRoom(stationType, room);
			evalResult = QVariant::fromValue(opId.GetID());
		}
		break;

	case F_ShipFindAI:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			NPC_ROLE npcrole = (NPC_ROLE)(m_Parms[pidx++].toInt());

			DaVessel* ves = FindShip(person, u0id);
			if(!ves)
				break;

			DaSentient* sent = ves->FindAI(npcrole);
			if(!sent)
				break;
			evalResult = QVariant::fromValue(sent->Id().GetID());
		}
		break;

	case F_ShipFindAIInRoom:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			NPC_ROLE npcrole = (NPC_ROLE)(m_Parms[pidx++].toInt());
			QString room = m_Parms[pidx++].toString();

			DaVessel* ves = FindShip(person, u0id);
			if(!ves)
				break;

			DaSentient* sent = ves->FindAIInRoom(npcrole, room);
			if(!sent)
				break;
			evalResult = QVariant::fromValue(sent->Id().GetID());
		}
		break;

	case F_ShipFindNpc:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			NPC_ROLE npcrole = (NPC_ROLE)(m_Parms[pidx++].toInt());

			DaVessel* ves = FindShip(person, u0id);
			if(!ves)
				break;

			DaSentient* sent = ves->FindNpc(npcrole);
			if(!sent)
				break;
			evalResult = QVariant::fromValue(sent->Id().GetID());
		}
		break;

	case F_ShipFindNpcInRoom:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			NPC_ROLE npcrole = (NPC_ROLE)(m_Parms[pidx++].toInt());
			QString room = m_Parms[pidx++].toString();

			DaVessel* ves = FindShip(person, u0id);
			if(!ves)
				break;

			DaSentient* sent = ves->FindNpcInRoom(npcrole, room);
			if(!sent)
				break;
			evalResult = QVariant::fromValue(sent->Id().GetID());
		}
		break;

	case F_UnitAboardShip:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);

			AuId shipId;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u0id));
					if(unit && unit->IsInSpacecraft())
					{
						// Locate the ship the unit is aboard.
						DaVessel* ship = unit->FindParentVesselInEnv();
						if(ship)
							shipId = ship->Id();
					}
				}
			}
			evalResult = QVariant::fromValue(shipId.GetID());
		}
		break;

	case F_UnitAtBuilding:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			INDUSTRYID iid = (INDUSTRYID)m_Parms[1].toInt();

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u0id));
			if(!unit)
				break;

			if(unit->InIndustry() == iid)
				evalResult = true;
		}
		break;

	case F_UnitAtWorld:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);

			AuId worldId;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u0id));
					if(unit)
					{
						// Locate the ship the unit is aboard.
						DaWorld* world = unit->FindParentPlanetInEnv();
						if(world)
							worldId = world->Id();
					}
				}
			}
			evalResult = QVariant::fromValue(worldId.GetID());
		}
		break;

	case F_UnitInOrbit:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u0id));
			if(!unit)
				break;

			evalResult = (!unit->IsInAtmosphere() && unit->FindParentPlanetInEnv());
		}
		break;

	case F_UnitRoomName:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u0id));
			if(!unit)
				break;

			evalResult = unit->CapsuleRoomName();
		}
		break;

	case F_UnitInsideShip:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);

			AuId shipId;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u0id));
					if(unit && unit->IsInsideSpacecraft())
					{
						// Locate the ship the unit is aboard.
						DaVessel* ship = unit->FindParentVesselInEnv();
						if(ship)
							shipId = ship->Id();
					}
				}
			}
			evalResult = QVariant::fromValue(shipId.GetID());
		}
		break;

	case F_UnitStationInTradeRange:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);

			AuId stationId;
			if(!u0id.IsNull())
			{
				POLITICAL_STANCE stanceParameter = (POLITICAL_STANCE)m_Parms[1].toInt();
				int stanceCheck = m_Parms[2].toInt();

				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u0id));
					if(unit && unit->IsNormal())
					{
						bool hasStation;
						DaVessel* station = unit->StationInTradeRange(unit->ObstacleRadius(), hasStation);

						// Assign the result to the associated variable. It could already be assigned.
						if(station)
						{
							POLITICAL_STANCE stanceOfUnit = PoliticalStance(station->EmpireId(), person.EmpireId());
							if(CompareStance(stanceOfUnit, stanceParameter, stanceCheck))
								stationId = station->Id();
						}
					}
				}
			}
			evalResult = QVariant::fromValue(stationId.GetID());
		}
		break;

	case F_WorldAtmosphereBreathable:
		evalResult = WorldAtmosphereBreathable(person, vars);
		break;

	case F_WorldClaimed:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->ClaimedByEmpire();
		}
		break;

	case F_WorldClaimedBy:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			qint32 empireSpec = m_Parms[pidx++].toInt();

			quint32 empireId = EmpireId(empireSpec, person);
			if(!empireId)
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->ClaimedByEmpire() == empireId;
		}
		break;

	case F_WorldHasCity:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorld* world = dynamic_cast<DaWorld*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->BdHere();
		}
		break;

	case F_WorldHydrographics:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorld* world = dynamic_cast<DaWorld*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = QVariant(world->Hydrographics());
		}
		break;

	case F_WorldLandLocation:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			double minAltMSL = m_Parms[pidx++].toDouble() * FeetPerMeter();
			double radius = m_Parms[pidx++].toDouble() * FeetPerMeter();

			AuGlobeLLF landLL;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
					if(world && world->IsNormal())
					{
						if(radius > 0)
							landLL = world->RandomLandLLNotForbidden(g_Au.Dice(), minAltMSL, radius);
						else
							landLL = world->RandomLandLL(g_Au.Dice(), minAltMSL);
						g_Au.Dice().Random();	// Arg was const. Step seed value to avoid getting the same number next time.
					}
				}
			}
			evalResult = QVariant::fromValue(landLL);
		}
		break;

	case F_WorldLandLocationNear:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			double minAltMSL = m_Parms[pidx++].toDouble() * FeetPerMeter();
			double minDist = m_Parms[pidx++].toDouble() * FeetPerMeter();
			double maxDist = m_Parms[pidx++].toDouble() * FeetPerMeter();

			AuGlobeLLF landLL;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
					if(world && world->IsNormal())
						landLL = world->RandomLandLLNear(g_Au.Dice(), ll, minAltMSL, minDist, maxDist);
					g_Au.Dice().Random();	// Arg was const. Step seed value to avoid getting the same number next time.
				}
			}
			evalResult = QVariant::fromValue(landLL);
		}
		break;

	case F_WorldOrbitFrigid:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorld* world = dynamic_cast<DaWorld*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->OrbitZone() == OZONE_Frigid;
		}
		break;

	case F_WorldOrbitHabitable:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorld* world = dynamic_cast<DaWorld*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->OrbitZone() == OZONE_Habitable;
		}
		break;

	case F_WorldOrbitInferno:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorld* world = dynamic_cast<DaWorld*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->OrbitZone() == OZONE_Inferno;
		}
		break;

	case F_WorldOrbitInner:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorld* world = dynamic_cast<DaWorld*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->OrbitZone() == OZONE_Inner;
		}
		break;

	case F_WorldOrbitOuter:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaWorld* world = dynamic_cast<DaWorld*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			evalResult = world->OrbitZone() == OZONE_Outer;
		}
		break;

	case F_WorldRelativeLL:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			double xoff = m_Parms[pidx++].toDouble() * FeetPerMeter();
			double yoff = m_Parms[pidx++].toDouble() * FeetPerMeter();

			AuGlobeLLF offLL;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
					if(world && world->IsNormal())
					{
						AuPoint loc = world->LocOfLL(ll);
						AuPoint vUp = -world->GravityDir(loc);

						AuMatrix4x4 t;
						t.Translate(loc);
						t *= AuPoint(0,0,1).GetAlignmentTransform(vUp);
						t.Translate(xoff, yoff, 0);
						loc = t.Translation();

						world->LLOfLoc(offLL, loc);

						//qDebug() << "F_WorldRelativeLL ll" << ll << "offset" << xoff << yoff << "offLL" << offLL;
					}
				}
			}
			evalResult = QVariant::fromValue(offLL);
		}
		break;

	case F_WorldIsGasGiant:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->BodyType() == BODY_GasGiant;
		}
		break;

	case F_WorldIsMoon:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->BodyType() == BODY_Moon;
		}
		break;

	case F_WorldIsPlanet:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->BodyType() == BODY_Planet;
		}
		break;

	case F_WorldIsPlanetoid:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->BodyType() == BODY_Planetoid;
		}
		break;

	case F_WorldIsRing:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->BodyType() == BODY_Ring;
		}
		break;

	case F_WorldIsRingworld:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->BodyType() == BODY_Ringworld;
		}
		break;

	case F_WorldIsTitan:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaAtom* atom = env->FindIdRecursive(u0id);
			if(!atom)
				break;

			evalResult = atom->BodyType() == BODY_Titan;
		}
		break;

	case F_WorldSupportsAnimals:
		evalResult = WorldSupportsAnimals(person, vars);
		break;

	case F_WorldSupportsPlants:
		evalResult = WorldSupportsPlants(person, vars);
		break;

	case F_AND:
	case F_Count:
	case F_EQ:
	case F_GE:
	case F_GT:
	case F_GoTo:
	case F_If:
	case F_LE:
	case F_LT:
	case F_NarrHint:	// Not sent to the client unless they ask for it.
	case F_NOP:
	case F_OR:
	case F_StoryAuthor:
	case F_StoryAutoRepeat:
	case F_StoryAutoRepeatUntilComplete:
	case F_StoryAutoStart:
	case F_StoryBrief:
	case F_StoryInterval:
	case F_StoryTitle:
		// These functions don't "do" anything.
		break;

	case F_AtomComm:
	case F_AtomCommChannelToAtom:
	case F_AtomCommChoice:
	case F_AtomCreateCharacter:
	case F_AvatarCreditGovtDebt:
	case F_AvatarItemMakePermanent:
	case F_AvatarItemSetDropOnDeath:
	case F_AvatarStoryResult:
	case F_CharacterDepart:
	case F_CharacterDie:
	case F_CharacterEquipFromCity:
	case F_CharacterSetIndestructible:
	case F_CharacterWantItem:
	case F_CharacterWantItemSpecific:
	case F_CreatureSetHealth:
	case F_CreatureSetMouth:
	case F_CreatureSetVisage:
	case F_ItemArmWeapon:
	case F_ItemCreateContent:
	case F_ItemDetonate:
	case F_ItemMakePermanent:
	case F_ItemSetDropOnDeath:
	case F_ItemSetMessage:
	case F_ItemSetState:
	case F_NarrImage:
	case F_NarrSay:
	case F_NarrSayAudio:
	case F_NarrSayChoice:
	case F_NarrTitle:
	case F_NarrVideo:
	case F_NpcDocTitle:
	case F_NpcSetRole:
	case F_SentientAttackUnit:
	case F_SentientCreateGear:
	case F_SentientDropGear:
	case F_SentientDropGearItem:
	case F_SentientFireWeapon:
	case F_SentientFollowAvatar:
	case F_SentientFollowSentient:
	case F_SentientGetItem:
	case F_SentientGoToAvatar:
	case F_SentientGoToLL:
	case F_SentientGoToLLExactly:
	case F_SentientGoToRoom:
	case F_SentientGoToUnit:
	case F_SentientHengeCeremony:
	case F_SentientItemMakePermanent:
	case F_SentientItemSetDropOnDeath:
	case F_SentientMoveBack:
	case F_SentientMoveForward:
	case F_SentientSay:
	case F_SentientSayAudio:
	case F_SentientSayChoice:
	case F_SentientSayImage:
	case F_SentientSayTo:
	case F_SentientSellGearItem:
	case F_SentientStay:
	case F_ShipChargeCapacitor:
	case F_ShipChargeShields:
	case F_ShipCreateCharacter:
	case F_ShipCreateCrew:
	case F_ShipCreateTroops:
	case F_ShipDocTitle:
	case F_ShipInteriorLight:
	case F_ShipManeuverDrive:
	case F_ShipPowerPlant:
	case F_ShipRefuel:
	case F_ShipRetreat:
	case F_ShipShields:
	case F_StoryDroppable:
	case F_VehicleRefuel:
	case F_WorldScheduleAsteroid:
	case F_WorldCreateCharacter:
	case F_WorldCreateIndigenous:
	case F_WorldCreateItem:
	case F_WorldCreateShip:
	case F_WorldCreateVehicle:
	case F_WorldHengeCeremony:
	case F_MiscDocTitle:
	case F_StoryDocTitle:
	case F_AvatarDocTitle:
	case F_AtomDocTitle:
	case F_BlueprintDocTitle:
	case F_CharacterDocTitle:
	case F_CityDocTitle:
	case F_CreatureDocTitle:
	case F_FindDocTitle:
	case F_HengeDocTitle:
	case F_ItemDocTitle:
	case F_NarrDocTitle:
	case F_SectorDocTitle:
	case F_SentientDocTitle:
	case F_UnitDocTitle:
	case F_VehicleDocTitle:
	case F_WorldDocTitle:
		qWarning("AuScriptLine::Eval ERROR Bad function %d in 'if' evaluation.", m_Func);
		break;
	}

	if(m_Negate)
		evalResult = QVariant(!evalResult.toBool());
	return evalResult;
}

// Execute this script line.
// Does not perform if() evaluations.
void AuScriptLine::Execute(const AuScript& script, quint32 storyInstance, DaPerson& person, QMap<QString, QVariant>& vars,
		const AuTime& blockStart, bool& storyCancelOk) const
{
	AuEther* ether = person.Ether();
	if(!ether)
		return;
	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScriptLine::Execute %1 script %2 person %3 %4 function %5 %6")
			.arg(storyInstance?QString("%1").arg(storyInstance):"catalyst")
			.arg(script.Id())
			.arg(person.Id().GetID())
			.arg(person.Name())
			.arg(m_Func)
			.arg(s_StoryFunc[m_Func].m_Name);
		g_Au.Log(logmsg);
	}

	// Execute functions that do something.
	QVariant evalResult;
	switch(m_Func)
	{
	case F_CONST:
	case F_VAR:
	case F_SecondsElapsed:
	case F_Wait:
	case F_DateIsChristmas:
	case F_Avatar:
	case F_AvatarConstructedBuildingHabitable:
	case F_AvatarConstructedBuildingHarsh:
	case F_AvatarCreditGovtDebt:
	case F_AvatarCurrentItem:
	case F_AvatarEnvironment:
	case F_AvatarEVAMinutes:
	case F_AvatarHadItem:
	case F_AvatarHasFood:
	case F_AvatarHasItem:
	case F_AvatarHasItemInHand:
	case F_AvatarHasItemInHandSpecific:
	case F_AvatarHasItemSpecific:
	case F_AvatarInCity:
	case F_AvatarInitScenario:
	case F_AvatarInOrbit:
	case F_AvatarInStudio:
	case F_AvatarInVehicle:
	case F_AvatarInVehicleMoving:
	case F_AvatarInVehicleOnGround:
	case F_AvatarItemCount:
	case F_AvatarLineageAvatar:
	case F_AvatarLineageDemiavatar:
	case F_AvatarLineageHighborn:
	case F_AvatarOnLineMinutes:
	case F_AvatarStoryCompleted:
	case F_AvatarStoryLastResult:
	case F_AvatarVehicleMinutes:
	case F_Atom:
	case F_AtomCity:
	case F_AtomDistanceToAtom:
	case F_AtomEmpire:
	case F_AtomFindBuilding:
	case F_AtomFindVehicle:
	case F_AtomHasAtom:
	case F_AtomHasBuilding:
	case F_AtomInSystem:
	case F_AtomIsAtom:
	case F_AtomSector:
	case F_AtomValid:
	case F_AtomWorld:
	case F_AtomWorldLL:
	case F_BlueprintSpacecraftLoad:
	case F_CityFindNewVehicle:
	case F_CityHasBuilding:
	case F_CityHasFood:
	case F_CityHasItem:
	case F_CityHasPower:
	case F_CityItemCount:
	case F_CityPopulation:
	case F_CreatureDyingOrDead:
	case F_FindGasGiant:
	case F_FindMoon:
	case F_FindPlanet:
	case F_FindRing:
	case F_FindRingWorld:
	case F_HengeSystemName:
	case F_HengeSystemWorld:
	case F_HengeWorld:
	case F_HengeWorldLL:
	case F_HengeWorldName:
	case F_HengeWorldPortalActive:
	case F_ItemOnGround:
	case F_ItemOnGroundSpecific:
	case F_SectorHasHengeSystem:
	case F_SectorHengeSystem:
	case F_SentientHasItem:
	case F_SentientHasItemSpecific:
	case F_ShipFindAI:
	case F_ShipFindAIInRoom:
	case F_ShipFindNpc:
	case F_ShipFindNpcInRoom:
	case F_ShipStationOccupant:
	case F_ShipStationOccupantInRoom:
	case F_UnitAboardShip:
	case F_UnitAtBuilding:
	case F_UnitAtWorld:
	case F_UnitInOrbit:
	case F_UnitInsideShip:
	case F_UnitRoomName:
	case F_UnitStationInTradeRange:
	case F_WorldAtmosphereBreathable:
	case F_WorldClaimed:
	case F_WorldClaimedBy:
	case F_WorldHasCity:
	case F_WorldHydrographics:
	case F_WorldLandLocation:
	case F_WorldLandLocationNear:
	case F_WorldOrbitFrigid:
	case F_WorldOrbitHabitable:
	case F_WorldOrbitInferno:
	case F_WorldOrbitInner:
	case F_WorldOrbitOuter:
	case F_WorldRelativeLL:
	case F_WorldIsGasGiant:
	case F_WorldIsMoon:
	case F_WorldIsPlanet:
	case F_WorldIsPlanetoid:
	case F_WorldIsRing:
	case F_WorldIsRingworld:
	case F_WorldIsTitan:
	case F_WorldSupportsAnimals:
	case F_WorldSupportsPlants:
		// These const functions can use the same code that is in the Eval( function.
		if(!m_Assignee.isEmpty())
			evalResult = Eval(person, storyInstance, blockStart, vars, 1000);
		break;

	case F_AvatarItemMakePermanent:
		{
			int pidx=0;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			person.StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin, inscription);
		}
		break;

	case F_AvatarItemSetDropOnDeath:
		{
			int pidx=0;
			int drop = m_Parms[pidx++].toInt();
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			person.StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin, inscription);
		}
		break;

	case F_AvatarStoryResult:
		person.RememberStoryResult(script.Id(), m_Parms[0].toInt());
		break;

	case F_AtomComm:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString channel = StringParameter(m_Parms[1].toString(), person, vars);	// Channel name
			AuCommStdChannel::RANGE range = (AuCommStdChannel::RANGE)m_Parms[2].toInt();	// Broadcast Range
			QString text = StringParameter(m_Parms[3].toString(), person, vars);

			// Send the comm message.
			AtomSendComm(person, storyInstance, text, channel, range, u0id);
		}
		break;

	case F_AtomCommChannelToAtom:
		AtomOpenChannel(person, vars);
		break;

	case F_AtomCommChoice:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString channel = StringParameter(m_Parms[pidx++].toString(), person, vars);	// Channel name
			AuCommStdChannel::RANGE range = (AuCommStdChannel::RANGE)m_Parms[pidx++].toInt();	// Broadcast Range
			int choiceValue = m_Parms[pidx++].toInt();									// Choice value
			QString text = StringParameter(m_Parms[pidx++].toString(), person, vars);

			// Send the comm message.
			AtomSendCommChoice(person, storyInstance, text, choiceValue, channel, range, u0id);
		}
		break;


	case F_AtomCreateCharacter:
		evalResult = AtomCreateCharacter(person, vars, script.Id(), storyInstance);
		break;

	case F_CharacterDepart:
		{
			QString u0 = m_Parms[0].toString();		// character variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* character = dynamic_cast<DaNpc*>(env->FindIdRecursive(u0id));
			if(!character)
				break;

			character->Kill();	// DHH Come up with a better way to depart the scene.
		}
		break;

	case F_CharacterDie:
		{
			QString u0 = m_Parms[0].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* character = dynamic_cast<DaNpc*>(env->FindIdRecursive(u0id));
			if(!character)
				break;

			character->SetIndestructible(false);

			AuCombatAttacker attacker;

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_InternalInjuries;
			weapon.m_Quality = 1;
			weapon.m_w1.m_Effect = WE_Crush;

			AuCombatResult result;
			result.m_Damage1 = character->CurrentHitPoints() - DpBody::DeadHitPoints();

			character->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);

			character->SetModified();
		}
		break;

	case F_CharacterEquipFromCity:
		{
			// Get parameter values.
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			AuGearSlot::SLOTTYPE slotType = (AuGearSlot::SLOTTYPE)m_Parms[pidx++].toInt();	// gear
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();							// commodity
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);	// optional inscription

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = dynamic_cast<DaNpc*>(env->FindIdRecursive(u0id));
			if(!npc)
				break;

			// Check to see if the character already has the item.
			const AuGear* ninv = npc->LockGearForRead();
			bool h = ninv->HasItemInSlot(slotType, cid);
			npc->UnlockGear();
			if(h)
				break;											// Character already has the item in that gear slot.

			// Take the item from the inventory of the city that contains the character.
			if(npc->BdId().IsNull())
				break;
			DaWorld* planet = npc->FindParentPlanetInEnv();
			if(!planet)
				break;											// This should not happen if they are in a city.
			DaAtom* atom = planet->FindSubatomId(npc->BdId());
			if(!atom)
				break;
			DaBd* bd = dynamic_cast<DaBd*>(atom);
			if(bd)
			{
				AuItemHash cinv;
				bd->CommunityInventoryAvailable(cinv, npc->EmpireId());
				AuItemHash::const_iterator ci = cinv.FindBest(cid);
				if(ci != cinv.end())
				{
					// Take the item from the city's inventory.
					AuItem* item = (*ci).second->Clone();
					item->SetCount(1);
					bd->CommunityRemoveInventory(*item);

					// Inscribe the nu item if an inscription was provided.
					if(!inscription.isEmpty() && item->Inscription().isEmpty())
						item->Inscribe(inscription);

					// We wouldn't give a story character a light if it wasn't supposed to be on...so far anyway.
					if(item->IsLight())
						item->SetState(1);

					// Add the item to the character's gear.
					npc->AddGearItemAt(slotType, *item);
					delete item;
				}
			}

			// Remember the character in the person's memory so they can be recalled later with the same information.
			AuDNABase* dna = npc->DNAnew();
			if(dna)
			{
				person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
							npc->Handedness(), npc->GrowthStage(), npc->EmpireId(), npc->LockGearForRead(), planet));
				npc->UnlockGear();
				delete dna;
			}
		}
		break;

	case F_CharacterSetIndestructible:
		{
			QString u0 = m_Parms[0].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			bool isIndestructible = m_Parms[1].toBool();

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* character = dynamic_cast<DaNpc*>(env->FindIdRecursive(u0id));
			if(!character)
				break;

			character->SetIndestructible(isIndestructible);
			character->SetModified();
		}
		break;

	case F_CharacterWantItem:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaNpc* sent = dynamic_cast<DaNpc*>(env->FindIdRecursive(u0id));
			if(sent && sent->IsNormal())
				sent->WantItem(cid);
		}
		break;

	case F_CharacterWantItemSpecific:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			quint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;

			DaNpc* sent = dynamic_cast<DaNpc*>(env->FindIdRecursive(u0id));
			if(sent && sent->IsNormal())
				sent->WantItem(cid, quality, empireOfOrigin, inscription);
		}
		break;

	case F_CreatureSetHealth:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			quint16 health = m_Parms[pidx++].toUInt();
			if(health > 100)
				break;

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaCreature* c = dynamic_cast<DaCreature*>(env->FindIdRecursive(u0id));
			if(!c)
				break;

			quint16 newHp = health * c->MaxHitPoints() / 100;
			if(newHp <= c->MaxHitPoints() && newHp != c->CurrentHitPoints())
			{
				c->SetHitPoints(newHp);
				c->SetModified();
			}
		}
		break;
	case F_CreatureSetMouth:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			AuVisage startVisage((AuVisage::VISAGE)m_Parms[pidx++].toInt());
			AuVisage endVisage((AuVisage::VISAGE)m_Parms[pidx++].toInt());

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaCreature* c = dynamic_cast<DaCreature*>(env->FindIdRecursive(u0id));
			if(!c)
				break;

			if(c->PoseVisage(startVisage, endVisage))
				c->SetModified();
		}
		break;
	case F_CreatureSetVisage:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			AuVisage visage((AuVisage::VISAGE)m_Parms[pidx++].toInt());

			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaCreature* c = dynamic_cast<DaCreature*>(env->FindIdRecursive(u0id));
			if(!c)
				break;

			if(c->PoseVisage(visage, visage))
				c->SetModified();
		}
		break;

	case F_ItemArmWeapon:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// unit variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;
			qint32 empire = m_Parms[pidx++].toInt();

			// Locate the item in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaItem* itemAtom = dynamic_cast<DaItem*>(env->FindIdRecursive(u0id));
			if(!itemAtom)
				break;
			DaUnit* unit = dynamic_cast<DaUnit*>(env->FindIdRecursive(u1id));
			if(!unit)
				break;

			quint32 empireId = EmpireId(empire, person);

			itemAtom->ArmWeapon(unit->Id(), unit->Name(), empireId);
		}
		break;

	case F_ItemCreateContent:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			bool isPermanent = m_Parms[pidx++].toBool();
			quint8 conditionPercent = m_Parms[pidx++].toUInt();
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			// Locate the item in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaItem* itemAtom = dynamic_cast<DaItem*>(env->FindIdRecursive(u0id));
			if(!itemAtom)
				break;

			quint32 empireId = EmpireId(empireOfOrigin, person);

			AuItem* item = itemAtom->LockItemForWrite();
			if(item)
			{
				// The item atom has an item.
				AuItemContent* cont = item->Content();
				quint8 cap = item->CapacityInPackUnits();
				if(cont && cont->ContainsPackUnits() < cap)
				{
					// The item is a container that is not full.
					AuItem* newItem = AuItem::Create(cid, quality, empireId);
					if(newItem)
					{
						// Adjust condition.
						if(conditionPercent < 100)
						{
							int cond = newItem->HitPoints() * conditionPercent / 100;
							if(cond < 1)
								cond = 1;
							newItem->SetHitPoints(cond);
						}

						// Inscribe the nu item if an inscription was provided.
						if(!inscription.isEmpty())
							newItem->Inscribe(inscription);

						// An item with a story instance gets deleted when the story is deleted.
						if(!isPermanent)
							newItem->SetStoryInstance(storyInstance);	// Item is not permanent.

						// Attempt to fit the nu item into the container.
						if(cont->ContainsPackUnits() + newItem->VolumeInPackUnits() <= cap)
						{
							cont->Insert(newItem);
							itemAtom->SetModified();
						}
						else
							delete newItem;
					}
				}
			}
			itemAtom->UnlockItem();
		}
		break;

	case F_ItemDetonate:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			// Locate the item in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaItem* itemAtom = dynamic_cast<DaItem*>(env->FindIdRecursive(u0id));
			if(!itemAtom)
				break;

			itemAtom->DetonateInitiate();
		}
		break;

	case F_ItemMakePermanent:
		{
			QString u0 = m_Parms[0].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			// Locate the item in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaItem* itemAtom = dynamic_cast<DaItem*>(env->FindIdRecursive(u0id));
			if(!itemAtom)
				break;

			itemAtom->StoryMakePermanent(storyInstance);
		}
		break;

	case F_ItemSetDropOnDeath:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			int drop = m_Parms[pidx++].toInt();

			// Locate the item in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaItem* itemAtom = dynamic_cast<DaItem*>(env->FindIdRecursive(u0id));
			if(!itemAtom)
				break;

			itemAtom->StorySetDropOnDeath(storyInstance, drop);
		}
		break;

	case F_ItemSetMessage:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString text = m_Parms[pidx++].toString();

			// Locate the item in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaItem* itemAtom = dynamic_cast<DaItem*>(env->FindIdRecursive(u0id));
			if(!itemAtom)
				break;

			itemAtom->SetItemMessage(text);
		}
		break;

	case F_ItemSetState:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// item variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			int state = m_Parms[pidx++].toInt();

			// Locate the item in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaItem* itemAtom = dynamic_cast<DaItem*>(env->FindIdRecursive(u0id));
			if(!itemAtom)
				break;

			itemAtom->SetItemState(state);
		}
		break;

	case F_NarrImage:
		{
			// Get parameter values.
			int pidx=0;
			QByteArray imageData(m_Parms[pidx++].toByteArray());						// Image file data
			QString title = StringParameter(m_Parms[pidx++].toString(), person, vars);
			QString text = StringParameter(m_Parms[pidx++].toString(), person, vars);

			QImage image;
			image.loadFromData(imageData);
			if(image.isNull())
				break;

			// Send the comm message.
			NarratorSendCommImage(person, storyInstance, title, text, script.Title(), image);
		}
		break;

	case F_NarrSay:
		// Narrator says something on the story channel.
		{
			// Get parameter values.
			QString text = StringParameter(m_Parms[0].toString(), person, vars);

			// Send the comm message.
			NarratorSendComm(person, storyInstance, text, script.Title());
		}
		break;

	case F_NarrSayAudio:
		// Narrator says something on the story channel, with audio.
		{
			// Get parameter values.
			QByteArray soundData(m_Parms[0].toByteArray());						// Audio file data
			QString text = StringParameter(m_Parms[1].toString(), person, vars);

			// Send the comm message.
			NarratorSendCommAudio(person, storyInstance, text, script.Title(), soundData);
		}
		break;

	case F_NarrSayChoice:
		// Send narrative choice to the person.
		{
			// Get parameter values.
			int choiceValue = m_Parms[0].toInt();									// Choice value
			QString text = StringParameter(m_Parms[1].toString(), person, vars);

			// Send the comm choice message.
			NarratorSendCommChoice(person, storyInstance, text, choiceValue, script.Title());
		}
		break;

	case F_NarrTitle:
		// Send narrative title to the person.
		{
			// Get parameter values.
			QString text = StringParameter(m_Parms[0].toString(), person, vars);

			// Send the comm message.
			NarratorSendCommTitle(person, storyInstance, text, script.Title());
		}
		break;

	case F_NarrVideo:
		//DHH Send video file to client.
		break;

	case F_NpcSetRole:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// npc variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			NPC_ROLE npcrole = (NPC_ROLE)m_Parms[pidx++].toInt();

			// Locate the npc in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = dynamic_cast<DaNpc*>(env->FindIdRecursive(u0id));
			if(!npc)
				break;

			if(npc->SetRole(npcrole))
				npc->SetModified();
		}
		break;

	case F_SentientAttackUnit:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[1].toString();	// unit variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to follow the person.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_AttackTarget, u1id, AuPoint(0,0), true);
		}
		break;

	case F_SentientCreateGear:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			AuGearSlot::SLOTTYPE slotType = (AuGearSlot::SLOTTYPE)m_Parms[pidx++].toInt();	// gear
			bool isPermanent = m_Parms[pidx++].toBool();
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			// Locate the sentient in the person's environment.
			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Check to see if the character already has the item. This happens when a section that outfits a character is repeated.
			bool h = true;	// Start true so we'll fail out if we get no gear.
			const AuGear* ninv = sent->LockGearForRead();
			if(ninv)
				h = ninv->HasItemInSlot(slotType, cid);
			sent->UnlockGear();
			if(h)
				break;											// Sentient already has the item in that gear slot.

			quint32 empireId = EmpireId(empireOfOrigin, person);

			// Create the item out of thin air.
			AuItem* newItem = AuItem::Create(cid, quality, empireId);
			if(!newItem)
				break;

			// Inscribe the nu item if an inscription was provided.
			if(!inscription.isEmpty())
				newItem->Inscribe(inscription);

			// We wouldn't give a story character a light if it wasn't supposed to be on...so far anyway.
			if(newItem->IsLight())
				newItem->SetState(1);

			// An item with a story instance gets deleted when the story is deleted.
			if(!isPermanent)
				newItem->SetStoryInstance(storyInstance);	// Item is not permanent.

			// Add the item to the character's gear.
			sent->AddGearItemAt(slotType, *newItem);
			delete newItem;

			// Remember the character in the person's memory so they can be recalled later with the same information.
			const DaNpc* npc = dynamic_cast<const DaNpc*>(sent);
			if(!npc)
				break;

			AuDNABase* dna = sent->DNAnew();
			if(!dna)
				break;

			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
						npc->Handedness(), npc->GrowthStage(), npc->EmpireId(), npc->LockGearForRead(), npc->HomeGalaxy(),
						npc->HomeSectorId(), npc->HomeSectorName(), npc->HomeSolarSystemId(), npc->HomeSolarSystemName(),
						npc->HomeWorldId(), npc->HomeWorldName(), npc->HomeWorldBodyType()));
			npc->UnlockGear();
			delete dna;
		}
		break;

	case F_SentientDropGear:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			AuGearSlot::SLOTTYPE slotType = (AuGearSlot::SLOTTYPE)m_Parms[1].toInt();	// gear

			// Locate the sentient in the person's environment.
			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			AuId itemId;
			DaItem* item = sent->DropGearSlot(slotType);
			if(item)
				itemId = item->Id();

			evalResult = QVariant::fromValue(itemId.GetID());

			const DaNpc* npc = dynamic_cast<const DaNpc*>(sent);
			if(!npc)
				break;

			// Remember the character in the person's memory so they can be recalled later with the same information.
			AuDNABase* dna = sent->DNAnew();
			if(!dna)
				break;

			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
						npc->Handedness(), npc->GrowthStage(), npc->EmpireId(), npc->LockGearForRead(), npc->HomeGalaxy(),
						npc->HomeSectorId(), npc->HomeSectorName(), npc->HomeSolarSystemId(), npc->HomeSolarSystemName(),
						npc->HomeWorldId(), npc->HomeWorldName(), npc->HomeWorldBodyType()));
			npc->UnlockGear();
			delete dna;
		}
		break;

	case F_SentientDropGearItem:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			COMMODITYID cid = (COMMODITYID)m_Parms[1].toInt();

			// Locate the sentient in the person's environment.
			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			AuId itemId;
			DaItem* item = sent->DropGearItem(cid);
			if(item)
				itemId = item->Id();

			evalResult = QVariant::fromValue(itemId.GetID());

			// Remember the character in the person's memory so they can be recalled later with the same information.
			const DaNpc* npc = dynamic_cast<const DaNpc*>(sent);
			if(!npc)
				break;

			AuDNABase* dna = sent->DNAnew();
			if(!dna)
				break;

			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
						npc->Handedness(), npc->GrowthStage(), npc->EmpireId(), npc->LockGearForRead(), npc->HomeGalaxy(),
						npc->HomeSectorId(), npc->HomeSectorName(), npc->HomeSolarSystemId(), npc->HomeSolarSystemName(),
						npc->HomeWorldId(), npc->HomeWorldName(), npc->HomeWorldBodyType()));
			npc->UnlockGear();
			delete dna;
		}
		break;

	case F_SentientFireWeapon:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;
			sent->FireWeapon();

			// Remember the character in the person's memory so they can be recalled later with the same information.
			const DaNpc* npc = dynamic_cast<const DaNpc*>(sent);
			if(!npc)
				break;

			AuDNABase* dna = sent->DNAnew();
			if(!dna)
				break;

			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(),
						npc->Shave(), npc->Handedness(), npc->GrowthStage(), npc->EmpireId(), npc->LockGearForRead(),
						npc->HomeGalaxy(), npc->HomeSectorId(), npc->HomeSectorName(), npc->HomeSolarSystemId(),
						npc->HomeSolarSystemName(), npc->HomeWorldId(), npc->HomeWorldName(), npc->HomeWorldBodyType()));
			npc->UnlockGear();
			delete dna;
		}
		break;

	case F_SentientFollowAvatar:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to follow the person.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_FollowTarget, person.Id(), AuPoint(0,0), true);
		}
		break;

	case F_SentientFollowSentient:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[1].toString();	// unit variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to follow the person.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_FollowTarget, u1id, AuPoint(0,0), true);
		}
		break;

	case F_SentientGetItem:
		{
			QString u0 = m_Parms[0].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[1].toString();	// item variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to follow the person.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_GetItem, u1id, AuPoint(0,0), true);
		}
		break;

	case F_SentientGoToAvatar:
		{
			QString u0 = m_Parms[0].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to go to the person.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_GoToTarget, person.Id(), AuPoint(0,0), true);
		}
		break;

	case F_SentientGoToLL:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(sent->FindParentPlanetInEnv());
			if(llworld)
			{
				AuPoint destLoc = llworld->LocOfLL(ll, llworld->TerrainAltitudeMSL(ll));

				// Order the sentient to go to the world location.
				AuCommChat msg;	// Not used by DaNpc::TakeOrder.
				sent->TakeOrder(msg, ORDER_GoToTarget, llworld->Id(), destLoc, true);
			}
		}
		break;

	case F_SentientGoToLLExactly:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(sent->FindParentPlanetInEnv());
			if(llworld)
			{
				AuPoint destLoc = llworld->LocOfLL(ll, llworld->TerrainAltitudeMSL(ll));

				// Order the sentient to go to the world location.
				AuCommChat msg;	// Not used by DaNpc::TakeOrder.
				sent->TakeOrder(msg, ORDER_GoToTargetExactly, llworld->Id(), destLoc, true);
			}
		}
		break;

	case F_SentientGoToRoom:
		{
			QString u0 = m_Parms[0].toString();					// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString roomName(m_Parms[1].toString());			// room name
			if(roomName.isEmpty())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			DaCapsule* capsule = dynamic_cast<DaCapsule*>(sent->Parent());
			if(capsule)
			{
				// Order the sentient to go to the room.
				AuCommChat msg;	// Not used by DaNpc::TakeOrder.
				sent->TakeOrder(msg, ORDER_GoToRoom, capsule->Id(), AuPoint(0,0), true, roomName);
			}
		}
		break;

	case F_SentientGoToUnit:
		{
			QString u0 = m_Parms[0].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[1].toString();	// unit variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to go to the unit.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_GoToTarget, u1id, AuPoint(0,0), true);
		}
		break;

	case F_SentientHengeCeremony:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(sent->FindParentPlanetInEnv());
			if(llworld)
			{
				AuPoint loc = llworld->LocOfLL(ll, llworld->TerrainAltitudeMSL(ll));

				// Order the character to go to the ceremony.
				AuCommChat msg;	// Not used by DaNpc::TakeOrder.
				sent->TakeOrder(msg, ORDER_HengeCeremony, llworld->Id(), loc, true);
			}
		}
		break;

	case F_SentientItemMakePermanent:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			sent->StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin, inscription);

			// Remember the character in the person's memory so they can be recalled later with the same information.
			const DaNpc* npc = dynamic_cast<const DaNpc*>(sent);
			if(!npc)
				break;

			AuDNABase* dna = sent->DNAnew();
			if(!dna)
				break;

			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
						npc->Handedness(), npc->GrowthStage(), npc->EmpireId(), npc->LockGearForRead(), npc->HomeGalaxy(),
						npc->HomeSectorId(), npc->HomeSectorName(), npc->HomeSolarSystemId(), npc->HomeSolarSystemName(),
						npc->HomeWorldId(), npc->HomeWorldName(), npc->HomeWorldBodyType()));
			npc->UnlockGear();
			delete dna;
		}
		break;

	case F_SentientItemSetDropOnDeath:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			int drop = m_Parms[pidx++].toInt();
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toUInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			sent->StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin, inscription);

			// Remember the character in the person's memory so they can be recalled later with the same information.
			const DaNpc* npc = dynamic_cast<const DaNpc*>(sent);
			if(!npc)
				break;

			AuDNABase* dna = sent->DNAnew();
			if(!dna)
				break;

			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
						npc->Handedness(), npc->GrowthStage(), npc->EmpireId(), npc->LockGearForRead(), npc->HomeGalaxy(),
						npc->HomeSectorId(), npc->HomeSectorName(), npc->HomeSolarSystemId(), npc->HomeSolarSystemName(),
						npc->HomeWorldId(), npc->HomeWorldName(), npc->HomeWorldBodyType()));
			npc->UnlockGear();
			delete dna;
		}
		break;

	case F_SentientMoveBack:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			double distance = m_Parms[pidx++].toDouble() * FeetPerMeter();	// distance
			if(distance <= 0)
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to move relative to itself.
			AuId moveTarget = sent->Parent()->Id();
			AuPoint moveLoc = sent->Transform().Project(0, -distance, 0);
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_GoToTargetExactly, moveTarget, moveLoc, true);
		}
		break;

	case F_SentientMoveForward:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// sentient variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			double distance = m_Parms[pidx++].toDouble() * FeetPerMeter();	// distance
			if(distance <= 0)
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the sentient to move relative to itself.
			AuId moveTarget = sent->Parent()->Id();
			AuPoint moveLoc = sent->Transform().Project(0, distance, 0);
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_GoToTargetExactly, moveTarget, moveLoc, true);
		}
		break;

	case F_SentientSay:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString text = StringParameter(m_Parms[pidx++].toString(), person, vars);
			AuVisage visage((AuVisage::VISAGE)m_Parms[pidx++].toInt());

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Send the comm message.
			SentientSay(person, storyInstance, text, AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Close, *sent, AuId(0),
					visage);
		}
		break;

	case F_SentientSayAudio:
		{
			// Get parameter values.
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QByteArray soundData(m_Parms[pidx++].toByteArray());						// Audio file data
			QString text = StringParameter(m_Parms[pidx++].toString(), person, vars);
			AuVisage visage((AuVisage::VISAGE)m_Parms[pidx++].toInt());

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Send the comm message.
			if(soundData.isEmpty())
				SentientSay(person, storyInstance, text, AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Close, *sent,
						AuId(0), visage);
			else
				SentientSayAudio(person, storyInstance, text, AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Close, *sent,
						AuId(0), soundData, visage);
		}
		break;

	case F_SentientSayImage:
		{
			// Get parameter values.
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QByteArray imageData(m_Parms[pidx++].toByteArray());						// Image file data
			QString title = StringParameter(m_Parms[pidx++].toString(), person, vars);
			QString text = StringParameter(m_Parms[pidx++].toString(), person, vars);
			AuVisage visage((AuVisage::VISAGE)m_Parms[pidx++].toInt());

			QImage image;
			image.loadFromData(imageData);
			if(image.isNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Send the comm message.
			SentientSayImage(person, storyInstance, title, text, AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Close,
					*sent, AuId(0), image, visage);
		}
		break;

	case F_SentientSayChoice:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			int choiceValue = m_Parms[pidx++].toInt();									// Choice value
			QString text = StringParameter(m_Parms[pidx++].toString(), person, vars);
			AuVisage visage((AuVisage::VISAGE)m_Parms[pidx++].toInt());

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Send the comm choice message.
			SentientSayChoice(person, storyInstance, text, choiceValue, AuCommStdChannel::VoiceChannelName(),
					AuCommStdChannel::Close, *sent, AuId(0), visage);
		}
		break;

	case F_SentientSayTo:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// unit variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;
			QString text = StringParameter(m_Parms[pidx++].toString(), person, vars);
			AuVisage visage((AuVisage::VISAGE)m_Parms[pidx++].toInt());

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Send the comm message.
			SentientSay(person, storyInstance, text, AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Close, *sent, u1id,
					visage);
		}
		break;

	case F_SentientSellGearItem:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// unit variable name
			AuId u1id = VariableAtomId(u1, vars);
			if(u1id.IsNull())
				break;
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint32 price = m_Parms[pidx++].toUInt();

			// Locate the sentient in the person's environment.
			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			AuId sectorId = sent->FindParentSector()->Id();
			GALAXY galaxy = sent->Galaxy();
			const AuGear* gear = sent->LockGearForRead();
			if(gear)
			{
				AuGearPos gpos = gear->FindBest(cid);
				if(gpos.IsValid())
				{
					const AuItem* item = gear->ItemAt(gpos);
					if(item)
					{
						QString empireOfOrigin;
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName(item->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();

						// Send the comm message.
						AuCommForSale msg(*item, empireOfOrigin, price, AuCommStdChannel::VoiceChannelName(), sent->Id(),
								sent->Name(), sectorId, galaxy, AuCommStdChannel::Close, AuCommStdChannel::Everyone);
						sent->Say(msg, u1);	// This causes me to turn to face the person I am talking to.

						// The sentient must monitor the voice channel for a response.
						g_Au.MonitorChannel(AuCommStdChannel::VoiceChannelName(), sent->Id());
					}
				}
			}
			sent->UnlockGear();
		}
		break;

	case F_SentientStay:
		{
			QString u0 = m_Parms[0].toString();	// unit variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;

			DaSentient* sent = FindSentient(person, u0id);
			if(!sent)
				break;

			// Order the character to stay put.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			sent->TakeOrder(msg, ORDER_Stay, AuId(0), AuPoint(0,0), true);
		}
		break;

	case F_ShipChargeCapacitor:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
						ship->RechargeCapacitor();
				}
			}
		}
		break;

	case F_ShipChargeShields:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
						ship->ShieldCharge(100000);
				}
			}
		}
		break;

	case F_ShipCreateCharacter:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			int placement = m_Parms[pidx++].toInt();
			QString name(m_Parms[pidx++].toString());						// Character Name
			GENDER gender = (GENDER)m_Parms[pidx++].toInt();
			HANDEDNESS handedness = (HANDEDNESS)m_Parms[pidx++].toInt();
			qint32 empireSpec = m_Parms[pidx++].toInt();
			AuDNABase* dna = DNAParameterNew(m_Parms[pidx++], person);
			if(!dna)
				break;
			GROWTHSTAGE stage = (GROWTHSTAGE)m_Parms[pidx++].toInt();

			// First check to see if the character is already in the environment.
			// We'll move their atom if they are found.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
			{
				delete dna;
				break;	// This should never happen.
			}

			// Locate the ship.
			DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
			if(!ship)
			{
				delete dna;
				break;
			}

			// Location
			AuMatrix4x4 t = FindStartingLocationAboard(ship, placement);
			AuAtomPos ap(ship->Id(), t);

			// Empire Id
			quint32 empireId = EmpireId(empireSpec, person);

			const DaWorld* world = ship->FindParentPlanetInEnv();	// May be 0.

			DaNpc* npc = RestoreNpcFromMemory(person, storyInstance, env, name, ap, AuCommStdChannel::CloseRangeDistance());
			if(!npc)
			{
				// Prepare to spawn a nu npc.
				// Spawn the atom in the environment, relative to the same parent as the atom.
				npc = new DaNpc(ROLE_Citizen, *dna, gender == Male ? AuDNABase::Alpha : AuDNABase::Beta, handedness, name);
				if(stage == Juvenile)
				{
					// DaNpc::GrowNaturally does not change m_MinutesToFullGrown for story characters. Whatever size I pick here is
					// my size until it is assertively changed.
					quint8 fullGrowthTime = npc->GrowthTime();
					npc->SetMinutesToFullGrown((fullGrowthTime + 1) / 2);	// At least 1.
				}
				npc->SetStoryInstance(storyInstance);
				npc->ConnectSignals();
				npc->Create();
				npc->SetPosAndOrient(ap);
				ether->AddAtom(npc);
				g_Au.AtomAdded(npc->Id());

				if(world)
					npc->SetHomeWorld(world);

				npc->ResetHitPoints();
				npc->ResetHunger();
				npc->ResetStamina();
				npc->Stand();
				npc->GoToWork();
				npc->SetInMotion();
				npc->SetInScene();
				npc->SetNormal();	// New atom is ready for normal thread processing.
				npc->TickTock();
			}

			AuId charId(npc->Id());

			npc->SetEmpireId(empireId);

			// Remember the character in the person's memory so they can be recalled later with the same information.
			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
						npc->Handedness(), npc->GrowthStage(), empireId, npc->LockGearForRead(), world));
			npc->UnlockGear();
			delete dna;

			evalResult = QVariant::fromValue(charId.GetID());
		}
		break;

	case F_ShipCreateCrew:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			AuDNABase* dna = DNAParameterNew(m_Parms[pidx++], person);
			if(!dna)
				break;

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
					{
						int berths = ship->CrewBerthsAvailable();
						while(berths--)
							ship->LoadPirateCrew(ship->EmpireId(), dna->Species());
					}
				}
			}
			delete dna;
		}
		break;

	case F_ShipCreateTroops:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			AuDNABase* dna = DNAParameterNew(m_Parms[pidx++], person);
			if(!dna)
				break;

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
					{
						int berths = ship->TroopBerthsAvailable();
						while(berths--)
							ship->LoadPirateTroop(ship->EmpireId(), dna->Species());
					}
				}
			}
			delete dna;
		}
		break;

	case F_ShipInteriorLight:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			int state = m_Parms[pidx++].toInt();

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
					{
						if(state == 0)
							ship->InteriorLightToggle(INTLIGHT_Dark);
						else if(state == 1)
							ship->InteriorLightToggle(INTLIGHT_Bright);
						else
							ship->InteriorLightToggle(INTLIGHT_Dim);
					}
				}
			}
		}
		break;

	case F_ShipManeuverDrive:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			int state = m_Parms[pidx++].toInt();

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
					{
						if(state == 0)
							ship->SetGravityDriveState(DaVessel::GD_Off);
						else if(state == 1)
							ship->SetGravityDriveState(DaVessel::GD_On);
						else
							ship->SetGravityDriveState(DaVessel::GD_Hold);
					}
				}
			}
		}
		break;

	case F_ShipPowerPlant:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			int state = m_Parms[pidx++].toInt();

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
					{
						if(state)
							ship->TurnPowerPlantOn();
						else
							ship->TurnPowerPlantOff();
					}
				}
			}
		}
		break;

	case F_ShipRefuel:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			quint8 quality = m_Parms[pidx++].toUInt();

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
						ship->Refuel(quality);
				}
			}
		}
		break;

	case F_ShipRetreat:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
					{
						ship->Retreat();
						ship->MissionSetActive(true);
					}
				}
			}
		}
		break;

	case F_ShipShields:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// ship variable name
			AuId u0id = VariableAtomId(u0, vars);
			int state = m_Parms[pidx++].toInt();

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVessel* ship = dynamic_cast<DaVessel*>(env->FindIdRecursive(u0id));
					if(ship && ship->IsNormal())
						ship->SetShieldState((DaVessel::SHIELD_STATE)state);
				}
			}
		}
		break;

	case F_VehicleRefuel:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// vehicle variable name
			AuId u0id = VariableAtomId(u0, vars);

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(env->FindIdRecursive(u0id));
					if(vehicle && vehicle->IsNormal())
						vehicle->Refuel();
				}
			}
		}
		break;

	case F_StoryDroppable:
		storyCancelOk = m_Parms[0].toBool();
		break;

	case F_WorldScheduleAsteroid:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			int mins = m_Parms[pidx++].toInt();	// world variable name
			WorldScheduleAsteroid(person, u0id, mins);
		}
		break;

	case F_WorldCreateCharacter:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			QString name(m_Parms[pidx++].toString());						// Character Name
			GENDER gender = (GENDER)m_Parms[pidx++].toInt();
			HANDEDNESS handedness = (HANDEDNESS)m_Parms[pidx++].toInt();
			qint32 empireSpec = m_Parms[pidx++].toInt();
			int dnaparm = pidx++;
			GROWTHSTAGE stage = (GROWTHSTAGE)m_Parms[pidx++].toInt();

			// First check to see if the character is already in the environment.
			// We'll move their atom if they are found.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.

			// Locate the world.
			DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			AuDNABase* dna = DNAParameterNew(m_Parms[dnaparm], person, world);
			if(!dna)
				break;

			// Location
			AuPoint loc = world->LocOfLL(ll);
			AuPoint vUp = -world->GravityDir(loc);
			AuMatrix4x4 t;
			t.Translate(loc);
			t *= AuPoint(0,0,1).GetAlignmentTransform(vUp);
			AuAtomPos ap(world->Id(), t);

			// Empire Id
			quint32 empireId = EmpireId(empireSpec, person);

			DaNpc* npc = RestoreNpcFromMemory(person, storyInstance, env, name, ap, AuCommStdChannel::CloseRangeDistance());
			if(!npc)
			{
				// Prepare to spawn a nu character.
				// Spawn the atom in the environment, relative to the same parent as the atom.
				npc = new DaNpc(ROLE_Citizen, *dna, gender == Male ? AuDNABase::Alpha : AuDNABase::Beta, handedness, name);
				if(stage == Juvenile)
				{
					// DaNpc::GrowNaturally does not change m_MinutesToFullGrown for story characters. Whatever size I pick here is
					// my size until it is assertively changed.
					quint8 fullGrowthTime = npc->GrowthTime();
					npc->SetMinutesToFullGrown((fullGrowthTime + 1) / 2);	// At least 1.
				}
				npc->SetStoryInstance(storyInstance);
				npc->ConnectSignals();
				npc->Create();
				npc->SetPosAndOrient(ap);
				ether->AddAtom(npc);
				g_Au.AtomAdded(npc->Id());

				npc->SetHomeWorld(world);

				npc->ResetHitPoints();
				npc->ResetHunger();
				npc->ResetStamina();
				npc->Stand();
				npc->GoToWork();
				npc->SetInMotion();
				npc->SetInScene();
				npc->SetNormal();	// New atom is ready for normal thread processing.
				npc->TickTock();
			}

			AuId charId(npc->Id());

			npc->SetEmpireId(empireId);

			if(g_Au.m_LogMessages & LOG_Story)
			{
				QString logmsg = QObject::tr("AuScriptLine::Execute %1 created npc %2 %3")
					.arg(storyInstance)
					.arg(npc->Id().GetID())
					.arg(npc->Name());
				g_Au.Log(logmsg);
			}

			// Remember the character in the person's memory so they can be recalled later with the same information.
			person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), script.Id(), npc->Haircut(), npc->Shave(),
						npc->Handedness(), npc->GrowthStage(), empireId, npc->LockGearForRead(), world));
			npc->UnlockGear();
			delete dna;

			evalResult = QVariant::fromValue(charId.GetID());
		}
		break;

	case F_WorldCreateIndigenous:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			if(u0id.IsNull())
				break;
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			int dnaparm = pidx++;

			// First check to see if the character is already in the environment.
			// We'll move their atom if they are found.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.

			// Locate the world.
			DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
			if(!world)
				break;

			AuDNABase* dna = DNAParameterNew(m_Parms[dnaparm], person, world);
			if(!dna)
				break;

			world->CreateIndigenousVillage(ll, dna->Species());
			delete dna;
		}
		break;

	case F_WorldCreateItem:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			bool isPermanent = m_Parms[pidx++].toBool();
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toInt();
			qint32 empireOfOrigin = m_Parms[pidx++].toInt();
			QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

			AuId itemId;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
					if(world && world->IsNormal())
					{
						quint32 empireId = EmpireId(empireOfOrigin, person);

						// Create the item out of thin air.
						AuItem* newItem = AuItem::Create(cid, quality, empireId);
						if(newItem)
						{
							// Inscribe the nu item if an inscription was provided.
							if(!inscription.isEmpty())
								newItem->Inscribe(inscription);

							// We wouldn't create a light if it wasn't supposed to be on...so far anyway.
							// This also applies to beacons.
							if(newItem->IsLight())
								newItem->SetState(1);

							// An item with a story instance gets deleted when the story is deleted.
							if(!isPermanent)
								newItem->SetStoryInstance(storyInstance);	// Item is not permanent.

							// Add the item to the character's gear.
							DaItem* item = world->CreateItem(ll, *newItem);
							delete newItem;

							if(item)
								itemId = item->Id();
						}
					}
				}
			}
			evalResult = QVariant::fromValue(itemId.GetID());
		}
		break;

	case F_WorldCreateShip:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			double altAGL = m_Parms[pidx++].toDouble() * FeetPerMeter();
			quint32 designId = m_Parms[pidx++].toUInt();
			quint8 quality = m_Parms[pidx++].toInt();
			qint32 owningEmpire = m_Parms[pidx++].toInt();
			QString name = m_Parms[pidx++].toString();

			AuId shipId;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
					if(world && world->IsNormal())
					{
						quint32 empireId = EmpireId(owningEmpire, person);

						// Create the vehicle out of thin air on the world.
						DaVessel* ship = world->CreateSpacecraft(ll, altAGL, designId, quality, empireId, name);
						if(ship)
						{
							shipId = ship->Id();
							//qDebug() << "AuStory::Exec created spacecraft" << ship->Id() << ship->Name() << "at" << ll << altAGL;
						}
					}
				}
			}
			evalResult = QVariant::fromValue(shipId.GetID());
		}
		break;

	case F_WorldCreateVehicle:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
			quint8 quality = m_Parms[pidx++].toInt();
			qint32 owningEmpire = m_Parms[pidx++].toInt();

			AuId vehicleId;
			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
					if(world && world->IsNormal())
					{
						quint32 empireId = EmpireId(owningEmpire, person);

						// Create the vehicle out of thin air on the world.
						DaVehicle* vehicle = world->CreateVehicle(ll, cid, quality, empireId);
						if(vehicle)
							vehicleId = vehicle->Id();
					}
				}
			}
			evalResult = QVariant::fromValue(vehicleId.GetID());
		}
		break;

	case F_WorldHengeCeremony:
		{
			int pidx=0;
			QString u0 = m_Parms[pidx++].toString();	// world variable name
			AuId u0id = VariableAtomId(u0, vars);
			QString u1 = m_Parms[pidx++].toString();	// ll variable name
			AuGlobeLLF ll = VariableLL(u1, vars);
			int state = m_Parms[pidx++].toInt();

			if(!u0id.IsNull())
			{
				DaAtom* env = person.FindEnvironmentAtom();
				if(env)
				{
					DaWorldLL* world = dynamic_cast<DaWorldLL*>(env->FindIdRecursive(u0id));
					if(world && world->IsNormal())
						world->IndigenousHengeCeremony(ll, state);
				}
			}
		}
		break;

	case F_ChoiceMade:
		// This function can only be processed during Eval.
		break;

	case F_AND:
	case F_Count:
	case F_EQ:
	case F_GE:
	case F_GT:
	case F_GoTo:
	case F_If:
	case F_LE:
	case F_LT:
	case F_NarrHint:	// Not sent to the client unless they ask for it.
	case F_NOP:
	case F_OR:
	case F_StoryAuthor:
	case F_StoryAutoRepeat:
	case F_StoryAutoRepeatUntilComplete:
	case F_StoryAutoStart:
	case F_StoryBrief:
	case F_StoryInterval:
	case F_StoryTitle:
	case F_MiscDocTitle:
	case F_NpcDocTitle:
	case F_StoryDocTitle:
	case F_AvatarDocTitle:
	case F_AtomDocTitle:
	case F_BlueprintDocTitle:
	case F_CharacterDocTitle:
	case F_CityDocTitle:
	case F_CreatureDocTitle:
	case F_FindDocTitle:
	case F_HengeDocTitle:
	case F_ItemDocTitle:
	case F_NarrDocTitle:
	case F_SectorDocTitle:
	case F_SentientDocTitle:
	case F_ShipDocTitle:
	case F_UnitDocTitle:
	case F_VehicleDocTitle:
	case F_WorldDocTitle:
		// These functions don't "do" anything.
		break;

		/*
	case F_CharDrop:
		// Character drops item.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// char
			COMMODITYID cid = (COMMODITYID)m_Parms[1].toInt();						// commodity

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;

			// Drop the item from the character's gear.
			npc->DropCommodity(cid);
		}
		break;
	case F_CharEquip:
		// Character adds item to gear.
		{
			// Get parameter values.
			int pidx=0;
			QString charName(m_Parms[pidx++].toString());								// char
			AuGearSlot::SLOTTYPE slotType = (AuGearSlot::SLOTTYPE)m_Parms[pidx++].toInt();	// gear
			COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();						// commodity
			quint8 quality = m_Parms[pidx++].toInt();								// quality

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;

			// Check to see if the character already has the item.
			const AuGear* ninv = npc->LockGearForRead();
			bool h = ninv->HasItemInSlot(slotType, cid);
			npc->UnlockGear();
			if(h)
				break;											// Character already has the item in that gear slot.

			// Create the item out of thin air.
			AuItem* item = AuItem::Create(cid, quality, npc->EmpireId());
			if(item)
			{
				// We wouldn't give a story character a light if it wasn't supposed to be on...so far anyway.
				if(item->IsLight())
					item->SetState(1);

				// Add the item to the character's gear.
				item->SetStoryInstance(npc->StoryInstance());	// Item created from the ether is not permanent.
				npc->AddGearItemAt(slotType, *item);
				delete item;
			}
		}
		break;

	case F_Character:
		if(!m_Assignee.isEmpty())
		{
			QString charName(m_Parms[0].toString());								// char

			AuId charId;
			DaAtom* env = person.FindEnvironmentAtom();
			if(env)
			{
				DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
				if(npc)
					charId = npc->Id();
			}

			evalResult = QVariant::fromValue(charId.GetID());
		}
		break;

	case F_CharGoToChar:
		// Character goes to the location of another character.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// char
			QString otherChar(m_Parms[1].toString());								// char

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;
			DaNpc* othernpc = env->FindNpcRecursive(otherChar, storyInstance);
			if(!othernpc)
				break;

			// Order the character to go to the person's location.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			npc->TakeOrder(msg, ORDER_GoToTarget, othernpc->Id(), AuPoint(0,0), true);
		}
		break;
	case F_CharGoToPerson:
		// Character goes to the location of the person.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// char

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;

			// Order the character to go to the person's location.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			npc->TakeOrder(msg, ORDER_GoToTarget, person.Id(), AuPoint(0,0), true);
		}
		break;
	case F_CharMoveBack:
		// Character moves backward a distance.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// char
			double distance = m_Parms[1].toDouble();								// distance

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;

			// Order the character to move relative to itself.
			AuId moveTarget = npc->Parent()->Id();
			AuPoint moveLoc = npc->Transform().Project(0, -distance, 0);
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			npc->TakeOrder(msg, ORDER_GoToTargetExactly, moveTarget, moveLoc, true);
		}
		break;
	case F_CharMoveForward:
		// Character moves forward a distance.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// char
			double distance = m_Parms[1].toDouble();								// distance

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;

			// Order the character to move relative to itself.
			AuId moveTarget = npc->Parent()->Id();
			AuPoint moveLoc = npc->Transform().Project(0, distance, 0);
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			npc->TakeOrder(msg, ORDER_GoToTargetExactly, moveTarget, moveLoc, true);
		}
		break;
	case F_CharSay:
		// Character says something on the Voice channel.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// Character Name
			QString text = StringParameter(m_Parms[1].toString(), person, vars);

			// Send the comm message.
			CharSendComm(person, storyInstance, text, AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Close, charName);
		}
		break;
	case F_CharSleep:
		// Character sleeps for a short time.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// char

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;

			// Order the character to stay put.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			npc->TakeOrder(msg, ORDER_Sleep, AuId(0), AuPoint(0,0), true);
		}
		break;

	case F_CharEnterSurgery:
		// Character goes to the nearest surgery unit.
		{
			// Get parameter values.
			QString charName(m_Parms[0].toString());								// char

			// Locate the character in the person's environment.
			DaAtom* env = person.FindEnvironmentAtom();
			if(!env)
				break;	// This should never happen.
			DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
			if(!npc)
				break;

			// Order the character to go to the nearest surgery unit and get into the unit.
			AuCommChat msg;	// Not used by DaNpc::TakeOrder.
			npc->TakeOrder(msg, ORDER_EnterSurgery, AuId(0), AuPoint(0,0), true);
		}
		break;
		*/
	}

	// Make the assignment.
	if(!m_Assignee.isEmpty())
		vars.insert(m_Assignee, evalResult);
}

// Resends this script line.
// Does not perform if() evaluations.
void AuScriptLine::Resend(const AuScript& script, quint32 storyInstance, DaPerson& person, QMap<QString, QVariant>& vars,
		const AuTime& blockStart, bool& storyCancelOk) const
{
	// Execute the function to resend its results, such as communications and conversation.
	// Some script functions must not be executed more than once. e.g. function calls that create objects or produce random results.
	if(!s_StoryFunc[m_Func].UniverseDynamicOnce() && !s_StoryFunc[m_Func].UniverseConstOnce())
		Execute(script, storyInstance, person, vars, blockStart, storyCancelOk);
}

bool AuScriptLine::SendHint(const AuScript& script, quint32 storyInstance, DaPerson& person,
		const QMap<QString, QVariant>& vars) const
{
	if(m_Func != F_NarrHint)
		return false;

	// Get parameter values.
	QString text = StringParameter(m_Parms[0].toString(), person, vars);									// Message Text

	// Send the comm message.
	NarratorSendComm(person, storyInstance, text, script.Title());
	return true;
}

//static
bool AuScriptLine::DNALoadFile(const QString& fname, AuDNA& dna)
{
	return dna.LoadFile(fname) == 0;
}

//static
bool AuScriptLine::DNALoadFile(const QString& fname, DhDNA& dna)
{
	// Parse the DNA string. It was already validated by the compiler.
	QFile file(fname);
	if(!file.open(QFile::ReadOnly))
		return false;

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_4_1);

	bool bMale = false;
	bool bLefty = false;

	// File Type Byte
	quint8 chrFileType;
	ds >> chrFileType;
	if(chrFileType == CHR_FILE_TYPE)
	{
		// Version Byte
		quint8 chrFileVersion;
		ds >> chrFileVersion;

		// Character Data
		QByteArray baDNA;
		ds >> baDNA;
		dna = DhDNA(baDNA);

		ds >> bMale;
		ds >> bLefty;
	}
	else
	{
		// Old 'Jacob' format file.
		file.seek(0);		// Restore file pos back to beginning.
		QByteArray baFileData = file.read(sizeof(DhDNA_));
		dna = DhDNA(baFileData);
		QByteArray ba = file.read(sizeof(bool)*2);
		QDataStream ds(&ba, QIODevice::ReadOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds >> bMale;
		ds >> bLefty;

		// Update size values for DNA changes that doubled length and thickness values. D.H.H. 12/22/2010
		if(dna.CalfLength*2 > 31)
			dna.CalfLength = 31;
		else
			dna.CalfLength *= 2;

		if(dna.CalfThickness*2 > 31)
			dna.CalfThickness = 31;
		else
			dna.CalfThickness *= 2;

		if(dna.ForeArmLength*2 > 31)
			dna.ForeArmLength = 31;
		else
			dna.ForeArmLength *= 2;

		if(dna.ForeArmThickness*2 > 31)
			dna.ForeArmThickness = 31;
		else
			dna.ForeArmThickness *= 2;

		if(dna.FootLength*2 > 31)
			dna.FootLength = 31;
		else
			dna.FootLength *= 2;

		if(dna.HandLength*2 > 31)
			dna.HandLength = 31;
		else
			dna.HandLength *= 2;

		if(dna.OverallSize*2 > 31)
			dna.OverallSize = 31;
		else
			dna.OverallSize *= 2;

		if(dna.ThighLength*2 > 31)
			dna.ThighLength = 31;
		else
			dna.ThighLength *= 2;

		if(dna.ThighThickness*2 > 31)
			dna.ThighThickness = 31;
		else
			dna.ThighThickness *= 2;

		if(dna.TorsoSegmentLength*2 > 31)
			dna.TorsoSegmentLength = 31;
		else
			dna.TorsoSegmentLength *= 2;

		if(dna.TorsoThickness*2 > 31)
			dna.TorsoThickness = 31;
		else
			dna.TorsoThickness *= 2;

		if(dna.UpperArmLength*2 > 31)
			dna.UpperArmLength = 31;
		else
			dna.UpperArmLength *= 2;

		if(dna.UpperArmThickness*2 > 31)
			dna.UpperArmThickness = 31;
		else
			dna.UpperArmThickness *= 2;
	}

	file.close();
	return true;
}

//static
AuDNABase* AuScriptLine::DNAParameterNew(const QVariant& parmVar, DaPerson& person, const DaWorldLL* world)
{
	AuDNABase* dna = 0;
	QString parm = parmVar.toString();
	if(!parm.isEmpty())
	{
		if(parm.compare("Avatar", Qt::CaseInsensitive) == 0)
		{
			// DNA matches the person.
		}
		else if(parm.compare("Indigenous", Qt::CaseInsensitive) == 0)
		{
			// DNA is obtained from the indigenous dna of the person's current worker zone on their planet.
			// If the worker DNA hasn't been set, the person's DNA is used.
			if(world && world->HasWorkerSpecies())
				dna = world->WorkerSpecies().CreateDNA();	// Returns a clone to me.
			else
			{
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(person.FindParentPlanetInEnv());
				if(llworld && llworld->HasWorkerSpecies())
					dna = llworld->WorkerSpecies().CreateDNA();	// Returns a clone to me.
			}
		}
		else if(parm.compare("Worker", Qt::CaseInsensitive) == 0)
		{
			// DNA is obtained from the worker dna of the person's current worker zone on their planet.
			const DaWorld* planet = person.FindParentPlanetInEnv();
			if(planet)
			{
				// The person is at a planet.
				if(person.BdId().IsNull())
				{
					// Person is not in a city.
					AuPoint planetLoc;
					person.GetLocationRelativeToParent(planetLoc, planet);
					Q_ASSERT(!planetLoc.IsNaN());
					dna = planet->WorkerSpecies().CreateDNA();	// The planet returns a clone if it has worker DNA.
				}
				else
				{
					// Person is in a city. Get worker DNA from the city because my current geographical area may be in
					// another zone that has invalid DNA.
					const DaAtom* atom = planet->FindSubatomId(person.BdId());
					if(atom)
					{
						const DaBd* bd = dynamic_cast<const DaBd*>(atom);
						if(bd)
						{
							AuSpecies species = bd->ResidentSpecies();
							dna = species.CreateDNA();
						}
					}
				}
			}
		}
	}
	else
	{
		// The parmVar better be a species.
		AuSpecies species = parmVar.value<AuSpecies>();
		dna = species.CreateDNA();
	}

	// There are various legitimate ways to arrive at this point with no dna.
	if(!dna)
		dna = person.DNAnew();	// Return a clone to the person.
	return dna;
}

//static
bool AuScriptLine::ValidateStringParameter(const QString& parm, const QStringList& varDefs)
{
	// Check for the presence of fields.
	if(parm.indexOf('[') < 0)
		return true;	// No fields in the string. This also catches the empty string case.

	//qDebug() << "AuScriptLine::StringParameter string with fields" << parm;
	// Our fields look like this [Var] or [Var.Field]. e.g. [P] or [P.Name] where P is an atom variable name.
	QString remainder(parm);
	do
	{
		int fieldIdx = remainder.indexOf('[');
		if(fieldIdx >= 0)
		{
			// Extract the leading part of the remainder, up to the field.
			if(fieldIdx > 0)
			{
				remainder = remainder.mid(fieldIdx);
				fieldIdx = 0;
			}

			int endIdx = remainder.mid(fieldIdx+1).indexOf(']') + (fieldIdx+1);	// Index of closing bracket.
			if(endIdx > fieldIdx)
			{
				// Extract the entire field including the brackets. e.g. [P.Name]
				QString field = remainder.mid(fieldIdx, endIdx-fieldIdx+1);

				// Drop the field from the remainder.
				remainder = remainder.mid(endIdx+1);

				// Expand the field.
				field = field.mid(1, field.length()-2);							// Drop the brackets.
				int dotIdx = field.indexOf('.');
				if(dotIdx >= 0)
				{
					// The field contains a dot.
					QString varname = field.left(dotIdx);						// Variable name, e.g. P
					if(!varDefs.contains(varname))
						return false;
				}
				else if(!varDefs.contains(field))	// No dot in the field. Treat the entire field as a variable name.
					return false;
			}
			else
				break;	// No closing bracket.
		}
		else
			break;	// No opening bracket found.
	}
	while(!remainder.isEmpty());
	return true;
}

//static
QString AuScriptLine::StringParameter(const QString& parm, DaPerson& person, const QMap<QString, QVariant>& vars)
{
	// Check for the presence of fields.
	if(parm.indexOf('[') < 0)
		return parm;	// No fields in the string. This also catches the empty string case.

	//qDebug() << "AuScriptLine::StringParameter string with fields" << parm;
	// Our fields look like this [Var] or [Var.Field]. e.g. [P] or [P.Name] where P is an atom variable name.
	QString remainder(parm);
	QString expanded;
	do
	{
		int fieldIdx = remainder.indexOf('[');
		if(fieldIdx >= 0)
		{
			// Extract the leading part of the remainder, up to the field.
			if(fieldIdx > 0)
			{
				expanded += remainder.left(fieldIdx);
				remainder = remainder.mid(fieldIdx);
				fieldIdx = 0;
			}

			int endIdx = remainder.mid(fieldIdx+1).indexOf(']') + (fieldIdx+1);	// Index of closing bracket.
			if(endIdx > fieldIdx)
			{
				// Extract the entire field including the brackets. e.g. [P.Name]
				QString field = remainder.mid(fieldIdx, endIdx-fieldIdx+1);

				// Drop the field from the remainder.
				remainder = remainder.mid(endIdx+1);

				// Expand the field.
				field = field.mid(1, field.length()-2);							// Drop the brackets.
				int dotIdx = field.indexOf('.');
				if(dotIdx >= 0)
				{
					// The field contains a dot.
					bool fieldExpanded = false;
					QString varname = field.left(dotIdx);						// Variable name, e.g. P
					//qDebug() << "AuScriptLine::StringParameter field" << field << "var" << varname << "vars" << vars;
					AuId u0id = VariableAtomId(varname, vars);
					if(!u0id.IsNull())
					{
						const DaAtom* env = person.FindEnvironmentAtom();
						if(env)
						{
							const DaAtom* atom = env->FindIdRecursive(u0id);
							if(atom)
							{
								QString atomField = field.mid(dotIdx+1);		// Atom field, e.g. Name
								if(atomField == "GlobeLL")
								{
									const DaWorldLL* llworld = atom->FindParentWorldInEnv();
									if(llworld)
									{
										AuGlobeLLF ll(atom->GlobeLL());
										expanded += ll.ToString();
										fieldExpanded = true;
									}
								}
								else if(atomField == "IndustryLL")
								{
									if(atom->InIndustry() != IID_None)
									{
										const DaWorldLL* llworld = atom->FindParentWorldInEnv();
										if(llworld)
										{
											AuGlobeLLF ll(atom->InIndustryLL());
											expanded += ll.ToString();
											fieldExpanded = true;
										}
									}
								}
								else if(atomField == "Name")
								{
									expanded += atom->Name();
									fieldExpanded = true;
								}
								else if(atomField == "NameWithEmpire")
								{
									const DaVessel* ship = dynamic_cast<const DaVessel*>(atom);
									if(ship)
									{
										expanded += ship->NameWithEmpire();
										fieldExpanded = true;
									}
								}
								else if(atomField == "TitleName")
								{
									const DaPerson* person = dynamic_cast<const DaPerson*>(atom);
									if(person)
									{
										expanded += person->TitleName();
										fieldExpanded = true;
									}
								}
								else if(atomField == "herhim")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->HerHim(false);
										fieldExpanded = true;
									}
								}
								else if(atomField == "HerHim")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->HerHim(true);
										fieldExpanded = true;
									}
								}
								else if(atomField == "hershis")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->HersHis(false);
										fieldExpanded = true;
									}
								}
								else if(atomField == "HersHis")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->HersHis(true);
										fieldExpanded = true;
									}
								}
								else if(atomField == "maamsir")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->MaamSir(false);
										fieldExpanded = true;
									}
								}
								else if(atomField == "MaamSir")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->MaamSir(true);
										fieldExpanded = true;
									}
								}
								else if(atomField == "momdad")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->MomDad(false);
										fieldExpanded = true;
									}
								}
								else if(atomField == "MomDad")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->MomDad(true);
										fieldExpanded = true;
									}
								}
								else if(atomField == "motherfather")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->MotherFather(false);
										fieldExpanded = true;
									}
								}
								else if(atomField == "MotherFather")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->MotherFather(true);
										fieldExpanded = true;
									}
								}
								else if(atomField == "MsMr")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->MsMr();
										fieldExpanded = true;
									}
								}
								else if(atomField == "niecenephew")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->NieceNephew(false);
										fieldExpanded = true;
									}
								}
								else if(atomField == "NieceNephew")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->NieceNephew(true);
										fieldExpanded = true;
									}
								}
								else if(atomField == "shehe")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->SheHe(false);
										fieldExpanded = true;
									}
								}
								else if(atomField == "SheHe")
								{
									const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
									if(creature)
									{
										expanded += creature->SheHe(true);
										fieldExpanded = true;
									}
								}
							}
						}
					}

					if(!fieldExpanded)
						expanded += QString("[%1]").arg(field);	// Field could not be expanded. Keep the field intact.
				}
				else
				{
					// No dot in the field. Treat the entire field as a variable name.
					QMap<QString, QVariant>::const_iterator ui = vars.find(field);
					if(ui != vars.cend())
					{
						if((*ui).canConvert<AuGlobeLLF>())
						{
							AuGlobeLLF ll((*ui).value<AuGlobeLLF>());
							//qDebug() << "field" << field << "ll" << ll << "tostr" << ll.ToString();
							expanded += ll.ToString();
						}
						else if((*ui).canConvert<AuId>())
						{
							AuId id((*ui).value<AuId>());
							expanded += id.StringId();
						}
						else
							expanded += (*ui).toString();
					}
					else
						expanded += QString("[%1]").arg(field);	// Field could not be expanded. Keep the field intact.
				}
			}
			else
			{
				// No closing bracket. Keep the entire remainder and quit.
				expanded += remainder;
				break;
			}
		}
		else
		{
			// No opening bracket found. Append the entire remainder and quit.
			expanded += remainder;
			break;
		}
	}
	while(!remainder.isEmpty());
	//qDebug() << "AuScriptLine::StringParameter expanded" << expanded;
	return expanded;
}

//static
AuId AuScriptLine::VariableAtomId(const QString& varName, const QMap<QString, QVariant>& vars)
{
	QMap<QString, QVariant>::const_iterator ui = vars.find(varName);
	if(ui == vars.cend())
	{
		qWarning() << "error: AuScriptLine::VariableAtomId undefined variable:" << varName;
		return AuId();
	}
	//qDebug() << "AuScriptLine::VariableAtomId var" << varName << "id" << ui.value().value<AuId>();
	AuId id((*ui).toUInt());
	if(id.IsNull() && (g_Au.m_LogMessages & LOG_Story))
		qDebug() << "error: AuScriptLine::VariableAtomId var" << varName << "id is null";	// Can be null just for testing id.
	return id;
}

//static
AuGlobeLLF AuScriptLine::VariableLL(const QString& varName, const QMap<QString, QVariant>& vars)
{
	QMap<QString, QVariant>::const_iterator ui = vars.find(varName);
	if(ui == vars.cend())
	{
		qWarning() << "error: AuScriptLine::VariableLL undefined variable:" << varName;
		return AuGlobeLLF();
	}
	//qDebug() << "AuScriptLine::VariableLL var" << varName << "ll" << ui.value().value<AuGlobeLLF>();
	return ui.value().value<AuGlobeLLF>();
}

//static
QString AuScriptLine::VariableString(const QString& varName, const QMap<QString, QVariant>& vars)
{
	QMap<QString, QVariant>::const_iterator ui = vars.find(varName);
	if(ui == vars.cend())
	{
		qWarning() << "error: AuScriptLine::VariableString undefined variable:" << varName;
		return QString();
	}
	//qDebug() << "AuScriptLine::VariableString var" << varName << "id" << ui.value().value<AuId>();
	return (*ui).toString();
}

//static
bool AuScriptLine::CompareStance(POLITICAL_STANCE stanceOfUnit, POLITICAL_STANCE stanceParameter, int stanceCheck)
{
	bool evalResult = false;
	switch(stanceCheck)
	{
	default:
		// 'stanceOfUnit' must match 'stanceParameter'.
		if(stanceOfUnit == stanceParameter)
			evalResult = true;
		break;
	case 1:
		// 'stanceOfUnit' must match or be better than 'stanceParameter'.
		switch(stanceOfUnit)
		{
		case POL_Empire:
			evalResult = true;
			break;
		case POL_Vassal:
			if(stanceParameter == POL_Vassal ||
					stanceParameter == POL_Friend ||
					stanceParameter == POL_Neutral ||
					stanceParameter == POL_Enemy)
				evalResult = true;
			break;
		case POL_Friend:
			if(stanceParameter == POL_Friend ||
					stanceParameter == POL_Neutral ||
					stanceParameter == POL_Enemy)
				evalResult = true;
			break;
		case POL_Neutral:
			if(stanceParameter == POL_Neutral ||
					stanceParameter == POL_Enemy)
				evalResult = true;
			break;
		case POL_Enemy:
			if(stanceParameter == POL_Enemy)
				evalResult = true;
			break;
		}
		break;
	case 2:
		// 'stanceOfUnit' must match or be worse than 'stanceParameter'.
		switch(stanceOfUnit)
		{
		case POL_Empire:
			if(stanceParameter == POL_Empire)
				evalResult = true;
			break;
		case POL_Vassal:
			if(stanceParameter == POL_Empire ||
					stanceParameter == POL_Vassal)
				evalResult = true;
			break;
		case POL_Friend:
			if(stanceParameter == POL_Empire ||
					stanceParameter == POL_Vassal ||
					stanceParameter == POL_Friend)
				evalResult = true;
			break;
		case POL_Neutral:
			if(stanceParameter == POL_Empire ||
					stanceParameter == POL_Vassal ||
					stanceParameter == POL_Friend ||
					stanceParameter == POL_Neutral)
				evalResult = true;
			break;
		case POL_Enemy:
			evalResult = true;
			break;
		}
		break;
	}
	return evalResult;
}

//static
quint32 AuScriptLine::EmpireId(qint32 empireSpec, const DaPerson& person)
{
	quint32 empireId;
	if(empireSpec == 0)
	{
		// Person's empire.
		empireId = person.EmpireId();
	}
	else if(empireSpec == -1)
	{
		// Any vassal
		empireId = person.EmpireId();	//DHH
	}
	else if(empireSpec == -2)
	{
		// Any friend
		empireId = person.EmpireId();	//DHH
	}
	else if(empireSpec == -3)
	{
		// Any neutral
		empireId = person.EmpireId();	//DHH
	}
	else if(empireSpec == -4)
	{
		// Any enemy
		empireId = person.EmpireId();	//DHH
	}
	else if(empireSpec == -5)
	{
		// No empire
		empireId = 0;
	}
	else
	{
		// Empire id was specified and validated by the compiler.
		empireId = empireSpec;
	}
	return empireId;
}

//static
AuMatrix4x4 AuScriptLine::FindStartingLocationAboard(const DaVessel* ship, int placement)
{
	bool destFound;
	AuPoint destLoc;
	if(placement == 1)
		destFound = ship->TroopOrCrewVoidLocation(destLoc);
	else if(placement == 2)
		destFound = ship->ColonistVoidLocation(destLoc);
	else
		destFound = ship->TransporterVoidLocation(destLoc);
	//qDebug() << "AuScriptLine::FindStartingLocationAboard ship" << ship->Id() << ship->Name() << "destFound" << destFound
		//<< "loc" << destLoc;
	if(!destFound)
		destLoc = ship->HelmLocation();
	
	AuMatrix4x4 t;
	t.Translate(destLoc);
	return t;
}

//static
AuMatrix4x4 AuScriptLine::FindStartingLocation(const DaAtom* refAtom, float angle, double distance)
{
	//qDebug() << "AuScriptLine::FindStartingLocation refAtom" << *refAtom << "angle" << (angle*RAD2DEG) << "distFt" << distance;
	// Determine the destination location.
	AuPoint destLoc = AuPoint(0,distance,0).RotateZ(angle);		// Relative to refAtom.
	AuPoint destLocPar = refAtom->Transform().Project(destLoc);	// Relative to refAtom's parent.
	AuPoint refLoc = refAtom->Location();						// Relative to refAtom's parent.

	// Test the path from the reference atom to the desired location.
	DaAtom* parent = refAtom->Parent();
	AuPoint hitLoc, hitNormal;
	if(parent->IsPathBlocked(refLoc, destLocPar, hitLoc, hitNormal, refAtom) ||
			parent->CheckChildDeparture(refLoc, destLocPar, refAtom))
	{
		//qDebug() << "AuScriptLine::FindStartingLocation refAtom" << *refAtom
			//<< "refLoc" << refLoc
			//<< "destLocPar" << destLocPar
			//<< "IsPathBlocked" << parent->IsPathBlocked(refLoc, destLocPar, hitLoc, hitNormal, refAtom)
			//<< "CheckChildDeparture" << parent->CheckChildDeparture(refLoc, destLocPar, refAtom);
		// The path to the destination location is blocked or it exits the domain of the parent atom.
		// Move destLocPar closer until we find an unobstructed spot, even if it's the refAtom's location.
		AuPoint destDirPar = (destLocPar - refLoc) / distance;	// Unit vector in the space of the parent.
		do
		{
			if(distance > 1.)
			{
				distance *= .9;
				destLocPar = refLoc + destDirPar * distance;	// Relative to refAtom's parent.
				//qDebug() << "AuScriptLine::FindStartingLocation calculated new loc" << destLocPar << "dist" << distance;
			}
			else
			{
				destLocPar = refLoc;
				//qDebug() << "AuScriptLine::FindStartingLocation using refLoc" << refLoc;
				break;
			}
		}
		while(parent->IsPathBlocked(refLoc, destLocPar, hitLoc, hitNormal, refAtom) ||
				parent->CheckChildDeparture(refLoc, destLocPar, refAtom));
	}

	destLoc = refAtom->Transform().UnProject(destLocPar);		// Relative to refAtom.
	//qDebug() << "AuScriptLine::FindStartingLocation destLoc" << destLoc;

	AuMatrix4x4 t = refAtom->Transform();	// Start with the person's matrix.
	t.Translate(destLoc);					// Move to the destination location.
	t.RotateZ(angle + PI);					// Rotate by the angle specified then about-face, to face the refAtom.
	return t;
}

void AuScriptLine::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(!m_Assignee.isEmpty())
		hasWhat |= 0x01;
	if(m_Negate)
		hasWhat |= 0x02;
	if(!m_Parms.empty())
		hasWhat |= 0x04;

	out << hasWhat;
	out << (quint16)m_Func;

	if(hasWhat & 0x01)
		out << m_Assignee;

	if(hasWhat & 0x04)
	{
		out << (quint8)m_Parms.size();
		for(QList<QVariant>::const_iterator pi = m_Parms.cbegin(); pi != m_Parms.cend(); pi++)
			out << *pi;
	}
}

void AuScriptLine::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;
	m_Negate = hasWhat & 0x02;

	quint16 ui16;
	in >> ui16;
	m_Func = (SCRIPT_FUNC)ui16;

	if(hasWhat & 0x01)
		in >> m_Assignee;
	else
		m_Assignee = "";

	m_Parms.clear();
	if(hasWhat & 0x04)
	{
		quint8 ui8;
		in >> ui8;
		while(ui8--)
		{
			QVariant v;
			in >> v;
			m_Parms.append(v);
		}
	}
}

QVariant AuScriptLine::AtomCreateCharacter(DaPerson& person, const QMap<QString, QVariant>& vars, unsigned int scriptId,
		quint32 storyInstance) const
{
	AuEther* ether = person.Ether();
	if(!ether)
		return 0;
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// unit variable name
	AuId u0id = VariableAtomId(u0, vars);
	if(u0id.IsNull())
		return 0;
	float angle = m_Parms[pidx++].toFloat()*DEG2RADf;							// Angle relative to atom's facing.
	double distance = m_Parms[pidx++].toDouble() * FeetPerMeter();				// Distance from atom.
	QString name(m_Parms[pidx++].toString());									// Character Name
	GENDER gender = (GENDER)m_Parms[pidx++].toInt();
	HANDEDNESS handedness = (HANDEDNESS)m_Parms[pidx++].toInt();
	qint32 empireSpec = m_Parms[pidx++].toInt();
	AuDNABase* dna = DNAParameterNew(m_Parms[pidx++], person);
	if(!dna)
		return 0;
	GROWTHSTAGE stage = (GROWTHSTAGE)m_Parms[pidx++].toInt();

	// First check to see if the character is already in the environment.
	// We'll delete the old atom if they are found.
	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
	{
		delete dna;
		return 0;	// This should never happen.
	}

	// Locate the reference atom.
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
	{
		delete dna;
		return 0;
	}
	if(atom->DoesQueryEnvironment() && atom->IsAboardVehicle())
	{
		delete dna;
		return 0;
	}

	// Location
	AuMatrix4x4 t = FindStartingLocation(atom, angle, distance);

	AuAtomPos ap;
	if((atom->DoesQueryEnvironment() && atom->IsAboardVehicle()) || atom->IsRider())
	{
		// Add the npc at the same parent atom as the atom's parent atom.
		t = atom->Parent()->Transform() * t;
		ap = AuAtomPos(atom->Parent()->ParentId(), t);
	}
	else
	{
		// Add the npc to the same parent atom as the person.
		ap = AuAtomPos(atom->ParentId(), t);
	}

	// Empire Id
	quint32 empireId = EmpireId(empireSpec, person);

	const DaWorld* homeWorld = person.FindParentPlanetInEnv();	// Can be 0

	DaNpc* npc = RestoreNpcFromMemory(person, storyInstance, env, name, ap, AuCommStdChannel::CloseRangeDistance());
	if(!npc)
	{
		// Prepare to spawn a nu character.
		// Spawn the atom in the environment, relative to the same parent as the atom.
		npc = new DaNpc(ROLE_Citizen, *dna, gender == Male ? AuDNABase::Alpha : AuDNABase::Beta, handedness, name);
		if(stage == Juvenile)
		{
			// DaNpc::GrowNaturally does not change m_MinutesToFullGrown for story characters. Whatever size I pick here is
			// my size until it is assertively changed.
			quint8 fullGrowthTime = npc->GrowthTime();
			npc->SetMinutesToFullGrown((fullGrowthTime + 1) / 2);	// At least 1.
		}
		npc->SetStoryInstance(storyInstance);
		npc->ConnectSignals();
		npc->Create();
		npc->SetPosAndOrient(ap);
		ether->AddAtom(npc);
		g_Au.AtomAdded(npc->Id());

		if(homeWorld)
			npc->SetHomeWorld(homeWorld);

		npc->ResetHitPoints();
		npc->ResetHunger();
		npc->ResetStamina();
		npc->Stand();
		npc->GoToWork();
		npc->SetInMotion();
		npc->SetInScene();
		npc->SetNormal();	// New atom is ready for normal thread processing.
		npc->TickTock();
	}

	AuId charId(npc->Id());

	npc->SetEmpireId(empireId);

	// Remember the character in the person's memory so they can be recalled later with the same information.
	// 'homeWorld' may be 0.
	person.Remember(AuMemCharacterDNA4(npc->Name(), *dna, npc->Chromatid(), scriptId, npc->Haircut(), npc->Shave(),
				npc->Handedness(), npc->GrowthStage(), empireId, npc->LockGearForRead(), homeWorld));
	npc->UnlockGear();
	delete dna;

	return QVariant::fromValue(charId.GetID());
}

DaNpc* AuScriptLine::RestoreNpc(DaPerson& person, quint32 storyInstance, const QString& charName, double range,
		QMap<QString, QVariant>& vars) const
{
	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return 0;	// This should never happen.

	// Check for existing character atom based on atom ids in the vars.
	AuId charId = VariableAtomId(charName, vars);
	if(!charId.IsNull())
	{
		// We already have an atom id for the character. Is it in our environment?
		DaNpc* npc = dynamic_cast<DaNpc*>(env->FindIdRecursive(charId));
		if(npc)
		{
			// Existing character atom was found.
			if(person.DistanceToAtom(npc, env) > range)
			{
				if(g_Au.m_LogMessages & LOG_Story)
				{
					QString logmsg = QObject::tr("AuScriptLine::RestoreNpc moving npc %1, is beyond %2' range")
						.arg(npc->Name())
						.arg(range);
					g_Au.Log(logmsg);
				}
				npc->ChangePosAndOrient(person.Pos());
			}
			return npc;
		}
	}

	// Cannot create a new character without a story instance. This happens during catalyst block tests so its ok.
	if(!storyInstance)
		return 0;

	// Do not restore a character while the person is in a vehicle.
	if(person.IsAboardVehicle())
		return 0;

	// Restore the character based on the person's memory of them.
	DaNpc* npc = RestoreNpcFromMemory(person, storyInstance, env, charName, person.Pos(), range);
	if(!npc)
		return 0;
	if(g_Au.m_LogMessages & LOG_Story)
	{
		DaNpc* nnpc = dynamic_cast<DaNpc*>(env->FindIdRecursive(npc->Id()));
		if(!nnpc)
			qDebug() << "npc" << *npc << "not found after creation";
		QString logmsg = QObject::tr("AuScriptLine::RestoreNpc restored %1 %2 from memory, old %3 not found")
			.arg(npc->Id().GetID())
			.arg(npc->Name())
			.arg(charId.GetID());
		g_Au.Log(logmsg);
	}

	// Update vars with nu atom id.
	vars.insert(charName, QVariant::fromValue(npc->Id().GetID()));
	return npc;
}

DaNpc* AuScriptLine::RestoreNpcFromMemory(DaPerson& person, quint32 storyInstance, DaAtom* env,
		const QString& charName, const AuAtomPos& charPos, double range) const
{
	DaNpc* npc = env->FindNpcRecursive(charName, storyInstance);
	if(npc)
	{
		// Existing character atom was found.
		if(person.DistanceToAtom(npc, env) > range)
			npc->ChangePosAndOrient(charPos);
		return npc;	// Existing character is returned.
	}

	AuMemCharacterDNA4 mem;
	if(!person.RecallCharacter(charName, mem))
		return 0;	// Person has no memory of the character.

	// Recreate the character from memory.
	npc = new DaNpc(ROLE_Citizen, *mem.DNA(), (AuDNABase::CHROMATID)mem.Chromatid(), mem.Hand(), charName);
	if(mem.GrowthStage() == Juvenile)
	{
		// DaNpc::GrowNaturally does not change m_MinutesToFullGrown for story characters. Whatever size I pick here is
		// my size until it is assertively changed.
		quint8 fullGrowthTime = npc->GrowthTime();
		npc->SetMinutesToFullGrown((fullGrowthTime + 1) / 2);	// At least 1.
	}
	npc->SetStoryInstance(storyInstance);
	npc->ConnectSignals();
	npc->Create();
	npc->SetPosAndOrient(charPos);
	person.Ether()->AddAtom(npc);
	g_Au.AtomAdded(npc->Id());

	if(mem.Gear())
		npc->SetGear(*mem.Gear());

	npc->SetHomeWorld(mem.HomeWorldId(), mem.HomeWorldName(), mem.HomeWorldBodyType(), mem.HomeSystemId(), mem.HomeSystemName(),
			mem.HomeSectorId(), mem.HomeSectorName(), mem.HomeGalaxy());

	npc->SetHairStyle(mem.Hair(), mem.Shave());
	npc->ResetHitPoints();
	npc->ResetHunger();
	npc->ResetStamina();
	npc->SetEmpireId(mem.EmpireId());
	npc->Stand();
	npc->GoToWork();
	npc->SetInMotion();
	npc->SetInScene();
	npc->SetNormal();	// New atom is ready for normal thread processing.
	npc->TickTock();
	return npc;
}

QVariant AuScriptLine::AtomFindVehicle(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// atom variable name
	AuId u0id = VariableAtomId(u0, vars);
	if(u0id.IsNull())
		return QVariant();
	COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();	// commodity
	int newness = m_Parms[pidx++].toInt();

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant();
	const DaAtom* a0 = env->FindIdRecursive(u0id);
	if(!a0)
		return QVariant();
	DaAtomList vehicles;
	a0->FindVehiclesRecursive(vehicles);
	for(DaAtomList::const_iterator vi = vehicles.cbegin(); vi != vehicles.cend(); vi++)
	{
		DaVehicle* veh = (DaVehicle*)*vi;
		if(veh->CommodityId() != cid)
			continue;
		if(veh->Parent() != a0 && veh->Parent()->Parent() != a0)
			continue;	// We want vehicles that are subordinate to a0 or to a subatom of a0, i.e. a building.
		if(newness == 1)
		{
			// Vehicle must be new.
			if(!veh->IsNew())
				continue;	// It's not new.
		}
		else if (newness == 2)
		{
			// Vehicle must not be new.
			if(veh->IsNew())
				continue;	// It's new.
		}
		return QVariant(veh->Id().GetID());
	}
	return QVariant();
}

QVariant AuScriptLine::AtomFindBuilding(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	QString u0 = m_Parms[0].toString();	// atom variable name
	AuId u0id = VariableAtomId(u0, vars);
	if(u0id.IsNull())
		return QVariant();
	INDUSTRYID iid = (INDUSTRYID)m_Parms[1].toInt();
	quint32 construction = m_Parms[2].toUInt();			// 0=yes, 1=no, 2=ok
	POLITICAL_STANCE stanceParameter = (POLITICAL_STANCE)m_Parms[3].toInt();
	int stanceCheck = m_Parms[4].toInt();

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant();

	const DaAtom* a0 = env->FindIdRecursive(u0id);
	if(!a0)
		return QVariant();

	// Check each bd subordinate to a0.
	DaAtomList bds;
	a0->FindBdRecursive(bds);
	for(DaAtomList::const_iterator ai = bds.cbegin(); ai != bds.cend(); ai++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*ai);
		if(!bd)
			continue;
		if(bd->IndustryId() != iid)
			continue;
		if(bd->IsUnderConstruction())
		{
			if(construction == 0 || construction == 2)
			{
				// Looking for one that is under construction, or it doesn't matter.
				POLITICAL_STANCE stanceOfUnit = PoliticalStance(person.EmpireId(), bd->EmpireId());
				if(CompareStance(stanceOfUnit, stanceParameter, stanceCheck))
					return QVariant(bd->Id().GetID());
			}
		}
		else
		{
			if(construction == 1 || construction == 2)
			{
				// Looking for one that is not under construction, or it doesn't matter.
				POLITICAL_STANCE stanceOfUnit = PoliticalStance(person.EmpireId(), bd->EmpireId());
				if(CompareStance(stanceOfUnit, stanceParameter, stanceCheck))
					return QVariant(bd->Id().GetID());
			}
		}
	}
	return QVariant();
}

QVariant AuScriptLine::AtomHasAtom(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	QString u0 = m_Parms[0].toString();	// atom variable name
	AuId u0id = VariableAtomId(u0, vars);
	if(u0id.IsNull())
		return QVariant(false);
	QString u1 = m_Parms[1].toString();	// unit variable name
	AuId u1id = VariableAtomId(u1, vars);
	if(u1id.IsNull())
		return QVariant(false);

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant(false);

	const DaAtom* a0 = env->FindIdRecursive(u0id);
	if(!a0 || !a0->FindIdRecursive(u1id))
		return QVariant(false);

	// Found atom with u1id subordinate to atom with u0id.
	return QVariant(true);
}

QVariant AuScriptLine::AtomCity(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// bd atom id

	if(u0id.IsNull())
		return QVariant(0);

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant(0);

	const DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom || !atom->IsNormal())
		return QVariant(0);

	if(!atom->DoesQueryEnvironment())
		return QVariant(0);

	return QVariant(atom->InBdCity().GetID());
}

QVariant AuScriptLine::AtomSector(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// unit variable name
	AuId u0id = VariableAtomId(u0, vars);

	if(u0id.IsNull())
		return QVariant(0);

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant(0);

	const DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom || !atom->IsNormal())
		return QVariant(0);

	const DaSector* sector = atom->FindParentSector();
	if(!sector)
		return QVariant(0);

	return QVariant(sector->Id().GetID());
}

QVariant AuScriptLine::AtomWorld(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// unit variable name
	AuId u0id = VariableAtomId(u0, vars);

	if(u0id.IsNull())
		return QVariant(0);

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant(0);

	const DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom || !atom->IsNormal())
		return QVariant(0);

	const DaWorldLL* world = atom->FindParentWorldInEnv();
	if(!world)
		return QVariant(0);

	return QVariant(world->Id().GetID());
}

QVariant AuScriptLine::AtomDistanceToAtom(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	QString u0 = m_Parms[0].toString();	// unit variable name
	AuId u0id = VariableAtomId(u0, vars);
	if(u0id.IsNull())
		return QVariant(0);
	QString u1 = m_Parms[1].toString();	// unit variable name
	AuId u1id = VariableAtomId(u1, vars);
	if(u1id.IsNull())
		return QVariant(0);

	// Locate the person's environment.
	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant(0);	// This should never happen.

	DaAtom* a0 = env->FindIdRecursive(u0id);
	if(!a0)
		return QVariant(0);
	DaAtom* a1 = env->FindIdRecursive(u1id);
	if(!a1)
		return QVariant(0);

	// Find the best common parent for most accurate distance calculation.
	DaAtom* commonParent = a0->CommonParent(a1, env);
	if(!commonParent)
		return QVariant(0);	// We are way too far apart if we're not even in the same environment.

	// Determine each of our locations relative to the common parent atom.
	AuPoint a0loc, a1loc;
	a0->GetLocationRelativeToParent(a0loc, commonParent);
	a1->GetLocationRelativeToParent(a1loc, commonParent);

	return QVariant(a0loc.Distance(a1loc) / FeetPerMeter());	// meters
}

QVariant AuScriptLine::AtomEmpire(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	QString u0 = m_Parms[0].toString();		// atom variable name
	AuId u0id = VariableAtomId(u0, vars);	// atom id
	if(u0id.IsNull())
		return 0;

	// Locate the person's environment.
	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return 0;	// This should never happen.

	DaAtom* a0 = env->FindIdRecursive(u0id);
	if(!a0)
		return 0;

	return QVariant(a0->EmpireId());
}

QVariant AuScriptLine::AtomHasBuilding(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	QString u0 = m_Parms[0].toString();	// atom variable name
	AuId u0id = VariableAtomId(u0, vars);
	if(u0id.IsNull())
		return QVariant(false);
	INDUSTRYID iid = (INDUSTRYID)m_Parms[1].toInt();
	quint32 construction = m_Parms[2].toUInt();			// 0=yes, 1=no, 2=ok
	POLITICAL_STANCE stanceParameter = (POLITICAL_STANCE)m_Parms[3].toInt();
	int stanceCheck = m_Parms[4].toInt();

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant(false);

	const DaAtom* a0 = env->FindIdRecursive(u0id);
	if(!a0)
		return QVariant(false);

	DaAtomList bdsAndCities;
	a0->FindBdRecursive(bdsAndCities);
	if(bdsAndCities.isEmpty())
		return QVariant(false);

	// Check each bd and city subordinate to a0.
	for(DaAtomList::const_iterator ai = bdsAndCities.cbegin(); ai != bdsAndCities.cend(); ai++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*ai);
		if(bd)
		{
			if(bd->IndustryId() != iid)
				continue;

			if(bd->IsUnderConstruction())
			{
				if(construction == 0 || construction == 2)
				{
					// Looking for one that is under construction, or it doesn't matter.
					POLITICAL_STANCE stanceOfUnit = PoliticalStance(bd->EmpireId(), person.EmpireId());
					if(CompareStance(stanceOfUnit, stanceParameter, stanceCheck))
						return QVariant(true);
				}
			}
			else
			{
				if(construction == 1 || construction == 2)
				{
					// Looking for one that is not under construction, or it doesn't matter.
					POLITICAL_STANCE stanceOfUnit = PoliticalStance(bd->EmpireId(), person.EmpireId());
					if(CompareStance(stanceOfUnit, stanceParameter, stanceCheck))
						return QVariant(true);
				}
			}
		}
	}
	return QVariant(false);
}

QVariant AuScriptLine::CityFindNewVehicle(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// city atom id
	if(u0id.IsNull())
		return QVariant(0);
	COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
	if(!DbCommodity::IsVehicle(cid))
		return QVariant(0);

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return QVariant(0);
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
		return QVariant(0);

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(bd)
	{
		// There could be a vehicle in town.
		DaWorld* planet = bd->FindParentPlanetInEnv();
		if(!planet)
			return QVariant(0);

		// Looking for a vehicle. This function could find a vehicle in another city if that city has an airport.
		// It checks the city id we pass it first.
		DaVehicle* vehicle = planet->FindNewVehicle(cid, bd->EmpireId(), u0id);
		if(vehicle && vehicle->InBdCity() == u0id)
			return QVariant(vehicle->Id().GetID());
	}
	return QVariant(0);
}

QVariant AuScriptLine::CityHasBuilding(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// city atom id
	if(u0id.IsNull())
		return false;
	INDUSTRYID iid = (INDUSTRYID)m_Parms[pidx++].toInt();
	unsigned int levels = m_Parms[pidx++].toUInt();

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
		return false;

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(bd)
	{
		AuCityReport report;
		if(!bd->ReportLast(report))
			return false;
		return report.BuildingLevels(iid) >= (int)levels;
	}
	return false;
}

QVariant AuScriptLine::CityHasFood(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// city atom id
	if(u0id.IsNull())
		return false;

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
		return false;

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(bd)
	{
		AuCityReport report;
		if(!bd->ReportLast(report))
			return false;
		return report.m_Inventory->HasFood(bd->ResidentEcoRole());
	}
	return false;
}

QVariant AuScriptLine::CityHasItem(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// city atom id
	if(u0id.IsNull())
		return false;
	COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
		return false;

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(bd)
	{
		if(DbCommodity::IsVehicle(cid))
		{
			// There could be a vehicle in town.
			DaWorld* planet = bd->FindParentPlanetInEnv();
			if(!planet)
				return false;

			// Looking for a vehicle. This function could find a vehicle in another city if that city has an airport.
			// It checks the city id we pass it first.
			DaVehicle* vehicle = planet->FindNewVehicle(cid, bd->EmpireId(), u0id);
			return QVariant(vehicle && vehicle->InBdCity() == u0id);
		}
		else
		{
			AuCityReport report;
			if(!bd->ReportLast(report))
				return false;
			return report.m_Inventory->contains(cid);
		}
	}
	return false;
}

QVariant AuScriptLine::CityHasPower(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// city atom id
	if(u0id.IsNull())
		return false;

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
		return false;

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(bd)
		return bd->HasPower();
	return false;
}

QVariant AuScriptLine::CityItemCount(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// city atom id
	if(u0id.IsNull())
		return 0;
	COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return 0;
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
		return 0;

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(bd)
	{
		AuCityReport report;
		if(!bd->ReportLast(report))
			return 0;
		return report.m_Inventory->RawCount(cid);
	}
	return 0;
}

QVariant AuScriptLine::CityPopulation(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// city variable name
	AuId u0id = VariableAtomId(u0, vars);		// city atom id
	if(u0id.IsNull())
		return 0;

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return 0;
	DaAtom* atom = env->FindIdRecursive(u0id);
	if(!atom)
		return 0;

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(bd)
	{
		AuCityReport report;
		if(!bd->ReportLast(report))
			return 0;
		return report.m_PopOrig;
	}
	return 0;
}

QVariant AuScriptLine::FindGasGiant(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(vars);
	int pidx=0;
	int orbit = m_Parms[pidx++].toInt();	// Orbit

	AuId worldId;
	DaAtom* env = person.FindEnvironmentAtom();
	if(env && person.FindParentStarInEnv())
	{
		// Person is not in deep space.
		// Get a list of all worlds in the solar system.
		DaAtomList worlds;
		env->FindPlanetsRecursive(worlds);

		// Make a list of the ids of those that meet our search criteria.
		QList<AuId> wids;
		for(DaAtomList::const_iterator wi = worlds.cbegin(); wi != worlds.cend(); wi++)
		{
			const DaWorldGasGiant* world = dynamic_cast<const DaWorldGasGiant*>(*wi);
			if(!world)
				continue;

			// Verify orbit.
			switch(orbit)
			{
			case OZONE_Inferno:
			case OZONE_Inner:
			case OZONE_Habitable:
			case OZONE_Outer:
			case OZONE_Frigid:
				if(world->OrbitZone() != orbit)
					continue;
				break;

			case 10:	// Any
				break;

			case 11:	// Mild
				if(world->OrbitZone() == OZONE_Inferno || world->OrbitZone() == OZONE_Frigid)
					continue;
				break;

			case 12:	// Harsh
				if(world->OrbitZone() != OZONE_Inferno && world->OrbitZone() != OZONE_Frigid)
					continue;
				break;

			case 13:	// Uninhabitable
				if(world->OrbitZone() == OZONE_Habitable)
					continue;
				break;
			}

			wids.append(world->Id());
		}

		// Randomly select one of the ids.
		if(!wids.isEmpty())
			worldId = wids[g_Au.Dice().Roll(1,wids.count(),-1)];
	}

	return QVariant(worldId.GetID());
}

QVariant AuScriptLine::FindMoon(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(vars);
	int pidx=0;
	int orbit = m_Parms[pidx++].toInt();	// Orbit

	AuId worldId;
	DaAtom* env = person.FindEnvironmentAtom();
	if(env && person.FindParentStarInEnv())
	{
		// Person is not in deep space.
		// Get a list of all worlds in the solar system.
		DaAtomList worlds;
		env->FindPlanetsRecursive(worlds);

		// Make a list of the ids of those that meet our search criteria.
		QList<AuId> wids;
		for(DaAtomList::const_iterator wi = worlds.cbegin(); wi != worlds.cend(); wi++)
		{
			const DaWorldGlobe* world = dynamic_cast<const DaWorldGlobe*>(*wi);
			if(!world)
				continue;

			// Verify moon-sized globe.
			if(!world->IsMoonSize())
				continue;

			// Verify orbit.
			switch(orbit)
			{
			case OZONE_Inferno:
			case OZONE_Inner:
			case OZONE_Habitable:
			case OZONE_Outer:
			case OZONE_Frigid:
				if(world->OrbitZone() != orbit)
					continue;
				break;

			case 10:	// Any
				break;

			case 11:	// Mild
				if(world->OrbitZone() == OZONE_Inferno || world->OrbitZone() == OZONE_Frigid)
					continue;
				break;

			case 12:	// Harsh
				if(world->OrbitZone() != OZONE_Inferno && world->OrbitZone() != OZONE_Frigid)
					continue;
				break;

			case 13:	// Uninhabitable
				if(world->OrbitZone() == OZONE_Habitable)
					continue;
				break;
			}

			wids.append(world->Id());
		}

		// Randomly select one of the ids.
		if(!wids.isEmpty())
			worldId = wids[g_Au.Dice().Roll(1,wids.count(),-1)];
	}

	return QVariant(worldId.GetID());
}

QVariant AuScriptLine::FindPlanet(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(vars);
	int pidx=0;
	int orbit = m_Parms[pidx++].toInt();	// Orbit
	int hydroMax = m_Parms[pidx++].toInt();	// Hydrographics Max

	AuId worldId;
	DaAtom* env = person.FindEnvironmentAtom();
	if(env && person.FindParentStarInEnv())
	{
		// Person is not in deep space.
		// Get a list of all worlds in the solar system.
		DaAtomList worlds;
		env->FindPlanetsRecursive(worlds);

		// Make a list of the ids of those that meet our search criteria.
		QList<AuId> wids;
		for(DaAtomList::const_iterator wi = worlds.cbegin(); wi != worlds.cend(); wi++)
		{
			const DaWorldGlobe* world = dynamic_cast<const DaWorldGlobe*>(*wi);
			if(!world)
				continue;

			// Verify planet-sized world.
			if(!world->CanHaveAtmosphere())
				continue;

			// Verify hydrographics.
			if(world->Hydrographics() > hydroMax)
				continue;

			// Verify orbit.
			switch(orbit)
			{
			case OZONE_Inferno:
			case OZONE_Inner:
			case OZONE_Habitable:
			case OZONE_Outer:
			case OZONE_Frigid:
				if(world->OrbitZone() != orbit)
					continue;
				break;

			case 10:	// Any
				break;

			case 11:	// Mild
				if(world->OrbitZone() == OZONE_Inferno || world->OrbitZone() == OZONE_Frigid)
					continue;
				break;

			case 12:	// Harsh
				if(world->OrbitZone() != OZONE_Inferno && world->OrbitZone() != OZONE_Frigid)
					continue;
				break;

			case 13:	// Uninhabitable
				if(world->OrbitZone() == OZONE_Habitable)
					continue;
				break;
			}

			wids.append(world->Id());
		}

		// Randomly select one of the ids.
		if(!wids.isEmpty())
			worldId = wids[g_Au.Dice().Roll(1,wids.count(),-1)];
	}

	return QVariant(worldId.GetID());
}

QVariant AuScriptLine::FindRing(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(vars);
	int pidx=0;
	int orbit = m_Parms[pidx++].toInt();	// Orbit

	AuId worldId;
	DaAtom* env = person.FindEnvironmentAtom();
	if(env && person.FindParentStarInEnv())
	{
		// Person is not in deep space.
		// Get a list of all worlds in the solar system.
		DaAtomList worlds;
		env->FindPlanetsRecursive(worlds);

		// Make a list of the ids of those that meet our search criteria.
		QList<AuId> wids;
		for(DaAtomList::const_iterator wi = worlds.cbegin(); wi != worlds.cend(); wi++)
		{
			const DaWorldRing* world = dynamic_cast<const DaWorldRing*>(*wi);
			if(!world)
				continue;

			// Verify orbit.
			switch(orbit)
			{
			case OZONE_Inferno:
			case OZONE_Inner:
			case OZONE_Habitable:
			case OZONE_Outer:
			case OZONE_Frigid:
				if(world->OrbitZone() != orbit)
					continue;
				break;

			case 10:	// Any
				break;

			case 11:	// Mild
				if(world->OrbitZone() == OZONE_Inferno || world->OrbitZone() == OZONE_Frigid)
					continue;
				break;

			case 12:	// Harsh
				if(world->OrbitZone() != OZONE_Inferno && world->OrbitZone() != OZONE_Frigid)
					continue;
				break;

			case 13:	// Uninhabitable
				if(world->OrbitZone() == OZONE_Habitable)
					continue;
				break;
			}

			wids.append(world->Id());
		}

		// Randomly select one of the ids.
		if(!wids.isEmpty())
			worldId = wids[g_Au.Dice().Roll(1,wids.count(),-1)];
	}

	return QVariant(worldId.GetID());
}

QVariant AuScriptLine::FindRingWorld(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(vars);
	AuId worldId;
	DaAtom* env = person.FindEnvironmentAtom();
	if(env && person.FindParentStarInEnv())
	{
		// Person is not in deep space.
		// Get a list of all worlds in the solar system.
		DaAtomList worlds;
		env->FindPlanetsRecursive(worlds);

		// Make a list of the ids of those that meet our search criteria.
		QList<AuId> wids;
		for(DaAtomList::const_iterator wi = worlds.cbegin(); wi != worlds.cend(); wi++)
		{
			const DaWorldRingSection* world = dynamic_cast<const DaWorldRingSection*>(*wi);
			if(!world)
				continue;

			wids.append(world->Id());
		}

		// Randomly select one of the ids.
		if(!wids.isEmpty())
			worldId = wids[g_Au.Dice().Roll(1,wids.count(),-1)];
	}

	return QVariant(worldId.GetID());
}

QVariant AuScriptLine::ItemOnGround(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(vars);
	COMMODITYID cid = (COMMODITYID)m_Parms[0].toInt();
	double distance = m_Parms[1].toInt() * FeetPerMeter();

	AuId itemId;
	distance *= distance;	// We will compare squared distances.
	DaAtom* parent = person.Parent();
	DaAtomList subs;
	parent->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaItem* itemAtom = dynamic_cast<const DaItem*>(*it);
		if(!itemAtom)
			continue;
		if(itemAtom->CommodityId() != cid)
			continue;
		if(itemAtom->Location().Distance2(person.Location()) > distance)
			continue;	// Item is too far away.

		itemId = itemAtom->Id();
		break;
	}

	return QVariant(itemId.GetID());
}

QVariant AuScriptLine::ItemOnGroundSpecific(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	double distance = m_Parms[pidx++].toInt() * FeetPerMeter();
	COMMODITYID cid = (COMMODITYID)m_Parms[pidx++].toInt();
	quint8 quality = m_Parms[pidx++].toUInt();
	quint32 empireOfOrigin = m_Parms[pidx++].toInt();
	QString inscription = StringParameter(m_Parms[pidx++].toString(), person, vars);

	AuId itemId;
	distance *= distance;	// We will compare squared distances.
	DaAtom* parent = person.Parent();
	DaAtomList subs;
	parent->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaItem* itemAtom = dynamic_cast<const DaItem*>(*it);
		if(!itemAtom)
			continue;
		if(itemAtom->Location().Distance2(person.Location()) > distance)
			continue;	// Item is too far away.

		const AuItem* item = itemAtom->LockItem();
		if(inscription.isEmpty())
		{
			if(item->CommodityId() == cid &&
					item->Quality() == quality &&
					item->EmpireOfOrigin() == empireOfOrigin)
				itemId = itemAtom->Id();
		}
		else
		{
			if(item->CommodityId() == cid &&
					item->Quality() == quality &&
					item->EmpireOfOrigin() == empireOfOrigin &&
					item->Inscription() == inscription)
				itemId = itemAtom->Id();
		}
		itemAtom->UnlockItem();
		if(!itemId.IsNull())
			break;
	}

	return QVariant(itemId.GetID());
}

QVariant AuScriptLine::SectorHengeSystem(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(person);
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// sector variable name
	AuId u0id = VariableAtomId(u0, vars);
	int hengeWorldType = m_Parms[pidx++].toInt();	// 0=any, 1=on globe, 2=on ringworld

	AuId systemId;
	if(!u0id.IsNull())
	{
		// Look for a henge system in the sector.
		AuIdList hdsystems;
		const AuHengeDestinations& hds = g_Au.LockHengeDestinations();
		for(AuHengeDestinations::const_iterator hdi = hds.cbegin(); hdi != hds.cend(); hdi++)
		{
			// Currently we only look for henges on habitable worlds that are outside a city.
			if(!(*hdi).IsHabitable() || !(*hdi).IsOutsideTown())
				continue;

			// Sometimes they care what kind of world the henge is on.
			if(hengeWorldType == 1 && (*hdi).m_WorldType != DaAtom::ATOM_WorldGlobe)
				continue;
			else if(hengeWorldType == 2 && (*hdi).m_WorldType != DaAtom::ATOM_WorldRingSection)
				continue;

			if((*hdi).m_SectorId == u0id)
				hdsystems.append((*hdi).m_SystemId);
		}
		g_Au.UnlockHengeDestinations();

		if(hdsystems.count() == 1)
			systemId = hdsystems.first();
		else if(!hdsystems.isEmpty())
		{
			AuIdList::const_iterator hdi = hdsystems.cbegin() + g_Au.Dice().Roll(1,hdsystems.count(),-1);
			systemId = *hdi;
		}
	}

	return QVariant(systemId.GetID());
}

/*
QVariant AuScriptLine::SystemHengeWorld(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(person);
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// system variable name
	AuId u0id = VariableAtomId(u0, vars);
	int hengeWorldType = m_Parms[pidx++].toInt();	// 0=any, 1=on globe, 2=on ringworld

	AuId worldId;
	if(!u0id.IsNull())
	{
		// Look for a henge world in the system.
		AuIdList hdworlds;
		const AuHengeDestinations& hds = g_Au.LockHengeDestinations();
		for(AuHengeDestinations::const_iterator hdi = hds.cbegin(); hdi != hds.cend(); hdi++)
		{
			// Currently we only look for henges on habitable worlds that are outside a city.
			if(!(*hdi).IsHabitable() || !(*hdi).IsOutsideTown())
				continue;

			// Sometimes they care what kind of world the henge is on.
			if(hengeWorldType == 1 && (*hdi).m_WorldType != DaAtom::ATOM_WorldGlobe)
				continue;
			else if(hengeWorldType == 2 && (*hdi).m_WorldType != DaAtom::ATOM_WorldRingSection)
				continue;

			if((*hdi).m_SystemId == u0id)
				hdworlds.append((*hdi).WorldId());
		}
		g_Au.UnlockHengeDestinations();

		if(hdworlds.count() == 1)
			worldId = hdworlds.first();
		else if(!hdworlds.isEmpty())
		{
			AuIdList::const_iterator hdi = hdworlds.cbegin() + g_Au.Dice().Roll(1,hdworlds.count(),-1);
			worldId = *hdi;
		}
	}

	return QVariant(worldId.GetID());
}
*/

QVariant AuScriptLine::SystemHengeWorldCatName(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	Q_UNUSED(person);
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// system variable name
	AuId u0id = VariableAtomId(u0, vars);
	int hengeWorldType = m_Parms[pidx++].toInt();	// 0=any, 1=on globe, 2=on ringworld

	QString worldCatName;
	if(!u0id.IsNull())
	{
		// Look for a henge world in the system.
		QStringList hdworlds;
		const AuHengeDestinations& hds = g_Au.LockHengeDestinations();
		for(AuHengeDestinations::const_iterator hdi = hds.cbegin(); hdi != hds.cend(); hdi++)
		{
			// Currently we only look for henges on habitable worlds that are outside a city.
			if(!(*hdi).IsHabitable() || !(*hdi).IsOutsideTown())
				continue;

			// Sometimes they care what kind of world the henge is on.
			if(hengeWorldType == 1 && (*hdi).m_WorldType != DaAtom::ATOM_WorldGlobe)
				continue;
			else if(hengeWorldType == 2 && (*hdi).m_WorldType != DaAtom::ATOM_WorldRingSection)
				continue;

			if((*hdi).m_SystemId == u0id)
				hdworlds.append((*hdi).m_WorldCatName);
		}
		g_Au.UnlockHengeDestinations();

		if(hdworlds.count() == 1)
			worldCatName = hdworlds.first();
		else if(!hdworlds.isEmpty())
		{
			QStringList::const_iterator hdi = hdworlds.cbegin() + g_Au.Dice().Roll(1,hdworlds.count(),-1);
			worldCatName = *hdi;
		}
	}

	return QVariant(worldCatName);
}

QVariant AuScriptLine::AtomOpenChannel(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();		// from variable name
	AuId u0id = VariableAtomId(u0, vars);
	if(u0id.IsNull())
		return QVariant(false);
	QString u1 = m_Parms[pidx++].toString();		// to variable name
	AuId u1id = VariableAtomId(u1, vars);
	if(u1id.IsNull())
		return false;
	QString channel = StringParameter(m_Parms[pidx++].toString(), person, vars);	// Channel name

	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;

	DaAtom* a0 = env->FindIdRecursive(u0id);
	if(!a0)
		return false;

	const DaAtom* a1 = env->FindIdRecursive(u1id);
	if(!a1)
		return false;

	const DaSector* sector = a0->FindParentSector();
	if(!sector)
		return false;

	// The initiator should start listening on the channel.
	g_Au.MonitorChannel(channel, a0->Id());

	// Open the channel to the recipient.
	a0->OpenChannelTo(a0->Id(), a0->Name(), sector->Id(), sector->Galaxy(), a1->Id(), channel, AuCommStdChannel::Everyone, false);
	return true;
}

QVariant AuScriptLine::WorldAtmosphereBreathable(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// world variable name
	AuId u0id = VariableAtomId(u0, vars);

	if(u0id.IsNull())
		return false;

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;

	const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(env->FindIdRecursive(u0id));
	if(!world || !world->IsNormal())
		return false;

	return world->HasBreathableAtmosphere();
}

QVariant AuScriptLine::WorldSupportsAnimals(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// world variable name
	AuId u0id = VariableAtomId(u0, vars);

	if(u0id.IsNull())
		return false;

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;

	const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(env->FindIdRecursive(u0id));
	if(!world || !world->IsNormal())
		return false;

	return world->CanHaveFauna();
}

QVariant AuScriptLine::WorldSupportsPlants(DaPerson& person, const QMap<QString, QVariant>& vars) const
{
	int pidx=0;
	QString u0 = m_Parms[pidx++].toString();	// world variable name
	AuId u0id = VariableAtomId(u0, vars);

	if(u0id.IsNull())
		return false;

	const DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return false;

	const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(env->FindIdRecursive(u0id));
	if(!world || !world->IsNormal())
		return false;

	return world->CanHaveFlora();
}

QDebug operator<<(QDebug dbg, const AuScriptLine& o)
{
	if(o.Assignee().isEmpty())
		dbg.nospace() << (o.Negate()?"Not":"") << s_StoryFunc[o.Function()].m_Name;
	else
		dbg.nospace() << o.Assignee() << "=" << (o.Negate()?"Not":"") << s_StoryFunc[o.Function()].m_Name;
	for(QList<QVariant>::const_iterator pi = o.Parameters().cbegin(); pi != o.Parameters().cend(); pi++)
	{
		if((*pi).canConvert<AuScriptLine>())
		{
			const AuScriptLine& v = (*pi).value<AuScriptLine>();
			dbg.nospace() << "\n " << v;
		}
		else if((*pi).canConvert<AuGlobeLLF>())
		{
			const AuGlobeLLF& v = (*pi).value<AuGlobeLLF>();
			dbg.nospace() << "\n " << v;
		}
		else if((*pi).canConvert<QString>())
		{
			const QString& v = (*pi).value<QString>();
			dbg.nospace() << "\n " << v;
		}
		else
		{
			dbg.nospace() << "\n  " << *pi;
		}
	}
	return dbg;
}

DaSentient* AuScriptLine::FindSentient(const DaPerson& person, const AuId& sentId) const
{
	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return 0;	// This should never happen.
	return dynamic_cast<DaSentient*>(env->FindIdRecursive(sentId));
}

DaVessel* AuScriptLine::FindShip(const DaPerson& person, const AuId& shipId) const
{
	DaAtom* env = person.FindEnvironmentAtom();
	if(!env)
		return 0;	// This should never happen.
	return dynamic_cast<DaVessel*>(env->FindIdRecursive(shipId));
}

void AuScriptLine::WorldScheduleAsteroid(DaPerson& person, const AuId& worldId, int minutesToImpact) const
{
	if(minutesToImpact <= 0)
		return;
	AuEther* ether = person.Ether();
	if(!ether)
		return;

	DaSolarSystem* system = person.FindParentSolarSystemInEnv();
	if(!system)
		return;	// This should never happen.

	// Locate the world.
	DaWorldLL* world = dynamic_cast<DaWorldLL*>(system->FindIdRecursive(worldId));
	if(!world)
		return;

	// Start the doomsday asteroid.
	AuPoint startAt;
	if(person.EnvLocation().z > 0)
		startAt = AuPoint(0,0,1);	// Person is above the system plane of the ecliptic so asteroid comes from above.
	else
		startAt = AuPoint(0,0,-1);	// Person is below the system plane of the ecliptic so asteroid comes from below.
	startAt = startAt.RotateX(g_Au.Dice().Roll(1,30)*DEG2RAD);	// Slight tilt for fun.

	// Calculate the distance so normal speed gets the asteroid to the target on time. The asteroid continuously
	// modulates its speed to arrive at the target on time. This calc will make it modulate to a speed close to
	// normal asteroid speed.
	AuTime asteroidImpact = AuTime::Now().AddMins(minutesToImpact);
	double distance = minutesToImpact * 60 * DaAsteroid::TargetPursueSpeed();
	if(distance > FeetPerParsec()/2)
		distance = FeetPerParsec()/2;	// Don't fall into the next solar system.
	startAt.Unit(distance);
	AuMatrix4x4 t;
	t.Translate(startAt);

	// Calculate release time so asteroid comes screaming in at high speed, instead of crawling. Slow asteroids are
	// boring. Too fast and they strike too quickly to have any fun.
	qint64 secsTravel = (qint64)(distance / DaAsteroid::TargetPursueSpeed());	// Time to travel the distance at TPS.
	AuTime timeOfRelease = asteroidImpact.AddSecs(-secsTravel);	// If earlier than now, the asteroid will start now.

	DaAsteroid* roid = new DaAsteroid(world->Id(), asteroidImpact, timeOfRelease);
	roid->ConnectSignals();
	roid->Create();
	roid->SetPosAndOrient(AuAtomPos(system->Id(), t));
	ether->AddAtom(roid);
	g_Au.AtomAdded(roid->Id());
	roid->SetNormal();	// New atom is ready for normal thread processing.
	roid->TickTock();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScriptLines::operator>>(QDataStream& out) const
{
	out << (quint32)size();
	for(const_iterator li = cbegin(); li != cend(); li++)
		out << *li;
	out << m_StartLine;
}

void AuScriptLines::operator<<(QDataStream& in)
{
	clear();
	quint32 ui32;
	in >> ui32;
	while(ui32--)
	{
		AuScriptLine line;
		in >> line;
		append(line);
	}

	if(m_VerRead >= 1)
		in >> m_StartLine;
}

QDebug operator<<(QDebug dbg, const AuScriptLines& o)
{
	// Dump the tokenized script.
	for(AuScriptLines::const_iterator li = o.cbegin(); li != o.cend(); li++)
		dbg.nospace() << "\n" << *li;
	return dbg;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Loads the script, compiles it and validates the data.
bool AuScript::Load(const QString& filePath)
{
	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScript::Load Compiling script %1 %2.").arg(m_Id).arg(filePath);
		g_Au.Log(logmsg);
	}

	QFile file(filePath);
	if(file.open(QIODevice::ReadOnly))
	{
		QTextStream in(&file);
		m_Result = Compile(in, filePath);
		file.close();
		if(m_Result == 0)
		{
			if(g_Au.m_LogMessages & LOG_Story)
			{
				QString logmsg = QObject::tr("AuScript::Load Script %1 compiled, %2 blocks.").arg(m_Id).arg(m_Body.size());
				g_Au.Log(logmsg);
			}
			return true;
		}
		else if(m_Result == 2)
		{
			if(g_Au.m_LogMessages & LOG_Story)
			{
				QString logmsg = QObject::tr("AuScript::Load Script %1 compile disabled.").arg(m_Id);
				g_Au.Log(logmsg);
			}
		}
	}
	else
	{
		// Errors and warnings are sent to stderr.
		QString sError = QString("%1: error: failed to open script file")
			.arg(filePath);
		CompilerAppendOutput(sError);
		m_Result = 1;
	}
	return false;
}

bool AuScript::LoadHar(const QString& harFileName)
{
	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScript::LoadHar Compiling script %1 %2.").arg(m_Id).arg(harFileName);
		g_Au.Log(logmsg);
	}

	QHash<QString, QByteArray> files;
	Hashpack::ReadFiles(harFileName, files);
	QHash<QString, QByteArray>::const_iterator psi = files.find("script.sh");
	if(psi != files.cend())
	{
		// Read the script from the hash table of compressed data.
		QByteArray src = AuUncompress(*psi, __FILE__, __LINE__,harFileName);
		QTextStream in(src);
		m_Result = Compile(in, harFileName);
		if(m_Result == 0)
		{
			if(g_Au.m_LogMessages & LOG_Story)
			{
				QString logmsg = QObject::tr("AuScript::LoadHar Script %1 compiled, %2 blocks.").arg(m_Id).arg(m_Body.size());
				g_Au.Log(logmsg);
			}
			return true;
		}
		else if(m_Result == 2)
		{
			if(g_Au.m_LogMessages & LOG_Story)
			{
				QString logmsg = QObject::tr("AuScript::LoadHar Script %1 compile disabled.").arg(m_Id);
				g_Au.Log(logmsg);
			}
		}
	}
	else
	{
		// Errors and warnings are sent to stderr.
		QString sError = QString("%1: error: failed to open script file")
			.arg(harFileName);
		CompilerAppendOutput(sError);
		m_Result = 1;
	}
	return false;
}

// Returns 0=ok, 1=error, 2=#nocompile
int AuScript::Compile(const QString& scriptFileName)
{
	QFile file(scriptFileName);
	if(file.open(QIODevice::ReadOnly))
	{
		QTextStream in(&file);
		m_Result = Compile(in, scriptFileName);
		file.close();
	}
	else
	{
		// Errors and warnings are sent to stderr.
		QString sError = QString("%1: error: failed to open script file")
			.arg(scriptFileName);
		CompilerAppendOutput(sError);
		m_Result = 1;
	}
	return m_Result;
}

int AuScript::Compile(QTextStream& in, const QString& scriptFile)
{
	bool noCompile = false;
	bool anyError = false;	// true if any line compiles with an error
	QString catalystBlock(" ");
	QString curBlockLabel;
	AuScriptLines curBlock;
	QString line;
	QStringList varDefs;	// Names of variables that get defined
	QMap<int, AuScriptBranch> branches;
	int scriptLine = 0;
	while(!in.atEnd())
	{
		line = in.readLine().trimmed();
		scriptLine++;

		//qDebug((const char*)line.toAscii());

		// Watch for our one and only compiler directive.
		if(line.left(10).compare("#nocompile", Qt::CaseInsensitive) == 0)
		{
			noCompile = true;
			break;
		}

		// Trim any comments from the line.
		// This could leave the entire line blank.
		// Look for a '#' that is not inside of quotes.
		bool inQuotes = false;
		for(int idx = 0; idx < line.length(); idx++)
		{
			if(line[idx] == '"')
				inQuotes = !inQuotes;
			else if(!inQuotes && line[idx] == '#')
			{
				line = line.left(idx).trimmed();
				break;
			}
		}

		// Skip blank lines.
		if(line.isEmpty())
			continue;

		// Parse the line.
		if(line[0] == ':')
		{
			// The line is a block label.
			if(!curBlockLabel.isEmpty())
			{
				// Terminate the current block.
				m_Body.insert(pair<QString, AuScriptLines>(curBlockLabel, curBlock));
				curBlock.clear();
			}
			curBlock.SetStartLine(scriptLine);
			curBlockLabel = line.mid(1);
			if(curBlockLabel.isEmpty())
			{
				// The empty block label is reserved for the catalyst of the story. This is the test that determines whether
				// conditions are favorable to begin the story.
				curBlockLabel = catalystBlock;
			}
			else if(curBlockLabel.indexOf(' ') >= 0)
			{
				QString sError = QString("%1:line %2: error: unexpected ' ' in block label '%3'")
					.arg(scriptFile).arg(scriptLine).arg(curBlockLabel);
				CompilerAppendOutput(sError);
				return 1;
			}
			else if(curBlockLabel.indexOf(';') >= 0)
			{
				QString sError = QString("%1:line %2: error: unexpected ' ' in block label '%3'")
					.arg(scriptFile).arg(scriptLine).arg(curBlockLabel);
				CompilerAppendOutput(sError);
				return 1;
			}
			if(m_Body.find(curBlockLabel) != m_Body.end())
			{
				QString sError = QString("%1:line %2: error: block label already defined '%3'")
					.arg(scriptFile).arg(scriptLine).arg(curBlockLabel);
				CompilerAppendOutput(sError);
				return 1;
			}
		}
		else
		{
			// Attempt to compile the line into a recognized function call.
			if(curBlockLabel.isEmpty())
			{
				QString sError = QString("%1:line %2: error: function calls not permitted before first block label: '%3'")
					.arg(scriptFile).arg(scriptLine).arg(line);
				CompilerAppendOutput(sError);
				return 1;	// Bad format
			}

			// Concatenate lines until we find one ending in a ;
			while(line.right(1) != ";" && !in.atEnd())
			{
				// Read the next line.
				QString nextline = in.readLine().trimmed();
				scriptLine++;

				// Trim any comments from the next line.
				// This could leave the entire next line blank.
				// Look for a '#' that is not inside of quotes.
				bool inQuotes = false;
				for(int idx = 0; idx < nextline.length(); idx++)
				{
					if(nextline[idx] == '"')
						inQuotes = !inQuotes;
					else if(!inQuotes && nextline[idx] == '#')
					{
						nextline = nextline.left(idx);
						break;
					}
				}

				// Skip blank lines.
				if(nextline.isEmpty())
					continue;	// Skip blank lines.

				// Concatenate the next line onto the line we have accumulated.
				if(line.right(1) == "\"")
				{
					// A string parameter has been broken across lines.
					if(nextline[0] == '"')
					{
						// Next line starts with a quote properly.
						line = line.left(line.length()-1);	// Drop the trailing quote from the current line.
						line += nextline.mid(1);			// Skip over the leading quote on the next line.
					}
					else
					{
						QString sError = QString("%1:line %2: error: unexpected end of line: '%3'")
							.arg(scriptFile).arg(scriptLine).arg(line);
						CompilerAppendOutput(sError);
						return 1;	// Bad format
					}
				}
				else
				{
					// The current line is not breaking in a string parameter.
					line += nextline;
				}
			}
			if(line.right(1) != ";")
			{
				// We got to the end without finding a semicolon.
				QString sError = QString("%1:line %2: error: expected ';' at end of line: '%3'")
					.arg(scriptFile).arg(scriptLine).arg(line);
				CompilerAppendOutput(sError);
				return 1;	// Bad format
			}

			// Strip trailing semicolon(s) from the line
			while(line.right(1) == ";")
				line = line.left(line.length()-1).trimmed();

			AuScriptLine subLine(curBlockLabel, line, false, scriptFile, scriptLine, *this, varDefs, m_BdDesigns, m_ScDesigns,
					branches);
			if(subLine.Function() != F_NOP)
			{
				// Function compiled into something other than a NOP.
				Q_ASSERT(s_StoryFunc[subLine.Function()].m_Func == subLine.Function());	// Function table is out of sync
				if(curBlockLabel == QChar(' '))
				{
					// We are compiling the catalyst block.
					if(!s_StoryFunc[subLine.Function()].OkInCatalyst())
					{
						QString sError = QString("%1:line %2: error: function not permitted in catalyst of story: '%3'")
							.arg(scriptFile).arg(scriptLine).arg(line);
						CompilerAppendOutput(sError);
						return 1;
					}
				}
				else
				{
					// We are compiling a block that is not the catalyst.
					if(!s_StoryFunc[subLine.Function()].OkInBody())
					{
						QString sError = QString("%1:line %2: error: function not permitted in body of story: '%3'")
							.arg(scriptFile).arg(scriptLine).arg(line);
						CompilerAppendOutput(sError);
						return 1;
					}
				}

				/*
				// Branch conditions are always inserted at the beginning of the list so we can find and iterate on them faster.
				if(subLine.Function() == F_If)
				{
					// Insert after the last if to maintain logical test order in blocks with multiple branch conditions.
					bool inserted = false;
					for(AuScriptLines::iterator iif = curBlock.begin(); iif != curBlock.end(); iif++)
						if((*iif).Function() != F_If)
						{
							// Insert the nu if before the first non-if statement.
							curBlock.insert(iif, subLine);
							inserted = true;
							break;
						}
					if(!inserted)
						curBlock.append(subLine);			// Append if that was not inserted.
				}
				else
				*/
					curBlock.append(subLine);				// Append non-if statements in sequence.
			}
			else
				anyError = true;
		}
	}
	if(noCompile)
	{
		m_Body.clear();
		m_sOutput.clear();
		return 2;	// Return quietly.
	}

	// Append the current block.
	if(!curBlockLabel.isEmpty())
		m_Body.insert(pair<QString, AuScriptLines>(curBlockLabel, curBlock));

	// Insure the catalyst block looks valid.
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.cend())
	{
		QString sError = QString("%1: error: story lacks a catalyst block")
			.arg(scriptFile);
		CompilerAppendOutput(sError);
		return 1;	// No catalyst block.
	}
	else if((*bit).second.isEmpty())
	{
		QString sError = QString("%1: error: catalyst block is empty")
			.arg(scriptFile);
		CompilerAppendOutput(sError);
		return 1;
	}

	// Look for at least one 'if' in the catalyst block.
	bool hasIf = false;
	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
		if((*it).Function() == F_If)
		{
			hasIf = true;
			break;
		}
	if(!hasIf)
	{
		QString sError = QString("%1: error: no 'if' in catalyst block")
			.arg(scriptFile);
		CompilerAppendOutput(sError);
		return 1;
	}

	// Insure title, author, and info are not empty.
	if(Title().isEmpty())
	{
		QString sError = QString("%1: error: no 'title' in catalyst block")
			.arg(scriptFile);
		CompilerAppendOutput(sError);
		return 1;
	}
	if(Author().isEmpty())
	{
		QString sError = QString("%1: error: no 'author' in catalyst block")
			.arg(scriptFile);
		CompilerAppendOutput(sError);
		return 1;
	}
	if(Brief().isEmpty())
	{
		QString sError = QString("%1: error: no 'brief' in catalyst block")
			.arg(scriptFile);
		CompilerAppendOutput(sError);
		return 1;
	}

	// Insure all branches go to valid blocks.
	for(QMap<int, AuScriptBranch>::const_iterator ci = branches.cbegin(); ci != branches.cend(); ci++)
	{
		unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bi = m_Body.find((*ci).ToLabel());
		if(bi == m_Body.cend())
		{
			// Destination block does not exist.
			QString sError = QString("%1:line %2: error: unresolved branch: '%3'")
				.arg(scriptFile)
				.arg((*ci).LineNumber())
				.arg((*ci).LineText());
			CompilerAppendOutput(sError);
			anyError = true;
		}
	}

	// Warn about blocks that have no branch to them.
	for(unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bi = m_Body.cbegin(); bi != m_Body.cend(); bi++)
	{
		if(bi == bit)
			continue;
		bool foundBranchTo = false;
		for(QMap<int, AuScriptBranch>::const_iterator ci = branches.cbegin(); ci != branches.cend(); ci++)
		{
			if((*ci).ToLabel() != (*bi).first)
				continue;	// Branch does not go to the block we are checking.
			if((*ci).BlockName() == (*ci).ToLabel())
				continue;	// Ignore branches to their own block.
			foundBranchTo = true;
			break;
		}

		if(!foundBranchTo)
		{
			// Nobody branches to the block.
			QString sWarning = QString("%1:line %2: warning: block is never the target of a branch: '%3'")
				.arg(scriptFile)
				.arg((*bi).second.StartLine())
				.arg((*bi).first);
			CompilerAppendOutput(sWarning);
		}
	}

	//if(!anyError)
		//qDebug() << *this;	// Dump the tokenized script.

	return anyError ? 1 : 0;
}

QDebug operator<<(QDebug dbg, const AuScript& o)
{
	dbg.nospace() << "----------Script " << o.Id() << "----------";
	dbg.nospace() << "\nBody " << o.Body().size() << " blocks";
	for(unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bi = o.Body().cbegin(); bi != o.Body().cend(); bi++)
	{
		dbg.nospace() << "\n----------";
		dbg.nospace() << "\n:" << (*bi).first;
		dbg.nospace() << (*bi).second;
	}
	return dbg;
}

// Return true if conditions are right to start this story.
bool AuScript::TestCatalyst(DaPerson& person, QString* startBlock, QMap<QString, QVariant>& vars) const
{
	//qDebug() << "AuScript::TestCatalyst script" << Id() << Title() << "avatar" << person.Id() << person.Name();
	if(m_Body.empty())
		return false;	// Body is empty. Likely a #nocompile.

	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
	{
		qWarning("AuScript::TestCatalyst script %d body has no catalyst!", Id());
		return false;	// Body does not contain a catalyst block.
	}

	// This test is performed before the story instance is created, to see if it can be created.
	// Commands that need a story instance are not permitted in the catalyst block. The story instance is generally only needed
	// to communicate on the comm or create story-related characters and other objects.
	quint32 storyInstance = 0;

	// Execute the portion of the catalyst that can be run, to set variables.
	bool storyCancelOk = true;	// Not relevant in the catalyst.
	for(AuScriptLines::const_iterator si = (*bit).second.cbegin(); si != (*bit).second.cend(); si++)
		(*si).Execute(*this, storyInstance, person, vars, AuTime(), storyCancelOk);

	// Test the catalyst of the script to see if conditions are ok to start.
	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
	{
		if((*it).Function() == F_If)
		{
			// This is an 'if' condition to test. If it is true, the story can be started.
			// We don't have a story instance id yet so pass 0 to TestBranch. It is currently only needed when searching for
			// props related to the story, which cannot have an affect in the catalyst block anyway.
			QString branchToBlock;
			if((*it).TestBranch(person, 0, AuTime::Now(), branchToBlock, vars))
			{
				if(startBlock)
					*startBlock = branchToBlock;	// first block label

				if(g_Au.m_LogMessages & LOG_Story)
				{
					QString logmsg = QObject::tr("AuScript::TestCatalyst %1 starting block %2, person %3 %4")
						.arg(storyInstance?QString("%1").arg(storyInstance):"catalyst")
						.arg(branchToBlock)
						.arg(person.Id().GetID())
						.arg(person.Name());
					g_Au.Log(logmsg);
				}
				return true;
			}
		}
	}
	return false;
}

QString AuScript::Author() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return "";	// Body does not contain a catalyst block.
	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
	{
		if((*it).Function() == F_StoryAuthor)
			return (*it).Parameter(0).toString();
	}
	return "";
}

QString AuScript::Brief() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return "";	// Body does not contain a catalyst block.
	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
	{
		if((*it).Function() == F_StoryBrief)
			return (*it).Parameter(0).toString();
	}
	return "";
}

QString AuScript::Title() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return "";	// Body does not contain a catalyst block.
	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
	{
		if((*it).Function() == F_StoryTitle)
			return (*it).Parameter(0).toString();
	}
	return "";
}

int AuScript::Interval() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return 0;	// Body does not contain a catalyst block.
	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
	{
		if((*it).Function() == F_StoryInterval)
			return (*it).Parameter(0).toInt();
	}
	return 0;
}

bool AuScript::AnyAutoStart() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return false;											// Body does not contain a catalyst block.

	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
		if((*it).Function() == F_StoryAutoStart ||
				(*it).Function() == F_StoryAutoRepeat ||
				(*it).Function() == F_StoryAutoRepeatUntilComplete)
			return true;

	return false;
}

bool AuScript::AutoStart() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return false;											// Body does not contain a catalyst block.

	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
		if((*it).Function() == F_StoryAutoStart)
			return true;

	return false;
}

bool AuScript::AutoRepeat() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return false;											// Body does not contain a catalyst block.

	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
		if((*it).Function() == F_StoryAutoRepeat)
			return true;

	return false;
}

bool AuScript::AutoRepeatUntilComplete() const
{
	QString catalystBlock(" ");
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(catalystBlock);
	if(bit == m_Body.end())
		return false;											// Body does not contain a catalyst block.

	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
		if((*it).Function() == F_StoryAutoRepeatUntilComplete)
			return true;

	return false;
}

static AuScriptLines s_EmptyScriptLines;	// Used for invalid return.
const AuScriptLines& AuScript::Block(const QString& blockLabel) const
{
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(blockLabel);
	if(bit != m_Body.end())
		return (*bit).second;
	return s_EmptyScriptLines;
}

// returns branch-to label resulting from choice picked
bool AuScript::ChoicePicked(DaPerson& person, quint32 storyInstance, const QString& blockLabel, qint8 value,
		const AuTime& blockStart, QString& branchToBlock, QMap<QString, QVariant>& vars) const
{
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(blockLabel);
	if(bit == m_Body.end())
		return false;	// Body does not contain a block with that name.

	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
		if((*it).Function() == F_If && (*it).TestBranch(person, storyInstance, blockStart, branchToBlock, vars, value))
			return true;

	// Narrator choice not found in block.
	return false;
}

bool AuScript::TestBranch(DaPerson& person, quint32 storyInstance, const QString& blockLabel, const AuTime& blockStart,
		QString& branchToBlock, QMap<QString, QVariant>& vars, bool& storyCancelOk) const
{
	unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bit = m_Body.find(blockLabel);
	if(bit == m_Body.end())
	{
		// Body does not contain a block with that name.
		// This is a critical failure and the story must end.
		QString errMsg = QString("AuScript Error: Story block '%1' not found. Terminating story.").arg(blockLabel);
		ScriptError(storyInstance, errMsg);
		branchToBlock = "";	// A blank block label with a true return will terminate the story instance.
		return true;
	}

	// Execute the portions of the block that do not modify the universe or produce random results, to update variables.
	for(AuScriptLines::const_iterator si = (*bit).second.cbegin(); si != (*bit).second.cend(); si++)
		if(s_StoryFunc[(*si).Function()].UniverseConst())
			(*si).Execute(*this, storyInstance, person, vars, blockStart, storyCancelOk);

	bool anyIf = false;	// true if this block contains any if's
	for(AuScriptLines::const_iterator it = (*bit).second.cbegin(); it != (*bit).second.cend(); it++)
	{
		if((*it).Function() == F_If)
		{
			if((*it).TestBranch(person, storyInstance, blockStart, branchToBlock, vars))
			{
				if(g_Au.m_LogMessages & LOG_Story)
				{
					QString logmsg = QObject::tr("AuScriptLine::TestBranch %1 block %2 if to branchToBlock %3, person %4 %5 ")
						.arg(storyInstance?QString("%1").arg(storyInstance):"catalyst")
						.arg(blockLabel)
						.arg(branchToBlock)
						.arg(person.Id().GetID())
						.arg(person.Name());
					g_Au.Log(logmsg);
				}
				return true;
			}
			anyIf = true;
		}
		else if((*it).Function() == F_GoTo)
		{
			(*it).GoToBranch(branchToBlock);
			if(g_Au.m_LogMessages & LOG_Story)
			{
				QString logmsg = QObject::tr("AuScriptLine::TestBranch %1 block %2 go to branchToBlock %3, person %4 %5")
					.arg(storyInstance?QString("%1").arg(storyInstance):"catalyst")
					.arg(blockLabel)
					.arg(branchToBlock)
					.arg(person.Id().GetID())
					.arg(person.Name());
				g_Au.Log(logmsg);
			}
			return true;
		}
	}

	// We get here if all 'if' evaluations returned false or if there weren't
	// any 'if' evaluations or 'goto's.
	if(!anyIf)
	{
		// Signify end of story with blank block name.
		branchToBlock = "";
		if(g_Au.m_LogMessages & LOG_Story)
		{
			QString logmsg = QObject::tr("AuScriptLine::TestBranch %1 block %2 The End, person %4 %5")
				.arg(storyInstance?QString("%1").arg(storyInstance):"catalyst")
				.arg(blockLabel)
				.arg(person.Id().GetID())
				.arg(person.Name());
			g_Au.Log(logmsg);
		}
		return true;
	}

	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuScriptLine::TestBranch %1 block %2 not ready to branch, person %4 %5")
			.arg(storyInstance?QString("%1").arg(storyInstance):"catalyst")
			.arg(blockLabel)
			.arg(person.Id().GetID())
			.arg(person.Name());
		g_Au.Log(logmsg);
	}
	return false;	// There are 'if's and they all returned false
}

// Send a script error message to the client on the narrator channel.
void AuScript::ScriptError(quint32 storyInstance, const QString& messageText) const
{
	AuCommNarrate msg(messageText, storyInstance, Title(), AuId(0), GALAXY_Count);
	g_Au.SendAtomChat(msg);
}

// Returns a list of function parameter types in human-readable form.
//static
QStringList AuScript::ListParameterTypes()
{
	QStringList parms;
	parms << "angle - Angle in degrees, in the range -360 to 360.";
	parms << "animal - A wild animal. A creature that is not sentient.";
	parms << "atom - A variable that references an atom. Most objects in the game universe are atoms.";
	parms << "avatar - A variable that references an avatar atom.";
	parms << "bd - Building blueprint id. Cannot be 0.";
	parms << "bool - Boolean value returned by calling one or more functions. Can use logical operators '''&''', '''|''', and '''!'''.";
	parms << "bool - Boolean argument. Accepts '''true''' or '''false''', not case sensitive.";
	parms << "building - Building name, not case sensitive.";
	parms << "city - A variable that references a capitol building atom.";
	parms << "commodity - Commodity name, not case sensitive.";
	parms << "channel - Communication channel name. Not limited to standard channel names.";
	parms << "character - A variable that references an npc atom who is associated with the current story. Characters disappear when their story ends. Characters are created with the npcrole of Citizen.";
	parms << "choiceValue - A value associated with multiple choices, in the range -127 to 127.";
	parms << "creature - A variable that references a creature atom. Animals and sentients are creature atoms.";
	parms << "dna - DNA specification. Loaded from a file if the string ends with '.chr' or '.dna'. Can also be Avatar, "
		"Indigenous or Worker. Worker returns Indigenous result if the avatar is not in a city.";
	parms << "empire - Empire name, case sensitive, or Empire, Vassal, Friend, Neutral, Enemy.";
	parms << "file - A file name found in the same path as the script.";
	parms << "gearslot - Gear slot, not case sensitive: '''Hand''', '''Head''', '''Face''', '''Neck''', '''Body''', '''Wear''', '''Limb''', '''Digit''', '''Waist'''. "
		"\n\n"
		"All creatures have at least one Hand. It is always their dominant hand. Do not expect more than that. "
		"\n\n"
		"All creatures have at least one Head. Both are treated as one when it comes to equipment. "
		"You cannot place a different hat or helmet on each head. Only one hat or helmet is needed for both heads. "
		"The same applies to faces and necks. "
		"\n\n"
		"All creatures have at least one body slot for carrying gear. "
		"\n\n"
		"Wear is specifically for things worn on the body, such as clothing and armor. "
		"\n\n"
		"Limb slots are for bracelets and anklets. A creature has one limb slot for each arm and leg on its body. "
		"\n\n"
		"Digit slots are for fingers and toes. A creature has one digit slot for each hand and foot on its body. "
		"\n\n"
		"All creatures have four waist slots; there is a size limit to items placed at the waist. ";
	parms << "gender - Gender of creature, not case sensitive: '''Male''', '''Female''', '''Asexual''', '''Hermaphrodite'''.";
	parms << "hand - Dominant hand of creature, not case sensitive: '''Left Handed''', '''Right Handed'''";
	parms << "int - Signed integer value. Decimals are truncated.";
	parms << "item - A variable that references an item atom. Item are atoms that represent a commodities in the environment.";
	parms << "ll - A variable that contains a longitude latitude coordinate on a world.";
	parms << "npc - A variable that references an npc atom. All AI controlled citizens are npc atoms.";
	parms << "npcrole - A role for an AI controlled sentient: '''Citizen''', '''Worker''', '''Passenger''', '''Crew''', '''Troop''', '''Officer''', '''IndigenousPerson''', "
		"'''IndigenousElder''', '''IndigenousHealer''', '''IndigenousStargazer'''.";
	parms << "orbit - Orbit: '''Inferno''', '''Inner''', '''Habitable''', '''Outer''' or '''Frigid''' also '''Any''', '''Harsh''' (Inferno or "
		"Frigid), '''Mild''' (Inner, Habitable, or Outer), '''Uninhabitable''' (not Habitable).";
	parms << "quality - Quality in the range 1 to 255.";
	parms << "range - Communication range, not case sensitive.";
	parms << "real - Signed real value.";
	parms << "sc - Spacecraft blueprint id. Cannot be 0.";
	parms << "scenario - '''Reach for the Stars''', '''Citizen of the Galaxy''', '''The Savage State''', '''Lost Atlantis'''. "
		"Comparison is not case sensitive. ";
	parms << "sector - A variable that references a sector atom.";
	parms << "sentient - A variable that references a sentient atom. Avatars and AI controlled citizens are sentient atoms.";
	parms << "ship - A variable that references a spacecraft atom, could be a station.";
	parms << "stage - Growth stage of creature, not case sensitive: '''Mature''', '''Juvenile'''.";
	parms << "stance - Political stance, not case sensitive. '''Empire''', '''Vassal''', '''Friend''', '''Neutral''', '''Enemy'''. Often used in conjunction "
		"with a stanceCheck int value which is 0=exact match, 1=stance equal or better sought, 2=stance equal or worse sought.";
	parms << "station - Station that can be occupied, not case sensitive: '''Command''', '''Citizen''', '''Designer''', '''Engine''', '''FireControl''', '''Game''', '''Helm''', '''Livestock''', '''Medic''', '''Navigation''', '''Patient''', '''Power''', '''Sensor''', '''Shield''', '''Transporter''', '''Troop''', '''Turret'''";
	parms << "system - A variable that references a solar system atom.";
	parms << "text - Text string delimited with \" quotes. May be split across script lines. "
		"Can contain fields to include variable information. e.g. [A.F] where A is a valid atom variable and F is a field. "
		"Fields available depend on the atom type: all atoms have Name; ships have NameWithEmpire; people have TitleName; "
		"creatures have HerHim, herhim, HersHis, hershis, MaamSir, maamsir, MomDad, momdad, MotherFather, motherfather, MsMr, "
		"NieceNephew, niecenephew, SheHe, shehe. "
		"e.g. '[monster.HerHim]' would be substituted with 'Her' or 'Him', depending on the gender of the 'monster' atom.";
	parms << "unit - A variable that references a unit atom. Avatars, citizens, animals, vehicles and spacecraft are unit atoms.";
	parms << "vehicle - A variable that references a vehicle atom.";
	parms << "visage - A pose for the face: "
		"'''None''', "
		"'''Agape''', "
		"'''Closed''', "
		"'''Cry2Sec''', "
		"'''Frown''', "
		"'''Grim''', "
		"'''Grin''', "
		"'''Laugh1Sec''', "
		"'''Laugh2Sec''', "
		"'''Laugh4Sec''', "
		"'''Open''', "
		"'''OpenWide''', "
		"'''Perplexed''', "
		"'''Pursed''', "
		"'''Sad''', "
		"'''Sardonic''', "
		"'''Smile''', and "
		"'''Yawn'''.";
	parms << "void - A function return value placeholder indicating a function does not return anything.";
	parms << "world - A variable that references a world atom.";
	return parms;
}

//static
QStringList AuScript::ListParameterTypesWiki()
{
	QStringList pt = ListParameterTypes();
	for(QStringList::iterator pi = pt.begin(); pi != pt.end(); pi++)
	{
		QString wp = "===" + *pi;
		wp.replace(" - ", "===\n");
		*pi = wp;
	}
	pt.prepend("==Function Argument Types==");
	return pt;
}

//static
QStringList AuScript::ListFunctionPrototypes()
{
	QStringList protos;

	// List catalyst functions first.
	for(unsigned int func=0; func < s_StoryFuncs; func++)
	{
		if(s_StoryFunc[func].m_ReturnParm.isEmpty())
			continue;

		if(s_StoryFunc[func].m_ReturnParm == "#")
		{
			QString line;
			for(int i=0; i<s_StoryFunc[func].m_Name.length(); i++)
				line += "-";

			QString proto;
			if(s_StoryFunc[func].m_Desc.isEmpty())
				proto = QString("\n%1\n%2")
					.arg(s_StoryFunc[func].m_Name)
					.arg(line);
			else
				proto = QString("\n%1\n%2\n\n%3")
					.arg(s_StoryFunc[func].m_Name)
					.arg(line)
					.arg(s_StoryFunc[func].m_Desc);
			protos.append(proto);
		}
		else if(s_StoryFunc[func].m_Func == F_If)
		{
			QString proto = QString("%2(%3) blockName - %4")
				.arg(s_StoryFunc[func].m_Name)
				.arg(s_StoryFunc[func].m_Parms)
				.arg(s_StoryFunc[func].m_Desc);
			protos.append(proto);
		}
		else if(s_StoryFunc[func].m_Func == F_GoTo)
		{
			QString proto = QString("%2 blockName - %4")
				.arg(s_StoryFunc[func].m_Name)
				.arg(s_StoryFunc[func].m_Desc);
			protos.append(proto);
		}
		else
		{
			QString proto = QString("%1 %2(%3) - %4")
				.arg(s_StoryFunc[func].m_ReturnParm)
				.arg(s_StoryFunc[func].m_Name)
				.arg(s_StoryFunc[func].m_Parms)
				.arg(s_StoryFunc[func].m_Desc);
			protos.append(proto);
		}
	}
	return protos;
}

//static
QStringList AuScript::ListFunctionPrototypesWiki()
{
	QStringList protos;
	protos << QString("==Functions==");

	// List catalyst functions first.
	for(unsigned int func=0; func < s_StoryFuncs; func++)
	{
		if(s_StoryFunc[func].m_ReturnParm.isEmpty())
			continue;

		QString desc(s_StoryFunc[func].m_Desc);
		desc.replace("\t\t",":: ");
		desc.replace("\t",": ");

		if(s_StoryFunc[func].m_ReturnParm == "#")
		{
			// Section begin
			QString proto;
			if(desc.isEmpty())
				proto = QString("===%1===")
					.arg(s_StoryFunc[func].m_Name);
			else
				proto = QString("===%1===\n%2")
					.arg(s_StoryFunc[func].m_Name)
					.arg(desc);
			protos.append(proto);
		}
		else if(s_StoryFunc[func].m_Func == F_If)
		{
			QString proto = QString("====%2(%3) blockName====\n%4")
				.arg(s_StoryFunc[func].m_Name)
				.arg(s_StoryFunc[func].m_Parms)
				.arg(desc);
			protos.append(proto);
		}
		else if(s_StoryFunc[func].m_Func == F_GoTo)
		{
			QString proto = QString("====%2 blockName====\n%4")
				.arg(s_StoryFunc[func].m_Name)
				.arg(desc);
			protos.append(proto);
		}
		else
		{
			QString proto = QString("====%1 %2(%3)====\n%4")
				.arg(s_StoryFunc[func].m_ReturnParm)
				.arg(s_StoryFunc[func].m_Name)
				.arg(s_StoryFunc[func].m_Parms)
				.arg(desc);
			protos.append(proto);
		}
	}
	return protos;
}

void AuScript::CompilerAppendOutput(const QString& text)
{
	m_sOutput.append(text);
}

void AuScript::operator>>(QDataStream& out) const
{
	out << (quint32)m_Id;
	quint8 ver = 2;
	quint32 verSize = ((quint32)ver << 24) | (quint32)m_Body.size();	// Bodies can't have more than 16777215 lines each.
	out << verSize;
	for(unordered_map<QString, AuScriptLines, AuStringHash>::const_iterator bi = m_Body.cbegin(); bi != m_Body.cend(); bi++)
	{
		out << (*bi).first;
		out << (*bi).second;
	}

	out << (quint32)0;	// Used to be character definitions.
	out << m_sOutput;

	// ver 2
	out << m_BdDesigns;
	out << m_ScDesigns;
}

void AuScript::operator<<(QDataStream& in)
{
	quint32 ui32;
	in >> ui32;
	m_Id = ui32;

	quint32 verSize;
	in >> verSize;

	quint8 ver = (verSize >> 24);
	quint32 c = verSize & 0x00ffffff;
	m_Body.clear();
	QString key;
	while(c--)
	{
		AuScriptLines bval(ver);
		in >> key;
		in >> bval;
		m_Body.insert(pair<QString, AuScriptLines>(key, bval));
	}

	in >> ui32;
	Q_ASSERT(ui32 == 0);	// We should never have stored any character definitions.

	in >> m_sOutput;

	if(ver >= 2)
	{
		in >> m_BdDesigns;
		in >> m_ScDesigns;
	}
}

void AuScript::ListDesignsNeeded(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const
{
	bdDesigns.unite(m_BdDesigns);
	scDesigns.unite(m_ScDesigns);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

/*
void AuScripts::LoadAll()
{
	//qDebug("AuScripts::LoadAll");
	clear();

	// The folder names within the stories folder establish the story id. e.g. The folder './stories/3.GettingStarted' contains the
	// data files for story number 3. The '.GettingStarted' part is optional and ignored.
	QDir dir("./stories");
	dir.setFilter(QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Readable);
	dir.setSorting(QDir::Name);

	QFileInfoList list = dir.entryInfoList();
	for(int id=0; id<list.size(); id++)
	{
		QFileInfo fileInfo = list.at(id);
		int dotIdx = fileInfo.fileName().indexOf(".");
		unsigned int storyId;
		if(dotIdx < 0)
			storyId = QVariant(fileInfo.fileName()).toUInt();				// No '.anything' part.
		else
			storyId = QVariant(fileInfo.fileName().left(dotIdx)).toUInt();	// Ignore the '.something' part.
		if(storyId <= 0)
			continue;	// Skip when name before dot does not evaluate to a positive integer.
		if(find(storyId) != end())
			continue;	// Skip repeat of same story id. Not really possible.

		// Load and compile the script of the story.
		AuScript story(storyId);
		if(story.Load(fileInfo.filePath() + "/script.sh"))
		{
			// The script file loaded successfully and compiled ok.
			insert(pair<unsigned int, AuScript>(storyId, story));
		}
	}
}
*/

void AuScripts::LoadAllLib()
{
	AuStoryRes auStoriesRes;	// Load the AuStoryRes library long enough to load the data. Then it can go.
	clear();

	// The folder names within the stories folder establish the story id. e.g. The folder './stories/3.GettingStarted' contains the
	// data files for story number 3. The '.GettingStarted' part is optional and ignored.
	QDir dir(":stories");
	dir.setFilter(QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Readable);
	dir.setSorting(QDir::Name);

	QFileInfoList list = dir.entryInfoList();
	for(int id=0; id<list.size(); id++)
	{
		QFileInfo fileInfo = list.at(id);
		int dotIdx = fileInfo.fileName().indexOf(".");
		unsigned int storyId;
		if(dotIdx < 0)
			storyId = QVariant(fileInfo.fileName()).toUInt();				// No '.anything' part.
		else
			storyId = QVariant(fileInfo.fileName().left(dotIdx)).toUInt();	// Ignore the '.something' part.
		if(storyId <= 0)
			continue;	// Skip when name before dot does not evaluate to a positive integer.
		if(find(storyId) != end())
			continue;	// Skip repeat of same story id. Not really possible.

		// Load and compile the script of the story.
		AuScript story(storyId);
		if(story.LoadHar(fileInfo.filePath() + "/script.har"))
		{
			// The script file loaded successfully and compiled ok.
			insert(pair<unsigned int, AuScript>(storyId, story));
		}
	}
}

void AuScripts::Load(unsigned int scriptId)
{
	if(!scriptId)
		return;

	// The folder names within the stories folder establish the story id. e.g. The folder './stories/3.GettingStarted' contains the
	// data files for story number 3. The '.GettingStarted' part is optional and ignored.
	QDir dir("./stories");
	dir.setFilter(QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Readable);
	dir.setSorting(QDir::Name);

	iterator si = find(scriptId);
	if(si != end())
		erase(si);

	QFileInfoList list = dir.entryInfoList();
	for(int id=0; id<list.size(); id++)
	{
		QFileInfo fileInfo = list.at(id);
		int dotIdx = fileInfo.fileName().indexOf(".");
		unsigned int storyId;
		if(dotIdx < 0)
			storyId = QVariant(fileInfo.fileName()).toUInt();				// No '.anything' part.
		else
			storyId = QVariant(fileInfo.fileName().left(dotIdx)).toUInt();	// Ignore the '.something' part.
		if(storyId != scriptId)
			continue;	// Skip when its not the story id we're looking for.

		// Load and compile the script of the story.
		AuScript story(storyId);
		if(story.Load(fileInfo.filePath() + "/script.sh"))
		{
			// The script file loaded successfully and compiled ok.
			insert(pair<unsigned int, AuScript>(storyId, story));
			qDebug() << "AuScripts::Load" << fileInfo.fileName() << "succeeded.";
		}
		else
			qDebug() << "AuScripts::Load" << fileInfo.fileName() << "failed.";
	}
}

QString AuScripts::ScriptTitle(unsigned int scriptId) const
{
	const_iterator si = find(scriptId);
	if(si == cend())
		return "";
	return (*si).second.Title();
}

quint32 AuScripts::HighestId() const
{
	quint32 highest = 0;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).first > highest)
			highest = (*it).first;
	return highest;
}

void AuScripts::ListDesignsNeeded(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it).second.ListDesignsNeeded(bdDesigns, scDesigns);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuStory::AuStory(unsigned int storyInstanceId, unsigned int scriptId, const QString& startBlock,
		const QMap<QString, QVariant>& vars) : m_Variables(vars)
{
	m_Block = startBlock;
	m_BlockStart = AuTime::Now();
	m_CancelOk = true;
	m_ScriptId = scriptId;
	m_StoryInstanceId = storyInstanceId;
	m_Started = false;
}

void AuStory::operator>>(QDataStream& out) const
{
	quint8 ver = 0;
	out << ver;
	out << m_Block;
	out << m_BlockStart;
	out << m_CancelOk;
	out << m_People;
	out << m_ScriptId;
	out << m_Started;
	out << m_StoryInstanceId;
	out << m_Variables;
}

void AuStory::operator<<(QDataStream& in)
{
	quint8 ver;
	in >> ver;
	in >> m_Block;
	in >> m_BlockStart;
	in >> m_CancelOk;
	in >> m_People;
	in >> m_ScriptId;
	in >> m_Started;
	in >> m_StoryInstanceId;
	in >> m_Variables;
}

void AuStory::Insert(QSqlDatabase& db) const
{
	Q_ASSERT(db.isOpen());
	Q_ASSERT(m_StoryInstanceId);
	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO g_Story (id, block, blockStart, people, vars, script, canxOk) "
			"VALUES (?,?,?,?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(m_StoryInstanceId);
	qa.addBindValue(m_Block);
	qa.addBindValue(m_BlockStart.UTC());

	// blob of people ids
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_People >> out;
		qa.addBindValue(ba);
	}

	// Variables
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);

		out << (quint16)m_Variables.count();
		for(QMap<QString, QVariant>::const_iterator vi = m_Variables.cbegin(); vi != m_Variables.cend(); vi++)
		{
			out << vi.key();
			out << vi.value();
		}

		qa.addBindValue(ba);
	}

	qa.addBindValue(m_ScriptId);
	qa.addBindValue(m_CancelOk);

	if(!qa.exec())
		qWarning() << "ERROR! AuStory::Insert FAILED" << sq;
}

void AuStory::Update(QSqlDatabase& db) const
{
	Q_ASSERT(db.isOpen());
	Q_ASSERT(m_StoryInstanceId);
	QString sq = QString("UPDATE g_Story SET block=?, blockStart=?, people=?, vars=?, canxOk=? WHERE id=%1").arg(m_StoryInstanceId);
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(m_Block);
	qa.addBindValue(m_BlockStart.UTC());

	// blob of people ids
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_People >> out;
		qa.addBindValue(ba);
	}

	// Variables
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);

		out << (quint16)m_Variables.count();
		for(QMap<QString, QVariant>::const_iterator vi = m_Variables.cbegin(); vi != m_Variables.cend(); vi++)
		{
			out << vi.key();
			out << vi.value();
		}

		qa.addBindValue(ba);
	}

	qa.addBindValue(m_CancelOk);

	if(!qa.exec())
		qWarning() << "ERROR! AuStory::Update FAILED" << sq;
}

bool AuStory::Select(QSqlDatabase& db)
{
	Q_ASSERT(db.isOpen());
	Q_ASSERT(m_StoryInstanceId);
	QString sq = QString("SELECT block, blockStart, people, vars, script, canxOk FROM g_Story WHERE id=%1;").arg(m_StoryInstanceId);
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	if(qry.next())
	{
		int qv=0;
		m_Block = qry.value(qv++).toString();
		QDateTime blockStart = qry.value(qv++).toDateTime();
		blockStart.setTimeSpec(Qt::UTC);
		m_BlockStart = blockStart;

		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			m_People << in;
		}

		// Variables
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);

			m_Variables.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				QString key;
				QVariant val;

				in >> key;
				in >> val;

				m_Variables.insert(key, val);
			}
		}

		m_ScriptId = qry.value(qv++).toUInt();
		m_CancelOk = qry.value(qv++).toBool();
		SetStarted();

		//if(m_ScriptId == 8)
			//AuScriptLine::DumpVars(m_Variables);

		return true;
	}
	return false;
}

// Branches to a block of the story and executes the block.
void AuStory::BranchTo(const QString& blockLabel, const AuScript& script, DaPerson& person, bool& storyCancelOk)
{
	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuStory::BranchTo %1 script %2 block %3 person %4 %5")
			.arg(m_StoryInstanceId)
			.arg(script.Id())
			.arg(blockLabel)
			.arg(person.Id().GetID())
			.arg(person.Name());
		g_Au.Log(logmsg);
	}

	m_Block = blockLabel;
	m_BlockStart.Set();
	ExecuteBlock(script, person, storyCancelOk);	// Executes the current block.
}

// Executes the current block.
void AuStory::ExecuteBlock(const AuScript& script, DaPerson& person, bool& storyCancelOk)
{
	const AuScriptLines& block = script.Block(m_Block);

	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuStory::ExecuteBlock %1 script %2 block %3 person %4 %5 %6 lines")
			.arg(m_StoryInstanceId)
			.arg(script.Id())
			.arg(m_Block)
			.arg(person.Id().GetID())
			.arg(person.Name())
			.arg(block.size());
		g_Au.Log(logmsg);
	}

	// Execute the lines of the block in the story script.
	for(AuScriptLines::const_iterator si = block.cbegin(); si != block.cend(); si++)
		(*si).Execute(script, m_StoryInstanceId, person, m_Variables, m_BlockStart, storyCancelOk);
}

// Resends the current block. For instance, a the current block ends in a list of choices. The player logs off. The next time
// they log on they need the list of choices again, without necessarily executing every function of the block all over again.
void AuStory::ResendBlock(const AuScript& script, DaPerson& person) const
{
	QMap<QString, QVariant> vars(m_Variables);

	// Resend the lines of the block in the story script.
	bool storyCancelOk = m_CancelOk;
	const AuScriptLines& block = script.Block(m_Block);
	for(AuScriptLines::const_iterator si = block.cbegin(); si != block.cend(); si++)
		(*si).Resend(script, m_StoryInstanceId, person, vars, m_BlockStart, storyCancelOk);
}

// Send the hint(s) associated with the current story block.
void AuStory::SendHint(const AuScript& script, DaPerson& person) const
{
	bool hintSent = false;
	const AuScriptLines& block = script.Block(m_Block);
	for(AuScriptLines::const_iterator si = block.cbegin(); si != block.cend(); si++)
		if((*si).SendHint(script, m_StoryInstanceId, person, m_Variables))
			hintSent = true;

	if(!hintSent)
	{
		AuCommNarrate msg(QObject::tr("No hint available."), m_StoryInstanceId, script.Title(), AuId(), GALAXY_Count);
		g_Au.SendAtomChat(msg);
	}
}

bool AuStory::ChoicePicked(DaPerson& person, qint8 value, const AuScript& script)
{
	// Found the script, which contains the blocks of the story.
	QString branchToLabel;
	if(!script.ChoicePicked(person, m_StoryInstanceId, Block(), value, BlockStart(), branchToLabel, m_Variables))
		return false;	// Not ok to branch now.

	// The script says its ok to branch now.
	// Branch to the next story label and execute it.
	Q_ASSERT(!branchToLabel.isEmpty());
	BranchTo(branchToLabel, script, person, m_CancelOk);
	return true;
}

int AuStory::Execute(const AuScript& script, DaPerson& person)
{
	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = QObject::tr("AuStory::Execute %1 script %2 %3 block %4 person %5 %6")
			.arg(m_StoryInstanceId)
			.arg(ScriptId())
			.arg(m_Started?"continuing":"starting")
			.arg(Block())
			.arg(person.Id().GetID())
			.arg(person.Name());
		g_Au.Log(logmsg);
	}

	int retval;
	if(!m_Started)
	{
		// Execute the start block, using constructor values for initial block name.
		SetStarted();
		ExecuteBlock(script, person, m_CancelOk);
		retval = 0;
	}
	else
		retval = ExecuteContinue(script, person, m_CancelOk);
	return retval;
}

// Returns 0=did not branch, 1=branched, 2=story ended
int AuStory::ExecuteContinue(const AuScript& script, DaPerson& person, bool& storyCancelOk)
{
	QString branchToLabel;
	if(!script.TestBranch(person, m_StoryInstanceId, Block(), BlockStart(), branchToLabel, m_Variables, storyCancelOk))
	{
		//qDebug() << "AuStory::ExecuteContinue no branch in script" << script.Id() << "storyInstance" << m_StoryInstanceId
			//<< "person" << person.Id() << person.Name();
		return 0;	// The script says not to branch now.
	}

	// The script says to branch now.
	// If the goto label is blank, the story is over.
	if(branchToLabel.isEmpty())
	{
		if(g_Au.m_LogMessages & LOG_Story)
		{
			QString logmsg = QObject::tr("AuStory::ExecuteContinue %1 script %2 person %3 %4 Story Ended")
				.arg(m_StoryInstanceId)
				.arg(script.Id())
				.arg(person.Id().GetID())
				.arg(person.Name());
			g_Au.Log(logmsg);
		}

		return 2;	// Depart the story.
	}

	// Branch to the next story label and execute it.
	//qDebug() << "AuStory::ExecuteContinue story branched to" << branchToLabel << "in script" << script.Id()
		//<< "storyInstance" << m_StoryInstanceId << "person" << person.Id() << person.Name();
	BranchTo(branchToLabel, script, person, storyCancelOk);
	return 1;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuStories::~AuStories()
{
	Clear();
}

void AuStories::Clear()
{
	for(iterator si = begin(); si != end(); si++)
		delete (*si).second;
	clear();
}

void AuStories::Load(QSqlDatabase& db)
{
	Q_ASSERT(db.isOpen());
	QString sq("SELECT id, block, blockStart, people, vars, script FROM g_Story;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		unsigned int storyInstance = qry.value(qv++).toUInt();
		QString block = qry.value(qv++).toString();
		QDateTime blockStart = qry.value(qv++).toDateTime();
		blockStart.setTimeSpec(Qt::UTC);

		AuIdList people;
		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			people << in;
		}

		// Variables
		QMap<QString, QVariant> vars;
		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);

			vars.clear();
			quint16 c;
			in >> c;
			while(c--)
			{
				QString key;
				QVariant val;

				in >> key;
				in >> val;

				vars.insert(key, val);
			}
		}

		unsigned int scriptId = qry.value(qv++).toUInt();

		AuTime t;
		t = blockStart;
		AuStory* story = new AuStory(storyInstance, scriptId, block, vars);
		story->Join(people);
		story->SetBlockStart(t);
		story->SetStarted();
		insert(pair<unsigned int, AuStory*>(storyInstance, story));
	}
}

void AuStories::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(unordered_map<unsigned int, AuStory*, AuStoryInstanceHash>::const_iterator si = cbegin(); si != cend(); si++)
		out << *(*si).second;
}

void AuStories::operator<<(QDataStream& in)
{
	Clear();
	quint16 ui16;
	in >> ui16;
	while(ui16--)
	{
		AuStory* s = new AuStory();
		in >> *s;
		insert(pair<unsigned int, AuStory*>(s->InstanceId(), s));
	}
}

void AuStories::ClearDefunct()
{
	iterator si = begin();
	while(si != end())
	{
		if((*si).second->IsDefunct())
		{
			delete (*si).second;
			si = erase(si);
		}
		else
			si++;
	}
}

