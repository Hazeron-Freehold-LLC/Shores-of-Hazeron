//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dabd.h,v 1.338 2022/05/17 15:45:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaBd_H
#define DaBd_H

#include "auatom.h"
#include "aubdreport.h"
#include "aubank.h"
#include "audamage.h"
#include "audnabase.h"
#include "aumakedesign.h"
#include "auordnance.h"
#include "auschelpers.h"
#include "aushipment.h"
#include "aushipsystem.h"
#include "auskill.h"
#include "auspecies.h"
#include "dacapsule.h"
#include "deinvitemid.h"
#include "demanufacturingprocess.h"
#include "dpconsoleempty.h"

class AuCommAcceptRefit;
class AuCommAcceptUpgradeInstall;
class AuCommAuction;
class AuCommBeamOver;
class AuCommBeamVehicleAtom;
class AuCommBeamVehicleCoordinate;
class AuCommBeamVehiclePlanet;
class AuCommBeamVehicleShip;
class AuCommBeamVehicleTransporter;
class AuCommBid;
class AuCommBidBarter;
class AuCommBidPickupShipment;
class AuCommBoardCrew;
class AuCommBoardCryo;
class AuCommBoardOfficers;
class AuCommBoardPassengers;
class AuCommBoardTroops;
class AuCommDebarkCryo;
class AuCommDebarkPassengers;
class AuCommDebarkTroops;
class AuCommDiplomaticMessage;
class AuCommForSale;
class AuCommHailRequest;
class AuCommodityExchange;
class AuCommOfferTEU;
class AuCommPacket;
class AuCommRequestExterminator;
class AuCommRequestHouseKeeping;
class AuCommRequestInventory;
class AuCommRequestRefit;
class AuCommRequestRepairAll;
class AuCommRequestRepairSystem;
class AuCommRequestRequestUpgradeSystem;
class AuCommRequestUpgradeSystem;
class AuDialog;
class AuEventLog;
class AuGame;
class AuPriceOffer;
class AuQuad;
class AuScPart;
class AuScRoute;
class AuTractorBeam;
class AuWorldEnclosure;
class DaNpc;
class DaSentient;
class DaVessel;
class DaWorldLL;
class DeInvItemId;
class PsLandingSystem;
class PsLaunchSystem;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Helper class used when preparing a sorted list of sell offers for the client.
class AUATOM_DLL SellOffer
{
public:
	QString				m_EmpireOfOrigin;
	quint16				m_InstallSecs;			// For upgrades
	AuItem*				m_Item;
	unsigned int		m_Price;
	quint16				m_Workers;

public:	// Construction
	SellOffer(const AuItem& item);

	SellOffer(const AuItem& item, quint32 countNeeded, quint16 installSecs, quint16 workers);

	inline ~SellOffer()
	{
		if(m_Item)
			delete m_Item;
	}

	inline SellOffer(const SellOffer& c)
	{
		m_EmpireOfOrigin = c.m_EmpireOfOrigin;
		m_InstallSecs = c.m_InstallSecs;
		if(c.m_Item)
			m_Item = c.m_Item->Clone();
		else
			m_Item = 0;
		m_Price = c.m_Price;
		m_Workers = c.m_Workers;
	}

	// true if this offer represents a better commodity than offer 'c'
	inline bool IsBetterProduct(const SellOffer& c) const
	{
		if(!m_Item)
			return false;
		if(!c.m_Item)
			return true;
		if(m_Item->Quality() > c.m_Item->Quality())
			return true;
		return false;
	}

	inline const SellOffer& operator=(const SellOffer& c)
	{
		m_EmpireOfOrigin = c.m_EmpireOfOrigin;
		m_InstallSecs = c.m_InstallSecs;
		if(m_Item)
		{
			delete m_Item;
			m_Item = 0;
		}
		if(c.m_Item)
			m_Item = c.m_Item->Clone();
		m_Price = c.m_Price;
		m_Workers = c.m_Workers;
		return *this;
	}

	// Needed by 'contains' function of QVector.
	// Only checks commodity id.
	inline bool operator==(const SellOffer& c) const
	{
		if(m_Item->IsItem(*c.m_Item))
			return false;
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuPriceOffer
{
public:
	quint32								m_BuyPrice;				// The price this city will pay for the item.
	quint32								m_SellPrice;			// The price this city will accept for the item.
	AuElapsedTimer						m_When;					// When this price offer was made.

public:
	inline AuPriceOffer(int buyPrice, int sellPrice)
	{
		m_BuyPrice = buyPrice;
		m_SellPrice = sellPrice;
		m_When.start();
	}

	inline AuPriceOffer()
	{
		m_BuyPrice = 0;
		m_SellPrice = 0;
	}

	inline AuPriceOffer(const AuPriceOffer& c)
	{
		m_BuyPrice = c.m_BuyPrice;
		m_SellPrice = c.m_SellPrice;
		m_When = c.m_When;
	}

public:	// Operators
	inline const AuPriceOffer& operator=(const AuPriceOffer& c)
	{
		m_BuyPrice = c.m_BuyPrice;
		m_SellPrice = c.m_SellPrice;
		m_When = c.m_When;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_BuyPrice;
		out << m_SellPrice;
		AuTime when;
		out << when;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_BuyPrice;
		in >> m_SellPrice;
		AuTime when;
		in >> when;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuPriceOffer& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuPriceOffer& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBdRender
{
public:
	enum AUDIO_SOURCE
	{
		AS_DoorExt,
		AS_DoorInt1,
		AS_DoorInt2,
		AS_DoorLift1,
		AS_DoorLift2,
		AS_PowerPlant,
		AS_Sensors,
		AS_TractorBeam,
		AS_MaxSources
	};

public:
	// OpenGL
	QHash<quint16, QString>				m_DoorRooms;			// Key=door id, value=room name
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
	QHash<quint16, quint8>				m_DoorRenderState;		// State of each door when last rendered.
	QHash<quint32, AuScVAO>				m_DoorTransparentVAOs;	// Transparent faces.
	QHash<quint32, AuScVAO>				m_IntDoorOpaqueVAOs;		// Opaque faces.
	QHash<quint32, AuScVAO>				m_IntDoorTransparentVAOs;	// Transparent faces.
	QSet<quint32>						m_DoorVAOsMade;

	// Turbo lift faces.
	QHash<quint16, AuScVAO>				m_LiftOpaqueVAOs;		// Turbo lift opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_LiftTransparentVAOs;	// Turbo lift transparent faces. Key is partId.

	// Fence and wall faces always use the default texture palette.
	QHash<quint32, AuTexturePalette>	m_FenceCustomTex;			// Texture palettes for custom fences.
	QHash<quint32, AuScVAO>				m_FenceCustomOpaqueVAO;			// From fence with custom texture palette.
	AuScVAO								m_FenceStyleOpaqueVAO;			// From fence with standard fence style texture palette.
	QHash<quint32, AuScVAO>				m_FenceCustomTransparentVAO;	// From fence with custom texture palette.
	AuScVAO								m_FenceStyleTransparentVAO;		// From fence with standard fence style texture palette.

	// Lights are turned on and off. Lenses are accompanied by a glow effect when on.
	QColor								m_HColor;								// Headlight = street light
	float								m_HCosInnerCone;						// Headlight = street light
	float								m_HCosOuterCone;						// Headlight = street light
	AuPoint								m_HDir;									// Headlight = street light
	float								m_HkL;									// Headlight = street light
	float								m_HkQ;									// Headlight = street light
	AuPoint								m_HPos;									// Headlight = street light
	bool								m_IsDark;
	float								m_LensBeaconAngle;						// Rotation angle of beacons.
	QHash<quint16, AuPoint>				m_LensBeaconAxes;						// Rotation axes for beacons.
	QHash<quint16, AuLensGlow>			m_LensBeaconGlows;						// Glow location and color for lenses.
	QHash<quint16, AuScVAO>				m_LensBeaconOpaqueVAOs;					// Opaque faces. Key is partId.
	QHash<quint16, AuScVAO>				m_LensBeaconTransparentVAOs;			// Transparent faces. Key is partId.
	QHash<quint16, AuLensGlow>			m_LensDimLightGlows;					// Candle light glow location and color for lenses.
	QHash<quint16, AuLensGlow>			m_LensHeadlightGlows;					// Powered light glow location and color for lenses.
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

	// Boards framework when under construction.
	AuScVAO								m_BoardsVAO;

	// Body stats.
	AuGLVAO								m_WeaponVAO;			// Guns/Weapons icon.

	// Launch and recovery systems.
	PsLandingSystem*					m_LandingSystem;
	PsLaunchSystem*						m_LaunchSystem;

	// Status displays.
	int									m_EmpireFlagDecals;		// Count of flag decals, -1 if unknown.
	int									m_EmpireNameDecals;		// Count of name decals, -1 if unknown.
	quint32								m_FlagEmpire;			// Empire id of current flag decal.
	int									m_LifeSupportStatus;	// Count of life support status displays, -1 if unknown.
	GLuint								m_LifeSupportStatusTex;
	int									m_WorldMaps;			// Count of world maps, -1 if unknown.
	GLuint								m_WorldMapTex;			// This is the generic world map texture.

	// OpenGL
	float								m_fShieldDuration[MAX_SHIELD_HIT_EFFECTS];
	float								m_fShieldEffect[MAX_SHIELD_HIT_EFFECTS*4];
	float								m_fShieldTexOffset;		// Texture offset used for shield rendering.
	AuElapsedTimer						m_shieldTime;			// Timer used for shield effect.
	AuTractorBeam*						m_TractorBeam;

	// Cursor when drawing nu Bd and showing existing Bd hit.
	AuGLVAO								m_CursorVAO;
	AuGLVAO								m_CursorCoverageVAO;
	bool								m_CursorSiteOnly;

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
	bool								m_SunsBlocked;

	// Empty command chair and consoles. These are the old style 3ds model-based stations. They are drawn when empty but the
	// DaCreature sitting there draws them when they are occupied, so they turn and move with the body of the occupant.
	DpConsoleEmpty						m_CmdEmpty;
	DpConsoleEmpty						m_FireEmpty;
	DpConsoleEmpty						m_NavEmpty;
	DpConsoleEmpty						m_SensorEmpty;
	DpConsoleEmpty						m_ShieldEmpty;
	DpConsoleEmpty						m_SurgeryEmpty;
	DpConsoleEmpty						m_TransEmpty;

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

private:
	// Data created by PrepareToRender.
	quint16 beaconPartId = 0;
	AuScTriRenderList beaconOpaqueTris, beaconTransparentTris;
	quint16 decalPartId = 0;
	AuScTriRenderList decalOpaqueTris, decalTransparentTris;
	quint16 hullPartId = 0;
	AuScTriRenderList hullOpaqueTris, hullTransparentTris;
	quint16 streetLightPartId = 0;
	AuScTriRenderList streetLightOpaqueTris, streetLightTransparentTris;
	quint16 strobePartId = 0;
	AuScTriRenderList strobeOpaqueTris, strobeTransparentTris;
	bool customFence = false;
	AuScTriRenderList boardsOpaqueTris;
public:
	QHash<quint32, AuSegmentArray> fenceSegs;	// Fence segments. Key is fence style or a design, which must be > STYLE_Count.
private:
	QHash<quint16, AuScTriRenderList> decalOwnOpaqueTris;
	QHash<quint16, AuScTriRenderList> decalOwnTransparentTris;
	QHash<quint16, AuScTriRenderList> liftOpaqueTris;
	QHash<quint16, AuScTriRenderList> liftTransparentTris;
	QHash<quint16, AuScTriRenderList> navOpaqueTris;
	QHash<quint16, AuScTriRenderList> navTransparentTris;
	QHash<quint16, AuScTriRenderList> ownOpaqueTris;
	QHash<quint16, AuScTriRenderList> ownTransparentTris;
	QHash<quint16, AuScTriRenderList> roomOpaqueTris;
	QHash<quint16, AuScTriRenderList> roomTransparentTris;
	QHash<quint16, AuScTriRenderList> sunnyroomOpaqueTris;
	QHash<quint16, AuScTriRenderList> sunnyroomTransparentTris;
	QHash<quint16, AuScTriRenderList> windowTranslucentTris;
	QHash<quint16, AuScTriRenderList> windowTransparentTris;

public:
	AuBdRender();
	AuBdRender(const AuBdRender& c);
	~AuBdRender();

public:
	void Animate(const AuId& id, const AuElapsedTimer& lastAnim, bool inDesignEnvironment, float illumination,
			BUILDING_STRUCTURE structure, bool lightsWorking);

	// Copy VAOs and other data from 'c' created by PrepareForRendering and CreateVAOs by calls from the model maker thread.
	void CopyVAOs(const AuBdRender& c);

	// OpenGL context must be current to call these functions.
	void CreatePartTextures(const AuScParts& parts, AuGLEffects& re);	// Must call PrepareForRendering first.
	void CreateTextures(const AuId& id, AuGLEffects& re);
	void CreateVAOs(AuOpenGLFunctions& GLf);
	void DeleteGL();

	// OpenGL context does not have to be current when calling this function. Can be called by a worker thread.
	void PrepareForRendering(const AuScParts& parts, const QSet<quint32>& bdDesigns, QColor themeColors[AuScFace::THEME_Count]);
	void PrepareForRenderingFrame(const AuScParts& parts, COMMODITYID hullMaterial, int percentComplete);

protected:
	void AnimateFacadeWindowLights(BUILDING_STRUCTURE structure, float illumination, bool lightsWorking);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaBd : public DaCapsule
{
protected:
	enum BLDCON
	{
		CON_Road						= 0x0001,
		CON_Wharf						= 0x0002,
		CON_Airport						= 0x0004,				// also space
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

protected:
	// Unguarded
	bool								m_bTaxSubmit;			// True if taxes where submitted this midnight hour.
	mutable bool						m_DesignCacheValid;
	bool								m_HasAir;				// true if the city had air at the last report
	INDUSTRYID							m_IndustryId;			// INDUSTRYID of this building.
	qint8								m_Morale;				// Current morale, as of the conclusion of the last city report.
	quint8								m_QMade;				// Quality of original construction. May exceed Q of design.

protected:
	mutable QReadWriteLock				m_BdLock;
	AuId								m_CapitolId;			// City capitol building id. 0 if none. 0 if military.
	QString								m_CityName;				// City name if this is a capitol. Base name if this is a base HQ.
	mutable quint32						m_FoodPerMeal;			// Amount of food nutrition points eaten by each citizen at a meal.
	AuId								m_FounderId;			// Capitol only
	QString								m_FounderName;			// Capitol only
	AuId								m_PlatformId;			// Id of platform, when placed on a platform. Not subordinate.
	AuCityReportHistory					m_ReportHistory;
	ECOROLE								m_ResidentEcoRole;
	quint16								m_Residents;			// Citizens who live in this building.
	QHash<AuId, quint16>				m_ResidentsEmployed;	// Citizens who live in this building who have jobs. Key is job Bd.
	AuSpecies							m_ResidentSpecies;		// Species of residents and workers.
	QHash<COMMODITYID, quint32>			m_RocksOnSite;			// Rocks on my site.
	AuElapsedTimer						m_tmCapitolChange;		// For client updates. Not serialized.
	AuElapsedTimer						m_tmCitizenChange;		// Residents or workers. For client updates. Not serialized.
	AuTime								m_tmReport;				// Time of the last CityCycle.
	AuElapsedTimer						m_tmResidentChange;		// Resident species change. For client updates. Not serialized.
	quint16								m_Workers;				// Workers at this building. May or may not have come from here.

	quint8								m_ReportPopBirths;		// Born since last report.
	quint8								m_ReportPopDeaths;		// Died since last report.
	quint8								m_ReportPopIndigenousArr;	// Arrived from the wilderness since last report.
	quint8								m_ReportPopIndigenousDep;	// Departed to the wilderness since last report.
	quint8								m_ReportPopSeafarersArr;	// Arrived by sea since last report.
	quint8								m_ReportPopSeafarersDep;	// Departed by sea since last report.
	quint8								m_ReportPopTravellersArr;	// Arrived by air|space since last report.
	quint8								m_ReportPopTravellersDep;	// Departed by air|space since last report.
	quint8								m_ReportPopWanderersArr;	// Arrived by land since last report.
	quint8								m_ReportPopWanderersDep;	// Departed by land since last report.

private:
	mutable QReadWriteLock				m_ModDbLock;
	AuElapsedTimer						m_tmModDb;

protected:
	mutable QReadWriteLock				m_VesselLock;
	float								m_CapacitorLevel;		// Quantity of energy units in the capacitor.

protected:
	// Ships treasury
	mutable QReadWriteLock				m_AccountLock;
	AuBank								m_Bank;					// Current currency aboard ship
	AuElapsedTimer						m_tmLastBank;			// for client updates, not stored in db

protected:
	mutable QReadWriteLock				m_BayLock;
	unordered_map<quint16, AuBayFired>	m_BayFired;				// Key is 'which' bay.
	double								m_BayRadarRange;		// calculated when needed, not stored in db
	AuElapsedTimer						m_tmBayFired;			// for client updates, not stored in db
	AuElapsedTimer						m_tmBayRanged;			// When m_BayRadarRange was last calculated. not stored in db
	unordered_map<quint16, AuBaySetting>	m_BaySetting;		// Settings and ammo for each bay, weapons and harvesters.

protected:
	mutable QReadWriteLock				m_CargoLock;
	AuItemHash							m_Cargo;				// Items in storage here.
	AuSpecies							m_LivestockSpecies;		// Species of livestock that is produced at this farm.
	AuKnownPatents						m_Patents;				// Knowledge of how to run processes.
	AuElapsedTimer						m_PatentsChanged;		// For updates to clients. Not serialized.
	quint16								m_StoreCommodity;		// COMMODITYID to store, for warehouses.
	AuElapsedTimer						m_tmCargoChange;
	AuItemHash							m_Tools;				// AuItems used for mfg and const, i.e. tools, electricity, etc.
	QList<COMMODITYID>					m_Vehicles;				// Vehicles at this building, for production bonuses. One entry per
																// vehicle to show presence. Not stored in the database.
	AuElapsedTimer						m_VehiclesTallied;		// For updates to m_Vehicles. Not serialized.
	// Broker only, guarded by m_CargoLock
	AuShipmentList						m_Shipments;			// Shipments to other cities.
	AuTradeAgreementList				m_TradeAgreements;		// Trade agreements with other brokers.

protected:
	mutable QReadWriteLock				m_CommLock;
	AuElapsedTimer						m_tmAlert;				// Time of last Alert or attack enemy broadcast.
	AuElapsedTimer						m_tmDiplomaticMessage;	// Time of last diplomatic message.
	AuElapsedTimer						m_tmSOS;				// Time of last SOS broadcast, so we don't send them too often.

protected:
	mutable QReadWriteLock				m_ConstructionLock;
	DeConstructionProcess*				m_pConstruct;			// Only exists during the construction of this development.

protected:
	mutable QReadWriteLock				m_MfgProcLock;
	DeManufacturingProcessArray			m_MfgProc;				// Manufacturing processes.
	AuElapsedTimer						m_tmMfgProcChange;

protected:
	mutable QReadWriteLock				m_DamageLock;
	AuDamage							m_Damage;				// Damage to my systems.
	AuTime								m_tmDamageChange;		// Also the time the ship was put in stasis, to delay restoration.

protected:
	// Games at game tables are allocated when created.
	mutable QReadWriteLock				m_GamesLock;
	unordered_map<quint16, AuGame*>		m_Games;				// Games in progress at game tables. Key is table 'which' index.
	AuElapsedTimer						m_GamesChanged;			// For updates to clients. Not serialized.

protected:
	mutable QReadWriteLock				m_LastInRangeLock;		// Last range time is mutable so I can reset it in SensorScan.
	mutable AuElapsedTimer				m_LastInEnemyRange;		// When in range of enemy bay weapon. Not stored or sent to client.

protected:
	mutable QReadWriteLock				m_NameWithEmpireLock;
	mutable QString						m_NameWithEmpire;		// ""=unknown

protected:
	mutable QReadWriteLock				m_ShieldsLock;
	double								m_ShieldCap;			// Shield capacitor
	mutable AuPoint						m_ShieldCenter;			// For shield generators.
	quint8								m_ShieldScram;			// Shield scram time, in seconds.

protected:
	mutable QReadWriteLock				m_SystemsLock;
	AuShipSystemArray					m_ssaShipSystems;		// List of all Ship Systems. ONLY A SUBSET IS SENT TO THE CLIENT.
	AuElapsedTimer						m_tmSystemsUpg;			// Time a system quality changed in the m_ssaShipSystems data.

protected:
	mutable QReadWriteLock				m_TurretLock;
	unordered_map<quint16, AuOrdnance>	m_Turrets;				// Key is 'which' turret. Tracks ammo and firing state.

private:
	// Unguarded
	bool								m_Ticked;				// True after first Tick with design loaded.
	bool								m_Tocked;				// True after first Tock with design loaded.

private:
	mutable QReadWriteLock				m_DesignLock;
	AuElapsedTimer						m_DesignChg;			// Time of last state change
	AuBdReport							m_Design;				// Design id, analysis and currently installed modules.
	AuElapsedTimer						m_DoorChg;				// Time of last state change
	QHash<quint16, AuTime>				m_DoorOpenTimes;		// Time automatic doors were opened last.
	AuDoorStateHash						m_DoorStates;			// Key is part id.
	QHash<quint16, QString>				m_HullDoorRooms;		// Server only. Key=door id, value=room name
	bool								m_HullDoorRoomsValid;	// True on server if m_HullDoorRooms has been collected.
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
	mutable QMutex						m_EventLogLock;			// Guards m_EventLog
	AuEventLog*							m_EventLog;				// For a summary of noteworthy events in each city report.

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
	mutable QReadWriteLock				m_TradeConnectionsLock;
	QHash<AuId, int>					m_TradeConnections;		// Ids of connected buildings and BLDCON bitmaps. Not stored in db
	AuElapsedTimer						m_TradeConnectionsChg;	// Time of last state change
	bool								m_Airport;				// True if I am connected to an airport on my own world.
	bool								m_Wharf;				// True if I am connected to a wharf on my own world.

private:	// Spacecraft Factory
	mutable QReadWriteLock				m_SpacecraftFactoryLock;
	quint8								m_CompanyMaxQ;			// Maximum quality designs permitted for company ship.s
	bool								m_CompanyOk;			// True if ok to build company ships.
	AuMakeDesign						m_DefaultDesign;		// Design to make when not making anything else.
	QList<AuMakeDesign>					m_DesignQueue;			// Designs waiting to be manufactured.
	int									m_DesignTries;			// Number of times we have tried loading the design.
	quint8								m_MakeMinQ;				// Minimum production Q.
	AuMakeDesign						m_MakingShip;			// Design of ship currently under construction, possibly ready.
	AuElapsedTimer						m_tmLastSpawnTry;

private:	// Fire Station
	mutable QReadWriteLock				m_FireStationLock;
	AuTime								m_FireStationCalled;	// When last called to fight a fire.

private:	// Fence or Wall
	mutable QReadWriteLock				m_FenceLock;
	AuElapsedTimer						m_FenceChangeTime;
	mutable bool						m_FenceExtentsValid;	// True if fence extents are valid.
	mutable AuPoint						m_FenceMax;				// Extents of segments adjusted to terrain.
	mutable AuPoint						m_FenceMin;				// Extents of segments adjusted to terrain.
	AuSegmentArray						m_FenceSegs;
	mutable QMap<quint32, AuSegmentArray>	m_FenceSegsCache;		// Current fence segments cache.
	mutable bool						m_FenceSegsCacheValid;
	quint8								m_FenceStyle;			// AuScFence::STYLE

private:
	mutable QReadWriteLock				m_JobsLock;
	AuJobInterdictSystem				m_JobInterdict;			// Will become valid if we detect enemy ship transmissions.
	unordered_map<AuId, AuJobDestroyEnemyStation, AuIdHash>	m_JobsEnemyStations;	// Key is station id.

private:
	mutable QReadWriteLock				m_PriceOfferLock;
	QMap<DeInvItemId, AuPriceOffer>		m_PriceOffers;			// Memory of prices offered by the BuyCost and SellCost functions.
																// Price offers are NOT keyed by the id of the person or officer
																// who requested them. Otherwise two people can work together to
																// take advantage of this.

private:
	// Transient population exists where passengers can be unloaded. They move from here to become residents when possible.
	mutable QReadWriteLock				m_TransientLock;
	QMap<AuSpecies, quint32>			m_TransientPopulation;

private:
	AuBdRender*							m_AV;					// Audio and visual rendering parameters.

public:	// Constructors
	// 'iid' is only needed when setting up to render a cursor.
	DaBd();
	virtual ~DaBd();

protected:
	DaBd(const DaBd& rhs);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	inline double AccountBalance() const
	{
		m_AccountLock.lockForRead();
		double v = m_Bank.BalanceEnding();
		m_AccountLock.unlock();
		return v;
	}

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual void ActivateTransporterSystem(int troomIndex);

	virtual bool ActiveTransporterPadLocation(quint16 which, AuPoint& padLoc) const;

	virtual void AddCargo(const AuItem& iNew);

	bool AddCommodity(const AuItem& iNew);

	virtual double AdjustAltMSLForDevelopments(const DaWorldLL& world, double terAltMSL, const AuGlobeLL& ll) const;

	virtual double AdjustAltMSLForDevelopments(const DaWorldLL& world, double terAltMSL, const AuGlobeLL& ll, bool& hideRocks,
			int& hideGrass, bool& hideShrubs, bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub,
			double& farmShrubMSL, COMMODITYID& orchardTree, double& orchardTreeMSL) const;

	// Reduces the count in 'iNew' by the amount that is stored.
	void AgentAddCommodity(AuItem& iNew);

	quint32 AirportCryoPassengers(const DaWorld* world, bool requestorInCoC) const;
	quint32 BaseCryoPassengers(const DaWorld* world, bool requestorInCoC) const;

	// Removes citizens from buildings accessible to this airport by ground that are part of my empire.
	// Fills 'speciesRemoved' with DNA from each build that people are taken from. May be less than 'howMany' on return.
	// Caller must delete the 'speciesRemoved' pointers.
	void AirportDecreasePassengers(DaWorld* world, quint32 howMany, QList<AuSpecies>& speciesRemoved);
	void BaseDecreasePassengers(DaWorld* world, quint32 howMany, QList<AuSpecies>& speciesRemoved);

	// Returns the number of citizens ready to depart as passengers.
	quint32 AirportPassengers(const DaWorld* world, bool requestorInCoC) const;

	virtual float AirPressure(const QString& roomName) const;

	// Returns Z rotation needed to align a building at 'll' with the edge of this development.
	// Returns 99 when no alignment is defined/possible.
	virtual float AlignmentRot(const DaWorldLL& llworld, const AuGlobeLLF& ll) const;

	virtual float AmbientLightEmitted(const AuGlobeLLF& ll) const;

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	virtual bool AnyDoorJammed(AuPoint& where) const;

	virtual bool AnyHullDoorOpen() const;

	virtual bool AnyHullDoorOpenInRoom(const QString& roomName) const;

	virtual bool AnyHullDoorUnlocked() const;

	virtual float ArmorLevel() const;

	quint32 Auditoriums() const;

	// Returns the total amount of funds this bank can loan to a player.
	// Tallies money throughout the entire city.
	// Only a bank will return a non-zero number.
	double BankLoanFundsAvailable() const;

	bool BayWeaponHasAmmo(quint16 whichBay) const;

	// Returns true if any bay weapon is locked on to 'targetId'.
	bool BayWeaponLocked(const AuId& targetId) const;

	virtual float BayWeaponPower(float unitsFired, quint16 which) const;

	inline quint8 BayWeaponPowerSetting(quint16 which) const
	{
		quint8 s;

		m_BayLock.lockForRead();
		unordered_map<quint16, AuBaySetting>::const_iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			s = (*bsi).second.m_Setting;
		else
			s = 40;			// 100% power is the default. This will be the level used by the AI unless set otherwise.
		m_BayLock.unlock();

		return s;
	}

	virtual double BayWeaponRange(quint16 which) const;

	inline AuId BayWeaponTarget(quint16 which) const
	{
		AuId target;

		m_BayLock.lockForRead();
		unordered_map<quint16, AuBaySetting>::const_iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			target = (*bsi).second.m_TargetId;
		m_BayLock.unlock();

		return target;
	}

	virtual float BayWeaponUnits(quint16 which) const;

	virtual AuPoint BerthLocation(const AuId& id) const
	{
		Q_UNUSED(id);
		return AuPoint(0,0);
	}

	// Returns the location of someone's berth. 'roomNumber' is an output parameter.
	virtual AuPoint BerthLocation(const AuId& id, quint16& roomNumber) const
	{
		Q_UNUSED(id);
		roomNumber = 0;
		return AuPoint(0,0);
	}

	virtual AuRosterEntry::TYPE BerthTypeOccupied(const AuId& id) const
	{
		Q_UNUSED(id);
		return AuRosterEntry::Invalid;
	}

	// Returns my site locations that are used to verify placement within a build zone.
	void BuildZoneCheckLocs(AuPointArray& bzLocs) const;

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

	// Returns true if 'ship' is on the ground at a bd of my empire, or on a geo road. The building or road location must be in my
	// empire. The building site containing 'ship' must be accessible by ground connection to this building.
	bool CanAccessByGround(const DaVessel* ship) const;

	virtual bool CanAddCargo(const AuItem& iNew) const;

	virtual bool CanBeSelected() const;

	void CancelWeaponBayFired(quint16 which);

	virtual bool CanEnterConsole(const DaPerson& who, quint16& stationId, AuStation& station, quint16& which) const;

	virtual bool CanGrabLadder(const DaAtom& who, const AuPoint& loc, AuMatrix4x4& mLadder);

	virtual bool CanGrabLadderAt(const DaAtom& who, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder);

	bool CanLoadTurretAmmunition(const AuItem& item, quint16 whichTurret) const;

	virtual bool CanOpenDoor(const DaSentient& who, unsigned int doorIndex) const;

	bool CanSelectBayWeaponAmmunition(const AuItem& item, quint16 whichBay) const;

	virtual bool CanSetInMotion() const;

	// Test to see if a person from 'empireId' can spawn at this building.
	// Returns 0=yes, 1=no enemy, 2=no not normal
	int CanSpawnHere(quint32 empireId) const;

	virtual float CapacitorLevel() const
	{
		m_VesselLock.lockForRead();
		float v = m_CapacitorLevel;
		m_VesselLock.unlock();
		return v;
	}

	virtual float CapacitorMax() const;

	// Returns the id of my capitol building, if I am part of a city. 0 if I am not part of a city.
	inline AuId CapitolId() const
	{
		// Capitol building is the capitol of its own city, by definition.
		if(IndustryId() == IID_Capitol)
			return Id();
		m_BdLock.lockForRead();
		AuId v = m_CapitolId;
		m_BdLock.unlock();
		return v;
	}

	virtual unsigned int CargoCapacityVolume() const;

	virtual bool CargoHas(COMMODITYID cid) const
	{
		m_CargoLock.lockForRead();
		bool v = m_Cargo.contains(cid);
		m_CargoLock.unlock();
		return v;
	}

	inline quint32 CargoRawCount(COMMODITYID cid, quint8 minQ) const
	{
		m_CargoLock.lockForRead();
		quint32 v = m_Cargo.RawCount(cid, minQ);
		m_CargoLock.unlock();
		return v;
	}

	// Total volume is calculated a bit oddly. Containers count as their pack size x their count. Other items count as their count.
	inline unsigned int CargoTotalVolume() const
	{
		m_CargoLock.lockForRead();
		unsigned int v = m_Cargo.TotalCargoVolume();
		m_CargoLock.unlock();
		return v;
	}

	virtual double CeilingHt(const AuPoint& loc, const AuPoint& gravityN, bool isLocInside) const;

	inline void ChargeCapacitor(float units)
	{
		float capMax = CapacitorMax();
		m_VesselLock.lockForWrite();
		if(m_CapacitorLevel < capMax)
		{
			m_CapacitorLevel += units;
			if(m_CapacitorLevel > capMax)
				m_CapacitorLevel = capMax;
		}
		m_VesselLock.unlock();
	}

	// Checks to see if 'unit' is granted access when calling for a beam up, recovery system, opening a hull door, etc.
	// Access does not override door locks.
	virtual bool CheckAccess(const DaUnit* unit) const;

	virtual bool CheckChildDeparture(const AuPoint& oldLoc, const AuPoint& nuLoc, const DaAtom* who) const;

	virtual bool CheckChildExit(const AuPoint &oldloc, const AuPoint &nuloc, const DaAtom* who) const;

	virtual bool CheckPeerJoin(const AuPoint &oldloc, const AuPoint &nuloc, const DaAtom* who);

	// Checks for trespassers.
	void CheckTrespass(const DaSolarSystem* system, float sensorRange);

	void CitizenDied(quint32 howMany);
	void TroopDied(quint32 howMany);
	void WorkerDied(DaWorld* world, quint32 howMany);

	// 'person' is the person triggering the encounter. The person is known to be a direct child of this bd atom.
	// 'existingWorkers' contains DaNpcs outside the building that could be used.
	// Returns true if an encounter occurs.
	bool CitizenEncounter(const DaPerson& person, DaAtomList& existingWorkers);

	virtual void CitizenPosts(AuScPartSet& posts) const;

	// Returns my name, prefixed with my city name if I have one.
	QString CityBuildingName() const;

	// Called at 13 minute intervals.
	void CityCycle();

	// Called on every building in the city|base after a regularly scheduled city cycle, to reset values that get tracked between
	// reports. 'morale' is the morale of the city at the conclusion of the report.
	void CityCycleReset(qint8 morale, bool hasAir);

	// Civilian buildings return the name of the city assigned at their capitol building.
	// Military buildings return the name of the base assigned at their military hq building.
	// Returns blank if I am not part of a city.
	QString CityName() const;

	// Returns my city name. If I am not in a city, then it returns my Name(), which returns my assigned name, unless my assigned
	// name is blank; then Name() returns my industry name.
	QString CityOrBuildingName() const;

	virtual void ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire);

	virtual QString CommName() const;

	virtual double CommunityAccountBalance() const;

	// Calls BaseAdd or BrokerAdd depending on what I am.
	virtual void CommunityAddCommodity(const AuItem& iNew);

	// Fills 'terminals' with all air terminals in my community.
	void CommunityAirTerminals(DaAtomList& terminals) const;

	// Consumes up to count commodities.
	// Returns the count actually consumed.
	quint32 CommunityConsumeSpareParts(COMMODITYID cid, unsigned int techLevel, int count);

	virtual void CommunityCreditAccount(double amount, AuBank::CATEGORY cat);

	virtual void CommunityDebitAccount(double amount, AuBank::CATEGORY cat);

	// Eats food for one meal cycle.
	// Returns true if food supply was adequate.
	// 'consumedLiquor' is true if any liquor is consumed while eating food.
	bool CommunityEatFood(ECOROLE workerEcoRole, quint32 foodToEat, bool& consumedLiquor);

	// Returns true if my community has an airport repair shop.
	bool CommunityHasRepairShop() const;

	// Increases the crew population by one.
	// 'sourceName' is used for outbreak event msg.
	void CommunityIncreaseCrew(const DaCreature* creature, const QString& sourceName);

	// Increases the population by one.
	// Adds one citizen to a building accessible to this airport by ground that are part of my empire.
	bool CommunityIncreasePopulation(const AuSpecies& species);

	// Increases the population by one.
	// 'sourceName' is used for outbreak event msg.
	bool CommunityIncreasePopulation(const DaCreature* creature, const QString& sourceName);

	// Increases the troop population by one.
	// 'sourceName' is used for outbreak event msg.
	void CommunityIncreaseTroops(const DaCreature* creature, const QString& sourceName);

	// Fills 'items' with all cargo available to this building, tallied from all ground connection in my community.
	// 'empireAsking' is used to check commerce restrictions.
	virtual void CommunityInventoryAvailable(AuItemHash& items, quint32 empireAsking) const;

	void CommunityPersonDied(quint32 howMany);

	// Includes transients.
	quint32 CommunityPopulation() const;

	quint32 CommunityRawCount(COMMODITYID cid, quint8 minQ) const;

	// Returns the actual count removed.
	quint32 CommunityRawRemove(COMMODITYID cid, quint8 minQ, quint32 howMany);

	void CommunityRemoveInventory(const AuItemHash& items);

	quint16 AirportRepairShopWorkers(const quint16 workersWanted) const;
	quint16 BaseRepairShopWorkers(const quint16 workersWanted) const;
	quint16 CommunityRepairShopWorkers(const quint16 workersWanted) const;

	virtual void CommunityRemoveInventory(const AuItem& items);

	// Returns the count of the offered commodity that will fit in inventory.
	// Returns 0 if none of the offered commodity will fit.
	// Never returns a number greater than the current offer.Count().
	// Calls BaseTest or BrokerTest depending on what I am.
	virtual quint32 CommunityTestFitCommodity(const AuItem& offer) const;

	virtual bool ContainsEnemyUnit(quint32 empireId) const;

	// Returns true if this building's design could potentially be occupied, depending on which industry it gets used for.
	// The return result is not based on this building's industry. This function is used in the design preview to light buildings
	// that might be occupied in the game universe.
	bool CouldBeOccupied() const;

	// Counts dormant ships that have been mothballed in this hangar.
	quint32 CountShipsInHangar();

	void Create();

	virtual void CreateRoadMap(const DaWorldLL& world, AuRoadMap& roadMap) const;

	void Demolish();

	void DeployTroopsToStation(DaVessel* station);

	virtual void DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc);

	// 'foodToEat' is in food nutrition value units, not food commodity count.
	// 'liquorToDrink' is in intoxication units, not liquor commodity count.
	// Returns the amount of food eaten. Sets 'liquorDrank' the the intox value of any that is consumed.
	quint32 EatMeal(quint32 foodToEat, ECOROLE workerEcoRole, quint32 liquorToDrink, quint32& liquorDrank);

	virtual void EmpireDeleted(quint32 empireId);

	quint16 EmptyHomes() const;

	// Configures 'enc' with information about this enclosure. Returns true on success.
	// Assumes caller has already checked HasEnclosedEnvironment().
	// All valid biodome designs have at least one environment void. Platforms may have them but they are not required.
	// Returns: 0=success, 1=my design is not ready, 2=I have no environment voids.
	int Enclosure(const DaWorldLL& world, AuWorldEnclosure& enc) const;

	// Test to see if this building's environment voids contains a proposed building at world transformation 'mBld'.
	// 'siteBoxRad' is in the space of the proposed building's design.
	// Returns false if IsEnclosure() returns false.
	// Returns false if my design data cache is not valid.
	bool EnclosureContainsCompletely(const DaWorldLL& llworld, const AuMatrix4x4& mBld, const AuPoint& siteBoxRad) const;

	// Returns the estimated cost of repairing as much of the ship as possible at this city.
	unsigned int EstimateRepairCostPossible(const DaVessel* ship, quint16& repairSecs, quint8 repairQ) const;

	unsigned int EstimateRepairCostPossible(const DaVessel* ship, SYSTEM system, quint16 which, quint16& repairSecs,
			quint8 repairQ) const;

	virtual quint32 ExperiencePointsForKilling() const;

	void ExploreWorld();

	void ExportToStation(DaVessel* station);

	// Adjusts m_FenceSegs to the current terrain. Also updates fence extents.
	void FenceAdjust();

	// Returns the 3D extents of m_FenceSegs.
	void FenceExtents(AuPoint& min, AuPoint& max) const;

	// Fills 'fenceSegs' with my fence segments, in my space.
	// Gets fence segments from my m_FenceSegs array and also from my blueprint, if I have one and I am not under construction.
	// Returns false if my design data is not available.
	bool FenceSegments(AuSegmentArray& fenceSegs) const;

	// Fills 'fenceSegs' with my fence segments, transformed to the space of my parent world.
	// Key is AuScFence::STYLE or a design id for custom fences, which is always > AuScFence::STYLE_Count.
	// Gets fence segments from my m_FenceSegs array and also from my blueprint, if I have one and I am not under construction.
	// Returns false if my design data is not available.
	bool FenceSegments(QHash<quint32, AuSegmentArray>& fenceSegs) const;

	// Return value is an AuScFence::STYLE.
	inline int FenceStyle() const
	{
		m_FenceLock.lockForRead();
		int fenceStyle = m_FenceStyle;
		m_FenceLock.unlock();
		return fenceStyle;
	}

	virtual void FindBdRecursive(DaAtomList& bds) const;

	virtual void FindBdRecursive(DaAtomList& bds, quint32 empireId) const;

	virtual void FindCitiesBdRecursive(DaAtomList& cities) const;

	virtual void FindCitiesBdRecursive(DaAtomList& cities, quint32 empireId) const;

	virtual void FindCitiesBdRecursive(DaAtomList& supplyCities, DaAtomList& repairCities, DaAtomList& troopCities,
			quint32 empireId);

	virtual const DaAtom* FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
			quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
			double& bestDistance) const;

	virtual void FindClosestShipLockOnRecursive(int scannerDomain, const AuId& scannerWorldId, quint32 scannerEmpire,
			const AuId& scannerCapsuleId, const AuPoint& scannerSysLoc, float sensorRange,
			const DaVessel*& enemyVessel, double& vesselRange,
			const DaVessel*& nonEnemyStation, double& stationRange,
			const DaBd*& enemyMilBd, double& milRange,
			const DaBd*& anyBd, double& bdRange) const;

	virtual void FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
			SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
			double& targetRange) const;

	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	virtual void FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// Adds hangared spacecraft to the list that are subordinate to this hangar. Not recursive.
	void FindHangaredSpaceships(DaAtomList& spaceships) const;

	DaVehicle* FindNewVehicle(COMMODITYID vehicleCID);

	DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher);

	virtual DaBd* FindParentBdInEnv();

	virtual const DaBd* FindParentBdInEnv() const;

	// Search for a platform location that contains pxFind.
	// 'tModel' already includes 'this' atom's transform.
	DaBd* FindPlatformLocationAtPixel(const QPoint& pxFind, const AuMatrix4x4& tDevicePersp, const AuMatrix4x4& tDeviceOrtho,
			const AuMatrix4x4& tModel, AuPoint& platformLoc) const;

	// Locates a dormant ship that has been mothballed in this hangar by 'personId'.
	DaVessel* FindShipInHangar(const AuId& personId);

	// Called by stations and cities.
	// Searches recursively for any trespassing cities or stations.
	const DaAtom* FindTrespasserRecursive(quint32 empireId, const AuPoint& systemLoc, float sensorRange) const;

	// Returns the probability of catching fire when damaged by incendiary weapons.
	int FireProbability() const;

	inline AuId FounderId() const
	{
		m_BdLock.lockForRead();
		AuId v(m_FounderId);
		m_BdLock.unlock();
		return v;
	}

	inline QString FounderName() const
	{
		m_BdLock.lockForRead();
		QString v(m_FounderName);
		m_BdLock.unlock();
		return v;
	}

	// This produces a tally of all money in the city. Just in this building if not part of a city.
	// Tallies money throughout the entire city.
	// Only a bank will return a non-zero number.
	double GovtAccountBalance() const;

	virtual int GunnerDomain(AuId& worldId) const;

	// A description of this city as told by an indigenous. Complete sentences.
	QString IndigenousDescription() const;

	virtual bool IsCivilian() const
	{
		return !IsMilitary();
	}

	virtual bool IsEthereal() const
	{
		return false;
	}

	// Returns true if this building HasEnvironmentEnclosure in a harsh environment, which means under water or unbreathable atmo.
	bool IsHarshEnvironmentEnclosure() const;

	// Returns the jurisdiction radius of this police station or capitol, in feet.
	// Returns 0 if this is not a police station or capitol.
	double JurisdictionRadius() const;

	virtual void KilledByUnit(const DaUnit* killer);

	// Varies based on presence of power.
	float LightAttenuationDistance() const;

	// Varies based on presence of power.
	QColor LightColor() const;

	// Varies based on presence of power.
	float LightkQ() const;

	virtual void LivestockPosts(AuScPartSet& posts) const;

	// Calls g_Au.LockScDesignsForRead.
	// Returns 0 if my design is not found.
	// Must call UnlockDesign even if 0 is returned.
	const AuBdDesign* LockDesign() const;

	inline const QHash<COMMODITYID, quint32>& LockRocksOnSite()
	{
		m_BdLock.lockForRead();
		return m_RocksOnSite;
	}

	inline void UnlockRocksOnSite()
	{
		m_BdLock.unlock();
	}

	// Add an event to this city's event log.
	inline void LogEvent(AuEventLogEntry::TYPE type)
	{
		LogEvent(AuEventLogEntry(type));
	}

	void LogEvent(const AuEventLogEntry& le);

	// Add an event to this city's event log.
	virtual void LogEvent(AuEventLogEntry::TYPE type, const QString& text, const AuId& atomId=AuId());

	// Mails the last city report to someone.
	void MailLastReport(const AuId& toId) const;

	quint16 MedicalStationInRoom(quint16 roomId) const;

	virtual bool MoonOrbitRingsVisible(const QString& roomName) const;

	virtual quint64 MemoryUsage() const;

	quint32 Offices() const;

	inline bool OnPlatform() const
	{
		//return !PlatformId().IsNull();
		return !m_PlatformId.IsNull();
	}

	virtual bool OrbitRingsVisible(const QString& roomName) const;

	virtual void PerformSensorConsoleScan(AuSensorBlipList& sensorBlips) const;

	virtual void PerformTransporterConsoleScan(AuSensorBlipList& sensorBlips, bool showAboard) const;

	inline AuId PlatformId() const
	{
		m_BdLock.lockForRead();
		AuId v(m_PlatformId);
		m_BdLock.unlock();
		return v;
	}

	void ProcessConstructionDemandTally(const AuItem& item, unsigned int& needCount) const;

	void ProcessDemandTally(const AuItem& item, unsigned int& stock, unsigned int& population, unsigned int& needCount,
			unsigned int& produceCount) const;

	virtual float RelativeAudioGain(const DaPerson* sceneAtom) const;

	// Removes a unit of fuel. Used when servicing vehicles.
	virtual bool RemoveFuel(COMMODITYID fuelCID);

	// Removes resident population from this building.
	// Returns the number of resident removed.
	quint32 RemoveIndigenous(quint32 howMany);	// People move to the wilderness.
	quint32 RemoveSeafarers(quint32 howMany);	// People depart by sea.
	quint32 RemoveTravellers(quint32 howMany);	// People depart by air|space.
	quint32 RemoveWanderers(quint32 howMany);	// People depart by land.

	// Sets fields in 'report' for reporting current city conditions.
	void ReportCurrent(AuCityReport& report, bool resetBankActivity) const;

	void ReportEventLog(AuEventLog& events) const;

	// Should only be called on the city capitol.
	bool ReportHistory(AuCityReportHistory& report) const;

	// Should only be called on the city capitol.
	bool ReportLast(AuCityReport& report) const;

	// Should only be called on the city capitol.
	bool ReportNextToLast(AuCityReport& report) const;

	// Returns true if I can account for the resident changing jobs, by having one that was employed at the 'fromBdId'.
	inline bool ResidentChangedJobs(const AuId& fromBdId, const AuId& toBdId)
	{
		bool changed;
		m_BdLock.lockForWrite();
		QHash<AuId, quint16>::iterator ei = m_ResidentsEmployed.find(fromBdId);
		if(ei != m_ResidentsEmployed.end())
		{
			// We have a resident employed at 'fromBdId'. Take one of them.
			if((*ei) > 1)
				(*ei)--;
			else
				m_ResidentsEmployed.erase(ei);					// It was the only one employed there.

			// Employ the resident at 'toBdId'.
			ei = m_ResidentsEmployed.find(toBdId);
			if(ei != m_ResidentsEmployed.end())
				(*ei)++;
			else
				m_ResidentsEmployed.insert(toBdId, 1);
			changed = true;
		}
		else
			changed = false;
		m_BdLock.unlock();
		if(changed)
			SetModified();
		return changed;
	}

	inline ECOROLE ResidentEcoRole() const
	{
		return m_ResidentEcoRole;
	}

	AuSpecies ResidentSpecies() const;

	inline quint16 Residents() const
	{
		m_BdLock.lockForRead();
		quint16 v = m_Residents;
		m_BdLock.unlock();
		return v;
	}

	inline quint16 ResidentsEmployed() const
	{
		quint16 v=0;
		m_BdLock.lockForRead();
		for(QHash<AuId, quint16>::const_iterator ei = m_ResidentsEmployed.cbegin(); ei != m_ResidentsEmployed.cend(); ei++)
			v += *ei;
		m_BdLock.unlock();
		return v;
	}

	quint16 ResidentsReadyToDepart(bool requestorInCoC) const;

	inline quint16 ResidentsUnemployed() const
	{
		return Residents() - ResidentsEmployed();
	}

	virtual void RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup=true);

	void SalesmanCalled(const DaPerson& person);

	void SeafarerMovedIn(const AuSpecies& species);

	virtual quint32 SellCost(const AuId& idToWho, const AuItem& item);

	void SendCityFacilities();

	void SetCapitol(const AuId& capitolId, quint32 capitolEmpire);

	inline void SetCityName(const QString& nuName)
	{
		Q_ASSERT(IndustryId() == IID_Capitol || IndustryId() == IID_BaseHeadquarters);
		m_BdLock.lockForWrite();
		if(m_CityName != nuName)
		{
			m_CityName = nuName;
			m_tmCapitolChange.Set();
			m_BdLock.unlock();
			SetModified();
		}
		else
			m_BdLock.unlock();
	}

	// 'fenceStyle' is an AuScFence::STYLE.
	void SetFence(int fenceStyle, const AuSegmentArray& fenceSegs);

	inline void SetFounder(const AuId& id, const QString& name)
	{
		m_BdLock.lockForWrite();
		m_FounderId = id;
		m_FounderName = name;
		m_BdLock.unlock();
	}

	inline void SetIndustry(INDUSTRYID iid)
	{
		m_IndustryId = iid;
	}

	virtual void SetModified(SET_MODIFIED_BITS notifyBits=NOTIFY_All);

	virtual bool SetMoonOrbitRingsVisible(const QString& roomName, bool visible);

	virtual bool SetOrbitRingsVisible(const QString& roomName, bool visible);

	inline void SetPlatformId(const AuId& platformId)
	{
		m_BdLock.lockForWrite();
		m_PlatformId = platformId;
		m_BdLock.unlock();
	}

	inline void SetStoreCommodity(COMMODITYID storeCID)
	{
		m_CargoLock.lockForWrite();
		m_StoreCommodity = storeCID;
		m_PatentsChanged.Set();
		m_CargoLock.unlock();
		SetModified();
	}

	virtual bool SetSystemNamesVisible(const QString& roomName, int visible);

	void SetTroopsPresent(int troops);

	virtual bool SetWormholeLinesVisible(const QString& roomName, bool visible);

	// Fills 'parkMs' with locations at which spacecraft can be spawned by a spacecraft factory. Return locations are relative
	// to my design.
	void SpacecraftParkingSpots(QList<AuMatrix4x4>& parkMs) const;
	void WaterVehicleParkingSpots(QList<AuMatrix4x4>& parkMs) const;

	// See code for return values.
	int SpawnTroop();

	// See code for return values.
	int SpawnWorker(const DaWorldLL* llworld, DaAtomList& existingWorkers);

	virtual bool Station(quint16 partId, AuStation& station) const;

	virtual bool Station(const DaCreature& who, AuStation& station) const;

	inline COMMODITYID StoreCommodity() const
	{
		m_CargoLock.lockForRead();
		COMMODITYID v = (COMMODITYID)m_StoreCommodity;
		m_CargoLock.unlock();
		return v;
	}

	// Capitol and HQ surrender all their buildings. Otherwise just calls SurrenderBuilding.
	// 'byWho' is the id of the avatar surrendering this bd. It must match the founder id or the founder id must be null.
	void Surrender(const AuId& byWho, quint32 surrenderingEmpire, quint32 toEmpire);

	void SurveySite(const DaWorldLL& world, QHash<COMMODITYID, quint32>& rocks);

	// Performs site survey on mines and wells that have no information in m_RocksOnSite.
	void SurveySiteUpdate();

	virtual int SystemNamesVisible(const QString& roomName) const;

	quint32 TradeConnectionsByAir() const;

	quint32 TradeConnectionsByLand() const;

	quint32 TradeConnectionsBySea() const;

	// Increases the population by one.
	// Adds one citizen to a building accessible to this airport by ground that are part of my empire.
	// If no building is available, the citizen is added to the transient population.
	void TransientIncreasePopulation(const AuSpecies& species);

	// Increases the population by one.
	// 'sourceName' is used for outbreak event msg.
	// If no building is available, the citizen is added to the transient population.
	void TransientIncreasePopulation(const DaCreature* creature, const QString& sourceName);

	quint16 TransporterStationInRoom(quint16 roomId) const;

	void TravellerMovedIn(const AuSpecies& species);

	void TravellerMovedIn(const DaCreature* creature, const QString& sourceName);

	virtual void TroopPosts(AuScPartSet& posts) const;

	// Calls g_Au.UnlockScDesigns.
	void UnlockDesign() const;

	void WandererMovedIn(const AuSpecies& species);

	inline quint16 Workers() const
	{
		m_BdLock.lockForRead();
		quint16 v = m_Workers;
		m_BdLock.unlock();
		return v;
	}

	virtual void WorldCaptured(quint32 byEmpire);

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

	virtual void CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, AuCombatResult& result);

	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	// Returns my location as briefly as possible in a single line suitable for use in a comm message.
	QString CommMessageLocation() const;

	// Gets a current tally of commodities I can supply.
	virtual void CommoditySupplies(AuCommodityExchange& cex) const;

	void CompleteConstruction();

	virtual void ConfigureHarvester();

	virtual bool ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const;

	void ConstructionFetch();

	void ConstructionGo(DaPerson* person);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	void CreditAccount(double amount, AuBank::CATEGORY cat);

	void CreditAccountCitizenSalary(double amount);

	void CreditAccountCitizenSales(double amount);

	// Deposit into government account by a player.
	// Credits the Govt account.
	void CreditAccountGovtDeposit(double amount);
	void CreditAccountGovtTaxLevy(double amount);
	void CreditAccountGovtSubsidyTax(double amount);

	virtual bool CrewDoorLocked(unsigned int doorIndex) const;

	// Complete the firing sequence of weapon bays.
	void CycleBayWeapons();

	virtual void CycleCapacitor();

	virtual void DamageAndDestroy(const QString& reason);

	virtual void DamageReportHp(AuDamage& damage) const;

	virtual void DamageReportPercent(AuDamage& damage, int structEquip) const;

	inline void DamageStateChanged()
	{
		m_SystemsLock.lockForWrite();
		if(UpdateSystemsHealthSystemsLocked())
			UpdateSystemsPowerSystemsLocked();
		m_SystemsLock.unlock();

		m_DamageLock.lockForWrite();
		m_tmDamageChange.Set();
		m_DamageLock.unlock();

		SetModified();
	}

	virtual void DeactivateTransporterSystems();

	void DebitAccount(double amount, AuBank::CATEGORY cat);

	// Takes from the entire city.
	void DebitAccountCitizenLoans(double amount);

	void DebitAccountCitizenPurchases(double amount);

	void DebitAccountCitizenSalary(double amount);

	void DebitAccountCitizenWithdrawal(double amount);

	void DebitAccountGovtRewards(double amount);

	void DebitAccountGovtTribute(double amount);

	// Withdrawal from government account by a player. Debits the Govt account.
	void DebitAccountGovtWithdrawal(double amount);

	// Returns true when 'which' bay is no longer attached.
	// Calls CancelWeaponBayFired when no longer active.
	bool DecayTractorBay(quint16 which);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

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

	bool DestroyCargo(unsigned int unitsDamage);

	bool DestroyFuel(unsigned int unitsDamage);

	// Copied from DaVessel. Destroys this building.
	void DestroyShip(const QString& reason);

	COMMODITYID DestroyVehicle();

	virtual void DischargeCapacitor(float units);

	double Distance(const AuPoint& loc) const;

	bool DoesManufacture() const;

	// Returns false if no residents remain to be employed anywhere.
	bool EmployResident(const AuId& atBdId);

	void EnableConstructionWorker(bool enable);

	void EnableManufacturingWorkerMPID(quint16 mpid, quint32 howMany);
	void EnableSpacecraftFactoryWorker(quint32 howMany);

	virtual AuPoint EntranceLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround,
			bool hullDoorsUsable) const;

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual AuPoint ExitLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround) const;

	void FetchVehicle();

	void FilterDestinationLoc(DaAtom* moveAtom, const AuPoint& oldLoc, AuPoint& nuLoc) const;

	virtual DaCreature* FindPatient(const DaSentient& doctor) const;

	// 'unitsFired' is the number of weapon bay units to fire.
	// Returns 'true' if the weapon is fired.
	bool FireBayWeapon(quint16 whichBay, const AuPoint& emitLoc, float unitsFired, const DaAtom* targetAtom,
			const AuPoint& targetSysLoc);

	// Fire department will return number of fire fighters when it is ready to respond to a fire call.
	// Changes its respond time so it will not be ready again until later, a few minutes.
	quint32 FireFighterResponse();

	virtual double FireRadius() const;

	// Returns 'true' if the turret is fired.
	bool FireTurret(DaAtom* system, quint16 which, COMMODITYID module, const AuMatrix4x4& mTur, DaAtom* sightedAtom,
			const AuPoint& sightedLoc, bool sightedShield);

	virtual bool FloorHt(const AuPoint& loc, const AuPoint& gravityN, double& htAbvFloor, bool& inGravityWell, bool& onStairs,
			AuPoint& stairNormal, bool isLocInside, const DaAtom* forWho) const;

	virtual AuPoint FloorLocInsideDoor(const AuPoint& doorLoc) const;

	// Returns a user interface created by the current game, enabling the person to configure and submit their next turn when
	// it is their turn.
	virtual AuDialog* GameCreateTurnUI(quint16 whichTable, const AuId& personsTurn, QWidget* parent);

	// Exits a player from any game they're involved in. Terminates the game if it's the last person to leave.
	virtual void GameExitPlayer(quint16 whichTable, const AuId& playerId);

	// Returns the icon for the game in play at a table.
	QIcon GameIcon(quint16 whichTable) const;

	// Returns the name of the game in play at a table.
	QString GameName(quint16 whichTable) const;

	// Selects a nu game for a game table, if there is no game currently active at that table.
	virtual bool GameSelected(quint16 whichTable, quint8 gameType, const AuId& dealerId);

	// Sets the players in a game according to who is at the same table as the dealer and their seating order.
	// Can be called on the client.
	virtual void GameSetPlayers(quint16 whichTable, AuGame* game, const AuId& dealerId) const;

	// Starts the game at a table. Game settings are sent in a byte array so they are generic between all game types.
	virtual void GameStart(DaPerson* dealer, quint8 gameType, const QByteArray& gameSettings);

	// NOTE: Expects m_GamesLock to be locked.
	inline void GamesSetModified()
	{
		m_GamesChanged.Set();
		SetModified();
	}

	// Games only get ticked every even second. No table top games need faster response than that.
	// 'validatePlayers' is true to verify all players are still present and in the game.
	void GamesTick(bool validatePlayers);

	// 'stationId' is the id of a game table. 'tableM' is filled with the position matrix for the table.
	// false is returned if the game table is not found.
	virtual bool GameTableLocation(quint16 stationId, AuMatrix4x4& tableM) const;

	// Sends the game turn info when a player executes their turn.
	// Game turn data is sent in a byte array so it is generic between all game types.
	virtual void GameTurn(DaPerson* person, quint8 gameType, const QByteArray& gameTurn);

	// Checks to see if the game turn UI is still appropriate for the current game state.
	// Returns false if the turnUI should be deleted.
	virtual bool GameTurnUIValid(quint16 whichTable, const AuId& personsTurn, AuDialog* turnUI);

	// Returns the AuGame::GameType() in play at a game table.
	quint8 GameType(quint16 whichTable) const;

	quint32 Generators() const;

	bool GetNextWeaponBayFired(AuBayFired& bayFired, quint16& which);

	// Returns the Current Power Consumption for the indicated system
	// Returns 0 on unknown system
	inline unsigned int GetPowerConsumption(SYSTEM q8SystemType) const
	{
		m_SystemsLock.lockForRead();
		unsigned int v = m_ssaShipSystems.GetPowerConsumption(q8SystemType);
		m_SystemsLock.unlock();
		return v;
	}

	// Returns the Current Power Consumption for the indicated system
	// Returns 0 on unknown system
	inline unsigned int GetPowerConsumption(SYSTEM q8SystemType, quint16 which) const
	{
		m_SystemsLock.lockForRead();
		unsigned int v = m_ssaShipSystems.GetPowerConsumption(q8SystemType, which);
		m_SystemsLock.unlock();
		return v;
	}

	inline unsigned int GetPowerConsumption(const AuSysId& sysId) const
	{
		if(sysId.Which())
			return GetPowerConsumption(sysId.System(), sysId.Which());
		return GetPowerConsumption(sysId.System());
	}

	virtual double Gravity(double cgDistance) const;

	virtual float GroundHeightAboveCG() const;

	void Harvest(quint16 which, COMMODITYID bayModule, double wpnRange, double targDist, int probabilityToHarvest,
			COMMODITYID harvestCID, quint8 harvestQ);

	virtual float HarvesterBayRange(float unitsFired, quint16 which) const;

	inline bool HasAirportAccess() const
	{
		return m_Airport;
	}

	// Returns true for buildings that actually create an enclosed environment using environment voids. This is not the same as
	// having a simple room with life support. Environment voids permit building construction and plant growth.
	// 1. My structure must be a biodome or platform.
	// 2. I must not be under construction.
	// 3. I must have a life support system. Life support is always considered to be functioning, regardless of power or damage.
	// 4. I must have at least one environment void. All valid biodome designs have at least one environment void. Platforms may
	//    have them but they are not required.
	bool HasEnclosedEnvironment() const;

	// True if this bds design has at least one environment void.
	bool HasEnvironmentVoid() const;

	// Returns true if this building is a capitol or police station and 'loc' is in my jurisdiction area.
	// Beware, does not check if this building is part of a city; that is required to actually assert a jurisdiction.
	bool HasJurisdictionAt(const DaWorld& world, const AuPoint& loc) const;

	// Returns true if this building is a capitol or police station and a circle of 'radius' at 'loc' is in my jurisdiction area.
	// Beware, does not check if this building is part of a city; that is required to actually assert a jurisdiction.
	bool HasJurisdictionOverlapAt(const DaWorld& world, const AuPoint& loc, double radius) const;

	bool HasLifeSupport() const;

	// True if this building has 'patentCID' at any quality.
	bool HasPatent(COMMODITYID patentCID) const;

	// True if this building has 'patentCID' at 'quality' or higher.
	bool HasPatent(COMMODITYID patentCID, quint8 quality) const;

	bool HasPower() const;

	virtual bool HasPowerPlant() const;

	virtual bool HasSensors() const;

	virtual bool HasShields() const;

	virtual bool HasSpaceVehicleLauncher() const;

	virtual bool HasSpaceVehicleRecoverySystem() const;

	// 'cid' is a tool in the sense of m_Tools or a vehicle in m_Vehicles.
	// Grabs a tool from inventory if necessary to return true if possible.
	bool HasToolOrVehicle(COMMODITYID cid);

	bool HasTradeConnection(const AuId& toBdOrCityId) const;

	bool HasTradeConnectionByGround(const AuId& toBdOrCityId) const;

	bool HasAnyTradeConnection() const;

	virtual bool HasTransporter() const;

	// Returns true if this building possessed a vehicle of a certain type recently.
	inline bool HasVehicle(COMMODITYID vehicleCID) const
	{
		m_CargoLock.lockForRead();
		bool v = m_Vehicles.contains(vehicleCID);
		m_CargoLock.unlock();
		return v;
	}

	virtual bool HasVehicleTransporter() const;

	bool HasVehicleType(VEHICLE_SPOT_TYPE vehicleCID) const;

	virtual bool HasWeaponBay() const;

	inline bool HasWharfAccess() const
	{
		return m_Wharf;
	}

	virtual bool HasWorkingSurgeryUnit() const;

	virtual double Height() const;

	virtual AuPoint HelmLocation() const;

	inline unsigned int HitsDamageStructural() const
	{
		m_DamageLock.lockForRead();
		unsigned int v = m_Damage.HitsDamageStructural();
		m_DamageLock.unlock();
		return v;
	}

	virtual int RemoteDoorIndex(bool forVehicle, const AuPoint& loc) const;

	virtual int RemoteDoorIndex(bool forVehicle, const AuPoint& loc, quint16 roomId) const;

	virtual bool HullDoorLocked(unsigned int doorIndex) const;

	// Returns volume of the hull in cubic meters.
	virtual quint32 HullVolume() const;

	// Returns volume of hull and everthing else allocated from that volume. Volumes are in whole cubic meters.
	void HullVolumeDistribution(quint32& hullVolume, quint32& arena, quint32& armor, quint32& auditorium, quint32& capacitor,
			quint32& hold, quint32& homeLarge, quint32& homeMedium, quint32& homeSmall, quint32& office, quint32& lifeSupport,
			quint32& lounge, quint32& powerPlant, quint32& retail, quint32& sensor, bool& hasSensor, quint32& shield,
			bool& hasShield, quint32& shop, QMap<quint16, AuScReportSysVol>& bays, quint32& skeleton) const;

	// Returns true if 'atom' is in the chain of command of this building.
	// Returns true if 'atom' has agent power enabled.
	virtual bool InChainOfCommand(const DaAtom* atom) const;

	// Returns true if the person is in the chain of command of this building.
	virtual bool InChainOfCommand(const AuId& personId, quint32 personEmpire) const;

	// Returns true if this building is part of the same community as 'bd'. i.e. both military or both have same capitol. Assumes
	// 'bd' is on the same world as this building.
	bool InCommunity(const DaBd& bd) const;

	// Returns true if my capitol is 'capitolId', even if both are zero.
	bool InCommunityCity(const AuId& capitolId) const;

	inline INDUSTRYID IndustryId() const
	{
		return m_IndustryId;
	}

	void IndustryInfo(quint32& homesLarge, quint32& homesMedium, quint32& homesSmall, quint32& manufacturingJobs,
			quint32& serviceJobs, quint32& stores, quint32& lounges, quint32& proJobs) const;

	void IndustryInfoJobs(quint32& homesLarge, quint32& homesMedium, quint32& homesSmall, quint32& manufacturingJobs,
			quint32& manufacturingJobsEnabled, quint32& serviceJobs, quint32& stores, quint32& lounges, quint32& proJobs) const;

	QString IndustryName() const;

	void InitPreview(const AuBdReport& r, const QString& bdName, quint32 empireId, const AuSpecies& residentSpecies);

	// Initializes this ship's m_ssaShipSystems systems data.
	// This is only called once in the lifetime of this ship atom, at the time it is first created.
	void InitSystems(COMMODITYID hullModule, quint8 quality);

	// Updates m_ssaShipSystems to match the systems reported by my design.
	void ReconcileSystems();

	// Returns true if this building would be in the jurisdiction area of a police station built or capitol at 'proposedBdLoc'.
	// 'jr' is the JurisdictionRadius of the proposed building.
	bool InProposedJurisdiction(const DaWorld& world, const AuPoint& proposedBdLoc, double jr) const;

	virtual void Insert(QSqlDatabase& db) const;

	virtual bool InsideRoom(const AuPoint& loc, quint16& roomId) const;

	virtual quint16 InsideRoomType(const AuPoint& loc, ROOMTYPE& roomType, QString& roomName) const;

	virtual bool InsideRoomType(const AuPoint& loc, bool getRoomUp, quint16& roomId, ROOMTYPE& roomType, double& htAbvFloor,
			QString& roomName, AuPoint& gravityN) const;

	virtual void InteriorAmbientLight(const QString& roomName, float& ambientR, float& ambientG, float& ambientB,
			bool& sunsBlocked) const;

	virtual INTERIOR_LIGHT InteriorLight(const QString& roomName) const;

	virtual bool InteriorLightToggle(INTERIOR_LIGHT state);

	virtual bool InteriorLightToggle(const QString& roomName, INTERIOR_LIGHT state);

	// Returns true if any of my space vehicle launches is in a non-zero state.
	bool IsAnyLauncherActive() const;

	inline bool IsBayFiring(quint16 which) const
	{
		m_BayLock.lockForRead();
		bool cw = m_BayFired.find(which) != m_BayFired.cend();
		m_BayLock.unlock();
		return cw;
	}

	// Returns true if this building is a capitol building that would be in the jurisdiction of a police station built at
	// 'proposedPoliceStation'.
	// 'jr' is the JurisdictionRadius of the proposed station.
	bool IsCapitolInJurisdictionAt(const DaWorld& world, const AuPoint& proposedPoliceStation, double jr) const;

	virtual bool IsCommandConsole(quint16 stationId) const;

	// Determine if a console is functioning for a given subsystem 'which'. It might be inoperative due to system damage.
	virtual bool IsConsoleOperational(const AuSysId& sysId) const;

	inline bool IsConstructionWorkerEnabled() const
	{
		m_ConstructionLock.lockForRead();
		bool v = m_pConstruct && !m_pConstruct->IsHalted();
		m_ConstructionLock.unlock();
		return v;
	}

	virtual bool IsControl(const DaAtom* sceneAtom) const;

	virtual bool IsDamaged() const;

	bool IsDamagedHeavily() const;

	virtual bool IsDesignDataCacheValid() const;

	inline bool IsDestroyed() const
	{
		return HitsDamageStructural() >= DesignHitsStructural();
	}

	virtual bool IsDoorIndexValid(unsigned int doorIndex) const;

	virtual bool IsDoorJammed(unsigned int doorIndex) const;

	virtual bool IsDoorLocked(unsigned int doorIndex) const;

	virtual bool IsDoorOnWall(unsigned int doorIndex) const;

	virtual bool IsDoorOpen(unsigned int doorIndex) const;

	// Returns true if workers at this building are considered essential, making them immune to being stolen by essential buildings.
	bool IsEssential(bool hasBreathableAtmo) const;

	// Returns the fuel commodity based on this building's power plant module, even if this building has no generator, for storage.
	// Returns CID_Invalid if this building has no power plant module or if it does not consume fuel.
	COMMODITYID Fuel() const;

	// Returns true if 'cid' is fuel for this building's power plant module, even if this building has no generator, for storage.
	// Returns true if 'cid' is CID_Hydrogen if this building's power plant module is invalid.
	inline bool IsFuel(COMMODITYID cid) const
	{
		return Fuel() == cid;
	}

	virtual bool IsHidingInGroundClutter() const;

	virtual bool IsHoleOnWall(unsigned int holeIndex) const;

	virtual bool IsHullDoor(int doorIndex) const;

	// Returns true if we've been in range of an enemy bay weapon within the last second.
	inline bool IsInEnemyRange() const
	{
		// A fire control station operator might only scan us once every 4500ms plus or minus.
		return WasInEnemyRange(8000);
	}

	virtual int IsInOrOnHull(const AuPoint& loc) const;

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsInsideAirBreathable() const;

	// Returns true if I have no trade connections to anywhere else.
	bool IsIsolated() const;

	bool IsMilitary() const;

	virtual bool IsObstacle(const DaAtom* toWho) const;

	virtual bool IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc) const;

	bool IsPlatform() const;

	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	virtual bool IsShipWeaponTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, AuPoint& targetSysLoc) const;

	bool IsStreetLightOn() const;

	// Returns true if there are no workers and no resident.
	inline bool IsVacant() const
	{
		m_BdLock.lockForRead();
		bool vacant = m_Workers == 0 && m_Residents == 0;
		m_BdLock.unlock();
		return vacant;
	}

	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho);

	// A quarry is a mine that has no shop space. It is limited to mining only stone.
	bool IsQuarry() const;

	bool IsSmoldering() const;

	// Returns true if 'targetShip' is being towed by my tractor beam.
	// 'tractorRange' is set to the maximum range of the tractor that is towing the target ship.
	// 'tractorAccel' is set to the maximum acceleration of the tractor that is towing the target ship.
	bool IsTowing(const AuId& targetShip, float targetMass, double& tractorRange, double& tractorAccel) const;

	inline bool IsTractorBeamAttached(AuPoint& epos) const
	{
		bool v = false;
		m_BayLock.lockForRead();
		for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
			if((*lfi).second.IsTractorAttached())
			{
				epos = (*lfi).second.m_EmitLoc;
				v = true;
				break;
			}
		m_BayLock.unlock();
		return v;
	}

	virtual bool IsTransporterRoom(quint16 roomId) const;

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual bool IsTransporterSystemActive(int troomIndex) const;

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual bool IsTransporterSystemOn(int troomIndex) const;

	inline bool IsUnderConstruction() const
	{
		return m_pConstruct;
	}

	virtual bool IsVisibleNearWorld() const;

	virtual void JamDoorClosed(unsigned int doorIndex);

	virtual void JamDoorOpen(unsigned int doorIndex);

	// Returns true if any jobs at all.
	bool Jobs(quint32& manufacturingJobs, quint32& proJobs, quint32& serviceJobs) const;

	// Returns the time the last Alert message was sent by a call to SendAlert or firing alert message broadcast.
	inline AuElapsedTimer LastAlert() const
	{
		m_CommLock.lockForRead();
		AuElapsedTimer v(m_tmAlert);
		m_CommLock.unlock();
		return v;
	}
	
	// Returns AuCityIndustryHash::TotalLevels.
	quint32 LastReportedBuildingLevels() const;

	// small, medium and large combined
	quint32 LastReportedHomes() const;

	quint32 LastReportedPopulation() const;

	quint32 LastReportedLevels(INDUSTRYID iid) const;

	// Returns the time the last SOS message was sent by a call to SendSOS.
	inline AuElapsedTimer LastSOS() const
	{
		m_CommLock.lockForRead();
		AuElapsedTimer v(m_tmSOS);
		m_CommLock.unlock();
		return v;
	}

	virtual double Length() const;

	// Test for life support functioning.
	virtual bool LifeSupportWorking() const;

	// Lists manufacturing processes that can be added to this building.
	int ListAddableProcesses(QMap<QString, quint16>& processes) const;

	virtual void ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const;

	virtual void ListOccupiedTroopStations(QSet<quint16>& troopStations, const AuId& exceptTroop) const;

	// List patents that are recorded in this city.
	// 'iid' is the industry in which the requestor is standing. At a university, all patents in the city are listed. At any other
	// industry, only patents that can be used by that industry are listed.
	void ListPatents(AuKnownPatents& patents, INDUSTRYID iid) const;

	// 'hitsNeeded' is set to the number of hit points of damage points that need repairing.
	// 'partsNeeded' is filled with parts needed for the repairs.
	// All parameters are output parameters. They will get set to something; 0 if no repairs are needed.
	void ListRepairsNeeded(quint32& hitsNeeded, AuRepairParts& partsNeeded) const;

	virtual void ListRoomNames(QStringList& roomNames) const;

	virtual void ListSystems(QList<AuSystemInfo>& systems) const;

	// Fills a list of the ids that are towed by tractor beams.
	// Returns false if no tractors are attached.
	inline bool ListTractorBeamTowed(AuIdList& towed) const
	{
		bool v = false;
		m_BayLock.lockForRead();
		for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
			if((*lfi).second.IsTractorAttached())
			{
				towed.append((*lfi).second.m_Target);
				v = true;
			}
		m_BayLock.unlock();
		return v;
	}

	AuSpecies LivestockSpecies() const;

	static DaBd* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);

	// 'whichTurret' is the system which identifier of the turret to load, from DaCreature::ConsoleWhich.
	void LoadTurretAmmunition(const AuItem& item, quint16 whichTurret);

	virtual QString LocationDesc(const DaAtom& atom) const;

	// Returns a pointer to the ammo item. The count is set the the number available in the hold.
	inline const AuItem* LockBayWeaponAmmunition(quint16 which) const
	{
		// This is locked for write because the code that calls this calls WeaponBayFired next.
		m_BayLock.lockForWrite();

		unordered_map<quint16, AuBaySetting>::const_iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end() && (*bsi).second.m_Ammo)
		{
			m_CargoLock.lockForWrite();
			AuItemHash::const_iterator ammoI = m_Cargo.Find(*(*bsi).second.m_Ammo);
			if(ammoI == m_Cargo.end())
				(*bsi).second.m_Ammo->SetCount(0);
			else
				(*bsi).second.m_Ammo->SetCount((*ammoI).second->Count());
			m_CargoLock.unlock();
			return (*bsi).second.Ammo();
		}
		return 0;
	}

	void ClearBankActivity();

	void ClearBankActivityAccountLocked();

	// Returns my current bank information.
	inline const AuBank& LockBank() const
	{
		m_AccountLock.lockForRead();
		return m_Bank;
	}

	inline AuBank& LockBankForWrite()
	{
		m_AccountLock.lockForWrite();
		return m_Bank;
	}

	inline void UnlockBank() const
	{
		m_AccountLock.unlock();
	}

	virtual AuItemHash& LockCargoForWrite()
	{
		m_CargoLock.lockForWrite();
		return m_Cargo;
	}

	virtual const AuItemHash& LockCargoForRead() const
	{
		m_CargoLock.lockForRead();
		return m_Cargo;
	}

	inline void UnlockPatents() const
	{
		m_CargoLock.unlock();
	}

	inline const DeConstructionProcess* LockConstructionProcessForRead() const
	{
		m_ConstructionLock.lockForRead();
		return m_pConstruct;
	}

	inline DeConstructionProcess* LockConstructionProcessForWrite() const
	{
		m_ConstructionLock.lockForWrite();
		return m_pConstruct;
	}

	inline const AuBdReport& LockDesignAnalysis() const
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

	inline const DeManufacturingProcessArray& LockManufacturingProcessesForRead() const
	{
		m_MfgProcLock.lockForRead();
		return m_MfgProc;
	}

	inline DeManufacturingProcessArray& LockManufacturingProcessesForWrite()
	{
		m_MfgProcLock.lockForWrite();
		return m_MfgProc;
	}

	// Beware this is the same lock as the cargo lock.
	inline const AuKnownPatents& LockPatentsForRead() const
	{
		m_CargoLock.lockForRead();
		return m_Patents;
	}

	inline const AuShipSystemArray& LockSystems() const
	{
		m_SystemsLock.lockForRead();
		return m_ssaShipSystems;	// List of all Ship Systems. ONLY A SUBSET OF THIS DATA IS SENT TO THE SCENE CLIENT.
	}

	inline unordered_map<quint16, AuOrdnance>& LockTurretOrdnance()
	{
		m_TurretLock.lockForWrite();
		return m_Turrets;
	}

	quint32 Lounges() const;

	virtual int LowestDeck() const;

	// Adds a manufacturing process for 'mpid' if I don't already have one and if all of my workers are not assigned to jobs.
	// Assigns one worker to the new process.
	void ManufactureAddMPID(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit);

	// Clears all manufacturing processes.
	void ManufactureClear();

	// Deletes manufacturing process 'mpid'.
	void ManufactureDeleteMPID(quint16 mpid);

	void ManufactureFetch(quint16 panelIndex);
	void ManufactureFetchMPID(quint16 mpid);

	void ManufactureGo(quint16 panelIndex, DaPerson* byWho);
	void ManufactureGoMPID(quint16 mpid, DaPerson* byWho);

	void ManufactureLowerMPID(quint16 mpid);

	void ManufactureMaxMPID(quint16 mpid);

	void ManufactureMinMPID(quint16 mpid);

	void ManufactureProcessSelected(quint16 panelIndex, quint32 mpid);

	void ManufactureProcessSetMinQMPID(quint16 mpid, quint8 minQ);
	void ManufactureProcessSetStockLimitMPID(quint16 mpid, quint32 stockLimit);

	void ManufactureRaiseMPID(quint16 mpid);

	void ManufactureSetMinQ(quint8 minQ);	// Sets minQ in all processes.

	quint32 ManufacturingJobs() const;

	// Color of this building's marker on a map.
	QColor MapColor() const;

	virtual float Mass() const;

	virtual float MaxBayWeaponRange() const;

	virtual float MaxTransporterRange() const;

	// Monitor/refresh all channels this ship should be monitoring.
	void MonitorCommChannels();

	// Returns morale at last city report.
	inline qint8 Morale() const
	{
		return m_Morale;
	}

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	virtual QString Name() const;

	virtual QString NameWithEmpire() const;

	virtual int OnDesignDeck(const AuPoint& loc) const;

	virtual bool OpenDoor(unsigned int doorIndex);

	bool OpenLauncherDoors(quint16 launcherId);

	virtual double OrientationSpeed() const;

	// Paints this building onto a surface map of a planet.
	virtual void PaintOnSurfaceMap(const DaWorldLL& world, QPainter& p, unsigned int width, unsigned int height) const;

	// If 'medicStationId' is found, 'surgeryId' will be set to the same value.
	virtual void PatientStation(quint16 medicStationId, quint16& surgeryId, AuStation& station) const;

	// Includes transients.
	virtual quint32 Population() const;

	virtual void PositionTurret(const DaCreature& gunner);

	void PositionTurret(quint16 turretId, float baseRot, float gunRot);

	// Test for power plant on and functioning.
	inline bool PowerPlantWorking() const
	{
		BUILDING_STRUCTURE bs = Structure();
		if(bs == BD_AerialPlatform ||
				bs == BD_SeaPlatform ||
				bs == BD_GroundPlatform)
			return true;

		// 100% damage to power plant with no shunts active will make it stop working.
		if(!SystemHitsRemaining(SYS_PowerPlant))
			return false;

		// Everything looks good.
		return true;
	}

	virtual float PowerPlantUnits() const;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& gearPos,
			qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed);

	// Returns the quality at which this ship was constructed. Any or all systems may have been upgraded since then.
	inline quint8 QualityMade() const
	{
		return m_QMade;
	}

	virtual quint8 QualityOfDesign() const;

	// Location returned is relative to my parent atom.
	AuPoint RandomSpawnLoc() const;

	inline void RechargeCapacitor()
	{
		float capMax = CapacitorMax();
		m_VesselLock.lockForWrite();
		m_CapacitorLevel = capMax;
		m_VesselLock.unlock();
	}

	// Returns true if 'patentCID' is recorded as a new patent, false if it was already on record.
	bool RecordPatent(COMMODITYID patentCID, quint8 quality);

	// Returns the number of new patents that are recorded.
	int RecordPatents(const AuKnownPatents& patents);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual quint32 PowerPlantHits() const;

	virtual AuPoint RemoteCameraLocation(quint16 cameraId) const;

	virtual quint16 RemoteCameraNext(quint16 currentCamera) const;

	virtual quint16 RemoteCameraPrevious(quint16 currentCamera) const;

	virtual bool RemoteCameraVerify(quint16 cameraId) const;

	// Reloads turret with current ordnance. Picks nu ordnance at random if current ordnance is gone.
	// Does nothing if current ordnance is full of ammo.
	// Returns true if the turret is loaded.
	virtual bool ReloadTurret(quint16 which);

	virtual bool RemoveCargo(const AuItem& iTake, quint32 howMany);

	bool RemoveCargoNoLock(const AuItem& iTake, quint32 howMany);

	virtual void RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
			POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
			const AuMatrix4x4& tDevOrtho, float hudIntensity);

	void RenderConstruction(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);

	// Sets up shader and uses my m_CursorVAO and m_CursorCoverageVAO.
	void RenderCursorHit(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool siteOnly,
			const QColor& color);

	// Shader has already been configured. Uses my m_CursorVAO and m_CursorCoverageVAO.
	void RenderCursorHit_Shader(AuGLEffects& re, bool siteOnly);

	// Shader has already been configured. Creates 'vao' if it is null; caller is responsible for calling vao.DeleteGL.
	// 'usage' is passed to glBufferData. e.g. GL_STREAM_DRAW or GL_STATIC_DRAW.
	// 'vaoCoverage' is the coverage area lines of police stations and shield generators
	void RenderCursorHit_Shader(AuGLVAO& vao, AuGLVAO& vaoCoverage, AuGLEffects& re, bool siteOnly, GLenum usage);

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

	virtual void RenderScGlobe(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass);
	virtual void RenderScGlobeFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass);
	virtual void RenderScRingworld(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom,
			RENDERPASS pass);
	virtual void RenderScRingworldFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom,
			RENDERPASS pass);

	virtual void RenderOnSurfaceTexture(AuGLEffects& re, float radiansPerFootX, float radiansPerFootY) const;

	void ReportEnemySensorContact(const DaSolarSystem* system, const AuSensorBlipList& sensorBlips, float sensorRange);

	virtual void RequestLaunch(DaCreature* pilot, DaVehicle* vehicle);

	virtual void RequestRecovery(DaCreature* pilot, DaVehicle* vehicle, const AuPoint& vehLoc);

	void ResetWeaponRadarRange();

	// Returns the number actually removed, based on number of residents living in this building.
	quint32 ResidentDied(quint32 howMany);

	virtual bool RoomHasNavHUD(const QString& roomName) const;

	virtual bool RoomOrderLoc(const QString& roomName, AuPoint& orderLoc) const;

	virtual bool RoughExtents(AuBox &fExt) const;

	virtual bool RoughExtentsWithShields(AuBox &fExt) const;

	virtual double RoughRadius() const;

	double RoughSiteRadius() const;

	bool Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, const DaAtom* toWho, bool isCrew) const;

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	// Selects weapon to fire from bay. Picks nu ordnance at random if current ordnance is gone.
	// Does nothing if current ordnance is available.
	// Returns true if the weapon bay selection is changed.
	// Returns false if 'which' bay is not configured with a weapon bay module.
	virtual bool SelectBayWeapon(quint16 which);

	// Returns false if 'which' bay is not configured with a weapon bay module.
	bool SelectBayWeaponAmmunition(const AuItem& ammo, quint16 which);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

	// Transmits a message on the Friend channel at Galaxy range.
	// 'text' is the text of the message.
	void SendAlert(const QString& text);

	// Transmits a message on the Friend channel at Galaxy range.
	// Stations also send the message on the Trade channel.
	// AI commanded spacecraft also send the message on the Hail channel.
	// 'msgType' selects the appropriate AuCommSOS-derived comm message type.
	//   0 = AuCommSOSManeuverDriveStation
	//   1 = AuCommSOSFuelStation
	//   2 = AuCommSOSManeuverDriveMarooned
	//   3 = AuCommSOSManeuverDriveAdrift
	//   4 = AuCommSOSFuelMarooned
	//   5 = AuCommSOSFuelShip
	//   6 = AuCommSOSAttack
	// 'msgText' and 'msgValue' are relevant to the comm message type.
	void SendSOS(int msgType, const QString& msgText, int msgValue);

	virtual void SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
			int scannerDomain, const AuId& scannerWorld,
			float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const;

	virtual float SensorRange() const;

	// Returns the deep space sensor range in pc.
	float SensorRangeDeepSpace() const;

	// Test for sensors on and functioning.
	virtual bool SensorsWorking() const
	{
		// Capacitor must have some power for system to work.
		if(!HasPower())
			return false;
		
		// 100% damage to sensors will make them stop working.
		if(!GetPowerConsumption(SYS_Sensor))
			return false;

		// Everything looks good.
		return true;
	}

	int ServiceJobs() const;

	inline void SetBayWeaponPowerSetting(quint16 which, quint8 powerSetting)
	{
		m_BayLock.lockForWrite();
		unordered_map<quint16, AuBaySetting>::iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			(*bsi).second.m_Setting = powerSetting;
		else
		{
			AuBaySetting baySetting;
			baySetting.m_Setting = powerSetting;
			m_BaySetting.insert(pair<quint16, AuBaySetting>(which, baySetting));
		}
		m_BayLock.unlock();
	}

	inline void SetBayWeaponTarget(quint16 which, const AuId& targetId)
	{
		m_BayLock.lockForWrite();
		unordered_map<quint16, AuBaySetting>::iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			(*bsi).second.m_TargetId = targetId;
		else
		{
			AuBaySetting baySetting;
			baySetting.m_TargetId = targetId;
			m_BaySetting.insert(pair<quint16, AuBaySetting>(which, baySetting));
		}
		m_BayLock.unlock();
	}

	void SetResidentSpecies(const AuSpecies& species);

	void SetDesign(const AuBdReport& r);

	virtual void SetEmpireId(quint32 idEmpire);

	void SetupPirate(quint32 empireId, quint8 quality, DaWorldLL* llworld);

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	inline void SetInEnemyRange() const
	{
		m_LastInRangeLock.lockForWrite();
		m_LastInEnemyRange.restart();
		m_LastInRangeLock.unlock();
	}

	// 'species' can be null to unset it.
	void SetLivestockSpecies(const AuSpecies& species);

	inline void SetMorale(qint8 morale)
	{
		m_Morale = morale;
	}

	virtual void SetName(const QString& name);

	// Returns the center of the shield bubble, which is a sphere.
	// Calculates where the center of the shield bubble should be, so that it does not intersect the atmosphere if possible.
	AuPoint ShieldBubbleCenter() const;

	double ShieldBubbleRadius() const;

	// Returns the energy in a given shield's capacitor.
	// Return value will be 0..ShieldUnitsWorking().
	inline double ShieldCapacitor() const
	{
		m_ShieldsLock.lockForRead();
		double v = m_ShieldCap;
		m_ShieldsLock.unlock();
		return v;
	}

	// Increases current shield power by 'power', limited to working shield units.
	inline void ShieldCharge(float power)
	{
		double powerMax = ShieldUnitsWorking();
		m_ShieldsLock.lockForWrite();
		m_ShieldCap += power;
		if(m_ShieldCap > powerMax)
			m_ShieldCap = powerMax;
		else if(m_ShieldCap < 0)
			m_ShieldCap = 0;
		m_ShieldsLock.unlock();
	}

	// 'loc' is in my space.
	// Does not test ShieldsWorking.
	bool ShieldContains(const AuPoint& loc) const;

	virtual bool ShieldsBlockPeerTransport(quint32 empireId, const AuPoint& loc) const;

	virtual bool ShieldsBlockTransportAboard() const
	{
		// Shields only block transporters if they are on and working.
		if(!ShieldsWorkingAndUp())
			return false;
		// Shields block transporters if every side has more than 2 units of energy.
		m_ShieldsLock.lockForRead();
		bool v = m_ShieldCap > 2 && !m_ShieldScram;
		m_ShieldsLock.unlock();
		return v;
	}

	virtual bool ShieldsBlockWeapons() const
	{
		// Shields only block transporters if they are on and working.
		if(!ShieldsWorkingAndUp())
			return false;
		// Shields could block weapons if any side has more than 2 units of energy.
		m_ShieldsLock.lockForRead();
		bool v = m_ShieldCap > 2 && !m_ShieldScram;
		m_ShieldsLock.unlock();
		return v;
	}

	// Returns the number of seconds remaining before shield is unscrambled.
	inline unsigned int ShieldScram() const
	{
		m_ShieldsLock.lockForRead();
		unsigned int v = m_ShieldScram;
		m_ShieldsLock.unlock();
		return v;
	}

	// Sets current shield power to 'power', limited to working shield units.
	inline void ShieldSetCharge(double power)
	{
		if(power < 0)
			return;
		double powerMax = ShieldUnitsWorking();
		m_ShieldsLock.lockForWrite();
		m_ShieldCap = power;
		if(m_ShieldCap > powerMax)
			m_ShieldCap = powerMax;
		m_ShieldsLock.unlock();
	}

	inline void ShieldSetScram(quint8 scramTime)
	{
		if(scramTime > 30)
			scramTime = 30;
		m_ShieldsLock.lockForWrite();
		m_ShieldScram = scramTime;
		m_ShieldsLock.unlock();
	}

	void ShieldStabilize();

	virtual bool ShieldSurfaceNear(const AuPoint& loc) const;

	virtual bool ShieldsWorkingAndOn() const;

	virtual bool ShieldsWorkingAndUp() const;

	virtual double ShieldUnits() const;

	// Returns the number of shield units functioning, adjusted for damage.
	double ShieldUnitsWorking() const;

	bool SightBuildZone(const AuPoint& eyeLoc, const AuMatrix4x4& model, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	virtual AuSysId SightedSystem(quint16 sightedPart) const;

	// Returns the distance at which this ship appears in a scene. Also the distance at which AI can see this ship with their eyes.
	inline static double SightRange2()
	{
		return 32000*FeetPerMeter() * 32000*FeetPerMeter();	// about 20 miles
	}

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4& model, AuPoint& sightedLoc, unsigned int& sightedPart,
			bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	// Returns the box radius of my site, centered at SiteCenter. X, Y and Z will hold the positive box radii.
	AuPoint SiteBoxRadius() const;

	// Returns the point at the center of my site, relative to my design.
	AuPoint SiteCenter() const;

	// This is a 2D projection of loc into my site. Any hit returns true.
	// 'loc' is in the space of the world.
	bool SiteContains(const AuPoint& loc) const;

	// 'siteGround' is a return parameter on true return.
	// 'hideGrass' is a return parameter on true return. It can be 0=no, 1=yes, 2=mow, 3=lawn
	bool SiteContains(const AuPoint& loc, ON_GROUND& siteGround, int& hideGrass) const;

	// 'locs' are in the space of the world.
	inline bool SiteContains(const AuPointArray& locs) const
	{
		for(AuPointArray::const_iterator li = locs.cbegin(); li != locs.cend(); li++)
			if(SiteContains(*li))
				return true;
		return false;
	}

	// Returns the number of a given rock commodity present in my site.
	// Uses m_RocksOnSite cache.
	quint32 SiteContainsResource(COMMODITYID cid) const;

	// This is a 2D projection of loc into my site. Any hit returns true.
	// 'loc' is in the space of the world.
	bool SiteExtentsContain(const AuPoint& loc) const;

	bool SiteExtentsContain(const AuPoint& loc, double radius) const;

	// Returns the box radius of my site and hull, centered at SiteCenter. X, Y and Z will hold the positive box radii.
	AuPoint SiteHullBoxRadius() const;

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	// 'cp' has already been configured with the construction 'material'.
	void StartConstruction(INDUSTRYID industryId, const AuBdReport& report, quint32 sitePrepLabor, quint16 mpid, bool workerOk,
			const AuSpecies& residentSpecies);

	virtual AuTime StasisReadyTime() const;

	// Retrieves the screen quad for 'stationId'.
	// Returns true on success, false if design is not loaded/ready.
	virtual bool StationScreen(quint16 stationId, AuQuad& screen) const;

	quint32 Stores() const;

	virtual BUILDING_STRUCTURE Structure() const;

	// Returns a number from 0 to 1, which is the ratio of current structural hit points to design structural hit points.
	float StructuralIntegrity() const;

	virtual quint32 SystemHits(const AuSysId& sysId) const;

	inline quint32 SystemHitsRemaining(const AuSysId& sysId) const
	{
		m_SystemsLock.lockForRead();
		quint32 v = SystemHitsRemainingSystemsLocked(sysId);
		m_SystemsLock.unlock();
		return v;
	}

	virtual quint32 SystemHitsRemainingSystemsLocked(const AuSysId& sysId) const;

	inline COMMODITYID SystemModule(const AuSysId& sysId) const
	{
		if(sysId.Which())
			return SystemModule(sysId.System(), sysId.Which());
		else
			return SystemModule(sysId.System());
	}

	// Current module of a system.
	inline COMMODITYID SystemModule(SYSTEM system) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		COMMODITYID v = SystemModuleSystemsLocked(system);
		m_SystemsLock.unlock();
		return v;
	}

	inline COMMODITYID SystemModuleSystemsLocked(SYSTEM system) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		return m_ssaShipSystems.SystemModule(system);
	}

	// Current module of a system.
	inline COMMODITYID SystemModule(SYSTEM system, quint16 which) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		COMMODITYID v = SystemModuleSystemsLocked(system, which);
		m_SystemsLock.unlock();
		return v;
	}

	inline COMMODITYID SystemModuleSystemsLocked(SYSTEM system, quint16 which) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		return m_ssaShipSystems.SystemModule(system, which);
	}

	virtual QString SystemName(const AuSysId& sysId) const;

	// Current quality of a system.
	// Initially quality at which the ship was manufactured. This may be greater than the quality of the design.
	virtual quint8 SystemQuality(SYSTEM system) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		quint8 v = m_ssaShipSystems.SystemQuality(system);
		m_SystemsLock.unlock();
		return v;
	}

	// Current quality of a system.
	// Initially quality at which the ship was manufactured. This may be greater than the quality of the design.
	virtual quint8 SystemQuality(SYSTEM system, quint16 which) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		quint8 v = m_ssaShipSystems.SystemQuality(system, which);
		m_SystemsLock.unlock();
		return v;
	}

	// Takes one of my workers, if I have any.
	inline bool TakeWorker()
	{
		bool tookWorker;
		m_BdLock.lockForWrite();
		if(m_Workers)
		{
			m_Workers--;
			tookWorker = true;
		}
		else
			tookWorker = false;
		m_BdLock.unlock();
		return tookWorker;
	}

	virtual AuPoint TargetLocation() const;

	// Tests this building to see if it is ready to decay due to abandonment.
	// Returns true if I'm ready to be destroyed.
	bool TestAbandonmentDecay() const;

	// Test 'bzLocs' to insure the building that returned them, using BuildZoneCheckLocs, can be built on this platform.
	// Returns true if the locations look ok.
	bool TestInBuildZone(const AuPointArray& bzLocs) const;

	// Returns true if this building should be SKIPPED due to the filter.
	// TRUE = SKIPPED.
	bool TestFindFilter(FINDDEV filter) const;

	virtual bool TestPowerSystem();

	virtual bool TestTransportLocation(const AuPoint& loc) const;

	virtual void TickOnClient(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual COMMODITYID TransporterModule(quint16 which) const
	{
		return SystemModule(SYS_Transporter, which);
	}

	virtual void TransporterPadLocations(quint16 which, AuPointArray& pads) const;

	virtual int TransporterPadsWorking(quint16 which) const
	{
		return GetPowerConsumption(SYS_Transporter, which);
	}

	virtual QList<quint16> TransporterSystemWhiches() const;

	virtual TRANSPORTER_TARGET TransporterTarget() const;

	virtual QString TransporterTargetName() const;

	virtual bool TransporterVoidLocation(AuPoint& voidLoc) const;

	virtual void ToggleDoorLock(const DaPerson& person, unsigned int targetPart);

	virtual void ToggleDoorState(const DaSentient& who, unsigned int doorIndex);

	virtual void ToggleHullDoor(int doorIndex);

	virtual float TractorBeamRange(float unitsFired, quint16 which) const;

	// Returns the transporter range in feet, limited to the current maximum sensor range.
	// Damage to transporters affects the number of pads that can be activated, not the range. 
	inline float TransporterRange(quint16 which) const
	{
		float sensorRange = SensorRange();
		const float tlBoost = QualityBoostF(SystemQuality(SYS_Transporter, which));
		const float maxTransporterRange = NominalTransporterRange() * tlBoost;	// Maximum possible transporter range at quality.
		if(sensorRange < maxTransporterRange)
			return sensorRange;
		return maxTransporterRange;
	}

	// 'which' is the transporter station/console part id.
	// Returns the part id since the console which is the same as the system which.
	virtual int TransporterSystemIndex(quint16 which) const;

	virtual int TransporterSystems() const;

	virtual quint16 TransporterSystemId(int padsDesired, bool mustBeOn) const;

	virtual quint16 TransporterWhich(const AuSensorBlip& blip) const;

	virtual quint16 TransporterWhich(const DaAtom& atom) const;

	// Returns the current turbo lift stop of the lift 'liftId'.
	QString TurboLiftStop(quint16 liftId) const;

	// 'liftStops' is filled with the lift stops available to 'liftId'.
	virtual QString TurboLiftStop(quint16 liftId, QStringList& liftStops) const;

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual void TurnTransporterSystemOff(int troomIndex);

	// 'troomIndex' as returned by TransporterSystemIndex is actually a quint16 transporter station part id.
	virtual void TurnTransporterSystemOn(int troomIndex);

	// Matrix used when aiming the gun barrel of a turret.
	//  AuPoint barrelLoc = barrelM.Translation();				// Axis point of barrel, relative to this ship.
	//  AuPoint barrelVec = barrelM.ProjectVector(0,0,-1);		// Aim vector of barrel at 0 elevation, relative to this ship.
	// 'baseRot' is rotation around the base/Z axis of the turret.
	void TurretAimMatrix(quint16 partId, float baseRot, AuMatrix4x4& barrelM) const;

	virtual AuItem* TurretAmmunitionClone(quint16 which) const
	{
		AuItem* v;
		m_TurretLock.lockForRead();
		unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.find(which);
		if(ti != m_Turrets.end() && (*ti).second.Ammo())
			v = (*ti).second.Ammo()->Clone();
		else
			v = 0;
		m_TurretLock.unlock();
		return v;
	}

	float TurretBaseRot(quint16 turretId) const;

	float TurretGunRot(quint16 turretId) const;

	inline bool TurretCanFire(quint16 which) const
	{
		bool v;
		m_TurretLock.lockForRead();
		unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.find(which);
		if(ti != m_Turrets.end())
			v = (*ti).second.CanFire();
		else
			v = false;
		m_TurretLock.unlock();
		return v;
	}

	virtual AuPoint TurretHemiVector(const AuPose& pose, quint16 which) const;

	// Matrix to shoot out of the gun barrel of a turret.
	//  AuPoint muzzleLoc = muzzleM.Translation();				// Relative to this ship.
	//  AuPoint muzzleVec = muzzleM.ProjectVector(0,0,1);		// Relative to this ship.
	// 'baseRot' is rotation around the base/Z axis of the turret.
	// 'gunRot' is gun elevation, rotation around the gun/X axis of the turret.
	// See TurretSightMatrix.
	void TurretMuzzleMatrix(quint16 partId, float baseRot, float gunRot, AuMatrix4x4& muzzleM) const;

	virtual double TurretRange(quint16 which) const;

	inline int TurretRoundsCount(quint16 which) const
	{
		int v;
		m_TurretLock.lockForRead();
		unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.find(which);
		if(ti != m_Turrets.cend())
			v = (*ti).second.RoundsCount();
		else
			v = 0;
		m_TurretLock.unlock();
		return v;
	}

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

	// 'workingAtBdId' is the building that is releasing its workers.
	// 'numToRelease' gets reduced if I was providing any of those workers.
	// false is returned when 'numToRelease' hits 0.
	// 'numToRelease' of 0xffff results in all workers at 'atBdId' being released from every building.
	bool UnemployResidents(const AuId& workingAtBdId, quint16& numToRelease);

	virtual bool UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const;

	virtual void UnjamDoor(unsigned int doorIndex);

	inline void UnlockBayWeaponAmmunition() const
	{
		m_BayLock.unlock();
	}

	virtual void UnlockCargo() const
	{
		m_CargoLock.unlock();
	}

	inline void UnlockConstructionProcess() const
	{
		m_ConstructionLock.unlock();
	}

	inline void UnlockDesignAnalysis() const
	{
		m_DesignLock.unlock();
	}

	inline void UnlockDoorStates() const
	{
		m_DesignLock.unlock();
	}

	inline void UnlockManufacturingProcesses() const
	{
		m_MfgProcLock.unlock();
	}

	inline void UnlockTurretOrdnance() const
	{
		m_TurretLock.unlock();
	}

	inline void UnlockSystems() const
	{
		m_SystemsLock.unlock();
	}

	virtual void Update(QSqlDatabase& db) const;

	// Updates ground connections.
	// Discovers if this bd is connected to a wharf, needed for sea connections.
	// Discovers if this bd is connected to an airport, needed for off world connections.
	void UpdateGroundTradeConnections();

	// Updates off world connections. Call UpdateGroundTradeConnections for all bds in the system first, for airport connections.
	// 'bdsAndCities' contains all the bd buildings and cities in the solar system, including this one.
	void UpdateOffWorldTradeConnections(const DaAtomList& bdsAndCities);

	// Updates sea connections. Call UpdateGroundTradeConnections for all bds on the world first, for wharf connections.
	void UpdateSeaTradeConnections();

	virtual void VehicleParkingSpots(VEHICLE_SPOT_TYPE spotType, QList<AuMatrix4x4>& parkingSpots) const;

	virtual quint16 VehicleTransporterSystemId(VEHICLE_SPOT_TYPE spotType, bool mustBeOn) const;

	virtual void WakeDoctor(const DaCreature& patient);

	// Returns true if we've been in range of an enemy bay weapon within the 'howRecentlyMs' ms of time.
	inline bool WasInEnemyRange(int howRecentlyMs) const
	{
		m_LastInRangeLock.lockForRead();
		bool inRange = (m_LastInEnemyRange.isValid() && m_LastInEnemyRange.elapsed() < howRecentlyMs) ? true : false;
		m_LastInRangeLock.unlock();
		return inRange;
	}

	virtual bool WeaponBayCanFire(quint16 which) const;

	virtual AuPoint WeaponBayEmitLoc(quint16 whichBay, const AuPoint& gunnerLoc) const;

	// Sets the last fired time for a weapon bay that has been fired 'now'.
	// 'emitLoc' is the origin of the beam effect relative to the ship.
	// 'targetLoc' is relative to this building.
	// 'locOnTarget' is relative to the target, for missile targeting.
	// 'ammo' is the ammo type fired, even if it is not consumed.
	// NOTE: m_BayLock is expected to be locked already.
	void WeaponBayFired(quint16 which, const AuPoint& emitLoc, float unitsFired, const AuId& target, const AuPoint& targetLoc,
			const AuPoint& locOnTarget, const AuItem* ammo);

	virtual void WeaponBayIds(QList<quint16>& bayIds) const;

	virtual quint16 WeaponBayNextId(quint16 which) const;

	virtual quint16 WeaponBayPreviousId(quint16 which) const;

	virtual int WeaponBays() const;

	virtual void WeaponBaysWithModule(QList<quint16>& bayIds, COMMODITYID bayModule) const;

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const;

	virtual double Width() const;

	virtual bool WormholeLinesVisible(const QString& roomName) const;

	QString WorstDamagedSystem() const;

	unsigned int WorstSystemDamage() const;

protected:
	void AnimateEffectTextures();

	void AnimateEmpireFlagAndName(AuGLEffects& re);

	void AnimateRenderStates();

	void AnimateWorldMap();

	// DesignLock is locked already when this is called.
	bool AnyTurboLiftDoorsBlockedOpen(quint16 liftId) const;

	int ApplyConstructionLabor(DaPerson* who);

	// Return true if the city inventory contains lCID.
	// This is used to test for satisfaction of a required component that is not consumed by a process. The item may be consumed
	// by this action if it fails its quality test, but not by the process per se.
	// Moves items into m_Tools when needed.
	bool ApplyToolOrVehicle(COMMODITYID cid);

	// 'whoInCoC' is true if 'who' is InChainOfCommand.
	// Returns part id of console if successful. 'stationType', 'opM', 'occPos' and 'emptySeatIndex' are output on success.
	quint16 AttemptConsoleEntry(const DaPerson& who, bool whoInCoC, quint16 consolePartId, const AuScPart& console,
			AuStation& station, const AuPoint& gravityN) const;

	void BalanceRoomAirPressures(bool lifeSupportWorking);

	void ChangeCitizenPopulation();

	// Returns the effect which index for a console. 'consoleId' should identify an engineer station or fire control station.
	quint8 ConsoleEffectWhich(quint16 consoleId) const;

	// Returns the effect which index for a transporter console.
	quint8 ConsoleTransporterEffectWhich(quint16 consoleId) const;

	void DesignGeometryNeeded();

	quint8 DoorState(quint16 door) const;

	// Returns a pointer to an array of effect textures that contains EFF_Count entries, one for each effect.
	const GLuint* EffectTextures() const;

	float EmpireIncomeTax() const;

	float EmpireSalesTax() const;

	// Employs residents of other buildings to construct this building.
	void EmployConstructionWorker();

	// Employs residents of other buildings to work jobs available in this building.
	void EmployWorkers();

	COMMODITYID FarmShrub() const;

	void FindGunners(DaAtomList& gunners) const;

	void GatherSystemsDataSystemsAndDesignLocked(quint8 quality);

	int GetLaborBoost(COMMODITYID cid);

	int GetProductionBoost(COMMODITYID cid);

	void GroundTravellerArrived();

	// Import process components from connected buildings.
	// Returns labor needed. Any import produces at least one point of labor.
	// 'waterSubQ' is non-zero if we can import water to satisfy water in the environment requirment, i.e. for irrigation.
	// 'wantSpecies' is true when the species of components is wanted in the result.
	// 'species' and 'isSentient' are set to the result, if any component with DNA is imported.
	quint32 ImportComponents(bool isRnD, DeProcessComponentArray& components, quint8 minQ, quint8 waterSubQ, bool wantSpecies,
			AuSpecies& species, bool& isSentient);

	// Import electricity once per minute to fill my storage, if I have any capacitor and I am not a power plant.
	void ImportMinutePower(DaSolarSystem* system);

	// Import electricity once per 10 sec to fill my shield capacitor, if I am a shield.
	void ImportShieldPower();

	// Import commodities to fill my storage, if I am a warehouse or have fuel storage.
	void ImportStorage();

	void IncreaseQ();

	void IndigenousCitizenMovedIn(const AuSpecies& species);

	// 'traveller' is 0=newborn, 1=air|space traveller, 2=sea, 3=land
	void NewResident(int traveller);

	void MakeOne(DaWorldLL* world, DaAtom* pWhoDidIt, DeManufacturingProcess& pProcess);

	bool ManufactureFetchMaterials(DaWorldLL* world, DeManufacturingProcess& mp);

	// Returns true if the door is opened.
	bool MoveLift(quint16 liftId, const DaAtomList& lockedAtoms, const QString& nuStop, quint16 doorId);

	void OperateLandingSystems();

	void OperateLifts();

	COMMODITYID OrchardTree() const;

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

	void RenderGameChair(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& loc,
			float rotZ, float rotAroundTable);

	void RenderGameTable(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& loc, float rotZ,
			quint16 whichTable);

	// 'sceneRoom' is the scene atom's room id, if they are aboard, otherwise 0.
	void RenderInteriorDoorVAOs(QHash<quint32, AuScVAO>& vaos, AuAtomRenderInfo& ari, const AuGLSLScBase& shader,
			quint16 sceneRoom, float extAmbR, float extAmbG, float extAmbB);

	void RenderInteriorRoomVAOs(QHash<quint16, AuScVAO>& vaos, AuAtomRenderInfo& ari, const AuGLSLScBase& shader);

	bool RenderLandingSystems(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, bool update);

	void RenderLensVAOs(QHash<quint16, AuScVAO>& vaos, const QHash<quint16, AuLensGlow>& glows, const AuGLSLScBase& shader,
			bool lightOn);

	void RenderLiftVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const AuGLSLScBase& shader);

	void RenderLightSprites(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

	void RenderOwnTextureVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader);

	void RenderPaths(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

	void RenderRoomVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader);

	void RenderShieldBubble(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const DaPerson* sceneAtom);

	bool RenderTractorBeams(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);

	void RenderTurretBaseVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, const AuGLSLScBase& shader);

	void RenderTurretGunVAOs(QHash<quint32, AuScVAO>& vaos, const AuMatrix4x4& model, const AuGLSLScBase& shader);

	void RenderWindowVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader);

	bool RenderVehicleLaunchers(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, bool update);

	void RepairDamage();

	void ResetActiveLaunchers();

	void RunConstructionProcess();

	void RunManufacturingProcesses(bool canOperate);

	// Determine how much this building site influences the altitude of 'loc'.
	// Returns 0 to 1, where 0 is no influence and 1 is full influence.
	// 1 is returned when loc is inside the level terrain area of the building site.
	// 0 is returned when loc is outside the margin of the building site.
	// n is returned when loc is in the margin of the building site, where 0 < n < 1.
	float SiteTerrainInfluence(const AuPoint& loc, AuPoint& siteLoc, quint8& terBlend) const;
	float SiteTerrainInfluence(const AuPoint& loc, AuPoint& siteLoc, bool& hideRocks, int& hideGrass, bool& hideShrubs,
			bool& hideTrees, WORLDTEX_TYPE& pavementType, COMMODITYID& farmShrub, COMMODITYID& orchardTree, quint8& terBlend) const;

	// 0=ok, 1=no room, 2=no animal DNA
	// 'animalSpecies' is a return value, valid on return of 0 or 1.
	int SpawnFarmAnimal(int maxAnimals, AuSpecies& animalSpecies);

	int SpawnVehicle(COMMODITYID cidVehicle, quint8 quality);

	inline quint32 StateVAOKey(quint16 partId, quint8 state) const
	{
		return (state << 16) | partId;
	}

	// Returns the count of the offered commodity that will fit in this city's inventory.
	// Returns 0 if none of the offered commodity will fit.
	// Never returns a number greater than the current offer.Count().
	quint32 TestFitCommodity(const AuItem& offer) const;

	// Unemploys workers at this building.
	void UnemployWorkers();

	// Gets all known patents from the empire that are useful to this industry into my patents list.
	void UpdateMyPatents();

	bool UpdateSystemsHealthSystemsLocked();

	void UpdateSystemsPowerSystemsLocked();

	bool VehicleParkingSpot(VEHICLE_SPOT_TYPE spotType, AuMatrix4x4& parkPosM);

	bool VehicleParkingSpot(VEHICLE_SPOT_TYPE spotType, AuMatrix4x4& parkPosM, const AuBdDesign* scd, DaWorldLL* llworld,
			DaAtomList& vehiclesAtWorld);

	void WindowSurfaceGlow(const QString& windowName, float& glowR, float& glowG, float& glowB) const;

public:
	// These interfaces enable commodities to be imported between DaBd objects.
	virtual void DischargeStoredPower(quint32 units);

	virtual const AuItemHash* LockCommoditiesForRead();

	virtual AuItemHash* LockCommoditiesForWrite();

	virtual bool RemoveCommodityNoLock(const AuItem& take, quint32 howMany);

	virtual quint32 StoredPowerLevel() const;

	quint32 TakeMoneyMfg(quint32 need);

	quint32 TakeMoneyRnD(quint32 need);

	virtual void UnlockCommoditiesForRead();

	virtual void UnlockCommoditiesForWrite(bool anyChange);

public:	// Spacecraft Factory
	void ClearProcess(DeManufacturingProcess& mp);

	inline quint8 CompanyMaxQ() const
	{
		return m_CompanyMaxQ;
	}

	inline bool CompanyOk() const
	{
		return m_CompanyOk;
	}

	inline quint32 DefaultDesignId() const
	{
		return m_DefaultDesign.DesignId();
	}

	inline const QString& DefaultDesignName() const
	{
		return m_DefaultDesign.DesignName();
	}

	inline quint8 DefaultQuality() const
	{
		return m_DefaultDesign.Quality();
	}

	void DeleteOrder(quint32 designId);

	inline const QList<AuMakeDesign>& DesignQueue() const
	{
		return m_DesignQueue;
	}

	inline bool IsWaitingToSpawn() const
	{
		return m_MakingShip.IsReadyToSpawn();
	}

	inline quint8 MakeMinQ() const
	{
		return m_MakeMinQ;
	}

	inline quint32 MakingDesignId() const
	{
		return m_MakingShip.DesignId();
	}

	inline const QString& MakingDesignName() const
	{
		return m_MakingShip.DesignName();
	}

	// Sends mail notification to the new owner of a spacecraft that construction is completed at this world.
	void NotifyNewSpacecraftOwner(const AuId& ownerId, const QString& ownerName, quint32 ownerEmpire, const DaVessel* ship) const;

	void QueueMakeShip(quint32 designId, const QString& designName, quint8 designQ, const AuId& ownerId, const QString& ownerName,
			quint32 ownerEmpire);

	bool SelectProcess(DeManufacturingProcess& pProcess);

	inline void SetCompanyPolicy(bool companyOk, quint8 companyMaxQ)
	{
		Q_ASSERT(companyMaxQ);
		m_CompanyOk = companyOk;
		m_CompanyMaxQ = companyMaxQ;
	}

	void SetDefaultMakeShip(quint32 designId, const QString& designName, quint8 designQ);

	inline void SetMakeMinQ(quint8 minQ)
	{
		Q_ASSERT(minQ);
		m_MakeMinQ = minQ;
	}

	void SpacecraftProcessCanceled(AuId& buyerId, QString& buyerName, double& buyerRefund);

	// Returns:
	//   0 = ok
	//   1 = blocked by unanalyzed ship
	//   2 = Cannot spawn another station in orbit or replace an existing station.
	//   3 = Cannot spawn another nu ship in orbit or replace an existing nu ship.
	//   4 = Orbit spawn location could not be found.
	//   5 = No station at which to spawn.
	//   6 = No ground location at which to spawn.
	int SpawnSc(const AuScReport& ship, quint8 initQuality, const AuId& ownerId, const QString& ownerName, quint32 ownerEmpire);

	void TerminateProcess(AuId& buyerId, QString& buyerName, double& buyerRefund);

	inline void TerminateSpawnWait()
	{
		m_MakingShip = AuMakeDesign();
	}

	void TryToSpawnShip();

public:	// Broker
	void FillShipments(bool shipNow);

	// Beware this is the same lock as the inventory lock.
	inline const AuShipmentList& LockShipmentsForRead() const
	{
		m_CargoLock.lockForRead();
		return m_Shipments;
	}

	// Beware this is the same lock as the inventory lock.
	inline const AuTradeAgreementList& LockTradeAgreementsForRead() const
	{
		m_CargoLock.lockForRead();
		return m_TradeAgreements;
	}

	// Returns: 0 if the shipment is created.
	//          1 Broker not found.
	//          2 Broker destroyed.
	//          3 Broker no levels.
	//          4 Broker at max.
	//          5 City at max.
	//          6 must not be insystem destination
	int ShipmentCreate(const AuShipment& shipment);

	// Returns: 0 if the shipment is deleted.
	//          1 Broker not found.
	//          2 Broker destroyed.
	//          3 Broker no levels.
	//          4 Shipment not found.
	int ShipmentDelete(const AuShipment& shipment);

	// Returns payment.
	virtual quint32 ShipmentDeliver(const AuItemTEUContainer& shipment);

	// 'atDest' is true when the destination city is accepting the trade agreement.
	// 'atDest' is false when the origin city is accepting the trade agreement.
	bool TradeAgreementAccept(const AuTradeAgreement& ta, bool atDest);

	// Returns: 0=ok, 1=I am not a broker, 2=proposal not found, 3=counter could not be added, 4=proposal not a proposal,
	// 5=counter not a proposal, 6=cannot counter my own proposal
	int TradeAgreementCounter(const AuTradeAgreement& proposal, const AuTradeAgreement& counter);

	// Returns: 0=ok, 1=I am no broker, 3=no offices, 4=shipper limit reached, 5=ta could not be added, 6=destination in my system
	int TradeAgreementCreate(const AuTradeAgreement& ta);

	// This is what happens at the far end when a trade agreement is deleted by somebody. It is effectively deleted but the status
	// shows what happened to it.
	// Returns true if it is deactivated.
	bool TradeAgreementDeactivate(const AuTradeAgreement& ta, AuTradeAgreement::STATE nuState);

	// Returns: 0=deleted at source, 1=deleted at dest, 2=not a broker, 3=ta not found
	int TradeAgreementDelete(const AuTradeAgreement& ta);

	// Returns the result: 0=ok, 1=not on a planet?, 2=no broker, 3=broker full, 4=no room for more proposals
	int TradeAgreementDeliver(const AuTradeAgreement& ta);

	// The remote broker informs me when it is ready to ship a shipment under the trade agreement 'ta'. That notification arrives
	// here. If I am not ready, I just ignore it. If I am ready, I will emit a TradeShipped signal and ship the shipment with a
	// ShipIt signal.
	// Returns true if the 'ta' is mine.
	bool TradeReadyToShipNow(AuTradeAgreement ta);

	// This is where I receive my response to my TradeReadyToShip signal when the remote broker is also ready. They signalled their
	// shipment was shipped and they shipped the shipment. I must now ship my corresponding shipment to them.
	// Returns true if the 'ta' is mine.
	bool TradeShippedNow(AuTradeAgreement ta);

	inline void UnlockShipments() const
	{
		m_CargoLock.unlock();
	}

	inline void UnlockTradeAgreements() const
	{
		m_CargoLock.unlock();
	}

protected:	// Comm
	void RxAcceptRefit(const AuCommAcceptRefit* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxAcceptUpgradeInstall(const AuCommAcceptUpgradeInstall* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxAuction(const AuCommAuction* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			POLITICAL_STANCE stanceToSender, DaPerson* person);

	void RxBeamOver(const AuCommBeamOver* msg, DaVessel* ship, DaPerson* person, const AuId& sectorId,
			AuCommStdChannel::RANGE range);

	void RxBeamVehicleAtom(const AuCommBeamVehicleAtom* msg, DaVessel* ship, AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehicleCoordinate(const AuCommBeamVehicleCoordinate* msg, DaVessel* ship, AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehiclePlanet(const AuCommBeamVehiclePlanet* msg, DaVessel* ship, AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehicleShip(const AuCommBeamVehicleShip* msg, DaVessel* ship, DaPerson* person,
			AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehicleTransporter(const AuCommBeamVehicleTransporter* msg, DaVessel* ship, DaPerson* person,
			AuCommStdChannel::RANGE limitedRange);

	void RxBid(const AuCommBid* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range, DaPerson* person,
			POLITICAL_STANCE stanceToSender, bool hasTransporter, bool inTransporterRange);

	void RxBidBarter(const AuCommBidBarter* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			DaPerson* person, bool hasTransporter, bool inTransporterRange, POLITICAL_STANCE stanceToSender);

	void RxBoardCrew(const AuCommBoardCrew* msg, quint32& countSupplied, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender, bool hasTransporter,
			bool inTransporterRange);

	void RxBoardCryo(const AuCommBoardCryo* msg, quint32& countSupplied, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender, bool hasTransporter,
			bool inTransporterRange);

	void RxBoardOfficers(const AuCommBoardOfficers* msg, quint32& countSupplied, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, bool hasTransporter, bool inTransporterRange);

	void RxBoardPassengers(const AuCommBoardPassengers* msg, quint32& countSupplied, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender, bool hasTransporter,
			bool inTransporterRange);

	void RxBidPickupShipment(const AuCommBidPickupShipment* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, bool hasTransporter, bool inTransporterRange);

	void RxBoardTroops(const AuCommBoardTroops* msg, quint32& countSupplied, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, bool hasTransporter, bool inTransporterRange);

	void RxDebarkCryo(const AuCommDebarkCryo* msg, quint32& countSupplied, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, bool hasTransporter, bool inTransporterRange);

	void RxDebarkPassengers(const AuCommDebarkPassengers* msg, quint32& countSupplied, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, bool hasTransporter, bool inTransporterRange);

	void RxDiplomaticMessage(const AuCommDiplomaticMessage* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxForSale(const AuCommForSale* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			POLITICAL_STANCE stanceToSender, DaPerson* person);

	void RxHailRequest(const AuCommHailRequest* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			DaPerson* person, const AuId& systemId);

	void RxOfferTEU(const AuCommOfferTEU* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			DaPerson* person, bool hasTransporter, bool inTransporterRange);

	void RxRequestExterminator(const AuCommRequestExterminator* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxRequestHouseKeeping(const AuCommRequestHouseKeeping* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxRequestInventory(const AuCommRequestInventory* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			DaPerson* person, bool inTransporterRange, POLITICAL_STANCE stanceToSender);

	void RxRequestRefit(const AuCommRequestRefit* msg, DaVessel* ship, const AuId& sectorId, AuCommStdChannel::RANGE range,
			DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxRequestRepairAll(const AuCommRequestRepairAll* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxRequestRepairSystem(const AuCommRequestRepairSystem* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender);

	void RxRequestUpgradeSystem(const AuCommRequestUpgradeSystem* msg, DaVessel* ship, const AuId& sectorId,
			AuCommStdChannel::RANGE range, DaPerson* person, POLITICAL_STANCE stanceToSender);

	// 'ship' may be null
	void SolicitSales(DaPerson* person, DaVessel* ship, COMMODITYFILTER filter, COMMODITYID cid, const AuCommPacket& msg,
			const AuId& sectorId, POLITICAL_STANCE stanceToSender);

	void SolicitShipments(DaPerson* person, DaVessel* ship, const AuCommPacket& msg, const AuId& sectorId);

	// inIndustry is IID_Count when trading spacecraft cargo.
	void TxForSaleChat(const AuCommPacket& msg, const AuId& sectorId, COMMODITYFILTER filter, bool freeStuff, quint32 senderEmpire,
			COMMODITYID cid);

private:
	// Returns the number of citizens ready to depart.
	quint32 BaseCrew(const DaWorld* world) const;
	quint32 BaseTroops(const DaWorld* world) const;

	// Removes citizens from buildings accessible to this airport by ground that are part of my empire.
	// Fills 'speciesRemoved' with DNA from each build that people are taken from. May be less than 'howMany' on return.
	// Caller must delete the 'speciesRemoved' pointers.
	void BaseDecreaseCrew(DaWorld* world, quint32 howMany, QList<AuSpecies>& speciesRemoved);
	void BaseDecreaseTroops(DaWorld* world, quint32 howMany, QList<AuSpecies>& speciesRemoved);

	// Returns the number of citizens ready to depart.
	quint32 AirportCrew(const DaWorld* world) const;
	quint32 AirportTroops(const DaWorld* world) const;

	// Removes citizens from buildings accessible to this airport by ground that are part of my empire.
	// Fills 'speciesRemoved' with DNA from each build that people are taken from. May be less than 'howMany' on return.
	// Caller must delete the 'speciesRemoved' pointers.
	void AirportDecreaseCrew(DaWorld* world, quint32 howMany, QList<AuSpecies>& speciesRemoved);
	void AirportDecreaseTroops(DaWorld* world, quint32 howMany, QList<AuSpecies>& speciesRemoved);

	quint32 AirportEmptyHomes(const DaWorld* world) const;
	quint32 BaseEmptyHomes(const DaWorld* world) const;

	// Increases the population of this city by one.
	// Adds one citizen to a building accessible to this airport by ground that are part of my empire.
	bool AirportIncreasePopulation(const AuSpecies& species);
	bool BaseIncreasePopulation(const AuSpecies& species);

	// Increases the population of this city by one.
	// 'sourceName' is used for outbreak event msg.
	bool AirportIncreasePopulation(const DaCreature* creature, const QString& sourceName);
	bool BaseIncreasePopulation(const DaCreature* creature, const QString& sourceName);

	quint32 AirportPopulation() const;
	quint32 BasePopulation() const;
	quint32 TransientPopulation() const;	// Only non-zero for airport terminal and base air terminal

	// Returns total homes, including those allocated for transient population.
	quint32 AirportTransientHomes(const DaWorld* world) const;
	quint32 BaseTransientHomes(const DaWorld* world) const;

	double BrokerAccountBalance() const;

	void BrokerCreditAccount(double amount, AuBank::CATEGORY cat);

	void BrokerDebitAccount(double amount, AuBank::CATEGORY cat);

	// Returns true if my storage is available to brokers for filling with 'storeCID'.
	bool BrokerFillsMyStorage(COMMODITYID storeCID) const;

	// Fills 'items' with all cargo available to this broker, tallied from all connected warehouses and storage tanks.
	// 'empireAsking' is used to check commerce restrictions.
	// Used by brokers.
	void BaseInventoryAvailable(AuItemHash& items, quint32 empireAsking) const;
	void BrokerInventoryAvailable(AuItemHash& items, quint32 empireAsking) const;

	void BaseRemoveInventory(const AuItem& item);
	void BrokerRemoveInventory(const AuItem& item);

	virtual quint32 BuyCost(const AuId& idFromWho, const AuItem& item);

	// Lists air terminals in their community.
	void BaseAirTerminals(DaAtomList& terminals) const;
	void BrokerAirTerminals(DaAtomList& terminals) const;

	// Add 'iNew' to warehouses and storage tanks that are accessible to this broker.
	void BaseAddCommodity(const AuItem& iNew);
	void BrokerAddCommodity(const AuItem& iNew);

	quint32 BaseRawCount(COMMODITYID cid, quint8 minQ) const;
	quint32 BrokerRawCount(COMMODITYID cid, quint8 minQ) const;

	quint32 BaseRawRemove(COMMODITYID cid, quint8 minQ, quint32 howMany);
	quint32 BrokerRawRemove(COMMODITYID cid, quint8 minQ, quint32 howMany);

	quint32 BaseTestFitCommodity(const AuItem& offer) const;
	quint32 BrokerTestFitCommodity(const AuItem& offer) const;

	void CallFireStation();

	void CreateFenceDeviceObjects(AuGLEffects& re, QHash<quint32, AuSegmentArray>& fenceSegs,
			const QColor themeColors[AuScFace::THEME_Count]);

	DaNpc* CreateWorkerAtStation(quint16 stationId, AuStation& station, DaAtomList& existingWorkers);

	// Returns true if I am destroyed.
	bool DamageSystem(const AuCombatResult& result, bool structural, quint32 hitPoints, int wpn1or2, const AuCombatWeapon& weapon,
			const QString& attackerName, bool protectedFromDestruction);

	// Returns the number of days this building has been abandoned. May be 0.
	int DaysAbandoned() const;

	// Calculate demand cost based on which context is appropriate.
	// The demand cost is the local cost of a commodity, calculated from the quality cost and modified by quality and local demand.
	// The quality cost 'qualCost' is the base commodity cost adjusted for quality.
	// 'idToWho' is a unit atom. It will be NULL when called by a retail console in town to display prices.
	quint32 DemandCost(const AuId& idToWho, const AuItem& item) const;

	// Cost of one unit of the item when the transaction occurs in this town.
	quint32 DemandCost(const AuItem& item) const;

	// Returns the estimated cost of repairing the ship at this city.
	// 'missing' indicates which commodities are not available for repairs if even short by one.
	// 'repairSecs' is the amount of time needed to conduct the repairs.
	// 'repairQ' is the minimum quality of parts to use.
	unsigned int EstimateRepairCost(const DaVessel* ship,
			QList<COMMODITYID>& noQ, QList<COMMODITYID>& shortQ, QList<COMMODITYID>& noSupply, QList<COMMODITYID>& enough,
			quint16& repairSecs, quint8 repairQ, quint16& workers) const;

	void ExpandPirateBase();

	void FixTurboLiftAirBug();

	// Returns true if I am ok for my environment.
	bool IsAdequateForEnvironment() const;

	// Assumes this building is already known to be on the same world and connected by ground to the broker.
	bool InCommunityBroker(quint32 brokerEmpireId) const;

	// Returns true if the process is ok for this building to run based on environment alone.
	bool IsEnvironmentalProcessOk(quint16 mpid) const;

	// Returns true if 'loc' is within 'nearDist' of any of my fence segments at ground level.
	bool IsNearFence(const AuPoint& loc, double nearDist) const;

	// Returns true if this building cannot be destroyed due to game rules.
	bool IsProtectedFromDestruction() const;

	void KillBuilding();

	// Returns: 0=none, 1=civilian wharf, 2=military wharf, 3=civilian airport, 4=military airport
	int NavBeacon(const DaPerson* sceneAtom, QList<AuMatrix4x4>& parkMs) const;

	void ObserveAsteroids();

	// Fills my storage to the max with a commodity.
	void PirateFillCommodity(quint32 empireId, COMMODITYID cid, quint8 quality);

	void PirateFillResidents(quint32 empireId, DaWorldLL* llworld);

	// NOTE: Expects m_PriceOfferLock to be locked on entry.
	QMap<DeInvItemId, AuPriceOffer>::const_iterator PriceOffer(const AuId& whoId, const AuItem& item);

	void RecalcSystemsHealthMax();

	void RenderScShader(AuGLSLScBase* shader, AuAtomRenderInfo& ari, const AuMatrix4x4& model, const DaPerson* sceneAtom);
	void RenderScShaderBlend(AuGLSLScBase* shader, AuAtomRenderInfo& ari, const AuMatrix4x4& model,
			const DaPerson* sceneAtom);

	void RenderSelectedIndicator(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, const AuMatrix4x4& model);

	// Returns cost to repair a commodity, based on commodity quality availability in my inventory.
	// 'noQ' set to which commodities are not available at the requested 'repairQ' or above.
	// 'shortQ' set to which commodities are in short supply at the requested 'repairQ' or above.
	// 'noSupply' set to which commodities are not available at any quality.
	// 'enough' set to which commodities are in short supply at any quality.
	// 'repairSecs' is the amount of time needed to conduct the repairs.
	unsigned int RepairCost(const AuItemHash& parts, COMMODITYID cid, unsigned int minQ, unsigned int count,
			QList<COMMODITYID>& noQ, QList<COMMODITYID>& shortQ, QList<COMMODITYID>& noSupply, QList<COMMODITYID>& enough,
			quint16& repairSecs) const;

	// Returns cost to repair as much of a commodity as possible, based on commodity quality availability in my inventory.
	// 'repairSecs' is the amount of time needed to conduct the repairs.
	unsigned int RepairCostPossible(const AuItemHash& parts, COMMODITYID cid, unsigned int techLevel, unsigned int count,
			quint16& repairSecs) const;

	void ReportAttacked(const QString& attackerName, quint32 attackerEmpire, const DaSolarSystem* system);

	// Selects a nonstructural system that has hit points.
	// Nonstructural systems are reported as "Equipment" on the design analysis report.
	// If no structural system has hit points, SYS_Count is returned.
	AuSysId SelectNonstructuralSystemToDamage() const;

	// Selects a structural system that has hit points.
	// If no structural system has hit points, SYS_Count is returned.
	AuSysId SelectStructuralSystemToDamage() const;

	void SpreadDisease(const DaCreature* creature, const QString& sourceName);

	// Surrenders this building to 'toEmpire'.
	void SurrenderBuilding(quint32 surrenderingEmpire, quint32 toEmpire);

	// Retrieves a list of all system ids, into 'sysIds'.
	void SystemIds(QSet<AuSysId>& sysIds, bool structural) const;

	// Returns species key of a transient taken from this building. Empty string if none.
	AuSpecies TakeTransient();

	void TallyCommunityVehicles();

	// Returns true if the environmental factor 'cid' is present at my world.
	bool TestEnvironmentalFactor(const DaWorldLL& world, COMMODITYID cid) const;

	QList<quint16> TransporterRoomIds() const;

	virtual void TxReplyChat(const QString& replyText, const AuCommPacket& msg, AuCommStdChannel::RANGE range);

	float WeaponRadarRange();

	void WorkerOperateShield();

	void WorkerOperateTurret();

	void WorkerOperateWeapon();
};

#endif // DaBd_H
