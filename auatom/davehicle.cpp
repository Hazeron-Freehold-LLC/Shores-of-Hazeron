//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: davehicle.cpp,v 1.1002 2022/05/17 16:58:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "davehicle.h"
//
#include "auatomrenderinfo.h"
#include "aubox.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "aueffect.h"
#include "augleffects.h"
#include "ausensorblip.h"
#include "ausound.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "davessel.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "psfiresmoketrail.h"
#include <QSqlError>
#include <QSqlQuery>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Tables of control inputs based on vehicle type.
DiControlInputArray s_DirtBikeControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_DirtBikeDefaults[] =
{
	DiControlInput(QObject::tr("Turn Left"), DaVehicle::CI_TurnLeft,
				Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
				true,false, DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Turn Right"), DaVehicle::CI_TurnRight,
				Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
				true,false, DiControlInput::WT_None,2, 0,0),

	// W, S, Q, kind of like moving your dude.
	DiControlInput(QObject::tr("Increase Throttle"),
				DaVehicle::CI_ThrustF,
				Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisNegative,
				0,100,true,false, DiControlInput::WT_None,0, 0,1),
	DiControlInput(QObject::tr("Decrease Throttle"),
				DaVehicle::CI_ThrustB,
				Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisPositive,
				0,100,true,false, DiControlInput::WT_None,0, 0,1),
	DiControlInput(QObject::tr("Set Throttle"),
				DaVehicle::CI_SetThrottle,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				-8,10,true,false, DiControlInput::WT_None,0, 0,0,true),	// axis exclusive
	DiControlInput(QObject::tr("Brake"),
				DaVehicle::CI_Brakes,
				Qt::Key_Space,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,true),

	DiControlInput(QObject::tr("Headlight On/Off"),
				DaVehicle::CI_LightOnOff,
				Qt::Key_L,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Engine On/Off"),
				DaVehicle::CI_EngineOnOff,
				Qt::Key_P,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Service Vehicle
	DiControlInput(QObject::tr("Refuel, Repair and Rearm"), DaVehicle::CI_Service,
				Qt::Key_K,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Door remote control
	DiControlInput(QObject::tr("Remote Door Switch"), DaVehicle::CI_DoorRemote,
				Qt::Key_M,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"), DaVehicle::CI_LookBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Forward"), DaVehicle::CI_LookForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left"), DaVehicle::CI_LookLeft,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"), DaVehicle::CI_LookRight,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"), DaVehicle::CI_LookLeftBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"), DaVehicle::CI_LookLeftForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"), DaVehicle::CI_LookRightBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"), DaVehicle::CI_LookRightForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),

	DiControlInput(QObject::tr("Release Ship Assignment"), DaVehicle::CI_ReleaseMotherShip,
			Qt::Key_U, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 0, 0,6),
};
int s_NumDirtBikeDefaults = sizeof(s_DirtBikeDefaults)/sizeof(DiControlInput);

DiControlInputArray s_SUVControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_SUVDefaults[] =
{
	DiControlInput(QObject::tr("Turn Left"), DaVehicle::CI_TurnLeft,
				Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
				true,false, DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Turn Right"), DaVehicle::CI_TurnRight,
				Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
				true,false, DiControlInput::WT_None,2, 0,0),

	// W, S, Q, kind of like moving your dude.
	DiControlInput(QObject::tr("Increase Throttle"),
				DaVehicle::CI_ThrustF,
				Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisNegative,
				0,100,true,false, DiControlInput::WT_None,0, 0,1),
	DiControlInput(QObject::tr("Decrease Throttle"),
				DaVehicle::CI_ThrustB,
				Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisPositive,
				0,100,true,false, DiControlInput::WT_None,0, 0,1),
	DiControlInput(QObject::tr("Set Throttle"),
				DaVehicle::CI_SetThrottle,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				-9,10,true,false, DiControlInput::WT_None,0, 0,0,true),	// axis exclusive
	DiControlInput(QObject::tr("Brake"),
				DaVehicle::CI_Brakes,
				Qt::Key_Space,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,true),

	DiControlInput(QObject::tr("Fire Guns"), QObject::tr("APC Only"),
				DaVehicle::CI_FireGun, Qt::Key_F, Qt::NoModifier,
				DiControlInput::MT_ButtonLeft, Qt::NoModifier,
				DiControlInput::JT_Button, 0, 100, true, false),
	DiControlInput(QObject::tr("Fire Weapon"), QObject::tr("APC Only"),
				DaVehicle::CI_FireWeapon, Qt::Key_G, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true, false),
	DiControlInput(QObject::tr("Select Weapon"), QObject::tr("APC Only"),
				DaVehicle::CI_SelectWeapon, Qt::Key_J, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false, false),

	DiControlInput(QObject::tr("Headlight On/Off"),
				DaVehicle::CI_LightOnOff,
				Qt::Key_L,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Engine On/Off"),
				DaVehicle::CI_EngineOnOff,
				Qt::Key_P,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Service Vehicle
	DiControlInput(QObject::tr("Refuel, Repair and Rearm"), DaVehicle::CI_Service,
				Qt::Key_K,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Door remote control
	DiControlInput(QObject::tr("Remote Door Switch"), DaVehicle::CI_DoorRemote,
				Qt::Key_M,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Seat position selection.
	DiControlInput(QObject::tr("Previous Seat"), DaVehicle::CI_PreviousSeat,
				Qt::Key_B,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Next Seat"), DaVehicle::CI_NextSeat,
				Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"), DaVehicle::CI_LookBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Forward"), DaVehicle::CI_LookForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left"), DaVehicle::CI_LookLeft,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"), DaVehicle::CI_LookRight,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"), DaVehicle::CI_LookLeftBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"), DaVehicle::CI_LookLeftForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"), DaVehicle::CI_LookRightBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"), DaVehicle::CI_LookRightForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),

	DiControlInput(QObject::tr("Release Ship Assignment"), DaVehicle::CI_ReleaseMotherShip,
			Qt::Key_U, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 0, 0,6),
};
int s_NumSUVDefaults = sizeof(s_SUVDefaults)/sizeof(DiControlInput);

DiControlInputArray s_SailBoatControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_SailBoatDefaults[] =
{
		DiControlInput(QObject::tr("Turn Left"), DaVehicle::CI_TurnLeft,
					Qt::Key_A,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
					true,false, DiControlInput::WT_None,2, 0,0),
		DiControlInput(QObject::tr("Turn Right"), DaVehicle::CI_TurnRight,
					Qt::Key_D,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
					true,false, DiControlInput::WT_None,2, 0,0),

		DiControlInput(QObject::tr("Raise Sail"),
					DaVehicle::CI_IncreaseThrottle,
					Qt::Key_W,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,true),
		DiControlInput(QObject::tr("Lower Sail"),
					DaVehicle::CI_DecreaseThrottle,
					Qt::Key_S,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,true),
		DiControlInput(QObject::tr("Furl Sail"),
					DaVehicle::CI_AllStop,
					Qt::Key_Space,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Set Sail"),
					DaVehicle::CI_SetThrottle,
					Qt::Key_unknown,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					-9,10,true,false, DiControlInput::WT_None,0, 0,0,true),	// axis exclusive

	// Service Vehicle
	DiControlInput(QObject::tr("Refuel, Repair and Rearm"), DaVehicle::CI_Service,
				Qt::Key_K,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Door remote control
	DiControlInput(QObject::tr("Remote Door Switch"), DaVehicle::CI_DoorRemote,
				Qt::Key_M,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Seat position selection.
	DiControlInput(QObject::tr("Previous Seat"), DaVehicle::CI_PreviousSeat,
				Qt::Key_B,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Next Seat"), DaVehicle::CI_NextSeat,
				Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"), DaVehicle::CI_LookBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Forward"), DaVehicle::CI_LookForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left"), DaVehicle::CI_LookLeft,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"), DaVehicle::CI_LookRight,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"), DaVehicle::CI_LookLeftBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"), DaVehicle::CI_LookLeftForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"), DaVehicle::CI_LookRightBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"), DaVehicle::CI_LookRightForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),

	DiControlInput(QObject::tr("Release Ship Assignment"), DaVehicle::CI_ReleaseMotherShip,
			Qt::Key_U, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 0, 0,6),
};
int s_NumSailBoatDefaults = sizeof(s_SailBoatDefaults)/sizeof(DiControlInput);

// These controls are used for space transport and space fighters F1 and F2.
DiControlInputArray s_SpaceFighterControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_SpaceFighterDefaults[] =
{
	DiControlInput(QObject::tr("Turn Left"),
			DaVehicle::CI_TurnLeft, Qt::Key_A, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisNegative, 0,100, true,false,
			DiControlInput::WT_None,2, 0,2),
	DiControlInput(QObject::tr("Turn Right"),
			DaVehicle::CI_TurnRight, Qt::Key_D, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisPositive, 0,100, true,false,
			DiControlInput::WT_None,2, 0,2),
	DiControlInput(QObject::tr("Pitch Up"),
			DaVehicle::CI_PitchUp, Qt::Key_W, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisPositive, 0,100, true,false,
			DiControlInput::WT_None,2, 0,1),
	DiControlInput(QObject::tr("Pitch Down"),
			DaVehicle::CI_PitchDown, Qt::Key_S, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisNegative, 0,100, true,false,
			DiControlInput::WT_None,2, 0,1),
	DiControlInput(QObject::tr("Roll Left"),
			DaVehicle::CI_RollLeft, Qt::Key_Z, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisNegative, 0,100, true,false,
			DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Roll Right"),
			DaVehicle::CI_RollRight, Qt::Key_C, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisPositive, 0,100, true,false,
			DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Thrust Forward"),
			DaVehicle::CI_ThrustF, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, true,false,
			DiControlInput::WT_None,0, 0,5),
	DiControlInput(QObject::tr("Thrust Backward"),
			DaVehicle::CI_ThrustB, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, true,false,
			DiControlInput::WT_None,0, 0,4),
	DiControlInput(QObject::tr("Thrust Left"),
			DaVehicle::CI_DecreaseThrottleL, Qt::Key_Left, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_HatLeft, 0,100, true,false,
			DiControlInput::WT_None,0, 0,0),
	DiControlInput(QObject::tr("Thrust Right"),
			DaVehicle::CI_IncreaseThrottleL, Qt::Key_Right, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_HatRight, 0,100, true,false,
			DiControlInput::WT_None,0, 0,0),
	DiControlInput(QObject::tr("Thrust Up"),
			DaVehicle::CI_IncreaseThrottleV, Qt::Key_Up, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_HatUp, 0,100, true,false,
			DiControlInput::WT_None,0, 0,0),
	DiControlInput(QObject::tr("Thrust Down"),
			DaVehicle::CI_DecreaseThrottleV, Qt::Key_Down, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_HatDown, 0,100, true,false,
			DiControlInput::WT_None,0, 0,0),
	DiControlInput(QObject::tr("Brake"),
			DaVehicle::CI_Brakes, Qt::Key_Space, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, true,false,
			DiControlInput::WT_None,0, 0,3),
	DiControlInput(QObject::tr("Brake Lateral and Vertical"),
			DaVehicle::CI_BrakesLV, Qt::Key_Q, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, true,false,
			DiControlInput::WT_None,0, 0,2),
	DiControlInput(QObject::tr("Hold Brakes"),
			DaVehicle::CI_Hold, Qt::Key_H, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// PageUp, PageDown, Delete, like a helm throttle control
	DiControlInput(QObject::tr("Maximum Forward Throttle"),
			DaVehicle::CI_MaxForwardThrottle, Qt::Key_PageUp, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Maximum Reverse Throttle"),
			DaVehicle::CI_MaxReverseThrottle, Qt::Key_PageDown, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Zero Throttle"),
			DaVehicle::CI_AllStop, Qt::Key_Delete, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Increase Throttle"),
			DaVehicle::CI_IncreaseThrottle, Qt::Key_PageUp, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Decrease Throttle"),
			DaVehicle::CI_DecreaseThrottle, Qt::Key_PageDown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Set Throttle"),
			DaVehicle::CI_SetThrottle, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisInvert, -10,10, true,false,
			DiControlInput::WT_None,0, 0,3,true),	// axis exclusive

	DiControlInput(QObject::tr("Fire Guns"),
			DaVehicle::CI_FireGun, Qt::Key_F, Qt::NoModifier,
			DiControlInput::MT_ButtonLeft, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, true,false,
			DiControlInput::WT_None,0, 0,0),
	DiControlInput(QObject::tr("Fire Weapon"),
			DaVehicle::CI_FireWeapon, Qt::Key_G, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, true,false,
			DiControlInput::WT_None,0, 0,1),
	DiControlInput(QObject::tr("Select Weapon"),
			DaVehicle::CI_SelectWeapon, Qt::Key_J, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,10),

	DiControlInput(QObject::tr("Headlight On/Off"),
			DaVehicle::CI_LightOnOff, Qt::Key_L, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,11),
	DiControlInput(QObject::tr("Engine On/Off"),
			DaVehicle::CI_EngineOnOff, Qt::Key_P, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,9),
	DiControlInput(QObject::tr("Orbit Rings On/Off"),
			DaVehicle::CI_OrbitRingsToggle, Qt::Key_O, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),

	// Service Vehicle
	DiControlInput(QObject::tr("Refuel, Repair and Rearm"),
			DaVehicle::CI_Service, Qt::Key_K, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),

	// Door remote control
	DiControlInput(QObject::tr("Remote Door Switch"),
			DaVehicle::CI_DoorRemote, Qt::Key_M, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,8),
	DiControlInput(QObject::tr("Request Launch"), DaVehicle::CI_RequestLaunch,
			Qt::Key_M, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,7),
	DiControlInput(QObject::tr("Request Recovery"), DaVehicle::CI_RequestRecovery,
			Qt::Key_N, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,6),

	// Seat position selection.
	DiControlInput(QObject::tr("Previous Seat"),
			DaVehicle::CI_PreviousSeat, Qt::Key_B, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Next Seat"),
			DaVehicle::CI_NextSeat, Qt::Key_N, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"),
			DaVehicle::CI_LookBack, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 1),
	DiControlInput(QObject::tr("Look Forward"),
			DaVehicle::CI_LookForward, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None, 1),
	DiControlInput(QObject::tr("Look Left"),
			DaVehicle::CI_LookLeft, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"),
			DaVehicle::CI_LookRight, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"),
			DaVehicle::CI_LookLeftBack, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"),
			DaVehicle::CI_LookLeftForward, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"),
			DaVehicle::CI_LookRightBack, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"),
			DaVehicle::CI_LookRightForward, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),

	DiControlInput(QObject::tr("Release Ship Assignment"), DaVehicle::CI_ReleaseMotherShip,
			Qt::Key_U, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 0, 0,6),
};
int s_NumSpaceFighterDefaults = sizeof(s_SpaceFighterDefaults)/sizeof(DiControlInput);

DiControlInputArray s_SpaceRocketControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_SpaceRocketDefaults[] =
{
		// Maneuver
		DiControlInput(QObject::tr("Turn Left"),
					DaVehicle::CI_TurnLeft, Qt::Key_A,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None, 0,100,
					true,false, DiControlInput::WT_None,2),
		DiControlInput(QObject::tr("Turn Right"),
					DaVehicle::CI_TurnRight, Qt::Key_D,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None, 0,100,
					true,false, DiControlInput::WT_None,2),
		DiControlInput(QObject::tr("Pitch Up"),
					DaVehicle::CI_PitchUp, Qt::Key_W,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
					true,false, DiControlInput::WT_None,2, 0,1),
		DiControlInput(QObject::tr("Pitch Down"),
					DaVehicle::CI_PitchDown, Qt::Key_S,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
					true,false, DiControlInput::WT_None,2, 0,1),
		DiControlInput(QObject::tr("Roll Left"),
					DaVehicle::CI_RollLeft, Qt::Key_Z,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
					true,false, DiControlInput::WT_None,2, 0,0),
		DiControlInput(QObject::tr("Roll Right"),
					DaVehicle::CI_RollRight, Qt::Key_C,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
					true,false, DiControlInput::WT_None,2, 0,0),
		DiControlInput(QObject::tr("Thrust Forward"),
					DaVehicle::CI_ThrustF, Qt::Key_unknown),
		DiControlInput(QObject::tr("Thrust Backward"),
					DaVehicle::CI_ThrustB, Qt::Key_unknown),
		DiControlInput(QObject::tr("Thrust Left"),
					DaVehicle::CI_DecreaseThrottleL, Qt::Key_Left),
		DiControlInput(QObject::tr("Thrust Right"),
					DaVehicle::CI_IncreaseThrottleL, Qt::Key_Right),
		DiControlInput(QObject::tr("Thrust Up"),
					DaVehicle::CI_IncreaseThrottleV, Qt::Key_Up),
		DiControlInput(QObject::tr("Thrust Down"),
					DaVehicle::CI_DecreaseThrottleV, Qt::Key_Down),
		DiControlInput(QObject::tr("Brake"),
					DaVehicle::CI_Brakes, Qt::Key_Space,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None),
		DiControlInput(QObject::tr("Brake Lateral and Vertical"),
					DaVehicle::CI_BrakesLV, Qt::Key_Q,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None),
		DiControlInput(QObject::tr("Hold Brakes"),
					DaVehicle::CI_Hold, Qt::Key_H, Qt::NoModifier,
					DiControlInput::MT_None, Qt::NoModifier,
					DiControlInput::JT_None, 0, 100, false),

		// PageUp, PageDown, Delete, like a helm throttle control
		DiControlInput(QObject::tr("Maximum Forward Throttle"),
					DaVehicle::CI_MaxForwardThrottle,
					Qt::Key_PageUp,
					Qt::ShiftModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Maximum Reverse Throttle"),
					DaVehicle::CI_MaxReverseThrottle,
					Qt::Key_PageDown,
					Qt::ShiftModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Zero Throttle"),
					DaVehicle::CI_AllStop,
					Qt::Key_Delete,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Increase Throttle"),
					DaVehicle::CI_IncreaseThrottle,
					Qt::Key_PageUp,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Decrease Throttle"),
					DaVehicle::CI_DecreaseThrottle,
					Qt::Key_PageDown,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Set Throttle"),
					DaVehicle::CI_SetThrottle,
					Qt::Key_unknown,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					-2,10,true,false, DiControlInput::WT_None,0, 0,0,true),	// axis exclusive

		DiControlInput(QObject::tr("Headlight On/Off"),
					DaVehicle::CI_LightOnOff,
					Qt::Key_L,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Engine On/Off"),
					DaVehicle::CI_EngineOnOff,
					Qt::Key_P,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Orbit Rings On/Off"),
					DaVehicle::CI_OrbitRingsToggle,
					Qt::Key_O,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),

	// Service Vehicle
	DiControlInput(QObject::tr("Refuel, Repair and Rearm"), DaVehicle::CI_Service,
				Qt::Key_K,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Door remote control
	DiControlInput(QObject::tr("Remote Door Switch"), DaVehicle::CI_DoorRemote,
				Qt::Key_M,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Request Launch"), DaVehicle::CI_RequestLaunch,
				Qt::Key_M,
				Qt::ShiftModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Request Recovery"), DaVehicle::CI_RequestRecovery,
				Qt::Key_N,
				Qt::ShiftModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Seat position selection.
	DiControlInput(QObject::tr("Previous Seat"), DaVehicle::CI_PreviousSeat,
				Qt::Key_B,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Next Seat"), DaVehicle::CI_NextSeat,
				Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"), DaVehicle::CI_LookBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Forward"), DaVehicle::CI_LookForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left"), DaVehicle::CI_LookLeft,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"), DaVehicle::CI_LookRight,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"), DaVehicle::CI_LookLeftBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"), DaVehicle::CI_LookLeftForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"), DaVehicle::CI_LookRightBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"), DaVehicle::CI_LookRightForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),

	DiControlInput(QObject::tr("Release Ship Assignment"), DaVehicle::CI_ReleaseMotherShip,
			Qt::Key_U, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 0, 0,6),
};
int s_NumSpaceRocketDefaults = sizeof(s_SpaceRocketDefaults)/sizeof(DiControlInput);

DiControlInputArray s_SubmarineControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_SubmarineDefaults[] =
{
		// Maneuver
		DiControlInput(QObject::tr("Turn Left"),
					DaVehicle::CI_TurnLeft, Qt::Key_A,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
					true,false, DiControlInput::WT_None,2, 0,0),
		DiControlInput(QObject::tr("Turn Right"),
					DaVehicle::CI_TurnRight, Qt::Key_D,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
					true,false, DiControlInput::WT_None,2, 0,0),
		DiControlInput(QObject::tr("Pitch Up"),
					DaVehicle::CI_PitchUp, Qt::Key_W,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
					true,false, DiControlInput::WT_None,2, 0,1),
		DiControlInput(QObject::tr("Pitch Down"),
					DaVehicle::CI_PitchDown, Qt::Key_S,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
					true,false, DiControlInput::WT_None,2, 0,1),
		DiControlInput(QObject::tr("Roll Left"),
					DaVehicle::CI_RollLeft, Qt::Key_Z,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None, 0,100,
					true,false, DiControlInput::WT_None,2),
		DiControlInput(QObject::tr("Roll Right"),
					DaVehicle::CI_RollRight, Qt::Key_C,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None, 0,100,
					true,false, DiControlInput::WT_None,2),
		DiControlInput(QObject::tr("Thrust Forward"),
					DaVehicle::CI_ThrustF, Qt::Key_unknown),
		DiControlInput(QObject::tr("Thrust Backward"),
					DaVehicle::CI_ThrustB, Qt::Key_unknown),
		DiControlInput(QObject::tr("Thrust Left"),
					DaVehicle::CI_DecreaseThrottleL, Qt::Key_Left),
		DiControlInput(QObject::tr("Thrust Right"),
					DaVehicle::CI_IncreaseThrottleL, Qt::Key_Right),
		DiControlInput(QObject::tr("Thrust Up"),
					DaVehicle::CI_IncreaseThrottleV, Qt::Key_Up),
		DiControlInput(QObject::tr("Thrust Down"),
					DaVehicle::CI_DecreaseThrottleV, Qt::Key_Down),
		DiControlInput(QObject::tr("Brake"),
					DaVehicle::CI_Brakes, Qt::Key_Space,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None),
		DiControlInput(QObject::tr("Brake Lateral and Vertical"),
					DaVehicle::CI_BrakesLV, Qt::Key_Q,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None),

		// PageUp, PageDown, Delete, like a helm throttle control
		DiControlInput(QObject::tr("Maximum Forward Throttle"),
					DaVehicle::CI_MaxForwardThrottle,
					Qt::Key_PageUp,
					Qt::ShiftModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100),
		DiControlInput(QObject::tr("Maximum Reverse Throttle"),
					DaVehicle::CI_MaxReverseThrottle,
					Qt::Key_PageDown,
					Qt::ShiftModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100),
		DiControlInput(QObject::tr("Zero Throttle"),
					DaVehicle::CI_AllStop,
					Qt::Key_Delete,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100),
		DiControlInput(QObject::tr("Increase Throttle"),
					DaVehicle::CI_IncreaseThrottle,
					Qt::Key_PageUp,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100),
		DiControlInput(QObject::tr("Decrease Throttle"),
					DaVehicle::CI_DecreaseThrottle,
					Qt::Key_PageDown,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100),
		DiControlInput(QObject::tr("Set Throttle"),
					DaVehicle::CI_SetThrottle,
					Qt::Key_unknown,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					-9,10,true,false, DiControlInput::WT_None,0, 0,0,true),	// axis exclusive

		DiControlInput(QObject::tr("Headlight On/Off"),
					DaVehicle::CI_LightOnOff,
					Qt::Key_L,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),
		DiControlInput(QObject::tr("Engine On/Off"),
					DaVehicle::CI_EngineOnOff,
					Qt::Key_P,
					Qt::NoModifier, DiControlInput::MT_None,
					Qt::NoModifier, DiControlInput::JT_None,
					0,100,false),

	// Service Vehicle
	DiControlInput(QObject::tr("Refuel, Repair and Rearm"), DaVehicle::CI_Service,
				Qt::Key_K,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100),

	// Door remote control
	DiControlInput(QObject::tr("Remote Door Switch"), DaVehicle::CI_DoorRemote,
				Qt::Key_M,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100),

	// Seat position selection.
	DiControlInput(QObject::tr("Previous Seat"), DaVehicle::CI_PreviousSeat,
				Qt::Key_B,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Next Seat"), DaVehicle::CI_NextSeat,
				Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"), DaVehicle::CI_LookBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Forward"), DaVehicle::CI_LookForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left"), DaVehicle::CI_LookLeft,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"), DaVehicle::CI_LookRight,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"), DaVehicle::CI_LookLeftBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"), DaVehicle::CI_LookLeftForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"), DaVehicle::CI_LookRightBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"), DaVehicle::CI_LookRightForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),

	DiControlInput(QObject::tr("Release Ship Assignment"), DaVehicle::CI_ReleaseMotherShip,
			Qt::Key_U, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 0, 0,6),
};
int s_NumSubmarineDefaults = sizeof(s_SubmarineDefaults)/sizeof(DiControlInput);

// These controls are used for helicopters.
DiControlInputArray s_HelicopterControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_HelicopterDefaults[] =
{
	DiControlInput(QObject::tr("Turn Left"),
			DaVehicle::CI_TurnLeft, Qt::Key_A, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisNegative, 0,100, true,false,
			DiControlInput::WT_None,2, 0,2),
	DiControlInput(QObject::tr("Turn Right"),
			DaVehicle::CI_TurnRight, Qt::Key_D, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisPositive, 0,100, true,false,
			DiControlInput::WT_None,2, 0,2),
	DiControlInput(QObject::tr("Pitch Up"),
			DaVehicle::CI_PitchUp, Qt::Key_W, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisPositive, 0,100, true,false,
			DiControlInput::WT_None,2, 0,1),
	DiControlInput(QObject::tr("Pitch Down"),
			DaVehicle::CI_PitchDown, Qt::Key_S, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisNegative, 0,100, true,false,
			DiControlInput::WT_None,2, 0,1),
	DiControlInput(QObject::tr("Roll Left"),
			DaVehicle::CI_RollLeft, Qt::Key_Z, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisNegative, 0,100, true,false,
			DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Roll Right"),
			DaVehicle::CI_RollRight, Qt::Key_C, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisPositive, 0,100, true,false,
			DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Climb"),
			DaVehicle::CI_IncreaseThrottleV, Qt::Key_Up, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_HatUp, 0,100, true,false,
			DiControlInput::WT_None,0, 0,0),
	DiControlInput(QObject::tr("Descend"),
			DaVehicle::CI_DecreaseThrottleV, Qt::Key_Down, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_HatDown, 0,100, true,false,
			DiControlInput::WT_None,0, 0,0),
	DiControlInput(QObject::tr("Hover"),
			DaVehicle::CI_Hold, Qt::Key_H, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// PageUp, PageDown, Delete, like a helm throttle control
	DiControlInput(QObject::tr("Maximum Throttle"),
			DaVehicle::CI_MaxForwardThrottle, Qt::Key_PageUp, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Zero Throttle"),
			DaVehicle::CI_AllStop, Qt::Key_Delete, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Increase Throttle"),
			DaVehicle::CI_IncreaseThrottle, Qt::Key_PageUp, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Decrease Throttle"),
			DaVehicle::CI_DecreaseThrottle, Qt::Key_PageDown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Set Throttle"),
			DaVehicle::CI_SetThrottle, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_AxisInvert, -10,10, true,false,
			DiControlInput::WT_None,0, 0,3,true),	// axis exclusive

	DiControlInput(QObject::tr("Headlight On/Off"),
			DaVehicle::CI_LightOnOff, Qt::Key_L, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,11),
	DiControlInput(QObject::tr("Engine On/Off"),
			DaVehicle::CI_EngineOnOff, Qt::Key_P, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,9),

	// Service Vehicle
	//DiControlInput(QObject::tr("Refuel, Repair and Rearm"),
	DiControlInput(QObject::tr("Refuel and Repair"),
			DaVehicle::CI_Service, Qt::Key_K, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),

	// Door remote control
	DiControlInput(QObject::tr("Remote Door Switch"),
			DaVehicle::CI_DoorRemote, Qt::Key_M, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_Button, 0,100, false,false,
			DiControlInput::WT_None,0, 0,8),

	// Seat position selection.
	DiControlInput(QObject::tr("Previous Seat"),
			DaVehicle::CI_PreviousSeat, Qt::Key_B, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),
	DiControlInput(QObject::tr("Next Seat"),
			DaVehicle::CI_NextSeat, Qt::Key_N, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"),
			DaVehicle::CI_LookBack, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 1),
	DiControlInput(QObject::tr("Look Forward"),
			DaVehicle::CI_LookForward, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None, 1),
	DiControlInput(QObject::tr("Look Left"),
			DaVehicle::CI_LookLeft, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"),
			DaVehicle::CI_LookRight, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"),
			DaVehicle::CI_LookLeftBack, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"),
			DaVehicle::CI_LookLeftForward, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"),
			DaVehicle::CI_LookRightBack, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"),
			DaVehicle::CI_LookRightForward, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100,false,false,
			DiControlInput::WT_None,1),

	DiControlInput(QObject::tr("Release Ship Assignment"), DaVehicle::CI_ReleaseMotherShip,
			Qt::Key_U, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0,100, false,false,
			DiControlInput::WT_None, 0, 0,6),
};
int s_NumHelicopterDefaults = sizeof(s_HelicopterDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaVehicle::DaVehicle(COMMODITYID cid, quint8 quality)
{
	Q_ASSERT(quality);
	for(int as=0; as<AS_MaxSources; as++)
	{
		m_AudioVehicle[as] = 0;
		m_Sound[as] = SOUND_None;
	}
	m_byRotSpeedX = 0;
	m_byRotSpeedY = 0;
	m_byRotSpeedZ = 0;
	m_cid = cid;
	m_DriverOnLine = false;
	m_Fuel = 0;
	m_Hits = 0;
	m_LauncherId = 0;
	m_LaunchSpeed = 0;
	m_Model = 0;
	m_OrdSelect = 0;
	m_Parked = false;
	m_Quality = quality;
	m_Switches = 0;
	m_Throttle = 0;
	m_ThrustLateral = 0;
	m_ThrustLong = 0;
	m_ThrustVertical = 0;
	m_wActions = ACT_None;
	for(int i=0; i<MAX_VEHICLE_CONTRAILS; i++)
		m_pEngineTrail[i] = 0;
}

DaVehicle::DaVehicle()
{
	for(int as=0; as<AS_MaxSources; as++)
	{
		m_AudioVehicle[as] = 0;
		m_Sound[as] = SOUND_None;
	}
	m_byRotSpeedX = 0;
	m_byRotSpeedY = 0;
	m_byRotSpeedZ = 0;
	m_cid = CID_Invalid;
	m_DriverOnLine = false;
	m_Fuel = 0;
	m_Hits = 0;
	m_LauncherId = 0;
	m_LaunchSpeed = 0;
	m_Model = 0;
	m_OrdSelect = 0;
	m_Parked = false;
	m_Quality = 1;
	m_Switches = 0;
	m_Throttle = 0;
	m_ThrustLateral = 0;
	m_ThrustLong = 0;
	m_ThrustVertical = 0;
	m_wActions = ACT_None;
	for(int i=0; i<MAX_VEHICLE_CONTRAILS; i++)
		m_pEngineTrail[i] = 0;
}

DaVehicle::~DaVehicle()
{
	for(int i=0; i<MAX_VEHICLE_CONTRAILS; i++)
		if(m_pEngineTrail[i])
		{
			delete m_pEngineTrail[i];
			m_pEngineTrail[i] = 0;
		}
}

DaVehicle::DaVehicle(const DaVehicle &rhs) : DaUnit(rhs)
{
	m_byRotSpeedX = rhs.m_byRotSpeedX;
	m_byRotSpeedY = rhs.m_byRotSpeedY;
	m_byRotSpeedZ = rhs.m_byRotSpeedZ;
	m_cid = rhs.m_cid;
	m_DriverOnLine = rhs.m_DriverOnLine;
	m_Parked = rhs.m_Parked;
	m_Quality = rhs.m_Quality;
	m_Switches = rhs.m_Switches;
	m_Throttle = rhs.m_Throttle;
	m_ThrustLateral = rhs.m_ThrustLateral;
	m_ThrustLong = rhs.m_ThrustLong;
	m_ThrustVertical = rhs.m_ThrustVertical;

	rhs.m_VehicleLock.lockForRead();
	m_Gun = rhs.m_Gun;
	m_Hits = rhs.m_Hits;
	m_MotherShip = rhs.m_MotherShip;
	m_MotherShipName = rhs.m_MotherShipName;
	m_Ord[0] = rhs.m_Ord[0];
	m_Ord[1] = rhs.m_Ord[1];
	m_OrdSelect = rhs.m_OrdSelect;
	m_tmFuelChanged = rhs.m_tmFuelChanged;
	m_tmHitsChanged = rhs.m_tmHitsChanged;
	m_tmLastBrake = rhs.m_tmLastBrake;
	m_tmLastBrakeLV = rhs.m_tmLastBrakeLV;
	m_tmLastPitchControl = rhs.m_tmLastPitchControl;
	m_tmLastRollControl = rhs.m_tmLastRollControl;
	m_tmLastThrottleControl = rhs.m_tmLastThrottleControl;
	m_tmLastThrottleFControl = rhs.m_tmLastThrottleFControl;
	m_tmLastThrottleLControl = rhs.m_tmLastThrottleLControl;
	m_tmLastThrottleVControl = rhs.m_tmLastThrottleVControl;
	m_tmLastTurnControl = rhs.m_tmLastTurnControl;
	m_wActions = rhs.m_wActions;
	rhs.m_VehicleLock.unlock();

	rhs.m_LaunchLock.lockForRead();
	m_LaunchChg = rhs.m_LaunchChg;
	m_LauncherExit = rhs.m_LauncherExit;
	m_LauncherId = rhs.m_LauncherId;
	m_LauncherShip = rhs.m_LauncherShip;
	m_LaunchSpeed = rhs.m_LaunchSpeed;
	rhs.m_LaunchLock.unlock();

	for(int as=0; as<AS_MaxSources; as++)
	{
		m_AudioVehicle[as] = rhs.m_AudioVehicle[as];
		m_Sound[as] = rhs.m_Sound[as];
	}
	m_Fuel = rhs.m_Fuel;
	m_Model = 0;
	for(int i=0; i<MAX_VEHICLE_CONTRAILS; i++)
		m_pEngineTrail[i] = 0;	// The will be created when needed.
}

DaAtom *DaVehicle::Clone() const
{
	return new DaVehicle(*this);
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaVehicle::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaUnit::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	// Start hasWhat with switches.
	quint16 hasWhat = 0;
	if(lastTx.IsNull())
		hasWhat |= 0x0001;	// One time only stuff.
	if(m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical)
		hasWhat |= 0x0002;
	if(!Vector().IsZero())
		hasWhat |= 0x0004;

	m_VehicleLock.lockForRead();
	if(lastTx.IsNull() || m_tmFuelChanged >= lastTx)
		hasWhat |= 0x0008;
	if(lastTx.IsNull() || m_tmHitsChanged >= lastTx)
		hasWhat |= 0x0010;
	if(lastTx.IsNull() || m_Gun.LastFired() >= lastTx)
		hasWhat |= 0x0020;
	if(lastTx.IsNull() || m_Ord[0].LastFired() >= lastTx)
		hasWhat |= 0x0040;
	if(lastTx.IsNull() || m_Ord[1].LastFired() >= lastTx)
		hasWhat |= 0x0080;
	if(lastTx.IsNull() || m_MotherShipChg >= lastTx)
		hasWhat |= 0x0100;
	m_VehicleLock.unlock();

	if(m_OrdSelect)
		hasWhat |= 0x0200;

	m_LaunchLock.lockForRead();
	if(tmLastUpdate.IsNull() || m_LaunchChg >= tmLastUpdate)
		hasWhat |= 0x0400;
	m_LaunchLock.unlock();

	toClient << hasWhat;

	quint8 switches = m_Switches;
	m_VehicleLock.lockForRead();
	if(m_wActions & ACT_Brake)
		switches |= 0x20;
	if(m_wActions & ACT_BrakeLateral)
		switches |= 0x40;
	if(m_wActions & ACT_BrakeVertical)
		switches |= 0x80;
	m_VehicleLock.unlock();

	toClient << switches;

	if(hasWhat & 0x0001)
	{
		// One time only stuff.
		toClient << (quint16)m_cid;
		toClient << m_Quality;
	}

	if(hasWhat & 0x0002)
	{
		// Throttle settings
		toClient << m_Throttle;
		toClient << m_ThrustLateral;
		toClient << m_ThrustLong;
		toClient << m_ThrustVertical;
	}

	if(hasWhat & 0x0004)
	{
		// Movement vector.
		AuPoint vec(Vector());
		toClient << (float)vec.x;
		toClient << (float)vec.y;
		toClient << (float)vec.z;
	}

	if(hasWhat & 0x0008)
		toClient << m_Fuel;

	if(hasWhat & 0x0010)
	{
		m_VehicleLock.lockForRead();
		toClient << m_Hits;
		m_VehicleLock.unlock();
	}

	if(hasWhat & 0x0020)
	{
		m_VehicleLock.lockForRead();
		m_Gun >> toClient;
		m_VehicleLock.unlock();
	}

	if(hasWhat & 0x0040)
	{
		m_VehicleLock.lockForRead();
		m_Ord[0] >> toClient;
		m_VehicleLock.unlock();
	}

	if(hasWhat & 0x0080)
	{
		m_VehicleLock.lockForRead();
		m_Ord[1] >> toClient;
		m_VehicleLock.unlock();
	}

	if(hasWhat & 0x0100)
	{
		toClient << m_MotherShip;
		toClient << m_MotherShipName;
	}

	if(hasWhat & 0x0400)
	{
		// Launch info.
		m_LaunchLock.lockForRead();
		toClient << m_LaunchChg;
		toClient << m_LauncherExit;
		toClient << m_LauncherId;
		toClient << m_LauncherShip;
		toClient << m_LaunchSpeed;
		m_LaunchLock.unlock();
	}
}

// Receive updates to this atom from a server.
void DaVehicle::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaUnit::RxUpdate(fromServer, isFirstUpdate);

	quint16 hasWhat;
	fromServer >> hasWhat;

	m_OrdSelect = (hasWhat & 0x0200) ? 1 : 0;

	quint8 switches;
	fromServer >> switches;
	m_Switches = switches & 0x1f;
	if(switches & 0x20)
		m_wActions |= ACT_Brake;		// Brakes are on.
	if(switches & 0x40)
		m_wActions |= ACT_BrakeLateral;	// Brakes are on.
	if(switches & 0x80)
		m_wActions |= ACT_BrakeVertical;	// Brakes are on.

	if(hasWhat & 0x0001)
	{
		// One time only stuff.
		quint16 cid;
		fromServer >> cid;
		m_cid = (COMMODITYID)cid;
		fromServer >> m_Quality;
		if(!m_Quality)
			m_Quality = 1;
	}

	if(hasWhat & 0x0002)
	{
		// Throttle settings
		qint8	throttle;
		qint8	thrustLateral;
		qint8	thrustLong;
		qint8	thrustVertical;
		fromServer >> throttle;
		fromServer >> thrustLateral;
		fromServer >> thrustLong;
		fromServer >> thrustVertical;
		if(g_Au.IsSceneApp())
		{
			// Local human driver manages thrusters. Don't override them with values from the server.
			DaPerson* humanDriverOnLine = dynamic_cast<DaPerson*>(Driver());
			if(!humanDriverOnLine)
			{
				m_Throttle = throttle;
				m_ThrustLateral = thrustLateral;
				m_ThrustLong = thrustLong;
				m_ThrustVertical = thrustVertical;
			}
			//else
				//qDebug() << "DaVehicle::RxUpdate" << Id() << Name() << "thrust" << thrustLong
					//<< "driver" << humanDriverOnLine->Id() << humanDriverOnLine->Name();
		}
		else
		{
			m_Throttle = throttle;
			m_ThrustLateral = thrustLateral;
			m_ThrustLong = thrustLong;
			m_ThrustVertical = thrustVertical;
		}
	}
	else
	{
		m_Throttle = 0;
		m_ThrustLateral = 0;
		m_ThrustLong = 0;
		m_ThrustVertical = 0;
	}

	if(hasWhat & 0x0004)
	{
		// Movement vector.
		float vx,vy,vz;
		fromServer >> vx;
		fromServer >> vy;
		fromServer >> vz;
		SetVector(AuPoint(vx,vy,vz));
	}
	else
		SetVector(AuPoint(0,0,0));

	if(hasWhat & 0x0008)
		fromServer >> m_Fuel;

	if(hasWhat & 0x0010)
		fromServer >> m_Hits;

	if(hasWhat & 0x0020)
		m_Gun << fromServer;

	if(hasWhat & 0x0040)
		m_Ord[0] << fromServer;

	if(hasWhat & 0x0080)
		m_Ord[1] << fromServer;

	if(hasWhat & 0x0100)
	{
		fromServer >> m_MotherShip;
		fromServer >> m_MotherShipName;
	}

	if(hasWhat & 0x0400)
	{
		// Launch info.
		fromServer >> m_LaunchChg;
		fromServer >> m_LauncherExit;
		fromServer >> m_LauncherId;
		fromServer >> m_LauncherShip;
		fromServer >> m_LaunchSpeed;
	}
}

// Refresh data from valid counterpart.
bool DaVehicle::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	bool transformChanged = DaUnit::Refresh(pAtom, ari);

	DaVehicle* pA = (DaVehicle*)pAtom;
	m_cid = pA->m_cid;
	m_Fuel = pA->m_Fuel;
	m_Quality = pA->m_Quality;
	m_Switches = pA->m_Switches;
	m_Throttle = pA->m_Throttle;
	m_ThrustLateral = pA->m_ThrustLateral;
	m_ThrustLong = pA->m_ThrustLong;
	//qDebug() << "DaVehicle::Refresh" << Id() << Name() << "thrust" << m_ThrustLong;
	m_ThrustVertical = pA->m_ThrustVertical;

	pA->m_VehicleLock.lockForRead();
	m_VehicleLock.lockForWrite();
	m_Gun = pA->m_Gun;
	m_Hits = pA->m_Hits;
	m_Ord[0] = pA->m_Ord[0];
	m_Ord[1] = pA->m_Ord[1];
	m_OrdSelect = pA->m_OrdSelect;
	m_MotherShip = pA->m_MotherShip;
	m_MotherShipName = pA->m_MotherShipName;

	// Only the brake bits are sent to the client, for braking sound effects.
	m_wActions &= ~ACT_BrakeMask;					// Turn off all brake bits.
	m_wActions |= (pA->m_wActions&ACT_BrakeMask);	// Turn on the brake bits we received from pA.

	m_VehicleLock.unlock();
	pA->m_VehicleLock.unlock();

	pA->m_LaunchLock.lockForRead();
	m_LaunchLock.lockForWrite();
	m_LaunchChg = pA->m_LaunchChg;
	m_LauncherExit = pA->m_LauncherExit;
	m_LauncherId = pA->m_LauncherId;
	m_LauncherShip = pA->m_LauncherShip;
	m_LaunchSpeed = pA->m_LaunchSpeed;
	m_LaunchLock.unlock();
	pA->m_LaunchLock.unlock();

	SetVector(pA->Vector());

	if(ari && transformChanged)
		InvalidateEnvironment();

	return transformChanged;
}

void DaVehicle::Create()
{
	DaUnit::Create();

	// Grab the model data right away so whoever created this vehicle can
	// figure out how to place it in the environment.
	AuGLModelCache& models = g_Au.LockGLModelsForWrite();
	m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(models.CreateGeometry(g_Au.CommodityModel(m_cid)));
	g_Au.UnlockGLModels();
}

// Only atoms that return true to IsTicker will have their Tick function called.
bool DaVehicle::IsTicker() const
{
	if(DaUnit::IsTicker())
		return true;

	if(!m_Model)
		return true;

	if(IsDriverOnLine())
		return true;
	else if(!m_Parked && IsAtRest() && IsInCapsule())
		return true;

	if(IsLaunching())
		return true;

	if(m_ThrustLateral)
		return true;
	if(m_ThrustLong)
		return true;
	if(m_ThrustVertical)
		return true;

	m_VehicleLock.lockForRead();
	bool v = m_wActions & (ACT_TurnMask|ACT_PitchMask|ACT_RollMask|ACT_BrakeMask);
	m_VehicleLock.unlock();
	if(v)
		return true;

	return false;
}

void DaVehicle::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// Insure we have loaded the model so we know our geometry.
	if(!m_Model)
	{
		AuGLModelCache& models = g_Au.LockGLModelsForWrite();
		m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(models.CreateGeometry(g_Au.CommodityModel(m_cid)));
		g_Au.UnlockGLModels();
	}

	//qDebug() << "DaVehicle::Tick" << Id() << Name() << "atRest" << IsAtRest() << "altMSL" << AltitudeMSL() << "vec" << Vector()
		//<< "inWater" << IsInWater() << "inEnc" << IsInsideEnclosure() << "inInd" << InIndustry();

	if(!IsAtRest())
		m_Parked = false;

	DaUnit::Tick(last, now);
	if(IsDead())
		return;

	// Set control decay to work best with the driver based on their ping time.
	if(!IsDriverOnLine())
	{
		// Driver is not on line or there is no driver.
		// Park vehicle in vehicle bay of spacecraft when nobody on line is driving it.
		if(!m_Parked && IsAtRest() && IsInCapsule())
		{
			// This vehicle is at rest in a vehicle bay of a spacecraft or building and it is not parked.
			m_Parked = true;	// Parking is a lot of work. Don't keep trying unless we need more movement or rotation to park.
			DaCapsule* ship = dynamic_cast<DaCapsule*>(Parent());
			if(ship)
			{
				// My immediate parent atom is the ship.
				// Attempt to auto park. (autopark)
				if(ship->IsDesignDataCacheValid())
				{
					//qDebug() << "DaVehicle::Tick" << Id() << Name() << "aboard" << ship->Id() << ship->Name() << "parking...";
					AuMatrix4x4 parkPosM;
					VEHICLE_SPOT_TYPE spotType;
					if(ship->ClosestVehicleParkingSpot(this, parkPosM, spotType))
					{
						bool anyChange = false;

						// Reassign myself to the ship.
						if(!ship->Is(MotherShip()))
						{
							SetMotherShip(ship->Id(), ship->Name());
							anyChange = true;
						}

						// Turn my engine off.
						if(IsEngineOn())
						{
							SwitchEngineOff();
							anyChange = true;
						}

						// Turn my lights off.
						if(IsLightOn())
						{
							SwitchLightOff();
							anyChange = true;
						}

						// An empty parking spot was found in the same room as the vehicle.
						int msElapsed = last.elapsed();

						// Center of vehicle model is aligned with parking spot location.
						AuPoint vMin, vMax;
						GetExtents(vMin, vMax);

						// Parking location is currently on the floor.
						// Adjust parking location to be at center height of vehicle above the floor.
						if(spotType == SPOT_Rocket)
						{
							// Rocket takes special handling.
							parkPosM.Translate(-(vMin.x + vMax.x)/2,-(vMin.z + vMax.z)/2,-vMin.y);
							parkPosM.RotateZ(PI);				// So cockpit window lines up with parking spot icon window.
							parkPosM.RotateX(PI/2);
						}
						else
						{
							// Non-rockets are all alike.
							parkPosM.Translate(-(vMin.x + vMax.x)/2,-(vMin.y + vMax.y)/2,-vMin.z);
						}
						AuPoint moveVec = parkPosM.Translation() - Location();	// Vector to parking location relative to the ship.
						double parkDist2 = moveVec.Length2();	// squared distance to parking location
						if(parkDist2 > .083)
						{
							// Move toward the parking spot.
							double maxDistToMove = 2. * msElapsed / 1000.;	// Max distance to move at 2 ft/sec.
							if(parkDist2 > maxDistToMove*maxDistToMove)
							{
								moveVec.Unit(maxDistToMove);	// Limit movement to max.
								m_Parked = false;				// Not parked yet; keep parking.
							}
							// Move without testing for obstacles!
							AuAtomPos pos(Pos());
							AuMatrix4x4 tmove;
							moveVec = pos.Transform().UnProjectVector(moveVec);	// Convert movement vector to be in my own space.
							tmove.Translate(moveVec);
							pos.TransformBy(tmove);
							ChangePosNotParent(pos);
							InvalidateEnvironment();
							anyChange = true;
						}

						AuPoint parkRot = parkPosM.GetRotation();
						if(!parkRot.FuzzyEquals(Rotation()))
						{
							// Rotate to match the parking spot rotation.
							AuPoint rotVec = parkRot - Rotation();					// Change in rotation needed.
							if(rotVec.x > PI)
								rotVec.x -= PI*2;
							else if(rotVec.x < -PI)
								rotVec.x += PI*2;
							if(rotVec.y > PI)
								rotVec.y -= PI*2;
							else if(rotVec.y < -PI)
								rotVec.y += PI*2;
							if(rotVec.z > PI)
								rotVec.z -= PI*2;
							else if(rotVec.z < -PI)
								rotVec.z += PI*2;

							// Limit X rotation to max in this time period.
							double maxAngToRot = 10.*DEG2RAD * msElapsed / 1000.;	// Max rotation at 10 degrees/sec.
							if(spotType == SPOT_Rocket)
							{
								if(rotVec.x > maxAngToRot)
								{
									rotVec.x = maxAngToRot;
									m_Parked = false;					// Not parked yet; keep parking.
								}
								else if(rotVec.x < -maxAngToRot)
								{
									rotVec.x = -maxAngToRot;
									m_Parked = false;					// Not parked yet; keep parking.
								}

								// Limit Y rotation to max in this time period.
								if(rotVec.y > maxAngToRot)
								{
									rotVec.y = maxAngToRot;
									m_Parked = false;					// Not parked yet; keep parking.
								}
								else if(rotVec.y < -maxAngToRot)
								{
									rotVec.y = -maxAngToRot;
									m_Parked = false;					// Not parked yet; keep parking.
								}
							}

							// Limit Z rotation to max in this time period.
							if(rotVec.z > maxAngToRot)
							{
								rotVec.z = maxAngToRot;
								m_Parked = false;					// Not parked yet; keep parking.
							}
							else if(rotVec.z < -maxAngToRot)
							{
								rotVec.z = -maxAngToRot;
								m_Parked = false;					// Not parked yet; keep parking.
							}

							AuAtomPos pos(Pos());
							AuPoint nuRot = pos.Rotation() + rotVec;
							AuMatrix4x4 tme;
							tme.Translate(pos.Translation());
							tme.Rotate(nuRot);
							pos.SetTransform(tme);
							ChangePosNotParent(pos);
							InvalidateEnvironment();
							anyChange = true;
							//qDebug() << "DaVehicle::Tick" << Id() << Name() << "aboard" << ship->Id() << ship->Name()
								//<< "rotating to parking spot" << (rotVec*RAD2DEG) << "myrot" << (Rotation()*RAD2DEG)
								//<< "parkrot" << (parkRot*RAD2DEG) << "maxAng" << (maxAngToRot*RAD2DEG);
						}
						//else
							//qDebug() << "DaVehicle::Tick" << Id() << Name() << "aboard" << ship->Id() << ship->Name()
								//<< "rotation is good";

						if(anyChange)
							SetModified();
					}
					//else
						//qDebug() << "DaVehicle::Tick" << Id() << Name() << "aboard" << ship->Id() << ship->Name()
							//<< "not in a parking spot";
				}
				else
					m_Parked = false;	// Keep trying until the ship's cache is valid so it can find me a spot.
			}
		}
	}

	if(IsAtRest() && g_Au.IsSceneSolo())
	{
		// Break loose from 'at rest' state if the engine and any throttles are on.
		// Engine needs to be on; sailboat doesn't have engine.
		if((m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical) &&
				(IsEngineOn() || CommodityId() == CID_SailBoat))
		{
			//qDebug() << "DaVehicle::TickOnClient" << Id() << Name() << "now in motion";
			if(CommodityId() != CID_Helicopter)
				SwitchHoldOff();
			SetInMotion();
			SetModified();
			TickTock();
		}
	}

	if(IsLaunching() && ParentId() != LaunchingVessel())
		ClearLauncher();

	DecayMoveActions();
}

void DaVehicle::TickOnClient(const AuElapsedTimer& last)
{
	// Insure we have loaded the model so we know our geometry.
	if(!m_Model)
	{
		AuGLModelCache& models = g_Au.LockGLModelsForWrite();
		m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(models.CreateGeometry(g_Au.CommodityModel(m_cid)));
		g_Au.UnlockGLModels();
	}

	DaUnit::TickOnClient(last);

	if(IsAtRest())
	{
		// Break loose from 'at rest' state if the engine and any throttles are on.
		// Engine needs to be on; sailboat doesn't have engine.
		if((m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical) &&
				(IsEngineOn() || CommodityId() == CID_SailBoat))
		{
			//qDebug() << "DaVehicle::TickOnClient" << Id() << Name() << "now in motion";
			if(CommodityId() != CID_Helicopter)
				SwitchHoldOff();
			SetInMotion();
			SetModified();
		}
	}

	if(IsLaunching() && ParentId() != LaunchingVessel())
		ClearLauncher();

	DecayMoveActions();
}

void DaVehicle::TickMovement(const AuElapsedTimer& last)
{
	if(IsEthereal())
	{
		// I am inside the launching vessel.
		if(LaunchChange().ElapsedSec() < 3)
		{
			// Wait three seconds to move.
			SetVector(AuPoint(0,0));
			SetAtRest();
		}
		else
		{
			// Blast off!
			AuPoint vector = Transform().UnProject(LauncherExit());	// Launcher exit relative to me.
			vector.Unit(LaunchSpeed());						// Launcher exit velocity.
			SetVector(vector);
			SetInMotion();
		}
		SetModified();
	}

	DaUnit::TickMovement(last);

	// On the client, TickMovement is only called on the scene atom and the atom controlled by the scene atom, which could only be
	// a vehicle, spacecraft or animal.
	qint64 msElapsed = last.elapsed();
	ApplyRotation(msElapsed);
}

void DaVehicle::DecayMoveActions()
{
	// Set control decay to work best with the driver based on their ping time.
	int controlDecayMs;
	if(IsDriverOnLine())
	{
		// The driver is a person who is on line.
		DaPerson* humanDriverOnLine = dynamic_cast<DaPerson*>(Driver());
		if(humanDriverOnLine)
			controlDecayMs = humanDriverOnLine->ControlDecayMs();	// Control decay is based on human driver on line.
		else
		{
			SetDriverOnLine(false);	// Driver is not on line.
			controlDecayMs = 150;
		}
	}
	else
		controlDecayMs = 150;

	// Shut off turn after a short time to stop rotating.
	m_VehicleLock.lockForRead();
	bool v = (m_wActions & (ACT_TurnMask|ACT_PitchMask|ACT_RollMask|ACT_BrakeMask)) ||
		m_ThrustLateral ||
		m_ThrustLong ||
		m_ThrustVertical;
	m_VehicleLock.unlock();
	if(!v)
		return;

	bool anyChange = false;
	m_VehicleLock.lockForWrite();
	if(m_wActions & ACT_TurnMask)
	{
		if(!m_tmLastTurnControl.isValid() || m_tmLastTurnControl.elapsed() > controlDecayMs)
		{
			m_wActions &= ~ACT_TurnMask;
			anyChange = true;
		}
	}

	// Shut off pitch after a short time to stop rotating.
	if(m_wActions & ACT_PitchMask)
	{
		if(!m_tmLastPitchControl.isValid() || m_tmLastPitchControl.elapsed() > controlDecayMs)
		{
			m_wActions &= ~ACT_PitchMask;
			anyChange = true;
		}
	}

	// Shut off roll after a short time to stop rotating.
	if(m_wActions & ACT_RollMask)
	{
		if(!m_tmLastRollControl.isValid() || m_tmLastRollControl.elapsed() > controlDecayMs)
		{
			m_wActions &= ~ACT_RollMask;
			anyChange = true;
		}
	}

	// Decay throttle settings.
	switch(m_cid)
	{
	case CID_DirtBike:
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
		// Decay throttle after it has not been held for a bit.
		// Lateral and vertical throttles are not used.
		if(m_ThrustLong)
		{
			if(!m_tmLastThrottleFControl.isValid() || m_tmLastThrottleFControl.elapsed() > controlDecayMs)
			{
				if(m_ThrustLong > 0)
					m_ThrustLong--;
				else
					m_ThrustLong++;
				anyChange = true;
			}
		}
		// Shut off brakes after a short time to stop braking.
		if(m_wActions & ACT_Brake)
		{
			if(!m_tmLastBrake.isValid() || m_tmLastBrake.elapsed() > controlDecayMs)
			{
				m_wActions &= ~ACT_Brake;
				anyChange = true;
			}
		}
		break;
	case CID_SailBoat:
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_SpaceRocket:
	case CID_Helicopter:
	case CID_Submarine:
		// Shut off momentary throttles (longitudinal, lateral and vertical) after a short time.
		if(m_ThrustLateral)
		{
			if(!m_tmLastThrottleLControl.isValid() || m_tmLastThrottleLControl.elapsed() > controlDecayMs)
			{
				if(m_ThrustLateral > 0)
					m_ThrustLateral--;
				else
					m_ThrustLateral++;
				anyChange = true;
			}
		}
		if(m_ThrustLong)
		{
			if(!m_tmLastThrottleFControl.isValid() || m_tmLastThrottleFControl.elapsed() > controlDecayMs)
			{
				if(m_ThrustLong > 0)
					m_ThrustLong--;
				else
					m_ThrustLong++;
				anyChange = true;
			}
		}
		if(m_ThrustVertical)
		{
			if(!m_tmLastThrottleVControl.isValid() || m_tmLastThrottleVControl.elapsed() > controlDecayMs)
			{
				if(m_ThrustVertical > 0)
					m_ThrustVertical--;
				else
					m_ThrustVertical++;
				anyChange = true;
			}
		}
		// Shut off brakes after a short time to stop braking.
		if(m_wActions & ACT_Brake)
		{
			if(!m_tmLastBrake.isValid() || m_tmLastBrake.elapsed() > controlDecayMs)
			{
				m_wActions &= ~ACT_Brake;
				anyChange = true;
			}
		}
		// Shut off lateral/vertical brakes after a short time to stop braking.
		if(m_wActions & (ACT_BrakeLateral|ACT_BrakeVertical))
		{
			if(!m_tmLastBrakeLV.isValid() || m_tmLastBrakeLV.elapsed() > controlDecayMs)
			{
				m_wActions &= ~(ACT_BrakeLateral|ACT_BrakeVertical);
				anyChange = true;
			}
		}
		break;
	default:
		break;
	}
	m_VehicleLock.unlock();
	if(anyChange)
		SetModified();
	//qDebug() << "DaVehicle::DecayMoveActions" << Id() << Name() << "decay" << controlDecayMs
		//<< "thrust" << m_ThrustLong;
}

unsigned int DaVehicle::IsTocker() const
{
	unsigned int ms = DaUnit::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is at least d_MSecPerMin.

	if(ms > d_MSecPerSec)
	{
		if(m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical)
		{
			bool occupant = false;
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
				if(!pAtom)
					continue;
				occupant = true;
				break;	// no need to keep searching
			}
			if(occupant)
				ms = d_MSecPerSec;
		}
	}

	if(ms > d_MSecPerSec)
	{
		const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
		if(parentStar)
			ms = d_MSecPerSec;
	}

	if(ms > 10*d_MSecPerSec)
	{
		if(HasAnyCondition())
			ms = 10*d_MSecPerSec;
	}

	if(ms > 30*d_MSecPerSec)
	{
		if(IsDriverOnLine())
			ms = 30*d_MSecPerSec;
		else if(IsEngineOn())
			ms = 30*d_MSecPerSec;
		else if(IsInsideCapsule())
		{
			if(CapsuleRoomType() == ROOM_Fuel)
				ms = 10*d_MSecPerSec;
		}
		else if(InArena())
		{
			if(InArenaOutsideBounds())
				ms = 10*d_MSecPerSec;
		}
		else if(!IsNew() || BdId().IsNull())
		{
			// Environmental decay.
			if(IsInVisibleWater())
			{
				// Vehicle is sitting in lava/water/liquid.
				if(IsInLava())
					ms = 5*d_MSecPerSec;
				else if(!IsWaterCraft())
				{
					// Sitting in water/liquid and not a watercraft.
					if(IsInSupercooledLiquid())
						ms = 10*d_MSecPerSec;
					else
						ms = d_MSecPerMin;
				}
			}

			// Atmospheric decay.
			if(!IsCGSubmerged())
			{
				if(IsInVacuum())
				{
					if(ms == 0 || ms > d_MSecPerHr)
						ms = d_MSecPerHr;
				}
				else
				{
					INDUSTRYID iid = InIndustry();
					if(iid == IID_VehicleFactory || iid == IID_BaseMotorPool)
					{
						if(ms == 0 || ms > d_MSecPerHr)
							ms = d_MSecPerHr;
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
							if(ms == 0 || ms > d_MSecPerHr)
								ms = d_MSecPerHr;
							break;
						case ATMTYPE_Corrosive:
							if(ms == 0 || ms > d_MSecPerMin)
								ms = d_MSecPerMin;
							break;
						case ATMTYPE_Insidious:
							if(ms == 0 || ms > d_MSecPerSec)
								ms = d_MSecPerSec;
							break;
						}
					}
				}
			}

			if(!BdId().IsNull() && InCityEmpire() != EmpireId())
				if(ms > d_MSecPerMin)
					ms = d_MSecPerMin;
		}
	}
	return ms;
}

void DaVehicle::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaUnit::Tock(last, now, scripts, tockTrans);

	DaPerson* humanDriverOnLine = dynamic_cast<DaPerson*>(Driver());
	if(humanDriverOnLine)
	{
		// Must keep empire of vehicle in sync with human driver. Otherwise they can cheat by changing empires in the seat.
		SetEmpireId(humanDriverOnLine->EmpireId());	// Does nothing if empire does not change.
		if(!IsDriverOnLine())
			SetDriverOnLine(true);
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else
		SetDriverOnLine(false);	// Driver is not on line.

	if(IsEngineOn())
	{
		// Fuel is burned at the rate of one fuel unit per minute.
		if(last.ElapsedMinTicks(now) > 0)
		{
			if(m_Fuel)
			{
				m_VehicleLock.lockForWrite();
				m_Fuel--;
				m_tmFuelChanged.Set();
				m_VehicleLock.unlock();
				SetModified();
			}

			// Space vehicles can explore space.
			if(IsSpaceVehicle() && !IsInAtmosphere())
			{
				quint32 empireId = EmpireId();
				if(empireId)
				{
					DaSolarSystem* system = FindParentSolarSystemInEnv();
					if(system)
						system->ExploredBy(empireId);
				}
			}
		}

		// No fuel means no engine.
		if(!m_Fuel)
		{
			SwitchEngineOff();									// Turn off engine
			SwitchHoldOff();									// Cancel brake hold.
			if(!IsSpaceRocket() || !IsInsideCapsule())
				ForceMotion();						// Start to drift in Zero-G, unless rocket parked? inside Capsule.
			SetModified();
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Kill the throttle(s) when there are no more people in the vehicle.
	if(m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical)
	{
		// A throttle is on.
		bool occupant = false;
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
			if(!pAtom)
				continue;
			occupant = true;
			break;	// no need to keep searching
		}

		if(!occupant)
		{
			// Nobody in the vehicle so kill the throttles.
			m_Throttle = 0;
			m_ThrustLateral = 0;
			m_ThrustLong = 0;
			m_ThrustVertical = 0;
			SetModified();
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Conditions are ticked so they can periodically affect this unit.
	TickConditions();
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Decay for garbage collection.
	// Decay is not applied when in spacecraft. The spacecraft will be subject
	// to its own decay rules, which will destroy this guy when it goes.
	if(IsInsideCapsule())
	{
		// Vehicles inside spacecraft do not decay, unless they somehow find their way into a fuel cell.
		if(CapsuleRoomType() == ROOM_Fuel)
		{
			// Vehicles are destroyed if they end up in a fuel cell.
			DestroyVehicle("Invalid location inside fuel cell.");
		}
	}
	else if(InArena())
	{
		if(InArenaOutsideBounds())
			DestroyVehicle(tr("Departed Arena Bounds"));
	}
	else
	{
		// This vehicle is not inside a spaceship.
		if(g_Au.m_EnableUnitDecay && (!IsNew() || BdId().IsNull()))
		{
			// "New" units that are in a city do not decay. If the city decays from under them, then they can decay.
			const int maxHits = MaxHitPoints();
			QString decayReason;

			// Environmental decay.
			WEAPONEFFECT decayEnv = WE_None;	// Decay due to environment contact.
			COMMODITYID decayEnvCID = CID_Invalid;
			int hitsEnv = 0;					// Hits environmental damage.
			if(IsInVisibleWater())
			{
				// Vehicle is sitting in lava/water/liquid.
				if(IsInLava())
				{
					// Sitting in lava.
					decayEnv = WE_Heat;
					hitsEnv = maxHits/10 + 1;	// 10% per second is pretty bad
					decayReason = "lava";
					decayEnvCID = CID_Magma;
				}
				else if(!IsWaterCraft())
				{
					// Sitting in water/liquid and not a watercraft.
					if(IsInSupercooledLiquid())
					{
						// Sitting in supercooled liquid.
						if(InAtmosphereType() == ATMTYPE_Insidious)
							decayEnv = WE_Acid;
						else if(InAtmosphereType() == ATMTYPE_Corrosive)
							decayEnv = WE_Corrosion;
						else
							decayEnv = WE_Cold;
						hitsEnv = maxHits/20 + 1;	// Not quite as bad as lava.
						decayReason = "supercooled liquid";
						decayEnvCID = CID_SupercooledLiquid;
					}
					else if(last.ElapsedMinTicks(now) > 0)
					{
						// Sitting in water.
						decayEnv = WE_Corrosion;
						hitsEnv = maxHits/50 + 1;
						decayReason = "water";
						decayEnvCID = CID_Hydrographics;
					}
				}
			}

			// Atmospheric decay.
			WEAPONEFFECT decayAtm = WE_None;	// Decay due to atmosphere.
			COMMODITYID decayAtmCID = CID_Invalid;
			int hitsAtm = 0;					// Hits atmospheric damage.
			if(!IsCGSubmerged())
			{
				if(IsInVacuum())
				{
					// Decay every two hours in vacuum.
					if(last.IsNewPeriod(now, d_MSecPerHr*2, Id().GetID()))
					{
						decayAtm = WE_Bludgeon;	// meteor impact?
						hitsAtm = maxHits/100 + 1;
						if(!decayReason.isEmpty())
							decayReason += " and ";
						decayReason += "meteor";
						decayAtmCID = CID_Ore;	// meteors are often high in metal content
					}
				}
				else
				{
					INDUSTRYID iid = InIndustry();
					if(iid == IID_VehicleFactory || iid == IID_BaseMotorPool)
					{
						// On road slab, decay at standard atmosphere rate.
						if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
						{
							decayAtm = WE_Corrosion;
							hitsAtm = (int)(maxHits/100 * AirDensity()) + 1;
							if(!decayReason.isEmpty())
								decayReason += " and ";
							decayReason += "weather";
							if(OrbitZone() == OZONE_Inferno)
								decayAtmCID = CID_InfernoAtmosphere;
							else if(OrbitZone() == OZONE_Frigid)
								decayAtmCID = CID_FrigidAtmosphere;
							else
								decayAtmCID = CID_Atmosphere;
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
							// Decay every hour in reasonable atmosphere.
							if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
							{
								decayAtm = WE_Corrosion;
								hitsAtm = (int)(maxHits/100 * AirDensity()) + 1;
								if(!decayReason.isEmpty())
									decayReason += " and ";
								decayReason += "weather";
								if(OrbitZone() == OZONE_Inferno)
									decayAtmCID = CID_InfernoAtmosphere;
								else if(OrbitZone() == OZONE_Frigid)
									decayAtmCID = CID_FrigidAtmosphere;
								else
									decayAtmCID = CID_Atmosphere;
							}
							break;
						case ATMTYPE_Corrosive:
							// Decay every minute in corrosive atmosphere.
							if(last.ElapsedMinTicks(now) > 0)
							{
								decayAtm = WE_Corrosion;
								hitsAtm = (int)(maxHits/50 * AirDensity()) + 1;
								if(!decayReason.isEmpty())
									decayReason += " and ";
								decayReason += "corrosive atmosphere";
								if(OrbitZone() == OZONE_Inferno)
									decayAtmCID = CID_InfernoAtmosphere;
								else if(OrbitZone() == OZONE_Frigid)
									decayAtmCID = CID_FrigidAtmosphere;
								else
									decayAtmCID = CID_Atmosphere;
							}
							break;
						case ATMTYPE_Insidious:
							// Decay every second in insidius atmosphere.
							decayAtm = WE_Acid;
							hitsAtm = (int)(maxHits/100 * AirDensity()) + 1;
							if(!decayReason.isEmpty())
								decayReason += " and ";
							decayReason += "insidious atmosphere";
							if(OrbitZone() == OZONE_Inferno)
								decayAtmCID = CID_InfernoAtmosphere;
							else if(OrbitZone() == OZONE_Frigid)
								decayAtmCID = CID_FrigidAtmosphere;
							else
								decayAtmCID = CID_Atmosphere;
							break;
						}
					}
				}
			}

			// Apply damage due to decay, if any.
			if(decayAtm != WE_None)
			{
				AuCombatAttacker attacker(decayReason);

				AuCombatWeapon weapon(WM_Internal);
				weapon.m_CID = decayAtmCID;
				weapon.m_w1.m_Effect = decayAtm;
				weapon.m_w2.m_Effect = decayEnv;

				AuCombatResult result;
				result.m_Damage1 = hitsAtm;
				result.m_Damage2 = hitsEnv;

				CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			}
			else if(decayEnv != WE_None)
			{
				AuCombatAttacker attacker(decayReason);

				AuCombatWeapon weapon(WM_Internal);
				weapon.m_CID = decayEnvCID;
				weapon.m_w1.m_Effect = decayEnv;

				AuCombatResult result;
				result.m_Damage1 = hitsEnv;

				CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			}
		}

		if(last.ElapsedMinTicks(now) > 0)
		{
			// People in cities notice the presense of enemy units. This causes workers to stop spawning which relieves a genocide
			// exploit. They are safer if they stay indoors.
			if(!InBdCity().IsNull() && InCityEmpire() != EmpireId())
			{
				// This unit is in a city.
				DaWorld* planet = FindParentPlanetInEnv();
				if(planet)
				{
					DaBd* bd = planet->FindBd(BdId());
					if(bd && bd->Residents() > 0)
					{
						// City with people might see something.
						if(bd->PoliticalStanceMakeContact(this) == POL_Enemy)
						{
							QString empireName;
							const DgData& gd = g_Au.LockDataForRead();
							gd.EmpireName(EmpireId(), empireName);
							g_Au.UnlockData();
							QString attName = tr("%2 %1").arg(Name()).arg(empireName);
							bd->LogEvent(AuEventLogEntry::EnemyVehicleSpotted, attName);
						}
					}
				}
			}
		}
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// From Think
	if(last.ElapsedSecTicks(now) > 0)
	{
		// Test for solar radiation damage if my parent is a sun.
		const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
		if(parentStar)
		{
			ApplySolarRadiation(parentStar, MaxHitPoints());
			tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
		}
	}
}

// Turn rate in radians per real world second. Adjusted for environment.
float DaVehicle::TurnRate() const
{
	switch(m_cid)
	{
	case CID_DirtBike:
		if(IsCGSubmerged())
			return 2.0f * DEG2RADf;
		else if(WaterDepth() > 6.f/12.f)
			return 50.f * DEG2RADf;
		else if(OkToOrient())
			return 60.f * DEG2RADf;
		break;
	case CID_SailBoat:
		if(IsInVisibleWater())
			return 20.f * DEG2RADf;
		return 2.f * DEG2RADf;
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
		if(WaterDepth() > 1.f)
			return 10.f * DEG2RADf;
		else if(IsOnGround())
			return 30.f * DEG2RADf;
		return 70.f * DEG2RADf;
	case CID_SpaceTransport:
		if(WaterDepth() > 1.f)
			return 5.f * DEG2RADf;
		else if(IsOnGround())
			return 15.f * DEG2RADf;
		return 35.f * DEG2RADf;
	case CID_SpaceRocket:
		if(WaterDepth() > 1.f)
			return 5.f * DEG2RADf;
		else if(IsOnGround())
			return 0;
		return 25.f * DEG2RADf;
	case CID_Helicopter:
		if(WaterDepth() > 1.f)
			return 5.f * DEG2RADf;
		else if(IsOnGround())
			return 20.f * DEG2RADf;
		return 50.f * DEG2RADf;
	case CID_Submarine:
		if(WaterDepth() > 3.f/12.f)
			return 30.f * DEG2RADf;
		return 2.f * DEG2RADf;
	case CID_SUV:
		if(WaterDepth() > 1.f)
		{
			if(IsOnWater())
				return 2.0f * DEG2RADf;
			else
				return 40.0f * DEG2RADf;
		}
		else if(OkToOrient())
			return 45.f * DEG2RADf;
		break;
	case CID_APC:
		if(WaterDepth() > 1.f)
		{
			if(IsOnWater())
				return 3.0f * DEG2RADf;
			else
				return 60.0f * DEG2RADf;
		}
		else if(OkToOrient())
			return 75.f * DEG2RADf;
		break;
	case CID_FarmMachinery:
		if(WaterDepth() > 1.f)
		{
			if(IsOnWater())
				return 1.0f * DEG2RADf;
			else
				return 20.0f * DEG2RADf;
		}
		else if(OkToOrient())
			return 22.5f * DEG2RADf;
		break;
	default:
		break;
	}
	return 0;
}

void DaVehicle::ApplyRotation(unsigned int msElapsed)
{
	if(!g_Au.IsSceneApp())
		return;
	if(IsLaunching())
		return;

	// This function requires some specific move actions to do anything.
	// Space fighter and submarine need engine power to turn.
	m_VehicleLock.lockForRead();
	int acti = m_wActions;
	m_VehicleLock.unlock();

	if((acti & (ACT_PitchMask|ACT_RollMask|ACT_TurnMask)) && CanTurn())
	{
		AuPoint vec(Vector());

		AuPoint oldVec;
		bool rotvec = false;
		if(!HasGravityDrive() || !IsEngineOn())
		{
			oldVec = Transform().ProjectVector(vec);
			rotvec = true;
		}
		AuPoint otn = Transform().ProjectVector(TerrainNormal());
		AuPoint ogn = Transform().ProjectVector(GravityNormal());

		//qDebug("DaVehicle::ApplyRotation %d msElapsed %d EffManeuver %g "
				//"turn rate %g rot speeds %d,%d,%d",
				//Id().GetID(),
				//msElapsed, m_EffManeuver,
				//TurnRate()*RAD2DEG,
				//m_byRotSpeedX, m_byRotSpeedY, m_byRotSpeedZ);

		// When animating local rotation, limit the time span so occasional
		// network lags won't cause huge turns.
		if(msElapsed > 150)
			msElapsed = 150;

		// Calculate turn amounts based on turn rate and time span.
		float rsec = (float)msElapsed/1000;	// Real world seconds that passed.
		float rMaxTurn = TurnRate() * rsec;	// Turn angle in this time period.

		float rTurnX = m_byRotSpeedX * rsec / 100;
		if (rTurnX > rMaxTurn)
			rTurnX = rMaxTurn;
		float rTurnY = m_byRotSpeedY * rsec / 100;
		if (rTurnY > rMaxTurn)
			rTurnY = rMaxTurn;
		float rTurnZ = m_byRotSpeedZ * rsec / 100;
		if (rTurnZ > rMaxTurn)
			rTurnZ = rMaxTurn;

		AuAtomPos pos(Pos());
		AuAtomPos nupos(pos);
		if(acti & ACT_TurnLeft)
		{
			if(MustMoveToTurn() && vec.y < 0)
				nupos.RotateZ(-rTurnZ);	// Turning reversed in cars in reverse.
			else
				nupos.RotateZ(rTurnZ);
		}
		else if(acti & ACT_TurnRight)
		{
			if(MustMoveToTurn() && vec.y < 0)
				nupos.RotateZ(rTurnZ);	// Turning reversed in cars in reverse.
			else
				nupos.RotateZ(-rTurnZ);
		}

		if(acti & ACT_PitchUp)
			nupos.RotateX(rTurnX);
		else if(acti & ACT_PitchDown)
			nupos.RotateX(-rTurnX);

		if(acti & ACT_RollLeft)
			nupos.RotateY(-rTurnY);
		else if(acti & ACT_RollRight)
			nupos.RotateY(rTurnY);

		if(nupos != pos)
			ChangePosAndOrientNotParent(nupos);

		// This atom's movement vector must be held in the direction it was going, sort of like real inertia.
		if(rotvec)
		{
			AuPoint nuVec = Transform().UnProjectVector(oldVec);
			SetVector(nuVec);
		}
		AuPoint ngn = Transform().UnProjectVector(ogn);
		SetGravityNormal(ngn);
		AuPoint ntn = Transform().UnProjectVector(otn);
		SetTerrainNormal(ntn);
		SetModified();
	}

	Orient(Location(), Transform().ProjectVector(TerrainNormal()));
}

double DaVehicle::OrientationSpeed() const
{
	switch(m_cid)
	{
	case CID_SpaceRocket:
		return PI;

	case CID_DirtBike:
		return PI/2;

	case CID_SUV:
	case CID_APC:
		return PI/3;

	case CID_FarmMachinery:
	case CID_SailBoat:
	case CID_Submarine:
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_Helicopter:
		return PI/4;

	default:
		break;
	}

	return DaAtom::OrientationSpeed();
}

bool DaVehicle::OkToOrient() const
{
	// We do not orient launching vehicles because they are positioned precisely on their launch pad.
	if(IsLaunching())
		return false;

	// We do not orient vehicles that are in the air.
	// We do not orient vehicles that CanPitchAndRoll so aircraft can
	// take off and submarines can maneuver under water.
	if((IsOnGround() || IsInVisibleWater() || (Vector().z < 0 && FallDistance() < 1)) && !CanPitchAndRoll())
		return DaUnit::OkToOrient();
	return false;
}

// returns true if rotation is changed
bool DaVehicle::Orient(const AuPoint& atLoc, const AuPoint& terNormal)
{
	// Must query environment before we can orient properly.
	AuAtomPos pos(Pos());
	if(!OkToOrient())
	{
		SetOrientRotation(pos.Rotation());
		return false;	// Rotation was not changed.
	}

	// This vehicle is touching terrain or water and it cannot pitch or
	// roll independently. If it is not 'on ground', it must be floating
	// in water.
	bool oriented = false;
	AuPoint oldVec;
	bool rotvec = false;
	if(!HasGravityDrive() || !IsEngineOn())
	{
		oldVec = pos.Transform().ProjectVector(Vector());
		rotvec = true;
	}
	AuPoint otn = pos.Transform().ProjectVector(TerrainNormal());
	AuPoint ogn = pos.Transform().ProjectVector(GravityNormal());

	if(IsTwoWheeler() && (IsOnGround() || (Vector().z < 0 && FallDistance() < 1)))
	{
		//qDebug() << "DaVehicle::Orient" << Id() << Name() << "Ok" << OkToOrient() << "2w" << IsTwoWheeler()
			//<< "onG" << IsOnGround() << "fall" << FallDistance() << "atRest" << IsAtRest();
		// Two wheeler has special orientation logic in bumpy terrain and stairs.
		// Wheels are on the terrain but the bike is vertical to the planet from side-to-side.
		AuPoint vy = pos.Transform().ProjectVector(AuPoint(0,1,0));
		vy.Unit();

		AuPoint vx = vy.Cross(ogn);
		vx.Unit();
		vy = ogn.Cross(vx);
		vy.Unit();
		double incline = asin(terNormal.Dot(vy));

		AuMatrix4x4 tOrient;
		tOrient.Translate(atLoc);
		tOrient.Rotate(AuPoint::CalculateRotation(vx, vy));
		tOrient.RotateX(-incline);
		AuPoint nuRot = tOrient.GetRotation();
		SetOrientRotation(nuRot);
		if(IsAtRest() && !nuRot.FuzzyEquals(pos.Rotation()))
		{
			pos.SetTransform(tOrient);
			ChangePosNotParent(pos);
			oriented = true;
		}
	}
	else if(LandingOrientation() == LORI_Tail)
	{
		// Rocket has unique orientation. Stands on its tail.
		AuPoint nuRot;
		if(!IsOnGround() ||
				IsOnHardFloor() ||	// Prevents creeping on building floors.
				/*IsOnMetal() ||*/
				IsOnWoodFloor())	// Prevents creeping on building floors.
		{
			// Orient level with gravity.
			// Calculate rotation to orient pointing along the up vector.
			AuPoint vz = pos.Transform().ProjectVector(AuPoint(0,0,1));
			vz.Unit();
			AuPoint vx = ogn.Cross(vz);
			vx.Unit();
			AuPoint vy = ogn;
			nuRot = AuPoint::CalculateRotation(vx,vy);
		}
		else
		{
			// Orient level with the terrain.
			// Calculate rotation to orient pointing along the up vector.
			AuPoint vz = pos.Transform().ProjectVector(AuPoint(0,0,1));
			vz.Unit();
			AuPoint vx = terNormal.Cross(vz);
			vx.Unit();
			AuPoint vy = terNormal;
			nuRot = AuPoint::CalculateRotation(vx,vy);
		}

		SetOrientRotation(nuRot);
		if(IsAtRest() && !nuRot.FuzzyEquals(pos.Rotation()))
		{
			AuMatrix4x4 tOrient;
			tOrient.Translate(atLoc);
			tOrient.Rotate(nuRot);
			pos.SetTransform(tOrient);
			ChangePosNotParent(pos);
			oriented = true;
		}
	}
	else if(IsOnHardFloor() ||	// Prevents creeping on building floors.
			/*IsOnMetal() ||*/
			IsOnWoodFloor() ||	// Prevents creeping on building floors.
			(IsInWater() && IsCGUnderWater() && Density() < DensityOfWater()))	// Sailboat u/water stops orienting to terrain.
	{
		// Orient level with gravity;
		oriented = DaUnit::Orient(atLoc, ogn);
	}
	else if(IsOnGround())
	{
		// Orient level with the terrain.
		oriented = DaUnit::Orient(atLoc, terNormal);
	}
	else
	{
		// Orient level with gravity;
		oriented = DaUnit::Orient(atLoc, ogn);
	}

	if(!oriented)
		return false;

	// This atom's movement vector must be held in the direction it was going, sort of like real inertia.
	if(rotvec)
	{
		AuPoint nuVec = pos.Transform().UnProjectVector(oldVec);
		SetVector(nuVec);
	}
	AuPoint ngn = pos.Transform().UnProjectVector(ogn);
	SetGravityNormal(ngn);
	AuPoint ntn = pos.Transform().UnProjectVector(otn);
	SetTerrainNormal(ntn);
	return true;
}

void DaVehicle::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaUnit::CreateDeviceObjects(ari);
	if(!m_Model)
	{
		// Start loading the 3ds model of the commodity using a worker thread.
		QString modelFile = g_Au.CommodityModel(m_cid);
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
	}
	else if(!m_Model->HasGLModel())
		m_Model->GLModel(*ari.re);						// GL display list isn't done yet. Attempt to create the GL display list.

	if(m_cid == CID_AdvSpaceFighter)
	{
 		if(!m_pEngineTrail[0])
 		{
			GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
			GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

			// Get the contrail color of the empire.
			const DgData& gd = g_Au.LockDataForRead();
			QColor contrailColor = gd.EmpireContrailColor(EmpireId());
			g_Au.UnlockData();

 			m_pEngineTrail[0] = new PsEngineTrail();
 			m_pEngineTrail[0]->SetMaxParticles(35);
			m_pEngineTrail[0]->SetLifeCycle(1.0f);
			m_pEngineTrail[0]->SetReleaseInterval(0.0f);
			m_pEngineTrail[0]->SetColor(contrailColor);
			m_pEngineTrail[0]->SetPosition(AuPoint(0.4, -12, 0.25));
			m_pEngineTrail[0]->SetSize(1200.0f);
			m_pEngineTrail[0]->SetTextureID(texIdStreak);
			m_pEngineTrail[0]->SetTextureID2(texIdGlow);
 		}
	}
	else if(m_cid == CID_SpaceFighter)
	{
 		if(!m_pEngineTrail[0])
 		{
			GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
			GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

			// Get the contrail color of the empire.
			const DgData& gd = g_Au.LockDataForRead();
			QColor contrailColor = gd.EmpireContrailColor(EmpireId());
			g_Au.UnlockData();

 			m_pEngineTrail[0] = new PsEngineTrail();
 			m_pEngineTrail[0]->SetMaxParticles(35);
			m_pEngineTrail[0]->SetLifeCycle(1.0f);
			m_pEngineTrail[0]->SetReleaseInterval(0.0f);
			m_pEngineTrail[0]->SetColor(contrailColor);
			m_pEngineTrail[0]->SetPosition(AuPoint(.2, -6.5, .9));
			m_pEngineTrail[0]->SetSize(1200.0f);
			m_pEngineTrail[0]->SetTextureID(texIdStreak);
			m_pEngineTrail[0]->SetTextureID2(texIdGlow);
 		}
	}
	else if(m_cid == CID_SpaceTransport)
	{
 		if(!m_pEngineTrail[0])
 		{
			GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
			GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

			// Get the contrail color of the empire.
			const DgData& gd = g_Au.LockDataForRead();
			QColor contrailColor = gd.EmpireContrailColor(EmpireId());
			g_Au.UnlockData();

			// Left Engine
 			m_pEngineTrail[0] = new PsEngineTrail();
 			m_pEngineTrail[0]->SetMaxParticles(35);
			m_pEngineTrail[0]->SetLifeCycle(1.0f);
			m_pEngineTrail[0]->SetReleaseInterval(0.0f);
			m_pEngineTrail[0]->SetColor(contrailColor);
			m_pEngineTrail[0]->SetPosition(AuPoint(-12, -12, 0));
			m_pEngineTrail[0]->SetSize(1000.0f);
			m_pEngineTrail[0]->SetTextureID(texIdStreak);
			m_pEngineTrail[0]->SetTextureID2(texIdGlow);

			// Right Engine
 			m_pEngineTrail[1] = new PsEngineTrail();
 			m_pEngineTrail[1]->SetMaxParticles(35);
			m_pEngineTrail[1]->SetLifeCycle(1.0f);
			m_pEngineTrail[1]->SetReleaseInterval(0.0f);
			m_pEngineTrail[1]->SetColor(contrailColor);
			m_pEngineTrail[1]->SetPosition(AuPoint(12, -12, 0));
			m_pEngineTrail[1]->SetSize(1000.0f);
			m_pEngineTrail[1]->SetTextureID(texIdStreak);
			m_pEngineTrail[1]->SetTextureID2(texIdGlow);
 		}
	}
	else if(m_cid == CID_SpaceRocket)
	{
		if(!m_pEngineTrail[0])
		{
			GLuint texIdSmoke = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Smoke", ":data/smoke.png");

			m_pEngineTrail[0] = new PsFireSmokeTrail();
			m_pEngineTrail[0]->SetMaxParticles(200);
			m_pEngineTrail[0]->SetNumToRelease(2);
			m_pEngineTrail[0]->SetLifeCycle(1.0f);
			m_pEngineTrail[0]->SetReleaseInterval(0.0f);
			QColor colFire;
			colFire.setRgbF(1, .25, .15);
			m_pEngineTrail[0]->SetColor(colFire);
			m_pEngineTrail[0]->SetPosition(AuPoint(0, -3, 0));
			m_pEngineTrail[0]->SetSize(50.0f);
			m_pEngineTrail[0]->SetVelocityVar(0.5f);
			m_pEngineTrail[0]->SetTextureID(texIdSmoke);
		}
	}
}

void DaVehicle::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	//qDebug() << "DaVehicle::DeleteDeviceObjects" << Id() << Name();
	DaUnit::DeleteDeviceObjects(ari);

	if(m_WeaponVAO.VAO())
		m_WeaponVAO.DeleteGL();

	// OpenAL audio sources
	for(int cs=0; cs<AS_MaxSources; cs++)
	{
		ari.StopSound(m_AudioVehicle[cs]);
		m_Sound[cs] = SOUND_None;
	}
}

// Renders this object in a scene.
// View transformation has already been set up.
void DaVehicle::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Do not render distant vehicles.
	if(pass != PASS_Near)
		return;

	if(g_Au.m_DebugSwitches & 0x00000020)
		return;

	if(!m_Model || !m_Model->HasGLModel())
		return;

	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(sceneAtom->Parent() != this)
	{
		// The scene atom is not aboard this vehicle.
		const AuPoint& pv = rmod.ViewLoc(Id());
		if(pv.z >= 0 && pv.Length2() > 30*30)
		{
			// this vehicle is behind the camera plane and more than 30' away from the camera
			// OpenAL audio sources
			RenderAudioSources(sceneAtom, model, ari.al, ari.fade);

			// Base atom class needs to render special effects.
			DaAtom::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
					shadScGlobeFog, shadScRingworld, shadScRingworldFog);
			return;
		}
	}

	// Render atoms subordinate to this vehicle, i.e. riding.
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);

	// Get ambient lighting and fog values.
	float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
	bool sunsBlocked;
	SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);
	AuPoint ambientUp = model.ProjectVector(GravityNormal());

	// Disable suns when they are blocked at my location. Prepare to restore them.
	QList<int> sunsDisabled;
	if(sunsBlocked)
		ari.re->DisableSuns(sunsDisabled);

	// Render the 'lit' version of the model, if there is one. It tends to look more interesting.
	if(IsInAtmosphere())
	{
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
			if(rsworld)
			{
				// Inside atmosphere of a ringworld.
				float shadowPanelSunAngles[MAX_SUNS];
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
				{
					const AuGLSun& sun = ari.re->Sun(sunIndex);
					shadowPanelSunAngles[sunIndex] = rsworld->ShadowPanelSunAngle(sun.m_ShadowAngle);
				}
				m_Model->RenderGLSLModelTexFogRingworld(projection, model, *ari.re, shadowPanelSunAngles, ambientR, ambientG,
						ambientB, ambientUp, IsLightOn(), fogDensity, fogR, fogG, fogB);
			}
			else
			{
				// Inside atmosphere of a globe.
				// Globe position in eye space is needed to calculate sun over-the-horizon effect.
				AuMatrix4x4 m;
				GetTransformRelativeToParent(m, world);
				AuPoint globeP = m.UnProject(0,0,0);		// Globe's position relative to me.
				globeP = model.Project(globeP);				// Globe's position in eye space.
				m_Model->RenderGLSLModelTexFogGlobe(projection, model, *ari.re, globeP, ambientR, ambientG, ambientB,
						ambientUp, IsLightOn(), fogDensity, fogR, fogG, fogB);
			}
		}
	}
	else
	{
		// Not inside atmosphere range of a world.
		m_Model->RenderGLSLModelTex(projection, model, *ari.re, ambientR, ambientG, ambientB, ambientUp, IsLightOn());
	}

	// Restore suns that I disabled.
	ari.re->EnableSuns(sunsDisabled);

	// Show sight surfaces for debugging.
	/*
	{
		AuQuadArray aTargetSurfaces;
		EnumSightSurfaces(aTargetSurfaces, AuMatrix4x4());
		ari.re->glDisable(GL_CULL_FACE);
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for(AuQuadArray::const_iterator qi = aTargetSurfaces.cend()-1; qi >= aTargetSurfaces.cbegin(); qi--)
		{
			ari.re->glColor4f(1,1,1,.25);
			ari.re->glBegin(GL_QUADS);
			ari.re->glNormal3d((*qi).nll.x, (*qi).nll.y, (*qi).nll.z);
			ari.re->glVertex3d((*qi).vll.x, (*qi).vll.y, (*qi).vll.z);
			ari.re->glVertex3d((*qi).vul.x, (*qi).vul.y, (*qi).vul.z);
			ari.re->glVertex3d((*qi).vur.x, (*qi).vur.y, (*qi).vur.z);
			ari.re->glVertex3d((*qi).vlr.x, (*qi).vlr.y, (*qi).vlr.z);
			ari.re->glEnd();

			ari.re->glColor4f(1,1,1,1);
			ari.re->glBegin(GL_LINE_STRIP);
			ari.re->glNormal3d((*qi).nll.x, (*qi).nll.y, (*qi).nll.z);
			ari.re->glVertex3d((*qi).vll.x, (*qi).vll.y, (*qi).vll.z);
			ari.re->glVertex3d((*qi).vul.x, (*qi).vul.y, (*qi).vul.z);
			ari.re->glVertex3d((*qi).vur.x, (*qi).vur.y, (*qi).vur.z);
			ari.re->glVertex3d((*qi).vlr.x, (*qi).vlr.y, (*qi).vlr.z);
			ari.re->glVertex3d((*qi).vll.x, (*qi).vll.y, (*qi).vll.z);
			ari.re->glEnd();
		}
		ari.re->glDisable(GL_BLEND);
		ari.re->glEnable(GL_CULL_FACE);
	}
	*/

	// Render engine trail in RenderLast.
	if(m_pEngineTrail[0] || /*m_Shield ||*/ IsAnyConditionVisible() || IsSmoldering() || ForwardSpeed() > SpeedOfLight()*.9)
		renderLast.Insert(rmod, this);

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, model, ari.al, ari.fade);

	// Base atom class needs to render special effects.
	DaAtom::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

void DaVehicle::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	DaAtom::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);
	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(m_pEngineTrail[0])
	{
		// Render engine contrail.
		ari.re->glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

		switch(CommodityId())
		{
		default:
			break;

		case CID_SpaceRocket:
			{
				// Rocket thrust renders points relative to the vehicle.
				if(IsEngineOn())
					m_pEngineTrail[0]->SetNumToRelease(2);
				else
					m_pEngineTrail[0]->SetNumToRelease(0);
				float fThrust = ThrustLongitudinal()*1.5f;
				if(fThrust <= 0)
					fThrust = 1;
				m_pEngineTrail[0]->SetVelocity(AuPoint(0, -fThrust, 0));
				if(m_pEngineTrail[0]->Update())
					if(m_pEngineTrail[0]->GetParticleCount())
						m_pEngineTrail[0]->Render(projection, model, *ari.re);
			}
			break;

		case CID_SpaceTransport:
			{
				// Bomber has two contrails.
				// Gravity thrust renders points relative to the parent atom of this vehicle.
				AuMatrix4x4 tt = model * Transform().Inverse();
				if(IsEngineOn())
				{
					m_pEngineTrail[0]->SetNumToRelease(1);
					m_pEngineTrail[1]->SetNumToRelease(1);
				}
				else
				{
					m_pEngineTrail[0]->SetNumToRelease(0);
					m_pEngineTrail[1]->SetNumToRelease(0);
				}
				m_pEngineTrail[0]->SetParent(ParentId(), Transform());
				m_pEngineTrail[1]->SetParent(ParentId(), Transform());
				if(m_pEngineTrail[0]->Update())
					if(m_pEngineTrail[0]->GetParticleCount())
						m_pEngineTrail[0]->Render(projection, tt, *ari.re);
				if(m_pEngineTrail[1]->Update())
					if(m_pEngineTrail[1]->GetParticleCount())
						m_pEngineTrail[1]->Render(projection, tt, *ari.re);
			}
			break;

		case CID_SpaceFighter:
		case CID_AdvSpaceFighter:
			{
				// Gravity thrust renders points relative to the parent atom of this vehicle.
				AuMatrix4x4 tt = model * Transform().Inverse();
				if(IsEngineOn())
					m_pEngineTrail[0]->SetNumToRelease(1);
				else
					m_pEngineTrail[0]->SetNumToRelease(0);
				m_pEngineTrail[0]->SetParent(ParentId(), Transform());
				if(m_pEngineTrail[0]->Update())
					if(m_pEngineTrail[0]->GetParticleCount())
						m_pEngineTrail[0]->Render(projection, tt, *ari.re);
			}
			break;
		}

		ari.re->glDisable(GL_BLEND);
		ari.re->glDepthMask(GL_TRUE);
	}

	// Render smoke and fire on burning/smoldering vehicles before shields.
	if(IsAnyConditionVisible())
	{
		float roughRadius = ObstacleRadius();
		if(IsBurning())
		{
			// Render fire with smoke on burning ships.
			RenderFire(projection, model, *ari.re, FireRadius());
			RenderSmoke(projection, model, *ari.re, roughRadius/2.);
		}
		else if(IsSmoldering())
			RenderSmoke(projection, model, *ari.re, roughRadius/2.);

		if(HasAcid())
			RenderAcid(projection, model, *ari.re, roughRadius/2);

		//if(HasDisease())
			//RenderDisease(projection, model, *ari.re, roughRadius/2);

		if(IsParalyzed())
			RenderParalysis(projection, model, *ari.re, roughRadius/2);

		//if(IsPoisoned())
			//RenderPoison(projection, model, *ari.re, roughRadius/2);
	}
	else if(IsSmoldering())
		RenderSmoke(projection, model, *ari.re, ObstacleRadius()/2.);
}

void DaVehicle::RenderHud(float sightXpos, float sightYpos, int hudIntensity, AuAtomRenderInfo& ari)
{
	const float ymax = .98f;

	AuPoint vec(Vector());
	switch(m_cid)
	{
	case CID_APC:
		// Speed indicated with a speedometer.
		{
			// Gauge size
			const float wid = .3f;

			// Calculate xpos,ypos at bottom center of screen
			float xpos = 0;
			float ypos = -ymax;

			ari.RenderSpeedometer(xpos, ypos, wid, ForwardSpeed());
		}

		// HUD
		if(IsEngineOn() || IsLightOn())
		{
			if(!IsEngineOn())
				hudIntensity = hudIntensity * 2 / 3;

			double speed, speedV, speedF, speedL, altitude, pitch, roll, heading, longitude, latitude;
			FlightInstruments(AuPoint(0,0), 0, speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude);
			if(vec.y < 0)
				speed = -speed;			// Correct sign of overall speed for reverse.

			ari.RenderAPCHud(sightXpos, sightYpos, heading, longitude, latitude, hudIntensity, false);
		}
		break;

	case CID_DirtBike:
	case CID_SUV:
	case CID_FarmMachinery:
		// Speed indicated with a speedometer.
		{
			// Gauge size
			const float wid = .3f;

			// Calculate xpos,ypos at bottom center of screen
			float xpos = 0;
			float ypos = -ymax;

			ari.RenderSpeedometer(xpos, ypos, wid, ForwardSpeed());
		}
		break;
	case CID_SailBoat:
		// Throttle indicated with a sail.
		if(ThrustLongitudinal())
		{
			// Size of sail is based on throttle.
			const float sailWid = ThrustLongitudinal()/75.f;

			// Calculate xpos,ypos at bottom center of screen
			float xpos = 0;
			float ypos = -ymax;

			if(sailWid > 0)
				ari.RenderSail(xpos, ypos, sailWid, 1,1,1,1);
			else
				ari.RenderSail(xpos, ypos, sailWid, 1,.5f,0,1);
		}

		// Speed indicated as a number in knots.
		if(!IsFuzzyZero(vec.y))
		{
			// Convert feet per second to nautical miles per hour.
			// 6076 feet = 1 nautical mile
			float knots = fabs(vec.y) * 3600.f / 6076.f;
			int iknots = (int)ceil(knots);
			float left = 0.01f;
			float base = -ymax + .02f;
			float r,g,b;
			if(vec.y < 0)
			{
				// amber
				r = 1;
				g = .7f;
				b = .2f;
			}
			else
			{
				// cyan
				r = .2f;
				g = 1;
				b = 1;
			}
			ari.RenderNumber(iknots, left, base, .05f, 0,false, r,g,b,1);
		}
		break;
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_AdvSpaceFighter:
	case CID_SpaceRocket:
	case CID_Helicopter:
	case CID_Submarine:
		// HUD
		if(IsEngineOn() || IsLightOn())
		{
			if(!IsEngineOn())
				hudIntensity = hudIntensity * 2 / 3;

			double speed, speedV, speedF, speedL, altitude, pitch, roll, heading, longitude, latitude;
			FlightInstruments(AuPoint(0,0), 0, speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude);
			if(vec.y < 0)
				speed = -speed;			// Correct sign of overall speed for reverse.

			if(!IsInAtmosphere() || m_cid == CID_Submarine)
			{
				// No stress indicator outside atmosphere or in submarine.
				ari.RenderAircraftHud(sightXpos, sightYpos, altitude, pitch, roll, heading, speed, speedV, speedL, longitude,
						latitude, hudIntensity, -1, m_Throttle, 0, false, IsHolding());
			}
			else
			{
				float hullStress = HullStress();
				ari.RenderAircraftHud(sightXpos, sightYpos, altitude, pitch, roll, heading, speed, speedV, speedL, longitude,
						latitude, hudIntensity, hullStress, m_Throttle, 0, false, IsHolding());
			}
		}
		break;
	default:
		break;
	}
}

bool DaVehicle::RoughExtents(AuBox &fExt) const
{
	if(m_Model)
	{
		// Use 3D extents of the 3ds model.
		AuPoint min,max;
		m_Model->GetExtents(min, max);
		max.z -= .75;	// Lower top extents to help tall vehicles to not block things on deck above in spacecraft.
		fExt.SetFromMinMax(min, max);
	}
	return true;
}

float DaVehicle::GroundHeightAboveCG() const
{
	if(!m_Model)
	{
		// Vehicles need the model extents NOW so they can query environment correctly.
		AuGLModelCache& models = g_Au.LockGLModelsForWrite();
		m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(models.CreateGeometry(g_Au.CommodityModel(m_cid)));
		g_Au.UnlockGLModels();
		if(!m_Model)
			return 0;
	}

	// Use 3D extents of the 3ds model.
	// Extents are relative to the CG.
	AuPoint min,max;
	m_Model->GetExtents(min, max);
	if(LandingOrientation() == LORI_Tail)
		return -min.y;	// Rocket has unique orientation. Stands on its tail.
	return -min.z;
}

bool DaVehicle::IsObstacle(const DaAtom* toWho) const
{
	if(IsEthereal())
		return false;
	if(IsInAtmosphere() != toWho->IsInAtmosphere())
		return false;
	const DaItem* item = dynamic_cast<const DaItem*>(toWho);
	if(item && item->IsActiveMissile() && item->IsArmedBy(Id()))
		return false;	// I am not an obstacle to my own munitions.
	return true;	// Spacecraft are usually obstacles to movement.
}

// 'blips' is the sensor blip array that is accumulating the scan data. Locations are relative to the solar system.
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
void DaVehicle::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	Q_UNUSED(scannerLoc);
	Q_UNUSED(scannerSensorRange);
	Q_UNUSED(scannerWeaponRange);
	Q_UNUSED(lockedBlip);
	// Must scan me if looking for locked-on blip and I'm it.
	if((!(scanContent & 0x0008) || !Is(lockedBlip)))
	{
		// Not looking for locked-on blip or I am not the locked-on blip.
		// I am only scanned for sensor consoles, transporter consoles, AI pilots, and AI turret gunners.
		if(!(scanContent & 0x0035))
			return;
	}

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	if(IsInSpacecraft())
		return;	// Vehicles don't show up when aboard a spacecraft.

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	POLITICAL_STANCE stance = PoliticalStance(scannerEmpire, EmpireId(), stanceCache);
	switch(m_cid)
	{
	case CID_DirtBike:
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
		blips.Add(AuBlipGroundVehicle(CommodityId(), Quality(), IsNew(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	case CID_SailBoat:
	case CID_Submarine:
		blips.Add(AuBlipBoat(CommodityId(), Quality(), IsNew(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_SpaceRocket:
	case CID_Helicopter:
		blips.Add(AuBlipAircraft(CommodityId(), Quality(), IsNew(), EmpireId(), stance, Id(), EnvLocation()));
		break;
	default:
		break;
	}
}

// Insert first time database record.
void DaVehicle::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO a_Vehicle (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
			"timeCreate,timeModified,timeTock,timeDeath, "
			"name,allegiance,arenaTeam,conditions,damageHistory, "
			"cid,actBits,atRest,vecX,vecY,vecZ,throttle,throttleLateral,throttleLong,"
			"throttleVertical,switches,fuel,hp,qual,ord,motherShip,motherShipName"
			") VALUES (?,?,?,?,?,?,?,?,"
			"?,?,?,?,"
			"?,?,?,?,?,"
			"?,?,?,?,?,?,?,?,?,"
			"?,?,?,?,?,?,?,?);");
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

	// DaVehicle values.
	qa.addBindValue(m_cid);
	qa.addBindValue(m_wActions);
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Throttle);
	qa.addBindValue(m_ThrustLateral);
	qa.addBindValue(m_ThrustLong);
	qa.addBindValue(m_ThrustVertical);
	qa.addBindValue(m_Switches);
	qa.addBindValue(m_Fuel);
	qa.addBindValue(m_Hits);
	qa.addBindValue(m_Quality);

	{
		// Weapons
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_VehicleLock.lockForRead();
		out << m_OrdSelect;
		m_Gun >> out;
		m_Ord[0] >> out;
		m_Ord[1] >> out;
		m_VehicleLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(MotherShip().GetID());
	qa.addBindValue(MotherShipName());

	if(!qa.exec())
		qWarning("DaVehicle::Insert %s FAILED.", (const char*)Name().toLatin1());
}

// Update existing database record.
void DaVehicle::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	sq = QString("UPDATE a_Vehicle SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, "
			"rotZ=?, timeModified=?, timeTock=?, timeDeath=?, name=?, "
			"allegiance=?, arenaTeam=?, conditions=?, damageHistory=?, cid=?, actBits=?, atRest=?, vecX=?, vecY=?, vecZ=?, "
			"throttle=?, throttleLateral=?, throttleLong=?, throttleVertical=?, switches=?, "
			"fuel=?, hp=?, ord=?, motherShip=?, motherShipName=? WHERE id=%2;").arg(Id().GetID());
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

	// DaVehicle values.
	qa.addBindValue(m_cid);
	qa.addBindValue(m_wActions);
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Throttle);
	qa.addBindValue(m_ThrustLateral);
	qa.addBindValue(m_ThrustLong);
	qa.addBindValue(m_ThrustVertical);
	qa.addBindValue(m_Switches);
	qa.addBindValue(m_Fuel);
	qa.addBindValue(m_Hits);

	{
		// Weapons
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_VehicleLock.lockForRead();
		out << m_OrdSelect;
		m_Gun >> out;
		m_Ord[0] >> out;
		m_Ord[1] >> out;
		m_VehicleLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(MotherShip().GetID());
	qa.addBindValue(MotherShipName());

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaVehicle::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaVehicle::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully
// loaded.
DaVehicle* DaVehicle::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
			"timeModified,timeTock,timeDeath, name,allegiance,arenaTeam,conditions,damageHistory, "
			"cid,actBits,atRest,vecX,vecY,vecZ,throttle,throttleLateral,throttleLong,"
			"throttleVertical,switches,fuel,hp,qual,ord,motherShip,motherShipName "
			"FROM a_Vehicle WHERE ");
	if(!idToLoad.IsNull())
		sq += QString(" id=%1").arg(idToLoad.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaVehicle *pnu = new DaVehicle;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
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

			// DaVehicle values.
			pnu->m_cid = (COMMODITYID)qa.value(qv++).toInt();
			pnu->m_wActions = qa.value(qv++).toInt();
			pnu->SetAtRestState(qa.value(qv++).toBool());
			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			pnu->SetVector(AuPoint(vx,vy,vz));
			pnu->m_Throttle = qa.value(qv++).toInt();
			pnu->m_ThrustLateral = qa.value(qv++).toInt();
			pnu->m_ThrustLong = qa.value(qv++).toInt();
			pnu->m_ThrustVertical = qa.value(qv++).toInt();
			pnu->m_Switches = qa.value(qv++).toInt();
			pnu->m_Fuel = qa.value(qv++).toInt();
			pnu->m_Hits = qa.value(qv++).toInt();
			pnu->m_Quality = qa.value(qv++).toInt();
			if(!pnu->m_Quality)
				pnu->m_Quality = 1;

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Weapons
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_OrdSelect;
				pnu->m_Gun << in;
				pnu->m_Ord[0] << in;
				pnu->m_Ord[1] << in;
			}

			pnu->m_MotherShip = AuId(qa.value(qv++).toUInt());
			pnu->m_MotherShipName = qa.value(qv++).toString();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
			DaItem::LoadSubatoms(ether, db, pnu->Id());
			DaNpc::LoadSubatoms(ether, db, pnu->Id());
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

QString DaVehicle::Name() const
{
	QString name;
	m_VehicleLock.lockForRead();
#if !defined(QT_NO_DEBUG)
	// Debug version tells me if the vehicle is new.
	if(IsNew())
	{
		if(m_MotherShip.IsNull())
			name = QString("New %1 %2").arg(g_Au.CommodityName(m_cid)).arg(Id().StringId());
		else if(m_MotherShipName.isEmpty())
			name = QString("New Ship? %1 %2").arg(g_Au.CommodityName(m_cid)).arg(Id().StringId());
		else
			name = QString("New %1 %2 %3").arg(m_MotherShipName).arg(g_Au.CommodityName(m_cid)).arg(Id().StringId());
	}
	else
#endif
	{
		if(m_MotherShip.IsNull())
			name = QString("%1 %2").arg(g_Au.CommodityName(m_cid)).arg(Id().StringId());
		else if(m_MotherShipName.isEmpty())
			name = QString("Ship? %1 %2").arg(g_Au.CommodityName(m_cid)).arg(Id().StringId());
		else
			name = QString("%1 %2 %3").arg(m_MotherShipName).arg(g_Au.CommodityName(m_cid)).arg(Id().StringId());
	}
	m_VehicleLock.unlock();
	return name;
}

QString DaVehicle::SpotDescription(quint32 spotterEmpire) const
{
	QString name = Name();
	QString empire;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(EmpireId(), empire);
	POLITICAL_STANCE stance = gd.EmpirePoliticalStance(spotterEmpire, EmpireId());
	g_Au.UnlockData();

	name = QString("%1 %3 Q%4").arg(name).arg(DbCommodity::QualityDesc(m_Quality)).arg(m_Quality);

	int maxHits = MaxHitPoints();
	int hp = CurrentHitPoints();
	if(hp == maxHits)
		name += tr(" %1 hp").arg(maxHits);
	else
		name += tr(" %1/%2 hp").arg(hp).arg(maxHits);

	if(!empire.isEmpty())
	{
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

// Report a longer description of this Atom for examination purposes.
QString DaVehicle::Description() const
{
	switch(ParkingSpotType())
	{
	case SPOT_GroundVehicle:
		return tr("Ground Vehicle");
	case SPOT_Rocket:
		return tr("Space Rocket");
	case SPOT_SpaceVehicle:
		return tr("Space Vehicle");
	case SPOT_WaterVehicle:
		return tr("Water Vehicle");
	case SPOT_Invalid:
	case SPOT_Count:
	default:
		return tr("Vehicle");
	}
}

void DaVehicle::ListSeatsOccupied(QList<int>& occupiedSeats) const
{
	occupiedSeats.clear();
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		const DaCreature* creature = dynamic_cast<const DaCreature*>(pAtom);
		if(creature)
		{
			// Creatures store seat index due to changing body locations.
			occupiedSeats.append(creature->VehicleSeatIndex());
		}
		else
		{
			// Non-creature atoms are placed just above a seat location.
			AuPoint seatLoc = pAtom->Location();
			int seatIndex = SeatAt(seatLoc);
			if(seatIndex >= 0)
				occupiedSeats.append(seatIndex);
		}
	}
}

void DaVehicle::ListSeatsOccupiedByPeopleOnLine(QList<int>& occupiedSeats) const
{
	occupiedSeats.clear();
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Caller only wants people who are on line right now.
		DaAtom* pAtom = *it;
		const DaPerson* person = dynamic_cast<const DaPerson*>(pAtom);
		if(!person)
			continue;
		if(!person->IsOnLine())
			continue;

		// People store seat index due to changing body locations.
		occupiedSeats.append(person->VehicleSeatIndex());
	}
}

int DaVehicle::EmptySeats() const
{
	if(!m_Model)
		return 0;
	int seats = m_Model->SeatCount();
	if(seats)
	{
		DaAtomList subs;
		ListNormalSubatoms(subs);
		if(seats > (int)subs.size())
			seats -= (int)subs.size();
		else
			seats = 0;
	}
	return seats;
}

int DaVehicle::FindEmptySeat(AuPoint& seatLoc, AuPose& pose) const
{
	if(!m_Model)
		return -1;
	int seats = m_Model->SeatCount();
	if(!seats)
		return -1;

	// Make a list of the locations of my occupants.
	QList<int> occupiedSeats;
	ListSeatsOccupied(occupiedSeats);

	// Look for an empty seat index.
	for(int seat=0; seat<seats; seat++)
		if(!occupiedSeats.contains(seat))
		{
			// Seat location appears unoccupied.
			seatLoc = m_Model->SeatLoc(seat);
			pose = SeatPose(seat);
			return seat;
		}

	// We get here if all seats are occupied.
	return -1;
}

// Returns the seat at the object location. Ignores Z due to objects that are either at the seat location or perhaps slightly above
// the seat location by the CG height of the object.
// -1 returned if no seat found
int DaVehicle::SeatAt(const AuPoint& objectLoc) const
{
	if(!m_Model)
		return -1;
	int seats = m_Model->SeatCount();
	if(!seats)
		return -1;

	int closestSeat = -1;
	double closestDist2 = 0;	// squared distance to the closest seat that is within 1'
	for(int seat=0; seat<seats; seat++)
	{
		AuPoint seatLoc = m_Model->SeatLoc(seat);
		double dist2 = seatLoc.Distance2_2D(objectLoc);
		if(dist2 < 1 && (closestSeat == -1 || dist2 < closestDist2))
		{
			// Found closest seat that is within 1' in 2D.
			closestSeat = seat;
			closestDist2 = dist2;
		}
	}
	return closestSeat;
}

// Returns offload location for a passenger seat location.
AuPoint DaVehicle::OffLoc(int seat) const
{
	if(m_Model)
		return m_Model->OffloadLoc(seat);
	return AuPoint(0,0);
}

AuPoint DaVehicle::SeatLoc(int seat) const
{
	if(!m_Model)
		return AuPoint(0,0);
	return m_Model->SeatLoc(seat);
}

AuPose DaVehicle::SeatPose(int seat) const
{
	if(!m_Model)
		return AuPose::Stand;

	switch(m_Model->SeatPose(seat))
	{
	default:
	case Au3dsGLSLStation::Invalid:
	case Au3dsGLSLStation::Stand:
	case Au3dsGLSLStation::Any:
	case Au3dsGLSLStation::All:
		return AuPose::Stand;
	case Au3dsGLSLStation::Crouch:
		return AuPose::Crouch;
	case Au3dsGLSLStation::Prone:
		return AuPose::Prone;
	case Au3dsGLSLStation::Sit:		// on the ground
		return AuPose::SitOnGround;
	case Au3dsGLSLStation::Seated:	// in a chair, like at a console
		return AuPose::Straddle;
	case Au3dsGLSLStation::Straddle:
		return AuPose::Straddle;
	case Au3dsGLSLStation::EyesOnly:
		return AuPose::EyesOnly;
	}
}

// Returns the person online or creature who is in a control seat.
DaCreature* DaVehicle::Driver() const
{
	if(!m_Model)
		return 0;
	int seats = m_Model->SeatCount();
	if(!seats)
		return 0;

	DaCreature* driver = 0;										// The driver if there is no person on line in a control seat.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaPerson* person = dynamic_cast<DaPerson*>(pAtom);
		if(person)
		{
			// Found a person aboard this vehicle.
			if(!person->IsInvisibleOffLine() && IsControlSeat(person->VehicleSeatIndex()))
				return person;
		}
		else if(!driver)
		{
			// We have not found a citizen driver yet.
			DaCitizen* citizen = dynamic_cast<DaCitizen*>(pAtom);
			if(citizen && IsControlSeat(citizen->VehicleSeatIndex()))
				driver = citizen;								// Remember citizen driver in case we find no human drivers.
		}
	}

	// We get here if no control seats are occupied by people on line or citizens.
	return driver;
}

// Returns true if a control seat is occupied by a person on line.
bool DaVehicle::IsControllerOnLine() const
{
	if(!m_Model)
		return false;
	int seats = m_Model->SeatCount();
	if(!seats)
		return false;
	if(!HasSubatoms())
		return false;

	// Make a list of the locations of my occupants.
	QList<int> occupiedSeats;
	ListSeatsOccupiedByPeopleOnLine(occupiedSeats);
	for(QList<int>::const_iterator it = occupiedSeats.cbegin(); it != occupiedSeats.cend(); it++)
		if(IsControlSeat(*it))
			return true;

	// We get here if no control seats are occupied by people on line.
	return false;
}

bool DaVehicle::IsSeatOccupied(int seat, bool onLineOnly) const
{
	if(!m_Model)
		return false;
	int seats = m_Model->SeatCount();
	if(!seats)
		return false;

	// Make a list of the seat indices of my occupants.
	QList<int> occupiedSeats;
	if(onLineOnly)
		ListSeatsOccupiedByPeopleOnLine(occupiedSeats);
	else
		ListSeatsOccupied(occupiedSeats);
	return occupiedSeats.contains(seat);
}

int DaVehicle::NextUnoccupiedSeat(int fromSeat) const
{
	if(!m_Model)
		return -1;
	int seats = m_Model->SeatCount();
	if(!seats)
		return -1;

	// Make a list of the seat indices of my occupants.
	QList<int> occupiedSeats;
	ListSeatsOccupied(occupiedSeats);

	// Search to the end for an empty seat.
	for(int seat=fromSeat+1; seat<seats; seat++)
		if(!occupiedSeats.contains(seat))
			return seat;										// The seat location is empty.

	// Search from the beginning for an empty seat.
	for(int seat=0; seat<fromSeat; seat++)
		if(!occupiedSeats.contains(seat))
			return seat;										// The seat location is empty.

	// We get here if no unoccupied seat was found.
	return -1;
}

int DaVehicle::PreviousUnoccupiedSeat(int fromSeat) const
{
	if(!m_Model)
		return -1;
	int seats = m_Model->SeatCount();
	if(!seats)
		return -1;

	// Make a list of the seat indices of my occupants.
	QList<int> occupiedSeats;
	ListSeatsOccupied(occupiedSeats);

	// Search back to the beginning for an empty seat.
	for(int seat=fromSeat-1; seat>=0; seat--)
		if(!occupiedSeats.contains(seat))
			return seat;										// The seat location is empty.

	// Search back from the end for an empty seat.
	for(int seat=seats-1; seat>fromSeat; seat--)
		if(!occupiedSeats.contains(seat))
			return seat;										// The seat location is empty.

	// We get here if no unoccupied seat was found.
	return -1;
}

void DaVehicle::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Get a pointer to the appropriate controls array for this type of vehicle.
	DiControlInputArray* myControls = 0;
	switch(m_cid)
	{
	default:
		return;
	case CID_DirtBike:
		myControls = &s_DirtBikeControls;
		break;
	case CID_SailBoat:
		myControls = &s_SailBoatControls;
		break;
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_AdvSpaceFighter:
		myControls = &s_SpaceFighterControls;
		break;
	case CID_Helicopter:
		myControls = &s_HelicopterControls;
		break;
	case CID_SpaceRocket:
		myControls = &s_SpaceRocketControls;
		break;
	case CID_Submarine:
		myControls = &s_SubmarineControls;
		break;
	case CID_APC:
	case CID_SUV:
	case CID_FarmMachinery:
		myControls = &s_SUVControls;
		break;
	}

	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = myControls->begin(); myci != myControls->end(); myci++)
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

void DaVehicle::RestoreDefaultControls()
{
	switch(m_cid)
	{
	default:
		return;
	case CID_DirtBike:
		s_DirtBikeControls.clear();
		for(int ic=0; ic<s_NumDirtBikeDefaults; ic++)
			s_DirtBikeControls.append(s_DirtBikeDefaults[ic]);
		break;
	case CID_SailBoat:
		s_SailBoatControls.clear();
		for(int ic=0; ic<s_NumSailBoatDefaults; ic++)
			s_SailBoatControls.append(s_SailBoatDefaults[ic]);
		break;
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_AdvSpaceFighter:
		s_SpaceFighterControls.clear();
		for(int ic=0; ic<s_NumSpaceFighterDefaults; ic++)
			s_SpaceFighterControls.append(s_SpaceFighterDefaults[ic]);
		break;
	case CID_Helicopter:
		s_HelicopterControls.clear();
		for(int ic=0; ic<s_NumHelicopterDefaults; ic++)
			s_HelicopterControls.append(s_HelicopterDefaults[ic]);
		break;
	case CID_SpaceRocket:
		s_SpaceRocketControls.clear();
		for(int ic=0; ic<s_NumSpaceRocketDefaults; ic++)
			s_SpaceRocketControls.append(s_SpaceRocketDefaults[ic]);
		break;
	case CID_Submarine:
		s_SubmarineControls.clear();
		for(int ic=0; ic<s_NumSubmarineDefaults; ic++)
			s_SubmarineControls.append(s_SubmarineDefaults[ic]);
		break;
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
		s_SUVControls.clear();
		for(int ic=0; ic<s_NumSUVDefaults; ic++)
			s_SUVControls.append(s_SUVDefaults[ic]);
		break;
	}
	SaveControls();
}

void DaVehicle::LoadControls() const
{
	QString settingsSection;

	// Init with my default controls.
	DiControlInputArray* myControls = 0;
	switch(m_cid)
	{
	default:
		return;
	case CID_DirtBike:
		settingsSection = "Controls/Motor Bike";
		s_DirtBikeControls.clear();
		for(int ic=0; ic<s_NumDirtBikeDefaults; ic++)
			s_DirtBikeControls.append(s_DirtBikeDefaults[ic]);
		myControls = &s_DirtBikeControls;
		break;
	case CID_SailBoat:
		settingsSection = "Controls/Sail Boat";
		s_SailBoatControls.clear();
		for(int ic=0; ic<s_NumSailBoatDefaults; ic++)
			s_SailBoatControls.append(s_SailBoatDefaults[ic]);
		myControls = &s_SailBoatControls;
		break;
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_AdvSpaceFighter:
		settingsSection = "Controls/Space Fighter";
		s_SpaceFighterControls.clear();
		for(int ic=0; ic<s_NumSpaceFighterDefaults; ic++)
			s_SpaceFighterControls.append(s_SpaceFighterDefaults[ic]);
		myControls = &s_SpaceFighterControls;
		break;
	case CID_Helicopter:
		settingsSection = "Controls/Helicopter";
		s_HelicopterControls.clear();
		for(int ic=0; ic<s_NumHelicopterDefaults; ic++)
			s_HelicopterControls.append(s_HelicopterDefaults[ic]);
		myControls = &s_HelicopterControls;
		break;
	case CID_SpaceRocket:
		settingsSection = "Controls/Space Rocket";
		s_SpaceRocketControls.clear();
		for(int ic=0; ic<s_NumSpaceRocketDefaults; ic++)
			s_SpaceRocketControls.append(s_SpaceRocketDefaults[ic]);
		myControls = &s_SpaceRocketControls;
		break;
	case CID_Submarine:
		settingsSection = "Controls/Submarine";
		s_SubmarineControls.clear();
		for(int ic=0; ic<s_NumSubmarineDefaults; ic++)
			s_SubmarineControls.append(s_SubmarineDefaults[ic]);
		myControls = &s_SubmarineControls;
		break;
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
		settingsSection = "Controls/SUV";
		s_SUVControls.clear();
		for(int ic=0; ic<s_NumSUVDefaults; ic++)
			s_SUVControls.append(s_SUVDefaults[ic]);
		myControls = &s_SUVControls;
		break;
	}

	// Load any overrides from the settings file.
	myControls->LoadSettings(settingsSection);
}

void DaVehicle::SaveControls() const
{
	QString settingsSection;

	// Get a pointer to the appropriate controls array for this type of vehicle.
	DiControlInputArray* myControls = 0;
	switch(m_cid)
	{
	default:
		return;
	case CID_DirtBike:
		settingsSection = "Controls/Motor Bike";
		myControls = &s_DirtBikeControls;
		break;
	case CID_SailBoat:
		settingsSection = "Controls/Sail Boat";
		myControls = &s_SailBoatControls;
		break;
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_AdvSpaceFighter:
		settingsSection = "Controls/Space Fighter";
		myControls = &s_SpaceFighterControls;
		break;
	case CID_Helicopter:
		settingsSection = "Controls/Helicopter";
		myControls = &s_HelicopterControls;
		break;
	case CID_SpaceRocket:
		settingsSection = "Controls/Space Rocket";
		myControls = &s_SpaceRocketControls;
		break;
	case CID_Submarine:
		settingsSection = "Controls/Submarine";
		myControls = &s_SubmarineControls;
		break;
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
		settingsSection = "Controls/SUV";
		myControls = &s_SUVControls;
		break;
	}

	// Store my controls in the settings file.
	myControls->SaveSettings(settingsSection);
}

void DaVehicle::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Get a pointer to the appropriate controls array for this type of vehicle.
	DiControlInputArray* myControls = 0;
	switch(m_cid)
	{
	default:
		return;
	case CID_DirtBike:
		myControls = &s_DirtBikeControls;
		break;
	case CID_SailBoat:
		myControls = &s_SailBoatControls;
		break;
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_AdvSpaceFighter:
		myControls = &s_SpaceFighterControls;
		break;
	case CID_Helicopter:
		myControls = &s_HelicopterControls;
		break;
	case CID_SpaceRocket:
		myControls = &s_SpaceRocketControls;
		break;
	case CID_Submarine:
		myControls = &s_SubmarineControls;
		break;
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
		myControls = &s_SUVControls;
		break;
	}

	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(myControls->isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = myControls->cbegin(); ci != myControls->cend(); ci++)
		aAllControls.append(*ci);
}

void DaVehicle::ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, const AuGearPos& pos,
		qint8 /*chSelValue*/, CONTROL_CONTEXT cc, int msElapsed)
{
	Q_UNUSED(pos);
	Q_UNUSED(msElapsed);
	if(!pSceneAtom)
		return;
	if(!IsNormal())
		return;
	DaPerson* person = (DaPerson*)pSceneAtom;
	int seat = person->VehicleSeatIndex();
	bool controlSeat = IsControlSeat(seat);
	bool tt = false;	// true to call TickTock
	//qDebug() << "DaVehicle::ProcessControlInput seat" << seat << "controlSeat" << controlSeat;
	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch((CONTROLINPUT)ci.GetId())
		{
		case CI_UNUSED0:
			break;
		case CI_RequestLaunch:
			if(cc != CONTROL_Client && controlSeat)
				RequestLaunch(pSceneAtom);
			break;
		case CI_RequestRecovery:
			if(cc != CONTROL_Client && controlSeat)
				RequestRecovery(pSceneAtom);
			break;
		case CI_Service:
			if(cc != CONTROL_Client)
				Service(person);
			break;
		case CI_DoorRemote:
			if(cc != CONTROL_Client && controlSeat)
				RemoteDoorSwitch(pSceneAtom);
			break;
		case CI_FireGun:
			if(cc != CONTROL_Client && controlSeat && !IsParalyzed())
			{
				if(FireGun(pSceneAtom))
				{
					pSceneAtom->Stats().ItemUsedAsWeapon(m_cid, m_Quality);
					pSceneAtom->IncreaseSkill(AuSkill::VehicleCombat, 1);
				}
			}
			break;
		case CI_FireWeapon:
			if(cc != CONTROL_Client && controlSeat && !IsParalyzed())
			{
				if(FireWeapon(pSceneAtom))
				{
					pSceneAtom->Stats().ItemUsedAsWeapon(m_cid, m_Quality);
					pSceneAtom->IncreaseSkill(AuSkill::VehicleCombat, 1);
				}
			}
			break;
		case CI_SelectWeapon:
			if(cc != CONTROL_Client && controlSeat)
			{
				if(m_OrdSelect)
					m_OrdSelect = 0;
				else
					m_OrdSelect = 1;
				SetModified();
			}
			break;
		case CI_PitchUp:
			if(cc != CONTROL_Server && controlSeat && CanPitchAndRoll())
			{
				m_VehicleLock.lockForWrite();
				m_wActions &= ~ACT_PitchMask;
				m_wActions |= ACT_PitchUp;
				m_byRotSpeedX = (unsigned char)ci.GetRangeValue();
				m_tmLastPitchControl.restart();
				m_VehicleLock.unlock();
				tt = true;
			}
			break;
		case CI_PitchDown:
			if(cc != CONTROL_Server && controlSeat && CanPitchAndRoll() && !OnGround())
			{
				m_VehicleLock.lockForWrite();
				m_wActions &= ~ACT_PitchMask;
				m_wActions |= ACT_PitchDown;
				m_byRotSpeedX = (unsigned char)ci.GetRangeValue();
				m_tmLastPitchControl.restart();
				m_VehicleLock.unlock();
				tt = true;
			}
			break;
		case CI_RollRight:
			if(cc != CONTROL_Server && controlSeat && CanPitchAndRoll())
			{
				if(!IsOnGround())
				{
					m_VehicleLock.lockForWrite();
					m_wActions &= ~ACT_RollMask;
					m_wActions |= ACT_RollRight;
					m_byRotSpeedY = (unsigned char)ci.GetRangeValue();
					m_tmLastRollControl.restart();
					m_VehicleLock.unlock();
					tt = true;
				}
				else if(CanTurn())
				{
					m_VehicleLock.lockForWrite();
					m_wActions &= ~ACT_TurnMask;
					m_wActions |= ACT_TurnRight;
					m_byRotSpeedZ = (unsigned char)ci.GetRangeValue();
					m_tmLastTurnControl.restart();
					m_VehicleLock.unlock();
					tt = true;
				}
			}
			break;
		case CI_RollLeft:
			if(cc != CONTROL_Server && controlSeat && CanPitchAndRoll())
			{
				if(!IsOnGround())
				{
					m_VehicleLock.lockForWrite();
					m_wActions &= ~ACT_RollMask;
					m_wActions |= ACT_RollLeft;
					m_byRotSpeedY = (unsigned char)ci.GetRangeValue();
					m_tmLastRollControl.restart();
					m_VehicleLock.unlock();
					tt = true;
				}
				else if(CanTurn())
				{
					m_VehicleLock.lockForWrite();
					m_wActions &= ~ACT_TurnMask;
					m_wActions |= ACT_TurnLeft;
					m_byRotSpeedZ = (unsigned char)ci.GetRangeValue();
					m_tmLastTurnControl.restart();
					m_VehicleLock.unlock();
					tt = true;
				}
			}
			break;
		case CI_TurnRight:
			// Some vehicles, such as cars and trucks, must be moving to
			// turn. They cannot turn in place. 1fps is about 2/3mph.
			if(cc != CONTROL_Server && controlSeat && CanTurn())
			{
				m_VehicleLock.lockForWrite();
				m_wActions &= ~ACT_TurnMask;
				m_wActions |= ACT_TurnRight;
				m_byRotSpeedZ = (unsigned char)ci.GetRangeValue();
				m_tmLastTurnControl.restart();
				m_VehicleLock.unlock();
				tt = true;
			}
			break;
		case CI_TurnLeft:
			// Some vehicles, such as cars and trucks, must be moving to
			// turn. They cannot turn in place. 1fps is about 2/3mph.
			//qDebug() << "DaVehicle::ProcessControlInput CI_TurnLeft";
			if(cc != CONTROL_Server && controlSeat && CanTurn())
			{
				m_VehicleLock.lockForWrite();
				m_wActions &= ~ACT_TurnMask;
				m_wActions |= ACT_TurnLeft;
				m_byRotSpeedZ = (unsigned char)ci.GetRangeValue();
				m_tmLastTurnControl.restart();
				m_VehicleLock.unlock();
				tt = true;
			}
			break;

		case CI_AllStop:
			if(controlSeat &&
					(m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical) && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleControl.restart();
				m_tmLastThrottleFControl.restart();
				m_tmLastThrottleLControl.restart();
				m_tmLastThrottleVControl.restart();
				m_Throttle = 0;
				m_ThrustLateral = 0;
				m_ThrustLong = 0;
				m_ThrustVertical = 0;
				m_VehicleLock.unlock();
				tt = true;
			}
			break;
		case CI_IncreaseThrottle:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleControl.restart();
				switch(m_cid)
				{
				case CID_DirtBike:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					if(m_Throttle < 10)
					{
						if(m_Throttle < 9)
							m_Throttle += 2;
						else
							m_Throttle++;
					}
					break;
				case CID_SailBoat:
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
				case CID_SpaceRocket:
				case CID_Helicopter:
				case CID_Submarine:
					if(m_Throttle < 10)
						m_Throttle++;
					break;
				default:
					break;
				}
				m_VehicleLock.unlock();
			}
			break;
		case CI_MaxForwardThrottle:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleControl.restart();
				switch(m_cid)
				{
				case CID_DirtBike:
				case CID_SailBoat:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					break;
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
				case CID_SpaceRocket:
				case CID_Helicopter:
				case CID_Submarine:
					if(m_Throttle < 10)
						m_Throttle = 10;
					break;
				default:
					break;
				}
				m_VehicleLock.unlock();
			}
			break;
		case CI_DecreaseThrottle:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleControl.restart();
				switch(m_cid)
				{
				case CID_DirtBike:
					if(m_Throttle > -8)
						m_Throttle--;
					break;
				case CID_SailBoat:
				case CID_Submarine:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					if(m_Throttle > -9)
						m_Throttle--;
					break;
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_Throttle > -10)
						m_Throttle--;
					break;
				case CID_Helicopter:
					if(m_Throttle > 0)
						m_Throttle--;
					break;
				case CID_SpaceRocket:	// Weak reverse in rocket.
					if(m_Throttle > -2)
						m_Throttle--;
					break;
				default:
					break;
				}
				m_VehicleLock.unlock();
			}
			break;
		case CI_MaxReverseThrottle:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleControl.restart();
				switch(m_cid)
				{
				case CID_DirtBike:
				case CID_SailBoat:
					break;
				case CID_Submarine:
					if(m_Throttle > -9)
						m_Throttle = -9;
					break;
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_Throttle > -10)
						m_Throttle = -10;
					break;
				case CID_Helicopter:
					if(m_Throttle > 0)
						m_Throttle = 0;
					break;
				case CID_SpaceRocket:	// Weak reverse in rocket.
					if(m_Throttle > -2)
						m_Throttle = -2;
					break;
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					break;
				default:
					break;
				}
				m_VehicleLock.unlock();
			}
			break;
		case CI_SetThrottle:
			if(controlSeat && m_Throttle != ci.GetRangeValue() && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleControl.restart();
				qint8 oldThrottle = m_Throttle;
				m_Throttle = ci.GetRangeValue();
				switch(m_cid)
				{
				case CID_DirtBike:
					if(m_Throttle < -8)
						m_Throttle = -8;
					else if(m_Throttle > 10)
						m_Throttle = 10;
					break;
				case CID_SailBoat:
				case CID_Submarine:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					if(m_Throttle < -9)
						m_Throttle = -9;
					else if(m_Throttle > 10)
						m_Throttle = 10;
					break;
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_Throttle < -10)
						m_Throttle = -10;
					else if(m_Throttle > 10)
						m_Throttle = 10;
					break;
				case CID_Helicopter:
					if(m_Throttle < 0)
						m_Throttle = 0;
					else if(m_Throttle > 10)
						m_Throttle = 10;
					break;
				case CID_SpaceRocket:
					if(m_Throttle < -2)
						m_Throttle = -2;
					else if(m_Throttle > 10)
						m_Throttle = 10;
					break;
				default:
					break;
				}
				quint8 nuThrottle = m_Throttle;
				m_VehicleLock.unlock();

				// Turn off hover when throttle is changed on helicopter.
				if(oldThrottle != nuThrottle && CommodityId() == CID_Helicopter)
					SwitchHoldOff();
			}
			break;
		case CI_ThrustF:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleFControl.restart();
				switch(m_cid)
				{
				case CID_AdvSpaceFighter:
				case CID_SailBoat:
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_SpaceRocket:
				case CID_Submarine:
					if(m_ThrustLong < 10)
					{
						m_ThrustLong++;
						tt = true;
					}
					break;
				case CID_Helicopter:
					if(m_ThrustLong < 8)
					{
						m_ThrustLong++;
						tt = true;
					}
					break;
				case CID_DirtBike:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					if(m_ThrustLong < 10)
					{
						if(m_ThrustLong < 9)
							m_ThrustLong += 2;
						else
							m_ThrustLong++;
						tt = true;
					}
					break;
				default:
					break;
				}
				//int thrust = m_ThrustLong;
				m_VehicleLock.unlock();
				//qDebug() << "DaVehicle::ProcessControlInput" << Id() << Name() << "CI_ThrustF" << thrust;

				// Turn off hold when throttle is applied.
				SwitchHoldOff();
			}
			break;
		case CI_ThrustB:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleFControl.restart();
				switch(m_cid)
				{
				case CID_DirtBike:
					if(m_ThrustLong > -8)
					{
						m_ThrustLong--;
						tt = true;
					}
					break;
				case CID_SailBoat:
				case CID_Submarine:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					if(m_ThrustLong > -9)
					{
						m_ThrustLong--;
						tt = true;
					}
					break;
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_ThrustLong > -10)
					{
						m_ThrustLong--;
						tt = true;
					}
					break;
				case CID_Helicopter:
					if(m_ThrustLong > -8)
					{
						m_ThrustLong--;
						tt = true;
					}
					break;
				case CID_SpaceRocket:
					if(m_ThrustLong > -2)
					{
						m_ThrustLong--;
						tt = true;
					}
					break;
				default:
					break;
				}
				m_VehicleLock.unlock();

				// Turn off hold when throttle is applied.
				SwitchHoldOff();
			}
			break;
		case CI_IncreaseThrottleL:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleLControl.restart();
				switch(m_cid)
				{
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_ThrustLateral < 5)
					{
						m_ThrustLateral++;
						tt = true;
					}
					break;
				case CID_Helicopter:
					if(m_ThrustLateral < 4)
					{
						m_ThrustLateral++;
						tt = true;
					}
					break;
				case CID_SpaceRocket:
					if(m_ThrustLateral < 2)
					{
						m_ThrustLateral++;
						tt = true;
					}
					break;
				case CID_Submarine:
					if(m_ThrustLateral < 9)
					{
						m_ThrustLateral++;
						tt = true;
					}
					break;
				case CID_DirtBike:
				case CID_SailBoat:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
				default:
					break;
				}
				m_VehicleLock.unlock();

				// Turn off hold when throttle is applied.
				SwitchHoldOff();
			}
			break;
		case CI_DecreaseThrottleL:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleLControl.restart();
				switch(m_cid)
				{
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_ThrustLateral > -5)
					{
						m_ThrustLateral--;
						tt = true;
					}
					break;
				case CID_Helicopter:
					if(m_ThrustLateral > -4)
					{
						m_ThrustLateral--;
						tt = true;
					}
					break;
				case CID_SpaceRocket:
					if(m_ThrustLateral > -2)
					{
						m_ThrustLateral--;
						tt = true;
					}
					break;
				case CID_Submarine:
					if(m_ThrustLateral > -9)
					{
						m_ThrustLateral--;
						tt = true;
					}
					break;
				case CID_DirtBike:
				case CID_SailBoat:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
				default:
					break;
				}
				m_VehicleLock.unlock();

				// Turn off hold when throttle is applied.
				SwitchHoldOff();
			}
			break;
		case CI_IncreaseThrottleV:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleVControl.restart();
				switch(m_cid)
				{
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_ThrustVertical < 6)
					{
						m_ThrustVertical++;
						tt = true;
					}
					break;
				case CID_Helicopter:
					if(m_ThrustVertical < 10)
					{
						m_ThrustVertical++;
						tt = true;
					}
					break;
				case CID_SpaceRocket:
					if(m_ThrustVertical < 2)
					{
						m_ThrustVertical++;
						tt = true;
					}
					break;
				case CID_Submarine:
					if(m_ThrustVertical < 9)
					{
						m_ThrustVertical++;
						tt = true;
					}
					break;
				case CID_DirtBike:
				case CID_SailBoat:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
				default:
					break;
				}
				m_VehicleLock.unlock();

				// Turn off hold when throttle is applied.
				SwitchHoldOff();
			}
			break;
		case CI_DecreaseThrottleV:
			if(controlSeat && !IsLaunching() && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastThrottleVControl.restart();
				switch(m_cid)
				{
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
					if(m_ThrustVertical > -6)
					{
						m_ThrustVertical--;
						tt = true;
					}
					break;
				case CID_Helicopter:
					if(m_ThrustVertical > -10)
					{
						m_ThrustVertical--;
						tt = true;
					}
					break;
				case CID_SpaceRocket:
					if(m_ThrustVertical > -2)
					{
						m_ThrustVertical--;
						tt = true;
					}
					break;
				case CID_Submarine:
					if(m_ThrustVertical > -9)
					{
						m_ThrustVertical--;
						tt = true;
					}
					break;
				case CID_DirtBike:
				case CID_SailBoat:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
				default:
					break;
				}
				m_VehicleLock.unlock();

				// Turn off hold when throttle is applied.
				SwitchHoldOff();
			}
			break;
		case CI_Brakes:
			if(controlSeat && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastBrake.restart();
				switch(m_cid)
				{
				case CID_DirtBike:
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
				case CID_SpaceRocket:
				case CID_Helicopter:
				case CID_Submarine:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
					m_wActions |= ACT_Brake;
					tt = true;
					break;
				case CID_SailBoat:
				default:
					break;
				}
				m_VehicleLock.unlock();
			}
			break;
		case CI_BrakesLV:
			if(controlSeat && !IsParalyzed())
			{
				m_VehicleLock.lockForWrite();
				m_tmLastBrakeLV.restart();
				switch(m_cid)
				{
				case CID_SpaceFighter:
				case CID_SpaceTransport:
				case CID_AdvSpaceFighter:
				case CID_SpaceRocket:
				case CID_Helicopter:
				case CID_Submarine:
					m_wActions |= (ACT_BrakeLateral|ACT_BrakeVertical);
					tt = true;
					break;
				case CID_DirtBike:
				case CID_SailBoat:
				case CID_SUV:
				case CID_APC:
				case CID_FarmMachinery:
				default:
					break;
				}
				m_VehicleLock.unlock();
			}
			break;
		case CI_NextSeat:
			if(cc != CONTROL_Server && m_Model)
			{
				int nextSeat = NextUnoccupiedSeat(seat);
				ChangeSeats(person, seat, nextSeat);
			}
			break;
		case CI_PreviousSeat:
			if(cc != CONTROL_Server && m_Model)
			{
				int prevSeat = PreviousUnoccupiedSeat(seat);
				ChangeSeats(person, seat, prevSeat);
			}
			break;
		case CI_Seat0:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(0))
				ChangeSeats(person, seat, 0);
			break;
		case CI_Seat1:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(1))
				ChangeSeats(person, seat, 1);
			break;
		case CI_Seat2:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(2))
				ChangeSeats(person, seat, 2);
			break;
		case CI_Seat3:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(3))
				ChangeSeats(person, seat, 3);
			break;
		case CI_Seat4:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(4))
				ChangeSeats(person, seat, 4);
			break;
		case CI_Seat5:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(5))
				ChangeSeats(person, seat, 5);
			break;
		case CI_Seat6:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(6))
				ChangeSeats(person, seat, 6);
			break;
		case CI_Seat7:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(7))
				ChangeSeats(person, seat, 7);
			break;
		case CI_Seat8:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(8))
				ChangeSeats(person, seat, 8);
			break;
		case CI_Seat9:
			if(cc != CONTROL_Server && m_Model && !IsSeatOccupied(9))
				ChangeSeats(person, seat, 9);
			break;

		case CI_Hold:
			if(cc != CONTROL_Client && controlSeat && !IsLaunching() && !IsParalyzed())
			{
				SwitchHoldToggle();
				if(!IsHolding())
					ForceMotion();
				SetModified();
			}
			break;

		case CI_EngineOnOff:
			if(cc != CONTROL_Client && controlSeat && !IsLaunching() && !IsParalyzed())
			{
				// Can always turn engine off but must have fuel to turn it on.
				if(IsEngineOn())
				{
					SwitchEngineOff();	// Turn off engine
					SwitchHoldOff();	// Cancel brake hold.
				}
				else if(Fuel() > 0)
				{
					// Turning on the engine costs an extra unit of fuel. This eliminates a possible cheat by turning off the
					// engine just before the minute and then turning it back on again. We only consume fuel at the whole minute.
					// Per Shane, this is a 50% probability so a unit of fuel is not ALWAYS burned when the engine is turned on.
					if(g_Au.Dice().Roll(1,2) == 2)
					{
						m_VehicleLock.lockForWrite();
						m_Fuel--;
						m_tmFuelChanged.Set();
						m_VehicleLock.unlock();
					}
					SwitchEngineOn();				// turn on
				}

				// Space rocket parked inside a spacecraft must not start moving or it falls to the floor.
				if(IsSpaceRocket() && IsInsideCapsule() &&					// Inside a Capsule
						!ThrustLateral() && !ThrustLongitudinal() && !ThrustVertical() &&	// No throttles on.
						Vector().Length2() < 1)									// Not moving.
					break;
				ForceMotion();
			}
			break;

		case CI_OrbitRingsToggle:
			if(!IsSpaceVehicle())
				break;
			if(cc != CONTROL_Client && controlSeat)
				SwitchOrbitRingsToggle();
			break;

		case CI_LightOnOff:
			if(cc != CONTROL_Client && controlSeat && HasLight())
				SwitchLightToggle();
			break;

		case CI_ReleaseMotherShip:
			if(cc != CONTROL_Client && controlSeat)
			{
				QString text;
				if(ClearMotherShip())
				{
					text = QString("%1 assignment released.").arg(Name());
					SetModified();
				}
				else
					text = QString("%1 is not assigned to a spacecraft.").arg(Name());
				AuCommThought msg(text, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			break;

		case CI_LookBack:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI);							// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookForward:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				//t.RotateZ(PI);						// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookLeft:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI/2);						// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookLeftBack:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI*3/4);						// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookLeftForward:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI/4);						// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookRight:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(-PI/2);						// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookRightBack:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(-PI*3/4);						// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookRightForward:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(pSceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(-PI/4);						// Set facing angle. 0 is straight ahead.
				pSceneAtom->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
				pSceneAtom->TiltHeadTo(0);
			}
			break;
		}
	}
	SetModified();
	if(tt && !IsTicking())
		TickTock();	// Get this crate moving.
}

// Moves an occupant from one seat to another.
// Called on the client.
void DaVehicle::ChangeSeats(DaPerson* occupant, int fromSeat, int toSeat)
{
	if(!occupant)
		return;
	if(fromSeat == toSeat)
		return;
	if(fromSeat < 0 || toSeat < 0)
		return;
	if(!m_Model)
		return;
	int seats = m_Model->SeatCount();
	if(!seats)
		return;
	if(fromSeat >= seats || toSeat >= seats)
		return;
	//qDebug() << "DaVehicle::ChangeSeats" << Id() << Name() << "occupant" << occupant->Id() << occupant->Name() << "fromSeat"
		//<< fromSeat << "toSeat" << toSeat;

	AuPoint fromSeatLoc = m_Model->SeatLoc(fromSeat);
	AuPoint toSeatLoc = m_Model->SeatLoc(toSeat);
	AuPoint moveVec = toSeatLoc - fromSeatLoc;
	AuMatrix4x4 t;
	t.Translate(moveVec);
	t *= occupant->Transform();
	occupant->SetVehicleSeatIndex(toSeat);
	occupant->ChangePosAndOrientNotParent(AuAtomPos(Id(), t));
	occupant->InvalidateEnvironment();
	occupant->SetModified();

	//DHH Probably need to verify the pose of the occupant is valid for the
	//DHH nu seat. So far, none of the vehicle models has seats with different
	//DHH requirements.
}

bool DaVehicle::IsValidSeatPose(int seatIndex, const AuPose& pose) const
{
	if(!m_Model)
		return false;
	int validPoses = m_Model->SeatPose(seatIndex);
	switch(pose.Pose())
	{
	case AuPose::Crouch:
		return (validPoses & Au3dsGLSLStation::Crouch) ? true : false;
	case AuPose::Prone:
		return (validPoses & Au3dsGLSLStation::Prone) ? true : false;
	case AuPose::SitOnGround:
		return (validPoses & Au3dsGLSLStation::Sit) ? true : false;
	case AuPose::Stand:
		return (validPoses & Au3dsGLSLStation::Stand) ? true : false;
	case AuPose::Straddle:
		return (validPoses & (Au3dsGLSLStation::Seated|Au3dsGLSLStation::Straddle)) ?
			true : false;
	case AuPose::EyesOnly:
		return (validPoses & Au3dsGLSLStation::EyesOnly) ? true : false;
	default:
		break;
	}
	return false;
}

// Render OpenAL audio sources
void DaVehicle::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	if(!al)
		return;
	float gain = RelativeAudioGain((const DaPerson*)sceneAtom) * fade;
	float pitch = 1;
	SOUND_EFFECT engineSnd = SOUND_None;
	float ggain = 0;
	float gpitch = 1;
	SOUND_EFFECT groundSnd = SOUND_None;
	AuPoint vec(Vector());
	if(!IsFuzzyZero(gain))
	{
		// Sound is audible.
		switch(m_cid)
		{
		case CID_SailBoat:
			if(IsInVisibleWater() && (ThrustLongitudinal() || !vec.IsFuzzyZero()))
			{
				// Only makes sail noise when throttle/sails up otherwise it never
				// stops making noise, even when nobody is in it.
				engineSnd = SOUND_SailBoat;
				pitch = fabs(vec.y) / 100.f + 1.f;
			}
			if(!vec.IsFuzzyZero() && OnGround() != OG_None)
			{
				// Make dragging on ground sound.
				groundSnd = (SOUND_EFFECT)(SOUND_DragForest + OnGround());
				ggain = gain * 4;
				gpitch = fabs(vec.y) / 100.f + 1.f;
				gpitch /= 2;
			}
			break;
		case CID_Submarine:
			if(IsEngineOn())
			{
				engineSnd = SOUND_Submarine;
				if(!IsFuzzyZero(vec.y))
				{
					engineSnd = SOUND_Submarine;
					float mph = fabs(vec.y * 3600.f/5280.f);
					pitch = mph / 30.f + .97f;
					if(pitch > 2.f)
						pitch = 2.f;
				}
			}
			if(!vec.IsFuzzyZero() && OnGround() != OG_None)
			{
				// Make dragging on ground sound.
				groundSnd = (SOUND_EFFECT)(SOUND_DragForest + OnGround());
				ggain = gain * 5;
				gpitch = fabs(vec.y) / 100.f + 1.f;
				gpitch /= 2;
			}
			break;
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
			if(!IsEngineOn())
				break;
			engineSnd = SOUND_SUVIdle;
			gain *= 2;	// sound file is quiet
			if(!IsFuzzyZero(vec.y))
			{
				// Lots of transmission choices for this guy from 4 to 6 speeds.
				// Last gearSpeed entry is "top" speed.
				float mph = fabs(vec.y * 3600.f/5280.f);
				const int gearSpeeds[] = {0, 12, 28, 43, 90};
				const int gears = sizeof(gearSpeeds)/sizeof(int);
				for(int ig=1; ig<gears; ig++)
				{
					if(mph <= gearSpeeds[ig])
					{
						pitch = (mph - gearSpeeds[ig-1]) /	// offset into range
							(gearSpeeds[ig] - gearSpeeds[ig-1]);	// the range
						break;
					}
				}
				pitch += .95f;
				if(pitch > 2.f)
					pitch = 2.f;
			}
			break;
		case CID_DirtBike:
			if(!IsEngineOn())
				break;
			if(IsFuzzyZero(vec.y))
				engineSnd = SOUND_DirtBikeIdle;
			else
			{
				// Gear shifting pitch transitions using Jim Pearson's gear
				// shift speed estimates for a Honda XR400.
				engineSnd = SOUND_DirtBike2000;
				float mph = fabs(vec.y * 3600.f/5280.f);
				const int gearSpeeds[] = {0, 10, 18, 25, 30, 45, 75};
				const int gears = sizeof(gearSpeeds)/sizeof(int);
				for(int ig=1; ig<gears; ig++)
				{
					if(mph <= gearSpeeds[ig])
					{
						pitch = (mph - gearSpeeds[ig-1]) /	// offset into range
							(gearSpeeds[ig] - gearSpeeds[ig-1]);	// the range
						break;
					}
				}
				pitch += .96f;
				if(pitch > 2.f)
					pitch = 2.f;
			}
			break;
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_AdvSpaceFighter:
			if(IsEngineOn())
			{
				m_VehicleLock.lockForRead();
				int acti = m_wActions;
				m_VehicleLock.unlock();

				engineSnd = SOUND_GravityDriveExt;
				float throttleGain;
				if(acti & ACT_Brake)
					throttleGain = 10;
				else
					throttleGain = abs(ThrustLongitudinal());
				float throttleLatGain;
				if(acti & ACT_BrakeLateral)
					throttleLatGain = 5;
				else
					throttleLatGain = abs(ThrustLateral());
				float throttleVertGain;
				if(acti & ACT_BrakeVertical)
					throttleVertGain = 6;
				else
					throttleVertGain = abs(ThrustVertical());

				// 21 is max of all throttles
				gain *= throttleGain + throttleLatGain + throttleVertGain + 5;
				pitch = .5f + (abs(ThrustLongitudinal()) + abs(ThrustLateral()) + abs(ThrustVertical())) * 1.5f/21.f;
				if(pitch > 2)
					pitch = 2;
				if(sceneAtom->Parent() == this)
					gain /= 4;
			}
			break;
		case CID_SpaceRocket:
			if(IsEngineOn())
			{
				m_VehicleLock.lockForRead();
				int acti = m_wActions;
				m_VehicleLock.unlock();

				engineSnd = SOUND_GravityDriveExt;
				float throttleGain;
				if(acti & ACT_Brake)
					throttleGain = 10;
				else
					throttleGain = abs(ThrustLongitudinal());
				float throttleLatGain;
				if(acti & ACT_BrakeLateral)
					throttleLatGain = 2;
				else
					throttleLatGain = abs(ThrustLateral());
				float throttleVertGain;
				if(acti & ACT_BrakeVertical)
					throttleVertGain = 2;
				else
					throttleVertGain = abs(ThrustVertical());

				// 14 is max of all throttles
				gain *= throttleGain + throttleLatGain + throttleVertGain + 5;
				pitch = 1.f + (abs(ThrustLongitudinal()) + abs(ThrustLateral()) + abs(ThrustVertical())) * 1.f/14.f;
				if(pitch > 2)
					pitch = 2;
			}
			break;
		case CID_Helicopter:
			if(IsEngineOn())
			{
				engineSnd = SOUND_Helicopter;
				// Helicopter throttle is never negative. Vertical thrust value can be negative.
				float throttleGain;
				if(ThrustVertical() > m_Throttle)
					throttleGain = ThrustVertical();
				else
					throttleGain = m_Throttle;

				gain *= throttleGain + 5;
				if(sceneAtom->Parent() == this)
					gain /= 4;

				// 10 is max throttleGain
				pitch = .5f + throttleGain * 1.5f/10.f;	// Range is .5 to 2.
				if(pitch > 2)
					pitch = 2;
			}
			break;
		default:
			break;
		}
	}

	AuPoint epos = model.Translation();
	SOUND_EFFECT cur = (SOUND_EFFECT)m_Sound[0];
	al->Play(m_AudioVehicle[0], cur, engineSnd, epos.x, epos.y, epos.z, gain, pitch, true,
			(float)AuSound::SoundReferenceDistance((SOUND_EFFECT)m_Sound[0]),
			AuSound::SoundRolloffFactor((SOUND_EFFECT)m_Sound[0]), 0);
	m_Sound[0] = cur;
	AuPoint gpos = model.Project(AuPoint(0,0,-GroundHeightAboveCG()));
	cur = (SOUND_EFFECT)m_Sound[1];
	al->Play(m_AudioVehicle[1], cur, groundSnd, gpos.x, gpos.y, gpos.z, ggain, gpitch, true,
			(float)AuSound::SoundReferenceDistance((SOUND_EFFECT)m_Sound[1]),
			AuSound::SoundRolloffFactor((SOUND_EFFECT)m_Sound[1]), 0);
	m_Sound[1] = cur;
}

// Acceleration Force
//  A = V / T * 2
// Where:
//  A is acceleration in ft/sec/sec
//  V is velocity in ft/sec
//  T is time in sec
//  * 2 due to drag. Otherwise it takes twice as long to get up to speed.
// 75mi/hr * 5280ft/3600sec / 6sec = 18.33ft/sec/sec * 2 = 36.66
#define VEHICLE_ACCELERATION(mph, sec)	(5280./3600. * (mph+10) / sec * 2.)	// Extra 10mph just for a bit of extra power.
const double s_APCAccel = VEHICLE_ACCELERATION(60, 10.5);
const double s_DirtBikeAccel = VEHICLE_ACCELERATION(60, 6);
const double s_FarmMachineryAccel = VEHICLE_ACCELERATION(100, 10);	// Lots of power, limited by max speed.
const double s_HelicopterAccel = 100.;
const double s_SailBoatAccel = VEHICLE_ACCELERATION(25, 15);
const double s_SpaceFighterAccel = 1023.;
const double s_SpaceRocketAccel = 767.;
const double s_SpaceTransportAccel = 600.;
const double s_SubmarineAccel = VEHICLE_ACCELERATION(35, 15);
const double s_SUVAccel = VEHICLE_ACCELERATION(60, 10.5);

// Some vehicle max speeds are limited by gearing, not by air resistance.
const double s_APCMaxSpeed = 120. * 5280./3600.;
const double s_APCMaxSpeedInWater = s_APCMaxSpeed/3.;
const double s_APCMaxSpeedUnderWater = 4. * 5280./3600.;
const double s_DirtBikeMaxSpeed = 95. * 5280./3600.;
const double s_DirtBikeMaxSpeedInWater = s_DirtBikeMaxSpeed/2.;
const double s_DirtBikeMaxSpeedUnderWater = 5. * 5280./3600.;
const double s_FarmMachineryMaxSpeed = 25. * 5280./3600.;
const double s_FarmMachineryMaxSpeedInWater = s_FarmMachineryMaxSpeed/4.;
const double s_FarmMachineryMaxSpeedUnderWater = 2. * 5280./3600.;
const double s_SUVMaxSpeed = 125. * 5280./3600.;
const double s_SUVMaxSpeedInWater = s_SUVMaxSpeed/3.;
const double s_SUVMaxSpeedUnderWater = 5. * 5280./3600.;

// Nominal Friction Force
// Friction force in ideal movement environment for vehicle.
// Needs to be less that the vehicle's acceleration force or it will not move.
// Used to calculate a Drag Coefficient that produces a desired maximum.
// Determines rate of deceleration in ft/sec/sec when no throttle applied.
const double s_APCFric = 5;
const double s_DirtBikeFric = 4;
const double s_FarmMachineryFric = 5;
const double s_HelicopterFric = 2;
const double s_SailBoatFric = 1;
const double s_SpaceFighterFric = 1;
const double s_SpaceRocketFric = .75;
const double s_SpaceTransportFric = 1;
const double s_SubmarineFric = 3;
const double s_SUVFric = 5;

// Drag Force in a fluid.
//
//  D = 1/2 CpAv*v
//
// Acceleration Force at equilibrium.
//
//  a = Dmax + F
//
// a is acceleration force of vehicle
// Dmax is drag force at desired maximum speed in standard atmosphere.
// F is friction.
//
//  C = (a - F)*2 / (pAv*v)
//
// C is Drag Coefficient
// a is vehicle acceleration force
// F is friction.
// p is fluid density (water = .5, standard atmosphere = .05)
// A is CrossSectionalArea
// v is equilibrium velocity in ft/sec
#define DRAG_COEFFICIENT(a,F,p,A,v) ((a-F)*2/(p*A*v*v))
float DaVehicle::DragCoefficient() const
{
	switch(m_cid)
	{
	case CID_DirtBike:
		return DRAG_COEFFICIENT((float)s_DirtBikeAccel, (float)s_DirtBikeFric, .05f, 2.84f, (float)s_DirtBikeMaxSpeed*4);
	case CID_SailBoat:
		return DRAG_COEFFICIENT((float)s_SailBoatAccel, (float)s_SailBoatFric, .5f, 40, 25.f*5280.f/3600.f);
	case CID_SpaceFighter:
	case CID_AdvSpaceFighter:
		return DRAG_COEFFICIENT((float)s_SpaceFighterAccel, (float)s_SpaceFighterFric, .05f, 60, 1023.f);
	case CID_SpaceTransport:
		return DRAG_COEFFICIENT((float)s_SpaceTransportAccel, (float)s_SpaceTransportFric, .05f, 120, 750.f);
	case CID_SpaceRocket:
		return DRAG_COEFFICIENT((float)s_SpaceRocketAccel, (float)s_SpaceRocketFric, .05f, 16, 1023.f);
	case CID_Helicopter:
		return DRAG_COEFFICIENT((float)s_HelicopterAccel, (float)s_HelicopterFric, .05f, 45, 250.f);
	case CID_Submarine:
		return DRAG_COEFFICIENT((float)s_SubmarineAccel, (float)s_SubmarineFric, .5f, 73, 35.f*5280.f/3600.f);
	case CID_SUV:
		return DRAG_COEFFICIENT((float)s_SUVAccel, (float)s_SUVFric, .05f, 55, (float)s_SUVMaxSpeed*4);
	case CID_APC:
		return DRAG_COEFFICIENT((float)s_APCAccel, (float)s_APCFric, .05f, 60, (float)s_APCMaxSpeed*4);
	case CID_FarmMachinery:
		return DRAG_COEFFICIENT((float)s_FarmMachineryAccel, (float)s_FarmMachineryFric, .05f, 55,
				(float)s_FarmMachineryMaxSpeed*4);
	default:
		return DaAtom::DragCoefficient();
	}
}

AuPoint DaVehicle::CrossSectionalArea() const
{
	switch(m_cid)
	{
	case CID_DirtBike:
		return AuPoint(13.74,2.84,4.28);
	case CID_SailBoat:
		return AuPoint(80,40,249);
	case CID_SpaceFighter:
	case CID_AdvSpaceFighter:
		return AuPoint(131,60,350);
	case CID_SpaceTransport:
		return AuPoint(250,120,700);
	case CID_SpaceRocket:
		return AuPoint(170,16,170);
	case CID_Helicopter:
		return AuPoint(125,45,150);
	case CID_Submarine:
		return AuPoint(167,73,200);
	case CID_SUV:
		return AuPoint(208,55,153);
	case CID_APC:
		return AuPoint(208,60,153);
	case CID_FarmMachinery:
		return AuPoint(208,55,153);
	default:
		return DaAtom::CrossSectionalArea();
	}
}

// Acceleration/deceleration in ft/sec/sec due to gravity and other factors.
AuPoint DaVehicle::Acceleration() const
{
	if(InArenaCannotAccelerate())
		return AuPoint(0,0);

	// Engine thrust factor per throttle setting.
	const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};

	// Determine if brake is on.
	m_VehicleLock.lockForRead();
	int acti = m_wActions;
	m_VehicleLock.unlock();
	bool brake = false;
	bool brakeLateral = false;
	bool brakeVertical = false;
	if(IsHolding())
	{
		brake = true;
		brakeLateral = true;
		brakeVertical = true;
	}
	else
	{
		if(acti & ACT_Brake)
			brake = true;
		if(acti & ACT_BrakeLateral)
			brakeLateral = true;
		if(acti & ACT_BrakeVertical)
			brakeVertical = true;
	}

	// Gravity is usually applied whenever this vehicle is not touching the
	// ground. There are various exceptions handled in DaAtom::Acceleration.
	int applyGravity = 1;

	AuPoint vec(Vector());

	AuPoint accel;
	switch(m_cid)
	{
	case CID_SUV:
		if(!IsEngineOn())
			break;
		if(IsOnGround() || WaterDepth() > 1.f)
		{
			// Maximum speed on the ground is limited by gearing, not by
			// air resistance. Reduce acceleration when approaching max speed.
			double power = 1;
			if((vec.y >= 0 && ThrustLongitudinal() > 0) ||
					(vec.y <= 0 && ThrustLongitudinal() < 0))
			{
				// Moving forward and accelerating forward or
				// moving backward and accelerating backward.
				if(IsInWater() && (IsCGUnderWater() || IsOnWater()))
					power = 1. -fabs(vec.y) / s_SUVMaxSpeedUnderWater;
				else if(WaterDepth() > 1.f)
					power = 1. -fabs(vec.y) / s_SUVMaxSpeedInWater;
				else
					power = 1. -fabs(vec.y) / s_SUVMaxSpeed;
				if(power < 0)
					power = 0;
			}

			// This floater can move in water.
			if(ThrustLongitudinal() >= 0)
				accel.y = s_SUVAccel * power * engineThrust[ThrustLongitudinal()];
			else
				accel.y = s_SUVAccel * power * -engineThrust[-ThrustLongitudinal()];

			// Weaken gravity while touching a road so going up and down steep road hills is easy.
			//if(IsTouchingGround() && InIndustry() == IID_Road)
				//applyGravity = 2;
		}
		break;
	case CID_APC:
		if(!IsEngineOn())
			break;
		if(IsOnGround() || WaterDepth() > 1.f)
		{
			// Maximum speed on the ground is limited by gearing, not by
			// air resistance. Reduce acceleration when approaching max speed.
			double power = 1;
			if((vec.y >= 0 && ThrustLongitudinal() > 0) ||
					(vec.y <= 0 && ThrustLongitudinal() < 0))
			{
				// Moving forward and accelerating forward or
				// moving backward and accelerating backward.
				if(IsInWater() && (IsCGUnderWater() || IsOnWater()))
					power = 1. -fabs(vec.y) / s_APCMaxSpeedUnderWater;
				else if(WaterDepth() > 1.f)
					power = 1. -fabs(vec.y) / s_APCMaxSpeedInWater;
				else
					power = 1. -fabs(vec.y) / s_APCMaxSpeed;
				if(power < 0)
					power = 0;
			}

			// This floater can move in water.
			if(ThrustLongitudinal() >= 0)
				accel.y = s_APCAccel * power * engineThrust[ThrustLongitudinal()];
			else
				accel.y = s_APCAccel * power * -engineThrust[-ThrustLongitudinal()];

			// Weaken gravity while touching a road so going up and down steep road hills is easy.
			//if(IsTouchingGround() && InIndustry() == IID_Road)
				//applyGravity = 2;
		}
		break;
	case CID_FarmMachinery:
		if(!IsEngineOn())
			break;
		if(IsOnGround() || WaterDepth() > 1.f)
		{
			// Maximum speed on the ground is limited by gearing, not by
			// air resistance. Reduce acceleration when approaching max speed.
			double power = 1;
			if((vec.y >= 0 && ThrustLongitudinal() > 0) ||
					(vec.y <= 0 && ThrustLongitudinal() < 0))
			{
				// Moving forward and accelerating forward or
				// moving backward and accelerating backward.
				if(IsInWater() && (IsCGUnderWater() || IsOnWater()))
					power = 1. -fabs(vec.y) / s_FarmMachineryMaxSpeedUnderWater;
				else if(WaterDepth() > 1.f)
					power = 1. -fabs(vec.y) / s_FarmMachineryMaxSpeedInWater;
				else
					power = 1. -fabs(vec.y) / s_FarmMachineryMaxSpeed;
				if(power < 0)
					power = 0;
			}

			// This floater can move in water.
			if(ThrustLongitudinal() >= 0)
				accel.y = s_FarmMachineryAccel * power * engineThrust[ThrustLongitudinal()];
			else
				accel.y = s_FarmMachineryAccel * power * -engineThrust[-ThrustLongitudinal()];

			// Weaken gravity while touching a road so going up and down steep road hills is easy.
			//if(IsTouchingGround() && InIndustry() == IID_Road)
				//applyGravity = 2;
		}
		break;
	case CID_DirtBike:
		if(!IsEngineOn())
			break;
		if(IsOnGround())
		{
			// Maximum speed on the ground is limited by gearing, not by
			// air resistance. Reduce acceleration when approaching max speed.
			double power = 1;
			if((vec.y >= 0 && ThrustLongitudinal() > 0) ||
					(vec.y <= 0 && ThrustLongitudinal() < 0))
			{
				// Moving forward and accelerating forward or
				// moving backward and accelerating backward.
				if(IsInWater() && IsCGUnderWater())
					power = 1. -fabs(vec.y) / s_DirtBikeMaxSpeedUnderWater;
				else if(WaterDepth() > 6.f/12.f)
					power = 1. -fabs(vec.y) / s_DirtBikeMaxSpeedInWater;
				else
					power = 1. -fabs(vec.y) / s_DirtBikeMaxSpeed;
				if(power < 0)
					power = 0;
			}

			if(ThrustLongitudinal() >= 0)
				accel.y = s_DirtBikeAccel * power * engineThrust[ThrustLongitudinal()];
			else
				accel.y = s_DirtBikeAccel * power * -engineThrust[-ThrustLongitudinal()];

			// Weaken gravity while touching a road so going up and down steep road hills is easy.
			//if(IsTouchingGround() && InIndustry() == IID_Road)
				//applyGravity = 2;
		}
		break;
	case CID_Submarine:
		if(!IsEngineOn())
			break;
		if(IsOnGround() || WaterDepth() > 3.f/12.f)
		{
			if(WaterDepth() > 3.f/12.f)
			{
				if(ThrustLateral() >= 0)
					accel.x = s_SubmarineAccel * engineThrust[ThrustLateral()];
				else
					accel.x = s_SubmarineAccel * -engineThrust[-ThrustLateral()];
			}
			if(ThrustLongitudinal() >= 0)
				accel.y = s_SubmarineAccel * engineThrust[ThrustLongitudinal()];
			else
				accel.y = s_SubmarineAccel * -engineThrust[-ThrustLongitudinal()];
		}
		if(IsCGSubmerged())
		{
			if(ThrustVertical() >= 0)
				accel.z = s_SubmarineAccel * engineThrust[ThrustVertical()];
			else
				accel.z = s_SubmarineAccel * -engineThrust[-ThrustVertical()];
		}
		break;
	case CID_SailBoat:
		// Throttle ranges from -5 to +10.
		// Let's assume the sailboat takes about 15 seconds to get up to
		// a maximum speed of 25mph. Its acceleration is...
		//  A = M / T
		// Where:
		//  A is Acceleration
		//  M is MaxSpeed in ft/sec
		//  T is time in sec
		// 25mi/hr * 5280ft/3600sec / 15sec = 2.44ft/sec/sec
		// Atmosphere density is applied hence the multiply by two
		// because standard density is .5. Atmosphere density is a factor
		// because this thing is blown by the wind.
		{
			float sailBoatForce = s_SailBoatAccel * AirDensity() * 2.f;
			accel.y = sailBoatForce * ThrustLongitudinal()/10.;
		}
		break;
	case CID_SpaceFighter:
	case CID_AdvSpaceFighter:
		if(IsEngineOn())
		{
			// Engine power will overcome the force of gravity.
			// Gravity is not applied when operated outside the atmosphere so
			// the fighter can hold its position.
			if(!IsInAtmosphere() && !InBlackHoleGravity())
				applyGravity = 0;

			// Lateral acceleration.
			if(!brakeLateral && ThrustLateral())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLateral() < 0)
					accel.x = -s_SpaceFighterAccel * engineThrust[-ThrustLateral()];
				else
					accel.x = s_SpaceFighterAccel * engineThrust[ThrustLateral()];
			}

			// Forward/reverse acceleration.
			if(!brake && ThrustLongitudinal())
			{
				// Throttle ranges from -10 to +10.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLongitudinal() < 0)
					accel.y = -s_SpaceFighterAccel * engineThrust[-ThrustLongitudinal()];
				else
					accel.y = s_SpaceFighterAccel * engineThrust[ThrustLongitudinal()];
			}

			// Vertical acceleration.
			if(!brakeVertical && ThrustVertical())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustVertical() < 0)
					accel.z = -s_SpaceFighterAccel * engineThrust[-ThrustVertical()];
				else
					accel.z = s_SpaceFighterAccel * engineThrust[ThrustVertical()];
			}
		}
		break;
	case CID_SpaceTransport:
		if(IsEngineOn())
		{
			// Engine power will overcome the force of gravity.
			// Gravity is not applied when operated outside the atmosphere so
			// the fighter can hold its position.
			if(!IsInAtmosphere() && !InBlackHoleGravity())
				applyGravity = 0;

			// Lateral acceleration.
			if(!brakeLateral && ThrustLateral())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLateral() < 0)
					accel.x = -s_SpaceTransportAccel * engineThrust[-ThrustLateral()];
				else
					accel.x = s_SpaceTransportAccel * engineThrust[ThrustLateral()];
			}

			// Forward/reverse acceleration.
			if(!brake && ThrustLongitudinal())
			{
				// Throttle ranges from -10 to +10.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLongitudinal() < 0)
					accel.y = -s_SpaceTransportAccel * engineThrust[-ThrustLongitudinal()];
				else
					accel.y = s_SpaceTransportAccel * engineThrust[ThrustLongitudinal()];
			}

			// Vertical acceleration.
			if(!brakeVertical && ThrustVertical())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustVertical() < 0)
					accel.z = -s_SpaceTransportAccel * engineThrust[-ThrustVertical()];
				else
					accel.z = s_SpaceTransportAccel * engineThrust[ThrustVertical()];
			}
		}
		break;
	case CID_SpaceRocket:
		if(IsEngineOn())
		{
			if(!IsOnGround() &&													// Not on the ground or a ship deck.
					!ThrustLateral() && !ThrustLongitudinal() && !ThrustVertical() &&	// No throttles on.
					vec.Length2() < FeetPerMeter()*3*FeetPerMeter()*3 &&		// Not moving significantly, < 3m/s.
					!IsOccupied())												// Not occupied.
			{
				// Unoccupied in space with engine on, autobrake to hold position, mainly for the tutorial.
				brake = true;
				brakeLateral = true;
				brakeVertical = true;
			}
			else if(IsInsideCapsule() &&										// Inside a capsule.
					!ThrustLateral() && !ThrustLongitudinal() && !ThrustVertical() &&	// No throttles on.
					vec.Length2() < 1)											// Not moving.
			{
				// Hold still inside spaceship, mainly for launching from space vehicle parking spots.
				brake = true;
				brakeLateral = true;
				brakeVertical = true;
			}

			// Lateral acceleration.
			if(!brakeLateral && ThrustLateral())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLateral() < 0)
					accel.x = -s_SpaceRocketAccel * engineThrust[-ThrustLateral()];
				else
					accel.x = s_SpaceRocketAccel * engineThrust[ThrustLateral()];
			}

			// Forward/reverse acceleration.
			if(!brake && ThrustLongitudinal())
			{
				// Throttle ranges from -10 to +10.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLongitudinal() < 0)
					accel.y = -s_SpaceRocketAccel * engineThrust[-ThrustLongitudinal()];
				else
					accel.y = s_SpaceRocketAccel * engineThrust[ThrustLongitudinal()];
			}

			// Vertical acceleration.
			if(!brakeVertical && ThrustVertical())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustVertical() < 0)
					accel.z = -s_SpaceRocketAccel * engineThrust[-ThrustVertical()];
				else
					accel.z = s_SpaceRocketAccel * engineThrust[ThrustVertical()];
			}
		}
		break;
	case CID_Helicopter:
		if(IsEngineOn())
		{
			float airDensity = AirDensity();
			if(airDensity <= .1f)
				break;	// Not enough air density for lift.
			if(IsHolding())
			{
				applyGravity = 0;
				break;
			}
			float heliForce = (float)s_HelicopterAccel * (airDensity - .1f)*2;

			/*
			float heliForce = (float)s_HelicopterAccel * (airDensity - .1f)*2 * (float)m_Throttle / 10.f;

			// Lateral acceleration.
			if(!brakeLateral && ThrustLateral())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLateral() < 0)
					accel.x = -heliForce * engineThrust[-ThrustLateral()];
				else
					accel.x = heliForce * engineThrust[ThrustLateral()];
			}

			// Forward/reverse acceleration.
			if(!brake && ThrustLongitudinal())
			{
				// Throttle ranges from -10 to +10.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustLongitudinal() < 0)
					accel.y = -heliForce * engineThrust[-ThrustLongitudinal()];
				else
					accel.y = heliForce * engineThrust[ThrustLongitudinal()];
			}

			// Vertical acceleration.
			if(!brakeVertical && ThrustVertical())
			{
				// Throttle ranges from -5 to +5.
				// This progression makes the throttle more controllable at
				// low speed.
				if(ThrustVertical() < 0)
					accel.z = -heliForce * engineThrust[-ThrustVertical()];
				else
					accel.z = heliForce * engineThrust[ThrustVertical()];
			}
			*/
			// Vertical acceleration.
			int tv = ThrustVertical();
			if(!tv)
				tv = m_Throttle;
			if(!brakeVertical && tv)
			{
				// Throttle ranges from -10 to +10.
				// This progression makes the throttle more controllable at low speed.
				if(tv < 0)
					accel.z = -heliForce * engineThrust[-tv];
				else
					accel.z = heliForce * engineThrust[tv];
			}
		}
		break;
	default:
		break;
	}

	// Add acceleration due to gravity.
	if(applyGravity)
	{
		//if(applyGravity == 1)
			accel += DaUnit::Acceleration();
		//else
			//accel += DaUnit::Acceleration()/2;
	}

	if(IsEngineOn())
	{
		if(HasGravityDrive())
		{
			// Braking by the space fighter must be applied after gravity in order
			// to counteract it properly. In fact, it simply ignores it since the
			// fighter can always produce more thrust than gravity. This also makes
			// the calculation a lot easier.
			// Lateral braking.
			if(brakeLateral)
			{
				// Use engine braking to stop.
				const float maxBrakeThrust = s_SpaceFighterAccel * engineThrust[10];
				accel.x = -vec.x*2;	// brake with opposite force
				if(accel.x < 0)
				{
					if(accel.x < -maxBrakeThrust)
						accel.x = -maxBrakeThrust;
				}
				else
				{
					if(accel.x > maxBrakeThrust)
						accel.x = maxBrakeThrust;
				}
			}

			// Forward/reverse braking.
			if(brake)
			{
				// Use engine braking to stop.
				const float maxBrakeThrust = s_SpaceFighterAccel * engineThrust[10];
				accel.y = -vec.y*2;	// brake with opposite force
				if(accel.y < 0)
				{
					if(accel.y < -maxBrakeThrust)
						accel.y = -maxBrakeThrust;
				}
				else
				{
					if(accel.y > maxBrakeThrust)
						accel.y = maxBrakeThrust;
				}
			}

			// Vertical braking.
			if(brakeVertical)
			{
				// Use engine braking to stop.
				const float maxBrakeThrust = s_SpaceFighterAccel * engineThrust[10];
				accel.z = -vec.z*2;	// brake with opposite force
				if(accel.z < 0)
				{
					if(accel.z < -maxBrakeThrust)
						accel.z = -maxBrakeThrust;
				}
				else
				{
					if(accel.z > maxBrakeThrust)
						accel.z = maxBrakeThrust;
				}
			}
		}
		else if(m_cid == CID_SpaceRocket)
		{
			// Lateral braking.
			if(brakeLateral)
			{
				// Use engine braking to stop.
				// Calculate maximum braking on this axis.
				float brakeForce = s_SpaceRocketAccel * engineThrust[10];

				// Calculate braking force needed to reduce axis acceleration to zero.
				float nuaccel = accel.x + vec.x;				// New acceleration after 'accel' added to current vector.
				if(brakeForce > fabs(nuaccel))
					brakeForce = fabs(nuaccel);

				// Brake with opposite force vs nu acceleration.
				if(nuaccel > 0)
					accel.x -= brakeForce;
				else
					accel.x += brakeForce;
			}

			// Forward/reverse braking.
			if(brake)
			{
				// Use engine braking to stop.
				// Calculate maximum braking on this axis.
				float brakeForce = s_SpaceRocketAccel * engineThrust[10];

				// Calculate braking force needed to reduce axis acceleration to zero.
				float nuaccel = accel.y + vec.y;				// New acceleration after 'accel' added to current vector.
				if(brakeForce > fabs(nuaccel))
					brakeForce = fabs(nuaccel);

				// Brake with opposite force vs nu acceleration.
				if(nuaccel > 0)
					accel.y -= brakeForce;
				else
					accel.y += brakeForce;
			}

			// Vertical braking.
			if(brakeVertical)
			{
				// Use engine braking to stop.
				// Calculate maximum braking on this axis.
				float brakeForce = s_SpaceRocketAccel * engineThrust[10];

				// Calculate braking force needed to reduce axis acceleration to zero.
				float nuaccel = accel.z + vec.z;				// New acceleration after 'accel' added to current vector.
				if(brakeForce > fabs(nuaccel))
					brakeForce = fabs(nuaccel);

				// Brake with opposite force vs nu acceleration.
				if(nuaccel > 0)
					accel.z -= brakeForce;
				else
					accel.z += brakeForce;
			}
		}
	}
	//if(m_cid == CID_SpaceFighter)
		//qDebug("DaVehicle::Acceleration spacefighter %d accel %g,%g,%g "
				//"vector %g,%g,%g onground %d",
				//Id().GetID(), accel.x, accel.y, accel.z,
				//ve.x, ve.y, vec.z, OnGround());
	return accel;
}

// Returns friction in ft/sec/sec, typically due to surface contact.
AuPoint DaVehicle::Friction() const
{
	// Determine if brake is on.
	m_VehicleLock.lockForRead();
	int acti = m_wActions;
	m_VehicleLock.unlock();
	bool brake = false;
	bool brakeLateral = false;
	bool brakeVertical = false;
	if(IsHolding())
	{
		brake = true;
		brakeLateral = true;
		brakeVertical = true;
	}
	else
	{
		if(acti & ACT_Brake)
			brake = true;
		if(acti & ACT_BrakeLateral)
			brakeLateral = true;
		if(acti & ACT_BrakeVertical)
			brakeVertical = true;
	}

	AuPoint vec(Vector());

	// Determine friction forward and lateral due to ground surface contact.
	// Assumes I'm oriented to the surface appropriately.
	double fricFwd = 0, fricLat = 0, fricVrt = 0;
	switch(m_cid)
	{
	case CID_SailBoat:
		// Add drag induced by sliding across the terrain.
		if(WaterDepth() > 3.f/12.f)
		{
			fricFwd = s_SailBoatFric;
			fricLat = 10;
			fricVrt = 40;

			switch(OnGround())
			{
			case OG_None:
			case OG_WaterSoundOnly:	// Never returned by OnGround().
				break;
			case OG_Forest:
			case OG_Snow:
			case OG_HardFloor:
			case OG_Metal:
			case OG_Pavement:
			case OG_WoodFloor:
			case OG_Rock:
			case OG_Sand:
			case OG_Savanna:
				fricFwd += 2;
				fricLat += 5;
				break;
			}
		}
		else
		{
			// Not in the water.
			switch(OnGround())
			{
			case OG_None:
			case OG_WaterSoundOnly:	// Never returned by OnGround().
				break;
			case OG_Forest:
			case OG_Snow:
			case OG_HardFloor:
			case OG_Metal:
			case OG_Pavement:
			case OG_WoodFloor:
			case OG_Rock:
			case OG_Sand:
			case OG_Savanna:
				fricFwd = 20;
				fricLat = 30;
				fricVrt = 5;

				// Allow minimal movement on land out of water so the boat can
				// get back into the water. When below 2 fps, set the forward
				// friction to that of the boat's normal water environ.
				if(ThrustLongitudinal() && fabs(vec.y) < 2)
					fricFwd = s_SailBoatFric;
				break;
			}
		}
		break;
	case CID_Submarine:
		// Add drag induced by sliding across the terrain and also braking.
		if(WaterDepth() > 3.f/12.f)
		{
			fricFwd = s_SubmarineFric;
			fricLat = s_SubmarineFric;
			fricVrt = s_SubmarineFric;
			if(brake)
				fricFwd += 10;
			if(brakeLateral)
				fricLat += 15;
			if(brakeVertical && IsCGSubmerged())
				fricVrt += 5;

			switch(OnGround())
			{
			case OG_None:
			case OG_WaterSoundOnly:	// Never returned by OnGround().
				break;
			case OG_Forest:
			case OG_Snow:
			case OG_HardFloor:
			case OG_Metal:
			case OG_Pavement:
			case OG_WoodFloor:
			case OG_Rock:
			case OG_Sand:
			case OG_Savanna:
				fricFwd += 1;
				fricLat += 2;
				break;
			}
		}
		else
		{
			// Not in the water.
			switch(OnGround())
			{
			case OG_None:
			case OG_WaterSoundOnly:	// Never returned by OnGround().
				break;
			case OG_Forest:
			case OG_Snow:
			case OG_HardFloor:
			case OG_Metal:
			case OG_Pavement:
			case OG_WoodFloor:
			case OG_Rock:
			case OG_Sand:
			case OG_Savanna:
				fricFwd = 25;
				fricLat = 40;
				fricVrt = 5;

				// Allow minimal movement on land out of water so the sub can
				// get back into the water.
				if(IsEngineOn() && ThrustLongitudinal() && fabs(vec.y) < 2)
					fricFwd = s_SubmarineFric;

				if(brake)
					fricFwd += 10;
				if(brakeLateral)
					fricLat += 15;
				break;
			}
		}
		break;
	case CID_SUV:
		if(WaterDepth() > 1.f)
		{
			if(IsOnWater() || IsCGSubmerged())
			{
				// Floating induces a lot of drag.
				if(IsOnWater())
				{
					fricFwd = 15;
					fricLat = 25;
				}

				// Allow minimal movement on land out of water so the sub can
				// get back into the water.
				if(IsEngineOn() && ThrustLongitudinal() && fabs(vec.y) < 2)
				{
					fricFwd = s_SUVFric;
					break;	// No more drag due to ground or I can't move.
				}
			}
			else
			{
				// In water induces some drag.
				fricFwd = 5;
				fricLat = 5;
			}
			fricVrt = 60;
		}

		// Automatic parking brake when engine is off or not moving with no throttle.
		if(!IsEngineOn() || (!ThrustLongitudinal() && fabs(vec.y) < 1))
			brake = true;

		// Add drag induced by rolling across the terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment or brakes.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += s_SUVFric + 5;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_Snow:
			fricFwd += s_SUVFric + 6;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_HardFloor:
			fricFwd += s_SUVFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Metal:
			fricFwd += s_SUVFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Pavement:
			fricFwd += s_SUVFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_WoodFloor:
			fricFwd += s_SUVFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Rock:
			fricFwd += s_SUVFric + 4;
			if(brake)
				fricFwd += 35;
			fricLat += 80;
			break;
		case OG_Sand:
			fricFwd += s_SUVFric + 2;
			if(brake)
				fricFwd += 30;
			fricLat += 40;
			break;
		case OG_Savanna:
			fricFwd += s_SUVFric + 3;
			if(brake)
				fricFwd += 25;
			fricLat += 50;
			break;
		}
		break;
	case CID_APC:
		if(WaterDepth() > 1.f)
		{
			if(IsOnWater() || IsCGSubmerged())
			{
				// Floating induces a lot of drag.
				if(IsOnWater())
				{
					fricFwd = 15;
					fricLat = 25;
				}

				// Allow minimal movement on land out of water so the sub can
				// get back into the water.
				if(IsEngineOn() && ThrustLongitudinal() && fabs(vec.y) < 2)
				{
					fricFwd = s_APCFric;
					break;	// No more drag due to ground or I can't move.
				}
			}
			else
			{
				// In water induces some drag.
				fricFwd = 5;
				fricLat = 5;
			}
			fricVrt = 10;
		}

		// Automatic parking brake when engine is off or not moving with no throttle.
		if(!IsEngineOn() || (!ThrustLongitudinal() && fabs(vec.y) < 1))
			brake = true;

		// Add drag induced by rolling across the terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment or brakes.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += s_APCFric + 5;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_Snow:
			fricFwd += s_APCFric + 6;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_HardFloor:
			fricFwd += s_APCFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Metal:
			fricFwd += s_APCFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Pavement:
			fricFwd += s_APCFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_WoodFloor:
			fricFwd += s_APCFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Rock:
			fricFwd += s_APCFric + 4;
			if(brake)
				fricFwd += 35;
			fricLat += 80;
			break;
		case OG_Sand:
			fricFwd += s_APCFric + 2;
			if(brake)
				fricFwd += 30;
			fricLat += 40;
			break;
		case OG_Savanna:
			fricFwd += s_APCFric + 3;
			if(brake)
				fricFwd += 25;
			fricLat += 50;
			break;
		}
		break;
	case CID_FarmMachinery:
		if(WaterDepth() > 1.f)
		{
			if(IsOnWater() || IsCGSubmerged())
			{
				// Floating induces a lot of drag.
				if(IsOnWater())
				{
					fricFwd = 15;
					fricLat = 25;
				}

				// Allow minimal movement on land out of water so the sub can get back into the water.
				if(IsEngineOn() && ThrustLongitudinal() && fabs(vec.y) < 2)
				{
					fricFwd = s_FarmMachineryFric;
					break;	// No more drag due to ground or I can't move.
				}
			}
			else
			{
				// In water induces some drag.
				fricFwd = 5;
				fricLat = 5;
			}
			fricVrt = 5;
		}

		// Automatic parking brake when engine is off or not moving with no throttle.
		if(!IsEngineOn() || (!ThrustLongitudinal() && fabs(vec.y) < 1))
			brake = true;

		// Add drag induced by rolling across the terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment or brakes.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += s_FarmMachineryFric + 5;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_Snow:
			fricFwd += s_FarmMachineryFric + 6;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_HardFloor:
			fricFwd += s_FarmMachineryFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Metal:
			fricFwd += s_FarmMachineryFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Pavement:
			fricFwd += s_FarmMachineryFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_WoodFloor:
			fricFwd += s_FarmMachineryFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Rock:
			fricFwd += s_FarmMachineryFric + 4;
			if(brake)
				fricFwd += 35;
			fricLat += 80;
			break;
		case OG_Sand:
			fricFwd += s_FarmMachineryFric + 2;
			if(brake)
				fricFwd += 30;
			fricLat += 40;
			break;
		case OG_Savanna:
			fricFwd += s_FarmMachineryFric + 3;
			if(brake)
				fricFwd += 25;
			fricLat += 50;
			break;
		}
		break;
	case CID_DirtBike:
		if(WaterDepth() > 6.f/12.f)
		{
			// Bike is touching water.
			if(IsCGSubmerged())
			{
				// Under water induces a lot of drag.
				fricFwd = 25;
				fricLat = 25;

				// Allow minimal movement under water so the bike can
				// get back out of the water.
				if(IsEngineOn() && ThrustLongitudinal() && fabs(vec.y) < 2)
				{
					fricFwd = s_DirtBikeFric;
					fricLat = s_DirtBikeFric;
					fricVrt = 5;
					break;	// No more drag due to ground or I can't move.
				}
			}
			else
			{
				// In water induces some drag.
				fricFwd = 5;
				fricLat = 5;
			}
			fricVrt = 5;
		}

		// Automatic parking brake when engine is off or not moving with no throttle.
		if(!IsEngineOn() || (!ThrustLongitudinal() && fabs(vec.y) < 1))
			brake = true;

		// Add drag induced by rolling across the terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment or brakes.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += s_DirtBikeFric + 10;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_Snow:
			fricFwd += s_DirtBikeFric + 12;
			if(brake)
				fricFwd += 20;
			fricLat += 30;
			break;
		case OG_HardFloor:
			fricFwd += s_DirtBikeFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Metal:
			fricFwd += s_DirtBikeFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Pavement:
			fricFwd += s_DirtBikeFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_WoodFloor:
			fricFwd += s_DirtBikeFric;
			if(brake)
				fricFwd += 40;
			fricLat += 60;
			break;
		case OG_Rock:
			fricFwd += s_DirtBikeFric;
			if(brake)
				fricFwd += 35;
			fricLat += 80;
			break;
		case OG_Sand:
			fricFwd += s_DirtBikeFric + 5;
			if(brake)
				fricFwd += 30;
			fricLat += 40;
			break;
		case OG_Savanna:
			fricFwd += s_DirtBikeFric + 7;
			if(brake)
				fricFwd += 25;
			fricLat += 50;
			break;
		}
		break;
	case CID_SpaceFighter:
	case CID_AdvSpaceFighter:
	case CID_SpaceTransport:
		if(AirDensity() > 0)
		{
			// In air.
			if(WaterDepth() > 1.f)
			{
				// In water induces some drag.
				fricFwd = 5;
				fricLat = 6;
				fricVrt = 10;
			}
			else
			{
				if(m_cid == CID_SpaceTransport)
				{
					fricFwd = s_SpaceTransportFric;
					fricLat = s_SpaceTransportFric;
					fricVrt = s_SpaceTransportFric;
				}
				else
				{
					fricFwd = s_SpaceFighterFric;
					fricLat = s_SpaceFighterFric;
					fricVrt = s_SpaceFighterFric;
				}
			}
		}
		else if(IsEngineOn())
		{
			// Outside atmosphere range of a planet and maneuver drives are
			// operating. Induce some friction if the throttles aren't on so
			// the ship will grind to a halt.
			// I could be inside a spacecraft in space.
			if(!ThrustLongitudinal() && fabs(vec.y) < 2)
				fricFwd = .5;
			if(!ThrustLateral() && fabs(vec.x) < 2)
				fricLat = .5;
			if(!ThrustVertical() && fabs(vec.z) < 2)
				fricVrt = .5;
		}

		// Add drag induced by terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += 20;
			fricLat += 100;
			break;
		case OG_Snow:
			fricFwd += 30;
			fricLat += 150;
			break;
		case OG_HardFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Metal:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Pavement:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_WoodFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Rock:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Sand:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Savanna:
			fricFwd += 15;
			fricLat += 75;
			break;
		}
		break;
	case CID_SpaceRocket:
		if(AirDensity() > 0)
		{
			// In air.
			if(WaterDepth() > 1.f)
			{
				// In water induces some drag.
				fricFwd = 4;
				fricLat = 8;
				fricVrt = 8;
			}
			else
			{
				fricFwd = s_SpaceRocketFric;
				fricLat = s_SpaceRocketFric;
				fricVrt = s_SpaceRocketFric;
			}
		}
		else if(IsEngineOn())
		{
			// Outside atmosphere range of a planet and maneuver drives are
			// operating. Induce some friction if the throttles aren't on so
			// the ship will grind to a halt.
			// I could be inside a spacecraft in space.
			if(!ThrustLongitudinal() && fabs(vec.y) < 2)
				fricFwd = .5;
			if(!ThrustLateral() && fabs(vec.x) < 2)
				fricLat = .5;
			if(!ThrustVertical() && fabs(vec.z) < 2)
				fricVrt = .5;
		}

		// Add drag induced by terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricLat += 100;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_Snow:
			fricLat += 150;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_HardFloor:
			fricLat += 50;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_Metal:
			fricLat += 50;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_Pavement:
			fricLat += 50;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_WoodFloor:
			fricLat += 50;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_Rock:
			fricLat += 75;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_Sand:
			fricLat += 75;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		case OG_Savanna:
			fricLat += 75;
			fricVrt = fricLat;	// Space rocket sits on its tail, hence the vertical friction.
			break;
		}
		break;
	case CID_Helicopter:
		if(AirDensity() > 0)
		{
			// In air.
			if(WaterDepth() > 1.f)
			{
				// In water induces some drag.
				fricFwd = 10;
				fricLat = 10;
				fricVrt = 50;
			}
			else
			{
				fricFwd = s_HelicopterFric;
				fricLat = s_HelicopterFric;
				fricVrt = s_HelicopterFric;
			}
		}

		// Add drag induced by terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += 20;
			fricLat += 100;
			break;
		case OG_Snow:
			fricFwd += 30;
			fricLat += 150;
			break;
		case OG_HardFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Metal:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Pavement:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_WoodFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Rock:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Sand:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Savanna:
			fricFwd += 15;
			fricLat += 75;
			break;
		}
		break;
	default:
		break;
	}
	return AuPoint(fricLat, fricFwd, fricVrt);
}

void DaVehicle::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(IsLightOn() && m_Model)
	{
		// My light is on.
		AuGLLight::ROLE lightRole = Is(person.ParentId()) ? AuGLLight::SceneVehicle : AuGLLight::Vehicle;

		// Put the light at the front of the vehicle.
		AuPoint min,max;
		m_Model->GetExtents(min, max);

		// Higher off the ground gives us better lighting.
		AuPoint lpos;			// Light position
		switch(m_cid)
		{
		case CID_DirtBike:
			lpos = model.Project(0, max.y-2, 2);
			break;
		default:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_AdvSpaceFighter:
		case CID_Helicopter:
			lpos = model.Project(0, max.y, 0);
			break;
		case CID_SpaceRocket:
			lpos = model.Project(0, max.y-.25, 0);
			break;
		case CID_Submarine:
			lpos = model.Project(0, max.y+.5, 0);
			break;
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
			lpos = model.Project(0, max.y, 2);
			break;
		}

// Attenuation range of lights.
#define DirtBikeLight		(100*FeetPerMeter())
#define SpaceFighterLight	(175*FeetPerMeter())
#define SpaceRocketLight	(125*FeetPerMeter())
#define SubmarineLight		(150*FeetPerMeter())
#define SUVLight			(120*FeetPerMeter())
		float cosInnerCone = .939692621f;		// 20 degrees
		float cosOuterCone = .5f;				// 60 degrees
		AuPoint lvec;			// Light direction vector
		QColor colLight;
		float kQ;		// 1/light radius squared
		float kL = .005f;	// The higher the number the quicker the falloff
		bool useCone = true;
		switch(m_cid)
		{
		default:
		case CID_DirtBike:
			lvec = model.ProjectVector(AuPoint(0,1,0));
			colLight = QColor(AuGLLight::ColorHalogen());
			kQ = 1.f/(float)(DirtBikeLight * DirtBikeLight);
			kL = .006f;	// Weaker light
			cosOuterCone = .766f;			// 40 degrees
			break;
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_AdvSpaceFighter:
		case CID_Helicopter:
			lvec = model.ProjectVector(AuPoint(0,1,0));
			colLight = QColor(AuGLLight::ColorCarbonArc());
			kQ = 1.f/(float)(SpaceFighterLight * SpaceFighterLight);
			break;
		case CID_SpaceRocket:
			colLight = QColor(AuGLLight::ColorCarbonArc());
			kQ = 1.f/(float)(SpaceRocketLight * SpaceRocketLight);
			useCone = false;
			break;
		case CID_Submarine:
			lvec = model.ProjectVector(AuPoint(0,.2588f,-.9659));	// 75 degrees down
			colLight = QColor(AuGLLight::ColorCarbonArc());
			kQ = 1.f/(float)(SubmarineLight * SubmarineLight);
			cosInnerCone = 0.5f;			// 60 degrees
			cosOuterCone = 0.0f;			// 90 degrees
			break;
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
			lvec = model.ProjectVector(AuPoint(0,1,0));
			colLight = QColor(AuGLLight::ColorHalogen());
			kQ = 1.f/(float)(SUVLight * SUVLight);
			kL = .004f;	// Stronger light because we are simulating two headlights
			break;
		}
		if(useCone)
			lights.append(AuGLLight(Id(), lightRole, kL,kQ, colLight, lpos, cosOuterCone, cosInnerCone, lvec));
		else
			lights.append(AuGLLight(Id(), lightRole, kL,kQ, colLight, lpos));
	}

	// Make lights for subordinate atoms.
	DaUnit::GetRenderLightsRecursive(person, model, suns, lights);
}

void DaVehicle::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaAtom* sceneAtom) const
{
	Q_UNUSED(sceneAtom);
	AuBox fExt;
	RoughExtents(fExt);
	fExt.Transform(tModel);
	fExt.GetQuads(aControlPanels);

	idPanels.append(0);
	idPanels.append(1);
	idPanels.append(2);
	idPanels.append(3);
	idPanels.append(4);
	idPanels.append(5);
}

void DaVehicle::EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
{
	Q_UNUSED(panelId);
	Q_UNUSED(controls);
	Q_UNUSED(ari);
}

bool DaVehicle::Refuel(const AuItem& fuel)
{
	if(fuel.CommodityId() != FuelType())
		return false;
	Refuel();
	return true;
}

void DaVehicle::Refuel()
{
	m_VehicleLock.lockForWrite();

	// Energy weapon automatically reloads when refueled.
	if(m_Gun.IsEnergyWeapon())
	{
		int maxRounds = MaxWeaponRounds(m_Gun.CommodityId());
		if(m_Gun.RoundsCount() < maxRounds)
			m_Gun.Reload(maxRounds);
	}

	// Refuel this vehicle.
	m_Fuel = FuelMax();
	m_tmFuelChanged.Set();
	m_VehicleLock.unlock();
	SetModified();
}

float DaVehicle::Density() const
{
	switch(m_cid)
	{
	case CID_DirtBike:
		return 230.f / 3.f;
	case CID_SailBoat:
		return 320.f / (7.f * (float)CubicFeetPerCubicMeter());
	case CID_AdvSpaceFighter:
		return Density(2, true);	// Density at armor level 2.
	case CID_SpaceFighter:
		return Density(1, true);	// Density at armor level 1.
	case CID_SpaceTransport:
		return Density(1, true);	// Density at armor level 1.
	case CID_SpaceRocket:
		return Density(0, true);	// Density at armor level 0.
	case CID_Helicopter:
		return Density(0, false);	// Density at armor level 0, unstreamlined.
	case CID_Submarine:
		return (float)DensityOfWater();	// so it neither sinks nor floats
	case CID_SUV:
		// From Wikipedia weight and dimensions for H2.
		return Mass(m_cid)/(2.f*(float)CubicFeetPerCubicMeter());
	case CID_FarmMachinery:
		return Mass(m_cid)/(2.f*(float)CubicFeetPerCubicMeter());
	case CID_APC:
		return Mass(m_cid)/(2.f*(float)CubicFeetPerCubicMeter());
	default:
		return (float)DensityOfWater();
	}
}

void DaVehicle::FilterFinalMoveVector(AuPoint& vector) const
{
	//qDebug() << "DaVehicle::FilterFinalMoveVector" << Id() << Name() << "atRest" << IsAtRest() << "altMSL" << AltitudeMSL()
		//<< "vec" << vector << "inWater" << IsInWater() << "inEnc" << IsInsideEnclosure()
		//<< "inInd" << InIndustry() << "inG" << IsInGravity() << "onG" << IsTouchingGround() << OnGround();
	// When sitting on the ground, eliminate the downward portion of the movement vector or we creep on hills.
	if(IsInGravity() && IsTouchingGround())
	{
		if(LandingOrientation() == LORI_Tail)
		{
			// Space Rocket orients on its tail with +Y up.
			if(vector.y < 0 && fabs(vector.x) < 1 && fabs(vector.z) < 1)
				vector.y = 0;
		}
		else
		{
			// Everything else orients with +Z up.
			if(vector.z < 0 && vector.Length2_2D() <= 1)
				vector.z = 0;
		}
	}

	// Limit movement vector to the speed of light.
	if(!vector.IsZero() && vector.Length2() > SpeedOfLight2())
		vector.Unit(SpeedOfLight());

	//if(m_cid == CID_DirtBike && IsLightOn())
		//qDebug("DaVehicle::FilterFinalMoveVector %d result %g,%g,%g TouchingGround %c InGravity %c FallDist %g",
				//Id().GetID(), vector.x, vector.y, vector.z,
				//IsTouchingGround()?'Y':'N',
				//IsInGravity()?'Y':'N',
				//FallDistance());
}

void DaVehicle::FilterPostMoveVector(AuPoint& vector) const
{
	if(!IsInGravity())
		return;
	//qDebug() << "DaVehicle::FilterPostMoveVector" << Id() << Name() << "atRest" << IsAtRest() << "altMSL" << AltitudeMSL()
		//<< "vec" << vector << "inWater" << IsInWater() << "inEnc" << IsInsideEnclosure()
		//<< "inInd" << InIndustry() << "inG" << IsInGravity() << "onG" << IsTouchingGround() << OnGround();
	// When sitting on the ground, eliminate the downward portion of the movement vector.
	// Eliminate gravity vector when blocked to help get free from the side of buildings.
	if(IsTouchingGround())
	{
		if(LandingOrientation() == LORI_Tail)
		{
			// Space Rocket orients on its tail with +Y up.
			if(vector.y < 0)
				vector.y = 0;
		}
		else
		{
			// Everything else orients with +Z up.
			if(vector.z < 0)
				vector.z = 0;
		}
	}
}

bool DaVehicle::DepartingSolarSystem()
{
	if(IsTraveling())
	{
		// This should not happen.
		qDebug("DaVehicle::DepartingSolarSystem %d %s is already traveling.", Id().GetID(), (const char*)Name().toLatin1());
		return true;
	}

	if(!IsOccupied())
	{
		// Unoccupied vehicles do not leave the solar system.
		DestroyVehicle("Collided with dark matter object.");
		return false;
	}
	return true;
}

void DaVehicle::DestroyVehicle(const QString& reason)
{
	DaAtom* killer = 0;
	AuId killerId = KillerId();	// Award the kill to the atom that did the most damage; this could be a spaceship.
	QString killerName;
	if(!killerId.IsNull())
	{
		DaAtom* solar = FindEnvironmentAtom();
		if(solar)
		{
			// Locate the atom that killed me.
			killer = solar->FindIdRecursive(killerId);
			if(killer)
			{
				killer->UnitKilled(this);
				killerName = killer->Name();
			}
		}
	}

	// Take all the people off the vehicle and kill them at my location.
	if(HasSubatoms())
	{
		// Award the kill(s) to the atom that did the most damage; this could be a spaceship.
		COMMODITYID weaponCID = DamagedUsingCID(killerId);
		quint8 weaponQ = DamagedUsingQ(killerId);
		QString killerName;
		if(killer)
			killerName = killer->Name();

		// This server's login credentials are needed to send mail.
		const DgData& gd = g_Au.LockDataForRead();
		unsigned int idPlayer = gd.PlayerId();
		unsigned int confirmNum = gd.ConfirmationNumber();
		g_Au.UnlockData();

		// My transform is the location at which people aboard will lie dead until they respawn somewhere.
		AuMatrix4x4 tme = Transform();
		DaAtomList people;
		DaAtomList subs;
		ListNormalSubatoms(subs);
		subs.FindLivePeopleRecursive(people);
		FindCitizens(people);	// Citizens help add to the debris cloud.
		for(DaAtomList::const_iterator it = people.begin(); it != people.end(); it++)
		{
			// Remove the atom from its parent, which could be a spacecraft subordinate to me.
			DaAtom* atom = *it;
			AuPoint spreadVec = atom->Location();	// Atom's location relative to the parent atom is their spread vector.
			spreadVec.Unit(g_Au.Dice().Roll(1,50));	// Random vector length up to 50 feet per second
			atom->ForcePosMove(AuAtomPos(ParentId(), tme * atom->Transform()), Vector() + spreadVec);

			DaPerson* person = dynamic_cast<DaPerson*>(atom);
			if(person)
			{
				if(person->IsNormal())
				{
					// Thrown from explosion in prone pose.
					person->SetGeneralPose(AuPose::Prone);

					// Kill the person immediately, with random bleed-out time.
					int killPts = person->CurrentHitPoints() + g_Au.Dice().Roll(1,-DpBody::DeadHitPoints()-5,-1);
					WEAPONEFFECT killType[] =
					{
						//WE_Acid,
						//WE_Suffocate,
						//WE_Stun,
						//WE_Poison,
						//WE_Radiation,
						WE_Crush,
						//WE_Sonic, 
						WE_Bludgeon,	// 8
						//WE_Cold,
						//WE_Disease,
						WE_Electric,	// 11
						WE_Heat,
						WE_Pierce,		// 13
						//WE_Implant,
						WE_Slash,		// 15
					};
					int killTypes = sizeof(killType)/sizeof(WEAPONEFFECT);

					AuCombatAttacker attacker(killerId, killerName, killer ? killer->EmpireId() : 0);

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = weaponCID;
					weapon.m_Quality = weaponQ;
					weapon.m_w1.m_Effect = killType[g_Au.Dice().Roll(1,killTypes,-1)];   // Random weapon effect for kill.;

					AuCombatResult result;
					result.m_Damage1 = killPts;

					person->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);

					// Send mail to the person so they know what happened.
					const DaGalaxy* galaxy = FindParentGalaxy();
					if(galaxy)
					{
						const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
						AuPoint galaxyLoc;
						AuId sysId, secId;
						QString sysName, secName;
						if(system)
						{
							const DaSector* sector = FindParentSector();
							system->GetLocationRelativeToParent(galaxyLoc, galaxy);
							sysId = system->Id();
							sysName = system->Name();

							QString where;
							if(!killerName.isEmpty())
								where = tr("The %6 was destroyed by %4 (%5) in the %1 system. It is in %2, %3.")
									.arg(system->NameKnownTo(person->EmpireId()))
									.arg(sector->Name())
									.arg(galaxy->Name())
									.arg(killerName)
									.arg(killerId.StringId())
									.arg(Name());
							else
								where = tr("The %6 was destroyed in the %1 system. It is in %2, %3.")
									.arg(system->NameKnownTo(person->EmpireId()))
									.arg(sector->Name())
									.arg(galaxy->Name())
									.arg(Name());

							g_Au.SendMail(idPlayer, confirmNum, Id(), Name(), person->Id(), AuMailMsg::MSG_Destroyed,
									tr("%1 Destroyed").arg(Name()),
									tr("%1<br><br>%2<br><br>%3").arg(g_Au.EncodeUTCString()).arg(reason).arg(where),
									sysId, sysName, secId, secName, Galaxy(), galaxyLoc);
						}
					}
				}
			}
			else
			{
				// Other creatures such as citizens and animals can be killed without a random killEffect.
				DaCreature* creature = dynamic_cast<DaCreature*>(atom);
				if(creature)
				{
					// Thrown from explosion in prone pose.
					creature->SetGeneralPose(AuPose::Prone);

					// Kill the creature immediately, with random bleed-out time.
					AuCombatAttacker attacker(killerId, killerName, killer ? killer->EmpireId() : 0);

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = weaponCID;
					weapon.m_Quality = weaponQ;
					weapon.m_w1.m_Effect = WE_Bleeding;

					AuCombatResult result;
					result.m_Damage1 = creature->CurrentHitPoints() + g_Au.Dice().Roll(1,-DpBody::DeadHitPoints()-5,-1);

					creature->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
				}
			}
			if(!atom->IsDead())
				atom->SetModified();
			atom->TickTock();	// Get moving.
		}
	}

	// Create an effect atom to carry the explosion effect.
	DaAtom* parent = Parent();
	if(parent)
	{
		DaEffect* explode = parent->CreateEffect();

		// Add the explosion effect to the effect atom.
		// Loudness/size based on my size.
		float gain;
		VISUAL_EFFECT visual = VISUAL_ExplodeVehicle;
		switch(m_cid)
		{
		case CID_DirtBike:
		default:
			gain = 4;
			break;
		case CID_SailBoat:
			gain = 6;
			break;
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
			gain = 14;
			visual = VISUAL_ExplodeSpacecraft;
			break;
		case CID_SpaceRocket:
			gain = 12;
			break;
		case CID_Helicopter:
			gain = 5;
			break;
		case CID_Submarine:
			gain = 10;
			break;
		case CID_SUV:
			gain = 7;
			break;
		case CID_FarmMachinery:
			gain = 8;
			break;
		case CID_APC:
			gain = 9;
			break;
		}

		if(LandingOrientation() == LORI_Tail)
		{
			AuMatrix4x4 m(Transform());
			m.RotateX(-PI/2);
			AuEffect effect(Location(), m.GetRotation(), Transform().ProjectVector(Vector()), SOUND_ExplodeVehicle,
					gain, 1, visual);
			explode->AddEffect(effect);
		}
		else
		{
			AuEffect effect(Location(), Rotation(), Transform().ProjectVector(Vector()), SOUND_ExplodeVehicle,
					gain, 1, visual);
			explode->AddEffect(effect);
		}
	}

	// Destroy this vehicle and everything still aboard.
	Kill();
}

// Must return true to enable refuel and repair.
bool DaVehicle::IsControl(const DaAtom* sceneAtom) const
{
	// Dirt bike allows refuel while aboard.
	// There's no place to carry items so why not; see below.
	if(m_cid == CID_DirtBike)
		return true;

	// Must not return true while aboard or I can't pick up items that have been dropped aboard.
	// Items cannot be picked up from outside the vehicle, otherwise I can't refuel or repair from outside.
	if(sceneAtom && (const DaAtom*)sceneAtom->Parent() == this)
		return false;

	// Return true so this vehicle can be refueled or repaired.
	return true;
}

float DaVehicle::HullStress() const
{
	// Certain tests have already been made.
	Q_ASSERT(IsInAtmosphere());

	float maxSpeed;
	switch(m_cid)
	{
	case CID_DirtBike:
		maxSpeed = 300;
		break;
	case CID_SailBoat:
		maxSpeed = 150;
		break;
	case CID_AdvSpaceFighter:
		maxSpeed = 850;
		break;
	case CID_SpaceFighter:
		maxSpeed = 800;
		break;
	case CID_SpaceTransport:
		maxSpeed = 600;
		break;
	case CID_SpaceRocket:
		maxSpeed = 1000;
		break;
	case CID_Helicopter:
		maxSpeed = 250;
		break;
	case CID_Submarine:
		maxSpeed = 400;
		break;
	case CID_SUV:
		maxSpeed = 350;
		break;
	case CID_APC:
		maxSpeed = 500;
		break;
	case CID_FarmMachinery:
		maxSpeed = 450;
		break;
	default:
		return 0;
	}

	float hullStress = fabs(Drag().y) / maxSpeed;
	return hullStress;
}

void DaVehicle::TestHullStress()
{
	// New vehicles at a building site do not suffer from hull stress damage or they get fetched and decay continuously.
	if(IsNew() && !BdId().IsNull())
		return;

	// Certain tests have already been made.
	Q_ASSERT(IsInAtmosphere());
	Q_ASSERT(!IsInVehicle());
	Q_ASSERT(!IsInsideCapsule());

	float hullStress = HullStress();
	if(hullStress > 1)
	{
		if(hullStress <= 1.2f)
		{
			// Stress is enough to induce damage but not outright destruction.
			// Determine hits of damage in one second at this speed.
			AuCombatAttacker attacker(tr("excess hull stress"));

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_w1.m_Effect = WE_Sonic;
			weapon.m_w2.m_Effect = WE_Heat;

			AuCombatResult result;
			int hits = MaxHitPoints()/5 + 1;	// 10% damage per second
			result.m_Damage1 = g_Au.Dice().Roll(1,hits);
			result.m_Damage2 = g_Au.Dice().Roll(1,hits);

			CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			//if(!IsNormal())
				//qDebug("DaVehicle::TestHullStress %d destroyed by excess hull stress %g.", Id().GetID(), hullStress);
		}
		else
		{
			// Stress is enough to flat-out destroy the vehicle.
			DestroyVehicle(tr("Disintegrated by extreme hull stress."));
			SetModified();
			//qDebug("DaVehicle::TestHullStress %d destroyed by extreme hull stress %g.", Id().GetID(), hullStress);
		}
	}

	// Submersible vehicle is immune to buckling under atmospheric pressure.
	if(!IsCGSubmerged() && !IsUnderGround() && !IsSubmersible() && AirDensity() > .95f)
	{
		// High atmospheric pressure. Induce 10% damage per second.
		AuCombatAttacker attacker(tr("atmospheric pressure"));

		AuCombatWeapon weapon(WM_Internal);
		weapon.m_w1.m_Effect = WE_Crush;

		AuCombatResult result;
		result.m_Damage1 = MaxHitPoints()/10;

		CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		SetModified();
	}
}

// NOTE: Rendering states must already be configured. Check actual usage of this function.
void DaVehicle::RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
		POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
		const AuMatrix4x4& tDevOrtho, float hudIntensity)
{
	Q_UNUSED(targetPart);

	// Create VAOs.
	if(!m_WeaponVAO.VAO())
	{
		// A commodity texture is applied to this. They are loaded upside down for sprites, from before I knu how to invert a
		// sprite. :P
		float s = 0.;
		float t = 0.;
		float u = 1.f;
		float v = 1.f;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(0,0,0, s,v);
		tri0.m_V1 = AuPointTex(0,1,0, s,t);
		tri0.m_V2 = AuPointTex(1,1,0, u,t);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(1,1,0, u,t);
		tri1.m_V1 = AuPointTex(1,0,0, u,v);
		tri1.m_V2 = AuPointTex(0,0,0, s,v);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;

		m_WeaponVAO.CreateForTexture(*ari.re, tris, GL_STATIC_DRAW);
	}

	// Calculate individual bar sizes based on two horizontal bars. I might actually draw 3 of them.
	const float barwid = wid;
	const float barspacing = .005f * g_Au.BodyStatSize()/100.f;
	const float barht = (ht - barspacing)/2;

	// Calculate square icon size.
	const float size = 64.f/1024.f * (float)g_Au.BodyStatSize()/100.f;
	float valbot = bottom;

	// First row of icons plus rounds counters and spacing.
	int hasGunOrAmmo = 0;
	if(m_Gun.CommodityId() != CID_Invalid || m_Gun.Ammo() || m_Ord[0].CommodityId() != CID_Invalid || m_Ord[0].Ammo())
		hasGunOrAmmo++;
	if(m_Ord[1].CommodityId() != CID_Invalid || m_Ord[1].Ammo())
		hasGunOrAmmo++;
	float indbot = valbot + barspacing + barht;
	if(FuelMax() > 0)
		indbot += barspacing + barht;
	if(hasGunOrAmmo)
		indbot += barspacing*2 + size + size/2;

	if(!isTarget)
	{
		GLuint texBarBack = AuTextureLibItem::LibraryTexture(*ari.re, AuTextureLibItem::LibraryTextureStatsVehicle());
		float bw = wid + .02f;
		float bh = ht + .02f;
		if(HasLight())
			bh += barspacing + size;
		if(FuelMax() <= 0)
			bh -= barspacing + barht;
		if(hasGunOrAmmo)
		{
			bh += barspacing*2 + size + size/2;
			if(hasGunOrAmmo == 2)
				bh += barspacing + size/2;
		}
		float bx = left + bw/2 - .01f;
		float by = bottom + bh/2 - .01f;
		ari.RenderSprite(bx, by, bw, bh, texBarBack, 1, 1, 1, .5);
	}
	else
		hasGunOrAmmo = 0;

	// Fuel
	if(!isTarget && FuelMax() > 0)
	{
		float a;
		if(IsEngineOn())
			a = 1;
		else
			a = .5;

		float frac = (float)Fuel()/(float)FuelMax();
		if(frac <= .1f)
			ari.RenderBar(left,valbot, barwid,barht, frac, 1, 1.f,.2f,.2f, 1.f/2.f,.2f/2.f,.2f/2.f, a);	// red/red
		else if(frac < .25f)
			ari.RenderBar(left,valbot, barwid,barht, frac, 1, 1.f,.7f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f, a);	// amber/amber
		else
			ari.RenderBar(left,valbot, barwid,barht, frac, 1, .2f,1.f,.2f, .2f/2.f,1.f/2.f,.2f/2.f, a);	// green/green

		valbot += barspacing + barht;
	}

	// Hits Points
	{
		float frac = (float)CurrentHitPoints()/(float)MaxHitPoints();
		if(frac <= .1f || IsAnyConditionVisible())
			ari.RenderBar(left,valbot, barwid,barht, frac, 0, 1.f,.2f,.2f, 1.f/2.f,.2f/2.f,.2f/2.f);	// red/red
		else if(frac < .5f)
			ari.RenderBar(left,valbot, barwid,barht, frac, 0, 1.f,.7f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// amber/amber
		else
			ari.RenderBar(left,valbot, barwid,barht, frac, 0, .2f,1.f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// green/amber

		if(isTarget)
		{
			// Political stance indicator.
			QColor col = PoliticalStanceColor(stance, IsCivilian());
			float stanceleft = left - barht * 2.f;
			ari.RenderItemEffect(IE_PoliticalStance, stanceleft, valbot, barht, col.redF(), col.greenF(), col.blueF(), 1);
		}

		valbot += barspacing + barht;
	}

	if(isTarget)
		return;

	double windowAspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());

	// Guns/Weapons
	if(hasGunOrAmmo)
	{
		AuMatrix4x4 projection;
		projection.Scale(windowAspect, 1, 1);

		ari.re->GLSL().TextureShader().Select();
		ari.re->GLSL().TextureShader().SetColor(1,1,1,1);

		// Gun Type
		float gunleft = left;
		float guncountleft = gunleft;
		AuMatrix4x4 mvguns;
		mvguns.Translate(gunleft, valbot, 0);
		mvguns.Scale(size, size, 1);

		if(m_Gun.CommodityId() != CID_Invalid)
		{
			// Load stock texture for the commodity. AuView::initializeGL creates them.
			GLuint glTexture = ari.CommodityTextureF(m_Gun.CommodityId());

			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * mvguns);
			ari.re->GLSL().TextureShader().SetTexture(glTexture);

			m_WeaponVAO.SelectGL();
			m_WeaponVAO.DrawGL();
			m_WeaponVAO.DeselectGL();
			gunleft += size + barspacing;
		}

		float ord0alpha, ord1alpha;
		if(m_OrdSelect)
		{
			// Weapon 2 is selected.
			ord0alpha = .5f;
			ord1alpha = 1.f;
		}
		else
		{
			// Weapon 1 is selected.
			ord0alpha = 1.f;
			ord1alpha = .5f;
		}

		// Weapon Type 1
		float ord0left = left + barwid/2.f;
		float ord0countleft = ord0left;	// Position for rounds counter.
		if(m_Ord[0].CommodityId() != CID_Invalid)
		{
			AuMatrix4x4 mvord;
			mvord.Translate(ord0left, valbot, 0);
			mvord.Scale(size, size, 1);

			GLuint glTexture = ari.CommodityTextureF(m_Ord[0].CommodityId());

			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * mvord);
			ari.re->GLSL().TextureShader().SetTexture(glTexture);
			ari.re->GLSL().TextureShader().SetColor(1.f, 1.f, 1.f, ord0alpha);

			m_WeaponVAO.SelectGL();
			m_WeaponVAO.DrawGL();
			m_WeaponVAO.DeselectGL();

			ord0left += size + barspacing;
		}

		// Weapon Type 2
		float ord1left = left + barwid/2.f;
		float ord1countleft = ord1left;	// Position for rounds counter.
		if(m_Ord[1].CommodityId() != CID_Invalid)
		{
			AuMatrix4x4 mvord;
			mvord.Translate(ord1left, indbot, 0);
			mvord.Scale(size, size, 1);

			GLuint glTexture = ari.CommodityTextureF(m_Ord[1].CommodityId());

			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * mvord);
			ari.re->GLSL().TextureShader().SetTexture(glTexture);
			ari.re->GLSL().TextureShader().SetColor(1.f, 1.f, 1.f, ord1alpha);

			m_WeaponVAO.SelectGL();
			m_WeaponVAO.DrawGL();
			m_WeaponVAO.DeselectGL();

			ord1left += size + barspacing;
		}

		ari.re->GLSL().TextureShader().Deselect();

		// Rounds counters use a different shader.
		{
			// Gun is always valid.
			if(m_Gun.CommodityId() != CID_Invalid)
				ari.RenderNumber(m_Gun.RoundsCount(), guncountleft, valbot + size + barspacing, size/2.f, 0,false,
						.2f,1.f,.2f,1);

			// Weapon displays a rounds counter above the icon.
			if(m_Ord[0].CommodityId() != CID_Invalid)
				ari.RenderNumber(m_Ord[0].RoundsCount(), ord0countleft, valbot + size + barspacing, size/2.f, 0,false,
						.2f,1.f,.2f,ord0alpha);

			// Weapon displays a rounds counter above the icon.
			if(m_Ord[1].CommodityId() != CID_Invalid)
				ari.RenderNumber(m_Ord[1].RoundsCount(), ord1countleft, indbot + size + barspacing, size/2.f, 0,false,
						.2f,1.f,.2f,ord1alpha);
		}

		// Gun Effects are always valid.
		if(m_Gun.Ammo())
		{
			AuCombatWeapon w(m_Gun.Ammo(), 1);
			if(w.m_w1.m_Effect != WE_None)
			{
				ari.RenderWeaponEffect(w.m_w1.m_Effect, gunleft, valbot, size, 1,1,1,1);
				gunleft += size + barspacing;
			}
			if(w.m_w2.m_Effect != WE_None)
				ari.RenderWeaponEffect(w.m_w2.m_Effect, gunleft, valbot, size, 1,1,1,1);
		}

		// Weapon Effects are only valid when we have ammunition.
		if(m_Ord[0].Ammo())
		{
			AuCombatWeapon w(m_Ord[0].Ammo(), 1);
			if(w.m_w1.m_Effect != WE_None)
			{
				ari.RenderWeaponEffect(w.m_w1.m_Effect, ord0left, valbot, size, 1,1,1,ord0alpha);
				ord0left += size + barspacing;
			}
			if(w.m_w2.m_Effect != WE_None)
				ari.RenderWeaponEffect(w.m_w2.m_Effect, ord0left, valbot, size, 1,1,1,ord0alpha);
		}

		// Weapon Effects are only valid when we have ammunition.
		if(m_Ord[1].Ammo())
		{
			AuCombatWeapon w(m_Ord[1].Ammo(), 1);
			if(w.m_w1.m_Effect != WE_None)
			{
				ari.RenderWeaponEffect(w.m_w1.m_Effect, ord1left, indbot, size, 1,1,1,ord1alpha);
				ord1left += size + barspacing;
			}
			if(w.m_w2.m_Effect != WE_None)
				ari.RenderWeaponEffect(w.m_w2.m_Effect, ord1left, indbot, size, 1,1,1,ord1alpha);
		}
	}

	// Render engine-on indicator.
	float indleft = left;
	if(FuelMax() > 0)
	{
		float alpha = IsEngineOn() ? 1.f : .15f;
		ari.RenderItemEffect(IE_PowerPlantOn, indleft, indbot, size, 1,1,1,alpha);
		indleft += size + barspacing;
	}

	// Render lights-on indicator.
	if(HasLight())
	{
		float alpha = IsLightOn() ? 1.f : .15f;
		ari.RenderItemEffect(IE_Light, indleft, indbot, size, 1,1,1,alpha);
		indleft += size + barspacing;
	}

	// Air density warning indicator.
	if(CommodityId() == CID_Helicopter)
	{
		float swleft = left + wid - (size + barspacing);	// Right justified.
		if(!IsEngineOn())
			ari.RenderItemEffect(IE_StallWarning, swleft, indbot, size, .1f,.1f,.1f,.75f);	// Dark grey
		else if(AirDensity() < .3f)
			ari.RenderItemEffect(IE_StallWarning, swleft, indbot, size, 1,.1f,.1f,.75f);	// Red
		else if(AirDensity() < .4f)
			ari.RenderItemEffect(IE_StallWarning, swleft, indbot, size, 1,1,.1f,.75f);		// Yellow
		else
			ari.RenderItemEffect(IE_StallWarning, swleft, indbot, size, .1f,1,.1f,.75f);	// Green
	}

	// Locked-on Blip
	if(!IsInSpacecraft() && (IsEngineOn() || IsLightOn()))
	{
		// HUD is visible.
		// HUD reticle colors
		float r = 213.f/255.f;
		float g = 246.f/255.f;
		float b = 213.f/255.f;
		if(!IsEngineOn())
			hudIntensity = hudIntensity * 2 / 3;
		float a = (float)hudIntensity / 10.f;

		DaAtom* system = FindEnvironmentAtom();
		if(m_Ord[m_OrdSelect].IsMissile())
		{
			// Current ordnance is guided.
			AuId gunnerWorldId;
			int gunnerDomain = GunnerDomain(gunnerWorldId);

			AuPoint targSysLoc;
			double dist = FeetPerOrbit();
			const DaAtom* target = system->FindClosestMissileTarget(Id(), gunnerDomain, gunnerWorldId, EmpireId(), EnvLocation(), 0,
					targSysLoc, dist);
			if(target)
			{
				// Get the target blip location in the scene atom's eye space.
				AuPoint targLoc = sceneAtom->EnvTransform().UnProject(targSysLoc);
				AuPoint px = tDevOrtho.Project(targLoc);
				if(px.z < 0)
				{
					// Blip is in front of the eye plane.
					// Calculate square icon size.
					const float size = 128.f/1024.f;

					// Get screen position of target.
					px = tDevPersp.Project(targLoc);
					float sightX = 2.f * px.x/ari.glw->rect().width() - 1.f;
					sightX /= windowAspect;
					sightX -= size/2.f;
					float sightY = 1.f - 2.f * px.y/ari.glw->rect().height();
					sightY -= size/2.f;
					ari.RenderItemEffect(IE_ReticleBox, sightX, sightY, size, r,g,b,a);
				}
			}
		}

		AuId motherShipId(MotherShip());
		if(!motherShipId.IsNull())
		{
			// Show a reticle on my mother ship.
			const DaVessel* motherShip = dynamic_cast<DaVessel*>(system->FindIdRecursive(motherShipId));
			if(motherShip)
			{
				// Get the motherShip blip location in the scene atom's eye space.
				AuPoint motherShipLoc = sceneAtom->EnvTransform().UnProject(motherShip->EnvLocation());
				AuPoint px = tDevOrtho.Project(motherShipLoc);
				if(px.z < 0)
				{
					// Blip is in front of the eye plane.
					// Calculate square icon size.
					const float size = 128.f/1024.f;

					// Get screen position of target.
					px = tDevPersp.Project(motherShipLoc);
					float sightX = 2.f * px.x/ari.glw->rect().width() - 1.f;
					sightX /= windowAspect;
					sightX -= size/2.f;
					float sightY = 1.f - 2.f * px.y/ari.glw->rect().height();
					sightY -= size/2.f;
					ari.RenderItemEffect(IE_MotherShipReticle, sightX, sightY, size, r,g,b,a);
				}
			}
		}
	}
}

void DaVehicle::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	DaUnit::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);

	if(!result.m_Damage1 && !result.m_Damage2)
		return;	// No damage to report.

	int iDamageDealt = 0;
	if(result.m_Damage1)
	{
		// Certain weapon effects do not affect vehicles.
		switch(weapon.m_w1.m_Effect)
		{
		case WE_None:
		case WE_Audible:
		case WE_Invalid:
		case WE_Bleeding:
		case WE_Disease:
		case WE_Suffocate:
		case WE_Stun:
		case WE_Poison:
		case WE_Starving:
			break;
		case WE_Acid:
		case WE_Bludgeon:
		case WE_Cold:
		case WE_Corrosion:
		case WE_Crush:
		case WE_Electric:
		case WE_Unused:
		case WE_Heat:
		case WE_Implant:
		case WE_Pierce:
		case WE_Radiation:
		case WE_Slash:
		case WE_Sonic:
		case WE_Generic:
			// Roll "saving throw" based on manufactured quality.
			m_VehicleLock.lockForWrite();
			if(result.m_Damage1 >= m_Hits)
			{
				iDamageDealt = m_Hits;
				m_Hits = 0;
			}
			else
			{
				iDamageDealt = result.m_Damage1;
				m_Hits -= result.m_Damage1;
			}
			m_tmHitsChanged.Set();
			m_VehicleLock.unlock();
			break;
		}
	}
	if(result.m_Damage2)
	{
		// Certain weapon effects do not affect vehicles.
		switch(weapon.m_w2.m_Effect)
		{
		case WE_None:
		case WE_Audible:
		case WE_Invalid:
		case WE_Bleeding:
		case WE_Disease:
		case WE_Suffocate:
		case WE_Stun:
		case WE_Poison:
		case WE_Starving:
			break;
		case WE_Acid:
		case WE_Bludgeon:
		case WE_Cold:
		case WE_Corrosion:
		case WE_Crush:
		case WE_Electric:
		case WE_Unused:
		case WE_Heat:
		case WE_Implant:
		case WE_Pierce:
		case WE_Radiation:
		case WE_Slash:
		case WE_Sonic:
		case WE_Generic:
			m_VehicleLock.lockForWrite();
			if(result.m_Damage2 >= m_Hits)
			{
				iDamageDealt += m_Hits;
				m_Hits = 0;
			}
			else
			{
				iDamageDealt += result.m_Damage2;
				m_Hits -= result.m_Damage2;
			}
			m_tmHitsChanged.Set();
			m_VehicleLock.unlock();
			break;
		}
	}

	// Record damage dealt by atom into this unit's damage log.
	if(iDamageDealt && (!attacker.m_Id.IsNull() || weapon.m_CID != CID_Invalid))
		RecordDamage(attacker.m_Id, iDamageDealt, weapon.m_CID, weapon.m_Quality);

	// Send a message to the occupants of this vehicle that they received damage.
	short int curHits = CurrentHitPoints();
	if(!curHits)
	{
		// Notify the occupants when they are destroyed.
		QString text = FormatDamageReceived(result.m_CriticalHits, weapon.m_w1.m_Effect, result.m_Damage1, result.Armor1(),
				weapon.m_w2.m_Effect, result.m_Damage2, result.Armor2(), attacker.m_Name);
		DestroyVehicle(text);		// Kill's this atom.
		result.m_IsDestroyed = true;
	}
	else
	{
		if(weapon.m_CID != CID_Fire && !IsBurning())
		{
			// The damage was not from an existing fire condition.
			// Did the vehicle catch on fire.
			float health = (float)curHits/(float)MaxHitPoints();
			if(health <= .1f)
			{
				// At 90% damage, all vehicles catch fire.
				AddCondition(new AuConFire(AuTime::Now()));
				SetModified();
			}
		}
		if((weapon.m_w1.m_Effect == WE_Acid && result.m_Damage1) || (weapon.m_w2.m_Effect == WE_Acid && result.m_Damage2))
		{
			// Acid damage may induce an acid condition on this creature.
			if(weapon.m_CID != CID_Acid && !HasAcid())
			{
				// The damage was not from an existing acid condition.
				// This vehicle got corroded.
				// Strength of the acid is determined by total unabsorbed strength of the acid damage.
				int acidStrength;
				if(weapon.m_w1.m_Effect == WE_Acid)
					acidStrength = result.m_Damage1;
				else
					acidStrength = result.m_Damage2;

				// Add the acid as a condition on this creature.
				//qDebug() << Id() << Name() << "corroded with acid, strength" << acidStrength;
				AuConAcid* acid = new AuConAcid(acidStrength, weapon.m_Quality, AuTime::Now());
				AddCondition(acid);
			}
		}
		if((result.m_Damage1 && weapon.IsIncendiary1()) || (result.m_Damage2 && weapon.IsIncendiary2()))
		{
			// Poison damage may induce a poisoned condition on this vehicle.
			if(weapon.m_CID != CID_Fire && !IsBurning())
			{
				// The damage was not from an existing fire condition.
				// This vehicle got burned.
				// 10% chance to add fire as a condition on this vehicle.
				if(g_Au.Dice().Roll(1,100) <= 10)
				{
					AuConFire* fire = new AuConFire(AuTime::Now());
					AddCondition(fire);
				}
			}
		}
		if(((weapon.m_w1.m_Effect == WE_Stun && result.m_Damage1) ||
					(weapon.m_w2.m_Effect == WE_Stun && result.m_Damage2)) && !IsParalyzed())
		{
			// Paralysis damage may induce a paralyzed condition on this vehicle.
			// This vehicle got paralyzed.
			// Strength of the paralysis is determined by total unabsorbed strength of the paralysis damage.
			int paralysisStrength;
			if(weapon.m_w1.m_Effect == WE_Stun)
				paralysisStrength = result.m_Damage1;
			else
				paralysisStrength = result.m_Damage2;

			// Add the disease as a condition on this vehicle.
			AuConParalysis* paralysis = new AuConParalysis(paralysisStrength, weapon.m_Quality, AuTime::Now());
			AddCondition(paralysis);
		}

		// Kick this object into motion.
		if(Mass() > 0 && wpnLoc != hitLoc)
		{
			// Calculate movement based on kinetic force applied.
			double movement = result.Armor1() + result.Armor2();
			if(IsKineticEffect(weapon.m_w1.m_Effect))
				movement += result.m_Damage1;
			if(IsKineticEffect(weapon.m_w2.m_Effect))
				movement += result.m_Damage2;
			if(movement > 0)
			{
				movement *= KineticDamageFtLbsPerSec() / Mass();
				AuPoint fromDir = wpnLoc - hitLoc;
				fromDir.Unit(movement);
				ForceVector(Vector() - fromDir);
			}
		}
		if(weapon.m_CID != CID_Atmosphere &&
				weapon.m_CID != CID_InfernoAtmosphere &&
				weapon.m_CID != CID_FrigidAtmosphere)
			SetUsed();												// Let this vehicle decay.
	}

	SetModified();

	//AuCommMessage msg("Intercom", Id(), Name(), FindParentSector()->Id(), AuCommMessage::Close, text, AuCommMessage::Chat);
	//g_Au.SendAtomChat(msg);
}

void DaVehicle::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	DaUnit::CombatArmorEffectiveness(mode, effect, hitShield, wpnLoc, hitLoc, baseAbsorb, baseBlock, auxAbsorb, auxBlock);

	// Get base armor values from the vehicle.
	float vehicleQMult = QualityBoostF(Quality());
	const DbCommodity& vehicleC = g_Au.Commodity(CommodityId());
	baseAbsorb += (vehicleC.ArmorModeAbsorb(mode) + vehicleC.ArmorEffectAbsorb(effect));
	baseBlock += (vehicleC.ArmorModeBlock(mode) + vehicleC.ArmorEffectBlock(effect)) * vehicleQMult;
}

int DaVehicle::WeaponCount() const
{
	if(m_Model)
		return m_Model->WeaponCount();
	return 0;
}

AuPoint DaVehicle::WeaponLocation(int weaponIndex) const
{
	if(m_Model)
		return m_Model->WeaponLocation(weaponIndex);
	return AuPoint(0,0);
}

AuPoint DaVehicle::WeaponDirection(int weaponIndex) const
{
	if(m_Model)
		return m_Model->WeaponDirection(weaponIndex);
	return AuPoint(0,0);
}

AuPoint DaVehicle::WeaponRotation(int weaponIndex) const
{
	if(m_Model)
	{
		// Determine rotation required to point forward vector (0,1,0) to
		// weapon firing direction.
		AuPoint dir = m_Model->WeaponDirection(weaponIndex);
		AuMatrix4x4 trot = AuPoint(0,1,0).GetAlignmentTransform(dir);
		return trot.GetRotation();
	}
	return AuPoint(0,0);
}

int DaVehicle::ColonistsAboard() const
{
	int colonists = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCitizen* citizen = dynamic_cast<DaCitizen*>(*it);
		if(!citizen)
			continue;
		if(!citizen->IsNpcPassenger())
			continue;
		colonists++;
	}
	return colonists;
}

int DaVehicle::TroopsAboard() const
{
	int troops = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCitizen* citizen = dynamic_cast<DaCitizen*>(*it);
		if(!citizen || !citizen->IsNpcTroop())
			continue;
		troops++;
	}
	return troops;
}

// Fill the array with quads for determining hit locations when targeting this atom.
void DaVehicle::EnumSightSurfaces(AuQuadArray &aTargetSurfaces, const AuMatrix4x4 &tModel) const
{
	// Get the six surfaces created by the rough extents.
	AuBox fExt;
	if(RoughExtents(fExt))
	{
		fExt.Transform(tModel);
		fExt.GetQuads(aTargetSurfaces);
	}

	// Add interior surfaces. This helps when entering a vehicle when the person's head is inside the extents box.
	aTargetSurfaces.AddQuad((fExt.m_fCorners[0]+fExt.m_fCorners[3])/2,
			(fExt.m_fCorners[4]+fExt.m_fCorners[7])/2,
			(fExt.m_fCorners[5]+fExt.m_fCorners[6])/2,
			(fExt.m_fCorners[1]+fExt.m_fCorners[2])/2);
	aTargetSurfaces.AddQuad((fExt.m_fCorners[0]+fExt.m_fCorners[1])/2,
			(fExt.m_fCorners[4]+fExt.m_fCorners[5])/2,
			(fExt.m_fCorners[6]+fExt.m_fCorners[7])/2,
			(fExt.m_fCorners[2]+fExt.m_fCorners[3])/2);
	aTargetSurfaces.AddQuad((fExt.m_fCorners[0]+fExt.m_fCorners[4])/2,
			(fExt.m_fCorners[1]+fExt.m_fCorners[5])/2,
			(fExt.m_fCorners[2]+fExt.m_fCorners[6])/2,
			(fExt.m_fCorners[3]+fExt.m_fCorners[7])/2);
}

bool DaVehicle::BoardTroop()
{
	// Locate a troop to move aboard this vehicle.
	const DaWorldLL* llworld = FindParentWorldInEnv();
	if(!llworld)
		return false;
	DaNpc* troop = llworld->FindTroopReadyToLeave(*this);
	if(!troop)
		return false;
	return BoardTroop(troop);
}

bool DaVehicle::BoardTroop(DaNpc* troop)
{
	// Find a seat location to place the citizen atom.
	AuPoint seatLoc;
	AuPose seatPose;
	if(!FindEmptySeat(seatLoc, seatPose))
		return false;

	// Move the citizen atom from the planet to this vehicle.
	troop->MountVehicle(this);
	return true;
}

/*
// Troops monitor the Intercom channel when people are aboard.
void DaVehicle::RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup)
{
	switch(msg.Type())
	{
	case AuCommMessage::DebarkTroops:
		{
			if(IsInSpacecraft())
			{
				// This vehicle is aboard a spacecraft.
				DaVessel* ship = FindParentVesselInEnv();
				if(!ship)
					break;	// Not likely due to IsInSpacecraft check.

				// Deploy the troops to the spacecraft.
				unsigned int deployed = 0;			// The number of troops deployed.
				unsigned int count = msg.Count();
				for(unsigned int troopidx=0; troopidx<count; troopidx++)
				{
					// Locate a troop aboard this vehicle.
					DaNpc* troop = FindTroop();
					if(!troop)
						continue;

					// Transfer the troop to the parent spacecraft.
					if(ship->BoardTroop(troop))
						deployed++;
				}
				// Send the result.
				QString text;
				if(deployed == 1)
					text = tr("One troop deployed from this vehicle to the ship we're aboard.");
				else if(deployed)
					text = tr("%1 troops deployed from this vehicle to the ship we're aboard.").arg(deployed);
				else
					text = tr("No troops deployed. No space was found inside the ship we're aboard.");
				AuCommMessage vehiclereply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
						msg.ReceptionRange(), text, -1, "", AuCommMessage::Chat);
				g_Au.SendAtomChat(vehiclereply);
			}
			else
			{
				// Send the reason for ignoring the request.
				QString text = tr("This vehicle must be on the ground or aboard a spacecraft to deploy troops.");
				AuCommMessage vehiclereply(msg.ChannelName(), Id(), Name(), msg.FromSectorId(),
						msg.ReceptionRange(), text, -1, "", AuCommMessage::Chat);
				g_Au.SendAtomChat(vehiclereply);
			}
		}
		break;
	case AuCommMessage::Accept:
	case AuCommMessage::AcceptApplication:
	case AuCommMessage::AcceptBerthCaptain:
	case AuCommMessage::AcceptBerthCaptainSlot:
	case AuCommMessage::AcceptBerthCrew:
	case AuCommMessage::AcceptBerthOfficer:
	case AuCommMessage::AcceptBerthPassenger:
	case AuCommMessage::AcceptBerthTroop:
	case AuCommMessage::AcceptBuddy:
	case AuCommMessage::AcceptDesign:
	case AuCommMessage::AcceptInvitation:
	case AuCommMessage::AcceptPosition:
	case AuCommMessage::AcceptRepairAll:
	case AuCommMessage::AcceptRepairSystem:
	case AuCommMessage::Auction:
	case AuCommMessage::AuctionDesign:
	case AuCommMessage::BeamOver:
	case AuCommMessage::Bid:
	case AuCommMessage::BidDesign:
	case AuCommMessage::BoardColonists:
	case AuCommMessage::BoardCrew:
	case AuCommMessage::BoardOfficers:
	case AuCommMessage::BoardTroops:
	case AuCommMessage::Chat:
	case AuCommMessage::ChatContinued:
	case AuCommMessage::ChatAudio:
	case AuCommMessage::ChatChoice:
	case AuCommMessage::ChatPrivate:
	case AuCommMessage::CityTrade:
	case AuCommMessage::ColonistsBoarded:
	case AuCommMessage::ColonistsDebarked:
	case AuCommMessage::CrewBoarded:
	case AuCommMessage::CrewBoardingComplete:
	case AuCommMessage::DebarkColonists:
	case AuCommMessage::DesignForSale:
	case AuCommMessage::ForSale:
	case AuCommMessage::HailReply:
	case AuCommMessage::HailRequest:
	case AuCommMessage::InventoryItem:
	case AuCommMessage::InviteToEmpire:
	case AuCommMessage::Jettison:
	case AuCommMessage::JoinEmpire:
	case AuCommMessage::Narrate:
	case AuCommMessage::NarrateAudio:
	case AuCommMessage::NarrateChoice:
	case AuCommMessage::NarrateImage:
	case AuCommMessage::NarrateTitle:
	case AuCommMessage::NarrateVideo:
	case AuCommMessage::OfferPosition:
	case AuCommMessage::OfferRepairAll:
	case AuCommMessage::OfferRepairSystem:
	case AuCommMessage::OfficersBoarded:
	case AuCommMessage::PassengerBoardingComplete:
	case AuCommMessage::PreBoard:
	case AuCommMessage::RequestBerthAny:
	case AuCommMessage::RequestBerthCaptain:
	case AuCommMessage::RequestBerthCaptainSlot:
	case AuCommMessage::RequestBerthCrew:
	case AuCommMessage::RequestBerthOfficer:
	case AuCommMessage::RequestBerthPassenger:
	case AuCommMessage::RequestBerthTroop:
	case AuCommMessage::RequestBuddy:
	case AuCommMessage::RequestExterminator:
	case AuCommMessage::RequestInventory:
	case AuCommMessage::RequestRepairAll:
	case AuCommMessage::RequestRepairSystem:
	case AuCommMessage::RevokeBid:
	case AuCommMessage::SolicitAllSales:
	case AuCommMessage::SolicitAmmoSales:
	case AuCommMessage::SolicitArmorSales:
	case AuCommMessage::SolicitConstructionMaterialSales:
	case AuCommMessage::SolicitDesignSales:
	case AuCommMessage::SolicitFoodSales:
	case AuCommMessage::SolicitFuelSales:
	case AuCommMessage::SolicitMedSales:
	case AuCommMessage::SolicitItemSales:
	case AuCommMessage::SolicitPartsSales:
	case AuCommMessage::SolicitToolSales:
	case AuCommMessage::SolicitWeaponSales:
	case AuCommMessage::SpotTargetOrTerrain:
	case AuCommMessage::System:
	case AuCommMessage::TroopBoardingComplete:
	case AuCommMessage::TroopsBoarded:
		break;
	case AuCommMessage::CommTypes:
		Q_ASSERT(false);
		break;
	}
}
*/

// 'hit' is the atom that was hit.
// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaVehicle::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	if(hitNormal.IsZero())
	{
		ForceVector(AuPoint(0,0));
		return AuPoint(0,0);
	}
	return DaUnit::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
}

// Returns true if any creature is aboard.
bool DaVehicle::IsOccupied() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* person = dynamic_cast<DaPerson*>(*it);
		if(person)
		{
			if(person->IsInvisibleOffLine())
				continue;	// Skip players who are not online.
			return true;
		}
		else
		{
			DaCreature* creature = dynamic_cast<DaCreature*>(*it);
			if(!creature)
				continue;
			return true;
		}
	}
	return false;
}

void DaVehicle::SetAtRest()
{
	//qDebug("DaVehicle::SetAtRest %d %s", Id().GetID(), (const char*)Name().toAscii());
	DaUnit::SetAtRest();
	Orient(Location(), Transform().ProjectVector(TerrainNormal()));	// Instant orient when at rest.
}

// Refuel, repair and rearm this vehicle from its environment.
// 'person' may provide fuel if it can't be acquired from the environment.
void DaVehicle::Service(DaPerson* person)
{
	if(IsLaunching())
		return;
	// Must be going less than 1m/s to get serviced.
	quint8 fuelMax = FuelMax();
	COMMODITYID fuelCID = FuelType();
	if(Speed() < FeetPerMeter())
	{
		if(IsInCapsule())
		{
			// Inside a spacecraft.
			// Get serviced if I'm in a vehicle bay.
			if(CapsuleRoomType() == ROOM_VehicleBay)
			{
				DaCapsule* ship = FindParentCapsuleInEnv();
				if(ship)
				{
					// Export restriction CID_Sunlight is a placeholder for "repairs", which isn't an actual commodity.
					POLITICAL_STANCE stance = ship->PoliticalStanceMakeContact(this);
					const DgData& gd = g_Au.LockDataForRead();
					bool isFuelRestricted = gd.EmpireCommerceRestricted(ship->EmpireId(), fuelCID, stance);
					bool isRepairRestricted = gd.EmpireCommerceRestricted(ship->EmpireId(), CID_Sunlight, stance);
					g_Au.UnlockData();

					if(!isFuelRestricted || !isRepairRestricted)
						RepairConditionsTell(person, 0, "");

					if(!isFuelRestricted)
					{
						// Export of fuel is not restricted based on political stance.
						// Take fuel from the ship.
						if(m_Fuel < fuelMax && ship->RemoveFuel(fuelCID))
						{
							// The ship gave up a unit of fuel.
							// Refuel this vehicle to the max.
							Refuel();

							QString text = QString("%1 refueled in vehicle bay.").arg(Name());
							AuCommThought msg(text, person->Id(), person->Name());
							g_Au.SendAtomChat(msg);
						}
					}

					if(!isRepairRestricted)
					{
						// Export of repairs is not restricted based on political stance.
						// Quality influences rate of repair.
						if(RepairHitPoints(10 + m_Quality/25))
						{
							QString text = QString("%1 repaired in vehicle bay.").arg(Name());
							AuCommThought msg(text, person->Id(), person->Name());
							g_Au.SendAtomChat(msg);

							// Player Statistics
							person->Stats().VehicleRepairsBought(this);
							person->IncreaseSkill(DriverSkill(), 1);
						}
					}

					if(CommodityId() == CID_SpaceFighter ||
							CommodityId() == CID_AdvSpaceFighter ||
							CommodityId() == CID_SpaceTransport ||
							CommodityId() == CID_APC)
					{
						// Weapon
						//DHH Weapon aboard ship
					}
				}
			}
		}
		else if(IsInAtmosphere())
		{
			// Inside an atmosphere and not inside a spacecraft.
			// Get serviced if I'm in the right place.
			DaWorld* planet = FindParentPlanetInEnv();
			if(planet)
			{
				if(!BdId().IsNull())
				{
					// At a building.
					INDUSTRYID iid = InIndustry();
					if(iid == IID_RetailStore ||
							iid == IID_Shed ||			// Refuel at sheds to allow little gas pumps, etc.
							iid == IID_StorageTank ||	// Refuel at storage tanks.
							((IsSpaceVehicle() || CommodityId() == CID_Helicopter) && (iid == IID_AircraftFactory ||
												  iid == IID_AirportTerminal ||
												  iid == IID_BaseAirTerminal)) ||
							(IsGroundVehicle() && (iid == IID_VehicleFactory ||
												   iid == IID_BaseMotorPool)) ||
							(IsWaterVehicle() && (iid == IID_BaseDock ||
												  iid == IID_ShipYard)))
					{
						// Export restriction CID_Sunlight is a placeholder for "repairs", which isn't an actual commodity.
						DaBd* bd = planet->FindBd(BdId());
						if(bd)
						{
							quint32 empireId = bd->EmpireId();
							POLITICAL_STANCE stance = bd->PoliticalStanceMakeContact(this);
							const DgData& gd = g_Au.LockDataForRead();
							bool isFuelRestricted = gd.EmpireCommerceRestricted(empireId, fuelCID, stance);
							bool isRepairRestricted = gd.EmpireCommerceRestricted(empireId, CID_Sunlight, stance);
							g_Au.UnlockData();

							if(!isFuelRestricted || !isRepairRestricted)
								RepairConditionsTell(person, 1, bd->Name());

							if(!isFuelRestricted)
							{
								// Export of fuel is not restricted based on political stance.
								// Take fuel from the ship.
								if(m_Fuel < fuelMax && bd->CommunityRawRemove(fuelCID, 0, 1))
								{
									// The community gave up a unit of fuel.
									// Refuel this vehicle to the max.
									Refuel();

									QString text = QString("%1 refueled at %2.").arg(Name()).arg(bd->Name());
									AuCommThought msg(text, person->Id(), person->Name());
									g_Au.SendAtomChat(msg);
								}
							}

							if(!isRepairRestricted)
							{
								// Export of repairs is not restricted based on political stance.
								// Quality influences rate of repair.
								if(RepairHitPoints(10 + m_Quality/25))
								{
									QString text = QString("%1 repaired at %2.").arg(Name()).arg(bd->Name());
									AuCommThought msg(text, person->Id(), person->Name());
									g_Au.SendAtomChat(msg);

									// Player Statistics
									person->Stats().VehicleRepairsBought(this);
									person->IncreaseSkill(DriverSkill(), 1);
								}
							}

							if(CommodityId() == CID_SpaceFighter ||
									CommodityId() == CID_AdvSpaceFighter ||
									CommodityId() == CID_SpaceTransport ||
									CommodityId() == CID_APC)
							{
								// Weapon
								//DHH Weapon from city
							}
						}
					}
				}
			}
		}
		else
		{
			// Not inside an atmosphere and not inside a spacecraft.
			// Get serviced by a station if I am within its trade range.
			bool hasStation;
			DaVessel* station = StationInTradeRange(0, hasStation);
			if(station)
			{
				// This vehicle is within the trade range of the station.
				// Export restriction CID_Sunlight is a placeholder for "repairs", which isn't an actual commodity.
				POLITICAL_STANCE stance = station->PoliticalStanceMakeContact(this);
				const DgData& gd = g_Au.LockDataForRead();
				bool isFuelRestricted = gd.EmpireCommerceRestricted(station->EmpireId(), fuelCID, stance);
				bool isRepairRestricted = gd.EmpireCommerceRestricted(station->EmpireId(), CID_Sunlight, stance);
				g_Au.UnlockData();

				if(!isFuelRestricted || !isRepairRestricted)
					RepairConditionsTell(person, 4, "");

				if(!isFuelRestricted)
				{
					// Export of fuel is not restricted based on political stance.
					// Take fuel from the station.
					if(m_Fuel < fuelMax && station->RemoveFuel(fuelCID))
					{
						// The station gave up a unit of fuel.
						// Refuel this vehicle to the max.
						Refuel();

						QString text = QString("%1 refueled by station.").arg(Name());
						AuCommThought msg(text, person->Id(), person->Name());
						g_Au.SendAtomChat(msg);
					}
				}

				if(!isRepairRestricted)
				{
					// Export of repairs is not restricted based on political stance.
					// Quality influences rate of repair.
					if(RepairHitPoints(10 + m_Quality/25))
					{
						QString text = QString("%1 repaired by station.").arg(Name());
						AuCommThought msg(text, person->Id(), person->Name());
						g_Au.SendAtomChat(msg);

						// Player Statistics
						person->Stats().VehicleRepairsBought(this);
						person->IncreaseSkill(DriverSkill(), 1);
					}
				}

				if(CommodityId() == CID_SpaceFighter ||
						CommodityId() == CID_AdvSpaceFighter ||
						CommodityId() == CID_SpaceTransport ||
						CommodityId() == CID_APC)
				{
					// Weapon
					//DHH Weapon at station
				}
			}
		}
	}

	// Get fuel from the person's inventory if I didn't get it anywhere else.
	// Don't get repairs from the person. They have to do that with the appropriate tool.
	if(m_Fuel < fuelMax)
	{
		const AuGear* pinv = person->LockGearForRead();
		AuGearPos pos = pinv->FindBest(fuelCID);
		person->UnlockGear();
		if(pos.IsValid())
		{
			// The person has an appropriate fuel item in their inventory.
			// Consumed a unit of the fuel item from the person's inventory.
			person->ConsumeGear(pos, 1);

			// Refuel this vehicle to the max.
			Refuel();

			QString text = QString("%1 refueled from my gear.").arg(Name());
			AuCommThought msg(text, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
}

void DaVehicle::operator>>(QDataStream& out) const
{
	DaUnit::operator>>(out);
	out << m_byRotSpeedX;
	out << m_byRotSpeedY;
	out << m_byRotSpeedZ;
	out << (quint16)m_cid;
	out << m_DriverOnLine;
	out << m_Fuel;
	out << m_Parked;
	out << m_Quality;
	out << m_Switches;
	out << m_Throttle;
	out << m_ThrustLateral;
	out << m_ThrustLong;
	out << m_ThrustVertical;

	m_VehicleLock.lockForRead();
	out << m_wActions;
	out << m_MotherShip;
	out << AuTime();	//m_MotherShipChg;
	out << m_MotherShipName;
	out << AuTime();	//m_tmFuelChanged;
	AuTime tmLastBrake;
	out << tmLastBrake;
	AuTime tmLastBrakeLV;
	out << tmLastBrakeLV;
	AuTime tmLastPitchControl;
	out << tmLastPitchControl;
	AuTime tmLastRollControl;
	out << tmLastRollControl;
	AuTime tmLastThrottleControl;
	out << tmLastThrottleControl;
	AuTime tmLastThrottleFControl;
	out << tmLastThrottleFControl;
	AuTime tmLastThrottleLControl;
	out << tmLastThrottleLControl;
	AuTime tmLastThrottleVControl;
	out << tmLastThrottleVControl;
	AuTime tmLastTurnControl;
	out << tmLastTurnControl;

	out << m_Hits;
	out << AuTime();	//m_tmHitsChanged;

	out << m_Gun;
	out << m_Ord[0];
	out << m_Ord[1];
	out << m_OrdSelect;
	m_VehicleLock.unlock();

	m_LaunchLock.lockForRead();
	out << m_LaunchChg;
	out << m_LauncherExit;
	out << m_LauncherId;
	out << m_LauncherShip;
	out << m_LaunchSpeed;
	m_LaunchLock.unlock();
}

void DaVehicle::operator<<(QDataStream& in)
{
	DaUnit::operator<<(in);
	in >> m_byRotSpeedX;
	in >> m_byRotSpeedY;
	in >> m_byRotSpeedZ;
	quint16 ui16;
	in >> ui16;
	m_cid = (COMMODITYID)ui16;
	in >> m_DriverOnLine;
	in >> m_Fuel;
	in >> m_Parked;
	in >> m_Quality;
	in >> m_Switches;
	in >> m_Throttle;
	in >> m_ThrustLateral;
	in >> m_ThrustLong;
	in >> m_ThrustVertical;

	m_VehicleLock.lockForWrite();
	in >> m_wActions;
	in >> m_MotherShip;
	AuTime _MotherShipChg;
	in >> _MotherShipChg;
	in >> m_MotherShipName;
	AuTime _tmFuelChanged;
	in >> _tmFuelChanged;
	AuTime tmLastBrake;
	in >> tmLastBrake;
	AuTime tmLastBrakeLV;
	in >> tmLastBrakeLV;
	AuTime tmLastPitchControl;
	in >> tmLastPitchControl;
	AuTime tmLastRollControl;
	in >> tmLastRollControl;
	AuTime tmLastThrottleControl;
	in >> tmLastThrottleControl;
	AuTime tmLastThrottleFControl;
	in >> tmLastThrottleFControl;
	AuTime tmLastThrottleLControl;
	in >> tmLastThrottleLControl;
	AuTime tmLastThrottleVControl;
	in >> tmLastThrottleVControl;
	AuTime tmLastTurnControl;
	in >> tmLastTurnControl;

	in >> m_Hits;
	AuTime _tmHitsChanged;
	in >> _tmHitsChanged;

	in >> m_Gun;
	in >> m_Ord[0];
	in >> m_Ord[1];
	in >> m_OrdSelect;
	m_VehicleLock.unlock();

	m_LaunchLock.lockForWrite();
	in >> m_LaunchChg;
	in >> m_LauncherExit;
	in >> m_LauncherId;
	in >> m_LauncherShip;
	in >> m_LaunchSpeed;
	m_LaunchLock.unlock();
}

DaAtom* DaVehicle::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
		AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	//qDebug() << "DaVehicle::SightTarget" << Id() << Name();
	// Don't sight myself or my subatoms.
	if(!IsSightTarget(sightingAtom))
		return pBest;

	// Determine my location in the space of the sighting atom.
	// If I am far enough away from the sight segment, this atom and my subordinate atoms can be ignored.
	AuPoint myLoc = tModel.Translation();
	if(myLoc.DistToSegment(eyeLoc, sightedLoc) > 100)
		return pBest;

	if(this != sightingAtom->Parent() || !dynamic_cast<const DaCreature*>(sightingAtom))
	{
		// The sighting atom is not aboard this vehicle or the sighting atom is not a creature.
		// The rough test passed so we need to get more detailed.
		if(SightTargetDetails(eyeLoc, tModel, sightedLoc, sightedPart, sightedShield))
		{
			pBest = (DaAtom*)this;			// Must cast me to non-const.
			if(sightedSurfaceNormal)
				*sightedSurfaceNormal = AuPoint(0,0);
		}

		// Don't recurse if this is an enclosed vehicle.
		if(IsEnclosed())
			return pBest;					// Enclosed vehicle and sighting atom is not inside.
	}

	// Base class performs a recursive search.
	return DaUnit::SightTarget(sightingAtom, eyeLoc, tModel, pBest, sightedLoc, sightedPart, sightedShield, sightedSurfaceNormal);
}

void DaVehicle::KilledByPerson(DaPerson* killer)
{
	// This vehicle was killed by another person.
	killer->VehicleDestroyed(this);
}

void DaVehicle::UnitKilled(DaUnit* victim)
{
	// This vehicle killed a unit.
	// Base class declares mutual enemies appropriately.
	// We really should not get here because attacks using this vehicle are credited using the gunner's id.
	DaUnit::UnitKilled(victim);
}

bool DaVehicle::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(sceneAtom);
	// Calculate scene atom location in my coordinate space.
	AuPoint sceneLoc = EnvTransform().UnProject(cameraEnvLoc);
	return sceneLoc.Length2() < (16000*FeetPerMeter() * 16000*FeetPerMeter());	// about 10 miles
}

bool DaVehicle::IsSightTarget(const DaAtom* toWho) const
{
	// Vehicle does not sight on itself.
	if(toWho == this)
		return false;

	// This atom can be sighted when targeting a weapon or tool from outside. Occupants of the vehicle cannot target it.
	// This must change if occupants need to be able to repair the vehicle while inside.
	// This is only applied to creatures otherwise plastic explosive doesn't hurt the vehicle to which attached.
	if(toWho && toWho->Parent() == this && dynamic_cast<const DaCreature*>(toWho))
		return false;

	return true;
}

// 'sightedPart' will be the index into the array of surfaces returned by EnumSightSurfaces.
// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
bool DaVehicle::SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
		unsigned int& sightedPart, bool& sightedShield) const
{
	// Return value.
	bool hitMe = false;

	// Get the sightable surfaces.
	AuQuadArray solidSurfs;
	EnumSightSurfaces(solidSurfs, tModel);

	// Find the closest surface intersection.
	unsigned int part = 0;
	AuPoint intersect, normal;
	for(AuQuadArray::const_iterator it = solidSurfs.cbegin(); it != solidSurfs.cend(); it++, part++)
	{
		if((*it).TestSegmentIntersection(intersect, normal, eyeLoc, sightedLoc))
		{
			// We found an intersection with a solid surface.
			// Move the look location to the intersection.
			sightedLoc = intersect;
			sightedPart = part;
			sightedShield = false;
			hitMe = true;
		}
	}
	return hitMe;
}

// Add a visual and/or audio effect to this atom.
// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
void DaVehicle::AddEffect(AuEffect& effect)
{
	// Don't add bleeding effects to vehicles.
	if(effect.Visual() == VISUAL_HitBludgeon)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Bludgeon, 1);
	}
	else if(effect.Visual() == VISUAL_HitPierce)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Pierce, 1);
	}
	else if(effect.Visual() == VISUAL_HitSlash)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Slash, 1);
	}

	DaUnit::AddEffect(effect);
}

// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'targetLoc' is an output parameter relative to my parent world.
bool DaVehicle::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	if(IsNew())
		return false;
	return DaUnit::IsGuardTowerTarget(gunnerEmpire, gunnerPosture, gunLoc, gunRange, targetLoc);
}

// Return true if this atom can be targetted by a turret on a spacecraft.
// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
bool DaVehicle::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	if(IsNew())
		return false;
	return DaUnit::IsShipTurretTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange,
			gunSysVec, targetSysLoc);
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaVehicle::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return;
	// Subatoms of open vehicles can be targetted with turrets.
	if(!IsEnclosed())
		DaAtom::FindClosestShipTurretTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc,
				gunRange, gunSysVec, targetAtom, targetRange, targetSysLoc);

	if(IsNew())
		return;
	if(shipId == MotherShip())
		return;

	// Perform range test.
	AuPoint targetVec = EnvLocation() - gunSysLoc;		// Vector to target from gun's location.
	double range2 = targetVec.Length2();
	if(range2 > gunRange*gunRange)
		return;

	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	if(targetVec.Dot(gunSysVec) <= 0)						// Test field of fire.
		return;

	if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return;		// I am not an enemy of the gunner.

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = sqrt(range2);
	targetSysLoc = EnvTransform().Project(TargetLocation());
}

bool DaVehicle::FireGun(const DaCreature* gunner)
{
	AuEther* ether = Ether();
	if(!ether)
		return false;
	if(InArenaCannotFireWeapons())
		return false;

	// Vehicle must have engine power to fire weapon.
	// Not gun.
	//if(!IsEngineOn())
		//return false;

	// Go all the way to the parent solar system.
	const DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return false;

	// Fire gun if it has ammo and we're not exceeding its rate of fire.
	m_VehicleLock.lockForRead();
	if(!m_Gun.CanFire())
	{
		m_VehicleLock.unlock();
		return false;
	}

	// Determine weapon location and firing direction.
	AuPoint emitLoc = GunEmitLoc(m_Gun.FireSequence());
	AuPoint emitDir;
	if(CommodityId() == CID_APC)
	{
		// Tilt gun up/down with head tilt.
		float headTilt = gunner->CurrentHeadTilt();
		//qDebug() << "headTilt" << (headTilt*RAD2DEGf);
		if(headTilt < -10.f*DEG2RADf)
			headTilt = -10.f*DEG2RADf;
		else if(headTilt > 15.f*DEG2RADf)
			headTilt = 15.f*DEG2RADf;
		emitDir = AuPoint(0,1,0).RotateX(headTilt);
	}
	else
		emitDir = AuPoint(0,1,0);

	if(m_Gun.IsEnergyWeapon())
	{
		// Determine appropriate weapon effects.
		AuCombatWeapon weapon(m_Gun.Ammo(), 1);
		weapon.m_CID = m_cid;
		SOUND_EFFECT sound = (SOUND_EFFECT)m_Gun.SoundFired();
		VISUAL_EFFECT visual = m_Gun.VisualFired();
		m_VehicleLock.unlock();

		// Select target(s) in the firing direction of the vehicle.
		// Initialize sight dist for target search at max range.
		AuPoint sightedLoc = emitLoc + emitDir * weapon.m_Range;

		// Get reverse transformation back up to the parent solar system
		const AuMatrix4x4& mesys = EnvTransform();
		AuMatrix4x4 tModel = mesys.Inverse();

		// Ask the solar system to find the nearest solid atom that is in front of the weapon.
		unsigned int sightedPart = 0;
		bool sightedShield = false;
		DaAtom* sightedAtom = system->SightTarget(this, emitLoc, tModel, 0, sightedLoc, sightedPart, sightedShield, 0);
		if(sightedAtom && !sightedAtom->IsNoncombatant())
		{
			// Target found and it is not protected from attack.
			// Add weapon fire effect to weapon.
			if(sound != SOUND_None || visual != VISUAL_None)
			{
				AuEffect effect(emitLoc, AuPoint(0,0), AuPoint(0,0), sound, 1, 1, visual);
				effect.SetTargetLoc(sightedLoc);
				effect.SetEnergyBeamEffect(weapon.m_w1.m_Effect);
				AddEffect(effect);
			}

			double targDist = emitLoc.Distance(sightedLoc);
			/*
			if(targDist < weapon.m_Range && weapon.m_Range > 0)
			{
				// Calculate damage of energy weapons due to distance dropoff to target hit location.
				// Energy weapons must know the actual distance to the target to calculate damage amount.
				float dropOff = 1.f - targDist / weapon.m_Range;
				const float weaponPower = TurretPower(m_Quality);
				float targetE = round(weaponPower * dropOff);
				weapon.m_w1.m_Damage = (int)ceilf(targetE/2.f);
				weapon.m_w1.m_DamageMod = (int)round(targetE) - weapon.m_w1.m_Damage;
			}
			else
			{
				// Weapon is out of range.
				weapon.m_w1.m_Damage = 0;
				weapon.m_w1.m_DamageMod = 0;
				weapon.m_w1.m_AP = 0;
				weapon.m_w1.m_ST = 0;
				weapon.m_w2.m_Damage = 0;
				weapon.m_w2.m_DamageMod = 0;
				weapon.m_w2.m_AP = 0;
				weapon.m_w2.m_ST = 0;
			}
			*/

			QString n;
			if(IsEnclosed())
				n = tr("%1 in %2").arg(gunner->Name()).arg(Name());
			else
				n = tr("%1 on %2").arg(gunner->Name()).arg(Name());
			AuCombatAttacker attacker(gunner->Id(), n, gunner->EmpireId());
			attacker.m_MuzzleLoc = emitLoc;

			AuCombatDefender defender(sightedAtom, sightedLoc, sightedShield, targDist);

			AuCombatResult result;

			AttackTarget(attacker, defender, weapon, true, result);
		}
		else
		{
			// Add weapon fire effect to weapon.
			if(sound != SOUND_None || visual != VISUAL_None)
			{
				AuEffect effect(emitLoc, AuPoint(0,0), AuPoint(0,0), sound, 1, 1, visual);
				effect.SetTargetLoc(sightedLoc);
				effect.SetEnergyBeamEffect(weapon.m_w1.m_Effect);
				AddEffect(effect);
			}
		}

		// Gun fired.
		// Expend ammo and update fire time and sequencer.
		m_VehicleLock.lockForWrite();
		m_Gun.Fired();

		// Energy weapon automatically reloads using one unit of fuel.
		if(!m_Gun.RoundsCount() && m_Fuel > 1)
		{
			m_Gun.Reload(MaxWeaponRounds(m_Gun.CommodityId()));
			m_Fuel--;
			m_tmFuelChanged.Set();
		}
		m_VehicleLock.unlock();
	}
	else
	{
		// Gun fires a bullet if it's not an energy weapon.
		// Shoot gun at target. It does not lock on to targets.
		DaMissile* bullet = new DaMissile(*m_Gun.Ammo(), 1, EmpireId());
		bullet->ConnectSignals();
		bullet->Create();

		SOUND_EFFECT sound = (SOUND_EFFECT)m_Gun.SoundFired();
		VISUAL_EFFECT visual = m_Gun.VisualFired();

		m_VehicleLock.unlock();

		// Create a transform to position the bullet at the gun muzzle location, pointing in the gun fire direction.
		AuMatrix4x4 aim = AuPoint(0,0,1).GetAlignmentTransform(emitDir);
		AuMatrix4x4 t(Transform());
		t.Translate(emitLoc);
		t *= aim;
		bullet->SetPosAndOrient(AuAtomPos(ParentId(), t));
		ether->AddAtom(bullet);

		bullet->SetInScene();							// Don't decay too fast right away.
		bullet->SetVector(aim.UnProject(Vector()) + AuPoint(0,0,bullet->MuzzleExitVelocity()));	// Muzzle velocity plus my vec.
		bullet->SetInMotion();
		const DaCreature* person = Driver();
		if(person)
			bullet->ArmWeapon(Id(), tr("%1 in a %2").arg(person->Name()).arg(Name()), EmpireId());
		else
			bullet->ArmWeapon(Id(), Name(), EmpireId());
		bullet->SetOwnerIgnored(true);	// Do not hit my owner/armedBy atom.

		g_Au.AtomAdded(bullet->Id());
		bullet->SetNormal();	// New atom is ready for normal thread processing.
		bullet->TickTock();		// Immediately start tick processing.

		// Add weapon fire effect to weapon.
		if(sound != SOUND_None || visual != VISUAL_None)
		{
			AuEffect effect(emitLoc, AuPoint(0,0), AuPoint(0,0), sound, 1, 1, visual);
			effect.SetTargetLoc(AuPoint(0, 1000, 0));
			AddEffect(effect);
		}

		// Gun fired.
		// Expend ammo and update fire time and sequencer.
		m_VehicleLock.lockForWrite();
		m_Gun.Fired();
		m_VehicleLock.unlock();
	}
	SetModified();

	return true;
}

bool DaVehicle::FireWeapon(const DaCreature* gunner)
{
	AuEther* ether = Ether();
	if(!ether)
		return false;
	if(InArenaCannotFireWeapons())
		return false;

	//qDebug() << "DaVehicle::FireWeapon" << Id() << Name() << "ordSel" << m_OrdSelect
		//<< "canFire" << m_Ord[m_OrdSelect].CanFire();
	Q_UNUSED(gunner);
	// Vehicle must have engine power to fire weapon.
	if(!IsEngineOn())
		return false;

	// Go all the way to the parent solar system.
	const DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return false;

	// Fire gun if it has ammo and we're not exceeding its rate of fire.
	m_VehicleLock.lockForRead();
	if(!m_Ord[m_OrdSelect].CanFire())
	{
		m_VehicleLock.unlock();
		return false;
	}

	// Determine weapon location and firing direction.
	AuPoint emitLoc = WeaponEmitLoc(m_Ord[m_OrdSelect].FireSequence());
	AuPoint emitDir;
	if(CommodityId() == CID_APC)
	{
		// Tilt gun up/down with head tilt.
		float headTilt = gunner->CurrentHeadTilt();
		//qDebug() << "headTilt" << (headTilt*RAD2DEGf);
		if(headTilt < -10.f*DEG2RADf)
			headTilt = -10.f*DEG2RADf;
		else if(headTilt > 15.f*DEG2RADf)
			headTilt = 15.f*DEG2RADf;
		emitDir = AuPoint(0,1,0).RotateX(headTilt);
	}
	else
		emitDir = AuPoint(0,1,0);

	AuId gunnerWorldId;
	int gunnerDomain = GunnerDomain(gunnerWorldId);

	AuPoint bestSysLoc;
	double bestDistance = FeetPerOrbit();
	const DaAtom* target;
	if(m_Ord[m_OrdSelect].IsMissile())
		target = system->FindClosestMissileTarget(Id(), gunnerDomain, gunnerWorldId, EmpireId(), EnvTransform().Project(emitLoc), 0,
				bestSysLoc, bestDistance);
	else
		target = 0;

	// Shoot ordnance at target.
	DaMissile* bullet;
	if(target)
		bullet = new DaMissile(*m_Ord[m_OrdSelect].Ammo(), 1, EmpireId(), target->Id(),
				target->EnvTransform().UnProject(bestSysLoc));
	else
		bullet = new DaMissile(*m_Ord[m_OrdSelect].Ammo(), 1, EmpireId());
	bullet->ConnectSignals();
	bullet->Create();

	// Create a transform to position the bullet at the gun muzzle location, pointing in the gun fire direction.
	AuMatrix4x4 aim;
	if(m_Ord[m_OrdSelect].IsDeployableWeapon())
		aim = AuPoint(1,0,0).GetAlignmentTransform(emitDir);
	else
		aim = AuPoint(0,0,1).GetAlignmentTransform(emitDir);

	SOUND_EFFECT sound = (SOUND_EFFECT)m_Ord[m_OrdSelect].SoundFired();
	VISUAL_EFFECT visual = m_Ord[m_OrdSelect].VisualFired();

	m_VehicleLock.unlock();

	AuMatrix4x4 t(Transform());
	t.Translate(emitLoc);
	t *= aim;
	bullet->SetPosAndOrient(AuAtomPos(ParentId(), t));
	ether->AddAtom(bullet);

	bullet->SetInScene();							// Don't decay too fast right away.
	bullet->SetVector(aim.UnProject(Vector()) + AuPoint(0,0,bullet->MuzzleExitVelocity()));	// Muzzle velocity plus my vec.
	bullet->SetInMotion();
	const DaCreature* person = Driver();
	if(person)
		bullet->ArmWeapon(Id(), tr("%1 in a %2").arg(person->Name()).arg(Name()), EmpireId());
	else
		bullet->ArmWeapon(Id(), Name(), EmpireId());
	bullet->SetOwnerIgnored(true);	// Do not hit my owner/armedBy atom.

	g_Au.AtomAdded(bullet->Id());
	bullet->SetNormal();	// New atom is ready for normal thread processing.
	bullet->TickTock();		// Immediately start tick processing.

	// Add weapon fire effect to weapon.
	if(sound != SOUND_None || visual != VISUAL_None)
	{
		AuEffect effect(emitLoc, AuPoint(0,0), AuPoint(0,0), sound, 1, 1, visual);
		AddEffect(effect);
	}

	// Weapon fired.
	// Expend ammo and update fire time and sequencer.
	m_VehicleLock.lockForWrite();
	m_Ord[m_OrdSelect].Fired();
	m_VehicleLock.unlock();

	SetModified();

	return true;
}

AuPoint DaVehicle::GunEmitLoc(int fireSeq) const
{
	if(!m_Model)
		return AuPoint(0,0);

	AuPoint min,max;
	m_Model->GetExtents(min, max);

	// Guns fire from apparent gun positions on the model.
	AuPoint emitLoc;
	switch(CommodityId())
	{
	default:
		break;

	case CID_APC:
		// Measured off the blender model.
		emitLoc.y = 6.2627;
		emitLoc.z = 1.9936;
		if(fireSeq & 1)
			emitLoc.x = 4.3484;
		else
			emitLoc.x = -4.6484;
		break;

	case CID_SpaceRocket:
		emitLoc.y = max.y;
		break;
	case CID_SpaceTransport:
		emitLoc.y = max.y * .05;
		if(fireSeq & 1)
			emitLoc.x = max.x*.85;
		else
			emitLoc.x = min.x*.85;
		emitLoc.z = max.z * .5;
		break;
	case CID_SpaceFighter:
		emitLoc.y = max.y * .35;
		if(fireSeq & 1)
			emitLoc.x = max.x*.9;
		else
			emitLoc.x = min.x*.9;
		emitLoc.z = min.z * .7;
		break;
	case CID_AdvSpaceFighter:
		emitLoc.y = max.y * .9;
		if(fireSeq & 1)
			emitLoc.x = max.x*.72;
		else
			emitLoc.x = min.x*.72;
		break;
	};
	return emitLoc;
}

AuPoint DaVehicle::WeaponEmitLoc(int fireSeq) const
{
	if(!m_Model)
		return AuPoint(0,0);

	// Bombs drop from the center.
	m_VehicleLock.lockForRead();
	if(m_Ord[m_OrdSelect].IsDeployableWeapon())
	{
		m_VehicleLock.unlock();
		return AuPoint(0,0);
	}

	AuPoint min,max;
	m_Model->GetExtents(min, max);
	m_VehicleLock.unlock();

	// Rockets and missiles fire from launchers.
	AuPoint emitLoc;
	switch(CommodityId())
	{
	default:
		break;

	case CID_APC:
		// Measured off the blender model.
		emitLoc.y = -.9464;
		emitLoc.z = 3.7402;
		if(fireSeq & 1)
			emitLoc.x = 1.9392;
		else
			emitLoc.x = -1.9392;
		break;

	case CID_SpaceRocket:
		emitLoc.y = max.y * .5;
		if(fireSeq & 1)
			emitLoc.x = max.x*.25;
		else
			emitLoc.x = min.x*.25;
		break;
	case CID_SpaceTransport:
		emitLoc.y = max.y * .15;
		if(fireSeq & 1)
			emitLoc.x = max.x*.35;
		else
			emitLoc.x = min.x*.35;
		break;
	case CID_SpaceFighter:
		emitLoc.y = max.y * .15;
		if(fireSeq & 1)
			emitLoc.x = max.x*.5;
		else
			emitLoc.x = min.x*.5;
		break;
	case CID_AdvSpaceFighter:
		emitLoc.y = max.y * .4;
		if(fireSeq & 1)
			emitLoc.x = max.x*.2;
		else
			emitLoc.x = min.x*.2;
		break;
	};
	return emitLoc;
}

DaVehicle::WEAPON_RACK DaVehicle::CanLoadAmmunition(const AuItem& item) const
{
	// Only certain vehicles can load ordnance.
	WEAPON_RACK loadRack = RACK_None;
	switch(item.CommodityId())
	{
	default:
		break;
	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiRa:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiAc:
	case CID_StasisBeam:
	case CID_NeutronBeam:
	case CID_ElectronBeam:
	case CID_ErgRay:
	case CID_FreezeRay:
	case CID_CompressorBeam:
	case CID_DisruptorBeam:
	case CID_HeatRay:
	case CID_ProtonBeam:
	case CID_DisintegratorRay:
		// Don't replace gun ammo if it's full.
		m_VehicleLock.lockForRead();
		if(!m_Gun.RoundsCount() || m_Gun.RoundsCount() < MaxWeaponRounds(m_Gun.CommodityId()))
			loadRack = RACK_Gun;	// Rack is not full.
		m_VehicleLock.unlock();
		break;
	case CID_BombBl:
	case CID_BombBlRa:
	case CID_BombBlPi:
	case CID_BombElSt:
	case CID_BombSuPo:
	case CID_BombSlCr:
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
		// Go for the empty rack if there is one.
		m_VehicleLock.lockForRead();
		if(!m_Ord[0].RoundsCount())
			loadRack = RACK_Ord0;	// Rack is empty.
		else if(!m_Ord[1].RoundsCount())
			loadRack = RACK_Ord1;	// Rack is empty.
		else if(m_Ord[0].RoundsCount() < MaxWeaponRounds(m_Ord[0].CommodityId()))
			loadRack = RACK_Ord0;	// Rack is not full.
		else if(m_Ord[1].RoundsCount() < MaxWeaponRounds(m_Ord[1].CommodityId()))
			loadRack = RACK_Ord1;	// Rack is not full.
		m_VehicleLock.unlock();
		break;
	}
	return loadRack;
}

void DaVehicle::LoadAmmunition(const AuItem& item, WEAPON_RACK rack)
{
	switch(rack)
	{
	case RACK_None:
		Q_ASSERT(false);
		break;
	case RACK_Gun:
		m_VehicleLock.lockForWrite();
		m_Gun.Load(item, MaxWeaponRounds(item.CommodityId()));
		m_VehicleLock.unlock();
		SetModified();
		break;
	case RACK_Ord0:
		m_VehicleLock.lockForWrite();
		m_Ord[0].Load(item, MaxWeaponRounds(item.CommodityId()));
		m_VehicleLock.unlock();
		SetModified();
		break;
	case RACK_Ord1:
		m_VehicleLock.lockForWrite();
		m_Ord[1].Load(item, MaxWeaponRounds(item.CommodityId()));
		m_VehicleLock.unlock();
		SetModified();
		break;
	}
}

const DaAtom* DaVehicle::FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
		quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
		double& bestDistance) const
{
	if(!IsGunnerDomainTarget(missileDomain, missileWorldId, launcherId))
		return bestAtom;
	if(Is(launcherId))
		return bestAtom;

	// Missiles only lock on to neutral and enemy targets.
	POLITICAL_STANCE stance = PoliticalStance(missileEmpireId, EmpireId());
	if(stance != POL_Enemy && stance != POL_Neutral)
		return bestAtom;

	double dist = EnvLocation().Distance(missileSysLoc);
	if(bestDistance < 0. || dist < bestDistance)
	{
		bestDistance = dist;
		bestSysLoc = EnvLocation();
		return this;
	}

	// Don't check my subatoms; they can't be targetted.
	return bestAtom;
}

void DaVehicle::KilledByUnit(const DaUnit* killer)
{
	if(!killer->EmpireId())
		return;							// Killer is probably a wild animal.

	if(InArena())
		return;

	if(EmpireId() == killer->EmpireId())
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

	AuDocVehicleDestroyed doc(Id(), Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
	emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
}

void DaVehicle::RemoteDoorSwitch(DaPerson* sceneAtom)
{
	if(IsInsideCapsule())
	{
		// This vehicle is inside a spacecraft.
		// Hull doors are not locked to people inside the hull.
		DaCapsule* ship = FindParentCapsuleInEnv();
		if(!ship)
			return;

		// Toggle the state of the large hull doors in the room I'm in that is nearest to me.
		unsigned int doorIndex = ship->RemoteDoorIndex(true, Location(), CapsuleRoomId());
		ship->ToggleHullDoor(doorIndex);
	}
	else
	{
		// This vehicle is not inside a spacecraft.
		DaAtom* system = FindEnvironmentAtom();
		if(!system)
			return;	// Not likely.

		// Determine my location relative to the solar system.
		AuPoint mysysloc = EnvLocation();
		DaAtomList ships;
		system->FindCapsulesRecursive(ships);
		if(ships.isEmpty())
			return;	// No ships in the system.

		// Find the closest non-enemy ship to this vehicle.
		// Hull doors always act locked to enemy units.
		DaCapsule* closeship = 0;
		double closedist = 0;
		AuMatrix4x4 tcloseship;
		for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
		{
			DaCapsule* v = (DaCapsule*)(*si);
			if(!v->CheckAccess(sceneAtom))
				continue;
			const AuMatrix4x4& tship = (*si)->EnvTransform();
			AuPoint shipsysloc = tship.Translation();
			if(!closeship)
			{
				closeship = (DaCapsule*)(*si);
				tcloseship = tship;
				closedist = shipsysloc.Distance(mysysloc);
			}
			else
			{
				double dist = shipsysloc.Distance(mysysloc);
				if(dist < closedist)
				{
					closeship = (DaCapsule*)(*si);
					tcloseship = tship;
					closedist = dist;
				}
			}
		}
		if(!closeship)
			return;	// No non-enemy ship found.
		//qDebug() << "DaVehicle::RemoteDoorSwitch closeship" << closeship->Id()
			//<< closeship->Name() << "dist" << closedist;
		if(closedist > 1000)
			return;	// Too far away.

		// Convert my system location to be relative to the spacecraft.
		AuPoint myshiploc = tcloseship.UnProject(mysysloc);

		// Locked hull doors can only be operated by members of the crew. That is officers, crew, or troops.
		unsigned int doorIndex = closeship->RemoteDoorIndex(true, myshiploc);
		bool hullDoorLocked = closeship->HullDoorLocked(doorIndex);
		if(hullDoorLocked && !closeship->IsCrewOrTroop(sceneAtom->Id()) && !closeship->InChainOfCommand(sceneAtom))
			return;	// The door is locked to the person.

		// Tell the ship to open the bay door.
		closeship->ToggleHullDoor(doorIndex);
	}
}

void DaVehicle::RequestLaunch(DaPerson* sceneAtom)
{
	if(IsLaunching())
		return;
	if(!IsInsideCapsule())
		return;

	DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return;

	ship->RequestLaunch(sceneAtom, this);
}

void DaVehicle::RequestRecovery(DaPerson* sceneAtom)
{
	if(IsInsideCapsule())
		return;

	// This vehicle is not inside a spacecraft.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.

	// Determine my location relative to the solar system.
	AuPoint mysysloc = EnvLocation();
	DaAtomList ships;
	system->FindVesselsRecursive(ships);
	if(ships.isEmpty())
		return;	// No ships in the system.

	// Find the closest accessible ship to this vehicle that has a recovery system.
	DaCapsule* closeship = 0;
	double closedist = 0;
	AuMatrix4x4 tcloseship;
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaCapsule* v = (DaCapsule*)(*si);
		if(!v->HasSpaceVehicleRecoverySystem())
			continue;
		if(v->IsHidingInGroundClutter())
			continue;
		if(!v->CheckAccess(sceneAtom))
			continue;
		const AuMatrix4x4& tship = v->EnvTransform();
		AuPoint shipsysloc = tship.Translation();
		if(!closeship)
		{
			closeship = v;
			tcloseship = tship;
			closedist = shipsysloc.Distance(mysysloc);
		}
		else
		{
			double dist = shipsysloc.Distance(mysysloc);
			if(dist < closedist)
			{
				closeship = v;
				tcloseship = tship;
				closedist = dist;
			}
		}
	}
	if(!closeship)
		return;	// No non-enemy ship found.

	if(closedist > 5000*FeetPerMeter())
		return;	// Too far away.

	// Convert my system location to be relative to the spacecraft.
	AuPoint myshiploc = tcloseship.UnProject(mysysloc);

	// Tell the ship to activate the recovery system.
	closeship->RequestRecovery(sceneAtom, this, myshiploc);
}

void DaVehicle::Launch(const AuId& shipId, quint16 launcherId, AuMatrix4x4& launchPadM, AuPoint& launchExit)
{
	// Move the launch position up to where I will sit properly.
	launchPadM.Translate(0,0,GroundHeightAboveCG());
	AuPoint launchLoc = launchPadM.Translation();

	// Move the exit location up as well.
	AuPoint cgvec = launchPadM.ProjectVector(0,0,GroundHeightAboveCG());
	launchExit += cgvec;

	// Launch tube exit velocity is based on the length of the tube.
	float launchSpeed = 50*FeetPerMeter() + launchLoc.Distance(launchExit);

	// Launch
	m_LaunchLock.lockForWrite();
	m_LaunchChg.Set();
	m_LauncherExit = launchExit;
	m_LauncherId = launcherId;
	m_LauncherShip = shipId;
	m_LaunchSpeed = launchSpeed;
	m_LaunchLock.unlock();
	SwitchHoldOff();

	// Position this vehicle.
	ForcePosStop(AuAtomPos(ParentId(), launchPadM));
	InvalidateEnvironmentRecursive();
}

void DaVehicle::Park(DaAtom* parent, AuMatrix4x4& parkPosM, VEHICLE_SPOT_TYPE spotType)
{
	// Center of vehicle model is aligned with parking spot location.
	AuPoint vMin, vMax;
	GetExtents(vMin, vMax);

	// Parking location is currently on the floor.
	// Adjust parking location to be at center height of vehicle above the floor.
	if(spotType == SPOT_Rocket)
	{
		// Rocket takes special handling.
		parkPosM.Translate(-(vMin.x + vMax.x)/2,-(vMin.z + vMax.z)/2,-vMin.y);
		parkPosM.RotateZ(PI);			// So cockpit window lines up with parking spot icon window.
		parkPosM.RotateX(PI/2);
	}
	else
	{
		// Non-rockets are all alike.
		parkPosM.Translate(-(vMin.x + vMax.x)/2,-(vMin.y + vMax.y)/2,-vMin.z);
	}

	m_Throttle = 0;
	m_ThrustLateral = 0;
	m_ThrustLong = 0;
	m_ThrustVertical = 0;
	m_byRotSpeedX = 0;
	m_byRotSpeedY = 0;
	m_byRotSpeedZ = 0;
	m_Parked = false;					// So autopark will fix me up.

	ForcePosStop(AuAtomPos(parent->Id(), parkPosM));
	InvalidateEnvironmentRecursive();
}

bool DaVehicle::MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs)
{
	Q_UNUSED(pMove);
	Q_UNUSED(vMove);
	Q_UNUSED(elapsedMs);
	return false;
}

//static
VEHICLE_SPOT_TYPE DaVehicle::ParkingSpotType(COMMODITYID vehicleCID)
{
	switch(vehicleCID)
	{
	default:
		return SPOT_Invalid;
	case CID_APC:
	case CID_DirtBike:
	case CID_FarmMachinery:
	case CID_MineMachinery:
	case CID_SUV:
	case CID_Tank:
		return SPOT_GroundVehicle;
	case CID_CommercialShip:
	case CID_NavalShip:
	case CID_SailBoat:
	case CID_Submarine:
		return SPOT_WaterVehicle;
	case CID_AdvSpaceFighter:
	case CID_Helicopter:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
		return SPOT_SpaceVehicle;
	case CID_SpaceRocket:
		return SPOT_Rocket;
	}
}

bool DaVehicle::InParkingSpot() const
{
	return true;
}

bool DaVehicle::CanBdFetch(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher) const
{
	if(!IsNormal())
		return false;
	if(!IsNew())
		return false;					// Only grab 'nu' vehicles.
	if(ParkingSpotType() != spotType)
		return false;					// Wrong type of parking spot.

	// Buildings only fetch vehicles from certain buildings.
	switch(spotType)
	{
	case SPOT_Invalid:
	case SPOT_Count:
	default:
		return false;
	case SPOT_GroundVehicle:
		// Ground vehicles are fetched from vehicle factories.
		if(InIndustry() != IID_VehicleFactory)
			return false;
		break;
	case SPOT_Rocket:
	case SPOT_SpaceVehicle:
		// Space vehicles are fetched from aircraft factories.
		if(InIndustry() != IID_AircraftFactory)
			return false;
		break;
	case SPOT_WaterVehicle:
		// Water vehicles are fetched from ship yards.
		if(InIndustry() != IID_ShipYard)
			return false;
		break;
	}

	switch(CommodityId())
	{
	case CID_APC:
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
		return fetcher->IsMilitary();	// Military only.
	case CID_DirtBike:
	case CID_SUV:
		return !fetcher->IsMilitary();	// Civilian only.
	case CID_SailBoat:
	case CID_Helicopter:
		if(fetcher->AirDensity() < .2f)
			return false;
		return !fetcher->IsMilitary();	// Civilian only.
	case CID_FarmMachinery:
		return fetcher->IndustryId() == IID_Farm || fetcher->IndustryId() == IID_Orchard;
	case CID_SpaceRocket:
	case CID_Submarine:
		return true;					// Ok for military or civilians.
	default:
	case CID_CommercialShip:
	case CID_MineMachinery:
	case CID_NavalShip:
	case CID_Tank:
		return false;					// Not implemented.
	}
	return false;
}

bool DaVehicle::CanFetch(VEHICLE_SPOT_TYPE spotType) const
{
	if(!IsNormal())
		return false;
	if(!IsNew())
		return false;					// Only grab 'nu' vehicles.
	if(ParkingSpotType() != spotType)
		return false;					// Wrong type of parking spot.

	// Spacecraft only fetch vehicles from certain buildings.
	switch(spotType)
	{
	case SPOT_Invalid:
	case SPOT_Count:
	default:
		return false;
	case SPOT_GroundVehicle:
		// Ground vehicles are fetched from vehicle factories.
		if(InIndustry() != IID_VehicleFactory)
			return false;
		break;
	case SPOT_Rocket:
	case SPOT_SpaceVehicle:
		// Space vehicles are fetched from aircraft factories.
		if(InIndustry() != IID_AircraftFactory)
			return false;
		break;
	case SPOT_WaterVehicle:
		// Water vehicles are fetched from ship yards.
		if(InIndustry() != IID_ShipYard)
			return false;
		break;
	}

	switch(CommodityId())
	{
	default:
	case CID_APC:
	case CID_AdvSpaceFighter:
	case CID_DirtBike:
	case CID_Helicopter:
	case CID_SailBoat:
	case CID_SpaceFighter:
	case CID_SpaceRocket:
	case CID_SpaceTransport:
	case CID_Submarine:
	case CID_SUV:
		break;
	case CID_FarmMachinery:
		return false;					// Not wanted.
	case CID_CommercialShip:
	case CID_MineMachinery:
	case CID_NavalShip:
	case CID_Tank:
		return false;					// Not implemented.
	}
	return true;
}

void DaVehicle::SetEmpireId(quint32 idEmpire)
{
	if(Allegiance() != idEmpire)
	{
		ClearMotherShip();
		DaUnit::SetEmpireId(idEmpire);
	}
}

bool DaVehicle::RepairDamage(COMMODITYID tool)
{
	// Insure the tool can repair this vehicle.
	// We check for any tool that could be used to make the vehicle.
	switch(m_cid)
	{
	case CID_SailBoat:
		// Hammer, Glue Gun, Sewing Needle
		if(tool != CID_Hammer && tool != CID_GlueGun && tool != CID_SewingNeedle)
			return false;
		break;
	case CID_DirtBike:
		// Glue Gun, Wrench, Torque Wrench
		if(tool != CID_GlueGun && tool != CID_Wrench && tool != CID_TorqueWrench)
			return false;
		break;
	case CID_Submarine:
		// Hammer, Glue Gun, Wrench, Soldering Iron
		if(tool != CID_Hammer && tool != CID_GlueGun && tool != CID_Wrench && tool != CID_SolderIron)
			return false;
		break;
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
	case CID_Helicopter:
		// Hammer, Glue Gun, Wrench, Torque Wrench
		if(tool != CID_Hammer && tool != CID_GlueGun && tool != CID_Wrench && tool != CID_TorqueWrench)
			return false;
		break;
	case CID_SpaceFighter:
	case CID_AdvSpaceFighter:
	case CID_SpaceTransport:
		// Screw Driver, Soldering Iron, Wrench, Torque Wrench
		if(tool != CID_ScrewDriver && tool != CID_SolderIron && tool != CID_Wrench && tool != CID_TorqueWrench)
			return false;
		break;
	case CID_SpaceRocket:
		// Fitting Wrench, Screw Driver, Wrench, Torque Wrench
		if(tool != CID_FittingWrench && tool != CID_ScrewDriver && tool != CID_Wrench && tool != CID_TorqueWrench)
			return false;
		break;
	default:
		return false;
	}

	// We get here if the tool can fix this vehicle.
	// Quality influences rate of repair.
	return RepairHitPoints(10 + m_Quality/25);
}

// Repairs hit points of damage if needed.
// Returns false if no repairs are needed.
bool DaVehicle::RepairHitPoints(quint16 repairHits)
{
	short int hp = CurrentHitPoints();
	int maxHits = MaxHitPoints();
	if(hp >= maxHits)
		return false;

	m_VehicleLock.lockForWrite();
	m_tmHitsChanged.Set();

	if((int)hp + (int)repairHits >= maxHits)
	{
		m_Hits = maxHits;
		m_VehicleLock.unlock();

		ClearUnitDamage();
	}
	else
	{
		m_Hits += repairHits;
		m_VehicleLock.unlock();
	}
	SetModified();
	return true;
}

// Repairs all damage instantly.
void DaVehicle::ResetHitPoints()
{
	m_VehicleLock.lockForWrite();
	m_Hits = MaxHitPoints();
	m_tmHitsChanged.Set();
	m_VehicleLock.unlock();
	RepairConditions();
	ClearUnitDamage();
}

void DaVehicle::RepairConditionsTell(const DaPerson* person, int where, const QString& whereName)
{
	bool hasAcid = HasCondition(AuConAcid::ConditionType());
	bool hasFire = HasCondition(AuConFire::ConditionType());
	bool hasPar = HasCondition(AuConParalysis::ConditionType());
	if(!RepairConditions())
		return;
	QString text;
	switch(where)
	{
	case 0:
		if(hasFire && (hasAcid || hasPar))
			text = tr("%1 fire extinguished and hull decontaminated in vehicle bay.").arg(Name());
		else if(hasFire)
			text = tr("%1 fire extinguished in vehicle bay.").arg(Name());
		else
			text = tr("%1 decontaminated in vehicle bay.").arg(Name());
		break;
	case 1:
		if(hasFire && (hasAcid || hasPar))
			text = tr("%1 fire extinguished and hull decontaminated at %2.").arg(Name()).arg(whereName);
		else if(hasFire)
			text = tr("%1 fire extinguished at %2.").arg(Name()).arg(whereName);
		else
			text = tr("%1 decontaminated at %2.").arg(Name()).arg(whereName);
		break;
	case 2:
		if(hasFire && (hasAcid || hasPar))
			text = tr("%1 fire extinguished and hull decontaminated on the tarmac.").arg(Name());
		else if(hasFire)
			text = tr("%1 fire extinguished on the tarmac.").arg(Name());
		else
			text = tr("%1 decontaminated on the tarmac.").arg(Name());
		break;
	case 3:
		if(hasFire && (hasAcid || hasPar))
			text = tr("%1 fire extinguished and hull decontaminated at the dock.").arg(Name());
		else if(hasFire)
			text = tr("%1 fire extinguished at the dock.").arg(Name());
		else
			text = tr("%1 decontaminated at the dock.").arg(Name());
		break;
	case 4:
		if(hasFire && (hasAcid || hasPar))
			text = tr("%1 fire extinguished and hull decontaminated by station.").arg(Name());
		else if(hasFire)
			text = tr("%1 fire extinguished by station.").arg(Name());
		else
			text = tr("%1 decontaminated by station.").arg(Name());
		break;
	}
	if(text.isEmpty())
		return;

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);
}

quint64 DaVehicle::MemoryUsage() const
{
	quint64 mem = DaUnit::MemoryUsage();
	mem += sizeof(DaVehicle) - sizeof(DaUnit);
	mem += StringMemoryUsage(m_MotherShipName);

	mem -= sizeof(AuOrdnance) * 3;
	mem += m_Gun.MemoryUsage();
	mem += m_Ord[0].MemoryUsage();
	mem += m_Ord[1].MemoryUsage();
	return mem;
}

void DaVehicle::BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire)
{
	Q_UNUSED(surrenderingEmpire);
	if(!IsNew())
		return;
	SetEmpireId(toEmpire);
}

void DaVehicle::WorldCaptured(quint32 byEmpire)
{
	if(!IsNew())
		return;
	if(EmpireId() == byEmpire)
		return;
	SetEmpireId(byEmpire);
	SetModified();
}

double DaVehicle::FallSafeSpeed() const
{
	if(IsGroundVehicle())
		return 20 * FeetPerMeter();
	else if(IsSpaceRocket())
		return 10 * FeetPerMeter();
	else if(IsSpaceVehicle())
		return 15 * FeetPerMeter();
	//else if(IsWaterVehicle())
		return 10 * FeetPerMeter();
}

float DaVehicle::RelativeAudioGain(const DaPerson* sceneAtom) const
{
	if(sceneAtom->Parent() == this)
	{
		if(sceneAtom->HelmetClosed())
			return .25f;
		return .5f;
	}
	return DaUnit::RelativeAudioGain(sceneAtom);
}

void DaVehicle::EmpireDeleted(quint32 empireId)
{
	if(EmpireId() != empireId)
		return;
	SetEmpireId(0);
}

bool DaVehicle::IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho)
{
	Q_UNUSED(toWho);
	if(!m_Model)
		return false;

	// Use 3D extents of the 3ds model.
	AuPoint min,max;
	m_Model->GetExtents(min, max);
	AuPoint center = (min + max)/2;
	AuPoint radii = (max - min)/2;
	AuPoint isect1, isect2;
	int i = oloc.IntersectEllipsoid(center, radii, nloc, isect1, isect2);
	if(i == 0)
		return false;

	if(i < 0)
	{
		// Both oloc and nloc are inside this vehicle.
		if(oloc.Length2() < nloc.Length2())
			return false;	// Allow movement away.
		hitLoc = oloc;
		hitNormal = oloc - center;
	}
	else if(i == 1 && oloc.IsFuzzyZero())
	{
		// One intersection moving from (0,0) is likely to be someone exiting the vehicle.
		return false;
	}
	else
	{
		// One or two intersections. We only care about the first.
		hitLoc = isect1;
		hitNormal = isect1 - center;
	}
	hitNormal.Unit();
	return true;
}

void DaVehicle::FindVehiclesRecursive(DaAtomList& vehicles) const
{
	//if(!IsNormal())
		//return;
	DaUnit::FindVehiclesRecursive(vehicles);
	vehicles.append((DaVehicle*)this);	// Cast away constness.
}

void DaVehicle::FindVehiclesRecursive(DaAtomList& vehicles, const AuId& motherShipId) const
{
	//if(!IsNormal())
		//return;
	DaUnit::FindVehiclesRecursive(vehicles, motherShipId);
	if(MotherShip() == motherShipId)
		vehicles.append((DaVehicle*)this);	// Cast away constness.
}

void DaVehicle::FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
		SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
		double& targetRange) const
{
	if(IsNew())
		return;
	DaUnit::FindClosestShipThreatRecursive(shipId, shipDomain, shipWorldId, shipEmpire, shieldPolicy, shipSysLoc, sensorRange,
			targetAtom, targetRange);
}

bool DaVehicle::HasWeapon() const
{
	switch(m_cid)
	{
	case CID_DirtBike:
	case CID_Submarine:
	case CID_SUV:
	case CID_FarmMachinery:
	case CID_Helicopter:
	case CID_SailBoat:
	case CID_SpaceRocket:
		return false;
	case CID_APC:
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
		return true;
	default:
		break;
	}
	return false;
}

DaAtom::ATOM_TYPE DaVehicle::AtomType() const
{
	return ATOM_Vehicle;
}

AuSkill::SKILL DaVehicle::DriverSkill() const
{
	switch(m_cid)
	{
	case CID_APC:
	case CID_DirtBike:
	case CID_FarmMachinery:
	case CID_SailBoat:
	case CID_SUV:
		return AuSkill::VehicleDriver;	// 2D movement
	case CID_AdvSpaceFighter:
	case CID_Helicopter:
	case CID_SpaceFighter:
	case CID_SpaceRocket:
	case CID_SpaceTransport:
	case CID_Submarine:
		return AuSkill::VehiclePilot;	// 3D movement
	default:
		break;
	}
	return AuSkill::Invalid;
}

quint32 DaVehicle::ExperiencePointsForKilling() const
{
	quint32 xp = MaxHitPoints()/100;
	if(!xp)
		return 1;
	return xp;
}

int DaVehicle::TestMissileHit(const AuPoint& missileLoc, float proximity) const
{
	double roughRad2 = RoughRadius()/2;			// 1/2 my rough radius
	return Location().Distance(missileLoc) > (roughRad2 + proximity) ? 0 : 1;
}

bool DaVehicle::IsEthereal() const
{
	return IsLaunching() && ParentId() == LaunchingVessel();	// Ethereal while launching.
}

QString DaVehicle::WeaponLoadDesc(WEAPON_RACK rack) const
{
	QString v;
	m_VehicleLock.lockForRead();
	switch(rack)
	{
	case RACK_None:
		break;
	case RACK_Gun:
		v = m_Gun.Desc();
		break;
	case RACK_Ord0:
		v = m_Ord[0].Desc();
		break;
	case RACK_Ord1:
		v = m_Ord[1].Desc();
		break;
	}
	m_VehicleLock.unlock();
	return v;
}

double DaVehicle::ExploreRadius(const DaWorldLL& world) const
{
	// Under water gives worst exploration radius.
	if(IsCGSubmerged())
		return ObstacleRadius() + 50*FeetPerMeter();

	// Gas giant can only be explored while underwater.
	if(world.IsGasGiant())
		return 0;

	// Not a gas giant.
	if(!IsInAtmosphere())
	{
		// Space vehicle outside atmosphere can see the entire world.
		if(IsSpaceVehicle())
			return -1;	// Explore all.
		return 0;	// Other vehicles outside the atmosphere are useless.
	}

	// Exploration radius improves with altitude above the ground.
	return ObstacleRadius() + 250*FeetPerMeter() + AltitudeAGL();
}

