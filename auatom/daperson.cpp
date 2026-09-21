//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daperson.cpp,v 1.2368 2022/05/19 15:08:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daperson.h"
//
#include "auatomrenderinfo.h"
#include "aubddesign.h"
#include "aubox.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "audice.h"
#include "aueffect.h"
#include "augleffects.h"
#include "auglobal.h"
#include "auhitnotify.h"
#include "aunoise.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "ausensorblip.h"
#include "auskill.h"
#include "ausound.h"
#include "austory.h"
#include "austylesheet.h"
#include "ausynccontroller.h"
#include "daanimal.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacapsule.h"
#include "dadesigner.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldll.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dpconsolenav.h"
#include "dpconsoletransporter.h"
#include "dpstationhelm.h"
#include "psentity.h"
#include "psweather.h"
#include <QSqlError>
#include <QSqlQuery>
//#include <QFile>			//DHH only for debug output
//#include <QTextStream>	//DHH only for debug output

#define c_AvatarTurnRate				(PIf*2 * 3)				// Radians per second turn rate for avatars.
#define c_SecsToOffLine					30						// Number of seconds before I go off line, since m_tmLastOnLine.

#define d_DoorLock						SOUND_Bleep1
#define d_Lights						SOUND_Bleep6
#define d_OrbitRings					SOUND_Bleep2
#define d_SwitchStation					SOUND_Bleep1
#define d_SystemNames					SOUND_Bleep7
#define d_Wormholes						SOUND_Bleep8

static GLuint g_TexEntity = 0;
static GLuint g_TexLightOmni = 0;

static GLuint s_BlipBdCivTexture = 0;
static GLuint s_BlipBdMilTexture = 0;
static GLuint s_BlipCreatureTexture = 0;
static GLuint s_BlipEquipmentTexture = 0;
static GLuint s_HelmetHUDTexL = 0;				// Helmet HUD texture for lefties.
static GLuint s_HelmetHUDTexR = 0;				// Helmet HUD texture for righties.
static GLuint s_HelmetHUDTexLBroken = 0;		// Helmet HUD texture for lefties.
static GLuint s_HelmetHUDTexRBroken = 0;		// Helmet HUD texture for righties.
static GLuint s_SnowTexture = 0;				// texture used for snow

static int s_CurSecond = 0;	// Current second for accumulating frames rendered.
static unsigned int	s_FrameMsLast;	// Frame time of last frame.
static int s_Frames[60];	// The number of frames actually rendered in each second.
AuMatrix4x4 s_LastParticleMatrix;

// Table of control inputs.
DiControlInputArray s_PersonControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_PersonDefaults[] =
{
	// Movement and Look Direction
	DiControlInput(QObject::tr("Look Up"), DaPerson::CI_LookUp,
				Qt::Key_unknown, Qt::NoModifier, DiControlInput::MT_Up,
				Qt::NoModifier, DiControlInput::JT_None, 0,30000,
				true,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Down"), DaPerson::CI_LookDown,
				Qt::Key_unknown, Qt::NoModifier, DiControlInput::MT_Down,
				Qt::NoModifier, DiControlInput::JT_None, 0,30000,
				true,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Turn Left"), DaPerson::CI_TurnLeft,
				Qt::Key_unknown, Qt::NoModifier, DiControlInput::MT_Left,
				Qt::NoModifier, DiControlInput::JT_None, 0,30000,
				true,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Turn Right"), DaPerson::CI_TurnRight,
				Qt::Key_unknown, Qt::NoModifier, DiControlInput::MT_Right,
				Qt::NoModifier, DiControlInput::JT_None, 0,30000,
				true,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Move Forward Continuously"), DaPerson::CI_MoveForwardContinuously,
				Qt::Key_Q, Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Move Forward"), DaPerson::CI_MoveForward,
				Qt::Key_W, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Move Backward"), DaPerson::CI_MoveBackward,
				Qt::Key_S, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Move Left"), DaPerson::CI_MoveLeft,
				Qt::Key_A, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None,
				0, 30000),
	DiControlInput(QObject::tr("Move Right"), DaPerson::CI_MoveRight,
				Qt::Key_D, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None,
				0, 30000),
	DiControlInput(QObject::tr("Move Up"), DaPerson::CI_MoveUp,
				Qt::Key_C, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None,
				0, 30000),
	DiControlInput(QObject::tr("Move Down"), DaPerson::CI_MoveDown,
				Qt::Key_Z, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None,
				0, 30000),
	DiControlInput(QObject::tr("Run Modifier"), DaPerson::CI_Run, Qt::Key_Shift,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Jump. Brake in Zero-G or When Flying."),
				DaPerson::CI_BrakeOrJump, Qt::Key_Space, Qt::NoModifier),
	DiControlInput(QObject::tr("Brake"),
				DaPerson::CI_Brake, Qt::Key_unknown, Qt::NoModifier),
	DiControlInput(QObject::tr("Jump"),
				DaPerson::CI_Jump, Qt::Key_unknown, Qt::NoModifier),
	DiControlInput(QObject::tr("Jump Power Decrease"),
				DaPerson::CI_JumpPowerDecrease,
				Qt::Key_J, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Jump Power Increase"),
				DaPerson::CI_JumpPowerIncrease,
				Qt::Key_J, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Move Power Decrease"),
				DaPerson::CI_MovePowerDecrease,
				Qt::Key_M, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Move Power Increase"),
				DaPerson::CI_MovePowerIncrease,
				Qt::Key_M, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Body positioning
	DiControlInput(QObject::tr("Crouch"), DaPerson::CI_Crouch,
				Qt::Key_CapsLock, Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier,
				DiControlInput::JT_None),
	DiControlInput(QObject::tr("Prone/Stand Toggle"),
				DaPerson::CI_ProneStandToggle, Qt::Key_X, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Sit/Stand Toggle"),
				DaPerson::CI_SitStandToggle, Qt::Key_S, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Mate Toggle"),
			QObject::tr("Pose your body for mating, if you are a naked adult creature that is not asexual. "
				"A creature of opposite gender can mount your body when in this pose, to impregnate the female. "
				"Hermaphrodites impregnate each other.<br><b>ONLINE ONLY. NO EFFECT IN SOLO PLAY.</b>"),
				DaPerson::CI_ReceptiveMateToggle, Qt::Key_X, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Enter/Exit Console, Ladder, Mount or Vehicle. Drop Rider. Pick Up Body."),
			DaPerson::CI_EnterExit,
			Qt::Key_E, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false, true),

	// Item selection and usage.
	DiControlInput(QObject::tr("Select Next Item"),
				DaPerson::CI_SelectNextItem, Qt::Key_BraceRight, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false, false,
				DiControlInput::WT_AwayFromUser),
	DiControlInput(QObject::tr("Select Previous Item"),
				DaPerson::CI_SelectPreviousItem, Qt::Key_BraceLeft, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false, false,
				DiControlInput::WT_TowardUser),
	DiControlInput(QObject::tr("Use Item"),
				DaPerson::CI_UseItem, Qt::Key_U, Qt::NoModifier,
				DiControlInput::MT_ButtonMiddle, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Drop Item"),
				DaPerson::CI_DropItem, Qt::Key_unknown, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Throw Item"),
				DaPerson::CI_ThrowItem, Qt::Key_V, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Fire Weapon/Use Item on Target"),
				DaPerson::CI_FireItem, Qt::Key_F, Qt::NoModifier,
				DiControlInput::MT_ButtonLeft, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true, true),
	DiControlInput(QObject::tr("Toggle Gun Sight/Use Item Alternate Action on Target"),
				DaPerson::CI_FireItemAlt, Qt::Key_G, Qt::ShiftModifier,
				DiControlInput::MT_ButtonLeft, Qt::AltModifier,
				DiControlInput::JT_None, 0, 100, true, true),
	DiControlInput(QObject::tr("Reload/Recharge Item"),
			QObject::tr("Reloads current weapon from gear. Reloads turret when sitting in a turret."),
				DaPerson::CI_ReloadItem, Qt::Key_R, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Eat Food from Gear"),
				DaPerson::CI_EatFood, Qt::Key_T, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Open/Close Helmet Visor"),
				DaPerson::CI_HelmetVisor, Qt::Key_H, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Place Item on Display"),
			QObject::tr("The item is prepared for permanent display. "
				"All plant and animal DNA contained in the item is killed. "
				"DNA samples can no longer be taken; plants cannot produce fruit or seeds."),
				DaPerson::CI_PlaceItem, Qt::Key_P, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false, true),
	DiControlInput(QObject::tr("Say 'Hello' on the Voice channel."),
			QObject::tr("This command is disabled while you are in a design environment. "),
				DaPerson::CI_SayHello, Qt::Key_R, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Room Light Brighter."),
			QObject::tr("Adjusts the light brighter, in the room you currently occupy. "),
				DaPerson::CI_RoomLightsBrighter, Qt::Key_L, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Room Light Darker."),
			QObject::tr("Adjusts the light darker, in the room you currently occupy. "),
				DaPerson::CI_RoomLightsDarker, Qt::Key_K, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Door Open"),
			QObject::tr("Opens the door you are facing. The door must be closed."),
				DaPerson::CI_DoorOpen, Qt::Key_O, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false, true),
	DiControlInput(QObject::tr("Door Lock"),
			QObject::tr("Locks or unlocks the door you are facing. The door must be closed."),
				DaPerson::CI_DoorLockToggle, Qt::Key_O, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false, true),

	// Map usage positions.
	DiControlInput(QObject::tr("Map Current Item to Position 1"), DaPerson::CI_MapPos0, Qt::Key_1, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 2"), DaPerson::CI_MapPos1, Qt::Key_2, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 3"), DaPerson::CI_MapPos2, Qt::Key_3, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 4"), DaPerson::CI_MapPos3, Qt::Key_4, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 5"), DaPerson::CI_MapPos4, Qt::Key_5, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 6"), DaPerson::CI_MapPos5, Qt::Key_6, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 7"), DaPerson::CI_MapPos6, Qt::Key_7, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 8"), DaPerson::CI_MapPos7, Qt::Key_8, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 9"), DaPerson::CI_MapPos8, Qt::Key_9, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Map Current Item to Position 10"), DaPerson::CI_MapPos9, Qt::Key_0, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),

	// Select mapped usage positions.
	DiControlInput(QObject::tr("Select Item Mapped to Position 1"), DaPerson::CI_SelectMapPos0, Qt::Key_1, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 2"), DaPerson::CI_SelectMapPos1, Qt::Key_2, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 3"), DaPerson::CI_SelectMapPos2, Qt::Key_3, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 4"), DaPerson::CI_SelectMapPos3, Qt::Key_4, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 5"), DaPerson::CI_SelectMapPos4, Qt::Key_5, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 6"), DaPerson::CI_SelectMapPos5, Qt::Key_6, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 7"), DaPerson::CI_SelectMapPos6, Qt::Key_7, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 8"), DaPerson::CI_SelectMapPos7, Qt::Key_8, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 9"), DaPerson::CI_SelectMapPos8, Qt::Key_9, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),
	DiControlInput(QObject::tr("Select Item Mapped to Position 10"), DaPerson::CI_SelectMapPos9, Qt::Key_0, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0,100,false),

	// Spawn Controls
	DiControlInput(QObject::tr("Declare Home City"),
				DaPerson::CI_SpawnTie, Qt::Key_AsciiTilde, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Recall Home"), DaPerson::CI_SpawnRecall,
				Qt::Key_Backspace, Qt::AltModifier, DiControlInput::MT_None,
				Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Command console modes.
	DiControlInput(QObject::tr("Command Station Command Mode"),
				DaPerson::CI_ConsoleCommand, Qt::Key_C, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Command Station Engineer Mode"),
				DaPerson::CI_ConsoleEngineer, Qt::Key_E, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Command Station Fire Control Mode"),
				DaPerson::CI_ConsoleWeapon, Qt::Key_F, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Command Station Helm Mode"),
				DaPerson::CI_ConsolePilot, Qt::Key_H, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Command Station Navigator Mode"),
				DaPerson::CI_ConsoleNavigator, Qt::Key_N, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Command Station Sensor Mode"),
				DaPerson::CI_ConsoleSensor, Qt::Key_S, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Command Station Shield Mode"),
				DaPerson::CI_ConsoleShield, Qt::Key_D, Qt::AltModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Command Console Adjustments
	DiControlInput(QObject::tr("Move Console Screen Lower"), DaPerson::CI_ConsoleLower,
				Qt::Key_BracketLeft, Qt::AltModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Move Console Screen Higher"), DaPerson::CI_ConsoleRaise,
				Qt::Key_BracketRight, Qt::AltModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Move Console Screen Farther"), DaPerson::CI_ConsoleMoveFarther,
				Qt::Key_Semicolon, Qt::AltModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Move Console Screen Closer"), DaPerson::CI_ConsoleMoveCloser,
				Qt::Key_Apostrophe, Qt::AltModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Tip Console Screen Away"), DaPerson::CI_ConsoleRotateAway,
				Qt::Key_Colon, Qt::ShiftModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Tip Console Screen Closer"), DaPerson::CI_ConsoleRotateToward,
				Qt::Key_QuoteDbl, Qt::ShiftModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None),
};
int s_NumPersonDefaults = sizeof(s_PersonDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Helper for selecting next/previous current item.
class SelItem_
{
public:
	AuGearPos		pos;				// Can reference a gear item or a personal body weapon.
	INVENTORYSUB	sub;

public:
	SelItem_(const AuGearPos& p, INVENTORYSUB s)
	{
		pos = p;
		sub = s;
	}
	SelItem_()
	{
		sub = IPS_Item;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaPerson::DaPerson(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, GROWTHSTAGE stage,
		quint32 empireId) : DaSentient(dna, chroma, hand, name, empireId, stage)
{
	Q_ASSERT(CI_LASTITEM <= 256);	// Control input values have exceeded the usable range of 0-255.
	m_AccountBalance = 0;
	m_AccountInactive = false;
	m_AccountOnLine = ACCT_Count;
	m_AgentBits = 0;
	m_ArenaShipDesign = 0;
	m_AutoEat = false;

	for(int cs=0; cs<PSOUND_Count; cs++)
	{
		m_AudioPerson[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}

	m_BerthType = AuRosterEntry::Invalid;
	m_byRotSpeedX = 0;
	m_byRotSpeedZ = 0;
	m_Con = false;
	m_CurrentGearSub = IPS_Item;
	m_InitScenario = 0;
	m_Lineage = LINEAGE_None;
	m_LoanBalance = 0;

	// Configure default mappings of items.
	for(int mp=0; mp<10; mp++)
		m_MapPos[mp] = AuHotKeyGear();	// Deactivated/unmapped.

	m_EnvHumidity = 0;
	m_EnvWindForce = 0;
	m_PSEntity = 0;
	m_Ping = 0;
	m_PowerJump = 15;
	m_PowerMove = 15;
	m_PowerMoveMax = 15;
	m_PrecipParticles = 0;
	m_Reputation = 0;
	m_RewardsOwed = 0;
	m_Script = 0;
	m_ShipWindForce = 0;
	m_TurnCount = 0;
	m_TurnIndex = 0;
	m_uiActions = ACT_None;
	m_WeightCarried = 0;
	m_Worms = 0;
	m_WxBAR = 0;
	m_WxOAT = -273;	// = 0 Kelvin
	m_WxRelHumidity = 0;
}

DaPerson::DaPerson()
{
	Q_ASSERT(CI_LASTITEM <= 256);	// Control input values have exceeded the usable range of 0-255.
	m_AccountBalance = 0;
	m_AccountInactive = true;
	m_AccountOnLine = ACCT_Count;
	m_AgentBits = 0;
	m_ArenaShipDesign = 0;
	m_AutoEat = false;

	for(int cs=0; cs<PSOUND_Count; cs++)
	{
		m_AudioPerson[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}

	m_BerthType = AuRosterEntry::Invalid;
	m_byRotSpeedX = 0;
	m_byRotSpeedZ = 0;
	m_Con = false;
	m_CurrentGearSub = IPS_Item;
	m_InitScenario = 0;
	m_Lineage = LINEAGE_None;
	m_LoanBalance = 0;

	// Configure default mappings of items.
	for(int mp=0; mp<10; mp++)
		m_MapPos[mp] = AuHotKeyGear();	// Deactivated/unmapped.

	m_EnvHumidity = 0;
	m_EnvWindForce = 0;
	m_PSEntity = 0;
	m_Ping = 0;
	m_PowerJump = 15;
	m_PowerMove = 15;
	m_PowerMoveMax = 15;
	m_PrecipParticles = 0;
	m_Reputation = 0;
	m_RewardsOwed = 0;
	m_Script = 0;
	m_ShipWindForce = 0;
	m_TurnCount = 0;
	m_TurnIndex = 0;
	m_uiActions = ACT_None;
	m_WeightCarried = 0;
	m_Worms = 0;
	m_WxBAR = 0;
	m_WxOAT = -273;	// = 0 Kelvin
	m_WxRelHumidity = 0;
}

void DaPerson::InitDNA()
{
	/*
	DhDNA dna;
	dna.InitDefaultHuman();
	SetDNA3(dna);
	*/
	AuDNA dna;
	dna.InitDefaultHuman();
	SetDNA(dna);
}

DaPerson::~DaPerson()
{
	m_Jobs.Clear();

	if(m_PrecipParticles)
		delete m_PrecipParticles;

	if(m_PSEntity)
		delete m_PSEntity;

	if(m_Script)
		delete m_Script;
}

DaPerson::DaPerson(const DaPerson &rhs) : DaSentient(rhs)
{
	//qDebug() << "DaPerson::DaPerson" << Id() << Name() << "this" << this;
	m_AgentBits = rhs.m_AgentBits;
	m_AutoEat = rhs.m_AutoEat;
	m_PowerJump = rhs.m_PowerJump;
	m_PowerMove = rhs.m_PowerMove;
	m_PowerMoveMax = rhs.m_PowerMoveMax;
	m_Stats = rhs.m_Stats;

	rhs.m_PersonLock.lockForRead();
	m_ArenaEntered = rhs.m_ArenaEntered;
	m_ArenaExitInv = rhs.m_ArenaExitInv;
	m_ArenaExitLoc = rhs.m_ArenaExitLoc;
	m_ArenaExitParent = rhs.m_ArenaExitParent;
	m_Dad = rhs.m_Dad;
	m_DesignPass = rhs.m_DesignPass;
	m_LastClaim = rhs.m_LastClaim;
	m_Log = rhs.m_Log;
	m_LogTime = rhs.m_LogTime;
	m_Mom = rhs.m_Mom;
	m_Selfie = rhs.m_Selfie;
	m_Worms = rhs.m_Worms;

	m_AccountBalance = rhs.m_AccountBalance;
	m_LoanBalance = rhs.m_LoanBalance;
	m_tmBalanceChange = rhs.m_tmBalanceChange;

	m_ArenaShipDesign = rhs.m_ArenaShipDesign;
	m_Con = rhs.m_Con;
	m_InitScenario = rhs.m_InitScenario;
	m_Lineage = rhs.m_Lineage;
	m_tmOfficeChange = rhs.m_tmOfficeChange;

	m_CurrentGearPos = rhs.m_CurrentGearPos;
	m_CurrentGearSub = rhs.m_CurrentGearSub;
	m_tmItemUseChange = rhs.m_tmItemUseChange;

	m_HitNotify = rhs.m_HitNotify;
	m_tmHitChange = rhs.m_tmHitChange;

	m_Domain = rhs.m_Domain;
	m_DomainChange = rhs.m_DomainChange;

	for(int mp=0; mp<10; mp++)
		m_MapPos[mp] = rhs.m_MapPos[mp];	// Deactivated/unmapped.

	m_Reputation = rhs.m_Reputation;
	m_tmReputationChange = rhs.m_tmReputationChange;

	m_BerthType = rhs.m_BerthType;
	m_SpawnBd = rhs.m_SpawnBd;
	m_SpawnBdName = rhs.m_SpawnBdName;
	m_SpawnBdSystem = rhs.m_SpawnBdSystem;
	m_SpawnBdSystemName = rhs.m_SpawnBdSystemName;
	m_SpawnDevLL = rhs.m_SpawnDevLL;
	m_SpawnShip = rhs.m_SpawnShip;
	m_SpawnShipIsPrivate = rhs.m_SpawnShipIsPrivate;
	m_SpawnShipName = rhs.m_SpawnShipName;
	m_tmSpawnChange = rhs.m_tmSpawnChange;		// Spawning location.

	m_uiActions = rhs.m_uiActions;
	m_byRotSpeedX = rhs.m_byRotSpeedX;
	m_byRotSpeedZ = rhs.m_byRotSpeedZ;
	m_tmLastBrake = rhs.m_tmLastBrake;
	m_LastJumpOrMovePower = rhs.m_LastJumpOrMovePower;
	m_tmLastMoveControl = rhs.m_tmLastMoveControl;
	m_tmLastMoveLControl = rhs.m_tmLastMoveLControl;
	m_tmLastMoveSControl = rhs.m_tmLastMoveSControl;
	m_tmLastMoveVControl = rhs.m_tmLastMoveVControl;
	m_tmLastWeaponFired = rhs.m_tmLastWeaponFired;

	m_TurnCount = rhs.m_TurnCount;
	m_TurnIndex = rhs.m_TurnIndex;
	for(int ti=0; ti<20; ti++)
	{
		m_TurnX[ti] = rhs.m_TurnX[ti];
		m_TurnZ[ti] = rhs.m_TurnZ[ti];
		m_tmTurn[ti] = rhs.m_tmTurn[ti];
	}
	rhs.m_PersonLock.unlock();

	rhs.m_GearLock.lockForRead();
	m_Gear = rhs.m_Gear;
	m_tmGearChange = rhs.m_tmGearChange;
	m_WeightCarried = rhs.m_WeightCarried;
	rhs.m_GearLock.unlock();

	rhs.m_JobsLock.lockForRead();
	for(AuJobList::const_iterator ji = rhs.m_Jobs.cbegin(); ji != rhs.m_Jobs.cend(); ji++)
		m_Jobs.append((*ji)->Clone());
	m_tmJobsChange = rhs.m_tmJobsChange;
	rhs.m_JobsLock.unlock();

	rhs.m_MemoryLock.lockForRead();
	for(AuMemories::const_iterator ji = rhs.m_Memories.cbegin(); ji != rhs.m_Memories.cend(); ji++)
		m_Memories.append((*ji)->Clone());
	rhs.m_MemoryLock.unlock();

	rhs.m_OnLineLock.lockForRead();
	m_AccountInactive = rhs.m_AccountInactive;
	m_AccountOnLine = rhs.m_AccountOnLine;
	m_Ping = rhs.m_Ping;
	m_tmLastOnLine = rhs.m_tmLastOnLine;
	rhs.m_OnLineLock.unlock();

	rhs.m_RewardsLock.lockForRead();
	m_RewardsOwed = rhs.m_RewardsOwed;
	m_tmRewardChange = rhs.m_tmRewardChange;
	rhs.m_RewardsLock.unlock();

	rhs.m_ScriptLock.lock();
	if(rhs.m_Script)
		m_Script = new AuScript(*rhs.m_Script);
	else
		m_Script = 0;
	rhs.m_ScriptLock.unlock();

	rhs.m_SeaChestLock.lockForRead();
	for(AuItemHash::const_iterator ii = rhs.m_SeaChest.begin(); ii != rhs.m_SeaChest.end(); ii++)
		m_SeaChest.insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));
	m_tmSeaChestChange = rhs.m_tmSeaChestChange;
	rhs.m_SeaChestLock.unlock();

	m_WxBAR = rhs.m_WxBAR;
	m_WxOAT = rhs.m_WxOAT;
	m_WxRelHumidity = rhs.m_WxRelHumidity;
	m_EnvHumidity = 0;
	m_EnvWindForce = 0;
	m_ShipWindForce = 0;
	for(int cs=0; cs<PSOUND_Count; cs++)
	{
		m_AudioPerson[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}

	m_ClosestGravityDrive = rhs.m_ClosestGravityDrive;
	m_ClosestPowerPlant = rhs.m_ClosestPowerPlant;
	m_ClosestSensor = rhs.m_ClosestSensor;
	m_ClosestFTLDrive = rhs.m_ClosestFTLDrive;
	m_ClosestSiren = rhs.m_ClosestSiren;

	m_CurrentLight = rhs.m_CurrentLight;

	m_PrecipParticles = 0;				// will be recreated when needed
	m_PSEntity = 0;	// will be recreated when needed
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaPerson::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaSentient::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	// hasWhat bits
	//  0x01 = spawn location change
	//  0x02 = hit change
	//  0x04 = hunger change									SCENE ATOM ONLY
	//  0x08 = hasMore2 follows
	//  0x10 = office change, arena team/design change, lineage, con
	//  0x20 = item use change
	//  0x40 = inventory change
	//  0x80 = hasMore follows									SCENE ATOM ONLY
	// hasMore bits												SCENE ATOM ONLY
	//  0x01 = domain change									SCENE ATOM ONLY
	//  0x02 = assertive transform change, client must accept	SCENE ATOM ONLY
	//  0x04 = story change										SCENE ATOM ONLY
	//  0x08 = reputation change and wormholes					SCENE ATOM ONLY
	//  0x10 = reward change									SCENE ATOM ONLY
	//  0x20 = account balance change							SCENE ATOM ONLY
	//  0x40 = did spawn tie									SCENE ATOM ONLY
	//  0x80 = is awaiting recall home							SCENE ATOM ONLY
	// hasMore2 bits
	//  0x1f = agent bits in bottom 5 bits
	//  0x40 = spring power										SCENE ATOM ONLY
	//  0x80 = sea chest change									SCENE ATOM ONLY
	quint8 hasWhat = 0;
	quint8 hasMore = 0;											// all hasMore bits are SCENE ATOM ONLY.
	quint8 hasMore2 = m_AgentBits;								// everybody gets agent bits all the time.
	if(lastTx.IsNull())
	{
		// This is the first time the atom has been sent to the client.
		if(Is(sceneAtom.Id()))
		{
			// Scene atom gets everything.
			hasWhat = 0xfd;										// everything except hit notification
			hasMore = 0x3F;										// everything except spawn tie bits
			hasMore2 |= 0xc0;									// sea chest in case I get into my berth
		}
		else
		{
			// Other atoms get everything except the SCENE ATOM ONLY stuff.
			hasWhat = 0x7b;										// everything that is not scene-atom only.
		}
	}
	else
	{
		// This is not the first time the atom has been sent to the client.
		m_PersonLock.lockForRead();
		if(!m_tmSpawnChange.IsNull() && m_tmSpawnChange >= lastTx)
			hasWhat |= 0x01;
		if(!m_tmOfficeChange.IsNull() && m_tmOfficeChange >= lastTx)
			hasWhat |= 0x10;
		if(!m_tmItemUseChange.IsNull() && m_tmItemUseChange >= lastTx)
			hasWhat |= 0x20;
		m_PersonLock.unlock();

		m_GearLock.lockForRead();
		if(!m_tmGearChange.IsNull() && m_tmGearChange >= lastTx)
			hasWhat |= 0x40;
		m_GearLock.unlock();

		//scene-atom only
		if(Is(sceneAtom.Id()))
		{
			hasWhat |= 0x80;	// hasMore byte AND terrain movement vector needed

			AuElapsedTimer hc = HungerChange();
			if(!hc.IsNull() && hc >= lastTx)
				hasWhat |= 0x04;

			m_JobsLock.lockForRead();
			if(!m_tmJobsChange.IsNull() && m_tmJobsChange >= lastTx)
				hasMore |= 0x04;
			m_JobsLock.unlock();

			m_PersonLock.lockForRead();

			if(!m_DomainChange.IsNull() && m_DomainChange >= lastTx)
				hasMore |= 0x01;

			if(!m_tmHitChange.IsNull() && m_tmHitChange >= lastTx)
				hasWhat |= 0x02;

			if(!m_tmReputationChange.IsNull() && m_tmReputationChange >= lastTx)
				hasMore |= 0x08;
			if(!m_tmBalanceChange.IsNull() && m_tmBalanceChange >= lastTx)
				hasMore |= 0x20;

			m_PersonLock.unlock();

			m_RewardsLock.lockForRead();
			if(!m_tmRewardChange.IsNull() && m_tmRewardChange >= lastTx)
				hasMore |= 0x10;
			m_RewardsLock.unlock();

			if(DidSpawnTie())
				hasMore |= 0x40;
			if(IsAwaitingSpawnRecall())
				hasMore |= 0x80;

			// Sea chest is sent to scene atom when they are in their berth.
			m_SeaChestLock.lockForRead();
			if(!m_tmSeaChestChange.IsNull() && m_tmSeaChestChange >= lastTx)
				hasMore2 |= 0x80;
			m_SeaChestLock.unlock();

			if(!m_LastJumpOrMovePower.IsNull() && m_LastJumpOrMovePower >= lastTx)
				hasMore2 |= 0x40;
		}
	}

	if(hasMore2)
		hasWhat |= 0x08;

	// Inform the client what will be included in this packet.
	toClient << hasWhat;
	if(hasWhat & 0x80)
		toClient << hasMore;

	// Send the data.
	if(hasWhat & 0x01)
	{
		m_PersonLock.lockForRead();
		toClient << m_SpawnBd;
		toClient << m_SpawnBdName;
		toClient << m_SpawnBdSystem;
		toClient << m_SpawnBdSystemName;
		toClient << m_SpawnDevLL;
		toClient << m_SpawnShip;
		toClient << m_SpawnShipIsPrivate;
		toClient << m_SpawnShipName;
		m_PersonLock.unlock();
	}

	if(hasWhat & 0x02)
	{
		m_PersonLock.lockForRead();
		m_HitNotify >> toClient;
		m_PersonLock.unlock();
	}

	if(hasWhat & 0x04)
	{
		quint16 hunger = CurrentHunger();
		if(m_AutoEat)
			hunger |= 0x8000;
		toClient << hunger;
	}

	if(hasWhat & 0x08)
		toClient << hasMore2;

	if(hasWhat & 0x10)
	{
		m_PersonLock.lockForRead();
		quint8 ui8 = 0;
		ui8 |= m_Lineage << 3;	// Requires 2 bits. b3 and b4
		if(m_Con)
			ui8 |= 0x80;		// b7

		toClient << ui8;
		toClient << m_ArenaShipDesign;
		m_PersonLock.unlock();
	}

	if(hasWhat & 0x20)
	{
		m_PersonLock.lockForRead();
		AuGearPos ip(m_CurrentGearPos);
		quint8 isub = m_CurrentGearSub;
		m_PersonLock.unlock();

		toClient << ip;
		toClient << isub;
	}

	if(hasWhat & 0x40)
	{
		m_GearLock.lockForRead();
		toClient << m_Gear;
		m_GearLock.unlock();
	}

	if(hasWhat & 0x80)
	{
		if(hasMore & 0x01)
		{
			m_PersonLock.lockForRead();
			toClient << m_Domain;
			m_PersonLock.unlock();
		}

		if(hasMore & 0x04)
		{
			m_JobsLock.lockForRead();
			toClient << m_Jobs;									// Send jobs info.
			m_JobsLock.unlock();
		}

		if(hasMore & 0x08)
		{
			m_PersonLock.lockForRead();
			toClient << m_Reputation;
			toClient << m_Worms;
			m_PersonLock.unlock();
		}

		if(hasMore & 0x10)
		{
			m_RewardsLock.lockForRead();
			toClient << m_RewardsOwed;
			m_RewardsLock.unlock();
		}

		if(hasMore & 0x20)
		{
			m_PersonLock.lockForRead();
			toClient << m_AccountBalance;
			toClient << m_LoanBalance;
			m_PersonLock.unlock();
		}
	}

	if(hasMore2 & 0x80)
	{
		m_SeaChestLock.lockForRead();
		toClient << m_SeaChest;
		m_SeaChestLock.unlock();
	}

	// Appended this one to the end so I wouldn't have to change net version.
	// It just fixes selection of units by scene atom, not critical.
	if(hasWhat & 0x01)
	{
		m_PersonLock.lockForRead();
		toClient << (quint8)m_BerthType;
		m_PersonLock.unlock();
	}

	if(hasMore2 & 0x40)
	{
		quint8 jumpmove = (m_PowerJump << 4) | m_PowerMoveMax;
		toClient << jumpmove;
	}
}

// Receive updates to this atom from a server.
void DaPerson::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaSentient::RxUpdate(fromServer, isFirstUpdate);

	// Find out what is in this data packet.
	quint8 hasWhat;
	quint8 hasMore;
	fromServer >> hasWhat;
	if(hasWhat & 0x80)
		fromServer >> hasMore;
	else
		hasMore = 0;

	if(hasWhat & 0x01)
	{
		fromServer >> m_SpawnBd;
		fromServer >> m_SpawnBdName;
		fromServer >> m_SpawnBdSystem;
		fromServer >> m_SpawnBdSystemName;
		fromServer >> m_SpawnDevLL;
		fromServer >> m_SpawnShip;
		fromServer >> m_SpawnShipIsPrivate;
		fromServer >> m_SpawnShipName;
	}
	
	if(hasWhat & 0x02)
		m_HitNotify << fromServer;

	// Fake a hit notify
	//double rot = Rotation().z;
	//while(rot < 0)
		//rot += PI*2;
	//WEAPONEFFECT rotEffect = (WEAPONEFFECT)(rot * (WE_Corrosion+1) / (PI*2));	// pick weapon effect based on facing
	//qDebug() << "hitEffect" << rotEffect << EffectName(rotEffect);
	//m_HitNotify = AuHitNotify(AuPoint(0,0,0), rotEffect,10,10, rotEffect,10,10);
	//m_HitNotify.RestartTime();
	//if(!m_HitNotify.IsHit())
	//{
		//m_HitNotify = AuHitNotify(AuPoint(0,1,0), WE_Pierce,10,10, WE_None,0,0);
		//m_HitNotify.RestartTime();
	//}

	if(hasWhat & 0x04)
	{
		quint16 hunger;
		fromServer >> hunger;
		SetHunger(hunger&0x7fff);
		m_AutoEat = hunger & 0x8000;
	}

	quint8 hasMore2;
	if(hasWhat & 0x08)
	{
		fromServer >> hasMore2;
		m_AgentBits = hasMore2 & 0x3f;
	}
	else
	{
		hasMore2 = 0;
		m_AgentBits = 0;
	}
	
	//qDebug() << "DaPerson::RxUpdate" << Id() << Name()
		//<< QString("hasWhat 0x%1").arg(hasWhat,0,16)
		//<< QString("hasMore 0x%1").arg(hasMore,0,16)
		//<< QString("hasMore2 0x%1").arg(hasMore,0,16);
	
	if(hasWhat & 0x10)
	{
		quint8 ui8;

		fromServer >> ui8;
		fromServer >> m_ArenaShipDesign;

		m_Lineage = (LINEAGE_TYPE)((ui8 & 0x18) >> 3);			// Bits b3 and b4
		m_Con = ui8 & 0x80;						// Bit b7.
	}

	if(hasWhat & 0x20)
	{
		fromServer >> m_CurrentGearPos;

		quint8 isub;
		fromServer >> isub;
		m_CurrentGearSub = (INVENTORYSUB)isub;
	}

	if(hasWhat & 0x40)
	{
		m_GearLock.lockForWrite();
		fromServer >> m_Gear;
		m_WeightCarried = m_Gear.Weight();
		//qDebug() << "DaPerson::RxUpdate" << Id() << Name() << "inv" << m_Gear.size() << "wt" << m_WeightCarried;
		//qDebug() << m_Gear;
		m_GearLock.unlock();
	}

	if(hasWhat & 0x80)
	{
		if(hasMore & 0x01)
			fromServer >> m_Domain;

		if(hasMore & 0x04)
			fromServer >> m_Jobs;								// Open story info.

		if(hasMore & 0x08)
		{
			fromServer >> m_Reputation;
			fromServer >> m_Worms;
		}

		if(hasMore & 0x10)
			fromServer >> m_RewardsOwed;

		if(hasMore & 0x20)
		{
			fromServer >> m_AccountBalance;
			fromServer >> m_LoanBalance;
		}

		if(hasMore & 0x40)
			m_uiActions |= ACT_SpawnTie;

		if(hasMore & 0x80)
			m_uiActions |= ACT_SpawnRecall;
		else
			m_uiActions &= ~ACT_SpawnRecall;
	}
	else
		m_uiActions &= ~ACT_SpawnRecall;

	if(hasMore2 & 0x80)
		fromServer >> m_SeaChest;

	// Appended this one to the end so I wouldn't have to change net version.
	// It just fixes selection of units by scene atom, not critical.
	if(hasWhat & 0x01)
	{
		quint8 ui8;
		fromServer >> ui8;
		m_BerthType = (AuRosterEntry::TYPE)ui8;
	}

	if(hasMore2 & 0x40)
	{
		quint8 jumpmove;
		fromServer >> jumpmove;
		m_PowerJump = jumpmove >> 4;		// top nibble
		m_PowerMoveMax = jumpmove & 0x0f;	// bottom nibble
	}
}

// Refresh data from valid counterpart.
bool DaPerson::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaSentient::Refresh(pAtom, ari);

	DaPerson* pA = (DaPerson*)pAtom;
	m_AgentBits = pA->m_AgentBits;
	m_AutoEat = pA->m_AutoEat;
	m_PowerJump = pA->m_PowerJump;
	m_PowerMoveMax = pA->m_PowerMoveMax;

	pA->m_PersonLock.lockForRead();
	m_PersonLock.lockForWrite();
	m_AccountBalance = pA->m_AccountBalance;
	m_LoanBalance = pA->m_LoanBalance;
	m_BerthType = pA->m_BerthType;
	m_Con = pA->m_Con;

	bool curItemChanged = m_CurrentGearPos != pA->m_CurrentGearPos;
	AuGearPos ipos = m_CurrentGearPos;
	m_CurrentGearPos = pA->m_CurrentGearPos;
	m_CurrentGearSub = pA->m_CurrentGearSub;
	if(pA->m_HitNotify.IsHit())
	{
		//qDebug() << "Refresh" << Id() << Name() << "isHit" << "this" << this;
		m_HitNotify = pA->m_HitNotify;
		m_HitNotify.RestartTime();
	}
	else
	{
		// Copy direction vector. Locally animated turn changes direction in bottom level atom. Must propagate that here.
		m_HitNotify.SetHitDirection(pA->m_HitNotify.HitDirection());
	}
	m_Domain = pA->m_Domain;
	m_InitScenario = pA->m_InitScenario;
	m_Lineage = pA->m_Lineage;
	m_Reputation = pA->m_Reputation;
	m_SpawnBd = pA->m_SpawnBd;
	m_SpawnBdName = pA->m_SpawnBdName;
	m_SpawnBdSystem = pA->m_SpawnBdSystem;
	m_SpawnBdSystemName = pA->m_SpawnBdSystemName;
	m_SpawnDevLL = pA->m_SpawnDevLL;
	m_SpawnShip = pA->m_SpawnShip;
	m_SpawnShipIsPrivate = pA->m_SpawnShipIsPrivate;
	m_SpawnShipName = pA->m_SpawnShipName;
	m_Worms = pA->m_Worms;
	m_PersonLock.unlock();
	pA->m_PersonLock.unlock();

	SetHunger(pA->CurrentHunger());

	const AuGear* pinv = pA->LockGearForRead();
	m_GearLock.lockForWrite();
	if(ari && curItemChanged && ipos.IsGear())
	{
		// Old current item may have been making noise or something.
		AuItem* item = m_Gear.ItemAt(ipos);
		if(item)
			item->DeleteRenderObjects(*ari->re, ari->al);
	}
	if(ari)
		m_Gear.Refresh(*pinv, ari->re, ari->al);
	else
		m_Gear.Refresh(*pinv, 0, 0);
	m_WeightCarried = pA->m_WeightCarried;
	m_GearLock.unlock();
	pA->UnlockGear();

	pA->m_JobsLock.lockForRead();
	m_JobsLock.lockForWrite();
	m_Jobs.Clear();
	for(AuJobList::const_iterator ji = pA->m_Jobs.cbegin(); ji != pA->m_Jobs.cend(); ji++)
		m_Jobs.append((*ji)->Clone());
	m_JobsLock.unlock();
	pA->m_JobsLock.unlock();

	//m_byRotSpeedX = pA->m_byRotSpeedX;
	//m_byRotSpeedZ = pA->m_byRotSpeedZ;

	pA->m_RewardsLock.lockForRead();
	m_RewardsLock.lockForWrite();
	m_RewardsOwed = pA->m_RewardsOwed;
	m_RewardsLock.unlock();
	pA->m_RewardsLock.unlock();

	pA->m_SeaChestLock.lockForRead();
	m_SeaChestLock.lockForWrite();
	m_SeaChest = pA->m_SeaChest;
	m_SeaChestLock.unlock();
	pA->m_SeaChestLock.unlock();

	// Set state of the spawn tie bit.
	if(pA->DidSpawnTie())
	{
		// The spawn tie bit may be propagated to this atom for the spawn tie graphic. Clear it now.
		m_PersonLock.lockForWrite();
		m_uiActions |= ACT_SpawnTie;							// Trigger the spawn tie graphic.
		m_PersonLock.unlock();
	}
	else
		ClearSpawnTie();

	// Set state of the recall home bit.
	m_PersonLock.lockForWrite();
	if(pA->IsAwaitingSpawnRecall())
		m_uiActions |= ACT_SpawnRecall;
	else
		m_uiActions &= ~ACT_SpawnRecall;

	if(pA->Actions() & ACT_SuitThruster)
		m_uiActions |= ACT_SuitThruster;
	else
		m_uiActions &= ~ACT_SuitThruster;

	if(pA->Actions() & ACT_SuitBrake)
		m_uiActions |= ACT_SuitBrake;
	else
		m_uiActions &= ~ACT_SuitBrake;
	m_PersonLock.unlock();

	return false;
}

void DaPerson::RefreshComplete()
{
	DaSentient::RefreshComplete();
	if(m_HitNotify.IsHit())
		m_HitNotify.ClearHit();

	if(DidSpawnTie())
		ClearSpawnTie();
}

DaAtom *DaPerson::Clone() const
{
	return new DaPerson(*this);
}

void DaPerson::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	if(AgentInvisible())
		return;

	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(AgentEntity())
	{
		RenderEntity(projection, model, *ari.re, MiniScale());		// Render this person as an entity if they are not invisible.
		return;
	}
	else if(IsDyingHighborn())
	{
		// Render dying highborn avatars as an entity.
		RenderHighbornEntity(projection, model, *ari.re, MiniScale());

		// Fall through to render the body too, like a regular avatar.
	}
	else if(IsNewbornHighborn())
	{
		// Render nuborn highborn avatars as an entity.
		RenderHighbornEntity(projection, model, *ari.re, MiniScale());
		return;
	}

	DaSentient::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);

	// Only the scene atom gets RenderLast called, unless it's an entity.
	// We also don't need the rest of this stuff when displaying mini me on the screen.
	if(sceneAtom != this || IsMini())
		return;

	if(InDesignerNotPreview() || IsEyeUnderWater())
		return;

	// Since this currently seems to be entirely the rain/snow effects, we'll just test this and go about our business
	if(!g_Au.DrawWeatherEffects())
		return;

	const DaWorldLL* planet = FindParentWorldInEnv();
	if(!planet)
		return;

	// 0-1 = No precip
	// 1-1.25 = Light precip
	// 1.25-1.5 = Moderate precip
	// 1.5-1.75 = Heavy precip
	// 1.75+ = Very Heavy precip
	float fPrecipDensity;
	float fReleaseInterval;
	float relativeHumidity = m_WxRelHumidity;
	if(relativeHumidity >= 1 && !std::isnan(relativeHumidity))
	{
		// Now set density to the number of particles to release per release interval. This number must be a whole number.
		fPrecipDensity = 100.f + (relativeHumidity-1.f) * 400.f;
		if(fPrecipDensity > 500.f)
			fPrecipDensity = 500.f;

		// Decrease the interval between nu particles as humidity increases.
		fReleaseInterval = .5f/relativeHumidity;
	}
	else
	{
		fPrecipDensity = 0;
		fReleaseInterval = 0;
	}
	float oat = m_WxOAT;	// In Celsius

	// Test max precip.
	/*
	{
		fPrecipDensity = 500;
		fReleaseInterval = .5f/2.f;
		relativeHumidity = 2;
		//oat = 0;
	}
	*/

	if(fPrecipDensity == 0 || oat >= 100)
	{
		// No nu precip or it's boiling hot outside. Let the particles fizzle out.
		if(!m_PrecipParticles || m_PrecipParticles->GetParticleCount() == 0)
			return;
	}

	// Rain falls faster so we can tolerate more particles.
	if(oat > 0)
		fPrecipDensity *= 1.5f;

	// Determine fall velocity and default size based on air temperature.
	bool bSnowShaders;					// Snow at 0 degrees Celsius.
	float fVelocity;
	float fSize;
	if(oat <= 0)
	{
		bSnowShaders = true;
		fVelocity = 1.5f * (float)FeetPerMeter();

		// Snow flake size gets smaller quickly as temp gets lower.
		// Our minimum snow flake size will be at -23.33C (-10F).
		float sizeFact = 1.f + oat/23.33f;						// 0 at -23.33C, 1 at 0C.
		if(sizeFact < 0)
			sizeFact = 0;
		else if(sizeFact > 1)
			sizeFact = 1;

		// Snow flakes can provide the real world sprite size because the sprite is exactly 1 square.
		// This is actually a scaling factor applied to the vertices of the snow flake VAO.
		fSize = AuNoiseF::Linear_Interpolate(.2f, 1.f, AuNoiseF::ExponentialCurve(sizeFact, 3));
	}
	else
	{
		bSnowShaders = false;	// Determine which shaders to select
		fVelocity = 8.f * (float)FeetPerMeter();

		// Rain drop size gets larger as humidity increases.
		float rangeFact = relativeHumidity - 1;				// 0 at 100%, 1 at 200%
		if(rangeFact < 0)
			rangeFact = 0;
		else if(rangeFact > 1)
			rangeFact = 1;

		// Unfortunately, the width of lines does not drop off with distance as is possible with the sprite shader.
		GLint range[2];
		ari.re->glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, range);
		if(range[1] > 4)
			range[1] = 4;
		fSize = AuNoiseF::Linear_Interpolate(range[0], range[1], rangeFact);
	}

	// Fall velocity is influenced by gravity.
	fVelocity *= planet->Gravity(CGAltitude())/32;

	// Find parent that is subordinate to the terrain.
	// Calculate change in particle positions due to movement/rotation.
	AuMatrix4x4 tFix;	// Transform to fix particles that were rendered in the last frame.
	// Determine my gravity normal relative to the environment right now. Gravity Normal is usually relative to my parent atom.
	AuPoint gN;    // gravity normal
	if(dynamic_cast<const DaVehicle*>(Parent()) ||
			dynamic_cast<const DaCapsule*>(Parent()) ||
			dynamic_cast<const DaCreature*>(Parent()))
	{
		// Recursion is probably overkill here. Let's at least allow for one more parent level, which might be a spacecraft.
		if(dynamic_cast<const DaCapsule*>(Parent()->Parent()))
		{
			AuMatrix4x4 t = Parent()->Transform() * Transform();
			// Calc grandparent vector relative to me.
			gN = t.UnProjectVector(Parent()->Parent()->GravityNormal());
			AuMatrix4x4 tparticle = Parent()->Parent()->Transform() * t;
			tFix = tparticle.Inverse() * s_LastParticleMatrix;
			s_LastParticleMatrix = tparticle;		// Remember last used matrix for the next pass.
		}
		else
		{
			// Use my parent atom's gravity vector.
			// Calc parent vector relative to me.
			gN = Transform().UnProjectVector(Parent()->GravityNormal());
			AuMatrix4x4 tparticle = Parent()->Transform() * Transform();
			tFix = tparticle.Inverse() * s_LastParticleMatrix;
			s_LastParticleMatrix = tparticle;		// Remember last used matrix for the next pass.
		}
	}
	else
	{
		gN = GravityNormal();    // Use my gravity normal.
		tFix = Transform().Inverse() * s_LastParticleMatrix;
		s_LastParticleMatrix = Transform();		// Remember last used matrix for the next pass.
	}

	// Constraint is the minimum length a particle must be from the person
	float fConstraint = 0;	// Default to 0 feet
	if(IsInsideBd())
		fConstraint = 150;	// Try to keep the precip out of the building.
	else if(IsInsideSpacecraft())
		fConstraint = 100;	// Try to keep the precip out of the spaceship.
	else if(IsInVehicle())
		fConstraint = 15;	// Try to keep the precip out of the vehicle.

	double moveSpeed = 0;
	AuPoint vwind;
	AuPoint vsource(0,0,1);	// Unit vector relative to me pointing toward the source of the particles.
	AuPoint terUp(0,0,1);	// Unit vector pointing up, for SunHorizonEffect.
	const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);

	AuMatrix4x4 t;
	GetTransformRelativeToParent(t, planet);

	// Get wind force and direction from planet's weather model.
	QPointF wind = planet->WindAt(GlobeLL());

	AuPoint w(wind.x(), wind.y());
	float windDir = w.ArcTangent_2D();		// Wind direction in radians
	float windForce = w.Length();			// Miles per hour
	windForce *= 5280.f;					// Convert to feet per hour
	windForce /= (60.f * 60.f);			// Convert to feet per sec

	// Calculate wind vector.
	terUp = -planet->GravityDir(planet->GetSurfaceLocation(GlobeLL()));

	AuPoint vnorth(0,0,1);
	AuPoint veast = vnorth.Cross(terUp);
	veast.Unit(windForce);			// Wind vec pointing east.
	AuMatrix4x4 tWind;
	tWind.RotateAxis(windDir, terUp);
	vwind = tWind.Project(veast);

	// Convert wind vector from globe space to my space.
	vwind = t.UnProjectVector(vwind);

	// Particle source vector influenced by environmental movement and wind.
	vsource = AuPoint(0,0, fVelocity);
	vsource += EnvironmentVector();				// Add my environment vector, which is also in feet per second.
	vsource -= vwind;							// Subtract wind vector, in feet per second.
	moveSpeed = vsource.Length();				// Particle speed relative to the viewer, in feet per second.
	if(moveSpeed != 0)
		vsource /= moveSpeed;					// Convert to a unit vector.
	else
		vsource = AuPoint(0,0,1);

	// Determine render method for particles.
	PsWeather::RENDER_METHOD renderMethod;
	if(bSnowShaders)
	{
		if(rsworld)
			renderMethod = PsWeather::RENDER_METHOD_Snow_Ringworld;
		else
			renderMethod = PsWeather::RENDER_METHOD_Snow;
	}
	else
		renderMethod = PsWeather::RENDER_METHOD_Rain;

	// Calculate matrix needed for generating nu particles.
	AuMatrix4x4 tNewParticles = AuPoint(0,0,1).GetAlignmentTransform(vsource);

	float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
	bool sunsBlocked;
	SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);

	if(!m_PrecipParticles)	// If no precipitation particle system has been created for this person create one now
	{
		// Create the textures we will use for our snow flake point sprites.
		if(!s_SnowTexture)
			s_SnowTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SnowFlake", ":data/SnowFlake.png");

		m_PrecipParticles = new PsWeather();
		m_PrecipParticles->SetRenderMethod(renderMethod);
		m_PrecipParticles->SetMaxParticles(25000);
		m_PrecipParticles->SetLifeCycle(60);
		m_PrecipParticles->SetVelocityVar(0.0f);
		m_PrecipParticles->SetPositionVar(200.f);				// Vary the position value of nu particles by factor of 100 feet
		m_PrecipParticles->Init(*ari.re);								// Initialize particle system
	}

	if(rsworld)
	{
		if(bSnowShaders)
		{
			ari.re->GLSL().SnowRingworldShader().Select(projection);
			ari.re->GLSL().SnowRingworldShader().SetModelView(model);
			ari.re->GLSL().SnowRingworldShader().SetTexture(s_SnowTexture);
			ari.re->GLSL().SnowRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);
			ari.re->GLSL().SnowRingworldShader().SetSize(fSize);

			// Ambient light
			ari.re->GLSL().SnowRingworldShader().SetAmbientLight(ambientR, ambientG, ambientB);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = ari.re->Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				ari.re->GLSL().SnowRingworldShader().SetSun(sunIndex, sun.m_Enabled && !sunsBlocked, sun.m_Attenuation,
						sun.m_ColorR, sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z,
						rsworld->ShadowPanelSunAngle(sun.m_ShadowAngle), sun.m_ShadowCount,
						sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<ari.re->MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = ari.re->Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				ari.re->GLSL().SnowRingworldShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}
		}
		else
		{
			ari.re->GLSL().RainRingworldShader().Select();
			ari.re->GLSL().RainRingworldShader().SetProjectionModelView(projection * model);
			ari.re->GLSL().RainRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Ambient light
			ari.re->GLSL().RainRingworldShader().SetAmbientLight(ambientR, ambientG, ambientB);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = ari.re->Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.
				AuPoint sunXVec = model.UnProjectVector(sun.m_XVec);
				AuPoint sunYVec = model.UnProjectVector(sun.m_YVec);

				// Configure shader for object space lighting.
				ari.re->GLSL().RainRingworldShader().SetSun(sunIndex, sun.m_Enabled && !sunsBlocked, sun.m_Attenuation,
						sun.m_ColorR, sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z,
						rsworld->ShadowPanelSunAngle(sun.m_ShadowAngle), sun.m_ShadowCount,
						sunXVec.x, sunXVec.y, sunXVec.z, sunYVec.x, sunYVec.y, sunYVec.z);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<ari.re->MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = ari.re->Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				ari.re->GLSL().RainRingworldShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}
		}
	}
	else
	{
		// Calculate the location of the planet in eye space.
		AuPoint globeP = rmod.Matrix(planet->Id()).Translation();
		if(bSnowShaders)
		{
			ari.re->GLSL().SnowShader().Select(projection);
			ari.re->GLSL().SnowShader().SetModelView(model);
			ari.re->GLSL().SnowShader().SetTexture(s_SnowTexture);
			ari.re->GLSL().SnowShader().SetFog(fogDensity, fogR, fogG, fogB);
			ari.re->GLSL().SnowShader().SetSize(fSize);

			// Globe position in model space is needed to calculate sun over-the-horizon effect.
			ari.re->GLSL().SnowShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

			// Ambient light
			ari.re->GLSL().SnowShader().SetAmbientLight(ambientR, ambientG, ambientB);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = ari.re->Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

				// Configure shader for object space lighting.
				ari.re->GLSL().SnowShader().SetSun(sunIndex, sun.m_Enabled && !sunsBlocked, sun.m_Attenuation, sun.m_ColorR,
						sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<ari.re->MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = ari.re->Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				ari.re->GLSL().SnowShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}
		}
		else
		{
			ari.re->GLSL().RainShader().Select();
			ari.re->GLSL().RainShader().SetProjectionModelView(projection * model);
			ari.re->GLSL().RainShader().SetFog(fogDensity, fogR, fogG, fogB);

			// Globe position in model space is needed to calculate sun over-the-horizon effect.
			ari.re->GLSL().RainShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

			// Ambient light
			ari.re->GLSL().RainShader().SetAmbientLight(ambientR, ambientG, ambientB);

			// Suns
			for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			{
				// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLSun& sun = ari.re->Sun(sunIndex);
				AuPoint sunLoc = model.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

				// Configure shader for object space lighting.
				ari.re->GLSL().RainShader().SetSun(sunIndex, sun.m_Enabled && !sunsBlocked, sun.m_Attenuation, sun.m_ColorR,
						sun.m_ColorG, sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
			}

			// Point lights
			for(unsigned int lightIndex=0; lightIndex<ari.re->MaxLights(); lightIndex++)
			{
				// Light location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
				const AuGLLight& light = ari.re->Light(lightIndex);
				AuPoint lightLoc = model.UnProject(light.m_Position);					// Light location relative to me.
				AuPoint spotDir;
				if(!light.m_Direction.IsZero())
					spotDir = model.UnProjectVector(light.m_Direction);					// Light direction relative to me.

				// Configure shader for object space lighting.
				ari.re->GLSL().RainShader().SetLight(lightIndex, light.m_bEnabled, light.m_kL, light.m_kQ, light.m_ColorR,
						light.m_ColorG, light.m_ColorB, lightLoc.x, lightLoc.y, lightLoc.z,
						light.m_cosOuter, light.m_cosInner, spotDir.x, spotDir.y, spotDir.z);
			}
		}
	}

	ari.re->glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set particle system parameters for this pass
	m_PrecipParticles->SetRenderMethod(renderMethod);
	m_PrecipParticles->SetSphereConstraint(fConstraint);
	m_PrecipParticles->SetNumToRelease((int)fPrecipDensity);
	m_PrecipParticles->SetReleaseInterval(fReleaseInterval);
	m_PrecipParticles->SetVelocity(AuPoint(0, 0, -fVelocity));
	m_PrecipParticles->SetSize(fSize);
	m_PrecipParticles->SetPosition(AuPoint(0, 0, moveSpeed * 2));
	m_PrecipParticles->SetWind(vwind);
	m_PrecipParticles->SetGravityNormal(gN);
	m_PrecipParticles->SetFixMatrix(tFix);
	m_PrecipParticles->SetNewMatrix(tNewParticles);

	// Update positions and then render the particles.
	// Only render if update succeeded
	if(!ari.IsViewTopDown() && m_PrecipParticles->Update())
		m_PrecipParticles->Render(projection, model, *ari.re);

	ari.re->glDisable(GL_BLEND);
	ari.re->glDepthMask(GL_TRUE);

	if(rsworld)
	{
		if(bSnowShaders)
			ari.re->GLSL().SnowRingworldShader().Deselect();
		else
			ari.re->GLSL().RainRingworldShader().Deselect();
	}
	else
	{
		if(bSnowShaders)
			ari.re->GLSL().SnowShader().Deselect();
		else
			ari.re->GLSL().RainShader().Deselect();
	}
}

// Render this nuborn highborn like an entity.
void DaPerson::RenderHighbornEntity(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double miniScale)
{
	// Create texture.
	if(!g_TexEntity)
		g_TexEntity = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	//qDebug() << "DaPerson::RenderEntity" << Id() << Name();
	if(!m_PSEntity)
	{
		m_PSEntity = new PsEntity;
		m_PSEntity->SetMaxParticles(1000);
		m_PSEntity->SetNumToRelease(75);
		m_PSEntity->SetReleaseInterval(0.05f);
		m_PSEntity->SetLifeCycle(3.0f);
		m_PSEntity->SetSize(100.0f);
		m_PSEntity->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSEntity->SetVelocityVar(.5f);
		m_PSEntity->SetTextureID(g_TexEntity);
		m_PSEntity->Init(re);
	}

	// Color the entity according to gender.
	switch(Gender())
	{
	case Female:
		m_PSEntity->SetColor(QColor(255, 174, 174));	// Pink
		break;
	case Male:
		m_PSEntity->SetColor(QColor(174, 174, 255));	// Blue
		break;
	case Asexual:
		m_PSEntity->SetColor(QColor(255, 255, 174));	// Yellow
		break;
	case Hermaphrodite:
		m_PSEntity->SetColor(QColor(174, 255, 174));	// Green
		break;
	}

	// Multiply by current transformation (Translation, rotation, and scale.)
	AuMatrix4x4 tThis = model;
	tThis.RotateX(CurrentHeadTilt());
	// Even though this code is identical to RenderEntity, this scale factor is only needed for the mini me display when I am born.
	// The dying avatar case works fine.
	if(miniScale != 1. && IsNewbornHighborn())
		tThis.Scale(miniScale, miniScale, miniScale);

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * tThis);
	re.GLSL().SpriteSizeShader().SetTexture(g_TexEntity);
	re.GLSL().SpriteSizeShader().SetColor(m_PSEntity->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSEntity->GetSize() * miniScale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	if(m_PSEntity->Update())
		m_PSEntity->Render(projection, tThis, re, miniScale);
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

// Render this person as an entity.
void DaPerson::RenderEntity(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double miniScale)
{
	// Create texture.
	if(!g_TexEntity)
		g_TexEntity = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	//qDebug() << "DaPerson::RenderEntity" << Id() << Name();
	if(!m_PSEntity)
	{
		m_PSEntity = new PsEntity;
		m_PSEntity->SetMaxParticles(1000);
		m_PSEntity->SetNumToRelease(75);
		m_PSEntity->SetReleaseInterval(0.05f);
		m_PSEntity->SetLifeCycle(3.0f);
		m_PSEntity->SetSize(100.0f);
		m_PSEntity->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSEntity->SetVelocityVar(.5f);
		m_PSEntity->SetTextureID(g_TexEntity);
		m_PSEntity->Init(re);
	}

	// Make the entity the same color as my empire's contrail color.
	const DgData& gd = g_Au.LockDataForRead();
	QColor contrailColor = gd.EmpireContrailColor(EmpireId());
	g_Au.UnlockData();
	m_PSEntity->SetColor(contrailColor);

	// Multiply by current transformation (Translation, rotation, and scale.)
	AuMatrix4x4 tThis = model;
	tThis.RotateX(CurrentHeadTilt());
	if(miniScale != 1.)
		tThis.Scale(miniScale, miniScale, miniScale);

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * tThis);
	re.GLSL().SpriteSizeShader().SetTexture(g_TexEntity);
	re.GLSL().SpriteSizeShader().SetColor(m_PSEntity->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSEntity->GetSize() * miniScale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	if(m_PSEntity->Update())
		m_PSEntity->Render(projection, tThis, re, miniScale);
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

// Only called for the scene atom.
void DaPerson::RenderScene(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, DaAtom* targetAtom, int targetLevel, const AuPoint& targetLoc, unsigned int targetPart,
		POLITICAL_STANCE targetStance, const AuMatrix4x4& tDevPersp, const AuMatrix4x4& tDevOrtho,
		int hudIntensity)
{
	DaSentient::RenderScene(projection, farProjection, model, ari);

	bool helmetHUDVisible;
	if(!ari.IsViewFirstPerson() || ari.m_ViewDist1st != 0 || IsTurretPose())
		helmetHUDVisible = false;
	else
	{
		m_GearLock.lockForRead();
		const AuItem* helmet = m_Gear.HelmetItemWorn();
		helmetHUDVisible = helmet &&				// Helmet is worn.
				helmet->IsEnvironmentHelmet() &&	// Helmet is an environment helmet.
				helmet->State() == 1;
		m_GearLock.unlock();
	}
	if(!ari.m_ShowHUD)
	{
		// Screen display elements are suppressed, possibly for a screen shot.
		if(!helmetHUDVisible || !g_Au.DrawHelmetHUDAlways())
			return;
	}

	ari.re->glDisable(GL_DEPTH_TEST);
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Calculate scale factor for indicators.
	bool leftOriented = g_Au.LeftRightOrientation() == 1 || (g_Au.LeftRightOrientation() == 0 && IsLeftHanded());
	float bscaley;
	if(helmetHUDVisible)
	{
		bscaley = .96f;							// We need the size to fit exactly in its designated spot on the helmet HUD.
		RenderHelmetHUD(ari, leftOriented);
	}
	else
		bscaley = g_Au.BodyStatSize()/100.f;		// User settings control size.
	double windowAspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());
	float bscalex = bscaley / (windowAspect*2);

	// Show frame rate in debug version.
	if(ari.m_ShowHUD && (g_Au.m_DebugSwitches & 0x00000001))
	{
		// Actual frames rendered in the last second.
		int prevSec = s_CurSecond - 1;
		if(prevSec < 0)
			prevSec = 59;
		int framesActual = s_Frames[prevSec];
		float r,g,b;
		if(framesActual >= 20)
		{
			r = .1f;
			g = 1;
			b = .1f;
		}
		else if(framesActual >= 10)
		{
			r = 1;
			g = 1;
			b = .1f;
		}
		else
		{
			r = 1;
			g = .1f;
			b = .1f;
		}
		ari.RenderNumber(framesActual, -0.5f*bscalex, 0.965f, .025f, 2,0, r,g,b,1);
	}

	if(ari.m_ShowHUD && (g_Au.m_DebugSwitches & 0x00000002))
	{
		// Last frame time
		float r,g,b;
		if(s_FrameMsLast == 0 || 1000/s_FrameMsLast >= 20)
		{
			r = .1f;
			g = 1;
			b = .1f;
		}
		else if(1000/s_FrameMsLast >= 10)
		{
			r = 1;
			g = 1;
			b = .1f;
		}
		else
		{
			r = 1;
			g = .1f;
			b = .1f;
		}
		ari.RenderNumber(s_FrameMsLast, .5f*bscalex, 0.965f, .025f, 2,0, r,g,b,1);
	}

	// Put information about my dude on the screen.
	// Weapon info is placed on the side of the screen that corresponds to the
	// handedness of the person. i.e. Right handed gets gun info on the right.
	// Health info is placed on the opposite side.
	const float barWid = .45f * bscalex;		// Width of all bars of information.
	const float barHt = .05f * bscaley;		// Height of each individual bar of information.
	const float barSpacing = .005f * bscaley;	// Spacing between individual bars, from DaCreature::RenderBodyStats
	const float jumpSpringHt = barHt*1.5f;
	const float moveSpringHt = barHt*1.5f;

	QRect rcClient = ari.glw->rect();
	const float ymax = 1;	//.99f;
	const float xmax = ymax/windowAspect;

	float left;
	if(leftOriented)
		left = xmax - barWid;	// Put health info on the right for lefties.
	else
		left = -xmax;			// Put health info on the left for righties.
	float bottom = -ymax;

	// Black backdrop to the bars.
	{
		GLuint texBarBack = AuTextureLibItem::LibraryTexture(*ari.re, AuTextureLibItem::LibraryTextureStatsBody());
		float bw = barWid + .02f;
		float bh = barHt*3 + barSpacing*4 +jumpSpringHt +moveSpringHt +.02f;
		float bx = left + bw/2 - .01f;
		float by = bottom + bh/2 - .01f;
		float r = .8f;
		float g = .8f;
		float b = .8f;
		float a = .5f;
		ari.RenderSprite(bx, by, bw, bh, texBarBack, r,g,b,a);
	}

	// Space for the two bars of information from DaCreature, plus spacing between them.
	RenderBodyStats(left, bottom + barHt + barSpacing, barWid, barHt*2 + barSpacing, false, 0, POL_Empire, ari, this, tDevPersp,
			tDevOrtho, hudIntensity);

	// Parent atom sometimes wants to render health. e.g. creature, vehicle. It is usually two bars tall.
	// Allow space for my three bars: health, stamina, and hunger, plus a taller jump bar, with a spacing between, and a margin.
	DaAtom* parent = Parent();
	if(parent)
	{
		float gap = 0;	//barSpacing * 2;
		float bot = bottom + barHt*3 + barSpacing*6 +jumpSpringHt +moveSpringHt + gap;
		parent->RenderBodyStats(left, bot, barWid, barHt*2 + barSpacing, false, 0, POL_Empire, ari, this, tDevPersp,
				tDevOrtho, hudIntensity);
	}

	// Hunger is at the bottom.
	{
		float valbot = bottom;
		float curval = CurrentHunger();
		float maxval = MaxHunger();
		float frac = curval/maxval;
		float r,g,b;
		if(frac <= 1.f/10.f)
		{
			// red
			r = 180.f/255.f;
			g = 40.f/255.f;
			b = 40.f/255.f;
		}
		else if(frac <= 1.f/3.f)
		{
			// orange
			r = 240.f/255.f;
			g = 180.f/255.f;
			b = 40.f/255.f;
		}
		else
		{
			// purple
			r = 150.f/255.f;
			g = 130.f/255.f;
			b = 150.f/255.f;
		}
		ari.RenderBar(left,valbot, barWid,barHt, frac, 4, r,g,b, r/10.f,g/10.f,b/10.f, 1.f);
	}

	// Jump Power
	{
		// This bar looks like a coiled spring. At 0 power, it is fully extended. It compresses as power increases.
		float valbot = bottom + barHt*3 + moveSpringHt + barSpacing*4;			// Above move power bar.
		float curval = m_PowerJump * MovementSpeedMod() + 1;						// Max is reduced by burden.
		float maxval = 16;
		if(curval < 1)
			curval = 1;
		else if(curval > 16)
			curval = 16;
		float fracval = curval/maxval;
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		ari.RenderJumpSpring(left,valbot, barWid,jumpSpringHt, fracval, .5f,.5f,.5f,1);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Move Power
	{
		// This bar looks like a compressed muscle. At 0 power, it is fully extended. It compresses as power increases.
		float valbot = bottom + barHt*3 + barSpacing*3;							// Above health bar.
		float curval = m_PowerMoveMax * MovementSpeedMod() + 1;					// Max is reduced by burden.
		float maxval = 16;
		if(curval < 1)
			curval = 1;
		else if(curval > 16)
			curval = 16;
		float fracval = curval/maxval;
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		ari.RenderMovePower(left,valbot, barWid,jumpSpringHt, fracval, .5f,.5f,.5f,1);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if(ari.m_ShowHUD)
	{
		// Character position
		if(ari.IsViewTopDown())
		{
			//determine circle size
			float circleSize = .05f;

			//determine circle color
			double curhp = CurrentHitPoints();
			double maxhp = MaxHitPoints();
			double frachp = curhp/maxhp;
			float r,g,b;
			if(frachp <= .1f || IsAnyConditionVisible())
			{
				// Bad health, bright red.
				r = 1;
				g = .2f;
				b = .2f;
			}
			else if(frachp < .5f)
			{
				// Poor health, bright amber.
				r = 1;
				g = .7f;
				b = .2f;
			}
			else
			{
				// Good health, bright blue.
				r = .2f;
				g = .5f;
				b = .9f;
			}

			//get position
			AuPoint px = tDevPersp.Project(AuPoint(0,0));
			float sightX = 2.f * px.x/rcClient.width() - 1.f;
			sightX /= windowAspect;
			float sightY = 1.f - 2.f * px.y/rcClient.height();

			ari.RenderLocation(sightX, sightY, circleSize, r,g,b,1);
		}
		else
		{
			// Health/condition of Target
			if(targetAtom)
			{
				float targetDistance = targetLoc.Length();
				if(targetDistance/10 <= targetLevel)
				{
					float left = .1f;		// slightly right of center
					float bottom = .075f;	// slightly up from center
					float barht = .05f;		// .05f  = .06f * 5.f/6.f
					float barwid = .275f;	// .275f = .33f * 5.f/6.f
					targetAtom->RenderBodyStats(left, bottom, barwid, barht, true, targetPart, targetStance, ari, this, tDevPersp,
							tDevOrtho, hudIntensity);
				}
			}

			// Crosshair
			//DHH Cannot shoot out of an enclosed vehicle.
			//DHH This should eventually be smarter so people can shoot through the
			//DHH windows or doors of an enclosed vehicle.
			if(!IsInVehicle())
			{
				// Calculate cross-hair Y offset based on targetted atom. Camera positioning behind/above the head induces this.
				float yoff = 0;
				if(targetAtom && !targetLoc.IsZero())
				{
					// No Y offset adjustment is needed in first person view when the view angle is zero, which is the normal first
					// person view angle.
					if(!ari.IsViewFirstPerson())// || ari.m_ViewAngle1st != 0)
					{
						// Calculate screen position for center of cross-hair.
						AuPoint ptl = tDevPersp.Project(targetLoc);
						float sightY = 2.f * ptl.y/rcClient.height() - 1.f;
						yoff = -sightY;
					}
				}

				float tl = (15.f - targetLevel)/15.f;
				float a = (float)hudIntensity / 10.f;

				ari.RenderCrosshair(0, yoff, tl, 1,.9f,.1f,a);
			}
		}
	}

	// Info about the current item in hand.
	AuGearPos gearPos(CurrentItemPos());
	INVENTORYSUB currentItemSub = CurrentItemSub();
	if(gearPos.IsBody())
	{
		// Body part weapon.
		// Draw a picture of the item at the corner of the screen.
		// This is drawn first so additional items such as rounds count can overlap the image.
		AuPoint boxSize(1,1);
		boxSize /= 8;
		boxSize.x *= bscalex;
		boxSize.y *= bscaley;

		{
			GLuint tex = CreateBodyPartTexture(*ari.re);

			float xpos;
			if(leftOriented)
				xpos = -xmax + boxSize.x/2. + .05f*bscalex;
			else
				xpos = xmax - boxSize.x/2. - .05f*bscalex;
			float ypos = bottom + boxSize.y/2.;

			ari.RenderSprite(xpos, ypos, boxSize.x, boxSize.y, tex, 1,1,1,1);

			// Show inventorysub indicator
			float digitht = .025f*bscaley;
			float digitwid = digitht * 22.8f / 32.f;	// Based on texture aspect ratio.
			float rcwid = digitwid;
			float left;
			if(leftOriented)
				left = -xmax + .05f;		// Info on left for lefties.
			else
				left = xmax - rcwid - .05f;	// Info on right for righties.

			ari.RenderNumber(currentItemSub+1, left, bottom, digitht, 0,0, .2f,.2f,1,1);
		}

		// Item Effects
		{
			// Make a list of the effects of this item in the currentUseSub.
			QVector<ITEMEFFECT> uses;	// Array will contain item effects.
			QVector<int> vals;	// Numeric values to overlay.
			AuCombatWeapon weapon(WM_Not);
			int ammoCur, ammoFull;
			WeaponAtUsePos(gearPos, currentItemSub, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect != WE_None || weapon.m_w2.m_Effect != WE_None)
			{
				switch(weapon.m_w1.m_Effect)
				{
				case WE_None:
				case WE_Audible:
				case WE_Invalid:
				case WE_Bleeding:
				case WE_Corrosion:
				case WE_Generic:
				case WE_Starving:
				case WE_Unused:
					break;
				case WE_Acid:
					uses.append(IE_Acid);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Suffocate:
					uses.append(IE_Suffocate);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Stun:
					uses.append(IE_Stun);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Poison:
					uses.append(IE_Poison);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Radiation:
					uses.append(IE_Radiation);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Crush:
					uses.append(IE_Crush);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Sonic:
					uses.append(IE_Sonic);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Bludgeon:
					uses.append(IE_Bludgeon);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Cold:
					uses.append(IE_Cold);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Disease:
					uses.append(IE_Disease);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Electric:
					uses.append(IE_Electric);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Heat:
					uses.append(IE_Heat);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Pierce:
					uses.append(IE_Pierce);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Implant:
					uses.append(IE_Implant);
					vals.append(weapon.MaxDamage1());
					break;
				case WE_Slash:
					uses.append(IE_Slash);
					vals.append(weapon.MaxDamage1());
					break;
				}

				switch(weapon.m_w2.m_Effect)
				{
				case WE_None:
				case WE_Audible:
				case WE_Invalid:
				case WE_Bleeding:
				case WE_Corrosion:
				case WE_Generic:
				case WE_Starving:
				case WE_Unused:
					break;
				case WE_Acid:
					uses.append(IE_Acid);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Suffocate:
					uses.append(IE_Suffocate);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Stun:
					uses.append(IE_Stun);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Poison:
					uses.append(IE_Poison);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Radiation:
					uses.append(IE_Radiation);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Crush:
					uses.append(IE_Crush);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Sonic:
					uses.append(IE_Sonic);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Bludgeon:
					uses.append(IE_Bludgeon);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Cold:
					uses.append(IE_Cold);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Disease:
					uses.append(IE_Disease);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Electric:
					uses.append(IE_Electric);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Heat:
					uses.append(IE_Heat);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Pierce:
					uses.append(IE_Pierce);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Implant:
					uses.append(IE_Implant);
					vals.append(weapon.MaxDamage2());
					break;
				case WE_Slash:
					uses.append(IE_Slash);
					vals.append(weapon.MaxDamage2());
					break;
				}
			}

			// Calculate xpos,ypos at center of first icon location.
			float xpos;
			if(leftOriented)
				xpos = -xmax + boxSize.x + .05f*bscalex + .05f*bscalex;
			else
				xpos = xmax - boxSize.x - .05f*bscalex - .05f*bscalex;

			float ypos = bottom + .05f*bscaley;

			// Now display all possible effects
			//qDebug("Current item has %d uses", uses.count());
			QVector<ITEMEFFECT>::const_iterator it = uses.cbegin();
			QVector<int>::const_iterator vit = vals.cbegin();
			for(; it != uses.cend() && vit != vals.cend(); it++, vit++)
			{
				float left = xpos - .05f*bscalex;
				float bottom = ypos - .05f*bscaley;

				ari.RenderItemEffect(*it, left, bottom, .1f*bscaley, 1,1,1,1);

				// Display number on top of weapon/item effect.
				if(*vit)
				{
					float r = .3f;
					float g = .3f;
					float b = 1;
					ari.RenderNumber(*vit, xpos-.05f*bscalex, ypos+.05f*bscaley, .04f*bscaley, 0,0, r,g,b,1);
				}

				if(leftOriented)
					xpos += .1f*bscalex;
				else
					xpos -= .1f*bscalex;
			}

			// Display the weapon mode (area, projectile, psionic, etc)
			if(weapon.m_Mode != WM_Not)
			{
				float left = xpos - .05f*bscalex;
				float bottom = ypos - .05f*bscaley;
				if(currentItemSub != IPS_Item && !SpecialWeaponReady())
				{
					// Current item is the special weapon and it's not ready for use.
					ari.RenderItemEffect(IE_SpecWeaponNotReady, left, bottom, .1f*bscaley, 1,1,1,1);
				}
				else
				{
					// Display the weapon mode (area, projectile, psionic, etc)
					ari.RenderWeaponMode(weapon.m_Mode, left, bottom, .1f*bscaley, 1,1,1,1);
				}
			}
		}
	}
	else if(gearPos.IsGear())
	{
		// Draw a picture of the item at the corner of the screen.
		// This is drawn first so additional items such as rounds count can overlap the image.
		m_GearLock.lockForRead();
		const AuItem* cItem = m_Gear.ItemAt(gearPos);
		AuItem* curItem;
		if(cItem)
			curItem = cItem->Clone();
		else
			curItem = 0;
		m_GearLock.unlock();
		if(curItem)
		{
			AuPoint boxSize = curItem->SingleObjectSize();
			boxSize /= 8;
			boxSize.x *= bscalex;
			boxSize.y *= bscaley;
			{
				GLuint tex = CreateItemTexture(*curItem, ari);

				float xpos;
				if(leftOriented)
					xpos = -xmax + boxSize.x/2. + .05f*bscalex;
				else
					xpos = xmax - boxSize.x/2. - .05f*bscalex;
				float ypos = bottom + boxSize.y/2.;

				// Must flip this sprite in Y.
				ari.RenderSprite(xpos, ypos, boxSize.x, -boxSize.y, tex, 1,1,1,1);

				// Show inventorysub indicator
				float digitht = .025f*bscaley;
				float digitwid = digitht * 22.8f / 32.f;	// Based on texture aspect ratio.
				float rcwid = digitwid;
				float left;
				if(leftOriented)
					left = -xmax + .05f;		// Info on left for lefties.
				else
					left = xmax - rcwid - .05f;	// Info on right for righties.

				ari.RenderNumber(currentItemSub+1, left, bottom, digitht, 0,0, .2f,.2f,1,1);
			}

			// Item Condition
			{
				float curhp = curItem->HitPoints();
				float maxhp = curItem->NewHitPoints();
				if(maxhp <= 0)
					maxhp = 1;
				float frachp = curhp/maxhp;

				float barwid = .02f * bscalex;
				float barht = .15f * bscaley;
				float left;
				if(leftOriented)
					left = -xmax;			// Item info on left for lefties.
				else
					left = xmax - barwid;	// Item info on right for righties.

				if(frachp <= .1f || (curhp == 1 && curhp < maxhp))
					ari.RenderItemConditionBar(left, bottom, barwid, barht, frachp, 1,.2f,.2f, .2f,.04f,.04f,1);
				else if(frachp < .5f)
					ari.RenderItemConditionBar(left, bottom, barwid, barht, frachp, 1,.7f,.2f, .2f,.14f,.04f,1);
				else
					ari.RenderItemConditionBar(left, bottom, barwid, barht, frachp, .2f,.2f,1, .04f,.04f,.2f,1);
			}

			// Item Effects
			{
				// Make a list of the effects of this item in the currentUseSub.
				QVector<ITEMEFFECT> uses;	// Array will contain indexes into texture.
				QVector<int> vals;			// Numeric values to overlay.
				WEAPONMODE mode = WM_Not;
				if(curItem->IsWeapon(currentItemSub))
				{
					COMMODITYID cid;
					quint8 quality;
					int ammoCur, ammoFull, damage1 = 0, damageMod1 = 0, damage2 = 0, damageMod2 = 0;
					float range, radius1, radius2;
					float ap1, st1, ap2, st2;
					WEAPONEFFECT effect1 = WE_None, effect2 = WE_None;
					curItem->WeaponInfo(currentItemSub, cid, quality, mode, range, ammoCur, ammoFull,
							effect1, damage1, damageMod1, radius1, ap1, st1,
							effect2, damage2, damageMod2, radius2, ap2, st2);
					//qDebug() << "DaPerson::RenderScene weapon" << currentItemSub << cid << quality << mode << range
						//<< "ammo" << ammoCur << ammoFull
						//<< "eff1" << effect1 << damage1 << damageMod1 << radius1 << ap1 << st1
						//<< "eff2" << effect2 << damage2 << damageMod2 << radius2 << ap2 << st2;
					switch(effect1)
					{
					case WE_None:
					case WE_Audible:
					case WE_Invalid:
					case WE_Bleeding:
					case WE_Corrosion:
					case WE_Generic:
					case WE_Starving:
					case WE_Unused:
						break;
					case WE_Acid:
						uses.append(IE_Acid);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Suffocate:
						uses.append(IE_Suffocate);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Stun:
						uses.append(IE_Stun);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Poison:
						uses.append(IE_Poison);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Radiation:
						uses.append(IE_Radiation);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Crush:
						uses.append(IE_Crush);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Sonic:
						uses.append(IE_Sonic);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Bludgeon:
						uses.append(IE_Bludgeon);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Cold:
						uses.append(IE_Cold);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Disease:
						uses.append(IE_Disease);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Electric:
						uses.append(IE_Electric);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Heat:
						uses.append(IE_Heat);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Pierce:
						uses.append(IE_Pierce);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Implant:
						uses.append(IE_Implant);
						vals.append(damage1 + damageMod1);
						break;
					case WE_Slash:
						uses.append(IE_Slash);
						vals.append(damage1 + damageMod1);
						break;
					}

					switch(effect2)
					{
					case WE_None:
					case WE_Audible:
					case WE_Invalid:
					case WE_Bleeding:
					case WE_Corrosion:
					case WE_Generic:
					case WE_Starving:
					case WE_Unused:
						break;
					case WE_Acid:
						uses.append(IE_Acid);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Suffocate:
						uses.append(IE_Suffocate);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Stun:
						uses.append(IE_Stun);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Poison:
						uses.append(IE_Poison);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Radiation:
						uses.append(IE_Radiation);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Crush:
						uses.append(IE_Crush);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Sonic:
						uses.append(IE_Sonic);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Bludgeon:
						uses.append(IE_Bludgeon);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Cold:
						uses.append(IE_Cold);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Disease:
						uses.append(IE_Disease);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Electric:
						uses.append(IE_Electric);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Heat:
						uses.append(IE_Heat);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Pierce:
						uses.append(IE_Pierce);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Implant:
						uses.append(IE_Implant);
						vals.append(damage2 + damageMod2);
						break;
					case WE_Slash:
						uses.append(IE_Slash);
						vals.append(damage2 + damageMod2);
						break;
					}
				}
				else
				{
					// Check for various non-weapon item effects.
					//qDebug("Current item is not a weapon");
					// Food and Liquor are mutually exclusive.
					if(curItem->IsFood())
					{
						if(curItem->IsFood(EcoRole()))
						{
							uses.append(IE_Food);
							vals.append(curItem->FoodNutritionValue(EcoRole()));
						}
					}
					else if(curItem->IsIntoxicant())
					{
						uses.append(IE_Liquor);
						vals.append(curItem->IntoxicantValue());
					}

					if(curItem->IsLight())
					{
						uses.append(IE_Light);
						vals.append(0);
					}

					switch(curItem->CommodityId())
					{
					case CID_Antidote:
						uses.append(IE_CurePoison);
						vals.append(0);
						break;
					case CID_Antitoxin:
						uses.append(IE_CureDisease);
						vals.append(0);
						break;
					case CID_DNASampleKit:
						uses.append(IE_SampleDNA);
						vals.append(0);
						break;
						/*
					case CID_Defibrilator:
						uses.append(IE_Revive);
						vals.append(0);
						break;
						*/
					case CID_Hammer:
						if(dynamic_cast<const DaVessel*>(targetAtom))
						{
							const DaVessel* targetShip = (const DaVessel*)targetAtom;
							COMMODITYID cid = targetShip->SystemModule(SYS_Hull);
							switch(cid)
							{
							case CID_MetalHullModule:
								{
									uses.append(IE_RepairMetal);

									// Determine how many are needed to repair the sighted part of the target.
									int needed = 0;
									needed = targetShip->AssessDamage(targetPart, CID_Metal);
									vals.append(needed);
								}
								break;
							case CID_MagmiumHullModule:
								{
									uses.append(IE_RepairMagmium);

									// Determine how many are needed to repair the sighted part of the target.
									int needed = 0;
									needed = targetShip->AssessDamage(targetPart, CID_Magmium);
									vals.append(needed);
								}
								break;
							case CID_VulciumHullModule:
								{
									uses.append(IE_RepairVulcium);

									// Determine how many are needed to repair the sighted part of the target.
									int needed = 0;
									needed = targetShip->AssessDamage(targetPart, CID_Vulcium);
									vals.append(needed);
								}
								break;
							case CID_AdamantineHullModule:
								{
									uses.append(IE_RepairAdamantine);

									// Determine how many are needed to repair the sighted part of the target.
									int needed = 0;
									needed = targetShip->AssessDamage(targetPart, CID_Adamantine);
									vals.append(needed);
								}
								break;
							default:
								break;
							}
						}
						else
						{
							uses.append(IE_RepairMetal);
							vals.append(0);
							uses.append(IE_RepairMagmium);
							vals.append(0);
							uses.append(IE_RepairVulcium);
							vals.append(0);
							uses.append(IE_RepairAdamantine);
							vals.append(0);
						}
						break;
					case CID_FittingWrench:
					case CID_Pliers:
					case CID_Wrench:
						{
							uses.append(IE_RepairModule);

							// Determine how many are needed to repair the sighted part of the target.
							int needed = 0;
							const DaVessel* targetShip = dynamic_cast<const DaVessel*>(targetAtom);
							if(targetShip)
							{
								// Test all modules.
								for(int modcid=CID_Invalid; modcid<CID_Count; modcid++)
									if(DbCommodity::IsModule((COMMODITYID)modcid))
									{
										needed = targetShip->AssessDamage(targetPart, (COMMODITYID)modcid);
										if(needed)
											break;
									}
							}
							vals.append(needed);
						}
						break;
					case CID_SolderIron:
						{
							uses.append(IE_RepairElectronicPart);

							// Determine how many are needed to repair the sighted part of the target.
							int needed = 0;
							const DaVessel* targetShip = dynamic_cast<const DaVessel*>(targetAtom);
							if(targetShip)
								needed = targetShip->AssessDamage(targetPart, CID_ElectronicPart);
							vals.append(needed);
						}
						break;
					case CID_TorqueWrench:
						{
							uses.append(IE_RepairMechanicalPart);

							// Determine how many are needed to repair the sighted part of the target.
							int needed = 0;
							const DaVessel* targetShip = dynamic_cast<const DaVessel*>(targetAtom);
							if(targetShip)
								needed = targetShip->AssessDamage(targetPart, CID_MechanicalPart);
							vals.append(needed);
						}
						break;
					case CID_Knife:
					case CID_MagmiumKnife:
					case CID_VulciumKnife:
						uses.append(IE_Butcher);
						vals.append(0);
						break;
					case CID_ScrewDriver:
						uses.append(IE_DisassembleModule);
						vals.append(0);
						break;
					case CID_MedicalKit:
						uses.append(IE_Heal);
						vals.append(curItem->Health());
						break;
					case CID_Shovel:
						uses.append(IE_Dig);
						vals.append(0);
						break;
					case CID_VacSuitHelmet:
					case CID_MetalPowerArmorHelmet:
					case CID_MagmiumPowerArmorHelmet:
					case CID_VulciumPowerArmorHelmet:
						uses.append(IE_Visor);
						vals.append(0);
						break;
					case CID_Water:
						uses.append(IE_Decontaminate);
						vals.append(0);
						break;

					default:
						break;
					}
				}

				// Calculate xpos,ypos at center of first icon location.
				float xpos;
				if(leftOriented)
					xpos = -xmax + boxSize.x + .05f*bscalex + .05f*bscalex;
				else
					xpos = xmax - boxSize.x - .05f*bscalex - .05f*bscalex;

				float ypos = bottom + .05f*bscaley;

				// Now display all the effects
				//qDebug("Current item has %d uses", uses.count());
				QVector<ITEMEFFECT>::const_iterator it = uses.cbegin();
				QVector<int>::const_iterator vit = vals.cbegin();
				for(; it != uses.cend() && vit != vals.cend(); it++, vit++)
				{
					float left = xpos - .05f*bscalex;
					float bottom = ypos - .05f*bscaley;

					ari.RenderItemEffect(*it, left, bottom, .1f*bscaley, 1,1,1,1);

					// Display number on top of weapon/item effect.
					if(*vit)
					{
						float r = .3f;
						float g = .3f;
						float b = 1;
						ari.RenderNumber(*vit, xpos-.05f*bscalex, ypos+.05f*bscaley, .04f*bscaley, 0,0, r,g,b,1);
					}

					if(leftOriented)
						xpos += .1f*bscalex;
					else
						xpos -= .1f*bscalex;
				}

				// Display the weapon mode (area, projectile, psionic, etc)
				if(mode != WM_Not)
				{
					float left = xpos - .05f*bscalex;
					float bottom = ypos - .05f*bscalex;
					ari.RenderWeaponMode(mode, left, bottom, .1f*bscaley, 1,1,1,1);
				}
			}

			// Rounds Count
			// Get rounds count of gun.
			int rounds = -1, ammoqty=1;	// -1 if no rounds counter needed
			float pctleft = 1.0;
			if(curItem->IsWeaponThatUsesAmmo(currentItemSub))
			{
				rounds = curItem->AmmoLoaded(currentItemSub);	// get rounds count
				ammoqty = curItem->AmmoQuantity(currentItemSub);
				if(ammoqty)
					pctleft = (float)rounds / (float)ammoqty;				
			}

			if(rounds >= 0)
			{
				float r,g,b;
				if(pctleft < .1 || rounds <= 1)
				{
					// red
					r = 1;
					g = .2f;
					b = .2f;
				}
				else if(pctleft < .5 && rounds <= 10)
				{
					// amber
					r = 1;
					g = .7f;
					b = .2f;
				}
				else
				{
					// green
					r = .2f;
					g = 1;
					b = .2f;
				}
				float base = bottom + .05f*bscaley + .01f*bscaley;
				if(leftOriented)
					ari.RenderNumber(rounds, -xmax+.05f*bscalex, base, .05f * bscaley, 0,0, r,g,b,1);	// Left justify on left.
				else
					ari.RenderNumber(rounds, xmax-.05f*bscalex, base, .05f * bscaley, 2,0, r,g,b,1);	// Right justify on right.
			}
			delete curItem;
		}
	}

	// Vehicle Status
	DaVehicle* vehicle = dynamic_cast<DaVehicle*>(parent);
	if(vehicle)
	{
		if(vehicle->IsControlSeat(VehicleSeatIndex()))
		{
			if(ari.IsViewTopDown())
			{
				// In top-down alt view, render the hud at the center of the screen.
				vehicle->RenderHud(0,0, hudIntensity, ari);
			}
			else
			{
				// Place hud straight ahead of me.
				AuMatrix4x4 tlook;
				tlook.Rotate(Rotation());
				if(vehicle->CommodityId() == CID_APC)
				{
					// Tilt gun up/down with head tilt.
					float headTilt = CurrentHeadTilt();
					//qDebug() << "headTilt" << (headTilt*RAD2DEGf);
					if(headTilt < -10.f*DEG2RADf)
						headTilt = -10.f*DEG2RADf;
					else if(headTilt > 15.f*DEG2RADf)
						headTilt = 15.f*DEG2RADf;
					tlook.RotateX(-headTilt);
				}

				AuPoint gunSight = tlook.UnProject(AuPoint(0,6,0));

				AuPoint px = tDevOrtho.Project(gunSight);
				if(px.z < 0)
				{
					// Hud is in front of the camera plane.
					px = tDevPersp.Project(gunSight);

					// Calculate screen position for center of gunsight.
					float sightX = 2.f * px.x/rcClient.width() - 1.f;
					sightX /= windowAspect;
					float sightY = 2.f * px.y/rcClient.height() - 1.f;
					vehicle->RenderHud(sightX, -sightY, hudIntensity, ari);
				}
				else
				{
					// Hud is behind me. Intensity 0 will cause the fighter hud to
					// be suppressed. Other huds, e.g. dirt bike speedometer, will
					// be drawn regardless of intensity.
					vehicle->RenderHud(0,0, 0, ari);
				}
			}
		}
	}
	else if(HasConsole() && IsHelmStationPose())
	{
		const DaVessel* v = dynamic_cast<DaVessel*>(parent);
		if(v)
		{
			// Pilot console renders HUD.
			const DpStation* helm = LockConsoleForRead();
			if(helm)
				helm->RenderHud(hudIntensity, ari, v->ETE());
			UnlockConsole();
		}
	}

	if(m_HitNotify.IsHit())
		m_HitNotify.DrawHit(ari, CurrentHeadTilt(), IsExoskeletal());

	// Restore rendering states.
	ari.re->glDisable(GL_BLEND);
	ari.re->glEnable(GL_DEPTH_TEST);

	// Calculation location of mini me.
	float indSize = .05f * bscalex;
	float bodyWid = indSize*2;
	float bodyHt;
	if(vehicle || (HasConsole() && dynamic_cast<DaCapsule*>(parent)) || dynamic_cast<DaCreature*>(parent) || IsEthereal())
		bodyHt = bodyWid;
	else if(IsFlying())
		bodyHt = indSize*(CGHeightStanding()+1);
	else if(IsSwimming())
		bodyHt = indSize*CGHeightStanding();
	else if(IsProne())
		bodyHt = indSize*TorsoThickness();
	else
		bodyHt = indSize*GroundHeightAboveCG();
	float indXPos = -xmax + barWid + barSpacing*2 + bodyWid;
	if(leftOriented)
		indXPos = -indXPos;
	float indYPos = -ymax + barSpacing + bodyHt;

	// Render mini me at lower corner of the display.
	if(g_Au.DrawMiniMe())
	{
		ari.re->glClear(GL_DEPTH_BUFFER_BIT);

		// Create projection and model matrices to place mini me on the screen.
		AuMatrix4x4 p;
		p.Scale(windowAspect, 1, -1);

		AuMatrix4x4 m;
		if(CurrentGeneralPose() == AuPose::EyesOnly)
			m.Translate(indXPos, indYPos + .4*indSize, -.5);
		else
		{
			m.Translate(indXPos, indYPos, -.5);
			m.RotateX(-PI/2);
			m.RotateZ(Rotation().z);
		}

		SetMiniScale(indSize);
		if(IsEthereal())
		{
			// Render entity only.
			// Unit/Atom renders special effects audio.
			AuRenderLastMap renderLast;
			AuModelMatrixHash rmod;
			rmod.insert(Id(), m);
			DaAtomList shadSc;
			DaAtomList shadScFog;
			DaAtomList shadScGlobe;
			DaAtomList shadScGlobeFog;
			DaAtomList shadScRingworld;
			DaAtomList shadScRingworldFog;
			DaUnit::RenderEnvironmentObject(projection, ari, rmod, this, renderLast, PASS_Near, shadSc, shadScFog, shadScGlobe,
					shadScGlobeFog, shadScRingworld, shadScRingworldFog);
			RenderLast(p, ari, rmod, this, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
		}
		else
			RenderMiniMe(p, m, ari, indSize);
		ResetMiniScale();
	}

	// Orientation indicator last so it is on top of mini me.
	// Not shown when manning a console in a building, hence the test for spacecraft parent.
	if(vehicle || (HasConsole() && dynamic_cast<DaCapsule*>(parent)) || dynamic_cast<DaCreature*>(parent))
	{
		ari.re->glDisable(GL_DEPTH_TEST);
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		float xpos = indXPos;
		float ypos = indYPos;
		float r = 1;
		float g = .7f;
		float b = .2f;
		float a = .6f;
		ari.RenderOrientationIndicator(xpos,ypos, Rotation().z, indSize, r,g,b,a);

		// Name of orientation indicator.
		if(vehicle)
		{
			// Seat index in a vehicle.
			ari.RenderNumber(VehicleSeatIndex()+1, xpos, ypos-indSize*1.5f, indSize/2, 1, 0, r, g, b, a);
		}
		else
		{
			// Console name.
			QString name = ConsoleName();
			QColor tcol;
			tcol.setRgbF(r,g,b,a);
			int xpix = rcClient.width()/2 * (1 + (xpos - indSize)*windowAspect);
			int ypix = rcClient.height()/2 * (1 - (ypos - indSize));
			ari.RenderText(xpix,ypix, name, tcol, Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine);
		}

		// Restore rendering states.
		ari.re->glDisable(GL_BLEND);
		ari.re->glEnable(GL_DEPTH_TEST);
	}
}

GLuint DaPerson::CreateItemTexture(const AuItem& item, AuAtomRenderInfo& ari) const
{
	// Load stock texture for the commodity. AuView::initializeGL creates them.
	return ari.CommodityTextureF(item.CommodityId());
}

GLuint DaPerson::CreateBodyPartTexture(AuGLEffects& re) const
{
	QString imageNames[] =
	{
		":/data/wl_Anus.png",
		":/data/wl_Eye.png",
		":/data/wl_Foot.png",
		":/data/wl_Hand.png",
		":/data/wl_Horns.png",
		":/data/wl_Mouth.png",
		":/data/wl_Tail.png",
		":/data/wl_Torso.png",
	};
	int imageIdx=0;

	INVENTORYPOS wpos = m_CurrentGearPos.MountBodyWeaponPos();
	if(wpos != IP_NumInventoryPos)
	{
		// Mount's body weapon.
		// Select the appropriate image to use.
		// NOTE: The wpos value has been translated to be relative to the mount, in the IP_My range.
		switch(wpos)
		{
		case IP_My_Anus:
			imageIdx = 0;
			break;
		case IP_My_Eyes:
			imageIdx = 1;
			break;
		case IP_My_Feet:
			imageIdx = 2;
			break;
		case IP_My_Hands:
			imageIdx = 3;
			break;
		case IP_My_Head:
			{
				DaCreature* mount = dynamic_cast<DaCreature*>(Parent());
				if(mount)
				{
					if(mount->HasDNA3())
					{
						if(mount->DNA3().MouthSize > mount->DNA3().HornSize)
							imageIdx = 5;	// Mouth
						else
							imageIdx = 4;	// Horns
					}
					else
					{
						//DHH animalia
					}
				}
			}
			break;
		case IP_My_Tail:
			imageIdx = 6;
			break;
		case IP_My_Torso:
			imageIdx = 7;
			break;
		default:
			return 0;
		}
	}
	else
	{
		// My body weapon.
		wpos = m_CurrentGearPos.PersonalBodyWeaponPos();
		if(wpos == IP_NumInventoryPos)
			return 0;

		// Select the appropriate image to use.
		switch(wpos)
		{
		case IP_My_Anus:
			imageIdx = 0;
			break;
		case IP_My_Eyes:
			imageIdx = 1;
			break;
		case IP_My_Feet:
			imageIdx = 2;
			break;
		case IP_My_Hands:
			imageIdx = 3;
			break;
		case IP_My_Head:
			if(HasDNA3())
			{
				if(DNA3().MouthSize > DNA3().HornSize)
					imageIdx = 5;	// Mouth
				else
					imageIdx = 4;	// Horns
			}
			else
			{
				imageIdx = 5;	// Mouth
				//DHH animalia Horns?
			}
			break;
		case IP_My_Tail:
			imageIdx = 6;
			break;
		case IP_My_Torso:
			imageIdx = 7;
			break;
		default:
			return 0;
		}
	}

	// Create a texture map from a bitmap, or just return the id if it has already been created.
	QString sid = QString("Body%1").arg(imageIdx);
	return re.CreateTextureFromBitmap(Id().GetID(), sid, imageNames[imageIdx]);
}

bool DaPerson::HelmetClosed() const
{
	// Helmet is open when operating a turret.
	if(IsTurretPose())
		return false;
	m_GearLock.lockForRead();
	const AuItem* helmet = m_Gear.HelmetItemWorn();
	bool h = helmet &&							// Helmet is worn.
			helmet->IsEnvironmentHelmet() &&	// Helmet is an environment helmet.
			helmet->State() == 1 &&				// Helmet visor is closed.
			helmet->HitPoints() > 1;			// Helmet seal is not broken.
	m_GearLock.unlock();
	return h;
}

quint8 DaPerson::WornHelmetState() const
{
	// Helmet is open when operating a turret.
	quint8 state;
	m_GearLock.lockForRead();
	const AuItem* helmet = m_Gear.HelmetItemWorn();
	if(helmet)
		state = helmet->State();
	else
		state = 0;
	m_GearLock.unlock();
	return state;
}

// Render OpenAL audio sources
void DaPerson::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	if(!al)
		return;
	if(sceneAtom == this)
	{
		// Breathing
		float curval = m_Stamina;
		float maxval = MaxStamina();
		float fracval = curval/maxval;
		float breathPitch;
		if(fracval <= 1.f/6.f)
			breathPitch = 1.5;
		else if(fracval < 1.f/3.f)
		{
			// Ramp breath pitch from 2 at 1/6 stamina down to 1 at 1/3 stamina.
			fracval -= 1.f/6.f;	// fracval now in the range 0..1/3
			fracval /= 1.f/3.f;	// fracval now in the range 0..1
			breathPitch = fracval * .5f + 1.f;	// breathPitch now 1..1.5
		}
		else
			breathPitch = 1;

		bool helmetClosed = false;
		SOUND_EFFECT cur = (SOUND_EFFECT)m_Sound[PSOUND_Breath];
		if(AgentEntity())
		{
			// Entity does not "breathe".
			al->Play(m_AudioPerson[PSOUND_Breath], cur, SOUND_None, 0,0,0, 0,1, true, 1,1,0);
		}
		else
		{
			helmetClosed = HelmetClosed();
			if(helmetClosed)
			{
				//qDebug() << "DaPerson::RenderAudioSources" << Id() << Name() << "breathing in helmet";
				// I am wearing an environment helmet with the visor closed and the seal is intact.
				if(IsMouthSubmerged())
					al->Play(m_AudioPerson[PSOUND_Breath], cur, SOUND_BreathingScuba, 0,0,0, fade/5, breathPitch, true, 1,0,0);
				else
					al->Play(m_AudioPerson[PSOUND_Breath], cur, SOUND_BreathingInMask, 0,0,0, fade/6, breathPitch, true, 1,0,0);
			}
			else
				al->Play(m_AudioPerson[PSOUND_Breath], cur, SOUND_None, 0,0,0, 0,1, true, 1,1,0);
		}
		m_Sound[PSOUND_Breath] = cur;

		// Wind
		{
			// Wind due to environment.
			float envWindForce = 0;
			float envWindGain = 0;
			float envWindPitch = 0;
			if(PlanetHasAtmosphere() && !IsEyeUnderWater())
			{
				// Not under water and this planet has a non-zero atm density.
				const DaWorldLL* llworld = FindParentWorldInEnv();
				if(llworld)
				{
					// Get wind force from planet's weather model.
					QPointF wind = llworld->WindAt(GlobeLL());
					AuPoint w(wind.x(), wind.y());
					float windForce = w.Length();				// Miles per hour
					windForce /= 100.f;
					windForce = AuNoiseF::Linear_Interpolate(m_EnvWindForce, windForce, .01f);	// Change wind sound gradually.
					envWindForce = windForce;
					if(windForce > 0)
					{
						// The wind can be heard.
						if(windForce > 1)
							windForce = 1;

						// Set gain based on current wind speed and atmosphere density.
						envWindPitch = windForce;
						envWindGain = windForce * AtmosphereDensity() / 2.f;
						switch(OrbitZone())
						{
						case OZONE_Inferno:
							envWindGain /= 16.f;
							break;
						case OZONE_Inner:
							envWindGain /= 8.f;
							break;
						case OZONE_Habitable:
							envWindGain /= 4.f;
							break;
						case OZONE_Outer:
						case OZONE_DeepSpace:
							// Default wind volume sound is loud, which is appropriate for outer zone high speed winds.
							break;
						case OZONE_Frigid:
							envWindGain *= 2.f;	// Very high speed wind in frigid zone.
							break;
						}

						// Spaceship will quiet the outside wind and it increases wind noise due to speed.
						if(IsInsideBd())
						{
							// Wind is quietest inside buildings.
							envWindGain *= .1f;
						}
						else if(IsInsideCapsule())
						{
							const DaCapsule* ship = FindParentCapsuleInEnv();
							if(ship)
							{
								// Wind is quiet inside the ship.
								envWindGain *= .15f;

								// Wind on hull is louder due to ship speed.
								float factor = ship->Speed() / 600.f;
								envWindPitch += factor;
								if(envWindPitch > 2)
									envWindPitch = 2;
								float speed = factor * ship->AtmosphereDensity();
								envWindGain += speed;
							}
						}
						else
						{
							const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
							if(vehicle)
							{
								// Wind is quiet inside an enclosed vehicle.
								// The EB_Vehicle bit is set only inside enclosed vehicles.
								if(IsInVehicle())
									envWindGain *= .2f;

								// Wind on hull is louder due to vehicle speed.
								float factor = fabs(vehicle->Speed()) / 600.f;
								envWindPitch += factor;
								if(envWindPitch > 2)
									envWindPitch = 2;
								float speed = factor * vehicle->AtmosphereDensity();
								envWindGain += speed;
							}
						}

						// Adjust wind noise for planet's overall atmosphere density.
						envWindGain *= (.5f + AtmosphereDensity()/2.f);
					}
				}
			}
			m_EnvWindForce = envWindForce;

			// Wind due to spacecraft equalizing pressure with the outside when a hull door is open.
			float shipWindForce = 0;
			float shipWindGain = 0;
			float shipWindPitch = 0;
			if(IsInsideCapsule())
			{
				const DaCapsule* ship = FindParentCapsuleInEnv();
				if(ship)
				{
					float insideAir = ship->AirPressure(CapsuleRoomName());	// 0..1
					float outsideAir;
					if(ship->CapacitorLevel() > 0 && ship->LifeSupportWorking())
					{
						// Life support functioning.
						if(ship->AnyHullDoorOpenInRoom(CapsuleRoomName()))
							outsideAir = ship->AirDensity();				// Room is equalizing to outside air pressure.
						else
						{
							// Room is returning to 50% standard density inside.
							if(insideAir < .5f)
							{
								outsideAir = insideAir + .1f;
								if(outsideAir > .5f)
									outsideAir = .5f;
							}
							else if(insideAir > .5f)
							{
								outsideAir = insideAir - .1f;
								if(outsideAir < .5f)
									outsideAir = .5f;
							}
							else
								outsideAir = .5f;
						}
					}
					else
					{
						// Life support not functioning.
						if(ship->AnyHullDoorOpen())
							outsideAir = ship->AirDensity();				// Room is equalizing to outside air pressure.
						else
							outsideAir = insideAir;							// Air pressure is static.
					}

					// Wind gain is the differential between the two pressures.
					if(!FuzzyEquals(outsideAir, insideAir))
					{
						// Air pressure is equalizing between inside and outside.
						float windForce = fabs(outsideAir - insideAir) * 2;	// 0..2
						windForce = AuNoiseF::Linear_Interpolate(m_ShipWindForce, windForce, .1f);	// Change wind sound gradually.
						shipWindForce = windForce;
						shipWindPitch = windForce * 2;			// 0..4
						if(shipWindPitch > 2)
							shipWindPitch = 2;
						shipWindGain = windForce * insideAir / 2.f;

						const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
						if(vehicle)
						{
							// Wind is quiet inside an enclosed vehicle.
							// The EB_Vehicle bit is set only inside enclosed vehicles.
							if(IsInVehicle())
								shipWindGain *= .2f;

							// Wind on hull is louder due to vehicle speed.
							float factor = fabs(vehicle->Speed()) / 600.f;
							shipWindPitch += factor;
							if(shipWindPitch > 2)
								shipWindPitch = 2;
							float speed = factor * vehicle->AtmosphereDensity();
							shipWindGain += speed;
						}

						// Adjust wind noise for ship's overall interior atmosphere density.
						shipWindGain *= (.5f + insideAir/2.f);
					}
				}
			}
			else if((m_uiActions & ACT_SuitMask) == ACT_SuitMask)
			{
				// EVA suit makes thruster noise while I am moving and braking.
				shipWindGain = .035f * 4;	// x4 to compensate for sound reduction of helmet
				shipWindPitch = 0.75f;
			}
			else if(m_uiActions & ACT_SuitThruster)
			{
				// EVA suit makes thruster noise while I am moving.
				shipWindGain = .025f * 4;	// x4 to compensate for sound reduction of helmet
				shipWindPitch = 0.9f;
			}
			else if(m_uiActions & ACT_SuitBrake)
			{
				// EVA suit makes thruster noise while I am moving.
				shipWindGain = .025f * 4;	// x4 to compensate for sound reduction of helmet
				shipWindPitch = 0.5f;
			}
			m_ShipWindForce = shipWindForce;

			// Combine the wind sound sources.
			float windGain = MAX(envWindGain, shipWindGain);
			if(helmetClosed)
				windGain *= .5f;	// Closed environment suit helmet cuts wind noise.

			float windPitch = MAX(envWindPitch, shipWindPitch);
			SOUND_EFFECT cur = (SOUND_EFFECT)m_Sound[PSOUND_Wind];
			al->Play(m_AudioPerson[PSOUND_Wind], cur, SOUND_Wind, 0,0,0, windGain*fade, windPitch, true, 1,0,0);
			m_Sound[PSOUND_Wind] = cur;
		}

		// Rain/stargate/lava sound.
		float fEnvHumidity = 0;
		cur = (SOUND_EFFECT)m_Sound[PSOUND_Rain];
		if(IsInAtmosphere())
		{
			// The same sound effect slot is used for rain, ringworld stargate, and lava sound because they don't need to play at
			// the same time. The ringworld stargate noise drowns out rain sound; neither of those is heard on an inferno world.
			const DaStar* star = FindParentStarInEnv();
			if(star && star->IsRingworldStargateOpen())
			{
				// The ringworld stargate is audible.
				float fStargatePitch = 1.f;
				float fStargateGain = 1.f * AtmosphereDensity();

				if(IsEyeUnderWater())
				{
					fStargateGain = .4f;	// Stargate can be heard under water regardless of atmo density.
					fStargatePitch = .8f;	// Pitch is lowered under water.
				}
				else if(IsInsideBd())
					fStargateGain *= .2f;	// Stargate is quietest inside buildings.
				else if(IsInsideCapsule())
					fStargateGain *= .3f;	// Stargate is quiet inside the ship.
				else if(IsInVehicle())
					fStargateGain *= .4f;

				al->Play(m_AudioPerson[PSOUND_Rain], cur, SOUND_RingworldStargate, 0,0,0, fStargateGain*fade, fStargatePitch, true,
						1,0,0);
			}
			else
			{
				const DaWorldGasGiant* gg = dynamic_cast<const DaWorldGasGiant*>(Parent());
				if(gg)
				{
					// Lava movement is constantly audible near the lava core.
					float lavaPitch = 1.f;
					float lavaGain = .5f;

					// Enclosures muffle outside sounds.
					if(IsInsideBd())
						lavaGain *= .2f;	// Lava is quietest inside buildings.
					else if(IsInsideCapsule())
						lavaGain *= .3f;	// Lava movement is quiet inside the ship.
					else if(IsInVehicle())
						lavaGain *= .4f;	// Lava is quiet inside an enclosed vehicle.

					// Altitude above lava is distance from sound.
					float alt = AltitudeMSL() + gg->SeaLevelRadius() - gg->GlobeRadius();
					if(alt < 0)
						alt = 0;

					al->Play(m_AudioPerson[PSOUND_Rain], cur, SOUND_Lava, 0,0,-alt, lavaGain*fade, lavaPitch, true);
				}
				else if(OrbitZone() == OZONE_Inferno)
				{
					// Lava movement is constantly audible near sea level.
					float lavaPitch = 1.f;
					float lavaGain = AtmosphereDensity() * .5f;

					// Enclosures muffle outside sounds.
					if(IsInsideBd())
						lavaGain *= .2f;	// Lava is quietest inside buildings.
					else if(IsInsideCapsule())
						lavaGain *= .3f;	// Lava movement is quiet inside the ship.
					else if(IsInVehicle())
						lavaGain *= .4f;	// Lava is quiet inside an enclosed vehicle.

					// Altitude above lava is distance from sound.
					float alt = AltitudeMSL();
					if(alt < 0)
						alt = 0;

					al->Play(m_AudioPerson[PSOUND_Rain], cur, SOUND_Lava, 0,0,-alt, lavaGain*fade, lavaPitch, true);
				}
				else if(!IsEyeUnderWater())
				{
					// Rain from 2 degrees celsius to 100 degrees celsius.
					float fHumidity = m_WxRelHumidity;
					if(fHumidity > 1 && m_WxOAT > 0 && m_WxOAT < 100 && !std::isnan(fHumidity))
						fHumidity = AuNoiseF::Linear_Interpolate(m_EnvHumidity, fHumidity, .05f);	// Change rain sound gradually.
					else
						fHumidity = AuNoiseF::Linear_Interpolate(m_EnvHumidity, 0, .25f);	// Terminate rain sound gradually.
					fEnvHumidity = fHumidity;
					float fRainPitch = fHumidity * .5f;
					float fRainGain = fHumidity * .5f * AtmosphereDensity();

					// Spaceship will quiet the outside rain and it increases rain noise due to speed.
					if(IsInsideBd())
					{
						// Rain is quietest inside buildings.
						fRainGain *= .2f;
					}
					else if(IsInsideCapsule())
					{
						const DaCapsule* ship = FindParentCapsuleInEnv();
						if(ship)
						{
							// Rain is quiet inside the ship.
							fRainGain *= .3f;

							// Rain on hull is louder due to ship speed.
							float factor = ship->Speed() / 600.f;
							fRainPitch += factor;
							if(fRainPitch > 2)
								fRainPitch = 2;
							float speed = factor * ship->AtmosphereDensity();
							fRainGain += speed;
						}
					}
					else
					{
						const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
						if(vehicle)
						{
							// Rain is quiet inside an enclosed vehicle.
							// The EB_Vehicle bit is set only inside enclosed vehicles.
							if(IsInVehicle())
								fRainGain *= .4f;

							// Rain on hull is louder due to vehicle speed.
							float factor = fabs(vehicle->Speed()) / 600.f;
							fRainPitch += factor;
							if(fRainPitch > 2)
								fRainPitch = 2;
							float speed = factor * vehicle->AtmosphereDensity();
							fRainGain += speed;
						}
					}

					if(helmetClosed)
						fRainGain *= .5f;	// Closed environment suit helmet cuts rain noise.

					al->Play(m_AudioPerson[PSOUND_Rain], cur, SOUND_Rain, 0,0,0, fRainGain*fade, fRainPitch, true, 1,0,0);
				}
				else
					al->Play(m_AudioPerson[PSOUND_Rain], cur, SOUND_None, 0,0,0, 0,1, true, 1,0,0);
			}
		}
		else
			al->Play(m_AudioPerson[PSOUND_Rain], cur, SOUND_None, 0,0,0, 0,1, true, 1,0,0);
		m_Sound[PSOUND_Rain] = cur;
		m_EnvHumidity = fEnvHumidity;
	}
	else
	{
		SOUND_EFFECT cur = (SOUND_EFFECT)m_Sound[PSOUND_Breath];
		al->Play(m_AudioPerson[PSOUND_Breath], cur, SOUND_None, 0,0,0, 0,1, true, 1,0,0);
		m_Sound[PSOUND_Breath] = cur;
		cur = (SOUND_EFFECT)m_Sound[PSOUND_Wind];
		al->Play(m_AudioPerson[PSOUND_Wind], cur, SOUND_None, 0,0,0, 0,1, true, 1,0,0);
		m_Sound[PSOUND_Wind] = cur;
		cur = (SOUND_EFFECT)m_Sound[PSOUND_Rain];
		al->Play(m_AudioPerson[PSOUND_Rain], cur, SOUND_None, 0,0,0, 0,1, true, 1,0,0);
		m_Sound[PSOUND_Rain] = cur;
	}

	// Some items make noise. Check my current item.
	AuGearPos gearPos(CurrentItemPos());
	if(gearPos.IsGear())
	{
		m_GearLock.lockForRead();
		AuItem* gearItem = m_Gear.ItemAt(gearPos);
		if(gearItem && gearItem->HasSound())
		{
			AuMatrix4x4 im(model);
			im.Translate(WeaponLoc());

			float gain = RelativeAudioGain(sceneAtom) * fade;
			float pitch = 1;
			gearItem->RenderSound(im, *al, gain, pitch);
		}
		m_GearLock.unlock();
	}
}

void DaPerson::PrepareForSampleRender()
{
	Stand();
}

// Renders this object in a scene.
// View transformation has already been set up.
void DaPerson::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Do not render distant creatures.
	if(pass != PASS_Near)
		return;

	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(AgentInvisible())
	{
		// OpenAL audio sources
		// Render my own audio sources. (Breathing, wind, rain)
		RenderAudioSources(sceneAtom, model, ari.al, ari.fade);
		return;		// Don't render anything, not even a noise.
	}

	//qDebug() << "DaPerson::RenderEnvironmentObject" << Id() << Name() << "head" << CurrentHeadTilt()*RAD2DEGf
		//<< "turn" << Rotation().z*RAD2DEG;
	if(AgentEntity() || IsNewbornHighborn())
	{
		// Render entity only.
		renderLast.Insert(rmod, this);

		// Unit/Atom renders special effects audio.
		DaUnit::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		// OpenAL audio sources
		// Render my own audio sources. (Breathing, wind, rain)
		RenderAudioSources(sceneAtom, model, ari.al, ari.fade);
		return;
	}

	// Scene atom needs to be rendered last to do rain/snow particle effect.
	if((sceneAtom == this && !InDesignerNotPreview() && !IsEyeUnderWater()))
		renderLast.Insert(rmod, this);
	else if(IsDyingHighborn())
		renderLast.Insert(rmod, this);	// Dying highborn renders entity in addition to body.

	// Don't render body when scene atom is in design environment
	// Don't render the dead body of a highborn so only the entity is seen.
	if(IsLineageHighborn() && IsBodyDead())
	{
		// Unit/Atom renders special effects audio.
		DaUnit::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		// OpenAL audio sources
		// Creature renders audio sources. (Walking, flapping, swimming)
		DaSentient::RenderAudioSources(sceneAtom, model, ari.al, ari.fade);

		// Render my own audio sources.
		RenderAudioSources(sceneAtom, model, ari.al, ari.fade);
		return;
	}

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, model, ari.al, ari.fade);

	// Base atom class needs to render special effects.
	DaSentient::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

// Sets up camera when this atom is going to render the scene.
void DaPerson::SetupCamera(AuMatrix4x4 &tCamera, double &rNearClip, double &rFarClip, AuAtomRenderInfo& ari) const
{
	DaSentient::SetupCamera(tCamera, rNearClip, rFarClip, ari);
	if(ari.IsViewTopDown())
	{
		tCamera.LoadIdentity();
		if(IsGameTablePose())
		{
			// At game table, top down view puts camera above the center of the table.
			if(IsInCapsule())
			{
				// At a spacecraft game table.
				const DaCapsule* ship = dynamic_cast<const DaCapsule*>(Parent());
				if(ship)
				{
					AuMatrix4x4 tableM;
					ship->GameTableLocation(StationId(), tableM);
					double tableZ = tableM.GetRotation().z;
					AuPoint mvec = -ConsolePoseOffset().RotateZ(tableZ-Rotation().z);	// Offset to center of game table.
					mvec.z += DeckHeight() - 1;					// 1' below ceiling.
					tCamera.Translate(mvec);
					tCamera.RotateX(-90.0 * DEG2RAD);
					tCamera.RotateX(90.0 * DEG2RAD + CurrentHeadTilt());
				}
			}
		}
		else
		{
			// Top down view is a top-down bird's eye view at the specified altitude.
			// Rocket has a strange orientation on the ground, sitting on its tail with its Y axis pointing up.
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(Parent());
			if(vehicle && vehicle->IsOnGround() && vehicle->LandingOrientation() == LORI_Tail)
			{
				AuPoint upvec = Transform().UnProjectVector(AuPoint(0,1,0));	// Up from rocket, relative to me.
				tCamera = AuPoint(0,0,1).GetAlignmentTransform(upvec);
			}
			// Some spacecraft have a strange orientation on the ground, sitting on its tail with its Y axis pointing up.
			DaVessel* v = dynamic_cast<DaVessel*>(Parent());
			if(v && v->IsOnGround() && v->LandingOrientation() == LORI_Tail)
			{
				AuPoint upvec = Transform().UnProjectVector(AuPoint(0,1,0));	// Up from rocket, relative to me.
				tCamera = AuPoint(0,0,1).GetAlignmentTransform(upvec);
			}
			tCamera.Translate(0,0, ari.m_ViewAlt);
			tCamera.RotateX(-90.0 * DEG2RAD);
			tCamera.RotateX(90.0 * DEG2RAD + CurrentHeadTilt());
		}
	}
}

void DaPerson::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaSentient::CreateDeviceObjects(ari);

	// Select a model for the current use item unless it is a clothing item.
	if(AgentAnimal())
		SetItemInHand(*ari.re, ari.al, 0);
	else
	{
		m_GearLock.lockForRead();
		SetItemInHand(*ari.re, ari.al, m_Gear.ItemVisibleInHandAt(CurrentItemPos()));
		m_GearLock.unlock();
	}
}

void DaPerson::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaSentient::DeleteDeviceObjects(ari);

	// OpenAL audio sources
	for(int cs=0; cs<PSOUND_Count; cs++)
	{
		ari.StopSound(m_AudioPerson[cs]);
		m_Sound[cs] = SOUND_None;
	}

	// Current item.
	AuGearPos gearPos(CurrentItemPos());
	if(gearPos.IsGear())
	{
		m_GearLock.lockForRead();
		AuItem* gearItem = m_Gear.ItemAt(gearPos);
		if(gearItem)
			gearItem->DeleteRenderObjects(*ari.re, ari.al);
		m_GearLock.unlock();
	}
}

void DaPerson::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_PersonControls.begin(); myci != s_PersonControls.end(); myci++)
		{
			if((*myci).GetId() == (*ci).GetId())
			{
				// Found the corresponding control id. Copy in the nu data.
				*myci = *ci;
				break;
			}
		}
	}
	SaveControls();
}

void DaPerson::RestoreDefaultControls()
{
	s_PersonControls.clear();
	for(int ic=0; ic<s_NumPersonDefaults; ic++)
		s_PersonControls.append(s_PersonDefaults[ic]);
	SaveControls();
}

void DaPerson::LoadControls() const
{
	// Init with my default controls.
	s_PersonControls.clear();
	for(int ic=0; ic<s_NumPersonDefaults; ic++)
		s_PersonControls.append(s_PersonDefaults[ic]);

	// Load any overrides from the settings file.
	s_PersonControls.LoadSettings("Controls/Person");

	/*
	for(DiControlInputArray::const_iterator ci = s_PersonControls.cbegin(); ci != s_PersonControls.cend(); ci++)
	{
		qDebug() << (*ci).Name() << "keyboard" << (*ci).Key() << (*ci).KeyModifier()
			<< "joystick" << (*ci).Joystick() << (*ci).JoystickAxisExclusive() << (*ci).JoystickIndex() << (*ci).JoystickSubIndex()
			<< "mouse" << (*ci).Mouse() << (*ci).MouseModifier() << (*ci).MouseWheel();
	}
	*/
}

void DaPerson::SaveControls() const
{
	// Store my controls in the settings file.
	s_PersonControls.SaveSettings("Controls/Person");
}

// A scene atom should fill this array with control inputs it can accept.
void DaPerson::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_PersonControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	// Restrict the commands available based on where I am.
	// Can't use IsInVehicle or IsAboardVehicle here because they require environment query.
	const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
	if(vehicle)
	{
		// Aboard a vehicle.
		if(vehicle->IsEnclosed())
		{
			// Inside an enclosed vehicle.
			for(DiControlInputArray::const_iterator ci = s_PersonControls.cbegin(); ci != s_PersonControls.cend(); ci++)
			{
				switch((*ci).GetId())
				{
				// Cannot fire personal weapons when inside an enclosed vehicle.
				//DHH This should eventually be smarter so people can shoot through the
				//DHH windows or doors of an enclosed vehicle.
				case CI_FireItem:
				case CI_FireItemAlt:
				case CI_MoveForward:
				case CI_MoveForwardContinuously:
				case CI_MoveBackward:
				case CI_MoveLeft:
				case CI_MoveRight:
				case CI_MoveUp:
				case CI_MoveDown:
				case CI_Run:
				case CI_BrakeOrJump:
				case CI_Brake:
				case CI_Jump:
				case CI_ProneStandToggle:
				case CI_SitStandToggle:
				case CI_ReceptiveMateToggle:
				case CI_Crouch:
					continue;	// Skip commands that can't be used while inside an enclosed vehicle.
				default:
					aAllControls.append(*ci);
					break;
				}
			}
		}
		else
		{
			// Aboard a vehicle that is not enclosed.
			for(DiControlInputArray::const_iterator ci = s_PersonControls.cbegin(); ci != s_PersonControls.cend(); ci++)
			{
				switch((*ci).GetId())
				{
				case CI_MoveForward:
				case CI_MoveForwardContinuously:
				case CI_MoveBackward:
				case CI_MoveLeft:
				case CI_MoveRight:
				case CI_MoveUp:
				case CI_MoveDown:
				case CI_Run:
				case CI_BrakeOrJump:
				case CI_Brake:
				case CI_Jump:
					// ProneStandToggle, SitStandToggle and Crouch are allowed for people aboard a sailboat. They get ignored
					// elsewhere when it doesn't make sense, like on a dirt bike.
					continue;	// Skip commands that can't be used while aboard vehicle that is not enclosed.
				default:
					aAllControls.append(*ci);
					break;
				}
			}
		}

		// Vehicle controls
		vehicle->EnumControlInputs(aAllControls);
	}
	else if(RemoteCameraId())
	{
		// Restrict commands available while viewing through a remote camera.
		for(DiControlInputArray::const_iterator ci = s_PersonControls.cbegin(); ci != s_PersonControls.cend(); ci++)
		{
			switch((*ci).GetId())
			{
			case CI_ConsoleCommand:
			case CI_ConsoleEngineer:
			case CI_ConsoleLower:
			case CI_ConsoleMoveFarther:
			case CI_ConsoleMoveCloser:
			case CI_ConsoleNavigator:
			case CI_ConsolePilot:
			case CI_ConsoleRaise:
			case CI_ConsoleRotateAway:
			case CI_ConsoleRotateToward:
			case CI_ConsoleSensor:
			case CI_ConsoleShield:
			case CI_ConsoleWeapon:
			case CI_EnterExit:
			case CI_LookDown:
			case CI_LookUp:
			case CI_MapPos0:
			case CI_MapPos1:
			case CI_MapPos2:
			case CI_MapPos3:
			case CI_MapPos4:
			case CI_MapPos5:
			case CI_MapPos6:
			case CI_MapPos7:
			case CI_MapPos8:
			case CI_MapPos9:
			case CI_ReloadItem:
			case CI_SelectMapPos0:
			case CI_SelectMapPos1:
			case CI_SelectMapPos2:
			case CI_SelectMapPos3:
			case CI_SelectMapPos4:
			case CI_SelectMapPos5:
			case CI_SelectMapPos6:
			case CI_SelectMapPos7:
			case CI_SelectMapPos8:
			case CI_SelectMapPos9:
			case CI_SelectNextItem:
			case CI_SelectPreviousItem:
			case CI_SpawnRecall:
			case CI_SpawnTie:
			case CI_TurnLeft:
			case CI_TurnRight:
			case CI_UseItem:
			case CI_EatFood:
			case CI_HelmetVisor:
			case CI_PlaceItem:
			case CI_SayHello:
			case CI_RoomLightsBrighter:
			case CI_RoomLightsDarker:
			case CI_JumpPowerDecrease:
			case CI_JumpPowerIncrease:
			case CI_MovePowerDecrease:
			case CI_MovePowerIncrease:
			case CI_RoomNavOrbitRings:
			case CI_RoomNavSystemNames:
			case CI_RoomNavWormholes:
			case CI_DropItem:
				// These commands can be used while viewing through a remote camera.
				aAllControls.append(*ci);
				break;
			case CI_Crouch:
			case CI_FireItem:
			case CI_FireItemAlt:
			case CI_BrakeOrJump:
			case CI_MoveBackward:
			case CI_MoveDown:
			case CI_MoveForward:
			case CI_MoveLeft:
			case CI_MoveRight:
			case CI_MoveUp:
			case CI_ProneStandToggle:
			case CI_SitStandToggle:
			case CI_Run:
			case CI_Stand:
			case CI_ThrowItem:
			case CI_Brake:
			case CI_Jump:
			case CI_MoveForwardContinuously:
			case CI_DoorLockToggle:
			case CI_DoorOpen:
			case CI_ReceptiveMateToggle:
			case CI_LASTITEM:
				continue;	// Skip commands that can't be used while viewing through a remote camera.
			}
		}

		// Console controls
		const DpStation* console = LockConsoleForRead();
		console->EnumControlInputs(aAllControls);
		UnlockConsole();
	}
	else if(HasConsole())
	{
		// Person is sitting at a console.
		for(DiControlInputArray::const_iterator ci = s_PersonControls.cbegin(); ci != s_PersonControls.cend(); ci++)
		{
			switch((*ci).GetId())
			{
			case CI_MoveForward:
			case CI_MoveForwardContinuously:
			case CI_MoveBackward:
			case CI_MoveLeft:
			case CI_MoveRight:
			case CI_MoveUp:
			case CI_MoveDown:
			case CI_Run:
			case CI_BrakeOrJump:
			case CI_Brake:
			case CI_Jump:
			case CI_ProneStandToggle:
			case CI_SitStandToggle:
			case CI_ReceptiveMateToggle:
			case CI_Crouch:
				continue;	// Skip commands that can't be used while sitting at a console.
			default:
				aAllControls.append(*ci);
				break;
			}
		}

		// Console controls
		const DpStation* console = LockConsoleForRead();
		console->EnumControlInputs(aAllControls);
		UnlockConsole();
	}
	else
	{
		// No restrictions on any person commands.
		for(DiControlInputArray::const_iterator ci = s_PersonControls.cbegin(); ci != s_PersonControls.cend(); ci++)
			aAllControls.append(*ci);
	}
}

void DaPerson::EnumControlPanels(AuQuadArray& aControlPanels, const AuMatrix4x4& tModel, QVector<qint16>& idPanels,
		const DaAtom* sceneAtom) const
{
	Q_UNUSED(sceneAtom);
	if(HasConsole() && IsConsolePose())
	{
		const DpStation* console = LockConsoleForRead();
		console->EnumControlPanels(aControlPanels, tModel, idPanels, *this);
		UnlockConsole();
	}
}

void DaPerson::EnumViewControls(QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
{
	if(IsAwaitingSpawn() || NeedsInit() || IsAwaitingSpawnRecall())
		return;

	const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
	if(vehicle)
	{
		// I am aboard a vehicle.
		GLuint texV = ari.CommodityTextureN(vehicle->CommodityId());
		bool inControl = vehicle->IsControlSeat(VehicleSeatIndex());
		if(inControl)
		{
			if(!vehicle->IsLaunching() && !vehicle->IsParalyzed() && vehicle->Fuel() > 0)
				controls.append(AuControlButton(tr("Engine"), Id(), 0, DaVehicle::CI_EngineOnOff, texV));
			if(vehicle->HasWeapon())
				controls.append(AuControlButton(tr("Weapon"), Id(), 0, DaVehicle::CI_SelectWeapon, texV));
			if(vehicle->HasLight())
				controls.append(AuControlButton(tr("Headlight"), Id(), 0, DaVehicle::CI_LightOnOff, texV));
			if(vehicle->IsSpaceVehicle())
			{
				controls.append(AuControlButton(tr("Orbits"), Id(), 0, DaVehicle::CI_OrbitRingsToggle, texV));
				if(vehicle->IsEngineOn() && !vehicle->IsLaunching())
				{
					if(!vehicle->IsParalyzed())
						controls.append(AuControlButton(tr("Hold"), Id(), 0, DaVehicle::CI_Hold, texV));
					if(vehicle->IsInCapsule())
						controls.append(AuControlButton(tr("Launch"), Id(), 0, DaVehicle::CI_RequestLaunch, texV));
					else
						controls.append(AuControlButton(tr("Land"), Id(), 0, DaVehicle::CI_RequestRecovery, texV));
				}
			}
			else if(vehicle->CommodityId() == CID_Helicopter)
			{
				if(vehicle->IsEngineOn())
				{
					if(!vehicle->IsParalyzed())
						controls.append(AuControlButton(tr("Hover"), Id(), 0, DaVehicle::CI_Hold, texV));
				}
			}
			controls.append(AuControlButton(tr("Door Remote"), Id(), 0, DaVehicle::CI_DoorRemote, texV));
			if(!vehicle->MotherShip().IsNull() && !vehicle->MotherShipName().isEmpty())
				controls.append(AuControlButton(tr("Deassign"), Id(), 0, DaVehicle::CI_ReleaseMotherShip, texV));
		}
		controls.append(AuControlButton(tr("Service"), Id(), 0, DaVehicle::CI_Service, texV));
		if(vehicle->EmptySeats())
		{
			controls.append(AuControlButton(tr("Next Seat"), Id(), 0, DaVehicle::CI_NextSeat, texV));
			controls.append(AuControlButton(tr("Back Seat"), Id(), 0, DaVehicle::CI_PreviousSeat, texV));
		}
	}

	if(IsConsolePose())
	{
		// At a Captain's chair I can switch console modes.
		const DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			if(IsCommandStationPose())
			{
				GLuint texCmd = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVCmd", ":data/dp_CmdConsole.png");
				controls.append(AuControlButton(tr("Headlight"), Id(), 0, DpStationEmpty::CI_HeadlightOnOff, texCmd));
				controls.append(AuControlButton(tr("Door Remote"), Id(), 0, DpStationEmpty::CI_DoorRemote, texCmd));
				controls.append(AuControlButton(tr("Hull Doors"), Id(), 0, DpStationEmpty::CI_CloseHullDoors, texCmd));
			}
			else if(IsHelmStationPose())
			{
				GLuint texPilot = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVPilot", ":data/dp_PilotConsole.png");
				controls.append(AuControlButton(tr("Door Remote"), Id(), 0, DpStationHelm::CI_DoorRemote, texPilot));
			}

			if(ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()))
			{
				GLuint texCmd = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVCmd", ":data/dp_CmdConsole.png");
				GLuint texEngineer = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVEngineer", ":data/dp_EngineerConsole.png");
				GLuint texFireControl = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVFireControl",
						":data/dp_FireControlConsole.png");
				GLuint texPilot = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVPilot", ":data/dp_PilotConsole.png");
				GLuint texNav = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVNav", ":data/dp_NavConsole.png");
				GLuint texSensor = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVSensor", ":data/dp_SensorConsole.png");
				GLuint texShield = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVShield", ":data/dp_ShieldConsole.png");
				if(!IsCommandStationPose())
					controls.append(AuControlButton(tr("Command"), Id(), 0, CI_ConsoleCommand, texCmd));
				if(!IsEngineStationPose() && ship->IsEngineerManned())
					controls.append(AuControlButton(tr("Engineer"), Id(), 0, CI_ConsoleEngineer, texEngineer));
				if(!IsFireControlStationPose() && ship->IsWeaponManned() && ship->HasWeaponBay())
					controls.append(AuControlButton(tr("Fire Control"), Id(), 0, CI_ConsoleWeapon, texFireControl));
				if(!IsHelmStationPose() && ship->IsPilotManned())
					controls.append(AuControlButton(tr("Helm"), Id(), 0, CI_ConsolePilot, texPilot));
				if(!IsNavStationPose() && ship->IsNavigatorManned())
					controls.append(AuControlButton(tr("Navigator"), Id(), 0, CI_ConsoleNavigator, texNav));
				if(!IsSensorStationPose() && ship->IsSensorManned())
					controls.append(AuControlButton(tr("Sensors"), Id(), 0, CI_ConsoleSensor, texSensor));
				if(!IsShieldStationPose() && ship->IsShieldManned())
					controls.append(AuControlButton(tr("Shields"), Id(), 0, CI_ConsoleShield, texShield));
			}
		}
	}
	else if(IsTurretPose())
	{
		GLuint texTurret = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVTurret", ":data/dp_TurretTopBottom.png");
		controls.append(AuControlButton(tr("Reload"), Id(), 0, (qint8)CI_ReloadItem, texTurret));
	}

	if(IsInCapsule())
	{
		const DaCapsule* capsule = FindParentCapsuleInEnv();
		if(capsule)
		{
			// Capsules may have remote cameras operated from the command station.
			if(IsCommandStationPose())
			{
				if(capsule->RemoteCameraNext(0))
				{
					// Capsule has a remote camera.
					GLuint texCam = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVCam", ":data/ScCamera.png");
					if(RemoteCameraId())
					{
						controls.append(AuControlButton(tr("Next"), Id(), 0, DpStationEmpty::CI_RemoteCameraNext, texCam));
						controls.append(AuControlButton(tr("Previous"), Id(), 0, DpStationEmpty::CI_RemoteCameraPrevious, texCam));
					}
					controls.append(AuControlButton(tr("Camera"), Id(), 0, DpStationEmpty::CI_RemoteCameraOnOff, texCam));
				}
			}

			// On bridge of capsule, there are switches for nav info.
			QString roomName = CapsuleRoomName();	// May be blank.
			if(!IsNavStationPose() && capsule->RoomHasNavHUD(roomName))
			{
				GLuint texNames = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVNames", ":data/ac_NamesOn.bmp");
				controls.append(AuControlButton(tr("Names"), Id(), 0, (qint8)CI_RoomNavSystemNames, texNames));
				GLuint texRings = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVRings", ":data/pc_RingsBoth.bmp");
				controls.append(AuControlButton(tr("Orbits"), Id(), 0, (qint8)CI_RoomNavOrbitRings, texRings));
				GLuint texWorms = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVWorms", ":data/ac_WormholesOn.bmp");
				controls.append(AuControlButton(tr("Wormholes"), Id(), 0, (qint8)CI_RoomNavWormholes, texWorms));
			}

			// Rooms have switches for lights.
			GLuint texLight = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVLight", ":data/Light.png");
			INTERIOR_LIGHT light = capsule->InteriorLight(roomName);
			switch(light)
			{
			case INTLIGHT_Dark:
				controls.append(AuControlButton(tr("Dim"), Id(), 0, (qint8)CI_RoomLightsBrighter, texLight));
				break;
			case INTLIGHT_Bright:
				controls.append(AuControlButton(tr("Dim"), Id(), 0, (qint8)CI_RoomLightsDarker, texLight));
				break;
			case INTLIGHT_Dim:
				controls.append(AuControlButton(tr("Bright"), Id(), 0, (qint8)CI_RoomLightsBrighter, texLight));
				controls.append(AuControlButton(tr("Dark"), Id(), 0, (qint8)CI_RoomLightsDarker, texLight));
				break;
			case INTLIGHT_Invalid:
				break;
			}
		}
	}

	AuGearPos gearPos(CurrentItemPos());
	INVENTORYSUB itemSub = CurrentItemSub();
	m_GearLock.lockForRead();
	if(gearPos.IsGear())
	{
		const AuItem* gearItem = m_Gear.ItemAt(gearPos);
		if(gearItem)
		{
			GLuint texCID = ari.CommodityTextureN(gearItem->CommodityId());
			if(!IsTurretPose())
			{
				const AuItemWeaponAmmo* item = dynamic_cast<const AuItemWeaponAmmo*>(gearItem);
				if(item)
				{
					bool okToReload = false;
					switch(itemSub)
					{
					case IPS_Item:
						break;			// Item mode never needs ammo.
					case IPS_Weapon1:
						okToReload = item->Ammo1Loaded() < item->Ammo1Qty();
						break;
					case IPS_Weapon2:
						if(item->Ammo2Type() == AMMO_Energy1)
							okToReload = item->Ammo1Loaded() < item->Ammo1Qty();
						else
							okToReload = item->Ammo2Loaded() < item->Ammo2Qty();
						break;
					}
					if(okToReload)
						controls.append(AuControlButton(tr("Reload"), Id(), 0, (qint8)CI_ReloadItem, texCID));
				}
			}

			if(gearItem->CommodityId() == CID_Water)
			{
				if(HasCondition(AuConAcid::ConditionType()))
					controls.append(AuControlButton(tr("Decontaminate"), Id(), 0, (qint8)CI_UseItem, texCID));
				else if(HasCondition(AuConFire::ConditionType()))
					controls.append(AuControlButton(tr("Extinguish"), Id(), 0, (qint8)CI_UseItem, texCID));
				else
					controls.append(AuControlButton(tr("Drink"), Id(), 0, (qint8)CI_UseItem, texCID));
			}
			else if(gearItem->IsFood(EcoRole()))
			{
				if(CurrentHunger() < MaxHunger())
				{
					if(gearItem->CommodityId() == CID_Milk)
						controls.append(AuControlButton(tr("Drink"), Id(), 0, (qint8)CI_UseItem, texCID));
					else
						controls.append(AuControlButton(tr("Eat"), Id(), 0, (qint8)CI_UseItem, texCID));
				}
			}
			else if(gearItem->IsIntoxicant())
				controls.append(AuControlButton(tr("Drink"), Id(), 0, (qint8)CI_UseItem, texCID));
			else if(gearItem->CommodityId() == CID_MedicalKit)
			{
				if(CurrentHitPoints() < MaxHitPoints())
					controls.append(AuControlButton(tr("Apply"), Id(), 0, (qint8)CI_UseItem, texCID));
			}
			else if(gearItem->CommodityId() == CID_Antidote)
			{
				if(HasCondition(AuConPoison::ConditionType()))
					controls.append(AuControlButton(tr("Apply"), Id(), 0, (qint8)CI_UseItem, texCID));
			}
			else if(gearItem->CommodityId() == CID_Antitoxin)
				controls.append(AuControlButton(tr("Immunize"), Id(), 0, (qint8)CI_UseItem, texCID));
			else if(gearItem->IsLight())
			{
				if(itemSub == IPS_Item)
				{
					if(gearItem->State())
						controls.append(AuControlButton(tr("Light Off"), Id(), 0, (qint8)CI_UseItem, texCID));
					else
						controls.append(AuControlButton(tr("Light On"), Id(), 0, (qint8)CI_UseItem, texCID));
				}
			}
			else if(gearItem->CommodityId() == CID_Drum)
			{
				if(gearItem->State())
					controls.append(AuControlButton(tr("Sound Off"), Id(), 0, (qint8)CI_UseItem, texCID));
				else
					controls.append(AuControlButton(tr("Sound On"), Id(), 0, (qint8)CI_UseItem, texCID));
			}
			else if(gearItem->IsEnvironmentHelmet())
			{
				if(gearItem->State())
					controls.append(AuControlButton(tr("Open"), Id(), 0, (qint8)CI_UseItem, texCID));
				else
					controls.append(AuControlButton(tr("Close"), Id(), 0, (qint8)CI_UseItem, texCID));
			}
			else if(gearItem->IsDetonatorWeapon())
				controls.append(AuControlButton(tr("Arm and Drop"), Id(), 0, (qint8)CI_UseItem, texCID));
			else if(gearItem->IsFuseWeapon())
				controls.append(AuControlButton(tr("Fuse and Drop"), Id(), 0, (qint8)CI_UseItem, texCID));
			else if(gearItem->CommodityId() == CID_SpacecraftTitle && !gearItem->IsTitleVoid())
				controls.append(AuControlButton(tr("Claim Title"), Id(), 0, (qint8)CI_UseItem, texCID));
			//else
				//controls.append(AuControlButton(tr("Use"), Id(), 0, (qint8)CI_UseItem, texCID));

			if(gearItem->IsDetonatorWeapon())
				controls.append(AuControlButton(tr("Arm and Throw"), Id(), 0, (qint8)CI_ThrowItem, texCID));
			else if(gearItem->IsFuseWeapon())
				controls.append(AuControlButton(tr("Fuse and Throw"), Id(), 0, (qint8)CI_ThrowItem, texCID));
			else
				controls.append(AuControlButton(tr("Throw"), Id(), 0, (qint8)CI_ThrowItem, texCID));

			controls.append(AuControlButton(tr("Drop"), Id(), 0, (qint8)CI_DropItem, texCID, true));

			controls.append(AuControlButton(tr("Place"), Id(), 0, (qint8)CI_PlaceItem, texCID, true));
		}
	}

	if(CurrentHunger() < MaxHunger() && m_Gear.HasFood(EcoRole()))
	{
		// Check same criteria used by EatMealFromGear to show the Eat Food button.
		unsigned short int eatThreshold = MaxHunger()/10;
		if(!eatThreshold)
			eatThreshold = 1;
		if(CurrentHunger() <= eatThreshold)
		{
			GLuint texGear = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVGear", ":data/Inventory.png");
			controls.append(AuControlButton(tr("Eat"), Id(), 0, (qint8)CI_EatFood, texGear));
		}
	}

	const AuItem* helmet = m_Gear.HelmetItemWorn();
	if(helmet && helmet->IsEnvironmentHelmet())
	{
		GLuint texCID = ari.CommodityTextureN(helmet->CommodityId());
		if(helmet->State())
			controls.append(AuControlButton(tr("Open"), Id(), 0, (qint8)CI_HelmetVisor, texCID));
		else
			controls.append(AuControlButton(tr("Close"), Id(), 0, (qint8)CI_HelmetVisor, texCID));
	}
	m_GearLock.unlock();
}

void DaPerson::EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
{
	Q_UNUSED(panelId);
	Q_UNUSED(controls);
	Q_UNUSED(ari);
}

bool DaPerson::IsControl(const DaAtom* sceneAtom) const
{
	if(this == sceneAtom && HasConsole() && IsConsolePose())
		return true;
	else
		return false;
}

void DaPerson::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
		DiControlInputArray &aActiveControls, bool bContinuous)
{
	if(HasConsole() && IsConsolePose())
	{
		DpStation* console = LockConsoleForRead();
		console->SampleControlPanelInput(idPanel, fPanelLoc, aActiveControls, bContinuous);
		UnlockConsole();
	}
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be
// passed.
// Only control inputs that have been activated will be passed.
void DaPerson::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, const AuGearPos& gearPos,
		qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed)
{
	//qDebug() << "DaPerson::ProcessControlInput" << Id() << Name() << "activeControls" << aActiveControls.count()
		//<< "isDead" << IsBodyDead() << "awaitingSpawn" << IsAwaitingSpawn() << "spawnRecalling" << IsAwaitingSpawnRecall()
		//<< "head" << CurrentHeadTilt()*RAD2DEGf << CurrentGeneralPose() << this;
	if(!IsNormal())
		return;
	// Don't process control input when dead or when waiting to spawn elsewhere or awaiting initial placement.
	if(IsBodyDead() || IsAwaitingSpawn() || NeedsInit())
		return;
	if(IsAwaitingSpawnRecall())
	{
		// While awaiting spawn recall, only one command can be processed. That is the spawn recall command to cancel the recall.
		if(cc == CONTROL_Client)
			return;
		for(int ic=0; ic<aActiveControls.count(); ic++)
		{
			const DiControlInput& ci = aActiveControls[ic];
			if(ci.GetId() == CI_SpawnRecall)
			{
				// Cancel awaiting recall state.
				m_PersonLock.lockForWrite();
				m_uiActions &= ~ACT_SpawnRecall;
				m_PersonLock.unlock();
				SetModified();

				// Player Statistics
				Stats().SpawnRecallCanceled();

				// Do not process any more input from this dude.
				AuCommThoughtMessage msg(AuCommThoughtMessage::RecallCanceled, Id(), Name());
				g_Au.SendAtomChat(msg);
				break;
			}
		}
		return;
	}

	// Base class does nothing.
	//DaSentient::ProcessControlInput(aActiveControls, sceneAtom, chPrimarySel, subPos, chSelValue, cc);
	//qDebug() << "DaPerson::ProcessControlInput" << Id() << Name() << "ladderPose" << IsLadderPose();

	DaCreature* poseWho = this;									// pose my dude unless mounted
	bool isFlyer = false;
	DaAtom* parent = Parent();
	DaCapsule* capsule = dynamic_cast<DaCapsule*>(parent);
	DaVessel* ship = dynamic_cast<DaVessel*>(parent);
	DaBd* bd = 0;
	DaAnimal* mount = 0;										// We only control mounted animals. People control themselves.
	DaVehicle* vehicle = 0;
	if(ship)
	{
		// When simulating local motion, we sometimes want smooth response from the console, particularly when rotating the ship.
		// Do not process console input if the ship is not normal; it could be waiting for a wormhole handoff.
		if(HasConsole() && IsConsolePose() && ship->IsNormal())
		{
			// Verify the console is operational.
			AuSysId sysId(ControlSysId());
			if(sysId.System() == SYS_Count || ship->IsConsoleOperational(sysId))
			{
				DpStation* console = LockConsoleForRead();
				if(console)
				{
					console->ProcessControlInput(aActiveControls, sceneAtom, *ship, cc, Location());
					if(sysId.Which() != ControlId())
					{
						// The console changed which system it is monitoring/controlling.
						m_ControlId = console->SystemWhich();
						SetModified();
					}
				}
				UnlockConsole();
			}
		}
	}
	else
	{
		bd = dynamic_cast<DaBd*>(parent);
		if(bd)
		{
			// When simulating local motion, we sometimes want smooth response from the console, particularly when rotating the bd.
			// Do not process console input if the bd is not normal; it could be waiting for a wormhole handoff.
			if(HasConsole() && IsConsolePose() && bd->IsNormal())
			{
				// Verify the console is operational.
				AuSysId sysId(ControlSysId());
				if(sysId.System() == SYS_Count || bd->IsConsoleOperational(sysId))
				{
					DpStation* console = LockConsoleForRead();
					if(console)
					{
						console->ProcessControlInput(aActiveControls, sceneAtom, *bd, cc, Location());
						if(sysId.Which() != ControlId())
						{
							// The console changed which system it is monitoring/controlling.
							m_ControlId = console->SystemWhich();
							SetModified();
						}
					}
					UnlockConsole();
				}
			}
		}
		else
		{
			vehicle = dynamic_cast<DaVehicle*>(parent);
			if(vehicle)
			{
				// When operating a vehicle, send the input to the vehicle.
				vehicle->ProcessControlInput(aActiveControls, sceneAtom, gearPos, chSelValue, cc, msElapsed);

				// Flying vehicles that cannot hover have movement restrictions.
				//if(mount->IsFlying() && !mount->CanHover())
					//isFlyer = true;
			}
			else
			{
				mount = dynamic_cast<DaAnimal*>(parent);
				if(mount)
				{
					// Mounted so pose the animal.
					poseWho = mount;

					// Flying mounts that cannot hover have movement restrictions.
					if(mount->IsFlying() && !mount->CanHover())
						isFlyer = true;
				}
			}
		}
	}

	// When dying, we can process one and only one input. That is E to get off someone's back who is carrying me. Otherwise I can
	// be taken captive forever.
	bool exitMountOnly = false;									// true if I am only allowed to exit my mount.
	if(IsBodyDyingOrDead())
	{
		if(!IsRider())
			return;												// I am not riding a mount.
		exitMountOnly = true;									// The only action I can perform is to exit my mount.
	}

	bool runMod = false;
	bool isRunning = poseWho->IsMovingFast();
	bool keepRunning = isRunning;
	bool continuousMove = false;
	unsigned int fwdback = 0;	// movement
	unsigned int lateral = 0;	// movement
	unsigned int vertical = 0;	// movement
	AuPose nuPose = poseWho->CurrentGeneralPose();
	quint16 nuStationId = poseWho->StationId();
	AuStation::OCCUPANT_POSITION nuOccPos = poseWho->ConsoleOccupantPosition();
	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_ReloadItem:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				if(IsTurretPose())
				{
					// Reload my turret.
					if(capsule)
					{
						if(capsule->ReloadTurret(StationId()))
						{
							AuItem* ammo = capsule->TurretAmmunitionClone(StationId());
							if(ammo)
							{
								// Send a log message to the player.
								AuCommThoughtMessage::MESSAGE tm;
								if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
									tm = AuCommThoughtMessage::TurretCharged;
								else
									tm = AuCommThoughtMessage::TurretLoaded;
								AuCommThoughtMessage msg(tm, ammo->ListName(), Id(), Name());
								delete ammo;
								g_Au.SendAtomChat(msg);
							}
						}
					}
				}
				else
				{
					// Reload the current item.
					QString ammoDesc;
					AuGearPos gearPos(CurrentItemPos());
					INVENTORYSUB isub = CurrentItemSub();
					m_GearLock.lockForWrite();
					if(m_Gear.ReloadItem(gearPos, isub, ammoDesc))
					{
						// Weight changes when ammo is consumed.
						UpdateGearWeight();

						// Send a log message to the player.
						const AuItem* item = m_Gear.ItemAt(gearPos);
						if(item)
						{
							QString text;
							if(ammoDesc.isEmpty())
								text = tr("%1 loaded.").arg(item->SayName());
							else
								text = tr("%1 loaded with %2.").arg(item->SayName()).arg(ammoDesc);
							Thought(text);
						}
					}
					m_GearLock.unlock();
				}
			}
			break;

		case CI_EatFood:
			if(cc != CONTROL_Client && !exitMountOnly)
				EatFoodFromGear();
			break;

		case CI_HelmetVisor:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				m_GearLock.lockForRead();
				AuGearPos pos = m_Gear.FindHelmetWorn();
				bool posValid = m_Gear.IsPosValid(pos);
				m_GearLock.unlock();
				if(posValid)
					UseGearAt(pos);
			}
			break;

		case CI_SayHello:
			if(cc != CONTROL_Client && !exitMountOnly && !InDesignEnvironment())
			{
				AuCommHailRequest hi(EmpireId(), QString(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
						FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(hi);
			}
			break;

		case CI_RoomLightsBrighter:
			if(cc != CONTROL_Client)
			{
				if(IsInCapsule())
				{
					DaCapsule* cap = FindParentCapsuleInEnv();
					if(cap)
					{
						QString roomName = CapsuleRoomName();
						switch(cap->InteriorLight(roomName))
						{
						case INTLIGHT_Dark:
							cap->InteriorLightToggle(roomName, INTLIGHT_Dim);
							cap->SetModified();
							break;
						case INTLIGHT_Bright:
							break;
						case INTLIGHT_Dim:
						case INTLIGHT_Invalid:
							cap->InteriorLightToggle(roomName, INTLIGHT_Bright);
							cap->SetModified();
							break;
						}
					}
				}
			}

			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Lights, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_RoomLightsDarker:
			if(cc != CONTROL_Client)
			{
				if(IsInCapsule())
				{
					DaCapsule* cap = FindParentCapsuleInEnv();
					if(cap)
					{
						QString roomName = CapsuleRoomName();
						switch(cap->InteriorLight(roomName))
						{
						case INTLIGHT_Dark:
							break;
						case INTLIGHT_Bright:
							cap->InteriorLightToggle(roomName, INTLIGHT_Dim);
							cap->SetModified();
							break;
						case INTLIGHT_Dim:
						case INTLIGHT_Invalid:
							cap->InteriorLightToggle(roomName, INTLIGHT_Dark);
							cap->SetModified();
							break;
						}
					}
				}
			}

			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Lights, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_RoomNavOrbitRings:
			if(cc != CONTROL_Client)
			{
				if(IsInCapsule())
				{
					QString roomName = CapsuleRoomName();
					if(!roomName.isEmpty())
					{
						DaCapsule* cap = FindParentCapsuleInEnv();
						if(cap)
						{
							bool oo = cap->OrbitRingsVisible(roomName);
							bool moono = cap->MoonOrbitRingsVisible(roomName);
							if(!oo)
							{
								cap->SetOrbitRingsVisible(roomName, true);
								cap->SetMoonOrbitRingsVisible(roomName, false);
							}
							else if(!moono)
							{
								cap->SetMoonOrbitRingsVisible(roomName, true);
							}
							else
							{
								cap->SetOrbitRingsVisible(roomName, false);
								cap->SetMoonOrbitRingsVisible(roomName, false);
							}
							cap->SetModified();
						}
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				if(IsInCapsule())
				{
					QString roomName = CapsuleRoomName();
					if(!roomName.isEmpty())
					{
						DaCapsule* cap = FindParentCapsuleInEnv();
						if(cap)
						{
							bool oo = cap->OrbitRingsVisible(roomName);
							bool moono = cap->MoonOrbitRingsVisible(roomName);
							if(!oo)
							{
								AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,.9f);
								sceneAtom->AddEffect(buttonClick);
							}
							else if(!moono)
							{
								AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,1);
								sceneAtom->AddEffect(buttonClick);
							}
							else
							{
								AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,.8f);
								sceneAtom->AddEffect(buttonClick);
							}
							cap->SetModified();
						}
					}
				}
			}
			break;
		case CI_RoomNavSystemNames:
			if(cc != CONTROL_Client)
			{
				if(IsInCapsule())
				{
					QString roomName = CapsuleRoomName();
					if(!roomName.isEmpty())
					{
						DaCapsule* cap = FindParentCapsuleInEnv();
						if(cap)
						{
							int vis = cap->SystemNamesVisible(roomName);
							vis++;
							if(vis > 2)
								vis = 0;
							cap->SetSystemNamesVisible(roomName, vis);
							cap->SetModified();
						}
					}
				}
			}

			if(cc != CONTROL_Server)
			{
				if(IsInCapsule())
				{
					QString roomName = CapsuleRoomName();
					if(!roomName.isEmpty())
					{
						DaCapsule* cap = FindParentCapsuleInEnv();
						if(cap)
						{
							int vis = cap->SystemNamesVisible(roomName);
							vis++;
							if(vis > 2)
								vis = 0;
							float bp = .8f + vis/10.f;	// button pitch
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SystemNames, 1,bp);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;
		case CI_RoomNavWormholes:
			if(cc != CONTROL_Client)
			{
				if(IsInCapsule())
				{
					QString roomName = CapsuleRoomName();
					if(!roomName.isEmpty())
					{
						DaCapsule* cap = FindParentCapsuleInEnv();
						if(cap)
						{
							bool vis = cap->WormholeLinesVisible(roomName);
							vis = !vis;
							cap->SetWormholeLinesVisible(roomName, vis);
							cap->SetModified();
						}
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				if(IsInCapsule())
				{
					QString roomName = CapsuleRoomName();
					if(!roomName.isEmpty())
					{
						DaCapsule* cap = FindParentCapsuleInEnv();
						if(cap)
						{
							bool vis = cap->WormholeLinesVisible(roomName);
							vis = !vis;
							float bp = vis ? 1 : .9f;	// button pitch
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Wormholes, 1,bp);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;

		case CI_DoorOpen:
			if(cc != CONTROL_Client)
			{
				if(!ci.TargetId().IsNull() && ci.TargetLoc().Length() < 10)
				{
					DaAtom* target = 0;
					DaAtom* system = FindEnvironmentAtom();
					if(system)
						target = system->FindIdRecursive(ci.TargetId());
					DaCapsule* capTarget = dynamic_cast<DaCapsule*>(target);
					if(capTarget)
						capTarget->ToggleDoorState(*this, ci.TargetPart());
				}
			}
			break;

		case CI_DoorLockToggle:
			if(cc != CONTROL_Client)
			{
				if(!ci.TargetId().IsNull() && ci.TargetLoc().Length() < 10)
				{
					DaAtom* target = 0;
					DaAtom* system = FindEnvironmentAtom();
					if(system)
						target = system->FindIdRecursive(ci.TargetId());
					DaCapsule* capTarget = dynamic_cast<DaCapsule*>(target);
					if(capTarget)
						capTarget->ToggleDoorLock(*this, ci.TargetPart());
				}
			}
			if(cc != CONTROL_Server)
			{
				if(!ci.TargetId().IsNull() && ci.TargetLoc().Length() < 10)
				{
					DaAtom* target = 0;
					DaAtom* system = FindEnvironmentAtom();
					if(system)
						target = system->FindIdRecursive(ci.TargetId());
					DaCapsule* capTarget = dynamic_cast<DaCapsule*>(target);
					if(capTarget)
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_DoorLock, 1,1.05f);
						sceneAtom->AddEffect(buttonClick);
					}
				}
			}
			break;

		case CI_PlaceItem:
			if(exitMountOnly)
				break;
			if(cc == CONTROL_Client)
				break;
			if(IsInCapsule() || (IsOnGround() && dynamic_cast<DaWorldLL*>(Parent())))
				PlaceItemOnTarget(CurrentItemPos(), ci.TargetId(), ci.TargetLoc());	// In a capsule or on the ground on a planet.
			break;

		case CI_BrakeOrJump:
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(!exitMountOnly)
			{
				// Entity brakes, never jumps; it's annoying.
				if(poseWho->IsInGravity() && poseWho->CanJump() && !AgentEntity())
				{
					// Interpret as jump when not in zero-G.
					poseWho->Jump(JumpPower(), poseWho->JumpVectorFromCurrentPose());
					nuPose = AuPose::Jump;
					poseWho->SetInMotion();
					poseWho->TickTock();
				}
				else
				{
					m_PersonLock.lockForWrite();
					m_uiActions |= ACT_Brake;					// Apply brakes instead of jumping.
					m_uiActions &= ~ACT_Continuous;				// Insure the continuous movement bit is off.
					m_tmLastBrake.restart();
					m_PersonLock.unlock();
				}
			}
			break;

		case CI_Brake:
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(!exitMountOnly)
			{
				m_PersonLock.lockForWrite();
				m_uiActions |= ACT_Brake;						// Apply brakes
				m_uiActions &= ~ACT_Continuous;					// Insure the continuous movement bit is off.
				m_tmLastBrake.restart();
				m_PersonLock.unlock();
			}
			break;

		case CI_Jump:
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;
			if(poseWho->CanJump())
			{
				poseWho->Jump(JumpPower(), poseWho->JumpVectorFromCurrentPose());
				nuPose = AuPose::Jump;
				poseWho->SetInMotion();
				poseWho->TickTock();
			}
			break;

		case CI_MovePowerIncrease:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				if(m_PowerMoveMax < 15)
				{
					m_PersonLock.lockForWrite();
					m_PowerMoveMax++;
					m_LastJumpOrMovePower.Set();
					m_PersonLock.unlock();
					SetModified();
				}
			}
			break;

		case CI_MovePowerDecrease:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				if(m_PowerMoveMax)
				{
					m_PersonLock.lockForWrite();
					m_PowerMoveMax--;
					if(m_PowerMove > m_PowerMoveMax)
						m_PowerMove = m_PowerMoveMax;
					m_LastJumpOrMovePower.Set();
					m_PersonLock.unlock();
					SetModified();
				}
			}
			break;

		case CI_JumpPowerIncrease:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				if(m_PowerJump < 15)
				{
					m_PersonLock.lockForWrite();
					m_PowerJump++;
					m_LastJumpOrMovePower.Set();
					m_PersonLock.unlock();
					SetModified();
				}
			}
			break;

		case CI_JumpPowerDecrease:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				if(m_PowerJump)
				{
					m_PersonLock.lockForWrite();
					m_PowerJump--;
					m_LastJumpOrMovePower.Set();
					m_PersonLock.unlock();
					SetModified();
				}
			}
			break;

		case CI_SpawnRecall:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				// Use the last move control time to know when respawn is ok.
				int spawnWait;
				if(IsLineageHighborn())
					spawnWait = g_Au.Dice().Roll(1,11,4);	// 5-15sec
				else if(IsLineageDemiavatar())
					spawnWait = g_Au.Dice().Roll(1,16,29);	// 30-45sec
				else
					spawnWait = g_Au.Dice().Roll(1,31,14);	// 15-45sec
				AuTime spawnTime = AuTime::Now().AddSecs(spawnWait);
				SpawnRecall(spawnTime);
				DismountAllRiders();
				SetModified();

				// Player Statistics
				Stats().SpawnRecalled();

				// Do not process any more input from this dude.
				AuCommThoughtMessage msg(AuCommThoughtMessage::NoPlaceLikeHome, Id(), Name());
				g_Au.SendAtomChat(msg);
				return;
			}
			break;
		case CI_SpawnTie:
			if(cc != CONTROL_Client && !exitMountOnly)
			{
				// Spawn tie to the developed terrain tile in which the person is standing.
				QString partingThought;
				if(BdId().IsNull())
				{
					partingThought = tr("To declare a home, I must first move to a building.");
				}
				else if(InIndustry() == IID_Road)
				{
					partingThought = tr("This road will not make a good home. "
							"I should move to a building, preferably one with a bedroom.");
				}
				else if(InIndustry() == IID_None)
				{
					partingThought = tr("This zone of the city is off limits. "
							"I should move to a building, preferably one with a bedroom.");
				}
				else
				{
					const DaWorld* planet = FindParentPlanetInEnv();
					if(planet)
					{
						if(!BdId().IsNull())
						{
							const DaBd* bd = planet->FindBd(BdId());
							const DaSolarSystem* system = planet->FindParentSolarSystemInEnv();
							if(bd && system)
							{
								quint32 bdempire = bd->EmpireId();
								POLITICAL_STANCE stance = PoliticalStance(bdempire, EmpireId());
								if(stance == POL_Friend || stance == POL_Empire || stance == POL_Vassal || bdempire == EmpireId())
								{
									// This person is standing in a developed tile of a city that is friendly toward the player.
									QString name;
									QString cityName = bd->CityName();
									if(cityName.isEmpty())
										name = tr("%1 at %2 on %3")
											.arg(bd->Name())
											.arg(InIndustryLL().ToString())
											.arg(planet->Name());
									else
										name = tr("%1 at %2 in %3")
											.arg(bd->Name())
											.arg(InIndustryLL().ToString())
											.arg(cityName);
									SetSpawnCity(BdId(), name, InIndustryLL(), system->Id(), system->Name());
									partingThought = tr("I declare this %1 to be my home.")
										.arg(name);
									SetModified();
								}
								else
								{
									QString empireName;
									const DgData& gd = g_Au.LockDataForRead();
									gd.EmpireName(bdempire, empireName);
									g_Au.UnlockData();
									if(stance == POL_Neutral)
										partingThought = tr("I can't declare this building my home. "
												"The owner %2 is neutral toward my empire.")
											.arg(empireName);
									else
										partingThought = tr("I can't declare this building my home. "
												"The owner %2 has declared my empire an enemy.")
											.arg(empireName);
								}
							}
							else
								partingThought = tr("The building is gone? What twilight zone have I found myself in? "
										"Recall home might be the answer.");
						}
					}
				}

				if(!partingThought.isEmpty())
					Thought(partingThought);
			}
			break;
		case CI_EnterExit:
			//qDebug() << "DaPerson::ProcessControlInput CI_EnterExit" << Id() << Name()
				//<< "parent" << Parent()->Id() << Parent()->Name() << "cc" << cc;
			keepRunning = false;
			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();					// Save console settings on the client.

			// Entry/exit is handled varyingly by the server or the client so we check the cc flag at each action.
			if(IsRider())
			{
				if(cc != CONTROL_Server)
				{
					// I am riding a creature. Dismount it.
					DismountCreature();
					nuPose = CurrentGeneralPose();					// Prevent pose changing code below from changing my pose.
					nuStationId = StationId();
					nuOccPos = ConsoleOccupantPosition();
					poseWho = this;
					mount = 0;
					m_PersonLock.lockForWrite();
					m_uiActions &= ~ACT_Continuous;					// Insure the continuous movement bit is off.
					m_PersonLock.unlock();
				}
				break;
			}
			if(exitMountOnly)
				break;

			// Check for other things to enter/exit/engage with.
			if(vehicle)
			{
				// I am riding a vehicle. Dismount it.
				if(cc != CONTROL_Server && DismountVehicle())
				{
					// Exit from vehicle was not blocked.
					nuPose = CurrentGeneralPose();				// Prevent pose changing code below from changing my pose.
					nuStationId = StationId();
					nuOccPos = ConsoleOccupantPosition();
					vehicle = 0;
					m_PersonLock.lockForWrite();
					m_uiActions &= ~ACT_Continuous;				// Insure the continuous movement bit is off.
					m_PersonLock.unlock();
					//qDebug() << "DaPerson::ProcessControlInput CI_EnterExit" << Id() << Name()
						//<< "dismounted vehicle, nu parent" << Parent()->Id() << Parent()->Name();
				}
			}
			else if(IsLockedPose())
			{
				// I am at a game table, console, or berth so I must move away from it.
				if(cc != CONTROL_Server)
				{
					//qDebug("DaPerson::ProcessControlInput - Leave console.");
					if(ship && nuPose.IsHelmStation())
						ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.
					ExitConsole();
					nuPose = CurrentGeneralPose();				// Prevent pose changing code below from changing my pose.
					nuStationId = StationId();
					nuOccPos = ConsoleOccupantPosition();
					m_PersonLock.lockForWrite();
					m_uiActions &= ~ACT_Continuous;				// Insure the continuous movement bit is off.
					m_PersonLock.unlock();
				}
			}
			else if(IsLadderPose())
			{
				// Stand is all that is needed to release the ladder.
				if(cc != CONTROL_Server)
				{
					poseWho->Stand();
					nuPose = CurrentGeneralPose();				// Prevent pose changing code below from changing my pose.
					nuStationId = StationId();
					nuOccPos = ConsoleOccupantPosition();
					m_PersonLock.lockForWrite();
					m_uiActions &= ~ACT_Continuous;				// Insure the continuous movement bit is off.
					m_PersonLock.unlock();
				}
			}
			else
			{
				// I am not engaged with anything.
				// Look for a nearby creature or vehicle to mount.
				if(!ci.TargetId().IsNull())
				{
					// Attempt to mount the creature or vehicle that is targeted.
					//qDebug("%d attempting to mount target %d", Id().GetID(), ci.TargetId().GetID());
					DaAtom* target = 0;
					DaAtom* system = FindEnvironmentAtom();
					if(system)
						target = system->FindIdRecursive(ci.TargetId());
					DaCreature* creature = dynamic_cast<DaCreature*>(target);
					if(creature)
					{
						// Some kind of creature is targeted.
						DaAnimal* animal = dynamic_cast<DaAnimal*>(target);
						DaPerson* person = dynamic_cast<DaPerson*>(target);
						POLITICAL_STANCE stance;
						if(cc != CONTROL_Server)
							stance = PoliticalStance(EmpireId(), creature->EmpireId());
						else
							stance = PoliticalStanceMakeContact(creature);
						if(IsMating())
						{
							if(cc != CONTROL_Server)
							{
								QString text = tr("%1 can't be mounted while I am having sex.")
									.arg(creature->Name());
								Thought(text);
							}
						}
						else if(IsMounted())
						{
							if(cc != CONTROL_Server)
							{
								QString text = tr("%1 can't be mounted while I am carrying a rider.").arg(creature->Name());
								Thought(text);
							}
						}
						else if(ci.TargetRange(CameraOffset()) > 7)
						{
							if(cc != CONTROL_Server)
							{
								// Send a message to the client informing them that the creature is too far away.
								QString text = tr("%1 is too far away to mount. I should try sighting on a closer part of %2.")
									.arg(creature->Name())
									.arg(creature->HerHim(false));
								Thought(text);
							}
						}
						else if(creature->IsBodyDead())
						{
							if(cc != CONTROL_Server)
							{
								// Send a message to the client informing them that the creature is dead.
								QString text(tr("%1 is dead.").arg(creature->Name()));
								Thought(text);
							}
						}
						else if(creature->IsBodyDyingOrDead())
						{
							// Attempt to pick up the dying body.
							// I can't pick up large wild animals.
							if(animal && Volume() < animal->Volume() * .9f)
							{
								if(cc != CONTROL_Server)
								{
									QString text(tr("%1 is too big for me to carry.").arg(creature->Name()));
									Thought(text);
								}
							}
							else if(IsMounted())
							{
								if(cc != CONTROL_Server)
								{
									QString text(tr("%1 can't be picked up while I am carrying a rider.").arg(creature->Name()));
									Thought(text);
								}
							}
							else
							{
								// Pick up the dying body. This will stabilize it and stop it from dying until it is dropped.
								creature->ForceMountCreature(this);
							}
						}
						else if(creature->IsMating())
						{
							if(cc != CONTROL_Server)
							{
								// Send a message to the client informing them that the creature is already mounted.
								QString text = tr("%1 can't be mounted while %2 is having sex.").arg(creature->Name())
									.arg(creature->SheHe(false));
								Thought(text);
							}
						}
						else if(creature->IsMounted())
						{
							if(cc != CONTROL_Server)
							{
								// Send a message to the client informing them that the creature is already mounted.
								QString text = tr("%1 is already carrying a rider.").arg(creature->Name());
								Thought(text);
							}
						}
						else if(creature->IsLockedPose())
						{
							if(cc != CONTROL_Server)
							{
								if(creature->CurrentGeneralPose().IsPost())
								{
									QString text = tr("%1 can't be mounted while %2 is on duty.")
										.arg(creature->Name())
										.arg(creature->SheHe(false));
									Thought(text);
								}
								else
								{
									QString text = tr("%1 can't be mounted while %2 is working.")
										.arg(creature->Name())
										.arg(creature->SheHe(false));
									Thought(text);
								}
							}
						}
						else if(creature->IsRider())
						{
							if(cc != CONTROL_Server)
							{
								QString text = tr("%1 can't be mounted while %2 is riding a mount.")
									.arg(creature->Name())
									.arg(creature->SheHe(false));
								Thought(text);
							}
						}
						/*
						else if(person && stance == POL_Enemy)
						{
							if(cc != CONTROL_Server)
							{
								// Send a message to the client informing them that the person is an enemy.
								QString text;
								if(creature->Gender() == Male)
									text = tr("%1 is an enemy so I can't mount him.").arg(creature->Name());
								else
									text = tr("%1 is an enemy so I can't mount her.").arg(creature->Name());
								Thought(text);
							}
						}
						*/
						else if(animal)
						{
							// Target is an animal.
							if(animal->IsWild())
							{
								if(cc != CONTROL_Server)
								{
									// Send a message to the client informing them that the animal is wild.
									QString text = tr("%1 is wild so I can't mount %2.")
										.arg(creature->Name())
										.arg(creature->HerHim(false));
									Thought(text);
								}
							}
							else if(animal && animal->Volume() < Volume() * .9f)
							{
								// I can't mount small wild animals
								if(cc != CONTROL_Server)
								{
									// Send a message to the client informing them that the creature is too small.
									QString text(tr("%1 is too small to ride.").arg(creature->Name()));
									Thought(text);
								}
							}
							else
							{
								// Animal meets all criteria for mounting.
								// Capture animals that belong to the enemy.
								if(cc != CONTROL_Client && stance == POL_Enemy)
								{
									animal->SetEmpireId(EmpireId());
									animal->SetModified();
								}

								// Mount the animal.
								MountCreature(animal);
								nuPose = AuPose::Stand;	// pose for animal.
								nuStationId = 0;
								poseWho = creature;
							}
						}
						else if(person)
						{
							// Target is an avatar.
							if(person->IsEthereal())
							{
								if(cc != CONTROL_Server)
								{
									QString text(tr("%1 is too intangible to mount.").arg(creature->Name()));
									Thought(text);
								}
							}
							else if(person->CurrentGeneralPose() != AuPose::Crouch &&
									person->CurrentGeneralPose() != AuPose::CrouchCreep &&
									person->CurrentGeneralPose() != AuPose::ReceptiveMate)
							{
								if(cc != CONTROL_Server)
								{
									QString text;
									if(!IsReadyForMating())
										text = tr("%1 must crouch to let me mount %2.")
											.arg(person->Name())
											.arg(person->HerHim(false));
									else
										text = tr("%1 must crouch or be receptive to mating to let me mount %2.")
											.arg(person->Name())
											.arg(person->HerHim(false));
									Thought(text);
								}
							}
							else if(person->CurrentGeneralPose() == AuPose::ReceptiveMate && person->IsReadyForMating())
							{
								// Person meets all criteria for mating. Mount them if I am ready for sex.
								if(!IsReadyForMating())
								{
									// I am clothed.
									QString text(tr("I must get undressed to have sex with %1.").arg(creature->Name()));
									Thought(text);
								}
								else
									MateWithCreature(person);
							}
							else
							{
								// Person meets all criteria for mounting.
								MountCreature(person);
							}
						}
						else
						{
							// Target is a citizen, which is the only creature that is not an animal or an avatar.
							if(InDesignPreview())
							{
								MountCreature(creature);
							}
							else if(creature->CurrentGeneralPose() == AuPose::ReceptiveMate &&
										creature->IsReadyForMating() && IsReadyForMating())
							{
								// The creature is ready for mating and so am I. :D
								if(creature->Gender() == Gender() && Gender() != Hermaphrodite)
								{
									// We are the same gender and not hermaphrodites.
									QString text = tr("%1 avoided me. %2 doesn't go that way.")
										.arg(creature->Name())
										.arg(creature->SheHe(false));
									Thought(text);
								}
								else if(!creature->IsSexuallyCompatible(*this))
								{
									// We are not sexually compatible.
									QString text = tr("%1 avoided me. %2 doesn't enjoy rishathra.")
										.arg(creature->Name())
										.arg(creature->SheHe(false));
									Thought(text);
								}
								else
								{
									MateWithCreature(creature);
									creature->PoseVisage(AuVisage(AuPose::MouthGrin, 0), AuVisage(AuPose::MouthSmile, 0));
								}
							}
							else
							{
								// Can only mount citizens who are on the same crew as you.
								// Other creatures don't go for this, unless they are receptive to mating and of the correct gender.
								DaCitizen* citizen = dynamic_cast<DaCitizen*>(creature);
								if(citizen && !SpawnShip().IsNull() && citizen->AssignedShip() == SpawnShip())
								{
									// Citizen is not an enemy and we are assigned to the same ship.
									MountCreature(citizen);
								}
								else
								{
									if(cc != CONTROL_Server)
									{
										QString text = tr("%1 avoided me. %2 seems unsure of my intentions.")
											.arg(creature->Name())
											.arg(creature->SheHe(false));
										Thought(text);
									}
								}
							}
						}
						break;
					}
					else
					{
						// Not targeted on a creature.
						DaVehicle* targetVeh = dynamic_cast<DaVehicle*>(target);
						if(targetVeh)
						{
							// Targeted on a vehicle.
							if(ci.TargetRange(CameraOffset()) > 7)
							{
								if(cc != CONTROL_Server)
								{
									// Send a message to the client informing them that the vehicle is too far away.
									QString text(tr("%1 is too far away to enter. I should try sighting on a closer part of it.")
											.arg(targetVeh->Name()));
									Thought(text);
								}
							}
							else if(targetVeh->IsNoncombatant() && PoliticalStanceMakeContact(targetVeh) == POL_Enemy)
							{
								if(cc != CONTROL_Server)
								{
									QString text(tr("%1 cannot be entered while its empire is a noncombatant. "
												"An empire is a noncombatant while none of its citizens is on line.")
											.arg(targetVeh->Name()));
									Thought(text);
								}
							}
							else if(IsMounted())
							{
								if(cc != CONTROL_Server)
								{
									QString text(tr("%1 cannot be entered while I am carrying a rider.").arg(targetVeh->Name()));
									Thought(text);
								}
							}
							else
							{
								// Vehicle is close enough to try mounting.
								int seatIndex = MountVehicle(targetVeh);
								if(seatIndex < 0)
								{
									if(cc != CONTROL_Server)
									{
										// Send a message to the client informing them that all the seats are occupied.
										QString text(tr("All %1 seats are occupied.").arg(targetVeh->Name()));
										Thought(text);
									}
								}
								else
								{
									// I have mounted the vehicle.
									//qDebug() << "DaPerson::ProcessControlInput CI_EnterExit" << Id() << Name()
										//<< "mounted vehicle" << Parent()->Id() << Parent()->Name();
									vehicle = targetVeh;
									nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
									nuStationId = StationId();
									nuOccPos = ConsoleOccupantPosition();

									if(cc != CONTROL_Client)
									{
										// Capture the vehicle if it belongs to the enemy and I am sitting in seat 0.
										if(seatIndex == 0)
										{
											// I am sitting in seat zero.
											POLITICAL_STANCE stance = PoliticalStanceMakeContact(vehicle);
											if(stance == POL_Enemy)
											{
												vehicle->SetEmpireId(EmpireId());
												vehicle->SetModified();

												// Player Statistics
												Stats().VehicleCaptured(vehicle);
												IncreaseSkill(AuSkill::VehicleCapture, 1);

												VehicleCaptured(vehicle);
											}
										}

										// Player Statistics
										Stats().VehicleEntered(vehicle);
									}
								}
							}
							break;
						}
					}
				}

				// I did not mount a vehicle or creature.
				if(IsMounted())
				{
					// I am carrying a rider.
					if(GrabLadder())
					{
						// Grabbed a ladder.
						nuPose = CurrentGeneralPose();			// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();
					}
					else
					{
						// I did not grab a ladder so drop my rider(s).
						if(CurrentGeneralPose().IsMatingOrWaiting())
							nuPose = AuPose::SitOnGround;
						if(cc != CONTROL_Client)
							DismountAllRiders();
					}
				}
				else if(capsule)
				{
					// I am aboard a spacecraft. Try to sit at the nearest console.
					// Entry to ceiling/floor turrets requires the player to be looking down/up respectively.
					AuStation::TYPE contype = EnterConsole(capsule, cc);
					if(contype != AuStation::TYPE_Count)
					{
						// Entered a console.
						nuPose = CurrentGeneralPose();			// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();
						if(cc == CONTROL_Server)
						{
							// Player Statistics
							SYSTEM system = StationSystem();
							if(system != SYS_Count)
								Stats().SpacecraftStationEntered(contype, capsule->SystemQuality(system, StationId()));
						}
					}
					else if(GrabLadder())
					{
						// Grabbed a ladder.
						nuPose = CurrentGeneralPose();			// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();
					}
				}
				else if(GrabLadder())
				{
					// Grabbed a ladder.
					nuPose = CurrentGeneralPose();				// Prevent pose changing code below from changing my pose.
					nuStationId = StationId();
					nuOccPos = ConsoleOccupantPosition();
				}
			}
			break;
		case CI_Crouch:
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(!exitMountOnly)
			{
				nuPose = AuPose::Crouch;
				nuStationId = 0;
			}
			break;
		case CI_Stand:
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(!exitMountOnly)
			{
				nuPose = AuPose::Stand;
				nuStationId = 0;
			}
			break;
		case CI_SitStandToggle:
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(IsPatientStationPose())
				break;
			if(!exitMountOnly)
			{
				if(!poseWho->CurrentGeneralPose().IsSittingOnGround())
					nuPose = AuPose::SitOnGround;
				else
					nuPose = AuPose::Stand;
				nuStationId = 0;
				//qDebug("DaPerson::ProcessControlInput %d ProneStandToggle %d",
						//Id().GetID(), nuPose);
			}
			break;
		case CI_ProneStandToggle:
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(IsPatientStationPose())
				break;
			if(!exitMountOnly)
			{
				if(poseWho->IsProne())
					nuPose = AuPose::Stand;
				else
					nuPose = AuPose::Prone;
				nuStationId = 0;
				//qDebug("DaPerson::ProcessControlInput %d ProneStandToggle %d",
						//Id().GetID(), nuPose);
			}
			break;
		case CI_ReceptiveMateToggle:
			if(!g_Au.IsMatingOk())
				break;	// Nerf solo sex.
			keepRunning = false;
			if(cc == CONTROL_Server)
				break;
			if(!IsReadyForMating())
				break;	// I am clothed, dead, immature, or something.
			if(!exitMountOnly)
			{
				if(poseWho->CurrentGeneralPose() == AuPose::ReceptiveMate)
				{
					nuPose = AuPose::SitOnGround;
					DismountAllRiders();	// Drop any f'ers when I stand up.
				}
				else
				{
					nuPose = AuPose::ReceptiveMate;
					DismountAllRiders();	// Drop riders so I can't rape them.
				}
				nuStationId = 0;
			}
			break;
		case CI_SelectNextItem:		// Select next usable inventory item.
			if(cc != CONTROL_Client && !exitMountOnly)
				SelectNextItem();
			break;
		case CI_SelectPreviousItem:	// Select previous usable inventory item.
			if(cc != CONTROL_Client && !exitMountOnly)
				SelectPreviousItem();
			break;
		case CI_ThrowItem:
			if(cc != CONTROL_Client && !exitMountOnly)
				ThrowCurrentItem();
			break;
		case CI_DropItem:
			if(cc != CONTROL_Client && !exitMountOnly)
				DropCurrentItem();
			break;
		case CI_UseItem:
			if(cc != CONTROL_Client && !exitMountOnly)
				UseItemOnTarget(Id(), 0, AuPoint(0,0), false);			// Use the current item on myself.
			break;
		case CI_FireItem:
			if(exitMountOnly)
				break;
			if(cc == CONTROL_Client || cc == CONTROL_Solo)
			{
				// "firing" a flag at the terrain tries to claim the world. On the client, this can trigger the display of a UI to 
				// create a new empire.
				if(!EmpireId() && !nuPose.IsTurret() && !InDesignEnvironment())
				{
					// I have no empire; I am not sitting at a turret gun; and I am not in a design environment.
					AuGearPos gearPos(CurrentItemPos());
					INVENTORYSUB isub = CurrentItemSub();

					// Limit rate-of-fire of weapons.
					m_PersonLock.lockForRead();
					bool isNull = !m_tmLastWeaponFired.isValid();
					qint64 millis = m_tmLastWeaponFired.elapsed();
					m_PersonLock.unlock();

					// Person fired/wielded with or without a weapon.
					if(gearPos.IsGear() && isub == IPS_Item)
					{
						// Check the "current" item to see if it is valid and that it is not a weapon.
						m_GearLock.lockForRead();
						const AuItem* item = m_Gear.ItemAt(gearPos);
						if(item && !item->IsWeapon(isub))
						{
							// A non-weapon item in hand can only be "fired" once per second.
							COMMODITYID cid = item->CommodityId();
							m_GearLock.unlock();
							if(cid == CID_Flag)
							{
								if(isNull || millis > 1000)
								{
									m_PersonLock.lockForWrite();
									m_tmLastWeaponFired.restart();
									m_PersonLock.unlock();

									// Check target.
									// The target might likely be myself or my parent spacecraft.
									DaAtom* pTarget = 0;
									if(!Is(ci.TargetId()))
									{
										if(ParentId() == ci.TargetId())
											pTarget = Parent();
										else
										{
											DaAtom* system = FindEnvironmentAtom();
											pTarget = system->FindIdRecursive(ci.TargetId());
										}
										const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(pTarget);
										if(llworld)
											emit CreateEmpire();
									}
								}
							}
						}
						else
							m_GearLock.unlock();
					}
				}

				// Client does no more with weapon firing.
				if(cc == CONTROL_Client)
					break;
			}

			if(nuPose.IsTurret())
			{
				// Fire gun if I'm sitting in a turret.
				if(ship)
				{
					// Immediately get values for statistics in case this shot from the turret destroys this capsule.
					quint8 turretQ = ship->SystemQuality(SYS_Turret, StationId());

					// Tell the capsule to fire the turret.
					if(ship->FireTurret(*this, false, AuId(0)))
					{
						// Turret fired.
						m_PersonLock.lockForWrite();
						m_tmLastWeaponFired.restart();
						m_PersonLock.unlock();

						// Player Statistics
						Stats().ItemUsedAsWeapon(CID_ShipTurret, turretQ);
						IncreaseSkill(AuSkill::Turret, 1);
					}
				}
				else if(bd)
				{
					//DHH Person fire turret at a bd.
				}
			}
			else
			{
				// I am not sitting at a turret gun.
				AuGearPos gearPos(CurrentItemPos());
				INVENTORYSUB isub = CurrentItemSub();

				// Limit rate-of-fire of weapons.
				m_PersonLock.lockForRead();
				bool isNull = !m_tmLastWeaponFired.isValid();
				qint64 millis = m_tmLastWeaponFired.elapsed();
				m_PersonLock.unlock();

				// Person fired/wielded with or without a weapon.
				if(gearPos.IsGear() && isub == IPS_Item)
				{
					// Check the "current" item to see if it is valid and that it is not a weapon.
					m_GearLock.lockForRead();
					const AuItem* item = m_Gear.ItemAt(gearPos);
					if(item && !item->IsWeapon(isub))
					{
						// A non-weapon item in hand can only be "fired" once per second.
						COMMODITYID cid = item->CommodityId();
						m_GearLock.unlock();

						double okRange;
						if(cid == CID_Flag)
							okRange = 10*FeetPerMeter();
						else
							okRange = 7;
						if((isNull || millis > 1000) && ci.TargetRange(CameraOffset()) < okRange)
						{
							m_PersonLock.lockForWrite();
							m_tmLastWeaponFired.restart();
							m_PersonLock.unlock();

							// The item in hand is valid and it is not a weapon.
							// Apply its non-weapon "IPS_Item" primary usage to the target.
							// The scene atom "uses" the item in hand.
							// This will consume "supplies" if applicable.
							UseItemEffect(cid, WM_Contact, WE_None, 0, isub, AuPoint(0,0));
							UseItemOnTarget(ci.TargetId(), ci.TargetPart(), ci.TargetLoc(), false);
						}
						break;
					}
					else
						m_GearLock.unlock();
				}

				// Ignore special body weapon if it is not ready.
				if(gearPos.IsBody() && isub != IPS_Item && !SpecialWeaponReady())
					break;	// Special weapon is not ready to be used.

				// Try using the current item as a weapon.
				// Weapon 1 effect will only be WE_None if the weapon in hand needs ammo and it is empty;
				// all other cases will produce some weapon effect, even if it means striking with my empty hands.
				AuCombatWeapon weapon(WM_Not);
				int ammoCur, ammoFull;
				WeaponAtUsePos(gearPos, isub, weapon, ammoCur, ammoFull);

				// Limit rate-of-fire of current item usage as weapon.
				// weapon.m_CID will be < CID_Invalid if the weapon is my own body.
				int msfire;	// millisecs between shots
				switch(weapon.m_CID)
				{
				case CID_Invalid:
				default:
					msfire = (int)(CycleSpeed()*1000);
					break;
				case CID_Pistol:
					if(weapon.m_Mode == WM_Contact)
						msfire = 500;
					else
						msfire = 400;
					break;
				case CID_LaserPistol:
					if(weapon.m_Mode == WM_Contact)
						msfire = 500;
					else
						msfire = 650;
					break;
				case CID_AK47:
				case CID_Rifle:
					if(weapon.m_Mode == WM_Contact)
						msfire = 800;
					else
						msfire = 180;
					break;
				case CID_Shotgun:
					if(weapon.m_Mode == WM_Contact)
						msfire = 700;
					else
						msfire = 600;
					break;
				case CID_LaserRifle:
					if(weapon.m_Mode == WM_Contact)
						msfire = 800;
					else
						msfire = 750;
					break;
				}
				m_PersonLock.lockForRead();
				bool f = millis < msfire && m_tmLastWeaponFired.isValid();
				m_PersonLock.unlock();
				if(f)
					break;
				m_PersonLock.lockForWrite();
				m_tmLastWeaponFired.restart();
				m_PersonLock.unlock();

				// The scene atom "uses" the weapon in hand.
				// This will consume ammo if applicable.
				// This may cause the weapon to break. If so, the weapon functions but this is the last time.
				// weapon.m_CID will be < CID_Invalid if the weapon is my own body.
				// wEffect1 will be WE_None if the item in hand is not a weapon.
				// 'ammoFull' will be non-zero if the weapon needs ammo to function.
				// 'ammoCur' will be non-zero if the weapon has ammo.
				// Create a special effect of the weapon functioning.
				if(weapon.m_w1.m_Effect != WE_None &&
						(weapon.m_CID < CID_Invalid || UseGearItem(gearPos, isub)))
				{
					// The weapon functioned.
					if(!ammoFull || ammoCur)
					{
						// Weapon does not use ammo or it has ammo.
						// Determine a target location relative to this atom, in the direction the weapon is facing, for firing
						// effect.
						AuPoint targetLoc;
						if(ci.TargetLoc().IsZero() && weapon.m_Mode == WM_Projectile && weapon.m_Range > 0)
							targetLoc = WeaponMuzzleLoc() + WeaponMuzzleVec() * weapon.m_Range;
						else
							targetLoc = ci.TargetLoc();
						UseItemEffect(weapon.m_CID, weapon.m_Mode, weapon.m_w1.m_Effect, weapon.MaxRadius(),
								isub, targetLoc);
					}

					// Player Statistics
					Stats().ItemUsedAsWeapon(weapon.m_CID, weapon.m_Quality);
					IncreaseSkill(AuSkill::Combat, 1);

					if(weapon.m_Mode == WM_Detonator || weapon.m_Mode == WM_Fuse)
					{
						// Detonator and fuse weapons must be placed on the target within a short range when aiming/shooting.
						// Otherwise we just hang on to them and do nothing. They can be dropped armed by 'U'sing them.
						if(!ci.TargetId().IsNull() && ci.TargetLoc().Length() < 5)
						{
							// Try to stick the weapon to the target.
							// Can't stick weapons to creatures or other items.
							DaAtom* system = FindEnvironmentAtom();
							Q_ASSERT(system);
							DaAtom* target = system->FindIdRecursive(ci.TargetId());
							if(target && !dynamic_cast<DaCreature*>(target) && !dynamic_cast<DaItem*>(target))
							{
								// Drop the weapon and arm it.
								DaItem* item = DropGearAt(gearPos);
								if(item)
								{
									// Target location is close enough to stick the weapon to it and it's not a creature.
									// Find a common parent atom, which will at least be the solar system.
									DaAtom* commonParent = CommonParent(target, 0);
									if(!commonParent)
										break;
									AuMatrix4x4 mycp;
									GetTransformRelativeToParent(mycp, commonParent);
									AuMatrix4x4 targcp;
									target->GetTransformRelativeToParent(targcp, commonParent);
									AuPoint myLocCP = mycp.Project(CameraOffset());			// my eye loc rel to common parent
									AuPoint targLocCP = mycp.Project(ci.TargetLoc());		// target loc rel to common parent
									AuPoint myLoc = targcp.Inverse().Project(myLocCP);		// my eye loc rel to target atom
									AuPoint targLoc = targcp.Inverse().Project(targLocCP);	// target loc rel to target atom
									AuPoint vecToMe = myLoc - targLoc;						// vector pointing to me to orient item
									vecToMe.Unit();

									// Position the weapon on the target.
									AuMatrix4x4 obRot = item->GroundUpVector().GetAlignmentTransform(vecToMe);
									AuMatrix4x4 tItem;
									tItem.Translate(targLoc);
									tItem *= obRot;
									item->ChangePosAndOrient(AuAtomPos(target->Id(), tItem));
									item->InvalidateEnvironment();

									// Arm the weapon.
									item->ArmWeapon(Id(), Name(), EmpireId());	// Start fuse or wait for detonator signal.
									item->SetAtRest();
									item->SetModified();

									if(weapon.m_Mode == WM_Detonator)
									{
										QString text(tr("%1 armed for detonation.").arg(item->Name()));
										Thought(text);
									}
									else if(weapon.m_Mode == WM_Fuse)
									{
										QString text(tr("%1 fuse initiated.").arg(item->Name()));
										Thought(text);
									}
								}
							}
						}
					}
					else if(weapon.m_Mode == WM_Area)
					{
						// Perform area attack that excludes me and others of my own kind.
						AuCombatAttacker attacker(Id(), Name(), EmpireId());
						if(weapon.m_CID < CID_Invalid)
						{
							if(mount)
								attacker.SetSpeciesExempt(Species(), mount->Species());	// Mount species is also exempt.
							else
								attacker.SetSpeciesExempt(Species());	// My DNA is exempt from my own area attack.
						}

						AuCombatDefender defender;

						AuCombatResult result;

						AttackTarget(attacker, defender, weapon, true, result);
					}
					else if(!ci.TargetId().IsNull())
					{
						// Was targetted on an atom.
						DaAtom* target = 0;
						DaAtom* system = FindEnvironmentAtom();
						if(system)
							target = system->FindIdRecursive(ci.TargetId());
						if(target && !target->IsNoncombatant())
						{
							// Ready to attack the target.
							AuCombatAttacker attacker(Id(), Name(), EmpireId());
							if(IsInVehicle())
							{
								const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
								if(vehicle)
									attacker.m_Name = tr("%1 in %2").arg(Name()).arg(vehicle->Name());
							}
							else if(IsOnVehicle())
							{
								const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
								if(vehicle)
									attacker.m_Name = tr("%1 on %2").arg(Name()).arg(vehicle->Name());
							}
							else if(IsInSpacecraft())
							{
								const DaVessel* ship = FindParentVesselInEnv();
								if(ship)
									attacker.m_Name = tr("%1 aboard %2").arg(Name()).arg(ship->NameWithEmpire());
							}
							else if(IsInBd())
							{
								const DaBd* bd = FindParentBdInEnv();
								if(bd)
									attacker.m_Name = tr("%1 in %2").arg(Name()).arg(bd->CityBuildingName());
							}

							AuCombatDefender defender(target, ci.TargetLoc(), ci.TargetShield(), ci.TargetLoc().Length());

							AuCombatResult result;

							AttackTarget(attacker, defender, weapon, true, result);
						}
					}
					
					// Notify body the special weapon was used so we can't use it again until its ready.
					if(gearPos.IsBody() && isub != IPS_Item)
						UseSpecialWeapon();
				}
				else if(weapon.m_w1.m_Effect == WE_None)
				{
					// Here if the weapon in hand reports no damage effect.
					// We've already culled out non-weapons so this must be an
					// out-of-ammo situation.
					//qDebug("DaPerson::ProcessControlInput %d fired weapon with no ammo", Id().GetID());
				}
				else
				{
					// Here if the weapon in hand reports a damage effect but
					// the weapon is not a body weapon and it failed the
					// UseItemInHand call. Uh, oh...
					qWarning("DaPerson::ProcessControlInput %d fired weapon %d with effect. This should never happen.",
							Id().GetID(), weapon.m_CID);
				}
			}
			break;
		case CI_FireItemAlt:
			if(cc == CONTROL_Client || exitMountOnly)
				break;
			if(!IsLockedPose())
			{
				// I am not sitting at a console.
				AuGearPos gearPos(CurrentItemPos());
				INVENTORYSUB isub = CurrentItemSub();

				// Limit rate-of-fire of weapons.
				m_PersonLock.lockForRead();
				bool isNull = !m_tmLastWeaponFired.isValid();
				qint64 millis = m_tmLastWeaponFired.elapsed();
				m_PersonLock.unlock();

				// Person fired/wielded with or without a weapon.
				if(gearPos.IsGear() && isub == IPS_Item)
				{
					// Check the "current" item to see if it is valid and that it is not a weapon.
					m_GearLock.lockForRead();
					const AuItem* gearItem = m_Gear.ItemAt(gearPos);
					if(gearItem && !gearItem->IsWeapon(isub))
					{
						// A non-weapon item in hand can only be "fired" once per second.
						COMMODITYID c = gearItem->CommodityId();
						m_GearLock.unlock();

						if((isNull || millis > 1000) && ci.TargetRange(CameraOffset()) < 7)
						{
							m_PersonLock.lockForWrite();
							m_tmLastWeaponFired.restart();
							m_PersonLock.unlock();

							// The item in hand is valid and it is not a weapon.
							// Apply its non-weapon "IPS_Item" alternate usage to the target.
							// The scene atom "uses" the item in hand.
							// This will consume "supplies" if applicable.
							UseItemEffect(c, WM_Contact, WE_None, 0, isub, AuPoint(0,0));
							UseItemOnTarget(ci.TargetId(), ci.TargetPart(), ci.TargetLoc(), true);
						}
						break;
					}
					else
						m_GearLock.unlock();
				}
			}
			break;

		case CI_Run:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			runMod = true;
			//qDebug("Run");
			break;
		case CI_LookUp:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			/*
			if(m_uiActions & ACT_LookUp)
			{
				int nuangle = m_byRotSpeedX + ci.GetRangeValue();
				if(nuangle > 0xffff)
					m_byRotSpeedX = 0xffff;
				else
					m_byRotSpeedX = nuangle;
			}
			else
			*/
				m_byRotSpeedX = ci.GetRangeValue();
			m_uiActions &= ~ACT_LookMask;
			m_uiActions |= ACT_LookUp;
			m_PersonLock.unlock();
			//qDebug() << "Look Up" << m_byRotSpeedX << m_byRotSpeedX/300.f << "degrees";
			break;
		case CI_LookDown:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			/*
			if(m_uiActions & ACT_LookDown)
			{
				int nuangle = m_byRotSpeedX + ci.GetRangeValue();
				if(nuangle > 0xffff)
					m_byRotSpeedX = 0xffff;
				else
					m_byRotSpeedX = nuangle;
			}
			else
			*/
				m_byRotSpeedX = ci.GetRangeValue();
			m_uiActions &= ~ACT_LookMask;
			m_uiActions |= ACT_LookDown;
			m_PersonLock.unlock();
			//qDebug() << "Look Down" << m_byRotSpeedX << m_byRotSpeedX/300.f << "degrees";
			break;
		case CI_MoveForward:
			keepRunning = false;	// Needs run mod to run.
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			m_tmLastMoveControl.restart();
			m_PersonLock.unlock();
			fwdback = ACT_MoveForward;
			//qDebug() << "Move Ahead" << IsMoving();
			break;
		case CI_MoveForwardContinuously:
			keepRunning = false;	// Needs run mod to run.
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			m_tmLastMoveControl.restart();
			m_PersonLock.unlock();
			continuousMove = true;
			//qDebug("Move Ahead");
			break;
		case CI_MoveBackward:
			keepRunning = false;	// Cannot run backward.
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			if(!isFlyer)
			{
				m_PersonLock.lockForWrite();
				m_tmLastMoveControl.restart();
				m_PersonLock.unlock();
				fwdback = ACT_MoveBackward;
				//qDebug("Move Back");
			}
			break;
		case CI_MoveLeft:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			m_tmLastMoveLControl.restart();
			lateral = ACT_MoveLeft;
			if(mount)
				m_byRotSpeedZ = ci.GetRangeValue();
			m_PersonLock.unlock();
			//qDebug("Move Left");
			break;
		case CI_MoveRight:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			m_tmLastMoveLControl.restart();
			lateral = ACT_MoveRight;
			if(mount)
				m_byRotSpeedZ = ci.GetRangeValue();
			m_PersonLock.unlock();
			//qDebug("Move Right");
			break;
		case CI_MoveDown:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			m_tmLastMoveVControl.restart();
			m_PersonLock.unlock();
			vertical = ACT_MoveDown;
			break;
		case CI_MoveUp:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			m_PersonLock.lockForWrite();
			m_tmLastMoveVControl.restart();
			m_PersonLock.unlock();
			vertical = ACT_MoveUp;
			break;
		case CI_TurnLeft:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			if(IsInBerth())
			{
				// Sleeping pose can't change too fast.
				m_PersonLock.lockForWrite();
				if(!m_tmLastMoveSControl.isValid() || m_tmLastMoveSControl.elapsed()/d_MSecPerSec >= 1)
				{
					// Change to next left berth pose.
					// This seems backward but I'm just going with it rather than go fix backward left/right body pose bugs.
					m_tmLastMoveSControl.restart();					// turn time
					ChangePosAndOrient(Pos());
					InvalidateEnvironment();
					switch(CurrentGeneralPose().Pose())
					{
					case AuPose::BerthBottomFaceDown:
						nuPose = AuPose::BerthBottomFaceLeft;
						break;
					default:
					case AuPose::BerthBottomFaceLeft:
						nuPose = AuPose::BerthBottomFaceUp;
						break;
					case AuPose::BerthBottomFaceRight:
						nuPose = AuPose::BerthBottomFaceDown;
						break;
					case AuPose::BerthBottomFaceUp:
						nuPose = AuPose::BerthBottomFaceRight;
						break;
					case AuPose::BerthTopFaceDown:
						nuPose = AuPose::BerthTopFaceLeft;
						break;
					case AuPose::BerthTopFaceLeft:
						nuPose = AuPose::BerthTopFaceUp;
						break;
					case AuPose::BerthTopFaceRight:
						nuPose = AuPose::BerthTopFaceDown;
						break;
					case AuPose::BerthTopFaceUp:
						nuPose = AuPose::BerthTopFaceRight;
						break;
					}
				}
				m_PersonLock.unlock();
			}
			else
			{
				//qDebug() << "Turn Left range" << m_byRotSpeedZ << ci.GetRangeValue();
				m_PersonLock.lockForWrite();
				m_tmLastMoveSControl.restart();		// turn time
				m_byRotSpeedZ = ci.GetRangeValue();
				m_uiActions &= ~ACT_TurnMask;
				m_uiActions |= ACT_TurnLeft;
				m_PersonLock.unlock();
			}
			break;
		case CI_TurnRight:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;

			if(IsInBerth())
			{
				// Sleeping pose can't change too fast.
				m_PersonLock.lockForWrite();
				if(!m_tmLastMoveSControl.isValid() || m_tmLastMoveSControl.elapsed()/d_MSecPerSec >= 1)
				{
					// Change to next right berth pose.
					// This seems backward but I'm just going with it rather than go fix backward left/right body pose bugs.
					m_tmLastMoveSControl.restart();					// turn time
					ChangePosAndOrient(Pos());
					InvalidateEnvironment();
					switch(CurrentGeneralPose().Pose())
					{
					case AuPose::BerthBottomFaceDown:
						nuPose = AuPose::BerthBottomFaceRight;
						break;
					case AuPose::BerthBottomFaceLeft:
						nuPose = AuPose::BerthBottomFaceDown;
						break;
					default:
					case AuPose::BerthBottomFaceRight:
						nuPose = AuPose::BerthBottomFaceUp;
						break;
					case AuPose::BerthBottomFaceUp:
						nuPose = AuPose::BerthBottomFaceLeft;
						break;
					case AuPose::BerthTopFaceDown:
						nuPose = AuPose::BerthTopFaceRight;
						break;
					case AuPose::BerthTopFaceLeft:
						nuPose = AuPose::BerthTopFaceDown;
						break;
					case AuPose::BerthTopFaceRight:
						nuPose = AuPose::BerthTopFaceUp;
						break;
					case AuPose::BerthTopFaceUp:
						nuPose = AuPose::BerthTopFaceLeft;
						break;
					}
				}
				m_PersonLock.unlock();
			}
			else
			{
				//qDebug() << "Turn Right range" << m_byRotSpeedZ << ci.GetRangeValue();
				m_PersonLock.lockForWrite();
				m_tmLastMoveSControl.restart();		// turn time
				m_byRotSpeedZ = ci.GetRangeValue();
				m_uiActions &= ~ACT_TurnMask;
				m_uiActions |= ACT_TurnRight;
				m_PersonLock.unlock();
			}
			break;
		case CI_SelectMapPos0:		// Set current use pos and sub to mapped pos index 0
		case CI_SelectMapPos1:
		case CI_SelectMapPos2:
		case CI_SelectMapPos3:
		case CI_SelectMapPos4:
		case CI_SelectMapPos5:
		case CI_SelectMapPos6:
		case CI_SelectMapPos7:
		case CI_SelectMapPos8:
		case CI_SelectMapPos9:
			if(exitMountOnly)
				break;
			if(cc != CONTROL_Client)
				SelectMappedGearItem(ci.GetId() - CI_SelectMapPos0);
			break;

		case CI_MapPos0:
		case CI_MapPos1:
		case CI_MapPos2:
		case CI_MapPos3:
		case CI_MapPos4:
		case CI_MapPos5:
		case CI_MapPos6:
		case CI_MapPos7:
		case CI_MapPos8:
		case CI_MapPos9:
			if(exitMountOnly)
				break;
			if(cc != CONTROL_Client)
				MapCurrentGearItem(ci.GetId() - CI_MapPos0);
			break;

		case CI_ConsoleCommand:		// Switch command console to command mode if sitting at a command console.
			if(exitMountOnly)
				break;

			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();					// Save console settings on the client.

			if(IsConsolePose())
			{
				// I am sitting at a console.
				// At a Captain's chair, I can switch modes if the ship has an engineer console that is manned.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()) &&
						ship->IsConsoleOperational(StationSysId()))
				{
					if(nuPose.IsHelmStation())
						ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.
					if(ConsoleIsIntegrated())
						SitAtConsole(StationId(), AuStation::Integrated_Command, ConsoleOccupantPosition(), 0, 0);
					else
						SitAtConsole(StationId(), AuStation::Console_Command, ConsoleOccupantPosition(), 0, 0);
					SetModified();
					nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
					nuStationId = StationId();
					nuOccPos = ConsoleOccupantPosition();

					if(cc != CONTROL_Server)
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SwitchStation, 1,1);
						sceneAtom->AddEffect(buttonClick);
					}
				}
			}
			break;
		case CI_ConsoleEngineer:		// Switch command console to engineer mode if sitting at a command console.
			if(exitMountOnly)
				break;

			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();					// Save console settings on the client.

			if(IsConsolePose())
			{
				// I am sitting at a console.
				// At a Captain's chair, I can switch modes if the ship has an engineer console that is manned.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()) && ship->IsEngineerManned() &&
						ship->IsConsoleOperational(StationSysId()))
				{
					// Cannot switch to console if ship is enemy to me.
					POLITICAL_STANCE stance;
					if(IsEthereal())
						stance = POL_Neutral;
					else
						stance = ship->PoliticalStanceMakeContact(this);
					if(stance != POL_Enemy)
					{
						if(nuPose.IsHelmStation())
							ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.

						quint16 which = ship->FTLWarpDrive();
						if(!which)
							which = ship->FTLWormholeDrive();

						if(ConsoleIsIntegrated())
							SitAtConsole(StationId(), AuStation::Integrated_Engine, ConsoleOccupantPosition(), 0, which);
						else
							SitAtConsole(StationId(), AuStation::Console_Engine, ConsoleOccupantPosition(), 0, which);
						SetModified();
						nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();

						if(cc != CONTROL_Server)
						{
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SwitchStation, 1,1);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;
		case CI_ConsoleNavigator:		// Switch command console to navigator mode if sitting at a command console.
			if(exitMountOnly)
				break;

			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();					// Save console settings on the client.

			if(IsConsolePose())
			{
				// I am sitting at a console.
				// At a Captain's chair, I can switch modes if the ship has a navigator console that is manned.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()) && ship->IsNavigatorManned()&&
						ship->IsConsoleOperational(StationSysId()))
				{
					// Cannot switch to console if ship is enemy to me.
					POLITICAL_STANCE stance;
					if(IsEthereal())
						stance = POL_Neutral;
					else
						stance = ship->PoliticalStanceMakeContact(this);
					if(stance != POL_Enemy)
					{
						if(nuPose.IsHelmStation())
							ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.
						if(ConsoleIsIntegrated())
							SitAtConsole(StationId(), AuStation::Integrated_Navigation, ConsoleOccupantPosition(), 0, 0);
						else
							SitAtConsole(StationId(), AuStation::Console_Navigation, ConsoleOccupantPosition(), 0, 0);
						SetModified();
						nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();

						if(cc != CONTROL_Server)
						{
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SwitchStation, 1,1);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;
		case CI_ConsolePilot:			// Switch command console to pilot mode if sitting at a command console.
			if(exitMountOnly)
				break;

			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();						// Save console settings on the client.

			if(IsConsolePose())
			{
				// I am sitting at a console.
				// At a Captain's chair, I can switch modes if the ship has a pilot console that is manned.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()) && ship->IsPilotManned() &&
						ship->IsConsoleOperational(StationSysId()))
				{
					// Cannot switch to console if ship is enemy to me.
					POLITICAL_STANCE stance;
					if(IsEthereal())
						stance = POL_Neutral;
					else
						stance = ship->PoliticalStanceMakeContact(this);
					if(stance != POL_Enemy)
					{
						if(nuPose.IsHelmStation())
							ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.

						quint16 which;
						if(ship->HasWarpFTLDrive())
							which = ship->FTLWarpDrive();
						else if(ship->HasWormholeFTLDrive())
							which = ship->FTLWormholeDrive();
						else
							which = 0;

						if(ConsoleIsIntegrated())
							SitAtConsole(StationId(), AuStation::Integrated_Helm, ConsoleOccupantPosition(), 0, which);
						else
							SitAtConsole(StationId(), AuStation::Console_Helm, ConsoleOccupantPosition(), 0, which);
						SetModified();
						nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();

						if(cc != CONTROL_Server)
						{
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SwitchStation, 1,1);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;
		case CI_ConsoleSensor:			// Switch command console to sensor mode if sitting at a command console.
			if(exitMountOnly)
				break;

			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();					// Save console settings on the client.

			if(IsConsolePose())
			{
				// I am sitting at a console.
				// At a Captain's chair, I can switch modes if the ship has a sensor console that is manned.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()) && ship->IsSensorManned() &&
						ship->IsConsoleOperational(StationSysId()))
				{
					// Cannot switch to console if ship is enemy to me.
					POLITICAL_STANCE stance;
					if(IsEthereal())
						stance = POL_Neutral;
					else
						stance = ship->PoliticalStanceMakeContact(this);
					if(stance != POL_Enemy)
					{
						if(nuPose.IsHelmStation())
							ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.
						if(ConsoleIsIntegrated())
							SitAtConsole(StationId(), AuStation::Integrated_Sensor, ConsoleOccupantPosition(), 0, 0);
						else
							SitAtConsole(StationId(), AuStation::Console_Sensor, ConsoleOccupantPosition(), 0, 0);
						SetModified();
						nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();

						if(cc != CONTROL_Server)
						{
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SwitchStation, 1,1);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;
		case CI_ConsoleShield:			// Switch command console to shield mode if sitting at a command console.
			if(exitMountOnly)
				break;

			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();					// Save console settings on the client.

			if(IsConsolePose())
			{
				// I am sitting at a console.
				// At a Captain's chair, I can switch modes if the ship has a shield console that is manned.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()) && ship->IsShieldManned() &&
						ship->IsConsoleOperational(StationSysId()))
				{
					// Cannot switch to console if ship is enemy to me.
					POLITICAL_STANCE stance;
					if(IsEthereal())
						stance = POL_Neutral;
					else
						stance = ship->PoliticalStanceMakeContact(this);
					if(stance != POL_Enemy)
					{
						if(nuPose.IsHelmStation())
							ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.
						if(ConsoleIsIntegrated())
							SitAtConsole(StationId(), AuStation::Integrated_Shield, ConsoleOccupantPosition(), 0, 0);
						else
							SitAtConsole(StationId(), AuStation::Console_Shield, ConsoleOccupantPosition(), 0, 0);
						SetModified();
						nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();

						if(cc != CONTROL_Server)
						{
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SwitchStation, 1,1);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;
		case CI_ConsoleWeapon:			// Switch command console to weapon mode if sitting at a command console.
			if(exitMountOnly)
				break;

			if(cc != CONTROL_Server)
				StoreClientConsoleSettings();					// Save console settings on the client.

			if(IsConsolePose())
			{
				// I am sitting at a console.
				// At a Captain's chair, I can switch modes if the ship has a weapon console that is manned.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->InChainOfCommand(this) && ship->IsCommandConsole(StationId()) && ship->IsWeaponManned() &&
						ship->HasWeaponBay() && ship->IsConsoleOperational(StationSysId()))
				{
					// Cannot switch to console if ship is enemy to me.
					POLITICAL_STANCE stance;
					if(IsEthereal())
						stance = POL_Neutral;
					else
						stance = ship->PoliticalStanceMakeContact(this);
					if(stance != POL_Enemy)
					{
						if(nuPose.IsHelmStation())
							ship->InvalidatePilotPerson();			// Invalidate cached person pilot so AI will take over quickly.

						QList<quint16> bayIds;
						ship->WeaponBayIds(bayIds);
						quint16 which = bayIds.first();

						if(ConsoleIsIntegrated())
							SitAtConsole(StationId(), AuStation::Integrated_FireControl, ConsoleOccupantPosition(), 0,
									which);
						else
							SitAtConsole(StationId(), AuStation::Console_FireControl, ConsoleOccupantPosition(), 0, which);
						SetModified();
						nuPose = CurrentGeneralPose();	// Prevent pose changing code below from changing my pose.
						nuStationId = StationId();
						nuOccPos = ConsoleOccupantPosition();

						if(cc != CONTROL_Server)
						{
							AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SwitchStation, 1,1);
							sceneAtom->AddEffect(buttonClick);
						}
					}
				}
			}
			break;
		case CI_ConsoleRaise:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;
			if(ConsoleIsIntegrated())
				break;
			DpConsole::Raise(Id());
			break;
		case CI_ConsoleLower:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;
			if(ConsoleIsIntegrated())
				break;
			DpConsole::Lower(Id());
			break;
		case CI_ConsoleMoveCloser:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;
			if(ConsoleIsIntegrated())
				break;
			DpConsole::MoveCloser(Id());
			break;
		case CI_ConsoleMoveFarther:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;
			if(ConsoleIsIntegrated())
				break;
			DpConsole::MoveFarther(Id());
			break;
		case CI_ConsoleRotateAway:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;
			if(ConsoleIsIntegrated())
				break;
			DpConsole::RotateAway(Id());
			break;
		case CI_ConsoleRotateToward:
			if(cc == CONTROL_Server)
				break;
			if(exitMountOnly)
				break;
			if(ConsoleIsIntegrated())
				break;
			DpConsole::RotateToward(Id());
			break;
		}
	}

	if(exitMountOnly)
		return;

	// Remaining actions relate to posing my body. Body posing is performed on the client.
	if(cc == CONTROL_Server)
		return;

	// Client benefits from applying rotation now, for smoother response.
	if(msElapsed)
		ApplyMovement(msElapsed);

	// Prevent some actions from dropping my run to a walk.
	if(!runMod && keepRunning)
	{
		m_uiActions |= ACT_Run;
		runMod = true;
	}

	// Continuous movement is like a toggle, unless also switching from run to walk or vice versa.
	// Don't change state of continuous bit if run modifier has not changed.
	unsigned int acti = Actions();
	if(continuousMove)
	{
		if(runMod == isRunning)
		{
			// Run modifier hasn't changed so toggle continuous movement action.
			if(!(acti & ACT_Continuous))
				fwdback |= ACT_MoveForward|ACT_Continuous;
		}
		else
		{
			// Run modifier has changed while continuous key held.
			fwdback |= ACT_MoveForward|ACT_Continuous;
		}
	}

	// Assume appropriate pose based on environment.
	// Also manipulates some action bits that may be needed on the client.
	AuPose curPose = poseWho->CurrentGeneralPose();
	quint16 curStationId = poseWho->StationId();
	AuStation::OCCUPANT_POSITION curOccPos = poseWho->ConsoleOccupantPosition();
	if(vehicle)
	{
		// Test validity of nu pose in seat position.
		if(curPose != nuPose && !vehicle->IsValidSeatPose(VehicleSeatIndex(), nuPose))
		{
			nuPose = curPose;									// New pose was not valid so no change.
			nuStationId = curStationId;
			nuOccPos = curOccPos;
		}

		// Don't move inside vehicle.
		m_PersonLock.lockForWrite();
		m_uiActions &= ~ACT_MoveMask;
		m_PersonLock.unlock();
	}
	else
	{
		// Combine movement flags.
		if(fwdback || lateral || vertical)
		{
			m_PersonLock.lockForWrite();
			if(fwdback)
			{
				m_uiActions &= ~(ACT_MoveForward|ACT_MoveBackward|ACT_Continuous);
				m_uiActions |= fwdback;
			}
			if(lateral)
			{
				m_uiActions &= ~(ACT_MoveLeft|ACT_MoveRight);
				m_uiActions |= lateral;
			}
			if(vertical)
			{
				m_uiActions &= ~(ACT_MoveUp|ACT_MoveDown);
				m_uiActions |= vertical;
			}

			// Can only run when moving forward, even combined with lateral movement.
			// Must have non-zero stamina to run.
			// Backward or lateral-only movement cannot be at run speed.
			if(((fwdback & ACT_MoveForward) || (lateral & (ACT_MoveLeft|ACT_MoveRight))) && runMod)
				m_uiActions |= ACT_Run;
			else
				m_uiActions &= ~ACT_Run;
			m_PersonLock.unlock();
		}

		// Posed creature must query its environment first.
		nuPose = SelectPoseForMeOrMount(nuPose, poseWho);
		nuStationId = poseWho->StationId();
		nuOccPos = poseWho->ConsoleOccupantPosition();
		//qDebug() << "DaPerson::ProcessControlInput" << Id() << Name() << "nuPose" << nuPose;
	}

	// Body pose may not have changed at all.
	// Test for eating pose because general pose for eating is standing for many animals. When mounting an animal that is eating,
	// it must change its pose to standing, which affects its insertion point.
	if(curPose == nuPose && !poseWho->IsEatingPose())
		return;
	
	// Body pose is not changed while dying.
	if(poseWho->IsBodyDyingOrDead())
		return;

	// Body pose is not changed while riding a mount. My body pose was set when I mounted up. It does not change until I dismount.
	if(poseWho == this && dynamic_cast<DaCreature*>(poseWho->Parent()))
		return;

	// Most pose changes need to kick my body into motion to make sure I adjust due to gravity.
	if(!poseWho->IsLockedPose() &&
			!dynamic_cast<DaVehicle*>(poseWho->Parent()))
	{
		poseWho->SetInMotion();
		poseWho->TickTock();
	}

	// Change my body pose.
	const double rCurHt = poseWho->GroundHeightAboveCG();	// Current CG ht
	if(poseWho->SetGeneralPose(nuPose, nuStationId, nuOccPos))
	{
		// Pose changed; check for collisions in nu position.
		const double rNewHt = poseWho->GroundHeightAboveCG();
		AuPoint moveVec(0, 0, rNewHt-rCurHt);
		if(vehicle)
		{
			// In a vehicle so move to seat location without collision testing.
			AuMatrix4x4 t;
			t.Translate(moveVec);
			AuAtomPos pos(poseWho->Pos());
			pos.TransformBy(t);
			poseWho->ChangePos(pos);
			poseWho->InvalidateEnvironment();
			poseWho->SetModified();
		}
		else
		{
			// Not in a vehicle.
			// Always permit pose changes in weightlessness.
			bool weightLess = !poseWho->IsInGravity() || (poseWho->IsInWater() && poseWho->IsCGUnderWater());

			// Always permit going to prone pose.
			// Otherwise must test for obstructions when changing pose using Move.
			//DHH Use a better test for collision without using "Move".
			if(weightLess || poseWho->IsProne() || poseWho->Move(moveVec, 0))
			{
				// Move to nu position was unobstructed.
				poseWho->InvalidateEnvironment();
				poseWho->SetModified();
				if(curPose == AuPose::MateOnBottom || nuPose == AuPose::ReceptiveMate)
					DismountAllRiders();	// Drop riders when I stop f'ing them.

				// Most pose changes need to kick my body into motion to make sure I adjust due to gravity.
				if(!poseWho->IsLockedPose())
				{
					poseWho->SetInMotion();
					poseWho->TickTock();
				}
			}
			else
			{
				// No room to change to the nu pose.
				// Restore previous pose.
				poseWho->SetGeneralPose(curPose, false);
			}
		}
	}
}

// Sets effect1 and effect2 to WE_None if the equipped weapon needs ammo but
// is out of ammo.
// When an object is used, 'loc' is set to WL_None and 'cid' is set to the
// commodity id of the item.
// When a physical body location is used, 'loc' is set to the weapon location
// and 'cid' is set to CID_Invalid.
// Returns true if the body special weapon is used.
bool DaPerson::BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
		int& ammoFull) const
{
	// Get info about physical body weapon; there is always a physical body weapon even if its my teeth.
	// This provides the base range provided by the physical body.
	// They physical body range is added to the weapon range.
	bool useSpecWpn = DaSentient::BestWeaponAtRange(targetRange, loc, weapon, ammoCur, ammoFull);

	// Check inventory item in use for weapon capability.
	AuGearPos gearPos(CurrentItemPos());
	if(!gearPos.IsGear())
		return useSpecWpn;										// No weapon in use. Use physical body weapon.

	INVENTORYSUB isub = CurrentItemSub();

	m_GearLock.lockForRead();
	const AuItem* gearItem = m_Gear.ItemAt(gearPos);
	if(!gearItem)
	{
		m_GearLock.unlock();
		return useSpecWpn;
	}

	if(gearItem->IsWeapon(isub))
	{
		// The person has a weapon in use.
		gearItem->WeaponInfo(isub, weapon.m_CID, weapon.m_Quality, weapon.m_Mode, weapon.m_Range, ammoCur, ammoFull,
				weapon.m_w1.m_Effect, weapon.m_w1.m_Damage, weapon.m_w1.m_DamageMod, weapon.m_w1.m_Radius,
				weapon.m_w1.m_AP, weapon.m_w1.m_ST,
				weapon.m_w2.m_Effect, weapon.m_w2.m_Damage, weapon.m_w2.m_DamageMod, weapon.m_w2.m_Radius,
				weapon.m_w2.m_AP, weapon.m_w2.m_ST);

		loc = DhDNA::WL_None;									// Body weapon was not employed.
		if(gearItem->CanFireWeapon(isub))
		{
			// When using contact weapons and projectile weapons, I can add a little range to what I can physically reach.
			if(weapon.m_Mode == WM_Contact || weapon.m_Mode == WM_Projectile)
			{
				// Add a little reach range to the range of projectile weapons. This is measured from my eye.
				weapon.m_Range += 6;

				// When using contact weapons, add a damage modifier due to my strength.
				if(weapon.m_Mode == WM_Contact)
				{
					// Melee weapons are affected by strength and strength increases damage.
					weapon.m_w1.m_DamageMod += StrengthMod();				// Strength mod increases base damage.
				}
			}
		}
		else
		{
			// The weapon can't be fired, most likely out of ammo.
			weapon.m_w1.m_Effect = WE_None;
			weapon.m_w2.m_Effect = WE_None;
		}
	}
	else
	{
		weapon.m_Mode = WM_Not;
		weapon.m_w1.m_Effect = WE_None;
		weapon.m_w2.m_Effect = WE_None;
	}
	m_GearLock.unlock();
	return false;												// Did not use body special weapon.
}

void DaPerson::WeaponAtUsePos(const AuGearPos& gearPos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const
{
	if(gearPos.IsBody())
	{
		// Body weapons are resolved by the base class.
		DaSentient::WeaponAtUsePos(gearPos, sub, weapon, ammoCur, ammoFull);
		return;
	}

	m_GearLock.lockForRead();
	const AuItem* gearItem = m_Gear.ItemAt(gearPos);
	if(gearItem)
	{
		// Get weapon info about an item in my inventory.
		gearItem->WeaponInfo(sub, weapon.m_CID, weapon.m_Quality, weapon.m_Mode, weapon.m_Range, ammoCur, ammoFull,
				weapon.m_w1.m_Effect, weapon.m_w1.m_Damage, weapon.m_w1.m_DamageMod, weapon.m_w1.m_Radius,
				weapon.m_w1.m_AP, weapon.m_w1.m_ST,
				weapon.m_w2.m_Effect, weapon.m_w2.m_Damage, weapon.m_w2.m_DamageMod, weapon.m_w2.m_Radius,
				weapon.m_w2.m_AP, weapon.m_w2.m_ST);

		if(gearItem->IsWeapon(sub))
		{
			// The person has a weapon in use.
			if(gearItem->CanFireWeapon(sub))
			{
				// When using contact weapons and projectile weapons, I can add a little range to what I can physically reach.
				if(weapon.m_Mode == WM_Contact || weapon.m_Mode == WM_Projectile)
				{
					// Add a little reach range to the range of projectile weapons. This is measured from my eye.
					weapon.m_Range += 6;

					// When using contact weapons, add a damage modifier due to my strength.
					if(weapon.m_Mode == WM_Contact)
					{
						// Melee weapons are affected by strength and strength increases damage.
						weapon.m_w1.m_DamageMod += StrengthMod();					// Strength mod increases base damage.
					}
				}
			}
			else
			{
				// The weapon can't be fired, most likely out of ammo.
				weapon.m_w1.m_Effect = WE_None;
				weapon.m_w2.m_Effect = WE_None;
			}
		}
	}
	else
	{
		// Not a body part and not a valid item in my inventory.
		// Set null values.
		weapon = AuCombatWeapon(WM_Not);
		ammoCur = 0;
		ammoFull = 0;
	}
	m_GearLock.unlock();
}

bool DaPerson::IsTicker() const
{
	if(DaSentient::IsTicker())
		return true;

	if(IsInvisibleOffLine())
		return false;

	if(HasConsole())
		return true;

	if(IsFlying() && IsTouchingGround())
		return true;

	return false;
}

void DaPerson::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	//if(Name() == "Lightspeed" && last.time().second() != now.time().second())
		//qDebug() << "DaPerson::Tick" << Id() << Name() << "state" << AtomState() << "parent" << Parent()->Id() << Parent()->Name()
			//<< "actions" << QString("0x%1").arg(m_uiActions, 0, 16);
	DaSentient::Tick(last, now);
	if(IsDead())
		return;

	// Don't do anything else if this person is not on line.
	// Demiavatars do not go off line.
	if(IsInvisibleOffLine())
		return;

	// Creature consoles must be ticked to stay in sync with ship systems.
	if(HasConsole())
	{
		DpStation* console = LockConsoleForRead();
		bool ct = console && console->Tick(last, now, this);
		UnlockConsole();
		if(ct)
			SetModified();
	}

	// Stop flying pose on the ground.
	if(IsFlying() && IsTouchingGround())
		Stand();
}

void DaPerson::TickMovement(const AuElapsedTimer& last)
{
	DaSentient::TickMovement(last);

	// On the client, TickMovement is only called on the scene atom and the atom controlled by the scene atom, which could only be
	// a vehicle, spacecraft or animal.
	qint64 msElapsed = last.elapsed();
	ApplyMovement(msElapsed);

	// Upate suit thruster state for client-side sound effects.
	// Entity does not make suit thruster noise.
	m_PersonLock.lockForWrite();
	m_uiActions &= ~ACT_SuitMask;	// Turn off the suit thruster and brake bits.
	unsigned int acti = m_uiActions;
	m_PersonLock.unlock();

	if(acti & (ACT_MoveMask|ACT_BrakeMask))
	{
		// I am thrusting or braking.
		if(!IsInGravity() &&
				!IsBodyDyingOrDead() &&
				!IsInSpacecraft() &&
				!IsInAtmosphere() &&
				!InDesignEnvironment())
		{
			// In space I can control my movement if I am wearing a space suit or if I am aboard a spacecraft.
			m_GearLock.lockForRead();
			const AuItem* item = m_Gear.ArmorItemWorn();
			bool hasSuit = item && item->IsEnvironmentSuit();
			m_GearLock.unlock();
			if(hasSuit || AgentEntity())
			{
				m_PersonLock.lockForWrite();
				if(m_uiActions & ACT_MoveMask)
					m_uiActions |= ACT_SuitThruster;
				if(m_uiActions & ACT_BrakeMask)
					m_uiActions |= ACT_SuitBrake;
				m_PersonLock.unlock();
			}
		}
	}

	DecayMoveActions();
}

void DaPerson::DecayMoveActions()
{
	// Test for all the kinds of movement that can decay.
	if(!(m_uiActions & (ACT_LookMask|ACT_MoveMask|ACT_TurnMask|ACT_Brake|ACT_Jump)))
		return;

	int controlDecayMs = ControlDecayMs();

	m_PersonLock.lockForWrite();

	// Shut off head tilt after a short time to stop tilting head.
	if(m_uiActions & ACT_LookMask)
	{
		if(!m_byRotSpeedX)
			m_uiActions &= ~ACT_LookMask;
	}

	// Shut off turn after a short time to stop rotating.
	if(m_uiActions & ACT_TurnMask)
	{
		if(!m_byRotSpeedZ)
			m_uiActions &= ~ACT_TurnMask;
	}

	// Shut off brakes after a short time to stop braking.
	if(m_uiActions & ACT_Brake)
	{
		qint64 millis = m_tmLastBrake.elapsed();
		if(!m_tmLastBrake.isValid() || millis > controlDecayMs)
		{
			m_uiActions &= ~ACT_Brake;

			// If hovering because my brakes are on, but I can't hover, I must now resume gliding.
			if(CurrentGeneralPose() == AuPose::Hover && !CanHover())
			{
				SetGeneralPose(AuPose::Glide);				// Resume gliding.
				TickTock();
			}
			else
			{
				// If my mount is hovering because my brakes are on, but it can't hover, the mount must now resume gliding.
				DaAnimal* mount = dynamic_cast<DaAnimal*>(Parent());	// We only control animals, not mounted people.
				if(mount && mount->CurrentGeneralPose() == AuPose::Hover && !mount->CanHover())
				{
					mount->SetGeneralPose(AuPose::Glide);		// Resume gliding.
					mount->TickTock();
				}
			}
		}
	}

	// Trigger jump
	if(m_uiActions & ACT_Jump)
	{
		// Boing!
		m_uiActions &= ~ACT_Jump;
		DaAnimal* mount = dynamic_cast<DaAnimal*>(Parent());	// We only control animals, not mounted people.
		if(mount)
		{
			if(mount->CanJump())
			{
				// Mount can jump; this is the end of its jump power-up cycle.
				mount->Jump(JumpPower(), mount->JumpVectorFromCurrentPose());
				mount->SetGeneralPose(AuPose::Jump);
				mount->SetInMotion();
				mount->TickTock();
			}
		}
		else if(CanJump())
		{
			// I can jump; this is the end of my jump power-up cycle.
			Jump(JumpPower(), JumpVectorFromCurrentPose());
			SetGeneralPose(AuPose::Jump);
			SetInMotion();
			TickTock();
		}
	}

	if(!(m_uiActions & ACT_MoveMask))
	{
		m_PersonLock.unlock();
		return;
	}

	// Ramp up move power so person accelerates rather than goes instantly to maximum speed.
	if(m_PowerMove < m_PowerMoveMax)
		m_PowerMove++;

	// Forward/backward movement
	if(m_uiActions & (ACT_MoveForward|ACT_MoveBackward) && !(m_uiActions & ACT_Continuous))
	{
		qint64 millis = m_tmLastMoveControl.elapsed();
		if(!m_tmLastMoveControl.isValid() || millis > controlDecayMs)
		{
			m_uiActions &= ~(ACT_MoveForward|ACT_MoveBackward);
			m_PowerMove = 0;
		}
	}

	// Lateral movement
	if(m_uiActions & (ACT_MoveLeft|ACT_MoveRight))
	{
		qint64 millis = m_tmLastMoveLControl.elapsed();
		if(!m_tmLastMoveLControl.isValid() || millis > controlDecayMs)
		{
			//if(Is(AuId(AuId::Haxus())))
				//qDebug("DaPerson::DtTick %d %s lateral movement decayed after %dms", Id().GetID(),
						//(const char*)Name().toAscii(), millis);
			m_uiActions &= ~(ACT_MoveLeft|ACT_MoveRight);
			m_PowerMove = 0;
		}
	}

	// Vertical movement
	if(m_uiActions & (ACT_MoveUp|ACT_MoveDown))
	{
		qint64 millis = m_tmLastMoveVControl.elapsed();
		if(!m_tmLastMoveVControl.isValid() || millis > controlDecayMs)
		{
			//if(Is(AuId(AuId::Haxus())))
				//qDebug("DaPerson::DtTick %d %s vertical movement decayed after %dms", Id().GetID(),
						//(const char*)Name().toAscii(), millis);
			m_uiActions &= ~(ACT_MoveUp|ACT_MoveDown);
			m_PowerMove = 0;
		}
	}

	m_PersonLock.unlock();

	DaAnimal* mount = dynamic_cast<DaAnimal*>(Parent());
	if(mount)
	{
		// Adjust head tilt of mount.
		if(m_uiActions & ACT_MoveMask)
		{
			// Insure mount is moving and looking so it climbs/descends properly.
			mount->SetInMotion();	// Kick my mount to keep it moving.
			mount->TickTock();

			// Mount takes vertical moves as head tilt, which induces climb or descent in flyers.
			if(m_uiActions & ACT_MoveUp)
				mount->TiltHeadTo(PIf/4.f);
			else if(m_uiActions & ACT_MoveDown)
				mount->TiltHeadTo(-PIf/4.f);
			else
				mount->TiltHeadTo(0);
		}
		else
		{
			// Insure mount is looking straight ahead.
			mount->TiltHeadTo(0);
		}
	}

	// Keep updating pose as long as I was moving on entry, even though I may no longer be moving. That way the body
	// stops running when out of stamina and runs again when it has stamina.
	DaCreature* poseWho;								// pose my dude unless mounted
	if(mount)
		poseWho = mount;
	else
		poseWho = this;
	AuPose nuPose = SelectPoseForMeOrMount(poseWho->CurrentGeneralPose(), poseWho);
	if(poseWho->CurrentGeneralPose() != nuPose)
	{
		//qDebug() << "DaPerson::DecayMoveActions" << Id() << Name() << "curPose" << poseWho->CurrentGeneralPose()
			//<< "nuPose" << nuPose;
		poseWho->SetGeneralPose(nuPose, poseWho->StationId(), poseWho->ConsoleOccupantPosition());
		poseWho->TickTock();
	}
}

unsigned int DaPerson::IsTocker() const
{
	unsigned int ms = DaSentient::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is not zero.

	if(ms > d_MSecPerSec)
	{
		if(DidSpawnTie())
			ms = d_MSecPerSec;
		else if(IsBodyDyingOrDead() && !IsRider())
			ms = d_MSecPerSec;
		else if(IsAwaitingSpawnRecall())
			ms = d_MSecPerSec;
		else if(m_uiActions & (ACT_SpawnAnywhere|ACT_SpawnLocal))
			ms = d_MSecPerSec;
	}
	if(IsBodyDead())
		return ms;

	if(ms > 10*d_MSecPerSec)
	{
		if(IsOnLine())
			ms = 10*d_MSecPerSec;
		else if(HasAnyCondition())
			ms = 10*d_MSecPerSec;
		else if(!IsLineageDemiavatar())
		{
			if(m_uiActions & ACT_Continuous)
				ms = 10*d_MSecPerSec;
			else if(InDesignEnvironment())
				ms = 10*d_MSecPerSec;
			else if(!IsLineageDemiavatar())
			{
				if(InArena())
					ms = 10*d_MSecPerSec;
				else if(IsLockedPose() && !IsInBerth())
					ms = 10*d_MSecPerSec;
			}

			if(ms > 10*d_MSecPerSec)
			{
				if(IsAwaitingSpawnRecall())
					ms = 10*d_MSecPerSec;
				else if(IsNormal() && !IsAtRest() && dynamic_cast<DaSolarSystem*>(Parent()))
					ms = 10*d_MSecPerSec;
			}
		}
	}
	if(IsInvisibleOffLine())
		return ms;

	if(ms > d_MSecPerMin)
		ms = d_MSecPerMin;
	if(ms > d_MSecPerSec)
	{
		if((!IsAirBreathable() && !IsInBerth() && !IsPatientStationPose() && !IsMouthSubmerged()) || IsInVisibleWater())
			ms = d_MSecPerSec;
	}

	return ms;
}

void DaPerson::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaSentient::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;

	// Watch for requirement for immediate respawn.
	if(Actions() & (ACT_SpawnAnywhere|ACT_SpawnLocal))
	{
		// LoadSubatoms has triggered an immediate respawn for this person because they were loaded with IsAwaitingSpawn true.
		m_PersonLock.lockForWrite();
		bool forceLocal = m_uiActions & ACT_SpawnLocal;			// forceLocal is true when I was assertively told to spawn here.
		m_uiActions &= ~(ACT_SpawnAnywhere|ACT_SpawnLocal);		// Turn off the respawn immediately bits.
		m_PersonLock.unlock();

		// Respawn now.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Tock" << Id() << Name() << "Respawn, forceLocal" << forceLocal;
		Respawn(forceLocal);
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

		// It is probably safest to do nothing else after calling Respawn.
		return;
	}

	if(last.ElapsedSecTicks(now) > 0)
	{
		// Test hit notify.
		/*
		if(now.time().second()%5 == 0 && Id() == AuId(AuId::Haxus()))
		{
			WEAPONEFFECT we1 = (WEAPONEFFECT)g_Au.Dice().Roll(1, 16, -1);
			WEAPONEFFECT we2 = (WEAPONEFFECT)g_Au.Dice().Roll(1, 16, -1);
			qDebug() << Id() << Name() << "we1" << we1 << EffectName(we1) << "we2" << we2 << EffectName(we2);
			m_HitNotify = AuHitNotify(AuPoint(0,1,0), we1,50,30, we2,10,0);
			m_tmHitChange.Set();
			SetModified(NOTIFY_Scene);
			m_HitNotify.RestartTime();
		}
		*/

		// Process dying state.
		// Turn off the spawn tie bit and hope the client got it already.
		// If they didn't get it, the player just won't get the visual effect and the player will probably just spawn tie again.
		ClearSpawnTie();

		// Body does not bleed when carried by another creature.
		if(IsBodyDyingOrDead() && !IsRider())
		{
			// Bleed out one point of health.
			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_InternalInjuries;
			weapon.m_w1.m_Effect = WE_Bleeding;

			AuCombatResult result;
			result.m_Damage1 = 1;

			CombatApplyDamage(AuCombatAttacker(), weapon, AuPoint(0,0), AuPoint(0,0), result);

			if(result.m_IsDestroyed || IsBodyDead())
			{
				// Demiavatar body already destroyed in CombatApplyDamage call to BodyHasDied. Drops meat pile, random item, etc.
				// Avatars and highborn will respawn when killed.
				//qDebug("DaPerson::Tick %d has died of their wounds.", Id().GetID());
				if(!IsLineageDemiavatar())
				{
					AddCondition(new AuConThought(tr("I died of my wounds."), AuTime::Now()));

					// The death event interrupts any attempt to spawn recall.
					m_PersonLock.lockForWrite();
					m_uiActions &= ~ACT_SpawnRecall;				// Be sure this is off.
					m_PersonLock.unlock();
				}

				if(!IsAIControlled())
				{
					// AI controlled avatars already did this when BodyHasDied was called.
					// Drop a random item from the person's inventory.
					// Agents do not drop an item when their agent power is enabled.
					if(!HasAgentPower())
					{
						m_GearLock.lockForWrite();
						if(DropRandomItem(m_Gear, true))
							UpdateGearWeight();	// Update my inventory weight.
						m_GearLock.unlock();
					}

					// Don't let too many identical meat piles appear in the same spot.
					if(IsInAnyScene())
					{
						// All avatars drop a head.
						AuId killerId = KillerId();
						QString killerName;
						if(!killerId.IsNull() && !Is(killerId))
						{
							DaAtom* env = FindEnvironmentAtom();
							if(env)
							{
								DaAtom* killer = env->FindIdRecursive(killerId);
								if(killer)
									killerName = killer->Name();
							}
						}
						QString inscription;
						if(!killerName.isEmpty())
							inscription = QString("%1 %2 killed by %3 %4")
								.arg(Name()).arg(Id().StringId())
								.arg(killerName).arg(killerId.StringId());
						else
							inscription = QString("%1 %2")
								.arg(Name()).arg(Id().StringId());

						AuDNABase* dna = DNAnew();
						if(dna)
						{
							if(AgentAnimal())
							{
								// When in "animal" mode, insure the DNA is also animal, not sentient, and the empire id is 0.
								AuItemDNA head(dna, Chromatid(), true, false, Handedness(), GrowthStage(), AuHairStyle::Unkempt,
										AuHairStyle::Unshaven, CID_Head, BodyQ(), 0, 1);
								head.Inscribe(inscription);
								DropItem(head, CameraOffset());
							}
							else
							{
								AuItemDNA head(dna, Chromatid(), true, true, Handedness(), GrowthStage(), Haircut(), Shave(),
										CID_Head, BodyQ(), EmpireId(), 1);
								head.Inscribe(inscription);

								const AuConditionList& cond = LockConditions();
								AuConditionList::const_iterator ci = cond.FindHighestBounty();
								if(ci != cond.cend())
								{
									const AuConBounty* bounty = (const AuConBounty*)*ci;
									head.SetBounty(AuBountyOnItem(bounty->PosterId(), bounty->PosterName(), Id(), Name(),
												bounty->Reason(), bounty->Reward()));
								}
								UnlockConditions();
								DropItem(head, CameraOffset());
							}

							// Highborn do not drop carcasses.
							DaAtom* parent = Parent();
							if(!IsLineageHighborn() && parent && parent->OkToDropMeat(Location(), CID_Animal, dna))
							{
								// Create an item to encapsulate the CID_Animal carcass left behind.
								if(AgentAnimal())
								{
									// When in "animal" mode, insure the DNA is also animal, not sentient, and the empire id is 0.
									AuItemDNA carcass(dna, Chromatid(), true, false, Handedness(), GrowthStage(), Haircut(),
											Shave(), CID_Animal, BodyQ(), 0, 1);
									DropItem(carcass);
								}
								else
								{
									AuItemDNA carcass(dna, Chromatid(), true, true, Handedness(), GrowthStage(), Haircut(), Shave(),
											CID_Animal, BodyQ(), EmpireId(), 1);
									DropItem(carcass);
								}
							}

							delete dna;
						}
					}
				}
			}
		}

		if(IsBodyDead())
		{
			// Person's body is dead.
			if(FindParentArena())
			{
				// I was killed in an arena.
				// Respawn, even if I'm a demiavatar.
				ResetHitPoints();							// Person "gets well" before respawning due to death.
				ResetStamina();
				ArenaRecall();
			}
			else if(!IsLineageDemiavatar())
			{
				// I am not a demiavatar.
				// Time to respawn.
				ResetHitPoints();							// Person "gets well" before respawning due to death.
				ResetStamina();
				m_PersonLock.lockForWrite();
				m_PersonLock.unlock();
				Respawn(false);							// Body position changes so IsBodyDead no longer returns true.
			}
			else
			{
				// I am a demiavatar and I am not in an arena.
				ReleaseBerth();
				Kill();
			}
			tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
			return;
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(IsOnLine())
	{
		// This person is on line.
		//qDebug() << "DaPerson::Tock" << Id() << Name() << "loc" << Location() << "sysLoc" << EnvLocation();
		// Decay on-line status.
		// The on-line status is refreshed regularly by the scene server thread, which includes the time the status was refreshed.
		// After the scene server stops refreshing my on-line status, I'll turn off the 'on line' flag.
		// This prevents people from simply vanishing from the scene by logging off. 
		// After exiting they'll linger in the universe for a while before disappearing.
		m_OnLineLock.lockForRead();
		bool o = m_tmLastOnLine.ElapsedSec() > c_SecsToOffLine;
		m_OnLineLock.unlock();
		if(o)
		{
			// Leave an informative time-stamped message in the log.
			if(g_Au.IsSceneWorker())
			{
				QString partingStatus = QString("%1 %2 off line").arg(Id().GetID()).arg(Name());

				const DaSector* sector = FindParentSector();
				if(sector)
					partingStatus += QString(", sector %1 %2").arg(sector->Id().GetID()).arg(sector->Name());

				const DaSolarSystem* system = FindParentSolarSystem();
				if(system)
					partingStatus += QString(", system %1 %2").arg(system->Id().GetID()).arg(system->Name());

				const DaWorld* world = FindParentPlanetInEnv();
				if(world)
					partingStatus += QString(", world %1 %2").arg(world->Id().GetID()).arg(world->Name());

				const DaVessel* ship = FindParentVesselInEnv();
				if(ship)
					partingStatus += QString(", ship %1 %2").arg(ship->Id().GetID()).arg(ship->NameWithEmpire());

				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
				if(vehicle)
					partingStatus += QString(", vehicle %1 %2").arg(vehicle->Id().GetID()).arg(vehicle->Name());

				const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
				if(mount)
					partingStatus += QString(", mount %1 %2").arg(mount->Id().GetID()).arg(mount->Name());

				partingStatus += ".";	// grammar grammar grammar

				g_Au.Log(partingStatus);
			}
			SetAccountTypeOnLine(ACCT_Count);
			SetModified(NOTIFY_Database);	// So online status is updated in db.
		}

		if(IsOnLine() && g_Au.m_EnableUnitDecay && last.ElapsedSecTicks(now) > 0)
		{
			// Environment suit takes environmental damage like an item while I am using it.
			if(!IsInsideSpacecraft() && !IsInVehicle() && IsProtectedByEnvironmentSuit())
			{
				// I am using my environment suit.
				bool decay = false;
				if(IsInVacuum())
				{
					if(IsInVisibleWater())
					{
						if(last.ElapsedHourTicks(now) > 0)
							decay = true;
					}
					else
					{
						if(last.ElapsedDayTicks(now) > 0)
							decay = true;
					}
				}
				else
				{
					switch(InAtmosphereType())
					{
					case ATMTYPE_Count:
						break;
					case ATMTYPE_Standard:
					case ATMTYPE_Tainted:
					case ATMTYPE_Exotic:
						if(IsInVisibleWater())
						{
							if(last.ElapsedMinTicks(now) > 0)
								decay = true;
						}
						else
						{
							if(last.ElapsedHourTicks(now) > 0)
								decay = true;
						}
						break;
					case ATMTYPE_Corrosive:
						if(IsInVisibleWater())
						{
							decay = true;
						}
						else
						{
							if(last.ElapsedMinTicks(now) > 0)
								decay = true;
						}
						break;
					case ATMTYPE_Insidious:
						decay = true;
						break;
					}
				}

				if(decay)
				{
					// Decay suit and helmet. Don't decay to destruction; decay to 1 will leak.
					m_GearLock.lockForRead();

					AuGearPos armorPos = m_Gear.FindArmorWorn();
					if(m_Gear.IsPosValid(armorPos))
					{
						const AuItem* armor = m_Gear.ArmorItemWorn();
						if(!armor || armor->HitPoints() <= 1)
							armorPos = AuGearPos();
					}

					AuGearPos helmPos = m_Gear.FindHelmetWorn();
					if(m_Gear.IsPosValid(helmPos))
					{
						const AuItem* helm = m_Gear.HelmetItemWorn();
						if(!helm || helm->HitPoints() <= 1)
							helmPos = AuGearPos();
					}

					m_GearLock.unlock();

					UseGearItem(armorPos, IPS_Item);

					UseGearItem(helmPos, IPS_Item);
				}
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else
	{
		// This person is not on line and invisible.
		if(m_uiActions & ACT_Continuous)
		{
			// Insure the continuous movement bit is off when they go off line.
			m_PersonLock.lockForWrite();
			m_uiActions &= ~ACT_Continuous;
			m_PersonLock.unlock();
		}

		// Exit design environment when not on line, if in a design environment.
		if(InDesignEnvironment())
			DesignEnvironmentExit();

		if(!IsLineageDemiavatar())
		{
			// Exit arena when not on line, if in an arena.
			if(InArena())
				ArenaExit();

			// Move away from my console when not on line.
			// I can stay in my berth, though.
			if(IsLockedPose() && !IsInBerth())
				ExitConsole();
		}

		// Stop from hurtling across the universe if I am moving in deep space, i.e. subordinate to a solar system.
		if(IsNormal() && !IsAtRest() && !IsAwaitingSpawnRecall() && dynamic_cast<DaSolarSystem*>(Parent()))
			SetVector(AuPoint(0,0,0));
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(IsAwaitingSpawnRecall())
	{
		// Person initiated recall home and is waiting to respawn.
		// The respawn time was stored in the last move control time when recall home was initiated.
		m_PersonLock.lockForRead();
		bool r = AuTime::Now() >= m_SpawnTime || m_SpawnTime.IsNull() || HasAgentPower();
		m_PersonLock.unlock();
		if(r)
		{
			// Recall home which could hand off this atom to another server.
			// People who initiate recall home do not "get well" before spawning.
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DaPerson::Tock" << Id() << Name() << "Respawn...";
			Respawn(false);
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DaPerson::Tock" << Id() << Name() << "Respawn done.";
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
		return;
	}

	// Don't do anything else if this person is not on line.
	// Demiavatars do not go off line.
	if(IsInvisibleOffLine())
	{
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
		return;
	}
	// I am either on line or a demiavatar.

	// Server needs to know the current gun model CID for effect placement.
	AuGearPos gearPos(CurrentItemPos());
	if(AgentAnimal())
		SetItemInHand(0);	// When impersonating an animal, do not show any items in my hand.
	else
	{
		m_GearLock.lockForRead();
		SetItemInHand(m_Gear.ItemAt(gearPos));
		m_GearLock.unlock();
	}

	// None of the rest of this stuff needs to be done in the first tock cycle.
	if(last.IsNull())
		return;

	// Conditions are ticked so they can periodically affect this person, but only while they are on line.
	TickConditions();
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	if(IsAgentPowerOn())
	{
		SetHunger(MaxHunger());
		ResetStamina();
	}

	int lastmin = last.MinsSinceEpoch();	
	int curmin = now.MinsSinceEpoch();

	// Only tick stories and apply the affects of hunger while on line and thus not AI controlled.
	if(IsOnLine())
	{
		// I am on line.
		// Process story lines in which this person is involved.
		TickStories(scripts);

		if(CurrentHunger())
		{
			// I have food in my belly.
			// City populations eat 20% of their hit points at every meal, which is every 13+ minutes.
			// The nominal 50hp person therefore eats about 10 food units per 13+ minutes, or about 45 units per hour.
			// If we take 1 food unit every other minute, that works out to 30 food units per hour. I like the sound of that better
			// than once per minute, which would be 60 food units per hour.
			// At this rate, a nominal 50 hit point person will last for 100 minutes before beginning to starve.
			// When starving, they will lose 2 hp per hour. That equals 25 hours for the nominal dude; 24 hours = 28 game days.
			// This pace of hunger growth matches that of an ECO_Hunter wild animal. Players happen to be ECO_Hunter types.
			if(lastmin != curmin && (curmin&1))
			{
				unsigned short int hungerGrowth = MaxHunger()/40;	// 1 point per 40 hp every other minute
				if(!hungerGrowth)
					hungerGrowth = 1;
				if(CurrentHunger() > hungerGrowth)
					SetHunger(CurrentHunger() - hungerGrowth);
				else
					SetHunger(0);

				// Eat right away when I get hungry enough.
				if(m_AutoEat)
					EatMealFromGear();
			}
		}
		else
		{
			// I am starving.
			// Eat before I take more hunger damage.
			if(lastmin != curmin && m_AutoEat)
				EatMealFromGear();

			if(g_Au.m_EnableCreatureDecay)
			{
				if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
				{
					AuCombatAttacker attacker(tr("hunger"));

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = CID_Starvation;
					weapon.m_w1.m_Effect = WE_Starving;

					AuCombatResult result;
					result.m_Damage1 = 2;

					CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
				}
			}
		}

		if(!InDesignEnvironment())
		{
			DaWorldLL* world = FindParentWorldInEnv();
			if(world)
			{
				// Push back the fog of war.
				ExploreWorld(world);

				if(IsInAtmosphere())
				{
					// At the world.
					// Player Statistics
					if(IsOnGround() && !IsAboardVehicle() && !IsInSpacecraft())
						Stats().PlanetTreaded(world);
					if(IsEyeSubmerged())
						Stats().PlanetDeepestSwim(world, SeaDepth());
					if(CGAltitude() > world->AtmosphereRadius() - 100*FeetPerMeter())
						Stats().PlanetHighestFall(world, CGAltitude());

					// Watch for presense in a town by a nubie emperor. They don't get to stay nubies forever.
					if(IsNewbieEmperor())
					{
						// Newbie emperor. Don't let them remain a nubie if they ever enter a town or board a spacecraft or vehicle.
						// That way they will start being treated like a regular player when they die.
						// While their spawn city is null, they respawn where they fall.
						// Spawn tie to invalid city id works ok. The player gets reassigned appropriately when they die.
						if(InIndustry() != IID_None)
						{
							const DaBd* bd = world->FindBd(BdId());
							const DaSolarSystem* system = world->FindParentSolarSystemInEnv();
							if(bd && system)
							{
								if(bd->EmpireId() == EmpireId())
								{
									QString name;
									QString cityName = bd->CityName();
									if(cityName.isEmpty())
										name = tr("%1 at %2 on %3")
											.arg(bd->Name())
											.arg(InIndustryLL().ToString())
											.arg(world->Name());
									else
										name = tr("%1 at %2 in %3")
											.arg(bd->Name())
											.arg(InIndustryLL().ToString())
											.arg(cityName);
									SetSpawnCity(BdId(), name, InIndustryLL(), system->Id(), system->Name());	// Tie to the building.
								}
								else
									ClearSpawnCity();
							}
						}
						else if(IsInSpacecraft())
							ClearSpawnCity();
						else if(IsAboardVehicle())
						{
							const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
							if(vehicle && vehicle->CommodityId() != CID_SailBoat)
								ClearSpawnCity();
						}
					}

					if(curmin != lastmin && !IsEthereal())
					{
						// Encounters are not triggered when I am an entity or invisible.

						// Offset the current minute so every avatar doesn't roll for encounters on the same minute.
						quint32 minoff = curmin + Id().GetID();
						if(minoff % 3 == 0)
						{
							if(BdId().IsNull() && InIndustry() != IID_Road)
							{
								// Trigger animal encounters every 3 minutes while in the atmosphere of a world.
								world->AnimalEncounter(GlobeLL(), 0);
							}
							else
							{
								// Store/refresh my memory of the city I am in.
								const DaSolarSystem* system = world->FindParentSolarSystem();
								const DaSector* sector = system ? system->FindParentSector() : 0;
								const DaGalaxy* galaxy = sector ? sector->FindParentGalaxy() : 0;
								if(galaxy)
								{
									const DaAtom* atom = world->FindSubatomId(InBdCity());
									const DaBd* bd = dynamic_cast<const DaBd*>(atom);
									if(bd)
										Remember(AuMemCity(bd->Id(), bd->CityName(), bd->GlobeLL(), world->Id(), world->Name(),
													system->Id(), system->Name(), sector->Id(), sector->Name(), galaxy->Galaxy()));
								}
							}
						}

						// Encounters with people.
						if(IsOnGround() && !IsInVehicle() && !IsInSpacecraft() && !InDesignEnvironment())
						{
							if(BdId().IsNull())
							{
								// Not in a city or at a building; trigger indigenous encounters.
								// This is not done when agent powers are on because it can establish the indigenous DNA of the world.
								if((minoff % 13) == 0 && !HasAgentPower())
									world->IndigenousEncounter(*this, GlobeLL(), 0);
							}
							else
							{
								// I am in a city; trigger encounters with citizens.
								world->CitizenEncounter(*this);
							}
						}
					}

					if(world->HasHenge() && !IsInSpacecraft() && !IsInVehicle() && !InDesignEnvironment())
					{
						// I am on the world, not aboard a spacecraft or inside an enclosed vehicle.
						// If my current item in hand is a henge key, it could activate a henge.
						if(CurrentItemIsHengeKey())
							world->ActivateHengeContaining(Location());
					}
				}
			}
		}

		if(curmin != lastmin)
		{
			// Remove inventory items associated with inactive story instances.
			DestructStoryItems();

			// Check for a story that can be started automatically.
			if(CanStartStory())
			{
				// Person has room to start another story. Try custom script.
				if(!AutoStartCustomScript())
					AutoStartStory(scripts);	// Custom script did not start or is not running. Try a new autostart script.
			}

			// Player Statistics
			Stats().OnLineMinute();	// Accumulate time online.

			// Every ten minutes we signal the empire record that one of its citizens is on line. It becomes noncombatant if it
			// doesn't hear from any citizen in a fifteen minute period.
			// This signal ends up in DgEmpire::CitizenOnLine.
			if((now.MinsSinceEpoch() + Id().GetID()) % 10 == 0)	// Every five minutes.
			{
				quint32 empireId = EmpireId();
				if(empireId)
					emit AvatarOnLine(Id(), empireId);
			}

			if(!InDesignEnvironment())
			{
				if(!IsInAtmosphere() && !IsInSpacecraft() && !IsAboardVehicle())
					Stats().EVA();	// Accumulate EVA time.

				DaCreature* mount = dynamic_cast<DaCreature*>(Parent());
				if(mount)
					Stats().AnimalRidden();
				else if(IsAboardVehicle())
				{
					const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
					if(vehicle)
						Stats().VehicleOperated(vehicle);
				}
				else if(HasConsole())
				{
					SYSTEM system = StationSystem();
					if(system != SYS_Count)
					{
						const DaVessel* ship = FindParentVesselInEnv();
						if(ship)
							Stats().SpacecraftStationOperated(ConsoleType(), ship->SystemQuality(system, StationId()));
					}
				}
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else
	{
		// I am not on line.
		if(curmin != lastmin && IsAIControlled())
		{
			// AI cobbled from DaCitOfficer.
			if(IsInsideSpacecraft())
			{
				// I am aboard a spacecraft.
				DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->IsDesignDataCacheValid())
				{
					if(AssignedShip().IsNull())
					{
						// I am not assigned to a spacecraft.
						if(PoliticalStance(EmpireId(), ship->EmpireId()) == POL_Enemy)
						{
							// Stop manning a battle station aboard a ship that has become an enemy.
							if(PrimaryOrder() == ORDER_BattleStation)
								RequestOrder();
						}
					}
					else if(ship->Is(AssignedShip()))
					{
						//qDebug() << "DaPerson::Tock" << Id() << Name() << "aboard assigned ship";
						// I am aboard my assigned ship.
						// Calculate time interval for daily report just like city report.
						// Pirates don't send log reports unless they have a human captain.
						m_PersonLock.lockForRead();
						int realSecs = m_LogTime.ElapsedSec();
						bool l = realSecs < 0 || (IsPirate() && ship->IsPirateWithFreeWill()) || m_LogTime.IsNull();
						m_PersonLock.unlock();
						if(l)
						{
							m_PersonLock.lockForWrite();
							m_LogTime.Set();
							m_PersonLock.unlock();
						}
						else
						{
							const float realSecsPerUDay = (float)RealDaysPerUYear() * (float)SecondsPerDay() / 365.f;
							float uDays = realSecs / realSecsPerUDay;	// Days since last meal.
							float uReports = uDays * 4;					// Four times per day.
							if(uReports >= 1)
							{
								if(Is(ship->AIFirstOfficer()))
									ShipLog(ship);					// Send log report, which includes my location and mission.
							}
						}

						// I am aboard my assigned ship.
						if(PoliticalStance(EmpireId(), ship->EmpireId()) == POL_Enemy)
						{
							// The ship's allegiance has changed so release my berth.
							ReleaseBerth(ship);
							if(IsLockedPose())
								Stand();							// Stop manning a console.
							SetModified();
						}
						else if(AuRosterEntry::IsCrewOrTroop(SpawnShipBerthType()))
						{
							// I am a working member of the crew.
							if(AuRosterEntry::IsOfficer(SpawnShipBerthType()))
							{
								// Officer
								// When there is no AI capt and no human officers, take command.
								if(!ship->IsCommandedByAI() && !ship->HasHumanOfficer() && !ship->InArena())
								{
									// Take command.
									TakeTheCon();
									ship->SetCrewPosture(Posture());			// Set crew posture to match mine.
								}

								if(HasTheCon())
								{
									// On DaSc ships, I must manage the alert condition. It triggers upward automatically.
									// I must lower it when appropriate.
									ALERT_CONDITION alert = ship->AlertCondition();	// Alert based on ship conditions.
									if(alert < ship->ApparentAlertCondition())
										ship->SetAlertCondition(alert);			// Reduce alert condition.

									// Man a station if I am not doing anything else.
									if(PrimaryOrder() == ORDER_Stay)
										GoToWork();
								}
							}
							else
							{
								// Crew or Troop
								// When there is an AI capt or no human officers, man a station if I am not doing anything else.
								if(PrimaryOrder() == ORDER_Stay && (!ship->HasHumanOfficer() || ship->IsCommandedByAI()))
									GoToWork();									// Find a station to man.
							}
						}

						DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ship->FindEnvironmentAtom());
						if(system)
						{
							if(IsPirate() && !ship->IsSpaceStation())
							{
								// Notify the solar system of the presense of pirate ships to prevent more encounters while I
								// am here.
								system->SetLastPirateShip();
							}

							if(AuRosterEntry::IsOfficer(SpawnShipBerthType()) && (!IsPirate() || !ship->IsPirateWithFreeWill()))
							{
								// Mark the solar system as visited so it does not decay. Pirates do not prevent decay.
								// This is particularly important if I now decide to scuttle the ship. This gives the owner up to a
								// week to retrieve their private ship before the system decays.
								system->SetLastOfficerVisited();
							}
						}
					}
					else
					{
						// I am aboard a spacecraft that is not my assigned ship.
						// Notify chain-of-command of my location periodically.
						// Pirates don't send log reports. I'm a pirate if my homeworld id is null.
						m_PersonLock.lockForRead();
						int realSecs = m_LogTime.ElapsedSec();
						bool l = realSecs < 0 || IsPirate() || m_LogTime.IsNull();
						m_PersonLock.unlock();
						if(l)
						{
							m_PersonLock.lockForWrite();
							m_LogTime.Set();
							m_PersonLock.unlock();
						}
					}
				}
			}
			else
			{
				// Not aboard ship so send location info periodically.
				// Calculate time interval for daily report just like city report.
				// Pirates don't send log reports. I'm a pirate if my homeworld id is null.
				m_PersonLock.lockForRead();
				int realSecs = m_LogTime.ElapsedSec();
				bool l = realSecs < 0 || IsPirate() || m_LogTime.IsNull();
				m_PersonLock.unlock();
				if(l)
				{
					m_PersonLock.lockForWrite();
					m_LogTime.Set();
					m_PersonLock.unlock();
				}
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Remainder from Think
	if(last.ElapsedSecTicks(now) > 0)
	{
		if(g_Au.m_EnableUnitDecay && !InDesignEnvironment())
		{
			// Take damage from the atmosphere if it is not breathable.
			// Atmosphere in my berth is always breathable so I can respawn there safely.
			// Environment seal of suit is broken when condition is at 1.
			if(!IsMouthSubmerged())
			{
				if(IsAirBreathable() || IsInBerth() || IsPatientStationPose() || IsProtectedByEnvironmentSuit())
					Inhale();
				else if(!HoldBreath() && !AgentInvincible())
					ApplyAtmosphericDamage();					// I am not wearing an environment suit or my suit leaks.
			}

			// Take damage from the surface liquid if it is not water. "Water" in the inferno/frigid zone is not water.
			// Environment suit does not protect me from this.
			if(IsInVisibleWater() && !AgentInvincible())
				ApplyLiquidDamage();
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

void DaPerson::Reward(double fReward, REWARDTYPE rewardType)
{
	// Increase amount owed to player by empire
	if(fReward != 0)
	{
		m_RewardsLock.lockForWrite();
		m_RewardsOwed += fReward;
		if(m_RewardsOwed < 0)
			m_RewardsOwed = 0;
		m_tmRewardChange.Set();
		m_RewardsLock.unlock();
	}

	// Now increase reputation based on rewardType
	switch(rewardType)
	{
	case REW_CaptureBase:
		IncreaseReputation(10);
		break;
	case REW_CaptureCapitalEmpire:
		IncreaseReputation(30);
		break;
	case REW_CaptureCapitalGalaxy:
		IncreaseReputation(25);
		break;
	case REW_CaptureCapitalSector:
		IncreaseReputation(20);
		break;
	case REW_CaptureCity:
		IncreaseReputation(5);
		break;
	case REW_CaptureStation:
		IncreaseReputation(10);
		break;
	case REW_CaptureSpaceship:
		IncreaseReputation(5);
		break;
	case REW_CaptureStarship:
		IncreaseReputation(1);
		break;
	case REW_DamageStation:
		IncreaseReputation(1);
		break;
	case REW_DamageSpaceship:
		IncreaseReputation(1);
		break;
	case REW_DamageStarship:
		IncreaseReputation(1);
		break;
	case REW_ColonizeHabitableWorld:
		IncreaseReputation(10);
		break;
	case REW_ColonizeUninhabitableWorld:
		IncreaseReputation(5);
		break;
	case REW_ExploreWormhole:
		IncreaseReputation(10);
		break;
	case REW_KillUnit:
		IncreaseReputation(1);
		break;
	case REW_DeliverTEU:
		IncreaseReputation(3);
		break;
	case REW_None:
	case REW_Salary:
	case REW_SpotCity:
	case REW_SpotEnemy:
	case REW_OrderComplete:
		break;
	}
	SetModified();
}

void DaPerson::Respawn(bool forceLocal)
{
	if(g_Au.m_LogMessages & LOG_Spawn)
		qDebug() << "DaPerson::Respawn" << *this << "forceLocal" << forceLocal;

	// Reset some values on this person as part of respawning.
	if(m_uiActions & ACT_SpawnRecall)
	{
		m_PersonLock.lockForWrite();
		m_uiActions &= ~ACT_SpawnRecall;	// Be sure this is off.
		m_PersonLock.unlock();
	}

	// Exit console in case I am in a table game.
	if(IsLockedPose())
		ExitConsole();
	else
	{
		DismountAllRiders();									// No passengers!
		Stand();												// Stand in place. Exit console does this.
		SetVector(AuPoint(0,0));								// Stop moving!
		SetModified();
	}

	GALAXY galaxy;
	const DaGalaxy* pg = FindParentGalaxy();
	if(pg)
		galaxy = pg->Galaxy();
	else
		galaxy = ShoresOfHazeron;

	// Attempt to respawn in the local atom data.
	// If this fails, we must propagate a request up to the scene master server who must then poll the server farm looking for
	// a spawn point.
	DaSolarSystem* system = FindParentSolarSystem();	// Can only do recursive id search safely from solar system level.
	if(!system)
	{
		// This happens when the contents of a solar system have decayed and the people there need to respawn somewhere else.
		// They are moved subordinate to their parent galaxy and then this function is called in DaSolarSystem::DecayContents.
		SetTraveling();
		SetGeneralPose(AuPose::Spawn);
		SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));

		// Respawn in my current galaxy, if I'm in one, otherwise Shores of Hazeron is the default.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere to depart decayed system";
		emit Spawn(Id(), EmpireId(), SpawnCity(), SpawnShip(), ArenaExitParent(), galaxy);
		return;
	}

	// Try to spawn at the arena exit.
	if(!ArenaExitParent().IsNull())
	{
		// Locate the city in the local atom data.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "try arena" << ArenaExitParent();
		DaAtom* arena = system->FindIdRecursive(ArenaExitParent());
		if(arena && arena->IsNormal())
		{
			// The arena world was found.
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DaPerson::Respawn" << *this << "arena world found" << *arena;
			ForcePosStop(AuAtomPos(arena->Id(), AuMatrix4x4()));
			FinishSpawn(ArenaExitLoc());
			return;
		}
		else
		{
			// The tied world was not found in the local data.
			if(!forceLocal)
			{
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere to find the arena exit" << ArenaExitParent();
				// Pass the respawn request up the chain because the world must be on another server.
				SetTraveling();
				SetGeneralPose(AuPose::Spawn);
				SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));
				emit Spawn(Id(), EmpireId(), SpawnCity(), SpawnShip(), ArenaExitParent(), galaxy);
				return;
			}
		}
	}

	// Try to spawn aboard a spacecraft.
	if(!SpawnShip().IsNull())
	{
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "trying spacecraft" << SpawnShip();
		// Locate the spacecraft in the local atom data.
		DaVessel* ship = dynamic_cast<DaVessel*>(system->FindIdRecursive(SpawnShip()));
		if(ship && ship->IsDesignDataCacheValid())
		{
			// The ship was found. Is it a viable spawn point?
			// NOTE: This test needs to match the one in AuSceneServerApp::QueryAtomServer.
			int can = ship->CanSpawnHere(Id(), EmpireId());
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DaPerson::Respawn" << *this << "found ship" << *ship << "canSpawn" << can;
			switch(can)
			{
			case 0:
				// Spawn at my berth location.
				ForcePosStop(AuAtomPos(ship->Id(), AuMatrix4x4()));
				FinishSpawn(ship->BerthLocation(Id()));
				return;
			case 1:
				// No because life support is down.
				AddCondition(new AuConThought(tr("I could not recall to my berth because the ship's life support system is down."),
							AuTime::Now()));
				break;
			default:
				// Lose the berth for all other reasons.
				SetSpawnShip(AuId(0), "", AuRosterEntry::Invalid, false);
				SetModified();
				break;
			}
			// Try city spawn logic.
		}
		else
		{
			// The tied ship was not found in the local data.
			// We'll go through this if the ship's design cache is not valid also, to give it time to load.
			if(!forceLocal)
			{
				// Pass the respawn request up the chain because the ship could be on another server.
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere to find my spacecraft" << SpawnShip();
				SetTraveling();
				SetGeneralPose(AuPose::Spawn);
				SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));
				emit Spawn(Id(), EmpireId(), SpawnCity(), SpawnShip(), AuId(0), galaxy);
				return;
			}
		}
	}

	// Try to spawn at the person's home building.
	if(!SpawnCity().IsNull())
	{
		// Locate the city in the local atom data.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "trying building" << SpawnCity();
		DaAtom* spawnAtom = system->FindIdRecursive(SpawnCity());
		if(spawnAtom)
		{
			// The spawn atom was found; it must be a building.
			DaBd* bd = dynamic_cast<DaBd*>(spawnAtom);
			if(bd)
			{
				// The bd building was found. Is it a viable spawn point?
				// NOTE: This test needs to match the one in AuSceneServerApp::QueryAtomServer.
				int can = bd->CanSpawnHere(EmpireId());
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "found bd" << *bd << "canSpawn" << can;
				switch(can)
				{
				case 0:
					{
						DaWorld* planet = bd->FindParentPlanetInEnv();
						if(planet)
						{
							ForcePosStop(AuAtomPos(planet->Id(), AuMatrix4x4()));
							FinishSpawn(bd->RandomSpawnLoc());
						}
					}
					return;
				default:
					ClearSpawnCity();
					SetModified();
					break;
				}
			}
			else
			{
				qWarning() << "error: DaPerson::Respawn" << Id() << Name() << "spawnAtom" << spawnAtom->Id()
					<< spawnAtom->Name() << "is not a building";
				ClearSpawnCity();
				SetModified();
			}
			// Try galaxy capital city.
		}
		else
		{
			// The tied city was not found in the local data.
			if(!forceLocal)
			{
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere to find my home bd" << SpawnCity();
				// Pass the respawn request up the chain because the ship could be on another server.
				SetTraveling();
				SetGeneralPose(AuPose::Spawn);
				SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));
				emit Spawn(Id(), EmpireId(), SpawnCity(), AuId(0), AuId(0), galaxy);
				return;
			}
		}
	}

	// Try to spawn in the person's galaxy capital city, unless it's their home city which was already checked.
	AuId capitolU, capitolG;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireCapital(EmpireId(), capitolU);
	capitolG = gd.EmpireGalaxyCapital(EmpireId(), galaxy);
	g_Au.UnlockData();
	if(!capitolG.IsNull() && capitolG != SpawnCity())
	{
		// Locate the galaxy capital city in the local atom data.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "trying galaxy capitol" << capitolG;
		DaAtom* capatom = system->FindIdRecursive(capitolG);
		if(capatom)
		{
			DaBd* bd = dynamic_cast<DaBd*>(capatom);
			if(bd)
			{
				// The building was found.
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "found bd" << *bd;
				// Is the capitol building controlled by this person's empire.
				if(bd->EmpireId() == EmpireId())
				{
					// Respawn in the town square.
					DaWorld* planet = bd->FindParentPlanetInEnv();
					if(planet)
					{
						ForcePosStop(AuAtomPos(planet->Id(), AuMatrix4x4()));
						FinishSpawn(bd->RandomSpawnLoc());
					}
					return;
				}
				// The capitol building is no longer friendly to the player.
			}
			else
				qWarning() << "error: DaPerson::Respawn" << *this << "capatom" << *capatom << "is not a building";
			// Try random empire city.
		}
		else
		{
			// The galaxy capital city was not found in the local data.
			if(!forceLocal)
			{
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere to find galaxy capitol" << capitolG;
				// Pass the respawn request up the chain because the capital city must be on another server.
				SetTraveling();
				SetGeneralPose(AuPose::Spawn);
				SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));
				emit Spawn(Id(), EmpireId(), capitolG, AuId(0), AuId(0), galaxy);
				return;
			}
		}
	}

	// Try to spawn in any random city in the person's empire, except the capital or their spawn city.
	AuId randomCity;
	if(forceLocal)
	{
		// Locate a random empire city in my present system. DgData::RandomEmpireCity gets me going, possibly to another server.
		// It's not likely to randomly generate the same city again when I actually arrive there. I just have to trust that
		// it sends me to a system that contains at least one empire city; I don't have to end up in that exact city.
		DaAtomList cities;
		system->FindBdRecursive(cities, EmpireId());
		cities.RemoveAtomId(capitolU);
		cities.RemoveAtomId(capitolG);
		cities.RemoveAtomId(SpawnCity());

		if(cities.count() == 1)
			randomCity = cities.first()->Id();
		else if(!cities.isEmpty())
		{
			int randomIndex = g_Au.Dice().Roll(1,cities.count(),-1);
			randomCity = cities[randomIndex]->Id();
		}
	}
	else
	{
		QSet<AuId> exceptIds;
		exceptIds.insert(capitolU);
		exceptIds.insert(capitolG);
		exceptIds.insert(SpawnCity());
		const DgData& gd = g_Au.LockDataForRead();
		gd.RandomEmpireCity(EmpireId(), randomCity, exceptIds);
		g_Au.UnlockData();
	}
	if(!randomCity.IsNull())
	{
		// Locate the random city in the local atom data.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "trying random bd" << randomCity;
		DaAtom* ranatom = system->FindIdRecursive(randomCity);
		if(ranatom)
		{
			DaBd* bd = dynamic_cast<DaBd*>(ranatom);
			if(bd)
			{
				// The bd building was found.
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "found bd" << *bd;
				// Is the random city controlled by this person's empire.
				if(bd->EmpireId() == EmpireId())
				{
					DaWorld* planet = bd->FindParentPlanetInEnv();
					if(planet)
					{
						ForcePosStop(AuAtomPos(planet->Id(), AuMatrix4x4()));
						FinishSpawn(bd->RandomSpawnLoc());
					}
					return;
				}
				// The random city is no longer friendly to the player.
			}
			else
				qWarning() << "error: DaPerson::Respawn" << *this << "ranatom" << *ranatom << "is not a building";
			// Try universe capital city.
		}
		else
		{
			// The random city was not found in the local data.
			if(!forceLocal)
			{
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere to find random city" << randomCity;
				// Pass the respawn request up the chain because the random city must be on another server.
				SetTraveling();
				SetGeneralPose(AuPose::Spawn);
				SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));
				emit Spawn(Id(), EmpireId(), randomCity, AuId(0), AuId(0), galaxy);
				return;
			}
		}
	}

	// Try to spawn in the person's universe capital city, unless it's their home city or galaxy capital which were already checked.
	if(!capitolU.IsNull() && capitolU != SpawnCity() && capitolU != capitolG)
	{
		// Locate the universe/empire capital city in the local atom data.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "trying universe capitol" << capitolU;
		DaAtom* capatom = system->FindIdRecursive(capitolU);
		if(capatom)
		{
			DaBd* bd = dynamic_cast<DaBd*>(capatom);
			if(bd)
			{
				// The building was found.
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "found bd" << *bd;
				// Is the capitol building controlled by this person's empire.
				if(bd->EmpireId() == EmpireId())
				{
					// Respawn in the town square.
					DaWorld* planet = bd->FindParentPlanetInEnv();
					if(planet)
					{
						ForcePosStop(AuAtomPos(planet->Id(), AuMatrix4x4()));
						FinishSpawn(bd->RandomSpawnLoc());
					}
					return;
				}
				// The capitol building is no longer friendly to the player.
			}
			else
				qWarning() << "error: DaPerson::Respawn" << *this << "capatom" << *capatom << "is not a building";
		}
		else
		{
			// The universe/empire capital city was not found in the local data.
			if(!forceLocal)
			{
				if(g_Au.m_LogMessages & LOG_Spawn)
					qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere to find universe capitol" << capitolG;
				// Pass the respawn request up the chain because the capital city must be on another server.
				SetTraveling();
				SetGeneralPose(AuPose::Spawn);
				SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));
				emit Spawn(Id(), EmpireId(), capitolU, AuId(0), AuId(0), galaxy);
				return;
			}
		}
	}

	// We only get here when we know none of the person's "tied" spawn points or any empire city exists.
	// This may be a nu player/emperor who has not planted their first flag yet.
	// This could also be a player who has lost all of the cities from their empire.
	DaWorldLL* planet = FindParentWorldInEnv();
	if(!planet)
	{
		// No parent planet.
		// This happens when a person has respawned but has no viable spawn points at all. They are passed up to the master server
		// who requests a nu empty planet be created for them, just like a starting homeworld. Then a notification is passed back
		// down to a worker to load that person in their nu solar system. The person shows up here, subordinate to the solar
		// system that contains their spawn world.
		// Locate the spawn world.
		DaWorldLL* habw = system->FindHabitableWorld(HABITABLE_HomeWorld);
		if(habw && habw->IsSuitableRespawnPlanet(EmpireId()))
		{
			// Make the person subordinate to the planet.
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DaPerson::Respawn" << *this << "in system" << *system << "suitable homeworld found" << *planet;
			ForcePosStop(AuAtomPos(habw->Id(), AuMatrix4x4()));

			// Position the person on the planet using logic similar to a nu emperor spawning there.
			habw->InitSpawnLocation(this);
			FinishSpawn(Location());
			return;
		}

		// No suitable planet was found.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "habitable homeworld not found in system" << *system;
	}

	if(g_Au.m_LogMessages & LOG_Spawn)
	{
		if(planet)
			qDebug() << "DaPerson::Respawn" << *this << "on world" << *planet
				<< "Bd" << planet->BdHere() << "atmoRad" << planet->AtmosphereRadius()
				<< "inAtmo" << IsInAtmosphere() << "CGalt" << CGAltitude();
		else
			qDebug() << "DaPerson::Respawn" << *this << "not on world";
	}

	if(planet && IsInAtmosphere() && planet->IsSuitableRespawnPlanet(EmpireId()))
	{
		// This person is at a suitable home planet.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::Respawn" << *this << "on undeveloped world";

		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
		if(llworld && IsInAtmosphere())
		{
			// Returning now without doing anything will result in this dude standing in place, alive and well where he fell.
			if(llworld->HasVillage())
			{
				// Respawn at village.
				AuGlobeLLF villageLL = llworld->VillageLL();
				AuGlobeLLF spawnLL = llworld->RandomLandLLNear(g_Au.Dice(), villageLL, .5, 3*FeetPerMeter(), 10*FeetPerMeter());
				AuPoint loc = llworld->GetSurfaceLocation(spawnLL, GroundHeightAboveCG());
				AuPoint upVec = -llworld->GravityDir(loc);

				AuMatrix4x4 tBody;
				tBody.Translate(loc);
				tBody *= AuPoint(0,0,1).GetAlignmentTransform(upVec);
				ForcePosStop(AuAtomPos(llworld->Id(), tBody));
			}
			else
				llworld->InitSpawnLocation(this);
		}
		else
			planet->InitSpawnLocation(this);
		FinishSpawn(Location());
		return;
	}

	// This person must respawn elsewhere.
	if(g_Au.m_LogMessages & LOG_Spawn)
		qDebug() << "DaPerson::Respawn" << *this << "spawning elsewhere. empire" << EmpireId() << "spawnCity"
			<< SpawnCity() << "spawnShip" << SpawnShip() << "isPrivate" << SpawnShipIsPrivate();
	SetTraveling();
	SetGeneralPose(AuPose::Spawn);							// prepare to respawn
	SyncController(AuSyncPose(AuPose::Spawn, 0, 0, 0));
	emit Spawn(Id(), EmpireId(), SpawnCity(), SpawnShip(), ArenaExitParent(), galaxy);
}

void DaPerson::FinishSpawn(const AuPoint& loc)
{
	if(g_Au.m_LogMessages & LOG_Spawn)
		qDebug() << "DaPerson::FinishSpawn" << Id() << Name() << "parent" << *Parent() << "arena"
			<< ArenaExitParent();

	// Don't remain invisible after spawn recall.
	m_AgentBits &= ~(AGENT_Invisible|AGENT_Animal|AGENT_Entity|AGENT_Invincible);

	if(!ArenaExitParent().IsNull())
	{
		// Exiting an arena.
		m_PersonLock.lockForWrite();
		m_ArenaExitParent = AuId(0);	// Turn off arena exit.
		AuGear ainv(m_ArenaExitInv);
		m_ArenaExitInv.Clear();
		m_PersonLock.unlock();

		// Restore inventory.
		m_GearLock.lockForWrite();
		m_Gear = ainv;
		m_tmGearChange.Set();
		m_GearLock.unlock();
	}

	bool inBerth = false;
	AuMatrix4x4 t;
	t.Translate(loc);
	DaVessel* ship = FindParentVesselInEnv();
	AuPose pose;
	if(ship)
	{
		// On board a ship I am spawning in a berth.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::FinishSpawn" << Id() << Name() << "aboard" << ship->Id() << ship->Name();
		AuMatrix4x4 berthM;
		if(ship->BerthPosition(*this, berthM))
		{
			// Select a berth pose so I will not suffocate when I wake up. My berth is a safe zone.
			int emptySeatIndex = ship->EmptyBunkInBerth(loc);	// Berth index 0=bottom berth, 1=top berth.
			AuPose sleepPose[8] =
			{
				AuPose::BerthBottomFaceDown,
				AuPose::BerthBottomFaceLeft,
				AuPose::BerthBottomFaceRight,
				AuPose::BerthBottomFaceUp,
				AuPose::BerthTopFaceDown,
				AuPose::BerthTopFaceLeft,
				AuPose::BerthTopFaceRight,
				AuPose::BerthTopFaceUp
			};
			int poseIndex = g_Au.Dice().Roll(1,4,(emptySeatIndex*4)-1);
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DaPerson::FinishSpawn" << Id() << Name() << "sleeping in bunk, pose" << sleepPose[poseIndex];
			pose = sleepPose[poseIndex];
			SetGeneralPose(pose);

			// Offset and rotate to the proper position and facing for my berth's rotation.
			t = berthM;
			inBerth = true;
		}
		else
		{
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DaPerson::FinishSpawn" << Id() << Name() << "standing at helm";
			// Stand at spawn location, which is the helm.
			t.Translate(0,0,CGHeightStanding());				// Move me up to the floor/terrain surface.
			Stand();
			pose = AuPose::Stand;
		}
		ForcePosStop(AuAtomPos(ship->Id(), t));
	}
	else
	{
		// Not on a ship so I am at a planet, most likely.
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "DaPerson::FinishSpawn" << Id() << Name() << "not aboard a ship";
		t.Translate(0,0,CGHeightStanding());					// Move me up to the floor/terrain surface.
		Stand();
		pose = AuPose::Stand;
		ForcePosStop(AuAtomPos(ParentId(), t));
	}
	if(g_Au.m_LogMessages & LOG_Spawn)
		qDebug() << "DaPerson::FinishSpawn" << *this << "sync pose" << AuPose(pose);
	SyncController(AuSyncPose(pose, 0, 0, 0));
	InvalidateEnvironment();
	SetVehicleSeatIndex(0);
	SetModified();

	if(!inBerth)
	{
		if(IsMouthSubmerged())
		{
			// I am submerged?
			int prot = IsProtectedByDivingGear();
			if(prot != 0)
			{
				// I am not protected by diving gear.
				if(prot == 1)
				{
					// All I need to do is close my visor.
					CloseHelmetVisor();
					QString text = tr("I instinctively closed my visor after recalling submerged.");
					AddCondition(new AuConThought(text, AuTime::Now()));
				}
				else if(BdId() == SpawnCity())
				{
					// I respawned at my home but I can't breathe. Let me die and spawn elsewhere.
					ClearSpawnCity();
					QString text = tr("I disowned my home because I recalled submerged there, without diving gear.");
					AddCondition(new AuConThought(text, AuTime::Now()));
				}
			}
		}
		else if(!IsAirBreathable())
		{
			// Atmosphere is not breathable.
			int prot = IsProtectedByEVSuit();
			if(prot != 0)
			{
				// I am not protected by an EV suit.
				if(prot == 1)
				{
					// All I need to do is close my visor.
					CloseHelmetVisor();
					QString text = tr("I instinctively closed my visor after recalling into a deadly environment.");
					AddCondition(new AuConThought(text, AuTime::Now()));
				}
				/*
				else if(BdId() == SpawnCity())
				{
					// I respawned at my home but I can't breathe. Let me die and spawn elsewhere.
					ClearSpawnCity();
					QString text = tr("I disowned my home because I recalled into a deadly environment there, without an EV suit.");
					AddCondition(new AuConThought(text, AuTime::Now()));
				}
				*/
				else if(IsInAtmosphere())
				{
					// POP! Emergency pressure tent from nowhere.
					AuItem* item = AuItem::Create(CID_PressureTent, 1, 0);
					if(item)
					{
						QString text;
						const DaWorld* world = FindParentWorldInEnv();
						if(world)
							text = tr("Deployed %1 on %4 at %3 for %2.")
								.arg(AuTime::CurrentUTC().toString())
								.arg(Name())
								.arg(GlobeLL().ToString())
								.arg(world->Name());
						else
							text = tr("Deployed %1 for %2.")
								.arg(AuTime::CurrentUTC().toString())
								.arg(Name());
						item->Inscribe(text);
						DropItem(*item);

						double cost = round(item->UnitCost() * 10);
						delete item;

						double bal = AccountBalance();
						if(cost > bal)
							cost = bal;
						DebitAccount(cost);

						if(cost > 0)
							text = tr("An emergency pressure tent deployed when I recalled into a deadly environment. "
									"I paid %L1%2 for this service.").arg(cost, 0,'f',0).arg(g_Au.CurrencySymbol());
						else
							text = tr("An emergency pressure tent deployed when I recalled into a deadly environment. "
									"This service was provided free of charge because I have no money.");
						AddCondition(new AuConThought(text, AuTime::Now()));
					}
				}
			}
		}
	}
	SetModified();
	if(g_Au.m_LogMessages & LOG_Spawn)
		qDebug() << "DaPerson::FinishSpawn" << *this << "done, parent" << *Parent();
}

void DaPerson::WillfullyChangedEmpire(quint32 nuEmpireId)
{
	quint32 oldEmpireId = EmpireId();							// Save my old empire id for the signal emitted at the end.
	SetEmpireId(nuEmpireId);									// Change empire of allegiance.
	// why? ClearSpawnCity();
	SetModified();

	if(!SpawnShip().IsNull())
	{
		// This person occupies a berth aboard a ship.
		if(nuEmpireId)
		{
			if(SpawnShipIsPrivate())
			{
				// Privately owned ship changes its allegiance to that of the owner.
				// Try to find their ship in this solar system.
				DaVessel* ship = dynamic_cast<DaVessel*>(FindEnvironmentAtom()->FindIdRecursive(SpawnShip()));
				if(ship && Is(ship->OwnerId()))
				{
					// Ship was found in this solar system and I am the owner.
					if(nuEmpireId)
						ship->SetEmpireId(nuEmpireId);
					//else
						//ship->SetPrivateOwner(AuId(0), "");
					ship->SetModified();
				}
			}
			else
			{
				// Release my berth, wherever the ship exists.
				emit ReleaseBerthSignal(Id(), AuId(0));
			}
		}
		else
		{
			// Release my berth, wherever the ship exists, unless it is a passenger berth.
			if(SpawnShipBerthType() != AuRosterEntry::PassengerHuman)
				emit ReleaseBerthSignal(Id(), AuId(0));
		}
	}

	if(nuEmpireId)
		emit AddCitizenToEmpire(oldEmpireId, nuEmpireId, Id(), AssignedName(), (quint8)Gender());
}

// This function is only called on the client during animate.
void DaPerson::ApplyMovement(unsigned int msElapsed)
{
	//qDebug() << "DaPerson::ApplyMovement" << Id() << Name() << "head" << CurrentHeadTilt()*RAD2DEGf
		//<< "turn" << Rotation().z*RAD2DEG << "ms" << msElapsed << "isTilting" << IsTiltingHead() << "rotX" << m_byRotSpeedX
		//<< "rotZ" << m_byRotSpeedZ
		//<< AuTime::Now();
	if(!IsNormal())
		return;

	if(!g_Au.IsSceneApp())
		return;

	if(IsInBerth() || IsPatientPronePose())
		return;

	// Determine if mounted on an animal.
	DaAnimal* mount = dynamic_cast<DaAnimal*>(Parent());
	bool isFlyer = false;
	if(mount)
	{
		//qDebug("Apply movement to mounted person %d", Id().GetID());
		// Flying mounts that cannot hover have movement restrictions.
		if(mount->IsFlying() && !mount->CanHover())
			isFlyer = true;
	}

	// This function requires some move actions to be valid, unless this is a moving flyer, in which case it must keep moving.
	unsigned int acti = Actions();
	if(!isFlyer && (acti & (ACT_LookMask|ACT_TurnMask|ACT_MoveMask)) == 0)
	{
		//qDebug("skipping %x due to no movement %d ms elapsed", acti, msElapsed);
		return;
	}

	// The server does not respond to head tilt or body turn. Those rotation values are sent directly from the client so they
	// can provide smooth apparent turn response.

	// When animating local rotation, limit the time span so occasional lags won't cause huge turns.
	//qDebug("moving 0x%x, %d ms elapsed", acti, msElapsed);
	if(msElapsed > 150)
		msElapsed = 150;
	float rsec = (float)msElapsed/1000;			// Real world secs that have passed.

	// Calculate turn rate that is the same for all player characters. Nobody likes having their turn rate based on their DNA.
	float turnRate;
	if(IsTurretPose())
	{
		turnRate = TurretTurnRate();			// Constant turn rate.
	}
	else
	{
		turnRate = c_AvatarTurnRate;			// Constant turn rate.
		if(CurrentGeneralPose() == AuPose::Glide || CurrentGeneralPose() == AuPose::Hover)
			turnRate /= 2.f;
		else if(CurrentGeneralPose() == AuPose::Fly)
			turnRate /= 2.5f;
		else if(CurrentGeneralPose() == AuPose::FlyFast)
			turnRate /= 3.f;
	}

	// Calculate turn amounts based on turn rate and time span.
	m_PersonLock.lockForWrite();
	float rTurnX = (float)m_byRotSpeedX/300.f * DEG2RADf;
	float rTurnZ = (float)m_byRotSpeedZ/300.f * DEG2RADf;
	float rTurnMountZ = rTurnZ/10;
	//if(rTurnX)
		//qDebug() << "control rotSpeedX" << m_byRotSpeedX << "rTurnX" << (rTurnX * RAD2DEGf) << "rsec" << rsec
			//<< "turnRate" << turnRate*RAD2DEGf << "used" << (int)(rTurnX * RAD2DEGf * 300);
	float rMaxTurn = turnRate * rsec;						// Max turn angle in this time period.
	if(rTurnX > rMaxTurn)
	{
		rTurnX = rMaxTurn;
		int rotused = (int)(rTurnX * RAD2DEGf * 300);
		m_byRotSpeedX -= rotused;
	}
	else if(rTurnX < -rMaxTurn)
	{
		rTurnX = -rMaxTurn;
		int rotused = (int)(rTurnX * RAD2DEGf * 300);
		m_byRotSpeedX -= rotused;
	}
	else
	{
		m_byRotSpeedX = 0;
	}
	//if(rTurnX)
		//qDebug() << "result rotSpeedX" << m_byRotSpeedX << "rTurnX" << (rTurnX * RAD2DEGf) << "maxTurn" << rMaxTurn*RAD2DEGf;

	if(rTurnZ > rMaxTurn)
	{
		rTurnZ = rMaxTurn;
		int rotused = (int)(rTurnZ * RAD2DEGf * 300);
		m_byRotSpeedZ -= rotused;
	}
	else if(rTurnZ < -rMaxTurn)
	{
		rTurnZ = -rMaxTurn;
		int rotused = (int)(rTurnZ * RAD2DEGf * 300);
		m_byRotSpeedZ -= rotused;
	}
	else
	{
		m_byRotSpeedZ = 0;
	}
	m_PersonLock.unlock();

	// Consoles can sometimes be broken.
	// This is skipped in design environment so consoles will swivel.
	if(!InDesignEnvironment() && IsLockedPose())
	{
		if(IsInCapsule())
		{
			const DaCapsule* ship = dynamic_cast<const DaCapsule*>(Parent());
			if(ship)
			{
				SYSTEM system = StationSystem();
				if(system != SYS_Count)
				{
					bool rotOk = true;
					if(IsHelmStationPose() || IsEngineStationPose())
						rotOk = true;			// Helm and Engineer Console can always rotate.
					else
						rotOk = !ship->IsNew();	// Turrets cannot rotate when the ship is "nu".
					if(!rotOk || ship->CapacitorLevel() == 0)
					{
						if(rTurnZ != 0 || system == SYS_Turret)
						{
							AuEffect effect(AuPoint(0,0), AuPoint(0,0), AuPoint(0,0), SOUND_ServoWeak, 1,1, VISUAL_None);
							AddEffect(effect);
						}

						// Don't permit Z rotation when console is unpowered.
						rTurnZ = 0;

						// Turrets don't allow head tilt either when unpowered.
						if(system == SYS_Turret)
							rTurnX = 0;
					}
					else if(!ship->IsConsoleOperational(StationSysId()))
					{
						if(rTurnZ != 0 || system == SYS_Turret)
						{
							AuEffect effect(AuPoint(0,0), AuPoint(0,0), AuPoint(0,0), SOUND_ServoStopped, 1,1, VISUAL_None);
							AddEffect(effect);
						}

						// Don't permit Z rotation when console is broken.
						rTurnZ = 0;

						// Turrets don't allow head tilt either when broken.
						if(system == SYS_Turret)
							rTurnX = 0;
					}
				}
			}
		}
	}

	// Averaging turn speed using recent turn amounts.
	if(rTurnX != 0 || rTurnZ != 0)
	{
		// Invert sign of negative turn angles.
		// Turn right is negative rotation around Z.
		if(acti & ACT_TurnRight)
			rTurnZ = -rTurnZ;

		// Look/tilt down is negative rotation around X.
		if(acti & ACT_LookDown)
			rTurnX = -rTurnX;

		m_PersonLock.lockForWrite();

		// Update turn average calculation values.
		m_TurnX[m_TurnIndex] = rTurnX;
		m_TurnZ[m_TurnIndex] = rTurnZ;
		m_tmTurn[m_TurnIndex].restart();
		m_TurnIndex++;
		if(m_TurnIndex > 19)
			m_TurnIndex = 0;
		if(m_TurnCount < 20)
			m_TurnCount++;

		if(g_Au.ControlAverageSpanMs())
		{
			// Calculate the averate turn rate over the past fraction of a second.
			const float avgSpanMs = g_Au.ControlAverageSpanMs();	// milliseconds
			float turnTimeX = 0;
			float turnTimeZ = 0;
			int turnVals = 0;
			for(int ti=0; ti<m_TurnCount; ti++)
			{
				qint64 elaps = m_tmTurn[ti].elapsed();
				if(elaps < avgSpanMs)
				{
					// Averages are weighted so they fade with time.
					float wt = (float)elaps / avgSpanMs;
					wt *= wt;
					wt = 1.f - wt;
					turnTimeX += m_TurnX[ti] * wt;
					turnTimeZ += m_TurnZ[ti] * wt;
					turnVals++;
				}
			}
			rTurnX = turnTimeX / turnVals;
			rTurnZ = turnTimeZ / turnVals;
		}

		m_PersonLock.unlock();
	}

	if(rTurnX != 0 || rTurnZ != 0)
	{
		AuPoint oldVec;
		bool rotvec = false;
		if(!CanControlMovement())
		{
			oldVec = Transform().ProjectVector(Vector());
			rotvec = true;
		}
		AuPoint otn = Transform().ProjectVector(TerrainNormal());
		AuPoint ogn = Transform().ProjectVector(GravityNormal());

		// Head swiveling actions.
		if(rTurnX != 0 && IsTiltingHead())
		{
			if(OkToTiltHead())
			{
				TiltHeadTo(CurrentHeadTilt()+rTurnX);			// TiltHeadTo will constrain to turret angle, if necessary.
				SetModified(NOTIFY_Scene);
			}
			else
			{
				// Not in gravity and not at a console. Rotate entire body instead of tilting head.
				TiltHeadTo(0);									// Look straight ahead.
				AuMatrix4x4 tMe = Transform();
				tMe.RotateX(rTurnX);							// Rotate body.
				ChangePosAndOrient(AuAtomPos(ParentId(), tMe));
				SetModified();
			}
		}

		// Body turning actions.
		// Constrain turn angle when engaged with a wall turret.
		if(rTurnZ != 0 && (acti & (ACT_TurnLeft|ACT_TurnRight)))
		{
			double constrainTurn;
			double gunDir;
			switch(CurrentGeneralPose().Pose())
			{
			//DHH Sc Console constrain turn for turrets.
			//case AuPose::Turret_Sit:
			//case AuPose::Turret_SitStationary:
			//case AuPose::Turret_SitSwivel3D:
			//case AuPose::Turret_Stand:
			//case AuPose::Turret_StandStationary:
			case AuPose::ConsoleTurret0:
				constrainTurn = PI/2;
				gunDir = 270 * DEG2RAD;
				break;
			case AuPose::ConsoleTurret45:
				constrainTurn = PI/2;
				gunDir = 315 * DEG2RAD;
				break;
			case AuPose::ConsoleTurret90:
				constrainTurn = PI/2;
				gunDir = 0 * DEG2RAD;
				break;
			case AuPose::ConsoleTurret135:
				constrainTurn = PI/2;
				gunDir = 45 * DEG2RAD;
				break;
			case AuPose::ConsoleTurret180:
				constrainTurn = PI/2;
				gunDir = 90 * DEG2RAD;
				break;
			case AuPose::ConsoleTurret225:
				constrainTurn = PI/2;
				gunDir = 135 * DEG2RAD;
				break;
			case AuPose::ConsoleTurret270:
				constrainTurn = PI/2;
				gunDir = 180 * DEG2RAD;
				break;
			case AuPose::ConsoleTurret315:
				constrainTurn = PI/2;
				gunDir = 225 * DEG2RAD;
				break;
			default:
				constrainTurn = 0;
				break;
			}
			if(constrainTurn != 0)
			{
				// Constrain turn to the limits of my turret arc.
				double zrot = Rotation().z;		// My current Z rotation.
				double nuz = zrot + rTurnZ;	// My proposed Z rotation.
				double deltaz = nuz - gunDir;	// Delta from turret direction.
				while(deltaz < -PI)
					deltaz += PI*2;
				while(deltaz > PI)
					deltaz -= PI*2;
				// Prevent turning beyond arc of gun.
				if(deltaz < -constrainTurn)
				{
					double diff = -constrainTurn - deltaz;
					rTurnZ += diff;
				}
				else if(deltaz > constrainTurn)
				{
					double diff = constrainTurn - deltaz;
					rTurnZ += diff;
				}
			}

			//if(rTurnZ < 0)
				//qDebug("Turning right %g degrees from Z = %g degrees", -rTurnZ*RAD2DEG, Rotation().z*RAD2DEG);
			//else
				//qDebug("Turning left %g degrees from Z = %g degrees", rTurnZ*RAD2DEG, Rotation().z*RAD2DEG);
			AuAtomPos tMe(Pos());
			tMe.RotateZ(rTurnZ);									// Rotate the matrix.
			ChangePosAndOrient(tMe);

			// Rotate my hit notification by the reverse of my body rotation.
			m_PersonLock.lockForWrite();
			m_HitNotify.RotateZ(-rTurnZ);
			m_PersonLock.unlock();

			SetModified();
		}

		// When this atom is not touching ground, its movement vector must be
		// held in the direction it was going, sort of like real inertia.
		if(rotvec)
		{
			AuPoint nuVec = Transform().UnProjectVector(oldVec);
			SetVector(nuVec);
		}
		AuPoint nuVec = Transform().UnProjectVector(ogn);
		SetGravityNormal(nuVec);
		nuVec = Transform().UnProjectVector(otn);
		SetTerrainNormal(nuVec);
	}

	if(mount)
	{
		if(acti & (ACT_MoveLeft|ACT_MoveRight))
		{
			AuPoint oldVec;
			bool rotvec = false;
			if(!mount->CanControlMovement())
			{
				oldVec = mount->Transform().ProjectVector(mount->Vector());
				rotvec = true;
			}
			AuPoint otn = mount->Transform().ProjectVector(mount->TerrainNormal());
			AuPoint ogn = mount->Transform().ProjectVector(mount->GravityNormal());

			// Mount takes lateral moves as turns.
			float rMaxTurnMount = mount->TurnRate() * rsec;			// Max turn angle in this time period.
			if(rTurnMountZ > rMaxTurnMount)
				rTurnMountZ = rMaxTurnMount;
			else if(rTurnMountZ < -rMaxTurnMount)
				rTurnMountZ = -rMaxTurnMount;

			if(acti & ACT_MoveLeft)
			{
				//qDebug("Turning mount left %g degrees", rTurnMountZ*RAD2DEG);
				AuAtomPos tMe(mount->Pos());
				tMe.RotateZ(rTurnMountZ);
				mount->ChangePosAndOrient(tMe);
			}
			else if(acti & ACT_MoveRight)
			{
				//qDebug("Turning mount right %g degrees", rTurnMountZ*RAD2DEG);
				AuAtomPos tMe(mount->Pos());
				tMe.RotateZ(-rTurnMountZ);
				mount->ChangePosAndOrient(tMe);
			}

			// When this atom is not touching ground, its movement vector must be
			// held in the direction it was going, sort of like real inertia.
			if(rotvec)
			{
				AuPoint nuVec = mount->Transform().UnProjectVector(oldVec);
				mount->SetVector(nuVec);
			}
			AuPoint nuVec = mount->Transform().UnProjectVector(ogn);
			mount->SetGravityNormal(nuVec);
			nuVec = mount->Transform().UnProjectVector(otn);
			mount->SetTerrainNormal(nuVec);
		}

		// Orient to the terrain under my feet.
		mount->Orient(mount->Location(), mount->Transform().ProjectVector(mount->TerrainNormal()));
		mount->SetModified();
	}

	// Orient to the terrain under my feet.
	Orient(Location(), Transform().ProjectVector(TerrainNormal()));

	if(IsTurretIntegratedPose())
	{
		// Update parent vessel with my turret's new position.
		DaCapsule* vessel = dynamic_cast<DaCapsule*>(Parent());
		if(vessel)
			vessel->PositionTurret(*this);
	}
}

void DaPerson::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaSentient::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

	AnimatePose(HasItemInHand(), MovementSpeedMod());

	m_CurrentLight = CurrentLightThatIsOn();

	// Remaining logic only applies to the scene atom.
	if(sceneAtom != this)
		return;

	// Current weather conditions.
	m_WxRelHumidity = 0;
	m_WxOAT = -273;	// = 0 Kelvin
	m_WxBAR = 0;
	if(IsInsideEnclosure())
	{
		m_WxRelHumidity = .6f;
		m_WxOAT = 20;
		m_WxBAR = 1;
	}
	else if(IsInAtmosphere())
	{
		const DaWorldLL* world = FindParentWorldInEnv();
		if(world)
			world->WeatherAt(GlobeLL(), AltitudeMSL(), m_WxRelHumidity, m_WxOAT, m_WxBAR);
	}

	/*
#if !defined(QT_NO_DEBUG)
	const DaWorld* world = FindParentPlanetInEnv();
	if(world)
	{
		AuMatrix4x4 m;
		GetTransformRelativeToParent(m, world);
		AuPoint ploc = m.Translation();			// My location relative to the world.
		AuPoint gdir = world->GravityDir(ploc);		// Gravity vector relative to the world.
		AuPoint gdirme = m.UnProjectVector(gdir);	// Gravity vector relative to me.
		qDebug() << "DaPerson::AnimateDeviceObjects" << Id() << Name() << "Gw" << gdir << "Gme" << gdirme;
	}
#endif
*/

	// Only the scene atom animates their console, otherwise every dude at a console will be doing this work in my scene.
	if(HasConsole())
	{
		if(IsInCapsule())
		{
			// Operating a spacecraft console.
			const DaCapsule* cap = dynamic_cast<const DaCapsule*>(Parent());
			if(cap)
			{
				// Verify the console is operational.
				AuSysId sysId(StationSysId());
				if(sysId.System() == SYS_Count || cap->IsConsoleOperational(sysId))
				{
					DpStation* console = LockConsoleForRead();
					const DaVessel* ves = dynamic_cast<const DaVessel*>(cap);
					const DaBd* bd = dynamic_cast<const DaBd*>(cap);
					if(ves)
						console->AnimateDeviceObjects(*ari.re, *ves, lastAnim, Location(), *this);
					else if(bd)
						console->AnimateDeviceObjects(*ari.re, *bd, lastAnim, Location(), *this);
					UnlockConsole();
				}
			}
		}
	}

	QueryEnvironmentLighting();

	// Person atoms must update their environment vector for proper precipitation movement relative to them and their vehicle.
	QueryEnvironmentVector();

	QueryAudioEnvironment();

	// Calculate frame rate.
	s_FrameMsLast = lastAnim.elapsed();

	// Count actual frames rendered per second.
	int sec = QTime::currentTime().second();
	if(sec != s_CurSecond)
	{
		s_CurSecond = sec;
		s_Frames[s_CurSecond] = 0;	// Init frames in nu current second.
	}
	s_Frames[s_CurSecond]++;	// Increment frames in current second.
}

// This function is only called for the scene atom.
void DaPerson::QueryAudioEnvironment()
{
	if(!IsInsideSpacecraft())
		return;

	const DaCapsule* ship = FindParentCapsuleInEnv();
	if(ship)
	{
		AuPoint myloc;
		GetLocationRelativeToParent(myloc, ship);
		ship->ClosestAudibleUnits(myloc, m_ClosestGravityDrive, m_ClosestPowerPlant, m_ClosestSensor, m_ClosestFTLDrive,
				m_ClosestSiren);
	}
}

void DaPerson::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(IsInVehicle())
		return;	// Don't add lights for myself or any subatoms when inside an enclosed vehicle.
	bool isSceneAtom = Is(person.Id());
	if(m_CurrentLight.IsValid() && (isSceneAtom || !(m_AgentBits & (AGENT_Invisible|AGENT_Animal))))
	{
		// I am equipped with a light and I am the scene atom, which means it is my own scene I am lighting, or I am not an agent
		// who is invisible or an animal.
		// Agent: my light will not be visible to other players when I am invisible or animal, but I can use my light to see.
		AuGLLight::ROLE lightRole = isSceneAtom ? AuGLLight::ScenePerson : AuGLLight::Person;
		if(m_CurrentLight.IsBody())
		{
			// Entity glow is on.
			float lightFlicker = AuItem::LightFlicker(CID_StoneFirePit, Id());
			if(IsInsideCapsule())
				lightFlicker /= 4.f;		// Tone down lights a lot when aboard a spacecraft or inside a building.

			AuPoint lightPos = model.Translation();
			QColor colLight;
			if(IsDyingHighborn() || IsNewbornHighborn())
			{
				// Color the entity according to gender.
				switch(Gender())
				{
				case Female:
					colLight = QColor(255, 174, 174);	// Pink
					break;
				case Male:
					colLight = QColor(174, 174, 255);	// Blue
					break;
				case Asexual:
					colLight = QColor(255, 255, 174);	// Yellow
					break;
				case Hermaphrodite:
					colLight = QColor(174, 255, 174);	// Green
					break;
				}
			}
			else
			{
				const DgData& gd = g_Au.LockDataForRead();
				colLight = gd.EmpireContrailColor(EmpireId());
				g_Au.UnlockData();
			}
			float lightRadius = 15.f * FeetPerMeter() * lightFlicker;
			float kQ = 1.f/(lightRadius * lightRadius);
			float kL = .0065f;
			lights.append(AuGLLight(Id(), lightRole, kL,kQ, colLight, lightPos));
		}
		else if(m_CurrentLight.IsGear())
		{
			// Personal lamp is on.
			m_GearLock.lockForRead();
			const AuItem* item = m_Gear.ItemAt(m_CurrentLight);
			if(item)
			{
				// Environment suit light stops working at 1 hp of health, when the seal also breaks.
				if(!item->IsEnvironmentSuit() ||
						item->HitPoints() > 1)
				{
					// Calculate flickering light effect for torch.
					float lightFlicker = item->LightFlicker(Id());
					if(IsInsideCapsule())
						lightFlicker /= 4.f;		// Tone down lights a lot when aboard a spacecraft or inside a building.

					float lightHt = .5f;	// 6" above my eye camera.
					AuPoint lightPos = model.Project(CameraOffset() + AuPoint(0,0,lightHt));
					QColor colLight = item->LightColor();
					float lightRadius = item->LightRadius() * lightFlicker;
					float kQ = 1.f/(lightRadius * lightRadius);
					float kL = .0065f;
					if(item->IsArmorHelmetClothingOrHat())
					{
						float cosInnerCone = .939692621f;		// 20 degrees
						float cosOuterCone = .766f;				// 40 degrees
						AuPoint lightVec = model.ProjectVector(AuPoint(0,1,0).RotateX(CurrentHeadTilt()));
						lights.append(AuGLLight(Id(), lightRole, kL,kQ, colLight, lightPos, cosOuterCone, cosInnerCone, lightVec));
					}
					else
						lights.append(AuGLLight(Id(), lightRole, kL,kQ, colLight, lightPos));
				}
			}
			m_GearLock.unlock();
		}
	}
	DaSentient::GetRenderLightsRecursive(person, model, suns, lights);
}

bool DaPerson::AddGearItemNoLock(AuGear& gear, const AuItem& cItem)
{
	bool v = DaSentient::AddGearItemNoLock(gear, cItem);
	if(v)
		Stats().ItemPossessed(cItem);	// Player Statistics
	return v;
}

void DaPerson::InitGearDesigner()
{
	/*
	AuItem* clothes = AuItem::Create(CID_JeansAndShirt, QualityBaseline(), EmpireId());

	// Put the items in my inventory. Leave my hands empty so the player can pick up a couple of things easily.
	m_GearLock.lockForWrite();
	if(clothes)
	{
		m_Gear.Insert(AuGearSlot::BodyWear, 0, *clothes);
		delete clothes;
	}
	m_WeightCarried = m_Gear.Weight();
	m_GearLock.unlock();
	*/

	AuItem* candle = AuItem::Create(CID_Candle, QualityBaseline(), EmpireId());
	AuItem* lantern = AuItem::Create(CID_Lantern, QualityBaseline(), EmpireId());

	// Put the items in my inventory. Leave my hands empty so the player can pick up a couple of things easily.
	m_GearLock.lockForWrite();
	if(candle)
	{
		m_Gear.Insert(AuGearSlot::Hand, 0, *candle);
		delete candle;
	}
	if(lantern)
	{
		m_Gear.Insert(AuGearSlot::Hand, 1, *lantern);
		delete lantern;
	}
	m_WeightCarried = m_Gear.Weight();
	m_GearLock.unlock();

	m_PersonLock.lockForWrite();

	// Bank account balance. Money is easy enough to get with a little hard work so there's no reason to start with any.
	m_AccountBalance = 0;
	m_LoanBalance = 0;

	m_PersonLock.unlock();
}

void DaPerson::InitGearAvatar()
{
	switch(InitScenario())
	{
	case ReachForTheStars:
		// Start with a knife and a welcome letter.
		{
			// Knife
			AuItem* knife = AuItem::Create(CID_Knife, QualityBaseline(), EmpireId());
			if(knife)
			{
				knife->Inscribe(QString("<i>%1</i>").arg(Name()));

				// Player Statistics
				Stats().ItemPossessed(*knife);
			}

			// Note
			const DgData& gd = g_Au.LockDataForRead();
			quint32 imperiumId = gd.EmpireId("Imperium");
			g_Au.UnlockData();
			AuItem* note = AuItem::Create(CID_Paper, 255, imperiumId);
			if(note)
			{
				note->Inscribe(tr("<p>"
							"%1,"
							"</p>"
							"<p>"
							"Welcome to <i>%2</i>."
							"</p>"
							"<p>"
							"The environment provides everything you need."
							" Your hand cursor forages from rocks, shrubs and trees."
							" Use the Gear window to handcraft things."
							" Handcraft a fire to ward off wild animals."
							"</p>"
							"<p>"
							"Take time to explore the area; there is no hurry."
							" Forage and handcraft useful items, such as a backpack and a medical kit."
							" Barter items with the people of your village."
							" The healer gives first aid."
							"</p>"
							"<p>"
							"Find a place where there is metal and oil nearby."
							" Top-down view (Backspace) shows resources; backslash (\\) changes what is shown."
							" Use the Construction window to build buildings."
							" Build a <i>mine</i> to gather stone; use the <b>Quarry by Haxus</b> blueprint."
							" Build a <i>farm</i> for food and shelter; <b>Pioneer Cabin by Haxus</b> is a good start."
							" Build <i>roads</i> to connect buildings that are far apart. "
							"</p>"
							"<p>"
							"Food and equipment is bought, sold and bartered at buildings."
							" <i>Retail stores</i> sell most non-food items."
							" <i>Cantinas</i> sell food and drinks."
							" <i>Grocers</i> sell food, drinks and medical supplies."
							" Buildings with a store will sell what they make and buy what they use."
							" Stand at a building and use the Building window to access its store."
							"</p>"
							"<p>"
							"<b>Handcraft a flag to form a government.</b>"
							" Build a <i>capitol</i> to be the seat of government."
							" Stand at the capitol to get reports and manage construction and manufacturing throughout the city."
							" Build a <i>police station</i> to enforce the boundary of the city."
							" Build a <i>bank</i> to mint money."
							"</p>"
							"<p>"
							"Build an <i>aircraft factory</i> to make space rockets."
							" Go to the aircraft factory and look at the Building window."
							" Build industries to make the things needed to complete construction and make the rocket."
							" Make hydrogen at a <i>refinery</i> to fuel the rocket."
							" Make environment suits and helmets at an <i>armorer</i> for protection in space."
							" <b>When the city has rockets, fuel, and environment suits and helmets,"
							" Rocket Training begins automatically.</b>"
							"</p>"
							"<p>"
							"Use the Cities and Bases window. "
							" Right-click a city to request reports."
							" Reports help manage a city."
							"</p>"
							"<p>"
							"Thanks for playing,"
							"</p>"
							"<p>"
							"<i>Haxus</i>"
							"<br>Architect of Hazeron"
							"</p>")
						.arg(Name())
						.arg(g_Au.GameName()));

				// Player Statistics
				Stats().ItemPossessed(*note);
			}

			// Put the items in my inventory. Leave my hands empty so the player can pick up a couple of things easily.
			AuGearPos knifePos;
			m_GearLock.lockForWrite();
			if(knife)
			{
				m_Gear.Insert(AuGearSlot::Waist, m_Gear.NextWhich(AuGearSlot::Waist), *knife);
				knifePos = m_Gear.FindItemPos(*knife);
				delete knife;
			}
			if(note)
			{
				m_Gear.Insert(AuGearSlot::Waist, m_Gear.NextWhich(AuGearSlot::Waist), *note);
				delete note;
			}
			m_WeightCarried = m_Gear.Weight();
			m_GearLock.unlock();

			// Configure to use the knife in weapon mode. This may help the nubie live long enough to figure out what's happening.
			m_PersonLock.lockForWrite();
			m_CurrentGearPos = knifePos;
			m_CurrentGearSub = IPS_Weapon1;
			m_PersonLock.unlock();
		}
		break;

	case CitizenOfTheGalaxy:
		// Start with a uniform and an EV suit, if the starting environment is not breathable or in space.
		{
			COMMODITYID clothingCID;
			if(Gender() == Female)
				clothingCID = CID_CapriHalterOutfit;
			else
				clothingCID = CID_JeansAndShirt;		// Jeans and tee shirt.
			AuItem* clothing = AuItem::Create(clothingCID, QualityBaseline(), EmpireId());

			// Player Statistics
			Stats().ItemPossessed(*clothing);

			AuItem* evsuit = 0;
			AuItem* evhelmet = 0;
			if(!IsAirBreathable() || !IsInAtmosphere())
			{
				evsuit = AuItem::Create(CID_VacSuit, QualityBaseline(), EmpireId());
				evhelmet = AuItem::Create(CID_VacSuitHelmet, QualityBaseline(), EmpireId());

				// Player Statistics
				Stats().ItemPossessed(*evsuit);
				Stats().ItemPossessed(*evhelmet);
			}

			// Put the items in my inventory. Leave my hands empty so the player can pick up a couple of things easily.
			m_GearLock.lockForWrite();
			m_Gear.Insert(AuGearSlot::BodyWear, m_Gear.NextWhich(AuGearSlot::BodyWear), *clothing);
			delete clothing;
			if(evsuit)
			{
				m_Gear.Insert(AuGearSlot::BodyWear, m_Gear.NextWhich(AuGearSlot::BodyWear), *evsuit);
				delete evsuit;
			}
			if(evhelmet)
			{
				m_Gear.Insert(AuGearSlot::Head, m_Gear.NextWhich(AuGearSlot::Head), *evhelmet);
				delete evhelmet;
			}
			m_WeightCarried = m_Gear.Weight();
			m_GearLock.unlock();
		}
		break;

	case SavageState:
		// Start with nothing.
		break;

	case LostAtlantis:
		// Start with a uniform and an EV suit.
		{
			COMMODITYID clothingCID = CID_RedUniform;
			AuItem* clothing = AuItem::Create(clothingCID, QualityBaseline(), EmpireId());
			AuItem* evsuit = AuItem::Create(CID_VacSuit, QualityBaseline(), EmpireId());
			AuItem* evhelmet = AuItem::Create(CID_VacSuitHelmet, QualityBaseline(), EmpireId());

			// Player Statistics
			Stats().ItemPossessed(*clothing);
			Stats().ItemPossessed(*evsuit);
			Stats().ItemPossessed(*evhelmet);

			// Put the items in my inventory. Leave my hands empty so the player can pick up a couple of things easily.
			m_GearLock.lockForWrite();
			m_Gear.Insert(AuGearSlot::BodyWear, m_Gear.NextWhich(AuGearSlot::BodyWear), *clothing);
			delete clothing;
			m_Gear.Insert(AuGearSlot::BodyWear, m_Gear.NextWhich(AuGearSlot::BodyWear), *evsuit);
			delete evsuit;
			m_Gear.Insert(AuGearSlot::Head, m_Gear.NextWhich(AuGearSlot::Head), *evhelmet);
			delete evhelmet;
			m_WeightCarried = m_Gear.Weight();
			m_GearLock.unlock();
		}
		break;

	case SCENARIO_Count:
		break;
	}

	// Bank account balance. Money is easy enough to get with a little hard work so there's no reason to start with any.
	m_PersonLock.lockForWrite();
	m_AccountBalance = 0;
	m_LoanBalance = 0;
	m_PersonLock.unlock();
}

bool DaPerson::TestMoveGear(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType) const
{
	m_GearLock.lockForRead();
	bool v = TestMoveGear(srcPos, destSlotType, m_Gear);
	m_GearLock.unlock();
	return v;
}

//static
bool DaPerson::TestMoveGear(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType, const AuGear& gear)
{
	// Check for a valid item at the source position.
	const AuItem* srcItem = gear.ItemAt(srcPos);
	if(!srcItem)
		return false;
	return AuGearSlot::CanHoldItem(destSlotType, *srcItem);
}

void DaPerson::MoveGear(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType)
{
	if(IsPatientStationPose() &&
			(destSlotType == AuGearSlot::BodyWear || destSlotType == AuGearSlot::Head))
		return;							// Person is in a surgery unit and dest location would cover the head or body.

	m_GearLock.lockForWrite();
	bool moved = m_Gear.MoveItem(srcPos, destSlotType);
	if(moved)
		UpdateGearWeight();	// This could cause me to start falling if I just took off my EVA suit.
	m_GearLock.unlock();
}

void DaPerson::MoveGear(const AuGearPos& srcPos, const AuGearPos& destPos)
{
	m_GearLock.lockForWrite();
	const AuItem* srcItem = m_Gear.ItemAt(srcPos);
	const AuItem* destItem = m_Gear.ItemAt(destPos);
	if(!srcItem || !destItem)
	{
		m_GearLock.unlock();
		return;
	}

	// Must get info about the items before we do the move because the pos indexes become invalid.
	QString srcDesc = srcItem->ListDesc();
	QString destName = destItem->SayName();
	COMMODITYID destCID = destItem->CommodityId();
	quint8 destQ = destItem->Quality();

	int moved = m_Gear.MoveItem(srcPos, destPos);
	if(moved)
	{
		if(moved == 2)
		{
			// Send a log message to the player.
			QString text = tr("%1 loaded with %2.")
				.arg(destName)
				.arg(srcDesc.toLower());
			Thought(text);
		}
		else if(moved == 3)
		{
			// Send a log message to the player.
			QString text = tr("%1 %2 planted with %3.")
				.arg(DbCommodity::QualityDesc(destQ))
				.arg(g_Au.CommodityName(destCID).toLower())
				.arg(srcDesc.toLower());
			Thought(text);
		}
		else if(moved == 11)
		{
		    // 10+1 = The data item exceeds the space available on this storage media.
			QString text = tr("Insufficient space on %1 to store %2.")
				.arg(g_Au.CommodityName(destCID).toLower())
				.arg(srcDesc.toLower());
			Thought(text);
		}
		else if(moved == 12)
		{
		    // 10+2 = The data item cannot be stored onto this storage media.
		}
		else if(moved == 13 ||
				moved == 14 ||
				moved == 15 ||
				moved == 21 ||
				moved == 24)
		{
		    // 10+3 = Source item patent stored successfully.
		    // 10+4 = Source item spacecraft blueprint stored successfully.
		    // 10+5 = Source item building blueprint stored successfully.
			// 10+11 = Source item mission stored successfully.
			// 10+14 = Source item paper stored successfully.
			QString text = tr("%1 stored to %2.")
				.arg(srcDesc)
				.arg(g_Au.CommodityName(destCID).toLower());
			Thought(text);
		}
		else if(moved == 16)
		{
		    // 10+6 = Source item data transferred.
			QString text = tr("%1 copied to %2.")
				.arg(srcDesc)
				.arg(g_Au.CommodityName(destCID).toLower());
			Thought(text);
		}
		else if(moved == 17)
		{
		    // 10+7 = Source item patent (or better) already contained on this storage media.
			QString text = tr("%2 already contains %1 (or better).")
				.arg(srcDesc.toLower())
				.arg(g_Au.CommodityName(destCID));
			Thought(text);
		}
		else if(moved == 18 ||
				moved == 23)
		{
		    // 10+8 = Source item blueprint already contained on this storage media.
			// 10+13 = Source item paper already contained on this storage media.
			QString text = tr("%2 already contains %1.")
				.arg(srcDesc.toLower())
				.arg(g_Au.CommodityName(destCID));
			Thought(text);
		}
		else if(moved == 19)
		{
			// 10+9 = Data item cannot be copied due to publisher restrictions.
			QString text = tr("Publisher restriction prevents copying %1 to %2.")
				.arg(srcDesc.toLower())
				.arg(g_Au.CommodityName(destCID));
			Thought(text);
		}
		else if(moved == 20)
		{
			// 10+10 = Data on one of the items in the 'dataItem' storage container cannot be copied due to publisher restrictions.
			QString text = tr("Publisher restriction prevents copying one or more of the data items from %1 to %2. "
					"Nothing was copied.")
				.arg(srcDesc.toLower())
				.arg(g_Au.CommodityName(destCID));
			Thought(text);
		}
		else if(moved == 22)
		{
			// 10+12 = Source item paper is blank.
			QString text = tr("Blank paper cannot be stored.");
			Thought(text);
		}

		// Weight can change when ammo is consumed. This could also cause me to start falling if I just took off my EVA suit.
		UpdateGearWeight();
	}
	m_GearLock.unlock();
}

int DaPerson::UseItem(AuItem& item, INVENTORYSUB sub)
{
	int rounds = item.AmmoLoaded(sub);
	int useResult = DaSentient::UseItem(item, sub);
	if(useResult == 2 || rounds != item.AmmoLoaded(sub))
	{
		// Condition degraded or ammo expended so client needs to know.
		m_tmGearChange.Set();
		SetModified();
	}
	return useResult;
}

// Return labor boost provided by a particular component id.
// The component must be in a "useable" position. i.e. not in a pack.
int DaPerson::GetLaborBoost(COMMODITYID cid)
{
	m_GearLock.lockForRead();
	AuGearPos pos = m_Gear.FindReadyItem(cid);
	const AuItem* item = m_Gear.ItemAt(pos);
	if(!item)
	{
		m_GearLock.unlock();
		return 0;						// Not likely.
	}

	int boost = item->GetLaborBoost();
	m_GearLock.unlock();

	UseGearItem(pos, IPS_Item);

	return boost;
}

// Return production boost provided by a particular component id.
// The component must be in a "useable" position. i.e. not in a pack.
int DaPerson::GetProductionBoost(COMMODITYID cid)
{
	m_GearLock.lockForRead();
	AuGearPos pos = m_Gear.FindReadyItem(cid);
	const AuItem* item = m_Gear.ItemAt(pos);
	if(!item)
	{
		m_GearLock.unlock();
		return 0;						// Not likely.
	}

	int boost = item->GetProductionBoost();
	m_GearLock.unlock();

	UseGearItem(pos, IPS_Item);

	return boost;
}

// Try to pick up an atom and add it to my inventory.
// Return true if I pick up the atom.
bool DaPerson::PickUp(DaAtom *pAtom)
{
	DaItem *pc = dynamic_cast<DaItem *>(pAtom);
	if(!pc)
		return false;	// At present I can only pick up commodities.

	const AuItem* item = pc->LockItem();
	if(!item)
	{
		pc->UnlockItem();
		return false;
	}
	AuItem* addItem = item->Clone();
	pc->UnlockItem();
	if(!addItem)
		return false;

	// Add the item to this atom's inventory if I have room.
	if(AddGearItem(*addItem))
	{
		// The item fit into my inventory somewhere.
		pc->Kill();					// Delete the item from the environment.

		QString text(tr("%1 grabbed.").arg(addItem->SayName()));
		Thought(text);
		delete addItem;
		return true;
	}
	else
	{
		// The item would not fit into my inventory.
		QString text(tr("I have no room to carry %1.").arg(addItem->SayName()));
		Thought(text);
		delete addItem;
		return false;
	}
}

// Throw the top level item.
void DaPerson::ThrowCurrentItem()
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Cannot throw items from inside an enclosed vehicle.
	if(IsInVehicle())
		return;

	// Item must be valid.
	AuGearPos gearPos(CurrentItemPos());
	m_GearLock.lockForWrite();
	const AuItem* gearItem = m_Gear.ItemAt(gearPos);
	if(!gearItem)
	{
		m_GearLock.unlock();
		return;
	}

	// A baseball is .33 lbs. A good pitch of 90 mph is about 132 fps.
	// A throw force of 43.56 ft lbs per second would achieve that.
	// Wikipedia: The mechanical horsepower, also known as imperial horsepower, of exactly 550 foot-pounds per second is
	//            approximately equivalent to 745.7 watts.
	// Default human has a DNA strength of 5, in the range 0..15.
	double throwSpeed = ThrowForce() / gearItem->Weight();
	AuPoint throwVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());
	AuPoint throwFrom = CameraOffset() + throwVec;			// Aiming works best if the object comes from in front of my eye.
	throwVec *= throwSpeed;

	if(!IsAboardVehicle())
	{
		// Check for an unobstructed path from my insertion point to the throw point.
		// This test is not performed when I am aboard an open vehicle, such as a dirt bike or sail boat, otherwise the vehicle
		// itself blocks the IsPathBlocked test.
		// NOTE: This could be exploited by a person aboard a dirt bike with their 'throwFrom' location inside the hull of a ship.
		AuPoint hitLoc, hitNormal;								// Unneeded output parameter of IsPathBlocked.
		if(Parent()->IsPathBlocked(Location(), Transform().Project(throwFrom), hitLoc, hitNormal, this))
		{
			m_GearLock.unlock();
			return;
		}
	}

	// Find a suitable parent atom for the item. Don't throw it inside a vehicle or onto my mount.
	DaAtom* itemParent = Parent();
	while(dynamic_cast<DaCreature*>(itemParent) || dynamic_cast<DaVehicle*>(itemParent))
		itemParent = itemParent->Parent();

	// Create a commodity atom to encapsulate the inventory item.
	DaItem* item = new DaItem(*gearItem);
	item->ConnectSignals();
	item->Create();

	// Position the item in the environment.
	AuMatrix4x4 tme;
	GetTransformRelativeToParent(tme, itemParent);
	tme.Translate(throwFrom);									// Transform to position the item at my hand throw location.
	item->SetPosAndOrient(AuAtomPos(itemParent->Id(), tme));
	ether->AddAtom(item);

	item->SetVector(Vector() + throwVec);						// Give the item a movement vector.
	item->SetInMotion();

	// Arm detonator and fuse weapons when thrown.
	if(gearItem->IsDetonatorWeapon())
	{
		item->ArmWeapon(Id(), Name(), EmpireId());							// Wait for detonator signal.
		QString text(tr("%1 armed for detonation when I threw it.").arg(item->Name()));
		Thought(text);
	}
	else if(gearItem->IsFuseWeapon())
	{
		item->ArmWeapon(Id(), Name(), EmpireId());							// Start fuse timer.
		QString text(tr("%1 fuse initiated when I threw it.").arg(item->Name()));
		Thought(text);
	}

	// Eliminate my inventory item.
	m_Gear.Erase(gearPos);
	UpdateGearWeight();
	m_GearLock.unlock();

	// Notify the world of the nu atom.
	g_Au.AtomAdded(item->Id());

	item->SetInScene();					// Don't decay too fast right away.
	item->SetNormal();					// New atom is ready for normal thread processing.
	item->TickTock();					// Get it moving right away.
}

// Throw the top level item.
void DaPerson::DropCurrentItem()
{
	DaItem* item = DropGearAt(CurrentItemPos());
	if(item)
	{
		QString text(tr("%1 dropped.").arg(item->Name()));
		Thought(text);
	}
}

// Inscribes the item that is in the 'pos' position.
// 'subPos' specifies a position within a container to inscribe. It will be (255,255)
// when invalid or when the container should be inscribed.
void DaPerson::InscribeGearAt(const AuGearPos& pos, const QString& inscription)
{
	if(!pos.IsGear())
		return;

	m_GearLock.lockForWrite();
	AuItem* item = m_Gear.ItemAt(pos);
	if(!item || !item->Inscription().isEmpty())
	{
		m_GearLock.unlock();
		return;
	}

	item->Inscribe(inscription);
	m_tmGearChange.Set();
	SetModified();
	m_GearLock.unlock();
}

// Sets the message text on the item that is in the 'pos' position.
// 'subPos' specifies a position within a container to inscribe. It will be (255,255)
// when invalid or when the container should be inscribed.
void DaPerson::SetMessage(const AuGearPos& pos, const QString& message)
{
	if(!pos.IsGear())
		return;

	m_GearLock.lockForWrite();
	AuItem* item = m_Gear.ItemAt(pos);
	if(!item)
	{
		m_GearLock.unlock();
		return;
	}

	item->SetMessage(message);
	m_tmGearChange.Set();
	SetModified();
	m_GearLock.unlock();
}

// 'subPos' specifies a position within a container. It will be (255,255) when invalid or when the container is the target.
void DaPerson::SetPictureFrame(const AuGearPos& pos, const QString& url, bool hideURL, bool rotate)
{
	if(!pos.IsGear())
		return;

	m_GearLock.lockForWrite();
	AuItem* item = m_Gear.ItemAt(pos);
	if(!item)
	{
		m_GearLock.unlock();
		return;
	}

	AuItemPicture* pic = dynamic_cast<AuItemPicture*>(item);
	if(pic)
	{
		// Set the picture url.
		pic->SetPictureURL(url, hideURL);
		pic->SetRotatePicture(rotate);
		m_tmGearChange.Set();
		SetModified();
	}
	m_GearLock.unlock();
}

// 'subPos' specifies a position within a container. It will be (255,255) when invalid or when the container is the target.
void DaPerson::SetFlowerPot(const AuGearPos& pos, quint8 plantSize)
{
	if(!pos.IsGear())
		return;

	m_GearLock.lockForWrite();
	AuItem* item = m_Gear.ItemAt(pos);
	if(!item)
	{
		m_GearLock.unlock();
		return;
	}

	AuItemFloraDNA* pot = dynamic_cast<AuItemFloraDNA*>(item);
	if(pot)
	{
		// Set the picture url.
		pot->SetPottedSize(plantSize);
		m_tmGearChange.Set();
		SetModified();
	}
	m_GearLock.unlock();
}

// Uses the item that is in the 'pos' position.
bool DaPerson::UseGearAt(const AuGearPos& pos)
{
	// Cannot use inventory items when comatose.
	if(IsBodyDyingOrDead())
		return false;

	if(!pos.IsGear())
		return false;

	bool itemUsedUp = false;
	m_GearLock.lockForWrite();
	AuItem* item = m_Gear.ItemAt(pos);
	if(item)
	{
		// Use the item.
		quint32 oldCount = item->Count();
		AuItem* replacementItem = UseItemOnTarget(item, Id(), 0, AuPoint(0,0), false);		// Use the item on myself.
		if(replacementItem != item)
		{
			// The item must be replaced by a nu item.
			m_Gear.ReplaceItem(pos, replacementItem);
			itemUsedUp = true;

			// Gear weight changed.
			UpdateGearWeight();
		}
		else if(item->Count() != oldCount)
		{
			if(item->Count() == 0)
			{
				m_Gear.Erase(pos);	// The item was used up.
				itemUsedUp = true;
			}

			// Gear weight changed.
			UpdateGearWeight();
		}
	}
	m_GearLock.unlock();
	return itemUsedUp;
}

void DaPerson::SensorScan(AuSensorBlipList& blips, const AuPoint& /*scannerLoc*/, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float /*scannerSensorRange*/, float /*scannerWeaponRange*/, int scanContent, const AuId& /*lockedBlip*/,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	// I am only scanned for AI turret gunners and transporter consoles.
	if(!(scanContent & 0x0024))
		return;

	// Only my own ship, that I hold a berth aboard, can detect me on transporter sensors.
	if(SpawnShip().IsNull() || SpawnShip() != scannerId)
		return;

	// Dying creatures do not show up on sensors.
	if(IsBodyDyingOrDead())
		return;

	if(IsEthereal())
		return;

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	// I hold a berth aboard the scanning ship. Return a blip that corresponds to my berth type.
	POLITICAL_STANCE stance = PoliticalStance(scannerEmpire, EmpireId(), stanceCache);
	switch(SpawnShipBerthType())
	{
	default:
		blips.Add(AuBlipAvatar(Name(), Species(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	case AuRosterEntry::Captain:
	case AuRosterEntry::CaptainSlot:
	case AuRosterEntry::OfficerHuman:
		blips.Add(AuBlipOfficer(Name(), Species(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	case AuRosterEntry::Crew:
		blips.Add(AuBlipCrew(Name(), Species(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	case AuRosterEntry::PassengerHuman:
		blips.Add(AuBlipPassenger(Name(), Species(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	case AuRosterEntry::Troop:
		blips.Add(AuBlipTroop(Name(), Species(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	}
}

// Insert first time database record.
void DaPerson::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	// Are we saving the selfie?
	QString selfieField, selfieQ;
	m_PersonLock.lockForRead();
	if(!m_Selfie.isNull())
	{
		selfieField = ",selfie";
		selfieQ = ",?";
	}
	m_PersonLock.unlock();

	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO a_Person (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
				"timeCreate,timeModified,timeTock,timeDeath,"
				"name,allegiance,arenaTeam,conditions,damageHistory,"
				"hp,sex,dna,dna4,hair,shave,"
				"islefty,pose,whichConsole,stationId,atRest,vecX,vecY,vecZ,stamina,hunger,autoEat,seatIndex,"
				"minsToFullGrown,bodyQ,orders,posture,"
				"bank,loan,agentBits,arenaEntered,arenaExitLocX,arenaExitLocY,arenaExitLocZ,arenaExitParent,arenaShipDesign,"
				"usepos,usesub,social,worms,designpass,spawncity,spawnCityName,spawnSystem,spawnSystemName,"
				"spawnx,spawny,spawnship,spawnShipIsPrivate,spawnShipName,berthType,inv,seaChest,rewards,online,"
				"lastOnLine,skills,lineage,dad,mom,accountInactive,con,jobs,memory,stats,log,logtime,script,domain,"
				"mappos%1) VALUES"
				"(?,?,?,?,?,?,?,?,"
				"?,?,?,?,"
				"?,?,?,?,?,"
				"?,?,?,?,?,?,"
				"?,?,?,?,?,?,?,?,?,?,?,?,"
				"?,?,?,?,"
				"?,?,?,?,?,?,?,?,?,"
				"?,?,?,?,?,?,?,?,?,"
				"?,?,?,?,?,?,?,?,?,?,"
				"?,?,?,?,?,?,?,?,?,?,?,?,?,?,"
				"?%2);").arg(selfieField).arg(selfieQ);
	qa.prepare(sq);
	qa.addBindValue(Id().GetID());
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(CreatedTime().UTC());
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaUnit values.
	qa.addBindValue(AssignedName());
	qa.addBindValue(Allegiance());
	qa.addBindValue(ArenaTeam());
	{
		// Conditions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << LockConditions();
		UnlockConditions();
		qa.addBindValue(ba);
	}
	{
		// Unit Damage History
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		const AuUnitDamageHash& ud = LockUnitDamage();
		out << (quint32)ud.size();
		for(AuUnitDamageHash::const_iterator di = ud.begin(); di != ud.end(); di++)
		{
			out << (*di).first;
			out << (*di).second;
		}
		UnlockUnitDamage();
		qa.addBindValue(ba);
	}

	// DaCreature values.
	qa.addBindValue(CurrentBodyHitPoints());
	qa.addBindValue((Chromatid() == AuDNABase::Alpha) ? AuDNABase::Beta : AuDNABase::Alpha);	// reversed for old gender values
	{
		// Clear old dna field.
		QByteArray ba;
		qa.addBindValue(ba);

		// Nu dna.
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		AuDNABase* dna = DNAnew();
		dna->WriteDynamic(out);
		delete dna;
		qa.addBindValue(ba);
	}
	qa.addBindValue(Haircut());
	qa.addBindValue(Shave());
	qa.addBindValue(IsLeftHanded());
	qa.addBindValue(CurrentGeneralPose().Pose());
	qa.addBindValue(ControlId());
	qa.addBindValue(StationId());
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Stamina);
	qa.addBindValue(CurrentHunger());
	qa.addBindValue(m_AutoEat);
	qa.addBindValue(m_SeatIndex);
	qa.addBindValue(m_MinutesToFullGrown);
	qa.addBindValue(m_BodyQ);
	{
		// Orders
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		LockOrdersForRead() >> out;
		UnlockOrders();
		qa.addBindValue(ba);
	}
	qa.addBindValue(Posture());

	// DaPerson values.
	qa.addBindValue(FuzzyClamp(m_AccountBalance));
	qa.addBindValue(FuzzyClamp(m_LoanBalance));
	qa.addBindValue(m_AgentBits);
	qa.addBindValue(m_ArenaEntered.UTC());
	qa.addBindValue(FuzzyClamp(m_ArenaExitLoc.x));
	qa.addBindValue(FuzzyClamp(m_ArenaExitLoc.y));
	qa.addBindValue(FuzzyClamp(m_ArenaExitLoc.z));
	qa.addBindValue(m_ArenaExitParent.GetID());
	qa.addBindValue(m_ArenaShipDesign);
	qa.addBindValue(m_CurrentGearPos.EncodeU16());
	//qDebug() << "DaPerson::Insert" << Id() << Name() << "gpos" <<
		//QString("0x%1").arg(m_CurrentGearPos.EncodeU16(), 4, 16, QChar('0'));
	qa.addBindValue(m_CurrentGearSub);
	qa.addBindValue(m_Reputation);
	qa.addBindValue((short int)m_Worms);
	qa.addBindValue(m_DesignPass);
	qa.addBindValue(m_SpawnBd.GetID());
	qa.addBindValue(m_SpawnBdName);
	qa.addBindValue(m_SpawnBdSystem.GetID());
	qa.addBindValue(m_SpawnBdSystemName);
	qa.addBindValue(m_SpawnDevLL.Longitude());
	qa.addBindValue(m_SpawnDevLL.Latitude());
	qa.addBindValue(m_SpawnShip.GetID());
	qa.addBindValue(m_SpawnShipIsPrivate);
	qa.addBindValue(m_SpawnShipName);
	qa.addBindValue(m_BerthType);

	// Inventory
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_GearLock.lockForRead();
		m_Gear >> out;
		m_ArenaExitInv >> out;
		quint8 ver = 0x01;
		out << ver;
		out << m_PowerMoveMax;
		out << m_PowerJump;
		m_GearLock.unlock();
		qa.addBindValue(ba);
	}

	// Sea Chest
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_SeaChestLock.lockForRead();
		m_SeaChest >> out;
		m_SeaChestLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(FuzzyClamp(m_RewardsOwed));
	qa.addBindValue(AccountTypeOnLine());
	qa.addBindValue(m_tmLastOnLine.UTC());
	{
		// Skills
		QByteArray ba;
		const AuSkillSet& skills = LockSkillsForRead();
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		out << skills;
		UnlockSkills();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_Lineage);
	qa.addBindValue(m_Dad.GetID());
	qa.addBindValue(m_Mom.GetID());
	qa.addBindValue(m_AccountInactive);
	qa.addBindValue(m_Con);

	// Open jobs
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_JobsLock.lockForRead();
		out << m_Jobs;
		m_JobsLock.unlock();
		qa.addBindValue(ba);
	}

	// Memories
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_MemoryLock.lockForRead();
		out << m_Memories;
		m_MemoryLock.unlock();
		qa.addBindValue(ba);
	}

	// Statistics
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_Stats >> out;
		qa.addBindValue(ba);
	}

	{
		// Captain's Log
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_PersonLock.lockForRead();
		m_Log >> out;
		m_PersonLock.unlock();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_LogTime.UTC());

	{
		// Script
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		if(m_Script)
		{
			out << (quint8)1;
			out << *m_Script;
		}
		else
			out << (quint8)0;
		qa.addBindValue(ba);
	}

	{
		// Domain
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		m_PersonLock.lockForRead();
		out << m_Domain;
		m_PersonLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Hot key mapping
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		for(int mp=0; mp<10; mp++)
			m_MapPos[mp] >> out;
		qa.addBindValue(ba);
	}

	if(!selfieField.isEmpty())
	{
		// Save my selfie.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_PersonLock.lockForRead();
		out << m_Selfie;
		m_PersonLock.unlock();
		qa.addBindValue(ba);
	}

	if(!qa.exec())
	{
		QSqlError err = qa.lastError();
		qWarning("DaPerson::Insert %s FAILED. sq '%s' err '%s'",
				(const char*)Name().toLatin1(),
				(const char*)sq.toLatin1(),
				(const char*)err.text().toLatin1());
	}
}

// Update existing database record.
void DaPerson::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	// Are we saving the selfie?
	QString selfieField;
	m_PersonLock.lockForRead();
	if(!m_Selfie.isNull())
		selfieField = ", selfie=?";
	m_PersonLock.unlock();

	QString sq;
	sq = QString("UPDATE a_Person SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, "
			"timeModified=?, timeTock=?, timeDeath=?, "
			"name=?, allegiance=?, arenaTeam=?, conditions=?, damageHistory=?, hp=?, dna=?, dna4=?, hair=?, shave=?, pose=?, "
			"whichConsole=?, stationId=?, atRest=?, "
			"vecX=?, vecY=?, vecZ=?, stamina=?, hunger=?, autoEat=?, seatIndex=?, minsToFullGrown=?, bodyQ=?, orders=?, posture=?, "
			"bank=?, loan=?, "
			"agentBits=?, arenaEntered=?, arenaExitLocX=?, arenaExitLocY=?, arenaExitLocZ=?, arenaExitParent=?, arenaShipDesign=?, "
			"usepos=?, usesub=?, social=?, "
			"worms=?, designpass=?, spawncity=?, spawnCityName=?, spawnSystem=?, spawnSystemName=?, spawnx=?, "
			"spawny=?, spawnship=?, spawnShipIsPrivate=?, spawnShipName=?, berthType=?, "
			"rewards=?, online=?, lastOnLine=?, skills=?, lineage=?, dad=?, mom=?, accountInactive=?, con=?, "
			"inv=?, seaChest=?, jobs=?, "
			"memory=?, stats=?, log=?, logtime=?, script=?, domain=?, "
			"mappos=?%2 "
			"WHERE id=%1;").arg(Id().GetID()).arg(selfieField);
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaUnit values.
	qa.addBindValue(AssignedName());
	qa.addBindValue(Allegiance());
	qa.addBindValue(ArenaTeam());
	{
		// Conditions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << LockConditions();
		UnlockConditions();
		qa.addBindValue(ba);
	}
	{
		// Unit Damage History
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		const AuUnitDamageHash& ud = LockUnitDamage();
		out << (quint32)ud.size();
		for(AuUnitDamageHash::const_iterator di = ud.begin(); di != ud.end(); di++)
		{
			out << (*di).first;
			out << (*di).second;
		}
		UnlockUnitDamage();
		qa.addBindValue(ba);
	}

	// DaCreature values.
	qa.addBindValue(CurrentBodyHitPoints());
	{
		// Clear old dna field.
		QByteArray ba;
		qa.addBindValue(ba);

		// Nu dna.
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		AuDNABase* dna = DNAnew();
		dna->WriteDynamic(out);
		delete dna;
		qa.addBindValue(ba);
	}
	qa.addBindValue(Haircut());
	qa.addBindValue(Shave());
	qa.addBindValue(CurrentGeneralPose().Pose());
	qa.addBindValue(ControlId());
	qa.addBindValue(StationId());
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Stamina);
	qa.addBindValue(CurrentHunger());
	qa.addBindValue(m_AutoEat);
	qa.addBindValue(m_SeatIndex);
	qa.addBindValue(m_MinutesToFullGrown);
	qa.addBindValue(m_BodyQ);
	{
		// Orders
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		LockOrdersForRead() >> out;
		UnlockOrders();
		qa.addBindValue(ba);
	}
	qa.addBindValue(Posture());

	// DaPerson values.
	qa.addBindValue(FuzzyClamp(m_AccountBalance));
	qa.addBindValue(FuzzyClamp(m_LoanBalance));
	qa.addBindValue(m_AgentBits);
	qa.addBindValue(m_ArenaEntered.UTC());
	qa.addBindValue(FuzzyClamp(m_ArenaExitLoc.x));
	qa.addBindValue(FuzzyClamp(m_ArenaExitLoc.y));
	qa.addBindValue(FuzzyClamp(m_ArenaExitLoc.z));
	qa.addBindValue(m_ArenaExitParent.GetID());
	qa.addBindValue(m_ArenaShipDesign);
	qa.addBindValue(m_CurrentGearPos.EncodeU16());
	//qDebug() << "DaPerson::Update" << Id() << Name() << "gpos" <<
		//QString("0x%1").arg(m_CurrentGearPos.EncodeU16(), 4, 16, QChar('0'));
	qa.addBindValue(m_CurrentGearSub);
	qa.addBindValue(m_Reputation);
	qa.addBindValue((short int)m_Worms);
	qa.addBindValue(m_DesignPass);
	qa.addBindValue(m_SpawnBd.GetID());
	qa.addBindValue(m_SpawnBdName);
	qa.addBindValue(m_SpawnBdSystem.GetID());
	qa.addBindValue(m_SpawnBdSystemName);
	qa.addBindValue(m_SpawnDevLL.Longitude());
	qa.addBindValue(m_SpawnDevLL.Latitude());
	qa.addBindValue(m_SpawnShip.GetID());
	qa.addBindValue(m_SpawnShipIsPrivate);
	qa.addBindValue(m_SpawnShipName);
	qa.addBindValue(m_BerthType);
	qa.addBindValue(FuzzyClamp(m_RewardsOwed));
	qa.addBindValue(AccountTypeOnLine());
	qa.addBindValue(m_tmLastOnLine.UTC());
	{
		// Skills
		QByteArray ba;
		const AuSkillSet& skills = LockSkillsForRead();
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		out << skills;
		UnlockSkills();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_Lineage);
	qa.addBindValue(m_Dad.GetID());
	qa.addBindValue(m_Mom.GetID());
	qa.addBindValue(m_AccountInactive);
	qa.addBindValue(m_Con);

	// Inventory
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_GearLock.lockForRead();
		m_Gear >> out;
		m_ArenaExitInv >> out;
		quint8 ver = 0x01;
		out << ver;
		out << m_PowerMoveMax;
		out << m_PowerJump;
		m_GearLock.unlock();
		qa.addBindValue(ba);
	}

	// Sea Chest
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_SeaChestLock.lockForRead();
		m_SeaChest >> out;
		m_SeaChestLock.unlock();
		qa.addBindValue(ba);
	}

	// Open jobs
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_JobsLock.lockForRead();
		out << m_Jobs;
		m_JobsLock.unlock();
		qa.addBindValue(ba);
	}

	// Memories
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_MemoryLock.lockForRead();
		out << m_Memories;
		m_MemoryLock.unlock();
		qa.addBindValue(ba);
	}

	// Statistics
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_Stats >> out;
		qa.addBindValue(ba);
	}

	{
		// Captain's Log
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_PersonLock.lockForRead();
		m_Log >> out;
		m_PersonLock.unlock();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_LogTime.UTC());

	{
		// Script
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		if(m_Script)
		{
			out << (quint8)1;
			out << *m_Script;
		}
		else
			out << (quint8)0;
		qa.addBindValue(ba);
	}

	{
		// Domain
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		m_PersonLock.lockForRead();
		out << m_Domain;
		m_PersonLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Hot key mapping
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		for(int mp=0; mp<10; mp++)
			m_MapPos[mp] >> out;
		qa.addBindValue(ba);
	}

	if(!selfieField.isEmpty())
	{
		// Save my selfie.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_PersonLock.lockForRead();
		out << m_Selfie;
		m_PersonLock.unlock();
		qa.addBindValue(ba);
	}

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaPerson::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaPerson::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

void DaPerson::StoredInDb()
{
	DaSentient::StoredInDb();

	// Clear the selfie. We only carry it around long enough to save it after it is set.
	m_Selfie = QImage();
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaPerson* DaPerson::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	// Are we loading the selfie?
	// NOTE: We do not load the selfie here unless we're running in webcgi mode, which means we are making an empire report and
	//       want the selfies.
	QString selfieField;
	if(g_Au.IsWebCGI())
		selfieField = ",selfie";

	QString sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
			"timeModified,timeTock,timeDeath, name,allegiance,arenaTeam,conditions,damageHistory, "
			"hp,sex,dna,dna4,hair,shave,"
			"islefty,pose,whichConsole,stationId,atRest,vecX,vecY,vecZ,stamina,hunger,autoEat,seatIndex,minsToFullGrown,bodyQ,"
			"orders,posture, "
			"bank,loan,agentBits,arenaEntered,arenaExitLocX,arenaExitLocY,arenaExitLocZ,arenaExitParent,arenaShipDesign,"
			"usepos,usesub,social,"
			"worms,designpass,spawncity,spawnCityName,spawnSystem,spawnSystemName,spawnx,spawny,spawnship,"
			"spawnShipIsPrivate,spawnShipName,berthType,inv,seaChest,"
			"rewards,online,lastOnLine,skills,lineage,dad,mom,accountInactive,con,jobs,memory,stats,log,"
			"logtime,script,domain,"
			"mappos%1 FROM a_Person WHERE")
		.arg(selfieField);

	if(!idToLoad.IsNull())
		sq += QString(" id=%1").arg(idToLoad.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";

	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	if(!qa.exec())
		qWarning() << "DaPerson::LoadSubatoms FAILED. sq" << sq << "err" << qa.lastError();
	while(qa.next())
	{
		DaPerson *pnu = new DaPerson;
		if(pnu)
		{
			int qv = 0;	// query value index
			// DaAtom values.
			quint32 id = qa.value(qv++).toUInt();
			pnu->SetId(AuId(id));
			double vx = qa.value(qv++).toDouble();
			double vy = qa.value(qv++).toDouble();
			double vz = qa.value(qv++).toDouble();
			double rx = qa.value(qv++).toDouble();
			double ry = qa.value(qv++).toDouble();
			double rz = qa.value(qv++).toDouble();
			AuAtomPos pos(parentId, vx,vy,vz, rx,ry,rz);
			pnu->SetPosAndOrient(pos);

			QDateTime tmCreated = qa.value(qv++).toDateTime();
			QDateTime tmModify = qa.value(qv++).toDateTime();
			QDateTime tmTocked = qa.value(qv++).toDateTime();
			QDateTime tmDeath = qa.value(qv++).toDateTime();
			tmCreated.setTimeSpec(Qt::UTC);
			tmModify.setTimeSpec(Qt::UTC);
			tmTocked.setTimeSpec(Qt::UTC);
			tmDeath.setTimeSpec(Qt::UTC);
			QDateTime now(QDateTime::currentDateTime());
			now.setTimeSpec(Qt::UTC);
			if(tmCreated > now)
				tmCreated = now;
			if(tmModify > now)
				tmModify = now;
			if(tmTocked > now)
				tmTocked = now;
			pnu->SetCreatedTime(tmCreated);
			pnu->SetModifyTime(tmModify);
			pnu->SetTockTime(tmTocked);
			pnu->SetDeathTime(tmDeath);

			// DaUnit values.
			pnu->SetName(qa.value(qv++).toString());
			pnu->SetAllegiance(qa.value(qv++).toUInt());
			pnu->SetArenaTeam(qa.value(qv++).toUInt());
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Conditions
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->LockConditionsForWrite();
				pnu->UnlockConditions();
			}
			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Unit Damage History
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				quint32 c;
				AuUnitDamageHash& ud = pnu->LockUnitDamageForWrite();
				in >> c;
				while(c--)
				{
					AuId i;
					AuUnitDamage d;

					in >> i;
					in >> d;

					ud.insert(pair<AuId, AuUnitDamage>(i, d));
				}
				pnu->UnlockUnitDamage();
			}

			// DaCreature values.
			pnu->SetHitPoints(qa.value(qv++).toInt());	// At this point, m_MinutesToFullGrown is still 0, which is good.
			pnu->SetChromatid((AuDNABase::CHROMATID)qa.value(qv++).toInt() ? AuDNABase::Alpha : AuDNABase::Beta);	// Was gender
			{
				// The nu style DNA takes precedence over the old style.
				QByteArray baol = qa.value(qv++).toByteArray();
				QByteArray banu = qa.value(qv++).toByteArray();
				if(banu.isEmpty())
					pnu->SetDNA(DhDNA(baol));	// Nu is empty so assume old is ok.
				else
				{
					// Nu is not empty so use it.
					QDataStream in(banu);
					in.setVersion(QDataStream::Qt_5_7);
					AuDNABase* dna = AuDNABase::ReadDynamic(in);
					if(dna)
					{
						pnu->SetDNA(*dna);
						delete dna;
					}
				}
			}
			AuHairStyle::STYLE hair = (AuHairStyle::STYLE)qa.value(qv++).toInt();
			AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)qa.value(qv++).toInt();
			pnu->SetHairStyle(hair, shave);
			pnu->SetLeftHanded(qa.value(qv++).toBool());
			AuPose curPose = AuPose((AuPose::POSE)qa.value(qv++).toInt());
			pnu->m_ControlId = qa.value(qv++).toUInt();
			quint16 stationId = qa.value(qv++).toUInt();
			pnu->SetAtRestState(qa.value(qv++).toBool());
			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			pnu->SetVector(AuPoint(vx,vy,vz));
			pnu->m_Stamina = qa.value(qv++).toInt();
			pnu->SetHunger(qa.value(qv++).toInt());
			pnu->m_AutoEat = qa.value(qv++).toBool();
			pnu->m_SeatIndex = qa.value(qv++).toInt();
			pnu->SetMinutesToFullGrown(qa.value(qv++).toUInt());	// Updates my growth stage in animalia
			pnu->m_BodyQ = qa.value(qv++).toUInt();
			if(pnu->m_BodyQ == 0)
				pnu->m_BodyQ = 128;
			qvi = qv++;
			AuAIOrderArray& orders = pnu->LockOrdersForWrite();
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				orders << in;
			}
			if(orders.isEmpty())
			{
				// Index 0 is always my fallback Stay order.
				orders.append(AuAIOrder(ORDER_Stay, parentId, pnu->Location(), 0, ""));
				pnu->UnlockOrders();
			}
			else
			{
				pnu->UnlockOrders();
				pnu->ResetOrderTime();
			}
			pnu->SetPosture((CITIZEN_POSTURE)qa.value(qv++).toInt());

			// DaPerson values.
			pnu->m_AccountBalance = qa.value(qv++).toDouble();
			pnu->m_LoanBalance = qa.value(qv++).toDouble();
			pnu->m_AgentBits = qa.value(qv++).toUInt();

			QDateTime arenaEntered = qa.value(qv++).toDateTime();
			arenaEntered.setTimeSpec(Qt::UTC);
			if(arenaEntered > now)
				arenaEntered = now;
			pnu->m_ArenaEntered = arenaEntered;

			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			pnu->m_ArenaExitLoc = AuPoint(vx, vy, vz);
			pnu->m_ArenaExitParent = AuId(qa.value(qv++).toUInt());
			pnu->m_ArenaShipDesign = qa.value(qv++).toUInt();
			pnu->m_CurrentGearPos = AuGearPos::DecodeU16(qa.value(qv++).toUInt());
			pnu->m_CurrentGearSub = (INVENTORYSUB)qa.value(qv++).toInt();
			pnu->m_Reputation = qa.value(qv++).toInt();
			pnu->m_Worms = qa.value(qv++).toInt();
			pnu->m_DesignPass = qa.value(qv++).toString();
			pnu->m_SpawnBd = AuId(qa.value(qv++).toUInt());
			pnu->m_SpawnBdName = qa.value(qv++).toString();
			pnu->m_SpawnBdSystem = AuId(qa.value(qv++).toUInt());
			pnu->m_SpawnBdSystemName = qa.value(qv++).toString();
			float spawnTileX = qa.value(qv++).toFloat();
			float spawnTileY = qa.value(qv++).toFloat();
			pnu->m_SpawnDevLL = AuGlobeLLF(spawnTileX, spawnTileY);
			pnu->m_SpawnShip = AuId(qa.value(qv++).toUInt());
			pnu->m_SpawnShipIsPrivate = qa.value(qv++).toBool();
			pnu->m_SpawnShipName = qa.value(qv++).toString();
			pnu->m_BerthType = (AuRosterEntry::TYPE)qa.value(qv++).toInt();

			// Load my gear.
			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_Gear;
				in >> pnu->m_ArenaExitInv;
				quint8 ver;
				in >> ver;
				if(ver >= 0x01)
				{
					in >> pnu->m_PowerMoveMax;
					in >> pnu->m_PowerJump;
				}
				else
				{
					// Init new move and jump values to their maximums.
					pnu->m_PowerMoveMax = 15;
					pnu->m_PowerJump = 15;
				}
				pnu->m_WeightCarried = pnu->GearWeight();
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_SeaChest << in;
			}

			pnu->m_RewardsOwed = qa.value(qv++).toDouble();
			pnu->m_AccountOnLine = (ACCOUNT)qa.value(qv++).toInt();
			QDateTime tmOnLine = qa.value(qv++).toDateTime();
			tmOnLine.setTimeSpec(Qt::UTC);
			if(tmOnLine > now)
				tmOnLine = now;
			pnu->m_tmLastOnLine = tmOnLine;

			QByteArray ba = qa.value(qv++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_5_7);
				in >> pnu->m_Skills;
			}
			pnu->m_Lineage = (LINEAGE_TYPE)qa.value(qv++).toUInt();
			pnu->m_Dad = AuId(qa.value(qv++).toUInt());
			pnu->m_Mom = AuId(qa.value(qv++).toUInt());
			pnu->m_AccountInactive = qa.value(qv++).toBool();
			pnu->m_Con = qa.value(qv++).toBool();

			// Jobs. Active stories are jobs.
			qvi = qv;
			qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_Jobs;
			}
			else
				pnu->m_Jobs.Clear();

			// Memories
			qvi = qv;
			qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_Memories;
			}

			// Statistics
			qvi = qv;
			qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Stats << in;

				if(pnu->m_Skills.Count() == 0)
					pnu->InitSkillsFromStats();	// Could be an old avatar. See if we can grab skill information from their stats.
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Captain's Log
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Log << in;
			}

			QDateTime lt = qa.value(qv++).toDateTime();
			lt.setTimeSpec(Qt::UTC);
			pnu->m_LogTime = lt;

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Script
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				quint8 hasScript;
				in >> hasScript;
				if(hasScript == 1)
				{
					AuScript script;
					in >> script;
					pnu->StorySetScript(script);
				}
			}

			ba = qa.value(qv++).toByteArray();
			if(!ba.isEmpty())
			{
				// Domain
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_5_7);
				in >> pnu->m_Domain;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Hot key mapping.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				for(int mp=0; mp<10; mp++)
					pnu->m_MapPos[mp] << in;
			}

			if(!selfieField.isEmpty())
			{
				qvi = qv++;
				if(!qa.value(qvi).isNull())
				{
					QByteArray ba = qa.value(qvi).toByteArray();
					QDataStream in(ba);
					in.setVersion(QDataStream::Qt_4_1);
					in >> pnu->m_Selfie;
				}
			}

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->RestorePose(curPose, stationId);				// Restore console after adding to my parent atom.
			pnu->StartThinking();
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			// Person can be carrying other people or animals.
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
			DaAnimal::LoadSubatoms(ether, db, pnu->Id());
			DaNpc::LoadSubatoms(ether, db, pnu->Id());

			if(g_Au.m_LogMessages & LOG_Travel)
			{
				QString logmsg = QString("DaPerson::LoadSubatoms %1 %2, parent %3, pose %5, created %6, modified %7")
					.arg(pnu->Id().GetID()).arg(pnu->Name())
					.arg(parentId.GetID())
					.arg(curPose.Pose())
					.arg(pnu->CreatedTime().UTC().toString())
					.arg(pnu->ModifyTime().UTC().toString());
				if(pnu->IsOnLine())
					logmsg += ", on line";
				if(pnu->IsAwaitingSpawn())
					logmsg += ", awaiting spawn";
				if(pnu->IsLineageDemiavatar())
					logmsg += ", demi";
				g_Au.Log(logmsg);
			}

			if(g_Au.IsSceneWorker())
			{
				// When a person is loaded, the worker server needs to insure that it has the person's working spacecraft design
				// loaded. That way it is immediately available if the person steps into a design studio. If the person happens
				// to be in a design studio, get all their designs now.
				const DaDesignBase* env = pnu->FindParentDesignBaseInEnv();
				if(env)
					g_Au.ScDesignsNeeded(pnu->Id());

				// When an atom is loaded on a worker server that is awaiting respawn, tell it to respawn.
				// NOTE: If the spawn point cannot be found, this could result in an unnecessary handoff to the master server.
				if(pnu->IsAwaitingSpawn())
				{
					if(g_Au.m_LogMessages & LOG_Spawn)
						qDebug() << "DaPerson::LoadSubatoms" << pnu->Id() << pnu->Name() << "IsAwaitingSpawn. "
							"Preparing to respawn at next Tock. forceLocal" << !idToLoad.IsNull();;
					// The specific id to load is used when assertively told to load a spawning atom on this server. In that case,
					// we force the atom to spawn locally if at all possible by passing forceLocal = true.
					// The id to load is null when simply loading atoms recursively. If we happen to load a spawning atom in that
					// case, give it the option to propagate across the server farm looking for its spawn point by passing
					// forceLocal = false.
					if(idToLoad.IsNull())
						pnu->SetSpawnAnywhere();	// Trigger spawn at next Tick with forceLocal = false.
					else
						pnu->SetSpawnLocal();		// Trigger spawn at next Tick with forceLocal = true.
				}
			}
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

void DaPerson::SetSpawnAnywhere()
{
	m_PersonLock.lockForWrite();
	m_uiActions |= ACT_SpawnAnywhere;
	m_PersonLock.unlock();
}

void DaPerson::SetSpawnLocal()
{
	m_PersonLock.lockForWrite();
	m_uiActions |= ACT_SpawnLocal;
	m_PersonLock.unlock();
}

// Returns wormhole sequence number.
int DaPerson::ExploredWormhole()
{
	m_PersonLock.lockForWrite();
	m_Worms++;
	int w = m_Worms;
	m_tmReputationChange.Set();
	m_PersonLock.unlock();

	// Player Statistics
	Stats().WormholeNamed();
	IncreaseSkill(AuSkill::SpaceExploration, 1);

	SetModified();
	return w;
}

void DaPerson::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	// Get armor afforded by body.
	DaSentient::CombatArmorEffectiveness(mode, effect, hitShield, wpnLoc, hitLoc, baseAbsorb, baseBlock, auxAbsorb, auxBlock);

	// Add together the body's armor with any worn armor.
	// Calculate armor from items that are in valid "use" locations.
	float invAbsorb, invBlock;
	m_GearLock.lockForRead();
	m_Gear.CombatArmorEffectiveness(mode, effect, invAbsorb, invBlock);
	m_GearLock.unlock();

	// Add together the body's armor with any worn armor.
	auxAbsorb += invAbsorb;
	auxBlock += invBlock;
}

void DaPerson::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	// Do not accept additional damage or return true if my body is already dead.
	if(IsBodyDead() || !IsNormal())
		return;

	// Don't damage body while in the design environment.
	if(InDesignEnvironment())
		return;

	// Don't damage body while awaiting spawn.
	if(IsAwaitingSpawn())
		return;

	// Armor takes damage before my body.
	// Wear down equipment.
	// Some weapon types are not blocked by armor.
	// Critical hits apply to body only, bypassing the armor.
	int bodyDamage1 = result.m_Damage1;
	int bodyDamage2 = result.m_Damage2;
	if(weapon.m_Mode != WM_Internal && weapon.m_Mode != WM_Psionic &&
			result.m_CriticalHits != 2 &&						// 2 crits means both effects were critical so no armor helps
			weapon.m_CID != CID_MedicalProcedure &&
			weapon.m_CID != CID_GivingBirth &&
			weapon.m_CID != CID_ImplantBirth &&
			weapon.m_CID != CID_Poison &&
			weapon.m_CID != CID_Disease &&
			weapon.m_CID != CID_Eaten &&
			weapon.m_CID != CID_ShipCapture &&
			weapon.m_CID != CID_Wormhole &&
			weapon.m_CID != CID_Starvation &&
			weapon.m_CID != CID_InternalInjuries)
	{
		if(bodyDamage1 && !result.m_CriticalHits)
		{
			// Absorb as much of the damage with my armor as possible before allowing it to hurt my body.
			m_GearLock.lockForWrite();
			int armorHits = m_Gear.CombatArmorHits(weapon.m_Mode, weapon.m_w1.m_Effect);
			if(armorHits >= bodyDamage1)
			{
				// My armor absorbs all of the damage.
				if(m_Gear.CombatApplyDamage(weapon.m_Mode, weapon.m_w1.m_Effect, bodyDamage1))
					UpdateGearWeight();
				bodyDamage1 = 0;
			}
			else
			{
				// My armor absorbs some of the damage.
				if(m_Gear.CombatApplyDamage(weapon.m_Mode, weapon.m_w1.m_Effect, armorHits))
					UpdateGearWeight();
				bodyDamage1 -= armorHits;
			}
			m_GearLock.unlock();
		}
		if(bodyDamage2)
		{
			// Absorb as much of the damage with my armor as possible before allowing it to hurt my body.
			m_GearLock.lockForWrite();
			int armorHits = m_Gear.CombatArmorHits(weapon.m_Mode, weapon.m_w2.m_Effect);
			if(armorHits >= bodyDamage2)
			{
				// My armor absorbs all of the damage.
				if(m_Gear.CombatApplyDamage(weapon.m_Mode, weapon.m_w2.m_Effect, bodyDamage2))
					UpdateGearWeight();
				bodyDamage2 = 0;
			}
			else
			{
				// My armor absorbs some of the damage.
				if(m_Gear.CombatApplyDamage(weapon.m_Mode, weapon.m_w2.m_Effect, armorHits))
					UpdateGearWeight();
				bodyDamage2 -= armorHits;
			}
			m_GearLock.unlock();
		}
	}

	// Remaining bodyDamage1 and bodyDamage2 points are applied to my body.
	DaSentient::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);

	// Player Statistics
	if(result.m_IsDestroyed || IsBodyDyingOrDead())
	{
		if(weapon.m_w1.m_Effect != WE_Bleeding)
			Stats().KilledBy(weapon.m_w1.m_Effect);

		if(weapon.m_CID == CID_SunBurn || weapon.m_CID == CID_Spaghettification)
		{
			const DaStar* parentStar = FindParentStarInEnv();
			if(parentStar)
				Stats().SunBurn(parentStar, 0);	// Killed by solar radiation or spaghettification. Stats record them the same.
		}
	}
	
	AuPoint fromDir = wpnLoc - hitLoc;
	if(!fromDir.IsFuzzyZero())
		fromDir.Unit();

	m_PersonLock.lockForWrite();
	m_HitNotify = AuHitNotify(fromDir,
			weapon.m_w1.m_Effect, bodyDamage1, result.m_BaseArmor1+result.m_AuxArmor1+(result.m_Damage1-bodyDamage1),
			weapon.m_w2.m_Effect, bodyDamage2, result.m_BaseArmor2+result.m_AuxArmor2+(result.m_Damage2-bodyDamage2));
	m_tmHitChange.Set();
	m_PersonLock.unlock();

	SetModified(NOTIFY_Scene);				// Trigger tx to client. db update not needed.

	// Send a message to this person that they received damage when it's not from a unit, e.g. starvation or disease.
	if(IsOnLine() && attacker.m_Id.IsNull() &&
			(result.m_Damage1 || result.m_BaseArmor1 || result.m_AuxArmor1 ||
			 result.m_Damage2 || result.m_BaseArmor2 || result.m_AuxArmor2))
	{
		QString text = FormatDamageReceived(result.m_CriticalHits,
				weapon.m_w1.m_Effect, bodyDamage1, result.m_BaseArmor1+result.m_AuxArmor1+(result.m_Damage1-bodyDamage1),
				weapon.m_w2.m_Effect, bodyDamage2, result.m_BaseArmor2+result.m_AuxArmor2+(result.m_Damage2-bodyDamage2),
				attacker.m_Name);
		Thought(text);
	}

	if(IsAIControlled())
	{
		// Operating as an AI controlled avatar.
		if(!attacker.m_Id.IsNull() && weapon.m_Mode != WM_Internal)
		{
			// I was hit by somebody.
			DaWorld* planet = 0;
			DaVessel* ship = 0;
			DaBd* bd = 0;
			if(IsInSpacecraft())
			{
				ship = FindParentVesselInEnv();
			}
			else if(!InBdCity().IsNull() && !attacker.m_Id.IsNull())
			{
				// I am in a bd city. Notify the city there has been an attack in town. This prevents workers from spawning
				// which helps to eliminate a genocide exploit. Players would hover over a city and systematically slaughter all the
				// citizens until there was no population. Then the loyalty is not a factor. That will still be possible in a small
				// town, if they wait until all of the populace spawns before attacking.
				// This also prevents an exploit where a troop at a console is killed, replacement ports to console, killed, repeat.
				planet = FindParentPlanetInEnv();
				if(planet)
				{
					bd = planet->FindBd(BdId());
					if(bd)
					{
						DaAtom* solar = planet->FindEnvironmentAtom();
						if(solar)
						{
							DaAtom* atom = solar->FindIdRecursive(attacker.m_Id);
							if(atom && atom->EmpireId())
							{
								POLITICAL_STANCE stance = PoliticalStance(EmpireId(), atom->EmpireId());
								if(stance == POL_Enemy || stance == POL_Neutral)
								{
									QString empireName;
									const DgData& gd = g_Au.LockDataForRead();
									gd.EmpireName(atom->EmpireId(), empireName);
									g_Au.UnlockData();
									QString attName = tr("%1 of %2").arg(atom->Name()).arg(empireName);
									bd->LogEvent(AuEventLogEntry::CitizenAttacked, attName);
								}
							}
						}
					}
				}
			}

			if(CurrentHitPoints() > 0)
			{
				// I am still alive.
				if(!IsAlert())
				{
					QString text = tr("I am under attack by %1.").arg(attacker.m_Name);
					BecomeAlert(text);		// Become alert and alert nearby troops.
				}
				if(Posture() != POSTURE_HoldFire && attacker.m_Id != OrderTarget())
				{
					// I was hit by something that is not the target of my current order.
					switch(Order())
					{
					case ORDER_Sleep:
					case ORDER_SleepDayDream:
					case ORDER_SleepEat:
					case ORDER_SleepExercise:
					case ORDER_SleepGame:
					case ORDER_SleepLetter:
					case ORDER_SleepMovie:
					case ORDER_SleepMusic:
					case ORDER_SleepPine:
					case ORDER_SleepPhone:
					case ORDER_SleepRead:
					case ORDER_SleepTalk:
					case ORDER_SleepText:
					case ORDER_SleepTV:
					case ORDER_SleepWeb:
					case ORDER_DamageReport:
					case ORDER_SurveyScanPlanet:
					case ORDER_SurveyScanSystem:
					case ORDER_TacticalScan:
					case ORDER_Stay:
					case ORDER_FollowMe:
					case ORDER_FollowTarget:
					case ORDER_GoToRoom:
					case ORDER_GoToTarget:
					case ORDER_GoToTargetExactly:
					case ORDER_HengeCeremony:
						// Attack the atom that attacked me.
						TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
						break;

					case ORDER_BattleStation:
						// On a spacecraft I get up from my station. In city I stay at my station.
						if(IsHelmStationPose())
						{
							// Attack the atom that attacked me if the ship is not moving.
							if(!ship || !ship->IsMoving())
								TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
						}
						else if(IsMedicStationPose())
						{
							// Attack the atom that attacked me if I don't have a patient.
							if(ship)
							{
								if(!ship->FindPatient(*this))
									TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
							}
							else if(bd && IsInsideBd())
							{
								if(!bd)
									bd = FindParentBdInEnv();
								if(!bd->FindPatient(*this))
									TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
							}
						}
						else if(IsInSpacecraft() || !IsTroopStationPose())
						{
							// Attack the atom that attacked me.
							TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
						}
						break;

					case ORDER_AttackTarget:
					case ORDER_ComeHere:
					case ORDER_ComeHereExactly:
					case ORDER_GetIn:
					case ORDER_GetOut:
					case ORDER_Garrison:
					case ORDER_CloseDoor:
					case ORDER_OpenDoor:
					case ORDER_GetItem:
					case ORDER_EnterSurgery:
					case ORDER_TakeCommand:
					case ORDER_ReleaseCommand:
					case ORDER_DropRider:
					case ORDER_MountMe:
					case ORDER_MountTarget:
						// Don't get distracted from my current order.
						break;
					}
				}
			}
		}

		// When body has died, drop meat and maybe an item.
		if(result.m_IsDestroyed)
			BodyHasDied(weapon.m_CID, weapon.m_Quality, weapon.m_w1.m_Effect, weapon.m_w2.m_Effect, attacker.m_Name);
		else if(weapon.m_CID != CID_MedicalProcedure)
			GetACheckup();										// Consider finding a sick bay for medical attention.
	}

	// Clean up the body.
	if(result.m_IsDestroyed)
	{
		if(IsInAtmosphere())
		{
			// If a person dies on a world with a village, the indigenous forget their grudges against the person.
			DaWorldLL* world = FindParentWorldInEnv();
			if(world)
				world->VillageResetReputation(Id());
		}
		RepairConditions();				// Don't remain on fire, paralyzed or melting with acid.
	}
}

// List all atoms in the scene. 'this' person is the scene atom.
bool DaPerson::ListSceneAtoms(DaAtomList& sceneAtoms)
{
	// Get a list of the atoms from the sector to this atom.
	DaAtom* uni = FindParentUniverse();
	DaAtomList apath;
	if(!uni || !ListAtomsFromParent(apath, uni))
		return false;
	apath.prepend(uni);	// Environment atom should be in this list also.

	AuPoint cameraEnvLoc = EnvTransform().Project(SceneCameraLocation());
	const DaAtom* env = FindEnvironmentAtom();

	// Now append all subatoms.
	//qDebug("DaAtom::ListSceneAtoms - apath %d", apath.count());
	bool inEnv = false;
	while(!apath.isEmpty())
	{
		DaAtom* pa = apath.first();
		apath.removeFirst();
		//qDebug("DaAtom::ListSceneAtoms - pa %s %s",
				//(const char*)pa->Name(), (const char*)pa->Description());
		sceneAtoms.AddIfUnique(pa);
		if(!inEnv && pa == env)
			inEnv = true;
		if(inEnv)
			pa->ListSceneSubatoms(this, cameraEnvLoc, sceneAtoms);	// In environment so get all subordinate scene atoms.
	}
	return true;
}

void DaPerson::ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms)
{
	DaSentient::ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
	AuEther* ether = Ether();
	if(!ether)
		return;
	if(!pSceneAtom->Is(Id()))
		return;

	if(!HasConsole())
		return;

	// Must monitor my nav console solar system.
	// If it's not current solar system, it won't be in the ListSceneAtoms hierarchy.
	const DpStation* console = LockConsoleForRead();
	if(console && AuStation::IsNavigation(console->Type()))
	{
		// This person is the scene atom.
		const DpStationNav* pca = (const DpStationNav*)console;
		const DaSolarSystem* pa = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
		if(pa && pca->DisplayedSystem() != pa->Id())
		{
			// My console is monitoring a system that is not the one I'm in.
			DaAtom* pa = ether->FindNormalAtom(pca->DisplayedSystem());
			if(pa)
			{
				//qDebug("DaPerson::ListSceneSubatoms - Listing solar system %d", (const char*)pca->DisplayedSystem().GetID());
				// Add the solar system atom to the scene atoms list.
				sceneAtoms.AddIfUnique(pa);

				// Get stars, planets and wormholes.
				pa->ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
			}
		}
	}
	UnlockConsole();
}

void DaPerson::SelectNextItem()
{
	// Make a list of the valid item positions, including subs and body parts.
	QList<SelItem_> validItems;
	int current = -1;	// Set to current item index in validItems if found.

	// Body part locations and those of my mount.
	AuGearPos gearPos(CurrentItemPos());
	for(qint8 ipos=IP_Mount_Anus; ipos<0; ipos++)
	{
		AuGearPos bpos((INVENTORYPOS)ipos);
		// Check for possible physical attack, which is the IPS_Item mode.
		AuCombatWeapon weapon(WM_Not);
		int ammoCur, ammoFull;
		WeaponAtUsePos(bpos, IPS_Item, weapon, ammoCur, ammoFull);
		if(weapon.m_Mode != WM_Not)
		{
			if(CurrentItemSub() == IPS_Item && gearPos == bpos)
				current = validItems.count();
			validItems.append(SelItem_(bpos, IPS_Item));
		}

		// Check for possible special attack, which is the IPS_Weapon1 mode.
		WeaponAtUsePos(bpos, IPS_Weapon1, weapon, ammoCur, ammoFull);
		if(weapon.m_Mode != WM_Not)
		{
			if(CurrentItemSub() == IPS_Weapon1 && gearPos == bpos)
				current = validItems.count();
			validItems.append(SelItem_(bpos, IPS_Weapon1));
		}
	}

	// Top level inventory items.
	m_GearLock.lockForRead();
	for(AuGear::const_iterator gi = m_Gear.cbegin(); gi != m_Gear.cend(); gi++)
	{
		AuGearPos index(*gi);
		if(!(*gi).Item().IsDetonatorWeapon() && !(*gi).Item().IsFuseWeapon())
		{
			// The IPS_Item position is valid since the item is valid.
			if(CurrentItemSub() == IPS_Item && gearPos == index)
				current = validItems.count();
			validItems.append(SelItem_(index, IPS_Item));
		}

		// Check for primary weapon capability.
		if((*gi).Item().IsWeapon(IPS_Weapon1))
		{
			if(CurrentItemSub() == IPS_Weapon1 && gearPos == index)
				current = validItems.count();
			validItems.append(SelItem_(index, IPS_Weapon1));
		}

		// Check for secondary weapon capability.
		if((*gi).Item().IsWeapon(IPS_Weapon2))
		{
			if(CurrentItemSub() == IPS_Weapon2 && gearPos == index)
				current = validItems.count();
			validItems.append(SelItem_(index, IPS_Weapon2));
		}
	}
	m_GearLock.unlock();

	if(validItems.isEmpty())
	{
		//qDebug("DaPerson::SelectNextItem no valid items to select");
		return;	// Nothing to select if there are no valid items.
	}
	
	// Select next current valid item index.
	int nucurrent;
	if(current < 0 || current >= validItems.count()-1)
		nucurrent = 0;
	else
		nucurrent = current+1;
	m_PersonLock.lockForWrite();
	m_CurrentGearPos = validItems[nucurrent].pos;
	m_CurrentGearSub = validItems[nucurrent].sub;
	m_tmItemUseChange.Set();
	m_PersonLock.unlock();
	SetModified();
	//qDebug("DaPerson::SelectNextItem current %d nucur %d nu pos %d sub %d",
			//current, nucurrent, m_CurrentUsePos, CurrentItemSub());
}

void DaPerson::SelectPreviousItem()
{
	// Make a list of the valid item positions, including subs and body parts.
	QList<SelItem_> validItems;
	int current = -1;	// Set to current item index in validItems if found.

	// Body part locations and those of my mount.
	AuGearPos gearPos(CurrentItemPos());
	for(qint8 ipos=IP_Mount_Anus; ipos<0; ipos++)
	{
		AuGearPos bpos((INVENTORYPOS)ipos);
		// Check for possible physical attack, which is the IPS_Item mode.
		AuCombatWeapon weapon(WM_Not);
		int ammoCur, ammoFull;
		WeaponAtUsePos(bpos, IPS_Item, weapon, ammoCur, ammoFull);
		if(weapon.m_Mode != WM_Not)
		{
			if(CurrentItemSub() == IPS_Item && gearPos == bpos)
				current = validItems.count();
			validItems.append(SelItem_(bpos, IPS_Item));
		}

		// Check for possible special attack, which is the IPS_Weapon1 mode.
		WeaponAtUsePos(bpos, IPS_Weapon1, weapon, ammoCur, ammoFull);
		if(weapon.m_Mode != WM_Not)
		{
			if(CurrentItemSub() == IPS_Weapon1 && gearPos == bpos)
				current = validItems.count();
			validItems.append(SelItem_(bpos, IPS_Weapon1));
		}
	}

	// Top level inventory items.
	m_GearLock.lockForRead();
	for(AuGear::const_iterator gi = m_Gear.cbegin(); gi != m_Gear.cend(); gi++)
	{
		AuGearPos index(*gi);
		if(!(*gi).Item().IsDetonatorWeapon() && !(*gi).Item().IsFuseWeapon())
		{
			// The IPS_Item position is valid since the item is valid.
			if(CurrentItemSub() == IPS_Item && gearPos == index)
				current = validItems.count();
			validItems.append(SelItem_(index, IPS_Item));
		}

		// Check for primary weapon capability.
		if((*gi).Item().IsWeapon(IPS_Weapon1))
		{
			if(CurrentItemSub() == IPS_Weapon1 && gearPos == index)
				current = validItems.count();
			validItems.append(SelItem_(index, IPS_Weapon1));
		}

		// Check for secondary weapon capability.
		if((*gi).Item().IsWeapon(IPS_Weapon2))
		{
			if(CurrentItemSub() == IPS_Weapon2 && gearPos == index)
				current = validItems.count();
			validItems.append(SelItem_(index, IPS_Weapon2));
		}
	}
	m_GearLock.unlock();

	if(validItems.isEmpty())
	{
		//qDebug("DaPerson::SelectPreviousItem no valid items to select");
		return;	// Nothing to select if there are no valid items.
	}
	
	// Select next current valid item index.
	int nucurrent;
	if(current < 1 || current >= validItems.count())
		nucurrent = validItems.count()-1;
	else
		nucurrent = current-1;
	m_PersonLock.lockForWrite();
	m_CurrentGearPos = validItems[nucurrent].pos;
	m_CurrentGearSub = validItems[nucurrent].sub;
	m_tmItemUseChange.Set();
	m_PersonLock.unlock();
	SetModified();
	//qDebug("DaPerson::SelectPreviousItem current %d nucur %d nu pos %d sub %d",
			//current, nucurrent, m_CurrentUsePos, CurrentItemSub());
}

void DaPerson::UseItemOnTarget(const AuId& target, unsigned int part, const AuPoint& targLoc, bool isAltMode)
{
	// Current use pos needs to be in a valid range.
	AuGearPos gearPos(CurrentItemPos());
	if(!gearPos.IsGear())
		return;

	INVENTORYSUB isub = CurrentItemSub();

	m_GearLock.lockForWrite();
	AuItem* gearItem = m_Gear.ItemAt(gearPos);
	if(!gearItem)
	{
		m_GearLock.unlock();
		return;
	}

	// Current use sub needs to be 'item' mode.
	if(isub != IPS_Item &&
			!gearItem->IsDetonatorWeapon() &&
			!gearItem->IsFuseWeapon())
	{
		m_GearLock.unlock();
		return;
	}

	// Use the current item's "item mode" on the target atom.
	quint32 oldCount = gearItem->Count();
	AuItem* replacementItem = UseItemOnTarget(gearItem, target, part, targLoc, isAltMode);
	if(replacementItem != gearItem)
	{
		// The item must be replaced by a nu item.
		m_Gear.ReplaceItem(gearPos, replacementItem);

		// Gear weight changed.
		UpdateGearWeight();
	}
	else if(gearItem->Count() != oldCount)
	{
		if(gearItem->Count() == 0)
			m_Gear.Erase(gearPos);	// The item was used up.

		// Gear weight changed.
		UpdateGearWeight();
	}
	m_GearLock.unlock();
}

// Expects inventory lock to be locked already.
AuItem* DaPerson::UseItemOnTarget(AuItem* item, const AuId& target, unsigned int part, const AuPoint& targLoc, bool isAltMode)
{
	// This function does nothing if the target id is null.
	if(target.IsNull())
		return item;

	// The target might likely be myself or my parent spacecraft.
	DaAtom* pTarget = 0;
	bool isSelf;
	if(Is(target))
	{
		pTarget = this;
		isSelf = true;
	}
	else if(ParentId() == target)
	{
		pTarget = Parent();
		if(!pTarget)
			return item;
		isSelf = false;
	}
	else
	{
		DaAtom* system = FindEnvironmentAtom();
		Q_ASSERT(system);
		pTarget = system->FindIdRecursive(target);
		if(!pTarget)
			return item;											// The target was not found in the solar system.
		isSelf = false;
	}

	// Use the item on the target.
	bool useItem = false;	// Set true if UseItem needs to be called to induce wear, consume ammo, etc.
	AuItem* replacementItem = item;	// May change to replacement item in some cases. This will be our return value.
	DaCreature* creature = dynamic_cast<DaCreature*>(pTarget);
	if(creature)
	{
		if(isSelf)
		{
			// Food and liquor can only be used on yourself.
			// Food and liquor are mutually exclusive; sorry no cherry cordials here.
			// Don't drink the water if they were probably trying to decontaminate themself or put out a fire.
			if(item->IsFood(EcoRole()) &&
					(item->CommodityId() != CID_Water || (!creature->HasAcid() && !creature->IsBurning())))
			{
				// The item is food for my ecological role, though it might have 0 food nutrition points (i.e. water).
				if(MaxHunger() - CurrentHunger() >= 1 || !item->FoodNutritionValue())
				{
					// Creature needs at least a point of food.
					useItem = true;								// Consume the food inventory item.
					int prevHunger = CurrentHunger();
					SetHungerLimitMax(CurrentHunger() + item->FoodNutritionValue());

					// Simple body Q effect. Higher Q food raises my body Q. Lower Q food reduces my body Q.
					if(item->Quality() > m_BodyQ && m_BodyQ < 255)
						m_BodyQ++;	// Increase to max of 255.
					else if(item->Quality() < m_BodyQ && m_BodyQ > 1)
						m_BodyQ--;	// Decrease to min of 1.

					SetModified();

					QString text;
					if(item->CommodityId() == CID_Water)
					{
						if(CurrentStamina() < MaxStamina())
						{
							ResetStamina();
							text = tr("I drank %1. That was refreshing.").arg(item->SayName());
						}
						else
							text = tr("I drank %1.").arg(item->SayName());
					}
					else if(item->CommodityId() == CID_Milk)
						text = tr("I drank %1 for %2 nutrition points. Got milk!")
							.arg(item->SayName()).arg(CurrentHunger() - prevHunger);
					else if(item->CommodityId() == CID_IceCream)
						text = tr("I ate %1 for %2 nutrition points. I scream, you scream, we all scream for ice cream.")
							.arg(item->SayName()).arg(CurrentHunger() - prevHunger);
					else if(item->CommodityId() == CID_Leather)
						text = tr("I chewed %1 for %2 nutrition points. That was chewy.")
							.arg(item->SayName()).arg(CurrentHunger() - prevHunger);
					else if(item->CommodityId() == CID_Bone)
						text = tr("I gnawed %1 for %2 nutrition points. That was crunchy.")
							.arg(item->SayName()).arg(CurrentHunger() - prevHunger);
					else if(item->CommodityId() == CID_Head)
						text = tr("I ate %1 for %2 nutrition points. Scrapple, the other grey meat.")
							.arg(item->SayName()).arg(CurrentHunger() - prevHunger);
					else
						text = tr("I ate %1 for %2 nutrition points. That was tasty.")
							.arg(item->SayName()).arg(CurrentHunger() - prevHunger);
					Thought(text);
				}
			}
			else if(item->IsIntoxicant())
			{
				// Consumed liquor. All creatures can consume liquor.
				useItem = true;									// Consume the liquor inventory item.

				QString text(tr("I consumed %1 for %2 intoxication points. I feel woozy.")
						.arg(item->SayName()).arg(item->IntoxicantValue()));
				Thought(text);
			}
			else if(item->IsLight())
			{
				// Lights don't get "applied" to others. When myself, turn my light on and off.
				QString text;
				int state = item->State();
				if(state)
				{
					item->SetState(0);	// Turn the light off.
					text = tr("My %1 light is off.").arg(item->SayName());
				}
				else
				{
					item->SetState(1);	// Turn the light on.
					text = tr("My %1 light is on.").arg(item->SayName());
					if(AgentInvisible())
						text += tr(" Nobody else can see my light while I am invisible.");
					else if(AgentAnimal())
						text += tr(" Nobody else can see my light while I am an animal.");
				}
				m_tmGearChange.Set();
				SetModified();
				useItem = true;	// wear down the light

				Thought(text);
			}
			else if(item->CommodityId() == CID_Drum)
			{
				// Drums don't get "applied" to others. When myself, turn my drum on and off.
				QString text;
				int state = item->State();
				if(state)
				{
					item->SetState(0);	// Turn the light off.
					text = tr("My %1 sound is off.").arg(item->SayName());
				}
				else
				{
					item->SetState(1);	// Turn the light on.
					text = tr("My %1 sound is on.").arg(item->SayName());
				}
				m_tmGearChange.Set();
				SetModified();
				useItem = true;	// wear down the light

				Thought(text);
			}
			else if(item->IsEnvironmentHelmet())
			{
				QString text;
				int state = item->State();
				if(state)
				{
					item->SetState(0);	// Open the helmet visor.
					if(item->HitPoints() > 1)
						text = tr("My %1 visor is open.").arg(item->SayName());
					else
						text = tr("My leaky %1 visor is open.").arg(item->SayName());
				}
				else
				{
					item->SetState(1);	// Close the helmet visor.
					if(item->HitPoints() > 1)
						text = tr("My %1 visor is closed.").arg(item->SayName());
					else
						text = tr("My %1 visor is closed but the seal is leaking.").arg(item->SayName());
				}
				m_tmGearChange.Set();
				SetModified();

				Thought(text);
			}
			else if(item->IsDetonatorWeapon())
			{
				// Drop the detonator or fuse weapon and arm it.
				DaItem* droppedItem = DropItem(*item);
				if(droppedItem)
				{
					// Arm the weapon.
					droppedItem->ArmWeapon(Id(), Name(), EmpireId());					// Wait for detonator signal.
					droppedItem->SetModified();

					QString text(tr("%1 armed for detonation in place.").arg(droppedItem->Name()));
					Thought(text);

					// Eliminate my inventory item.
					item->Consume(1);
				}
			}
			else if(item->IsFuseWeapon())
			{
				// Drop the detonator or fuse weapon and arm it.
				DaItem* droppedItem = DropItem(*item);
				if(droppedItem)
				{
					// Arm the weapon.
					droppedItem->ArmWeapon(Id(), Name(), EmpireId());					// Start fuse timer.
					droppedItem->SetModified();

					QString text(tr("%1 fuse initiated in place.").arg(droppedItem->Name()));
					Thought(text);

					// Eliminate my inventory item.
					item->Consume(1);
				}
			}
			else if(item->CommodityId() == CID_SpacecraftTitle)
			{
				AuId shipId = item->PropertyId();
				QString shipName = item->PropertyName();
				QString text;
				if(item->IsTitleVoid())
					text = tr("The title is null and void; it cannot be used. "
							"%1 %2 claimed ownership %3, using a copy of the title. ")
						.arg(item->ClaimantName())
						.arg(item->ClaimantId().StringId())
						.arg(item->ClaimTime().UTC().toLocalTime().toString());
				else
				{
					text = tr("Under the authority granted by this title of ownership, I hereby claim %1, tail number %2. "
							"All existing titles to %1 are now null and void.")
						.arg(shipName)
						.arg(shipId.StringId());
					emit ClaimTitle(shipId, shipName, Id(), Name(), EmpireId());

					// Eliminate my inventory item.
					item->Consume(1);
				}
				Thought(text);
			}
		}

		bool goodDeed = false;
		switch(item->CommodityId())
		{
		case CID_Water:
			{
				// Item usage decontaminates acid and puts out fires.
				AuCondition* condition = creature->FindConditionClone(AuConAcid::ConditionType());
				if(condition)
				{
					// Cure one acid.
					QString condName = condition->Name();
					if(item->Quality() >= condition->Quality())
					{
						if(creature->RemoveCondition(condition))
						{
							creature->SetModified();
							goodDeed = true;

							QString text(tr("Decontaminated %1 using %2.").arg(condName).arg(item->SayName()));
							Thought(text);

							// Notify the patient if it wasn't me.
							if(creature != this && !AgentInvisible())
							{
								if(AgentEntity())
									text = tr("%1 decontaminated my %2.").arg(Name()).arg(condName);
								else
									text = tr("%1 decontaminated my %2 using %3.").arg(Name()).arg(condName).arg(item->SayName());
								creature->Thought(text);
							}
						}
					}
					else
					{
						// Reduce one acid.
						if(creature->ReduceCondition(condition, item->Quality()))
						{
							creature->SetModified();
							goodDeed = true;

							AuCondition* redcon = creature->FindConditionClone(AuConAcid::ConditionType());
							if(redcon)
							{
								QString text(tr("Reduced %1 to %2 using %3.").arg(condName).arg(redcon->Name())
										.arg(item->SayName()));
								Thought(text);

								// Notify the patient if it wasn't me.
								if(creature != this && !AgentInvisible())
								{
									if(AgentEntity())
										text = tr("%1 reduced my %2 to %3.").arg(Name()).arg(condName).arg(redcon->Name());
									else
										text = tr("%1 reduced my %2 to %3 using %4.").arg(Name()).arg(condName).arg(redcon->Name()).
											arg(item->SayName());
									creature->Thought(text);
								}
								delete redcon;
							}
						}
					}
					delete condition;
					useItem = true;	// wear down the antidote
				}
				else
				{
					// Put out one fire.
					AuCondition* condition = creature->FindConditionClone(AuConFire::ConditionType());
					if(condition)
					{
						// Cure one acid.
						QString condName = condition->Name();
						if(creature->RemoveCondition(condition))
						{
							creature->SetModified();
							goodDeed = true;

							QString text(tr("Extinguished %1 using %2.").arg(condName).arg(item->SayName()));
							Thought(text);

							// Notify the patient if it wasn't me.
							if(creature != this && !AgentInvisible())
							{
								if(AgentEntity())
									text = tr("%1 extinguished my %2.").arg(Name()).arg(condName);
								else
									text = tr("%1 extinguished my %2 using %3.").arg(Name()).arg(condName).arg(item->SayName());
								creature->Thought(text);
							}
						}
						delete condition;
						useItem = true;	// wear down the antidote
					}
				}
			}
			break;

		case CID_Antidote:
			{
				// Item usage cures poison.
				AuCondition* condition = creature->FindConditionClone(AuConPoison::ConditionType());
				if(condition)
				{
					// Cure one poison.
					QString condName = condition->Name();
					int removalDamage = condition->RemovalCost();
					if(item->Quality() >= condition->Quality())
					{
						if(creature->RemoveCondition(condition))
						{
							if(removalDamage)
							{
								AuCombatAttacker attacker(Id(), tr("%1 curing %2").arg(Name()).arg(condName), EmpireId());

								AuCombatWeapon weapon(WM_Internal);
								weapon.m_CID = CID_Antidote;
								weapon.m_Quality = item->Quality();
								weapon.m_w1.m_Effect = WE_Poison;

								AuCombatResult result;
								result.m_Damage1 = g_Au.Dice().Roll(1,10);

								CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
							}
							creature->SetModified();
							goodDeed = true;

							QString text(tr("Cured %1 using %2.").arg(condName).arg(item->SayName()));
							Thought(text);

							// Notify the patient if it wasn't me.
							if(creature != this && !AgentInvisible())
							{
								if(AgentEntity())
									text = tr("%1 cured my %2.").arg(Name()).arg(condName);
								else
									text = tr("%1 cured my %2 using %3.").arg(Name()).arg(condName).arg(item->SayName());
								creature->Thought(text);
							}
						}
					}
					else
					{
						// Reduce one poisoning.
						if(creature->ReduceCondition(condition, item->Quality()))
						{
							if(removalDamage)
							{
								AuCombatAttacker attacker(Id(), tr("%1 reducing %2").arg(Name()).arg(condName), EmpireId());

								AuCombatWeapon weapon(WM_Internal);
								weapon.m_CID = CID_Antidote;
								weapon.m_Quality = item->Quality();
								weapon.m_w1.m_Effect = WE_Poison;

								AuCombatResult result;
								result.m_Damage1 = g_Au.Dice().Roll(1,10);

								CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
							}
							creature->SetModified();
							goodDeed = true;

							AuCondition* redcon = creature->FindConditionClone(AuConPoison::ConditionType());
							if(redcon)
							{
								QString text(tr("Reduced %1 to %2 using %3.").arg(condName).arg(redcon->Name())
										.arg(item->SayName()));
								Thought(text);

								// Notify the patient if it wasn't me.
								if(creature != this && !AgentInvisible())
								{
									if(AgentEntity())
										text = tr("%1 reduced my %2 to %3.").arg(Name()).arg(condName).arg(redcon->Name());
									else
										text = tr("%1 reduced my %2 to %3 using %4.").arg(Name()).arg(condName).arg(redcon->Name())
											.arg(item->SayName());
									creature->Thought(text);
								}
								delete redcon;
							}
						}
					}
					delete condition;
					useItem = true;	// wear down the antidote
				}
			}
			break;

		case CID_Antitoxin:
			{
				// Locate existing immunization, if any.
				AuConAntitoxin* oldAnti;
				const AuConAntitoxin* ob =
					dynamic_cast<const AuConAntitoxin*>(creature->FindConditionLock(AuConAntitoxin::ConditionType()));
				if(ob)
					oldAnti = (AuConAntitoxin*)ob->Clone();
				else
					oldAnti = 0;
				creature->UnlockConditions();

				QString text;
				if(oldAnti && oldAnti->EffectiveQuality() >= item->Quality())
				{
					if(oldAnti->EffectiveQuality() > item->Quality())
						text = QObject::tr("%1 is already immunized with a better quality antitoxin.").arg(creature->Name());
					else
						text = QObject::tr("%1 is already immunized with the same quality antitoxin.").arg(creature->Name());
					Thought(text);
				}
				else
				{
					// Patient is not immunized.
					if(oldAnti)
						creature->RemoveCondition(oldAnti);	// Remove the old antitoxin.

					// Add a new immunization.
					AuConAntitoxin* anti = new AuConAntitoxin(item->Quality(), AuTime::Now());
					creature->AddCondition(anti);
					creature->SetModified();
					goodDeed = true;

					if(oldAnti)
						text = QObject::tr("%1 immunization against disease boosted using %2.")
							.arg(creature->Name()).arg(item->SayName());
					else
						text = QObject::tr("%1 immunized against disease using %2.").arg(creature->Name()).arg(item->SayName());
					Thought(text);

					// Notify the patient if it wasn't me.
					if(creature != this && !AgentInvisible())
					{
						if(oldAnti)
						{
							if(AgentEntity())
								text = tr("%1 boosted my immunity to disease.").arg(Name());
							else
								text = tr("%1 boosted my immunization against disease using %3.").arg(Name()).arg(item->SayName());
						}
						else
						{
							if(AgentEntity())
								text = tr("%1 immunized me against disease.").arg(Name());
							else
								text = tr("%1 immunized me against disease using %3.").arg(Name()).arg(item->SayName());
						}
						creature->Thought(text);
					}
				}
				if(oldAnti)
					delete oldAnti;
				useItem = true;	// wear down the medical kit
			}
			break;

		case CID_MedicalKit:
			{
				// Item usage heals body.
				short int curhp = creature->CurrentHitPoints();
				short int maxhp = creature->MaxHitPoints();
				if(curhp < maxhp)
				{
					// Locate existing bandage, if any.
					AuCondition* oldBandage;
					const AuCondition* ob = creature->FindConditionLock(AuConBandage::ConditionType());
					if(ob)
						oldBandage = ob->Clone();
					else
						oldBandage = 0;
					creature->UnlockConditions();

					QString text;
					if(oldBandage && oldBandage->Quality() >= item->Quality())
					{
						if(oldBandage->Quality() > item->Quality())
							text = QObject::tr("%1 is already wrapped in better quality bandages.").arg(creature->Name());
						else
							text = QObject::tr("%1 is already wrapped in the same quality bandages.").arg(creature->Name());
						Thought(text);
					}
					else
					{
						// Patient is injured. Bandage them.
						if(oldBandage)
							creature->RemoveCondition(oldBandage);	// Remove the old bandage.

						// Add a new bandage.
						AuConBandage* bandage = new AuConBandage(item->Quality(), AuTime::Now());
						creature->AddCondition(bandage);
						creature->SetModified();
						goodDeed = true;

						if(oldBandage)
							text = QObject::tr("%1 bandages replaced using %2.").arg(creature->Name()).arg(item->SayName());
						else
							text = QObject::tr("%1 bandaged using %2.").arg(creature->Name()).arg(item->SayName());
						Thought(text);

						// Notify the patient if it wasn't me.
						if(curhp > 0 && creature != this && !AgentInvisible())
						{
							if(oldBandage)
							{
								if(AgentEntity())
									text = tr("%1 replaced my bandages.").arg(Name());
								else
									text = tr("%1 replaced my bandages using %3.").arg(Name()).arg(item->SayName());
							}
							else
							{
								if(AgentEntity())
									text = tr("%1 bandaged my wound.").arg(Name());
								else
									text = tr("%1 bandaged my wound using %3.").arg(Name()).arg(item->SayName());
							}
							creature->Thought(text);
						}
					}
					if(oldBandage)
						delete oldBandage;
					useItem = true;	// wear down the medical kit
				}
			}
			break;

		case CID_DNASampleKit:
			if(!isSelf)	// My inventory is already locked so IsWearingEnvironmentSuit will deadlock.
			{
				// Item usage samples DNA of the target.
				DaCitizen* citizen = dynamic_cast<DaCitizen*>(creature);
				DaPerson* person = dynamic_cast<DaPerson*>(creature);
				if(creature->IsWearingEnvironmentSuit())
				{
					// Environment suit on citizen or person prevents sampling their DNA.
					QString text(tr("DNA collection was blocked by the EV suit on %1.").arg(creature->Name()));
					Thought(text);

					if(person)
					{
						// Notify the person of the attempt.
						QString text(tr("My EV suit blocked a DNA collection attempt by %1.").arg(Name()));
						person->Thought(text);
					}

					useItem = true;								// wear down the DNA sample kit
					break;
				}

				// The unused DNA sample kit becomes a DNA sample that stores the DNA of the creature.
				replacementItem = AuItem::Create(CID_DNASample, item->Quality(), item->EmpireOfOrigin());
				if(replacementItem)
				{
					replacementItem->SetHitPoints(item->HitPoints());
					AuItemDNA* repDNA = dynamic_cast<AuItemDNA*>(replacementItem);
					Q_ASSERT(repDNA);
					AuDNABase* dna = creature->DNAnew();
					if(dna)
					{
						if(citizen || (person && !person->AgentAnimal()))
							repDNA->SetDNA(dna, true, true, creature->GrowthStage());		// Store DNA of sentient creature.
						else
							repDNA->SetDNA(dna, true, false, creature->GrowthStage());		// Store DNA of animal.
						delete dna;
					}

					// Notify the person of their success.
					QString text(tr("DNA sample collected from %1.").arg(creature->Name()));
					Thought(text);
					if(person)
					{
						// Notify the person they've been sampled.
						QString text(tr("%1 collected a sample of my DNA.").arg(Name()));
						person->Thought(text);
					}
				}
			}
			break;

		default:
			break;
		}

		// Player Statistics
		if(isSelf)
		{
			if(item->IsDetonatorWeapon() || item->IsFuseWeapon())
			{
				Stats().ItemUsedAsWeapon(*item);
				IncreaseSkill(AuSkill::Combat, 1);
			}
			else if(useItem || item->IsEnvironmentHelmet())
				Stats().ItemUsedOnSelf(*item);
		}
		else
		{
			if(goodDeed)
			{
				DaCitizen* ind = dynamic_cast<DaCitizen*>(creature);
				if(ind && ind->IsNpcIndigenous())
				{
					// Doing a good deed to an indigenous raises my reputation with the tribe.
					// At this range, it is considered contact with the village.
					DaWorldLL* llworld = FindParentWorldInEnv();
					if(llworld)
						llworld->IndigenousHelpedBy(Id(), true);
				}
			}

			if(useItem)
				Stats().ItemUsedOnOther(*item);
		}
	}
	else if(!InDesignEnvironment())
	{
		// Spacecraft and other things can only be targeted for item usage when not in a design studio. Otherwise those pesky
		// players disassemble the ship in the design studio for high-tech parts.
		DaVessel* ship = dynamic_cast<DaVessel*>(pTarget);
		if(ship)
		{
			// NOTE: Some tools can repair/remove multiple kinds of commodities. The same object should not be made of
			// overlapping commodities because a slight ambiguity occurs. The result of repairing/removing a commodity will
			// be in the order the commodities are added to the repairCIDs list here. e.g. A ScrewDriver will repair/remove
			// Computers before Gigacells.
			QList<COMMODITYID> repairCIDs;	// Filled with commodities that can be repaired/removed using the item.
			switch(item->CommodityId())
			{
			case CID_Water:
				{
					// Water can decontaminate acid and put out fires.
					AuCondition* condition = ship->FindConditionClone(AuConAcid::ConditionType());
					if(condition)
					{
						// Decontaminate one acid.
						QString condName = condition->Name();
						if(item->Quality() >= condition->Quality())
						{
							if(ship->RemoveCondition(condition))
							{
								ship->SetModified();

								QString text(tr("Decontaminated %1 using %2.").arg(condName).arg(item->SayName()));
								Thought(text);
							}
						}
						else
						{
							// Reduce one acid.
							if(ship->ReduceCondition(condition, item->Quality()))
							{
								ship->SetModified();

								AuCondition* redcon = ship->FindConditionClone(AuConAcid::ConditionType());
								if(redcon)
								{
									QString text(tr("Reduced %1 to %2 using %3.").arg(condName).arg(redcon->Name())
											.arg(item->SayName()));
									Thought(text);
									delete redcon;
								}
							}
						}
						delete condition;
						useItem = true;	// wear down the water
					}
					else
					{
						// Put out one fire.
						AuCondition* condition = ship->FindConditionClone(AuConFire::ConditionType());
						if(condition)
						{
							// Douse one fire.
							QString condName = condition->Name();
							if(ship->RemoveCondition(condition))
							{
								ship->SetModified();
								QString text(tr("Extinguished %1 using %2.").arg(condName).arg(item->SayName()));
								Thought(text);
							}
							delete condition;
							useItem = true;	// wear down the water
						}
					}
				}
				break;
			case CID_Hammer:
				repairCIDs.append(CID_Adamantine);
				repairCIDs.append(CID_Metal);
				repairCIDs.append(CID_Magmium);
				repairCIDs.append(CID_Vulcium);
				break;
			case CID_FittingWrench:
			case CID_Pliers:
			case CID_Wrench:
				// Modules
				for(int modcid=CID_Invalid; modcid<CID_Count; modcid++)
					if(DbCommodity::IsModule((COMMODITYID)modcid))
						repairCIDs.append((COMMODITYID)modcid);
				break;
			case CID_SolderIron:
				repairCIDs.append(CID_ElectronicPart);
				break;
			case CID_TorqueWrench:
				repairCIDs.append(CID_MechanicalPart);
				break;
			default:
				break;
			}

			// Attempt to repair a unit of damage on the ship.
			if(!repairCIDs.isEmpty())
			{
				if(isAltMode)
				{
					// Remove the commodity item from the targeted system.
					if(ship == Parent() || ship->IsReadyForRepairs())
					{
						// The removed item induces a repair requirement on the system.
						// An item will not be removed if the system is at zero hit points.
						for(QList<COMMODITYID>::const_iterator rci = repairCIDs.cbegin(); rci != repairCIDs.cend(); rci++)
						{
							AuSysId sysId;
							int status;
							quint8 quality = ship->RemoveComponent(ShipRoomType(), part, *rci, sysId, status);
							if(quality)
							{
								// The removed item is added to my inventory if it fits.
								AuItem* removedPart = AuItem::Create(*rci, quality, ship->EmpireId());
								if(removedPart)
								{
									if(!AddGearItemNoLock(m_Gear, *removedPart))
									{
										// Replace the part; it won't fit in my inventory.
										bool systemModuleChange = false;	// Don't change system module when putting it back.
										ship->ReplaceComponent(ShipRoomType(), part, *rci, quality, 1, sysId, systemModuleChange,
												false);

										QString text = tr("I cannot remove %1. There is no place in my gear to store it.")
												.arg(removedPart->SayName());
										Thought(text);
									}
									else
									{
										useItem = true;	// Wear down the tool.

										// Player Statistics
										Stats().ItemUsedOnOther(*item);
										IncreaseSkill(AuSkill::Engineer, 1);

										QString text;
										if(status == 4)
											text = tr("Removed the last %1 from %2 using %3.")
												.arg(removedPart->SayName())
												.arg(ship->SystemName(sysId))
												.arg(item->SayName());
										else
											text = tr("Removed %1 from %2 using %3.")
												.arg(removedPart->SayName())
												.arg(ship->SystemName(sysId))
												.arg(item->SayName());
										Thought(text);
									}
									delete removedPart;
								}

								// We stop looking once we have removed a single commodity, even if it didn't fit into my inventory.
								break;
							}
							else if(status == 3)
							{
								// The system cannot give up any more of those components.
								QString text;
								if(DbCommodity::IsModule(*rci) ||
										*rci == CID_ElectronicPart ||	// Vehicle recovery system.
										*rci == CID_MechanicalPart)		// Vehicle launcher.
								{
									if(sysId.Which())
										text = tr("No more %1 can be removed from the subsystem.").arg(g_Au.CommodityName(*rci));
									else
										text = tr("No more %1 can be removed from the system.").arg(g_Au.CommodityName(*rci));
								}
								else
									text = tr("No more %1 plates can be removed from the hull.").arg(g_Au.CommodityName(*rci));
								Thought(text);
							}
						}
					}
					else
					{
						// Parts cannot be removed from the outside of a ship unless it is ready for repairs.
						QString text(tr("Parts cannot be removed from the outside unless the ship is rigged for repairs."));
						Thought(text);
					}
				}
				else
				{
					// Repair the damaged commodity item.
					if(ship == Parent() || ship->IsReadyForRepairs())
					{
						// Tools can fix a number of items in one go based on their quality.
						int fixCount = (int)log2f(item->Quality() + 1) + 1;
						bool partInstalled = false;
						for(QList<COMMODITYID>::const_iterator rci = repairCIDs.cbegin(); rci != repairCIDs.cend(); rci++)
						{
							// Attempt to repair damage to the sighted system using parts from the hold. It may not be damaged.
							// Updates 'fixCount' to the count actually fixed.
							AuSysId fixedSysId;
							bool systemModuleChange = false;
							quint32 fixedCount = ship->RepairDamage(ShipRoomType(), part, *rci, fixCount, fixedSysId,
									systemModuleChange);
							useItem = fixedCount;					// "Use" the tool if any damage is repaired by this.
							if(useItem)
							{
								QString text;
								if(fixedCount > 1)
									text = tr("Repaired %1 with %3, using %2 spare parts from the hold.")
										.arg(ship->SystemName(fixedSysId))
										.arg(fixedCount)
										.arg(item->SayName());
								else
									text = tr("Repaired %1 with %3, using a spare part from the hold.")
										.arg(ship->SystemName(fixedSysId))
										.arg(item->SayName());
								Thought(text);
							}
							else
							{
								// The ship did not have the spare parts in the hold or the system is not damaged.
								// Try to install a part from my inventory.
								AuGearPos pos = m_Gear.FindBest(*rci);
								if(pos.IsGear())
								{
									// Found the best part of that type in my inventory.
									// Try to install it onto the targeted system.
									const AuItem* myPart = m_Gear.ItemAt(pos);
									Q_ASSERT(myPart);
									AuItem* partUsed = myPart->Clone();
									partUsed->SetCount(1);
									AuSysId fixedSysId;
									bool systemModuleChange = true;	// Allow system upgrade from personal gear.
									fixedCount = ship->RepairDamage(ShipRoomType(), part, *partUsed, 1, fixedSysId,
											systemModuleChange, AgentPowerEnabled());
									if(fixedCount)
									{
										// Installed onto ship. Consume item from my inventory.
										useItem = true;
										ConsumeItemNoLock(m_Gear, pos, IPS_Item, *partUsed);

										QString text;
										if(systemModuleChange)
											text = tr("Repaired %1 with %3, using %2 from my gear. "
													"The first new module changed the system type "
													"because all the old modules were gone. ")
												.arg(ship->SystemName(fixedSysId))
												.arg(partUsed->ListDesc())
												.arg(item->SayName());
										else
											text = tr("Repaired %1 with %3, using %2 from my gear.")
												.arg(ship->SystemName(fixedSysId))
												.arg(partUsed->ListDesc())
												.arg(item->SayName());
										Thought(text);
									}
									delete partUsed;
								}
							}

							if(useItem)
							{
								// Player Statistics
								Stats().ItemUsedOnOther(*item);
								Stats().SpacecraftRepairedByHand(ship);
								AuSysId sysId = ship->SightedSystem(part);
								if(sysId.System() != SYS_Count)
									Stats().SpacecraftSystemRepaired(sysId.System());
								IncreaseSkill(AuSkill::Engineer, 1);

								// We stop looking once we have done something.
								partInstalled = true;
								break;
							}
						}

						if(!partInstalled)
						{
							// No parts were installed. Try fixing simple hit points.
							quint32 hitsFixed = QualityBoosted(fixCount, item->Quality());
							AuSysId sysIdRepaired;
							quint32 fixedCount = ship->RepairHitPoints(part, hitsFixed, sysIdRepaired);
							if(fixedCount)
							{
								useItem = true;
								QString text;
								if(fixedCount > 1)
									text = tr("%3 repaired %L2 hit points of damage to %1.")
										.arg(ship->SystemName(sysIdRepaired))
										.arg(fixedCount)
										.arg(item->SayName());
								else
									text = tr("%3 repaired one hit point of damage to %1.")
										.arg(ship->SystemName(sysIdRepaired))
										.arg(item->SayName());
								Thought(text);
							}
						}
					}
					else
					{
						// Parts cannot be repaired from the outside of a ship unless it is ready for repairs.
						QString text(tr("Parts cannot be installed from the outside unless the ship is rigged for repairs."));
						Thought(text);
					}
				}
			}
		}
		else
		{
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(pTarget);
			if(vehicle)
			{
				// Vehicles can be refueled and repaired using inventory items.
				if(item->CommodityId() == CID_Water)
				{
					// Water can decontaminate acid and put out fires.
					AuCondition* condition = vehicle->FindConditionClone(AuConAcid::ConditionType());
					if(condition)
					{
						// Decontaminate one acid.
						QString condName = condition->Name();
						if(item->Quality() >= condition->Quality())
						{
							if(vehicle->RemoveCondition(condition))
							{
								vehicle->SetModified();

								QString text(tr("Decontaminated %1 using %2.").arg(condName).arg(item->SayName()));
								Thought(text);
							}
						}
						else
						{
							// Reduce one acid.
							if(vehicle->ReduceCondition(condition, item->Quality()))
							{
								vehicle->SetModified();

								AuCondition* redcon = vehicle->FindConditionClone(AuConAcid::ConditionType());
								if(redcon)
								{
									QString text(tr("Reduced %1 to %2 using %3.").arg(condName).arg(redcon->Name())
											.arg(item->SayName()));
									Thought(text);
									delete redcon;
								}
							}
						}
						delete condition;
						useItem = true;	// wear down the water
					}
					else
					{
						// Put out one fire.
						AuCondition* condition = vehicle->FindConditionClone(AuConFire::ConditionType());
						if(condition)
						{
							// Douse one fire.
							QString condName = condition->Name();
							if(vehicle->RemoveCondition(condition))
							{
								vehicle->SetModified();
								QString text(tr("Extinguished %1 using %2.").arg(condName).arg(item->SayName()));
								Thought(text);
							}
							delete condition;
							useItem = true;	// wear down the water
						}
					}
				}
				else if(item->CommodityId() == vehicle->FuelType())
				{
					// The commodity is fuel for the vehicle. Only use it if the vehicle needs the fuel.
					if(vehicle->Fuel() < vehicle->FuelMax())
					{
						// Refuel the vehicle using the commodity.
						vehicle->Refuel(*item);

						// Player Statistics
						Stats().ItemUsedOnOther(*item);
						IncreaseSkill(vehicle->DriverSkill(), 1);

						// Consume one of the fuel units.
						item->Consume(1);

						QString text(tr("%1 refueled using fuel from my gear.").arg(vehicle->Name()));
						Thought(text);
					}
				}
				else
				{
					// Attempt to load ordnance onto the vehicle.
					DaVehicle::WEAPON_RACK rack = vehicle->CanLoadAmmunition(*item);
					if(rack != DaVehicle::RACK_None)
					{
						// Rearm the vehicle using the item.
						vehicle->LoadAmmunition(*item, rack);

						// Player Statistics
						Stats().ItemUsedOnOther(*item);
						IncreaseSkill(vehicle->DriverSkill(), 1);

						// Consume one of the units.
						item->Consume(1);

						QString text;
						switch(rack)
						{
						case DaVehicle::RACK_None:
						case DaVehicle::RACK_Gun:
							text = tr("%1 rearmed gun using ordnance from my gear.").arg(vehicle->Name());
							break;
						case DaVehicle::RACK_Ord0:
							text = tr("%1 rearmed weapon rack 1 using ordnance from my gear.").arg(vehicle->Name());
							break;
						case DaVehicle::RACK_Ord1:
							text = tr("%1 rearmed weapon rack 2 using ordnance from my gear.").arg(vehicle->Name());
							break;
						}
						Thought(text);
					}
					else
					{
						// Vehicle determines effect of using the item.
						useItem = vehicle->RepairDamage(item->CommodityId());	// Wear down the tool if any damage is repaired.
						if(useItem)
						{
							// Player Statistics
							Stats().ItemUsedOnOther(*item);
							Stats().VehicleRepairedByHand(vehicle);
							IncreaseSkill(vehicle->DriverSkill(), 1);

							QString text(tr("%1 repaired using %2.").arg(vehicle->Name()).arg(item->SayName()));
							Thought(text);
						}
					}
				}
			}
			else
			{
				DaItem* pItem = dynamic_cast<DaItem*>(pTarget);
				if(pItem)
				{
					// If the item is a dead animal or fish carcass, this person may be able to butcher it.
					if(item->CommodityId() == CID_Knife || item->CommodityId() == CID_MagmiumKnife ||
							item->CommodityId() == CID_VulciumKnife)
					{
						// When butchering, the result is not immediately picked up.
						// Instead, the result is left lying where the item was.
						// The item 'pItem' disappears.
						quint8 quality = pItem->Quality();
						quint32 empireId = pItem->EmpireOfOrigin();
						bool dnaSentient = pItem->DNAIsSentient();
						bool dnaViable = pItem->DNAIsViable();
						GROWTHSTAGE growth = pItem->DNAGrowthStage();
						AuDNABase::CHROMATID dnaChroma = pItem->DNAChromatid();
						AuDNABase* dna = pItem->DNAnew();
						if(dna)
						{
							// DNA tells us what parts can be butchered from the item.
							if(pItem->CommodityId() == CID_Animal ||
									pItem->CommodityId() == CID_Fish ||
									pItem->CommodityId() == CID_Head)
							{
								// Player Statistics
								Stats().ItemUsedOnOther(*item);
								IncreaseSkill(AuSkill::Butcher, 1);

								AuMatrix4x4 t = pItem->Transform();

								if(pItem->CommodityId() == CID_Animal)
									DropItemButchered(CID_AnimalMeat, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
								else if(pItem->CommodityId() == CID_Fish)
									DropItemButchered(CID_FishMeat, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);

								if(dna->HasBrain(dnaChroma, pItem->CommodityId()))
								{
									t.Translate(.5, 0.5, 0);
									DaItem* item = DropItemButchered(CID_Brain, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
									if(item && !pItem->Inscription().isEmpty())
										item->Inscribe(pItem->Inscription());
								}

								if(dna->HasLeather(dnaChroma, pItem->CommodityId()))
								{
									t.Translate(-.75, 0.5, 1);
									DropItemButchered(CID_Leather, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
								}

								if(dna->HasBone(dnaChroma, pItem->CommodityId()))
								{
									t.Translate(-.5, -.6, .7);
									DropItemButchered(CID_Bone, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
								}

								if(dna->HasHeart(dnaChroma, pItem->CommodityId()))
								{
									t.Translate(.5, .5, .7);
									DropItemButchered(CID_Heart, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
								}

								if(dna->HasStomach(dnaChroma, pItem->CommodityId()))
								{
									t.Translate(.5, -.5, .7);
									DropItemButchered(CID_Stomach, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
								}

								if(dna->HasLungs(dnaChroma, pItem->CommodityId()))
								{
									t.Translate(-1, 1, .5);
									DropItemButchered(CID_Lungs, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
								}

								if(dna->HasGills(dnaChroma, pItem->CommodityId()))
								{
									t.Translate(.5, -1, 0);
									DropItemButchered(CID_Gills, quality, empireId, pItem->Parent(), t,
											dna, dnaChroma, dnaViable, dnaSentient, growth);
								}

								// The item atom can be killed now.
								pItem->Kill();
								useItem = true;	// Wear down the tool.

								QString text(tr("I butchered the %1.").arg(pItem->SayName()));
								Thought(text);
							}
						}
						else
						{
							// No DNA so use old time butchering algorithm.
							if(pItem->CommodityId() == CID_Animal)
							{
								// Player Statistics
								Stats().ItemUsedOnOther(*item);
								IncreaseSkill(AuSkill::Butcher, 1);

								// Animals reduce to meat and hide.
								AuMatrix4x4 t = pItem->Transform();

								// Create another item to be the meat portion.
								DropItemButchered(CID_AnimalMeat, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Create an item to be the leather portion.
								t.Translate(-.75, 0.5, 1);
								DropItemButchered(CID_Leather, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Create another item to be the bone portion.
								t.Translate(-.5, -.6, .7);
								DropItemButchered(CID_Bone, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Heart
								t.Translate(.5, .5, .7);
								DropItemButchered(CID_Heart, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Stomach
								t.Translate(.5, -.5, .7);
								DropItemButchered(CID_Stomach, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Lungs
								t.Translate(-1, 1, .5);
								DropItemButchered(CID_Lungs, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// The item atom can be killed now.
								pItem->Kill();
								useItem = true;	// Wear down the tool.

								QString text(tr("Butchering the %1 produced %2, %3, %4 and some organs.")
										.arg(pItem->SayName())
										.arg(g_Au.CommodityName(CID_AnimalMeat))
										.arg(g_Au.CommodityName(CID_Bone))
										.arg(g_Au.CommodityName(CID_Leather)));
								Thought(text);
							}
							else if(pItem->CommodityId() == CID_Fish)
							{
								// Player Statistics
								Stats().ItemUsedOnOther(*item);
								IncreaseSkill(AuSkill::Butcher, 1);

								// Fish reduce to fish meat and no hide.
								AuMatrix4x4 t = pItem->Transform();

								// Create another item to be the meat portion.
								DropItemButchered(CID_FishMeat, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Heart
								t.Translate(.5, .5, .7);
								DropItemButchered(CID_Heart, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Stomach
								t.Translate(.5, -.5, .7);
								DropItemButchered(CID_Stomach, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// Gills
								t.Translate(-1, 1, .5);
								DropItemButchered(CID_Gills, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// This atom can be killed now.
								pItem->Kill();
								useItem = true;	// Wear down the tool.

								QString text(tr("Butchering the %1 produced %2 and some organs.")
										.arg(pItem->SayName())
										.arg(g_Au.CommodityName(CID_FishMeat)));
								Thought(text);
							}
							else if(pItem->CommodityId() == CID_Head)
							{
								// Player Statistics
								Stats().ItemUsedOnOther(*item);
								IncreaseSkill(AuSkill::Butcher, 1);

								// Heads reduce to bone and brain.
								AuMatrix4x4 t = pItem->Transform();

								// Brain
								DaItem* item = DropItemButchered(CID_Brain, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);
								if(item && !pItem->Inscription().isEmpty())
									item->Inscribe(pItem->Inscription());

								// Bone
								t.Translate(-.5, -.6, .7);
								DropItemButchered(CID_Bone, quality, empireId, pItem->Parent(), t,
										dna, dnaChroma, dnaViable, dnaSentient, growth);

								// The item atom can be killed now.
								pItem->Kill();
								useItem = true;	// Wear down the tool.

								QString text(tr("Butchering the %1 produced %2 and a %3.")
										.arg(g_Au.CommodityName(CID_Head))
										.arg(g_Au.CommodityName(CID_Bone))
										.arg(g_Au.CommodityName(CID_Brain)));
								Thought(text);
							}
						}
						if(dna)
							delete dna;
					}
					else if(item->CommodityId() == CID_ScrewDriver)
					{
						// When disassembling, the result is not immediately picked up.
						// Instead, the result is left lying where the item was.
						// The item 'pItem' disappears.
						if(DbCommodity::IsModule(pItem->CommodityId()))
						{
							// Player Statistics
							Stats().ItemUsedOnOther(*item);
							//IncreaseSkill(AuSkill::Butcher, 1);

							// Modules reduce to their component parts.
							AuMatrix4x4 t = pItem->Transform();

							const DgData& gd = g_Au.LockDataForRead();
							for(DbManufacturingProcessArray::const_iterator pi = gd.m_bManufacturingProcessArray.cbegin();
									pi != gd.m_bManufacturingProcessArray.cend(); pi++)
							{
								if((*pi).CommodityId() == pItem->CommodityId())
								{
									// Found the process to manufacture the module.
									// Make a part for each of its components that are required and consumed by the process.
									quint8 quality = pItem->Quality();
									quint32 empireId = pItem->EmpireOfOrigin();
									double xoff = 0;
									for(DbManufacturingComponentArray::const_iterator ci =
											gd.m_bManufacturingComponentArray.cbegin();
											ci != gd.m_bManufacturingComponentArray.cend(); ci++)
									{
										if((*ci).m_idmp != (*pi).Id())
											continue;	// Skip components that go to other processes.
										if(!(*ci).IsRequired() || !(*ci).QtyConsumed())
											continue;	// Skip things that are not required and consumed.
										if(!g_Au.Commodity((*ci).CommodityId()).CanBeCargo())
											continue;	// Skip electricity and other intangible things.
										double yoff = 0;
										unsigned int amtToGet = ((*ci).QtyConsumed() + 1)/2;	// Extract 1/2 of the components.
										for(unsigned int ii=0; ii < amtToGet; ii++)
										{
											// Create an item of the component.
											t.Translate(xoff, yoff, 0);
											yoff += .083;
											DropItemDisassembled((*ci).CommodityId(), quality, empireId, pItem->Parent(), t);
										}
										xoff += .083;
									}
									break;
								}
							}
							g_Au.UnlockData();

							// The item atom can be killed now.
							COMMODITYID cid = pItem->CommodityId();
							pItem->Kill();
							useItem = true;	// Wear down the tool.

							QString text(tr("I disassembled the %1 using my %2.")
									.arg(g_Au.CommodityName(cid))
									.arg(item->SayName()));
							Thought(text);
						}
					}
					else if(item->CommodityId() == CID_DNASampleKit)
					{
						// Using a DNA sample kit on something.
						// Item usage samples DNA of the target item when it has viable DNA stored within it.
						if(pItem->CommodityId() == CID_DNASampleKit)
						{
							// Cannot take a sample from a sample.
							QString text(tr("DNA cannot be collected from %1.").arg(pItem->Name()));
							Thought(text);
						}
						else if(pItem->DNAIsViable())
						{
							// The unused DNA sample kit becomes a DNA sample that stores the DNA of the item.
							replacementItem = AuItem::Create(CID_DNASample, item->Quality(), item->EmpireOfOrigin());
							if(replacementItem)
							{
								replacementItem->SetHitPoints(item->HitPoints());
								AuItemDNA* repDNA = dynamic_cast<AuItemDNA*>(replacementItem);
								Q_ASSERT(repDNA);
								AuDNABase* dna = pItem->DNAnew();
								if(dna)
								{
									// DNA of sampled item.
									repDNA->SetDNA(dna, pItem->DNAIsViable(), pItem->DNAIsSentient(), pItem->DNAGrowthStage());
									delete dna;

									// Notify the person of their success.
									QString text(tr("DNA collected from %1.").arg(pItem->Name()));
									Thought(text);
								}
							}
						}
						else
						{
							AuDNABase* dna = pItem->DNAnew();
							if(dna)
							{
								// The target has DNA but it is not viable.
								delete dna;
								QString text(tr("DNA collection failed. The DNA of %1 is not viable.").arg(pItem->Name()));
								Thought(text);
							}
							else if(pItem->ItemType() == AuItemDNA::ItemType())
							{
								// The target can have DNA but it has none.
								QString text(tr("DNA collection failed. %1 has no DNA.").arg(pItem->Name()));
								Thought(text);
							}
							else
							{
								// The target does not store DNA.
								QString text(tr("DNA cannot be collected from %1.").arg(pItem->Name()));
								Thought(text);
							}
						}
					}
					else if(item->CommodityId() == CID_Seed)
					{
						// Plant the seed if possible.
						if(pItem->PlantSeed(*item))
							useItem = true;										// Consume the seed inventory item.
					}
				}
				else
				{
					DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(pTarget);
					if(llworld)
					{
						if(item->CommodityId() == CID_Flag)
						{
							AuPoint worldLoc = Transform().Project(targLoc);	// Location relative to the planet.
							useItem = ClaimWorld(llworld, worldLoc);			// Returns false if I have no empire.
						}
					}
					else
					{
						DaBd* bd = dynamic_cast<DaBd*>(pTarget);
						if(bd)
						{
							if(item->CommodityId() == CID_Water)
							{
								// Water can decontaminate acid and put out fires.
								AuCondition* condition = bd->FindConditionClone(AuConAcid::ConditionType());
								if(condition)
								{
									// Decontaminate one acid.
									QString condName = condition->Name();
									if(item->Quality() >= condition->Quality())
									{
										if(bd->RemoveCondition(condition))
										{
											bd->SetModified();

											QString text(tr("Decontaminated %1 using %2.").arg(condName).arg(item->SayName()));
											Thought(text);
										}
									}
									else
									{
										// Reduce one acid.
										if(bd->ReduceCondition(condition, item->Quality()))
										{
											bd->SetModified();

											AuCondition* redcon = bd->FindConditionClone(AuConAcid::ConditionType());
											if(redcon)
											{
												QString text(tr("Reduced %1 to %2 using %3.").arg(condName).arg(redcon->Name())
														.arg(item->SayName()));
												Thought(text);
												delete redcon;
											}
										}
									}
									delete condition;
									useItem = true;	// wear down the water
								}
								else
								{
									// Put out one fire.
									AuCondition* condition = bd->FindConditionClone(AuConFire::ConditionType());
									if(condition)
									{
										// Douse one fire.
										QString condName = condition->Name();
										if(bd->RemoveCondition(condition))
										{
											bd->SetModified();
											QString text(tr("Extinguished %1 using %2.").arg(condName).arg(item->SayName()));
											Thought(text);
										}
										delete condition;
										useItem = true;	// wear down the water
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Wear down/consume the original item.
	if(useItem)
		UseItem(*item, IPS_Item);

	return replacementItem;
}

DaItem* DaPerson::DropItemButchered(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, DaAtom* itemParent, AuMatrix4x4& t,
		AuDNABase* dna, AuDNABase::CHROMATID dnaChroma, bool dnaIsViable, bool dnaIsSentient, GROWTHSTAGE dnaGrowthStage)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!dna)
		return DropItemDisassembled(cid, quality, empireOfOrigin, itemParent, t);
	HANDEDNESS hand = RightHanded;
	AuHairStyle::STYLE hair = AuHairStyle::Unkempt;
	AuHairStyle::SHAVE shave = AuHairStyle::Unshaven;
	AuItemDNA* item = new AuItemDNA(dna, dnaChroma, dnaIsViable, dnaIsSentient, hand, dnaGrowthStage, hair, shave, cid, quality,
			empireOfOrigin);
	if(!item)
		return 0;

	AuMatrix4x4 ti(t * item->GroundUpVector().GetAlignmentTransform(AuPoint(0,0,1)));
	DaItem* li = new DaItem(*item);
	li->ConnectSignals();
	li->Create();
	li->SetPosAndOrient(AuAtomPos(itemParent->Id(), ti));
	ether->AddAtom(li);

	li->SetInMotion();
	li->SetInScene();					// Don't decay too fast right away.
	g_Au.AtomAdded(li->Id());
	li->SetNormal();					// New atom is ready for normal thread processing.
	li->TickTock();						// Get it moving so there is no delay before it falls.

	delete item;
	return li;
}

DaItem* DaPerson::DropItemDisassembled(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, DaAtom* itemParent, AuMatrix4x4& t)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	AuItem* item = AuItem::Create(cid, quality, empireOfOrigin);
	if(!item)
		return 0;

	AuMatrix4x4 ti(t * item->GroundUpVector().GetAlignmentTransform(AuPoint(0,0,1)));
	DaItem* li = new DaItem(*item);
	li->ConnectSignals();
	li->Create();
	li->SetPosAndOrient(AuAtomPos(itemParent->Id(), ti));
	ether->AddAtom(li);

	li->SetInMotion();
	li->SetInScene();					// Don't decay too fast right away.
	g_Au.AtomAdded(li->Id());
	li->SetNormal();					// New atom is ready for normal thread processing.
	li->TickTock();						// Get it moving so there is no delay before it falls.

	delete item;
	return li;
}

// 'true' if 'scriptId' is in my list of open stories
bool DaPerson::IsStoryOpen(unsigned int scriptId, const AuStories& stories) const
{
	m_JobsLock.lockForRead();
	for(AuJobList::const_iterator ji = m_Jobs.cbegin(); ji != m_Jobs.cend(); ji++)
	{
		const AuJobStory* storyJob = dynamic_cast<const AuJobStory*>(*ji);
		if(storyJob && storyJob->StoryInstance() && stories.ScriptId(storyJob->StoryInstance()) == scriptId)
		{
			m_JobsLock.unlock();
			return true;
		}
	}
	m_JobsLock.unlock();
	return false;
}

bool DaPerson::TestStoryCatalyst(const AuScript& script, const AuStories& stories, QString* startBlock, QMap<QString,
		QVariant>& vars)
{
	//qDebug() << "DaPerson::TestStoryCatalyst" << Id() << Name();
	if(IsStoryOpen(script.Id(), stories))
	{
		//qDebug("DaPerson::TestStoryCatalyst script %d is already open", script.Id());
		return false;	// Story is already open by this person.
	}

	if(HasStartedStory(script.Id()))
	{
		// This person has started the story script before.
		// Scripts sometimes specify an interval.
		// The interval is the minutes required between story starts.
		AuTime lastStart = LastStoryTerminated(script.Id());
		int secs = lastStart.ElapsedSec();
		int minutes = secs / 60;
		if(minutes < script.Interval())
		{
			//qDebug("DaPerson::TestStoryCatalyst too soon to restart script %d, minutes %d, script interval %d",
					//script.Id(), minutes, script.Interval());
			return false;	// Too soon to restart this story.
		}
	}

	return script.TestCatalyst(*this, startBlock, vars);
}

void DaPerson::StartStory(unsigned int scriptId, quint32 storyInstance)
{
	Q_ASSERT(storyInstance);
	if(!CanStartStory())
		return;													// Person already has the max stories open.

	QString title;
	QString author;
	QString brief;
	if(scriptId)
	{
		title = g_Au.StoryTitle(storyInstance);
		author = g_Au.StoryAuthor(storyInstance);
		brief = g_Au.StoryBrief(storyInstance);
	}
	else
	{
		m_ScriptLock.lock();
		if(m_Script)
		{
			title = m_Script->Title();
			author = m_Script->Author();
			brief = m_Script->Brief();
		}
		m_ScriptLock.unlock();
	}

	// Person can start a nu story instance.
	AuJobStory* story = new AuJobStory(storyInstance, title, author, brief);
	m_JobsLock.lockForWrite();
	m_Jobs.append(story);
	m_tmJobsChange.Set();
	m_JobsLock.unlock();

	Remember(AuMemStory(scriptId));	// Remember when we last started every different story script.

	// Player Statistics
	Stats().StoryStarted();

	SetModified();
}

void DaPerson::StoryChoicePicked(quint32 storyInstance, qint8 pickValue, AuStory* story, const AuScripts& scripts,
		const AuId& offeredBy)
{
	//qDebug("DaPerson::StoryChoicePicked %d %s storyInstance %d value %d", Id().GetID(), (const char*)Name().toAscii(),
			//storyInstance, pickValue);
	if(storyInstance == 1 && !offeredBy.IsNull())
	{
		// Choice offered by an AI controlled citizen, not part of a story.
		// Locate the AI controlled citizen who offered the choice.
		DaAtom* system = FindEnvironmentAtom();
		if(system)
		{
			DaSentient* sent = dynamic_cast<DaSentient*>(system->FindIdRecursive(offeredBy));
			if(sent)
				sent->ConversationChoicePicked(*this, pickValue);
		}

		return;
	}

	if(!IsInStoryInstance(storyInstance))
	{
		//qDebug("DaPerson::StoryChoicePicked %d %s story not open", Id().GetID(), (const char*)Name().toAscii());
		DepartStory(storyInstance, story);
		return;
	}

	// This person really does have the story instance open.
	if(story)
	{
		bool result = false;
		if(story->ScriptId())
		{
			// Standard story script.
			AuScripts::const_iterator sit = scripts.find(story->ScriptId());
			if(sit != scripts.end())
				result = story->ChoicePicked(*this, pickValue, (*sit).second);
		}
		else
		{
			// Custom story script.
			m_ScriptLock.lock();
			if(m_Script)
				result = story->ChoicePicked(*this, pickValue, *m_Script);
			m_ScriptLock.unlock();
		}

		if(result)
		{
			// Notify global data so story can be updated in db.
			g_Au.StoryChanged(storyInstance);
			SetModified();
		}
	}
	else
		qWarning("DaPerson::StoryChoicePicked ERROR story instance %d not found", storyInstance);
}

// Sends the dialog of the current story block without re-executing it.
void DaPerson::StoryResendCurrentBlock(quint32 storyInstance, const AuStory* story, const AuScripts& scripts)
{
	if(!IsInStoryInstance(storyInstance))
	{
		qDebug("DaPerson::StoryResendCurrentBlock %d %s story not open", Id().GetID(), (const char*)Name().toLatin1());
		return;
	}

	// This person really does have the story instance open.
	if(story)
	{
		// Found the story data, which tells us the script and the current block of the script.
		if(story->ScriptId())
		{
			// Standard story script.
			AuScripts::const_iterator sit = scripts.find(story->ScriptId());
			if(sit != scripts.end())
				story->ResendBlock((*sit).second, *this);
		}
		else
		{
			// Custom story script.
			m_ScriptLock.lock();
			if(m_Script)
				story->ResendBlock(*m_Script, *this);
			m_ScriptLock.unlock();
		}
	}
	else
		qWarning("DaPerson::StoryResendCurrentBlock ERROR story instance %d not found", storyInstance);
}

// Send the hint associated with the current story block.
void DaPerson::StorySendHint(quint32 storyInstance, const AuStory* story, const AuScripts& scripts)
{
	if(!IsInStoryInstance(storyInstance))
	{
		qDebug("DaPerson::StorySendHint %d %s story not open", Id().GetID(), (const char*)Name().toLatin1());
		return;
	}

	// This person really does have the story instance open.
	if(story)
	{
		// Found the story data, which tells us the script and the current block of the script.
		if(story->ScriptId())
		{
			// Standard story script.
			AuScripts::const_iterator sit = scripts.find(story->ScriptId());
			if(sit != scripts.end())
				story->SendHint((*sit).second, *this);
		}
		else
		{
			// Custom story script.
			m_ScriptLock.lock();
			if(m_Script)
				story->SendHint(*m_Script, *this);
			m_ScriptLock.unlock();
		}
	}
	else
		qWarning("DaPerson::StorySendHint ERROR story instance %d not found", storyInstance);
}

void DaPerson::TickStories(const AuScripts& scripts)
{
	m_JobsLock.lockForRead();
	bool e = m_Jobs.isEmpty();
	m_JobsLock.unlock();
	if(e)
		return;

	m_JobsLock.lockForWrite();
	//qDebug() << "DaPerson::TickStories" << Id() << Name() << "jobs" << m_Jobs.count();
	for(AuJobList::iterator ji = m_Jobs.begin(); ji != m_Jobs.end(); ji++)
	{
		AuJobStory* storyJob = dynamic_cast<AuJobStory*>(*ji);
		if(!storyJob)
			continue;

		quint32 storyInstance = storyJob->StoryInstance();
		if(storyInstance)
		{
			// A story instance is open.
			//qDebug() << "DaPerson::TickStories" << Id() << Name() << "storyInstance" << storyInstance;
			// Does the location/condition of this person affect the outcome of the story.
			bool storyDeparted = false;
			AuStory* story = g_Au.LockStory(storyInstance);
			if(story)
			{
				// Execute the story to see if conditions are right to branch or end.
				int result = 3;	// Start a result code that cancels the story. A valid story must be found to keep it alive.
				if(story->ScriptId())
				{
					// Standard story script.
					AuScripts::const_iterator sit = scripts.find(story->ScriptId());
					if(sit != scripts.end())
						result = story->Execute((*sit).second, *this);
				}
				else
				{
					// Custom story script.
					m_ScriptLock.lock();
					if(m_Script)
						result = story->Execute(*m_Script, *this);
					m_ScriptLock.unlock();
				}

				if(result == 1)
				{
					// Story branched to the next story label and executed it.
					// Notify global data so story can be updated in db.
					g_Au.StoryChanged(storyInstance);
					SetModified();
				}
				else if(result == 2)
				{
					// Story ended.
					DepartStoryNoLock(storyInstance, story);
					storyDeparted = true;

					// Player Statistics
					Stats().StoryCompleted(story->ScriptId());
				}
				else if(result == 3)
				{
					// The story is not valid.
					// Cancel the story as if it was dropped.
					DepartStoryNoLock(storyInstance, story);
					storyDeparted = true;
				}
			}
			else
			{
				//qDebug("DaPerson::TickStories Story instance %d not found", storyInstance);
				// Depart the story since it no longer exists.
				DepartStoryNoLock(storyInstance, story);
				storyDeparted = true;
			}
			g_Au.UnlockStory(storyInstance);

			// If a story was ended, break out of the loop. DepartStory modifies my m_Jobs list and any remaining stories
			// can wait until the next cycle to be ticked. None are that urgent.
			if(storyDeparted)
				break;
		}
	}
	m_JobsLock.unlock();
	//qDebug("DaPerson::TickStories %d %s done", Id().GetID(), (const char*)Name().toAscii());
}

bool DaPerson::HasFinishedStory(quint32 scriptId) const
{
	return Stats().FinishedStory(scriptId);
}

void DaPerson::DepartStory(quint32 storyInstance, AuStory* story)
{
	m_JobsLock.lockForWrite();
	DepartStoryNoLock(storyInstance, story);
	m_JobsLock.unlock();
}

void DaPerson::DepartStoryNoLock(quint32 storyInstance, AuStory* story)
{
	//qDebug() << "DaPerson::DepartStoryNoLock" << storyInstance;
	for(AuJobList::iterator ji = m_Jobs.begin(); ji != m_Jobs.end(); ji++)
	{
		AuJobStory* storyJob = dynamic_cast<AuJobStory*>(*ji);
		if(storyJob && storyJob->StoryInstance() == storyInstance)
		{
			// Found the job that corresponds to the story instance. Delete it and remove it from my jobs list.
			delete storyJob;
			m_Jobs.erase(ji);
			m_tmJobsChange.Set();
			SetModified();
			//qDebug() << "DaPerson::DepartStoryNoLock story removed from jobs list" << storyInstance;
			break;
		}
	}

	// Remove this person's id from the story's list of people participating. Clean up the story if I'm the last to leave.
	if(story)
	{
		RememberStoryTerminated(story->ScriptId());
		if(!story->Depart(Id()))
		{
			// This was the last person to depart the story.
			// Notify global data so story can be deleted from db.
			g_Au.StoryDeleted(storyInstance);
			//qDebug() << "DaPerson::DepartStoryNoLock story instance deleted" << storyInstance;
		}
		else
		{
			// Notify global data so story can be updated in db.
			g_Au.StoryChanged(storyInstance);
			//qDebug() << "DaPerson::DepartStoryNoLock story instance changed" << storyInstance;
		}
	}
	//else
		//qDebug() << "DaPerson::DepartStoryNoLock no story to depart" << storyInstance;

	StoryClearScript();	// So it doesn't restart.
}

DaBd* DaPerson::CurrentBd() const
{
	if(BdId().IsNull())
		return 0;

	// Person is in a city.
	const DaWorld* planet = FindParentPlanetInEnv();
	if(!planet)
		return 0;

	return planet->FindBd(BdId());
}

AuPoint DaPerson::Acceleration() const
{
	// When mounted, acceleration is determined by the mount according to my desires. My dude sits still.
	if(IsRider())
		return AuPoint(0,0);

	//if(m_PowerMove)
		//qDebug() << "DaPerson::Acceleration" << Id() << Name() << "movePower" << m_PowerMove;
	bool aiControlled = IsAIControlled();

	// Entity is not affected by gravity.
	if(!AgentEntity())
	{
		if(IsInGravity())
		{
			// I am inside the powerful immediate gravitation force of a planet's atmosphere or a spacecraft or a vehicle.
			// This creature does not get to control acceleration when not touching the ground, unless it is flying, swimming,
			// in space, or positioned for using a gravity well, hatch, or turret. Did I miss anything?
			if(!CanControlMovement())
				return DaSentient::Acceleration();				// Gravity force alone.
		}
		else
		{
			// I am outside any powerful immediate gravitational force. I can still be drawn to the nearest gravity well.
			// In space I can control my movement if I am wearing a space suit or if I am aboard a spacecraft.
			if(IsBodyDyingOrDead())
				return DaSentient::Acceleration();				// Gravity force alone.
			else if(!IsInSpacecraft() && !IsInAtmosphere() && !InDesignEnvironment())
			{
				if(InBlackHoleGravity())
					return DaSentient::Acceleration();			// Gravity force alone.

				// EVA suit holds position in most outer space environments, except near a black hole.
				const AuGear* gear = LockGearForRead();
				const AuItem* item = gear->ArmorItemWorn();
				bool g = !item || !item->IsEnvironmentSuit();
				UnlockGear();
				if(g)
					return DaSentient::Acceleration();			// Gravity force alone.
			}
		}
	}

	AuPoint vec(Vector());

	float speed;
	AuPoint lookMove;
	if(aiControlled)
	{
		// Movement speed determined by my current general body pose.
		speed = MovementSpeed(1);	// in feet per second

		double force;
		if(speed == 0)
			force = 0;
		else
		{
			// Speed is non-zero.
			if(IsOnGround() || IsLadderPose() || (InLadderWell() && IsInGravity()))
				force = (speed + GravityStandard()) * 2;	// Need extra force to overcome ground friction and gravity.
			else
				force = speed;
		}

		// Movement actions.
		AuPoint fmove;		// Forward/backward movement.
		if(speed != 0)
			fmove = AuPoint(0,1,0);	// This animal is in a moving pose.

		// Cumulative movement vector.
		// Tilt movement vector according to head tilt.
		lookMove = fmove.RotateX(CurrentHeadTilt());
		if(!lookMove.IsZero())
			lookMove.Unit(force);

		// I can control my movements so limit forward movement distance to stop better at hatches and things.
		float mdl = MoveDistLimit();
		if(mdl >= 0)
		{
			AuPoint friction = Friction();
			if(vec.y + lookMove.y - friction.y > mdl)
				lookMove.y = mdl - vec.y + friction.y;
		}
	}
	else
	{
		// Movement speed is determined by my current general body pose and burden carried.
		speed = MovementSpeed((float)m_PowerMove/15);		// speed is in feet per second

		// Movement actions.
		bool moveForward = false;
		bool moveBackward = false;
		unsigned int acti = Actions();
		if(!(acti & ACT_Brake))
		{
			if((acti & ACT_MoveForward) || (IsFlying() && !CanHover()))
				moveForward = true;
			else if(acti & ACT_MoveBackward)
				moveBackward = true;
		}

		AuPoint fmove;		// Forward/backward movement.
		float hillmod = 1;
		if(moveForward || moveBackward)
		{
			// Calculate forward movement vector.
			if(moveForward)
				fmove = AuPoint(0,1,0);
			else
				fmove = AuPoint(0,-1,0);
			if(IsOnGround() && IsInGravity() && !CanBeUnderGround() &&
					!IsInGravityWell() && !IsInVisibleWater() && !IsJumping() && !IsFlying() && !IsLadderPose())
			{
				// Tilt forward vector to move parallel to the ground.
				double gDot = fmove.Dot(TerrainNormal());	// Distance along gravity vector.
				fmove = fmove.RotateX(-asin(gDot));			// Rotate movement vector parallel to the ground.
				if(gDot < 0)
					hillmod = 1+gDot;						// Uphill movement reduces speed based on steepness.
			}
			else
			{
				// Tilt forward movement vector according to head tilt.
				fmove = fmove.RotateX(CurrentHeadTilt());
			}
		}

		AuPoint lmove;		// Lateral movement.
		if(acti & ACT_MoveLeft)
			lmove = AuPoint(-0.75,0,0);
		else if (acti & ACT_MoveRight)
			lmove = AuPoint(0.75,0,0);

		AuPoint vmove;		// Vertical movement
		if(acti & ACT_MoveDown)
			vmove = AuPoint(0,0,-1);
		else if (acti & ACT_MoveUp)
		{
			// Only allow up movement in zero G, swimming, flying, or climbing ladder.
			if(!IsInGravity() || IsInGravityWell() || IsInVisibleWater() || IsFlying() || IsLadderPose() || AgentEntity())
				vmove = AuPoint(0,0,0.5);
		}

		// Calculate movement force.
		double force;
		if(speed == 0)
			force = 0;
		else
		{
			// Speed is non-zero.
			if((IsInGravity() && (IsOnGround() || InLadderWell())) || IsLadderPose())
				force = (speed + GravityStandard()) * 2;	// Need extra force to overcome ground friction and gravity.
			else
				force = speed;
		}

		if(force > 0)
		{
			if(InDesignerNotPreview() && !IsOnGround())
			{
				const DaDesignBase* pde = FindParentDesignBaseInEnv();
				if(pde && !pde->IsRenderSolid(Id()) && !pde->IsPreview())
				{
					// Move fast in design studio wire frame view.
					// 'force' is currently in feet per second, based on my DNA.
					// Lets modify it to produce a uniform strength from all creatures in a design environment.
					if(HasDNA3())
					{
						// Take DNA speed out, leaving the speed factor. See DpBody::MovementSpeed.
						force /= ((float)DNA3().Speed + 1);
					}
					else
					{
						//DHH animalia
					}
					force *= 20;						// Treat all creatures like they are the same speed. Standwalk = 20.
				}
			}
			else
			{
				// Modulate final force by hill modifier.
				//qDebug() << "force" << force << "hillmod" << hillmod << "finalforce" << (force * hillmod);
				force *= hillmod;
			}
		}

		// Cumulative movement vector.
		lookMove = fmove + lmove + vmove;
		if(!lookMove.IsZero())
			lookMove.Unit(force);
	}

	// Now that we know the final desired movement vector, check to see if the
	// person is too fast and accelerating. This person can't go faster if
	// they're already beyond their maximum speed; they can decelerate however.
	// This logic only applies when inside atmosphere range of a planet or
	// inside a spacecraft.
	if(IsInGravity())
	{
		if(fabs(vec.x) > speed/2)
		{
			// Currently moving too fast in X; only permit deceleration.
			if(fabs(vec.x + lookMove.x) > fabs(vec.x))
				lookMove.x = 0;
		}
		if(fabs(vec.y) > speed)
		{
			// Currently moving too fast in Y; only permit deceleration.
			if(fabs(vec.y + lookMove.y) > fabs(vec.y))
				lookMove.y = 0;
		}
		if(vec.z > speed/2 || vec.z < -speed)
		{
			// Currently moving too fast in Z; only permit deceleration.
			if(fabs(vec.z + lookMove.z) > fabs(vec.z))
				lookMove.z = 0;
		}

		// Adjust up vector based on air density.
		if(IsFlying() && lookMove.z > 0)
		{
			float fd = AirDensity() - .15f;	// According to DaCreature::CanFly, atmo density .15 is needed to fly.
			//qDebug() << "flying up" << lookMove.z << "fd" << fd << "moveZ" << (lookMove.z * fd * 2);
			if(fd < 0)
				fd = 0;
			lookMove.z *= fd * 2;	// Earth standard density is .7.
		}

		// Add force of gravity, if any.
		// Entity is not affected by gravity.
		if(!AgentEntity())
			lookMove += DaSentient::Acceleration();
	}
	else if(IsAIControlled())
	{
		// I am weightless. Try to control my speed.
		float mdl = MoveDistLimit();
		if(mdl > 0 && IsMovingPose())
		{
			// I must be hoping to get somewhere EVA. Good luck!
			// Try to keep my speed down to where I can stop at the destination.
			const float brakeForce = 15;						// Force of braking in fps along each XYZ axis.
			float secsToStopX = fabs(vec.x) / brakeForce;	// Seconds to stop current X vector with brakes on.
			float secsToStopY = fabs(vec.y) / brakeForce;	// Seconds to stop current Y vector with brakes on.
			float secsToStopZ = fabs(vec.z) / brakeForce;	// Seconds to stop current Z vector with brakes on.
			float secsToGetThereX = 0;
			float secsToGetThereY = 0;
			float secsToGetThereZ = 0;
			if(vec.x)
			{
				secsToGetThereX = mdl / fabs(vec.x);	// Seconds to arrive in X at current speed.
				if(secsToStopX >= secsToGetThereX)
					lookMove.x = 0;								// Going too fast in X. Do not add to velocity in X.
			}
			if(vec.y)
			{
				secsToGetThereY = mdl / fabs(vec.y);	// Seconds to arrive in Y at current speed.
				if(secsToStopY >= secsToGetThereY)
					lookMove.y = 0;								// Going too fast in Y. Do not add to velocity in Y.
			}
			if(vec.z)
			{
				secsToGetThereZ = mdl / fabs(vec.z);	// Seconds to arrive in Z at current speed.
				if(secsToStopZ >= secsToGetThereZ)
					lookMove.z = 0;								// Going too fast in Z. Do not add to velocity in Z.
			}

			//qDebug() << "DaPerson::Acceleration" << Id() << Name() << "dist" << mdl << "lookMove" << lookMove
				//<< "secsToStop" << secsToStopX << secsToStopY << secsToStopZ
				//<< "secsToGetThere" << secsToGetThereX << secsToGetThereY << secsToGetThereZ;
		}
		else
			lookMove = AuPoint(0,0);							// I don't want to move.
	}
	//lookMove.Dump("accel lookMove");
	return lookMove;
}

AuPoint DaPerson::Friction() const
{
	// When mounted, friction is determined by the mount.
	if(IsRider())
		return AuPoint(0,0);

	// Determine if brake is on.
	bool aiControlled = IsAIControlled();
	if(!aiControlled)
	{
		if(!(Actions() & ACT_Brake))
			return DaSentient::Friction();							// Friction from environment alone when not trying to stop.

		if(InDesignerNotPreview())
			return AuPoint(10000,10000,10000);						// Braking in design studio needs to stop me NOW.
	}

	// Check for cases in which this person cannot control their braking.
	if(IsInGravity())
	{
		// I am inside the powerful immediate gravitation force of a planet's atmosphere or a spacecraft or a vehicle.
		// This creature does not get to control acceleration when not touching the ground, unless it is flying, swimming,
		// in space, or positioned for using a gravity well, hatch, or turret. Did I miss anything?
		if(!CanControlMovement())
			return DaSentient::Friction();						// Friction from environment alone.
	}
	else
	{
		// I am outside any powerful immediate gravitational force. I can still be drawn to the nearest gravity well.
		// In space I can control my movement if I am wearing a space suit or if I am aboard a spacecraft.
		if(IsBodyDyingOrDead())
			return DaSentient::Friction();						// Friction from environment alone.
		else if(!IsInSpacecraft() && !IsInAtmosphere())
		{
			const AuGear* gear = LockGearForRead();
			const AuItem* item = gear->ArmorItemWorn();
			bool f = !item || !item->IsEnvironmentSuit();
			UnlockGear();
			if(f)
				return DaSentient::Friction();					// Friction from environment alone.
		}

		// I am weightless. Try to control my speed.
		if(aiControlled)
		{
			bool applyBrake = false;
			float mdl = MoveDistLimit();
			if(mdl > 0 && IsMovingPose())
			{
				// I must be hoping to get somewhere EVA. Good luck!
				// Try to keep my speed down to where I can stop at the destination.
				const float brakeForce = 15;						// Force of braking in fps along each XYZ axis.
				AuPoint vec(Vector());
				float secsToStopX = fabs(vec.x) / brakeForce;	// Seconds to stop current X vector with brakes on.
				float secsToStopY = fabs(vec.y) / brakeForce;	// Seconds to stop current Y vector with brakes on.
				float secsToStopZ = fabs(vec.z) / brakeForce;	// Seconds to stop current Z vector with brakes on.
				float secsToGetThereX = 0;
				float secsToGetThereY = 0;
				float secsToGetThereZ = 0;
				if(vec.x)
				{
					secsToGetThereX = mdl / fabs(vec.x);	// Seconds to arrive in X at current speed.
					if(secsToStopX >= secsToGetThereX)
						applyBrake = true;							// Going too fast in X. Apply brakes in X.
				}
				if(vec.y)
				{
					secsToGetThereY = mdl / fabs(vec.y);	// Seconds to arrive in Y at current speed.
					if(secsToStopY >= secsToGetThereY)
						applyBrake = true;							// Going too fast in Y. Apply brakes in Y.
				}
				if(vec.z)
				{
					secsToGetThereZ = mdl / fabs(vec.z);	// Seconds to arrive in Z at current speed.
					if(secsToStopZ >= secsToGetThereZ)
						applyBrake = true;							// Going too fast in Z. Apply brakes in Z.
				}

				//qDebug() << "DaSentient::Friction" << Id() << Name() << "dist" << mdl << "applyBrake" << applyBrake
					//<< "secsToStop" << secsToStopX << secsToStopY << secsToStopZ
					//<< "secsToGetThere" << secsToGetThereX << secsToGetThereY << secsToGetThereZ;
			}
			else
				applyBrake = true;									// I don't want to move.

			// Apply normal braking if I want.
			if(applyBrake)
				return DaSentient::Friction() + AuPoint(15,15,15);
		}
	}

	if(aiControlled)
		return DaSentient::Friction();								// Friction from environment alone.

	// Apply normal braking.
	if(!IsOnGround() && CanFly() && IsFlying())
		return DaSentient::Friction() + AuPoint(30,30,30);		// Flying creatures need lots of friction in the air to stop.
	else
		return DaSentient::Friction() + AuPoint(15,15,15);
}

QString DaPerson::TitleName() const
{
	QString s(AssignedName());

	const DgData& gd = g_Au.LockDataForRead();
	QString title = gd.EmpireCitizenTitle(EmpireId(), Id());
	g_Au.UnlockData();
	if(!title.isEmpty())
	{
		if(s.isEmpty())
			return title + " " + tr("Person");
		else
			return title + " " + s;
	}

	// Return name without title.
	if(s.isEmpty())
		return tr("Person");
	else
		return s;
}

QString DaPerson::Name() const
{
	if(AgentEntity())
		return tr("Entity");
	else if(AgentAnimal())
		return SpeciesName();	// Get the name of my specie, or a generic name when the specie is not named.

	QString s(AssignedName());
	if(s.isEmpty())
		return tr("Person");
	else
		return s;
}

QString DaPerson::Description() const
{
	if(AgentEntity())
		return tr("Entity");
	return DaSentient::Description();
}

QString DaPerson::SpotDescription(quint32 spotterEmpire) const
{
	if(AgentEntity())
		return tr("Entity");

	// Get the name of the species, or a generic name when the species is not named.
	if(AgentAnimal())
	{
		// Report as if this agent is a wild animal.
		QString name = tr("%1 %2")
			.arg(GenderName(Gender()))
			.arg(SpeciesName());

		int curHits = CurrentHitPoints();
		int maxHits = MaxHitPoints();
		if(curHits == maxHits)
			name += tr(" %1 hp").arg(maxHits);
		else
			name += tr(" %1/%2 hp").arg(curHits).arg(maxHits);
		return name;
	}

	// Construct the typical spot description of this person.
	QString name = tr("%1, %2 %3")
		.arg(Name())
		.arg(GenderName(Gender()))
		.arg(SpeciesName());

	int curHits = CurrentHitPoints();
	int maxHits = MaxHitPoints();
	if(curHits == maxHits)
		name += tr(" %1 hp").arg(maxHits);
	else
		name += tr(" %1/%2 hp").arg(curHits).arg(maxHits);

	if(EmpireId())
	{
		QString empire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(EmpireId(), empire);
		POLITICAL_STANCE stance = gd.EmpirePoliticalStance(spotterEmpire, EmpireId());
		g_Au.UnlockData();
		switch(stance)
		{
		case POL_Friend:
			return tr("Friendly %1 %2").arg(empire).arg(name);
		case POL_Vassal:
			return tr("Overlord %1 %2").arg(empire).arg(name);
		case POL_Neutral:
			return tr("Neutral %1 %2").arg(empire).arg(name);
		case POL_Enemy:
			return tr("Enemy %1 %2").arg(empire).arg(name);
		case POL_Empire:
			return tr("%1 %2").arg(empire).arg(name);
		}
	}
	return name;
}

AuGearPos DaPerson::CurrentLightThatIsOn() const
{
	// Can't use my light when inside an enclosed vehicle.
	if(IsInVehicle())
		return AuGearPos();

	// Not inside an enclosed vehicle.
	if(AgentEntity() || IsDyingHighborn() || IsNewbornHighborn())
		return AuGearPos(IP_My_Head);							// Entity emits light from its body.

	m_GearLock.lockForRead();
	AuGearPos pos = m_Gear.LightOnPos();
	m_GearLock.unlock();
	return pos;
}

// Attempts to get an item from the cargo hold and place it into my inventory.
void DaPerson::GetFromCargo(const AuItem& item)
{
	// Locate my parent spacecraft.
	DaVessel* ship = FindParentVesselInEnv();
	if(ship)
	{
		if(IsInBerth() &&														// Person is in their berth AND
				(ship->CanPassengersAccessSeaChest() ||							// policy allows chest access OR
				 !AuRosterEntry::IsPassenger(ship->BerthTypeOccupied(Id()))))	// it's not a passenger berth.
		{
			// When in a berth, I am accessing my sea chest.
			const AuItemHash& cargo = LockSeaChestForRead();
			AuItemHash::const_iterator cargoI = cargo.Find(item);
			if(cargoI == cargo.end())
			{
				UnlockSeaChest();
				return;	// The item was not found in my sea chest.
			}
			AuItem* myItem = (*cargoI).second->Clone();
			UnlockSeaChest();

			myItem->SetCount(1);	// One item was retrieved.

			// Test my inventory to see if there is room for the nu item.
			if(!AddGearItem(*myItem))
			{
				delete myItem;
				return;	// The item did not fit into my inventory.
			}

			// Remove the item from my sea chest.
			if(LockSeaChestForWrite().TakeItem(*myItem, 1))
				m_tmSeaChestChange.Set();
			UnlockSeaChest();
			delete myItem;
		}
		else
		{
			// Locate the item aboard the ship.
			AuItem* myItem = 0;
			if(item.CommodityId() == CID_Hydrogen)
			{
				// Look in the ship's fuel tank.
				if(ship->FuelQuantity() < 1)
					return;	// The ship has insufficient fuel.
				myItem = AuItem::Create(CID_Hydrogen, ship->FuelQuality(), ship->EmpireId());
			}
			else
			{
				// Look in the ship's cargo hold.
				const AuItemHash& sinv = ship->LockCargoForRead();
				AuItemHash::const_iterator cargoI = sinv.Find(item);
				if(cargoI == sinv.end())
				{
					ship->UnlockCargo();
					return;	// The item was not found in the ship's cargo hold.
				}
				myItem = (*cargoI).second->Clone();
				ship->UnlockCargo();

				myItem->SetCount(1);	// One item was retrieved.
			}

			// Test my inventory to see if there is room for the nu item.
			if(AddGearItem(*myItem))
			{
				// The item fit into my inventory.
				// Remove the item from the cargo hold/fuel tank of the ship.
				ship->RemoveCargo(*myItem, myItem->Count());
			}

			delete myItem;
		}
	}
	else if(!IsAboardVehicle() &&
			InIndustry() != IID_None &&
			!SpawnCity().IsNull() &&
			SpawnCity() == BdId() &&
			FuzzyEquals(InIndustryLL().Longitude(), SpawnDevLL().Longitude()) &&
			FuzzyEquals(InIndustryLL().Latitude(), SpawnDevLL().Latitude()))
	{
		// When in my home, I am accessing my sea chest.
		const AuItemHash& cargo = LockSeaChestForRead();
		AuItemHash::const_iterator cargoI = cargo.Find(item);
		if(cargoI == cargo.end())
		{
			UnlockSeaChest();
			return;	// The item was not found in my sea chest.
		}
		AuItem* myItem = (*cargoI).second->Clone();
		UnlockSeaChest();

		myItem->SetCount(1);	// One item was retrieved.

		// Test my inventory to see if there is room for the nu item.
		if(!AddGearItem(*myItem))
		{
			delete myItem;
			return;	// The item did not fit into my inventory.
		}

		// Remove the item from my sea chest.
		if(LockSeaChestForWrite().TakeItem(*myItem, 1))
			m_tmSeaChestChange.Set();
		UnlockSeaChest();
		delete myItem;
	}
}

// Attempts to put an item from my inventory into the cargo hold.
void DaPerson::PutIntoCargo(const AuItem& item)
{
	// Locate the item in my own inventory.
	AuGearPos pos;
	AuItem* myItem = 0;
	m_GearLock.lockForRead();
	for(AuGear::const_iterator it = m_Gear.begin(); it != m_Gear.end(); it++)
	{
		if((*it).IsItem(item))
		{
			// The offered item has been located.
			if(!IsInBerth() && (*it).Item().Content() && !(*it).Item().Content()->empty())
				break;	// Non-empty containers may not be put into the cargo hold. In berth they go into sea chest.
			myItem = (*it).Item().Clone();
			myItem->SetCount(1);
			pos = AuGearPos(*it);
			break;
		}
		else if((*it).Item().Content())
		{
			const AuItemContent* contents = (*it).Item().Content();
			for(AuItemContent::const_iterator cit = contents->begin(); cit != contents->end(); cit++)
			{
				if((*cit).second->IsItem(item))
				{
					// The offered item has been located in a container.
					if(!IsInBerth() && (*cit).second->Content() && !(*cit).second->Content()->empty())
						break;	// Non-empty containers may not be put into the cargo hold. In berth they go into sea chest.
					pos = AuGearPos(*it, (*cit).first);
					myItem = (*cit).second->Clone();
					myItem->SetCount(1);
					break;
				}
			}
			if(myItem)
				break;
		}
	}
	m_GearLock.unlock();
	if(!myItem)
		return;	// The item was not found in my own inventory.

	// Locate my parent spacecraft.
	DaVessel* ship = FindParentVesselInEnv();
	if(ship)
	{
		if(IsInBerth() &&														// Person is in their berth AND
				(ship->CanPassengersAccessSeaChest() ||							// policy allows chest access OR
				 !AuRosterEntry::IsPassenger(ship->BerthTypeOccupied(Id()))))	// it's not a passenger berth.
		{
			// When in a berth, I am accessing my sea chest.
			// Test the sea chest to see if there is room for the nu item.
			if(!CanAddToSeaChest(*myItem))
			{
				delete myItem;
				return;	// Cannot add the item to my sea chest.
			}

			// Put the item into the my sea chest.
			LockSeaChestForWrite().AddItem(*myItem);
			m_tmSeaChestChange.Set();
			UnlockSeaChest();
		}
		else
		{
			// Test the ship's inventory to see if there is room for the nu item.
			if(!ship->CanAddCargo(*myItem))
			{
				delete myItem;
				return;	// Ship cannot add the item to its cargo.
			}

			// Put the item into the cargo hold/fuel tank of the ship.
			ship->AddCargo(*myItem);
		}
	}
	else if(!IsAboardVehicle() &&
			InIndustry() != IID_None &&
			!SpawnCity().IsNull() &&
			SpawnCity() == BdId() &&
			FuzzyEquals(InIndustryLL().Longitude(), SpawnDevLL().Longitude()) &&
			FuzzyEquals(InIndustryLL().Latitude(), SpawnDevLL().Latitude()))
	{
		// When in my home, I am accessing my sea chest.
		// Test the sea chest to see if there is room for the nu item.
		if(!CanAddToSeaChest(*myItem))
		{
			delete myItem;
			return;	// Cannot add the item to my sea chest.
		}

		// Put the item into the my sea chest.
		LockSeaChestForWrite().AddItem(*myItem);
		m_tmSeaChestChange.Set();
		UnlockSeaChest();
	}

	delete myItem;

	// Take the item from my inventory.
	ConsumeGear(pos, 1);
}

bool DaPerson::CanAddToSeaChest(const AuItem& iNew) const
{
	// Capacity of sea chest must not be exceeded.
	const AuItemHash& cargo = LockSeaChestForRead();
	int seaChestRoom = SeaChestCapacity() - cargo.TotalUnits();
	bool v;
	if((int)iNew.Count() <= seaChestRoom)
		v = cargo.CanAddItem(iNew);
	else
		v = false;
	UnlockSeaChest();
	return v;
}

// 'hit' is the atom that was hit.
// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaPerson::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	if(hitNormal.IsZero())
	{
		SetVector(AuPoint(0,0));
		return AuPoint(0,0);
	}
	return DaUnit::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
}

// Locates the person's assigned spacecraft in the solar system.
DaVessel* DaPerson::FindAssignedShip()
{
	// Nothing to do if this person isn't spawn-tied to a ship.
	if(SpawnShip().IsNull())
		return 0;

	// This person might be aboard their ship.
	DaVessel* ship = FindParentVesselInEnv();
	if(ship->Is(SpawnShip()))
		return ship;

	// Let's search this person's planet.
	DaWorld* planet = FindParentPlanetInEnv();
	if(planet)
	{
		// This person is at a planet. This search will also check this planet's moons, if any.
		DaVessel* ship = dynamic_cast<DaVessel*>(planet->FindIdRecursive(SpawnShip()));
		if(ship)
			return ship;
	}

	// Didn't find this person's ship nearby. Search the entire solar system.
	DaAtom* system = FindEnvironmentAtom();
	if(system)
		return dynamic_cast<DaVessel*>(system->FindIdRecursive(SpawnShip()));

	// This person isn't in a solar system and they're not aboard their ship. They're probably in trouble.
	return 0;
}

void DaPerson::operator>>(QDataStream& out) const
{
	DaSentient::operator>>(out);
	out << m_AgentBits;

	quint8 hasWhat = m_AutoEat;	// bit 0
	if(m_InitScenario != 0)
		hasWhat |= 0x02;
	hasWhat |= 0x80;			// bit 8 says we are no longer storing spawn galaxy and there is only one sea chest
	hasWhat |= 0x40;			// Active ping time not stored.

	out << hasWhat;
	out << m_PowerJump;
	out << m_PowerMove;
	out << m_PowerMoveMax;
	out << m_Stats;

	if(hasWhat & 0x02)
		out << m_InitScenario;

	m_PersonLock.lockForRead();
	out << m_ArenaEntered;
	out << m_ArenaExitInv;
	out << m_ArenaExitLoc;
	out << m_ArenaExitParent;
	out << m_Dad;
	out << m_DesignPass;
	AuTime lastClaim;
	out << lastClaim;
	out << AuTime();	//m_LastJumpOrMovePower;
	out << m_Log;
	out << m_LogTime;
	out << m_Mom;
	out << m_Selfie;
	out << m_Worms;

	out << m_AccountBalance;
	out << m_LoanBalance;
	out << AuTime();	//m_tmBalanceChange;

	out << m_ArenaShipDesign;
	out << m_Con;
	out << (quint8)m_Lineage;
	out << AuTime();	//m_tmOfficeChange;

	out << m_CurrentGearPos;
	out << (quint8)m_CurrentGearSub;
	out << AuTime();	//m_tmItemUseChange;

	out << m_Domain;
	out << AuTime();	//m_DomainChange;

	for(int mp=0; mp<10; mp++)
		out << m_MapPos[mp];

	out << m_Reputation;
	out << AuTime();	//m_tmReputationChange;

	out << (quint8)m_BerthType;
	out << m_SpawnBd;
	out << m_SpawnBdName;
	out << m_SpawnBdSystem;
	out << m_SpawnBdSystemName;
	out << m_SpawnDevLL;
	out << m_SpawnShip;
	out << m_SpawnShipIsPrivate;
	out << m_SpawnShipName;
	out << AuTime();	//m_tmSpawnChange;

	out << m_uiActions;
	out << m_byRotSpeedX;
	out << m_byRotSpeedZ;
	out << m_TurnCount;
	out << m_TurnIndex;
	for(int ti=0; ti<20; ti++)
	{
		out << m_TurnX[ti];
		out << m_TurnZ[ti];
		AuTime tmTurn;
		out << tmTurn;
	}
	AuTime tmLastBrake;
	out << tmLastBrake;
	out << m_SpawnTime;
	AuTime tmLastMoveLControl;
	out << tmLastMoveLControl;
	AuTime tmLastMoveSControl;
	out << tmLastMoveSControl;
	AuTime tmLastMoveTControl;
	out << tmLastMoveTControl;
	AuTime tmLastMoveVControl;
	out << tmLastMoveVControl;
	AuTime tmLastWeaponFired;
	out << tmLastWeaponFired;

	m_PersonLock.unlock();

	m_GearLock.lockForRead();
	out << m_Gear;
	out << AuTime();	//m_tmGearChange;	
	out << m_WeightCarried;
	m_GearLock.unlock();

	m_JobsLock.lockForRead();
	out << m_Jobs;
	out << AuTime();	//m_tmJobsChange;
	m_JobsLock.unlock();

	m_MemoryLock.lockForRead();
	out << m_Memories;
	m_MemoryLock.unlock();

	m_OnLineLock.lockForRead();
	quint8 ui8 = m_AccountOnLine;	// bottom nibble
	if(m_AccountInactive)
		ui8 |= 0x80;				// top bit
	out << ui8;
	out << m_Ping;
	out << m_tmLastOnLine;
	m_OnLineLock.unlock();

	m_RewardsLock.lockForRead();
	out << m_RewardsOwed;
	out << AuTime();	//m_tmRewardChange;
	m_RewardsLock.unlock();

	m_ScriptLock.lock();
	if(m_Script)
	{
		out << (quint8)1;
		out << *m_Script;
	}
	else
		out << (quint8)0;
	m_ScriptLock.unlock();

	m_SeaChestLock.lockForRead();
	out << m_SeaChest;
	out << AuTime();	//m_tmSeaChestChange;	
	m_SeaChestLock.unlock();
}

void DaPerson::operator<<(QDataStream& in)
{
	DaSentient::operator<<(in);
	in >> m_AgentBits;

	quint8 hasWhat;
	in >> hasWhat;

	m_AutoEat = hasWhat & 0x01;	// bit 0
	in >> m_PowerJump;
	in >> m_PowerMove;
	in >> m_PowerMoveMax;
	in >> m_Stats;

	if(hasWhat & 0x02)
		in >> m_InitScenario;

	m_PersonLock.lockForWrite();
	if(!(hasWhat & 0x40))
	{
		AuTime activePing;
		in >> activePing;
	}
	in >> m_ArenaEntered;
	in >> m_ArenaExitInv;
	in >> m_ArenaExitLoc;
	in >> m_ArenaExitParent;
	in >> m_Dad;
	in >> m_DesignPass;
	AuTime lastClaim;
	in >> lastClaim;
	AuTime lastJumpOrMovePower;
	in >> lastJumpOrMovePower;
	in >> m_Log;
	in >> m_LogTime;
	in >> m_Mom;
	in >> m_Selfie;
	in >> m_Worms;

	in >> m_AccountBalance;
	in >> m_LoanBalance;
	AuTime tmBalanceChange;
	in >> tmBalanceChange;

	in >> m_ArenaShipDesign;
	in >> m_Con;
	quint8 ui8;
	in >> ui8;
	m_Lineage = (LINEAGE_TYPE)ui8;
	AuTime tmOfficeChange;
	in >> tmOfficeChange;

	in >> m_CurrentGearPos;
	in >> ui8;
	m_CurrentGearSub = (INVENTORYSUB)ui8;
	AuTime tmItemUseChange;
	in >> tmItemUseChange;

	in >> m_Domain;
	AuTime domainChange;
	in >> domainChange;

	for(int mp=0; mp<10; mp++)
		in >> m_MapPos[mp];

	in >> m_Reputation;
	AuTime tmReputationChange;
	in >> tmReputationChange;

	in >> ui8;
	m_BerthType = (AuRosterEntry::TYPE)ui8;
	in >> m_SpawnBd;
	in >> m_SpawnBdName;
	in >> m_SpawnBdSystem;
	in >> m_SpawnBdSystemName;
	in >> m_SpawnDevLL;
	if(!(hasWhat & 0x80))
		in >> ui8;	// used to be spawn galaxy
	in >> m_SpawnShip;
	in >> m_SpawnShipIsPrivate;
	in >> m_SpawnShipName;
	AuTime tmSpawnChange;
	in >> tmSpawnChange;

	in >> m_uiActions;
	in >> m_byRotSpeedX;
	in >> m_byRotSpeedZ;
	in >> m_TurnCount;
	in >> m_TurnIndex;
	for(int ti=0; ti<20; ti++)
	{
		in >> m_TurnX[ti];
		in >> m_TurnZ[ti];
		AuTime tmTurn;
		in >> tmTurn;
	}
	AuTime tmLastBrake;
	in >> tmLastBrake;
	in >> m_SpawnTime;;
	AuTime tmLastMoveLControl;
	in >> tmLastMoveLControl;
	AuTime tmLastMoveSControl;
	in >> tmLastMoveSControl;
	AuTime tmLastMoveTControl;
	in >> tmLastMoveTControl;
	AuTime tmLastMoveVControl;
	in >> tmLastMoveVControl;
	AuTime tmLastWeaponFired;
	in >> tmLastWeaponFired;

	m_PersonLock.unlock();

	m_GearLock.lockForWrite();
	in >> m_Gear;
	AuTime tmGearChange;
	in >> tmGearChange;	
	in >> m_WeightCarried;
	m_GearLock.unlock();

	m_JobsLock.lockForWrite();
	in >> m_Jobs;
	AuTime tmJobsChange;
	in >> tmJobsChange;
	m_JobsLock.unlock();

	m_MemoryLock.lockForWrite();
	in >> m_Memories;
	m_MemoryLock.unlock();

	m_OnLineLock.lockForWrite();
	in >> ui8;
	m_AccountOnLine = (ACCOUNT)(ui8 & 0x0f);	// bottom nibble
	m_AccountInactive = ui8 & 0x80;				// top bit
	in >> m_Ping;
	in >> m_tmLastOnLine;
	m_OnLineLock.unlock();

	m_RewardsLock.lockForWrite();
	in >> m_RewardsOwed;
	AuTime tmRewardChange;
	in >> tmRewardChange;
	m_RewardsLock.unlock();

	m_ScriptLock.lock();
	if(m_Script)
		delete m_Script;
	in >> ui8;
	if(ui8)
	{
		m_Script = new AuScript;
		in >> *m_Script;
	}
	else
		m_Script = 0;
	m_ScriptLock.unlock();

	m_SeaChestLock.lockForWrite();
	if(hasWhat & 0x80)
	{
		in >> m_SeaChest;
	}
	else
	{
		// Used to store four sea chests, one in each dimension.
		in >> m_SeaChest;

		AuItemHash s;
		in >> s;
		in >> s;
		in >> s;
	}
	AuTime tmSeaChestChange;
	in >> tmSeaChestChange;	
	m_SeaChestLock.unlock();
}

// Returns true if this person is sitting in a captain's chair, regardless of viewing mode.
bool DaPerson::IsInCaptainsChair(const DaVessel* ship) const
{
	if(IsCommandStationPose())
		return true;										// Console is a captain's chair.
	else if(IsEngineStationPose() ||
			IsFireControlStationPose() ||
			IsNavStationPose() ||
			IsHelmStationPose() ||
			IsSensorStationPose() ||
			IsShieldStationPose())
		return ship->IsCommandConsole(StationId());		// Console could be a captain's chair in another viewing mode.
	return false;
}

bool DaPerson::CanBeUnderGround() const
{
	if(AgentEntity())
		return true;
	return DaSentient::CanBeUnderGround();
}

// Returns true for agent regardless of agent powers state.
// Only returns true if this player is actually on line.
bool DaPerson::IsOnLineAsAgent() const
{
	if(g_Au.IsSceneApp())
	{
		// Scene client has to answer the question without using m_AccountOnLine, which is not sent to this atom.
		// Determine the client person's unit id and if they are an agent.
		const DgData& gd = g_Au.LockDataForRead();
		AuId clientUnit = gd.ClientUnitId();
		bool isAgent = gd.ClientPlayerIsAgent();
		g_Au.UnlockData();

		// Only return true if I am actually the client player's unit and they are an agent.
		return Is(clientUnit) ? isAgent : false;
	}
	else
		return AcctIsAgent(AccountTypeOnLine());
}

bool DaPerson::IsOnLineAsMinion() const
{
	if(g_Au.IsSceneApp())
	{
		// Scene client has to answer the question without using m_AccountOnLine, which is not sent to this atom.
		// Determine the client person's unit id and if they are an agent.
		const DgData& gd = g_Au.LockDataForRead();
		AuId clientUnit = gd.ClientUnitId();
		bool isMinion = gd.ClientPlayerIsMinion();
		g_Au.UnlockData();

		// Only return true if I am actually the client player's unit and they are an agent.
		return Is(clientUnit) ? isMinion : false;
	}
	else
		return AcctIsMinion(AccountTypeOnLine());
}

// Returns true for architect regardless of agent powers state.
// Only returns true if this player is actually on line.
bool DaPerson::IsOnLineAsArchitect() const
{
	if(g_Au.IsSceneApp())
	{
		// Scene client has to answer the question without using m_AccountOnLine, which is not sent to this atom.
		// Determine the client person's unit id and if they are an architect.
		const DgData& gd = g_Au.LockDataForRead();
		AuId clientUnit = gd.ClientUnitId();
		bool isArchitect = gd.ClientPlayerIsArchitect();
		g_Au.UnlockData();

		// Only return true if I am actually the client player's unit and they are an architect.
		return Is(clientUnit) ? isArchitect : false;
	}
	else
		return AcctIsArchitect(AccountTypeOnLine());
}

// Returns true for programmer regardless of agent powers state.
// Only returns true if this player is actually on line.
bool DaPerson::IsOnLineAsProgrammer() const
{
	if(g_Au.IsSceneApp())
	{
		// Scene client has to answer the question without using m_AccountOnLine, which is not sent to this atom.
		// Determine the client person's unit id and if they are a programmer.
		const DgData& gd = g_Au.LockDataForRead();
		AuId clientUnit = gd.ClientUnitId();
		bool isProgrammer = gd.ClientPlayerIsProgrammer();
		g_Au.UnlockData();

		// Only return true if I am actually the client player's unit and they are a programmer.
		return Is(clientUnit) ? isProgrammer : false;
	}
	else
		return AcctIsProgrammer(AccountTypeOnLine());
}

bool DaPerson::CurrentWeaponIsShotgun(float& range, float& patternAngle) const
{
	// One chart I found showed a shotgun spread as reaching 50cm in radius at 25m range.
	// That is the arc tangent of (50cm/2500cm), which is 1.145762838 degrees.
	patternAngle = 1.145762838f*DEG2RADf;

	AuCombatWeapon weapon(WM_Not);
	int ammoCur, ammoFull;
	WeaponAtUsePos(CurrentItemPos(), CurrentItemSub(), weapon, ammoCur, ammoFull);
	range = weapon.m_Range;
	return weapon.m_CID == CID_Shotgun;
}

AuStation::TYPE DaPerson::EnterConsole(DaCapsule* capsule, CONTROL_CONTEXT cc)
{
	AuEther* ether = Ether();
	if(!ether)
		return AuStation::TYPE_Count;
	quint16 stationId;
	AuStation station;
	quint16 which;
	if(!capsule->CanEnterConsole(*this, stationId, station, which))
		return AuStation::TYPE_Count;

	if(station.Type() == AuStation::Integrated_Designer)
	{
		// Entered a designer console. Create a designer instance.
		if(InDesignEnvironment())
		{
			if(cc != CONTROL_Client)
				Thought(tr("The designer station cannot be entered while in the designer preview."));
			return AuStation::TYPE_Count;
		}
		else
		{
			if(cc != CONTROL_Client)
			{
				DaDesignBase* pdbase = capsule->FindDesigner(stationId);
				if(!pdbase)
				{
					// Enter new style designer. No mode is chosen yet. User picks mode, e.g. File|New|Spacecraft
					// Create a nu design studio atom subordinate to the parent terrain.
					DaDesigner* pde = new DaDesigner(stationId);
					pde->ConnectSignals();
					pde->Create();
					// Set the transform to the scene atom's current position.
					// This is where they will exit the studio.
					pde->SetPosAndOrient(Pos());
					ether->AddAtom(pde);
					g_Au.AtomAdded(pde->Id());

					// The person needs to become subordinate to the design environment.
					AuMatrix4x4 t;
					t.Translate(0, 0, GroundHeightAboveCG());
					ForcePosStop(AuAtomPos(pde->Id(), t));
					SetModified();
					emit EnteredDesigner(Id(), DaDesignBase::DESIGN_Unknown);

					// Start with a new design to establish a mode and, more importantly, the owner.
					pde->NewDesign(Id(), Name(), AuScDesign());

					pde->SetNormal();	// New atom is ready for normal thread processing.
					pde->TickTockRecursive();
				}
				else if(pdbase->IsNormal())
				{
					// The design environment exists and is not slated to die.
					// Join the architect inside.
					// The person becomes subordinate to the design environment.
					AuMatrix4x4 t;
					t.Translate(0, 0, GroundHeightAboveCG());
					ForcePosStop(AuAtomPos(pdbase->Id(), t));
					SetModified();
					pdbase->InitDesignPlane(Id());
					emit EnteredDesigner(Id(), pdbase->DesignMode());
				}
			}
			return station.Type();
		}
	}
	else
	{
		if(cc != CONTROL_Client)
		{
			// Kick AI occupant if there is one manning the console.
			capsule->KickAIStationOccupant(stationId, station.SeatIndex());

			// Wake up the doctor; he might be sleeping.
			if(AuStation::IsPatient(station.Type()))
				capsule->WakeDoctor(*this);
		}

		// Move myself to the console seat position.
		// Must call SitAtConsole after atom is moved to console location.
		ForcePosStop(AuAtomPos(capsule->Id(), station.OpM()));
		AuPose pose = SitAtConsole(stationId, station.Type(), station.OccPos(), station.SeatIndex(), which);
		SyncController(AuSyncPose(pose, 0, stationId, station.SeatIndex()));
		InvalidateEnvironment();
		SetAtRest();
		SetModified();
		return station.Type();
	}
}

bool DaPerson::IsSightTarget(const DaAtom* toWho) const
{
	// Don't sight myself.
	if(toWho == this)
		return false;

	// Creature is not sighted by themself, bay weapons, or base weapons.
	if(!DaSentient::IsSightTarget(toWho))
		return false;

	// Person cannot be targeted when awaiting spawn.
	if(IsAwaitingSpawn())
		return false;

	// Ethereal cannot be targeted.
	if(IsEthereal())
		return false;

	return true;
}

void DaPerson::LoanBorrow(double amount)
{
	if(amount <= 0)
		return;
	m_PersonLock.lockForWrite();
	m_LoanBalance += amount;
#if !defined(Q_OS_WIN)
	m_LoanBalance = round(m_LoanBalance * 100.) / 100.;	// Round to the nearest .01.
#endif
	m_tmBalanceChange.Set();
	m_PersonLock.unlock();
	SetModified();
}

void DaPerson::LoanPay(double amount)
{
	if(amount <= 0)
		return;
	m_PersonLock.lockForWrite();
	m_LoanBalance -= amount;
	if(m_LoanBalance < 0)
		m_LoanBalance = 0;
#if !defined(Q_OS_WIN)
	m_LoanBalance = round(m_LoanBalance * 100.) / 100.;	// Round to the nearest .01.
#endif
	m_tmBalanceChange.Set();
	m_PersonLock.unlock();
	SetModified();
}

void DaPerson::CreditAccount(double amount)
{
	if(amount == 0)
		return;
	if(amount < 0)
		qDebug("CreditAccount amount is negative: %g. Why not use DebitAccount?", amount);
	m_PersonLock.lockForWrite();
	m_AccountBalance += amount;
	if(m_AccountBalance < 0)	// Could happen when 'amount' is negative.
		m_AccountBalance = 0;
#if !defined(Q_OS_WIN)
	else
	{
		m_AccountBalance = round(m_AccountBalance * 100.) / 100.;	// Round to the nearest .01.
		if(m_AccountBalance > 1000000000000000)
			m_AccountBalance = 1000000000000000;
	}
#endif
	m_tmBalanceChange.Set();
	m_PersonLock.unlock();
	SetModified();
}

void DaPerson::DebitAccount(double amount)
{
	if(amount == 0)
		return;
	m_PersonLock.lockForWrite();
	m_AccountBalance -= amount;
	if(m_AccountBalance < 0)
		m_AccountBalance = 0;
#if !defined(Q_OS_WIN)
	else
		m_AccountBalance = round(m_AccountBalance * 100.) / 100.;	// Round to the nearest .01.
#endif
	m_tmBalanceChange.Set();
	m_PersonLock.unlock();
	SetModified();
}

void DaPerson::JobAccepted(const AuJobOffer& jobOffer, const AuScript& script)
{
	Q_ASSERT(jobOffer.Job());
	switch(jobOffer.Job()->Type())
	{
	case AuJob::Story:
		Q_ASSERT(CanStartStory());
		{
			// Insure the player is eligible to start the story.
			QString startBlock;
			const AuStories* stories = g_Au.LockStoriesForRead();
			bool okToStart;
			QMap<QString, QVariant> vars;
			if(stories)
				okToStart = TestStoryCatalyst(script, *stories, &startBlock, vars);
			else
				okToStart = false;
			g_Au.UnlockStories();

			if(okToStart)
			{
				// Create a story instance.
				unsigned int storyInstance = g_Au.NextStoryId();
				AuStory *story = new AuStory(storyInstance, script.Id(), startBlock, vars);
				story->Join(Id());

				AuStories* stories = g_Au.LockStoriesForWrite();
				if(stories)
					stories->insert(pair<unsigned int, AuStory*>(storyInstance, story));
				g_Au.UnlockStories();

				g_Au.StoryAdded(storyInstance);

				// Start story for the person.
				StartStory(script.Id(), storyInstance);
			}
		}
		break;

	case AuJob::Assassin:
	case AuJob::BountyHunterPerson:
	case AuJob::BountyHunterShip:
	case AuJob::BountyHunterVehicle:
	case AuJob::CommandShipInOrbit:
	case AuJob::CommandShipOnGround:
	case AuJob::DeliverCargoConstruction:
	case AuJob::DeliverCargoDisease:
	case AuJob::DeliverCargoGasping:
	case AuJob::DeliverCargoManufacture:
	case AuJob::DeliverCargoPoison:
	case AuJob::DeliverCargoRats:
	case AuJob::DeliverCargoSpacecraftFactory:
	case AuJob::DeliverCargoSuffocating:
	case AuJob::DeliverFoodHungry:
	case AuJob::DeliverFoodStarving:
	case AuJob::DeliverPassengersColonists:
	case AuJob::DeliverPassengersDeserted:
	case AuJob::DeliverPassengersWorkers:
	case AuJob::DestroyEnemyStation:
	case AuJob::InterdictSystem:
	case AuJob::PickUpPassengersHomeless:
	case AuJob::PickUpPassengersOverpopulated:
	case AuJob::PickUpPassengersStranded:
	case AuJob::TransferPrisoners:
	case AuJob::VisitSystem:
		{
			// Remove previous job(s) that this nu job replaces.
			m_JobsLock.lockForWrite();
			AuJobList::iterator ji = m_Jobs.begin();
			while(ji != m_Jobs.end())
				if(jobOffer.Job()->DoesReplace(*ji))
				{
					delete *ji;
					ji = m_Jobs.erase(ji);
				}
				else
					ji++;

			// Add the nu job to my list of jobs.
			m_Jobs.append(jobOffer.Job()->Clone());
			m_tmJobsChange.Set();
			m_JobsLock.unlock();
			SetModified();
		}
		break;

	case AuJob::JobTypes:
	case AuJob::InvalidJob:
		break;
	}
}

void DaPerson::JobAccepted(const AuJobOffer& jobOffer)
{
	if(!jobOffer.Job())
		return;
	const AuJobStory* storyJob = dynamic_cast<const AuJobStory*>(jobOffer.Job());
	if(storyJob)
	{
		// The offer is a story script job. Conditions are different.
		// Story jobs require access to the story scripts.
		bool storyStarted = false;
		if(CanStartStory())
		{
			// Person has room to start another story.
			// EXCEPTION! AuJobStory stores a script id instead of a story instance when it is used as a job offer.
			const AuScripts* scripts = g_Au.LockScripts();
			AuScripts::const_iterator it = scripts->find(storyJob->StoryInstance());
			if(it != scripts->cend())
			{
				JobAccepted(jobOffer, (*it).second);
				storyStarted = true;
			}
			g_Au.UnlockScripts();
		}

		// Send a result log channel message on failure only.
		if(!storyStarted)
		{
			QString resultMsg;
			resultMsg = tr("Story could not be started: %1").arg(jobOffer.Job()->Title());
			AuCommThought msg(resultMsg, Id(), Name());
			g_Au.SendAtomChat(msg);
		}
	}
	else
	{
		// The job is not a story script job.
		// Insure the offer matches one of the offers that is currently open.
		bool foundJob = false;
		const AuJobOffers* jobOffers = g_Au.LockJobOffersForRead();
		for(AuJobOffers::const_iterator joi = jobOffers->cbegin(); joi != jobOffers->cend(); joi++)
		{
			if(jobOffer == *joi)
			{
				foundJob = true;
				break;
			}
		}
		g_Au.UnlockJobOffers();

		if(foundJob)
			JobAccepted(jobOffer, AuScript(1));

		// Send a result log channel message.
		QString resultMsg;
		if(foundJob)
			resultMsg = tr("Job offer accepted: %1").arg(jobOffer.Job()->Title());
		else
			resultMsg = tr("Job offer is no longer available: %1").arg(jobOffer.Job()->Title());
		AuCommThought msg(resultMsg, Id(), Name());
		g_Au.SendAtomChat(msg);
	}
}

void DaPerson::JobClosed(const AuJob* job)
{
	const AuJobStory* jobStory = dynamic_cast<const AuJobStory*>(job);
	if(jobStory)
	{
		quint32 storyInstance = jobStory->StoryInstance();
		if(storyInstance)
		{
			bool cancelOk;
			AuStory* story = g_Au.LockStory(storyInstance);
			if(story)
				cancelOk = story->CancelOk();	// The story can set a no cancel flag on itself.
			else
				cancelOk = true;				// Cancel storyInstance that has no story; null story is ok.
			if(cancelOk)
				DepartStory(storyInstance, story);
			g_Au.UnlockStory(storyInstance);

			// Tell the person.
			if(!cancelOk)
				Thought(tr("'%1' cannot be dropped at this time.").arg(jobStory->StoryTitle()));
		}
	}
	else
	{
		m_JobsLock.lockForWrite();
		for(AuJobList::iterator ji = m_Jobs.begin(); ji != m_Jobs.end(); ji++)
			if((*ji)->Compare(job))
			{
				// Found the corresponding job. Delete it and remove it from my jobs list.
				delete *ji;
				m_Jobs.erase(ji);
				m_tmJobsChange.Set();
				SetModified();
				break;
			}
		m_JobsLock.unlock();
	}
}

void DaPerson::CargoDelivered(const AuId& cityId, const AuItem& item)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.CargoDelivered(this, cityId, item))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::CryoDelivered(const AuId& cityId, quint16 count)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.CryoDelivered(this, cityId, count))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::CryoPickedUp(const AuId& cityId, const AuId& worldId, quint16 count)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.CryoPickedUp(this, cityId, worldId, count))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::PassengersDelivered(const AuId& cityId, quint16 count)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.PassengersDelivered(this, cityId, count))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::PassengersPickedUp(const AuId& cityId, const AuId& worldId, quint16 count)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.PassengersPickedUp(this, cityId, worldId, count))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::CitizenKilled(DaCitizen* victim)
{
	// This person killed a citizen.
	// Determine result based on political stance of my empire toward the victim's empire.
	POLITICAL_STANCE stance = PoliticalStanceMakeContact(victim);
	switch(stance)
	{
	case POL_Enemy:
		// Killed an enemy unit
		// Increase amount owed by empire based on the type of unit killed.
		if(!victim->IsCivilian())
			Reward(0, REW_KillUnit);
		break;
	case POL_Neutral:
		// Killed a neutral unit.
		break;
	case POL_Friend:
	case POL_Empire:
	case POL_Vassal:
		// Killed friendly unit.
		DecreaseReputation(1);
		break;
	}
}

void DaPerson::PersonKilled(DaPerson* victim)
{
	// Person was killed.
	POLITICAL_STANCE stance = PoliticalStanceMakeContact(victim);
	if(stance == POL_Empire)
	{
		// This person killed another person in the same empire.
		// Murder of fellow empire members reduces reputation.
		DecreaseReputation(1);
	}
	else
	{
		// Empires are different and the victim is a person.
		// Determine result based on political stance of my empire toward the victim's empire.
		switch(stance)
		{
		case POL_Enemy:
			// Killed an enemy person.
			Reward(0, REW_KillUnit);
			break;
		case POL_Neutral:
			// Killed a neutral person.
			break;
		case POL_Friend:
		case POL_Empire:
		case POL_Vassal:
			// Killed friendly person.
			DecreaseReputation(1);
			break;
		}
	}

	// Notify jobs list.
	m_JobsLock.lockForWrite();
	if(m_Jobs.PersonKilled(this, victim))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::PrisonersLoaded(const AuId& cityId, quint16 count)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.PrisonersLoaded(this, cityId, count))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::PrisonersUnloaded(const AuId& cityId, quint16 count)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.PrisonersUnloaded(this, cityId, count))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::SpacecraftCaptured(DaVessel* ship, const AuId& inSystemId)
{
	// Lodge a diplomatic incident with the losing empire.
	const DaSector* sector = FindParentSector();
	const DaSolarSystem* system = FindParentSolarSystem();
	const DaWorld* world = FindParentPlanetInEnv();
	QString sectorName = sector ? sector->Name() : "";
	QString systemName = system ? system->Name() : "";
	QString worldName = world ? world->Name() : "";
	switch(ship->Structure())
	{
	case SC_Count:
	case SC_Deprecated:
		break;
	case SC_Hull:
	case SC_Spaceship:
		{
			AuDocSpaceshipCaptured doc(ship->Name(), Name(), Galaxy(), sectorName, systemName, worldName);
			emit GovDossierAddDocument(ship->EmpireId(), EmpireId(), doc.Pack());
		}
		break;
	case SC_Starship:
		{
			AuDocStarshipCaptured doc(ship->Name(), Name(), Galaxy(), sectorName, systemName, worldName);
			emit GovDossierAddDocument(ship->EmpireId(), EmpireId(), doc.Pack());
		}
		break;
	case SC_Station:
		{
			AuDocSpaceStationCaptured doc(ship->Name(), Name(), Galaxy(), sectorName, systemName, worldName);
			emit GovDossierAddDocument(ship->EmpireId(), EmpireId(), doc.Pack());
		}
		break;
	}

	m_JobsLock.lockForWrite();
	if(m_Jobs.SpacecraftCaptured(this, ship, inSystemId))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::SpacecraftCommanded(const AuId& shipId)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.SpacecraftCommanded(this, shipId))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::SpacecraftDestroyed(DaVessel* ship, const AuId& inSystemId)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.SpacecraftDestroyed(this, ship, inSystemId))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::SystemVisited(const AuId& systemId, DaBd& bd)
{
	if(!bd.EmpireId())
		return;
	m_JobsLock.lockForWrite();
	if(m_Jobs.SystemVisited(this, systemId, bd))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::VehicleCaptured(DaVehicle* vehicle)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.VehicleCaptured(this, vehicle))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

void DaPerson::VehicleDestroyed(DaVehicle* vehicle)
{
	m_JobsLock.lockForWrite();
	if(m_Jobs.VehicleDestroyed(this, vehicle))
	{
		m_tmJobsChange.Set();
		SetModified();
	}
	m_JobsLock.unlock();
}

bool DaPerson::HadItem(COMMODITYID cid) const
{
	return Stats().HadItem(cid);
}

int DaPerson::EVAMinutes() const
{
	return Stats().EVAMinutes();
}

int DaPerson::VehicleMinutes(COMMODITYID vehicleCID) const
{
	//qDebug() << "DaPerson::HasVehicleMinutes" << g_Au.CommodityName(vehicleCID) << "minutes" << minutesMinimum
		//<< "vehicleMinutes" << Stats().VehicleMinutes(vehicleCID);
	return Stats().VehicleMinutes(vehicleCID);
}

void DaPerson::AutoStartStory(const AuScripts& scripts)
{
	//qDebug() << "DaPerson::AutoStartStory scripts" << scripts.count();
	bool okToStart = false;
	unsigned int scriptIdToStart = 0;
	QString startBlock;

	// Check for stories that can be started automatically.
	QString block;
	QMap<QString, QVariant> vars;
	const AuStories* stories = g_Au.LockStoriesForRead();
	if(stories)
	{
		//qDebug() << "DaPerson::AutoStartStory stories" << stories->count();
		for(AuScripts::const_iterator sit = scripts.begin(); sit != scripts.end(); sit++)
		{
			// Only certain stories can be started automatically.
			if(!(*sit).second.AnyAutoStart())
			{
				//qDebug() << "script does not autostart" << Id() << Name() << "scriptId" << (*sit).second.Id();
				continue;											// The script cannot be started automatically.
			}

			// Only one autostart story can be open at a time.
			unsigned int scriptId = (*sit).second.Id();
			if(IsStoryOpen(scriptId, *stories))
			{
				// I already have a story open that was started automatically, or at least could have started automatically.
				// This is why we keep searching after we find a potential story, to see if we are already executing a story that
				// was auto-started. We only auto-start one story at a time.
				//qDebug() << "autostart story is already open" << Id() << Name() << "scriptId" << scriptId;
				okToStart = false;
				break;
			}

			// Only stories that have never been started can start automatically, unless they are auto repeaters.
			if((*sit).second.AutoStart() && HasStartedStory(scriptId))
			{
				//qDebug() << "has already started story" << Id() << Name() << "scriptId" << scriptId;
				continue;											// I started that story once. It cannot autostart again.
			}
			else if((*sit).second.AutoRepeatUntilComplete() && HasFinishedStory(scriptId))
			{
				//qDebug() << "has already finished story" << Id() << Name() << "scriptId" << scriptId;
				continue;											// I finished that story once. It cannot autostart again.
			}

			// Keep the first autostart story we find that is ok to start.
			// We check the entire list to insure we do not have any autostart stories open at all.
			// Due to catalyst conditions, the autostart stories may get started out of script id order.
			// No attempt is made to maintain a numeric sequence there.
			if(okToStart)
				continue;											// We already have a story that is ok to start automatically.

			// The story can be autostarted. Check the catalyst.
			vars.clear();
			okToStart = TestStoryCatalyst((*sit).second, *stories, &block, vars);
			//qDebug() << "autostart story is not okToStart" << Id() << Name() << "scriptId" << scriptId;
			if(okToStart)
			{
				scriptIdToStart = scriptId;
				startBlock = block;
			}
		}
	}
	g_Au.UnlockStories();

	if(okToStart)
	{
		// Create a story instance.
		unsigned int storyInstance = g_Au.NextStoryId();
		//qDebug() << "DaPerson::AutoStartStory" << Id() << Name() << "script" << scriptIdToStart
			//<< "storyInstance" << storyInstance << "startBlock" << startBlock;
		AuStory* story = new AuStory(storyInstance, scriptIdToStart, startBlock, vars);
		story->Join(Id());

		AuStories* stories = g_Au.LockStoriesForWrite();
		if(stories)
			stories->insert(pair<unsigned int, AuStory*>(storyInstance, story));
		g_Au.UnlockStories();

		g_Au.StoryAdded(storyInstance);

		// Start story for the person.
		StartStory(scriptIdToStart, storyInstance);
	}
}

bool DaPerson::AutoStartCustomScript()
{
	//qDebug() << "DaPerson::AutoStartStory scripts" << scripts.count();
	bool okToStart = false;
	unsigned int scriptIdToStart = 0;
	QString startBlock;

	// Check for stories that can be started automatically.
	QString block;
	QMap<QString, QVariant> vars;
	m_ScriptLock.lock();
	if(m_Script)
	{
		//qDebug() << "DaPerson::AutoStartStory stories" << stories->count();
		const AuStories* stories = g_Au.LockStoriesForRead();
		if(stories)
		{
			// Custom scripts are always autostart stories.
			// Only one autostart story can be open at a time.
			unsigned int scriptId = m_Script->Id();
			if(!IsStoryOpen(scriptId, *stories))
			{
				// I do not have the story open.
				// Check the catalyst.
				okToStart = TestStoryCatalyst(*m_Script, *stories, &block, vars);
				if(okToStart)
				{
					scriptIdToStart = scriptId;
					startBlock = block;
				}
				//else
					//qDebug() << "DaPerson::AutoStartCustomScript" << Id() << Name() << "script" << m_Script->Title()
						//<< "cannot autostart yet, waiting...";
			}
		}
		g_Au.UnlockStories();
	}
	m_ScriptLock.unlock();

	if(okToStart)
	{
		// Create a story instance.
		unsigned int storyInstance = g_Au.NextStoryId();
		AuStory* story = new AuStory(storyInstance, scriptIdToStart, startBlock, vars);
		story->Join(Id());

		AuStories* stories = g_Au.LockStoriesForWrite();
		if(stories)
			stories->insert(pair<unsigned int, AuStory*>(storyInstance, story));
		g_Au.UnlockStories();

		g_Au.StoryAdded(storyInstance);

		// Start story for the person.
		StartStory(scriptIdToStart, storyInstance);
	}
	return okToStart;
}

void DaPerson::SpawnRecall(const AuTime& spawnTime)
{
	m_PersonLock.lockForWrite();

	m_uiActions &= ~ACT_Continuous;								// Insure the continuous movement bit is off.
	m_uiActions |= ACT_SpawnRecall;

	m_SpawnTime = spawnTime;

	m_PersonLock.unlock();
}

void DaPerson::KilledByPerson(DaPerson* killer)
{
	// This person was killed by another person.
	killer->PersonKilled(this);
}

void DaPerson::UnitKilled(DaUnit* victim)
{
	// Agent who is animal, entity or invisible does not collect stats, gain xp, or trigger war.
	if(m_AgentBits&(AGENT_Animal|AGENT_Entity|AGENT_Invisible))
		return;

	// Player Statistics
	POLITICAL_STANCE stance = PoliticalStanceMakeContact(victim);
	Stats().KilledUnit(victim, stance, victim->DamagedUsingCID(Id()));
	IncreaseSkill(AuSkill::Combat, victim->ExperiencePointsForKilling());

	// Empire pays rewards and jobs list is notified for killing various types of units.
	victim->KilledByPerson(this);

	// Base class declares mutual enemies appropriately.
	DaSentient::UnitKilled(victim);
}

void DaPerson::SetOnLine(ACCOUNT acctType, unsigned int avgPingMs, const AuTime& when)
{
	// When the person goes from off line to on line, emit the AvatarOnLine signal. That way switching characters will immediately
	// send this notification.
	if(AccountTypeOnLine() == ACCT_Count && acctType != ACCT_Count)
	{
		quint32 empireId = EmpireId();
		if(empireId)
			emit AvatarOnLine(Id(), empireId);
	}

	m_OnLineLock.lockForWrite();
	if(m_AccountOnLine != acctType)
		m_AccountOnLine = acctType;
	m_AccountInactive = false;	// Dormant demiavatar is no longer dormant when the owner is really on line.
	m_Ping = avgPingMs;
	m_tmLastOnLine = when;
	m_OnLineLock.unlock();
}

void DaPerson::HandcraftItem(int mpid)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Must not allow handcraft at a building under construction when the building is required.
	bool inBd = false;			// true if in a bd that is not under construction and not destroyed.
	bool hasPower = false;
	AuKnownPatents patents;
	DaWorldLL* llworld = FindParentWorldInEnv();
	AuId bldCity;
	if(llworld)
	{
		if(!BdId().IsNull())
		{
			DaBd* bd = llworld->FindBd(BdId());
			if(bd && !bd->IsUnderConstruction() && !bd->IsDestroyed())
			{
				inBd = true;
				hasPower = bd->HasPower();
				bd->ListPatents(patents, bd->IndustryId());
			}
		}
	}

	// Add patents in my possession.
	ListPatents(patents);

	// Init the manufacturing process.
	DeManufacturingProcess mp;
	mp.SetHandCraftingProcessId(mpid);

	// Do not handcraft fires when they won't burn.
	COMMODITYID cid = mp.CommodityId();
	if(cid == CID_StoneFirePit || cid == CID_BurningBarrel)
	{
		// Buildings always have air. Spacecraft usually have air.
		if(!IsInCapsule())
		{
			QString response;
			if(IsInVisibleWater())
				response = tr("The fire won't burn in water.");
			else if(!IsAirBreathable() && InAtmosphereType() != ATMTYPE_Exotic)
				response = tr("The fire won't light without air.");
			if(!response.isEmpty())
			{
				// Tell them why they can't make a fire.
				Thought(response);
				return;
			}
		}
	}

	// Determine industry requirement for the process.
	int craft = 0;	// craft value from b_ManufacturingProcess record
	INDUSTRYID idi = IID_None;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator mpi = gd.m_bManufacturingProcessArray.cbegin();
			mpi < gd.m_bManufacturingProcessArray.cend(); mpi++)
	{
		if((*mpi).m_id == mpid)
		{
			craft = (*mpi).m_craft;
			idi = (INDUSTRYID)(*mpi).m_idi;
			break;
		}
	}
	g_Au.UnlockData();
	if(!craft)
	{
		qWarning("warning: Player %d %s: Invalid HandCraft mpid %d, idi %d. Not a handcraftable process.",
				Id().GetID(), (const char*)Name().toLatin1(), mpid, idi);
		return;	// This process should not have been run.
	}

	bool atAppropriateIndustry = inBd && idi == InIndustry();	// regardless if it is a requirement
	if(craft == 2 && !atAppropriateIndustry)
	{
		qWarning("warning: Player %d %s: Invalid HandCraft idi %d, mpid %d. Not standing in correct industry.",
				Id().GetID(), (const char*)Name().toLatin1(), idi, mpid);
		return;	// This process should not have been run.
	}

	// Electricity must be supplied by the building.
	if(hasPower)
		mp.SupplyAllPower(0);

	// Try to satisfy the needs of the process using the person's inventory. We already got the patents.
	AuSpecies species;
	bool dnaSentient = false;
	AuGear* inven = LockGearForWrite();
	for(DeProcessComponentArray::iterator pci = mp.Components().begin(); pci != mp.Components().end(); pci++)
	{
		if(!(*pci).IsRequired())
			continue;	// Handcrafting processes only care about required modifiers.
		if((*pci).IsPower())
			continue;	// Electricity is not taken from the person's inventory.
		if(DbCommodity::IsPatent((*pci).CommodityId()))
		{
			(*pci).SetAppliedMaterial(1, patents.PatentQuality((*pci).CommodityId()));
			continue;
		}

		// Determine how many are needed.
		int needToHave = (*pci).QtyConsumed();
		if(!needToHave)
			needToHave = 1;	// Required component doesn't consume the item so the consumed qty is 0; it is still needed.

		// Determine how many are in the person's inventory.
		int hasItems = 0;
		int hasQuality = 0;
		for(AuGear::const_iterator ii = inven->begin(); ii != inven->end() && hasItems < needToHave; ii++)
		{
			if((*ii).Item().CommodityId() == (*pci).CommodityId())
			{
				hasItems += (*ii).Item().Count();
				if(hasItems > needToHave)
					hasItems = needToHave;
				if(!hasQuality || hasQuality > (*ii).Item().Quality())	// Result is the lowest quality applied.
					hasQuality = (*ii).Item().Quality();

				// Processes that produce items that store DNA will pick up DNA from components that get consumed.
				if(mp.DNAIsOutput() && (*pci).QtyConsumed() && species.IsNull() && !(*ii).Item().DNASpecies().IsNull())
				{
					species = (*ii).Item().DNASpecies();
					dnaSentient = (*ii).Item().DNAIsSentient();
				}
			}

			// Look inside of containers.
			if((*ii).Item().Content())
			{
				const AuItemContent* contents = (*ii).Item().Content();
				for(AuItemContent::const_iterator ci = contents->cbegin(); ci != contents->cend() && hasItems < needToHave; ci++)
				{
					if((*ci).second->CommodityId() == (*pci).CommodityId())
					{
						hasItems += (*ci).second->Count();
						if(hasItems > needToHave)
							hasItems = needToHave;
						if(!hasQuality || hasQuality > (*ci).second->Quality())	// Result is the lowest quality applied.
							hasQuality = (*ci).second->Quality();

						// Processes that produce items that store DNA will pick up DNA from components that get consumed.
						if(mp.DNAIsOutput() && (*pci).QtyConsumed() && species.IsNull() && !(*ci).second->DNASpecies().IsNull())
						{
							species = (*ci).second->DNASpecies();
							dnaSentient = (*ci).second->DNAIsSentient();
						}
					}
				}
			}
		}

		// Set the component as if the materials had been applied.
		if(hasItems)
			(*pci).SetAppliedMaterial(hasItems, hasQuality);
	}

	// Processes that produce items that store DNA have picked up DNA from components that get consumed.
	if(!species.IsNull())
		mp.SetSpecies(species, dnaSentient);

	if(mp.CanRun())
	{
		// All needed components were found in the person's inventory.
		// Consume the components and produce the item. This code does not check for item failure due to quality, it just
		// assumes everything works.
		for(DeProcessComponentArray::iterator pci = mp.Components().begin(); pci != mp.Components().end(); pci++)
		{
			if(!(*pci).IsRequired())
				continue;	// Handcrafting processes only care about required modifiers.
			if((*pci).IsPower())
				continue;	// Electricity is not taken from the person's inventory.

			// Determine how many are needed.
			int needToHave = (*pci).QtyConsumed();
			if(!needToHave)
				continue;		// We're only looking for components that get consumed this time around.

			// Determine how many are in the person's inventory.
			int hasItems = 0;
			AuGear::iterator ii = inven->begin();
			while(ii != inven->end() && hasItems < needToHave)
			{
				if((*ii).Item().CommodityId() == (*pci).CommodityId())
				{
					// Consume the commodity.
					int consume = (*ii).Item().Count();
					if(consume > needToHave - hasItems)
						consume = needToHave - hasItems;
					(*ii).Item().Consume(consume);
					hasItems += consume;
					if((*ii).Item().Count() == 0)
					{
						ii = inven->erase(ii);
						continue;
					}
				}

				// Look inside of containers.
				if((*ii).Item().Content())
				{
					AuItemContent* contents = (*ii).Item().Content();
					AuItemContent::iterator ci = contents->begin();
					while(ci != contents->end() && hasItems < needToHave)
					{
						if((*ci).second->CommodityId() == (*pci).CommodityId())
						{
							// Consume the commodity.
							int consume = (*ci).second->Count();
							if(consume > needToHave - hasItems)
								consume = needToHave - hasItems;
							(*ci).second->Consume(consume);
							hasItems += consume;
							if((*ci).second->Count() == 0)
							{
								// The item was completely consumed.
								// Containers are not supposed to contain invalid items; remove it.
								ci = contents->Erase(ci);
								continue;
							}
						}
						ci++;
					}
				}

				ii++;
			}
		}

		// Create the resulting item.
		mp.RunHandCraft(patents);
		quint8 quality = mp.LastOutputQuality();
		if(cid != CID_Invalid && quality)
		{
			// Player Statistics
			Stats().ItemHandCrafted(cid, quality, atAppropriateIndustry);
			IncreaseSkill(AuSkill::Handcraft, 1);

			QString response;
			if(cid == CID_Money)
			{
				UpdateGearWeight();

				// Calculate the number of coins that are exactly equal to the value of the metals used.
				float coinsMade = 0;
				int gold = mp.Components().QtyApplied(CID_Gold);
				if(gold)
					coinsMade = g_Au.CommodityQualityCost(CID_Gold, mp.LastOutputQuality()) * gold;

				// Make a whole number of coins. Make at least one coin.
				coinsMade = floorf(coinsMade);
				if(coinsMade < 1)
					coinsMade = 1;

				// Money is added directly to the person's account.
				CreditAccount(coinsMade);

				// The person's account always has room for money.
				response = tr("%L1%2 handcrafted bullion added to my personal account.")
					.arg((int)coinsMade)
					.arg(g_Au.CurrencySymbol());
			}
			else if(DbCommodity::IsVehicle(cid))
			{
				if(DaVehicle::Mass(cid) > 0)
				{
					UpdateGearWeight();
					UnlockGear();

					// Spawn a vehicle at the agent's location and rotation.
					// Create the nu vehicle.
					DaVehicle* vehicle = new DaVehicle(cid, quality);
					vehicle->ConnectSignals();
					vehicle->Create();
					vehicle->SetPosAndOrient(Pos());
					ether->AddAtom(vehicle);

					vehicle->ResetHitPoints();
					vehicle->SetEmpireId(EmpireId());
					vehicle->SetInMotion();
					g_Au.AtomAdded(vehicle->Id());
					vehicle->SetNormal();	// New atom is ready for normal thread processing.

					// Place the person aboard the vehicle.
					ForceMountVehicle(vehicle);	// Deadlocks if gear is locked.
					response = tr("Handcrafted %1 %2 Q%3.").arg(vehicle->Name()).arg(DbCommodity::QualityDesc(quality));
					Thought(response);

					vehicle->TickTockRecursive();
					return;
				}
				else
					response = tr("Cannot handcraft unimplemented vehicle %1.").arg(g_Au.CommodityName(cid));
			}
			else if(cid == CID_StoneFirePit || cid == CID_BurningBarrel)
			{
				// Spawn a fire at the person's location and rotation.
				AuItem* item = AuItem::Create(cid, quality, EmpireId());
				item->SetState(1);	// Light the fire.

				// The fire is created on the ground at my feet.
				DropItem(*item);
				UpdateGearWeight();
				response = tr("Handcrafted %1 on the ground.").arg(item->ListDesc());

				delete item;
			}
			else
			{
				AuItem* item = AuItem::Create(cid, quality, EmpireId());
				AuItemDNA* itemDNA = dynamic_cast<AuItemDNA*>(item);
				if(itemDNA)
				{
					// The result of the process can store DNA.
					// Capture DNA from components added to the process.
					AuDNABase* dna = mp.DNASpecies().CreateDNA();
					if(dna)
					{
						itemDNA->SetDNA(dna, mp.DNAIsViable(), mp.DNAIsSentient(), Mature);
						delete dna;
					}
				}

				// Add the item to the scene atom's inventory if they have room.
				if(AddGearItemNoLock(m_Gear, *item))
				{
					// The person had room in their inventory.
					response = tr("Handcrafted %1.").arg(item->ListDesc());
				}
				else
				{
					// The person's inventory had no room. Drop the item as if the person was holding it then dropped it.
					DropItem(*item);
					UpdateGearWeight();
					response = tr("Handcrafted %1 but dropped it because there is no place to carry it.").arg(item->ListDesc());
				}

				delete item;
			}

			// Send a response on the Log channel.
			Thought(response);
		}
	}
	UnlockGear();
}

void DaPerson::SetAgentBits(quint8 bits)
{
	// Found the agent.
	if(m_AgentBits != bits)
	{
		m_AgentBits = bits;
		SetModified();
	}

	// Report the result.
	QString response;
	if(IsOnLineAsMinion())
		response = tr("Minion Settings:");
	else
		response = tr("Agent Settings:");

	QString title;
	if(HasArchitectPower())
		response += tr(" Architect");
	else if(HasProgrammerPower())
		response += tr(" Programmer");
	else if(HasAgentPower())
		response += tr(" Agent");
	else
		response += tr(" Player");

	// Show agent special settings.
	if(bits & AGENT_Invincible)
		response += tr(", Invincible");
	if(bits & AGENT_Invisible)
		response += tr(", Invisible");
	if(bits & AGENT_Animal)
		response += tr(", Animal");
	if(bits & AGENT_Entity)
		response += tr(", Entity");
	if(bits & AGENT_Incognito)
		response += tr(", Incognito");

	Thought(response);
}

// 'makeWhat' is 0=invalid, 1=one item, 2='count' items, 3=city inv, 4=ship cargo, 5=utility pouch, 6=knapsack, 7=backpack,
// 8=grav pack
void DaPerson::AgentMakeCommodity(int makeWhat, COMMODITYID cid, quint8 quality, quint32 count, GALAXY destGalaxy,
		const AuPoint& destGalaxyLoc)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Choose an empire for the new item(s).
	quint32 empireId;
	if(AgentIncognito())
		empireId = 0;
	else
		empireId = EmpireId();	// Not incognito. Use my current empire.

	bool canBeCargo = g_Au.Commodity(cid).CanBeCargo();

	switch(makeWhat)
	{
	case 0:
	default:
		return;

	case 1:		// One item
		if(DbCommodity::IsVehicle(cid))
		{
			if(DaVehicle::Mass(cid) > 0)
			{
				// Spawn a vehicle at the agent's location and rotation.
				// Create the nu vehicle.
				DaVehicle* vehicle = new DaVehicle(cid, quality);
				vehicle->ConnectSignals();
				vehicle->Create();
				vehicle->SetPosAndOrient(Pos());
				ether->AddAtom(vehicle);

				vehicle->ResetHitPoints();
				vehicle->SetEmpireId(EmpireId());
				vehicle->SetInMotion();
				if(vehicle->FuelType() != CID_Invalid)
				{
					// Refuel the vehicle.
					AuItem* fuel = AuItem::Create(vehicle->FuelType(), quality, empireId);
					vehicle->Refuel(*fuel);
					delete fuel;

					// Turn the engine on if we're not on the ground.
					if(!vehicle->IsOnGround())
						vehicle->SwitchEngineOn();
				}
				g_Au.AtomAdded(vehicle->Id());
				vehicle->SetNormal();	// New atom is ready for normal thread processing.

				// Place the agent aboard the vehicle.
				ForceMountVehicle(vehicle);

				vehicle->TickTockRecursive();
			}
		}
		else if(cid == CID_StoneFirePit || cid == CID_BurningBarrel)
		{
			// Spawn an item at the agent's location, as if they dropped it.
			AuItem* item = AuItem::Create(cid, quality, empireId);
			if(IsInCapsule())
				item->SetState(1);	// Light the fire.
			else
			{
				const DaWorld* world = FindParentPlanetInEnv();
				if(world)
				{
					QString response;
					if(IsInVisibleWater())
						response = tr("The fire won't burn in water.");
					else if(!IsAirBreathable() && InAtmosphereType() != ATMTYPE_Exotic)
						response = tr("The fire won't light without air.");
					else
						item->SetState(1);	// Light the fire.
					if(!response.isEmpty())
						Thought(response);
				}
			}

			QString resultMsg = tr("%1 created on the ground.").arg(item->ListDesc());
			Thought(resultMsg);

			// Drop the item at the person's current location.
			DropItem(*item);				// Drop a nu item into the environment.
			UpdateGearWeight();

			delete item;
		}
		else if(canBeCargo)
		{
			// Spawn an item at the agent's location, as if they dropped it.
			AuItem* item = AuItem::Create(cid, quality, empireId);
			if(item->CanBeCarried())
			{
				// Configure destination for preons.
				const DaUniverse* universe = FindParentUniverse();
				if(universe)
				{
					const DaGalaxy* galaxy = universe->FindGalaxy(destGalaxy);
					if(galaxy)
						item->SetGalaxyLoc(galaxy->Id(), galaxy->Name(), destGalaxyLoc);
				}

				QString resultMsg = tr("%1 dropped in environment.").arg(item->ListDesc());
				Thought(resultMsg);

				// Drop the item at the person's current location.
				DaItem* itemDropped = DropItem(*item);				// Drop a nu item into the environment.
				if(itemDropped)
					PickUp(itemDropped);							// Attempt to pick up the nuly dropped item.
			}
			else
			{
				QString resultMsg = tr("%1 cannot be carried in inventory.").arg(item->SayName());
				Thought(resultMsg);
			}
			delete item;
		}
		break;

	case 2:		// 'count' items.
		if(canBeCargo)
		{
			// Spawn up to 10 of the items on the ground.
			AuItem* item = AuItem::Create(cid, quality, empireId);

			// Configure destination for preons.
			const DaUniverse* universe = FindParentUniverse();
			if(universe)
			{
				const DaGalaxy* galaxy = universe->FindGalaxy(destGalaxy);
				if(galaxy)
					item->SetGalaxyLoc(galaxy->Id(), galaxy->Name(), destGalaxyLoc);
			}

			// Drop the item at the person's current location.
			if(count > 10)
				count = 10;
			else if(count < 1)
				count = 1;

			item->SetCount(count);
			QString resultMsg = tr("%1 dropped in environment.").arg(item->ListDesc());
			Thought(resultMsg);

			item->SetCount(1);									// Drop one at a time.
			while(count--)
			{
				DaItem* itemDropped = DropItem(*item);		// Drop a nu item into the environment.
				if(itemDropped)
					PickUp(itemDropped);					// Attempt to pick up the nuly dropped item.
			}

			delete item;
		}
		break;

	case 3:		// City inventory
		if(canBeCargo)
		{
			// When in a city, the items are added to the city's inventory.
			DaWorld* planet = FindParentPlanetInEnv();
			if(planet)
			{
				if(IsAtBd() || IsInBd())
				{
					DaBd* bd = planet->FindBd(BdId());
					if(bd)
					{
						AuItem* item = AuItem::Create(cid, quality, empireId, count);

						// Configure destination for preons.
						const DaUniverse* universe = FindParentUniverse();
						if(universe)
						{
							const DaGalaxy* galaxy = universe->FindGalaxy(destGalaxy);
							if(galaxy)
								item->SetGalaxyLoc(galaxy->Id(), galaxy->Name(), destGalaxyLoc);
						}

						// Add the cargo to the bd's community inventory if it will fit.
						quint32 itemc = item->Count();
						bd->AgentAddCommodity(*item);
						itemc -= item->Count();			// subtract the items that could not be added, leaving the net added
						item->SetCount(itemc);			// Report the number added instead of the number left over.
						QString resultMsg;
						if(itemc)
							resultMsg = tr("%1 was added to %6's community inventory.")
								.arg(item->ListDesc())
								.arg(bd->Name());
						else
							resultMsg = tr("None of %1 was added to %6's community inventory.")
								.arg(item->ListDesc())
								.arg(bd->Name());
						delete item;
						Thought(resultMsg);
					}
				}
			}
		}
		break;

	case 4:		// Ship cargo
		if(canBeCargo)
		{
			// When aboard a ship, the items are added to the ship's hold.
			DaCapsule* ship = FindParentCapsuleInEnv();
			if(ship)
			{
				AuItem* item = AuItem::Create(cid, quality, empireId, count);

				// Configure destination for preons.
				const DaUniverse* universe = FindParentUniverse();
				if(universe)
				{
					const DaGalaxy* galaxy = universe->FindGalaxy(destGalaxy);
					if(galaxy)
						item->SetGalaxyLoc(galaxy->Id(), galaxy->Name(), destGalaxyLoc);
				}

				// Add the cargo to the ship's hold if it will fit.
				QString resultMsg;
				if(ship->CanAddCargo(*item))
				{
					ship->AddCargo(*item);
					if(cid == CID_Hydrogen)
						resultMsg = tr("%1 added to %6's fuel cells.")
							.arg(item->ListDesc())
							.arg(ship->Name());
					else if(cid == CID_Electricity)
						resultMsg = tr("%1 added to %6's capacitor.")
							.arg(item->ListDesc())
							.arg(ship->Name());
					else
						resultMsg = tr("%1 added to %6's hold.")
							.arg(item->ListDesc())
							.arg(ship->Name());
				}
				else
				{
					if(cid == CID_Hydrogen)
						resultMsg = tr("%1 could not be added to %6's fuel cells. No room?")
							.arg(item->ListDesc())
							.arg(ship->Name());
					else if(cid == CID_Electricity)
						resultMsg = tr("%1 could not be added to %6's capacitor. No room?")
							.arg(item->ListDesc())
							.arg(ship->Name());
					else
						resultMsg = tr("%1 could not be added to %6's hold. No room?")
							.arg(item->ListDesc())
							.arg(ship->Name());
				}

				delete item;

				Thought(resultMsg);
			}
		}
		break;

	case 5:		// Utility Pouch
		{
			// Spawn an item at the agent's location, as if they dropped it.
			AuItem* kit = AuItem::Create(CID_UtilityPouch, quality, empireId);

			AuItem* torq = AuItem::Create(CID_TorqueWrench, quality, empireId);
			kit->FitItemIntoContainer(*torq);

			AuItem* pliers = AuItem::Create(CID_Pliers, quality, empireId);
			kit->FitItemIntoContainer(*pliers);

			AuItem* wrench = AuItem::Create(CID_Wrench, quality, empireId);
			kit->FitItemIntoContainer(*wrench);

			AuItem* knife = AuItem::Create(CID_Knife, quality, empireId);
			kit->FitItemIntoContainer(*knife);

			AuItem* hammer = AuItem::Create(CID_Hammer, quality, empireId);
			kit->FitItemIntoContainer(*hammer);

			AuItem* solder = AuItem::Create(CID_SolderIron, quality, empireId);
			kit->FitItemIntoContainer(*solder);

			AuItem* sew = AuItem::Create(CID_SewingNeedle, quality, empireId);
			kit->FitItemIntoContainer(*sew);

			AuItem* screwd = AuItem::Create(CID_ScrewDriver, quality, empireId);
			kit->FitItemIntoContainer(*screwd);

			AuItem* glue = AuItem::Create(CID_GlueGun, quality, empireId);
			kit->FitItemIntoContainer(*glue);

			QString resultMsg = tr("%1 dropped in environment.").arg(kit->ListDesc());
			Thought(resultMsg);

			// Drop the item at the person's current location.
			DaItem* itemDropped = DropItem(*kit);				// Drop a nu kit into the environment.
			if(itemDropped)
				PickUp(itemDropped);							// Attempt to pick up the nuly dropped kit.

			delete glue;
			delete hammer;
			delete kit;
			delete knife;
			delete pliers;
			delete screwd;
			delete sew;
			delete solder;
			delete torq;
			delete wrench;
		}
		break;

	case 6:		// Knapsack
		{
			// Spawn an item at the agent's location, as if they dropped it.
			AuItem* kit = AuItem::Create(CID_Knapsack, quality, empireId);

			AuItem* med = AuItem::Create(CID_MedicalKit, quality, empireId);
			kit->FitItemIntoContainer(*med);

			AuItem* cloth = AuItem::Create(CID_JeansAndShirt, quality, empireId);
			kit->FitItemIntoContainer(*cloth);

			AuItem* knife = AuItem::Create(CID_Knife, quality, empireId);
			kit->FitItemIntoContainer(*knife);

			AuItem* lamp = AuItem::Create(CID_Lantern, quality, empireId);
			kit->FitItemIntoContainer(*lamp);

			AuItem* water = AuItem::Create(CID_Water, quality, empireId);
			kit->FitItemIntoContainer(*water);

			AuItem* antit = AuItem::Create(CID_Antitoxin, quality, empireId);
			kit->FitItemIntoContainer(*antit);

			AuItem* antid = AuItem::Create(CID_Antidote, quality, empireId);
			kit->FitItemIntoContainer(*antid);

			AuItem* milk = AuItem::Create(CID_Milk, quality, empireId);
			kit->FitItemIntoContainer(*milk);

			AuItem* pistol = AuItem::Create(CID_Pistol, quality, empireId);
			AuItem* ammo = AuItem::Create(CID_PistolAmmo, quality, empireId);
			pistol->LoadAmmoIntoWeapon(*ammo);
			kit->FitItemIntoContainer(*pistol);

			for(int a=0; a<2; a++)
				kit->FitItemIntoContainer(*ammo);

			QString resultMsg = tr("%1 dropped in environment.").arg(kit->ListDesc());
			Thought(resultMsg);

			// Drop the item at the person's current location.
			DaItem* itemDropped = DropItem(*kit);				// Drop a nu kit into the environment.
			if(itemDropped)
				PickUp(itemDropped);							// Attempt to pick up the nuly dropped kit.

			delete ammo;
			delete antid;
			delete antit;
			delete cloth;
			delete kit;
			delete knife;
			delete lamp;
			delete med;
			delete milk;
			delete pistol;
			delete water;
		}
		break;

	case 7:		// Backpack
		{
			// Spawn an item at the agent's location, as if they dropped it.
			AuItem* kit = AuItem::Create(CID_Backpack, quality, empireId);

			AuItem* rifle = AuItem::Create(CID_AK47, quality, empireId);
			AuItem* ammo = AuItem::Create(CID_RifleAmmo, quality, empireId);
			rifle->LoadAmmoIntoWeapon(*ammo);
			kit->FitItemIntoContainer(*rifle);

			AuItem* med = AuItem::Create(CID_MedicalKit, quality, empireId);
			kit->FitItemIntoContainer(*med);

			AuItem* cloth = AuItem::Create(CID_JeansAndShirt, quality, empireId);
			kit->FitItemIntoContainer(*cloth);

			AuItem* dyn = AuItem::Create(CID_Dynamite, quality, empireId);
			for(int a=0; a<2; a++)
				kit->FitItemIntoContainer(*dyn);

			AuItem* lamp = AuItem::Create(CID_Lantern, quality, empireId);
			kit->FitItemIntoContainer(*lamp);

			AuItem* water = AuItem::Create(CID_Water, quality, empireId);
			kit->FitItemIntoContainer(*water);

			AuItem* antit = AuItem::Create(CID_Antitoxin, quality, empireId);
			kit->FitItemIntoContainer(*antit);

			AuItem* antid = AuItem::Create(CID_Antidote, quality, empireId);
			kit->FitItemIntoContainer(*antid);

			for(int a=0; a<6; a++)
				kit->FitItemIntoContainer(*ammo);

			AuItem* milk = AuItem::Create(CID_Milk, quality, empireId);
			for(int m=0; m<6; m++)
				kit->FitItemIntoContainer(*milk);

			AuItem* pistol = AuItem::Create(CID_Pistol, quality, empireId);
			AuItem* pammo = AuItem::Create(CID_PistolAmmo, quality, empireId);
			pistol->LoadAmmoIntoWeapon(*pammo);
			kit->FitItemIntoContainer(*pistol);

			for(int a=0; a<6; a++)
				kit->FitItemIntoContainer(*pammo);

			QString resultMsg = tr("%1 dropped in environment.").arg(kit->ListDesc());
			Thought(resultMsg);

			// Drop the item at the person's current location.
			DaItem* itemDropped = DropItem(*kit);				// Drop a nu kit into the environment.
			if(itemDropped)
				PickUp(itemDropped);							// Attempt to pick up the nuly dropped kit.

			delete ammo;
			delete antid;
			delete antit;
			delete cloth;
			delete dyn;
			delete kit;
			delete lamp;
			delete med;
			delete milk;
			delete pammo;
			delete pistol;
			delete rifle;
			delete water;
		}
		break;

	case 8:		// Grav Pack
		{
			// Spawn an item at the agent's location, as if they dropped it.
			AuItem* kit = AuItem::Create(CID_GravPack, quality, empireId);

			AuItem* envs = AuItem::Create(CID_VacSuit, quality, empireId);
			kit->FitItemIntoContainer(*envs);

			AuItem* rifle = AuItem::Create(CID_LaserRifle, quality, empireId);
			AuItem* ammo = AuItem::Create(CID_Gigacell, quality, empireId);
			rifle->LoadAmmoIntoWeapon(*ammo);
			kit->FitItemIntoContainer(*rifle);

			AuItem* med = AuItem::Create(CID_MedicalKit, quality, empireId);
			kit->FitItemIntoContainer(*med);

			AuItem* envh = AuItem::Create(CID_VacSuitHelmet, quality, empireId);
			kit->FitItemIntoContainer(*envh);

			AuItem* cloth = AuItem::Create(CID_JeansAndShirt, quality, empireId);
			kit->FitItemIntoContainer(*cloth);

			AuItem* c4 = AuItem::Create(CID_PlasticExplosive, quality, empireId);
			for(int a=0; a<4; a++)
				kit->FitItemIntoContainer(*c4);

			AuItem* dyn = AuItem::Create(CID_Dynamite, quality, empireId);
			for(int a=0; a<4; a++)
				kit->FitItemIntoContainer(*dyn);

			AuItem* water = AuItem::Create(CID_Water, quality, empireId);
			kit->FitItemIntoContainer(*water);

			AuItem* pistol = AuItem::Create(CID_LaserPistol, quality, empireId);
			pistol->LoadAmmoIntoWeapon(*ammo);
			kit->FitItemIntoContainer(*pistol);

			for(int a=0; a<11; a++)
				kit->FitItemIntoContainer(*ammo);

			AuItem* antit = AuItem::Create(CID_Antitoxin, quality, empireId);
			kit->FitItemIntoContainer(*antit);

			AuItem* antid = AuItem::Create(CID_Antidote, quality, empireId);
			kit->FitItemIntoContainer(*antid);

			AuItem* milk = AuItem::Create(CID_Milk, quality, empireId);
			for(int m=0; m<8; m++)
				kit->FitItemIntoContainer(*milk);

			QString resultMsg = tr("%1 dropped in environment.").arg(kit->ListDesc());
			Thought(resultMsg);

			// Drop the item at the person's current location.
			DaItem* itemDropped = DropItem(*kit);				// Drop a nu kit into the environment.
			if(itemDropped)
				PickUp(itemDropped);							// Attempt to pick up the nuly dropped kit.

			delete ammo;
			delete antid;
			delete antit;
			delete c4;
			delete cloth;
			delete dyn;
			delete envh;
			delete envs;
			delete kit;
			delete med;
			delete milk;
			delete pistol;
			delete rifle;
			delete water;
		}
		break;
	}
}

void DaPerson::SoloMakeCommodity(COMMODITYID cid, quint8 quality, quint16 count)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	if(DbCommodity::IsVehicle(cid))
	{
		if(DaVehicle::Mass(cid) > 0)
		{
			// Spawn a vehicle at the agent's location and rotation.
			// Create the nu vehicle.
			DaVehicle* vehicle = new DaVehicle(cid, quality);
			vehicle->ConnectSignals();
			vehicle->Create();
			vehicle->SetPosAndOrient(Pos());
			ether->AddAtom(vehicle);

			vehicle->ResetHitPoints();
			vehicle->SetEmpireId(EmpireId());
			vehicle->SetInMotion();
			if(vehicle->FuelType() != CID_Invalid)
			{
				// Refuel the vehicle.
				AuItem* fuel = AuItem::Create(vehicle->FuelType(), quality, 0);
				vehicle->Refuel(*fuel);
				delete fuel;

				// Turn the engine on if we're not on the ground.
				if(!vehicle->IsOnGround())
					vehicle->SwitchEngineOn();
			}
			g_Au.AtomAdded(vehicle->Id());
			vehicle->SetNormal();	// New atom is ready for normal thread processing.

			// Place the agent aboard the vehicle.
			ForceMountVehicle(vehicle);

			vehicle->TickTockRecursive();
		}
	}
	else if(count > 1)
	{
		// Spawn a quantity of items.
		AuItem* item = AuItem::Create(cid, quality, 0);

		// Drop the item at the person's current location.
		if(count > 10)
			count = 10;
		else if(count < 1)
			count = 1;

		item->SetCount(count);
		QString resultMsg = tr("%1 dropped in environment.").arg(item->ListDesc());
		Thought(resultMsg);

		item->SetCount(1);									// Drop one at a time.
		while(count--)
		{
			DaItem* itemDropped = DropItem(*item);		// Drop a nu item into the environment.
			if(itemDropped)
				PickUp(itemDropped);					// Attempt to pick up the nuly dropped item.
		}

		delete item;
	}
	else if(cid == CID_StoneFirePit || cid == CID_BurningBarrel)
	{
		// Spawn an item at the agent's location, as if they dropped it.
		AuItem* item = AuItem::Create(cid, quality, 0);
		if(IsInCapsule())
			item->SetState(1);	// Light the fire.
		else
		{
			const DaWorld* world = FindParentPlanetInEnv();
			if(world)
			{
				if(IsAirBreathable() && !IsInVisibleWater())
					item->SetState(1);	// Light the fire.
			}
		}

		QString resultMsg = tr("%1 created on the ground.").arg(item->ListDesc());
		Thought(resultMsg);

		// Drop the item at the person's current location.
		DropItem(*item);				// Drop a nu item into the environment.
		UpdateGearWeight();

		delete item;
	}
	else
	{
		// Spawn an item at the person's location, as if they dropped it.
		AuItem* item = AuItem::Create(cid, quality, 0);
		if(item->CanBeCarried())
		{
			QString resultMsg = tr("%1 dropped in environment.").arg(item->ListDesc());
			Thought(resultMsg);

			// Drop the item at the person's current location.
			DaItem* itemDropped = DropItem(*item);				// Drop a nu item into the environment.
			if(itemDropped)
				PickUp(itemDropped);							// Attempt to pick up the nuly dropped item.
		}
		else
		{
			QString resultMsg = tr("%1 cannot be carried in inventory.").arg(item->SayName());
			Thought(resultMsg);
		}
		delete item;
	}
}

bool DaPerson::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	// I always show up in my own scene.
	if(Is(sceneAtom->Id()))
		return true;

	// People only show up in a scene when they are on line.
	if(IsInvisibleOffLine())
	{
		// If I am carrying a passenger, then I am in the scene if they are.
		bool riderInScene = false;
		DaAtomList subs;
		ListSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			if((*it)->IsInScene(sceneAtom, cameraEnvLoc))
			{
				riderInScene = true;
				break;
			}
		if(riderInScene)
			return true;
		return false;
	}

	// Base class variation measures distance between us.
	return DaSentient::IsInScene(sceneAtom, cameraEnvLoc);
}

QString DaPerson::CommMessageLocation() const
{
	if(InArena())
	{
		if(FindParentArena())
			return FindParentArena()->Name();
		return tr("Arena.");
	}

	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return tr("Lost in limbo.");	// Not likely.
	QString systemName;
	if(system->IsEmptySystem())
		systemName = tr("deep space");
	else
		systemName = tr("the %1 system").arg(system->Name());
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return tr("Solar system lost in limbo.");	// Not likely.
	const DaGalaxy* galaxy = system->FindParentGalaxy();
	if(!galaxy)
		return tr("Sector lost in limbo.");	// Not likely.

	const DaVessel* ship = FindParentVesselInEnv();
	const DaVessel* shipShip = ship ? ship->Parent()->FindParentVesselInEnv() : 0;
	const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
	const DaWorldLL* planet = FindParentWorldInEnv();
	if(planet)
	{
		AuPoint planetLoc;
		GetLocationRelativeToParent(planetLoc, planet);
		Q_ASSERT(!planetLoc.IsNaN());

		QString strLongLat;
		AuGlobeLLF planetLL;
		if(planet->LLOfLoc(planetLL, planetLoc))
			strLongLat = planetLL.ToString();

		QString liquid;
		switch(planet->OrbitZone())
		{
		case OZONE_Inferno:
			liquid = tr("lava");
			break;
		case OZONE_Inner:
		case OZONE_Habitable:
		case OZONE_Outer:
			liquid = tr("water");
			break;
		case OZONE_Frigid:
		case OZONE_DeepSpace:
			liquid = tr("supercooled liquid");
			break;
		}

		if(shipShip)
		{
			if(shipShip->IsOnGround())
				return tr("I am aboard %7, which is aboard %8 on the ground on %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
			else if(shipShip->IsInWater())
				return tr("I am aboard %7, which is aboard %8 in the %1 on %2 at %3 in the %4 %5, %6.")
					.arg(liquid)
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
			else if(shipShip->IsInAtmosphere())
				return tr("I am aboard %7, which is aboard %8 flying over %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
			else if(!shipShip->IsMoving())
				return tr("I am aboard %7, which is aboard %8 orbiting %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
			else
				return tr("I am aboard %7, which is aboard %8 maneuvering near %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
		}
		else if(ship)
		{
			if(ship->IsOnGround())
				return tr("I am aboard %7 on the ground on %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else if(ship->IsInWater())
				return tr("I am aboard %7 in the %1 on %2 at %3 in the %4 %5, %6.")
					.arg(liquid)
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else if(ship->IsInAtmosphere())
				return tr("I am aboard %7 flying over %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else if(!ship->IsMoving())
				return tr("I am aboard %7 orbiting %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else
				return tr("I am aboard %7 maneuvering near %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
		}
		else if(vehicle)
		{
			if(IsInVehicle())
			{
				if(vehicle->IsOnGround())
					return tr("I am in a %6, on the ground on %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else if(vehicle->IsInWater())
					return tr("I am in a %6, in the %1 on %2 at %3 in the %4 %5, %7.")
						.arg(liquid)
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else if(vehicle->IsInAtmosphere())
					return tr("I am in a %6, flying over %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else if(!vehicle->IsMoving())
					return tr("I am in a %6, orbiting %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else
					return tr("I am in a %6, maneuvering near %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
			}
			else
			{
				if(vehicle->IsOnGround())
					return tr("I am on a %6, on the ground on %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else if(vehicle->IsInWater())
					return tr("I am on a %6, in the %1 on %2 at %3 in the %4 %5, %7.")
						.arg(liquid)
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else if(vehicle->IsInAtmosphere())
					return tr("I am on a %6, flying over %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else if(!vehicle->IsMoving())
					return tr("I am on a %6, orbiting %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else
					return tr("I am on a %6, maneuvering near %2 at %3 in the %4 %5, %7.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
			}
		}
		else
		{
			if(IsOnGround())
				return tr("I am on the ground on %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
			else if(IsInWater())
				return tr("I am in the %1 on %2 at %3 in the %4 %5, %6.")
					.arg(liquid)
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
			else if(IsInAtmosphere())
				return tr("I am flying over %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
			else if(!IsMoving())
				return tr("I am orbiting %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
			else
				return tr("I am maneuvering near %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
		}
	}
	else if(FindParentStarInEnv())
	{
		if(shipShip)
		{
			if(!ship->IsMoving())
				return tr("I am aboard %7, which is aboard %8 holding position in %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
			else
				return tr("I am aboard %7, which is aboard %8 traveling through %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
		}
		else if(ship)
		{
			if(!ship->IsMoving())
				return tr("I am aboard %7 holding position in %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else
				return tr("I am aboard %7 traveling through %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
		}
		else if(vehicle)
		{
			if(IsInVehicle())
			{
				if(!vehicle->IsMoving())
					return tr("I am in a %5, holding position in %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else
					return tr("I am in a %5, traveling through %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
			}
			else
			{
				if(!vehicle->IsMoving())
					return tr("I am on a %5, holding position in %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else
					return tr("I am on a %5, traveling through %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
			}
		}
		else
		{
			if(!IsMoving())
				return tr("I am holding position in %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
			else
				return tr("I am traveling through %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
		}
	}
	else
	{
		if(!system->IsEmptySystem())
			systemName = tr("deep space in the %1 system").arg(system->Name());
		if(shipShip)
		{
			if(!ship->IsMoving())
				return tr("I am aboard %7, which is aboard %8 holding position in %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
			else
				return tr("I am aboard %7, which is aboard %8 traveling through %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire())
					.arg(shipShip->NameWithEmpire());
		}
		else if(ship)
		{
			if(!ship->IsMoving())
				return tr("I am aboard %7 holding position in %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else
				return tr("I am aboard %7 traveling through %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
		}
		else if(vehicle)
		{
			if(IsInVehicle())
			{
				if(!vehicle->IsMoving())
					return tr("I am in a %5, holding position in %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else
					return tr("I am in a %5, traveling through %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
			}
			else
			{
				if(!vehicle->IsMoving())
					return tr("I am on a %5, holding position in %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
				else
					return tr("I am on a %5, traveling through %2 in the %3 %4, %6.")
						.arg(systemName)
						.arg(sector->Name()).arg(sector->Description())
						.arg(vehicle->Name())
						.arg(galaxy->Name());
			}
		}
		else
		{
			if(!IsMoving())
				return tr("I am holding position in %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
			else
				return tr("I am traveling through %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name());
		}
	}
}

AuPose DaPerson::SelectPoseForMeOrMount(const AuPose& pose, const DaCreature* poseWho) const
{
	//qDebug() << "DaPerson::SelectPoseForMeOrMount" << Id() << Name() << "poseWho" << poseWho->Id() << poseWho->Name()
		//<< poseWho->CurrentGeneralPose()
		//<< "tryPose" << nuPose << "moving" << IsMoving() << "submerged" << poseWho->IsCGSubmerged();
	AuPose nuPose(pose);
	unsigned int acti = Actions();
	if(acti & ACT_MoveMask)
	{
		// Movement controls are active.
		if(poseWho->IsLadderPose())
			nuPose = AuPose::LadderClimb;
		else if(poseWho->IsCGSubmerged() && poseWho->InDeepEnoughLiquidToSwim())
		{
			// Under water legs swim when moving.
			//qDebug("Swim");
			if((acti & ACT_Run) && poseWho->CurrentStamina())
				nuPose = AuPose::SwimFast;
			else
				nuPose = AuPose::Swim;
		}
		else if(poseWho->CanFly() && !poseWho->IsInGravityWell() &&
				((acti & ACT_MoveUp) || (poseWho->IsFlying() && !poseWho->IsOnGround())))
		{
			// Fly.
			//qDebug("Fly");
			// Creature can fly and atmosphere density permits flight.
			// Creature is not in a gravity well or on a ladder.
			// MoveUp command is active or body is already flying and not on the ground.
			if(nuPose != AuPose::Prone)		// Prone pose results from X key, when I'm trying to stop flying.
			{
				if((acti & ACT_Run) && poseWho->CurrentStamina())
					nuPose = AuPose::FlyFast;	// Moving fast.
				else
					nuPose = AuPose::Fly;		// Moving normal speed.
			}
		}
		else
		{
			// Not under water or flying.
			if(poseWho->IsFlying())
				nuPose = AuPose::StandWalk;		// Moving while not supposed to be flying.
			else if(poseWho->IsSwimming() && poseWho->IsOnGround())
				nuPose = AuPose::ProneCreep;		// Swimming on ground while not in water.

			if((acti & ACT_Run) && poseWho->CurrentStamina())
			{
				// I am moving fast and I have stamina. I'm not under water or flying
				//qDebug("Run");
				if(nuPose == AuPose::Crouch)
					nuPose = AuPose::CrouchCreep;
				else if(nuPose == AuPose::Prone)
					nuPose = AuPose::ProneCreep;
				else if(nuPose == AuPose::SitOnGround)
					nuPose = AuPose::SitOnGroundCreep;
				else if(nuPose == AuPose::Swim ||
						nuPose == AuPose::SwimFast ||
						nuPose == AuPose::TreadWater)
					nuPose = AuPose::SwimFast;
				else if(nuPose == AuPose::Stand ||
						nuPose == AuPose::StandWalk)
					nuPose = AuPose::StandRun;
			}
			else
			{
				// I am moving normal speed. I'm not under water or flying
				//qDebug("Move");
				if(nuPose == AuPose::Crouch)
					nuPose = AuPose::CrouchCreep;
				else if(nuPose == AuPose::Prone)
					nuPose = AuPose::ProneCreep;
				else if(nuPose == AuPose::SitOnGround)
					nuPose = AuPose::SitOnGroundCreep;
				else if(nuPose == AuPose::Swim ||
						nuPose == AuPose::SwimFast ||
						nuPose == AuPose::TreadWater)
					nuPose = AuPose::Swim;
				else if(nuPose == AuPose::Stand ||
						nuPose == AuPose::StandRun)
					nuPose = AuPose::StandWalk;
			}
		}
	}
	else if(!poseWho->IsLockedPose())
	{
		// No movement controls are active.
		if(poseWho->IsLadderPose())
			nuPose = AuPose::Ladder;
		else if(poseWho->IsCGSubmerged() && poseWho->InDeepEnoughLiquidToSwim())
			nuPose = AuPose::TreadWater;	// Under water legs tread water while not moving.
		else if(poseWho->CanHover() && !poseWho->IsInGravityWell() && poseWho->IsFlying() && !poseWho->IsOnGround())
		{
			// Creature can hover and atmosphere density permits flight.
			// Creature is not in a gravity well or on a ladder.
			// Body is already flying and not on the ground.
			if(nuPose != AuPose::Prone)		// Prone pose results from X key, when I'm trying to stop flying.
				nuPose = AuPose::Hover;
		}
		else if(poseWho->CanFly() && !poseWho->IsInGravityWell() && poseWho->IsFlying() && !poseWho->IsOnGround())
		{
			// Creature can fly and atmosphere density permits flight.
			// Creature is not in a gravity well or on a ladder.
			// Body is already flying and not on the ground.
			if(nuPose != AuPose::Prone)		// Prone pose results from X key, when I'm trying to stop flying.
			{
				if(acti & ACT_Brake)
					nuPose = AuPose::Hover;		// Flap wings when braking.
				else
					nuPose = AuPose::Glide;
			}
		}
		else
		{
			// Not under water or flying.
			if(poseWho->IsFlying())
				nuPose = AuPose::Stand;		// Not supposed to be flying.
			else if(poseWho->IsSwimming() && poseWho->IsOnGround())
				nuPose = AuPose::Prone;		// Swimming on ground while not in water.

			if(nuPose == AuPose::CrouchCreep)
				nuPose = AuPose::Crouch;
			else if(nuPose == AuPose::ProneCreep)
				nuPose = AuPose::Prone;
			else if(nuPose == AuPose::SitOnGroundCreep)
				nuPose = AuPose::SitOnGround;
			else if(nuPose == AuPose::Swim ||
					nuPose == AuPose::SwimFast)
				nuPose = AuPose::Prone;
			else if(nuPose == AuPose::StandRun ||
					nuPose == AuPose::StandWalk ||
					nuPose == AuPose::TreadWater)
				nuPose = AuPose::Stand;
		}
	}
	//qDebug() << "DaPerson::SelectPoseForMeOrMount" << Id() << Name() << "poseWho" << poseWho->Id() << poseWho->Name()
		//<< "nuPose" << nuPose;
	return nuPose;
}

void DaPerson::RenderItemInHand(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, int pass,
		const DaAtom* sceneAtom, AuRenderLastMap* renderLast)
{
	// Invisible or entity agent does not render gun.
	// Dying highborn renders its gun because the body remains visible.
	if(!IsEthereal())
		DaSentient::RenderItemInHand(projection, model, ari, pass, sceneAtom, renderLast);
}

void DaPerson::DesignEnvironmentExit()
{
	// Find the parent design environment; return quietly if there isn't one.
	DaAtom* pde = FindParentDesignBaseInEnv();
	if(!pde)
		return;

	DismountAllRiders();										// No passengers!

	// Set my transform to that of the design environment. It was set to the person's current transform when the design
	// environment was created. That person was likely me.
	ForcePosMove(AuAtomPos(pde->ParentId(), pde->Transform()), AuPoint(0,0));
	Stand();
	SyncController(AuSyncPose(AuPose::Stand, CurrentHeadTilt(), 0, 0));
	SetModified();
}

void DaPerson::ArenaExit()
{
	// Find the parent arena; return quietly if there isn't one.
	DaAtom* pde = FindParentArena();
	if(!pde)
		return;

	// Put this atom subordinate to the parent of the arena.
	ForcePosMove(AuAtomPos(pde->ParentId(), pde->Transform()), AuPoint(0,0));
	
	// Restore my health.
	ResetHitPoints();				// Person "gets well" before exiting arena.

	// Clear up conditions inflicted while in the arena.
	AuTime entered = WhenArenaEntered();
	AuConditionList& conditions = LockConditionsForWrite();
	AuConditionList::iterator ci = conditions.begin();
	while(ci != conditions.end())
	{
		if((*ci)->IsRemovedOnArenaExit(entered))
		{
			// The condition was created after entry into the arena.
			delete *ci;
			ci = conditions.erase(ci);
		}
		else
			ci++;
	}
	UnlockConditions();

	// I must respawn to my exit point. It will notice I am exiting an arena and fix me up.
	Respawn(false);
}

void DaPerson::ArenaRecall()
{
	// Find the parent arena; return quietly if there isn't one.
	DaArena* arena = FindParentArena();
	if(!arena)
		return;

	if(IsLockedPose())
		ExitConsole();
	else
	{
		DismountAllRiders();									// No passengers!
		Stand();												// Stand in place. Exit console does this.
		SetVector(AuPoint(0,0));								// Stop moving!
		SetModified();
	}

	AuMatrix4x4 t;
	if(ArenaTeam() == DgEmpire::ArenaTeam1Id())
		t.Translate((c_ArenaRingRadius+c_ArenaBoundsRadius)/2, 0, 0);
	else if(ArenaTeam() == DgEmpire::ArenaTeam2Id())
		t.Translate(-(c_ArenaRingRadius+c_ArenaBoundsRadius)/2, 0, 0);
	else
		t.Translate(0, 0, (c_ArenaRingRadius+c_ArenaBoundsRadius)/2);

	ForcePosStop(AuAtomPos(arena->Id(), t));
}

void DaPerson::ArenaStatus()
{
	// Find the parent arena; return quietly if there isn't one.
	DaArena* arena = FindParentArena();
	if(!arena)
		return;

	DaAtomList spacecraft;
	arena->FindVesselsRecursive(spacecraft);

	DaAtomList people;
	arena->FindPeopleOnLineRecursive(people);

	QString text = tr("%1, %2")
		.arg(arena->Name())
		.arg(arena->MatchStateDesc());
	Thought(text);

	QString team1Name, team2Name;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(DgEmpire::ArenaTeam1Id(), team1Name);
	gd.EmpireName(DgEmpire::ArenaTeam2Id(), team2Name);
	g_Au.UnlockData();

	text = tr("%1 spacecraft present. ")
		.arg(spacecraft.count());

	if(arena->Team1Ships())
		text += tr("%2 has %3 of %4 spacecraft remaining to launch. ")
			.arg(team1Name)
			.arg(arena->Team1Ships())
			.arg(arena->TeamShips());
	else
		text += tr("%2 has launched all spacecraft. ")
			.arg(team1Name);

	if(arena->Team2Ships())
		text += tr("%2 has %3 of %4 spacecraft remaining to launch. ")
			.arg(team2Name)
			.arg(arena->Team2Ships())
			.arg(arena->TeamShips());
	else
		text += tr("%2 has launched all spacecraft. ")
			.arg(team2Name);
	Thought(text);

	for(DaAtomList::const_iterator si = spacecraft.begin(); si != spacecraft.end(); si++)
	{
		const DaVessel* ship = (const DaVessel*)*si;
		if(!arena->HasHailed((*si)->Id()))
			text = tr("%1, not ready.").arg(ship->NameWithEmpire());
		else
			text = tr("%1, ready.").arg(ship->NameWithEmpire());
		Thought(text);
	}

	text = tr("%1 avatars present on line.").arg(people.count());
	Thought(text);

	for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
	{
		const DaPerson* person = (const DaPerson*)*pi;
		const DaVessel* ship = person->FindParentVesselInEnv();
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person->Parent());
		QString empireName;
		if(person->ArenaTeam() == DgEmpire::ArenaTeam1Id())
			empireName = team1Name;
		else if(person->ArenaTeam() == DgEmpire::ArenaTeam2Id())
			empireName = team2Name;
		else
		{
			const DgData& gd = g_Au.LockDataForRead();
			gd.EmpireName(person->ArenaTeam(), empireName);
			g_Au.UnlockData();
		}

		if(ship && vehicle)
			text = tr("%1, %2, in %3, aboard %4.")
				.arg(person->Name()).arg(empireName).arg(vehicle->Name()).arg(ship->NameWithEmpire());
		else if(ship)
			text = tr("%1, %2, aboard %3.")
				.arg(person->Name()).arg(empireName).arg(ship->NameWithEmpire());
		else if(vehicle)
			text = tr("%1, %2, in %3.")
				.arg(person->Name()).arg(empireName).arg(vehicle->Name());
		else if(person->InArenaOutsideBounds())
			text = tr("%1, %2, EVA outside the arena.")
				.arg(person->Name()).arg(empireName);
		else if(person->InArenaOutsideRing())
			text = tr("%1, %2, EVA on the sidelines.")
				.arg(person->Name()).arg(empireName);
		else
			text = tr("%1, %2, EVA.")
				.arg(person->Name()).arg(empireName);
		Thought(text);
	}
}

DaUnit* DaPerson::ParentControlAtom() const
{
	if(!IsOnLine())
		return 0;

	// Check for pilot console; it's an easy test.
	if(IsHelmStationPose())
	{
		DaVessel* ship = dynamic_cast<DaVessel*>(Parent());
		if(ship)
			return ship;
		else
			return 0;					// Cannot be in control of anything else using pilot console.
	}

	// Animal parent is in my control until we permit passengers on animals. DaCreature is not tested because a citizen must be
	// issued orders if it is carrying me and a person has their own free will if they are carrying me. Only animals are directly
	// controlled by the rider.
	DaAnimal* mount = dynamic_cast<DaAnimal*>(Parent());
	if(mount)
		return mount;

	// Vehicle requires a test to see if I am in a control seat so it is checked last.
	DaVehicle* vehicle = dynamic_cast<DaVehicle*>(Parent());
	if(vehicle)
	{
		if(vehicle->IsControlSeat(VehicleSeatIndex()))
			return vehicle;
		else
			return 0;					// Cannot be in control of anything else aboard a vehicle.
	}

	return 0;
}

// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'targetLoc' is an output parameter relative to my parent world.
bool DaPerson::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	if(IsEthereal())
		return false;
	return DaSentient::IsGuardTowerTarget(gunnerEmpire, gunnerPosture, gunLoc, gunRange, targetLoc);
}

// Return true if this atom can be targetted by a turret on a spacecraft.
// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
bool DaPerson::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	if(IsEthereal())
		return false;
	return DaSentient::IsShipTurretTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange,
			gunSysVec, targetSysLoc);
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaPerson::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	if(IsEthereal())
		return;
	DaSentient::FindClosestShipTurretTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc,
			gunRange, gunSysVec, targetAtom, targetRange, targetSysLoc);
}

// Used when conducting barter transactions. Adds cItem.Count() items to my inventory. If they don't fit, they fall at my feet.
void DaPerson::AddGearItems(const AuItem& cItem)
{
	AuItem* oneItem = cItem.Clone();
	if(!oneItem)
		return;
	oneItem->SetCount(1);
	for(unsigned int it=0; it<cItem.Count(); it++)
		if(!AddGearItem(*oneItem))
			DropItem(*oneItem);				// Drop a nu item into the environment.
	delete oneItem;
}

// Checks to see if I have cItem.Count() of the items.
bool DaPerson::HasGearItems(const AuItem& cItem) const
{
	m_GearLock.lockForRead();
	bool v = m_Gear.CountAvailableItems(cItem) >= cItem.Count();
	m_GearLock.unlock();
	return v;
}

void DaPerson::Forage(const DaWorldLL* world, COMMODITYID cid, bool isTree, int what)
{
	// Forage a commodity from the environment.
	quint8 quality;
	if(cid == CID_Seed)
		quality = world->NaturalResourceQuality(CID_Vegetation, world->ResourceZone(GlobeLL().Longitude()));
	else
		quality = world->NaturalResourceQuality(cid, world->ResourceZone(GlobeLL().Longitude()));
	if(!quality)
		return;	// Hacker sending a bogus commodity id?

	AuFloraDNA dna;
	quint8 plantType = 0;
	if(isTree)
	{
		// Forage a tree resource.
		dna = world->TreeDNA(GlobeLL(), cid);
		plantType = DeFlora::Tree;
	}
	else if(DbCommodity::IsShrubResource(cid))
	{
		// Forage a shrub resource.
		dna = world->ShrubDNA(GlobeLL(), cid);
		plantType = DeFlora::Shrub;
	}
	else if(cid == CID_Seed)
	{
		// Forage a grass seed. We don't get resources from grass.
		dna = world->GroundCoverDNA(GlobeLL());
		plantType = DeFlora::GroundCover;
	}

	Forage(quality, cid, dna, plantType, world->GenSeed(), world->ResourceZone(GlobeLL().Longitude()),
			world->LatitudeRegion(GlobeLL().Latitude()), what);
}

void DaPerson::Forage(quint8 quality, COMMODITYID cid, const AuFloraDNA& dna, quint8 plantType, quint32 worldGenSeed,
		int worldResourceZone, AuFloraDNA::ZONE latRegion, int what)
{
	if(InDesignEnvironment())
		return;
	// We only get a seed when foraging from ground cover.
	QString text;
	if(cid != CID_Seed && (what == 0 || what == 1))
	{
		AuItem* item = AuItem::Create(cid, quality, 0);
		if(!item)
			return;

		// Add the item to the scene atom's inventory if they have room.
		if(AddGearItem(*item))
		{
			// Player Statistics
			Stats().ItemForaged(*item);
			IncreaseSkill(AuSkill::Forage, 1);

			// The item was added to the person's inventory.
			text = tr("Foraged %1.").arg(item->SayName());
		}
		else if(DropItem(*item))
		{
			// Player Statistics
			Stats().ItemForaged(*item);
			IncreaseSkill(AuSkill::Forage, 1);

			// The item did not fit into the person's inventory and was dropped in the environment instead.
			text = tr("Foraged %1 and dropped it. No room to carry %1.").arg(item->SayName());
		}
		else
		{
			// The item could not be dropped in the environment.
			text = tr("%1 could not be foraged. Something is wrong with the item.").arg(item->SayName());
			delete item;
			Thought(text);
			return;
		}
		
		delete item;
	}

	// Forage a seed with plant commodities.
	if(dna != AuFloraDNA() && (what == 0 || what == 2))
	{
		// Forage a seed too.
		AuItem* seed = AuItem::Create(CID_Seed, quality, 0);
		if(seed)
		{
			AuItemFloraDNA* seedItem = dynamic_cast<AuItemFloraDNA*>(seed);
			if(seedItem)
			{
				seedItem->SetDNA(dna, true, worldGenSeed, worldResourceZone, plantType, cid, latRegion);

				// Add the item to the scene atom's inventory if they have room.
				if(AddGearItem(*seedItem))
				{
					// Player Statistics
					Stats().ItemForaged(*seedItem);
					IncreaseSkill(AuSkill::Forage, 1);

					// The item was added to the person's inventory.
					if(text.isEmpty())
						text = tr("Foraged %1.").arg(seedItem->SayName().toLower());
					else
						text += tr(" Got a %1 too.").arg(seedItem->SayName().toLower());
				}
				else if(cid == CID_Seed && DropItem(*seedItem))
				{
					// When trying to get a seed from grass, drop the seed on the ground. Other seeds are consequential to
					// foraging the plant's commodity.
					// Player Statistics
					Stats().ItemForaged(*seedItem);
					IncreaseSkill(AuSkill::Forage, 1);

					// The seed did not fit into the person's inventory and was dropped in the environment instead.
					text = tr("Foraged %1 and dropped it. No room to carry %1.").arg(seedItem->SayName());
				}
			}

			delete seed;
		}
	}

	if(!text.isEmpty())
		Thought(text);
}

// 'useIt' is 0=no, 1=yes, 2=yes if it's a weapon
bool DaPerson::AddGearItemUseDrop(const AuItem& cItem, int useIt, bool dropIt)
{
	// Select an appropriate location to carry the item.
	AuGearSlot::SLOTTYPE slotType = AuGearSlot::Invalid;
	if(useIt)
	{
		// Place weapon in my dominant hand and wear clothing/armor appropriately.
		if(cItem.Content())
		{
			slotType = AuGearSlot::Body;
		}
		else if(cItem.IsWornOnHead())
		{
			slotType = AuGearSlot::Head;
		}
		else if(cItem.IsWornOnFace())
		{
			slotType = AuGearSlot::Face;
		}
		else if(cItem.IsWornOnNeck())
		{
			slotType = AuGearSlot::Neck;
		}
		else if(cItem.IsWornOnBody())
		{
			slotType = AuGearSlot::BodyWear;
		}
		else if(cItem.IsWornOnLimb())
		{
			slotType = AuGearSlot::Limb;
		}
		else if(cItem.IsWornOnDigit())
		{
			slotType = AuGearSlot::Digit;
		}
		else if(useIt == 1 || cItem.IsWeapon())
		{
			slotType = AuGearSlot::Hand;
		}
	}

	m_GearLock.lockForWrite();
	bool success = AddGearItemAt(m_Gear, slotType, cItem, dropIt);
	m_GearLock.unlock();
	return success;
}

void DaPerson::TakeOrder(const AuCommPacket& msg, CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc,
		bool firstInGroup, const QString& orderText)
{
	if(!IsAIControlled())
		return;

	if(AssignedShip().IsNull())
		return;

	QString addr;
	m_PersonLock.lockForRead();
	if(msg.FromId() == m_Dad)
		addr = tr("dad");
	else if(msg.FromId() == m_Mom)
		addr = tr("mom");
	else
		addr = tr("sir");
	m_PersonLock.unlock();

	QString myresponse;
	QString channelName = msg.ChannelName();
	AuCommStdChannel::SCOPE scope = msg.Scope();
	if(order == ORDER_Garrison)
	{
		// Insure I can take a berth aboard my current ship.
		if(IsInsideSpacecraft())
		{
			QString errResponse;
			const DaVessel* ship = FindParentVesselInEnv();
			if(!ship)
				errResponse = tr("Sorry %1, I can't do that right now.")
					.arg(addr);
			else if(PoliticalStance(ship->EmpireId(), EmpireId()) == POL_Enemy ||
					PoliticalStance(EmpireId(), ship->EmpireId()) == POL_Enemy)
				errResponse = tr("Sorry %1, I can't take a berth on an enemy spacecraft.")
					.arg(addr);
			else if(ship->CaptainsBerthOccupied() &&
					!ship->OfficerBerthsAvailable() &&
					!ship->CrewBerthsAvailable() &&
					!ship->TroopBerthsAvailable() &&
					!ship->PassengerBerthsAvailable())
				errResponse = tr("Sorry %1, this spacecraft doesn't have an empty bunk for me. I'm not picky; I'll take any kind.")
					.arg(addr);
			if(!errResponse.isEmpty())
			{
				AuCommChat myreply(errResponse, msg.ChannelName(), Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(),
						msg.ReceptionRange(), msg.Scope());
				Say(myreply);
				return;
			}
		}
	}

	// Quietly accept the order.
	if(order == ORDER_TakeCommand || order == ORDER_ReleaseCommand)
	{
		// Taking and releasing the con is done as an amendment so it does not change what I am already doing.
		AmendOrder(order, orderTarget, orderLoc, 0);
		SetModified();
	}
	else
		DaSentient::TakeOrder(msg, order, orderTarget, orderLoc, firstInGroup, orderText);
	if(!firstInGroup)
		return;	// Only the first in the group responds to the order.

	// Send a response to the person who gave me the order.
	if(msg.RecipientCount() > 1)
	{
		QString responses[] =
		{
			"Aye %1. We'll stay here.",	// ORDER_Stay
			"Aye %1. We'll attack that target.",	// ORDER_AttackTarget
			"Aye %1. We'll move to your location.",	// ORDER_ComeHere
			"Aye %1. We'll move to your exact location.",	// ORDER_ComeHereExactly
			"Aye %1. We'll follow you.",	// ORDER_FollowMe
			"Aye %1. We'll follow that target.",	// ORDER_FollowTarget
			"Aye %1. We'll get in with you.",	// ORDER_GetIn
			"Aye %1. We'll get out.",	// ORDER_GetOut
			"Aye %1. We'll move to that location.",	// ORDER_GoToTarget
			"Aye %1. We'll move exactly to that location.",	// ORDER_GoToTargetExactly
			"Aye %1. We'll fortify our positions.",	// ORDER_Garrison
			"Aye %1. We'll close the doors.",	// ORDER_CloseDoor
			"Aye %1. We'll open the doors.",	// ORDER_OpenDoor
			"Aye %1. We'll get that.",	// ORDER_GetItem
			"Aye %1. We'll man stations.",	// ORDER_BattleStation
			"Aye %1. We'll sleep here.",	// ORDER_Sleep
			"Aye %1. We'll daydream.",	// ORDER_SleepDayDream
			"Aye %1. We'll eat.",	// ORDER_SleepEat
			"Aye %1. We'll exercise.",	// ORDER_SleepExercise
			QString("Aye %1. We'll play %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
			"Aye %1. We'll write a letter.",	// ORDER_SleepLetter
			"Aye %1. We'll watch a movie.",	// ORDER_SleepMovie
			"Aye %1. We'll listen to music.",	// ORDER_SleepMusic
			"Aye %1. We'll pine for our loves.",	// ORDER_SleepPine
			"Aye %1. We'll talk on the phone.",	// ORDER_SleepPhone
			"Aye %1. We'll read.",	// ORDER_SleepRead
			"Aye %1. We'll talk.",	// ORDER_SleepTalk
			"Aye %1. We'll text.",	// ORDER_SleepText
			"Aye %1. We'll watch TV.",	// ORDER_SleepTV
			"Aye %1. We'll surf the web.",	// ORDER_SleepWeb
			"Aye %1. We'll send a damage report.",	// ORDER_DamageReport
			"Aye %1. We'll conduct a planet survey scan.",	// ORDER_SurveyScanPlanet
			"Aye %1. We'll conduct a system survey scan.",	// ORDER_SurveyScanSystem
			"Aye %1. We'll conduct a tactical scan.",	// ORDER_TacticalScan
			"Aye %1. We'll report to the doc for a checkup.",	// ORDER_EnterSurgery
			"Aye %1. We have the con.",	// ORDER_TakeCommand
			"Aye %1. You have the con.",	// ORDER_ReleaseCommand
			"Aye %1. We'll drop our riders.",				// ORDER_DropRider
			"Aye %1. We'll mount you.",					// ORDER_MountMe
			"Aye %1. We'll mount that target.",			// ORDER_MountTarget
			"Aye %1. We'll dance a jig.",			// ORDER_HengeCeremony
			"Aye %1. We'll move to %2.",	// ORDER_GoToRoom
		};
		myresponse = responses[Order()].arg(addr);
	}
	else
	{
		QString responses[] =
		{
			"Aye %1. I'll stay here.",	// ORDER_Stay
			"Aye %1. I'll attack that target.",	// ORDER_AttackTarget
			"Aye %1. I'll move to your location.",	// ORDER_ComeHere
			"Aye %1. I'll move to your exact location.",	// ORDER_ComeHereExactly
			"Aye %1. I'll follow you.",	// ORDER_FollowMe
			"Aye %1. I'll follow that target.",	// ORDER_FollowTarget
			"Aye %1. I'll get in with you.",	// ORDER_GetIn
			"Aye %1. I'll get out.",	// ORDER_GetOut
			"Aye %1. I'll move to that location.",	// ORDER_GoToTarget
			"Aye %1. I'll move exactly to that location.",	// ORDER_GoToTargetExactly
			"Aye %1. I'll fortify my position.",	// ORDER_Garrison
			"Aye %1. I'll close the door.",	// ORDER_CloseDoor
			"Aye %1. I'll open the door.",	// ORDER_OpenDoor
			"Aye %1. I'll get that.",	// ORDER_GetItem
			"Aye %1. I'll man a station.",	// ORDER_BattleStation
			"Aye %1. I'll sleep here.",	// ORDER_Sleep
			"Aye %1. I'll daydream.",	// ORDER_SleepDayDream
			"Aye %1. I'll eat.",	// ORDER_SleepEat
			"Aye %1. I'll exercise.",	// ORDER_SleepExercise
			QString("Aye %1. I'll play %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
			"Aye %1. I'll write a letter.",	// ORDER_SleepLetter
			"Aye %1. I'll watch a movie.",	// ORDER_SleepMovie
			"Aye %1. I'll listen to music.",	// ORDER_SleepMusic
			"Aye %1. I'll pine for my love.",	// ORDER_SleepPine
			"Aye %1. I'll talk on the phone.",	// ORDER_SleepPhone
			"Aye %1. I'll read.",	// ORDER_SleepRead
			"Aye %1. I'll talk.",	// ORDER_SleepTalk
			"Aye %1. I'll text.",	// ORDER_SleepText
			"Aye %1. I'll watch TV.",	// ORDER_SleepTV
			"Aye %1. I'll surf the web.",	// ORDER_SleepWeb
			"Aye %1. I'll send a damage report.",	// ORDER_DamageReport
			"Aye %1. I'll conduct a planet survey scan.",	// ORDER_SurveyScanPlanet
			"Aye %1. I'll conduct a system survey scan.",	// ORDER_SurveyScanSystem
			"Aye %1. I'll conduct a tactical scan.",	// ORDER_TacticalScan
			"Aye %1. I'll report to the doc for a checkup.",	// ORDER_EnterSurgery
			"Aye %1. I have the con.",	// ORDER_TakeCommand
			"Aye %1. You have the con.",	// ORDER_ReleaseCommand
			"Aye %1. I'll drop my rider.",					// ORDER_DropRider
			"Aye %1. I'll mount you.",						// ORDER_MountMe
			"Aye %1. I'll mount that target.",				// ORDER_MountTarget
			"Aye %1. I'll dance a jig.",			// ORDER_HengeCeremony
			"Aye %1. I'll move to %2.",	// ORDER_GoToRoom
		};
		myresponse = responses[Order()].arg(addr);
	}

	if(myresponse.contains("%"))
	{
		AuCommChat myreply(myresponse.arg(orderText), msg.ChannelName(), Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), scope);
		Say(myreply, AuPose::MouthClosed);
	}
	else
	{
		AuCommChat myreply(myresponse, msg.ChannelName(), Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), scope);
		Say(myreply, AuPose::MouthClosed);
	}
}

void DaPerson::TakePosture(const AuCommPacket& msg, CITIZEN_POSTURE posture, bool firstInGroup)
{
	if(!IsAIControlled())
		return;

	// Don't permit posture change orders to unassigned Crew.
	if(AssignedShip().IsNull())
		return;

	// Base class quietly accepts the posture change.
	DaSentient::TakePosture(msg, posture, firstInGroup);
	if(!firstInGroup)
		return;	// Only the first in the group responds to the posture change.

	QString addr;
	m_PersonLock.lockForRead();
	if(msg.FromId() == m_Dad)
		addr = tr("dad");
	else if(msg.FromId() == m_Mom)
		addr = tr("mom");
	else
		addr = tr("sir");
	m_PersonLock.unlock();

	// Send a response to the person who gave me the posture change.
	QString myresponse;
	switch(Posture())
	{
	case POSTURE_Berserk:
		if(msg.RecipientCount() > 1)
			myresponse = tr("Aye %1. We'll kill everything we see.").arg(addr);
		else
			myresponse = tr("Aye %1. I'll kill everything I see.").arg(addr);
		break;
	case POSTURE_DefendSelf:
		if(msg.RecipientCount() > 1)
			myresponse = tr("Aye %1. We'll hold our fire unless attacked.").arg(addr);
		else
			myresponse = tr("Aye %1. I'll hold my fire unless attacked.").arg(addr);
		break;
	case POSTURE_DefendArea:
		if(msg.RecipientCount() > 1)
			myresponse = tr("Aye %1. We'll watch for threats that get close.").arg(addr);
		else
			myresponse = tr("Aye %1. I'll watch for threats that get close.").arg(addr);
		break;
	case POSTURE_Offensive:
		if(msg.RecipientCount() > 1)
			myresponse = tr("Aye %1. We'll watch for threats.").arg(addr);
		else
			myresponse = tr("Aye %1. I'll watch for threats.").arg(addr);
		break;
	case POSTURE_HoldFire:
		if(msg.RecipientCount() > 1)
			myresponse = tr("Aye %1. We'll hold our fire.").arg(addr);
		else
			myresponse = tr("Aye %1. I'll hold my fire.").arg(addr);
		break;
	}

	AuCommChat myreply(myresponse, msg.ChannelName(), Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(),
			msg.Scope());
	Say(myreply, AuPose::MouthClosed);
}

void DaPerson::GoToWork()
{
	if(IsInSpacecraft())
	{
		// I am aboard a spacecraft.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship && ship->Is(AssignedShip()))
		{
			// I am aboard my assigned spacecraft. Man a station.
			SetOrder(ORDER_BattleStation, ship->Id(), AuPoint(0,0));
			PoseVisage(AuVisage(AuPose::MouthClosed, 0), AuVisage(AuPose::MouthClosed, 0));
		}
	}
}

// Check to see if I need to run to the sick bay for surgery.
// Insures the ship I am aboard has a functioning surgery unit and medical kits in the hold.
// Returns true if an order is amended to get medical attention.
bool DaPerson::GetACheckup()
{
	// Different kinds of characters have different thresholds for pain.
	if(CurrentHitPoints() >= MaxHitPoints()*2/3)
		return false;											// I am not hurt enough to want medical attention.
	return DaSentient::GetACheckup();
}

// Officer only mans Command Station.
void DaPerson::DoBattleStation()
{
	//g_Au.Log(QString("DaPerson::DoBattleStation %1 %2").arg(Id().GetID()).arg(Name()));
	if(IsRider() || IsMounted())
	{
		// I am on a mount or I am mounted; I am at my station.
		StayIdle();
		return;
	}

	if(IsAboardVehicle())
	{
		// I am in a vehicle; I am at my station.
		// Do random turning and looking behavior.
		StayIdle();
		return;
	}

	if(!IsInSpacecraft())
	{
		// I'm not in a vehicle or a spacecraft. Cancel this order and move on to the next order.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Check to see if I need to run to the sick bay for surgery.
	if(GetACheckup())
		return;													// I have decided to get medical attention.

	// I am aboard a spacecraft.
	DaVessel* ship = dynamic_cast<DaVessel*>(Parent());
	if(!ship)
	{
		// I'm not in a vehicle or a spacecraft. Cancel this order and move on to the next order.
		RequestOrder();	// Ask for my next order.
		return;	// Parent ship not found or not ready for analysis.
	}
	if(PoliticalStance(EmpireId(), ship->EmpireId()) == POL_Enemy)
	{
		RequestOrder();	// Ask for my next order.
		return;	// Ship is not of my empire.
	}
	if(!ship->IsDesignDataCacheValid())
	{
		// Wait for ship data cache to catch up.
		StayIdle();
		return;
	}
	else if(!ship->IsNormal())
		return;				// Wait for ship to be normal. It is probably waiting to wormhole or something.

	// Do station AI based on the berth I hold.
	if(AuRosterEntry::IsOfficer(SpawnShipBerthType()))
	{
		// Officer AI cobbled from DaCitOfficer::DoBattleStation.
		if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 2)
		{
			// Troops are always watching for threats.
			if(LookForThreats(false))
				return;						// Threat spotted; pursue and destroy.
		}

		// Command the ship to accomplish its mission.
		bool imInCommand = false;
		if(HasTheCon() && Is(ship->AIFirstOfficer()))
		{
			// I am in command because I am the first officer and I have the con.
			imInCommand = true;
			CommandShip(ship);
		}

		if(IsCommandStationPose())
		{
			// Sit idly in my Captain's chair.
			StayIdle();

			// Take a nap if the mission is inactive or has no orders.
			if(!imInCommand || !ship->IsOnAMission())
				TakeANap();										// Sleep when I am not on duty.
		}
		else
		{
			// I am not manning a command console yet.
			// Locate the closest unmanned command console to my location.
			// If all consoles are manned, I'll go to my bunk and wait.
			// If I don't have a bunk, I'll wait where I am standing.
			quint16 stationId;
			AuStation station;
			if(ship->ClosestUnoccupiedCommandStation(Location(), stationId, station))
			{
				// Found an unmanned console. Try to sit at the console if it is close enough.
				AuPoint destLoc = station.OpM().Translation();
				AuPoint conLoc = station.OpM().UnProject(Location());	// My location in the space of the console.
				if(conLoc.Length2_2D() < 4*4 || ship->OkToTeleportToStation(this))
				{
					// I am close enough to the console or it's ok to just teleport this dude to his station.
					// Adjust for pose.
					// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
					station.OffsetOpM(ConsolePoseOffset(station.Type(), station.OccPos(), CGHeightStanding(), TorsoThickness(),
								station.SeatIndex()));
					DoSitAtConsole(stationId, station, 0);
				}
				else
				{
					// Move close to the unmanned console.
					// Amend my orders with a movement order that will get me there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), destLoc, stationId);
				}
			}
			else if(ship->BerthDetailIdOccupied(Id()))
			{
				// No unmanned console and I have a berth aboard this ship.
				quint16 roomId;
				AuPoint berthLoc = ship->BerthLocation(Id(), roomId);

				// When going to my berth, don't be too precise about it or my bunkmate and I stand on top of each other.
				if(roomId == ShipRoomId())
				{
					// I am in my bunk room. That's close enough.
					StayIdle();		// Wait for an empty console.

					// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
					//qDebug("DaPerson::DoBattleStation %d %s taking a nap in my bunk",
						//Id().GetID(), (const char*)Name().toAscii());
					TakeANap();
				}
				else
				{
					// Move to my bunk room.
					// Adjust the destination location to be in the room and not exactly on the floor, ceiling, or wall line.
					AuPoint destLoc = berthLoc - ship->DesignCG();
					destLoc.z += CGHeightStanding();
					if(ship->OkToTeleportToStation(this))
					{
						// It's ok to just teleport this dude to his bunk.
						DoTeleport(destLoc);
					}
					else
					{
						// Amend my orders with a movement order that will get me there.
						AmendOrder(ORDER_GoToTarget, ship->Id(), destLoc, 0);
					}
				}
			}
			else
			{
				// No unmanned console and I don't have a berth aboard this ship.
				StayIdle();		// Wait for an empty console.

				// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
				//qDebug("DaPerson::DoBattleStation %d %s taking a nap", Id().GetID(), (const char*)Name().toAscii());
				TakeANap();
			}
		}
	}
	else if(SpawnShipBerthType() == AuRosterEntry::Crew)
	{
		// Crew AI copied from DaCitCrew::DoBattleStation.
		if(IsEngineStationPose())
		{
			// I am operating a sensor console.
			// Operate engineer console even when not in a scene because it is used to turn on the power plant and drives.
			OperateEngineerConsole(ship);
		}
		else if(IsFireControlStationPose())
		{
			// I am operating a fire control console.
			// Operate fire control consoles even when not in a scene because they can reach a long way, beyond the scene.
			OperateFireControlConsole(ship);
		}
		else if(IsNavStationPose())
		{
			// I am operating a nav console.
			// Operate nav console even when not in a scene because it is used to execute mission orders.
			if(Is(ship->AINavigator()))
				OperateNavigatorConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsHelmStationPose())
		{
			// I am operating a helm.
			// Operate pilot console even when not in a scene because it is used to turn on the power plant and gravity drive.
			if(Is(ship->AIPilot()))
				OperatePilotConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsPowerStationPose())
		{
			// I am operating a power relay console.
			// Operate power relay console even when not in a scene because changes may be needed to fix the ship.
			if(Is(ship->AIPowerRelay()))
				OperatePowerRelayConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsSensorStationPose())
		{
			// I am operating a sensor console.
			// Operate sensor console even when not in a scene because it is used to target the weapon bay.
			if(Is(ship->AISensor()))
				OperateSensorConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsShieldStationPose())
		{
			// I am operating a sensor console.
			// Operate shield console even when not in a scene because it is used to turn on the shields.
			if(Is(ship->AIShield()))
				OperateShieldConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsMedicStationPose())
		{
			// I am operating a surgery console.
			// Only operate surgery console if I'm in somebody's scene.
			// WHY? Don't sick dudes go to the sick bay when they need help?
			//if(IsInAnyScene())
				OperateSurgeryConsole(ship);
			//else
				//TakeANap();							// Sleep when I am not on duty.
		}
		else if(IsTransporterStationPose())
		{
			// I am operating a sensor console.
			// Operate transporter console even when not in a scene because it is used to turn off the power when not in use.
			OperateTransporterConsole(ship);
		}
		else if(IsLockedPose())
		{
			// I am manning some other(?) console.
			if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
			{
				// Off duty crew watches for threats.
				if(LookForThreats(false))
					return;						// Threat spotted; pursue and destroy.
			}
			TakeANap();							// Sleep when I am not on duty.
		}
		else
		{
			// I am not manning a console yet.
			if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
			{
				// Off duty crew watches for threats.
				if(LookForThreats(false))
					return;						// Threat spotted; pursue and destroy.
			}

			// Locate the closest unmanned console to my location.
			// If all consoles are manned, I'll go to my bunk and wait.
			// If I don't have a bunk, I'll wait where I am standing.
			quint16 stationId;
			AuStation station;
			quint16 which;
			if(ship->ClosestUnoccupiedCrewStation(Location(), stationId, station, which, BestCrewStationSkill()))
			{
				// Found an unmanned console. Try to sit at the console if it is close enough.
				AuPoint destLoc = station.OpM().Translation();
				AuPoint conLoc = station.OpM().UnProject(Location());	// My location in the space of the console.
				if(conLoc.Length2_2D() < 4*4 || ship->OkToTeleportToStation(this))
				{
					// I am close enough to the console or it's ok to just teleport this dude to his station.
					// Adjust for pose.
					// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
					station.OffsetOpM(ConsolePoseOffset(station.Type(), station.OccPos(), CGHeightStanding(), TorsoThickness(),
								station.SeatIndex()));
					DoSitAtConsole(stationId, station, which);
				}
				else
				{
					// Move close to the unmanned console.
					// Amend my orders with a movement order that will get me there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), destLoc, stationId);
				}
			}
			else if(ship->IsBerthedHere(Id()))
			{
				// No unmanned console and I have a berth aboard this ship.
				quint16 roomId;
				AuPoint berthLoc = ship->BerthLocation(Id(), roomId);

				// When going to my berth, don't be too precise about it or my bunkmate and I stand on top of each other.
				if(roomId == ShipRoomId())
				{
					// I am in my bunk room. That's close enough.
					StayIdle();							// Wait for an empty turret.

					// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
					TakeANap();							// Sleep when I am not on duty.
				}
				else
				{
					// Move to my bunk room.
					// Adjust the destination location to be in the room and not exactly on the floor, ceiling, or wall line.
					AuPoint destLoc = berthLoc - ship->DesignCG();
					destLoc.z += CGHeightStanding();
					if(ship->OkToTeleportToStation(this))
					{
						// It's ok to just teleport this dude to his station.
						DoTeleport(destLoc);
					}
					else
					{
						// Amend my orders with a movement order that will get me there.
						AmendOrder(ORDER_GoToTarget, ship->Id(), destLoc, 0);
					}
				}
			}
			else
			{
				// No unmanned console and I don't have a berth aboard this ship.
				StayIdle();							// Wait for an empty console.

				// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
				TakeANap();							// Sleep when I am not on duty.
			}
		}
	}
	else if(SpawnShipBerthType() == AuRosterEntry::Troop)
	{
		// Troop AI cobbled from DaCitTroop::DoBattleStation.
		// Aboard a spacecraft, troops only operate turrets.
		// Turrets are not operated when not in any scene.
		// This also prevents this dude from moving to a turret when not in any scene.
		if(IsInAnyScene())
		{
			// Troops are always watching for threats.
			if(IsTurretPose())
			{
				// Troops watch for threats less often when their head is in a turret.
				if(LastLookForThreatsTime().ElapsedSec() > 5 && LookForThreats(false))
					return;					// Threat spotted; pursue and destroy.
			}
			else
			{
				// Troops are alert for threats when not operating a turret.
				if(LastLookForThreatsTime().ElapsedSec() > 1 && LookForThreats(false))
					return;					// Threat spotted; pursue and destroy.
			}
		}
		else
		{
			// I am not in any scene.
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
			if(system && !system->IsEnemyEmpirePresent(ship->EmpireId()))
				return;	// Do nothing when the system contains no enemy units.
		}

		// Close jammed doors if the atmosphere inside is bad.
		AuPoint doorLoc;
		if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
		{
			doorLoc = ship->FloorLocInsideDoor(doorLoc);
			AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
			AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		}
		else if(IsTurretPose())
		{
			// I am operating a turret.
			// We only operate turrets when we are in a scene. Turrets don't reach far enough to need this work outside of a scene.
			// This test also insures that my turret is not currently turning; when turning we let the turret keep turning until it
			// reaches the angle I selected.
			OperateTurret(ship);
		}
		else if(IsLockedPose())
		{
			// I am manning some other console, probably a guard post.
			TakeANap();							// Sleep when I am not on duty.
		}
		else
		{
			// I am not manning a turret or at a guard post yet.
			// Locate the closest unmanned turret or troop station to my location.
			// If all turrets and troop stations are manned, I'll go to my bunk and wait.
			// If I don't have a bunk, I'll wait where I am standing.
			quint16 stationId;
			AuStation station;
			quint16 which;
			if(ship->ClosestUnoccupiedTroopStation(Location(), stationId, station, which, Id()))
			{
				// Found an unmanned turret or troop station. Try to sit at the turret if it is close enough.
				AuPoint destLoc = station.OpM().Translation();
				//if(Location().Distance2_2D(destLoc) < 5*5 || ship->OkToTeleportToStation(this))
				AuPoint conLoc = station.OpM().UnProject(Location());	// My location in the space of the console.
				if(conLoc.Length2_2D() < 4*4 || ship->OkToTeleportToStation(this))
				{
					// I am close enough to the turret/post or it's ok to just teleport this dude to his station.
					// Adjust for pose.
					// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
					station.OffsetOpM(ConsolePoseOffset(station.Type(), station.OccPos(), CGHeightStanding(), TorsoThickness(),
								station.SeatIndex()));
					DoSitAtConsole(stationId, station, which);
				}
				else
				{
					// Move close to the unmanned turret station or guard post loc.
					// Amend my orders with a movement order that will get me there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), destLoc, stationId);
				}
			}
			else if(ship->BerthDetailIdOccupied(Id()))
			{
				// No unmanned turret and I have a berth aboard this ship.
				quint16 roomId;
				AuPoint berthLoc = ship->BerthLocation(Id(), roomId);

				// When going to my berth, don't be too precise about it or my bunkmate and I stand on top of each other.
				if(roomId == ShipRoomId())
				{
					// I am in my bunk room. That's close enough.
					StayIdle();		// Wait for an empty turret.

					// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
					TakeANap();
				}
				else
				{
					// Move to my bunk room.
					// Adjust the destination location to be in the room and not exactly on the floor, ceiling, or wall line.
					AuPoint destLoc = berthLoc - ship->DesignCG();
					destLoc.z += CGHeightStanding();
					if(ship->OkToTeleportToStation(this))
					{
						// It's ok to just teleport this dude to his station.
						DoTeleport(destLoc);
					}
					else
					{
						// Amend my orders with a movement order that will get me there.
						AmendOrder(ORDER_GoToTarget, ship->Id(), destLoc, 0);
					}
				}
			}
			else
			{
				// No unmanned turret and I don't have a berth aboard this ship.
				StayIdle();		// Wait for an empty turret.

				// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
				TakeANap();
			}
		}
	}
	else
	{
		// Passengers don't do anything.
		RequestOrder();	// Ask for my next order.
		return;
	}
}

void DaPerson::ShipLog(DaVessel* ship)
{
	// Do not send ship's log while in arena.
	if(InArena())
		return;

	//qDebug() << "DaPerson::ShipLog" << Id() << Name() << "aboard" << ship->Id() << ship->Name() << "when" << now;
	QString title = tr("Captain's Log");
	QString rpt = MailHeader(ship, title, tr("Commander,"));

	AuMailMsg::MSGTYPE rptMessageType = AuMailMsg::MSG_ShipLog;	// Types are defined in aumailmsg.h

	// Check for unexplored destination.
	if(ship->IsOnAMission() && !DecideWhetherToTravel(ship))
		rpt += tr("<p>Our mission takes us into unexplored space. "
				"I ordered the crew to hold our position until you are present.");

	// Comments
	rpt += tr("<p>The crew has high spirits. "
			"Every day seems filled with anticipation.");

	// Closing
	rpt += tr("<p>"
		"Yours in service,<br>"
		"<i>Demiavatar %1</i><p>").arg(Name());

	// Event Log
	m_PersonLock.lockForWrite();
	QString eventRpt = m_Log.ReportHtml("EVENT LOG", m_LogTime);
	if(!eventRpt.isEmpty())
	{
		rpt += eventRpt;
		if(rptMessageType != AuMailMsg::MSG_ShipLogDistress && m_Log.ContainsAlert(m_LogTime))
			rptMessageType = AuMailMsg::MSG_ShipLogAlert;		// Types are defined in aumailmsg.h
	}
	m_LogTime.Set();
	m_PersonLock.unlock();

	// Damage Report
	rpt += ship->DamageReportHtml();

	// Account
	rpt += "<b>ACCOUNT</b><div style=\"margin-left: 10px;\">";
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> %L2%3 Ship's Fund<br>")
		.arg(g_Au.CommodityBitmapName(CID_Money))
		.arg((int)ship->AccountBalance())
		.arg(g_Au.CurrencySymbol());
	if(!ship->IsPrivatelyOwned())
	{
		rpt += QString("<img height=\"16\" src=\"qrc%1\"> %L2%3 Fleet Tax Collected<br>")
			.arg(g_Au.CommodityBitmapName(CID_Money))
			.arg((int)ship->AccountBalanceSubsidyTax())
			.arg(g_Au.CurrencySymbol());
	}
	rpt += "</div>";

	// Fuel
	rpt += "<b>FUEL</b><div style=\"margin-left: 10px;\">";
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> Hydrogen %2 Q%3 in %4 Fuel Cells<br>")
		.arg(g_Au.CommodityBitmapName(CID_Hydrogen))
		.arg((int)ship->FuelQuantity())
		.arg(ship->FuelQuality())
		.arg(ship->FuelCapacity());
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> Electricity %2 in %3 Capacitor<br>")
		.arg(g_Au.CommodityBitmapName(CID_Electricity))
		.arg((int)ship->CapacitorLevel())
		.arg((int)ship->CapacitorMax());
	if(ship->FuelQuantity() < 1 && ship->CapacitorPercent() < 1)
		rptMessageType = AuMailMsg::MSG_ShipLogDistress;	// Types are defined in aumailmsg.h
	rpt += "</div>";

	// Cargo in Hold
	if(ship->CargoCapacityVolume())
	{
		rpt += tr("<b>CARGO</b><div style=\"margin-left: 10px;\">");
		rpt += tr("Hold Space %1<br>").arg(ship->CargoCapacityVolume());

		// Make a map of the inventory table contents so it will be alphabetized.
		QString qual;
		QMap<QString, QMap<CommoditySubKey, QString> > inven;		// Key is commodity name.
		QMap<QString, QString> bitmap;		// Key is commodity name.
		const AuItemHash& sinv = ship->LockCargoForRead();
		for(AuItemHash::const_iterator it = sinv.begin(); it != sinv.end(); it++)
		{
			QString commod = (*it).second->ListName();
			if(!bitmap.contains(commod))
				bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

			QMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = inven.find(commod);
			if(iit == inven.end())
				iit = inven.insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

			if(iit != inven.end())
			{
				CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
				if((*it).second->EmpireOfOrigin())
				{
					// The item has a known empire of origin.
					QString empireOfOrigin;
					if((*it).second->EmpireOfOrigin() != EmpireId())
					{
						// Item was created in an empire different from mine.
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();
					}
					if(!empireOfOrigin.isEmpty())
					{
						// Item was created in another empire.
						(*iit).insert(key, tr("%1 Q%2 Made in %3")
								.arg((*it).second->Count())
								.arg((*it).second->Quality())
								.arg(empireOfOrigin));
					}
					else
					{
						// Item was created in my empire.
						(*iit).insert(key, tr("%1 Q%2")
								.arg((*it).second->Count())
								.arg((*it).second->Quality()));
					}
				}
				else
				{
					// The item does not have a known empire of origin. Probably hand made or a carcass.
					(*iit).insert(key, tr("<i>%1 Q%2</i>")
							.arg((*it).second->Count())
							.arg((*it).second->Quality()));
				}
			}
		}
		ship->UnlockCargo();

		// Create a table of commodities.
		if(!inven.isEmpty())
		{
			rpt += tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			for(QMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = inven.cbegin();
					iit != inven.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += tr("<tr><td><img height=\"16\" src=\"qrc%3\">%1</td><td>%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}
		rpt += "</div>";
	}

	// Mission
	rpt += "<b>MISSION</b><div style=\"margin-left: 10px;\">";
	if(ship->MissionIsEmpty())
	{
		rpt += QString("%1, %2, No Orders<br>")
			.arg(ship->IsMissionActive()?"Active":"On Hold")
			.arg((ship->MissionType() == AuMission::Repeat)?"Cyclic":"Do Once");
	}
	else
	{
		rpt += QString("%1, %2<br>")
			.arg(ship->IsMissionActive()?"Active":"On Hold")
			.arg((ship->MissionType() == AuMission::Repeat)?"Cyclic":"Do Once");

		rpt += ship->MissionHtmlReportTable();
	}
	rpt += "</div>";

	// Roster
	rpt += "<b>ROSTER</b><div style=\"margin-left: 10px;\">";
	{
		if(ship->CaptainsBerthType() == AuRosterEntry::CaptainSlot)
			rpt += QString("No Berths, Captain Slot Only<br>");
		else
		{
			if(ship->OfficerBerths())
				rpt += QString("%L1 Officer Berths<br>").arg(ship->OfficerBerths());
			if(ship->CrewBerths())
				rpt += QString("%L1 Crew Berths<br>").arg(ship->CrewBerths());
			if(ship->TroopBerths())
				rpt += QString("%L1 Troop Berths<br>").arg(ship->TroopBerths());
			if(ship->PassengerBerths())
			{
				if(ship->IsFirstClass())
					rpt += QString("%L1 1st Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else if(ship->IsSecondClass())
					rpt += QString("%L1 2nd Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else
					rpt += QString("%L1 3rd Class Passenger Berths<br>").arg(ship->PassengerBerths());
			}
			if(ship->CryoBerths())
				rpt += QString("%L1 Cryo Berths<br>").arg(ship->CryoBerths());
		}

		// Create a map of the roster so it will be alphabetized.
		QMap<QString, const char*> occs;		// Key is occupant name, value is icon resource name.
		const AuRoster& roster = ship->LockRoster();
		for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
			occs.insert((*ri).OccupantName(), (*ri).OccupantIcon());
		ship->UnlockRoster();

		rpt += tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(32, 32, 32);\"><tbody>");
		for(QMap<QString, const char*>::const_iterator oi = occs.cbegin(); oi != occs.cend(); oi++)
			rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td></tr>") .arg(*oi) .arg(oi.key());
		rpt += "</tbody></table><br>";

		// List occupied cryo berths.
		QMap<QString, const char*> cryos;		// Key is occupant name, value is icon resource name.
		const AuCryoBerths& cryo = ship->LockCryoBerths();
		for(AuCryoBerths::const_iterator it = cryo.constBegin(); it != cryo.constEnd(); it++)
		{
			QString specieName = it.key().NameOrDesc();
			QString c = QString("%1, %L2").arg(specieName).arg(*it);
			cryos.insert(c, ":data/CryoPassenger.png");
		}
		ship->UnlockCryoBerths();

		if(!cryos.empty())
		{
			rpt += tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(32, 32, 32);\"><tbody>");
			for(QMap<QString, const char*>::const_iterator oi = cryos.cbegin(); oi != cryos.cend(); oi++)
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td></tr>") .arg(*oi) .arg(oi.key());
			rpt += "</tbody></table><br>";
		}
	}
	rpt += "</div>";

	// Send the report by mail to this person.
	const DgData& gd = g_Au.LockDataForRead();
	unsigned int idPlayer = gd.PlayerId();				// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	// List myself, captain and human officers as recipients.
	AuIdList recipients;
	recipients.append(Id());
	const AuRoster& roster = ship->LockRoster();
	for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
		if((*ri).IsHumanOfficer() && !Is((*ri).m_OccId))	// Also includes captain. Skips myself.
			recipients.append((*ri).m_OccId);
	ship->UnlockRoster();

	// Collect current system and world information for mail message. It helps people navigate to the mail origin.
	AuId systemId, worldId;
	QString systemName, worldName;
	AuPoint galaxyLoc;
	GALAXY galaxy = GALAXY_Count;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(system)
	{
		systemId = system->Id();
		systemName = system->Name();
		const DaGalaxy* pg = system->FindParentGalaxy();
		if(pg)
		{
			galaxy = pg->Galaxy();
			system->GetLocationRelativeToParent(galaxyLoc, pg);
		}
	}
	const DaWorld* world = FindParentPlanetInEnv();
	if(world)
	{
		worldId = world->Id();
		worldName = world->Name();
	}

	g_Au.SendBulkMail(idPlayer, confirmNum, ship->Id(), ship->Name(), recipients, rptMessageType, title, rpt,
			systemId, systemName, worldId, worldName, galaxy, galaxyLoc);

	SetModified(NOTIFY_Database);
}

bool DaPerson::InChainOfCommand(const AuId& personId, quint32 personEmpire) const
{
	Q_UNUSED(personEmpire);
	if(!IsAIControlled())
		return false;	// Only in chain-of-command while AI controlled.

	// Demiavatars are always in the chain of command of their parents.
	m_PersonLock.lockForRead();
	bool d = personId == m_Dad || personId == m_Mom;
	m_PersonLock.unlock();
	if(d)
		return true;

	if(SpawnShip().IsNull())
		return false;	// Non-crew is not in anyone's chain-of-command.

	if(SpawnShipBerthType() == AuRosterEntry::Captain || SpawnShipBerthType() == AuRosterEntry::CaptainSlot)
		return false;	// Captain is not in anyone's chain-of-command.

	// I am an AI controlled crew member.
	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return false;

	const DaVessel* ship = dynamic_cast<const DaVessel*>(system->FindIdRecursive(SpawnShip()));
	AuRosterEntry::TYPE personBerthType;
	if(ship)
	{
		// Found my ship. I can use the roster to check command chain.
		personBerthType = ship->BerthTypeOccupied(personId);
	}
	else
	{
		// Did not find my ship. Can I find the person?
		const DaPerson* person = dynamic_cast<const DaPerson*>(system->FindIdRecursive(personId));
		if(!person)
			return false;	// Did not find the person either.

		// Found the person.
		if(person->SpawnShip() != SpawnShip())
			return false;	// Crew of different ships.

		personBerthType = person->SpawnShipBerthType();
	}

	// I always answer to captain and human officers.
	if(personBerthType == AuRosterEntry::Captain || personBerthType == AuRosterEntry::OfficerHuman)
		return true;

	// Passengers also answer to crew.
	if(SpawnShipBerthType() == AuRosterEntry::PassengerHuman)
		return personBerthType == AuRosterEntry::Crew;

	// I am not in the person's chain-of-command if we made it this far.
	return false;
}

bool DaPerson::UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const
{
	//if(Is(sceneAtom->Id()))
		//return true;
	if(!InChainOfCommand(sceneAtom->Id(), sceneAtom->EmpireId()))
		return false;
	if(topDownView && IsInSpacecraft())
	{
		if(!sceneAtom->IsInSpacecraft())
			return false;
		if(FindParentVesselInEnv() != sceneAtom->FindParentVesselInEnv())
			return false;
	}
	return true;
}

QString DaPerson::DescribeActivity() const
{
	if(IsMating())
		return DaSentient::DescribeActivity();
	QString order;
	switch(Order())
	{
	default:
		order = DaSentient::DescribeActivity();
		break;
	case ORDER_SleepDayDream:
		order = tr("Reliving Adventures as an Avatar");
		break;
	case ORDER_SleepExercise:
		order = tr("Yoga");
		break;
	case ORDER_SleepPine:
		order = tr("Wishing %1 Had Better Graphics").arg(g_Au.GameName());
		break;
	case ORDER_SleepRead:
		order = tr("Reading a Novel");
		break;
	}

	if(HasTheCon())
	{
		if(dynamic_cast<const DaVessel*>(Parent()))
			order = tr("Commanding %1, %2").arg(Parent()->Name()).arg(order);
		else
			order = tr("Commanding Spacecraft, %1").arg(order);
	}

	return order;
}

// Only called when I am AI controlled.
void DaPerson::BodyHasDied(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
		const QString& whoName)
{
	// Remove me from my owner's login account.
	emit DemiavatarDied(Id());

	if(!IsPirate())
	{
		// Officer assigned to a ship drops event log disk even if the event log is empty.
		// Pirates do not do this because they do not maintain an event log.
		if(!AssignedShip().IsNull() && AuRosterEntry::IsOfficer(SpawnShipBerthType()))
		{
			// I am assigned to a ship and I am not a pirate.
			QString home = SpawnCityName();

			// Assigned ship and description. Could be blank.
			QString shipName, shipDesc;
			bool shipPrivate = false;
			{
				const DaVessel* ship = FindParentVesselInEnv();
				if(!ship || !ship->Is(AssignedShip()))
				{
					// Not aboard a ship or it's not my assigned ship.
					const DaAtom* system = FindEnvironmentAtom();
					if(system)
						ship = dynamic_cast<const DaVessel*>(system->FindIdRecursive(AssignedShip()));
					else
						ship = 0;
				}
				if(ship)
				{
					shipName = ship->NameWithEmpire();
					shipDesc = ship->Description();
					shipPrivate = ship->IsPrivatelyOwned();
				}
			}

			// Drop the item. The disk is created out of thin air so I will assign it a random quality value.
			m_PersonLock.lockForRead();
			AuItemOfficerLog itemLog(Name(), home, shipName, shipDesc, shipPrivate, m_Log, CID_OfficerLog, QualityBaseline(),
					EmpireId());
			m_PersonLock.unlock();

			DropItem(itemLog);
		}

		// Demiavatar reports death. Pirates do not send mail
		DeathLog(weaponCID, weaponQ, effect1, effect2, whoName);	// Demiavatar notifies chain-of-command when dying.
	}

	// Base class drops a random item and meat. It also releases my berth.
	DaSentient::BodyHasDied(weaponCID, weaponQ, effect1, effect2, whoName);
}

// Demiavatar notifies chain-of-command when killed.
void DaPerson::DeathLog(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
		const QString& whoName)
{
	// Do not send death log while in arena.
	if(InArena())
		return;

	QString title = tr("Defeat");
	QString rpt = MailHeader(tr("Re: %1").arg(title), "Family and Friends,");

	// Reason for Destruction
	// I'm not really sure if we ever get here with an invalid weaponCID.
	if(weaponCID != CID_Invalid)
	{
		// Report weapon Q only if it's more than 0.
		QString tlgotme = QString(" Q%1").arg(weaponQ);
		if(!whoName.isEmpty())
		{
			if(effect2 != WE_None)		// Always implies effect1 != WE_None.
				rpt += tr("I have fallen in the line of duty.<p>"
						"%3 got me using a %4%5. "
						"These %6 and %7 wounds are fatal; I am dying."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(whoName).arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect2)).arg(EffectName(effect1));
			else if(effect1 != WE_None)
				rpt += tr("I have fallen in the line of duty.<p>"
						"%3 got me using a %4%5. "
						"This %6 wound is fatal; I am dying."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(whoName).arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect1));
			else
				rpt += tr("I have fallen in the line of duty.<p>"
						"%3 got me using a %4%5."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(whoName).arg(g_Au.CommodityName(weaponCID)).arg(tlgotme);
		}
		else
		{
			if(effect2 != WE_None)		// Always implies effect1 != WE_None.
				rpt += tr("I have fallen in the line of duty.<p>"
						"%4%5 got me. "
						"These %6 and %7 wounds are fatal; I am dying."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect2)).arg(EffectName(effect1));
			else if(effect1 != WE_None)
				rpt += tr("I have fallen in the line of duty.<p>"
						"%4%5 got me. "
						"This %6 wound is fatal; I am dying."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect1));
			else
				rpt += tr("I have fallen in the line of duty.<p>"
						"%4%5 got me."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(g_Au.CommodityName(weaponCID)).arg(tlgotme);
		}
	}
	else
	{
		if(effect2 != WE_None)		// Always implies effect1 != WE_None.
			rpt += tr("I have fallen in the line of duty.<p>"
					"These %6 and %7 wounds are fatal; I am dying."
					"<p>"
					"Unconsiously,<br>"
					"<i>%1</i>")
				.arg(Name())
				.arg(EffectName(effect2)).arg(EffectName(effect1));
		else if(effect1 != WE_None)
			rpt += tr("I have fallen in the line of duty.<p>"
					"This %6 wound is fatal; I am dying."
					"<p>"
					"Unconsiously,<br>"
					"<i>%1</i>")
				.arg(Name())
				.arg(EffectName(effect1));
		else
			rpt += tr("I have fallen in the line of duty.<p>"
					"Unconsiously,<br>"
					"<i>%1</i>")
				.arg(Name());
	}

	// Send the report by mail to my officers and to my parents.
	const DgData& gd = g_Au.LockDataForRead();
	unsigned int idPlayer = gd.PlayerId();				// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	AuIdList recipients;
	// List captain and human officers as recipients.
	const DaVessel* ship = FindParentVesselInEnv();
	if(ship && ship->Is(AssignedShip()))
	{
		const AuRoster& roster = ship->LockRoster();
		for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
			if((*ri).IsHumanOfficer())	// Also includes captain.
				recipients.append((*ri).m_OccId);
		ship->UnlockRoster();
	}

	m_PersonLock.lockForRead();
	if(!m_Dad.IsNull() && !recipients.contains(m_Dad))
		recipients.append(m_Dad);
	if(!m_Mom.IsNull() && !recipients.contains(m_Mom))
		recipients.append(m_Mom);
	m_PersonLock.unlock();

	if(!recipients.isEmpty())
	{
		// Collect current system and world information for mail message. It helps people navigate to the mail origin.
		AuId systemId, worldId;
		QString systemName, worldName;
		AuPoint galaxyLoc;
		GALAXY galaxy = GALAXY_Count;
		const DaSolarSystem* system = FindParentSolarSystem();
		if(system)
		{
			systemId = system->Id();
			systemName = system->Name();
			const DaGalaxy* pg = system->FindParentGalaxy();
			if(pg)
			{
				galaxy = pg->Galaxy();
				system->GetLocationRelativeToParent(galaxyLoc, pg);
			}
		}
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			worldId = world->Id();
			worldName = world->Name();
		}

		g_Au.SendBulkMail(idPlayer, confirmNum, Id(), Name(), recipients, AuMailMsg::MSG_OfficerDeath, title, rpt,
				systemId, systemName, worldId, worldName, galaxy, galaxyLoc);
	}

	SetModified(NOTIFY_Database);
}

void DaPerson::TransporterSetCoord(const AuPoint& coord)
{
	if(!HasConsole())
		return;

	DpStation* console = LockConsoleForRead();
	DpStationTransporter* transcon = dynamic_cast<DpStationTransporter*>(console);
	if(transcon)
		transcon->SetCoord(coord);
	UnlockConsole();
	SetModified(NOTIFY_Scene);
}

void DaPerson::TransporterSetLongLat(const AuGlobeLLF& ll)
{
	if(!HasConsole())
		return;

	DpStation* console = LockConsoleForRead();
	DpStationTransporter* transcon = dynamic_cast<DpStationTransporter*>(console);
	if(transcon)
		transcon->SetLongLat(ll);
	UnlockConsole();
	SetModified(NOTIFY_Scene);
}

// Borrow 'amount' from the bank.
void DaPerson::BankLoan(double amount, const AuId& opBd)
{
	if(InIndustry() != IID_Bank && InIndustry() != IID_Capitol)
	{
		Thought(tr("I must go to a bank to take out a loan."));
		return;
	}

	double creditLimit = CreditLimit();
	double loanBalance = LoanBalance();
	double availCredit = creditLimit - loanBalance;
	if(availCredit <= 0)
	{
		Thought(tr("You have no credit available to borrow any money."));
		return;
	}

	if(amount > availCredit)
	{
		Thought(tr("You have insufficient credit available to borrow that much money."));
		return;
	}

	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;

	double bankFunds = bd->BankLoanFundsAvailable();
	if(amount > bankFunds)
	{
		Thought(tr("Bank reserve policies limit loans to %L1%2 at this time. I apologize for the inconvenience.")
				.arg(bankFunds, 0,'f',0)
				.arg(g_Au.CurrencySymbol()));
		return;
	}

	// Borrow the money.
	bd->DebitAccountCitizenLoans(amount);
	LoanBorrow(amount);
	CreditAccount(amount);
	SetModified();
	Thought(tr("%L1%2 was transferred to your cash account. Thank you for your business.")
			.arg(amount, 0,'f',0)
			.arg(g_Au.CurrencySymbol()));
}

// Pay 'amount' toward my current bank loan debt.
void DaPerson::BankPay(double amount, const AuId& opBd)
{
	if(InIndustry() != IID_Bank && InIndustry() != IID_Capitol)
	{
		Thought(tr("I must go to a bank to make a loan payment."));
		return;
	}

	double accountBalance = AccountBalance();
	if(amount > accountBalance)
	{
		Thought(tr("You do not have that much money."));
		return;
	}

	if(amount > LoanBalance())
	{
		Thought(tr("You do not owe that much money to the bank."));
		return;
	}

	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;

	// Make a loan payment.
	bd->CreditAccount(amount, AuBank::Civ_CreditLoans);
	LoanPay(amount);
	DebitAccount(amount);
	SetModified();

	// Zero out my loan balance if I ended up with a fraction.
	if(LoanBalance() < 1.)
		LoanPay(LoanBalance());

	if(LoanBalance() > 0)
		Thought(tr("%L1%2 was paid toward your loan. Your remaining loan balance is %L3%2. Thank you for your business.")
				.arg(amount, 0,'f',0)
				.arg(g_Au.CurrencySymbol())
				.arg(LoanBalance(), 0,'f',0));
	else
		Thought(tr("%L1%2 was paid toward your loan. Congratulations! You are debt free. Thank you for your business.")
				.arg(amount, 0,'f',0)
				.arg(g_Au.CurrencySymbol()));
}

double DaPerson::CreditLimit() const
{
	double rep = Reputation();
	if(rep < 0.)
		rep = -.25;	// So final result will be 25k.
	double limit = (rep + .5) * 100000.;
	if(limit > 1000000000000000)
		limit = 1000000000000000;
	return limit;
}

bool DaPerson::HasJob(const AuJobOffer& jobOffer) const
{
	if(!jobOffer.Job())
		return false;

	bool hasJob = false;
	m_JobsLock.lockForRead();
	for(AuJobList::const_iterator ji = m_Jobs.cbegin(); ji != m_Jobs.cend(); ji++)
	{
		if(jobOffer.Job()->IsLike(**ji))
		{
			hasJob = true;
			break;
		}
	}
	m_JobsLock.unlock();
	return hasJob;
}

QImage DaPerson::SelfPortrait(AuAtomRenderInfo& ari, const QSize& size, const QColor& background)
{
	// Setup the texture that we will be rendering into.
	GLuint glTex;
	ari.re->glGenTextures(1, &glTex);
	ari.re->glActiveTexture(GL_TEXTURE0);
	ari.re->glBindTexture(GL_TEXTURE_2D, glTex);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ari.re->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	ari.re->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_FLOAT, NULL);

	// Get current frame buffer and render buffer bindings; they must be restored properly.
	GLint oldfbo;
	ari.re->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);
	GLint oldrbo;
	ari.re->glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldrbo);

	// Attach the texture to the frame buffer so we can render to it
	GLuint fbo;
	ari.re->glGenFramebuffers(1, &fbo);
	ari.re->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	ari.re->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, glTex, 0);

	// Create a render buffer for the depth component of the frame buffer rendering.
	GLuint depth_rb;
	ari.re->glGenRenderbuffers(1, &depth_rb);
	ari.re->glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
	ari.re->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.width(), size.height());
	ari.re->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);

	// Clear the scene in the frame buffer.
	ari.re->glClearColor(background.redF(), background.greenF(), background.blueF(), background.alphaF());
	ari.re->glViewport(0, 0, size.width(), size.height());
	ari.re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	ari.re->glEnable(GL_CULL_FACE);		// Enable face culling.
	ari.re->glFrontFace(GL_CW);

	// Create projection and model matrices to place mini me in the scene.
	double cght = CGHeightStanding();
	float headTilt = CurrentHeadTilt();	// Straighten head so CameraOffset does not bob up and down when I look up and down. It
	TiltHeadTo(0);						// makes my guy look like he's stretching.
	double viewHeight = (CameraOffset().z + cght + 1) * 1.1;	// Multiply to create a little margin.
	TiltHeadTo(headTilt);
	if(viewHeight < 3)
		viewHeight = 3;
	GLfloat windowAspect = (GLfloat)size.height() / (GLfloat)size.width();
	AuMatrix4x4 p;
	p.Scale(windowAspect, 1, 1./-100.);	// Scene depth is 100'.
	AuMatrix4x4 m;
	if(CurrentGeneralPose() == AuPose::EyesOnly || IsBodyDyingOrDead() || CurrentGeneralPose().IsMatingOrWaiting())
	{
		m.Translate(0, 0, -20);			// Positioned 20' into the scene.
		m.RotateX(-PI/2);
		m.RotateZ(Rotation().z);
	}
	else
	{
		double gnd = (-viewHeight/2 + CGHeightStanding() + .5)/(viewHeight/2);
		m.Translate(0, gnd, -20);		// Positioned 20' into the scene.
		m.RotateX(-PI/2);
		m.RotateZ(Rotation().z);
	}

	// Render mini me.
	double minisize = 1./(viewHeight/2.);
	SetMiniScale(minisize);
	if(IsEthereal())
	{
		// Render entity only.
		AuRenderLastMap renderLast;
		AuModelMatrixHash rmod;
		rmod.insert(Id(), m);
		DaAtomList shadSc;
		DaAtomList shadScFog;
		DaAtomList shadScGlobe;
		DaAtomList shadScGlobeFog;
		DaAtomList shadScRingworld;
		DaAtomList shadScRingworldFog;
		DaUnit::RenderEnvironmentObject(p, ari, rmod, this, renderLast, PASS_Near, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
				shadScRingworld, shadScRingworldFog);
		RenderLast(p, ari, rmod, this, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
	}
	else
		RenderMiniMe(p, m, ari, minisize);
	ResetMiniScale();

	// We are now finished with our frame buffer and associated Z buffer.
	ari.re->glBindRenderbuffer(GL_RENDERBUFFER, oldrbo);
	ari.re->glDeleteRenderbuffers(1, &depth_rb);
	ari.re->glBindFramebuffer(GL_FRAMEBUFFER, oldfbo);
	ari.re->glDeleteFramebuffers(1, &fbo);

	// At this point the texture is complete.
	// Make a QImage out of the texture that was created.
	QImage img(size, QImage::Format_ARGB32);
	img.fill(Qt::black);
	GLfloat* buffer = (GLfloat*)malloc(size.width()*size.height()*4*sizeof(GLfloat));
	ari.re->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
	for(int w=0; w<size.width(); w++)
	{
		for(int h=0; h<size.height(); h++)
		{
			int i = ((h*size.width()) + w)*4;
			QRgb value = qRgba((int)(buffer[i]*255), (int)(buffer[i+1]*255), (int)(buffer[i+2]*255), (int)(buffer[i+3]*255));
			//QRgb value = qRgba((int)(buffer[i]*255), (int)(buffer[i+1]*255), (int)(buffer[i+2]*255), 255);
			img.setPixel(w, size.height()-h-1, value);	// Mirror vertically.
		}
	}
	free(buffer);

	// Clean up.
	ari.re->glClearColor(0,0,0,1);
	ari.re->glBindTexture(GL_TEXTURE_2D, 0);
	ari.re->glDeleteTextures(1, &glTex);
	ari.ResetViewport();

	return img;
}

void DaPerson::UpdateSelfie(const QImage& img)
{
	m_PersonLock.lockForWrite();
	m_Selfie = img;
	m_PersonLock.unlock();
	SetModified(NOTIFY_Database);
}

int DaPerson::ResetStory(quint32 scriptId)
{
	// Forget that I ever finished the story.
	Stats().ForgetStory(scriptId);

	m_JobsLock.lockForWrite();

	// Depart from the story if it is currently active.
	int result = 0;
	for(AuJobList::iterator ji = m_Jobs.begin(); ji != m_Jobs.end(); ji++)
	{
		AuJobStory* storyJob = dynamic_cast<AuJobStory*>(*ji);
		if(!storyJob)
			continue;

		quint32 storyInstance = storyJob->StoryInstance();
		if(!storyInstance)
			continue;

		AuStory* story = g_Au.LockStory(storyInstance);
		if(!story || story->ScriptId() != scriptId)
		{
			g_Au.UnlockStory(storyInstance);
			continue;
		}

		DepartStoryNoLock(storyInstance, story);
		result = 1;
		g_Au.UnlockStory(storyInstance);
		break;
	}

	m_JobsLock.unlock();

	// Forget that I ever started the story.
	ForgetStoryStarted(scriptId);

	SetModified();
	return result;
}

void DaPerson::DestructStoryItems()
{
	//qDebug() << "DaPerson::DestructStoryItems" << Id() << Name();
	AuGear* gear = LockGearForWrite();
	if(gear && gear->DestructStoryItems())
		UpdateGearWeight();
	UnlockGear();
}

void DaPerson::SetSoundHeard(const AuPoint& fromDir, float audibility)
{
	m_PersonLock.lockForWrite();
	m_HitNotify = AuHitNotify(fromDir, WE_Audible, audibility*20, 0, WE_None, 0, 0);
	m_tmHitChange.Set();
	m_PersonLock.unlock();
	SetModified();
}

void DaPerson::Remember(const AuMemory& mem)
{
	m_MemoryLock.lockForWrite();
	m_Memories.Remember(mem);
	m_MemoryLock.unlock();
	SetModified(NOTIFY_Database);
}

AuTime DaPerson::FirstMemoryOfCity(const AuId& cityId) const
{
	AuTime memTime;
	m_MemoryLock.lockForRead();
	memTime = m_Memories.FirstMemoryOfCity(cityId);
	m_MemoryLock.unlock();
	return memTime;
}

void DaPerson::ForgetCity(const AuId& cityId)
{
	m_MemoryLock.lockForWrite();
	m_Memories.ForgetCity(cityId);
	m_MemoryLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaPerson::ForgetStoryStarted(quint32 scriptId)
{
	m_MemoryLock.lockForWrite();
	m_Memories.ForgetStoryStarted(scriptId);
	m_MemoryLock.unlock();
	SetModified(NOTIFY_Database);
}

bool DaPerson::HasStartedStory(quint32 scriptId) const
{
	m_MemoryLock.lockForRead();
	bool v = m_Memories.HasStartedStory(scriptId);
	m_MemoryLock.unlock();
	return v;
}

// Last time a story script was ever started.
AuTime DaPerson::LastStoryStart(unsigned int scriptId) const
{
	m_MemoryLock.lockForRead();
	AuTime v = m_Memories.LastStartedStory(scriptId);
	m_MemoryLock.unlock();
	return v;
}

AuTime DaPerson::LastStoryTerminated(unsigned int scriptId) const
{
	m_MemoryLock.lockForRead();
	AuTime v = m_Memories.LastTerminatedStory(scriptId);
	m_MemoryLock.unlock();
	return v;
}

quint8 DaPerson::StoryLastResult(quint32 scriptId) const
{
	m_MemoryLock.lockForRead();
	quint8 v = m_Memories.LastStoryResult(scriptId);
	m_MemoryLock.unlock();
	return v;
}

void DaPerson::RememberStoryResult(quint32 scriptId, quint8 result)
{
	m_MemoryLock.lockForWrite();
	m_Memories.RememberStoryResult(scriptId, result);
	m_MemoryLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaPerson::RememberStoryTerminated(quint32 scriptId)
{
	m_MemoryLock.lockForWrite();
	m_Memories.RememberStoryTerminated(scriptId);
	m_MemoryLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaPerson::InitNewAvatar(DaWorldLL* llworld)
{
	// This is expected to be called when they are at a world.
	if(!llworld)
		return;
	AuEther* ether = llworld->Ether();
	if(!ether)
		return;

	AuPoint loc = llworld->RandomAvatarLocation();
	AuGlobeLLF spawnLL;
	llworld->LLOfLoc(spawnLL, loc);
	AuPoint upVec = -llworld->GravityDir(loc);

	AuMatrix4x4 tBody;
	tBody.Translate(loc);
	tBody *= AuPoint(0,0,1).GetAlignmentTransform(upVec);
	tBody.Translate(0, 0, GroundHeightAboveCG()); 
	SetPosAndOrient(AuAtomPos(llworld->Id(), tBody));
	ether->AddAtom(this);

	ResetHitPoints();						// Now that I have a body...
	ResetHunger();
	ResetStamina();
	Stand();								// Clears NeedsInit state.

	g_Au.AtomAdded(Id());
	SetNormal();
	InitGearAvatar();
	SetModified();

	llworld->SetWorkerSpecies(Species());	// This is my home planet.
	if(InitScenario() == ReachForTheStars)
	{
		// Create an indigenous village near my spawn location.
		AuGlobeLLF villageLL = llworld->RandomLandLLNear(g_Au.Dice(), spawnLL, 1, 3*FeetPerMeter(), 10*FeetPerMeter());
		llworld->CreateIndigenousVillage(villageLL, Species());
		llworld->VillageSetContacted(Id(), 1);
	}
	llworld->SetModified();
}

bool DaPerson::InitNewAvatarAboardShip(DaVessel* ves)
{
	if(!ves)
		return false;
	AuEther* ether = ves->Ether();
	if(!ether)
		return false;

	AuPoint spawnLoc;
	if(!ves->TransporterVoidLocation(spawnLoc))
		return false;
	AuPoint upVec = -ves->GravityDir(spawnLoc);

	AuMatrix4x4 tBody;
	tBody.Translate(spawnLoc);
	tBody *= AuPoint(0,0,1).GetAlignmentTransform(upVec);
	tBody.Translate(0, 0, GroundHeightAboveCG()); 
	SetPosAndOrient(AuAtomPos(ves->Id(), tBody));
	ether->AddAtom(this);

	ResetHitPoints();						// Now that I have a body...
	ResetHunger();
	ResetStamina();
	Stand();								// Clears NeedsInit state.

	g_Au.AtomAdded(Id());
	SetNormal();

	ves->OccupyBerth(AuRosterEntry::Captain, Id(), Name());
	SetSpawnShip(ves->Id(), ves->Name(), AuRosterEntry::Captain, ves->IsPrivatelyOwned());
	InitGearAvatar();
	SetModified();
	return true;
}

bool DaPerson::InitNewAvatarAtCity(AuEther& ether, const AuId& bdId)
{
	const DaBd* bd = dynamic_cast<const DaBd*>(ether.FindNormalAtom(bdId));
	if(!bd)
		return false;
	const DaWorldLL* world = bd->FindParentWorldInEnv();
	if(!world)
		return false;
	const DaSolarSystem* system = world->FindParentSolarSystemInEnv();
	if(!system)
		return false;

	QString name = tr("%1 at %2 on %3")
			.arg(bd->Name())
			.arg(bd->GlobeLL().ToString())
			.arg(world->Name());
	SetSpawnCity(bdId, name, bd->GlobeLL(), system->Id(), system->Name());

	AuPoint spawnLoc = bd->RandomSpawnLoc();
	AuId spawnParent = world->Id();
	AuPoint upVec = -world->GravityDir(spawnLoc);
	upVec.Unit();

	AuMatrix4x4 tBody;
	tBody.Translate(spawnLoc);
	tBody *= AuPoint(0,0,1).GetAlignmentTransform(upVec);
	tBody.Translate(0, 0, GroundHeightAboveCG()); 
	SetPosAndOrient(AuAtomPos(spawnParent, tBody));
	ether.AddAtom(this);

	ResetHitPoints();						// Now that I have a body...
	ResetHunger();
	ResetStamina();
	Stand();								// Clears NeedsInit state.

	g_Au.AtomAdded(Id());
	SetNormal();
	InitGearAvatar();
	WillfullyChangedEmpire(bd->EmpireId());
	SetModified();
	return true;
}

bool DaPerson::InitNewAvatarAtEmperor(AuEther& ether, const AuId& emperorId)
{
	const DaPerson* emperor = dynamic_cast<const DaPerson*>(ether.FindNormalAtom(emperorId));
	if(!emperor)
		return false;
	if(emperor->IsAboardVehicle())
		return false;
	if(emperor->IsLockedPose())
		return false;

	SetPosAndOrient(emperor->Pos());
	ether.AddAtom(this);

	ResetHitPoints();						// Now that I have a body...
	ResetHunger();
	ResetStamina();
	Stand();								// Clears NeedsInit state.

	g_Au.AtomAdded(Id());
	SetNormal();
	InitGearAvatar();
	WillfullyChangedEmpire(emperor->EmpireId());
	SetModified();
	return true;
}

void DaPerson::SelectMappedGearItem(int which)
{
	Q_ASSERT(which >= 0 && which < 10);
	AuGearPos selectPos;
	INVENTORYSUB selectSub = IPS_Item;

	if(!m_MapPos[which].IsNull())
	{
		if(m_MapPos[which].IsMountBodyPos())
		{
			// Select a body weapon on my mount.
			const DaCreature* mount = dynamic_cast<const DaCreature*>(Parent());
			if(mount)
			{
				selectPos = m_MapPos[which].GearPos();
				selectSub = m_MapPos[which].UseSub();
			}
		}
		else if(m_MapPos[which].IsPersonalBodyPos())
		{
			// Select a body weapon on me.
			selectPos = m_MapPos[which].GearPos();
			selectSub = m_MapPos[which].UseSub();
		}
		else
		{
			// Select a personal gear item.
			AuGearPos mpos(m_MapPos[which].GearPos());
			if(mpos.IsGear())
			{
				const AuGear* gear = LockGearForRead();
				AuGear::const_iterator gi = gear->FindPos(mpos);
				if(gi != gear->cend())
				{
					selectPos = mpos;
					selectSub = m_MapPos[which].UseSub();

					// All items have item mode. Not all items have weapon modes.
					if(selectSub == IPS_Weapon1 || selectSub == IPS_Weapon2)
					{
						// Insure the item at that position can be used in 'selectSub' mode.
						COMMODITYID cid;
						quint8 quality;
						int ammoCur, ammoFull, damage1 = 0, damageMod1 = 0, damage2 = 0, damageMod2 = 0;
						float range, radius1, radius2;
						float ap1, st1, ap2, st2;
						WEAPONEFFECT effect1 = WE_None, effect2 = WE_None;
						WEAPONMODE mode = WM_Not;
						(*gi).Item().WeaponInfo(selectSub, cid, quality, mode, range, ammoCur, ammoFull,
								effect1, damage1, damageMod1, radius1, ap1, st1,
								effect2, damage2, damageMod2, radius2, ap2, st2);
						if(mode == WM_Not)
							selectSub = IPS_Item;		// The item does not have that weapon mode.
					}
				}
				UnlockGear();
			}
		}
	}

	m_PersonLock.lockForWrite();
	m_CurrentGearPos = selectPos;
	m_CurrentGearSub = selectSub;
	m_tmItemUseChange.Set();
	m_PersonLock.unlock();
	SetModified();
}

void DaPerson::MapCurrentGearItem(int which)
{
	QString gearName;
	INVENTORYPOS gearIP = IP_NumInventoryPos;
	AuGearSlot::SLOTTYPE slotType = AuGearSlot::Invalid;
	quint8 slotWhich = 0;
	INVENTORYSUB useSub = IPS_Item;

	AuGearPos g = CurrentItemPos();
	if(g.IsBody())
	{
		gearIP = g.BodyPos();
		useSub = m_CurrentGearSub;

		if(useSub == IPS_Item)
			gearName = g_Au.CommodityName((COMMODITYID)gearIP);
		else
			gearName = tr("%1 special weapon").arg(g_Au.CommodityName((COMMODITYID)gearIP));
	}
	else if(g.IsGear() && g.IsTopLevel())
	{
		// This person has a "current" top-level item selected.
		m_GearLock.lockForRead();
		if(m_Gear.IsPosValid(g))
		{
			const AuItem* item = m_Gear.ItemAt(g);
			if(item)
			{
				useSub = m_CurrentGearSub;

				slotType = g.Slot();
				slotWhich = g.Which();
			}
		}
		m_GearLock.unlock();
	}

	m_PersonLock.lockForWrite();
	m_MapPos[which] = AuHotKeyGear(gearIP, useSub, slotType, slotWhich);
	m_PersonLock.unlock();

	SetModified();

	QString text;
	if(gearIP != IP_NumInventoryPos)
		text = tr("Position %1 mapped to %2.").arg(which+1).arg(gearName);	// Body part
	else if(slotType == AuGearSlot::Invalid)
		text = tr("Cleared item mapping from position %1.").arg(which+1);
	else
	{
		// Determine the maximum number of items that can be carried in the slot type.
		int slotMaxItems = 0;
		switch(slotType)
		{
		case AuGearSlot::Invalid:
		case AuGearSlot::Type_Count:
			break;	// No slot was found to carry the item.

		case AuGearSlot::Hand:
			slotMaxItems = 2;
			break;

		case AuGearSlot::Head:
		case AuGearSlot::Face:
		case AuGearSlot::Neck:
		case AuGearSlot::Body:
			slotMaxItems = 1;
			break;

		case AuGearSlot::BodyWear:
			slotMaxItems = 2;
			break;

		case AuGearSlot::Limb:
		case AuGearSlot::Digit:
		case AuGearSlot::Waist:
			slotMaxItems = 4;
			break;
		}

		if(slotMaxItems > 1)
			text = tr("Position %1 mapped to %2 %3, mode %4.")
				.arg(which+1)
				.arg(AuGearSlot::SlotTypeName(slotType))
				.arg(slotWhich+1)
				.arg(useSub+1);
		else
			text = tr("Position %1 mapped to %2, mode %3.")
				.arg(which+1)
				.arg(AuGearSlot::SlotTypeName(slotType))
				.arg(useSub+1);
	}
	Thought(text);
}

void DaPerson::KilledByUnit(const DaUnit* killer)
{
	if(!killer->EmpireId())
		return;							// Killer is probably a wild animal.

	if(m_AgentBits&(AGENT_Animal|AGENT_Entity|AGENT_Invisible))
		return;

	POLITICAL_STANCE victimStance = PoliticalStanceMakeContact(killer);
	Stats().KilledByUnit(killer, victimStance, DamagedUsingCID(killer->Id()));

	if(InArena())
		return;

	if(victimStance == POL_Empire)
		return;
	// Victim is from a different empire as the killer.

	// Victim's empire logs this as a diplomatic incident.
	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
	if(!system)
		return;	// No parent solar system?

	GALAXY galaxy = system->Galaxy();
	const DaSector* sector = system->FindParentSector();
	const DaWorld* world = FindParentPlanetInEnv();
	QString sectorName = sector ? sector->Name() : "";
	QString systemName = system ? system->Name() : "";
	QString worldName = world ? world->Name() : "";

	AuDocAvatarKilled doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
	emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
}

bool DaPerson::KeepsAutomaticDoorsOpen() const
{
	if(IsEthereal())
		return false;
	return DaSentient::KeepsAutomaticDoorsOpen();
}

bool DaPerson::OpensAutomaticDoors() const
{
	if(IsEthereal())
		return false;
	return DaSentient::OpensAutomaticDoors();
}

void DaPerson::EatMealFromGear()
{
	unsigned short int curHunger = CurrentHunger();
	unsigned short int maxHunger = MaxHunger();
	if(curHunger >= maxHunger)
		return;							// I am not hungry at all.

	unsigned short int eatThreshold = maxHunger/10;
	if(!eatThreshold)
		eatThreshold = 1;
	if(curHunger > eatThreshold)
		return;							// I am not hungry enough to eat a meal.

	// Time to eat a meal. Consume as much food as I can until my tummy is full.
	while(EatFoodFromGear() && CurrentHunger() < maxHunger) {};
}

bool DaPerson::EatFoodFromGear()
{
	// Locate food edible to me in my gear. Only top level items and containers are checked. It does not go deeper into containers.
	m_GearLock.lockForRead();
	AuGearPos pos = m_Gear.FindFood(EcoRole());
	bool posValid = m_Gear.IsPosValid(pos);
	m_GearLock.unlock();
	if(!posValid)
		return false;
	// Eat the food.
	UseGearAt(pos);
	return true;
}

void DaPerson::SetAutoEat(bool autoEat)
{
	m_AutoEat = autoEat;
	SetHungerChanged();
}

void DaPerson::ListPatents(AuKnownPatents& patents) const
{
	LockGearForRead()->ListPatents(patents);
	UnlockGear();
}

void DaPerson::DeleteStoredData(const AuGearPos& srcPos, const QSet<COMMODITYID>& patentCIDs, const QSet<quint32>& scDesigns,
		const QSet<quint32>& bdDesigns, const QSet<QString>& missions, const QSet<QString>& papers)
{
	m_GearLock.lockForWrite();
	bool deleted = false;
	if(m_Gear.DeleteStoredBlueprintsBd(srcPos, bdDesigns))
		deleted = true;
	if(m_Gear.DeleteStoredBlueprintsSc(srcPos, scDesigns))
		deleted = true;
	if(m_Gear.DeleteStoredMissions(srcPos, missions))
		deleted = true;
	if(m_Gear.DeleteStoredPapers(srcPos, papers))
		deleted = true;
	if(m_Gear.DeleteStoredPatents(srcPos, patentCIDs))
		deleted = true;
	if(deleted)
		UpdateGearWeight();	// This could cause me to start falling if I just took off my EVA suit.
	m_GearLock.unlock();
}

DaBd* DaPerson::BdBuild(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
		const AuId& platformId, quint16 mpid, bool workerOk)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return 0;
	DaBd* platform;
	if(!platformId.IsNull())
		platform = world->FindBd(platformId);
	else
		platform = 0;

	quint32 idEmpire = EmpireId();
	if(!idEmpire)
	{
		// Some buildings cannot be made if I do not have an empire.
		switch(industryId)
		{
		case IID_Bank:
		case IID_Capitol:
		case IID_PoliceDepartment:
		case IID_BaseAcademy:
		case IID_BaseAirTerminal:
		case IID_BaseBarracks:
		case IID_BaseDock:
		case IID_BaseGuardPost:
		case IID_BaseGuardTower:
		case IID_BaseHeadquarters:
		case IID_BaseMarineBarracks:
		case IID_BaseMotorPool:
		case IID_BaseRadar:
		case IID_BaseShield:
		case IID_BaseWeapon:
			return 0;					// Cannot build without an empire.
		default:
			break;						// Ok to build without an empire.
		}
	}

	AuBdDesign design;
	const DgData& bdd = g_Au.LockBdDesignsForRead();
	bool hasDesign = bdd.BdDesign(designId, design);
	g_Au.UnlockBdDesigns();
	if(!hasDesign)
	{
		Thought(tr("Blueprint %1 is not loaded; the server is loading it. "
					"I should try again in a moment.").arg(designId));
		return 0;
	}

	AuBdReport report;
	design.ReportDesign(report);
	report.SetConstructionMaterial(material);
	DaBd* exDev;
	QString cityName;
	int testResult = world->TestBdLocation(industryId, idEmpire, &report, material, buildLoc, 0, zRotation, platform, exDev, 0,
			cityName);
	if(testResult != 0)
	{
		Thought(DaWorldLL::TestBdResultText(testResult));
		return 0;
	}

	quint32 sitePrepLabor = 10;

	AuMatrix4x4 mBld;
	world->BuildingConstructionTransform(buildLoc, zRotation, platform, report.SiteCenter(), mBld);

	DaBd* building = new DaBd;
	building->ConnectSignals();
	building->Create();
	building->SetPosAndOrient(AuAtomPos(world->Id(), mBld));
	ether->AddAtom(building);

	building->StartConstruction(industryId, report, sitePrepLabor, mpid, workerOk, Species());
	building->SetEmpireId(idEmpire);
	if(industryId == IID_Capitol || industryId == IID_BaseHeadquarters)
		building->SetFounder(Id(), Name());
	if(platform)
		building->SetPlatformId(platformId);	// Buildings know what platform they were built on, so they can be detroyed with it.
	g_Au.AtomAdded(building->Id());
	building->SetNormal();				// New atom is ready for normal thread processing.

	world->BdAdded();					// Update city presence. Adjust fences.
	if(industryId == IID_PoliceDepartment || industryId == IID_Capitol)
		world->JurisdictionPoliceJoin(building);				// This could change city jurisdictions and claim buildings.
	else
		world->JurisdictionJoin(building);						// Join a city if the nu building falls in its jurisdiction.

	// Military headquarters captures all cities on the world.
	if(industryId == IID_BaseHeadquarters)
		world->Capture(idEmpire, Id(), Name());

	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(world->FindEnvironmentAtom());
	if(system)
	{
		system->UpdateTradeConnections();						// So we can import construction materials.
		if(SpawnCity().IsNull())
		{
			// The person has no city spawn point so automatically tie to the building they just created.
			QString name = tr("%1 at %2 on %3")
					.arg(building->Name())
					.arg(building->GlobeLL().ToString())
					.arg(world->Name());
			SetSpawnCity(building->Id(), name, building->GlobeLL(), system->Id(), system->Name());
			SetModified();
		}
	}

	// Player Statistics
	Stats().BuildingPlanned(industryId, world, 1);
	IncreaseSkill(AuSkill::CityPlanner, 1);
	building->TickTock();
	return building;
}

DaBd* DaPerson::BdBuildStorage(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc,
		float zRotation, const AuId& platformId, quint16 mpid, bool workerOk, COMMODITYID storeCID)
{
	DaBd* bd = BdBuild(industryId, designId, material, buildLoc, zRotation, platformId, mpid, workerOk);
	if(bd)
		bd->SetStoreCommodity(storeCID);
	return bd;
}

DaBd* DaPerson::BdPreview(const AuPoint& buildLoc, float zRotation)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return 0;
	DaDesigner* de = FindParentDesignerInEnv();
	if(!de)
		return 0;

	AuBdReport report;
	const AuBdDesign* design = dynamic_cast<const AuBdDesign*>(de->LockDesignForRead());
	if(design)
		design->ReportDesign(report);
	de->UnlockDesign();

	QString designName(report.DesignName());
	if(designName.isEmpty())
		designName = tr("Unnamed Design");

	DaBd* exDev;
	int testResult = world->TestBdPreviewLocation(&report, buildLoc, zRotation, exDev);
	if(testResult != 0)
	{
		Thought(DaWorldLL::TestBdResultText(testResult));
		return 0;
	}

	AuMatrix4x4 mBld;
	world->BuildingConstructionTransform(buildLoc, zRotation, 0, report.SiteCenter(), mBld);

	DaBd* building = new DaBd;
	building->ConnectSignals();
	building->Create();
	building->InitPreview(report, designName, EmpireId(), Species());
	building->SetPosAndOrient(AuAtomPos(world->Id(), mBld));
	ether->AddAtom(building);
	g_Au.AtomAdded(building->Id());
	building->SetNormal();				// New atom is ready for normal thread processing.
	world->BdAdded();					// Update city presence. Adjust fences.

	// Update world cache when a Enclosure is completed.
	if(building->HasEnclosedEnvironment())
	{
		world->InvalidateEnclosureCache();
		world->InvalidateEnvironmentRecursive();
	}
	building->TickTock();
	return building;
}

DaBd* DaPerson::BdOp(const AuId& opBd)
{
	if(!IsAtBd() && !IsInBd())
		return 0;						// I am not at or in any buidling.
	DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return 0;
	if(!opBd.IsNull() && EmpireId())
	{
		if(InIndustry() != IID_Capitol && InIndustry() != IID_BaseHeadquarters)
			return 0;	// wut?
		return dynamic_cast<DaBd*>(world->FindIdRecursive(opBd));	// I am at a capitol building. Find the building to operate.
	}
	return dynamic_cast<DaBd*>(world->FindIdRecursive(BdId()));	// Find the building I am at.
}

void DaPerson::BdConstructionEnable(bool enable, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(bd)
		bd->EnableConstructionWorker(enable);
	else
	{
		DaWorldLL* world = FindParentWorldInEnv();
		if(!world)
			return;
		world->GeoConstructionEnable(InIndustryLL(), enable);
	}
}

void DaPerson::BdConstructionFetch(const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(bd)
		bd->ConstructionFetch();
	else if(IsAtGeo())
	{
		DaWorldLL* world = FindParentWorldInEnv();
		if(!world)
			return;
		world->GeoConstructionFetch(InIndustryLL());
	}
}

void DaPerson::BdConstructionGo(const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(bd)
		bd->ConstructionGo(this);
	else
	{
		DaWorldLL* world = FindParentWorldInEnv();
		if(!world)
			return;
		AuGear* gear = LockGearForWrite();
		bool gearChanged = world->GeoConstructionGo(InIndustryLL(), *gear);
		if(gearChanged)
			UpdateGearWeight();
		UnlockGear();
	}
}

void DaPerson::BdManufactureSpacecraftEnableWorker(quint32 enabled, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to employ workers."));
		return;
	}
	bd->EnableSpacecraftFactoryWorker(enabled);
}

void DaPerson::BdManufactureEnabledMPID(quint16 mpid, quint32 enabled, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change the number of workshops assigned."));
		return;
	}
	bd->EnableManufacturingWorkerMPID(mpid, enabled);
}

void DaPerson::BdManufactureFetch(quint16 panelIndex, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	bd->ManufactureFetch(panelIndex);
}

void DaPerson::BdManufactureFetchMPID(quint16 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	bd->ManufactureFetchMPID(mpid);
}

void DaPerson::BdManufactureGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos,
		const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	quint32 empireId = EmpireId();
	if(!empireId)
		return;
	if(patentCIDs.empty())
		return;
	if(!maxQ)
		return;
	AuKnownPatents patents = bd->LockPatentsForRead();
	bd->UnlockPatents();

	QString resultMsg;
	AuGear* pinv = LockGearForWrite();
	AuItem* invmedia = pinv->ItemAt(mediaPos);
	if(invmedia)
	{
		// Person had blank media, either a disk or paper.
		AuItemStorageMedia* storage = dynamic_cast<AuItemStorageMedia*>(invmedia);
		if(storage)
		{
			// Copy all selected patents to the storage media.
			int stored = 0;
			for(QSet<COMMODITYID>::const_iterator pi = patentCIDs.cbegin(); pi != patentCIDs.cend(); pi++)
			{
				quint8 patentQ = patents.PatentQuality(*pi);
				if(!patentQ)
					continue;	// City doesn't have the patent. wtf?
				if(patentQ > maxQ)
					patentQ = maxQ;
				storage->StorePatent(*pi, patentQ);
				stored++;
			}
			resultMsg = tr("%1 patents were stored to the %2.").arg(stored).arg(storage->SayName());
			UpdateGearWeight();
			UnlockGear();
		}
		else
		{
			// Copy one selected patent to the blank media. That is all it will hold.
			COMMODITYID patentCID = *patentCIDs.cbegin();
			quint8 patentQ = patents.PatentQuality(patentCID);
			if(patentQ)
			{
				if(patentQ > maxQ)
					patentQ = maxQ;
				AuItem* media = invmedia->Clone();
				UnlockGear();
				if(media)
				{
					// Use up the media to get a copy of the patent.
					media->SetCount(1);
					ConsumeItem(*media);
					if(media->UseItem())					// Quality check the result.
					{
						// Replace the blank media with a patent.
						AuItem* patentItem;
						if(media->CommodityId() == CID_BlankDisk)
							patentItem = new AuItem(patentCID, patentQ, bd->EmpireId());
						else
							patentItem = new AuItem(patentCID, patentQ, bd->EmpireId());

						// Put it wherever there's room.
						AddGearItem(*patentItem);
						resultMsg = tr("I got %1.").arg(patentItem->SayName());
						delete patentItem;
					}
					else
						resultMsg = tr("The %1 was defective; it failed while trying to get the patent. "
								"I should try again with another blank disk or paper.").arg(media->SayName());
					delete media;
				}
			}
			else
				UnlockGear();
		}
	}
	else
	{
		UnlockGear();
		resultMsg = tr("I need storage media to get a copy of the patent.");
	}
	if(!resultMsg.isEmpty())
		Thought(resultMsg);
}

void DaPerson::BdManufactureGo(quint16 panelIndex, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	bd->ManufactureGo(panelIndex, this);
}

void DaPerson::BdManufactureGoMPID(quint16 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	bd->ManufactureGoMPID(mpid, this);
}

void DaPerson::BdManufactureLowerMPID(quint16 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change production line priority."));
		return;
	}
	bd->ManufactureLowerMPID(mpid);
}

void DaPerson::BdManufactureRaiseMPID(quint16 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change production line priority."));
		return;
	}
	bd->ManufactureRaiseMPID(mpid);
}

void DaPerson::BdManufactureMaxMPID(quint16 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change production line priority."));
		return;
	}
	bd->ManufactureMaxMPID(mpid);
}

void DaPerson::BdManufactureMinMPID(quint16 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change production line priority."));
		return;
	}
	bd->ManufactureMinMPID(mpid);
}

void DaPerson::BdManufactureAddMPID(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to add a production line."));
		return;
	}
	bd->ManufactureAddMPID(mpid, minQ, workshops, stockLimit);
}

void DaPerson::BdManufactureClear(const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to clear production lines."));
		return;
	}
	bd->ManufactureClear();
}

void DaPerson::BdManufactureDeleteMPID(quint16 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to remove a production line."));
		return;
	}
	bd->ManufactureDeleteMPID(mpid);
}

void DaPerson::BdManufactureProcessSelected(quint16 panelIndex, qint32 mpid, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change a production line."));
		return;
	}
	bd->ManufactureProcessSelected(panelIndex, mpid);
}

void DaPerson::BdManufactureSetMinQ(quint8 minQ, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change production quality."));
		return;
	}
	bd->ManufactureSetMinQ(minQ);
}

void DaPerson::BdManufactureProcessSetMinQMPID(quint16 mpid, quint8 minQ, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change production quality."));
		return;
	}
	bd->ManufactureProcessSetMinQMPID(mpid, minQ);
}

void DaPerson::BdManufactureProcessSetStockLimitMPID(quint16 mpid, quint32 stockLimit, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change stock limit."));
		return;
	}
	bd->ManufactureProcessSetStockLimitMPID(mpid, stockLimit);
}

void DaPerson::BdManufactureRecordPatents(const AuKnownPatents& patents, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	QString resultMsg;
	if(patents.size() == 1)
	{
		// Record the patent.
		QHash<COMMODITYID, quint8>::const_iterator pi = patents.cbegin();
		if(bd->RecordPatents(patents))
			resultMsg = tr("The patent office in %1 recorded the %2.")
				.arg(bd->CityOrBuildingName())
				.arg(g_Au.CommodityName(pi.key()));
		else
			resultMsg = tr("The %2 is already recorded at the patent office in %1.")
				.arg(bd->CityOrBuildingName())
				.arg(g_Au.CommodityName(pi.key()));
	}
	else
	{
		// Record the patents.
		int recorded = bd->RecordPatents(patents);
		if(recorded == patents.size())
			resultMsg = tr("The patent office in %1 recorded %2 new patents.")
				.arg(bd->CityOrBuildingName())
				.arg(recorded);
		else if(recorded)
			resultMsg = tr("The patent office in %1 recorded %2 new patents. "
					"The other %3 were already on record.")
				.arg(bd->CityOrBuildingName())
				.arg(recorded)
				.arg(patents.size() - recorded);
		else
			resultMsg = tr("All of those patents are already recorded at the patent office in %1.")
				.arg(bd->CityOrBuildingName());
	}
	Thought(resultMsg);
}

void DaPerson::BdManufactureSetLivestock(const AuSpecies& species, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change livestock species."));
		return;
	}
	bd->SetLivestockSpecies(species);
}

void DaPerson::BdManufactureSetWarehouseStorage(COMMODITYID storeCID, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change storage."));
		return;
	}
	bd->SetStoreCommodity(storeCID);
}

bool DaPerson::ClaimWorld(DaWorldLL* llworld, const AuPoint& flagLoc)
{
	quint32 empireId = EmpireId();
	if(!empireId)
		return false;	// I cannot claim a world if I have no empire.
	AuGlobeLLF flagLL;
	if(!llworld->LLOfLoc(flagLL, flagLoc))
		return false;	// Somebody is trying to be funny.
	const DaSolarSystem* system = llworld->FindParentSolarSystem();
	if(!system)
		return false;
	const DaSector* sector = llworld->FindParentSector();
	if(!sector)
		return false;
	const DaGalaxy* galaxy = llworld->FindParentGalaxy();
	if(!galaxy)
		return false;
	GALAXY galaxyEnum = galaxy->Galaxy();
	QString sectorName = sector->Name();
	QString systemName = system->Name();
	QString worldName = llworld->Name();

	QString myEmpireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(empireId, myEmpireName);
	g_Au.UnlockData();

	// Insure it is ok to claim the world.
	quint32 claimedBy = llworld->ClaimedByEmpire();
	if(claimedBy)
	{
		// World is already claimed. What is my stance toward the current empire who claims the world.
		bool okToClaim = false;
		POLITICAL_STANCE stance = PoliticalStance(empireId, claimedBy);
		switch(stance)
		{
		case POL_Empire:
			{
				QString claimTime = llworld->ClaimTime().UTC().toString();
				QString claimLL = llworld->ClaimedAtLL().ToString();
				QString claimant = QString("%1 %2")
					.arg(llworld->ClaimedByPersonName())
					.arg(llworld->ClaimedByPersonId().StringId());
				AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimByEmpire, myEmpireName, claimTime, claimLL, claimant,
						Id(), Name());
				g_Au.SendAtomChat(msg);
			}
			break;
		case POL_Friend:
			{
				QString claimTime = llworld->ClaimTime().UTC().toString();
				QString claimLL = llworld->ClaimedAtLL().ToString();
				QString claimant = QString("%1 %2")
					.arg(llworld->ClaimedByPersonName())
					.arg(llworld->ClaimedByPersonId().StringId());
				QString empireName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(claimedBy, empireName);
				g_Au.UnlockData();
				AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimByFriend, empireName, claimTime, claimLL, claimant,
						Id(), Name());
				g_Au.SendAtomChat(msg);
			}
			break;
		case POL_Vassal:	// I am a vassal to them.
			{
				QString claimTime = llworld->ClaimTime().UTC().toString();
				QString claimLL = llworld->ClaimedAtLL().ToString();
				QString claimant = QString("%1 %2")
					.arg(llworld->ClaimedByPersonName())
					.arg(llworld->ClaimedByPersonId().StringId());
				QString empireName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(claimedBy, empireName);
				g_Au.UnlockData();
				AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimByVassal, empireName, claimTime, claimLL, claimant,
						Id(), Name());
				g_Au.SendAtomChat(msg);
			}
			break;
		case POL_Neutral:
			{
				QString empireName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(claimedBy, empireName);
				g_Au.UnlockData();
				if(LastClaimTime().ElapsedSec() > 60)
				{
					// This message tells them to try again to claim it anyway.
					QString claimTime = llworld->ClaimTime().UTC().toString();
					QString claimLL = llworld->ClaimedAtLL().ToString();
					QString claimant = QString("%1 %2")
						.arg(llworld->ClaimedByPersonName())
						.arg(llworld->ClaimedByPersonId().StringId());
					AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimByNeutral, empireName, claimTime, claimLL, claimant,
							Id(), Name());
					g_Au.SendAtomChat(msg);
					LastClaimTimeSet();	// This person now has 60 seconds to try again.
				}
				else
				{
					// They did it again within 60 seconds.
					if(llworld->HasAlienMilitaryHQ(empireId))
					{
						AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimAlienHQ, empireName, Id(), Name());
						g_Au.SendAtomChat(msg);
					}
					else
					{
						// No hq means I can steal the claim.
						QString claimTime = llworld->ClaimTime().UTC().toString();
						QString claimant = QString("%1 %2")
							.arg(llworld->ClaimedByPersonName())
							.arg(llworld->ClaimedByPersonId().StringId());
						AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimLostNeutral, empireName, claimTime, claimant,
								Id(), Name());
						g_Au.SendAtomChat(msg);
						okToClaim = true;
					}
				}
			}
			break;
		case POL_Enemy:
			{
				QString empireName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(claimedBy, empireName);
				g_Au.UnlockData();
				if(llworld->HasAlienMilitaryHQ(empireId))
				{
					AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimAlienHQ, empireName, Id(), Name());
					g_Au.SendAtomChat(msg);
				}
				else
				{
					// No hq means I can steal the claim.
					QString claimTime = llworld->ClaimTime().UTC().toString();
					QString claimant = QString("%1 %2")
						.arg(llworld->ClaimedByPersonName())
						.arg(llworld->ClaimedByPersonId().StringId());
					AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimLostEnemy, empireName, claimTime, claimant,
							Id(), Name());
					g_Au.SendAtomChat(msg);
					okToClaim = true;
				}
			}
			break;
		}
		if(!okToClaim)
			return false;

		// Record a diplomatic incident over this.
		AuDocClaimHijacked doc(Id(), Name(), galaxyEnum, sectorName, systemName, worldName);
		emit GovDossierAddDocument(claimedBy, empireId, doc.Pack());
	}
	else
	{
		// World is not currently claimed.
		AuCommThoughtMessage msg(AuCommThoughtMessage::WorldUnclaimed, Id(), Name());
		g_Au.SendAtomChat(msg);
	}

	// Make a sound.
	AuPoint soundLoc = Transform().UnProject(flagLoc);			// Flag location relative to me.
	AuEffect effect(soundLoc, AuPoint(0,0), AuPoint(0,0), SOUND_WorldClaimed, 1,1, VISUAL_None);
	AddEffect(effect);

	// I used up the flag. Claim the world.
	bool personHasIndDNA = llworld->SetClaimedBy(*this, flagLL);
	if(llworld->IsHabitable(HABITABLE_System))
	{
		// The world is habitable.
		if(personHasIndDNA)
		{
			// My DNA matches the indigenous people of the world. My claim may have just established the indigenous DNA. I don't
			// really know if it was a virgin world because indigenous encounters can set the indigenous DNA. If I wait around long
			// enough, there will be an indigenous encounter, whether I claim the world or not.
			AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimedHabitableSameDNA, myEmpireName, Id(), Name());
			g_Au.SendAtomChat(msg);
			Stats().ClaimedHabitableSameDNA();
			IncreaseSkill(AuSkill::SpaceExploration, 1);
		}
		else
		{
			// My DNA does not match the indigenous people of the world.
			AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimedHabitableDiffDNA, myEmpireName, Id(), Name());
			g_Au.SendAtomChat(msg);
			Stats().ClaimedHabitableDifferentDNA();
			IncreaseSkill(AuSkill::SpaceExploration, 1);
		}
	}
	else
	{
		// World is uninhabitable.
		AuCommThoughtMessage msg(AuCommThoughtMessage::WorldClaimedUninhabitable, myEmpireName, Id(), Name());
		g_Au.SendAtomChat(msg);
		Stats().ClaimedUninhabitable();
		IncreaseSkill(AuSkill::SpaceExploration, 1);
	}

	// Remember the worlds that I claim right away.
	Remember(AuMemClaim(llworld->ClaimedAtLL(), llworld->Id(), llworld->Name(), system->Id(), systemName, sector->Id(), sectorName,
				galaxyEnum));
	return true;
}

void DaPerson::QuitClaimWorld()
{
	DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(Parent());
	if(!llworld)
		return;

	QString myEmpireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(EmpireId(), myEmpireName);
	g_Au.UnlockData();

	quint32 claimedBy = llworld->ClaimedByEmpire();
	if(EmpireId() != claimedBy)
	{
		// The world is not claimed by my empire.
		AuCommThoughtMessage msg(AuCommThoughtMessage::QuitClaimNoClaim, myEmpireName, Id(), Name());
		g_Au.SendAtomChat(msg);
		return;
	}
	else if(llworld->HasAnyMilitaryHQ())
	{
		// Claim is required for hq. Any hq prevents quit.
		AuCommThoughtMessage msg(AuCommThoughtMessage::QuitClaimHQ, Id(), Name());
		g_Au.SendAtomChat(msg);
		return;

	}
	else if(!Is(llworld->ClaimedByPersonId()) || llworld->HasAnyCapitol())
	{
		// I did not claim this world originally or a capitol has been built.
		// The world must be in my chain of command to quit the claim now.
		if(!llworld->InChainOfCommand(Id()))
		{
			AuCommThoughtMessage msg(AuCommThoughtMessage::QuitClaimCoC, Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}
	}

	// Quit claim the world.
	llworld->SetClaimQuit(Id(), Name());
	AuCommThoughtMessage msg(AuCommThoughtMessage::QuitClaim, myEmpireName, Id(), Name());
	g_Au.SendAtomChat(msg);
}

void DaPerson::BdStoreBuyItem(const AuItem& buyItem, const AuItem& offerItem, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(offerItem.CommodityId() == CID_Money)
	{
		// I offered cash.
		if(offerItem.Count() > AccountBalance())
		{
			QString offerAmt = QString("%L1%2").arg(offerItem.Count()).arg(g_Au.CurrencySymbol());
			AuCommThoughtMessage msg(AuCommThoughtMessage::InsufficientFunds, buyItem.SayName(), offerAmt, Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}

		// I have the money. My offer is good.
		// Check selling price.
		double salePrice = buyItem.TotalCost() * .8;
		if(offerItem.Count() < salePrice)
		{
			// Tell the buyer why.
			QString amt = QString("%L1%2").arg(salePrice, 0,'f',0).arg(g_Au.CurrencySymbol());
			AuCommThoughtMessage msg(AuCommThoughtMessage::ItemNoDeal, buyItem.SayName(), amt, Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}

		// Take the item from the building.
		int buyItemCount = buyItem.Count();
		if(!bd->RemoveCargo(buyItem, buyItemCount))
		{
			AuCommThoughtMessage msg(AuCommThoughtMessage::BuyItemGone, buyItem.SayName(), Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}

		// The item was removed from the building.
		// Take my money.
		DebitAccount(offerItem.Count());

		// Add the item(s) to my gear one by one. Items that don't fit will fall to the ground.
		int dropped = 0;
		AuItem* boughtItem = buyItem.Clone();
		boughtItem->SetCount(1);
		for(int ic=0; ic<buyItemCount; ic++)
		{
			if(AddGearItem(*boughtItem))
			{
				// The item fit into my gear.
			}
			else
			{
				// The item did not fit into my gear; drop it.
				DropItem(*boughtItem);
				dropped++;
			}
		}
		delete boughtItem;

		// Think the result.
		QString offerAmt = QString("%L1%2").arg(offerItem.Count()).arg(g_Au.CurrencySymbol());
		QString amt = QString("%L1").arg(buyItemCount);
		if(dropped)
		{
			QString dropAmt = QString("%L1").arg(dropped);
			AuCommThoughtMessage msg(AuCommThoughtMessage::ItemPurchasedDropped, buyItem.SayName(), amt, offerAmt, dropAmt, Id(),
					Name());
			g_Au.SendAtomChat(msg);
		}
		else
		{
			AuCommThoughtMessage msg(AuCommThoughtMessage::ItemPurchased, buyItem.SayName(), amt, offerAmt, Id(), Name());
			g_Au.SendAtomChat(msg);
		}
	}
	else
	{
		// I offered a trade.
		unsigned int offerItems = LockGearForRead()->CountAvailableItems(offerItem);
		UnlockGear();
		if(offerItems < offerItem.Count())
		{
			QString offerAmt = QString("%L1").arg(offerItem.Count());
			AuCommThoughtMessage msg(AuCommThoughtMessage::OfferItemGone, offerItem.SayName(), offerAmt, Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}

		// I have the goods. My offer is good.
		// Check selling price.
		double offerprice = offerItem.TotalCost();
		double salePrice = buyItem.TotalCost() * .8;
		if(offerprice < salePrice)
		{
			// Tell the buyer why.
			QString amt = QString("%L1%2").arg(salePrice, 0,'f',0).arg(g_Au.CurrencySymbol());
			AuCommThoughtMessage msg(AuCommThoughtMessage::ItemNoDeal, buyItem.SayName(), amt, Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}

		// Take the item from the building.
		int buyItemCount = buyItem.Count();
		if(!bd->RemoveCargo(buyItem, buyItemCount))
		{
			AuCommThoughtMessage msg(AuCommThoughtMessage::BuyItemGone, buyItem.SayName(), Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}

		// The item was removed from the building.
		// Take my offered items.
		LockGearForWrite()->RemoveItems(offerItem);
		UnlockGear();

		// Add the item(s) to my gear one by one. Items that don't fit will fall to the ground.
		int dropped = 0;
		AuItem* boughtItem = buyItem.Clone();
		boughtItem->SetCount(1);
		for(int ic=0; ic<buyItemCount; ic++)
		{
			if(AddGearItem(*boughtItem))
			{
				// The item fit into my gear.
			}
			else
			{
				// The item did not fit into my gear; drop it.
				DropItem(*boughtItem);
				dropped++;
			}
		}
		delete boughtItem;

		// Think the result.
		QString buyAmt = QString("%L1").arg(buyItemCount);
		QString offerAmt = QString("%L1").arg(offerItem.Count());
		if(dropped)
		{
			QString dropAmt = QString("%L1").arg(dropped);
			AuCommThoughtMessage msg(AuCommThoughtMessage::ItemTradedDropped, offerItem.SayName(), offerAmt, buyItem.SayName(),
					buyAmt, dropAmt, Id(), Name());
			g_Au.SendAtomChat(msg);
		}
		else
		{
			AuCommThoughtMessage msg(AuCommThoughtMessage::ItemTraded, offerItem.SayName(), offerAmt, buyItem.SayName(), buyAmt,
					Id(), Name());
			g_Au.SendAtomChat(msg);
		}
	}
}

void DaPerson::BdTakeItem(const AuItem& takeItem, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
		return;

	// Insure the amount to remove is sane.
	int takeItemCount = takeItem.Count();
	if(takeItemCount > 100)
		takeItemCount = 100;

	// Take the item from the building.
	if(!bd->RemoveCargo(takeItem, takeItemCount))
	{
		AuCommThoughtMessage msg(AuCommThoughtMessage::BuyItemGone, takeItem.SayName(), Id(), Name());
		g_Au.SendAtomChat(msg);
		return;
	}

	// Add the item(s) to my gear one by one. Items that don't fit will fall to the ground.
	int dropped = 0;
	AuItem* takenItem = takeItem.Clone();
	takenItem->SetCount(1);
	for(int ic=0; ic<takeItemCount; ic++)
	{
		if(AddGearItem(*takenItem))
		{
			// The item fit into my gear.
		}
		else
		{
			// The item did not fit into my gear; drop it.
			DropItem(*takenItem);
			dropped++;
		}
	}
	delete takenItem;

	// Think the result.
	QString amt = QString("%L1").arg(takeItemCount);
	if(dropped)
	{
		QString dropAmt = QString("%L1").arg(dropped);
		AuCommThoughtMessage msg(AuCommThoughtMessage::ItemTakenDropped, takeItem.SayName(), amt, dropAmt, Id(),
				Name());
		g_Au.SendAtomChat(msg);
	}
	else
	{
		AuCommThoughtMessage msg(AuCommThoughtMessage::ItemTaken, takeItem.SayName(), amt, Id(), Name());
		g_Au.SendAtomChat(msg);
	}
}

void DaPerson::BdDiscardItem(const AuItem& takeItem, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
		return;

	// Insure the amount to remove is sane.
	int takeItemCount = takeItem.Count();
	if(takeItemCount <= 0)
		return;

	// Take the item from the building.
	if(!bd->RemoveCargo(takeItem, takeItemCount))
	{
		AuCommThoughtMessage msg(AuCommThoughtMessage::BuyItemGone, takeItem.SayName(), Id(), Name());
		g_Au.SendAtomChat(msg);
		return;
	}

	// Think the result.
	QString amt = QString("%L1").arg(takeItemCount);
	AuCommThoughtMessage msg(AuCommThoughtMessage::ItemDiscarded, takeItem.SayName(), amt, Id(), Name());
	g_Au.SendAtomChat(msg);
}

void DaPerson::GeoRoad(const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, float width, quint8 geoType)
{
	DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return;
	COMMODITYID material;
	if(geoType == AuGeoRoadAsphalt::GeoType())
		material = CID_CrudeOil;
	else
		material = CID_Stone;
	DaBd* exDev;
	QString cityName;
	int testResult = world->TestBdLocation(IID_Road, EmpireId(), 0, material, startLoc, width, 0, 0, exDev, &endLoc, cityName);
	if(testResult != 0)
	{
		Thought(DaWorldLL::TestBdResultText(testResult));
		return;
	}
	world->AddRoad(startLoc, endLoc, isSlab, width, geoType, false);
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(world->FindEnvironmentAtom());
	if(system)
		system->UpdateTradeConnections();						// So we can import construction materials.
}

void DaPerson::GeoLandscape(const AuPoint& startLoc, const AuPoint& endLoc, float width, bool hideRocks, int hideGrass,
		bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement)
{
	DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return;
	DaBd* exDev;
	QString cityName;
	int testResult = world->TestBdLocation(IID_Landscape, EmpireId(), 0, CID_Stone, startLoc, width, 0, 0, exDev, &endLoc,
			cityName);
	if(testResult != 0)
	{
		Thought(DaWorldLL::TestBdResultText(testResult));
		return;
	}
	world->AddLandscape(startLoc, endLoc, width, hideRocks, hideGrass, hideShrubs, hideTrees, pavement, false);
}

AuId DaPerson::BdRename(const QString& nuName, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return AuId(0);
	if(!bd->InChainOfCommand(this))
	{
		Thought(tr("I do not have the authority to rename the building."));
		return AuId(0);
	}
	if(bd->AssignedName() == nuName)
	{
		Thought(tr("The building is already named %1.").arg(nuName));
		return AuId(0);
	}
	bd->SetName(nuName);
	bd->SetModified();
	if(nuName.isEmpty())
		Thought(tr("The building is now unnamed."));
	else
		Thought(tr("The name of the building is now %1.").arg(nuName));
	return bd->Id();
}

void DaPerson::BdManufactureSpacecraftBuyCompanyShip(quint32 designId, AuGearPos pos, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;

	// Buildings with no empire id cannot build spacecraft.
	quint32 empireId = bd->EmpireId();
	if(!empireId)
	{
		Thought(QObject::tr("This spacecraft factory is not part of any empire. It cannot manufacture spacecraft."));
		return;
	}
	quint32 myEmpireId = EmpireId();
	if(!myEmpireId)
	{
		Thought(QObject::tr("I cannot buy a spacecraft because I am not a member of any empire. "));
		return;
	}

	// Found the scene atom.
	quint8 maxQ = 0;
	if(bd->InChainOfCommand(this))
	{
		maxQ = 255;
	}
	else
	{
		maxQ = bd->CompanyMaxQ();
		if(bd->CompanyOk())
		{
			// Company ok setting is on.
			// Check commerce restrictions based upon political stance.
			POLITICAL_STANCE stance = bd->PoliticalStanceMakeContact(this);
			const DgData& gd = g_Au.LockDataForRead();
			bool restricted = gd.EmpireCommerceRestricted(empireId, CID_SupercooledLiquid, stance);
			g_Au.UnlockData();
			if(restricted)
			{
				QString ename;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(empireId, ename);
				g_Au.UnlockData();
				Thought(QObject::tr("%1 commerce restrictions prohibit the factory from building a ship for me.").arg(ename));
				return;
			}
		}
		else
		{
			Thought(QObject::tr("The factory is not authorized to build company spacecraft."));
			return;
		}
	}

	// I have the authority to purchase a company ship.
	QString designName;
	quint8 designQ = 0;
	double cost = 0;
	bool hasMedia = false;
	bool useOnceMedia = false;
	bool stanceOk = true;		// Set false if publish stance prohibits me from opening the design.
	bool hasDesign = false;		// purchase requires the design for price.
	AuGear* pinv = LockGearForWrite();
	AuItem* gitem = pinv->ItemAt(pos);
	if(gitem)
	{
		AuItemScDesign* scitem = dynamic_cast<AuItemScDesign*>(gitem);
		if(scitem)
		{
			// Found a new AuScDesign disk.
			if(scitem->Exchange().CanBeManufactured() && designId == scitem->Exchange().DesignId() &&
					scitem->Exchange().DesignQuality() <= maxQ)
			{
				designName = scitem->Exchange().DesignName();
				designQ = scitem->Exchange().DesignQuality();
				hasMedia = true;
				useOnceMedia = scitem->Exchange().UseOnce();
				stanceOk = scitem->Exchange().AccessibleTo(Id(), myEmpireId);
				cost = scitem->Exchange().EstimatedCost();
				hasDesign = true;

				// If we are sure we are going to select the process, use up the media.
				if(stanceOk && cost > 0 && cost <= AccountBalance())
				{
					// We will make the ship.
					if(useOnceMedia)
						pinv->Erase(pos);	// Take the use-once media.
				}
				else
					designId = 0;
			}
		}
		else
		{
			AuItemStorageMedia* storageMedia = dynamic_cast<AuItemStorageMedia*>(gitem);
			if(storageMedia)
			{
				AuScDesignExchange::const_iterator ei = storageMedia->BlueprintsSc().find(designId);
				if(ei != storageMedia->BlueprintsSc().cend() && (*ei).CanBeManufactured())
				{
					designName = (*ei).DesignName();
					designQ = (*ei).DesignQuality();
					hasMedia = true;
					useOnceMedia = (*ei).UseOnce();
					stanceOk = (*ei).AccessibleTo(Id(), myEmpireId);
					cost = (*ei).EstimatedCost();
					hasDesign = true;

					// If we are sure we are going to select the process, use up the media.
					if(stanceOk && cost > 0 && cost <= AccountBalance())
					{
						// We will make the ship.
						if(useOnceMedia)
						{
							// Erase the use-once blueprint.
							QSet<quint32> blueprints;
							blueprints.insert(designId);
							storageMedia->DeleteStoredBlueprintsSc(blueprints);
						}
					}
					else
						designId = 0;
				}
			}
		}
	}
	UnlockGear();

	if(!hasMedia)
	{
		Thought(QObject::tr("The design media was not accepted. I should try that again."));
		return;
	}
	else if(!stanceOk)
	{
		Thought(QObject::tr("The design was published under terms that make it inaccessible to my empire."));
		return;
	}
	else if(!hasDesign)
	{
		Thought(QObject::tr("The design is being loaded to calculate the cost. I should try again shortly."));
		return;
	}
	else if(cost > AccountBalance())
	{
		Thought(QObject::tr("I do not have the %L1%2 needed to buy the spacecraft.")
				.arg(cost, 0,'f',0)
				.arg(g_Au.CurrencySymbol()));
		return;
	}

	if(!designId || cost <= 0)
		return;

	// Take my money to buy the ship.
	DebitAccount(cost);
	bd->QueueMakeShip(designId, designName, designQ, Id(), Name(), myEmpireId);

	if(useOnceMedia)
		Thought(QObject::tr("I gave them %L1%2. The factory will send mail when my ship is done. "
					"They destroyed the blueprint because it was only authorized for one use.")
				.arg(cost, 0,'f',0)
				.arg(g_Au.CurrencySymbol()));
	else
		Thought(QObject::tr("I gave them %L1%2. The factory will send mail when my ship is done.")
				.arg(cost, 0,'f',0)
				.arg(g_Au.CurrencySymbol()));
}

void DaPerson::BdManufactureSpacecraftCommissionShip(quint32 designId, AuGearPos pos, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->EmpireId())
	{
		Thought(QObject::tr("This spacecraft factory is not part of any empire. It cannot manufacture spacecraft."));
		return;
	}
	else if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to commission a spacecraft at this factory."));
		return;
	}

	bool hasMedia = false;
	bool useOnceMedia = false;
	bool stanceOk = true;		// Set false if publish stance prohibits me from opening the design.

	// I have the authority to commission a fleet ship.
	QString designName;
	quint8 designQ = 0;
	AuGear* pinv = LockGearForWrite();
	AuItem* gitem = pinv->ItemAt(pos);
	if(gitem)
	{
		AuItemScDesign* scitem = dynamic_cast<AuItemScDesign*>(gitem);
		if(scitem)
		{
			// Found a new AuScDesign disk.
			if(scitem->Exchange().CanBeManufactured() && designId == scitem->Exchange().DesignId())
			{
				designName = scitem->Exchange().DesignName();
				designQ = scitem->Exchange().DesignQuality();
				hasMedia = true;
				useOnceMedia = scitem->Exchange().UseOnce();
				stanceOk = scitem->Exchange().AccessibleTo(Id(), EmpireId());

				// If we are sure we are going to select the process, use up the media.
				if(stanceOk)
				{
					if(useOnceMedia)
						pinv->Erase(pos);	// We will make the ship. Take the use-once media.
				}
				else
					designId = 0;
			}
		}
		else
		{
			AuItemStorageMedia* storageMedia = dynamic_cast<AuItemStorageMedia*>(gitem);
			if(storageMedia)
			{
				AuScDesignExchange::const_iterator ei = storageMedia->BlueprintsSc().find(designId);
				if(ei != storageMedia->BlueprintsSc().cend() && (*ei).CanBeManufactured())
				{
					designName = (*ei).DesignName();
					designQ = (*ei).DesignQuality();
					hasMedia = true;
					useOnceMedia = (*ei).UseOnce();
					stanceOk = (*ei).AccessibleTo(Id(), EmpireId());

					// If we are sure we are going to select the process, use up the media.
					if(stanceOk)
					{
						// We will make the ship.
						if(useOnceMedia)
						{
							// Erase the use-once blueprint.
							QSet<quint32> blueprints;
							blueprints.insert(designId);
							storageMedia->DeleteStoredBlueprintsSc(blueprints);
						}
					}
					else
						designId = 0;
				}
			}
		}
	}
	UnlockGear();
	if(!hasMedia)
	{
		Thought(QObject::tr("The design media was not accepted. I should try that again."));
		return;
	}
	else if(!stanceOk)
	{
		Thought(QObject::tr("The design was published under terms that make it inaccessible to my empire."));
		return;
	}

	if(!designId)
		return;

	bd->QueueMakeShip(designId, designName, designQ, AuId(0), "", 0);

	if(useOnceMedia)
		Thought(QObject::tr("The factory will build the spacecraft. "
					"They destroyed the blueprint because it was only authorized for one use."));
	else
		Thought(QObject::tr("The factory will build the spacecraft."));
}

void DaPerson::BdManufactureSpacecraftOrderDelete(quint32 designId, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to cancel orders at this factory."));
		return;
	}
	bd->DeleteOrder(designId);
	Thought(QObject::tr("The factory order was canceled."));
}

void DaPerson::BdManufactureSpacecraftProcessCanceled(const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to stop the running process."));
		return;
	}

	AuId buyerId;
	QString buyerName;
	double buyerRefund;
	bd->SpacecraftProcessCanceled(buyerId, buyerName, buyerRefund);

	// Refund the buyer's money, if it was a private purchase.
	if(!buyerId.IsNull() && buyerRefund > 0)
		emit SendFunds(Id(), Name(), buyerId, buyerRefund, Galaxy(), 3, 0);
}

void DaPerson::BdManufactureSpacecraftProcessDefaultSelected(quint32 designId, const QString& designName, quint8 designQ,
		const AuId& opBd)
{
	if(!designQ)
		return;
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change the default manufacturing process."));
		return;
	}
	bd->SetDefaultMakeShip(designId, designName, designQ);
}

void DaPerson::BdManufactureSpacecraftSetCompanyPolicy(bool companyOk, quint8 companyMaxQ, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change the company build policy."));
		return;
	}
	bd->SetCompanyPolicy(companyOk, companyMaxQ);
	bd->SetModified();
}

void DaPerson::BdManufactureSpacecraftSetMinQ(quint8 minQ, const AuId& opBd)
{
	DaBd* bd = BdOp(opBd);
	if(!bd)
		return;
	if(!bd->InChainOfCommand(this))
	{
		Thought(QObject::tr("I am not authorized to change the production quality."));
		return;
	}
	bd->SetMakeMinQ(minQ);
	bd->SetModified();
}

void DaPerson::EmpireGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos)
{
	quint32 empireId = EmpireId();
	if(!empireId)
		return;
	if(patentCIDs.empty())
		return;
	if(!maxQ)
		return;
	DgEmpire emp;
	emp.m_id = empireId;
	const DgData& gd = g_Au.LockDataForRead();
	bool hasEmp = gd.Empire(emp);
	g_Au.UnlockData();
	if(!hasEmp)
		return;

	QString resultMsg;
	AuGear* pinv = LockGearForWrite();
	AuItem* invmedia = pinv->ItemAt(mediaPos);
	if(invmedia)
	{
		// Person had blank media, either a disk or paper.
		AuItemStorageMedia* storage = dynamic_cast<AuItemStorageMedia*>(invmedia);
		if(storage)
		{
			// Copy all selected patents to the storage media.
			int stored = 0;
			for(QSet<COMMODITYID>::const_iterator pi = patentCIDs.cbegin(); pi != patentCIDs.cend(); pi++)
			{
				quint8 patentQ = emp.PatentQuality(*pi);
				if(!patentQ)
					continue;	// City doesn't have the patent. wtf?
				if(patentQ > maxQ)
					patentQ = maxQ;
				storage->StorePatent(*pi, patentQ);
				stored++;
			}
			resultMsg = tr("%1 patents were stored to the %2.").arg(stored).arg(storage->SayName());
			UpdateGearWeight();
			UnlockGear();
		}
		else
		{
			// Copy one selected patent to the blank media. That is all it will hold.
			COMMODITYID patentCID = *patentCIDs.cbegin();
			quint8 patentQ = emp.PatentQuality(patentCID);
			if(patentQ)
			{
				if(patentQ > maxQ)
					patentQ = maxQ;
				AuItem* media = invmedia->Clone();
				UnlockGear();
				if(media)
				{
					// Use up the media to get a copy of the patent.
					media->SetCount(1);
					ConsumeItem(*media);
					if(media->UseItem())					// Quality check the result.
					{
						// Replace the blank media with a patent.
						AuItem* patentItem;
						if(media->CommodityId() == CID_BlankDisk)
							patentItem = new AuItem(patentCID, patentQ, empireId);
						else
							patentItem = new AuItem(patentCID, patentQ, empireId);

						// Put it wherever there's room.
						AddGearItem(*patentItem);
						resultMsg = tr("I got %1.").arg(patentItem->SayName());
						delete patentItem;
					}
					else
						resultMsg = tr("The %1 was defective; it failed while trying to get the patent. "
								"I should try again with another blank disk or paper.").arg(media->SayName());
					delete media;
				}
			}
			else
				UnlockGear();
		}
	}
	else
	{
		UnlockGear();
		resultMsg = tr("I need storage media to get a copy of the patent.");
	}
	if(!resultMsg.isEmpty())
		Thought(resultMsg);
}

void DaPerson::EmpireRecordPatents(const AuKnownPatents& patents)
{
	quint32 empireId = EmpireId();
	if(!empireId)
		return;
	// Record the patents.
	int had = 0;
	int recorded = 0;
	QString empireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(empireId, empireName);
	for(QHash<COMMODITYID, quint8>::const_iterator pi = patents.cbegin(); pi != patents.cend(); pi++)
	{
		if(gd.EmpireHasPatent(empireId, pi.key(), *pi))
			had++;
		else
		{
			emit EmpireRecordPatent(empireId, pi.key(), *pi);
			recorded++;
		}

	}
	g_Au.UnlockData();

	// Report the result.
	QString resultMsg;
	if(patents.size() == 1)
	{
		QHash<COMMODITYID, quint8>::const_iterator pi = patents.cbegin();
		if(recorded)
			resultMsg = tr("The %1 patent office recorded the %2.")
				.arg(empireName)
				.arg(g_Au.CommodityName(pi.key()));
		else
			resultMsg = tr("The %2 is already recorded at the %1 patent office.")
				.arg(empireName)
				.arg(g_Au.CommodityName(pi.key()));
	}
	else
	{
		if(recorded && had)
			resultMsg = tr("The %1 patent office recorded %2 new patents. "
					"The other %3 were already on record.")
				.arg(empireName)
				.arg(recorded)
				.arg(had);
		else if(recorded)
			resultMsg = tr("The %1 patent office recorded %2 new patents.")
				.arg(empireName)
				.arg(recorded);
		else
			resultMsg = tr("All of those patents are already recorded at the %1 patent office.")
				.arg(empireName);
	}
	Thought(resultMsg);
}

void DaPerson::InitPreview(quint8 previewType)
{
	if(previewType <= 1)
		return;
	DaDesigner* de = FindParentDesignerInEnv();
	if(!de)
		return;
	DaStar* star = de->FindStar();
	if(!star)
		return;
	AuId parentId(ParentId());
	AuPoint loc, upVec;
	DaWorldLL* claim = 0;
	switch(previewType)
	{
	case 2:	// sun
		{
			DaVessel* sc = star->FindVessel();
			if(sc)
			{
				// Move me to the ship.
				parentId = sc->Id();
				loc = sc->HelmLocation();
				upVec = AuPoint(0,0,1);
			}
			else
			{
				// Move me to orbit the sun.
				parentId = star->Id();
				double orad = star->OrbitRadius(star->HabitableZone()) * FeetPerAu();
				loc = AuPoint(orad,0,0);
				upVec = AuPoint(0,0,1);
			}
		}
		break;
	case 3:	// world
		{
			DaWorldLL* world = star->GetHabitableWorld(HABITABLE_HomeWorld);
			if(!world)
				return;
			claim = world;
			DaVessel* sc = world->FindVessel();
			if(sc)
			{
				// Move me to the ship.
				parentId = sc->Id();
				loc = sc->HelmLocation();
				upVec = AuPoint(0,0,1);
			}
			else
			{
				// Move me to the world.
				parentId = world->Id();
				loc = world->AvatarPreviewLocation(previewType);
				upVec = -world->GravityDir(loc);
			}
		}
		break;
	case 4:	// moon
		{
			DaWorldLL* world = star->GetHabitableWorld(HABITABLE_HomeWorld);
			if(!world)
				return;
			DaAtomList moons;
			world->FindMoons(moons);
			if(moons.empty())
				return;
			DaWorldLL* moon = dynamic_cast<DaWorldLL*>(moons.first());
			if(!moon)
				return;
			claim = moon;
			DaVessel* sc = moon->FindVessel();
			if(sc)
			{
				// Move me to the ship.
				parentId = sc->Id();
				loc = sc->HelmLocation();
				upVec = AuPoint(0,0,1);
			}
			else
			{
				// Move me to the moon.
				parentId = moon->Id();
				loc = moon->AvatarPreviewLocation(previewType);
				upVec = -moon->GravityDir(loc);
			}
		}
		break;
	case 5:	// inferno
		{
			DaWorldLL* world = star->GetWorld(OZONE_Inferno);
			if(!world)
				return;
			claim = world;
			DaVessel* sc = world->FindVessel();
			if(sc)
			{
				// Move me to the ship.
				parentId = sc->Id();
				loc = sc->HelmLocation();
				upVec = AuPoint(0,0,1);
			}
			else
			{
				// Move me to the world.
				parentId = world->Id();
				loc = world->AvatarPreviewLocation(previewType);
				upVec = -world->GravityDir(loc);
			}
		}
		break;
	case 6:	// frigid
		{
			DaWorldLL* world = star->GetWorld(OZONE_Frigid);
			if(!world)
				return;
			claim = world;
			DaVessel* sc = world->FindVessel();
			if(sc)
			{
				// Move me to the ship.
				parentId = sc->Id();
				loc = sc->HelmLocation();
				upVec = AuPoint(0,0,1);
			}
			else
			{
				// Move me to the world.
				parentId = world->Id();
				loc = world->AvatarPreviewLocation(previewType);
				upVec = -world->GravityDir(loc);
			}
		}
		break;
	case 7:	// gas giant
		{
			DaWorldLL* world = star->GetGasGiant();
			if(!world)
				return;
			claim = world;
			DaVessel* sc = world->FindVessel();
			if(sc)
			{
				// Move me to the ship.
				parentId = sc->Id();
				loc = sc->HelmLocation();
				upVec = AuPoint(0,0,1);
			}
			else
			{
				// Move me to the world.
				parentId = world->Id();
				loc = world->AvatarPreviewLocation(previewType);
				upVec = -world->GravityDir(loc);
			}
		}
		break;
	case 8:	// ringworld
		{
			DaWorldLL* world = star->GetHabitableWorld(HABITABLE_Ringworld);
			if(!world)
				return;
			claim = world;
			DaVessel* sc = world->FindVessel();
			if(sc)
			{
				// Move me to the ship.
				parentId = sc->Id();
				loc = sc->HelmLocation();
				upVec = AuPoint(0,0,1);
			}
			else
			{
				// Move me to the world.
				parentId = world->Id();
				loc = world->AvatarPreviewLocation(previewType);
				upVec = -world->GravityDir(loc);
			}
		}
		break;
	case 9:	// sea bottom
		{
			DaWorldLL* world = star->GetHabitableWorld(HABITABLE_HomeWorld);
			if(!world)
				return;
			claim = world;
			// Move me to the world.
			parentId = world->Id();
			loc = world->AvatarPreviewLocation(previewType);
			upVec = -world->GravityDir(loc);
		}
		break;
	}
	if(parentId.IsNull())
		return;

	AuMatrix4x4 tBody;
	tBody.Translate(loc);
	tBody *= AuPoint(0,0,1).GetAlignmentTransform(upVec);
	tBody.Translate(0, 0, GroundHeightAboveCG()); 
	ForcePosStop(AuAtomPos(parentId, tBody));
	Stand();
	SyncController(AuSyncPose(AuPose::Stand, 0, 0, 0));
	if(claim)
		claim->SetClaimedBy(*this, GlobeLL());
	SetModified();
}

void DaPerson::ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const
{
	if(!InDesignEnvironment())
		return;
	// Hang onto all finalized designs made by me.
	const DgData& bdd = g_Au.LockBdDesignsForRead();
	for(AuBdDesignHash::const_iterator bdi = bdd.m_BdDesigns->cbegin(); bdi != bdd.m_BdDesigns->cend(); bdi++)
		if(!(*bdi).second.IsDeleted())
			bdDesigns.insert((*bdi).second.Id());
	g_Au.UnlockBdDesigns();

	const DgData& sdd = g_Au.LockScDesignsForRead();
	for(AuScDesignHash::const_iterator sci = sdd.m_ScDesigns->cbegin(); sci != sdd.m_ScDesigns->cend(); sci++)
		if(!(*sci).second.IsDeleted())
			scDesigns.insert((*sci).second.Id());
	g_Au.UnlockScDesigns();
}

void DaPerson::RenderHelmetHUD(AuAtomRenderInfo& ari, bool leftOriented)
{
	/* These have already been tested.
	if(!ari.IsViewFirstPerson())
		return;
	if(ari.m_ViewDist1st > 0)
		return;
	if(Helmet() == CID_Invalid)
		return;
		*/
	bool helmetBroken = false;
	bool suitBroken = false;
	bool suitWorn = false;
	const AuGear* gear = LockGearForRead();
	const AuItem* helmet = gear->HelmetItemWorn();
	if(helmet && helmet->HitPoints() <= 1)
		helmetBroken = true;					// Helmet seal is broken. We are only called if it is an env helmet.
	const AuItem* armor = gear->ArmorItemWorn();
	if(armor && armor->IsEnvironmentSuit())
	{
		suitWorn = true;
		if(armor->HitPoints() <= 1)
			suitBroken = true;						// Suit seal is broken.
	}
	UnlockGear();

	// Create textures.
	GLuint hudTex;
	if(helmetBroken)
	{
		if(leftOriented)
		{
			if(!s_HelmetHUDTexLBroken)
			{
				QImage img = QImage(":data/HelmetHUDBroken.png");
				QImage imgL = QImage(img.size(), img.format());
				imgL.fill(0);
				QPainter p(&imgL);
				p.translate(img.width(),0);
				p.scale(-1,1);
				p.drawImage(0,0, img);
				p.end();
				s_HelmetHUDTexLBroken = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "HHLB", imgL);
			}
			hudTex = s_HelmetHUDTexLBroken;
		}
		else
		{
			if(!s_HelmetHUDTexRBroken)
				s_HelmetHUDTexRBroken = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "HHRB", ":data/HelmetHUDBroken.png");
			hudTex = s_HelmetHUDTexRBroken;
		}
	}
	else
	{
		if(leftOriented)
		{
			if(!s_HelmetHUDTexL)
			{
				QImage img = QImage(":data/HelmetHUD.png");
				QImage imgL = QImage(img.size(), img.format());
				imgL.fill(0);
				QPainter p(&imgL);
				p.translate(img.width(),0);
				p.scale(-1,1);
				p.drawImage(0,0, img);
				p.end();
				s_HelmetHUDTexL = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "HHL", imgL);
			}
			hudTex = s_HelmetHUDTexL;
		}
		else
		{
			if(!s_HelmetHUDTexR)
				s_HelmetHUDTexR = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "HHR", ":data/HelmetHUD.png");
			hudTex = s_HelmetHUDTexR;
		}
	}
	if(!hudTex)
		return;
	if(!g_TexLightOmni)
		g_TexLightOmni = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "LOmni", ":data/txLightOmni.png");

	// HUD colors
	float r = 1;
	float g = 1;
	float b = 1;
	float a = 1;//(float)hudIntensity / 10.f;

	double windowAspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());
	const float ymax = 1;
	const float xmax = ymax/windowAspect;

	ari.re->glDepthMask(GL_FALSE);
	//ari.re->glDisable(GL_DEPTH_TEST);
	//ari.re->glEnable(GL_BLEND);
	//ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Additive blending for extra brightness.

	AuMatrix4x4 mv;

	ari.re->GLSL().TextureShader().Select();
	ari.re->GLSL().TextureShader().SetProjectionModelView(mv);
	ari.re->GLSL().TextureShader().SetColor(r,g,b,a);
	ari.re->GLSL().TextureShader().SetTexture(hudTex);

	float xy=1;
	QList<AuTriTex> tris;
	tris << AuTriTex(AuPointTex(-xy, -xy, 0, 0,0),
			AuPointTex(-xy, xy, 0, 0,1),
			AuPointTex(xy, xy, 0, 1,1));
	tris << AuTriTex(AuPointTex(xy, xy, 0, 1,1),
			AuPointTex(xy, -xy, 0, 1,0),
			AuPointTex(-xy, -xy, 0, 0,0));

	AuGLVAO vao;
	vao.CreateForTexture(*ari.re, tris, GL_STREAM_DRAW);
	if(vao.VAO())
	{
		// Screen overlay.
		vao.SelectGL();
		vao.DrawGL();

		// Air breathability indicator.
		float airR = 0;
		float airG = 0;
		float airB = 0;
		if((!IsAirBreathable() && !IsInBerth() && !IsPatientStationPose()) || IsMouthSubmerged())
			airR = 1;
		else
			airG = 1;
		double dotSize = .2;
		mv.Translate(0,ymax,0);
		mv.Scale(dotSize*windowAspect, dotSize, 1);
		ari.re->GLSL().TextureShader().SetProjectionModelView(mv);
		ari.re->GLSL().TextureShader().SetColor(airR,airG,airB,a);
		ari.re->GLSL().TextureShader().SetTexture(g_TexLightOmni);
		vao.DrawGL();

		// Helmet integrity indicator.
		float xpos;
		float ypos = ymax - 12.f/300.f;
		double indicatorSize = .05;
		float indicatorR = 0;
		float indicatorG = 0;
		float indicatorB = 0;
		if(helmetBroken)
		{
			xpos = -(926.f +4 - 600.f)/600.f;
			indicatorR = AuTime::CurrentCycle(500)/2 + .5f;
		}
		else
		{
			float cycle = AuTime::CurrentCycle(8765);
			if(cycle < .333f)
				xpos = -(926.f +4 - 600.f)/600.f;
			else if(cycle < .667f)
				xpos = -(950.f +4 - 600.f)/600.f;
			else
				xpos = -(974.f +4 - 600.f)/600.f;
			indicatorG = 1;
		}
		ari.re->GLSL().TextureShader().SetColor(indicatorR,indicatorG,indicatorB,a);
		mv.LoadIdentity();
		mv.Translate(xpos,ypos,0);
		mv.Scale(indicatorSize*windowAspect, indicatorSize, 1);
		ari.re->GLSL().TextureShader().SetProjectionModelView(mv);
		vao.DrawGL();

		if(suitWorn)
		{
			// Suit integrity indicator.
			if(suitBroken)
			{
				xpos = (926.f - 600.f)/600.f;
				indicatorR = AuTime::CurrentCycle(500)/2 + .5f;
			}
			else
			{
				float cycle = AuTime::CurrentCycle(10987);
				if(cycle < .333f)
					xpos = (926.f - 600.f)/600.f;
				else if(cycle < .667f)
					xpos = (950.f - 600.f)/600.f;
				else
					xpos = (974.f - 600.f)/600.f;
				indicatorG = 1;
			}
			ari.re->GLSL().TextureShader().SetColor(indicatorR,indicatorG,indicatorB,a);
			mv.LoadIdentity();
			mv.Translate(xpos,ypos,0);
			mv.Scale(indicatorSize*windowAspect, indicatorSize, 1);
			ari.re->GLSL().TextureShader().SetProjectionModelView(mv);
			vao.DrawGL();
		}

		vao.DeselectGL();
		vao.DeleteGL();
	}

	ari.re->GLSL().TextureShader().Deselect();

	// Render some numeric info.
	double speed, speedV, speedF, speedL, altitude, pitch, roll, heading, longitude, latitude;
	int mode = FlightInstruments(AuPoint(0,0), 0, speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude,
			latitude);

	float xpos = 0;
	float ypos = 0;
	float size = .05f;
	a = .8f;

	// Heading and altimeter.
	if(mode <= 2)
	{
		// Heading in degrees
		xpos = -xmax/4 - .02f;
		ypos = ymax - .15f;
		ari.RenderNumber(round(heading * RAD2DEG), xpos,ypos,size, 2, 0, r,g,b,a);

		// Altimeter in meters
		xpos = -xmax/4 + .02f;
		ari.RenderNumber(round(altitude / FeetPerMeter()), xpos,ypos,size, 0, 0, r,g,b,a);

		// Longitude in degrees
		xpos = xmax/4 - .02f;
		ari.RenderNumber(round(longitude*RAD2DEGf), xpos,ypos,size, 2, 3, r,g,b,a);

		// Latitude in degrees
		xpos = xmax/4 + .02f;
		ari.RenderNumber(round(latitude*RAD2DEGf), xpos,ypos,size, 0, 2, r,g,b,a);
	}

	// Speed in meters per second.
	if(speed != 0)
	{
		if(Vector().y < 0)
			speed = -speed;			// Correct sign of overall speed for reverse.
		xpos = 0;
		ypos = ymax - .25f;
		ari.RenderNumber(round(speed/FeetPerMeter()), xpos,ypos,size, 1, 0, r,g,b,a);
	}

	RenderHelmetHUDRadar(ari, leftOriented);
	//ari.re->glDisable(GL_BLEND);
	//ari.re->glEnable(GL_DEPTH_TEST);
	ari.re->glDepthMask(GL_TRUE);
}

void DaPerson::RenderHelmetHUDRadar(AuAtomRenderInfo& ari, bool leftOriented)
{
	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;
	DaAtomList sceneAtoms;
	env->ListNormalSubatomsRecursive(sceneAtoms);

	// Find all units within range.
	AuId parentId(ParentId());
	double ptSizeCreature = ari.glw->height()/300 * 6;
	double ptSizeEquipment = ari.glw->height()/300 * 10;
	double radarRange = 100*FeetPerMeter();
	double radarRange2 = radarRange * radarRange;
	QList<AuPointAlpha> empireBdCiv;
	QList<AuPointAlpha> friendBdCiv;
	QList<AuPointAlpha> neutralBdCiv;
	QList<AuPointAlpha> enemyBdCiv;
	QList<AuPointAlpha> empireBdMil;
	QList<AuPointAlpha> friendBdMil;
	QList<AuPointAlpha> neutralBdMil;
	QList<AuPointAlpha> enemyBdMil;
	QList<AuPointAlpha> empireCreaturesCiv;
	QList<AuPointAlpha> friendCreaturesCiv;
	QList<AuPointAlpha> neutralCreaturesCiv;
	QList<AuPointAlpha> enemyCreaturesCiv;
	QList<AuPointAlpha> empireCreaturesMil;
	QList<AuPointAlpha> friendCreaturesMil;
	QList<AuPointAlpha> neutralCreaturesMil;
	QList<AuPointAlpha> enemyCreaturesMil;
	QList<AuPointAlpha> empireEquipmentCiv;
	QList<AuPointAlpha> friendEquipmentCiv;
	QList<AuPointAlpha> neutralEquipmentCiv;
	QList<AuPointAlpha> enemyEquipmentCiv;
	QList<AuPointAlpha> empireEquipmentMil;
	QList<AuPointAlpha> friendEquipmentMil;
	QList<AuPointAlpha> neutralEquipmentMil;
	QList<AuPointAlpha> enemyEquipmentMil;
	bool foundBdCiv = false;
	bool foundBdMil = false;
	bool foundCreature = false;
	bool foundEquipment = false;
	for(DaAtomList::const_iterator it = sceneAtoms.cbegin(); it != sceneAtoms.cend(); it++)
	{
		DaUnit* atom = dynamic_cast<DaUnit*>(*it);
		if(!atom)
			continue;
		AuPointAlpha locRelToMe;
		const DaBd* bd = dynamic_cast<const DaBd*>(atom);
		if(bd)
		{
			AuBox ext;
			bd->RoughExtents(ext);
			AuPoint cen = (ext.m_fCorners[0] + ext.m_fCorners[6])/2;
			double radX = cen.x - ext.m_fCorners[0].x;
			double radY = cen.y - ext.m_fCorners[0].y;
			double rad;
			if(radX > radY)
				rad = radX;
			else
				rad = radY;
			rad /= 3;
			if(rad > 30)
				rad = 30;
			locRelToMe = AuPointAlpha(EnvTransform().UnProject(bd->EnvTransform().Project(cen)), ari.glw->height()/300 * rad);
		}
		else
		{
			if(Is(atom->Id()) || parentId == atom->Id())
				continue;	// Don't show myself or my vehicle/spacecraft/mount.
			if(atom->IsUnderGround() || atom->IsInVehicle())
				continue;	// Don't show underground units.
			if(atom->IsInsideCapsule() && atom->ParentId() != parentId)
				continue;	// Don't show units inside other units that I am not subordinate to.
			locRelToMe = AuPointAlpha(EnvTransform().UnProject(atom->EnvLocation()), ptSizeCreature);
		}
		locRelToMe.z = 0;
		if(locRelToMe.Length2() > radarRange2)
			continue;
		POLITICAL_STANCE stance = PoliticalStance(EmpireId(), atom->EmpireId());
		if(bd)
		{
			if(bd->IsMilitary())
			{
				foundBdMil = true;
				switch(stance)
				{
				case POL_Friend:
					friendBdMil.append(locRelToMe);
					break;
				case POL_Neutral:
					neutralBdMil.append(locRelToMe);
					break;
				case POL_Enemy:
					enemyBdMil.append(locRelToMe);
					break;
				case POL_Empire:
				case POL_Vassal:
					empireBdMil.append(locRelToMe);
					break;
				}
			}
			else
			{
				foundBdCiv = true;
				switch(stance)
				{
				case POL_Friend:
					friendBdCiv.append(locRelToMe);
					break;
				case POL_Neutral:
					neutralBdCiv.append(locRelToMe);
					break;
				case POL_Enemy:
					enemyBdCiv.append(locRelToMe);
					break;
				case POL_Empire:
				case POL_Vassal:
					empireBdCiv.append(locRelToMe);
					break;
				}
			}
		}
		else if(dynamic_cast<DaCreature*>(atom))
		{
			if(atom->IsEthereal())
				continue;
			foundCreature = true;
			if(atom->IsCivilian())
			{
				switch(stance)
				{
				case POL_Friend:
					friendCreaturesCiv.append(locRelToMe);
					break;
				case POL_Neutral:
					neutralCreaturesCiv.append(locRelToMe);
					break;
				case POL_Enemy:
					enemyCreaturesCiv.append(locRelToMe);
					break;
				case POL_Empire:
				case POL_Vassal:
					empireCreaturesCiv.append(locRelToMe);
					break;
				}
			}
			else
			{
				switch(stance)
				{
				case POL_Friend:
					friendCreaturesMil.append(locRelToMe);
					break;
				case POL_Neutral:
					neutralCreaturesMil.append(locRelToMe);
					break;
				case POL_Enemy:
					enemyCreaturesMil.append(locRelToMe);
					break;
				case POL_Empire:
				case POL_Vassal:
					empireCreaturesMil.append(locRelToMe);
					break;
				}
			}
		}
		else
		{
			locRelToMe.m_Alpha = ptSizeEquipment;
			foundEquipment = true;
			if(atom->IsCivilian())
			{
				switch(stance)
				{
				case POL_Friend:
					friendEquipmentCiv.append(locRelToMe);
					break;
				case POL_Neutral:
					neutralEquipmentCiv.append(locRelToMe);
					break;
				case POL_Enemy:
					enemyEquipmentCiv.append(locRelToMe);
					break;
				case POL_Empire:
				case POL_Vassal:
					empireEquipmentCiv.append(locRelToMe);
					break;
				}
			}
			else
			{
				switch(stance)
				{
				case POL_Friend:
					friendEquipmentMil.append(locRelToMe);
					break;
				case POL_Neutral:
					neutralEquipmentMil.append(locRelToMe);
					break;
				case POL_Enemy:
					enemyEquipmentMil.append(locRelToMe);
					break;
				case POL_Empire:
				case POL_Vassal:
					empireEquipmentMil.append(locRelToMe);
					break;
				}
			}
		}
	}
	if(!foundBdCiv && !foundBdMil && !foundCreature && !foundEquipment)
		return;

	if(!s_BlipBdCivTexture)
		s_BlipBdCivTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "BdCiv", ":data/RadarBuildingCiv.png");
	if(!s_BlipBdMilTexture)
		s_BlipBdMilTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "BdMil", ":data/RadarBuildingMil.png");
	if(!s_BlipCreatureTexture)
		s_BlipCreatureTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
	if(!s_BlipEquipmentTexture)
		s_BlipEquipmentTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "StarSprite", ":data/Star.png", GL_LINEAR,
				GL_LINEAR, true, 2);

	AuMatrix4x4 pmv;
	double windowAspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());
	double radarScale = 148./600.;	// Radar display is 148 pixels in height on a 600 pixel tall image.
	if(leftOriented)
		pmv.Translate(-(1066.-600)/600., -(568.-300)/300., 0.);
	else
		pmv.Translate((1066.-600)/600., -(568.-300)/300., 0.);
	double screenRange = radarRange+10*FeetPerMeter();	// A little larger than radar range so blips don't overlap the edges.
	pmv.Scale(1/screenRange*windowAspect * radarScale, 1/screenRange * radarScale, 1);

	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	ari.re->glEnable(GL_PROGRAM_POINT_SIZE); 
	ari.re->GLSL().SpriteActualSizeShader().Select();
	ari.re->GLSL().SpriteActualSizeShader().SetProjectionModelView(pmv);
	if(foundBdCiv)
	{
		ari.re->GLSL().SpriteActualSizeShader().SetTexture(s_BlipBdCivTexture);

		if(!empireBdCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Empire, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, empireBdCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!friendBdCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Friend, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, friendBdCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!neutralBdCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Neutral, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, neutralBdCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!enemyBdCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Enemy, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, enemyBdCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}
	if(foundBdMil)
	{
		ari.re->GLSL().SpriteActualSizeShader().SetTexture(s_BlipBdMilTexture);

		if(!empireBdMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Empire, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, empireBdMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!friendBdMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Friend, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, friendBdMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!neutralBdMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Neutral, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, neutralBdMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!enemyBdMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Enemy, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, enemyBdMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}
	if(foundCreature)
	{
		ari.re->GLSL().SpriteActualSizeShader().SetTexture(s_BlipCreatureTexture);

		if(!empireCreaturesCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Empire, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, empireCreaturesCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!friendCreaturesCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Friend, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, friendCreaturesCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!neutralCreaturesCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Neutral, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, neutralCreaturesCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!enemyCreaturesCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Enemy, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, enemyCreaturesCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!empireCreaturesMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Empire, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, empireCreaturesMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!friendCreaturesMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Friend, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, friendCreaturesMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!neutralCreaturesMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Neutral, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, neutralCreaturesMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!enemyCreaturesMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Enemy, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, enemyCreaturesMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}
	if(foundEquipment)
	{
		ari.re->GLSL().SpriteActualSizeShader().SetTexture(s_BlipEquipmentTexture);

		if(!empireEquipmentCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Empire, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, empireEquipmentCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!friendEquipmentCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Friend, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, friendEquipmentCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!neutralEquipmentCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Neutral, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, neutralEquipmentCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!enemyEquipmentCiv.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Enemy, true));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, enemyEquipmentCiv, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!empireEquipmentMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Empire, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, empireEquipmentMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!friendEquipmentMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Friend, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, friendEquipmentMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!neutralEquipmentMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Neutral, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, neutralEquipmentMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		if(!enemyEquipmentMil.empty())
		{
			ari.re->GLSL().SpriteActualSizeShader().SetColor(PoliticalStanceColor(POL_Enemy, false));
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, enemyEquipmentMil, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}

	ari.re->GLSL().SpriteActualSizeShader().Deselect();
	ari.re->glDisable(GL_PROGRAM_POINT_SIZE); 
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

quint64 DaPerson::MemoryUsage() const
{
	quint64 mem = DaSentient::MemoryUsage();
	mem += sizeof(DaPerson) - sizeof(DaSentient);
	mem += m_Stats.MemoryUsage();
	mem += m_ArenaExitInv.MemoryUsage();
	mem += StringMemoryUsage(m_DesignPass);
	mem += m_Log.MemoryUsage();
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	mem += m_Selfie.sizeInBytes();
#else
	mem += m_Selfie.byteCount();
#endif
	mem += StringMemoryUsage(m_SpawnBdName);
	mem += StringMemoryUsage(m_SpawnBdSystemName);
	mem += StringMemoryUsage(m_SpawnShipName);
	mem += m_Gear.MemoryUsage();
	mem += m_Jobs.MemoryUsage();
	mem += m_Memories.MemoryUsage();
	mem += m_SeaChest.MemoryUsage();
	return mem;
}

void DaPerson::BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire)
{
	Q_UNUSED(surrenderingEmpire);
	Q_UNUSED(toEmpire);
}

void DaPerson::EmpireDeleted(quint32 empireId)
{
	if(EmpireId() != empireId)
		return;
	// Empire is not supposed to be deleted while it has avatars.
	qWarning() << "error: DaPerson::EmpireDeleted" << Id() << Name() << "state" << AtomState() << "empire" << empireId;
	SetEmpireId(0);
}

bool DaPerson::HasConstructedBuildingHabitable(INDUSTRYID iid) const
{
	return Stats().HasConstructedBuildingHabitable(iid);
}

bool DaPerson::HasConstructedBuildingHarsh(INDUSTRYID iid) const
{
	return Stats().HasConstructedBuildingHarsh(iid);
}

bool DaPerson::StoryClearScript()
{
	bool clearedScript = false;
	m_ScriptLock.lock();
	if(m_Script)
	{
		delete m_Script;
		m_Script = 0;
		clearedScript = true;
	}
	m_ScriptLock.unlock();
	return clearedScript;
}

int DaPerson::StoryCancelScript()
{
	if(!StoryClearScript())
		return 0;		// No script to clear.
	if(!ResetStory(0))
		return 1;		// Script cleared.
	return 2;			// Script and job cleared.
}

void DaPerson::StorySetScript(const AuScript& script)
{
	m_ScriptLock.lock();
	if(m_Script)
		delete m_Script;
	m_Script = new AuScript(script);
	m_ScriptLock.unlock();
}

void DaPerson::StoryScriptSubmitted(const AuScript& script)
{
	//qDebug() << "DaPerson::StoryScriptSubmitted" << Id() << Name() << script.Title();
	// Terminate current custom story, if any.
	Q_ASSERT(script.Id() == 0);
	ResetStory(script.Id());		// Forgets ever having started or finished the script.
	if(script.AnyAutoStart())
	{
		StorySetScript(script);		// The script is capable of auto starting. Set it as my current custom story script.
		AutoStartCustomScript();	// Attempt to start the script immediately.
	}
}

void DaPerson::FindLivePeopleRecursive(DaAtomList& people)
{
	//if(IsDead())
		//return;
	DaSentient::FindLivePeopleRecursive(people);
	people.append(this);
}

void DaPerson::FindPeopleRecursive(DaAtomList& people) const
{
	//if(!IsNormal())
		//return;
	DaSentient::FindPeopleRecursive(people);
	people.append((DaPerson*)this);	// Cast away constness.
}

const DaPerson* DaPerson::FindAIPersonRecursive() const
{
	//if(!IsNormal())
		//return 0;
	if(IsAIControlled())
		return this;
	return DaSentient::FindAIPersonRecursive();
}

const DaPerson* DaPerson::FindPersonOnLineRecursive() const
{
	//if(!IsNormal())
		//return 0;
	if(IsOnLine())
		return this;
	return DaSentient::FindPersonOnLineRecursive();
}

void DaPerson::FindPeopleAwakeRecursive(DaAtomList& people)
{
	//if(!IsNormal())
		//return;
	DaSentient::FindPeopleAwakeRecursive(people);
	if(IsOnLine() || IsAIControlled())
		people.append(this);
}

void DaPerson::FindPeopleOnLineRecursive(DaAtomList& people) const
{
	//if(!IsNormal())
		//return;
	DaSentient::FindPeopleOnLineRecursive(people);
	if(IsOnLine())
		people.append((DaPerson*)this);	// Cast away constness.
}

void DaPerson::FindPeopleOnLineRecursiveInStory(DaAtomList& people, quint32 storyInstance)
{
	//if(!IsNormal())
		//return;
	DaSentient::FindPeopleOnLineRecursiveInStory(people, storyInstance);
	if(IsOnLine() && IsInStoryInstance(storyInstance))
		people.append(this);
}

void DaPerson::FindAIPeopleRecursive(const AuId& shipAssigned, DaAtomList& aipeople) const
{
	//if(!IsNormal())
		//return;
	DaSentient::FindAIPeopleRecursive(shipAssigned, aipeople);
	if(AssignedShip() == shipAssigned && IsAIControlled())
		aipeople.append((DaAtom*)this);	// Cast away constness.
}

// Finds person atoms that are !IsDead. This means that people who are Normal or Travelling will be found.
DaPerson* DaPerson::FindPersonRecursive() const
{
	//if(IsDead())
		//return 0;
	return (DaPerson*)this;	// Cast away constness.
}

bool DaPerson::TakeItemBounty(const QString& bountyHunter, const AuBountyOnItem& bountyOnItem)
{
	if(!DaSentient::TakeItemBounty(bountyHunter, bountyOnItem))
		return false;
	IncreaseSkill(AuSkill::BountyHunter, 1);
	SetModified();
	QString text;
	if(HasCondition(AuConBounty::ConditionType()))
		text = tr("%1 collected the %L2%3 bounty on me for '%4'. It is gone.")
			.arg(bountyHunter)
			.arg(bountyOnItem.Amount(), 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(bountyOnItem.Reason());
	else
		text = tr("%1 collected the %L2%3 bounty on me for '%4'. I am free of bounties.")
			.arg(bountyHunter)
			.arg(bountyOnItem.Amount(), 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(bountyOnItem.Reason());
	if(IsOnLine())
		Thought(text);
	else
		AddCondition(new AuConThought(text, AuTime::Now()));
	return true;
}

void DaPerson::BountyCollected(const AuGearPos& gearPos, const AuBountyOnItem& bounty)
{
	if(bounty.Amount() <= 0)
		return;

	// Find the item with the bounty.
	bool hasBountyOnItem = false;
	m_GearLock.lockForWrite();
	AuItem* bountyItem = m_Gear.ItemAt(gearPos);
	if(bountyItem->Bounty() == bounty)
	{
		bountyItem->SetBounty(AuBountyOnItem());	// Clear the bounty once it is redeemed.
		m_tmGearChange.Set();
		hasBountyOnItem = true;
	}
	m_GearLock.unlock();
	if(!hasBountyOnItem)
		return;

	QString text = tr("I collected the %L2%3 bounty on %1 for '%4'.")
		.arg(bounty.Name())
		.arg(bounty.Amount(), 0,'f',0)
		.arg(AuGlobal::CurrencySymbol())
		.arg(bounty.Reason());
	Thought(text);
	Stats().BountyCollected(bounty.Amount());
	IncreaseSkill(AuSkill::BountyHunter, 1);
	CreditAccount(bounty.Amount());	// Calls SetModified
}

void DaPerson::BountyExpired(const AuGearPos& gearPos, const AuBountyOnItem& bounty)
{
	if(bounty.Amount() <= 0)
		return;

	// Find the item with the bounty.
	bool hasBountyOnItem = false;
	m_GearLock.lockForWrite();
	AuItem* bountyItem = m_Gear.ItemAt(gearPos);
	if(bountyItem->Bounty() == bounty)
	{
		bountyItem->SetBounty(AuBountyOnItem());	// Clear the bounty once it is redeemed.
		m_tmGearChange.Set();
		hasBountyOnItem = true;
	}
	m_GearLock.unlock();
	if(!hasBountyOnItem)
		return;

	QString text = tr("Someone else already collected the %L2%3 bounty on %1 for '%4'. I got nothing.")
		.arg(bounty.Name())
		.arg(bounty.Amount(), 0,'f',0)
		.arg(AuGlobal::CurrencySymbol())
		.arg(bounty.Reason());
	Thought(text);
	SetModified();
}

void DaPerson::PlaceItemOnTarget(const AuGearPos& gearPos, const AuId& targetId, const AuPoint& targetLoc)
{
	if(!targetId.IsNull() && targetLoc.Length() > 10)
	{
		// Target is too far away.
		Thought(tr("The target is too far away."));
		return;
	}

	// Remove the item from my inventory.
	DaItem* item = DropGearAt(gearPos);
	if(!item)
	{
		AuCommThoughtMessage msg(AuCommThoughtMessage::NoItemsPlaced, Id(), Name());
		g_Au.SendAtomChat(msg);
		return;
	}

	// Determine initial placement location, relative to my parent atom.
	AuMatrix4x4 mItem;
	if(targetId.IsNull())
	{
		// Nothing is targeted.
		// Drop the item at my feet.
		AuPoint placeLoc = Transform().Project(0, 0, -GroundHeightAboveCG() + item->GroundHeightAboveCG());
		mItem.Translate(placeLoc);
		mItem.Rotate(Rotation());
		item->ChangePosAndOrient(AuAtomPos(ParentId(), mItem));
		item->InvalidateEnvironment();
		Thought(tr("I placed the %1 on display at my feet.")
				.arg(item->SpotDescription(EmpireId())));
	}
	else if(targetId == ParentId())
	{
		// Parent atom is targeted, e.g. the ship I am inside of.
		// Place the item at the target location on my parent atom.
		AuPoint placeLoc = Transform().Project(targetLoc);
		mItem.Translate(placeLoc);
		mItem.Rotate(Rotation());
		item->ChangePosAndOrient(AuAtomPos(ParentId(), mItem));
		item->InvalidateEnvironment();
		Thought(tr("I placed the %1 on display.")
				.arg(item->SpotDescription(EmpireId())));
	}
	else
	{
		// Something is targeted that is not far away and it is not my parent atom.
		// Place the item on the target atom.
		DaAtom* env = FindEnvironmentAtom();
		if(!env)
			return;
		DaAtom* target = env->FindIdRecursive(targetId);
		if(!target || dynamic_cast<DaCreature*>(target))
			return;

		// Determine target location, relative to the target.
		DaAtom* commonParent = CommonParent(target, env);
		if(!commonParent)
			return;
		AuMatrix4x4 tMe, tTarg;
		GetTransformRelativeToParent(tMe, commonParent);
		target->GetTransformRelativeToParent(tTarg, commonParent);
		AuPoint placeLoc = tMe.Project(targetLoc);		// Relative to commonParent.
		placeLoc = tTarg.UnProject(placeLoc);			// Relative to the target atom.
		mItem.Translate(placeLoc);
		item->ChangePosAndOrient(AuAtomPos(target->Id(), mItem));
		item->InvalidateEnvironment();
		Thought(tr("I placed the %1 on display on %2.")
				.arg(item->SpotDescription(EmpireId()))
				.arg(target->Name()));
	}

	item->SetStaticDisplay(Id(), Name(), CapsuleRoomName());
	item->SetAtRest();
	item->InvalidateEnvironment();
}

unsigned int DaPerson::OnLineMinutes() const
{
	return Stats().OnLineMinutes();
}

void DaPerson::ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
		const QString& ownerName, quint32 ownerEmpire)
{
	DaSentient::ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
	if(IsDead())
		return;

	bool arena = false;
	m_PersonLock.lockForWrite();
	if(m_ArenaExitInv.VoidOwnershipTitle(propertyId, ownerId, ownerName))
		arena = true;
	m_PersonLock.unlock();

	bool gear = false;
	m_GearLock.lockForWrite();
	if(m_Gear.VoidOwnershipTitle(propertyId, ownerId, ownerName))
	{
		gear = true;
		m_tmGearChange.Set();	
	}
	m_GearLock.unlock();

	bool seachest = false;
	m_SeaChestLock.lockForWrite();
	if(m_SeaChest.VoidOwnershipTitle(propertyId, ownerId, ownerName))
	{
		seachest = true;
		m_tmSeaChestChange.Set();	
	}
	m_SeaChestLock.unlock();

	if(arena || gear || seachest)
	{
		SetModified();

		if(arena)
			Thought(tr("A title to %1 %2, in the gear I checked upon entry to the arena, was claimed by %3 %4. It is now useless.")
					.arg(propertyName)
					.arg(propertyId.StringId())
					.arg(ownerName)
					.arg(ownerId.StringId()));

		if(gear)
			Thought(tr("A title to %1 %2, in my gear, was claimed by %3 %4. It is now useless.")
					.arg(propertyName)
					.arg(propertyId.StringId())
					.arg(ownerName)
					.arg(ownerId.StringId()));

		if(seachest)
			Thought(tr("A title to %1 %2, in my sea chest, was claimed by %3 %4. It is now useless.")
					.arg(propertyName)
					.arg(propertyId.StringId())
					.arg(ownerName)
					.arg(ownerId.StringId()));
	}
}

void DaPerson::AddBounty(const AuId& posterId, const QString& posterName, const QString& reason, double reward, int why)
{
	DaSentient::AddBounty(posterId, posterName, reason, reward, why);
	switch(why)
	{
	case 0:		// Player put a bounty on me.
		break;
	case 1:
		Stats().ShipmentMineJettisoned();
		break;
	case 2:
		Stats().ShipmentJettisonedByMe();
		break;
	case 3:
		Stats().ShipmentJettisonedByOther();
		break;
	case 4:
		Stats().ShipmentMineOpened();
		break;
	case 5:
		Stats().ShipmentOpenedByMe();
		break;
	case 6:
		Stats().ShipmentOpenedByOther();
		break;
	default:
		break;
	}
}

DaAtom::ATOM_TYPE DaPerson::AtomType() const
{
	return ATOM_Avatar;
}

DaBd* DaPerson::BdBuildFence(const AuPoint& startLoc, const AuPoint& endLoc, int fenceStyle, const AuId& platformId,
		quint32 customDesignId, int mat)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Determine construction material and validate fence style.
	COMMODITYID material;
	double postSpacing;
	AuId archId;
	QString archName;
	QString designName;
	QString designDesc;
	switch((AuScFence::STYLE)fenceStyle)
	{
	case AuScFence::STYLE_Count:
	default:
		return 0;
	case AuScFence::Posts:
	case AuScFence::Wire:
	case AuScFence::Range:
	case AuScFence::SplitRail:
		material = CID_Wood;
		postSpacing = AuScFence::FencePostSpacing((AuScFence::STYLE)fenceStyle);
		break;
	case AuScFence::WoodYard:
		material = CID_Lumber;
		postSpacing = AuScFence::FencePostSpacing((AuScFence::STYLE)fenceStyle);
		break;
	case AuScFence::PostsT:
	case AuScFence::WireT:
	case AuScFence::RangeT:
	case AuScFence::ChainLink:
	case AuScFence::ChainLinkRazorWire:
		material = CID_Metal;
		postSpacing = AuScFence::FencePostSpacing((AuScFence::STYLE)fenceStyle);
		break;
	case AuScFence::SandBagWall:
		material = CID_Textiles;
		postSpacing = AuScFence::FencePostSpacing((AuScFence::STYLE)fenceStyle);
		break;
	case AuScFence::BrickWall:
	case AuScFence::StoneWall:
		material = CID_Stone;
		postSpacing = AuScFence::FencePostSpacing((AuScFence::STYLE)fenceStyle);
		break;
	case AuScFence::Custom:
		material = (COMMODITYID)mat;
		{
			AuBdDesign customDesign;
			const DgData& bdd = g_Au.LockBdDesignsForRead();
			bool hasDesign = bdd.BdDesign(customDesignId, customDesign);
			g_Au.UnlockBdDesigns();
			if(!hasDesign)
			{
				Thought(tr("Fence blueprint %1 is not loaded; the server is loading it. "
							"I should try again in a moment.").arg(customDesignId));
				return 0;
			}
			else
			{
				archId = customDesign.ArchitectId();
				archName = customDesign.ArchitectName();
				designName = customDesign.Name();
				designDesc = customDesign.Description();
				AuScFaceList section;
				customDesign.VisibleFaces(section, AuScPart::GroupNameFenceSection());
				AuPoint smin,smax;
				section.Extents(smin,smax);
				postSpacing = smax.x;
			}
		}
		break;
	}

	// Create a new fence.
	DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return 0;
	DaBd* platform;
	if(!platformId.IsNull())
		platform = world->FindBd(platformId);
	else
		platform = 0;

	AuMatrix4x4 mBld;
	world->BuildingConstructionTransform(startLoc, 0, platform, AuPoint(0,0), mBld);

	// Create a design of the fence.
	AuBdDesign design;
	AuSegmentArray fenceSegs;
	AuSegment(startLoc, endLoc).SplitEvenly(postSpacing, fenceSegs);
	fenceSegs.Transform(mBld.Inverse());			// Segs are now relative to the building's transform.
	if(!design.CreateFence(fenceSegs, (AuScFence::STYLE)fenceStyle, customDesignId, designName, designDesc, archId, archName))
		return 0;

	quint32 idEmpire = EmpireId();
	INDUSTRYID industryId = IID_Fence;

	AuBdReport report;
	design.ReportDesign(report);
	report.SetConstructionMaterial(material);
	DaBd* exDev;
	QString cityName;
	int testResult = world->TestBdLocation(industryId, idEmpire, &report, material, endLoc, 0, 0, platform, exDev, &startLoc,
			cityName);
	if(testResult != 0)
	{
		Thought(DaWorldLL::TestBdResultText(testResult));
		return 0;
	}

	quint32 sitePrepLabor = 10;

	DaBd* building = new DaBd;
	building->ConnectSignals();
	building->Create();
	building->SetPosAndOrient(AuAtomPos(world->Id(), mBld));
	ether->AddAtom(building);

	building->StartConstruction(industryId, report, sitePrepLabor, 0, false, Species());
	building->SetEmpireId(idEmpire);
	if(platform)
		building->SetPlatformId(platformId);	// Buildings know what platform they were built on, so they can be detroyed with it.
	building->SetFence(fenceStyle, fenceSegs);	// After platform id is established.
	g_Au.AtomAdded(building->Id());
	building->SetNormal();						// New atom is ready for normal thread processing.
	world->BdAdded();							// Update city presence. Adjust fences.
	world->JurisdictionJoin(building);			// Join a city if the nu building falls in its jurisdiction.

	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(world->FindEnvironmentAtom());
	if(system)
		system->UpdateTradeConnections();		// So we can import construction materials.

	// Player Statistics
	Stats().BuildingPlanned(industryId, world, 1);
	IncreaseSkill(AuSkill::CityPlanner, 1);
	building->TickTock();
	//qDebug() << "DaPerson::BdBuildFence" << *this << "fence created" << *building;
	return building;
}

DaBd* DaPerson::BdPreviewFence(const AuPoint& startLoc, const AuPoint& endLoc)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Determine construction material and validate fence style.
	DaDesigner* de = FindParentDesignerInEnv();
	if(!de)
		return 0;
	AuScFence::STYLE fenceStyle = AuScFence::Custom;	// Custom style.
	COMMODITYID material = CID_Stone;
	AuScFaceList section;
	de->VisibleFaces(section, AuScPart::GroupNameFenceSection());
	AuPoint smin,smax;
	section.Extents(smin,smax);
	double postSpacing = smax.x;

	// Create a preview fence.
	DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return 0;

	AuMatrix4x4 mBld;
	world->BuildingConstructionTransform(startLoc, 0, 0, AuPoint(0,0), mBld);

	// Create a design of the fence.
	AuBdDesign design;
	AuSegmentArray fenceSegs;
	AuSegment(startLoc, endLoc).SplitEvenly(postSpacing, fenceSegs);
	fenceSegs.Transform(mBld.Inverse());			// Segs are now relative to the building's transform.
	if(!design.CreateFence(fenceSegs, (AuScFence::STYLE)fenceStyle))
		return 0;

	AuBdReport report;
	design.ReportDesign(report);

	QString designName(report.DesignName());
	if(designName.isEmpty())
		designName = tr("Unnamed Design");

	report.SetConstructionMaterial(material);
	DaBd* exDev;
	//int testResult = world->TestBdLocation(IID_Fence, 0, &report, material, endLoc, 0, 0, 0, exDev, &startLoc);
	int testResult = world->TestBdPreviewLocation(&report, startLoc, 0, exDev);
	if(testResult != 0)
	{
		Thought(DaWorldLL::TestBdResultText(testResult));
		return 0;
	}

	DaBd* building = new DaBd;
	building->ConnectSignals();
	building->Create();
	building->SetPosAndOrient(AuAtomPos(world->Id(), mBld));
	ether->AddAtom(building);

	building->InitPreview(report, designName, EmpireId(), Species());
	building->SetFence(fenceStyle, fenceSegs);
	g_Au.AtomAdded(building->Id());
	building->SetNormal();						// New atom is ready for normal thread processing.
	world->BdAdded();							// Update city presence. Adjust fences.

	//qDebug() << "DaPerson::BdBuildFence" << *this << "fence created" << *building;
	building->TickTock();
	return building;
}

bool DaPerson::GainExperienceForWorking()
{
	if(!DaSentient::GainExperienceForWorking())
		return false;	// Not time to gain experience.

	// People do not gain experience when not on line, unless they are AI controlled.
	if(!IsOnLine() && !IsAIControlled())
		return true;

	// Gain 1 experience point for doing my job.
	if(InDesignEnvironment())
	{
		if(!IsAIControlled())
			IncreaseSkill(AuSkill::Architect, 1);
	}
	else if(IsAboardVehicle())
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
		if(vehicle && vehicle->Driver() == this)
			IncreaseSkill(vehicle->DriverSkill(), 1);
	}
	else switch(ConsoleType())
	{
	case AuStation::Berth_1_0:
	case AuStation::Berth_1_135:
	case AuStation::Berth_1_180:
	case AuStation::Berth_1_225:
	case AuStation::Berth_1_270:
	case AuStation::Berth_1_315:
	case AuStation::Berth_1_45:
	case AuStation::Berth_1_90:
	case AuStation::Berth_2_0:
	case AuStation::Berth_2_135:
	case AuStation::Berth_2_180:
	case AuStation::Berth_2_225:
	case AuStation::Berth_2_270:
	case AuStation::Berth_2_315:
	case AuStation::Berth_2_45:
	case AuStation::Berth_2_90:
	case AuStation::Berth_Captain:
	case AuStation::Berth_Crew:
	case AuStation::Berth_Officer:
	case AuStation::Berth_Passenger:
	case AuStation::Berth_Troop:
	case AuStation::Citizen:
	case AuStation::Console_Command:
	case AuStation::Console_Patient:
	case AuStation::Integrated_Command:
	case AuStation::Integrated_Designer:
	case AuStation::Integrated_Patient:
	case AuStation::Livestock:
	case AuStation::Table_Game:
	case AuStation::Table_Game_0:
	case AuStation::Table_Game_45:
	case AuStation::TYPE_Count:		// Not at a console.
		// No specific console. Gain command skill if I am a commander.
		if((HasTheCon() || IsOnLine()) && AuRosterEntry::IsOfficer(SpawnShipBerthType()))
			IncreaseSkill(AuSkill::Command, 1);
		else if(IsInsideSpacecraft())
			IncreaseSkill(AuSkill::SpacecraftOperations, 1);	// Generic spaceship skill just for being aboard.
		break;
	case AuStation::Console_Engine:
	case AuStation::Integrated_Engine:
		IncreaseSkill(AuSkill::Engineer, 1);
		break;
	case AuStation::Console_FireControl:
	case AuStation::Integrated_FireControl:
		IncreaseSkill(AuSkill::FireControl, 1);
		break;
	case AuStation::Console_Helm:
	case AuStation::Integrated_Helm:
		IncreaseSkill(AuSkill::Helm, 1);
		break;
	case AuStation::Console_Medic:
	case AuStation::Integrated_Medic:
		IncreaseSkill(AuSkill::Medic, 1);
		break;
	case AuStation::Console_Navigation:
	case AuStation::Integrated_Navigation:
		IncreaseSkill(AuSkill::Navigator, 1);
		break;
	case AuStation::Console_Power:
	case AuStation::Integrated_Power:
		IncreaseSkill(AuSkill::Power, 1);
		break;
	case AuStation::Console_Sensor:
	case AuStation::Integrated_Sensor:
		IncreaseSkill(AuSkill::Sensor, 1);
		break;
	case AuStation::Console_Shield:
	case AuStation::Integrated_Shield:
		IncreaseSkill(AuSkill::Shield, 1);
		break;
	case AuStation::Console_Transporter:
	case AuStation::Integrated_Transporter:
		IncreaseSkill(AuSkill::Transporter, 1);
		break;
	case AuStation::Turret_Integrated:
	case AuStation::Turret_Bottom:
	case AuStation::Turret_Top:
	case AuStation::Turret_0:
	case AuStation::Turret_45:
	case AuStation::Turret_90:
	case AuStation::Turret_135:
	case AuStation::Turret_180:
	case AuStation::Turret_225:
	case AuStation::Turret_270:
	case AuStation::Turret_315:
		IncreaseSkill(AuSkill::Turret, 1);
		break;
	case AuStation::Troop:
		IncreaseSkill(AuSkill::Guard, 1);
		break;
	}
	return true;
}

void DaPerson::InitSkillsFromStats()
{
	AuSkillSet skills;
	// Command
	int min = Stats().StationMinutes(AuStation::Console_Command, true) +
		Stats().StationMinutes(AuStation::Console_Command, false) +
		Stats().MissionOrders();
	int skill = (min+1) / 2;	// 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Command, skill);

	// Engineer
	min = Stats().StationMinutes(AuStation::Console_Engine, true) +
		Stats().StationMinutes(AuStation::Console_Engine, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Engineer, skill);

	// FireControl
	min = Stats().StationMinutes(AuStation::Console_FireControl, true) +
		Stats().StationMinutes(AuStation::Console_FireControl, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::FireControl, skill);

	// Helm
	min = Stats().StationMinutes(AuStation::Console_Helm, true) +
		Stats().StationMinutes(AuStation::Console_Helm, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Helm, skill);

	// Navigator
	min = Stats().StationMinutes(AuStation::Console_Navigation, true) +
		Stats().StationMinutes(AuStation::Console_Navigation, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Navigator, skill);

	// Power
	min = Stats().StationMinutes(AuStation::Console_Power, true) +
		Stats().StationMinutes(AuStation::Console_Power, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Power, skill);

	// Sensor
	min = Stats().StationMinutes(AuStation::Console_Sensor, true) +
		Stats().StationMinutes(AuStation::Console_Sensor, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Sensor, skill);

	// Shield
	min = Stats().StationMinutes(AuStation::Console_Shield, true) +
		Stats().StationMinutes(AuStation::Console_Shield, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Shield, skill);

	// Medic
	min = Stats().StationMinutes(AuStation::Console_Medic, true) +
		Stats().StationMinutes(AuStation::Console_Medic, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Medic, skill);

	// Transporter
	min = Stats().StationMinutes(AuStation::Console_Transporter, true) +
		Stats().StationMinutes(AuStation::Console_Transporter, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Transporter, skill);

	// Turret
	min = Stats().StationMinutes(AuStation::Turret_Integrated, true) +
		Stats().StationMinutes(AuStation::Turret_Integrated, false);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::Turret, skill);

	// VehicleDriver
	min = Stats().VehicleMinutes(CID_APC) +
		Stats().VehicleMinutes(CID_DirtBike) +
		Stats().VehicleMinutes(CID_FarmMachinery) +
		Stats().VehicleMinutes(CID_SailBoat) +
		Stats().VehicleMinutes(CID_SUV);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::VehicleDriver, skill);

	// VehiclePilot
	min = Stats().VehicleMinutes(CID_AdvSpaceFighter) +
		Stats().VehicleMinutes(CID_Helicopter) +
		Stats().VehicleMinutes(CID_SpaceFighter) +
		Stats().VehicleMinutes(CID_SpaceRocket) +
		Stats().VehicleMinutes(CID_SpaceTransport) +
		Stats().VehicleMinutes(CID_Submarine);
	skill = (min+1) / 2;    // 1 skill point every 2 minutes
	if(skill)
		skills.IncreaseSkill(AuSkill::VehiclePilot, skill);

	// Guard

	// Handcraft
	quint32 handcraft = Stats().ItemsHandCrafted();
	if(handcraft)
		skills.IncreaseSkill(AuSkill::Handcraft, handcraft);

	// Architect
	quint32 arch = Stats().ArchitectScore();
	if(arch)
		skills.IncreaseSkill(AuSkill::Architect, arch);

	// SpacecraftCapture
	quint32 spacecraftCapture = Stats().SpacecraftCaptureScore();
	if(spacecraftCapture)
		skills.IncreaseSkill(AuSkill::SpacecraftCapture, spacecraftCapture);

	// SpacecraftCombat

	// SpaceExploration
	quint32 explore = Stats().ExploreScore();
	if(explore)
		skills.IncreaseSkill(AuSkill::SpaceExploration, explore);

	// Combat

	// Construction

	// Manufacture

	// Trade

	// EVSuit
	quint32 eva = Stats().EVAMinutes();
	if(eva)
		skills.IncreaseSkill(AuSkill::EVSuit, eva);

	// Forage
	quint32 forage = Stats().ItemsForaged();
	if(forage)
		skills.IncreaseSkill(AuSkill::Forage, forage);

	// CityPlanner
	quint32 cityPlan = Stats().BuildingsPlanned();
	if(cityPlan)
		skills.IncreaseSkill(AuSkill::CityPlanner, cityPlan);

	// BountyHunter
	quint32 bounties = Stats().BountiesCollected();
	if(bounties)
		skills.IncreaseSkill(AuSkill::BountyHunter, bounties);

	// VehicleCapture
	quint32 vehCap = Stats().VehiclesCaptured();
	if(vehCap)
		skills.IncreaseSkill(AuSkill::VehicleCapture, vehCap);

	// VehicleCombat
	quint32 vused = Stats().VehiclesUsedAsWeapon();
	if(vused)
		skills.IncreaseSkill(AuSkill::VehicleCombat, vused);

	// SquadLeader
	quint32 sords = Stats().SquadOrders();
	if(sords)
		skills.IncreaseSkill(AuSkill::SquadLeader, sords);

	// Exobiology
	quint32 named = Stats().CreatureSpeciesNamed();
	if(named)
		skills.IncreaseSkill(AuSkill::Exobiology, named);

	// Butcher

	m_SkillsLock.lockForWrite();
	m_Skills = skills;
	m_SkillsLock.unlock();
}

quint32 DaPerson::JobExperiencePoints() const
{
	return TotalExperiencePoints();
}

//static
quint32 DaPerson::PromotionGoal(quint8 rank)
{
	quint32 promotions[6] =
	{
		5000,							// Points needed to be promoted to Lieutenant Junior Grade.
		20000,							// Points needed to be promoted to Lieutenant
		80000,							// Points needed to be promoted to Commander.
		320000,							// Points needed to be promoted to Captain.
		640000,							// Points needed to be promoted to Admiral.
		0								// No promotion beyond Admiral.
	};
	return promotions[rank];
}

bool DaPerson::DomainAdd(const AuId& id, const QString& name)
{
	m_PersonLock.lockForWrite();
	bool didIt = m_Domain.Add(id, name);
	if(didIt)
		m_DomainChange.Set();
	m_PersonLock.unlock();
	if(didIt)
		SetModified();
	return didIt;
}

bool DaPerson::DomainRemove(const AuId& id)
{
	m_PersonLock.lockForWrite();
	bool didIt = m_Domain.Remove(id);
	if(didIt)
		m_DomainChange.Set();
	m_PersonLock.unlock();
	if(didIt)
		SetModified();
	return didIt;
}

bool DaPerson::CloseHelmetVisor()
{
	bool closedIt = false;
	const AuGear* gear = LockGearForRead();
	const AuItem* helmet = gear->HelmetItemWorn();
	if(helmet && helmet->HitPoints() > 1 && helmet->IsEnvironmentHelmet() && helmet->State() == 0)
	{
		// I am wearing a helmet. Seal is intact. Visor is open.
		AuGearPos pos = gear->FindHelmetWorn();
		bool posValid = gear->IsPosValid(pos);
		UnlockGear();
		if(posValid && !UseGearAt(pos))
			closedIt = true;	// Helmet did not break.
	}
	else
		UnlockGear();
	return closedIt;
}

int DaPerson::IsProtectedByEVSuit() const
{
	// Check for leaky environment suit.
	const AuGear* gear = LockGearForRead();
	const AuItem* armor = gear->ArmorItemWorn();
	if(!armor || armor->HitPoints() <= 1)
	{
		// No suit or suit seal is broken.
		UnlockGear();
		return 2;
	}

	// Check for leaky environment helmet.
	const AuItem* helmet = gear->HelmetItemWorn();
	if(!helmet || !helmet->IsEnvironmentHelmet() || helmet->HitPoints() <= 1)
	{
		// No helmet, helmet is not env, or seal is broken.
		UnlockGear();
		return 2;
	}

	// Check for helmet visor open.
	if(helmet->State() != 1)
	{
		// Helmet visor is open.
		UnlockGear();
		return 1;
	}
	UnlockGear();
	return 0;	// ok
}

int DaPerson::IsProtectedByDivingGear() const
{
	// Check for leaky environment helmet.
	const AuGear* gear = LockGearForRead();
	const AuItem* helmet = gear->HelmetItemWorn();
	if(!helmet || !helmet->IsEnvironmentHelmet() || helmet->HitPoints() <= 1)
	{
		// No helmet, helmet is not env, or seal is broken.
		UnlockGear();
		return 2;
	}

	// Check for helmet visor open.
	if(helmet->State() != 1)
	{
		// Helmet visor is open.
		UnlockGear();
		return 1;
	}
	UnlockGear();
	return 0;	// ok
}

// 'type' 0=animal encounter, 1=pirate station, 2=pirate ship, 3=load officer, 4=load crew, 5=load troops, 6=load passengers
void DaPerson::AgentEncounter(quint8 type, quint32 designId)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Found the agent.
	DaSolarSystem* system = FindParentSolarSystemInEnv();
	if(!system)
		return;
	QString response;
	if(type == 0)
	{
		// Animal Encounter
		int result;
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld)
			result = llworld->AnimalEncounter(GlobeLL(), 1);	// Any animal
		else
			result = 4;

		switch(result)
		{
		default:
			response = tr("%1 animals spawned.").arg(-result);
			break;
		case 1:
			response = tr("No animals spawned. Animal encounters are disabled.");
			break;
		case 2:
			response = tr("No animals spawned. No creature or thing is present in the area.");
			break;
		case 3:
			response = tr("No animals spawned. The animal encounter table was broken and it had to be recreated. "
					"Try agin.");
			break;
		case 4:
			response = tr("No animals spawned. I am not on a planet.");
			break;
		case 5:
			response = tr("No animals spawned. Planet cannot have animals.");
			break;
		case 6:
			response = tr("No animals spawned. Planet does not have animals.");
			break;
		case 7:
			response = tr("No animals spawned. Person online not found in the area.");
			break;
		case 8:
			response = tr("No animals spawned. Maximum animal population is already present.");
			break;
		case 9:
			response = tr("No animals spawned. A development exists at the chosen spawn location.");
			break;
		case 10:
			response = tr("No animals spawned. Terrain area has no aerial creatures at this time of day.");
			break;
		}
	}
	else if(type == 1)
	{
		// Pirate
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			//  0=pirate encountered, base in system
			//  1=pirate encountered, base in sector, not in system
			//  1000+=probability of encounter (+1000)
			// -1=no pirate empires ready
			// -2=probability of encounter is 0 or less
			// -11=no pirate base in sector yet, encounter triggered
			// -12=pirate base not expected to be in this system but it is
			// -13=encounter location landed outside the system
			// -14=no pirate ship design
			// -15=pirate ship design selected but it wasn't loaded
			int result = system->PirateEncounter(ship, true);
			switch(result)
			{
			case 0:
				response = tr("Pirate encountered. Pirate base is in this system.");
				break;
			case 1:
				response = tr("Pirate encountered. Pirate base is in this sector but not in this system.");
				break;
			default:
				if(result > 1000)
				{
					int prob = result - 1000;
					response = tr("No pirate encountered. Probability of encounter is %1%.").arg(prob);
				}
				else
					response = tr("No pirate encountered. Unexpected result %1.").arg(result);
				break;
			case -1:
				response = tr("No pirate empires are ready for encounters.");
				break;
			case -2:
				response = tr("No pirate encountered. The probability is less than or equal 0%.");
				break;
			case -11:
				response = tr("No pirate base exists in this sector. "
						"The manager was notified; check its output to see if a pirate base appeared in the sector. ");
				break;
			case -12:
				response = tr("No pirate encountered. A pirate base is unexpectedly present in this system.");
				break;
			case -13:
				response = tr("No pirate encountered. The encounter location landed outside the current solar system.");
				break;
			case -14:
				response = tr("No pirate encountered. No spacecraft designs.");
				break;
			case -15:
				response = tr("No pirate encountered. Spacecraft design selected is not loaded.");
				break;
			}
		}
		else
			response = tr("I must board a spacecraft to trigger a pirate encounter.");
	}
	else if(type == 2)
	{
		// Asteroid
		DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			// Start the doomsday asteroid.
			int minutesToImpact = designId;			// 'designId' is the minutes before the asteroid is to hit.
			AuTime timeOnTarget(AuTime::Now());
			timeOnTarget = timeOnTarget.AddMins(minutesToImpact);
			AuPoint startAt = AuParticleSystem::GetRandomVector();

			// Calculate the distance so normal speed gets the asteroid to the target on time. The asteroid continuously
			// modulates its speed to arrive at the target on time. This calc will make it modulate to a speed close to
			// normal asteroid speed.
			double distance = minutesToImpact * 60 * DaAsteroid::TargetPursueSpeed();
			if(distance > FeetPerParsec()/2)
				distance = FeetPerParsec()/2;	// Don't fall into the next solar system.
			startAt.Unit(distance);
			AuMatrix4x4 t;
			t.Translate(startAt);

			// Calculate release time so asteroid comes screaming in at high speed, instead of crawling. Slow asteroids are
			// boring. Too fast and they strike too quickly to have any fun.
			qint64 secsTravel = (qint64)(distance / DaAsteroid::TargetPursueSpeed());	// Time to travel the dist at TPS.
			AuTime timeOfRelease = timeOnTarget.AddSecs(-secsTravel);	// If earlier than now, the asteroid will start now.

			DaAsteroid* roid = new DaAsteroid(world->Id(), timeOnTarget, timeOfRelease);
			roid->ConnectSignals();
			roid->Create();
			roid->SetPosAndOrient(AuAtomPos(system->Id(), t));
			ether->AddAtom(roid);

			roid->SetNormal();	// New atom is ready for normal thread processing.
			g_Au.AtomAdded(roid->Id());
			response = tr("Asteroid heading for %1 will impact at %2.")
				.arg(world->Name())
				.arg(timeOnTarget.UTC().toString());
			roid->TickTock();
		}
		else
			response = tr("I must be at a world to conjure an asteroid.");
	}
	else if(type == 3)
	{
		// Spawn aerial creatures only.
		int result;
		DaWorldLL* llworld = FindParentWorldInEnv();
		if(llworld)
			result = llworld->AnimalEncounter(GlobeLL(), 2);	// Aerial animal only
		else
			result = 4;

		switch(result)
		{
		default:
			response = tr("%1 animals spawned.").arg(-result);
			break;
		case 1:
			response = tr("No animals spawned. Animal encounters are disabled.");
			break;
		case 2:
			response = tr("No animals spawned. No creature or thing is present in the area.");
			break;
		case 3:
			response = tr("No animals spawned. The animal encounter table was broken and it had to be recreated. "
					"Try agin.");
			break;
		case 4:
			response = tr("No animals spawned. I am not on a planet.");
			break;
		case 5:
			response = tr("No animals spawned. Planet cannot have animals.");
			break;
		case 6:
			response = tr("No animals spawned. Planet does not have animals.");
			break;
		case 7:
			response = tr("No animals spawned. Person online not found in the area.");
			break;
		case 8:
			response = tr("No animals spawned. Maximum animal population is already present.");
			break;
		case 9:
			response = tr("No animals spawned. A development exists at the chosen spawn location.");
			break;
		case 10:
			response = tr("No animals spawned. Terrain area has no aerial creatures at this time of day.");
			break;
		}
	}
	else if(type == 4)
	{
		// Load officer.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			if(ship->OfficerBerthsAvailable())
			{
				// Load an officer.
				DaNpc* officer = ship->LoadPirateOfficer(ship->EmpireId(), Species());
				if(!ship->IsPirate())
					officer->SetHomeWorld(AuId(1), Name(), BODY_TYPE_Count, system->Id(), system->Name(),
							system->FindParentSector()->Id(), system->FindParentSector()->Name(), system->Galaxy());
				response = tr("%1 came aboard.").arg(officer->Name());
			}
			else
				response = tr("This ship has no officer berth available.");
		}
		else
			response = tr("I must board a spacecraft to load an officer.");
	}
	else if(type == 5)
	{
		// Load crew.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			int berths = ship->CrewBerthsAvailable();
			if(berths)
			{
				QString names;
				while(berths--)
				{
					// Load a crewman.
					DaNpc* crew = ship->LoadPirateCrew(ship->EmpireId(), Species());
					if(names.isEmpty())
						names = crew->Name();
					else if(!berths)
						names += " and " + crew->Name();
					else
						names += ", " + crew->Name();
				}
				response = tr("%1 came aboard.").arg(names);
			}
			else
				response = tr("This ship has no crew berths available.");
		}
		else
			response = tr("I must board a spacecraft to load crew.");
	}
	else if(type == 6)
	{
		// Load troops.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			int berths = ship->TroopBerthsAvailable();
			if(berths)
			{
				QString names;
				while(berths--)
				{
					// Load a troop.
					DaNpc* troop = ship->LoadPirateTroop(ship->EmpireId(), Species());
					if(!troop)
						continue;
					if(names.isEmpty())
						names = troop->Name();
					else if(!berths)
						names += " and " + troop->Name();
					else
						names += ", " + troop->Name();
				}
				response = tr("%1 came aboard.").arg(names);
			}
			else
				response = tr("This ship has no troop berths available.");
		}
		else
			response = tr("I must board a spacecraft to load troops.");
	}
	else if(type == 7)
	{
		// Load passengers.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			int berths = ship->PassengerBerthsAvailable();
			if(berths)
			{
				QString names;
				while(berths--)
				{
					// Load a passenger.
					DaCitizen* passenger = ship->LoadPiratePassenger(ship->EmpireId(), Species());
					if(names.isEmpty())
						names = passenger->Name();
					else if(!berths)
						names += " and " + passenger->Name();
					else
						names += ", " + passenger->Name();
				}
				response = tr("%1 came aboard.").arg(names);
			}
			else
				response = tr("This ship has no passenger berths available.");
		}
		else
			response = tr("I must board a spacecraft to load passengers.");
	}
	else if(type == 8)
	{
		// Equip agent's current ship as a pirate.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			if(ship->OfficerBerthsAvailable())
			{
				ship->SetupPirate(EmpireId(), ship->QualityMade());
				ship->SetModified();
				response = tr("An officer and his crew of scaly wags came aboard. "
						"%1 is rigged and loaded for action.").arg(ship->NameWithEmpire());
			}
			else
				response = tr("I must board a spacecraft with officer berths to equip it as a pirate.");
		}
		else
			response = tr("I must board a spacecraft to equip it as a pirate.");
	}
	else if(type == 9)
	{
		// Monster encounter aboard a ship.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			int monsters = ship->MonsterEncounterNow();
			response = tr("%1 monsters spawned aboard %2.").arg(monsters).arg(ship->NameWithEmpire());
		}
		else
			response = tr("I must board a spacecraft to trigger a monster encounter.");
	}
	else if(type == 10 || type == 11 || type == 12)
	{
		// Indigenout encounter.
		int result;
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld)
		{
			const DaPerson* encperson;
			const DaPerson* mount = dynamic_cast<const DaPerson*>(Parent());
			if(mount)
				encperson = mount;
			else
				encperson = this;
			if(type == 10)
			{
				// Indigenout encounter forced even if too recent.
				result = llworld->IndigenousEncounter(*encperson, encperson->GlobeLL(), 1);
			}
			else if(type == 11)
			{
				// Indigenout encounter force move of village if necessary.
				result = llworld->IndigenousEncounter(*encperson, encperson->GlobeLL(), 2);
			}
			else
			{
				// Indigenout encounter force move of village if necessary.
				// Reset encperson's reputation.
				llworld->VillageResetReputation(encperson->Id());
				result = llworld->IndigenousEncounter(*encperson, encperson->GlobeLL(), 2);
			}
		}
		else
			result = 99;

		switch(result)
		{
		default:
			response = tr("No indigenous spawned. Unknown result code was %1.").arg(result);
			break;
		case 0:
			response = tr("Indigenous encountered. Their village is at %1.").arg(llworld->VillageLL().ToString());
			break;
		case 99:
			response = tr("No indigenous spawned. I am not on a planet.");
			break;
		case 1:
			response = tr("No indigenous spawned. Indigenous encounters are disabled.");
			break;
		case 2:
			response = tr("No indigenous spawned. This planet is not habitable.");
			break;
		case 3:
			response = tr("No indigenous spawned. This planet is not inhabited.");
			break;
		case 4:
			response = tr("No indigenous spawned. This worker zone on this planet is not inhabited.");
			break;
		case 5:
			response = tr("No indigenous spawned. No creature or thing is present in this area.");
			break;
		case 6:
			response = tr("No indigenous spawned. No place for village near %1.").arg(GlobeLL().ToString());
			break;
		case 7:
			response = tr("No indigenous spawned. Village is too far away.");
			break;
		case 8:
			response = tr("No indigenous spawned. Village is too close.");
			break;
		case 9:
			response = tr("No indigenous spawned. Village had encounter recently.");
			break;
		}
	}
	else
		response = tr("Hocus pocus mumbo jumbo.");

	if(!response.isEmpty())
		Thought(response);
}

bool DaPerson::IsProtectedByEnvironmentSuit() const
{
	return IsProtectedByEVSuit() == 0;
}

bool DaPerson::IsCivilian() const
{
	return EmpireId() == 0;
}

bool DaPerson::IsEthereal() const
{
	if(IsInvisibleOffLine())
		return true;
	if(m_AgentBits & (AGENT_Entity|AGENT_Invisible))
		return true;
	if(IsNewbornHighborn())
		return true;
	return false;
}

bool DaPerson::IsOffLine30Days() const
{
	if(g_Au.IsSceneClient() && IsLineageDemiavatar())
		return false;	// Client does not receive last online time. Client only receives demis whose account has been active.
	return LastOnLine().ElapsedDay() > 30;
}

bool DaPerson::IsInvisibleOffLine() const
{
	if(IsLineageDemiavatar())
	{
		// Demiavatar is not visible when its login account is no longer active.
		return IsOffLine30Days();
	}
	else
	{
		// Full avatar is not visible when it is not on line.
		return !IsOnLine();
	}
}

NPC_ROLE DaPerson::NpcRole() const
{
	if(IsAIControlled())
	{
		switch(SpawnShipBerthType())
		{
		case AuRosterEntry::Invalid:
			break;
		case AuRosterEntry::Captain:
		case AuRosterEntry::CaptainSlot:
		case AuRosterEntry::OfficerHuman:
		case AuRosterEntry::OfficerAI:
			return ROLE_Officer;
		case AuRosterEntry::Crew:
			return ROLE_Crew;
		case AuRosterEntry::PassengerAI:
		case AuRosterEntry::PassengerHuman:
			return ROLE_Passenger;
		case AuRosterEntry::Troop:
			return ROLE_Troop;
		}
	}
	return ROLE_Count;
}

bool DaPerson::IsAIControlled() const
{
	// Only demiavatars can be AI controlled.
	if(!IsLineageDemiavatar())
		return false;

	// Server sends client the current order when AI controlled.
	if(g_Au.IsSceneClient())
		return HasOrder();

	// Demiavatar is not AI controlled when the avatar is on line.
	if(IsOnLine())
		return false;

	// Demiavatar stops being AI controlled after 30 days of not being on line.
	// Server stops including them in the scene, too.
	if(IsOffLine30Days())
		return false;

	// Demiavatar is not on line and hasn't been off line too long.
	return true;
}

bool DaPerson::RecallCharacter(const QString& charName, AuMemCharacterDNA4& mem) const
{
	m_MemoryLock.lockForRead();
	bool recalled = m_Memories.RecallCharacter(charName, mem);
	m_MemoryLock.unlock();
	return recalled;
}

void DaPerson::CommAccept(const AuCommAccept& msg)
{
	// The 'accepter' is the seller, the 'bidder' is the buyer, which may be a person or a city.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Seller is not in a solar system? Problem!

	// Buyer and seller must be different.
	AuId idBuyer = msg.BuyerId();
	if(Is(idBuyer))
		return;

	DaAtom* buyer = system->FindIdRecursive(idBuyer);
	if(!buyer)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::BuyerSellerSameSystem, msg.ChannelName(), Id(),
				Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Buyer not found in the same solar system; moved away?
	}

	// Must locate the exact bid in the list of active bids.
	if(!g_Au.HasPendingBid(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::BidNotActive, msg.ChannelName(), buyer->Id(), buyer->Name(), msg.FromSectorId(),
				msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; bid is no longer active.
	}

	// The seller must be aboard a ship or must be
	// in a grocery, cantina, or retail store.
	DaVessel* sellerShip = 0;
	if(IsInSpacecraft())
		sellerShip = FindParentVesselInEnv();

	// Prepare the cargo item.
	if(!msg.Item())
		return;	// Invalid data.
	AuItem* iNew = msg.Item()->Clone();
	if(!iNew)
		return;	// wtf?
	quint32 totalcost = msg.Price();
	QString empireNameOfOrigin = msg.EmpireOfOrigin();

	// Validate the transaction based on the buyer.
	DaBd* buyerBd = dynamic_cast<DaBd*>(buyer);				// Set if buyer is a bd; else 0.
	DaPerson* buyerPerson = dynamic_cast<DaPerson*>(buyer);	// Set if buyer is a person; else 0.
	DaNpc* buyerChar = dynamic_cast<DaNpc*>(buyer);			// Set if buyer is a story character; else 0.
	DaVessel* buyerShip = 0;								// Set if buyer is a person aboard a ship; else 0.
	if(buyerBd)
	{
		// Insure physical locations permit acceptance of the offer.
		// - Seller ship must be in transporter range of the city, or
		// - Seller ship must be on a road slab that belongs to the city, or
		// - Seller ship Near a friendly station in this solar system, or
		// - Seller in a cantina, grocery, or retail store in the city.
		if(sellerShip)
		{
			// Selling person is aboard a spacecraft.
			bool inCity = buyerBd->CanAccessByGround(sellerShip);	// True if I can access the ship by ground.
			bool hasStation = false;
			bool nearStation = false;
			if(!inCity && sellerShip->StationInTradeRange(sellerShip->RoughRadius(), hasStation))
				nearStation = true;	// Found a station in trade range.

			bool hasTransporter = false;
			bool inTransporterRange = false;
			if(sellerShip->FindParentPlanetInEnv() == buyer->FindParentPlanetInEnv())
			{
				// The ship is at this planet.
				// Allow trade if the ship can reach the city with its transporters.
				float transporterRange = sellerShip->MaxTransporterRange();
				if(transporterRange > 0)
				{
					transporterRange *= transporterRange;				// squared for faster test against squared distance
					hasTransporter = true;
					if(inCity)
						inTransporterRange = true;
					else
					{
						AuPoint cityLoc = buyer->Location();
						AuPoint shipLoc;
						sellerShip->GetLocationRelativeToParent(shipLoc, sellerShip->FindParentPlanetInEnv());
						if(cityLoc.Distance2(shipLoc) < transporterRange)
							inTransporterRange = true;
					}
				}
			}

			// Ignore Accept if the seller is too far away.
			if(iNew->IsPower())
			{
				if(!inCity)
				{
					AuCommCityMessage::MESSAGE msgId = AuCommCityMessage::MoveBdToSellPower;
					AuCommCityMessage mymsg(msgId, msg.ChannelName(), buyer->Id(), buyer->Name(), msg.FromSectorId(),
							msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
					g_Au.SendAtomChat(mymsg);
					delete iNew;
					return;
				}
			}
			else
			{
				if(!inCity && !nearStation && !inTransporterRange)
				{
					AuCommCityMessage::MESSAGE msgId;
					if(hasTransporter)
					{
						if(hasStation)
							msgId = AuCommCityMessage::MoveBdTransporterStationToSell;
						else
							msgId = AuCommCityMessage::MoveBdTransporterToSell;
					}
					else
					{
						if(hasStation)
							msgId = AuCommCityMessage::MoveBdStationToSell;
						else
							msgId = AuCommCityMessage::MoveBdToSell;
					}
					AuCommCityMessage mymsg(msgId, msg.ChannelName(), buyer->Id(), buyer->Name(), msg.FromSectorId(),
							msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
					g_Au.SendAtomChat(mymsg);
					delete iNew;
					return;
				}
			}
		}

		// Insure the buyer city can store the merchandise.
		// TestFitCommodity will adjust the count in iNew to the quantity the city can accept, potentially replacing
		// lesser quality stuff.
		if(buyerBd->CommunityTestFitCommodity(*iNew) != iNew->Count())
		{
			QString text = tr("We don't have room to add %1 units.").arg(iNew->Count());
			AuCommCityChat mymsg(text, msg.ChannelName(), buyer->Id(), buyer->Name(), msg.FromSectorId(), msg.FromGalaxy(),
					msg.ReceptionRange(), msg.Scope());
			g_Au.SendAtomChat(mymsg);
			delete iNew;
			return;	// Ignore; buyer has no room to store merchandise.
		}
	}
	else if(buyerPerson)
	{
		// Insure the buyer can afford the cost.
		buyerShip = buyerPerson->FindParentVesselInEnv();
		double fAccountBalance;
		bool isPersonal;
		if(buyerShip && buyerShip != sellerShip)
		{
			// Buyer is aboard a ship and it is not the same as the
			// seller's ship. They must be an officer to buy cargo.
			if(buyerShip->InChainOfCommand(buyerPerson))
			{
				fAccountBalance = buyerShip->AccountBalance();
				isPersonal = false;
			}
			else
			{
				AuCommCityMessage mymsg(AuCommCityMessage::MustBeOfficerToBuy, msg.ChannelName(), Id(),
						Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
				g_Au.SendAtomChat(mymsg);
				delete iNew;
				return;	// Ignore the offer; insufficient authority.
			}
		}
		else
		{
			// Buyer is not aboard a ship or buyer is aboard the same ship as the seller. This is a personal transaction.
			fAccountBalance = buyerPerson->AccountBalance();
			isPersonal = true;
		}
		if(totalcost > fAccountBalance)
		{
			QString text;
			if(isPersonal)
				text = tr("You do not have %L1%2.").arg(totalcost).arg(AuGlobal::CurrencySymbol());
			else
				text = tr("Your ship does not have %L1%2.").arg(totalcost).arg(AuGlobal::CurrencySymbol());
			AuCommCityChat mymsg(text, msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
					msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
			g_Au.SendAtomChat(mymsg);
			delete iNew;
			return;	// Ignore the offer; insufficient funds.
		}

		// Insure physical locations permit acceptance of the offer.
		// - Buyer in a space ship, and
		// - Buyer ship near seller's space ship.
		// or
		// - Buyer and seller not in a ship, or
		// - Buyer and seller aboard the same ship.
		if(sellerShip && buyerShip)
		{
			// Buyer and seller are aboard ships.
			if(sellerShip == buyerShip)
			{
				// Buyer and seller are aboard the same ship.
				// This is a personal transaction between them.
				AuPoint sellerLoc, buyerLoc;
				GetLocationRelativeToParent(sellerLoc, sellerShip);
				buyerPerson->GetLocationRelativeToParent(buyerLoc, buyerShip);
				buyerShip = 0;
				sellerShip = 0;
				if(sellerLoc.Distance(buyerLoc) > 20)
				{
					AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactPersonal, msg.ChannelName(), Id(),
							Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
					g_Au.SendAtomChat(mymsg);
					delete iNew;
					return;
				}
			}
			else
			{
				// Buyer and seller are aboard different ships.
				// This is a cargo transaction between them.
				AuPoint sellerLoc = sellerShip->EnvLocation();
				AuPoint buyerLoc = buyerShip->EnvLocation();
				float tradeRange = sellerShip->TransferRange(buyerShip);
				float distApart = sellerLoc.Distance(buyerLoc);
				if(distApart > tradeRange)
				{
					QString text(tr("Our space ships are too far apart to transfer the cargo."));
					AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactCargo, msg.ChannelName(), Id(),
							Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
					g_Au.SendAtomChat(mymsg);
					delete iNew;
					return;
				}
			}
		}
		else if(sellerShip || buyerShip)
		{
			// Buyer or seller is aboard a ship but the other is not.
			// We do not support this transaction.
			//DHH We could allow buying/selling ships cargo here.
			AuCommCityMessage mymsg(AuCommCityMessage::BadLocToTransact, msg.ChannelName(), Id(),
					Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
			g_Au.SendAtomChat(mymsg);
			delete iNew;
			return;
		}
		else
		{
			// Buyer and seller are not aboard ships.
			// This is a personal transaction between them.
			AuPoint sellerLoc = EnvLocation();
			AuPoint buyerLoc = buyerPerson->EnvLocation();
			if(sellerLoc.Distance(buyerLoc) > 20)
			{
				AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactItem, msg.ChannelName(), Id(),
						Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
				g_Au.SendAtomChat(mymsg);
				delete iNew;
				return;
			}
		}

		// Insure the buyer can store the merchandise.
		if(buyerShip && !buyerShip->CanAddCargo(*iNew))
		{
			QString text;
			if(iNew->CommodityId() == CID_Hydrogen)
				text = tr("My fuel cells don't have room to add %1 units.").arg(iNew->Count());
			else if(iNew->IsPower())
				text = tr("My capacitor doesn't have room to add %1 units.").arg(iNew->Count());
			else
				text = tr("My cargo hold doesn't have room to add %1 units.").arg(iNew->Count());
			AuCommCityChat mymsg(text, msg.ChannelName(), buyer->Id(), buyer->Name(), msg.FromSectorId(), msg.FromGalaxy(),
					msg.ReceptionRange(), msg.Scope());
			g_Au.SendAtomChat(mymsg);
			delete iNew;
			return;	// Ignore; buyer has no room to store merchandise.
		}
	}
	else if(buyerChar)
	{
		// Insure the buyer can afford the cost.
		double fAccountBalance;
		fAccountBalance = buyerChar->AccountBalance();
		if(totalcost > fAccountBalance)
		{
			QString text = tr("You do not have %L1%2.").arg(totalcost).arg(AuGlobal::CurrencySymbol());
			AuCommCityChat mymsg(text, msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
					msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
			g_Au.SendAtomChat(mymsg);
			delete iNew;
			return;	// Ignore the offer; insufficient funds.
		}

		// Insure physical locations permit acceptance of the offer.
		// This is a personal transaction between them.
		AuPoint sellerLoc = EnvLocation();
		AuPoint buyerLoc = buyerChar->EnvLocation();
		if(sellerLoc.Distance(buyerLoc) > 20)
		{
			AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactItem, msg.ChannelName(), Id(),
					Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
			g_Au.SendAtomChat(mymsg);
			delete iNew;
			return;
		}
	}

	// Locate the item for sale.
	bool transactionCompleted = false;
	if(sellerShip)
	{
		// Locate the item for sale on the seller's ship.
		if(iNew->CommodityId() == CID_Hydrogen)
		{
			// Fuel is taken from the seller's fuel cells.
			if(sellerShip->FuelQuantity() >= iNew->Count() &&
					sellerShip->FuelQuality() >= iNew->Quality())
				transactionCompleted = true;	// The seller still has the fuel.
		}
		else if(iNew->CommodityId() == CID_Electricity)
		{
			// Electricity is taken from the seller's capacitor.
			if(sellerShip->CapacitorLevel() >= iNew->Count())
				transactionCompleted = true;	// The seller still has the power.
		}
		else
		{
			// Cargo other than fuel and electricity is taken from the cargo hold.
			const AuItemHash& ssinv = sellerShip->LockCargoForRead();
			for(AuItemHash::const_iterator it = ssinv.find(iNew->CommodityId());
					it != ssinv.cend() && (*it).first == iNew->CommodityId(); it++)
			{
				if((*it).second->IsItem(*iNew) && (*it).second->Count() >= iNew->Count())
				{
					// The offered item has been located.
					transactionCompleted = true;
					break;	// No need to keep looking.
				}
			}
			sellerShip->UnlockCargo();
		}
	}

	AuGear* spinv = LockGearForWrite();

	if(!transactionCompleted)
	{
		// Did not locate the item on the ship, or there is no ship.
		sellerShip = 0;

		// Seller is a person. The buyer is a person or a city, not a ship.
		// Locate the item(s) for sale in the seller's gear.
		AuItemHash combinedInv;
		spinv->CombineItemsForSale(combinedInv);
		for(AuItemHash::const_iterator it = combinedInv.cbegin(); it != combinedInv.cend(); it++)
		{
			if((*it).second->IsItem(*iNew) && (*it).second->Count() >= iNew->Count())
			{
				// The offered item has been located in sufficient quantity.
				transactionCompleted = true;
				break;
			}
		}

		// When the buyer is a person, transfer the items to the buyer's inventory so we know how many of them fit.
		if(transactionCompleted && (buyerPerson || buyerChar))
		{
			// The items were found in the seller's inventory.
			// Transfer items from one person to the other.
			AuItem* iTransfer = iNew->Clone();
			int countToTransfer = iNew->Count();
			for(int ipos=0; ipos<spinv->size() && countToTransfer > 0; ipos++)
			{
				// Reposition in each loop iteration due to consuming items from my inventory.
				AuGear::const_iterator gi = spinv->cbegin() + ipos;
				const AuItemContent* contents = (*gi).Item().Content();
				if(contents && !contents->empty())
				{
					// Transfer the items in the container but not the container itself.
					bool keepSearching;
					do
					{
						keepSearching = false;	// Will be set true if we need to search the container again.
						for(AuItemContent::const_iterator cit = contents->cbegin(); cit != contents->cend(); cit++)
						{
							if((*cit).second->Content() && !(*cit).second->Content()->empty())
								continue;	// Skip non-empty containers.
							if((*cit).second->IsItem(*iNew))
							{
								// An offered item has been located in a container.
								if((int)(*cit).second->Count() > countToTransfer)
									iTransfer->SetCount(countToTransfer);
								else
									iTransfer->SetCount((*cit).second->Count());
								bool added;
								if(buyerPerson)
									added = buyerPerson->AddGearItem(*iTransfer);
								else
									added = buyerChar->AddGearItemUseDrop(*iTransfer, 1,false);
								if(added)
								{
									ConsumeGearNoLock(*spinv, AuGearPos(*gi, (*cit).first), iTransfer->Count());
									countToTransfer -= iTransfer->Count();

									// Must break out and restart the search in the container because consuming an item may have
									// invalidated the container contents iterator.
									keepSearching = true;
									break;
								}
							}
						}
					}
					while(countToTransfer > 0 && keepSearching);
				}
				else if((*gi).Item().IsItem(*iNew))
				{
					// An offered item has been located.
					// Transfer the top-level item or empty container.
					if((int)(*gi).Item().Count() > countToTransfer)
						iTransfer->SetCount(countToTransfer);
					else
						iTransfer->SetCount((*gi).Item().Count());
					bool added;
					if(buyerPerson)
						added = buyerPerson->AddGearItem(*iTransfer);
					else
						added = buyerChar->AddGearItemUseDrop(*iTransfer, 1,false);
					if(added)
					{
						ConsumeGearNoLock(*spinv, AuGearPos(*gi), iTransfer->Count());
						countToTransfer -= iTransfer->Count();
					}
				}
			}

			delete iTransfer;
			iTransfer = 0;

			if(countToTransfer == (int)iNew->Count())
			{
				// No items were transferred.
				UnlockGear();

				AuCommCityMessage mymsg(AuCommCityMessage::BuyerInventoryFull, msg.ChannelName(), Id(),
						Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
				g_Au.SendAtomChat(mymsg);
				delete iNew;
				return;	// Ignore the offer; no room to store the stuff.
			}
			else if(countToTransfer)
			{
				// Some items were transferred but not all of them fit.
				// Adjust values based on actual amount transferred.
				int countTransferred = iNew->Count() - countToTransfer;	// Count of items actually transferred.
				float unitPrice = (float)totalcost / iNew->Count();
				totalcost = (int)(unitPrice * countTransferred);		// Cost of items actually transferred.
				iNew->SetCount(countToTransfer);						// Adjust final count in nu item.

				QString text = tr("The buyer's inventory had room for only %1; cost adjusted to %L2%3.")
					.arg(countTransferred).arg(totalcost).arg(g_Au.CurrencySymbol());
				AuCommCityChat mymsg(text, msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
						msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
				g_Au.SendAtomChat(mymsg);
			}
		}
	}

	if(transactionCompleted)
	{
		// The cargo was found aboard the sellers ship or in their gear.
		// Perform the transaction.

		// Revoke the bid so it cannot be accepted again.
		g_Au.BidRevoked(idBuyer, msg.ChannelName());

		// Seller is a person/ship so take the cargo from them and put the money in their account.
		if(sellerShip)
		{
			// Player Statistics
			Stats().ItemSoldAsCargo(*iNew);
			IncreaseSkill(AuSkill::Trade, 1);

			// Credit ship amount after taxes and alert city of transaction.
			sellerShip->CreditAccountSales(totalcost);

			// Remove the item from their ship's cargo.
			// Hydrogen gets removed from fuel cells.
			sellerShip->RemoveCargo(*iNew, iNew->Count());
		}
		else
		{
			// Player Statistics
			Stats().ItemSoldAsInventory(*iNew);
			IncreaseSkill(AuSkill::Trade, 1);

			// This is an individual transaction.
			CreditAccount(totalcost);

			// Buyer
			if(!buyerPerson && !buyerChar)
			{
				// Remove the item(s) from the seller person's inventory.
				// This has already been done when the buyer is a person or character.
				int countToTransfer = iNew->Count();
				for(int ipos=0; ipos<spinv->size() && countToTransfer > 0; ipos++)
				{
					// Reposition in each loop iteration due to consuming items from my inventory.
					AuGear::const_iterator gi = spinv->cbegin() + ipos;
					const AuItemContent* contents = (*gi).Item().Content();
					if(contents && !contents->empty())
					{
						// Consume the items in the container but not the container itself.
						bool keepSearching;
						do
						{
							keepSearching = false;	// Will be set true if we need to search the container again.
							for(AuItemContent::const_iterator cit = contents->cbegin(); cit != contents->cend(); cit++)
							{
								if((*cit).second->Content() && !(*cit).second->Content()->empty())
									continue;	// Skip non-empty containers.
								if((*cit).second->IsItem(*iNew))
								{
									// An offered item has been located in a container.
									if((int)(*cit).second->Count() > countToTransfer)
									{
										ConsumeGearNoLock(*spinv, AuGearPos(*gi, (*cit).first),
												countToTransfer);
										countToTransfer = 0;
									}
									else
									{
										countToTransfer -= (*cit).second->Count();
										ConsumeGearNoLock(*spinv, AuGearPos(*gi, (*cit).first),
												(*cit).second->Count());

										// Must break out and restart the search in the container because consuming an item may
										// have invalidated the container contents iterator.
										keepSearching = true;
									}
									break;
								}
							}
						}
						while(countToTransfer > 0 && keepSearching);
					}
					else if((*gi).Item().IsItem(*iNew))
					{
						// An offered item has been located.
						// Consume the top-level item or empty container.
						if((int)(*gi).Item().Count() > countToTransfer)
						{
							ConsumeGearNoLock(*spinv, AuGearPos(*gi), countToTransfer);
							countToTransfer = 0;
						}
						else
						{
							countToTransfer -= (*gi).Item().Count();
							ConsumeGearNoLock(*spinv, AuGearPos(*gi), (*gi).Item().Count());
						}
					}
				}
			}
		}

		// Buyer
		if(buyerPerson)
		{
			// Buyer is a person/ship so take the money from their
			// account and put the cargo on their ship.
			// When the buyer is an individual, the item has
			// already been added to their inventory.

			// Hydrogen gets routed to fuel cells.
			if(buyerShip)
			{
				buyerShip->AddCargo(*iNew);
				buyerShip->DebitAccount(totalcost);
			}
			else
			{
				buyerPerson->DebitAccount(totalcost);
			}
		}
		else if(buyerChar)
		{
			// Buyer is a story character so take the money from their account.
			// The item has already been added to their inventory.
			buyerChar->DebitAccount(totalcost);
		}
		else if(buyerBd)
		{
			// Buyer is a city so add the cargo to the city's inventory.
			CargoDelivered(buyerBd->Id(), *iNew);
			buyerBd->CommunityAddCommodity(*iNew);
			// The transaction is complete.
			// Take the money from the city.
			buyerBd->CommunityDebitAccount(totalcost, AuBank::Civ_DebitPurchases);
		}
	}
	else
	{
		// The seller does not have the item for sale.
		AuCommCityMessage::MESSAGE msgId;
		if(sellerShip)
		{
			if(iNew->CommodityId() == CID_Hydrogen)
				msgId = AuCommCityMessage::ShipNoFuelToSell;
			else
				msgId = AuCommCityMessage::ShipNoCargoToSell;
		}
		else
			msgId = AuCommCityMessage::PersonalNoItemToSell;
		AuCommCityMessage mymsg(msgId, msg.ChannelName(), buyer->Id(), buyer->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
	}

	UnlockGear();

	delete iNew;
}

void DaPerson::CommAcceptBarter(const AuCommAcceptBarter& msg)
{
	// The 'accepter' is the seller, the 'bidder' is the buyer, which may be a person or a city.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Seller is not in a solar system? Problem!

	// Buyer and seller must be different.
	AuId idBuyer = msg.BuyerId();
	if(Is(idBuyer))
		return;

	DaAtom* buyer = system->FindIdRecursive(idBuyer);
	if(!buyer)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::BuyerSellerSameSystem, msg.ChannelName(), Id(),
				Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Buyer not found in the same solar system; moved away?
	}

	// Must locate the exact bid in the list of active bids.
	if(!g_Au.HasPendingBidBarter(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::BidNotActive, msg.ChannelName(), buyer->Id(), buyer->Name(), msg.FromSectorId(),
				msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; bid is no longer active.
	}

	// Only people make barter offers.
	// Cities do not make barter offers. They depend on the player to decide what they want.
	DaPerson* buyerPerson = dynamic_cast<DaPerson*>(buyer);
	if(!buyerPerson)
		return;

	// Insure the buyer and seller still have the item(s) they offered.
	if(!buyerPerson->HasGearItems(*msg.OfferItem()))
	{
		QString text(tr("You do not have %1.").arg(msg.OfferItem()->ListDesc()));
		AuCommCityChat mymsg(text, msg.ChannelName(), Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(),
				msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}
	else if(!HasGearItems(*msg.Item()))
	{
		QString text(tr("You do not have %1.").arg(msg.Item()->ListDesc()));
		AuCommCityChat mymsg(text, msg.ChannelName(), buyerPerson->Id(), buyerPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Complete the transaction.
	// Revoke the bid so it cannot be accepted again.
	g_Au.BidRevoked(idBuyer, msg.ChannelName());

	// Remove the offered items from the buyer's inventory and the item for sale from the seller's inventory, to make room to
	// add nu items. We have a copy of the offered items on the message.
	buyerPerson->RemoveGearItems(*msg.OfferItem());
	RemoveGearItems(*msg.Item());

	// Add the traded items.
	buyerPerson->AddGearItems(*msg.Item());
	AddGearItems(*msg.OfferItem());
}

void DaPerson::CommAcceptRepair(const AuCommAcceptRepair& msg)
{
	// The 'accepter' is the person who needs repairs to their ship;
	// the 'bidder' is the city that offered the repairs.

	// Locate the city.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Seller is not in a solar system? Problem!

	DaAtom* atom = system->FindIdRecursive(msg.CityId());
	if(!atom)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::CitySameSystem, msg.ChannelName(), Id(), Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Person not found in the same solar system; moved away?
	}

	DaBd* bd = dynamic_cast<DaBd*>(atom);
	if(!bd)
		return;	// Something is wrong. Atom is not a bd.

	// The person must be aboard a ship.
	if(!IsInsideSpacecraft())
	{
		AuCommCityMessage mymsg(AuCommCityMessage::AcceptRepairAboardShip, msg.ChannelName(), atom->Id(), atom->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}
	DaVessel* ship = FindParentVesselInEnv();
	if(!ship)
		return;

	// Must locate the exact bid in the list of active bids.
	if(!g_Au.HasPendingRepairOffer(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::RepairOfferExpired, msg.ChannelName(), atom->Id(), atom->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; repair offer is no longer active.
	}

	if(ship->RepairSecondsRemaining())
	{
		AuCommCityMessage mymsg(AuCommCityMessage::NoRepairShipGettingRepairs, msg.ChannelName(), atom->Id(), atom->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; ship is already getting repairs.
	}

	// Validate the transaction based on the buyer.
	// Insure physical locations permit acceptance of the offer.
	// - Ship must be on a road slab that belongs to the city, or
	// - Ship must be near a friendly station in this solar system.
	bool inCity = false;
	bool nearStation = false;
	bool hasStation = false;
	if(bd)
		inCity = bd->CanAccessByGround(ship);	// True if I can access the ship by ground.
	if(!inCity && !ship->IsInAtmosphere())
	{
		// Look for a non-enemy station nearby since ship is not in the atmosphere.
		DaVessel* station = ship->StationInTradeRange(ship->RoughRadius(), hasStation);
		if(station)
			nearStation = true;
	}

	// Ignore AcceptRepair if the ship is too far away.
	if(!inCity && !nearStation)
	{
		AuCommCityMessage::MESSAGE msgId;
		if(bd)
		{
			if(hasStation)
				msgId = AuCommCityMessage::MoveBdStationToRepair;
			else
				msgId = AuCommCityMessage::MoveBdToRepair;
		}
		else
		{
			if(hasStation)
				msgId = AuCommCityMessage::MoveStationToRepair;
			else
				msgId = AuCommCityMessage::MoveToRepair;
		}
		AuCommCityMessage mymsg(msgId, msg.ChannelName(), atom->Id(), atom->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Verify ship is motionless with key systems off for repairs.
	AuCommCityMessage::MESSAGE msgId = AuCommCityMessage::MessageCount;
	if(ship->IsMoving())
		msgId = AuCommCityMessage::NoRepairMoving;
	else if(ship->SensorsWorking())
		msgId = AuCommCityMessage::NoRepairSensors;
	else if(ship->ShieldsWorkingAndOn())
		msgId = AuCommCityMessage::NoRepairShields;
	else if(ship->IsAnyFTLDriveWorking())
		msgId = AuCommCityMessage::NoRepairFTL;
	else if(ship->GravityDriveWorking())
		msgId = AuCommCityMessage::NoRepairManeuver;
	else if(ship->PowerPlantWorking())
		msgId = AuCommCityMessage::NoRepairPowerPlant;
	if(msgId != AuCommCityMessage::MessageCount)
	{
		AuCommCityMessage mymsg(msgId, msg.ChannelName(), atom->Id(), atom->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure the ship has enough money.
	int price = msg.Price();
	double fAccountBalance = ship->AccountBalance();
	if(price > fAccountBalance)
	{
		QString text = tr("Your ship does not have %1.").arg(QVariant(price).toString() + AuGlobal::CurrencySymbol());
		AuCommCityChat mymsg(text, msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
					msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore the offer; insufficient funds.
	}

	// Repair the ship.
	QString text;
	switch(msg.Type())
	{
	default:
		price = 0;
		text = QString(tr("No spacecraft systems repaired."));
		break;
	case AuCommPacket::AcceptRepairAll:
		try
		{
			const AuCommAcceptRepairAll& msgAccept = dynamic_cast<const AuCommAcceptRepairAll&>(msg);
			if(msgAccept.PartsUnavailableAtMinQ().empty() &&
					msgAccept.PartsShortAtMinQ().empty() &&
					msgAccept.PartsNone().empty() &&
					msgAccept.PartsEnough().empty())			// Repair all, no exceptions
			{
				ship->AddRepairTime(msgAccept.RepairSeconds());
				ship->RepairAll(bd);
				text = tr("All spacecraft system repairs will be completed in %1 seconds.")
					.arg(msgAccept.RepairSeconds());
			}
			else
			{
				// The city was lacking something we needed.
				text = tr("Spacecraft repairs will be completed in %1 seconds.")
					.arg(msgAccept.RepairSeconds());

				// Availability of parts.
				if(!msgAccept.PartsUnavailableAtMinQ().empty())
				{
					text += QObject::tr(" Parts unavailable at that quality: ");
					for(QList<COMMODITYID>::const_iterator mi = msgAccept.PartsUnavailableAtMinQ().cbegin();
							mi != msgAccept.PartsUnavailableAtMinQ().cend(); mi++)
					{
						if(mi == msgAccept.PartsUnavailableAtMinQ().cbegin())
							text += g_Au.CommodityName(*mi);
						else
							text += ", " + g_Au.CommodityName(*mi);
					}
					text += ".";
				}
				if(!msgAccept.PartsShortAtMinQ().empty())
				{
					if(msgAccept.MinQ() > 1)
						text += QObject::tr(" Parts in short supply at that quality: ");
					else
						text += QObject::tr(" Parts in short supply: ");
					for(QList<COMMODITYID>::const_iterator mi = msgAccept.PartsShortAtMinQ().cbegin();
							mi != msgAccept.PartsShortAtMinQ().cend(); mi++)
					{
						if(mi == msgAccept.PartsShortAtMinQ().cbegin())
							text += g_Au.CommodityName(*mi);
						else
							text += ", " + g_Au.CommodityName(*mi);
					}
					text += ".";
				}
				if(!msgAccept.PartsNone().empty())
				{
					text += QObject::tr(" Parts not available: ");
					for(QList<COMMODITYID>::const_iterator mi = msgAccept.PartsNone().cbegin();
							mi != msgAccept.PartsNone().cend(); mi++)
					{
						if(mi == msgAccept.PartsNone().cbegin())
							text += g_Au.CommodityName(*mi);
						else
							text += ", " + g_Au.CommodityName(*mi);
					}
					text += ".";
				}

				ship->AddRepairTime(msgAccept.RepairSeconds());
				ship->RepairAllPossible(bd, msg.MinQ());
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		break;
	case AuCommPacket::AcceptRepairSystem:
		try
		{
			const AuCommAcceptRepairSystem& msgAccept = dynamic_cast<const AuCommAcceptRepairSystem&>(msg);
			if(msgAccept.PartsUnavailableAtMinQ().empty() &&
					msgAccept.PartsShortAtMinQ().empty() &&
					msgAccept.PartsNone().empty() &&
					msgAccept.PartsEnough().empty())			// Repair all, no exceptions
			{
				ship->AddRepairTime(msgAccept.RepairSeconds());
				ship->RepairSystem(bd, msg.SysId(), msg.MinQ());
				text = tr("Spacecraft %1 will be fully repaired in %2 seconds.")
						.arg(ScSystemName(msg.System()))
						.arg(msgAccept.RepairSeconds());
			}
			else
			{
				// The city was lacking something we needed.
				text = tr("Spacecraft %1 repairs will be completed in %2 seconds. Parts not available: ")
						.arg(ScSystemName(msg.System()))
						.arg(msgAccept.RepairSeconds());

				// Availability of parts.
				if(!msgAccept.PartsUnavailableAtMinQ().empty())
				{
					text += QObject::tr(" Parts unavailable at that quality: ");
					for(QList<COMMODITYID>::const_iterator mi = msgAccept.PartsUnavailableAtMinQ().cbegin();
							mi != msgAccept.PartsUnavailableAtMinQ().cend(); mi++)
					{
						if(mi == msgAccept.PartsUnavailableAtMinQ().cbegin())
							text += g_Au.CommodityName(*mi);
						else
							text += ", " + g_Au.CommodityName(*mi);
					}
					text += ".";
				}
				if(!msgAccept.PartsShortAtMinQ().empty())
				{
					if(msgAccept.MinQ() > 1)
						text += QObject::tr(" Parts in short supply at that quality: ");
					else
						text += QObject::tr(" Parts in short supply: ");
					for(QList<COMMODITYID>::const_iterator mi = msgAccept.PartsShortAtMinQ().cbegin();
							mi != msgAccept.PartsShortAtMinQ().cend(); mi++)
					{
						if(mi == msgAccept.PartsShortAtMinQ().cbegin())
							text += g_Au.CommodityName(*mi);
						else
							text += ", " + g_Au.CommodityName(*mi);
					}
					text += ".";
				}
				if(!msgAccept.PartsNone().empty())
				{
					text += QObject::tr(" Parts not available: ");
					for(QList<COMMODITYID>::const_iterator mi = msgAccept.PartsNone().cbegin();
							mi != msgAccept.PartsNone().cend(); mi++)
					{
						if(mi == msgAccept.PartsNone().cbegin())
							text += g_Au.CommodityName(*mi);
						else
							text += ", " + g_Au.CommodityName(*mi);
					}
					text += ".";
				}

				ship->AddRepairTime(msgAccept.RepairSeconds());
				QList<COMMODITYID> cidExceptions;	// Parts to not repair.
				cidExceptions.append(msgAccept.PartsUnavailableAtMinQ());
				cidExceptions.append(msgAccept.PartsShortAtMinQ());
				cidExceptions.append(msgAccept.PartsNone());
				ship->RepairSystemExcept(bd, msg.SysId(), cidExceptions, msg.MinQ());
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		break;
	}

	// Complete the transaction.
	// Revoke the bid so it cannot be accepted again.
	g_Au.BidRevoked(atom->Id(), msg.ChannelName());

	// Take the money from the ship's account.
	ship->DebitAccount(price);
	bd->CommunityCreditAccount(price, AuBank::Civ_CreditSales);

	// Notify the person that the work is complete.
	AuCommCityChat mymsg(text, msg.ChannelName(), atom->Id(), atom->Name(), msg.FromSectorId(), msg.FromGalaxy(),
			msg.ReceptionRange(), msg.Scope());
	g_Au.SendAtomChat(mymsg);

	// Player Statistics
	Stats().SpacecraftRepairsBought(ship);
	IncreaseSkill(AuSkill::Command, 1);
}

bool DaPerson::CommAcceptBerth(int berthType, const AuCommAcceptBerth& msg)
{
	// Acceptor needs to be an officer or the captain.
	// They must also be aboard a ship.
	DaVessel* ship = FindParentVesselInEnv();
	if(!ship)
		return false;	// Acceptor is not aboard a spacecraft.

	// The acceptor is aboard a spacecraft.
	switch(berthType)
	{
	case AuRosterEntry::Invalid:
	case AuRosterEntry::CaptainSlot:
	case AuRosterEntry::Captain:
	case AuRosterEntry::OfficerAI:
	case AuRosterEntry::PassengerAI:
		break;
	case AuRosterEntry::OfficerHuman:	// Officer berth must be accepted by the captain only.
		if(!Is(ship->CaptainId()))
			return false;		// Acceptor is not the captain.
		break;
	case AuRosterEntry::Crew:
	case AuRosterEntry::PassengerHuman:
	case AuRosterEntry::Troop:
		{
			// Non-officer berths may be accepted by any person in the ship's chain-of-command.
			if(!ship->InChainOfCommand(this))
				return false;	// Acceptor does not have the authority to accept the request.
		}
		break;
	}

	return ship->CommAcceptBerth(berthType, msg);
}

void DaPerson::CommAcceptInvitation(const AuCommPacket& msg)
{
	WillfullyChangedEmpire(msg.FromEmpireId());
}

void DaPerson::ExploreWorld(DaWorldLL* world)
{
	if(IsInBd())
	{
		// I am in a building.
		DaBd* bd = FindParentBdInEnv();
		if(bd)
		{
			// Update fog of war for only my avatar.
			// The building handles exploration for its own empire at appropriate times.
			AuPoint gloc = bd->SiteCenter();
			world->ElevateLocMSL(gloc, 0);	// Building site center at sea level.
			double exploreRadius = bd->RoughSiteRadius();
			world->Explore(Id(), EmpireId(), gloc, exploreRadius);
		}
	}
	else if(IsInSpacecraft())
	{
		// I am aboard a spacecraft that is not in a building.
		DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			// Spacecraft explore for their empire only, not for mine.
			// I must do my avatar's exploration separately if my empire is different.
			double exploreRadius = ship->ExploreRadius(*world);
			if(exploreRadius < 0)
			{
				world->ExploreAll(AuId(), ship->EmpireId());
				if(EmpireId() != ship->EmpireId())
					world->ExploreAll(Id(), EmpireId());
			}
			else if(exploreRadius > 0)
			{
				// Determine ship's sea level location.
				AuPoint gloc = world->GlobeLocOfLL(ship->GlobeLL());
				world->Explore(AuId(), ship->EmpireId(), gloc, exploreRadius);
				if(EmpireId() != ship->EmpireId())
					world->Explore(Id(), EmpireId(), gloc, exploreRadius);
			}
		}
	}
	else if(IsAboardVehicle())
	{
		// I am aboard a vehicle that is not aboard a spacecraft.
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(Parent());
		if(vehicle)
		{
			// Vehicles explore for me and my empire.
			double exploreRadius = vehicle->ExploreRadius(*world);
			if(exploreRadius < 0)
				world->ExploreAll(Id(), EmpireId());
			else if(exploreRadius > 0)
			{
				// Determine vehicle's sea level location.
				AuPoint gloc = world->GlobeLocOfLL(vehicle->GlobeLL());
				world->Explore(Id(), EmpireId(), gloc, exploreRadius);
			}
		}
	}
	else
	{
		// I am not in anything.
		if(IsEyeSubmerged())
		{
			// Determine my globe level location. This is sea level on a globe or ringworld, core level on a gas giant.
			AuPoint gloc = world->GlobeLocOfLL(GlobeLL());
			double exploreRadius = 50*FeetPerMeter();
			world->Explore(Id(), EmpireId(), gloc, exploreRadius);
		}
		else if(!world->IsGasGiant())
		{
			// Gas giant can only be explored while underwater.
			if(IsInAtmosphere())
			{
				// Determine my sea level location.
				AuPoint gloc = world->GlobeLocOfLL(GlobeLL());
				// Exploration radius improves with altitude above the ground.
				double exploreRadius = 250*FeetPerMeter() + AltitudeAGL();
				world->Explore(Id(), EmpireId(), gloc, exploreRadius);
			}
		}
	}
}

void DaPerson::SetAccountActive()
{
	//qDebug() << "DaPerson::SetAccountActive" << *this;
	bool chg;
	m_OnLineLock.lockForWrite();
	if(m_AccountInactive)
	{
		m_AccountInactive = false;
		chg = true;
	}
	else
		chg = false;
	m_OnLineLock.unlock();
	if(chg)
		SetModified(NOTIFY_Database);

	// Emit a ping to all of my land systems.
	AuIdList land;
	m_PersonLock.lockForRead();
	if(!m_Domain.empty() || !m_SpawnBdSystem.IsNull())
	{
		m_Domain.SystemIds(land);
		land.AddIfUnique(m_SpawnBdSystem);
		if(g_Au.m_LogMessages & LOG_Debug)
		{
			QString logmsg = QString("%1 %2 pinging spawn system %4 %5 and %3 land systems")
				.arg(Id().GetID())
				.arg(Name())
				.arg(m_Domain.size())
				.arg(m_SpawnBdSystem.GetID())
				.arg(m_SpawnBdSystemName);
			g_Au.Log(logmsg);
		}
	}
	m_PersonLock.unlock();

	// Notify my land solar systems that my account is still active.
	if(!land.empty())
		emit AvatarAccountActive(land);
}

