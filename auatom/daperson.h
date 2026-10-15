//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daperson.h,v 1.691 2022/05/17 16:20:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaPerson_H
#define DaPerson_H

#include "auglobal.h"
#include "auhitnotify.h"
#include "auitem.h"
#include "aujob.h"
#include "aumemory.h"
#include "auplayerstats.h"
#include "auroster.h"
#include "dasentient.h"

class AuBox;
class AuCommAccept;
class AuCommAcceptBarter;
class AuCommAcceptRepair;
class AuGLEffects;
class AuKnownPatents;
class AuQuadArray;
class AuScript;
class AuScripts;
class AuStories;
class AuStory;
class DaCapsule;
class DaCitizen;
class DaVessel;
class DaWorldLL;
class PsEntity;
class PsWeather;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGearKeyPos is used to associate a gear item in a particular usage slot and mode with a hot-key.
// e.g. The laser rifle in my hand in weapon mode 1.

class AuHotKeyGear
{
private:	// Properties
	INVENTORYPOS						m_IPos;					// INVENTORYPOS of body part or mount's body part.
	AuGearSlot::SLOTTYPE				m_SlotType;				// AuGearSlot::SLOTTYPE
	quint8								m_SlotWhich;			// 'which' index of the slot.
	INVENTORYSUB						m_UseSub;				// INVENTORYSUB Sub item usage mode.

public:	// Constructors
	inline AuHotKeyGear(INVENTORYPOS ipos, INVENTORYSUB isub, AuGearSlot::SLOTTYPE slotType, quint8 slotWhich)
	{
		m_IPos = ipos;
		m_SlotType = slotType;
		m_SlotWhich = slotWhich;
		m_UseSub = isub;
	}

	inline AuHotKeyGear()
	{
		m_IPos = IP_NumInventoryPos;
		m_SlotType = AuGearSlot::Invalid;
		m_SlotWhich = 0;
		m_UseSub = IPS_Item;
	}

	inline AuHotKeyGear(const AuHotKeyGear& c)
	{
		m_IPos = c.m_IPos;
		m_SlotType = c.m_SlotType;
		m_SlotWhich = c.m_SlotWhich;
		m_UseSub = c.m_UseSub;
	}

public:	// Operators
	inline const AuHotKeyGear& operator=(const AuHotKeyGear& c)
	{
		m_IPos = c.m_IPos;
		m_SlotType = c.m_SlotType;
		m_SlotWhich = c.m_SlotWhich;
		m_UseSub = c.m_UseSub;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (qint16)m_IPos;
		out << (quint8)m_SlotType;
		out << m_SlotWhich;
		out << (quint8)m_UseSub;
	}

	inline void operator<<(QDataStream& in)
	{
		qint16 i16;
		in >> i16;
		m_IPos = (INVENTORYPOS)i16;
		quint8 ui8;
		in >> ui8;
		m_SlotType = (AuGearSlot::SLOTTYPE)ui8;
		in >> m_SlotWhich;
		in >> ui8;
		m_UseSub = (INVENTORYSUB)ui8;

		// Sanity check on m_IPos because it used to be a COMMODITYID. It was only used for the corresponding INVENTORYPOS values
		// when this hot key referenced a body part.
		if(m_IPos >= IP_ArmorArms || m_IPos < IP_Mount_Anus)
			m_IPos = IP_NumInventoryPos;
	}

public:	// Methods
	inline AuGearPos GearPos() const
	{
		if(IsBodyPos())
			return AuGearPos(InventoryPos());
		else
			return AuGearPos(GearSlotType(), GearSlotWhich());
	}

	inline AuGearSlot::SLOTTYPE GearSlotType() const
	{
		return m_SlotType;
	}

	inline quint8 GearSlotWhich() const
	{
		return m_SlotWhich;
	}

	inline INVENTORYPOS InventoryPos() const
	{
		return m_IPos;
	}

	inline bool IsBodyPos() const
	{
		return IsPersonalBodyPos() || IsMountBodyPos();
	}

	inline bool IsGearPos() const
	{
		return GearSlotType() != AuGearSlot::Invalid;
	}

	inline bool IsMountBodyPos() const
	{
		return InventoryPos() >= IP_Mount_Anus && InventoryPos() <= IP_Mount_Torso;
	}

	inline bool IsNull() const
	{
		return !IsGearPos() && !IsBodyPos();
	}

	inline bool IsPersonalBodyPos() const
	{
		return InventoryPos() >= IP_My_Anus && InventoryPos() <= IP_My_Torso;
	}

	inline INVENTORYSUB UseSub() const
	{
		return m_UseSub;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuHotKeyGear& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHotKeyGear& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaPerson : public DaSentient  
{
	enum PERSON_SOUNDS
	{
		PSOUND_Breath,
		PSOUND_Wind,
		PSOUND_Rain,
		PSOUND_Count	// LAST ITEM
	};

public:
	enum CONTROLINPUT
	{
		// This enumeration starts high enough not to overlap the control inputs created by consoles when enumerating them.
		// The final control input value fits into the low 7 bits of a byte for network efficiency.
		CI_ConsoleCommand = 100,// Switch command console to command mode if sitting at a command console. (No console, just seat.)
		CI_ConsoleEngineer,		// Switch command console to engineer mode if sitting at a command console.
		CI_ConsoleLower,
		CI_ConsoleMoveFarther,
		CI_ConsoleMoveCloser,
		CI_ConsoleNavigator,	// Switch command console to navigator mode if sitting at a command console.
		CI_ConsolePilot,		// Switch command console to pilot mode if sitting at a command console.
		CI_ConsoleRaise,
		CI_ConsoleRotateAway,
		CI_ConsoleRotateToward,
		CI_ConsoleSensor = 110,		// Switch command console to sensor mode if sitting at a command console.
		CI_ConsoleShield,		// Switch command console to shield mode if sitting at a command console.
		CI_ConsoleWeapon,		// Switch command console to weapon mode if sitting at a command console.
		CI_Crouch,
		CI_EnterExit,			// Enter/exit vehicle or control station.
		CI_FireItem,			// Apply effects of current item to target. e.g. Tool repairs component. Weapon fires at target.
		CI_FireItemAlt,			// Apply alt effects of current item to target. e.g. Tool removes component.
		CI_BrakeOrJump,					// Brake when flying or in zero-G, jump when on ground.
		CI_LookDown,
		CI_LookUp,
		CI_MapPos0 = 120,				// Map current use pos and sub to map pos index 0
		CI_MapPos1,
		CI_MapPos2,
		CI_MapPos3,
		CI_MapPos4,
		CI_MapPos5,
		CI_MapPos6,
		CI_MapPos7,
		CI_MapPos8,
		CI_MapPos9,
		CI_MoveBackward = 130,
		CI_MoveDown,
		CI_MoveForward,
		CI_MoveLeft,
		CI_MoveRight,
		CI_MoveUp,
		CI_ProneStandToggle,
		CI_ReloadItem,			// Reload/recharge current item with ammo/battery in immediate gear.
		CI_Run,
		CI_SelectMapPos0,		// Set current use pos and sub to mapped pos index 0
		CI_SelectMapPos1 = 140,
		CI_SelectMapPos2,
		CI_SelectMapPos3,
		CI_SelectMapPos4,
		CI_SelectMapPos5,
		CI_SelectMapPos6,
		CI_SelectMapPos7,
		CI_SelectMapPos8,
		CI_SelectMapPos9,
		CI_SelectNextItem,		// Select next usable gear item.
		CI_SelectPreviousItem = 150,	// Select previous usable gear item.
		CI_SpawnRecall,			// Initiate respawn at tied location.
		CI_SpawnTie,			// Tie to current city/ship for spawn.
		CI_Stand,
		CI_TurnLeft,
		CI_TurnRight,
		CI_UseItem,				// Apply effects of current item to myself. Drops detonator&fuse weapons in place armed.
		CI_EatFood,
		CI_ThrowItem,			// Throws the current item.
		CI_Brake,				// Brake
		CI_Jump = 160,				// Jump
		CI_HelmetVisor,			// Helmet visor open/closed toggle.
		CI_MoveForwardContinuously,
		CI_PlaceItem,
		CI_SayHello,
		CI_RoomLightsBrighter,
		CI_RoomLightsDarker,
		CI_DoorLockToggle,
		CI_JumpPowerDecrease,
		CI_JumpPowerIncrease,
		CI_MovePowerDecrease = 170,
		CI_MovePowerIncrease,
		CI_RoomNavOrbitRings,
		CI_RoomNavSystemNames,
		CI_RoomNavWormholes,
		CI_DropItem,
		CI_ReceptiveMateToggle,
		CI_SitStandToggle,
		CI_DoorOpen,
		CI_LASTITEM				// THE VALUE OF THIS ENUM MUST NOT EXCEED 256
	};

	// Unguarded.
public:
	quint8								m_AgentBits;			// bits of special information for agents
private:
	bool								m_AutoEat;				// True to automatically eat a piece of food when hunger < 50%.
	quint8								m_InitScenario;			// Initial scenario chosen.
	quint8								m_PowerJump;			// power applied when jumping 0-15.
	quint8								m_PowerMove;			// power applied when moving 0-15.
	quint8								m_PowerMoveMax;			// max power applied when moving 0-15.
	AuPlayerStats						m_Stats;				// Statistics (internally guarded).

private:
	mutable QReadWriteLock				m_PersonLock;
	AuTime								m_ArenaEntered;			// Time arena was entered.
	AuGear								m_ArenaExitInv;			// Gear when I entered arena, for restore on exit.
	AuPoint								m_ArenaExitLoc;			// My location when I exit arena, if I'm in one.
	AuId								m_ArenaExitParent;		// My parent atom when I exit arena, if I'm in one.
	AuId								m_Dad;					// Id of male parent, only if they were an avatar.
	QString								m_DesignPass;			// Password for entry to design studio.
	AuElapsedTimer						m_LastClaim;			// Time of last claim to a world. Not stored in db.
	AuElapsedTimer						m_LastJumpOrMovePower;	// Time last jump or move power changed.
	AuEventLog							m_Log;					// For a summary of noteworthy events in each report.
	AuTime								m_LogTime;				// Time the log was last sent to my chain of command.
	AuId								m_Mom;					// Id of female parent, could be avatar or citizen.
	QImage								m_Selfie;				// Not loaded from database. Only stored when set, then cleared.
	unsigned short int					m_Worms;				// wormholes discovered as captain

	// Account balance.
	double								m_AccountBalance;
	double								m_LoanBalance;
	AuElapsedTimer						m_tmBalanceChange;		// Account Balance

	// Arena ship design, Con, Lineage, Office
	quint32								m_ArenaShipDesign;		// Ship design selected when I last entered an arena.
	bool								m_Con;					// True while this officer thinks it has the con.
	LINEAGE_TYPE						m_Lineage;
	AuElapsedTimer						m_tmOfficeChange;		// Office position. Also arena team and ship design. Con also.

	// Current item usage.
	AuGearPos							m_CurrentGearPos;		// Current gear item for fire/activate. May be a body position.
	INVENTORYSUB						m_CurrentGearSub;		// Subitem mode of current gear item.
	AuElapsedTimer						m_tmItemUseChange;		// Current gear item pos changed.

	// Hit Notification
	AuHitNotify 						m_HitNotify;
	AuElapsedTimer						m_tmHitChange;

	// Domain designated immune to decay.
	AuDomain							m_Domain;
	AuElapsedTimer						m_DomainChange;

	// Mapped gear positions. These pos values are mapped to keys on the keyboard.
	AuHotKeyGear						m_MapPos[10];

	// Reputation
	int									m_Reputation;			// Within my own empire. Irrelevant to other empires.
	AuElapsedTimer						m_tmReputationChange;	// Reputation

	// Spawn Info
	AuRosterEntry::TYPE					m_BerthType;			// berth type held aboard the spawn ship
	AuId								m_SpawnBd;				// Bd tied to. 0 for nubie emperors. 1 when not tied to a bd.
	QString								m_SpawnBdName;
	AuId								m_SpawnBdSystem;		// System containing city tied to, or spawn system for nubes.
	QString								m_SpawnBdSystemName;
	AuGlobeLLF							m_SpawnDevLL;			// Spawn at dev LL on DaWorldLL.
	AuId								m_SpawnShip;			// Ship last tied to.
	bool								m_SpawnShipIsPrivate;	// True if my spawn ship is privately owned.
	QString								m_SpawnShipName;
	AuElapsedTimer						m_tmSpawnChange;		// Spawning location.

	// Current actions.
	quint32								m_uiActions;			// ACT_ Bits of current actions to process during Tick.
	quint16								m_byRotSpeedX;			// Rotation speed about X (look up/down), in 100ths of a degree.
	quint16								m_byRotSpeedZ;			// Rotation speed about Z (turn left/right), in 100ths of a degree.
	AuTime								m_SpawnTime;
	quint8								m_TurnCount;
	quint8								m_TurnIndex;
	float								m_TurnX[20];
	float								m_TurnZ[20];
	AuElapsedTimer						m_tmTurn[20];
	AuElapsedTimer						m_tmLastBrake;			// Time last brake control received.
	AuElapsedTimer						m_tmLastMoveControl;	// Time last movement control received.
	AuElapsedTimer						m_tmLastMoveLControl;	// Time last lateral control received.
	AuElapsedTimer						m_tmLastMoveSControl;	// Time last swivel (turn Z) control received.
	AuElapsedTimer						m_tmLastMoveVControl;	// Time last vertical control received.
	AuElapsedTimer						m_tmLastWeaponFired;	// Time last fired for rate-of-fire.

private:
	mutable QReadWriteLock				m_GearLock;
	AuGear								m_Gear;					// clothing and equipment
	AuElapsedTimer						m_tmGearChange;	
	float								m_WeightCarried;		// Current weight of gear, in lbs.

private:
	mutable QReadWriteLock				m_JobsLock;
	AuJobList							m_Jobs;					// Info about jobs, such as active stories.
	AuElapsedTimer						m_tmJobsChange;

private:
	mutable QReadWriteLock				m_MemoryLock;
	AuMemories							m_Memories;

private:
	mutable QReadWriteLock				m_OnLineLock;			// Guards online status variables
	bool								m_AccountInactive;		// True if the owning player's account has become inactive.
	ACCOUNT								m_AccountOnLine;		// When on line. Reset to ACCT_Count 30sec after last SetOnLine.
	quint32								m_Ping;					// Ping time in ms while IsOnLine. Not stored. Scene server only.
	AuTime								m_tmLastOnLine;

private:
	// Rewards to be given to player by their empire.  These are used as credits.
	// Each time one of these happen the player is awarded 1 credit towards this
	// reward.  The player claims their rewards by turning in their credits at
	// a bank.  This approach to applying rewards will limit the server communication
	// between the client
	mutable QReadWriteLock				m_RewardsLock;
	double								m_RewardsOwed;
	AuElapsedTimer						m_tmRewardChange;		// Rewards Owed

private:
	mutable QMutex						m_ScriptLock;
	AuScript*							m_Script;				// Private script, if any.

private:
	mutable QReadWriteLock				m_SeaChestLock;
	AuItemHash							m_SeaChest;				// Personal items, accessible in my berth or home.
	AuElapsedTimer						m_tmSeaChestChange;	

private:
	// Render parameters only referenced on the scene client do not need locks.
	float								m_WxBAR;				// Outside air pressure in bars.
	float								m_WxOAT;				// Outside Air Temperature in Celsius
	float								m_WxRelHumidity;		// Relative humidity.

	// OpenAL/Sound
	unsigned int						m_AudioPerson[PSOUND_Count];	// sound sources
	float								m_EnvHumidity;			// Interpolated value for smooth transitions to current conditions.
	float								m_EnvWindForce;
	float								m_RainGain;
	float								m_ShipWindForce;
	int									m_Sound[PSOUND_Count];	// current SOUND_EFFECT

	// Audio environment data gathered only for scene atom on client via call to QueryAudioEnvironment
	AuPoint								m_ClosestFTLDrive;		// Only valid aboard a ship.
	AuPoint								m_ClosestGravityDrive;	// Only valid aboard a ship.
	AuPoint								m_ClosestPowerPlant;	// Only valid aboard a ship.
	AuPoint								m_ClosestSensor;		// Only valid aboard a ship.
	AuPoint								m_ClosestSiren;			// Only valid aboard a ship.

	// Cached during AnimateDeviceObjects.
	AuGearPos							m_CurrentLight;			// returned by CurrentLightThatIsOn

	// OpenGL
	PsWeather*							m_PrecipParticles;
	PsEntity*							m_PSEntity;

public:
	DaPerson(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
			GROWTHSTAGE stage, quint32 idEmpire);
	DaPerson();
	virtual ~DaPerson();

protected:
	DaPerson(const DaPerson &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual void AddBounty(const AuId& posterId, const QString& posterName, const QString& reason, double reward, int why);

	virtual quint8 AgentBits() const
	{
		return m_AgentBits;
	}

	// 'type' 0=animal encounter, 1=pirate station, 2=pirate ship, 3=load officer, 4=load crew, 5=load troops, 6=load passengers
	void AgentEncounter(quint8 type, quint32 designId);

	// 'makeWhat' is 0=invalid, 1=one item, 2='count' items, 3=city inv, 4=ship cargo, 5=utility pouch, 6=knapsack, 7=backpack,
	//   8=grav pack.
	// 'destGalaxy' and 'destGalaxyLoc' are only needed for preons.
	void AgentMakeCommodity(int makeWhat, COMMODITYID cid, quint8 quality, quint32 count, GALAXY destGalaxy,
			const AuPoint& destGalaxyLoc);

	inline bool AutoEat() const
	{
		return m_AutoEat;
	}

	// 'platformId' is zero when building on the world.
	// 'platformId' is non-zero when building on a platform.
	DaBd* BdBuild(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
			const AuId& platformId, quint16 mpid, bool workerOk);

	// Adds to existing fence or wall if possible.
	// 'fenceStyle' is an AuScFence::STYLE.
	// 'platformId' is zero when building on the world.
	// 'platformId' is non-zero when building on a platform.
	// 'mat' is a COMMODITYID for construction of custom fence designs.
	DaBd* BdBuildFence(const AuPoint& startLoc, const AuPoint& endLoc, int fenceStyle, const AuId& platformId,
			quint32 customDesignId, int mat);

	// 'platformId' is zero when building on the world.
	// 'platformId' is non-zero when building on a platform.
	DaBd* BdBuildStorage(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
			const AuId& platformId, quint16 mpid, bool workerOk, COMMODITYID storeCID);

	// 'opBd' is the building I am operating, when I am doing this from a capitol building. Otherwise, the building I am at is
	// operated.
	void BdConstructionEnable(bool enable, const AuId& opBd);

	// 'opBd' is the building I am operating, when I am doing this from a capitol building. Otherwise, the building I am at is
	// operated.
	void BdConstructionFetch(const AuId& opBd);

	// 'opBd' is the building I am operating, when I am doing this from a capitol building. Otherwise, the building I am at is
	// operated.
	void BdConstructionGo(const AuId& opBd);

	void BdDiscardItem(const AuItem& item, const AuId& opBd);

	void BdManufactureAddMPID(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit, const AuId& opBd);
	void BdManufactureClear(const AuId& opBd);
	void BdManufactureDeleteMPID(quint16 mpid, const AuId& opBd);
	void BdManufactureEnabledMPID(quint16 mpid, quint32 enabled, const AuId& opBd);
	void BdManufactureFetch(quint16 panelIndex, const AuId& opBd);
	void BdManufactureFetchMPID(quint16 mpid, const AuId& opBd);
	void BdManufactureGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos, const AuId& opBd);
	void BdManufactureGo(quint16 panelIndex, const AuId& opBd);
	void BdManufactureGoMPID(quint16 mpid, const AuId& opBd);
	void BdManufactureLowerMPID(quint16 mpid, const AuId& opBd);
	void BdManufactureMaxMPID(quint16 mpid, const AuId& opBd);
	void BdManufactureMinMPID(quint16 mpid, const AuId& opBd);
	void BdManufactureProcessSelected(quint16 panelIndex, qint32 mpid, const AuId& opBd);
	void BdManufactureProcessSetMinQMPID(quint16 mpid, quint8 minQ, const AuId& opBd);
	void BdManufactureProcessSetStockLimitMPID(quint16 mpid, quint32 stockLimit, const AuId& opBd);	// 0 = no limit
	void BdManufactureRaiseMPID(quint16 mpid, const AuId& opBd);
	void BdManufactureRecordPatents(const AuKnownPatents& knownPatents, const AuId& opBd);
	void BdManufactureSetLivestock(const AuSpecies& species, const AuId& opBd);
	void BdManufactureSetMinQ(quint8 minQ, const AuId& opBd);	// Sets 'minQ' in all processes.
	void BdManufactureSetWarehouseStorage(COMMODITYID storeCID, const AuId& opBd);

	void BdManufactureSpacecraftBuyCompanyShip(quint32 designId, AuGearPos pos, const AuId& opBd);
	void BdManufactureSpacecraftCommissionShip(quint32 designId, AuGearPos pos, const AuId& opBd);
	void BdManufactureSpacecraftOrderDelete(quint32 designId, const AuId& opBd);
	void BdManufactureSpacecraftProcessCanceled(const AuId& opBd);
	void BdManufactureSpacecraftProcessDefaultSelected(quint32 designId, const QString& designName, quint8 designQ,
			const AuId& opBd);
	void BdManufactureSpacecraftSetCompanyPolicy(bool companyOk, quint8 companyMaxQ, const AuId& opBd);
	void BdManufactureSpacecraftSetMinQ(quint8 minQ, const AuId& opBd);
	void BdManufactureSpacecraftEnableWorker(quint32 enabled, const AuId& opBd);

	// Returns the DaBd building I am operating.
	// When 'opBd' is null, the building I am at is operated.
	// Otherwise, 'opBd' is the building I am operating, as when I am doing this from a capitol building.
	DaBd* BdOp(const AuId& opBd);

	DaBd* BdPreview(const AuPoint& buildLoc, float zRotation);

	DaBd* BdPreviewFence(const AuPoint& startLoc, const AuPoint& endLoc);

	// Returns the id of the atom that is renamed, if any.
	AuId BdRename(const QString& nuName, const AuId& opBd);

	void BdStoreBuyItem(const AuItem& buyItem, const AuItem& offerItem, const AuId& opBd);

	void BdTakeItem(const AuItem& item, const AuId& opBd);

	void BountyCollected(const AuGearPos& gearPos, const AuBountyOnItem& bountyOnItem);

	void BountyExpired(const AuGearPos& gearPos, const AuBountyOnItem& bountyOnItem);

	virtual void BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire);

	virtual void ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire);

	// Clears my spawn tied city.
	// Clears my newbie status so I no longer return true to IsNewbieEmperor.
	inline void ClearSpawnCity()
	{
		m_PersonLock.lockForWrite();
		m_SpawnBd = AuId(1);			// This causes IsNewbieEmperor to return false.
		m_SpawnBdName = QString();
		m_SpawnBdSystem = AuId();
		m_SpawnBdSystemName = QString();
		m_tmSpawnChange.Set();
		m_PersonLock.unlock();
	}

	// Returns true if a functional (seal intact) environment helmet visor was open and is now closed.
	bool CloseHelmetVisor();

	void DeleteStoredData(const AuGearPos& srcPos, const QSet<COMMODITYID>& patentCIDs, const QSet<quint32>& scDesigns,
			const QSet<quint32>& bdDesigns, const QSet<QString>& missions, const QSet<QString>& papers);

	virtual void EmpireDeleted(quint32 empireId);

	void EmpireGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos);

	void EmpireRecordPatents(const AuKnownPatents& knownPatents);

	// 'world' is my parent world, at some level.
	void ExploreWorld(DaWorldLL* world);

	void GeoLandscape(const AuPoint& startLoc, const AuPoint& endLoc, float width, bool hideRocks, int hideGrass, bool hideShrubs,
			bool hideTrees, WORLDTEX_TYPE pavement);

	void GeoRoad(const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, float width, quint8 geoType);

	void InitNewAvatar(DaWorldLL* llworld);

	bool InitNewAvatarAboardShip(DaVessel* ves);

	bool InitNewAvatarAtCity(AuEther& ether, const AuId& bdId);

	bool InitNewAvatarAtEmperor(AuEther& ether, const AuId& emperorId);

	inline SCENARIO InitScenario() const
	{
		return (SCENARIO)m_InitScenario;
	}

	virtual bool KeepsAutomaticDoorsOpen() const;

	unsigned int OnLineMinutes() const;

	virtual bool OpensAutomaticDoors() const;

	// Returns outside air pressure in bar. This is environmental, not affected by being inside a building, vehicle, or
	// spacecraft.
	// CAVEAT: Only valid for the scene atom on the client.
	inline float OutsideAirPressure() const
	{
		return m_WxBAR;
	}

	// Returns outside air temperature in celsius. This is environmental, not affected by being inside a building, vehicle, or
	// spacecraft.
	// CAVEAT: Only valid for the scene atom on the client.
	inline float OutsideAirTemp() const
	{
		return m_WxOAT;
	}

	// Relative humidity is the ratio of vapor pressure to saturation pressure. <=1 is not precipitating. >1 is precipitating.
	// 0-1 = No precip
	// 1-1.25 = Light precip
	// 1.25-1.5 = Moderate precip
	// 1.5-1.75 = Heavy precip
	// 1.75+ = Very Heavy precip
	// CAVEAT: Only valid for the scene atom on the client.
	inline float RelativeHumidity() const
	{
		return m_WxRelHumidity;
	}

	void SetAutoEat(bool autoEat);

	inline void SetInitScenario(SCENARIO scenario)
	{
		m_InitScenario = scenario;
	}

	void SetSpawnAnywhere();

	void SetSpawnLocal();

	// Make an item in single player mode.
	void SoloMakeCommodity(COMMODITYID cid, quint8 quality, quint16 count);

	virtual bool TakeItemBounty(const QString& bountyHunter, const AuBountyOnItem& bountyOnItem);

	inline AuPoint ArenaExitLoc() const
	{
		m_PersonLock.lockForRead();
		AuPoint v(m_ArenaExitLoc);
		m_PersonLock.unlock();
		return v;
	}

	inline AuId ArenaExitParent() const
	{
		m_PersonLock.lockForRead();
		AuId v(m_ArenaExitParent);
		m_PersonLock.unlock();
		return v;
	}

	inline quint32 ArenaShipDesign() const
	{
		m_PersonLock.lockForRead();
		quint32 v = m_ArenaShipDesign;
		m_PersonLock.unlock();
		return v;
	}

	// Return the commodity id of the clothing that is outermost for rendering of appearance.
	virtual COMMODITYID Armor() const
	{
		// No apparent clothing when posing as an animal.
		if(m_AgentBits & AGENT_Animal)
			return CID_Invalid;

		m_GearLock.lockForRead();
		COMMODITYID c = m_Gear.BodyArmorOrClothingWorn();
		m_GearLock.unlock();
		return c;
	}

	// Starts my custom script, if I have one and its catalyst test succeeds.
	bool AutoStartCustomScript();

	void AutoStartStory(const AuScripts& scripts);

	// Borrow 'amount' from the bank.
	// 'opBd' is the bank DaBd that is operated, when doing this from a capitol building, otherwise it is null.
	void BankLoan(double amount, const AuId& opBd);

	// Pay 'amount' toward my current bank loan debt.
	// 'opBd' is the bank DaBd that is operated, when doing this from a capitol building, otherwise it is null.
	void BankPay(double amount, const AuId& opBd);

	// Returns current weight of burden in lbs.
	// Includes burden of riders when mounted.
	virtual float Burden() const
	{
		if(m_AgentBits & AGENT_Entity)
			return 0;
		float base = DaSentient::Burden();
		m_GearLock.lockForRead();
		float v = m_WeightCarried + base;
		m_GearLock.unlock();
		return v;
	}

	// True if this atom is permitted to go below the surface of the terrain.
	// This could be any hard surface, such as ground terrain or the floor of a spacecraft or building.
	virtual bool CanBeUnderGround() const;

	inline void ClearSpawnShip()
	{
		if(!m_SpawnShip.IsNull())
		{
			m_PersonLock.lockForWrite();
			m_BerthType = AuRosterEntry::Invalid;
			m_SpawnShip = AuId(0);
			m_SpawnShipIsPrivate = false;
			m_SpawnShipName = "";
			m_tmSpawnChange.Set();

			m_uiActions |= ACT_SpawnTie;	// Trigger the spawn tie graphic.
			m_PersonLock.unlock();
		}
	}

	inline void ClearSpawnTie()
	{
		m_PersonLock.lockForWrite();
		m_uiActions &= ~ACT_SpawnTie;
		m_PersonLock.unlock();
	}

	const AuPoint& ClosestFTLDrive() const
	{
		//Q_ASSERT(g_Au.IsSceneApp());
		return m_ClosestFTLDrive;
	}

	const AuPoint& ClosestGravityDrive() const
	{
		//Q_ASSERT(g_Au.IsSceneApp());
		return m_ClosestGravityDrive;
	}

	const AuPoint& ClosestPowerPlant() const
	{
		//Q_ASSERT(g_Au.IsSceneApp());
		return m_ClosestPowerPlant;
	}

	const AuPoint& ClosestSensor() const
	{
		//Q_ASSERT(g_Au.IsSceneApp());
		return m_ClosestSensor;
	}

	const AuPoint& ClosestSiren() const
	{
		//Q_ASSERT(g_Au.IsSceneApp());
		return m_ClosestSiren;
	}

	// This accept code handles transactions accepted by this person.
	// I am the seller.
	void CommAccept(const AuCommAccept& msg);

	// This accept code handles transactions accepted by this person.
	// I am the seller.
	void CommAcceptBarter(const AuCommAcceptBarter& msg);

	virtual bool CommAcceptBerth(int berthType, const AuCommAcceptBerth& msg);

	void CommAcceptInvitation(const AuCommPacket& msg);

	// This accept repair code handles repair offers accepted by this person.
	// I am the one getting the repairs for my ship.
	void CommAcceptRepair(const AuCommAcceptRepair& msg);

	// Returns my location as briefly as possible in a single line suitable for use in a comm message.
	QString CommMessageLocation() const;

	// Returns the number of milliseconds that movement controls take to decay.
	inline quint32 ControlDecayMs() const
	{
		// On the client, controls decay between calls to ProcessControlInput.
		// Client-side decay time must exceed d_SyncInterval from aumodel.cpp, currently 10ms.
		if(g_Au.IsSceneApp())
			return 20;

		// Controls decay instantly when this player is not on line.
		if(!IsOnLine())
			return 0;

		// Controls decay slightly later than this person's average ping time.
		quint32 decayMs = Ping() + g_Au.ControlDecayMargin();

		// Control decay must last at least 150ms due to the minimum interval at which clients sent UDP control packets.
		// Otherwise movement is jerky when ping time is excellent, as in primeval world.
		if(decayMs < 150)
			return 150;

		// Maximum decay time is 3/4 second.
		if(decayMs > 750)
			return 750;

		return decayMs;
	}

	double CreditLimit() const;

	// Returns head tilt if it's ok to tilt the head, otherwise returns the X rotation.
	// Used to rotate the body in zero-G vs the head when it's better to rotate the body.
	// Refer to 'OkToTiltHead'.
	inline float CurrentHeadTiltOrXRot() const
	{
		if(OkToTiltHead())
			return CurrentHeadTilt();
		else
			return Rotation().x;
	}

	// Returns true if the current weapon acts like a shotgun in that its shot spreads across an area.
	// 'range' is the maximum range of the shotgun, whether or not it is loaded.
	// 'patternAngle' is the angle of spread of the pattern off the centerline. Full spread = 2x patternAngle. In radians.
	// Returns false if the current weapon is not a shotgun.
	bool CurrentWeaponIsShotgun(float& range, float& patternAngle) const;

	// Retrieves my current domain.
	inline void Domain(AuDomain& domain) const
	{
		m_PersonLock.lockForRead();
		domain = m_Domain;
		m_PersonLock.unlock();
	}

	// Returns true if ok.
	// Returns false if there is no room to add another system to my domain.
	bool DomainAdd(const AuId& id, const QString& name);

	// Returns false on failure.
	bool DomainRemove(const AuId& id);

	int EVAMinutes() const;

	AuTime FirstMemoryOfCity(const AuId& cityId) const;

	void ForgetCity(const AuId& cityId);

	void ForgetStoryStarted(quint32 scriptId);

	inline const AuEventLog& LockEventLog() const
	{
		m_PersonLock.lockForRead();
		return m_Log;
	}

	inline void UnlockEventLock() const
	{
		m_PersonLock.unlock();
	}

	// Can return 0.
	inline const AuScript* LockScript() const
	{
		m_ScriptLock.lock();
		return m_Script;
	}

	inline void UnlockScript() const
	{
		m_ScriptLock.unlock();
	}

	// 'previewType' 0=none, 1=empty space, 2=sun, 3=sun and planet, 4=sun, planet and moon, 5=inferno, 6=frigid, 7=gas giant,
	//   8=ringworld, 9=sea bottom
	void InitPreview(quint8 previewType);

	// Build skill information from stats.
	void InitSkillsFromStats();

	virtual bool IsAIControlled() const;

	// The status of account activity (true or false) is updated once per day in AuSceneServerApp::DeactivateAvatars.
	inline bool IsAccountActive() const
	{
		return !IsAccountInactive();
	}

	inline bool IsAccountInactive() const
	{
		m_OnLineLock.lockForWrite();
		bool isInactive = m_AccountInactive;
		m_OnLineLock.unlock();
		return isInactive;
	}

	// This is called once per day while my account is active.
	void SetAccountActive();

	inline void SetAccountInactive()
	{
		m_OnLineLock.lockForWrite();
		m_AccountInactive = true;
		m_OnLineLock.unlock();
	}

	virtual bool IsCivilian() const;

	virtual bool IsEthereal() const;

	// Returns true after this avatar has not been on line for a while (30 days?).
	bool IsOffLine30Days() const;

	// Returns true when this person is invisible because they are off line.
	// Demiavatars do not disappear from the game universe when the player is off line. They become AI controlled instead until
	// the owning player's account goes dormant.
	virtual bool IsInvisibleOffLine() const;

	// True if my lineage is 100% avatar, including avatars spawned nu into the universe.
	inline bool IsLineageAvatar() const
	{
		m_PersonLock.lockForRead();
		bool v = m_Lineage == LINEAGE_None || m_Lineage == LINEAGE_Avatar;
		m_PersonLock.unlock();
		return v;
	}

	// True if my lineage contains some citizen.
	inline bool IsLineageDemiavatar() const
	{
		m_PersonLock.lockForRead();
		bool v = m_Lineage == LINEAGE_Demiavatar;
		m_PersonLock.unlock();
		return v;
	}

	// True if my lineage is 100% avatar, excluding avatars spawned nu into the universe.
	inline bool IsLineageHighborn() const
	{
		m_PersonLock.lockForRead();
		bool v = m_Lineage == LINEAGE_Avatar;
		m_PersonLock.unlock();
		return v;
	}

	virtual bool IsProtectedByEnvironmentSuit() const;

	virtual quint32 JobExperiencePoints() const;

	inline float JumpPower() const
	{
		return (float)(m_PowerJump+1) / 16.f;
	}

	virtual void LogEvent(AuEventLogEntry::TYPE type, const QString& desc, const AuId& atomId=AuId())
	{
		AuEventLogEntry logEntry(type, desc, atomId);
		m_PersonLock.lockForWrite();
		m_Log.AddEventTrim(logEntry);
		m_PersonLock.unlock();
	}

	virtual QString Description() const;

	// Enters the console of a ship near my current location.
	// Kicks current AI console operator.
	// Wakes doctor in room if it's a surgery unit.
	// Returns detail type of the console on success.
	// Returns AuStation::TYPE_Count if ship->CanEnterConsole fails at my current location.
	AuStation::TYPE EnterConsole(DaCapsule* ship, CONTROL_CONTEXT cc);

	virtual AuId Father() const
	{
		AuId v;
		m_PersonLock.lockForRead();
		v = m_Mom;
		m_PersonLock.unlock();
		return v;
	}

	// Commodity 'cid' is foraged from this atom's current location on 'world'. A seed is also foraged, if there is one.
	// 'isTree' is true when the commodity was foraged from a tree.
	// 'what' 0=cid and seed, 1=cid only, 2=seed only
	void Forage(const DaWorldLL* world, COMMODITYID cid, bool isTree, int what);

	// 'what' 0=cid and seed, 1=cid only, 2=seed only
	void Forage(quint8 quality, COMMODITYID cid, const AuFloraDNA& dna, quint8 plantType, quint32 worldGenSeed,
			int worldResourceZone, AuFloraDNA::ZONE latRegion, int what);

	virtual void FindAIPeopleRecursive(const AuId& shipAssigned, DaAtomList& aipeople) const;

	virtual const DaPerson* FindAIPersonRecursive() const;

	// Searches the solar system to find this person's spacecraft.
	DaVessel* FindAssignedShip();

	// Finds !IsDead people, which gets IsNormal and IsTraveling people.
	virtual void FindLivePeopleRecursive(DaAtomList& people);

	virtual void FindPeopleAwakeRecursive(DaAtomList& people);

	virtual void FindPeopleOnLineRecursive(DaAtomList& people) const;

	virtual void FindPeopleOnLineRecursiveInStory(DaAtomList& people, quint32 storyInstance);

	// Finds IsNormal people.
	virtual void FindPeopleRecursive(DaAtomList& people) const;

	virtual const DaPerson* FindPersonOnLineRecursive() const;

	virtual DaPerson* FindPersonRecursive() const;

	void HandcraftItem(int mpid);

	// Returns true if this person has ever begun construction of 'iid', regardless of whether they ever finished it.
	bool HasConstructedBuildingHabitable(INDUSTRYID iid) const;

	bool HasConstructedBuildingHarsh(INDUSTRYID iid) const;

	bool HasJob(const AuJobOffer& jobOffer) const;

	// Return the commodity id of the helmet for rendering of appearance.
	virtual COMMODITYID Helmet() const
	{
		// No apparent helmet when posing as an animal.
		if(m_AgentBits & AGENT_Animal)
			return CID_Invalid;

		COMMODITYID c(CID_Invalid);
		m_GearLock.lockForRead();
		const AuItem* helmet = m_Gear.HelmetItemWorn();
		if(helmet && helmet->State() == 1 && helmet->HitPoints() > 1)
			c = helmet->CommodityId();
		else
		{
			const AuItem* hat = m_Gear.HatItemWorn();
			if(hat)
				c = hat->CommodityId();
		}
		m_GearLock.unlock();
		return c;
	}

	bool HelmetClosed() const;

	void InitGearAvatar();

	void InitGearDesigner();

	// The only person who ever has a 0 spawn city id is a nubie emperor.
	// When an emperor player is no longer a nubie, the spawn city is set to 1 or the id of a city, but never back to 0 for the
	// life of the character.
	// The spawn city of non-emperor players is initialized to the city in which they first spawn.
	inline bool IsNewbieEmperor() const
	{
		m_PersonLock.lockForRead();
		bool v = m_SpawnBd.IsNull();
		m_PersonLock.unlock();
		return v;
	}

	// Called when acceptance has been received from this person for a job that was offered.
	// This call completes the acceptance process.
	void JobAccepted(const AuJobOffer& jobOffer);

protected:
	// 'script' is only valid when the job is a story-type offer.
	void JobAccepted(const AuJobOffer& jobOffer, const AuScript& script);

public:
	// Called when this person has chosen to close a job that is open.
	void JobClosed(const AuJob* job);

	inline const AuJobList& LockJobs() const
	{
		m_JobsLock.lockForRead();
		return m_Jobs;
	}

	inline void UnlockJobs() const
	{
		m_JobsLock.unlock();
	}

	// Called when this unit is killed by a person.
	virtual void KilledByPerson(DaPerson* killer);

	// Called when this unit is killed by another unit.
	virtual void KilledByUnit(const DaUnit* killer);

	inline LINEAGE_TYPE Lineage() const
	{
		m_PersonLock.lockForRead();
		LINEAGE_TYPE v = m_Lineage;
		m_PersonLock.unlock();
		return v;
	}

	virtual void ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const;

	virtual AuId Mother() const
	{
		AuId v;
		m_PersonLock.lockForRead();
		v = m_Mom;
		m_PersonLock.unlock();
		return v;
	}

	// Returns parent atom if this person is in control of its position, i.e. mount, vehicle if in drivers seat,
	// or ship if at helm.
	DaUnit* ParentControlAtom() const;

	inline quint32 Ping() const
	{
		m_OnLineLock.lockForRead();
		quint32 ping = m_Ping;
		m_OnLineLock.unlock();
		return ping;
	}

	inline void PrepareToEnterArena(quint8 team)
	{
		// Duplicate my current gear so it can be restored when I exit the arena.
		m_PersonLock.lockForWrite();
		m_ArenaEntered.Set();
		m_ArenaExitInv.clear();

		m_GearLock.lockForRead();
		m_ArenaExitInv = m_Gear;
		m_GearLock.unlock();

		// Store my current parent id and location so I can return to that spot when I exit the arena.
		m_ArenaExitLoc = Location();
		m_ArenaExitParent = ParentId();
		m_PersonLock.unlock();

		SetArenaTeam(team);
	}

	bool RecallCharacter(const QString& charName, AuMemCharacterDNA4& mem) const;

	// Stores or refreshes a memory.
	void Remember(const AuMemory& mem);

	// Sets the memory 'result' of running 'scriptId'.
	void RememberStoryResult(quint32 scriptId, quint8 result);

	// Sets the memory of terminating 'scriptId'. May be dropped or ended normally.
	void RememberStoryTerminated(quint32 scriptId);

	// 'forceLocal' is true when loading from db and in spawn state. We'll take any local spawn point before deferring then.
	void Respawn(bool forceLocal);

	void Reward(double fReward, REWARDTYPE iRewardType);

	inline AuItemHash& LockSeaChestForWrite()
	{
		m_SeaChestLock.lockForWrite();
		return m_SeaChest;
	}

	inline const AuItemHash& LockSeaChestForRead() const
	{
		m_SeaChestLock.lockForRead();
		return m_SeaChest;
	}

	// Updates my current selfie image.
	void UpdateSelfie(const QImage& img);

	inline void UnlockSeaChest() const
	{
		m_SeaChestLock.unlock();
	}

	inline static int SeaChestCapacity()
	{
		return 100;
	}

	inline const QImage& SelfieNoLock() const
	{
		return m_Selfie;
	}

	// Calls SetModified if this is actually a change.
	// Sends log message to the agent describing current state of all agent bits.
	void SetAgentBits(quint8 bits);

	inline void SetArenaShipDesign(quint32 designId)
	{
		m_PersonLock.lockForWrite();
		m_ArenaShipDesign = designId;
		m_tmOfficeChange.Set();
		m_PersonLock.unlock();
	}

	inline void SetLineage(LINEAGE_TYPE lineage, const AuId& dad, const AuId& mom)
	{
		m_PersonLock.lockForWrite();
		m_Dad = dad;
		m_Lineage = lineage;
		m_Mom = mom;
		m_PersonLock.unlock();
	}

	// 'fromDir' is relative to me.
	// 'audibility' is in the range 0 to 1.
	void SetSoundHeard(const AuPoint& fromDir, float audibility);

	// Called on environment atom to recursively cause all sentient atoms in the conversation address cone of 'talker' to start
	// listening on the Voice comm channel. See InConeOfAddress.
	virtual void StartVoiceConversationRecursive(const DaPerson& talker) const
	{
		// People do not respond to casual attempts to strike up a conversation when they are not AI controlled.
		if(IsAIControlled())
			DaSentient::StartVoiceConversationRecursive(talker);
	}

	inline AuPlayerStats& Stats()
	{
		return m_Stats;
	}

	inline const AuPlayerStats& Stats() const
	{
		return m_Stats;
	}

	// Cancels the custom script and any associated story job.
	// Returns:
	// 0 = No story script to cancel.
	// 1 = Story script canceled.
	// 2 = Story script and associated story job canceled.
	int StoryCancelScript();

	void TransporterSetCoord(const AuPoint& coord);

	void TransporterSetLongLat(const AuGlobeLLF& ll);

	virtual TRANSPORTER_TARGET TransporterTarget() const
	{
		return TT_Avatar;
	}

	inline double GetReward() const
	{
		m_RewardsLock.lockForRead();
		double rew =  m_RewardsOwed;
		m_RewardsLock.unlock();
		return rew;
	}

	inline void ReduceReward(double howMuch)
	{
		m_RewardsLock.lockForWrite();
		if(howMuch >= m_RewardsOwed)
			m_RewardsOwed = 0;
		else
			m_RewardsOwed -= howMuch;
		m_tmRewardChange.Set();
		m_RewardsLock.unlock();
	}

	inline void ResetReward()
	{
		m_RewardsLock.lockForWrite();
		m_RewardsOwed = 0;
		m_tmRewardChange.Set();
		m_RewardsLock.unlock();
	}

	// Forgets ever having started or finished a given story script.
	// Stops the story if it is currently active.
	// Returns:
	// 0 = no story job to stop
	// 1 = story job stopped
	int ResetStory(quint32 scriptId);

	// Called when this atom is credited with killing a unit.
	// All unit kills go through here.
	// This is where empires declare mutual enemies based on unit kills. See DaUnit::UnitKilled.
	virtual void UnitKilled(DaUnit* victim);

	void WillfullyChangedEmpire(quint32 nuEmpireId);

	// 'attacker' is the atom that is causing the damage. The attacker id should be NULL when the attacker is not a DaUnit, like
	//   when it is the environment or a sun or something.
	// 'weapon' describes the weapon that was used to inflict the damage.
	// 'wpnLoc' is the location of the weapon that is attacking this atom, relative to this atom.
	// 'hitLoc' is the location of the hit on this atom, relative to this atom.
	// 'result' contains the results of the attack, except the IsDestroyed state; this function call sets that.
	// Sets the IsDestroyed result in 'result', if this atom is killed. This is temporal death, not necessarily atomic death.
	// IsDestroyed result is not modified if this atom is already dead.
	// The only thing this base class function does is sends a log message to 'whoId', if it's not NULL.
	virtual void CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, AuCombatResult& result);

	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	inline bool DidSpawnTie() const
	{
		m_PersonLock.lockForRead();
		bool v = (m_uiActions & ACT_SpawnTie) ? true : false;
		m_PersonLock.unlock();
		return v;
	}

	bool HadItem(COMMODITYID cid) const;

	// True if this person is an agent who has their powers enabled right now.
	virtual bool HasAgentPower() const
	{
		return IsAgentPowerOn();
	}

	// True if this person is a programmer level agent who has their powers enabled right now.
	// Only valid on the client if this person is the scene atom.
	inline bool HasArchitectPower() const
	{
		return IsAgentPowerOn() && IsOnLineAsArchitect();
	}

	// True if this person is a programmer level agent who has their powers enabled right now.
	// Only valid on the client if this person is the scene atom.
	inline bool HasProgrammerPower() const
	{
		return IsAgentPowerOn() && IsOnLineAsProgrammer();
	}

	int VehicleMinutes(COMMODITYID vehicleCID) const;

	// Returns true for agent regardless of agent powers enabled state.
	// Only returns true if this player is actually on line.
	// Only valid on the client if this person is the scene atom.
	bool IsOnLineAsAgent() const;

	// Returns true for architect regardless of agent powers enabled state.
	// Only returns true if this player is actually on line.
	// Only valid on the client if this person is the scene atom.
	bool IsOnLineAsArchitect() const;

	// Returns true for minions.
	// Only returns true if this player is actually on line.
	// Only valid on the client if this person is the scene atom.
	bool IsOnLineAsMinion() const;

	// Returns true for programmer regardless of agent powers enabled state.
	// Only returns true if this player is actually on line.
	// Only valid on the client if this person is the scene atom.
	bool IsOnLineAsProgrammer() const;

	// Returns true for person who is on line.
	// Returns true for scene atom person when playing solo. Their SetAccountTypeOnLine function gets called in AuModel.
	// Only returns true if this player is actually on line.
	inline bool IsOnLine() const
	{
		// Always true when called on every person atom on the scene client because the scene client does not receive person atoms
		// who are not on line.
		if(g_Au.IsSceneClient())
			return true;
		return AccountTypeOnLine() != ACCT_Count;
	}

	inline ACCOUNT AccountTypeOnLine() const
	{
		m_OnLineLock.lockForRead();
		ACCOUNT acctOnLine = m_AccountOnLine;
		m_OnLineLock.unlock();
		return acctOnLine;
	}

	// 'acctOnLine' is ACCT_Count when off line.
	inline void SetAccountTypeOnLine(ACCOUNT acctOnLine)
	{
		m_OnLineLock.lockForWrite();
		m_AccountOnLine = acctOnLine;
		m_OnLineLock.unlock();
	}

	inline bool IsAgentPowerOn() const
	{
		return m_AgentBits & AGENT_Enable;
	}

	// true after spawn recall has been initiated but before Respawn has been called.
	// true during the spawn recall fading shifting rune wait animation
	inline bool IsAwaitingSpawnRecall() const
	{
		m_PersonLock.lockForRead();
		bool v = (m_uiActions & ACT_SpawnRecall) ? true : false;
		m_PersonLock.unlock();
		return v;
	}

	// Returns true if this person is sitting in a captain's chair, regardless of viewing mode.
	bool IsInCaptainsChair(const DaVessel* ship) const;

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	inline bool IsMoving() const
	{
		m_PersonLock.lockForRead();
		bool v = (m_uiActions & ACT_MoveMask) ? true : false;
		m_PersonLock.unlock();
		return v;
	}

	inline bool IsTiltingHead() const
	{
		m_PersonLock.lockForRead();
		bool v = (m_uiActions & (ACT_LookUp|ACT_LookDown)) ? true : false;
		m_PersonLock.unlock();
		return v;
	}

	// Returns true when it's ok to tilt the head instead of rotating the entire body.
	// In zero-G it's preferable to rotate the body.
	// NOTE: The head should tilt under the same conditions under which the body orients, in DaSentient::Orient.
	//       When the body does not orient, tilting of the head changes the orientation of the body instead.
	inline bool OkToTiltHead() const
	{
		return IsInSpacecraft() ||
			IsInAtmosphere() ||
			IsAboardVehicle() ||
			IsLockedPose() ||
			CanControlMovement();
	}

	inline bool IsTurning() const
	{
		m_PersonLock.lockForRead();
		bool v = (m_uiActions & ACT_TurnMask) ? true : false;
		m_PersonLock.unlock();
		return v;
	}

	inline unsigned int Actions() const
	{
		m_PersonLock.lockForRead();
		unsigned int v = m_uiActions;
		m_PersonLock.unlock();
		return v;
	}

	virtual bool IsSightTarget(const DaAtom* toWho) const;

	// true if this person has a slot available to start another story.
	inline bool CanStartStory() const
	{
		// Count currently open stories. A player can only have so many open at a time.
		int storiesOpen = 0;
		m_JobsLock.lockForRead();
		for(AuJobList::const_iterator ji = m_Jobs.cbegin(); ji != m_Jobs.cend(); ji++)
			if((*ji)->Type() == AuJob::Story)
				storiesOpen++;
		m_JobsLock.unlock();
		return storiesOpen < MaxStoriesOpen();
	}

	DaBd* CurrentBd() const;	// May return 0

	void DepartStory(quint32 storyInstance, AuStory* stories);

	bool HasFinishedStory(quint32 scriptId) const;

	inline int MaxStoriesOpen() const
	{
		return 8;
	}

	// Starts the recall home sequence for this person. Not logged in stats as a recall.
	// Caller should DismountAllRiders.
	// 'spawnTime' is when to actually recall, typically 15 to 45 seconds from now. Can be now to recall immediately.
	void SpawnRecall(const AuTime& spawnTime);

	quint8 StoryLastResult(quint32 scriptId) const;

	// Client submits a custom story script.
	// Terminates current story, if any.
	// Starts the new script or reports as a thought why it won't start.
	void StoryScriptSubmitted(const AuScript& script);

	// true if eligible.
	// NOTE: Does not check CanStartStory.
	bool TestStoryCatalyst(const AuScript& script, const AuStories& stories, QString* startBlock, QMap<QString, QVariant>& vars);

	// True if story has ever been started, whether or not it is currently open.
	bool HasStartedStory(quint32 scriptId) const;

	// Returns true if this person is involved in the story 'storyInstance'
	virtual bool IsInStoryInstance(quint32 storyInstance) const
	{
		m_JobsLock.lockForRead();
		for(AuJobList::const_iterator ji = m_Jobs.cbegin(); ji != m_Jobs.cend(); ji++)
		{
			const AuJobStory* storyJob = dynamic_cast<const AuJobStory*>(*ji);
			if(storyJob && storyJob->StoryInstance() == storyInstance)
			{
				m_JobsLock.unlock();
				return true;
			}
		}
		m_JobsLock.unlock();
		return false;
	}

	// 'true' if 'scriptId' is in my list of open stories
	bool IsStoryOpen(unsigned int scriptId, const AuStories& stories) const;

	inline AuTime LastOnLine() const
	{
		m_OnLineLock.lockForRead();
		AuTime v(m_tmLastOnLine);
		m_OnLineLock.unlock();
		return v;
	}

	// Last time a story script was ever started.
	// Returns AuTime() if the script has never been started.
	AuTime LastStoryStart(unsigned int scriptId) const;

	// Last time a story script was ever ended, either finished or dropped.
	// Returns AuTime() if the script has never been terminated.
	AuTime LastStoryTerminated(unsigned int scriptId) const;

	void StartStory(unsigned int scriptId, quint32 storyInstance);

	QString StoryAuthor(quint32 storyInstance) const
	{
		m_JobsLock.lockForRead();
		for(AuJobList::const_iterator ji = m_Jobs.cbegin(); ji != m_Jobs.cend(); ji++)
		{
			const AuJobStory* storyJob = dynamic_cast<const AuJobStory*>(*ji);
			if(storyJob && storyJob->StoryInstance() == storyInstance)
			{
				QString v(storyJob->StoryAuthor());
				m_JobsLock.unlock();
				return v;
			}
		}
		m_JobsLock.unlock();
		return QString();
	}

	QString StoryBrief(quint32 storyInstance) const
	{
		m_JobsLock.lockForRead();
		for(AuJobList::const_iterator ji = m_Jobs.cbegin(); ji != m_Jobs.cend(); ji++)
		{
			const AuJobStory* storyJob = dynamic_cast<const AuJobStory*>(*ji);
			if(storyJob && storyJob->StoryInstance() == storyInstance)
			{
				QString v(storyJob->StoryBrief());
				m_JobsLock.unlock();
				return v;
			}
		}
		m_JobsLock.unlock();
		return QString();
	}

	// 'offeredBy' is the atom id who offered the choice. It will be null for narrator choices.
	void StoryChoicePicked(quint32 storyInstance, qint8 pickValue, AuStory* story, const AuScripts& scripts, const AuId& offeredBy);

	// Sends the dialog of the current story block without re-executing it.
	void StoryResendCurrentBlock(quint32 storyInstance, const AuStory* story, const AuScripts& scripts);

	// Send the hint associated with the current story block.
	void StorySendHint(quint32 storyInstance, const AuStory* story, const AuScripts& scripts);

	QString StoryTitle(quint32 storyInstance) const
	{
		m_JobsLock.lockForRead();
		for(AuJobList::const_iterator ji = m_Jobs.cbegin(); ji != m_Jobs.cend(); ji++)
		{
			const AuJobStory* storyJob = dynamic_cast<const AuJobStory*>(*ji);
			if(storyJob && storyJob->StoryInstance() == storyInstance)
			{
				QString v(storyJob->StoryTitle());
				m_JobsLock.unlock();
				return v;
			}
		}
		m_JobsLock.unlock();
		return QString();
	}

	void TickStories(const AuScripts& scripts);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void RefreshComplete();

	virtual void SetEmpireId(quint32 idEmpire)
	{
		if(Allegiance() != idEmpire)
		{
			// Empire has changed for this person.
			DaSentient::SetEmpireId(idEmpire);

			// Set reputation to 0.
			if(Reputation() > 0)
				DecreaseReputation(Reputation());
			else
				IncreaseReputation(-Reputation());

			// Cancel debt owed by former empire.
			ResetReward();
		}
	}

	// ACCT_Count is interpreted as 'off line'.
	// 'avgPingMs' is the average ping time in milliseconds.
	void SetOnLine(ACCOUNT acctType, unsigned int avgPingMs, const AuTime& when);

	void SetSpawnCity(const AuId& city, const QString& cityName, const AuGlobeLLF& devLL, const AuId& system,
			const QString& systemName)
	{
		m_PersonLock.lockForWrite();
		m_SpawnBd = city;
		m_SpawnBdName = cityName;
		m_SpawnBdSystem = system;
		m_SpawnBdSystemName = systemName;
		m_SpawnDevLL = devLL;
		m_tmSpawnChange.Set();
		m_uiActions |= ACT_SpawnTie;		// Trigger the spawn tie graphic.
		m_PersonLock.unlock();
	}

	inline bool SetSpawnCitySystemName(const QString& systemName)
	{
		if(SpawnCitySystemName() == systemName)
			return false;
		m_PersonLock.lockForWrite();
		m_SpawnBdSystemName = systemName;
		m_tmSpawnChange.Set();
		m_PersonLock.unlock();
		return true;
	}

	void SetSpawnShip(const AuId& spacecraft, const QString& spacecraftName, AuRosterEntry::TYPE berthType, bool isPrivate)
	{
		m_PersonLock.lockForWrite();
		if(m_SpawnShip != spacecraft || m_SpawnShipName != spacecraftName || m_BerthType != berthType ||
				m_SpawnShipIsPrivate != isPrivate)
		{
			m_SpawnShip = spacecraft;
			m_SpawnShipIsPrivate = isPrivate;
			m_SpawnShipName = spacecraftName;
			m_BerthType = berthType;
			m_tmSpawnChange.Set();

			m_uiActions |= ACT_SpawnTie;	// Trigger the spawn tie graphic.
		}
		m_PersonLock.unlock();
	}

	inline bool SpacecraftNameChanged(const AuId& shipId, const QString& nuName)
	{
		if(SpawnShip() != shipId)
			return false;

		m_PersonLock.lockForWrite();
		m_SpawnShipName = nuName;
		m_tmSpawnChange.Set();
		m_PersonLock.unlock();

		SetModified();
		return true;
	}

	// This function returns 0 when the spawn city id is 1.
	// Spawn city 1 is a bogus value to clear the "nubie emperor" status of this dude. It is never the id of a real city, more
	// likely to be the Universe itself.
	// To test for the special spawn city 1 case, see IsNewbieEmperor.
	inline AuId SpawnCity() const
	{
		AuId v;
		m_PersonLock.lockForRead();
		if(m_SpawnBd != AuId(1))
			v = m_SpawnBd;
		m_PersonLock.unlock();
		return v;
	}

	inline QString SpawnCityName() const
	{
		m_PersonLock.lockForRead();
		QString v(m_SpawnBdName);
		m_PersonLock.unlock();
		return v;
	}

	virtual QString HomeSolarSystemName() const
	{
		return SpawnCitySystemName();
	}

	inline AuId SpawnCitySystem() const
	{
		AuId v;
		m_PersonLock.lockForRead();
		if(m_SpawnBdSystem != AuId(1))
			v = m_SpawnBdSystem;
		m_PersonLock.unlock();
		return v;
	}

	inline QString SpawnCitySystemName() const
	{
		m_PersonLock.lockForRead();
		QString v(m_SpawnBdSystemName);
		m_PersonLock.unlock();
		return v;
	}

	// Development LL when spawn location is on a DaWorldLL.
	inline AuGlobeLLF SpawnDevLL() const
	{
		m_PersonLock.lockForRead();
		AuGlobeLLF v(m_SpawnDevLL);
		m_PersonLock.unlock();
		return v;
	}

	inline AuId SpawnShip() const
	{
		m_PersonLock.lockForRead();
		AuId v(m_SpawnShip);
		m_PersonLock.unlock();
		return v;
	}

	inline AuRosterEntry::TYPE SpawnShipBerthType() const
	{
		m_PersonLock.lockForRead();
		AuRosterEntry::TYPE v = m_BerthType;
		m_PersonLock.unlock();
		return v;
	}

	inline bool SpawnShipIsPrivate() const
	{
		m_PersonLock.lockForRead();
		bool v = m_SpawnShipIsPrivate;
		m_PersonLock.unlock();
		return v;
	}

	inline QString SpawnShipName() const
	{
		m_PersonLock.lockForRead();
		QString v(m_SpawnShipName);
		m_PersonLock.unlock();
		return v;
	}

	// Returns the State() of the HelmetItemWorn().
	quint8 WornHelmetState() const;

public:
	void ArenaExit();

	void ArenaRecall();

	void ArenaStatus();

	inline AuElapsedTimer LastClaimTime() const
	{
		m_PersonLock.lockForRead();
		AuElapsedTimer v(m_LastClaim);
		m_PersonLock.unlock();
		return v;
	}

	inline void LastClaimTimeSet()
	{
		m_PersonLock.lockForWrite();
		m_LastClaim.restart();
		m_PersonLock.unlock();
	}

	inline AuTime WhenArenaEntered() const
	{
		m_PersonLock.lockForRead();
		AuTime v(m_ArenaEntered);
		m_PersonLock.unlock();
		return v;
	}

	virtual DaAtom *Clone() const;

	virtual bool IsTicker() const;

	virtual quint64 MemoryUsage() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TickMovement(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual QString Name() const;

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	// Returns name with title.
	QString TitleName() const;

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	void DesignEnvironmentExit();

	inline QString DesignPassword() const
	{
		m_PersonLock.lockForRead();
		QString v(m_DesignPass);	// Only valid on server. Not valid on client.
		m_PersonLock.unlock();
		return v;
	}

	inline void SetDesignPassword(const QString& pass)
	{
		m_PersonLock.lockForWrite();
		m_DesignPass = pass;
		m_PersonLock.unlock();
	}

	inline int Reputation() const
	{
		m_PersonLock.lockForRead();
		int v = m_Reputation;
		m_PersonLock.unlock();
		return v;
	}

	virtual ATOM_TYPE AtomType() const;

	void DecreaseReputation(int inc)
	{
		Q_ASSERT(inc >= 0);
		if(inc > 0)
		{
			m_PersonLock.lockForWrite();
			m_Reputation -= inc;
			m_tmReputationChange.Set();
			m_PersonLock.unlock();
		}
	}
	void IncreaseReputation(int inc)
	{
		Q_ASSERT(inc >= 0);
		if(inc > 0)
		{
			m_PersonLock.lockForWrite();
			m_Reputation += inc;
			m_tmReputationChange.Set();
			m_PersonLock.unlock();
		}
	}

	virtual double AccountBalance() const
	{
		m_PersonLock.lockForRead();
		double v = m_AccountBalance;
		m_PersonLock.unlock();
		return v;
	}

	inline double LoanBalance() const
	{
		m_PersonLock.lockForRead();
		double v = m_LoanBalance;
		m_PersonLock.unlock();
		return v;
	}

	virtual void CreditAccount(double amount);

	virtual void DebitAccount(double amount);

	void LoanBorrow(double amount);

	void LoanPay(double amount);

	// 'blips' is the sensor blip array that is accumulating the scan data. Locations are relative to the solar system.
	// 'scannerLoc' is the location of the atom who is performing the scan, relative to the solar sytem.
	// 'scannerId' is the id of the atom who is performing the scan. Currently either a ship or a city.
	// 'scannerEmpire' is the empire id of the atom who is performing the scan.
	// 'scannerSensorRange' is the range in feet of the scanner atom's sensors.
	// 'scannerWeaponRange' is the range in feet of the scanner atom's bay/base weapons.
	// 'scanContent' is a bitmask of data that is sought in the scan
	//   b0 = sensor console blips
	//   b1 = fire control console blips
	//   b2 = transporter console blips
	//   b3 = locked-on blip
	//   b4 = AI pilot obstacle blips
	//   b5 = AI turret gunner target blips
	//   b6 = Scanning spacecraft wants its own subatoms to be scanned.
	// 'lockedBlip' is the id of the currently locked sensor blip
	virtual void SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
			int scannerDomain, const AuId& scannerWorld,
			float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const;

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	virtual void EnumControlPanels(AuQuadArray& aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	// Fills 'controls' with controls that could be activated by this scene atom. These are controls that do not require spotting
	// a particular target, such as entering a console, or adjusting room lighting.
	void EnumViewControls(QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	void LoadControls() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual bool IsControl(const DaAtom* sceneAtom) const;

	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// List all the patents in my possession.
	void ListPatents(AuKnownPatents& patents) const;

	// List all atoms in the scene. 'this' person is the scene atom.
	bool ListSceneAtoms(DaAtomList& sceneAtoms);

	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& gearPos,
			qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
			DiControlInputArray &aActiveControls, bool bContinuous);

	// Interaction with atoms subordinate to this atom.
	virtual double GroundClimbMax() const
	{
		// Override to return maximum amount of ground climb this unit can achieve.
		return 5.0;
	}

	// Returns incremented wormhole sequence number.
	int ExploredWormhole();

	inline int WormholesExplored() const
	{
		// Returns current wormhole sequence number.
		m_PersonLock.lockForRead();
		int v = m_Worms;
		m_PersonLock.unlock();
		return v;
	}

public:
	// Rendering functions.
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);
	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);
protected:
	GLuint CreateBodyPartTexture(AuGLEffects& re) const;
	GLuint CreateItemTexture(const AuItem& item, AuAtomRenderInfo& ari) const;

public:
	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	void RenderScene(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, DaAtom* targetAtom, int targetLevel, const AuPoint& targetLoc, unsigned int targetPart,
			POLITICAL_STANCE targetStance, const AuMatrix4x4& tDevPersp, const AuMatrix4x4& tDevOrtho,
			int hudIntensity);

	QImage SelfPortrait(AuAtomRenderInfo& ari, const QSize& size, const QColor& background);

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	void SetupCamera(AuMatrix4x4& camera, double& nearClip, double& farClip, AuAtomRenderInfo& ari) const;

	void PrepareForSampleRender();

	// The item will replace an existing item and, if 'dropIt' is true, will cause the replaced item to be dropped if it is used
	// for the same purpose. i.e. a helmet will cause me to drop my current helmet and use the nu helmet instead.
	// 'useIt' is 0=no, 1=yes, 2=only put in dominant hand if it's a weapon
	// 'useIt' puts weapons in my dominant hand or wear clothing and armor.
	// 'useIt' is 0 to put the item somewhere besides my dominant hand, clothing or armor positions.
	// Returns false if unable to add the item.
	virtual bool AddGearItemUseDrop(const AuItem &cItem, int useIt, bool dropIt);

	// Used when conducting barter transactions. Adds cItem.Count() items to my gear. If they don't fit, they fall at my feet.
	void AddGearItems(const AuItem& cItem);

	// Finds an item that is 'ready' for use. Item can be in top level gear or one level inside a container at a belt position.
	// Returns true if an item is found.
	inline AuGearPos FindReadyItem(COMMODITYID cid) const
	{
		m_GearLock.lockForRead();
		AuGearPos v = m_Gear.FindReadyItem(cid);
		m_GearLock.unlock();
		return v;
	}

	void GetFromCargo(const AuItem& item);

	// Checks to see if I have cItem.Count() of the items.
	bool HasGearItems(const AuItem& cItem) const;

	void InscribeGearAt(const AuGearPos& pos, const QString& inscription);

	void PutIntoCargo(const AuItem& item);

	void SetFlowerPot(const AuGearPos& pos, quint8 plantSize);

	void SetMessage(const AuGearPos& pos, const QString& message);

	void SetPictureFrame(const AuGearPos& pos, const QString& url, bool hideURL, bool rotate);

	// Uses the item that is in the 'pos' position.
	// Returns true if the item breaks due to usage.
	bool UseGearAt(const AuGearPos& pos);

	void GearItemsForSale(AuItemHash& c) const
	{
		m_GearLock.lockForRead();
		m_Gear.CombineItemsForSale(c);
		m_GearLock.unlock();
	}

	virtual const AuGear* LockGearForRead() const
	{
		m_GearLock.lockForRead();
		return &m_Gear;
	}

	virtual AuGear* LockGearForWrite()
	{
		m_GearLock.lockForWrite();
		return &m_Gear;
	}

	virtual void UnlockGear() const
	{
		m_GearLock.unlock();
	}

	int GetLaborBoost(COMMODITYID cid);

	int GetProductionBoost(COMMODITYID cid);

	// Returns cid of current use item.
	inline COMMODITYID CurrentItemCID() const
	{
		AuGearPos g = CurrentItemPos();
		if(!g.IsGear())
			return CID_Invalid;

		// This person has a "current" item selected.
		m_GearLock.lockForRead();
		if(!m_Gear.IsPosValid(g))
		{
			m_GearLock.unlock();
			return CID_Invalid;
		}

		COMMODITYID cid = m_Gear.ItemAt(g)->CommodityId();
		m_GearLock.unlock();
		return cid;
	}

	/*
	// Returns condition of current use item
	inline quint8 CurrentItemCondition() const
	{
		AuGearPos g = CurrentItemPos();
		if(!g.IsGear())
			return 0;

		// This person has a "current" item selected.
		m_GearLock.lockForRead();
		if(!m_Gear.IsPosValid(g))
		{
			m_GearLock.unlock();
			return 0;
		}

		quint8 v = m_Gear.ItemAt(g)->HitPoints();
		m_GearLock.unlock();
		return v;
	}
	*/

	inline bool CurrentItemIsHengeKey() const
	{
		AuGearPos g = CurrentItemPos();
		if(!g.IsGear())
			return false;

		// This person has a "current" item selected.
		m_GearLock.lockForRead();
		if(!m_Gear.IsPosValid(g))
		{
			m_GearLock.unlock();
			return false;
		}

		bool v = m_Gear.ItemAt(g)->IsHengeKey();
		m_GearLock.unlock();
		return v;
	}

	inline bool CurrentItemIsStargateKey() const
	{
		AuGearPos g = CurrentItemPos();
		if(!g.IsGear())
			return false;

		// This person has a "current" item selected.
		m_GearLock.lockForRead();
		if(!m_Gear.IsPosValid(g))
		{
			m_GearLock.unlock();
			return false;
		}

		bool v = m_Gear.ItemAt(g)->IsStargateKey();
		m_GearLock.unlock();
		return v;
	}

	virtual AuGearPos CurrentItemPos() const
	{
		m_PersonLock.lockForRead();
		AuGearPos v(m_CurrentGearPos);
		m_PersonLock.unlock();
		return v;
	}

	virtual INVENTORYSUB CurrentItemSub() const
	{
		m_PersonLock.lockForRead();
		INVENTORYSUB v = m_CurrentGearSub;
		m_PersonLock.unlock();
		return v;
	}

	/*
	inline quint8 CurrentItemQuality() const
	{
		AuGearPos g = CurrentItemPos();
		if(!g.IsGear())
			return 0;

		// This person has a "current" item selected.
		m_GearLock.lockForRead();
		if(!m_Gear.IsPosValid(g))
		{
			m_GearLock.unlock();
			return 0;
		}

		quint8 v = m_Gear.ItemAt(g)->Quality();
		m_GearLock.unlock();
		return v;
	}
	*/

	// Move an item to a top-level empty slot.
	void MoveGear(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType);

	// Move an item to a slot that already contains an item.
	void MoveGear(const AuGearPos& srcPos, const AuGearPos& destPos);

	// Returns true if the item at srcPos could be moved to the top level slot type. Does not check to see if there is room to
	// move the item to that slot type, just that it is ok to store the item in the slot type.
	bool TestMoveGear(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType) const;

	static bool TestMoveGear(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType, const AuGear& gear);

	// Try to pick up an atom and add it to my gear.
	// Return TRUE if I take the atom.
	bool PickUp(DaAtom *pAtom);

	bool TestGearQualityAt(const AuGearPos& pos);

public:
	virtual bool BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
			int& ammoFull) const;

	virtual void WeaponAtUsePos(const AuGearPos& pos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const;

protected:
	void ApplyMovement(unsigned int msElapsed);

	bool CanAddToSeaChest(const AuItem& iNew) const;

	// Returns gear position of the "current" light that is ON.
	// The current light is never inside a container, such as in a backpack.
	// The current light is the first light in the gear that is "on".
	// IP_My_Head is returned by entity.
	// Invalid pos is returned if no light is on.
	// Invalid pos is returned if this atom is subordinate to a vehicle that has a light.
	AuGearPos CurrentLightThatIsOn() const;

	void InitDNA();

	// Calculates the current weight of the gear in pounds.
	inline float GearWeight() const
	{
		m_GearLock.lockForRead();
		float v = m_Gear.Weight();
		m_GearLock.unlock();
		return v;
	}

	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	void QueryAudioEnvironment();
	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	void SelectNextItem();
	void SelectPreviousItem();

	// Clears the current story script. Does not terminate current story.
	// Returns true if a script was actually cleared.
	bool StoryClearScript();

	// Sets/replaces current story script. Does not terminate current story.
	void StorySetScript(const AuScript& script);

	void DropCurrentItem();

	void ThrowCurrentItem();

	// Use the current item's "item mode" on a target atom.
	// 'isAltMode' uses the alternate mode of the item, e.g. remove parts rather than install parts.
	void UseItemOnTarget(const AuId& target, unsigned int part, const AuPoint& targLoc, bool isAltMode);

	// Use an item's "item mode" on a target atom.
	// 'isAltMode' uses the alternate mode of the item, e.g. remove parts rather than install parts.
	// Returns 'item', or a replacement item if 'item' turns into something else, e.g. DNASampleKit turns into DNASample.
	// Replacement item is allocated on the heap and belongs to the caller. Original 'item' must also be managed by the caller.
	// NOTE: Expects gear lock to be locked already.
	AuItem* UseItemOnTarget(AuItem* item, const AuId& target, unsigned int part, const AuPoint& targLoc, bool isAltMode);

	float WeaponRange() const;	// Maximum range of weapon in hand.

	/*
	double Height() const
	{
		double rht = 5.75;
		switch(m_ePosition)
		{
		default:
		case POS_Stand:
			rht = 5.75;
			break;
		case POS_Crouch:
			rht = 28.0/12.0;
			break;
		case POS_Prone:
			rht = 12.0/12.0;
			break;
		}
		return rht;
	}
	*/

public:
	static DaPerson* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record
	virtual void StoredInDb();						// Called when this atom has been inserted or updated in the database.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

	// Physics
	// Acceleration in ft/sec/sec.
	virtual AuPoint Acceleration() const;

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

	void QuitClaimWorld();

	// Returns true if this unit can be selected by the scene atom to be given orders.
	virtual bool UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const;

	// Update my gear weight.
	// Expects gear lock to be locked for write.
	virtual void UpdateGearWeight()
	{
		m_WeightCarried = m_Gear.Weight();
		m_tmGearChange.Set();

		// When drifting in space, a gear change could allow me to begin to be pulled by gravity.
		if(!IsInGravity() && !IsInSpacecraft() && !IsInAtmosphere())
			SetInMotion();

		SetModified();
	}

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	void CargoDelivered(const AuId& cityId, const AuItem& item);
	void CitizenKilled(DaCitizen* victim);
	void CryoDelivered(const AuId& cityId, quint16 count);
	void CryoPickedUp(const AuId& cityId, const AuId& worldId, quint16 count);
	void PassengersDelivered(const AuId& cityId, quint16 count);
	void PassengersPickedUp(const AuId& cityId, const AuId& worldId, quint16 count);
	void PersonKilled(DaPerson* victim);
	void PrisonersLoaded(const AuId& cityId, quint16 count);
	void PrisonersUnloaded(const AuId& cityId, quint16 count);
	void SpacecraftCaptured(DaVessel* ship, const AuId& inSystemId);
	void SpacecraftCommanded(const AuId& shipId);
	void SpacecraftDestroyed(DaVessel* ship, const AuId& inSystemId);
	void SystemVisited(const AuId& systemId, DaBd& bd);
	void VehicleCaptured(DaVehicle* vehicle);
	void VehicleDestroyed(DaVehicle* vehicle);

protected:
	virtual bool AddGearItemNoLock(AuGear& gear, const AuItem& cItem);

	void DecayMoveActions();

	void FinishSpawn(const AuPoint& loc);

	// True while this dying highborn looks like an entity.
	inline bool IsDyingHighborn() const
	{
		if(!IsLineageHighborn())
			return false;	// Not highborn.

		if(IsBodyDead())
			return true;

		if(IsBodyDying())
			return true;

		return false;
	}

	// True while this nuborn highborn looks like an entity.
	inline bool IsNewbornHighborn() const
	{
		if(IsFullGrown())
			return false;	// Not in growth phase.
		if(!IsLineageHighborn())
			return false;	// Not highborn.

		// Newborn for the first minute to two minutes, based on when the minute ticks over after they are born.
		return m_MinutesToFullGrown >= GrowthTime()-1;
	}

	void RenderEntity(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double miniScale);

	virtual void RenderItemInHand(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, int pass,
			const DaAtom* sceneAtom, AuRenderLastMap* renderLast);

	void RenderHighbornEntity(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double miniScale);

	void RenderHelmetHUD(AuAtomRenderInfo& ari, bool leftOriented);

	void RenderHelmetHUDRadar(AuAtomRenderInfo& ari, bool leftOriented);

	// Selects a general pose for me or my mount based on movement controls and current pose.
	// 'nuPose' is current/proposed pose that may be overridden depending on all kinds of circumstances.
	AuPose SelectPoseForMeOrMount(const AuPose& nuPose, const DaCreature* poseWho) const;

public:	// AI
	virtual AuId AssignedShip() const
	{
		return SpawnShip();
	}

	virtual AuRosterEntry::TYPE AssignedShipBerthType() const
	{
		return SpawnShipBerthType();
	}

	virtual void ClearShipAssignment()
	{
		m_PersonLock.lockForWrite();
		m_SpawnShip = AuId(0);
		m_BerthType = AuRosterEntry::Invalid;
		m_PersonLock.unlock();
	}

	virtual QString DescribeActivity() const;

	// Creates and drops an item that was butchered from something with DNA.
	DaItem* DropItemButchered(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, DaAtom* itemParent, AuMatrix4x4& t,
			AuDNABase* dna, AuDNABase::CHROMATID dnaChroma, bool dnaIsViable, bool dnaIsSentient, GROWTHSTAGE dnaGrowthStage);

	// Creates and drops an item that was disassembled from something.
	DaItem* DropItemDisassembled(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, DaAtom* itemParent, AuMatrix4x4& t);

	// Returns true if this is an officer in a person's chain-of-command.
	// If true, the person is permitted to command this officer.
	virtual bool InChainOfCommand(const AuId& personId, quint32 personEmpire) const;

	// From DaNpc for an officer
	virtual quint8 Rank() const
	{
		quint32 xp = CommandExperiencePoints();
		if(xp < PromotionGoalRank(0))
			return 0;
		else if(xp < PromotionGoalRank(1))
			return 1;
		else if(xp < PromotionGoalRank(2))
			return 2;
		else if(xp < PromotionGoalRank(3))
			return 3;
		else if(xp < PromotionGoalRank(4))
			return 4;
		else
			return 5;
	}

	virtual QString RankAbbr() const
	{
		return "";
	}

	virtual QString RankName() const
	{
		if(IsLineageHighborn())
			return tr("Highborn");
		else if(IsLineageDemiavatar())
			return tr("Demiavatar");
		else
			return tr("Avatar");
	}

protected:	// AI
	// Called when demiavatar dies permanently. Kill's this atom.
	virtual void BodyHasDied(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
			const QString& killerName);

	void DeathLog(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
			const QString& whoName);

	void DepartStoryNoLock(quint32 storyInstance, AuStory* stories);

	void DestructStoryItems();

	virtual void DoBattleStation();

	virtual bool GainExperienceForWorking();

	// Check to see if I need to run to the sick bay for surgery.
	// Insures the ship I am aboard has a functioning surgery unit and medical kits in the hold.
	// Returns true if an order is amended to get medical attention.
	virtual bool GetACheckup();

	// Called after placement/spawn at nu job site, such as aboard a nuly assigned ship.
	virtual void GoToWork();

	virtual bool HasTheCon() const
	{
		return m_Con;
	}

	virtual NPC_ROLE NpcRole() const;

	// From DaNpc for an officer
	// Returns the total number of experience points needed to get promoted to the next rank above 'rank'.
	// This is the total experience points needed for the rank, not the experience points remaining to get there.
	// 0 is returned if 'rank' is the maximum rank.
	static quint32 PromotionGoalRank(quint8 rank);

	virtual void ReleaseTheCon()
	{
		m_PersonLock.lockForWrite();
		m_Con = false;
		m_tmOfficeChange.Set();
		m_PersonLock.unlock();

		SetModified();
	}

	virtual void ShipLog(DaVessel* ship);

	virtual void TakeOrder(const AuCommPacket& msg, CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc,
			bool firstInGroup, const QString& orderText=QString(""));

	virtual void TakePosture(const AuCommPacket& msg, CITIZEN_POSTURE order, bool firstInGroup);

	virtual void TakeTheCon()
	{
		m_PersonLock.lockForWrite();
		m_Con = true;
		m_tmOfficeChange.Set();
		m_PersonLock.unlock();

		SetModified();
	}

private:
	bool ClaimWorld(DaWorldLL* llworld, const AuPoint& flagLoc);

	// Returns true if any food is eaten.
	bool EatFoodFromGear();

	// Consumes as much food as possible to get full.
	void EatMealFromGear();

	// Returns 0 if I am protected from the environment by an environment suit, regardless of my actual environment.
	// Returns 1 if all I need to do is close my helmet.
	// Returns 2 if not at all.
	int IsProtectedByEVSuit() const;

	// Returns 0 if I am protected from the environment by diving gear, regardless of my actual environment.
	// Returns 1 if all I need to do is close my helmet.
	// Returns 2 if not at all.
	int IsProtectedByDivingGear() const;

	// 'which' is an index into m_MapPos.
	void MapCurrentGearItem(int which);

	void PlaceItemOnTarget(const AuGearPos& gearPos, const AuId& targetId, const AuPoint& targetLoc);

	// 'which' is an index into m_MapPos.
	void SelectMappedGearItem(int which);

	virtual int UseItem(AuItem& item, INVENTORYSUB sub);
};

#endif // DaPerson_H
