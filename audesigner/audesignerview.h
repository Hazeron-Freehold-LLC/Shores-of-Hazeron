//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerview.h,v 1.67 2022/05/04 21:37:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerView_H
#define AuDesignerView_H

#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "audesigner.h"
#include "audnabase.h"
#include "auether.h"
#include "aupose.h"
#include "auroster.h"
#include "auselectedunit.h"
#include "auview.h"
#include "dadesignbase.h"
#include <QPoint>

class AuAtomRenderInfo;
class AuDialog;
class AuGameSettings;
class AuGLModelCache;
class AuGLModelMaker;
class AuItemManipulator;
class DaGalaxy;
class DaItem;
class DaWorldLL;
class DdCommand;
class DiControlInputArray;
class DtGL;
class DtTarget;
class QImage;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QTimer;
class QWheelEvent;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuForageHistory
{
private:
	COMMODITYID							m_CID;					// Top bit will be set when it came from a tree.
	AuPoint								m_GlobeLoc;				// Location relative to globe when on a DaWorldLL.
	AuId								m_TerrainId;			// DaWorldLL id.
	AuElapsedTimer						m_When;

public:
	inline AuForageHistory(COMMODITYID cid, const AuId& terrainId, const AuPoint& globeLoc)
	{
		m_CID = cid;
		m_TerrainId = terrainId;
		m_GlobeLoc = globeLoc;
		m_When.start();
	}

	inline AuForageHistory()
	{
		m_CID = CID_Invalid;
	}

	inline AuForageHistory(const AuForageHistory& c)
	{
		m_CID = c.m_CID;
		m_GlobeLoc = c.m_GlobeLoc;
		m_TerrainId = c.m_TerrainId;
		m_When = c.m_When;
	}

	inline const AuForageHistory& operator=(const AuForageHistory& c)
	{
		m_CID = c.m_CID;
		m_GlobeLoc = c.m_GlobeLoc;
		m_TerrainId = c.m_TerrainId;
		m_When = c.m_When;

		return *this;
	}

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)(m_CID & 0x7fff);
	}

	inline bool IsExpired() const
	{
		// Water can be foraged quicker than other stuff.
		if(CommodityId() == CID_Water)
			return m_When.elapsed()/d_MSecPerSec >= 15;			// Allow 15 seconds to fill a bottle.
		else
			return m_When.elapsed()/d_MSecPerMin >= 10;			// Other things take 10 minutes to collect from the same spot again.
	}

	// Returns true if this forage history record prevents a forage attempt.
	// IsExpired has been checked already. See usage.
	inline bool PreventsForageOnGlobe(const AuForageHistory& forageAttempt) const
	{
		if(m_CID != forageAttempt.m_CID)
			return false;
		if(m_TerrainId != forageAttempt.m_TerrainId)
			return false;
		if(m_GlobeLoc.Distance2(forageAttempt.m_GlobeLoc) > 20*20)
			return false;

		// The forage attempt is prevented.
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuSprite is used for the little flying things on the screen during ShowWaitIndicator.

class AuSprite
{
public:
	int			m_pic;	// Sprite image in texture, 0-7
	float		m_rot;	// Sprite rotation
	float		m_xpos;
	float		m_ypos;
	float		m_zpos;

	AuSprite();

	void Move(float speed);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuViewButton
{
public:
	// NOTE: Do not change the values of these enums. They are stored in the client's settings.
	enum VIEWBUTTON
	{
		VB_Invalid,
		VB_Agent,
		VB_Architect,
		VB_ArenaExit,
		VB_ArenaRecall,
		VB_ArenaStatus,
		VB_Bio,
		VB_Cargo,
		VB_SaveSolitaire,
		VB_Comm,
		VB_UNUSED3,
		VB_UNUSED5,
		VB_Exit,
		VB_FullScreen,
		VB_Government,
		VB_Help,
		VB_HideWindows,
		VB_Gear,
		VB_Locator,
		VB_Mail,
		VB_UNUSED6,
		VB_Programmer,
		VB_Recall,
		VB_Settings,
		VB_Spacecraft,
		VB_UnitOrders,
		VB_About,
		VB_Avatar1,
		VB_Avatar2,
		VB_Avatar3,
		VB_Avatar4,
		VB_Avatar5,
		VB_Avatar6,
		VB_SpacecraftAccount,
		VB_SpacecraftBounties,
		VB_SpacecraftDamage,
		VB_SpacecraftMission,
		VB_SpacecraftPolicy,
		VB_SpacecraftRoster,
		VB_CargoHold,
		VB_CargoOrdnance,
		VB_CargoTransfer,
		VB_LocatorCompass,
		VB_LocatorGPS,
		VB_LocatorStarMap,
		VB_LocatorWorldMap,
		VB_GovernAnnouncements,
		VB_GovernCities,
		VB_GovernCitizens,
		VB_GovernCommerce,
		VB_GovernDiplomacy,
		VB_GovernPolicy,
		VB_UNUSED4,
		VB_GovernTaxes,
		VB_Forum,
		VB_Wiki,
		VB_WarDimensionStandings,
		VB_UNUSED8,
		VB_Avatars,
		VB_CargoManifest,
		VB_SpacecraftDesignExchange,
		VB_HazeronHome,
		VB_Minion,
		VB_PlayersOnline,
		VB_Videos,
		VB_BdDesignExchange,
		VB_Building,
		VB_ConstructionBd,
		VB_GovernPatents,
		VB_UNUSED1,
		VB_SpacecraftShipments,
		VB_GearHandCraft,
		VB_LocatorSystemScan,
		VB_SpacecraftSpecs,
		VB_UNUSED7,
		VB_COUNT						// LAST ITEM
	};

public:
	bool								m_Enabled;
	VIEWBUTTON							m_Id;
	VIEWBUTTON							m_Parent;				// For sub buttons.
	AuPoint								m_ScreenPos;			// GL unit coord, as if screen is -1 to 1 in both X and Y axes.
	QString								m_Text;					// Overrides default text.
	GLuint								m_Texture;
	QString								m_Tip;					// Overrides default tip.

public:
	AuViewButton(VIEWBUTTON buttonId)
	{
		m_Enabled = true;
		m_Id = buttonId;
		m_Parent = VB_Invalid;
		m_Texture = 0;
	}

	AuViewButton(VIEWBUTTON buttonId, const QString& tip)
	{
		m_Enabled = true;
		m_Id = buttonId;
		m_Parent = VB_Invalid;
		m_Texture = 0;
		m_Tip = tip;
	}

	AuViewButton(VIEWBUTTON buttonId, const QString& text, const QString& tip)
	{
		m_Enabled = true;
		m_Id = buttonId;
		m_Parent = VB_Invalid;
		m_Text = text;
		m_Texture = 0;
		m_Tip = tip;
	}

	AuViewButton(const AuViewButton& c)
	{
		m_Enabled = c.m_Enabled;
		m_Id = c.m_Id;
		m_Parent = c.m_Parent;
		m_ScreenPos = c.m_ScreenPos;
		m_Text = c.m_Text;
		m_Texture = c.m_Texture;
		m_Tip = c.m_Tip;
	}

	const AuViewButton& operator=(const AuViewButton& c)
	{
		m_Enabled = c.m_Enabled;
		m_Id = c.m_Id;
		m_Parent = c.m_Parent;
		m_ScreenPos = c.m_ScreenPos;
		m_Text = c.m_Text;
		m_Texture = c.m_Texture;
		m_Tip = c.m_Tip;
		return *this;
	}

	inline QString Image() const
	{
		return Image(m_Id);
	}

	static QString Image(VIEWBUTTON vb);

	// True if this button is hidden right now because it is not available to the user.
	// 'hasEmpire' is true if the scene atom has an empire, false if they are a free avatar.
	bool IsHidden(SCENE_CONTEXT sceneContext, bool hasEmpire) const;

	inline bool IsSub() const
	{
		return m_Parent != VB_Invalid;
	}

	// True if this button can be painted at all.
	bool IsVisible() const;

	// Name used as the button text.
	inline QString Name() const
	{
		if(!m_Text.isEmpty())
			return m_Text;
		return Name(m_Id);
	}

	static QString Name(VIEWBUTTON vb);

	// Name in settings is longer to group sub items.
	inline QString SettingsName() const
	{
		return SettingsName(m_Id);
	}

	static QString SettingsName(VIEWBUTTON vb);

	inline QString TextureName() const
	{
		return QString("VB%1").arg(m_Id);
	}

	inline QString Tip() const
	{
		if(!m_Tip.isEmpty())
			return Tip(m_Tip);
		return Tip(TipBase(m_Id));
	}

	// Returns the base tip, with %2 where the key command should be plugged in.
	static QString TipBase(VIEWBUTTON vb);

	// Replaces %2 in tipBase with the current key command binding and returns the result.
	QString Tip(const QString& tipBase) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuRenderMsg is used to track AuCommPackets as they arrive.

#define s_TitleFadeTime					15						// Time before title narrative fades.
#define s_MessageFadeTime				10						// Time before title narrative fades.

class AuRenderMsg
{
private:
	QString								m_Channel;
	AuElapsedTimer						m_DisplayTime;			// UTC time this message was first rendered on the screen.
	QString								m_FromName;
	AuCommStdChannel::RANGE				m_Range;				// Actual sender range, as determined by the server.
	AuCommStdChannel::SCOPE				m_Scope;				// Message transmission scope.
	POLITICAL_STANCE					m_StanceTowardTarget;	// For SpotTargetOrTerrain messages.
	bool								m_TargetIsCivilian;		// For SpotTargetOrTerrain messages.
	QString								m_Text;
	bool								m_ToneSounded;
	AuCommPacket::TYPE					m_Type;

public:
	inline AuRenderMsg(const AuCommPacket* msg)
	{
		m_Channel = msg->ChannelName();
		m_FromName = msg->FromName();
		m_Range = msg->ReceptionRange();;
		m_Scope = msg->Scope();

		const AuCommSpotTargetOrTerrain* msgSpot = dynamic_cast<const AuCommSpotTargetOrTerrain*>(msg);
		if(msgSpot)
		{
			m_StanceTowardTarget = msgSpot->StanceTowardTarget();
			m_TargetIsCivilian = msgSpot->IsCivilian();
		}
		else
		{
			m_StanceTowardTarget = POL_Neutral;
			m_TargetIsCivilian = false;
		}

		m_Text = msg->Text();
		m_ToneSounded = false;
		m_Type = msg->Type();
	}

	inline AuRenderMsg()
	{
		m_StanceTowardTarget = POL_Neutral;
		m_Range = AuCommStdChannel::Close;
		m_Scope = AuCommStdChannel::Everyone;
		m_TargetIsCivilian = false;
		m_ToneSounded = false;
		m_Type = AuCommPacket::CommTypes;	// This is the "invalid" value.
	}

	inline AuRenderMsg(const AuRenderMsg& c)
	{
		m_Channel = c.m_Channel;
		m_DisplayTime = c.m_DisplayTime;
		m_FromName = c.m_FromName;
		m_Range = c.m_Range;
		m_Scope = c.m_Scope;
		m_StanceTowardTarget = c.m_StanceTowardTarget;
		m_Text = c.m_Text;
		m_TargetIsCivilian = c.m_TargetIsCivilian;
		m_ToneSounded = c.m_ToneSounded;
		m_Type = c.m_Type;
	}

public:	// Operators
	inline const AuRenderMsg& operator=(const AuRenderMsg& c)
	{
		m_Channel = c.m_Channel;
		m_DisplayTime = c.m_DisplayTime;
		m_FromName = c.m_FromName;
		m_Range = c.m_Range;
		m_Scope = c.m_Scope;
		m_StanceTowardTarget = c.m_StanceTowardTarget;
		m_Text = c.m_Text;
		m_TargetIsCivilian = c.m_TargetIsCivilian;
		m_ToneSounded = c.m_ToneSounded;
		m_Type = c.m_Type;
		return *this;
	}

public:	// Methods
	inline const QString& ChannelName() const
	{
		return m_Channel;
	}

	QColor Color() const;

	inline bool DecayMessage() const
	{
		int secsUp = SecondsOnDisplay();
		if(!secsUp)
			return false;	// Message either hasn't been displayed yet or hasn't been displayed long enough to worry about decay.

		// Different types of messages decay at different rates.
		if(AuCommPacket::IsNarratorMessage(m_Type))
		{
			if(m_Type == AuCommPacket::NarrateTitle)
			{
				if(secsUp > s_TitleFadeTime)
					return true;	// Narrator titles decay after 10 seconds.
			}
			else
			{
				if(secsUp > s_MessageFadeTime)
					return true;	// Narrator messages decay after 30 seconds.
			}
		}
		else
		{
			if(secsUp > s_MessageFadeTime)
				return true;		// Regular chat messages decay after 10 seconds.
		}
		return false;	// This message has not completely decayed yet.
	}

	inline const QString& FromName() const
	{
		return m_FromName;
	}

	inline bool IsTargetCivilian() const
	{
		return m_TargetIsCivilian;
	}

	inline int MSecsOnDisplay() const
	{
		// If the display time is null, this message has not been displayed yet.
		if(!m_DisplayTime.isValid())
			return 0;
		return m_DisplayTime.elapsed();
	}

	QString OnScreenMessage() const;

	inline AuCommStdChannel::RANGE Range() const
	{
		return m_Range;
	}

	inline int SecondsOnDisplay() const
	{
		// If the display time is null, this message has not been displayed yet.
		if(!m_DisplayTime.isValid())
			return 0;
		return m_DisplayTime.elapsed()/d_MSecPerSec;
	}

	inline void SetDisplayed()
	{
		// Only set the displayed time the first time this function is called. Otherwise the message never decays.
		if(!m_DisplayTime.isValid())
			m_DisplayTime.start();
	}

	inline void SetText(const QString& text)
	{
		m_Text = text;
	}

	inline void SetType(AuCommPacket::TYPE type)
	{
		m_Type = type;
	}

	inline POLITICAL_STANCE StanceTowardTarget() const
	{
		return m_StanceTowardTarget;
	}

	inline const QString& Text() const
	{
		return m_Text;
	}

	// Return value is a SOUND_EFFECT
	int ToneToPlay();

	inline AuCommPacket::TYPE Type() const
	{
		return m_Type;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDESIGNER_DLL AuDesignerView : public QOpenGLWidget, public AuView
{
	Q_OBJECT
public:
	enum CONTROLINPUT
	{
		CI_Title = 0,						// Used for items that form titles in the setting UI.
		CI_ConsoleUI,
		CI_EndCommand,
		CI_FirstThirdPersonViewToggle,
		CI_FreeCamera,
		CI_HudIntensityDecrease,
		CI_HudIntensityIncrease,
		CI_InputModeHandTemporary,
		CI_InputModeToggle,
		CI_MessageIntensity,
		CI_ScreenElementsToggle,
		CI_ScreenShot,
		CI_ScreenVideo_REMOVED,
		CI_SpotTargetComm,
		CI_SpotTargetLog,
		CI_SquadCreate0,
		CI_SquadCreate1,
		CI_SquadCreate2,
		CI_SquadCreate3,
		CI_SquadCreate4,
		CI_SquadCreate5,
		CI_SquadCreate6,
		CI_SquadCreate7,
		CI_SquadCreate8,
		CI_SquadCreate9,
		CI_SquadOrderAttackTarget,
		CI_SquadOrderBattleStation,
		CI_SquadOrderCloseDoor,
		CI_SquadOrderComeHere,
		CI_SquadOrderDropRider,
		CI_SquadOrderEnterSurgery,
		CI_SquadOrderFollowMe,
		CI_SquadOrderFollowTarget,
		CI_SquadOrderGarrison,
		CI_SquadOrderGetIn,
		CI_SquadOrderGetItem,
		CI_SquadOrderGetOut,
		CI_SquadOrderGoToTarget,
		CI_SquadOrderMountMe,
		CI_SquadOrderMountTarget,
		CI_SquadOrderOpenDoor,
		CI_SquadOrderReleaseCommand,
		CI_SquadOrderStay,
		CI_SquadOrderTakeCommand,
		CI_SquadPostureBerserk,
		CI_SquadPostureDefendArea,
		CI_SquadPostureDefendSelf,
		CI_SquadPostureHoldFire,
		CI_SquadPostureOffensive,
		CI_SquadSelect0,
		CI_SquadSelect1,
		CI_SquadSelect2,
		CI_SquadSelect3,
		CI_SquadSelect4,
		CI_SquadSelect5,
		CI_SquadSelect6,
		CI_SquadSelect7,
		CI_SquadSelect8,
		CI_SquadSelect9,
		CI_TacticalCurrentDeck_REMOVED,
		CI_TacticalDeckHigher_REMOVED,
		CI_TacticalDeckLower_REMOVED,
		CI_TopDownNextInfo,
		CI_TopDownPrevInfo,
		CI_TopDownViewToggle,
		CI_ViewAngleDecrease,
		CI_ViewAngleIncrease,
		CI_ViewDistanceDecrease,
		CI_ViewDistanceIncrease,
		CI_DesignerGrab,
		CI_DesignerRotate,
		CI_DesignerStretch,
		CI_DragConstrainX,
		CI_DragConstrainY,
		CI_DragConstrainZ,
		CI_DesignerPushPull,
		CI_LASTITEM = 132,					// THIS ENUM MUST NOT EXCEED 132 DUE TO DiControlInput USE OF DaPerson::CONTROLINPUT
	};

	enum CURSOR
	{
	CURSOR_Unknown,
	CURSOR_None,
	CURSOR_Arrow,
	CURSOR_LeftHand,
	CURSOR_RightHand,
	};

	enum KEYBITS
	{
	KEYBIT_LeftButton	= 0x00000001,
	KEYBIT_MiddleButton	= 0x00000002,
	KEYBIT_RightButton	= 0x00000004,
	KEYBIT_Alt			= 0x00000008,
	KEYBIT_Control		= 0x00000010,
	KEYBIT_Meta			= 0x00000020,
	KEYBIT_Shift		= 0x00000040,
	};

	enum INPUTMODE
	{
		Character,	// Input is streaming to the character.
		UI			// Input is free to operate user interfaces, e.g. dialogs.
	};

private:
	AuElapsedTimer						m_Animated;				// Time animate was called.
    bool								m_bFreeCamera;			// true to allow free flying camera.
	bool								m_bFreeMouse;			// true when mouse is free, so we don't keep repositioning the mouse
	AuGLVAO								m_BusyVAO;
	bool								m_bWaitingForScene;		// True while waiting for scene atom.
	float								m_CameraAnimSpeed;		// in feet per second. 0 = no animation
	CURSOR								m_Cursor;				// Current cursor.
	float								m_Fade;					// Fade-out effect. 0=black/quieter, 1=no fade
    float								m_fCameraSpeed;			// Positive is forward, negative for reverse.
	QList<AuForageHistory>				m_ForageHistory;		// To prevent too much foraging in the same area too fast.
	bool								m_HandInputTemporary;
    AuId								m_idSceneAtom;			// The atom that provides the proper scene for m_pViewAtom.
	AuId								m_idWaitScene;			// Scene atom for which we are waiting.
    INPUTMODE							m_InputMode;			// current input mode
	AuElapsedTimer						m_InputModeChanged;		// When input mode changed, for button fade.
	bool								m_IsLefty;				// true for lefty layout
	AuItemManipulator*					m_ItemManipulator;
	QTimer*								m_KickStarter;			// Scene animation kick starter.
	AuGLModelMaker*						m_MM;					// Model maker.
	unsigned int						m_ModalDlgCount;		// non-zero while a modal dialog is up. Incremented/decremented
																// for each dialog exec()'d.
	AuGLModelCache*						m_ModelCache;
    QPoint								m_MouseLast;			// Last mouse pixel location.
	Qt::MouseButtons					m_MouseButtonsUI;		// Buttons during last mouse press event. Mouse move events never
																// report buttons per Qt Doc.
	bool								m_MouseDragging;		// true if the pointer moves enough while pressed to trigger drag
	bool								m_MouseGrabbed;			// true if we have grabbed the mouse.
    QPoint								m_MousePress;			// Mouse pixel location at last mouse press event.
	bool								m_MousePressed;			// true the mouse button is depressed, for area selection
	bool								m_MouseSnapped;			// true if the cursor snapped to an existing shape
    QPoint								m_MouseVecUI;			// Accumulated mouse movement vector.
	AuGLVAO								m_NetVAO;
	DdCommand*							m_pCommand[2];				// Current draw command.
	AuAtomRenderInfo*					m_RenderInfo;			// Values needed by atoms when rendering them.
	bool								m_RenderPaused;
    double								m_rFarClip;				// Distance from camera to far clip plane.
    double								m_rNearClip;			// Distance from camera to near clip plane.
	SCENE_CONTEXT						m_SceneContext;			// Current scene context.
	bool								m_SceneRendered;		// True after one scene rendering frame with the current scene atom.
	AuElapsedTimer						m_SizeChanged;			// When size of window changed, for button fade.
	double								m_SizeFoVY;				// field of view Y angle, for triggering size change notification
	float								m_SpawnTieEffect;		// Spawn tie. 1=full on, 0=no effect
	AuGLVAO								m_SpawnWaitVAO;
    AuMatrix4x4							m_tCamera;				// Camera transformation matrix. a.k.a. Model View matrix.
	AuMatrix4x4							m_tFarProjection;		// Projection matrix with big near clip distance.
	AuMatrix4x4							m_tProjection;			// Projection matrix with small near clip distance.
    AuMatrix4x4							m_tSceneCamera;			// Camera transformation matrix of scene atom.

	// Design Mode
	QString								m_AnchorDesc;			// Description of anchor, may be blank.
    QPoint								m_AnchorLoc;			// Mouse pixel location at 'anchor' location.
	bool								m_AnchorSelected;		// true if anchor part selected on mouse down.
	QString								m_DesignerFileName;
	AuMatrix4x4							m_DesignPlane;			// Current design plane.
	int									m_DragConstraint;		// 0=none, 1=X, 2=Y, 4=Z restraint when dragging with the mouse.
	QString								m_FinalizeReport;		// Result of last attempt to finalize.
	bool								m_IsDesignMode;			// true when in design env and design toolbar visible
	bool								m_IsDesignPreview;		// true while previewing design as real atomic DaSc
	bool								m_VertexSelected;		// true if vertex selected on mouse down.

	// Game Table
	AuDialog					*		m_GameUI;				// Modeless dialog for interacting with the current game.
	AuElapsedTimer						m_GameUITime;			// Time started when m_GameUI shown.

    // Selection
	AuSelectedUnitArray	 				m_Selected;				// Information about currently selected atoms.
	AuSelectedUnitArray	 				m_Squad[10];			// Squad selection sets.
	bool								m_View1stPref;			// 1st person vs 3rd person view preference. true = 1st person.
	int									m_ViewDeckRel;			// Deck to view aboard ship in tactical display mode, relative to
																// scene atom's current deck.

	// Communications
	AuElapsedTimer						m_MessageDecay;			// Time last decayed messages.
	int									m_MessageIntensity;		// Display intensity (0..3)
	mutable QMutex						m_MessageLock;
	QVector<AuRenderMsg>				m_Messages;				// Recent comm messages.
	unsigned int						m_NumberOfMessages;

	// Top-down view overlay text indicator.
	QString								m_OverlayText;
	AuElapsedTimer						m_OverlayTime;

	AuGLVAO								m_ButtonVAO;			// Same VAO can be used for all the buttons, and the glow.
	AuControlButton						m_ControlButtonAt;		// Control button that is under the cursor.
	AuControlButton						m_ControlButtonPressed;	// Control button under the mouse when it was pressed.
	QList<AuControlButton>				m_ControlButtons;		// Buttons that get overlayed on the screen.
	AuGLVAO								m_SubStartVAO;			// Same VAO can be used for all the buttons.
	AuGLVAO								m_SubMidVAO;			// Same VAO can be used for all the buttons.
	AuGLVAO								m_SubEndVAO;			// Same VAO can be used for all the buttons.
	AuViewButton::VIEWBUTTON			m_ViewButtonAt;			// Button that is under the cursor.
	int									m_ViewButtonColumns;	// Number of columns view buttons are wrapped into.
	AuElapsedTimer						m_ViewButtonEntered;	// When the view button was entered, for tool tip.
	GLuint								m_ViewButtonGlow[4];	// Texture for highlighted button faces.
	AuViewButton::VIEWBUTTON			m_ViewButtonNear;		// Button that is near the cursor, with possible sub buttons.
	AuViewButton::VIEWBUTTON			m_ViewButtonPressed;	// View button under the mouse when it was pressed.
	QList<AuViewButton>					m_ViewButtons;			// Buttons that get overlayed on the screen.
	GLuint								m_ViewButtonTex;		// Texture for button faces.
	QString								m_ViewButtonTexName;	// Style texture loaded int m_ViewButtonTex for button faces.

private:
	mutable QMutex						m_ControlLock;
	unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>	m_ActiveKeys;	// Keys currently down. Time is used by DiControlInput; starts null.
	Qt::KeyboardModifiers				m_ActiveKeyMods;		// Keyboard modifiers active at last keyboard event.
	Qt::KeyboardModifiers				m_ActiveMouseMods;		// Keyboard modifiers active at last mouse event.
	AuElapsedTimer						m_AtomControlTime;
	unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>	m_AtomKeys;		// Keys currently down. Time is used by DiControlInput; starts null.
	Qt::KeyboardModifiers				m_AtomKeyMods;			// Keyboard modifiers active at last keyboard event.
	Qt::KeyboardModifiers				m_AtomMouseMods;		// Keyboard modifiers active at last mouse event.
	Qt::MouseButtons					m_MouseButtonsAtom;		// Buttons during last mouse press event.
	QPoint								m_MousePos;
	QPoint								m_MousePosLast;
    QPoint								m_MouseVecAtom;			// Accumulated mouse movement vector.
    DiControlInputArray*				m_pControlsAtom;		// Control inputs for current scene object.
    DiControlInputArray*				m_pControlsUI;			// Control inputs for current UI.

private:
	AuElapsedTimer						m_SceneAtomImgTime;		// So we don't make one in every frame.
	mutable QReadWriteLock				m_SceneImgLock;
	QImage								m_SceneAtomImage;

private:
	mutable QReadWriteLock				m_SceneInfoLock;
	quint8								m_AgentBits;			// Agent bits from scene atom
	int									m_DesignMode;			// Is a DaDesignBase::DESIGNMODE.
    AuId								m_idSceneSector;		// The sector atom that currently contains the scene atom.
    AuId								m_idSceneSolarSystem;	// The solar system atom that currently contains the scene atom.
	bool								m_InCity;				// true while the scene atom is in a city and not in a spacecraft
	bool								m_IsShipCommand;		// true if person is in chain-of-command of the ship they are aboard
	bool								m_OnLLWorld;			// Person is on a DaWorldLL at some level and in the atmosphere.
	quint16								m_PersonStationId;		// station id, 0 = not at a station.
	AuPose								m_PersonPose;			// current pose of the scene atom, or Init if not valid.
	AuRosterEntry::TYPE					m_SceneAtomBerth;
	quint32								m_SceneAtomEmpire;		// Empire of the scene atom.
	int									m_SceneAtomEnvType;		// construction window: 0=space, 1=desert, 2=plants, 3=water, 4=moon
    AuId								m_SceneAtomInShip;		// Ship id of the ship that contains the scene atom.
	QString								m_SceneAtomName;		// Name of the scene atom.
	AuId								m_SceneAtomOwnerId;		// The owner id of a private ship officered by the scene atom, if
																// it is found in the environment. Otherwise, the scene atom id.
    AuId								m_SceneAtomScopeShip;	// Ship id of the scene atom used for crew scope chat messages.
	bool								m_SceneAtomTransition;	// result of last call to scene atom's IsInTransition function.
	quint32								m_SceneAtomWorldBuildingCount;	// Current building count on the world.
	quint32								m_SceneAtomWorldFenceCount;	// Current fence count on the world.
	quint32								m_SceneAtomWorldStreetLightCount;	// Current street light count on the world.
	quint32								m_SceneAtomWorldBuildingLimit;	// Limit to building volume on the world, in cubic meters.
	quint32								m_SceneAtomWorldBuildingVolume;	// Current building volume on the world, in cubic meters.
    GALAXY								m_SceneGalaxy;			// The galaxy that currently contains the scene atom.

private:
	// The view keeps its own copy of the atoms in the scene.
	QReadWriteLock						m_SceneLock;			// Locked while accessing the scene atom pointer in m_SceneEther.
	AuEther								m_SceneEther;			// Ether of this scene.

private:
	// Targeting.
	mutable QReadWriteLock				m_TargetLock;
	QString								m_TargetDesc;			// Target description when targetted.
	AuDNABase*							m_TargetDNA;			// DNA of target.
	AuId								m_TargetId;				// Id of targeted atom. May be null.
	bool								m_TargetIsCivilian;
	int									m_TargetLevel;			// Target acquisition level. [0..15]
	AuPoint								m_TargetLoc;			// Sighted location on target rel to scene atom.
	unsigned int						m_TargetPart;			// Target part when last sighted.
	bool								m_TargetShield;			// True if target is surface of shield bubble.
	AuId								m_TargetSpotId;			// Id of spotted atom. May be null.
	AuPoint								m_TargetSpotLoc;		// Sighted location on target rel to spotted atom.
	POLITICAL_STANCE					m_TargetStance;			// Political stance of scene atom's empire toward the spotted atom.
	DtTarget*							m_TargetThread;

	// Unit orders sprite.
	AuId								m_OrderId;				// Atom that m_OrderLoc is relative to.
	AuPoint								m_OrderLoc;				// Location of go to order.
	AuElapsedTimer						m_OrderTime;			// Time goto order was made.

private:
	// OpenGL
	GLint								m_GLTextureUnits;		// GL_MAX_TEXTURE_IMAGE_UNITS
	DtGL*								m_GLThread;
	GLint								m_GLUniformLocs;		// GL_MAX_UNIFORM_LOCATIONS

	// Textures
	GLuint								m_texBusy;
	GLuint								m_texFadeEffect;
	GLuint								m_texNet;
	GLuint								m_texOrder;
	GLuint								m_texServerHandoff;
	GLuint								m_texSpawnWait;
	GLuint								m_texWarpSides;
	GLuint								m_texWormholeSides;
	GLuint								m_texWormholeSpace;

	float								m_BusyAngle;
	bool								m_IsWarping;			// true for warp effect, otherwise wormhole effect for wait anim
	int									m_Net1Down;
	int									m_Net1Up;
	int									m_Net2Down;
	int									m_Net2Up;
	int									m_Net3Down;
	int									m_Net3Up;
	int									m_Net4Down;
	int									m_Net4Up;
	AuGLVAO								m_OrderVAO;
	QVector<AuSprite>					m_Sprites;
	float								m_SpaceAngle;
	float								m_WormholeCenterOffset;
	float								m_WormholeSpeed;
	float								m_WormholeTunnel;

public:
	AuDesignerView(QWidget *parent=0);
	~AuDesignerView();

	// Animate scene atom device objects, e.g. console textures.
	// Called from DtGL thread when threaded OpenGL is supported. A shared context is selected as current.
	void AnimateSceneAtoms(DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	INPUTMODE AssessInputMode() const;	// Does the work to determine what the input mode should be right now.

	virtual void ChatReceived(AuCommPacket* msg);

	// Returns current active command.
	DdCommand* Command();

	const DdCommand* Command() const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	QString ControlBinding(CONTROLINPUT control) const;

	QKeySequence ControlKeySequence(CONTROLINPUT control) const;

	// Warn against losing changes to the current design. Provide an option to cancel.
	// Returns true if it's ok to proceed, false if user wants to cancel.
	bool DesignerConfirmLoseChanges();

	// Updates a drag operation in designer, for grab, rotate, and stretch.
	void DesignerDragging(const AuMatrix4x4& t, bool isCopy);

	// Initiates a drag operation in designer, for grab, rotate, and stretch.
	void DesignerDragStart(const AuPoint& ref);

	// Returns the id numbers of the currently restorable grid positions in the designer.
	QSet<quint8> DesignerGridPositions();

	bool DesignerIsViewOnly();

	bool DesignerObjectModeOk();

	void DesignerPaintInUse(QList<QColor>& paintInUse);

	// Returns the current top-level selection. i.e. No vertices, edges and faces.
	inline const AuScSelection& DesignerPartSelection() const
	{
		return m_RenderInfo->DesignerPartSelection();
	}

	inline const AuScSelection& DesignerPartObjectSelection() const
	{
		return m_RenderInfo->DesignerPartObjectSelection();
	}

	// Returns the id numbers of the currently restorable view positions in the designer.
	QSet<quint8> DesignerViewPositions();

	// For designer.
	// Returns Command()->DesignPlane() while a command is running. Default command returns DesignPlaneMatrix.
	// Returns DragPlane while dragging.
	// Returns DesignPlaneMatrix otherwise.
	AuMatrix4x4 DesignPlane() const;

	inline const AuMatrix4x4& DesignPlaneMatrix() const
	{
		return m_DesignPlane;
	}

	inline bool DragSelectedParts() const
	{
		if(DesignerObjectMode())
			return m_RenderInfo->DragPartSelected();
		else
			return m_RenderInfo->AnchorPartSelected();
	}

	void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void GameOver(quint8 gameType, const QByteArray& gameResult);

	void GameSelect(quint8 gameType, bool gameSelected);

	void GameTurn(const AuId& personsTurn);

	inline void AddMessage(const AuRenderMsg& rm)
	{
		m_MessageLock.lock();
		m_Messages.append(rm);
		m_MessageLock.unlock();
	}

	// Returns true if any selected part is sealed by an architect other than the current one.
	bool HasSealedPartsSelected();

	inline bool IsCharacterInputMode() const
	{
		return m_InputMode == Character;
	}

	// Returns true if person is sitting at a console of a spacecraft. Returns false if sitting at a turret or a game table.
	inline bool IsPersonAtConsole() const
	{
		return PersonPose().IsConsole() && !IsPersonAtGameTable();
	}

	// Returns true if person is sitting at a console of a spacecraft. Returns false if sitting at a turret.
	inline bool IsPersonAtGameTable() const
	{
		return PersonPose().IsGameTable() && PersonStationId();
	}

	inline GLint OpenGLTextureUnits() const
	{
		return m_GLTextureUnits;
	}

	inline GLint OpenGLUniformLocs() const
	{
		return m_GLUniformLocs;
	}

	inline quint16 PersonStationId() const
	{
		m_SceneInfoLock.lockForRead();
		quint16 v = m_PersonStationId;
		m_SceneInfoLock.unlock();
		return v;
	}

	inline AuPose PersonPose() const
	{
		m_SceneInfoLock.lockForRead();
		AuPose v(m_PersonPose);
		m_SceneInfoLock.unlock();
		return v;
	}

	// Returns true if person is in the chain-of-command of the ship they are aboard.
	virtual bool IsPersonInCommand() const
	{
		m_SceneInfoLock.lockForRead();
		bool v = m_IsShipCommand;
		m_SceneInfoLock.unlock();
		return v;
	}

	inline bool IsUIInputMode() const
	{
		return m_InputMode == UI;
	}

	void LoadControls() const;

	virtual void LocalLogMessage(const QString& text);

	void OperateTurboLift(const AuId& shipId, const AuId& personId, quint16 liftId);

	virtual quint8 PersonAgentBits() const
	{
		m_SceneInfoLock.lockForRead();
		quint8 v = m_AgentBits;
		m_SceneInfoLock.unlock();
		return v;
	}

	void ProcessControlInput(const DiControlInputArray& aActiveControls);

	void RestoreDefaultControls();

	void SaveControls() const;

	// Returns true if the selection actually changes.
	virtual bool SelectUnit(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
			NPC_ROLE npcRole, bool selectIt);

	void SelectUnitToggle(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
			NPC_ROLE npcRole);

	virtual const AuSelectedUnitArray& SelectedUnits() const
	{
		return m_Selected;
	}

	void SetDesignPlane(const AuMatrix4x4& designPlane);

	virtual void SetSelectedBd(const AuId& idBd);

	void SquadCreate(int auquadIndex);

	// So far, 'orderText' is only needed for the go to room order. It is expected to be a room name then.
	virtual void SquadIssueOrder(CITIZEN_ORDER order, const QString& orderText=QString(""));

	virtual void SquadIssuePosture(CITIZEN_POSTURE posture);

	void SquadSelect(int auquadIndex, bool addToCurrentSelection);

	void SquadSendOrder(AuCommPacket::TYPE msgType, const QString& msgText) const;

	void StopThreads();

	virtual const AuDNABase* TargetDNA() const
	{
		return m_TargetDNA;
	}

	virtual const AuId& TargetId() const
	{
		return m_TargetId;
	}

	// These LockScene functions only lock the atom mutex.
	virtual void LockSceneAtom(DaPerson** sceneAtom);
	void LockSceneAtomForWrite(DaPerson** sceneAtom);
	virtual void UnlockSceneAtom();

	// Locates the closest fence post on the terrain to the current cursor location.
	// Sets 'postLoc' and 'platformId' to the post's info on true return.
	// The caller has not locked the scene atom.
	bool FindClosestFencePost(AuPoint& postLoc, AuId& platformId);

	// Locates a terrain location at a given pixel.
	// 'fLoc' is set to the terrain location, relative to the atom returned.
	// NULL is returned if no terrain point is found.
	// Scene must be locked.
	DaWorldLL* FindTerrainLocationAtPixel(DaPerson* sceneAtom, const QPoint& pxFind, AuPoint& fLoc) const;

	// Use this one if you want to find platforms.
	// 'platform' will be set to 0 if a platform is not found, set to its address if one is found.
	DaWorldLL* FindTerrainLocationAtPixel(DaPerson* sceneAtom, const QPoint& pxFind, AuPoint& fLoc, DaBd** platform) const;

	void GetPixelTransforms(AuMatrix4x4 &tDevicePersp, AuMatrix4x4 &tDeviceOrtho) const;

	void GetPixelTransforms(AuMatrix4x4 &tDevice, AuMatrix4x4 &tPersp, AuMatrix4x4 &tOrtho) const;

	// returns a DaDesignBase::DESIGNMODE
	int DesignMode() const;

	// Returns BD_Invalid if not in building designer.
	BUILDING_STRUCTURE DesignBdStructure();

	// Returns true if in a DaDesignBase instance.
	bool IsInDesignEnvironment() const;

	// Returns true if in a DaDesigner instance.
	bool IsInNewDesignEnvironment() const;

	// Scene atom is subordinate to a DaWorldLL atom at some level and in the atmosphere.
	inline bool IsOnLLWorld() const
	{
		m_SceneInfoLock.lockForRead();
		bool v = m_OnLLWorld;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual const AuId& SceneAtomId() const
	{
		return m_idSceneAtom;
	}

	virtual quint32 SceneAtomEmpire() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomEmpire;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual int SceneAtomEnvType() const
	{
		m_SceneInfoLock.lockForRead();
		int v = m_SceneAtomEnvType;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual QImage SceneAtomImage() const
	{
		m_SceneImgLock.lockForRead();
		QImage v = m_SceneAtomImage;
		m_SceneImgLock.unlock();
		return v;
	}

	inline bool SceneAtomInTransition() const
	{
		m_SceneInfoLock.lockForRead();
		bool v = m_SceneAtomTransition;
		m_SceneInfoLock.unlock();
		return v;
	}

	inline bool SceneAtomIsOfficer() const
	{
		m_SceneInfoLock.lockForRead();
		bool v = !m_SceneAtomScopeShip.IsNull() && AuRosterEntry::IsOfficer(m_SceneAtomBerth);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual QString SceneAtomName() const
	{
		m_SceneInfoLock.lockForRead();
		QString v(m_SceneAtomName);
		m_SceneInfoLock.unlock();
		return v;
	}

	// If the scene atom is an officer of a private ship, returns the owner id of that ship if it is found in the environment.
	// Otherwise, returns the scene atom id.
	virtual AuId SceneAtomOwnerId() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_SceneAtomOwnerId);
		m_SceneInfoLock.unlock();
		return v;
	}

	// Returns the id of the scene atom's spawn ship.
	virtual AuId SceneAtomScopeShip() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_SceneAtomScopeShip);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual GALAXY SceneGalaxy() const
	{
		m_SceneInfoLock.lockForRead();
		GALAXY v = m_SceneGalaxy;
		m_SceneInfoLock.unlock();
		return v;
	}

	// Returns the id of the ship that contains the scene atom, if any.
	virtual AuId SceneAtomInShip() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_SceneAtomInShip);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual quint32 SceneAtomWorldBuildingCount() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldBuildingCount;
		m_SceneInfoLock.unlock();
		return v;
	}

	inline quint32 SceneAtomWorldFenceCount() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldFenceCount;
		m_SceneInfoLock.unlock();
		return v;
	}

	inline quint32 SceneAtomWorldStreetLightCount() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldStreetLightCount;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual quint32 SceneAtomWorldBuildingVolumeLimit() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldBuildingLimit;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual quint32 SceneAtomWorldBuildingVolume() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldBuildingVolume;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual AuId SceneSector() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_idSceneSector);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual AuId SceneSolarSystem() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_idSceneSolarSystem);
		m_SceneInfoLock.unlock();
		return v;
	}

	void SetSceneAtom(const AuId& idScene);

	// Returns the last "spotted" atom id.
	virtual AuId SpottedAtom() const
	{
		m_TargetLock.lockForRead();
		AuId v(m_TargetSpotId);
		m_TargetLock.unlock();
		return v;
	}

	// Returns the last "spotted" location, relative to the spotted atom.
	virtual AuPoint SpottedLocation() const
	{
		m_TargetLock.lockForRead();
		AuPoint v(m_TargetSpotLoc);
		m_TargetLock.unlock();
		return v;
	}

	void StartingLocationEstablished();	// Used during first-time planet init.

	QString TargetDescription();

	// Scene must already be locked to call this function.
	AuPoint FromPixelToScenePlane(DaAtom* parentAtSomeLevel, DaPerson* sceneAtom, const AuPoint& fpxLoc);

	AuPoint FromPixelToDesignPlane(const QPoint& pxLoc, bool roundToDesignGrid, const AuPoint* refLoc);

	// Scene must already be locked to call this function.
	// Return coordinate is relative to the parent design studio.
	// 'roundToDesignGrid' is true to enable grid rounding in DaDesignBase.
	AuPoint FromPixelToDesignPlane(DaPerson* sceneAtom, const QPoint& pxLoc, bool roundToDesignGrid, const AuPoint* refLoc);

	AuPoint FromPixelToPlane(const QPoint& pxLoc, const AuMatrix4x4& plane, bool roundToDesignGrid, const AuPoint* refLoc);

	AuMatrix4x4 GetDesignPlaneTransform();

	virtual SCENE_CONTEXT SceneContext() const
	{
		return m_SceneContext;
	}

protected:
	virtual void initializeGL();
	virtual void paintGL();

	virtual void focusInEvent(QFocusEvent* e);
	virtual void focusOutEvent(QFocusEvent* e);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);

	void ActivateWindow();

protected slots:
	void AnimateScene();

	void SaveSolitaireCheckPoint();

	void ScreenShot();

public slots:
	void AvatarOnLine(AuId id);

	void CommandPromptChanged(const QString& prompt);

    // ALL coordinate input in response to designer commands eventually gets to here, regardless of whether it is
    // input using the mouse or entered into the AuDesignerBar.
	// 'fromKeyboard' is true if coordinate input was made using the keyboard. i.e. person entered the coordinate in the edit box.
	// 'fromKeyboard' is false if coordinate input was made using the mouse by clicking in the scene.
	// 'keymods' is the Qt::KeyboardModifiers that were active when the mouse was clicked or enter was pressed.
    void CoordinateEntered(AuPoint coord, bool fromKeyboard, int keymods);

	// Process a coordinate entered into the AuDesignerBar edit control. It is sent to us still in its string form.
	// 'keymods' is the Qt::KeyboardModifiers from the enter event.
	void CoordinateEntered(QString coord, int keymods);

	void ControlsChanged();

	virtual void DeselectAllUnits();

	// Returns true if the designer is in object isolate mode. Only applies if also in face mode.
	inline bool DesignerObjectIsolate() const
	{
		return m_RenderInfo->DesignerObjectIsolate();
	}

	// Returns true if the designer is in face mode.
	inline bool DesignerObjectMode() const
	{
		return m_RenderInfo->DesignerObjectMode();
	}

	// Returns part id that is the target of a paste operation when in face mode. More than one part can be operated on at once
	// in face mode. Returns 0 if not in face mode.
	quint16 DesignerObjectModeTargetPart() const;

	// Starts a command to place the building design on the terrain on the designer preview world.
	void DesignerPreviewBd();

	void DesignerPreviewPeople();

	void DesignerPreviewSettings();

	void DesignerSetPartSelection(AuScIdSet selected);

	// Ends all commands at the 'runLevel' and above.
	void EndCommand(int runLevel);

	// Ends all running commands.
	void EndCommands();

	// End highest level running command.
	void EndCurrentCommand();

	// The local player has quit the current game.
	void GameQuit();

	// The local player has finished their turn at a game table.
	void GameTurn(quint8 gameType, QByteArray gameSettings);

	void PageAgent();

	void PauseRendering();

	void ResumeRendering();

	void SaveWhereabouts();

	void SetDesignerObjectIsolate(bool objectIsolate);

	void SetDesignerObjectMode(bool objectMode);

	void SetDesignMode(bool isDesignMode);

	void SetNumberOfMessages(int iNumber)
	{
		m_NumberOfMessages = (unsigned int)iNumber;
	}

	void SpawnRecall();

	void SpawnTie();

	void TabKeyPressed(Qt::KeyboardModifiers mods);

	// City Planning Commands
	// Starts a command to draw a Bd building.
	// 'cp' is based on the original design material 'originalMaterial'. It must be changed to 'constructionMaterial' for the final
	// process. This enables the DdPlanBdBuilding command to change construction material in response to key commands.
	void PlanBd(INDUSTRYID industryId, const AuBdReport& report, COMMODITYID constructionMaterial, quint16 mpid, bool enableWorker);

	// Changes construction process in current running command to plan a Bd building.
	void PlanBdConstructionProcess(const AuBdReport& report);

	// Starts a command to draw a Bd fence.
	// 'fenceStyle' is an AuScFence::STYLE.
	// 'constructionMaterial' is a COMMODITYID.
	void PlanBdFence(int fenceStyle, quint32 customDesignId, COMMODITYID constructionMaterial);

	// Changes construction material in current running command to plan a Bd building.
	void PlanBdMaterial(COMMODITYID constructionMaterial);

	void PlanClearCraters();		// Starts a command to clear craters.

	void PlanClearDevelopments();	// Starts a command to clear land developments such as roads and buildings.

	void PlanGeoRoadAsphalt();
	void PlanGeoRoadConcrete();
	void PlanGeoRoadDirt();
	void PlanGeoRoadGrade();
	void PlanGeoLandscapeClearing();
	void PlanGeoLandscapeDefoliate();
	void PlanGeoLandscapeIrrigate();

	void PlanManufacturingProcess(int iProcessID);

	void PlanManufacturingWorkerOk(bool enableWorker);

	// DaDesigner
	void DesignerAlign();
	void DesignerAlignLiftStop();
	void DesignerAssociate();
	void DesignerBarrier();
	void DesignerBdHullMax();
	void DesignerBdLibraryDelete();
	void DesignerBdLibraryOpen();
	void DesignerBerthCapt();
	void DesignerBerthCrew();
	void DesignerBerthName();
	void DesignerBerthOfficer();
	void DesignerBerthPassenger();
	void DesignerBerthTroop();
	void DesignerBoundGround();
	void DesignerBoundMax();
	void DesignerBuildZone();
	void DesignerCaptName();
	void DesignerChangeDNA();
	void DesignerConsoleCapt();
	void DesignerConsoleEngineer();
	void DesignerConsoleFireControl();
	void DesignerConsoleHelm();
	void DesignerConsoleMedic();
	void DesignerConsoleNavigator();
	void DesignerConsolePowerRelay();
	void DesignerConsoleSensor();
	void DesignerConsoleShield();
	void DesignerConsoleTransporter();
	void DesignerCopy();
	void DesignerCopySymmetrical();
	void DesignerCut();
	void DesignerCutFace();
	void DesignerCuttingJig();
	void DesignerDecalJig();
	void DesignerDelete();
	void DesignerDepart();
	void DesignerDeselectTextureVertex(DaDesigner* pde, quint16 partId, quint32 partVertex);
	void DesignerDeselectVertices();
	void DesignerDissociate();
	void DesignerDivideFaces();
	void DesignerDoorJig();
	void DesignerDoorProperties();
	void DesignerEffectIntensity();
	void DesignerEmitterContrail();
	void DesignerEmpireFlag();
	void DesignerEmpireName();
	void DesignerExit();
	void DesignerExportSTL();
	void DesignerFence();
	void DesignerFinalize();
	void DesignerFinalizeReport();
	void DesignerFlatten();
	void DesignerFractureFaces();
	void DesignerFuseFaces();
	void DesignerGameTable();
	void DesignerGearOpen();
	void DesignerGearSaveBd();
	void DesignerGearSaveSc();
	void DesignerGlow();
	void DesignerGoToSelection();
	void DesignerGrid3pt();
	void DesignerGridMove();
	void DesignerGridPositionLoad1();
	void DesignerGridPositionLoad2();
	void DesignerGridPositionLoad3();
	void DesignerGridPositionLoad4();
	void DesignerGridPositionLoad5();
	void DesignerGridPositionLoad6();
	void DesignerGridPositionLoad7();
	void DesignerGridPositionLoad8();
	void DesignerGridPositionLoad9();
	void DesignerGridPositionOrigin();
	void DesignerGridPositionSave1();
	void DesignerGridPositionSave2();
	void DesignerGridPositionSave3();
	void DesignerGridPositionSave4();
	void DesignerGridPositionSave5();
	void DesignerGridPositionSave6();
	void DesignerGridPositionSave7();
	void DesignerGridPositionSave8();
	void DesignerGridPositionSave9();
	void DesignerGridXY();
	void DesignerGridXZ();
	void DesignerGridYZ();
	void DesignerHullExtrusion();
	void DesignerHullSphereEqDivs();
	void DesignerHullSphereLatLong();
	void DesignerHullSpindle();
	void DesignerHullTubing();
	void DesignerImport3DS();
	void DesignerImportSTL();
	void DesignerInteriorExtrusion();
	void DesignerInteriorSphereEqDivs();
	void DesignerInteriorSphereLatLong();
	void DesignerInteriorSpindle();
	void DesignerInteriorTubing();
	void DesignerItLibraryDelete();
	void DesignerItLibraryOpen();
	void DesignerJigCut();
	void DesignerJoinParts();
	void DesignerLadderPath();
	void DesignerLandingGearExtrusion();
	void DesignerLandingGearSphereEqDivs();
	void DesignerLandingGearSphereLatLong();
	void DesignerLandingGearSpindle();
	void DesignerLandingGearTubing();
	void DesignerLandingSystem();
	void DesignerLaunchTube();
	void DesignerLaunchTubeJig();
	void DesignerLibrarySave();
	void DesignerLightBulb();
	void DesignerLightBulbProps();
	void DesignerLightLensProps();
	void DesignerLoopCut();
	void DesignerMakeDecal();
	void DesignerMakeDoor();
	void DesignerMakeHull();
	void DesignerMakeLandingGear();
	void DesignerMakeLightLens();
	void DesignerMakeMesh();
	void DesignerMakeNavLightLens();
	void DesignerMakeRoom();
	void DesignerMakeRotatingBeaconLens();
	void DesignerMakeStrobeLens();
	void DesignerMakeTurboLift();
	void DesignerMakeTurret();
	void DesignerMakeWindow();
	void DesignerMove();
	void DesignerMoveDoorSwitches();
	void DesignerMoveMenu();			// Grab initiated off menu.
	void DesignerNewBuilding();
	void DesignerNewItem();
	void DesignerNewSpacecraft();
	void DesignerNewVehicle();
	void DesignerObjectExtrusion();
	void DesignerObjectFace();
	void DesignerObjectQuad();
	void DesignerObjectSphereEqDivs();
	void DesignerObjectSphereLatLong();
	void DesignerObjectSpindle();
	void DesignerObjectTriangle();
	void DesignerObjectTubing();
	void DesignerOpacity();
	void DesignerOpen();
	void DesignerPaint();
	void DesignerPanelCapacitor();
	void DesignerPanelFTL();
	void DesignerPanelLifeSupport();
	void DesignerPanelManeuver();
	void DesignerPanelPowerPlant();
	void DesignerPanelSensor();
	void DesignerPanelShield();
	void DesignerPanelWeapon();
	void DesignerParkGroundVehicle();
	void DesignerParkRocket();
	void DesignerParkSpaceVehicle();
	void DesignerParkSpacecraft();
	void DesignerParkWaterVehicle();
	void DesignerPaste();
	void DesignerPasteAt();
	void DesignerPasteRadial();
	void DesignerPodBay();
	void DesignerPolish();
	void DesignerProperties();
	void DesignerPunchFace();
	void DesignerPushPull();
	void DesignerRedo();
	void DesignerRemoteCamera();
	void DesignerReverseFaces();
	void DesignerRoomExtrusion();
	void DesignerRoomSphereEqDivs();
	void DesignerRoomSphereLatLong();
	void DesignerRoomSpindle();
	void DesignerRoomTubing();
	void DesignerRotate();
	void DesignerSave();
	void DesignerSaveAs();
	void DesignerScLibraryDelete();
	void DesignerScLibraryOpen();
	void DesignerSelectAll();
	void DesignerSelectContiguous();
	void DesignerSelectEdges();
	void DesignerSelectFaces();
	void DesignerSelectInvert();
	void DesignerSelectNone();
	void DesignerSelectTextureVertex(DaDesigner* pde, quint16 partId, quint32 partVertex);
	void DesignerSelectVertices();
	void DesignerSelectedProperties();
	void DesignerSelfIllumination();
	void DesignerSetPreviewHasPower(bool hasPower);
	void DesignerSetPreviewTimeOfDay(float);
	void DesignerSetRoomProperties();
	void DesignerSetVoidProperties();
	void DesignerShipName();
	void DesignerShrub();
	void DesignerSite();
	void DesignerSiteMax();
	void DesignerSiteProperties();
	void DesignerSketch();
	void DesignerSmooth();
	void DesignerSplitFaces();
	void DesignerStationCapt();
	void DesignerStationCitizen();
	void DesignerStationDesigner();
	void DesignerStationEngineer();
	void DesignerStationFireControl();
	void DesignerStationGunner();
	void DesignerStationHelm();
	void DesignerStationLivestock();
	void DesignerStationMedic();
	void DesignerStationNavigator();
	void DesignerStationPowerRelay();
	void DesignerStationSensor();
	void DesignerStationShield();
	void DesignerStationTransporter();
	void DesignerStationTroop();
	void DesignerStatusAlert();
	void DesignerStatusLifeSupport();
	void DesignerStatusMission();
	void DesignerStatusWorldMap();
	void DesignerStretch();
	void DesignerStretchMenu();			// Stretch initiated from menu.
	void DesignerTextureCylindrical(int layer);	// layer 0=base, 1=effects
	void DesignerTexturePalette();
	void DesignerTextureProjected(int layer);	// layer 0=base, 1=effects
	void DesignerTextureSpherical(int layer);	// layer 0=base, 1=effects
	void DesignerTransporterPadLocs();
	void DesignerTree();
	void DesignerTurretBase();
	void DesignerTurretGun();
	void DesignerUndo();
	void DesignerViewPositionGrid();
	void DesignerViewPositionLoad1();
	void DesignerViewPositionLoad2();
	void DesignerViewPositionLoad3();
	void DesignerViewPositionLoad4();
	void DesignerViewPositionLoad5();
	void DesignerViewPositionLoad6();
	void DesignerViewPositionLoad7();
	void DesignerViewPositionLoad8();
	void DesignerViewPositionLoad9();
	void DesignerViewPositionOrigin();
	void DesignerViewPositionSave1();
	void DesignerViewPositionSave2();
	void DesignerViewPositionSave3();
	void DesignerViewPositionSave4();
	void DesignerViewPositionSave5();
	void DesignerViewPositionSave6();
	void DesignerViewPositionSave7();
	void DesignerViewPositionSave8();
	void DesignerViewPositionSave9();
	void DesignerVisibleFaces(AuScFaceList& faces);
	void DesignerVoidEnvironmentExtrusion();
	void DesignerVoidEnvironmentSpindle();
	void DesignerVoidHullExtrusion();
	void DesignerVoidHullSpindle();
	void DesignerVoidRoomExtrusion();
	void DesignerVoidRoomSpindle();
	void DesignerWalkPath();
	void DesignerWeaponEmitLoc();
	void DesignerWindowJig();

	void SpotTargetOrTerrain(bool logOnly);

	void ToggleFullScreen();

public:
	virtual QString DesignAnalysis();

	// Returns current material setting of building in design studio.
	BUILDMATERIAL DesignBuildingMaterial();

	bool DesignIsGravityOn() const;
	bool DesignIsRenderShaded() const;
	bool DesignIsSavable();

	bool DesignSaveBuildingFile();

	bool DesignSaveSpaceShipFile();

	bool DoesSceneOwnDesign();

	inline bool IsDesignPreview() const
	{
		return m_IsDesignPreview;
	}

	inline bool IsModalDlgUp() const
	{
		return m_ModalDlgCount ? true : false;
	}

	// Called by AuModel, which is another thread.
	virtual void ProcessControls();

	virtual void SetModalDlgUp(bool isUp)
	{
		m_MouseDragging = false;		// turn off the drag box if a dialog goes up or down.
		m_MousePressed = false;			// turn off mouse press if a dialog goes up or down.
		m_ViewButtonEntered = AuElapsedTimer();
		m_ViewButtonNear = AuViewButton::VB_Invalid;
		m_ViewButtonPressed = AuViewButton::VB_Invalid;

		// Increment the modal dialog counter based on whether this is an up or a down notification.
		if(isUp)
		{
			// Increment count of modal dialogs that are up.
			m_ModalDlgCount++;
		}
		else
		{
			// Decrement count of modal dialogs that are up.
			Q_ASSERT(m_ModalDlgCount);	// Fails if you call SetModalDlgUp(false) more times than SetModalDlgUp(true).
			if(m_ModalDlgCount)
				m_ModalDlgCount--;
		}
	}

	// Called by DtTarget.
	void UpdateTarget(DaPerson* sceneAtom, bool descAlso);

signals:
	void ChatReleased();	// emitted when keyboard grab released for chat

	void DesignerAnchorChanged(QString);

	void DesignerCommandPrompt(QString);

	void DesignerGrabQueued();

	void DesignerPreviewChanged();

	void DesignerRotateQueued();

	void DesignerSelectionChanged();

	void DesignerStretchQueued();

	void RequestEndCommand();

	void RequestScreenShot();

	void SceneAtomChanged();

	// 'sceneContext' is a SCENE_CONTEXT value.
	void SceneContextChanged(int sceneContext);

	void SelectGame();

	void SpotTargetOrTerrain(QString targetDesc, int stanceTowardTarget, bool targetIsCivilian, bool logOnly);

	void ToggleScreenReady();

private:
	void CheckDossier();

	AuControlButton ControlButtonAt(const QPoint& mousePos) const;

	void ControlButtonClicked(const AuControlButton& button);

	double ControlButtonLength() const;

	double ControlButtonTextLength(const QString& text) const;

	void DesignerGridPositionLoad(int number);

	void DesignerGridPositionSave(int number);

	void DesignerMove(const AuPoint& ref);

	void DesignerPushPull(const AuPoint& ref);

	void DesignerRotate(const AuPoint& ref);

	void DesignerStretch(const AuPoint& ref, double stretchBase);

	void DesignerViewPositionLoad(int number);

	void DesignerViewPositionSave(int number);

	void DrawDiamond(const AuMatrix4x4& model, const AuPoint& loc, float r, float g, float b, bool filled);

	void DrawGrip(const AuMatrix4x4& model, const AuPoint& loc, float r, float g, float b);

	void GrabMouse();

	bool GripLoc(const AuMatrix4x4& model, const AuPoint& loc, QPoint& gripLoc) const;

	void ProcessAtomControls(DaPerson* sceneAtom);

	void ProcessDesignerMouseDoubleClick(QMouseEvent* e);

	void ProcessDesignerMouseMove(QMouseEvent* e);

	void ProcessDesignerMousePress(QMouseEvent* e);

	void ProcessDesignerMouseRelease(QMouseEvent* e);

	void ProcessUIControls();

	// Returns true if the mouse was released, false if I was not grabbing the mouse when this was called.
	bool ReleaseMouse();

	void RenderCommMessages();

	void ResetActiveKeyTimes();

	void SelectPreferredFirstThirdPersonView();

	bool SelectUnitToggle(DaPerson* sceneAtom, DaAtom *pSelect);

	void SetCursor();

	void SetCursor(DaPerson* sceneAtom);

	// emits the SceneContextChanged signal if the view context actually changes.
	void SetSceneContext(SCENE_CONTEXT viewContext);

	void SetupProjection();

	void ShowItemManipulator(const DaItem& item);

	// 'sceneAtom' may be null.
	void UpdateSceneInfo(DaPerson* sceneAtom);

	void OnViewResetCamera();
	void CreateDeviceObjects(DaPerson* sceneAtom);

	//void DeleteDeviceObjects(DaAtom& root, bool staticToo);

	void FinishScene();

	void RenderOverlayText();

	void RenderScene();

	void RenderScene(DaPerson* sceneAtom);

	void RenderTactical(DaPerson* sceneAtom, DaAtom* pEnvironment);

	void CreateViewButtons();

	void RenderControlButtons();

	void RenderViewButtons();

	void RenderWindowSize();

	void PositionControlButtons();

	void PositionViewButtons();

	void ViewButtonClicked(AuViewButton::VIEWBUTTON button);

	double ViewButtonRadius() const;

	double ViewButtonSubLength() const;

	double ViewButtonSubLength(const QString& text) const;

	double ViewButtonSubRadius() const;

	// Returns the view button that contains 'mousePos' within the button disc.
	AuViewButton::VIEWBUTTON ViewButtonAt(const QPoint& mousePos) const;

	bool ViewButtonIsEnabled(AuViewButton::VIEWBUTTON vb) const;

	// Returns the view button that is closest to 'mousePos', within a search radius that is larger than the button disc.
	AuViewButton::VIEWBUTTON ViewButtonNear(const QPoint& mousePos) const;

	void ShowBusyIndicator();
	void ShowMouseThumbTwiddleInstruction();
	void ShowNetActivity();
	void ShowOrderLoc();
	void ShowScreenFadeEffect();
	void ShowSpawnRecallWait();
	void ShowSpawnTieEffect();
	void ShowWarpIndicator();
	void ShowWormholeIndicator();

	bool StartScene(DaPerson** sceneAtom);

	void SyncScene();

	void ToggleTopDownView();
	void ToggleFirstThirdPersonView();
	DaAtom *FindAtomAt(DaPerson* sceneAtom, const QPoint &pxFind, unsigned int &uiPoly, AuPoint &fPolyLoc) const;
	DaAtom *FindAtomClosestTo(DaPerson* sceneAtom, const QPoint &pxFind) const;

	// 'fPanelLoc' is the location within the panel that was hit.
	// 'hitLoc' is the hit location, relative to the camera.
	DaAtom *FindControlAt(DaPerson* sceneAtom, const QPoint &pxFind, qint16 &idPanel, AuPoint &fPanelLoc, AuPoint& hitLoc) const;

	QPoint ToPixel(const AuPoint &fLoc) const;
	AuPoint ToFPixel(const AuPoint &fLoc) const;
	AuPoint FromPixel(const AuPoint &fpxLoc) const;
	void StartCommand(DdCommand *pCommand);
	void OnViewFilterGlare();
	void OnViewFreeflyingcamera();

	inline bool IsDesignMode() const
	{
		// When an atom is getting input, we're not in design mode.
		if(IsCharacterInputMode())
			return false;

		// Must be in design environment to be in design mode.
		if(!IsInDesignEnvironment())
			return false;

		return m_IsDesignMode;
	}

	// Key command processing.
	bool ProcessCameraKeyCommand(QKeyEvent *e);

	// Mouse event processing
	bool ProcessAtomMousePress(QMouseEvent* e);
	bool ProcessAtomMouseRelease(QMouseEvent* e);
	bool ProcessAtomWheelEvent(QWheelEvent* e);
	bool ProcessCameraMouseMove(QMouseEvent* e);
	bool ProcessUIMousePress(QMouseEvent* e);
	bool ProcessUIMouseRelease(QMouseEvent* e);

	// Camera functions.
	void ResetCamera();
	void StopCamera();
	void MoveForward(double distance);	// Negative is backward.
	void Roll(double rAngle);
	void Pitch(double rAngle);
	void Yaw(double rAngle);
	void PanHorizontal(double feet);
	void PanVertical(double feet);
	void ChangeCameraSpeed(float ftpersec);	// Positive faster, negative slower/reverse.

	// Sync functions.
	void AtomAdded(DaAtom* atom);
	void AtomChanged(DaAtom* atom);
	void AtomDeparted(const AuId& atomId);
	void AtomParentChanged(const DaAtom* atom);
	void AtomsCleared();

	// Base class overrides.
protected:
	virtual void resizeEvent(QResizeEvent* event);
};

#endif // AuDesignerView_H
