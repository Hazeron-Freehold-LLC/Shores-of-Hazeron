//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationhelm.cpp,v 1.51 2022/04/14 16:44:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationhelm.h"
//
#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "ausound.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"

#define d_Beacons						SOUND_Bleep8
#define d_CloseDoors					SOUND_Bleep4
#define d_FTLSelect						SOUND_Bleep4
#define d_FTLEngage						SOUND_Bleep5
#define d_FTLSetting					SOUND_Bleep6
#define d_GravHold						SOUND_Bleep2
#define d_GravOnOff						SOUND_Bleep1
#define d_Mode							SOUND_Bleep5
#define d_Moor							SOUND_Bleep6
#define d_Headlight						SOUND_Bleep7
#define d_LandingGear					SOUND_Bleep3
#define d_NavLights						SOUND_Bleep2
#define d_PowerPlant					SOUND_Bleep3
#define d_Stop							SOUND_Bleep8
#define d_Strobes						SOUND_Bleep1
#define d_Throttle						SOUND_Bleep7

// Table of control inputs.
DiControlInputArray s_PilotConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_PilotConsoleDefaults[] =
{
	// Maneuver
	DiControlInput(QObject::tr("Turn Left"),
				DpStationHelm::CI_TurnLeft, Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None, 0,100,
				true,false, DiControlInput::WT_None,2),
	DiControlInput(QObject::tr("Turn Right"),
				DpStationHelm::CI_TurnRight, Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None, 0,100,
				true,false, DiControlInput::WT_None,2),
	DiControlInput(QObject::tr("Pitch Up"),
				DpStationHelm::CI_PitchUp, Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
				true,false, DiControlInput::WT_None,2, 0,1),
	DiControlInput(QObject::tr("Pitch Down"),
				DpStationHelm::CI_PitchDown, Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
				true,false, DiControlInput::WT_None,2, 0,1),
	DiControlInput(QObject::tr("Roll Left"),
				DpStationHelm::CI_RollLeft, Qt::Key_Z,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisNegative, 0,100,
				true,false, DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Roll Right"),
				DpStationHelm::CI_RollRight, Qt::Key_C,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_AxisPositive, 0,100,
				true,false, DiControlInput::WT_None,2, 0,0),
	DiControlInput(QObject::tr("Thrust Forward"),
				DpStationHelm::CI_ThrustF, Qt::Key_unknown),
	DiControlInput(QObject::tr("Thrust Backward"),
				DpStationHelm::CI_ThrustB, Qt::Key_unknown),
	DiControlInput(QObject::tr("Thrust Left"),
				DpStationHelm::CI_DecreaseThrottleL, Qt::Key_Left),
	DiControlInput(QObject::tr("Thrust Right"),
				DpStationHelm::CI_IncreaseThrottleL, Qt::Key_Right),
	DiControlInput(QObject::tr("Thrust Up"),
				DpStationHelm::CI_IncreaseThrottleV, Qt::Key_Up),
	DiControlInput(QObject::tr("Thrust Down"),
				DpStationHelm::CI_DecreaseThrottleV, Qt::Key_Down),
	DiControlInput(QObject::tr("Brake"),
				DpStationHelm::CI_Brakes, Qt::Key_Space,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None),
	DiControlInput(QObject::tr("Brake Lateral and Vertical"),
				DpStationHelm::CI_BrakesLV, Qt::Key_Q,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None),

	// PageUp, PageDown, Delete, helm throttle control
	DiControlInput(QObject::tr("Maximum Forward Throttle"),
				DpStationHelm::CI_Throttle10,
				Qt::Key_PageUp,
				Qt::ShiftModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Maximum Reverse Throttle"),
				DpStationHelm::CI_ThrottleNeg10,
				Qt::Key_PageDown,
				Qt::ShiftModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Increase Throttle"),
				DpStationHelm::CI_IncreaseThrottle,
				Qt::Key_PageUp,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Decrease Throttle"),
				DpStationHelm::CI_DecreaseThrottle,
				Qt::Key_PageDown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("All Stop"),
				DpStationHelm::CI_AllStop,
				Qt::Key_Delete,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false),
	DiControlInput(QObject::tr("Set Throttle"),
				DpStationHelm::CI_SetThrottle,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				-10,10,true,false, DiControlInput::WT_None,0, 0,0,true),

	DiControlInput(QObject::tr("Moorings Secure/Cast Off"),
				DpStationHelm::CI_MooringsOnOff,
				Qt::Key_O, Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Headlight On/Off"),
				DpStationHelm::CI_HeadlightOnOff,
				Qt::Key_L, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Navigation Lights On/Off"),
				DpStationHelm::CI_NavLightsOnOff,
				Qt::Key_N, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Rotating Beacons On/Off"),
				DpStationHelm::CI_BeaconsOnOff,
				Qt::Key_B, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Strobes On/Off"),
				DpStationHelm::CI_StrobesOnOff,
				Qt::Key_S, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Landing Gear Extend/Retract"),
				DpStationHelm::CI_GearUpDown,
				Qt::Key_G, Qt::ShiftModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Gravity Drive
	DiControlInput(QObject::tr("Gravity Drive On/Off"),
				DpStationHelm::CI_GravityDriveOnOff,
				Qt::Key_G, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Gravity Drive Full/Hold"),
				DpStationHelm::CI_GravityDriveHold, Qt::Key_H,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Power Plant
	DiControlInput(QObject::tr("Power Plant On/Off"),
				DpStationHelm::CI_PowerPlantOnOff, Qt::Key_P,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// FTL Drive
	DiControlInput(QObject::tr("Select FTL Drive"),
				DpStationHelm::CI_FTLSelect, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Engage or Disengage FTL Drive"),
				DpStationHelm::CI_FTLEngage, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Increase FTL Drive Setting"),
				DpStationHelm::CI_FTLSettingUp, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Decrease FTL Drive Setting"),
				DpStationHelm::CI_FTLSettingDown, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Mode Cycle
	DiControlInput(QObject::tr("Reference Planet/System/Sector"),
				DpStationHelm::CI_ModeCycle, Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Door Controls
	DiControlInput(QObject::tr("Close All Hull Doors"),
				DpStationHelm::CI_CloseHullDoors, Qt::Key_B,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Remote Door Switch"),
				DpStationHelm::CI_DoorRemote, Qt::Key_M,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Look Directions
	DiControlInput(QObject::tr("Look Back"), DpStationHelm::CI_LookBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Forward"), DpStationHelm::CI_LookForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left"), DpStationHelm::CI_LookLeft,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right"), DpStationHelm::CI_LookRight,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Back"), DpStationHelm::CI_LookLeftBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Left Forward"), DpStationHelm::CI_LookLeftForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Back"), DpStationHelm::CI_LookRightBack,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
	DiControlInput(QObject::tr("Look Right Forward"), DpStationHelm::CI_LookRightForward,
				Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None,
				0,100,false,false, DiControlInput::WT_None,1),
};
int s_NumPilotConsoleDefaults = sizeof(s_PilotConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationHelm::DpStationHelm() : DpStation()
{
	m_Capacitor = 0;
	m_CapacitorTrend = 0;
	m_FuelAboard = 0;
	m_GravityDriveState = DaVessel::GD_Off;
	m_PowerPlantOn = false;
	m_Throttle = 0;
	m_FTLDriveCommit = FTL_COMMIT_Dormant;
	m_FTLDriveOn = false;
	m_FTLDriveSetting = 0;
	m_WhichFTL = 0;

	m_CaptureSequence = 0;

	// Init with bogus values to insure an update.
	m_LastFTLDriveCommit = FTL_COMMIT_Invalid;
	m_LastFTLDriveOn = -1;
	m_LastFTLDriveSetting = -1;
	m_LastWhichFTL = -1;
	m_LastAnyDoorsOpen = false;	// can be valid because invalid m_lLastHullDoors triggers the update
	m_LastGravityDriveState = DaVessel::GD_Invalid;
	m_LastInEnemyRange = -1;
	m_LastLatitude = 0;
	m_LastLongitude = 0;
	m_LastMode = -1;
	m_LastMooringState = -1;
	m_LastStress = -1;
	m_lLastBeacon = -1;
	m_lLastCapacitor = -1;
	m_lLastCapTrend = -1;
	m_lLastHeadlight = -1;
	m_lLastFuel = -1;
	m_lLastNavLight = -1;
	m_lLastPowerOn = -1;
	m_lLastSpeedSetting = 9999;
	m_lLastStrobe = -1;
	m_rLastAltitude = -123.456;
	m_rLastHeading = 9999.0;
	m_rLastPitch = 9999.0;
	m_rLastRoll = 9999.0;
	m_rLastSpeed = 0.0;
	m_rLastLSpeed = 0.0;
	m_rLastVSpeed = 0.0;
	m_Mode = 0;
	m_ScreenTex = 0;
}

DpStationHelm::~DpStationHelm()
{
}

DpStationHelm::DpStationHelm(const DpStationHelm& c) : DpStation(c)
{
	m_Capacitor = c.m_Capacitor;
	m_CapacitorTrend = c.m_CapacitorTrend;
	m_FuelAboard = c.m_FuelAboard;
	m_GravityDriveState = c.m_GravityDriveState;
	m_PowerPlantOn = c.m_PowerPlantOn;
	m_Throttle = c.m_Throttle;
	m_FTLDriveCommit = c.m_FTLDriveCommit;
	m_FTLDriveOn = c.m_FTLDriveOn;
	m_FTLDriveSetting = c.m_FTLDriveSetting;
	m_WhichFTL = c.m_WhichFTL;
	m_CaptureSequence = c.m_CaptureSequence;
	m_LastFTLDriveCommit = c.m_LastFTLDriveCommit;
	m_LastFTLDriveOn = c.m_LastFTLDriveOn;
	m_LastFTLDriveSetting = c.m_LastFTLDriveSetting;
	m_LastWhichFTL = c.m_LastWhichFTL;
	m_LastAnyDoorsOpen = c.m_LastAnyDoorsOpen;
	m_LastInEnemyRange = c.m_LastInEnemyRange;
	m_lLastBeacon = c.m_lLastBeacon;
	m_lLastCapacitor = c.m_lLastCapacitor;
	m_lLastCapTrend = c.m_lLastCapTrend;
	m_lLastHeadlight = c.m_lLastHeadlight;
	m_lLastFuel = c.m_lLastFuel;
	m_LastGravityDriveState = c.m_LastGravityDriveState;
	m_LastLatitude = c.m_LastLatitude;
	m_LastLongitude = c.m_LastLongitude;
	m_LastMode = c.m_LastMode;
	m_LastStress = c.m_LastStress;
	m_lLastNavLight = c.m_lLastNavLight;
	m_lLastPowerOn = c.m_lLastPowerOn;
	m_lLastSpeedSetting = c.m_lLastSpeedSetting;
	m_lLastStrobe = c.m_lLastStrobe;
	m_rLastAltitude = c.m_rLastAltitude;
	m_rLastHeading = c.m_rLastHeading;
	m_rLastPitch = c.m_rLastPitch;
	m_rLastRoll = c.m_rLastRoll;
	m_rLastSpeed = c.m_rLastSpeed;
	m_rLastLSpeed = c.m_rLastLSpeed;
	m_rLastVSpeed = c.m_rLastVSpeed;
	m_Mode = c.m_Mode;
	m_ScreenTex = 0;
}

void DpStationHelm::Tx(QDataStream& out) const
{
	DpStation::Tx(out);

	quint8 switches = 0x00;
	if(m_CapacitorTrend > 0)
		switches |= 0x01;
	else if(m_CapacitorTrend < 0)
		switches |= 0x02;

	if(m_GravityDriveState == DaVessel::GD_On)
		switches |= 0x04;
	else if(m_GravityDriveState == DaVessel::GD_Hold)
		switches |= 0x08;

	if(m_PowerPlantOn)
		switches |= 0x10;

	if(m_FTLDriveOn)
		switches |= 0x20;

	if(m_FTLDriveCommit > FTL_COMMIT_Dormant && m_FTLDriveCommit < FTL_COMMIT_ChargedBegin)
		switches |= 0x40;	// precharge phase
	else if(m_FTLDriveCommit >= FTL_COMMIT_ChargedBegin && m_FTLDriveCommit <= FTL_COMMIT_ChargedEnd)
		switches |= 0x80;	// charged hot phase
	else if(m_FTLDriveCommit > FTL_COMMIT_ChargedEnd)
		switches |= 0xc0;	// discharge phase

	out << switches;
	out << m_Throttle;
	out << (quint8)m_Capacitor;
	out << (quint8)m_FuelAboard;
	out << m_FTLDriveSetting;
	out << m_WhichFTL;
}

void DpStationHelm::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);

	quint8 switches;
	in >> switches;

	if(switches & 0x01)
		m_CapacitorTrend = 1;
	else if(switches & 0x02)
		m_CapacitorTrend = -1;
	else
		m_CapacitorTrend = 0;

	if(switches & 0x04)
		m_GravityDriveState = DaVessel::GD_On;
	else if(switches & 0x08)
		m_GravityDriveState = DaVessel::GD_Hold;
	else
		m_GravityDriveState = DaVessel::GD_Off;

	m_PowerPlantOn = (switches & 0x10) ? true : false;

	m_FTLDriveOn = (switches & 0x20) ? true : false;

	if((switches & 0xc0) == 0xc0)
		m_FTLDriveCommit = FTL_COMMIT_Discharge1;
	else if(switches & 0x40)
		m_FTLDriveCommit = FTL_COMMIT_Precharge1;
	else if(switches & 0x80)
		m_FTLDriveCommit = FTL_COMMIT_ChargedBegin;
	else
		m_FTLDriveCommit = FTL_COMMIT_Dormant;

	in >> m_Throttle;

	quint8 v8;
	in >> v8;
	m_Capacitor = (char)v8;
	in >> v8;
	m_FuelAboard = (char)v8;

	in >> m_FTLDriveSetting;
	in >> m_WhichFTL;
}

// Refresh data from counterpart.
void DpStationHelm::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationHelm* pA = (const DpStationHelm*)pCon;
	m_GravityDriveState = pA->m_GravityDriveState;
	m_Mode = pA->m_Mode;
	m_PowerPlantOn = pA->m_PowerPlantOn;
	m_FTLDriveCommit = pA->m_FTLDriveCommit;
	m_FTLDriveOn = pA->m_FTLDriveOn;
	m_FTLDriveSetting = pA->m_FTLDriveSetting;
	m_WhichFTL = pA->m_WhichFTL;
	m_Throttle = pA->m_Throttle;
	m_Capacitor = pA->m_Capacitor;
	m_CapacitorTrend = pA->m_CapacitorTrend;
	m_FuelAboard = pA->m_FuelAboard;
}

// Called between rendering scene frames that contains this object.
void DpStationHelm::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);

	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("PCon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	// Get current flight instrument readings from ship.
	double speed, speedF, speedL, speedV, altitude, pitch, roll, heading, longitude, latitude;
	int modeActual = ship.FlightInstruments(ship.DesignCenterOffset(), m_Mode, speed, speedF, speedL, speedV, altitude,
			pitch, roll, heading, longitude, latitude);
	if(!modeActual)
	{
		// Without a valid mode reference, some flight instrumentation values are invalid. Make them wander noticeably.
		altitude = m_rLastAltitude + g_Au.Dice().Roll(1,21,-11) * 123;
		pitch = m_rLastPitch + g_Au.Dice().Roll(1,21,-11)*DEG2RAD;
		roll = m_rLastRoll + g_Au.Dice().Roll(1,21,-11)*DEG2RAD;
		heading = m_rLastHeading + g_Au.Dice().Roll(1,21,-11)*DEG2RAD;
		longitude = 0;
		latitude = 0;
	}

	m_rLastLSpeed = speedL;
	m_rLastVSpeed = speedV;
	m_LastLatitude = latitude;
	m_LastLongitude = longitude;

	if(m_Throttle != m_lLastSpeedSetting || speed != m_rLastSpeed)
	{
		// Throttle bug.
		QImage bmbug(":data/pc_ThrottleBug.png");
		//QImage bmmabug(":data/pc_MaxAirBug.bmp");
		//if(!bmbug.isNull() && !bmmabug.isNull())
		if(!bmbug.isNull())
		{
			// Throttle
			if(m_Throttle != m_lLastSpeedSetting || speed != m_rLastSpeed)
			{
				m_lLastSpeedSetting = m_Throttle;
				m_rLastSpeed = speed;

				QPixmap bmt;
				//if (m_Throttle >= 0)
					bmt.load(":data/pc_Throttle.bmp");	// Forward throttle or 0
				//else
					//bmt.load(":data/pc_Throttle_Rev.bmp");	// Reverse throttle.
				if(!bmt.isNull())
				{
					QPainter p(&bmt);

					// Indicate max in-atmosphere speed.
					/*
					int maxairtick;
					if(ship.DesignIsStreamlined())
						maxairtick = 9;
					else
						maxairtick = 8;
					int iy;
					iy = 39 - abs(maxairtick) * 2;
					p.drawImage(7,iy, bmmabug);
					*/

					// Indicate current speed setting with the bug.
					int iy = 20 - m_Throttle * 2;
					p.drawImage(1,iy, bmbug);

					// Indicate the current actual speed.
					//qDebug("speed %g", speed);
					if(speed != 0.0)
					{
						//if (speed < 0.0)
							//p.setPen(QPen(QColor(255,255,0), 0.f));
						//else
							p.setPen(QPen(QColor(0,255,0), 0.f));

						// Coarse resolution speed.
						int ispeedtick = (int)ceil(fabs(speed)/10000.0 * 2.0);
						int ilinelen[19] = {105, 53, 27, 14, 7, 4, 2, 1, 1, 1,
							1, 1, 1, 1, 1, 1, 1, 1, 1};
						int iv;
						for(iv=0; iv<19; iv++)
						{
							if(iv < 10 && speed < 0.0)
								continue;

							int iy = 2 + iv*2;
							if (ispeedtick > ilinelen[iv])
								p.drawLine(17,iy, 17+ilinelen[iv]-1, iy);
							else
								p.drawLine(17,iy, 17+ispeedtick-1, iy);
						}

						// Fine resolution speed.
						ispeedtick = (int)ceil(fabs(speed)/10.0 * 2.0);
						int ilinelen2[9] = {103, 51, 26, 13, 7, 3, 2, 1, 1};
						for (iv=0; iv<9; iv++)
						{
							//if(fabs(speed) > DaVessel::MovementSpeed(9-iv))
							if(fabs(speed) > pow((float)2,9-iv)-1.)
								continue;
							int iy = 22 + iv*2;
							if (ispeedtick > ilinelen2[iv])
								p.drawLine(20,iy, 20+ilinelen2[iv]-1, iy);
							else
								p.drawLine(20,iy, 20+ispeedtick-1, iy);
						}
					}

					QImage imt = bmt.toImage();
					pTex->ReplaceSubImage(re, imt, 14,82);
				}
			}
		}
	}

	if((int)altitude != (int)m_rLastAltitude)
	{
		m_rLastAltitude = altitude;
		m_LastInEnemyRange = -1;	// Force update of enemy range indicator because this instrument overlaps it.

		QPixmap bmalt(":data/pc_Altimeter.bmp");
		if (!bmalt.isNull())
		{
			// Draw hundreds, thousands, and ten-thousands indicator needles.
			QPainter p(&bmalt);
			QColor colPen1, colPen2, colPen3, colPen4, colPen5;
			if (altitude < 0.0)
			{
				colPen1 = QColor(0,255,255);
				colPen2 = QColor(0,205,205);
				colPen3 = QColor(0,155,155);
				colPen4 = QColor(0,105,105);
				colPen5 = QColor(0,55,55);
				altitude = -altitude;
			}
			else
			{
				colPen1 = QColor(255,255,255);
				colPen2 = QColor(205,205,205);
				colPen3 = QColor(155,155,155);
				colPen4 = QColor(105,105,105);
				colPen5 = QColor(55,55,55);
			}

			// Convert from feet to meters
			altitude /= FeetPerMeter();
			double r1M = altitude/1000000.0;
			int i1M = (int)r1M;				// Whole millions.
			double r100k = (altitude - (i1M * 1000000))/100000.0;
			int i100k = (int)r100k;			// Whole hundred-thousands.
			double r10k = (altitude - (i1M * 1000000 + i100k * 100000))/10000.0;
			int i10k = (int)r10k;			// Whole ten-thousands.
			double r1k = (altitude - (i1M * 1000000 + i100k * 100000 + i10k * 10000))/1000.0;
			int i1k = (int)r1k;
			double rhund = (altitude - (i1M * 1000000 + i100k * 100000 + i10k * 10000 + i1k * 1000))/100.0;

			// Millions
			p.setPen(QPen(colPen5, 5));
			double rang = (90.0 - r1M/10.0 * 360.0) * DEG2RAD;
			int ix = (int)(cos(rang) * 10.0);
			int iy = (int)(sin(rang) * 10.0);
			p.drawLine(38+ix,38-iy, 38,38);

			// Hundred thousands
			p.setPen(QPen(colPen4, 5));
			rang = (90.0 - r100k/10.0 * 360.0) * DEG2RAD;
			ix = (int)(cos(rang) * 10.0);
			iy = (int)(sin(rang) * 10.0);
			p.drawLine(38+ix,38-iy, 38,38);

			// Ten thousands
			p.setPen(QPen(colPen3, 3));
			rang = (90.0 - r10k/10.0 * 360.0) * DEG2RAD;
			ix = (int)(cos(rang) * 15.0);
			iy = (int)(sin(rang) * 15.0);
			p.drawLine(38+ix,38-iy, 38,38);

			// Thousands
			p.setPen(QPen(colPen2, 2));
			rang = (90.0 - r1k/10.0 * 360.0) * DEG2RAD;
			ix = (int)(cos(rang) * 25.0);
			iy = (int)(sin(rang) * 25.0);
			p.drawLine(38+ix,38-iy, 38,38);

			// Hundreds
			p.setPen(QPen(colPen1, 1));
			rang = (90.0 - rhund/10.0 * 360.0) * DEG2RAD;
			ix = (int)(cos(rang) * 35.0);
			iy = (int)(sin(rang) * 35.0);
			p.drawLine(38+ix,38-iy, 38,38);

			QImage imalt = bmalt.toImage();
			pTex->ReplaceSubImage(re, imalt, 7,5);
		}
	}

	// Calculate pitch, roll, and heading.
	// Attitude Indicator
	if(fabs(pitch-m_rLastPitch)*RAD2DEG > 1.0 || fabs(roll-m_rLastRoll)*RAD2DEG > 1.0)
	{
		m_rLastPitch = pitch;
		m_rLastRoll = roll;
		m_LastInEnemyRange = -1;	// Force update of enemy range indicator because this instrument overlaps it.
		m_LastMode = -1;			// Force update of mode indicator because this instrument overlaps it.

		QPixmap bmatt(":data/pc_AttitudeIndBase.bmp");
		if (!bmatt.isNull())
		{
			QPainter p(&bmatt);

			// Draw the attitude indicator.
			const double rc = cos(roll);
			const double rs = sin(roll);
			const double rps = sin(pitch);

			int ip = (int)(-rps * 38);	// Vertical offset along pitch line.
			int icx = 38 + (int)(-rs * ip);
			int icy = 38 + (int)(rc * ip);

			if(modeActual == 1)
			{
				// Near planet.
				p.setPen(QPen(QColor(0,0,0), 0.f));
				p.setBrush(QColor(100,200,200));
			}
			else if(modeActual == 2)
			{
				// Referencing system plane.
				p.fillRect(0,0,77,77, QColor(50,25,0));
				p.setPen(QPen(QColor(0,200,200), 0.f));
				p.setBrush(QColor(10,10,50));
			}
			else if(modeActual == 4)
			{
				// Near star.
				p.setPen(QPen(QColor(0,0,0), 0.f));
				p.setBrush(QColor(100,200,200));
			}
			else
			{
				// Referencing galactic plane.
				p.fillRect(0,0,77,77, QColor(20,0,0));
				p.setPen(QPen(QColor(108,0,200), 0.f));
				p.setBrush(QColor(0,0,20));
			}

			// Fill the sky portion.
			int irollx = (int)(rc * 150);
			int irolly = (int)(rs * 150);
			QVector<QPoint> pxfill;
			pxfill.resize(4);
			pxfill[0] = QPoint(icx-irollx, icy-irolly);
			pxfill[1] = QPoint(icx+irollx, icy+irolly);
			pxfill[2] = QPoint(icx+irolly, icy-irollx);
			pxfill[3] = pxfill[0];
			p.drawPolygon(pxfill);
			p.drawPolyline(pxfill);

			// Add some hash marks.
			if(modeActual == 1)
				p.setPen(QPen(QColor(192,192,192), 0.f));
			else if(modeActual == 2)
				p.setPen(QPen(QColor(0,0,0), 0.f));
			else if(modeActual == 4)
				p.setPen(QPen(QColor(192,192,192), 0.f));
			else
				p.setPen(QPen(QColor(0,0,100), 0.f));
			int ihx = irolly/10;
			int ihy = irollx/10;
			for (int ia=10; ia<=55; ia+=10)
			{
				if (ia == 40)
					ia = 45;
				else if (ia == 55)
					ia = 90;
				double rh = sin(ia*DEG2RAD) * 38;
				int ichx = (int)(rs * rh);
				int ichy = (int)(rc * rh);
				p.drawLine(icx-ichx +ihy, icy+ichy +ihx, icx-ichx -ihy, icy+ichy -ihx);
				p.drawLine(icx+ichx -ihy, icy-ichy -ihx, icx+ichx +ihy, icy-ichy +ihx);
			}

			QPixmap bmattface(":data/pc_AttitudeInd.png");
			p.drawPixmap(0,0,bmattface);

			QImage imatt = bmatt.toImage();
			pTex->ReplaceSubImage(re, imatt, 89,5);
		}
	}

	// Compass
	if(fabs(heading-m_rLastHeading)*RAD2DEG > 1.0)
	{
		m_rLastHeading = heading;
		m_LastMode = -1;			// Force update of mode indicator because this instrument overlaps it.

		QPixmap bmcomp(":data/pc_Compass.bmp");
		if (!bmcomp.isNull())
		{
			QPainter p(&bmcomp);

			// Draw the compass card.
			double rotang = heading;
			QPoint pxtick1, pxtick2;
			for (int itick=0; itick<360; itick+=10)
			{
				double rc = cos(rotang);
				double rs = sin(rotang);
				pxtick1.setX(38 + (int)(rc * 38));
				pxtick1.setY(38 - (int)(rs * 38));
				if (itick == 90)
				{
					p.setPen(QPen(QColor(255,255,255), 0.f));
					pxtick2.setX(38);
					pxtick2.setY(38);
				}
				else if (itick%90 == 0)
				{
					p.setPen(QPen(QColor(255,255,255), 0.f));
					pxtick2.setX(38 + (int)(rc * 25));
					pxtick2.setY(38 - (int)(rs * 25));
				}
				else if (itick%30 == 0)
				{
					p.setPen(QPen(QColor(255,255,255), 0.f));
					pxtick2.setX(38 + (int)(rc * 34));
					pxtick2.setY(38 - (int)(rs * 34));
				}
				else
				{
					p.setPen(QPen(QColor(128,128,128), 0.f));
					pxtick2.setX(38 + (int)(rc * 35));
					pxtick2.setY(38 - (int)(rs * 35));
				}
				p.drawLine(pxtick1, pxtick2);

				rotang += 10.0*DEG2RAD;
			}

			QImage imcomp = bmcomp.toImage();
			pTex->ReplaceSubImage(re, imcomp, 171,5);
		}
	}

	// Enemy attack-range indicator.
	int isInEnemyRange = ship.WasInEnemyRange(3000)?1:0;
	if(isInEnemyRange != m_LastInEnemyRange)
	{
		m_LastInEnemyRange = isInEnemyRange;
		QImage bm;
		if(isInEnemyRange)
			bm.load(":data/pc_EnemyRange.png");
		else
			bm.load(":data/ac_Blank16x16.bmp");
		pTex->ReplaceSubImage(re, bm, 78,68);
	}

	// Mode indicator
	if(m_Mode != m_LastMode)
	{
		m_LastMode = m_Mode;
		QImage bm;
		switch(m_Mode)
		{
		case 0:
		default:
			bm.load(":data/pc_AutoMode.png");
			break;
		case 1:
			bm.load(":data/pc_PlanetMode.png");
			break;
		case 2:
			bm.load(":data/pc_SystemMode.png");
			break;
		case 3:
			bm.load(":data/pc_SectorMode.png");
			break;
		case 4:
			bm.load(":data/pc_StarMode.png");
			break;
		}
		pTex->ReplaceSubImage(re, bm, 160,68);
	}

	// Lights Indicators
	int headlight = ship.IsHeadlightOn();
	if(headlight != m_lLastHeadlight)
	{
		m_lLastHeadlight = headlight;

		QImage bml;
		if(headlight)
			bml.load(":data/pc_LightOn.bmp");
		else
			bml.load(":data/pc_LightOff.bmp");
		pTex->ReplaceSubImage(re, bml, 146,99);
	}

	int navlight = ship.IsNavLightOn();
	if(navlight != m_lLastNavLight)
	{
		m_lLastNavLight = navlight;

		QImage bml;
		if(navlight)
			bml.load(":data/pc_NavLightOn.bmp");
		else
			bml.load(":data/pc_NavLightOff.bmp");
		pTex->ReplaceSubImage(re, bml, 146,90);
	}

	int beacon = ship.IsRotatingBeaconOn();
	if(beacon != m_lLastBeacon)
	{
		m_lLastBeacon = beacon;

		QImage bml;
		if(beacon)
			bml.load(":data/pc_BeaconOn.bmp");
		else
			bml.load(":data/pc_BeaconOff.bmp");
		pTex->ReplaceSubImage(re, bml, 164,99);
	}

	int strobe = ship.IsStrobeOn();
	if(strobe != m_lLastStrobe)
	{
		m_lLastStrobe = strobe;

		QImage bml;
		if(strobe)
			bml.load(":data/pc_StrobeOn.bmp");
		else
			bml.load(":data/pc_StrobeOff.bmp");
		pTex->ReplaceSubImage(re, bml, 164,90);
	}

	bool anyDoorsOpen = ship.AnyHullDoorOpen();
	if(anyDoorsOpen != m_LastAnyDoorsOpen)
	{
		m_LastAnyDoorsOpen = anyDoorsOpen;

		QImage bml;
		if(anyDoorsOpen)
			bml.load(":data/pc_HullDoorsUnlockedO.png");
		else
			bml.load(":data/pc_HullDoorsUnlocked.png");
		pTex->ReplaceSubImage(re, bml, 182,90);
	}

	if((m_PowerPlantOn?1:0) != m_lLastPowerOn)
	{
		m_lLastPowerOn = m_PowerPlantOn;

		QImage bmbut;
		if (m_PowerPlantOn)
			bmbut.load(":data/pc_PowerPlantOn.bmp");
		else
			bmbut.load(":data/pc_PowerPlantOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 218,90);
	}
	if(m_GravityDriveState != m_LastGravityDriveState)
	{
		m_LastGravityDriveState = m_GravityDriveState;

		QImage bmbut;
		if(m_GravityDriveState == DaVessel::GD_On)
			bmbut.load(":data/pc_ManeuverOn.bmp");
		else if(m_GravityDriveState == DaVessel::GD_Hold)
			bmbut.load(":data/pc_ManeuverPark.bmp");
		else
			bmbut.load(":data/pc_ManeuverOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 236,90);
	}

	// Mooring indicator/switch.
	if((int)ship.IsMoored() != m_LastMooringState)
	{
		m_LastMooringState = ship.IsMoored();

		QImage bmbut;
		if(ship.IsMoored())
			bmbut.load(":data/pc_Moored.bmp");
		else
			bmbut.load(":data/pc_Unmoored.bmp");
		pTex->ReplaceSubImage(re, bmbut, 236,108);
	}

	// Capacitor
	if(m_Capacitor != m_lLastCapacitor ||
			m_CapacitorTrend != m_lLastCapTrend ||
			m_FTLDriveSetting != m_LastFTLDriveSetting)	// power usage indicator
	{
		m_lLastCapacitor = m_Capacitor;
		m_lLastCapTrend = m_CapacitorTrend;

		const int cwid = 57;
		QPixmap pmf(cwid,5);
		if(!pmf.isNull())
		{
			int fullto = (cwid * m_Capacitor + 99) / 100;	// Round up
			int jump = 0;
			if(ship.HasFTLDrive() && ship.CapacitorMax() > 0.)
				jump = (int)ceil(ship.PowerNeededToEngageFTLDrive(m_WhichFTL, m_FTLDriveSetting) * cwid / ship.CapacitorMax());

			QPainter p(&pmf);
			p.fillRect(0,0,cwid,5,Qt::black);
			if(jump > 0)
			{
				// FTL jump usage is nice to see.
				// Jump requirement is a horizontal bar below capacitor
				// level in a different color.
				p.fillRect(0,2,jump,3,QColor(128,0,255));
			}
			if(m_Capacitor > 0)
			{
				// Capacitor has power.
				if(m_CapacitorTrend > 0)
					p.fillRect(0,0,fullto,3,QColor(0,255,0));	// charging
				else if(m_CapacitorTrend < 0)
					p.fillRect(0,0,fullto,3,QColor(255,0,0));	// discharging
				else
					p.fillRect(0,0,fullto,3,QColor(255,202,0));	// neutral
			}
			QImage bmf = pmf.toImage();
			pTex->ReplaceSubImage(re, bmf, 146,110);
		}
	}

	// FTL Ready Indicator
	if(m_WhichFTL != m_LastWhichFTL ||
			m_FTLDriveCommit != m_LastFTLDriveCommit ||
			(m_FTLDriveOn?1:0) != m_LastFTLDriveOn ||
			m_FTLDriveSetting != m_LastFTLDriveSetting)
	{
		m_LastFTLDriveCommit = m_FTLDriveCommit;
		m_LastFTLDriveOn = m_FTLDriveOn;
		m_LastFTLDriveSetting = m_FTLDriveSetting;
		m_LastWhichFTL = m_WhichFTL;

		bool hasFTL = true;
		QImage bml;
		if(m_FTLDriveCommit == FTL_COMMIT_ChargedBegin)
		{
			// FTL drive is hot.
			if(DbCommodity::IsWormholeFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)))
			{
				if(m_FTLDriveSetting == 0)
					bml.load(":data/pc_WormReady.bmp");
				else if(m_FTLDriveSetting == 1)
					bml.load(":data/pc_WormNReady.bmp");
				else
					bml.load(":data/pc_WormIReady.bmp");
			}
			else if(DbCommodity::IsWarpFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)) && m_FTLDriveSetting < 9)
			{
				// Warp factor shown on button.
				const char* ftlSettings[] =
				{
					":data/pc_Warp1.png",
					":data/pc_Warp2.png",
					":data/pc_Warp3.png",
					":data/pc_Warp4.png",
					":data/pc_Warp5.png",
					":data/pc_Warp6.png",
					":data/pc_Warp7.png",
					":data/pc_Warp8.png",
					":data/pc_Warp9.png",
				};
				bml.load(ftlSettings[m_FTLDriveSetting]);
			}
			else
				bml.load(":data/ac_Blank16x16.bmp");
		}
		else if(m_FTLDriveCommit == FTL_COMMIT_Precharge1)
		{
			// FTL drive is charging.
			if(DbCommodity::IsWormholeFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)))
			{
				if(m_FTLDriveSetting == 0)
					bml.load(":data/pc_WormDormant.bmp");
				else if(m_FTLDriveSetting == 1)
					bml.load(":data/pc_WormNDormant.bmp");
				else
					bml.load(":data/pc_WormIDormant.bmp");
			}
			else if(DbCommodity::IsWarpFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)) && m_FTLDriveSetting < 9)
			{
				// Warp factor shown on button.
				const char* ftlSettings[] =
				{
					":data/ec_Warp1.png",
					":data/ec_Warp2.png",
					":data/ec_Warp3.png",
					":data/ec_Warp4.png",
					":data/ec_Warp5.png",
					":data/ec_Warp6.png",
					":data/ec_Warp7.png",
					":data/ec_Warp8.png",
					":data/ec_Warp9.png",
				};
				bml.load(ftlSettings[m_FTLDriveSetting]);
			}
			else
				bml.load(":data/ac_Blank16x16.bmp");

			// Overlay the warming indicator. I don't feel like making a bunch of new bitmaps.
			QPainter p(&bml);
			QImage warming(":data/pc_FTLCharging.png");
			p.drawImage(bml.rect(), warming);
		}
		else if(m_FTLDriveCommit == FTL_COMMIT_Discharge1)
		{
			// FTL drive is discharging.
			if(DbCommodity::IsWormholeFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)))
			{
				if(m_FTLDriveSetting == 0)
					bml.load(":data/pc_WormDormant.bmp");
				else if(m_FTLDriveSetting == 1)
					bml.load(":data/pc_WormNDormant.bmp");
				else
					bml.load(":data/pc_WormIDormant.bmp");
			}
			else if(DbCommodity::IsWarpFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)) && m_FTLDriveSetting < 9)
			{
				// Warp factor shown on button.
				const char* ftlSettings[] =
				{
					":data/ec_Warp1.png",
					":data/ec_Warp2.png",
					":data/ec_Warp3.png",
					":data/ec_Warp4.png",
					":data/ec_Warp5.png",
					":data/ec_Warp6.png",
					":data/ec_Warp7.png",
					":data/ec_Warp8.png",
					":data/ec_Warp9.png",
				};
				bml.load(ftlSettings[m_FTLDriveSetting]);
			}
			else
				bml.load(":data/ac_Blank16x16.bmp");

			// Overlay the warming indicator. I don't feel like making a bunch of new bitmaps.
			QPainter p(&bml);
			QImage warming(":data/pc_FTLDischarging.png");
			p.drawImage(bml.rect(), warming);
		}
		else
		{
			// FTL drive is cold.
			if(DbCommodity::IsWormholeFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)))
			{
				if(m_FTLDriveSetting == 0)
					bml.load(":data/pc_WormDormant.bmp");
				else if(m_FTLDriveSetting == 1)
					bml.load(":data/pc_WormNDormant.bmp");
				else
					bml.load(":data/pc_WormIDormant.bmp");
			}
			else if(DbCommodity::IsWarpFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)) && m_FTLDriveSetting < 9)
			{
				// Warp factor shown on button.
				const char* ftlSettings[] =
				{
					":data/ec_Warp1.png",
					":data/ec_Warp2.png",
					":data/ec_Warp3.png",
					":data/ec_Warp4.png",
					":data/ec_Warp5.png",
					":data/ec_Warp6.png",
					":data/ec_Warp7.png",
					":data/ec_Warp8.png",
					":data/ec_Warp9.png",
				};
				bml.load(ftlSettings[m_FTLDriveSetting]);
			}
			else
			{
				bml.load(":data/ac_Blank16x16.bmp");	// No FTL drive.
				hasFTL = false;
			}
		}
		pTex->ReplaceSubImage(re, bml, 211,108);

		if(hasFTL)
		{
			// Up and down arrows for adjusting the FTL drive setting.
			{
				if(m_FTLDriveOn && m_FTLDriveCommit == FTL_COMMIT_Dormant)
					bml.load(":data/pc_FTLArrows.png");
				else
					bml.load(":data/pc_FTLArrowsDim.png");
				pTex->ReplaceSubImage(re, bml, 227,108);
			}

			// Selector indicates power on/off state of current drive.
			{
				if(m_FTLDriveOn)
					bml.load(":data/pc_FTLSelect.png");
				else
					bml.load(":data/pc_FTLSelectOff.png");
				pTex->ReplaceSubImage(re, bml, 204,111);
			}
		}
		else
		{
			// Blank out FTL button areas.
			bml.load(":data/pc_FTLArrowsBlack.png");
			pTex->ReplaceSubImage(re, bml, 227,108);

			bml.load(":data/pc_FTLSelectBlack.png");
			pTex->ReplaceSubImage(re, bml, 204,111);
		}
	}

	// Fuel.
	if(m_FuelAboard != m_lLastFuel)
	{
		m_lLastFuel = m_FuelAboard;

		const int fgwid = 56;
		QPixmap pmf(fgwid,3);
		if(!pmf.isNull())
		{
			int fullto = (fgwid * m_FuelAboard + 99) / 100;	// Round up
			QPainter p(&pmf);
			p.fillRect(0,0,fgwid,3,Qt::black);
			if(fullto > 16)			// in the green zone
				p.fillRect(0,0,fullto,3,QColor(84,209,255));
			else if(fullto > 6)		// in the yellow zone
				p.fillRect(0,0,fullto,3,QColor(255,245,84));
			else if(fullto > 0)		// in the red zone
				p.fillRect(0,0,fullto,3,QColor(255,72,72));
			QImage bmf = pmf.toImage();
			pTex->ReplaceSubImage(re, bmf, 146,120);
		}
	}

	if(ship.HasRetractableLandingGear())
	{
		int gearDown = ship.IsLandingGearExtended();
		QImage bml;
		if(gearDown)
			bml.load(":data/pc_GearDown.png");
		else
			bml.load(":data/pc_GearUp.png");
		pTex->ReplaceSubImage(re, bml, 236,72);
	}
	else
	{
		QImage bml(":data/ac_Blank16x16.bmp");
		pTex->ReplaceSubImage(re, bml, 236,72);
	}

	if(ship.IsInAtmosphere())
		m_LastStress = ship.HullStress();
	else
		m_LastStress = -1;
}

void DpStationHelm::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_PilotConsoleControls.begin(); myci != s_PilotConsoleControls.end(); myci++)
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

void DpStationHelm::RestoreDefaultControls()
{
	s_PilotConsoleControls.clear();
	for(int ic=0; ic<s_NumPilotConsoleDefaults; ic++)
		s_PilotConsoleControls.append(s_PilotConsoleDefaults[ic]);
	SaveControls();
}

void DpStationHelm::LoadControls() const
{
	// Init with my default controls.
	s_PilotConsoleControls.clear();
	for(int ic=0; ic<s_NumPilotConsoleDefaults; ic++)
		s_PilotConsoleControls.append(s_PilotConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_PilotConsoleControls.LoadSettings("Controls/Pilot Console");
}

void DpStationHelm::SaveControls() const
{
	// Store my controls in the settings file.
	s_PilotConsoleControls.SaveSettings("Controls/Pilot Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationHelm::EnumControlInputs(DiControlInputArray &aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_PilotConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_PilotConsoleControls.cbegin(); ci != s_PilotConsoleControls.cend(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be // passed.
// Only control inputs that have been activated will be passed.
void DpStationHelm::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	if(!ship.IsNormal())
		return;

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_CloseHullDoors:
			if(cc != CONTROL_Client)
				ship.CloseAllHullDoors();
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_CloseDoors, 1, 1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ModeCycle:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Server)
			{
				switch(m_Mode)
				{
				case 0:											// Auto mode
					m_Mode = 1;									// Planet mode
					break;
				case 1:											// Planet mode
					m_Mode = 4;									// Star mode
					break;
				case 4:											// Star mode
					m_Mode = 2;									// System mode
					break;
				case 2:											// System mode
					m_Mode = 3;									// Sector mode
					break;
				default:
				case 3:											// Sector mode
					m_Mode = 0;									// Auto mode
					break;
				}
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Mode, 1, .8f + m_Mode/20.f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_DoorRemote:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Client)
			{
				if(ship.IsInsideSpacecraft())
				{
					// This spacecraft is inside a spacecraft.
					// Hull doors are not locked to people inside the hull.
					DaCapsule* inship = dynamic_cast<DaCapsule*>(ship.Parent());
					if(!inship)
						break;

					// Toggle the state of the large hull doors in the room I'm in that is nearest to me.
					unsigned int doorIndex = inship->RemoteDoorIndex(false, ship.Location(), ship.ShipRoomId());
					inship->ToggleHullDoor(doorIndex);
				}
				else
				{
					// This vehicle is not inside a spacecraft.
					DaAtom* system = ship.FindEnvironmentAtom();
					if(!system)
						break;	// Not likely.

					// Determine my location relative to the solar system.
					AuPoint mysysloc = ship.EnvLocation();
					DaAtomList ships;
					system->FindCapsulesRecursive(ships);
					if(ships.isEmpty())
						break;	// No ships in the system.

					// Find the closest accessible ship to this vehicle.
					DaCapsule* closeship = 0;
					double closedist = 0;
					AuMatrix4x4 tcloseship;
					for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
					{
						// Skip myself.
						if((*si)->Is(ship.Id()))
							continue;
						DaCapsule* v = (DaCapsule*)(*si);
						if(!v->CheckAccess(sceneAtom))
							continue;
						const AuMatrix4x4& tship = (*si)->EnvTransform();
						AuPoint shipsysloc = tship.Project(AuPoint(0,0));
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
						break;	// No non-enemy ship found.
					if(closedist > 1500)
						break;	// Too far away.

					// Convert my system location to be relative to the spacecraft.
					AuPoint myshiploc = tcloseship.UnProject(mysysloc);

					// Locked hull doors can only be operated by members of the crew. That is officers, crew, or troops.
					unsigned int doorIndex = closeship->RemoteDoorIndex(false, myshiploc);
					bool hullDoorLocked = closeship->HullDoorLocked(doorIndex);
					if(hullDoorLocked && !closeship->IsCrewOrTroop(sceneAtom->Id()) && !closeship->InChainOfCommand(sceneAtom))
						break;	// The door is locked to the person.

					// Tell the ship to open the bay door closest to my ship location.
					closeship->ToggleHullDoor(doorIndex);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_CloseDoors, 1, .9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_MooringsOnOff:
			// This is the padlock button when not logged in.
			if(cc != CONTROL_Client)
			{
				// Mooring controls ok on nu ship. Will unset nu state of ship when unmoored.
				if(ship.IsMoored())
				{
					// A ship cannot be unmoored while it has repairs remaining.
					if(!ship.RepairSecondsRemaining())
					{
						ship.Unmoor();
						ship.SetModified();
					}
				}
				else
				{
					ship.Moor();
					ship.SetModified();
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Moor, 1, ship.IsMoored()?1:.90f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_IncreaseThrottle:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.IncreaseThrottle();
			ship.SetModified();
			break;
		case CI_DecreaseThrottle:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.DecreaseThrottle();
			ship.SetModified();
			break;
		case CI_IncreaseThrottleL:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.IncreaseThrustLateral();
			ship.SetModified();
			break;
		case CI_DecreaseThrottleL:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.DecreaseThrustLateral();
			ship.SetModified();
			break;
		case CI_IncreaseThrottleV:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.IncreaseThrustVertical();
			ship.SetModified();
			break;
		case CI_DecreaseThrottleV:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.DecreaseThrustVertical();
			ship.SetModified();
			break;
		case CI_ThrustF:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.IncreaseThrustLongitudinal();
			ship.SetModified();
			break;
		case CI_ThrustB:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.DecreaseThrustLongitudinal();
			ship.SetModified();
			break;

		case CI_HeadlightOnOff:
			// This control cannot be activated when the ship is "nu".
			if(ship.IsNew())
				break;
			if(cc != CONTROL_Client)
			{
				ship.HeadlightToggle(!ship.IsHeadlightOn());
				ship.SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Headlight, 1,
						ship.IsHeadlightOn()?.90f:1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_BeaconsOnOff:
			// This control cannot be activated when the ship is "nu".
			if(ship.IsNew())
				break;
			if(cc != CONTROL_Client)
			{
				ship.RotatingBeaconToggle(!ship.IsRotatingBeaconOn());
				ship.SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Beacons, 1,
						ship.IsRotatingBeaconOn()?.90f:1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_NavLightsOnOff:
			// This control cannot be activated when the ship is "nu".
			if(ship.IsNew())
				break;
			if(cc != CONTROL_Client)
			{
				ship.NavLightToggle(!ship.IsNavLightOn());
				ship.SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_NavLights, 1,
						ship.IsNavLightOn()?.90f:1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_StrobesOnOff:
			// This control cannot be activated when the ship is "nu".
			if(ship.IsNew())
				break;
			if(cc != CONTROL_Client)
			{
				ship.StrobeToggle(!ship.IsStrobeOn());
				ship.SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Strobes, 1,
						ship.IsStrobeOn()?.90f:1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_GearUpDown:
			// This control cannot be activated when the ship is "nu".
			if(ship.IsNew() || !ship.HasRetractableLandingGear())
				break;
			if(cc != CONTROL_Client)
			{
				ship.LandingGearToggle(!ship.IsLandingGearExtended());
				ship.SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_LandingGear, 1,
						ship.IsLandingGearExtended()?.90f:1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_FTLSelect:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(!ship.HasWarpFTLDrive() || !ship.HasWormholeFTLDrive())
				break;
			if(cc != CONTROL_Client)
			{
				quint16 warpDrive = ship.FTLWarpDrive();
				if(m_WhichFTL == warpDrive)
					m_WhichFTL = ship.FTLWormholeDrive();
				else
					m_WhichFTL = warpDrive;
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLSelect, 1, 1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_FTLEngage:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Client)
			{
				if(ship.CanEngageFTLNow(m_WhichFTL))
					ship.EngageFTLDrive(m_WhichFTL);
				else if(ship.CanDisengageFTLNow(m_WhichFTL))
					ship.DischargeFTLDrive(m_WhichFTL);
			}
			if(cc != CONTROL_Server)
			{
				if(ship.IsFTLDriveHot(m_WhichFTL))
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLEngage, 1, 1);
					sceneAtom->AddEffect(buttonClick);
				}
				else
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLEngage, 1, .75f);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;

		case CI_FTLSettingUp:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Client)
			{
				// Rolls over to 0 when limit exceeded based on module.
				ship.SetFTLDriveSetting(m_WhichFTL, ship.FTLDriveSetting(m_WhichFTL)+1);
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLSetting, 1, .9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_FTLSettingDown:
			// This control cannot be activated when the ship is "nu".
			if(ship.IsNew())
				break;
			if(cc != CONTROL_Client)
			{
				if(ship.FTLDriveSetting(m_WhichFTL))
					ship.SetFTLDriveSetting(m_WhichFTL, ship.FTLDriveSetting(m_WhichFTL)-1);	// Reduce setting from non-zero.
				else
				{
					// Reduce setting at 0; wrap to the highest possible setting.
					COMMODITYID ftlDriveMod = ship.SystemModule(SYS_FTLDrive, m_WhichFTL);
					switch(ftlDriveMod)
					{
					case CID_Warp1FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 0);
						break;
					case CID_Warp2FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 1);
						break;
					case CID_Warp3FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 2);
						break;
					case CID_Warp4FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 3);
						break;
					case CID_Warp5FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 4);
						break;
					case CID_Warp6FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 5);
						break;
					case CID_Warp7FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 6);
						break;
					case CID_Warp8FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 7);
						break;
					case CID_Warp9FTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 8);
						break;
					case CID_WormholePFTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 0);
						break;
					case CID_WormholePNFTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 1);
						break;
					case CID_WormholePNNFTLDriveModule:
						ship.SetFTLDriveSetting(m_WhichFTL, 2);
						break;
					default:
						break;
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLSetting, 1, .8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_PowerPlantOnOff:
			if(cc != CONTROL_Client)
			{
				if(ship.IsPowerPlantOn())
					ship.TurnPowerPlantOff();
				else
					ship.TurnPowerPlantOn();
			}
			if(cc != CONTROL_Server)
			{
				if(ship.IsPowerPlantOn())
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_PowerPlant, 1, .75);
					sceneAtom->AddEffect(buttonClick);
				}
				else
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_PowerPlant, 1, 1);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_GravityDriveOnOff:
			if(ship.IsNew() && !ship.IsPowerPlantOn())
				break;	// This control cannot be activated when the ship is "nu" and the power plant is off.
			// Gravity drive state cannot be changed on a station.
			if(cc != CONTROL_Client)
			{
				switch(ship.GravityDriveState())
				{
				case DaVessel::GD_Off:
				case DaVessel::GD_Invalid:
					ship.SetGravityDriveState(DaVessel::GD_On);
					break;
				case DaVessel::GD_On:
				case DaVessel::GD_Hold:
					ship.SetGravityDriveState(DaVessel::GD_Off);
					break;
				}
			}
			if(cc != CONTROL_Server)
			{
				switch(ship.GravityDriveState())
				{
				case DaVessel::GD_Off:
				case DaVessel::GD_Invalid:
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravOnOff, 1, 1);
						sceneAtom->AddEffect(buttonClick);
					}
					break;
				case DaVessel::GD_On:
				case DaVessel::GD_Hold:
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravOnOff, 1, .75f);
						sceneAtom->AddEffect(buttonClick);
					}
					break;
				}
			}
			break;
		case CI_GravityDriveOnOffHold:
			if(ship.IsNew() && !ship.IsPowerPlantOn())
				break;	// This control cannot be activated when the ship is "nu" and the power plant is off.
			if(cc != CONTROL_Client)
			{
				switch(ship.GravityDriveState())
				{
				case DaVessel::GD_Off:
				case DaVessel::GD_Invalid:
					ship.SetGravityDriveState(DaVessel::GD_On);
					break;
				case DaVessel::GD_On:
					if(ship.IsSpaceStation())
						ship.SetGravityDriveState(DaVessel::GD_Off);
					else
						ship.SetGravityDriveState(DaVessel::GD_Hold);
					break;
				case DaVessel::GD_Hold:
					ship.SetGravityDriveState(DaVessel::GD_Off);
					break;
				}
			}
			if(cc != CONTROL_Server)
			{
				switch(ship.GravityDriveState())
				{
				case DaVessel::GD_Off:
				case DaVessel::GD_Invalid:
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravOnOff, 1, 1);
						sceneAtom->AddEffect(buttonClick);
					}
					break;
				case DaVessel::GD_On:
					if(ship.IsSpaceStation())
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravOnOff, 1, .75);
						sceneAtom->AddEffect(buttonClick);
					}
					else
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravHold, 1, 1);
						sceneAtom->AddEffect(buttonClick);
					}
					break;
				case DaVessel::GD_Hold:
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravOnOff, 1, .75);
						sceneAtom->AddEffect(buttonClick);
					}
					break;
				}
			}
			break;
		case CI_GravityDriveHold:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Client)
			{
				switch(ship.GravityDriveState())
				{
				case DaVessel::GD_Off:
				case DaVessel::GD_Invalid:
					break;
				case DaVessel::GD_On:
					if(!ship.IsSpaceStation())
						ship.SetGravityDriveState(DaVessel::GD_Hold);
					break;
				case DaVessel::GD_Hold:
					ship.SetGravityDriveState(DaVessel::GD_On);
					break;
				}
			}
			if(cc != CONTROL_Server)
			{
				switch(ship.GravityDriveState())
				{
				case DaVessel::GD_Off:
				case DaVessel::GD_Invalid:
					break;
				case DaVessel::GD_On:
					if(!ship.IsSpaceStation())
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravHold, 1, 1);
						sceneAtom->AddEffect(buttonClick);
					}
					break;
				case DaVessel::GD_Hold:
					{
						AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_GravOnOff, 1, 1);
						sceneAtom->AddEffect(buttonClick);
					}
					break;
				}
			}
			break;

		case CI_PitchUp:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			ship.PitchUp((unsigned char)ci.GetRangeValue());
			break;
		case CI_PitchDownTurnLeft:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			if(!ship.IsOnGround())
				ship.PitchDownTurnLeft((unsigned char)ci.GetRangeValue());
			break;
		case CI_PitchDownTurnRight:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			if(!ship.IsOnGround())
				ship.PitchDownTurnRight((unsigned char)ci.GetRangeValue());
			break;
		case CI_PitchUpTurnLeft:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			if(!ship.IsOnGround())
				ship.PitchUpTurnLeft((unsigned char)ci.GetRangeValue());
			break;
		case CI_PitchUpTurnRight:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			if(!ship.IsOnGround())
				ship.PitchUpTurnRight((unsigned char)ci.GetRangeValue());
			break;
		case CI_PitchDown:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			ship.PitchDown((unsigned char)ci.GetRangeValue());
			break;
		case CI_RollRight:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			ship.RollRight((unsigned char)ci.GetRangeValue());
			break;
		case CI_RollLeft:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			ship.RollLeft((unsigned char)ci.GetRangeValue());
			break;
		case CI_TurnLeft:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			ship.TurnLeft((unsigned char)ci.GetRangeValue());
			break;
		case CI_TurnRight:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc == CONTROL_Server)
				break;
			ship.TurnRight((unsigned char)ci.GetRangeValue());
			break;

		case CI_AllStop:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.SetThrottle(0);
			ship.SetThrustLateral(0);
			ship.SetThrustLongitudinal(0);
			ship.SetThrustVertical(0);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Stop, 1, 1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		// Throttle settings.
		case CI_SetThrottle:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != ci.GetRangeValue())
				ship.SetThrottle(ci.GetRangeValue());
			break;
		case CI_Throttle0:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 0)
				ship.SetThrottle(0);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle1:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 1)
				ship.SetThrottle(1);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 1/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle2:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 2)
				ship.SetThrottle(2);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 2/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle3:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 3)
				ship.SetThrottle(3);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 3/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle4:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 4)
				ship.SetThrottle(4);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 4/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle5:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 5)
				ship.SetThrottle(5);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 5/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle6:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 6)
				ship.SetThrottle(6);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 6/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle7:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 7)
				ship.SetThrottle(7);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 7/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle8:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 8)
				ship.SetThrottle(8);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 8/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle9:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 9)
				ship.SetThrottle(9);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 9/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Throttle10:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != 10)
				ship.SetThrottle(10);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 10/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg1:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -1)
				ship.SetThrottle(-1);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 1/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg2:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -2)
				ship.SetThrottle(-2);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 2/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg3:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -3)
				ship.SetThrottle(-3);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 3/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg4:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -4)
				ship.SetThrottle(-4);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 4/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg5:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -5)
				ship.SetThrottle(-5);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 5/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg6:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -6)
				ship.SetThrottle(-6);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 6/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg7:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -7)
				ship.SetThrottle(-7);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 7/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg8:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -8)
				ship.SetThrottle(-8);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 8/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg9:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -9)
				ship.SetThrottle(-9);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 9/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ThrottleNeg10:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(ship.Throttle() != -10)
				ship.SetThrottle(-10);
			if(cc != CONTROL_Server)
			{
				float buttonPitch = .5f + 10/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Throttle, 1, buttonPitch);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Brakes:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.Brake();
			break;
		case CI_BrakesLV:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.BrakeLV();
			break;

		case CI_LookBack:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI);							// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookForward:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				//t.RotateZ(PI);						// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookLeft:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI/2);						// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookLeftBack:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI*3/4);						// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookLeftForward:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(PI/4);						// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookRight:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(-PI/2);						// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookRightBack:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(-PI*3/4);						// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		case CI_LookRightForward:
			if(cc != CONTROL_Server)
			{
				AuMatrix4x4 t;
				t.Translate(sceneAtom->Location());	// Keep them in the same place relative to the vehicle.
				t.RotateZ(-PI/4);						// Set facing angle. 0 is straight ahead.
				sceneAtom->ChangePosAndOrientNotParent(AuAtomPos(ship.Id(), t));
				sceneAtom->TiltHeadTo(0);
			}
			break;
		}
	}
	//qDebug("DpStationHelm::ProcessControlInput exit");
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DpStationHelm::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
		bool bContinuous)
{
	if(idPanel)
		return;
	double panWid, panHt;
	PanelDimensions(panWid, panHt);
	int ix = (int)(fPanelLoc.x * 256 / panWid);	// X pixel in texture map.
	int iy = (int)(fPanelLoc.y * 128 / panHt);
	QPoint pxHit(ix, iy);

	// Note that the top/bottom is reversed in the rectangles. This works with
	// the iy we've calculated that is also measured from the bottom, not the
	// top as usual in GDI.
	DiPanelControl acontrols[] =
	{
		DiPanelControl(CI_HeadlightOnOff, QRect(146,99,16,7)),
		DiPanelControl(CI_NavLightsOnOff, QRect(146,90,16,7)),
		DiPanelControl(CI_BeaconsOnOff, QRect(164,99,16,7)),
		DiPanelControl(CI_StrobesOnOff, QRect(164,90,16,7)),
		DiPanelControl(CI_GearUpDown, QRect(236,72,16,16)),
		DiPanelControl(CI_PowerPlantOnOff, QRect(218,90,16,16)),
		DiPanelControl(CI_GravityDriveOnOffHold, QRect(236,90,16,16)),
		DiPanelControl(CI_MooringsOnOff, QRect(236,108,16,16)),
		DiPanelControl(CI_AllStop, QRect(4,99,8,8)),
		DiPanelControl(CI_Throttle0, QRect(14,100,10,3)),
		DiPanelControl(CI_Throttle1, QRect(14,103,10,2)),
		DiPanelControl(CI_Throttle2, QRect(14,105,10,2)),
		DiPanelControl(CI_Throttle3, QRect(14,107,10,2)),
		DiPanelControl(CI_Throttle4, QRect(14,109,10,2)),
		DiPanelControl(CI_Throttle5, QRect(14,111,10,2)),
		DiPanelControl(CI_Throttle6, QRect(14,113,10,2)),
		DiPanelControl(CI_Throttle7, QRect(14,115,10,2)),
		DiPanelControl(CI_Throttle8, QRect(14,117,10,2)),
		DiPanelControl(CI_Throttle9, QRect(14,119,10,2)),
		DiPanelControl(CI_Throttle10, QRect(14,121,10,2)),
		DiPanelControl(CI_ThrottleNeg1, QRect(14,98,10,2)),
		DiPanelControl(CI_ThrottleNeg2, QRect(14,96,10,2)),
		DiPanelControl(CI_ThrottleNeg3, QRect(14,94,10,2)),
		DiPanelControl(CI_ThrottleNeg4, QRect(14,92,10,2)),
		DiPanelControl(CI_ThrottleNeg5, QRect(14,90,10,2)),
		DiPanelControl(CI_ThrottleNeg6, QRect(14,88,10,2)),
		DiPanelControl(CI_ThrottleNeg7, QRect(14,86,10,2)),
		DiPanelControl(CI_ThrottleNeg8, QRect(14,84,10,2)),
		DiPanelControl(CI_ThrottleNeg9, QRect(14,82,10,2)),
		DiPanelControl(CI_ThrottleNeg10, QRect(14,80,10,2)),
		DiPanelControl(CI_CloseHullDoors, QRect(182,90,16,16)),
		DiPanelControl(CI_ModeCycle, QRect(160,68,16,16)),
		DiPanelControl(CI_FTLEngage, QRect(211,108,16,16)),
		DiPanelControl(CI_FTLSelect, QRect(204,111,6,10)),
		DiPanelControl(CI_FTLSettingUp, QRect(227,116,8,8)),
		DiPanelControl(CI_FTLSettingDown, QRect(227,108,8,8)),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);
	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

void DpStationHelm::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStation::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
	if(seatedCreature && seatedCreature->Is(sceneAtom.Id()))
	{
		const DaVessel* ship = sceneAtom.FindParentVesselInEnv();
		if(ship)
		{
			if(ship->HasWaypoint())
			{
				// Pilot console only draw the next waypoint.
				DeWaypointRoute route;
				route.append(*ship->LockWaypoint(0));
				ship->UnlockWaypoint();
				RenderCourse(route, farProjection, tModel, sceneAtom, ari, false, true);
			}

			// Draw the lock-on reticle.
			if(!ship->LockedBlip().IsNull())
				RenderSensorLock(ship->LockedBlip(), ship->LockedStance(), ship->LockedIsCivilian(), ship->LockedSysLoc(), tModel,
						sceneAtom, ari);
		}
	}
}

void DpStationHelm::RenderHud(int hudIntensity, AuAtomRenderInfo& ari, int eteSecs) const
{
	if(m_Capacitor == 0)
		return;
	ari.RenderAircraftHud(0,0, m_rLastAltitude, m_rLastPitch, m_rLastRoll, m_rLastHeading, m_rLastSpeed, m_rLastVSpeed,
			m_rLastLSpeed, m_LastLongitude, m_LastLatitude, hudIntensity, m_LastStress, m_Throttle, (float)eteSecs,
			(m_LastInEnemyRange==1)?true:false, m_GravityDriveState == DaVessel::GD_Hold);
}

bool DpStationHelm::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
	if(!ship || !ship->IsDesignDataCacheValid())
		return false;

	// Capture sequence is only run once. Negative value indicates that it has already been run or otherwise processed for the
	// current helm station operator.
	if(m_CaptureSequence >= 0 &&
			person->EmpireId() &&
			!person->m_AgentBits &&
			!last.SameStartSec(now) &&
			!ship->IsNoncombatant())
	{
		// Ships protected from attack cannot be captured.
		// Capture sequence has not been initiated on this helm station.
		// Agents with any powers active cannot capture ships.
		// Check for capture by enemy at helm once per second.
		POLITICAL_STANCE stance = person->PoliticalStanceMakeContact(ship);
		if(stance == POL_Enemy)
		{
			// Person atom is sitting at enemy ship's helm.
			if(m_CaptureSequence == 0)
			{
				// The person at this console initiates the capture sequence.
				m_CaptureSequence = 30;

				// Notify crew and intercom that capture sequence has been initiated.
				const DgData& gd = g_Au.LockDataForRead();
				QString empireName;
				gd.EmpireName(person->EmpireId(), empireName);
				g_Au.UnlockData();
				QString text = QObject::tr("Capture sequence initiated at the helm by %1 of %2. %3 seconds remaining.")
					.arg(person->Name()).arg(empireName).arg(m_CaptureSequence);

				// Send the message on the intercom channel, so the person capturing the ship gets some feedback.
				const DaSector* sector = ship->FindParentSector();
				AuCommChat intMsg(text, AuCommStdChannel::IntercomChannelName(), ship->Id(), ship->Name(),
						sector->Id(), sector->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
				g_Au.SendAtomChat(intMsg);

				// Send the message on the crew channel, so the crew knows they are losing their ship.
				AuCommChat crewMsg(text, AuCommStdChannel::CrewChannelName(), ship->Id(), ship->Name(),
						sector->Id(), sector->Galaxy(), AuCommStdChannel::Universe,
						AuCommStdChannel::CrewMates);
				crewMsg.SetScopeData(ship->Id().GetID());	// Crew channel needs ship's id to know the scope of the message.
				g_Au.SendAtomChat(crewMsg);
			}
			else
			{
				// The enemy person at this console has already initiated the capture sequence.
				m_CaptureSequence--;
				if(ship->IsPrivatelyOwned())
				{
					// A privately owned ship does not change its nationality so it must only announce capture once.
					// Otherwise the person at the helm gets spammed with messages constantly.
					if(m_CaptureSequence == 0)
					{
						// Company ship captured.
						m_CaptureSequence = -1;	// Capture sequence has been run at this console.

						// Player Statistics
						person->Stats().SpacecraftCaptured(ship);
						person->IncreaseSkill(AuSkill::SpacecraftCapture, 1);

						if(!person->InArena())
							person->SpacecraftCaptured(ship, ship->FindParentSolarSystem()->Id());

						// Empire id is not changed for privately owned ship. Consequently, the empire designated reward for
						// capturing enemy vessels is not paid.
						// The roster is not cleared for privately owned vessels so people can still respawn aboard.

						// Loot the ship's fund.
						double acctBalance = ship->AccountBalance();
						if(acctBalance > 0)
						{
							// Transfer all ship's funds to the person capturing the ship.
							person->CreditAccount(acctBalance);	// Credit the person.
							ship->DebitAccount(acctBalance);		// Debit the ship.
						}

						// Initiate self destruct sequence in 3 min.
						ship->MissionClear();	// Clear the previous mission, without notifying the pirate captain, if any.
						ship->MissionAppendOrder(AuMissionOrderScuttle(3 * 60));

						// Notify crew and intercom that capture sequence has been completed.
						const DgData& gd = g_Au.LockDataForRead();
						QString empireName;
						gd.EmpireName(person->EmpireId(), empireName);
						g_Au.UnlockData();
						QString text;
						if(acctBalance > 0)
							text = QObject::tr("Capture sequence completed. %L3%4 transferred to %1 of %2. "
									"Flag of company vessel not changed. Self destruct sequence initiated, 30 seconds.")
								.arg(person->Name()).arg(empireName).arg(acctBalance, 0,'f',0).arg(g_Au.CurrencySymbol());
						else
						{
							text = QObject::tr("Capture sequence completed by %1 of %2. Ship's fund is empty. "
									"Flag of company vessel not changed. Self destruct sequence initiated, 30 seconds.")
								.arg(person->Name()).arg(empireName);
						}

						// Send the message on the intercom channel, so the person capturing the ship gets some feedback.
						const DaSector* sector = ship->FindParentSector();
						AuCommChat intMsg(text, AuCommStdChannel::IntercomChannelName(), ship->Id(), ship->Name(), sector->Id(),
								sector->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
						g_Au.SendAtomChat(intMsg);

						// Send the message on the crew channel, so the crew knows they are losing their ship.
						AuCommChat crewMsg(text, AuCommStdChannel::CrewChannelName(), ship->Id(), ship->Name(), sector->Id(),
								sector->Galaxy(), AuCommStdChannel::Universe, AuCommStdChannel::CrewMates);
						crewMsg.SetScopeData(ship->Id().GetID());	// Crew channel scope needs ship's id.
						g_Au.SendAtomChat(crewMsg);
					}
					else if(m_CaptureSequence <= 5 || m_CaptureSequence%5 == 0)
					{
						// Notify crew and intercom that capture sequence is still in progress.
						const DgData& gd = g_Au.LockDataForRead();
						QString empireName;
						gd.EmpireName(person->EmpireId(), empireName);
						g_Au.UnlockData();
						QString text = QObject::tr("Capture sequence in progress by %1 of %2. %3 seconds remaining.")
							.arg(person->Name()).arg(empireName).arg(m_CaptureSequence);

						// Send the message on the intercom channel, so the person capturing the ship gets some feedback.
						const DaSector* sector = ship->FindParentSector();
						AuCommChat intMsg(text, AuCommStdChannel::IntercomChannelName(), ship->Id(), ship->Name(),
								sector->Id(), sector->Galaxy(), AuCommStdChannel::Close,
								AuCommStdChannel::Intercom);
						g_Au.SendAtomChat(intMsg);

						// Send the message on the crew channel, so the crew knows they are losing their ship.
						AuCommChat crewMsg(text, AuCommStdChannel::CrewChannelName(), ship->Id(), ship->Name(),
								sector->Id(), sector->Galaxy(), AuCommStdChannel::Universe,
								AuCommStdChannel::CrewMates);
						crewMsg.SetScopeData(ship->Id().GetID());	// Crew channel scope needs ship's id.
						g_Au.SendAtomChat(crewMsg);
					}
				}
				else
				{
					if(m_CaptureSequence == 0)
					{
						// Fleet ship captured.
						m_CaptureSequence = -1;	// Capture sequence has been run at this console.

						// Player Statistics
						person->Stats().SpacecraftCaptured(ship);
						person->IncreaseSkill(AuSkill::SpacecraftCapture, 1);

						if(!person->InArena())
							person->SpacecraftCaptured(ship, ship->FindParentSolarSystem()->Id());

						// A subsidized ship changes its nationality.
						ship->SetEmpireId(person->EmpireId());

						DaAtomList execute;
						DaAtomList subs;
						ship->ListSubatoms(subs);
						DaAtomList people;
						subs.FindPeopleRecursive(people);
						for(DaAtomList::const_iterator it = people.begin(); it != people.end(); it++)
						{
							// Get person atom aboard the ship
							DaPerson* personAboard = (DaPerson*)(*it);
							if(!personAboard->IsInvisibleOffLine())
								continue;		// Skip person who is on line.

							// Kill offline non-friendly units aboard the ship. Friendly units get to remain aboard.
							POLITICAL_STANCE stance = person->PoliticalStanceMakeContact(personAboard);
							if(stance != POL_Enemy)
								continue;

							// The person cannot respawn aboard this ship.
							if(personAboard->SpawnShip() == ship->Id())
								personAboard->ClearSpawnShip();

							execute.append(personAboard);
						}

						QStringList executed;
						for(DaAtomList::const_iterator pi = execute.begin(); pi != execute.end(); pi++)
						{
							DaPerson* personAboard = (DaPerson*)*pi;

							// Kill the person immediately, with no bleed-out time. They will respawn somewhere else.
							// The person at the helm gets credit for the kills.
							AuCombatAttacker attacker(person->Id(),
									QObject::tr("execution by %1 while capturing ship").arg(person->Name()), person->EmpireId());

							AuCombatWeapon weapon(WM_Internal);
							weapon.m_CID = CID_ShipCapture;
							weapon.m_w1.m_Effect = WE_Bleeding;

							AuCombatResult result;
							result.m_Damage1 = personAboard->CurrentHitPoints() - DpBody::DeadHitPoints();

							personAboard->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
							personAboard->SetModified();

							executed.append(personAboard->Name());
						}

						ship->ClearCrewAndTroopsFromRoster();

						// Loot the ship's fund.
						double acctBalance = ship->AccountBalance();
						if(acctBalance > 0)
						{
							// Transfer all ship's funds to the person capturing the ship.
							person->CreditAccount(acctBalance);	// Credit the person.
							ship->DebitAccount(acctBalance);		// Debit the ship.
						}
						ship->SetModified();

						// Grant empire designated reward to capturing enemy spacecraft.
						DgEmpire empire;
						empire.m_id = person->EmpireId();
						const DgData& gd = g_Au.LockDataForRead();
						gd.Empire(empire);
						g_Au.UnlockData();
						if(ship->IsSpaceStation())
							person->Reward(0, REW_CaptureStation);
						else if(ship->IsStarship())
							person->Reward(0, REW_CaptureStarship);
						else
							person->Reward(0, REW_CaptureSpaceship);

						// Deactivate the mission so the ship will stop attacking.
						ship->MissionSetActive(false);

						// Notify everyone aboard via intercom that the capture sequence has completed. There is no crew to
						// receive a message on the crew channel.
						QString text;
						if(acctBalance > 0)
							text = QObject::tr("Capture sequence completed. %L3%4 transferred to %1. Flag of vessel "
									"changed to %2.")
								.arg(person->Name()).arg(empire.Name()).arg(acctBalance, 0,'f',0).arg(g_Au.CurrencySymbol());
						else
							text = QObject::tr("Capture sequence completed by %1. Flag of vessel changed to %2.")
								.arg(person->Name()).arg(empire.Name());

						// List players who were executed.
						if(!executed.isEmpty())
						{
							text += QObject::tr(" Offline players executed: ");
							for(QStringList::const_iterator ei = executed.cbegin(); ei != executed.cend(); ei++)
							{
								if(ei != executed.cbegin())
									text += ", ";
								text += *ei;
							}
						}

						// Send the message on the intercom channel, so the person capturing the ship gets some feedback.
						const DaSector* sector = ship->FindParentSector();
						AuCommChat intMsg(text, AuCommStdChannel::IntercomChannelName(), ship->Id(), ship->Name(), sector->Id(),
								sector->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
						g_Au.SendAtomChat(intMsg);
					}
					else if(m_CaptureSequence <= 5 || m_CaptureSequence%5 == 0)
					{
						// Notify crew and intercom that capture sequence is in progress.
						const DgData& gd = g_Au.LockDataForRead();
						QString empireName;
						gd.EmpireName(person->EmpireId(), empireName);
						g_Au.UnlockData();
						QString text = QObject::tr("Capture sequence in progress by %1 of %2. %3 seconds remaining.")
							.arg(person->Name()).arg(empireName).arg(m_CaptureSequence);

						// Send the message on the intercom channel, so the person capturing the ship gets some feedback.
						const DaSector* sector = ship->FindParentSector();
						AuCommChat intMsg(text, AuCommStdChannel::IntercomChannelName(), ship->Id(), ship->Name(),
								sector->Id(), sector->Galaxy(), AuCommStdChannel::Close,
								AuCommStdChannel::Intercom);
						g_Au.SendAtomChat(intMsg);

						// Send the message on the crew channel, so the crew knows they are losing their ship.
						AuCommChat crewMsg(text, AuCommStdChannel::CrewChannelName(), ship->Id(), ship->Name(),
								sector->Id(), sector->Galaxy(), AuCommStdChannel::Universe,
								AuCommStdChannel::CrewMates);
						crewMsg.SetScopeData(ship->Id().GetID());	// Crew channel scope needs ship's id.
						g_Au.SendAtomChat(crewMsg);
					}
				}
			}
		}
		else
		{
			// Stop testing for capture sequence. This will require the helmsman to exit the console and reenter the console if
			// they become an enemy of the ship while manning the helm.
			m_CaptureSequence = -1;
		}
	}

	m_Capacitor = ship->CapacitorPercent();
	m_CapacitorTrend = ship->CapacitorTrend();
	m_FuelAboard = ship->FuelPercent();
	m_GravityDriveState = ship->GravityDriveState();
	m_PowerPlantOn = ship->IsPowerPlantOn();
	m_Throttle = ship->Throttle();
	m_FTLDriveOn = ship->FTLDriveWorking(m_WhichFTL);
	if(ship->FTLDriveState(m_WhichFTL) == FTL_DRIVE_On)
		m_FTLDriveCommit = ship->FTLDriveCommitState(m_WhichFTL);
	else
		m_FTLDriveCommit = FTL_COMMIT_Dormant;
	m_FTLDriveSetting = ship->FTLDriveSetting(m_WhichFTL);

	if(!m_WhichFTL)
	{
		if(ship->HasWarpFTLDrive())
			m_WhichFTL = ship->FTLWarpDrive();
		else if(ship->HasWormholeFTLDrive())
			m_WhichFTL = ship->FTLWormholeDrive();
		else
			m_WhichFTL = 0xffff;
	}

	return true;
}

void DpStationHelm::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationHelm::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("PCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/pc_Console.bmp");
}

void DpStationHelm::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationHelm::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_Capacitor);
	mem += sizeof(m_CapacitorTrend);
	mem += sizeof(m_FuelAboard);
	mem += sizeof(m_GravityDriveState);
	mem += sizeof(m_PowerPlantOn);
	mem += sizeof(m_Throttle);
	mem += sizeof(m_FTLDriveCommit);
	mem += sizeof(m_FTLDriveOn);
	mem += sizeof(m_FTLDriveSetting);
	mem += sizeof(m_WhichFTL);
	mem += sizeof(m_CaptureSequence);
	mem += sizeof(m_LastFTLDriveCommit);
	mem += sizeof(m_LastFTLDriveOn);
	mem += sizeof(m_LastFTLDriveSetting);
	mem += sizeof(m_LastWhichFTL);
	mem += sizeof(m_LastAnyDoorsOpen);
	mem += sizeof(m_LastGravityDriveState);
	mem += sizeof(m_LastInEnemyRange);
	mem += sizeof(m_LastLatitude);
	mem += sizeof(m_LastLongitude);
	mem += sizeof(m_LastMode);
	mem += sizeof(m_LastMooringState);
	mem += sizeof(m_LastStress);
	mem += sizeof(m_lLastBeacon);
	mem += sizeof(m_lLastCapacitor);
	mem += sizeof(m_lLastCapTrend);
	mem += sizeof(m_lLastFuel);
	mem += sizeof(m_lLastHeadlight);
	mem += sizeof(m_lLastNavLight);
	mem += sizeof(m_lLastPowerOn);
	mem += sizeof(m_lLastSpeedSetting);
	mem += sizeof(m_lLastStrobe);
	mem += sizeof(m_Mode);
	mem += sizeof(m_rLastSpeed);
	mem += sizeof(m_rLastLSpeed);
	mem += sizeof(m_rLastVSpeed);
	mem += sizeof(m_rLastAltitude);
	mem += sizeof(m_rLastPitch);
	mem += sizeof(m_rLastRoll);
	mem += sizeof(m_rLastHeading);
	mem += sizeof(m_ScreenTex);
	return mem;
}
