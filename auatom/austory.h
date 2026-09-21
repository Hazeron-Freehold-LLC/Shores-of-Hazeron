//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austory.h,v 1.135 2022/05/16 22:43:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStory_H
#define AuStory_H

#include "auatom.h"
#include "auglobal.h"
#include "auglobell.h"
#include "auid.h"
#include "autime.h"
#include <QMetaType>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QVariant>

class AuAtomPos;
class AuDNA;
class AuDNABase;
class AuScript;
class AuStories;
class DaNpc;
class DaPerson;
class DaSentient;
class DaVessel;
class DaWorldLL;
class DhDNA;
class QDataStream;
class QDebug;
class QSqlDatabase;
class QTextStream;

class AUATOM_DLL AuScriptIdHash
{
public:
	inline size_t operator()(const unsigned int& o) const
	{
		return hash<unsigned int>()(o);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuStoryInstanceHash
{
public:
	inline size_t operator()(const unsigned int& o) const
	{
		return hash<unsigned int>()(o);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// So far it's ok to insert or reorder these. The value is not serialized anywhere.
// NOTE: The order of this enum must match the order of the entries in the s_StoryFunc array.
enum SCRIPT_FUNC
{
	F_MiscDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_NOP,								// No op
	F_CONST,							// Constant numeric value.
	F_VAR,								// Defined variable value.
	F_AND,								// AND boolean operation
	F_OR,								// OR boolean operation
	F_EQ,								// == boolean operation
	F_LE,								// <= boolean operation
	F_LT,								// < boolean operation
	F_GE,								// >= boolean operation
	F_GT,								// > boolean operation
	F_GoTo,								// unconditional story branch
	F_If,								// evaluation for story branch

	F_ChoiceMade,
	F_DateIsChristmas,					// true if the current date is 12/24 or 12/25
	F_SecondsElapsed,
	F_Wait,

	// Story functions define the story in the catalyst.
	F_StoryDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_StoryAuthor,						// declares story author(s) in catalyst block
	F_StoryAutoRepeat,
	F_StoryAutoRepeatUntilComplete,
	F_StoryAutoStart,					// Enables story to start once automatically when the catalyst conditions are right.
	F_StoryBrief,						// declares brief description of story in catalyst block
	F_StoryDroppable,
	F_StoryInterval,					// minutes between restarts in catalyst block
	F_StoryTitle,						// declares story title in catalyst block

	// Avatar functions operate on the avatar who is involved in the story.
	F_AvatarDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_Avatar,
	F_AvatarConstructedBuildingHabitable,	// Test to see if the avatar has ever planned construction of a building on hab world.
	F_AvatarConstructedBuildingHarsh,	// Test to see if the avatar has ever planned construction of a building in harsh env.
	F_AvatarCreditGovtDebt,
	F_AvatarCurrentItem,
	F_AvatarEnvironment,
	F_AvatarEVAMinutes,
	F_AvatarHadItem,					// true if person has ever possessed a commodity in their inventory
	F_AvatarHasFood,					// true if any food is in inventory
	F_AvatarHasItem,					// true if commodity is in inventory. Parm: commodity
	F_AvatarHasItemInHand,
	F_AvatarHasItemInHandSpecific,
	F_AvatarHasItemSpecific,
	F_AvatarInCity,						// true if person is in a city
	F_AvatarInitScenario,				// true if an avatar started using a specified scenario
	F_AvatarInOrbit,					// true if person is in orbit around a planet
	F_AvatarInStudio,					// true if inside a design studio
	F_AvatarInVehicle,					// true if parent atom is a vehicle
	F_AvatarInVehicleMoving,			// true if parent atom is a vehicle that is moving
	F_AvatarInVehicleOnGround,			// true if parent atom is a vehicle that is moving
	F_AvatarItemCount,
	F_AvatarItemMakePermanent,
	F_AvatarItemSetDropOnDeath,
	F_AvatarLineageAvatar,				// true if avatar lineage is a regular avatar
	F_AvatarLineageDemiavatar,			// true if avatar lineage is demiavatar
	F_AvatarLineageHighborn,			// true if avatar lineage is highborn
	F_AvatarOnLineMinutes,
	F_AvatarStoryCompleted,				// Test to see if the current person has completed a particular story script.
	F_AvatarStoryLastResult,			// Returns the result of the last run of the story by the avatar.
	F_AvatarStoryResult,				// Remembers a result. Can be called at any time for remembering an intermediate result.
	F_AvatarVehicleMinutes,

	// Atom functions operate on a specific atom.
	F_AtomDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_Atom,
	F_AtomCity,
	F_AtomComm,
	F_AtomCommChannelToAtom,
	F_AtomCommChoice,
	F_AtomCreateCharacter,
	F_AtomDistanceToAtom,
	F_AtomEmpire,
	F_AtomFindBuilding,
	F_AtomFindVehicle,
	F_AtomHasAtom,
	F_AtomHasBuilding,
	F_AtomInSystem,
	F_AtomIsAtom,
	F_AtomSector,
	F_AtomValid,
	F_AtomWorld,
	F_AtomWorldLL,

	F_BlueprintDocTitle,				// Not an actual function. Used when generating documention from this table.
	F_BlueprintSpacecraftLoad,

	// Character functions operate on character atoms.
	F_CharacterDocTitle,				// Not an actual function. Used when generating documention from this table.
	F_CharacterDepart,
	F_CharacterDie,
	F_CharacterEquipFromCity,
	F_CharacterSetIndestructible,
	F_CharacterWantItem,
	F_CharacterWantItemSpecific,

	// City functions operate on city and bd atoms.
	F_CityDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_CityFindNewVehicle,
	F_CityHasBuilding,					// true if minimum count of completed building levels in town
	F_CityHasFood,						// true if any food is in inventory
	F_CityHasItem,						// true if commodity is present in inventory. Vehicles must exist in town in nu condition.
	F_CityHasPower,						// true if power is present in inventory.
	F_CityItemCount,
	F_CityPopulation,

	// Creature functions operate on creature atoms.
	F_CreatureDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_CreatureDyingOrDead,
	F_CreatureSetHealth,
	F_CreatureSetMouth,
	F_CreatureSetVisage,

	// Find functions locate atoms in the current environment, e.g. solar system, arena, design studio.
	F_FindDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_FindGasGiant,
	F_FindMoon,
	F_FindPlanet,
	F_FindRing,
	F_FindRingWorld,

	// Henge functions access data cached about henge destination worlds.
	F_HengeDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_HengeSystemName,
	F_HengeSystemWorld,
	F_HengeWorld,
	F_HengeWorldLL,
	F_HengeWorldName,
	F_HengeWorldPortalActive,

	// Item functions operate on item atoms.
	F_ItemDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_ItemArmWeapon,
	F_ItemCreateContent,
	F_ItemDetonate,
	F_ItemMakePermanent,
	F_ItemOnGround,
	F_ItemOnGroundSpecific,
	F_ItemSetDropOnDeath,
	F_ItemSetMessage,
	F_ItemSetState,

	// Narrator functions communicate with the person in the story.
	F_NarrDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_NarrHint,							// an optional text hint about what to do next. the user must request the hint.
	F_NarrImage,
	F_NarrSay,							// narrate text to player
	F_NarrSayAudio,						// play a sound file
	F_NarrSayChoice,					// Parms: value, text of option choice
	F_NarrTitle,						// narrate title text of block to player
	F_NarrVideo,						// Show a video file.

	// Npc functions operate on npc atoms.
	F_NpcDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_NpcSetRole,

	// Sector functions operate on sector atoms.
	F_SectorDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_SectorHasHengeSystem,
	F_SectorHengeSystem,

	// Sentient functions operate on sentient being atoms.
	F_SentientDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_SentientAttackUnit,
	F_SentientCreateGear,
	F_SentientDropGear,
	F_SentientDropGearItem,
	F_SentientFireWeapon,
	F_SentientFollowAvatar,
	F_SentientFollowSentient,
	F_SentientGetItem,
	F_SentientGoToAvatar,
	F_SentientGoToLL,
	F_SentientGoToLLExactly,
	F_SentientGoToRoom,
	F_SentientGoToUnit,
	F_SentientHasItem,
	F_SentientHasItemSpecific,
	F_SentientHengeCeremony,
	F_SentientItemMakePermanent,
	F_SentientItemSetDropOnDeath,
	F_SentientMoveBack,
	F_SentientMoveForward,
	F_SentientSay,
	F_SentientSayAudio,
	F_SentientSayChoice,
	F_SentientSayImage,
	F_SentientSayTo,
	F_SentientSellGearItem,
	F_SentientStay,

	// Ship functions operate on spaceship atoms.
	F_ShipDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_ShipChargeCapacitor,
	F_ShipChargeShields,
	F_ShipCreateCharacter,
	F_ShipCreateCrew,
	F_ShipCreateTroops,
	F_ShipFindAI,
	F_ShipFindAIInRoom,
	F_ShipFindNpc,
	F_ShipFindNpcInRoom,
	F_ShipInteriorLight,
	F_ShipManeuverDrive,
	F_ShipPowerPlant,
	F_ShipRefuel,
	F_ShipRetreat,
	F_ShipShields,
	F_ShipStationOccupant,
	F_ShipStationOccupantInRoom,

	// Unit functions operate on unit atoms.
	F_UnitDocTitle,						// Not an actual function. Used when generating documention from this table.
	F_UnitAboardShip,
	F_UnitAtBuilding,
	F_UnitAtWorld,
	F_UnitInOrbit,
	F_UnitInsideShip,
	F_UnitRoomName,
	F_UnitStationInTradeRange,

	// Vehicle functions operate on vehicle atoms.
	F_VehicleDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_VehicleRefuel,

	// World functions operate on world atoms.
	F_WorldDocTitle,					// Not an actual function. Used when generating documention from this table.
	F_WorldAtmosphereBreathable,
	F_WorldClaimed,
	F_WorldClaimedBy,
	F_WorldCreateCharacter,
	F_WorldCreateIndigenous,
	F_WorldCreateItem,
	F_WorldCreateShip,
	F_WorldCreateVehicle,
	F_WorldHasCity,
	F_WorldHengeCeremony,
	F_WorldHydrographics,
	F_WorldIsGasGiant,
	F_WorldIsMoon,
	F_WorldIsPlanet,
	F_WorldIsPlanetoid,
	F_WorldIsRing,
	F_WorldIsRingworld,
	F_WorldIsTitan,
	F_WorldLandLocation,
	F_WorldLandLocationNear,
	F_WorldOrbitFrigid,
	F_WorldOrbitHabitable,
	F_WorldOrbitInferno,
	F_WorldOrbitInner,
	F_WorldOrbitOuter,
	F_WorldRelativeLL,
	F_WorldScheduleAsteroid,
	F_WorldSupportsAnimals,
	F_WorldSupportsPlants,
	F_Count								// LAST ITEM!
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuScriptBranch
{
	QString								m_BlockName;			// The block that contains the branch line.
	int									m_LineNumber;			// The line number of the branch line.
	QString								m_LineText;				// The branch line text.
	QString								m_ToLabel;				// The block label that is the destination of the branch.

public:	// Constructors
	inline AuScriptBranch(const QString& blockName, int lineNumber, const QString& lineText, const QString& toLabel)
	{
		m_BlockName = blockName;
		m_LineNumber = lineNumber;
		m_LineText = lineText;
		m_ToLabel = toLabel;
	}

	inline AuScriptBranch(const AuScriptBranch& c)
	{
		m_BlockName = c.m_BlockName;
		m_LineNumber = c.m_LineNumber;
		m_LineText = c.m_LineText;
		m_ToLabel = c.m_ToLabel;
	}

public:	// Operators
	inline const AuScriptBranch& operator=(const AuScriptBranch& c)
	{
		m_BlockName = c.m_BlockName;
		m_LineNumber = c.m_LineNumber;
		m_LineText = c.m_LineText;
		m_ToLabel = c.m_ToLabel;
		return *this;
	}

public:	// Methods
	inline const QString& BlockName() const
	{
		return m_BlockName;
	}

	inline int LineNumber() const
	{
		return m_LineNumber;
	}

	inline const QString& LineText() const
	{
		return m_LineText;
	}

	inline const QString& ToLabel() const
	{
		return m_ToLabel;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuScriptLine
{
private:
	QString								m_Assignee;				// Name of variable assigned the result of this function.
	SCRIPT_FUNC							m_Func;
	bool								m_Negate;				// true if this eval result is NOT'ed
	QList<QVariant>						m_Parms;

public:	// Constructors
	AuScriptLine(const QString& blockName, const QString& lineText, bool isEval, const QString& scriptFile, int scriptLine,
			AuScript& script, QStringList& varDefs, QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns,
			QMap<int, AuScriptBranch>& branches);
	AuScriptLine(SCRIPT_FUNC func);
	AuScriptLine();
	virtual ~AuScriptLine();

	inline AuScriptLine(const AuScriptLine& c) : m_Parms(c.m_Parms)
	{
		m_Assignee = c.m_Assignee;
		m_Func = c.m_Func;
		m_Negate = c.m_Negate;
	}

public:	// Operators
	inline AuScriptLine& operator=(const AuScriptLine& c)
	{
		m_Assignee = c.m_Assignee;
		m_Func = c.m_Func;
		m_Negate = c.m_Negate;
		m_Parms = c.m_Parms;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline const QString& Assignee() const
	{
		return m_Assignee;
	}

	static void DumpVars(const QMap<QString, QVariant>& vars);

	// Perform the actions of this script line.
	// 'storyCancelOk' may be changed to indicate if the story is droppable or not.
	void Execute(const AuScript& script, quint32 storyInstance, DaPerson& person, QMap<QString, QVariant>& vars,
			const AuTime& blockStart, bool& storyCancelOk) const;

	inline SCRIPT_FUNC Function() const
	{
		return m_Func;
	}

	void GoToBranch(QString& branchToBlock) const;

	inline bool Negate() const
	{
		return m_Negate;
	}

	inline QVariant Parameter(unsigned int index) const
	{
		if(index < (unsigned int)m_Parms.count())
			return m_Parms[index];
		return QVariant();
	}

	inline const QList<QVariant>& Parameters() const
	{
		return m_Parms;
	}

	static int RegisterMetaType();

	void Resend(const AuScript& script, quint32 storyInstance, DaPerson& person, QMap<QString, QVariant>& vars,
			const AuTime& blockStart, bool& storyCancelOk) const;

	bool SendHint(const AuScript& script, quint32 storyInstance, DaPerson& person, const QMap<QString, QVariant>& vars) const;

	// Valid range of 'choicePicked' values is -127 to 127. The default guarantees that all F_ChoiceMade comparisons will fail,
	// unless a valid value is passed.
	// 'storyInstance' is 0 when testing branches in the catalyst.
	bool TestBranch(DaPerson& person, quint32 storyInstance, const AuTime& blockStart, QString& branchToBlock,
			QMap<QString, QVariant>& vars, int choicePicked=1000) const;

private:
	QVariant AtomCity(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomCreateCharacter(DaPerson& person, const QMap<QString, QVariant>& vars, unsigned int scriptId,
			quint32 storyInstance) const;

	QVariant AtomDistanceToAtom(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomEmpire(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomFindBuilding(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomFindVehicle(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomHasAtom(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomHasBuilding(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomOpenChannel(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant AtomSector(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	// Send a comm message from any atom.
	void AtomSendComm(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, const QString& channel,
			AuCommStdChannel::RANGE range, const AuId& fromId) const;

	void AtomSendCommChoice(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, int choiceValue,
			const QString& channel, AuCommStdChannel::RANGE range, const AuId& fromId) const;

	QVariant AtomWorld(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant CityFindNewVehicle(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant CityHasBuilding(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant CityHasFood(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant CityHasItem(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant CityHasPower(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant CityItemCount(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant CityPopulation(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant FindGasGiant(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant FindMoon(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant FindPlanet(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant FindRing(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant FindRingWorld(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	DaSentient* FindSentient(const DaPerson& person, const AuId& sentId) const;

	DaVessel* FindShip(const DaPerson& person, const AuId& shipId) const;

	QVariant ItemOnGround(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant ItemOnGroundSpecific(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	// Insure the character named 'charName' exists within 'range' of 'person'.
	// 'vars' are updated with a new atom id if the character must be restored from memory.
	DaNpc* RestoreCharacter(DaPerson& person, quint32 storyInstance, const QString& charName, double range,
			QMap<QString, QVariant>& vars) const;

	// When an existing character is restored, it is placed at 'charPos' unless it is already closer than 'range' to the 'person'.
	DaNpc* RestoreCharacterFromMemory(DaPerson& person, quint32 storyInstance, DaAtom* env, const QString& charName,
			const AuAtomPos& charPos, double range) const;

	QVariant SectorHengeSystem(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	// Say a comm message from a sentient atom to someone. This causes them to turn to face the person to whom they are speaking.
	// 'toWho' can be null.
	void SentientSay(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, const QString& channel,
			AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho, const AuVisage& visage) const;

	void SentientSayAudio(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, const QString& channel,
			AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho, const QByteArray& soundData,
			const AuVisage& visage) const;

	void SentientSayChoice(DaPerson& sceneAtom, quint32 storyInstance, const QString& messageText, int choiceValue,
			const QString& channel, AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho,
			const AuVisage& visage) const;

	void SentientSayImage(DaPerson& sceneAtom, quint32 storyInstance, const QString& imageTitle, const QString& messageText,
			const QString& channel, AuCommStdChannel::RANGE range, DaSentient& sent, const AuId& toWho, const QImage& image,
			const AuVisage& visage) const;

	//QVariant SystemHengeWorld(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant SystemHengeWorldCatName(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	// 'stanceOfUnit' is an active stance like that of a city toward the story character.
	// 'stanceParameter' is the stance parameter from the script.
	// 'stanceCheck' is
	//   0='stanceOfUnit' must match 'stanceParameter'.
	//   1='stanceOfUnit' must match or be better than 'stanceParameter'.
	//   2='stanceOfUnit' must match or be worse than 'stanceParameter'.
	static bool CompareStance(POLITICAL_STANCE stanceOfUnit, POLITICAL_STANCE stanceParameter, int stanceCheck);

	void Compile(const QString& lineText, bool isEval, const QString& scriptFile, int scriptLine, AuScript& script,
			QStringList& varDefs, QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns);

	static bool DNALoadFile(const QString& fname, AuDNA& dna);

	static bool DNALoadFile(const QString& fname, DhDNA& dna);

	// 'world' can be null. It is only used when parm is 'Indigenous'. If 'world' is null, the person's parent world is used. If no
	// parent world then the person's DNA is used.
	// Caller must delete the pointer.
	static AuDNABase* DNAParameterNew(const QVariant& parm, DaPerson& person, const DaWorldLL* world=0);

	static quint32 EmpireId(qint32 empireSpec, const DaPerson& person);

	// This function is only executed on script lines that are part of an 'if' statement.
	// 'storyInstance' is used when locating existing characters. It can be 0, such as during catalyst checking.
	QVariant Eval(DaPerson& person, quint32 storyInstance, const AuTime& blockStart, QMap<QString, QVariant>& vars,
			int choicePicked) const;

	static AuMatrix4x4 FindStartingLocation(const DaAtom* refAtom, float angle, double disance);

	static AuMatrix4x4 FindStartingLocationAboard(const DaVessel* ship, int placement);

	// Send a comm message from the narrator.
	void NarratorSendComm(DaPerson& person, quint32 storyInstance, const QString& messageText, const QString& channel) const;

	// Send a comm message from the narrator.
	void NarratorSendCommAudio(DaPerson& person, quint32 storyInstance, const QString& messageText, const QString& channel,
		const QByteArray& soundData) const;

	// Send a choice by comm message from the narrator.
	void NarratorSendCommChoice(DaPerson& person, quint32 storyInstance, const QString& messageText, int choiceValue,
			const QString& channel) const;

	// Send a comm message from the narrator.
	void NarratorSendCommImage(DaPerson& person, quint32 storyInstance, const QString& imageTitle, const QString& messageText,
			const QString& channel, const QImage& image) const;

	// Send a title by comm message from the narrator.
	void NarratorSendCommTitle(DaPerson& person, quint32 storyInstance, const QString& messageText,
			const QString& channel) const;

	QStringList SplitFields(const QString& parms) const;

	// Substitutes fields in the 'parm' string using atom variable values.
	static QString StringParameter(const QString& parm, DaPerson& person, const QMap<QString, QVariant>& vars);

	static bool ValidateStringParameter(const QString& parm, const QStringList& varDefs);

	static AuId VariableAtomId(const QString& varName, const QMap<QString, QVariant>& vars);

	static AuGlobeLLF VariableLL(const QString& varName, const QMap<QString, QVariant>& vars);

	static QString VariableString(const QString& varName, const QMap<QString, QVariant>& vars);

	QVariant WorldAtmosphereBreathable(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	void WorldScheduleAsteroid(DaPerson& person, const AuId& worldId, int minsToImpact) const;

	QVariant WorldSupportsAnimals(DaPerson& person, const QMap<QString, QVariant>& vars) const;

	QVariant WorldSupportsPlants(DaPerson& person, const QMap<QString, QVariant>& vars) const;
};

// Declared as a metatype so it can be stored in a QVariant.
// Also requires call to qRegisterMetaType.
Q_DECLARE_METATYPE(AuScriptLine);

QDataStream& operator<<(QDataStream& out, const AuScriptLine& o);
QDataStream& operator>>(QDataStream& in, AuScriptLine& o);

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuScriptLine& o);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuScriptLines : public QList<AuScriptLine>
{
	int									m_StartLine;			// Line number in script at which this block started.
	quint8								m_VerRead;				// Set when deserializing. Not stored.

public:
	inline AuScriptLines()
	{
		m_StartLine = 0;
		m_VerRead = 0;
	}

	inline AuScriptLines(quint8 readVer)
	{
		m_StartLine = 0;
		m_VerRead = readVer;
	}

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	inline int StartLine() const
	{
		return m_StartLine;
	}

	inline void SetStartLine(int startLine)
	{
		m_StartLine = startLine;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScriptLines& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScriptLines& o)
{
	o << in;
	return in;
}

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuScriptLines& o);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuScript
{
private:
	QSet<quint32>						m_BdDesigns;							// Bd designs used by this script.
	unordered_map<QString, AuScriptLines, AuStringHash>			m_Body;			// Key=block label. Data=compiled lines of script.
	unsigned int						m_Id;									// Script id unique to this story. 0=Custom.
	QSet<quint32>						m_ScDesigns;							// Sc designs used by this script.
	QStringList							m_sOutput;								// Lines of compiler output.
	int									m_Result;								// Last result of calling Compile.

public:	// Constructors
	inline AuScript(unsigned int id)
	{
		Q_ASSERT(id);
		m_Id = id;
		m_Result = -1;
	}

	inline AuScript()
	{
		m_Id = 0;
		m_Result = -1;
	}

	inline AuScript(const AuScript& c) : m_BdDesigns(c.m_BdDesigns), m_Body(c.m_Body), m_ScDesigns(c.m_ScDesigns),
		m_sOutput(c.m_sOutput)
	{
		m_Id = c.m_Id;
		m_Result = c.m_Result;
	}

public:	// Operators
	inline AuScript& operator=(const AuScript& c)
	{
		m_BdDesigns = c.m_BdDesigns;
		m_Body = c.m_Body;
		m_Id = c.m_Id;
		m_Result = c.m_Result;
		m_ScDesigns = c.m_ScDesigns;
		m_sOutput = c.m_sOutput;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	// Returns true if AutoRepeat, AutoRepeatUntilComplete, or AutoStart.
	bool AnyAutoStart() const;

	QString Author() const;

	// Returns true if this story is allowed to autostart repeatedly. When false, a story can only autostart once for each person.
	bool AutoRepeat() const;

	// Returns true if this story is allowed to autostart repeatedly until finished. When false, a story can only autostart once
	// for each person.
	bool AutoRepeatUntilComplete() const;

	// Returns true if this story is allowed to start automatically based on catalyst conditions.
	bool AutoStart() const;

	const AuScriptLines& Block(const QString& blockLabel) const;

	inline int BlockCount() const
	{
		return (int)m_Body.size();
	}

	inline const unordered_map<QString, AuScriptLines, AuStringHash>& Body() const
	{
		return m_Body;
	}

	QString Brief() const;

	// returns branch-to label resulting from choice picked
	bool ChoicePicked(DaPerson& person, quint32 storyInstance, const QString& blockLabel, qint8 value, const AuTime& blockStart,
			QString& branchToBlock, QMap<QString, QVariant>& vars) const;

	// Returns 0=ok, 1=error, 2=#nocompile
	int Compile(const QString& scriptFileName);

	void CompilerAppendOutput(const QString& text);

	inline int CompileResult() const
	{
		return m_Result;
	}

	inline const QStringList& CompilerOutput() const
	{
		return m_sOutput;
	}

	int Interval() const;	// Minutes required between restarts.

	// Lists design ids needed by all scripts. Used to preload and maintain those designs in the cache.
	void ListDesignsNeeded(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const;

	// Returns a list of function prototypes in human-readable form.
	static QStringList ListFunctionPrototypes();

	// Returns a list of function prototypes in wiki form.
	static QStringList ListFunctionPrototypesWiki();

	// Returns a list of function parameter types in human-readable form.
	static QStringList ListParameterTypes();

	// Returns a list of function parameter types in wiki form.
	static QStringList ListParameterTypesWiki();

	// Loads the script from a script.sh file.
	// Returns true if the script is loaded and compiles ok.
	bool Load(const QString& fileName);

	// Loads the script from a script.sh file contained in a .har file.
	bool LoadHar(const QString& harFileName);

	inline unsigned int Id() const
	{
		return m_Id;
	}

	QString Title() const;

	// Returns true if conditions are right to branch to another block of the story from 'blockLabel', started at 'blockStart'.
	// 'branchToBlock' is set to the goto script label, which will be blank if the story is over.
	// 'vars' may be updated.
	// 'storyCancelOk' may be changed to indicate if the story is droppable or not.
	bool TestBranch(DaPerson& person, quint32 storyInstance, const QString& blockLabel, const AuTime& blockStart,
			QString& branchToBlock, QMap<QString, QVariant>& vars, bool& storyCancelOk) const;

	// Returns true if conditions are right for this story to begin.
	// Sets label of first block.
	// 'vars' is filled with any variables declared in the catalyst.
	bool TestCatalyst(DaPerson& person, QString* startBlock, QMap<QString, QVariant>& vars) const;

private:
	// Returns 0=ok, 1=error, 2=#nocompile
	int Compile(QTextStream& in, const QString& scriptFile);
	QString CompileFunction(const QString& funcCall, bool isEval) const;
	void ScriptError(quint32 storyInstance, const QString& messageText) const;
	QStringList SplitFields(const QString& parms) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScript& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScript& o)
{
	o << in;
	return in;
}

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuScript& o);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// AuScripts map key is script id.

class AuScripts : public unordered_map<unsigned int, AuScript, AuScriptIdHash>
{
public:
	quint32 HighestId() const;

	// Lists design ids needed by all scripts. Used to preload and maintain those designs in the cache.
	void ListDesignsNeeded(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const;

	// Load all scripts from stories folders.
	//void LoadAll();

	// Load all scripts from the AuStories library.
	void LoadAllLib();

	void Load(unsigned int scriptId);

	QString ScriptTitle(unsigned int scriptId) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuStory
{
private:
	QString								m_Block;				// Label of current block in body.
	AuTime								m_BlockStart;			// UTC when current block started.
	bool								m_CancelOk;				// false when cancel is disabled due to critical story segment.
	AuIdList							m_People;				// Ids of people participating.
	unsigned int						m_ScriptId;				// Script id.
	bool								m_Started;
	unsigned int						m_StoryInstanceId;		// Id unique to this instance.
	QMutex								m_StoryLock;			// Guards this story object.
	QMap<QString, QVariant>				m_Variables;			// Key is variable name. Variables persist after the catalyst block.

public:
	// 'var's are passed from the catalyst.
	AuStory(unsigned int storyInstanceId, unsigned int scriptId, const QString& startBlock, const QMap<QString, QVariant>& vars);

	inline AuStory(unsigned int storyInstanceId)
	{
		m_CancelOk = true;
		m_ScriptId = 0;
		m_Started = false;
		m_StoryInstanceId = storyInstanceId;
	}

	inline AuStory()
	{
		m_CancelOk = true;
		m_ScriptId = 0;
		m_Started = false;
		m_StoryInstanceId = 0;
	}

	inline AuStory(const AuStory& c) : m_People(c.m_People), m_Variables(c.m_Variables)
	{
		m_Block = c.m_Block;
		m_BlockStart = c.m_BlockStart;
		m_CancelOk = c.m_CancelOk;
		m_ScriptId = c.m_ScriptId;
		m_Started = c.m_Started;
		m_StoryInstanceId = c.m_StoryInstanceId;
		//m_StoryLock
	}

public:	// Operators
	inline const AuStory& operator=(const AuStory& c)
	{
		m_Block = c.m_Block;
		m_BlockStart = c.m_BlockStart;
		m_CancelOk = c.m_CancelOk;
		m_People = c.m_People;
		m_ScriptId = c.m_ScriptId;
		m_Started = c.m_Started;
		m_StoryInstanceId = c.m_StoryInstanceId;
		//m_StoryLock
		m_Variables = c.m_Variables;

		return *this;
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	inline const QString& Block() const
	{
		return m_Block;
	}

	inline const AuTime& BlockStart() const
	{
		return m_BlockStart;
	}

	inline bool CancelOk() const
	{
		return m_CancelOk;
	}

	// Returns branch-to label resulting from choice picked.
	bool ChoicePicked(DaPerson& person, qint8 value, const AuScript& script);

	// Returns false if no people remain in this story after person departs.
	inline bool Depart(const AuId& person)
	{
		m_People.removeAll(person);
		return m_People.isEmpty() ? false : true;
	}

	// Returns 0=story did not branch, 1=story branched, 2=story ended
	int Execute(const AuScript& script, DaPerson& person);

	void Insert(QSqlDatabase& db) const;

	inline unsigned int InstanceId() const
	{
		return m_StoryInstanceId;
	}

	inline bool IsDefunct() const
	{
		return m_People.empty();
	}

	// Gets a list of the people involved in the story.
	inline void People(AuIdList& people) const
	{
		people = m_People;
	}

	// Adds a person to this active story, if they're not already in the story.
	inline void Join(const AuId& person)
	{
		m_People.AddIfUnique(person);
	}

	inline void Join(const AuIdList& people)
	{
		for(AuIdList::const_iterator it = people.constBegin(); it != people.constEnd(); it++)
			Join(*it);
	}

	inline void Lock()
	{
		m_StoryLock.lock();
	}

	void ResendBlock(const AuScript& script, DaPerson& person) const;

	inline unsigned int ScriptId() const
	{
		return m_ScriptId;
	}

	bool Select(QSqlDatabase& db);

	void SendHint(const AuScript& script, DaPerson& person) const;

	inline void SetBlockStart(const AuTime& blockStart)
	{
		m_BlockStart = blockStart;
	}

	inline void SetStarted()
	{
		m_Started = true;
	}

	inline void SetVariables(const QMap<QString, QVariant>& vars)
	{
		m_Variables = vars;
	}

	inline void Unlock()
	{
		m_StoryLock.unlock();
	}

	void Update(QSqlDatabase& db) const;

protected:
	// Branches to a block of the story and executes the block.
	void BranchTo(const QString& blockLabel, const AuScript& script, DaPerson& person, bool& storyCancelOk);

	void ExecuteBlock(const AuScript& script, DaPerson& person, bool& storyCancelOk);

	// Returns 0=story did not branch, 1=story branched, 2=story ended
	int ExecuteContinue(const AuScript& script, DaPerson& person, bool& storyCancelOk);
};

inline QDataStream& operator<<(QDataStream& out, const AuStory& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuStory& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// AuStories map key is story instance id

class AuStories : public unordered_map<unsigned int, AuStory*, AuStoryInstanceHash>
{
public:
	~AuStories();

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	// Clears all stories.
	void Clear();

	// Clears stories that respond true to AuStory::IsDefunct.
	void ClearDefunct();

	void Load(QSqlDatabase& db);

	inline AuStory* LockStory(unsigned int storyInstance) const
	{
		const_iterator si = find(storyInstance);
		if(si == end())
			return 0;

		(*si).second->Lock();
		return (*si).second;
	}

	inline unsigned int ScriptId(unsigned int storyInstance) const
	{
		const_iterator si = find(storyInstance);
		if(si == end())
			return 0;
		return (*si).second->ScriptId();
	}

	inline void UnlockStory(unsigned int storyInstance) const
	{
		const_iterator si = find(storyInstance);
		if(si != end())
			(*si).second->Unlock();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuStories& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuStories& o)
{
	o << in;
	return in;
}

#endif // AuStory_H
