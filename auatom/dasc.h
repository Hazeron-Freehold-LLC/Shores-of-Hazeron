//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasc.h,v 1.195 2022/05/17 16:31:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaSc_H
#define DaSc_H

#include "auschelpers.h"
#include "auscreport.h"
#include "autime.h"
#include "davessel.h"
#include "dpconsoleempty.h"

class AuQuad;
class AuRefitMaterials;
class AuRefitShip;
class AuScPart;
class AuScRoute;
class PsEngineTrail;
class PsLandingSystem;
class PsLaunchSystem;

class AUATOM_DLL DaSc : public DaVessel
{
protected:
	enum AUDIO_SOURCE
	{
		AS_DoorExt,
		AS_DoorInt1,
		AS_DoorInt2,
		AS_DoorLift1,
		AS_DoorLift2,
		AS_GravityDriveExt,
		AS_GravityDriveInt,
		AS_PowerPlant,
		AS_Sensors,
		AS_TractorBeam,
		AS_Siren,
		AS_MaxSources
	};

	enum CONTROLINPUT
	{
		CI_OpenCloseDoor,
		CI_OperateTurboLift,
		CI_LockUnlockDoor,
	};

	enum TRANSPORTER_STATE
	{
		TRANSPORTER_Off,
		TRANSPORTER_On,
		TRANSPORTER_Active,
	};

private:
	// Unguarded
	ALERT_CONDITION						m_AlertCondition;		// Alert condition set by the commander.
	AuElapsedTimer						m_DbLoadTime;			// When loaded from the db last. Not stored.
	mutable bool						m_DesignCacheValid;
	bool								m_NavLights;			// On/off state of all nav lights.
	bool								m_New;					// True while this ship is still new.
	bool								m_RotatingBeacons;		// On/off state of all rotating beacons.
	bool								m_Strobes;				// On/off state of all strobes.
	bool								m_Ticked;				// True after first Tick with design loaded.
	bool								m_Tocked;				// True after first Tock with design loaded.

private:
	mutable QReadWriteLock				m_DesignLock;
	AuElapsedTimer						m_DesignChg;			// Time of last state change
	AuScReport							m_Design;				// Design id, analysis and currently installed modules.
	AuElapsedTimer						m_DoorChg;				// Time of last state change
	QHash<quint16, AuTime>				m_DoorOpenTimes;		// Time automatic doors were opened last.
	AuDoorStateHash						m_DoorStates;			// Key is part id.
	AuElapsedTimer						m_LandingGearChg;		// Time of last state change
	AuLandingGearStateHash				m_LandingGearStates;	// Key is part id. Value is AuScLandingGear::STATES.
	AuLiftCalls							m_LiftCalls;			// Pending call per lift. Not stored in db.
	AuElapsedTimer						m_LiftChg;				// Time of last state change
	AuLiftHash							m_Lifts;				// Key is turbo lift part id. Value is name of current stop.
	mutable AuPoint						m_TargetLoc;			// TargetLocation cached result.
	mutable bool						m_TargetLocValid;

private:
	mutable QReadWriteLock				m_AirLock;
	AuElapsedTimer						m_AirChg;				// Time of last state change
	AuAirPressureHash					m_AirPressures;			// Key is part id. Value is .5% per increment, so 200=100%, std=50%.

private:
	mutable QReadWriteLock				m_FTLLock;
	AuElapsedTimer						m_FTLChg;				// Time of last state change
	AuFTLDriveStateHash					m_FTLDrives;			// Key is part id.

private:
	mutable QReadWriteLock				m_LandingSystemLock;
	AuElapsedTimer						m_LandingSystemChg;		// Time of last state change
	AuLandingSystemStateHash			m_LandingSystemStates;	// Key is part id. Value is state of landing system.

private:
	mutable QReadWriteLock				m_LauncherLock;
	AuElapsedTimer						m_LauncherChg;			// Time of last state change
	AuLauncherStateHash					m_LauncherStates;		// Key is part id. Value is state of vehicle launcher.

private:
	mutable QReadWriteLock				m_RoomLightLock;
	AuElapsedTimer						m_RoomLightChg;			// Time of last state change
	AuRoomLightStateHash				m_RoomLightStates;		// Key is part id. Value is on/off state of light.

private:
	// Transporter room states. Design analysis knows the number of transporter rooms.
	mutable QReadWriteLock				m_TRoomLock;
	AuElapsedTimer						m_TRoomChg;				// Time of last transporter room state change
	AuTransporterSystemStateHash		m_TRoomStates;			// Key is part id of a room void. Value is TRANSPORTER_STATE.

private:
	mutable QReadWriteLock				m_TurretPosLock;
	AuElapsedTimer						m_TurretPosChg;			// Time of last state change
	AuTurretStateHash					m_TurretPos;			// Key is part id.

private:
	// OpenAL
	unsigned int						m_AudioShip[AS_MaxSources];
	int									m_DoorExt;				// 0=no change, 1=opened, 2=closed
	AuPoint								m_DoorExtLoc;			// Location of door relative to design.
	int									m_DoorInt1;				// 0=no change, 1=opened, 2=closed
	AuPoint								m_DoorInt1Loc;			// Location of door relative to design.
	int									m_DoorInt2;				// 0=no change, 1=opened, 2=closed
	AuPoint								m_DoorInt2Loc;			// Location of door relative to design.
	int									m_DoorLift1;			// 0=no change, 1=opened, 2=closed
	AuPoint								m_DoorLift1Loc;			// Location of door relative to design.
	int									m_DoorLift2;			// 0=no change, 1=opened, 2=closed
	AuPoint								m_DoorLift2Loc;			// Location of door relative to design.
	int									m_Siren;				// 0=no change, 1=opened, 2=closed

	// OpenGL
	QHash<quint16, QString>				m_DoorRooms;			// Key=door id, value=room name
	bool								m_DoorRoomsValid;		// True on server if m_DoorRooms has been collected. On server it
																// is guarded by m_DesignLock and only contains hull doors.
	QHash<quint16, QString>				m_RoomNames;			// Key=room id, value=room name
	QSet<QString>						m_SunlitRooms;			// Names of rooms whose interiors are in sunlight.
	AuTexturePalette					m_SurfaceTex;			// Texture palette.
	bool								m_VAOsCreated;			// true after we have created the VAOs.
	QHash<quint16, QString>				m_WindowRooms;			// Key=window id, value=room name
	AuRoomLightStateHash				m_WindowLightStates;	// Key is part id. Value is on/off state of fake window light.

	// Most normal faces are in the opaque and transparent VAOs.
	GLuint								m_DamageTex[9];
	GLuint								m_EffectCur[EFF_Count];
	int									m_EffectFrame;			// Last frame of sequential effect animation, 0-5, once per sec.
	int									m_EffectRandFrame;		// Last frame of random effect animation, 0-5, once per sec.
	GLuint								m_EffectTex[EFF_Count+6][6];	// Six frames of animation for each kind of surface effect.
																// Extras for low power trans, grav drive, ftl drive and ftl hull.
	int									m_HullEffectAppl;		// How to apply hull surface effect: 0=none, 1=lit by env, 2=glow
	QHash<quint16, AuScVAO>				m_OpaqueVAOs;			// Opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_TransparentVAOs;		// Transparent faces. Key is partId.

	// These faces require their own texture, such as a screens, icons and sketches.
	QHash<quint16, AuScVAO>				m_OwnOpaqueVAOs;		// Faces that need their own textures, e.g. screens, icons.
	QHash<quint16, GLuint>				m_OwnTextures;			// One per part that needs its own texture.
	QHash<quint16, AuScVAO>				m_OwnTransparentVAOs;	// Transparent faces that need their own textures, e.g. sketches.

	// Decals get blended in after opaque faces but before transparent faces, in the opaque pass.
	QHash<quint16, AuScVAO>				m_DecalOpaqueVAOs;		// Opaque faces of decals. Key is partId.
	QHash<quint16, AuScVAO>				m_DecalTransparentVAOs;	// Transparent faces of decals. Key is partId.
	QHash<quint16, AuScVAO>				m_OwnDecalOpaqueVAOs;	// Opaque faces of decals with own texture. Key is partId.
	QHash<quint16, AuScVAO>				m_OwnDecalTransparentVAOs;	// Transparent faces of decals with own texture. Key is partId.

	// Rooms get lit according to interior lighting conditions.
	QHash<quint16, AuScVAO>				m_IntRoomOpaqueVAOs;	// Opaque faces of interior rooms with no windows. Key is partId.
	QHash<quint16, AuScVAO>				m_IntRoomTransparentVAOs;	// Transparent faces of interior rooms. Key is partId.
	QHash<quint16, AuScVAO>				m_RoomOpaqueVAOs;		// Opaque faces of rooms with windows. Key is partId.
	QHash<quint16, AuScVAO>				m_RoomTransparentVAOs;	// Transparent faces of rooms with windows. Key is partId.

	// These faces are from windows, which change based on lighting conditions.
	QHash<quint16, AuScVAO>				m_WindowTranslucentVAOs;	// Translucent faces of windows.
	QHash<quint16, AuScVAO>				m_WindowTransparentVAOs;	// Transparent faces of windows.

	AuElapsedTimer						m_RenderStateTime;		// Last time render states were animated.

	// Door VAOs are keyed using StateVAOKey.
	QHash<quint32, AuScVAO>				m_DoorOpaqueVAOs;		// Opaque faces.
	QHash<quint16, quint8>				m_DoorRenderState;		// State of each door when rendered.
	QHash<quint32, AuScVAO>				m_DoorTransparentVAOs;	// Transparent faces.
	QHash<quint32, AuScVAO>				m_IntDoorOpaqueVAOs;		// Opaque faces.
	QHash<quint32, AuScVAO>				m_IntDoorTransparentVAOs;	// Transparent faces.
	QSet<quint32>						m_DoorVAOsMade;

	// Landing gear VAOs are keyed using StateVAOKey.
	QHash<quint16, quint8>				m_LandingGearRenderState;	// State of each door when rendered.
	QHash<quint32, AuScVAO>				m_LandingGearOpaqueVAOs;		// Opaque faces.
	QHash<quint32, AuScVAO>				m_LandingGearTransparentVAOs;	// Transparent faces.
	QSet<quint32>						m_LandingGearVAOsMade;

	// Turbo lift faces.
	QHash<quint16, AuScVAO>				m_LiftOpaqueVAOs;		// Turbo lift opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_LiftTransparentVAOs;	// Turbo lift transparent faces. Key is partId.

	// Lights are turned on and off. Lenses are accompanied by a glow effect when on.
	QColor								m_HColor;								// Headlight
	float								m_HCosInnerCone;						// Headlight
	float								m_HCosOuterCone;						// Headlight
	AuPoint								m_HDir;									// Headlight
	float								m_HkL;									// Headlight
	float								m_HkQ;									// Headlight
	AuPoint								m_HPos;									// Headlight
	float								m_LensBeaconAngle;						// Rotation angle of beacons.
	QHash<quint16, AuPoint>				m_LensBeaconAxes;						// Rotation axes for beacons.
	QHash<quint16, AuLensGlow>			m_LensBeaconGlows;						// Glow location and color for lenses.
	QHash<quint16, AuScVAO>				m_LensBeaconOpaqueVAOs;					// Opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_LensBeaconTransparentVAOs;			// Transparent faces. Key is partId.
	QHash<quint16, AuLensGlow>			m_LensHeadlightGlows;					// Glow location and color for lenses.
	QHash<quint16, AuScVAO>				m_LensHeadlightOpaqueVAOs;				// Opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_LensHeadlightTransparentVAOs;			// Transparent faces. Key is partId.
	QHash<quint16, AuLensGlow>			m_LensNavGlows;							// Glow location and color for lenses.
	QHash<quint16, AuScVAO>				m_LensNavOpaqueVAOs;					// Opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_LensNavTransparentVAOs;				// Transparent faces. Key is partId.
	QHash<quint16, AuLensGlow>			m_LensStrobeGlows;						// Glow location and color for lenses.
	QHash<quint16, AuScVAO>				m_LensStrobeOpaqueVAOs;					// Opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_LensStrobeTransparentVAOs;			// Transparent faces. Key is partId.
	GLuint								m_LightOmniTex;
	AuGLVAO								m_LightSpotVAO;							// VAO for glow effect on spot lights.
	GLuint								m_LightSpotTex;
	int									m_StrobesLit;							// Counter set to number of frames to stay lit.

	// Turret bases and guns. Gunner screens are in the m_Own vaos.
	QHash<quint16, AuScVAO>				m_TurBaseOpaqueVAOs;		// Opaque faces of turret bases. Key is partId.
	QHash<quint16, AuScVAO>				m_TurBaseTransparentVAOs;	// Transparent faces of turret bases. Key is partId.
	QHash<quint32, AuScVAO>				m_TurGunOpaqueVAOs;			// Opaque faces of turret guns. Key is StateVAOKey.
	QHash<quint32, AuScVAO>				m_TurGunTransparentVAOs;	// Transparent faces of turret guns. Key is StateVAOKey.
	QSet<quint32>						m_TurGunVAOsMade;

	// Empty command chair and consoles. These are the old style 3ds model-based stations. They are drawn when empty but the
	// DaCreature sitting there draws them when they are occupied, so they turn and move with the body of the occupant.
	DpConsoleEmpty						m_CmdEmpty;
	DpConsoleEmpty						m_EngineEmpty;
	DpConsoleEmpty						m_FireEmpty;
	DpConsoleEmpty						m_NavEmpty;
	DpConsoleEmpty						m_PilotEmpty;
	DpConsoleEmpty						m_PowerEmpty;
	DpConsoleEmpty						m_SensorEmpty;
	DpConsoleEmpty						m_ShieldEmpty;
	DpConsoleEmpty						m_SurgeryEmpty;
	DpConsoleEmpty						m_TransEmpty;

	// Engines
	QColor								m_ContrailColor;		// Acquired from empire record during animate.
	QList<PsEngineTrail*>				m_Contrails;
	bool								m_ContrailsValid;		// True if m_Contrails is valid, even if its empty.

	// Body stats.
	AuGLVAO								m_WeaponVAO;			// Guns/Weapons icon.

	// Launch and recovery systems.
	PsLandingSystem*					m_LandingSystem;
	PsLaunchSystem*						m_LaunchSystem;

	// Status displays.
	GLuint								m_AlertGreen;
	GLuint								m_AlertRedBright;
	GLuint								m_AlertRedDim;
	int									m_AlertStatus;			// Count of alert status displays, -1 if unknown.
	GLuint								m_AlertYellowBright;
	GLuint								m_AlertYellowDim;
	QHash<quint16, QString>				m_BerthNames;			// Berth names in current decals, keyed by decal id.
	int									m_BerthNameDecals;		// Count of berth name decals, -1 if unknown.
	QString								m_CaptName;				// Capt name in current decals.
	int									m_CaptNameDecals;		// Count of capt name decals, -1 if unknown.
	int									m_EmpireFlagDecals;		// Count of flag decals, -1 if unknown.
	int									m_EmpireNameDecals;		// Count of name decals, -1 if unknown.
	quint32								m_FlagEmpire;			// Empire id of current flag decal.
	int									m_LifeSupportStatus;	// Count of life support status displays, -1 if unknown.
	GLuint								m_LifeSupportStatusTex;
	int									m_MissionStatusETESecs;	// Last ETE in seconds displayed on mission status.
	GLuint								m_MissionStatusTex;
	QString								m_ShipName;				// Ship name in current decals.
	int									m_ShipNameDecals;		// Count of ship name decals, -1 if unknown.
	int									m_WorldMaps;			// Count of world maps, -1 if unknown.
	GLuint								m_WorldMapTex;			// This is the generic world map texture.

	// Current ambient light and fog states.
	float								m_AmbientR;
	float								m_AmbientG;
	float								m_AmbientB;
	AuPoint								m_AmbientUp;
	float								m_FogDensity;
	float								m_FogR;
	float								m_FogG;
	float								m_FogB;
	bool								m_InsideVisible;
	bool								m_IsAboard;
	int									m_MyExtLight;
	bool								m_SunsBlocked;

public:	// Constructors
	DaSc();
	virtual ~DaSc();

protected:
	DaSc(const DaSc& rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual void ActivateTransporterSystem(int troomIndex);

	virtual bool ActiveTransporterPadLocation(quint16 which, AuPoint& padLoc) const;

	virtual float AirPressure(const QString& roomName) const;

	// Apparent alert condition is controlled by the commander.
	virtual ALERT_CONDITION ApparentAlertCondition() const
	{
		return m_AlertCondition;
	}

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	virtual bool AnyDoorJammed(AuPoint& where) const;

	virtual bool AnyHullDoorOpen() const;

	virtual bool AnyHullDoorOpenInRoom(const QString& roomName) const;

	virtual bool AnyHullDoorUnlocked() const;

	virtual float ArmorLevel() const;

	virtual float BayWeaponPower(float unitsFired, quint16 which) const;

	virtual double BayWeaponRange(quint16 which) const;

	virtual float BayWeaponUnits(quint16 which) const;

	virtual AuPoint BerthLocation(const AuId& id) const;

	virtual AuPoint BerthLocation(const AuId& id, quint16& roomNumber) const;

	virtual bool BerthPosition(const DaCreature& occupant, AuMatrix4x4& berthM) const;

	virtual QString BerthRoomName(const AuRosterEntry& rosterEntry) const;

	virtual bool BridgeVisible(const QString& roomName) const;

	// Returns:
	//  0=lift already at stop, door already open
	//  1=no door at turbo lift stop
	//  2=not ready to move
	//  3=a door is stuck open,
	//  4=broken turbo lift data
	//  5=turbo lift moved to stop and door opened
	//  6=lift already at stop, door opened
	//  7=a door is blocked open
	virtual int CallTurboLift(quint16 liftId, const QString& liftStop);

	virtual bool CanBeSelected() const;

	virtual bool CanEnterConsole(const DaPerson& who, quint16& stationId, AuStation& station, quint16& which) const;

	virtual bool CanEnterNegativeWormhole() const;

	virtual bool CanEnterNeutralWormhole() const;

	virtual bool CanGrabLadder(const DaAtom& who, const AuPoint& loc, AuMatrix4x4& mLadder);

	virtual bool CanGrabLadderAt(const DaAtom& who, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder);

	virtual bool CanSetInMotion() const;

	virtual float CapacitorMax() const;

	virtual unsigned int CaptainsBerths() const;

	virtual unsigned int CargoCapacityVolume() const;

	virtual double CeilingHt(const AuPoint& loc, const AuPoint& gravityN, bool isLocInside) const;

	virtual bool CheckChildExit(const AuPoint &oldloc, const AuPoint &nuloc, const DaAtom* who) const;

	virtual bool CheckPeerJoin(const AuPoint &oldloc, const AuPoint &nuloc, const DaAtom* who);

	virtual void CitizenPosts(AuScPartSet& posts) const;

	virtual DaAtom* Clone() const;

	// Does not call SetModified.
	virtual void CloseAllDoors();

	virtual void CloseAllHullDoors();

	virtual void CloseAutomaticDoors();

	virtual void CloseDoor(unsigned int doorIndex);

	bool CloseLauncherDoors(quint16 launcherId);

	virtual void ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestGravityDrive, AuPoint& closestPowerPlant,
			AuPoint& closestSensor, AuPoint& closestFTLDrive, AuPoint& closestSiren) const;

	virtual int ClosestClosedDoor(const DaAtom* toWho, double& doorDistance) const;

	virtual int ClosestDoor(const AuPoint& toLoc, double& doorDistance) const;

	virtual int ClosestOpenDoor(const DaAtom* toWho, double& doorDistance) const;

	virtual bool ClosestUnoccupiedCommandStation(const AuPoint& toLoc, quint16& stationId, AuStation& station) const;

	virtual bool ClosestUnoccupiedCrewStation(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
			AuSkill::SKILL skill) const;

	virtual bool ClosestUnoccupiedSurgeryUnit(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
			bool isCrew) const;

	virtual bool ClosestUnoccupiedTroopStation(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
			const AuId& ignoreTroop) const;

	virtual bool ClosestVehicleParkingSpot(const DaVehicle* vehicle, AuMatrix4x4& parkPosM, VEHICLE_SPOT_TYPE& spotType) const;

	virtual bool ColonistVoidLocation(AuPoint& voidLoc) const;

	virtual void CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, AuCombatResult& result);

	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	virtual void ConfigureHarvester();

	virtual bool ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual unsigned int CrewBerths() const;

	virtual bool CrewDoorLocked(unsigned int doorIndex) const;

	virtual quint32 CryoBerths() const;

	virtual void CycleCapacitor();

	virtual void CycleFTLDrive();

	virtual void DamageAndDestroy(const QString& reason);

	virtual void DamageReportHp(AuDamage& damage) const;

	virtual void DamageReportPercent(AuDamage& damage, int structEquip) const;

	virtual void DeactivateTransporterSystems();

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual float Density() const;

	virtual QString Description() const;

	virtual AuPoint DesignCenterOffset() const;

	virtual AuPoint DesignCG() const;

	virtual unsigned int DesignHitsStructural() const;

	virtual unsigned int DesignHitsTotal() const;

	virtual AuId DesignArchitectId() const;

	virtual QString DesignArchitectName() const;

	virtual quint32 DesignId() const;

	virtual QString DesignName() const;

	virtual QString DesignReport(quint8 qualityMinimum, bool showTips) const;

	virtual void DischargeFTLDrive(quint16 whichFTL);

	virtual int EmptyBunkInBerth(const AuPoint& consoleLoc) const;

	virtual void EngageFTLDrive(quint16 whichFTL);

	virtual bool EnginesRequireEngineer() const;

	virtual AuPoint EntranceLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround,
			bool hullDoorsUsable) const;

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual int EstimateRefit(const AuRefitShip& refit, double& estCost, quint32& estSeconds, AuRefitMaterials& estMaterials) const;

	virtual AuPoint ExitLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround) const;

	virtual void FetchVehicle();

	void FilterDestinationLoc(DaAtom* moveAtom, const AuPoint& oldLoc, AuPoint& nuLoc) const;

	virtual DaCreature* FindPatient(const DaSentient& doctor) const;

	virtual bool FloorHt(const AuPoint& loc, const AuPoint& gravityN, double& htAbvFloor, bool& inGravityWell, bool& onStairs,
			AuPoint& stairNormal, bool isLocInside, const DaAtom* forWho) const;

	virtual AuPoint FloorLocInsideDoor(const AuPoint& doorLoc) const;

	virtual FTL_COMMIT FTLDriveCommitState(quint16 whichFTL) const;

	virtual quint8 FTLDriveSetting(quint16 whichFTL) const;

	virtual FTL_DRIVE_STATE FTLDriveState(quint16 whichFTL) const;

	virtual float FTLDriveUnitsToEngage() const;

	virtual quint16 FTLWarpDrive() const;

	virtual quint16 FTLWormholeDrive() const;

	virtual quint32 FuelCapacity() const;

	virtual bool GameTableLocation(quint16 stationId, AuMatrix4x4& tableM) const;

	virtual double Gravity(double cgDistance) const;

	virtual float GroundHeightAboveCG() const;

	virtual float HarvesterBayRange(float unitsFired, quint16 which) const;

	virtual bool HasFTLDrive() const;

	virtual bool HasManeuverDrive() const;

	virtual bool HasPowerPlant() const;

	virtual bool HasRetractableLandingGear() const;

	virtual bool HasSensors() const;

	virtual bool HasShields() const;

	virtual bool HasSpaceVehicleLauncher() const;

	virtual bool HasSpaceVehicleRecoverySystem() const;

	virtual bool HasTransporter() const;

	virtual bool HasWarpFTLDrive() const;

	virtual bool HasWeaponBay() const;

	virtual bool HasWorkingSurgeryUnit() const;

	virtual bool HasWormholeFTLDrive() const;

	virtual double Height() const;

	virtual AuPoint HelmLocation() const;

	virtual int RemoteDoorIndex(bool forVehicle, const AuPoint& loc) const;

	virtual int RemoteDoorIndex(bool forVehicle, const AuPoint& loc, quint16 roomId) const;

	virtual bool HullDoorLocked(unsigned int doorIndex) const;

	virtual STREAMLINE HullStreamline() const;

	virtual float HullStress() const;

	virtual quint32 HullVolume() const;

	// Returns volume of hull and everthing else allocated from that volume. Volumes are in whole cubic meters.
	void HullVolumeDistribution(quint32& hullVolume, quint32& armor, quint32& capacitor, quint32& cryoBerths, quint32& fuel,
			quint32& hold, quint32& lifeSupport, quint32& maneuverDrive, quint32& powerPlant, quint32& sensor, bool& hasSensor,
			quint32& shield, bool& hasShield, QMap<quint16, AuScReportSysVol>& ftlDrives,
			QMap<quint16, AuScReportSysVol>& bays, quint32& skeleton) const;

	// 'previewType' 0=none, 1=empty space, 2=sun, 3=sun and planet, 4=sun, planet and moon, 5=inferno, 6=frigid, 7=gas giant,
	//   8=ringworld, 9=sea bottom
	void InitPreview(quint8 previewType, const QString& studioName, const AuScDesign& scd, const DaPerson& person);

	// Initializes this ship's m_ssaShipSystems systems data.
	// This is only called once in the lifetime of this ship atom, at the time it is first created.
	void InitSystems(quint8 quality);

	// Updates m_ssaShipSystems to match the systems reported by my design.
	void ReconcileSystems();

	virtual void Insert(QSqlDatabase& db) const;

	virtual bool InsideRoom(const AuPoint& loc, quint16& roomId) const;

	virtual quint16 InsideRoomType(const AuPoint& loc, ROOMTYPE& roomType, QString& roomName) const;

	virtual bool InsideRoomType(const AuPoint& loc, bool getRoomUp, quint16& roomId, ROOMTYPE& roomType, double& htAbvFloor,
			QString& roomName, AuPoint& gravityN) const;

	virtual bool InstallSystemModule(const AuSysId& sysId, const AuItem& module, quint16 installSecs);

	virtual void InteriorAmbientLight(const QString& roomName, float& ambientR, float& ambientG, float& ambientB,
			bool& sunsBlocked) const;

	virtual INTERIOR_LIGHT InteriorLight(const QString& roomName) const;

	virtual bool InteriorLightToggle(INTERIOR_LIGHT state);

	virtual bool InteriorLightToggle(const QString& roomName, INTERIOR_LIGHT state);

	virtual bool IsAnyFTLDriveHot() const;

	virtual bool IsAnyFTLDriveWorking() const;

	// Returns true if any of my space vehicle launches is in a non-zero state.
	bool IsAnyLauncherActive() const;

	virtual bool IsCommandConsole(quint16 stationId) const;

	virtual bool IsControl(const DaAtom* sceneAtom) const;

	virtual bool IsDesignDataCacheValid() const;

	virtual bool IsDoorIndexValid(unsigned int doorIndex) const;

	virtual bool IsDoorJammed(unsigned int doorIndex) const;

	virtual bool IsDoorLocked(unsigned int doorIndex) const;

	virtual bool IsDoorOnWall(unsigned int doorIndex) const;

	virtual bool IsDoorOpen(unsigned int doorIndex) const;

	virtual bool IsEthereal() const;

	virtual bool IsFirstClass() const;

	virtual bool IsFTLDriveHot(quint16 whichFTL) const;

	virtual bool IsFTLDriveOn(quint16 whichFTL) const;

	virtual bool IsHoleOnWall(unsigned int holeIndex) const;

	virtual bool IsHullDoor(int doorIndex) const;

	virtual int IsInOrOnHull(const AuPoint& loc) const;

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsInsideAirBreathable() const;

	virtual bool IsLandingGearExtended() const;

	virtual bool IsNavLightOn() const;

	virtual bool IsNew() const;

	virtual bool IsObstacle(const DaAtom* toWho) const;

	virtual bool IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc) const;

	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho);

	virtual bool IsRotatingBeaconOn() const;

	virtual bool IsSecondClass() const;

	virtual bool IsSpaceStation() const;

	virtual bool IsStrobeOn() const;

	virtual bool IsTransporterRoom(quint16 roomId) const;

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual bool IsTransporterSystemActive(int troomIndex) const;

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual bool IsTransporterSystemOn(int troomIndex) const;

	virtual void JamDoorClosed(unsigned int doorIndex);

	virtual void JamDoorOpen(unsigned int doorIndex);

	virtual void LandingGearToggle(bool isExtended);

	virtual LANDING_ORIENT LandingOrientation() const;

	virtual double Length() const;

	virtual void ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const;

	virtual void ListOccupiedTroopStations(QSet<quint16>& troopStations, const AuId& exceptTroop) const;

	virtual void ListRoomNames(QStringList& roomNames) const;

	virtual void ListSystems(QList<AuSystemInfo>& systems) const;

	virtual void LivestockPosts(AuScPartSet& posts) const;

	static DaSc* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);

	virtual QString LocationDesc(const DaAtom& atom) const;

	inline const AuScReport& LockDesignAnalysis() const
	{
		m_DesignLock.lockForRead();
		return m_Design;
	}

	inline const AuDoorStateHash& LockDoorStates() const
	{
		m_DesignLock.lockForRead();
		return m_DoorStates;
	}

	virtual void LockLiftContents(quint16 liftId, DaAtomList& atomsInLift);

	virtual int LowestDeck() const;

	virtual float ManeuverAcceleration() const;

	virtual float Mass() const;

	virtual float MaxAtmosphereSpeed(const DaWorld* planet) const;

	virtual int MaxAtmosphereThrottle() const;

	virtual float MaxBayWeaponRange() const;

	virtual int MaxCapacitorWarpFactor() const;

	virtual float MaxTransporterRange() const;

	virtual quint64 MemoryUsage() const;

	virtual bool MoonOrbitRingsVisible(const QString& roomName) const;

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	virtual void NavLightToggle(bool onState);

	virtual void RotatingBeaconToggle(bool onState);

	virtual void StrobeToggle(bool onState);

	virtual void OccupyBerth(AuRosterEntry::TYPE type, const AuId& occId, const QString& occName);

	virtual unsigned int OfficerBerths() const;

	virtual int OnDesignDeck(const AuPoint& loc) const;

	virtual bool OpenDoor(unsigned int doorIndex);

	bool OpenLauncherDoors(quint16 launcherId);

	virtual bool OrbitRingsVisible(const QString& roomName) const;

	virtual double OrientationSpeed() const;

	virtual unsigned int PassengerBerths() const;

	// If 'medicStationId' is found, 'surgeryId' will be set to the same value.
	virtual void PatientStation(quint16 medicStationId, quint16& surgeryId, AuStation& station) const;

	virtual void PositionTurret(const DaCreature& gunner);

	virtual double PowerNeededToEngageFTLDrive(quint16 whichFTL, quint8 driveSetting) const;

	virtual float PowerPlantUnits() const;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& gearPos,
			qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed);

	virtual quint8 QualityOfDesign() const;

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual quint32 PowerPlantHits() const;

	virtual void ReengageFTLDrive(quint16 whichFTL);

	// All values must add up to the total hull volume. Values are in whole cubic meters.
	virtual void Refit(const AuRefitShip& refit, quint32 seconds);

	virtual AuPoint RemoteCameraLocation(quint16 cameraId) const;

	virtual quint16 RemoteCameraNext(quint16 currentCamera) const;

	virtual quint16 RemoteCameraPrevious(quint16 currentCamera) const;

	virtual bool RemoteCameraVerify(quint16 cameraId) const;

	virtual quint8 RemoveComponent(ROOMTYPE inRoomType, quint16 sightedPart, COMMODITYID cid, AuSysId& sysIdAffected, int& status);

	virtual void RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
			POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
			const AuMatrix4x4& tDevOrtho, float hudIntensity);

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

	virtual void RenderSc(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass);
	virtual void RenderScFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass);
	virtual void RenderScGlobe(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass);
	virtual void RenderScGlobeFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass);
	virtual void RenderScRingworld(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom,
			RENDERPASS pass);
	virtual void RenderScRingworldFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom,
			RENDERPASS pass);

	virtual void RepairCriticalSystems();

	virtual quint32 ReplaceComponent(ROOMTYPE inRoomType, unsigned int sightedPart, COMMODITYID cid, quint8 quality,
			unsigned int fixCount, AuSysId& sysIdRepaired, bool& systemModuleChange, bool forceUpgrade);

	virtual void RequestLaunch(DaCreature* pilot, DaVehicle* vehicle);

	virtual void RequestRecovery(DaCreature* pilot, DaVehicle* vehicle, const AuPoint& vehLoc);

	virtual bool RoomHasNavHUD(const QString& roomName) const;

	virtual bool RoomOrderLoc(const QString& roomName, AuPoint& orderLoc) const;

	virtual bool RoughExtents(AuBox &fExt) const;

	virtual bool RoughExtentsWithShields(AuBox &fExt) const;

	virtual double RoughRadius() const;

	bool Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, const DaAtom* toWho, bool isCrew) const;

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

	virtual void SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
			int scannerDomain, const AuId& scannerWorld,
			float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const;

	virtual float SensorRange() const;

	virtual void SetAlertCondition(ALERT_CONDITION alert);

	bool SetBridgeVisible(const QString& roomName, bool visible);

	void SetDesign(const AuScReport& r);

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	virtual void SetFTLDriveSetting(quint16 whichFTL, quint8 setting);

	virtual bool SetMoonOrbitRingsVisible(const QString& roomName, bool visible);

	virtual void SetNotNew();

	virtual bool SetOrbitRingsVisible(const QString& roomName, bool visible);

	virtual void SetSystemQuality(const AuSysId& sysIdRepaired, quint8 quality);

	virtual bool SetSystemNamesVisible(const QString& roomName, int visible);

	virtual void SetupArena(const QString& arenaName, quint8 team, quint8 shipQ, bool spawnOfficer, bool spawnCrew,
			bool spawnTroops, bool spawnPassengers, bool spawnVehicles);

	virtual void SetupLostAtlantis(quint32 empireId, quint8 quality, const AuSpecies& crewSpecies);

	virtual void SetupPirate(quint32 empireId, quint8 quality);

	virtual bool SetWormholeLinesVisible(const QString& roomName, bool visible);

	// 'loc' is in my space.
	// Does not test ShieldsWorking.
	bool ShieldContains(const AuPoint& loc) const;

	// Returns x, y, and z radii of shield bubble ellipsoid, even if this ship has no shields.
	virtual AuPoint ShieldRadii() const;

	virtual bool ShieldSurfaceNear(const AuPoint& loc) const;

	virtual double ShieldUnits() const;

	virtual AuSysId SightedSystem(quint16 sightedPart) const;

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
	// 'sightedPart' will be 0xffff if shield bubble was hit, only if eyeLoc is outside the shield bubble.
	bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4& model, AuPoint& sightedLoc, unsigned int& sightedPart,
			bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	virtual void SpawnArenaVehicle();

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	virtual int SystemNamesVisible(const QString& roomName) const;

	virtual AuTime StasisReadyTime() const;

	virtual bool Station(quint16 partId, AuStation& station) const;

	virtual bool Station(const DaCreature& who, AuStation& station) const;

	// Retrieves the screen quad for 'stationId'.
	// Returns true on success, false if design is not loaded/ready.
	virtual bool StationScreen(quint16 stationId, AuQuad& screen) const;

	virtual SPACECRAFT_STRUCTURE Structure() const;

	virtual quint32 SystemHits(const AuSysId& sysId) const;

	virtual quint32 SystemHitsRemainingSystemsLocked(const AuSysId& sysId) const;

	virtual quint32 SystemModuleCount(const AuSysId& sysId) const;

	virtual QString SystemName(const AuSysId& sysId) const;

	virtual AuPoint TargetLocation() const;

	virtual bool TestPowerSystem();

	virtual bool TestTransportLocation(const AuPoint& loc) const;

	virtual void TickOnClient(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void ToggleDoorLock(const DaPerson& person, unsigned int targetPart);

	virtual void ToggleDoorState(const DaSentient& who, unsigned int doorIndex);

	virtual float TractorBeamRange(float unitsFired, quint16 which) const;

	virtual void TransporterPadLocations(quint16 which, AuPointArray& pads) const;

	// 'which' is the transporter station/console part id.
	// Returns the part id since the console which is the same as the system which.
	virtual int TransporterSystemIndex(quint16 which) const;

	virtual int TransporterSystems() const;

	virtual QList<quint16> TransporterSystemWhiches() const;

	virtual quint16 TransporterSystemId(int padsDesired, bool mustBeOn) const;

	virtual bool TransporterVoidLocation(AuPoint& voidLoc) const;

	virtual quint16 TransporterWhich(const AuSensorBlip& blip) const;

	virtual quint16 TransporterWhich(const DaAtom& atom) const;

	virtual unsigned int TroopBerths() const;

	virtual bool TroopOrCrewVoidLocation(AuPoint& voidLoc) const;

	virtual void TroopPosts(AuScPartSet& posts) const;

	// Returns the current turbo lift stop of the lift 'liftId'.
	QString TurboLiftStop(quint16 liftId) const;

	// 'liftStops' is filled with the lift stops available to 'liftId'.
	virtual QString TurboLiftStop(quint16 liftId, QStringList& liftStops) const;

	virtual void TurnAllFTLDrivesOff();

	virtual void TurnFTLDriveOn(quint16 whichFTL);

	virtual void TurnFTLDriveOff(quint16 whichFTL);

	virtual float TurnRate() const;

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual void TurnTransporterSystemOff(int troomIndex);

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual void TurnTransporterSystemOn(int troomIndex);

	// Matrix used when aiming the gun barrel of a turret.
	//  AuPoint barrelLoc = barrelM.Translation();				// Axis point of barrel, relative to this ship.
	//  AuPoint barrelVec = barrelM.ProjectVector(0,0,-1);		// Aim vector of barrel at 0 elevation, relative to this ship.
	// 'baseRot' is rotation around the base/Z axis of the turret.
	void TurretAimMatrix(quint16 partId, float baseRot, AuMatrix4x4& barrelM) const;

	virtual AuPoint TurretHemiVector(const AuPose& pose, quint16 which) const;

	// Matrix to shoot out of the gun barrel of a turret.
	//  AuPoint muzzleLoc = muzzleM.Translation();				// Relative to this ship.
	//  AuPoint muzzleVec = muzzleM.ProjectVector(0,0,1);		// Relative to this ship.
	// 'baseRot' is rotation around the base/Z axis of the turret.
	// 'gunRot' is gun elevation, rotation around the gun/X axis of the turret.
	// See TurretSightMatrix.
	void TurretMuzzleMatrix(quint16 partId, float baseRot, float gunRot, AuMatrix4x4& muzzleM) const;

	virtual double TurretRange(quint16 which) const;

	virtual int Turrets() const;

	// Matrix to view out of the gun sight of a turret.
	//  AuPoint sightLoc = sightM.Translation();				// Relative to this ship.
	//  AuPoint sightVec = sightM.ProjectVector(0,0,-1);		// Relative to this ship.
	// 'baseRot' is rotation around the base/Z axis of the turret.
	// 'gunRot' is gun elevation, rotation around the gun/X axis of the turret.
	// See TurretMuzzleMatrix.
	void TurretSightMatrix(quint16 partId, float baseRot, float gunRot, AuMatrix4x4& muzzleM) const;

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual bool UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const;

	virtual void UnjamDoor(unsigned int doorIndex);

	inline void UnlockDesignAnalysis() const
	{
		m_DesignLock.unlock();
	}

	inline void UnlockDoorStates() const
	{
		m_DesignLock.unlock();
	}

	virtual void Update(QSqlDatabase& db) const;

	virtual void VehicleParkingSpots(VEHICLE_SPOT_TYPE spotType, QList<AuMatrix4x4>& parkingSpots) const;

	virtual quint16 VehicleTransporterSystemId(VEHICLE_SPOT_TYPE spotType, bool mustBeOn) const;

	virtual void WakeDoctor(const DaCreature& patient);

	virtual bool WeaponBayCanFire(quint16 which) const;

	virtual AuPoint WeaponBayEmitLoc(quint16 whichBay, const AuPoint& gunnerLoc) const;

	virtual void WeaponBayIds(QList<quint16>& bayIds) const;

	virtual quint16 WeaponBayNextId(quint16 which) const;

	virtual quint16 WeaponBayPreviousId(quint16 which) const;

	virtual int WeaponBays() const;

	virtual void WeaponBaysWithModule(QList<quint16>& bayIds, COMMODITYID bayModule) const;

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const;

	virtual double Width() const;

	virtual bool WormholeLinesVisible(const QString& roomName) const;

protected:
	void AnimateAlertStatus(const DaPerson* sceneAtom, const AuElapsedTimer& last);

	void AnimateBerthNames(AuGLEffects& re);

	void AnimateCaptName(AuGLEffects& re);

	void AnimateEffectTextures();

	void AnimateEmpireFlagAndName(AuGLEffects& re);

	void AnimateMissionStatus(const DaPerson* sceneAtom, AuGLEffects& re, const AuElapsedTimer& last);

	void AnimateRenderStates();

	void AnimateShipName(AuGLEffects& re);

	void AnimateWorldMap();

	// DesignLock is locked already when this is called.
	bool AnyTurboLiftDoorsBlockedOpen(quint16 liftId) const;

	// 'whoInCoC' is true if 'who' is InChainOfCommand.
	// 'gravityN' is 'who.GravityNormal()'
	// Returns part id of console if successful. 'stationType', 'opM', 'occPos' and 'emptySeatIndex' are output on success.
	quint16 AttemptConsoleEntry(const DaPerson& who, bool whoInCoC, quint16 consolePartId, const AuScPart& console,
			AuStation& station, const AuPoint& gravityN) const;

	void BalanceRoomAirPressures(bool lifeSupportWorking);

	// Returns the effect which index for a console. 'consoleId' should identify an engineer station or fire control station.
	quint8 ConsoleEffectWhich(quint16 consoleId) const;

	// Returns the effect which index for a transporter console.
	quint8 ConsoleTransporterEffectWhich(quint16 consoleId) const;

	// Returns true if I am destroyed.
	bool DamageSystem(const AuCombatResult& result, bool structural, quint32 hitPoints, int wpn1or2, const AuCombatWeapon& weapon,
			const QString& attackerName, bool& cargoLost, bool& fuelLeaked);

	virtual void DowngradeSystemModules(quint8 repairQ, const QSet<AuSysId>& systemsRepaired);

	void DesignGeometryNeeded();

	quint8 DoorState(quint16 door) const;

	// Returns a pointer to an array of effect textures that contains EFF_Count entries, one for each effect.
	const GLuint* EffectTextures() const;

	void FindGunners(DaAtomList& gunners) const;

	void FixTurboLiftAirBug();

	void GatherSystemsDataSystemsAndDesignLocked(quint8 quality);

	// Calls g_Au.LockScDesignsForRead.
	// Returns 0 if my design is not found.
	// Must call UnlockDesign even if 0 is returned.
	const AuScDesign* LockDesign() const;

	// Returns true if the door is opened.
	bool MoveLift(quint16 liftId, const DaAtomList& lockedAtoms, const QString& nuStop, quint16 doorId);

	void OperateLandingSystems();

	void OperateLifts();

	void RecalcSystemsHealthMax();

	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	void RenderDesign(const AuMatrix4x4& model, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, bool transparentPass,
			bool isAboard, bool insideVisible,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	void RenderDesignVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader);

	// 'sceneRoom' is the scene atom's room id, if they are aboard, otherwise 0.
	void RenderDoorVAOs(QHash<quint32, AuScVAO>& vaos, const AuGLSLScBase& shader, quint16 sceneRoom, float extAmbR, float extAmbG,
			float extAmbB);

	// 'sceneRoom' is the scene atom's room id, if they are aboard, otherwise 0.
	void RenderInteriorDoorVAOs(QHash<quint32, AuScVAO>& vaos, AuAtomRenderInfo& ari, const AuGLSLScBase& shader,
			quint16 sceneRoom, float extAmbR, float extAmbG, float extAmbB);

	void RenderInteriorRoomVAOs(QHash<quint16, AuScVAO>& vaos, AuAtomRenderInfo& ari, const AuGLSLScBase& shader);

	void RenderLandingGearVAOs(QHash<quint32, AuScVAO>& vaos, const AuGLSLScBase& shader);

	bool RenderLandingSystems(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, bool update);

	void RenderLensVAOs(QHash<quint16, AuScVAO>& vaos, const QHash<quint16, AuLensGlow>& glows, const AuGLSLScBase& shader,
			bool lightOn);

	void RenderLiftVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const AuGLSLScBase& shader);

	void RenderLightSprites(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

	void RenderOwnTextureVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader);

	void RenderPaths(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

	void RenderRoomVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader);

	void RenderScShader(AuGLSLScBase* shader, AuAtomRenderInfo& ari, const AuMatrix4x4& model, const DaPerson* sceneAtom);
	void RenderScShaderBlend(AuGLSLScBase* shader, AuAtomRenderInfo& ari, const AuMatrix4x4& model,
			const DaPerson* sceneAtom);

	void RenderTurretBaseVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, const AuGLSLScBase& shader);

	void RenderTurretGunVAOs(QHash<quint32, AuScVAO>& vaos, const AuMatrix4x4& model, const AuGLSLScBase& shader);

	void RenderWindowVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader);

	bool RenderVehicleLaunchers(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, bool update);

	void ResetActiveLaunchers();

	// Selects a nonstructural system that has hit points.
	// Nonstructural systems are reported as "Equipment" on the design analysis report.
	// If no structural system has hit points, SYS_Count is returned.
	AuSysId SelectNonstructuralSystemToDamage() const;

	// Selects a structural system that has hit points.
	// If no structural system has hit points, SYS_Count is returned.
	AuSysId SelectStructuralSystemToDamage() const;

	inline quint32 StateVAOKey(quint16 partId, quint8 state) const
	{
		return (state << 16) | partId;
	}

	virtual void SystemIds(QSet<AuSysId>& sysIds, bool structural) const;

	QList<quint16> TransporterRoomIds() const;

	void TurnOnBridgeIndicators();

	// Calls g_Au.UnlockScDesigns.
	void UnlockDesign() const;

	bool VehicleParkingSpot(VEHICLE_SPOT_TYPE spotType, AuMatrix4x4& parkPosM, const AuScDesign* scd, DaAtomList& vehiclesAboard);

	void WindowSurfaceGlow(const QString& windowName, float& glowR, float& glowG, float& glowB) const;
};

#endif // DaSc_H
