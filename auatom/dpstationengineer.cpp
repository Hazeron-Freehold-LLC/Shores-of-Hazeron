//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationengineer.cpp,v 1.19 2022/04/14 16:44:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationengineer.h"
//
#include "auatomrenderinfo.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausound.h"
#include "daperson.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <qpainter.h>
#include <qsqlquery.h>
#include <QPixmap>

#define d_FTLEngage						SOUND_Bleep5
#define d_FTLPower						SOUND_Bleep7
#define d_FTLSelect						SOUND_Bleep4
#define d_FTLSetting					SOUND_Bleep6
#define d_GravHold						SOUND_Bleep2
#define d_GravOnOff						SOUND_Bleep1
#define d_PowerPlant					SOUND_Bleep3

// Table of control inputs.
DiControlInputArray s_EngineerConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_EngineerConsoleDefaults[] =
{
	// FTL Drive
	DiControlInput(QObject::tr("FTL Drive On/Off"),
				DpStationEngineer::CI_FTLOnOff, Qt::Key_O,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("FTL Drive Setting"),
				DpStationEngineer::CI_FTLSettingToggle, Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("FTL Drive Engage"),
				DpStationEngineer::CI_FTLCommit, Qt::Key_C,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select FTL Drive"),
				DpStationEngineer::CI_FTLSelect, Qt::Key_F,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Gravity Drive Drive
	DiControlInput(QObject::tr("Gravity Drive On/Off"),
				DpStationEngineer::CI_GravityDriveOnOff, Qt::Key_G,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Gravity Drive Full/Hold"),
				DpStationEngineer::CI_GravityDriveHold, Qt::Key_H,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Turn Rate Tune Up"),
				DpStationEngineer::CI_TurnRateSkewPP, Qt::Key_Up,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Turn Rate Tune Down"),
				DpStationEngineer::CI_TurnRateSkewNN, Qt::Key_Down,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Turn Rate Tune Left"),
				DpStationEngineer::CI_TurnRateSkewPN, Qt::Key_Left,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Turn Rate Tune Right"),
				DpStationEngineer::CI_TurnRateSkewNP, Qt::Key_Right,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Turn Rate Tune Hold"),
				DpStationEngineer::CI_TurnRateSkewHold, Qt::Key_Home,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Acceleration Tune Left"),
				DpStationEngineer::CI_AccelSkewP, Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Acceleration Tune Right"),
				DpStationEngineer::CI_AccelSkewN, Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Acceleration Tune Hold"),
				DpStationEngineer::CI_AccelSkewHold, Qt::Key_Z,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Power Plant
	DiControlInput(QObject::tr("Power Plant On/Off"),
				DpStationEngineer::CI_PowerPlantOnOff, Qt::Key_P,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Plant Tune Up"),
				DpStationEngineer::CI_PowerPlantSkewP, Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Power Plant Tune Down"),
				DpStationEngineer::CI_PowerPlantSkewN, Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Power Plant Tune Hold"),
				DpStationEngineer::CI_PowerPlantSkewHold, Qt::Key_X,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumEngineerConsoleDefaults = sizeof(s_EngineerConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationEngineer::DpStationEngineer(quint16 whichFTL) : DpStation()
{
	// Ship system values monitored on server.
	m_AccelSkew = 0;
	m_Capacitor = 0;
	m_CapacitorTrend = 0;
	m_FuelAboard = 0;
	m_GravityDriveState = DaVessel::GD_Off;
	m_PowerPlantOn = false;
	m_PowerPlantSkew = 0;
	m_TurnSkewX = 0;
	m_TurnSkewY = 0;
	m_FTLCommitState = FTL_COMMIT_Dormant;
	m_FTLDriveSetting = 0;
	m_FTLReady = false;
	m_FTLState = FTL_DRIVE_Off;

	// Init with bogus values to insure an update.
	m_bSkewAccelGraphInit = true;
	m_bSkewPowerPlantGraphInit = true;
	memset(m_chLastAccelSkew, 0, sizeof(m_chLastAccelSkew));
	memset(m_chLastPowerPlantSkew, 0, sizeof(m_chLastPowerPlantSkew));
	m_eLastFTLCommit = FTL_COMMIT_Invalid;
	m_eLastFTLState = FTL_DRIVE_Invalid;
	m_lLastCapacitor = -1;
	m_lLastCapTrend = -1;
	m_lLastFuel = -1;
	m_lLastPowerOn = -1;
	m_LastGravityDriveState = DaVessel::GD_Invalid;
	m_LastWhichFTL = -1;
	m_lLastFTLSetting = -1;
	m_lLastFTLReady = -1;
	m_WhichFTL = whichFTL;
	m_ScreenTex = 0;
}

DpStationEngineer::~DpStationEngineer()
{
}

DpStationEngineer::DpStationEngineer(const DpStationEngineer& c) : DpStation(c)
{
	m_PowerPlantOn = c.m_PowerPlantOn;
	m_GravityDriveState = c.m_GravityDriveState;
	m_FTLCommitState = c.m_FTLCommitState;
	m_FTLDriveSetting = c.m_FTLDriveSetting;
	m_FTLReady = c.m_FTLReady;
	m_FTLState = c.m_FTLState;
	m_FuelAboard = c.m_FuelAboard;
	m_Capacitor = c.m_Capacitor;
	m_CapacitorTrend = c.m_CapacitorTrend;
	m_TurnSkewX = c.m_TurnSkewX;
	m_TurnSkewY = c.m_TurnSkewY;
	m_AccelSkew = c.m_AccelSkew;
	m_PowerPlantSkew = c.m_PowerPlantSkew;
	m_lLastFTLSetting = c.m_lLastFTLSetting;
	m_bSkewAccelGraphInit = c.m_bSkewAccelGraphInit;	// true to init skew graphs.
	m_bSkewPowerPlantGraphInit = c.m_bSkewPowerPlantGraphInit;	// true to init skew graphs.
	memcpy(m_chLastAccelSkew, c.m_chLastAccelSkew, sizeof(char)*51);
	memcpy(m_chLastPowerPlantSkew, c.m_chLastPowerPlantSkew, sizeof(char)*137);
	m_eLastFTLCommit = c.m_eLastFTLCommit;
	m_eLastFTLState = c.m_eLastFTLState;
	m_lLastFuel = c.m_lLastFuel;
	m_lLastPowerOn = c.m_lLastPowerOn;
	m_LastGravityDriveState = c.m_LastGravityDriveState;
	m_LastWhichFTL = c.m_LastWhichFTL;
	m_lLastFTLReady = c.m_lLastFTLReady;
	m_WhichFTL = c.m_WhichFTL;
	m_ScreenTex = 0;
}

void DpStationEngineer::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);
	quint8 switches;
	in >> switches;
	m_CapacitorTrend = 0;
	if(switches & 0x01)
		m_CapacitorTrend = 1;
	else if(switches & 0x02)
		m_CapacitorTrend = -1;
	if(switches & 0x04)
		m_GravityDriveState = DaVessel::GD_On;
	else if(switches & 0x08)
		m_GravityDriveState = DaVessel::GD_Hold;
	else
		m_GravityDriveState = DaVessel::GD_Off;
	m_PowerPlantOn = (switches & 0x10) ? true : false;
	m_FTLReady = (switches & 0x40) ? true : false;

	quint8 ftl;
	in >> ftl;
	m_FTLState = (FTL_DRIVE_STATE)(ftl & 0x03);
	m_FTLCommitState = (FTL_COMMIT)(ftl >> 2);	// 0..63 range

	quint8 v8;
	in >> v8;
	m_FuelAboard = (char)v8;
	in >> v8;
	m_TurnSkewX = (char)v8;
	in >> v8;
	m_TurnSkewY = (char)v8;
	in >> v8;
	m_AccelSkew = (char)v8;
	in >> v8;
	m_PowerPlantSkew = (char)v8;
	in >> v8;
	m_Capacitor = (char)v8;

	in >> m_FTLDriveSetting;

	if(switches & 0x20)
		in >> m_WhichFTL;
	else
		m_WhichFTL = 0;
}

void DpStationEngineer::Tx(QDataStream& out) const
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
	if(m_WhichFTL)
		switches |= 0x20;
	if(m_FTLReady)
		switches |= 0x40;
	out << switches;

	quint8 ftl = 0x00;
	if(m_FTLState != FTL_DRIVE_Invalid)
		ftl |= m_FTLState;
	if(m_FTLCommitState != FTL_COMMIT_Invalid)
		ftl |= (m_FTLCommitState << 2);
	out << ftl;

	out << (quint8)m_FuelAboard;
	out << (quint8)m_TurnSkewX;
	out << (quint8)m_TurnSkewY;
	out << (quint8)m_AccelSkew;
	out << (quint8)m_PowerPlantSkew;
	out << (quint8)m_Capacitor;

	out << m_FTLDriveSetting;

	if(switches & 0x20)
		out << m_WhichFTL;
}

// Refresh data from valid counterpart.
void DpStationEngineer::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationEngineer* pA = (const DpStationEngineer*)pCon;
	m_PowerPlantOn = pA->m_PowerPlantOn;
	m_GravityDriveState = pA->m_GravityDriveState;
	m_FTLCommitState = pA->m_FTLCommitState;
	m_FTLDriveSetting = pA->m_FTLDriveSetting;
	m_FTLReady = pA->m_FTLReady;
	m_FTLState = pA->m_FTLState;
	m_FuelAboard = pA->m_FuelAboard;
	m_Capacitor = pA->m_Capacitor;
	m_CapacitorTrend = pA->m_CapacitorTrend;
	m_TurnSkewX = pA->m_TurnSkewX;
	m_TurnSkewY = pA->m_TurnSkewY;
	m_AccelSkew = pA->m_AccelSkew;
	m_PowerPlantSkew = pA->m_PowerPlantSkew;
	m_WhichFTL = pA->m_WhichFTL;
	//m_lLastFTLSetting = pA->m_lLastFTLSetting;
	//m_bSkewAccelGraphInit = pA->m_bSkewAccelGraphInit;	// true to init skew graphs.
	//m_bSkewPowerPlantGraphInit = pA->m_bSkewPowerPlantGraphInit;	// true to init skew graphs.
	//memcpy(m_chLastAccelSkew, pA->m_chLastAccelSkew, sizeof(char)*51);
	//memcpy(m_chLastPowerPlantSkew, pA->m_chLastPowerPlantSkew, sizeof(char)*137);
	//m_eLastFTLCommit = pA->m_eLastFTLCommit;
	//m_eLastFTLState = pA->m_eLastFTLState;
	//m_lLastFuel = pA->m_lLastFuel;
	//m_lLastPowerOn = pA->m_lLastPowerOn;
	//m_LastGravityDriveState = pA->m_LastGravityDriveState;
	//m_lLastFTLReady = pA->m_lLastFTLReady;
}

void DpStationEngineer::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationEngineer::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("ECon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/ec_Console.bmp");
}

void DpStationEngineer::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

bool DpStationEngineer::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	Q_UNUSED(last);
	Q_UNUSED(now);
	DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
	if(!ship || !ship->IsDesignDataCacheValid())
		return false;

	// Monitor ship systems on server.
	m_AccelSkew = ship->AccelSkew();
	m_Capacitor = ship->CapacitorPercent();
	m_CapacitorTrend = ship->CapacitorTrend();
	m_FuelAboard = ship->FuelPercent();
	m_GravityDriveState = ship->GravityDriveState();
	m_PowerPlantOn = ship->IsPowerPlantOn();
	m_PowerPlantSkew = ship->PowerPlantSkew();
	m_TurnSkewX = ship->TurnRateSkewX();
	m_TurnSkewY = ship->TurnRateSkewY();
	m_FTLCommitState = ship->FTLDriveCommitState(m_WhichFTL);
	m_FTLDriveSetting = ship->FTLDriveSetting(m_WhichFTL);
	m_FTLReady = ship->CanEngageFTLNow(m_WhichFTL);
	m_FTLState = ship->FTLDriveState(m_WhichFTL);

	return true;
}

// Called between rendering scene frames that contains this object.
void DpStationEngineer::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);

	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("ECon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	// Update the texture map with the current control states.
	// Drive Power and Status Indicators
	if((m_PowerPlantOn?1:0) != m_lLastPowerOn)
	{
		m_lLastPowerOn = m_PowerPlantOn;

		QImage bmbut;
		QImage bmlit;
		if(m_PowerPlantOn)
		{
			bmbut.load(":data/ec_PowerOn.bmp");
			bmlit.load(":data/ec_StatusLightGreen.bmp");
		}
		else
		{
			bmbut.load(":data/ec_PowerOff.bmp");
			bmlit.load(":data/ec_StatusLightOff.bmp");
		}
		pTex->ReplaceSubImage(re, bmbut, 156,13);
		pTex->ReplaceSubImage(re, bmlit, 157,32);
	}
	if(m_GravityDriveState != m_LastGravityDriveState)
	{
		m_LastGravityDriveState = m_GravityDriveState;

		QImage bmbut;
		QImage bmlit;
		if(m_GravityDriveState == DaVessel::GD_On)
		{
			bmbut.load(":data/ec_PowerOn.bmp");
			bmlit.load(":data/ec_StatusLightGreen.bmp");
		}
		else if(m_GravityDriveState == DaVessel::GD_Hold)
		{
			bmbut.load(":data/ec_PowerPark.bmp");
			bmlit.load(":data/ec_StatusLightPurple.bmp");
		}
		else
		{
			bmbut.load(":data/ec_PowerOff.bmp");
			bmlit.load(":data/ec_StatusLightOff.bmp");
		}
		pTex->ReplaceSubImage(re, bmbut, 156,90);
		pTex->ReplaceSubImage(re, bmlit, 157,109);
	}

	bool whichFTLChanged = m_LastWhichFTL != m_WhichFTL;
	m_LastWhichFTL = m_WhichFTL;
	if(m_FTLState != m_eLastFTLState || m_FTLState == FTL_DRIVE_Warming || whichFTLChanged)
	{
		m_eLastFTLState = m_FTLState;

		QImage bmbut;
		QImage bmlit;
		if(ship.SystemModule(SYS_FTLDrive, m_WhichFTL) != CID_Invalid)
		{
			switch(m_FTLState)
			{
			default:
			case FTL_DRIVE_Off:
				bmbut.load(":data/ec_PowerOff.bmp");
				bmlit.load(":data/ec_StatusLightOff.bmp");
				break;
			case FTL_DRIVE_Warming:
				bmbut.load(":data/ec_PowerOn.bmp");
				if(m_FTLCommitState&1)
					bmlit.load(":data/ec_StatusLightYellowDim.bmp");
				else
					bmlit.load(":data/ec_StatusLightYellow.bmp");
				break;
			case FTL_DRIVE_On:
				bmbut.load(":data/ec_PowerOn.bmp");
				bmlit.load(":data/ec_StatusLightGreen.bmp");
				break;
			}
		}
		else
		{
			bmbut.load(":data/ac_Blank16x16.bmp");
			bmlit.load(":data/ac_Blank16x16.bmp");
		}
		pTex->ReplaceSubImage(re, bmbut, 219,108);
		pTex->ReplaceSubImage(re, bmlit, 237,109);
	}

	// FTL setting indicator.
	bool ftlDriveSettingChanged;
	if((int)m_FTLDriveSetting != m_lLastFTLSetting || whichFTLChanged)
	{
		ftlDriveSettingChanged = true;
		m_lLastFTLSetting = m_FTLDriveSetting;

		QImage bmbut;
		if(DbCommodity::IsWormholeFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)))
		{
			if(m_FTLDriveSetting == 0)
				bmbut.load(":data/ec_WormChargePositive.bmp");
			else if(m_FTLDriveSetting == 1)
				bmbut.load(":data/ec_WormChargeNegative.bmp");
			else
				bmbut.load(":data/ec_WormChargeNeutral.bmp");
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
			bmbut.load(ftlSettings[m_FTLDriveSetting]);
		}
		else
		{
			// Ship has no FTL drive module we recognize. Blank the button.
			bmbut.load(":data/ac_Blank16x16.bmp");
		}
		pTex->ReplaceSubImage(re, bmbut, 177,108);
	}
	else
		ftlDriveSettingChanged = false;

	// FTL commit button.
	if(ship.HasFTLDrive() || whichFTLChanged)
	{
		if((m_FTLReady?1:0) != m_lLastFTLReady)
		{
			m_lLastFTLReady = m_FTLReady;

			QImage bmbut;
			if(m_FTLReady)
				bmbut.load(":data/ec_FTLFuelOn.bmp");
			else
				bmbut.load(":data/ec_FTLFuelOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 177,4);
		}

		// Selector indicates power on/off state of current drive.
		QImage bml;
		if(ship.FTLWarpDrive() && ship.FTLWormholeDrive())
			bml.load(":data/ec_FTLSelect.png");
		else
			bml.load(":data/ec_FTLSelectBlack.png");
		pTex->ReplaceSubImage(re, bml, 241,4);
	}
	else
	{
		// Blank out FTL button areas.
		QImage bml(":data/ec_FTLSelectBlack.png");
		pTex->ReplaceSubImage(re, bml, 241,4);
	}

	// FTL drive commit/charge state indicator.
	if(m_FTLCommitState != m_eLastFTLCommit || whichFTLChanged)
	{
		//qDebug("FTL Commit State %d", m_FTLCommitState);
		m_eLastFTLCommit = m_FTLCommitState;

		QImage bmFTL;
		if(DbCommodity::IsWormholeFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)))
		{
			if(m_FTLState == FTL_DRIVE_On)
			{
				if(m_FTLDriveSetting == 0)
				{
					switch(m_FTLCommitState)
					{
					case FTL_COMMIT_Invalid:
					case FTL_COMMIT_Dormant:
					case FTL_COMMIT_Discharged:
						bmFTL.load(":data/ec_WormDormant.bmp");
						break;
					case FTL_COMMIT_Precharge1:
						bmFTL.load(":data/ec_WormPrecharge1.bmp");
						break;
					case FTL_COMMIT_Precharge2:
						bmFTL.load(":data/ec_WormPrecharge2.bmp");
						break;
					case FTL_COMMIT_Precharge3:
						bmFTL.load(":data/ec_WormPrecharge3.bmp");
						break;
					case FTL_COMMIT_Precharge4:
						bmFTL.load(":data/ec_WormPrecharge4.bmp");
						break;
					case FTL_COMMIT_Precharge5:
						bmFTL.load(":data/ec_WormPrecharge5.bmp");
						break;
					case FTL_COMMIT_Precharge6:
						bmFTL.load(":data/ec_WormPrecharge6.bmp");
						break;
					case FTL_COMMIT_Discharge1:
						bmFTL.load(":data/ec_WormDischarge1.bmp");
						break;
					case FTL_COMMIT_Discharge2:
						bmFTL.load(":data/ec_WormDischarge2.bmp");
						break;
					case FTL_COMMIT_Discharge3:
						bmFTL.load(":data/ec_WormDischarge3.bmp");
						break;
					case FTL_COMMIT_Discharge4:
						bmFTL.load(":data/ec_WormDischarge4.bmp");
						break;
					case FTL_COMMIT_Discharge5:
						bmFTL.load(":data/ec_WormDischarge5.bmp");
						break;
					case FTL_COMMIT_Discharge6:
						bmFTL.load(":data/ec_WormDischarge6.bmp");
						break;
					default:
						// Remaining states are Charged
						switch(m_FTLCommitState%3)
						{
						case 0:
							bmFTL.load(":data/ec_WormCharged.bmp");
							break;
						case 1:
							bmFTL.load(":data/ec_WormCharged2.bmp");
							break;
						case 2:
							bmFTL.load(":data/ec_WormCharged3.bmp");
							break;
						}
						break;
					}
				}
				else
				{
					switch(m_FTLCommitState)
					{
					case FTL_COMMIT_Invalid:
					case FTL_COMMIT_Dormant:
					case FTL_COMMIT_Discharged:
						bmFTL.load(":data/ec_WormDormant.bmp");
						break;
					case FTL_COMMIT_Precharge1:
						bmFTL.load(":data/ec_WormNPrecharge1.bmp");
						break;
					case FTL_COMMIT_Precharge2:
						bmFTL.load(":data/ec_WormNPrecharge2.bmp");
						break;
					case FTL_COMMIT_Precharge3:
						bmFTL.load(":data/ec_WormNPrecharge3.bmp");
						break;
					case FTL_COMMIT_Precharge4:
						bmFTL.load(":data/ec_WormNPrecharge4.bmp");
						break;
					case FTL_COMMIT_Precharge5:
						bmFTL.load(":data/ec_WormNPrecharge5.bmp");
						break;
					case FTL_COMMIT_Precharge6:
						bmFTL.load(":data/ec_WormNPrecharge6.bmp");
						break;
					case FTL_COMMIT_Discharge1:
						bmFTL.load(":data/ec_WormNDischarge1.bmp");
						break;
					case FTL_COMMIT_Discharge2:
						bmFTL.load(":data/ec_WormNDischarge2.bmp");
						break;
					case FTL_COMMIT_Discharge3:
						bmFTL.load(":data/ec_WormNDischarge3.bmp");
						break;
					case FTL_COMMIT_Discharge4:
						bmFTL.load(":data/ec_WormNDischarge4.bmp");
						break;
					case FTL_COMMIT_Discharge5:
						bmFTL.load(":data/ec_WormNDischarge5.bmp");
						break;
					case FTL_COMMIT_Discharge6:
						bmFTL.load(":data/ec_WormNDischarge6.bmp");
						break;
					default:
						// Remaining states are Charged
						switch(m_FTLCommitState%3)
						{
						case 0:
							bmFTL.load(":data/ec_WormNCharged1.bmp");
							break;
						case 1:
							bmFTL.load(":data/ec_WormNCharged2.bmp");
							break;
						case 2:
							bmFTL.load(":data/ec_WormNCharged3.bmp");
							break;
						}
						break;
					}
				}
			}
			else
				bmFTL.load(":data/ec_WormDormant.bmp");
		}
		else if(DbCommodity::IsWarpFTLDriveModule(ship.SystemModule(SYS_FTLDrive, m_WhichFTL)))
		{
			if(m_FTLState == FTL_DRIVE_On)
			{
				switch(m_FTLCommitState)
				{
				case FTL_COMMIT_Invalid:
				case FTL_COMMIT_Dormant:
				case FTL_COMMIT_Discharged:
					bmFTL.load(":data/ec_WarpDormant.bmp");
					break;
				case FTL_COMMIT_Precharge1:
					bmFTL.load(":data/ec_WarpPrecharge1.bmp");
					break;
				case FTL_COMMIT_Precharge2:
					bmFTL.load(":data/ec_WarpPrecharge2.bmp");
					break;
				case FTL_COMMIT_Precharge3:
					bmFTL.load(":data/ec_WarpPrecharge3.bmp");
					break;
				case FTL_COMMIT_Precharge4:
					bmFTL.load(":data/ec_WarpPrecharge4.bmp");
					break;
				case FTL_COMMIT_Precharge5:
					bmFTL.load(":data/ec_WarpPrecharge5.bmp");
					break;
				case FTL_COMMIT_Precharge6:
					bmFTL.load(":data/ec_WarpPrecharge6.bmp");
					break;
				case FTL_COMMIT_Discharge1:
					bmFTL.load(":data/ec_WarpDischarge1.bmp");
					break;
				case FTL_COMMIT_Discharge2:
					bmFTL.load(":data/ec_WarpDischarge2.bmp");
					break;
				case FTL_COMMIT_Discharge3:
					bmFTL.load(":data/ec_WarpDischarge3.bmp");
					break;
				case FTL_COMMIT_Discharge4:
					bmFTL.load(":data/ec_WarpDischarge4.bmp");
					break;
				case FTL_COMMIT_Discharge5:
					bmFTL.load(":data/ec_WarpDischarge5.bmp");
					break;
				case FTL_COMMIT_Discharge6:
					bmFTL.load(":data/ec_WarpDischarge6.bmp");
					break;
				default:
					// Remaining states are Charged
					switch(m_FTLCommitState%3)
					{
					case 0:
						bmFTL.load(":data/ec_WarpCharged1.bmp");
						break;
					case 1:
						bmFTL.load(":data/ec_WarpCharged2.bmp");
						break;
					case 2:
						bmFTL.load(":data/ec_WarpCharged3.bmp");
						break;
					}
					break;
				}
			}
			else
				bmFTL.load(":data/ec_WarpDormant.bmp");
		}
		else
		{
			// Ship has no FTL drive module we recognize. Blank the panel.
			bmFTL.load(":data/ec_FTLBlank.png");
		}
		pTex->ReplaceSubImage(re, bmFTL, 177,18);
	}

	// Fuel.
	if(m_FuelAboard != m_lLastFuel)
	{
		m_lLastFuel = m_FuelAboard;

		const int fgwid = 56;
		QPixmap pmf(fgwid,4);
		if(!pmf.isNull())
		{
			int fullto = (fgwid * m_FuelAboard + 99) / 100;	// Round up
			QPainter p(&pmf);
			p.fillRect(0,0,fgwid,4,Qt::black);
			if(fullto > 16)			// in the green zone
				p.fillRect(0,0,fullto,4,QColor(84,209,255));
			else if(fullto > 6)		// in the yellow zone
				p.fillRect(0,0,fullto,4,QColor(255,245,84));
			else if(fullto > 0)		// in the red zone
				p.fillRect(0,0,fullto,4,QColor(255,72,72));
			QImage bmf = pmf.toImage();
			pTex->ReplaceSubImage(re, bmf, 6,54);
		}
	}

	// Capacitor
	if(m_Capacitor != m_lLastCapacitor || m_CapacitorTrend != m_lLastCapTrend || ftlDriveSettingChanged || whichFTLChanged)
	{
		m_lLastCapacitor = m_Capacitor;
		m_lLastCapTrend = m_CapacitorTrend;

		QPixmap pmf(100,5);
		if(!pmf.isNull())
		{
			int jump = 0;
			if(ship.HasFTLDrive() && ship.CapacitorMax() > 0.)
				jump = Percent(ship.PowerNeededToEngageFTLDrive(m_WhichFTL, m_FTLDriveSetting), ship.CapacitorMax());

			QPainter p(&pmf);
			p.fillRect(0,0,100,5,Qt::black);
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
					p.fillRect(0,0,m_Capacitor,3,QColor(0,255,0));	// charging
				else if(m_CapacitorTrend < 0)
					p.fillRect(0,0,m_Capacitor,3,QColor(255,0,0));	// discharge
				else
					p.fillRect(0,0,m_Capacitor,3,QColor(255,202,0));// neutral
			}
			QImage bmf = pmf.toImage();
			pTex->ReplaceSubImage(re, bmf, 70,53);
		}
	}

	// Update live monitoring controls.
	if(m_GravityDriveState == DaVessel::GD_On ||
			m_GravityDriveState == DaVessel::GD_Hold)
	{
		LogAccelSkew(m_AccelSkew);

		// Turn rate skew
		QImage imtr(":data/ec_TurnRateSkew.bmp");
		QPoint pxmid(28,28);
		AuPoint fskew(m_TurnSkewX, m_TurnSkewY);
//			double rlongest = sqrt((28 * 28) * 2);
		const double rlongest = 39.597979746446661366447284277872;
		double rdist = fskew.Length_2D();
		rdist = rdist / rlongest * 28.0;
		fskew.Unit(rdist);
		QPoint pxloc = pxmid + QPoint(round(fskew.x), round(fskew.y));
		imtr.setPixel(pxloc.x(),pxloc.y(), qRgb(255,255,0));
		imtr.setPixel(pxloc.x()+1,pxloc.y(), qRgb(192,192,0));
		imtr.setPixel(pxloc.x()-1,pxloc.y(), qRgb(192,192,0));
		imtr.setPixel(pxloc.x(),pxloc.y()+1, qRgb(192,192,0));
		imtr.setPixel(pxloc.x(),pxloc.y()-1, qRgb(192,192,0));
		imtr.setPixel(pxloc.x()+1,pxloc.y()+1, qRgb(128,128,0));
		imtr.setPixel(pxloc.x()+1,pxloc.y()-1, qRgb(128,128,0));
		imtr.setPixel(pxloc.x()-1,pxloc.y()+1, qRgb(128,128,0));
		imtr.setPixel(pxloc.x()-1,pxloc.y()-1, qRgb(128,128,0));
		pTex->ReplaceSubImage(re, imtr, 5,66);

		// Acceleration skew
		QImage ima(":data/ec_AccelSkew.bmp");
		for(int iy=0; iy<51; iy++)
			ima.setPixel(41 + round(m_chLastAccelSkew[iy]), 50-iy, qRgb(0,255,0));
		pTex->ReplaceSubImage(re, ima, 68,70);
	}

	if(m_PowerPlantOn)
	{
		LogPowerPlantSkew(m_PowerPlantSkew);

		// Vertical Acceleration skew
		QImage imva(":data/ec_VAccelSkew.bmp");
		for(int ix=0; ix<137; ix++)
			imva.setPixel(ix, 17+round(m_chLastPowerPlantSkew[ix]), qRgb(255,0,0));
		pTex->ReplaceSubImage(re, imva, 13,8);
	}
}

void DpStationEngineer::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
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
				// Engineer console only draws the next waypoint, not the entire route.
				DeWaypointRoute route;
				route.append(*ship->LockWaypoint(0));
				ship->UnlockWaypoint();
				RenderCourse(route, farProjection, tModel, sceneAtom, ari, false, true);
			}
		}
	}
}

void DpStationEngineer::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.constBegin(); ci != aAllControls.constEnd(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_EngineerConsoleControls.begin(); myci != s_EngineerConsoleControls.end(); myci++)
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

void DpStationEngineer::RestoreDefaultControls()
{
	s_EngineerConsoleControls.clear();
	for(int ic=0; ic<s_NumEngineerConsoleDefaults; ic++)
		s_EngineerConsoleControls.append(s_EngineerConsoleDefaults[ic]);
	SaveControls();
}

void DpStationEngineer::LoadControls() const
{
	// Init with my default controls.
	s_EngineerConsoleControls.clear();
	for(int ic=0; ic<s_NumEngineerConsoleDefaults; ic++)
		s_EngineerConsoleControls.append(s_EngineerConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_EngineerConsoleControls.LoadSettings("Controls/Engineer Console");
}

void DpStationEngineer::SaveControls() const
{
	// Store my controls in the settings file.
	s_EngineerConsoleControls.SaveSettings("Controls/Engineer Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationEngineer::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_EngineerConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_EngineerConsoleControls.constBegin();
			ci != s_EngineerConsoleControls.constEnd(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationEngineer::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch (ci.GetId())
		{
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

		case CI_FTLCommit:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Client)
				ship.EngageFTLDrive(m_WhichFTL);
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

		case CI_FTLSettingToggle:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Client)
				ship.SetFTLDriveSetting(m_WhichFTL, ship.FTLDriveSetting(m_WhichFTL)+1);	// Rolls over to 0 when limit exceeded.
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLSetting, 1, .9f);
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
		case CI_FTLOnOff:
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			if(cc != CONTROL_Client)
			{
				if(ship.FTLDriveState(m_WhichFTL) == FTL_DRIVE_Off)
					ship.TurnFTLDriveOn(m_WhichFTL);
				else
					ship.TurnFTLDriveOff(m_WhichFTL);
			}
			if(cc != CONTROL_Server)
			{
				if(ship.FTLDriveState(m_WhichFTL) == FTL_DRIVE_Off)
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLPower, 1, 1.1f);
					sceneAtom->AddEffect(buttonClick);
				}
				else
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_FTLPower, 1, .85f);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_TurnRateSkewPP:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.TurnRateSkewChange(-4,-4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_TurnRateSkewPN:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.TurnRateSkewChange(-4,4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_TurnRateSkewNP:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.TurnRateSkewChange(4,-4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_TurnRateSkewNN:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.TurnRateSkewChange(4,4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_TurnRateSkewHold:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.TurnRateSkewChange(0,0, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_AccelSkewP:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.AccelSkewChange(-4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_AccelSkewN:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.AccelSkewChange(4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_AccelSkewHold:
			if(cc == CONTROL_Client)
				break;
			if(ship.IsNew())
				break;	// This control cannot be activated when the ship is "nu".
			ship.AccelSkewChange(0, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_PowerPlantSkewP:
			if(cc == CONTROL_Client)
				break;
			ship.PowerPlantSkewChange(-4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_PowerPlantSkewN:
			if(cc == CONTROL_Client)
				break;
			ship.PowerPlantSkewChange(4, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		case CI_PowerPlantSkewHold:
			if(cc == CONTROL_Client)
				break;
			ship.PowerPlantSkewChange(0, 5);	//DHH The 5 seconds duration should be determined by character skill level.
			break;
		}
	}
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DpStationEngineer::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
		bool bContinuous)
{
	if(idPanel)
		return;
	double panWid, panHt;
	PanelDimensions(panWid, panHt);
	int ix = (int)(fPanelLoc.x * 256 / panWid);	// X pixel in texture map.
	int iy = (int)(fPanelLoc.y * 128 / panHt);
	QPoint pxHit(ix, iy);

	// Note that the top/bottom is reversed in the rectangles. This works with the iy
	// we've calculated that is also measured from the bottom, not the top as usual in
	// GDI.
	DiPanelControl acontrols[] =
	{
		// FTL Drive Controls
		DiPanelControl(CI_FTLOnOff, QRect(219,108,16,16)),
		DiPanelControl(CI_FTLSettingToggle, QRect(177,108,16,16)),
		DiPanelControl(CI_FTLCommit, QRect(177,4,11,11)),
		DiPanelControl(CI_FTLSelect, QRect(241,4,6,10)),

		// Gravity Drive Controls
		DiPanelControl(CI_GravityDriveOnOffHold, QRect(156,90,16,16)),
		DiPanelControl(CI_TurnRateSkewNP, QRect(55,116,5,5)),
		DiPanelControl(CI_TurnRateSkewNN, QRect(55,68,5,5)),
		DiPanelControl(CI_TurnRateSkewPP, QRect(7,116,5,5)),
		DiPanelControl(CI_TurnRateSkewPN, QRect(7,68,5,5)),
		DiPanelControl(CI_TurnRateSkewHold, QRect(13,74,41,41)),
		DiPanelControl(CI_AccelSkewN, QRect(113,65,11,3)),
		DiPanelControl(CI_AccelSkewP, QRect(95,65,11,3)),
		DiPanelControl(CI_AccelSkewHold, QRect(68,70,83,51)),

		// Power Plant Controls
		DiPanelControl(CI_PowerPlantOnOff, QRect(156,13,16,16)),
		DiPanelControl(CI_PowerPlantSkewP, QRect(5,31,5,9)),
		DiPanelControl(CI_PowerPlantSkewN, QRect(5,11,5,9)),
		DiPanelControl(CI_PowerPlantSkewHold, QRect(12,7,139,34)),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);

	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

void DpStationEngineer::LogAccelSkew(char chSkew)
{
	for (int il=49; il>=0; il--)
		m_chLastAccelSkew[il+1] = m_chLastAccelSkew[il];
	m_chLastAccelSkew[0] = chSkew;

	if (m_bSkewAccelGraphInit)
		InitAccelSkewGraph();
}

void DpStationEngineer::LogPowerPlantSkew(char chSkew)
{
	for (int il=135; il>=0; il--)
		m_chLastPowerPlantSkew[il+1] = m_chLastPowerPlantSkew[il];
	m_chLastPowerPlantSkew[0] = chSkew;

	if (m_bSkewPowerPlantGraphInit)
		InitPowerPlantSkewGraph();
}

void DpStationEngineer::InitAccelSkewGraph()
{
	//qDebug("DpStationEngineer::InitAccelSkewGraph %x", (unsigned int)this);
	for (int il=1; il<51; il++)
	{
		char chVal = m_chLastAccelSkew[il-1];
		char chAccelSkew = (char)g_Au.Dice().Roll(1,5,-3);		// -2, -1, 0, 1 or 2
		chVal += chAccelSkew;
		if (chVal > 40)
			chVal = 40;
		else if (chVal < -40)
			chVal = -40;
		m_chLastAccelSkew[il] = chVal;
	}

	m_bSkewAccelGraphInit = false;
}

void DpStationEngineer::InitPowerPlantSkewGraph()
{
	for (int il=1; il<137; il++)
	{
		char chVal = m_chLastPowerPlantSkew[il-1];
		char chPowerPlantSkew = (char)g_Au.Dice().Roll(1,5,-3);		// -2, -1, 0, 1 or 2
		chVal += chPowerPlantSkew;
		if (chVal > 16)
			chVal = 16;
		else if (chVal < -16)
			chVal = -16;
		m_chLastPowerPlantSkew[il] = chVal;
	}

	m_bSkewPowerPlantGraphInit = false;
}

quint64 DpStationEngineer::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_AccelSkew);
	mem += sizeof(m_Capacitor);
	mem += sizeof(m_CapacitorTrend);
	mem += sizeof(m_FuelAboard);
	mem += sizeof(m_GravityDriveState);
	mem += sizeof(m_PowerPlantOn);
	mem += sizeof(m_PowerPlantSkew);
	mem += sizeof(m_TurnSkewX);
	mem += sizeof(m_TurnSkewY);
	mem += sizeof(m_FTLState);
	mem += sizeof(m_FTLCommitState);
	mem += sizeof(m_FTLDriveSetting);
	mem += sizeof(m_FTLReady);
	mem += sizeof(m_WhichFTL);
	mem += sizeof(m_bSkewAccelGraphInit);
	mem += sizeof(m_bSkewPowerPlantGraphInit);
	mem += sizeof(m_chLastAccelSkew);
	mem += sizeof(m_chLastPowerPlantSkew);
	mem += sizeof(m_eLastFTLCommit);
	mem += sizeof(m_eLastFTLState);
	mem += sizeof(m_lLastCapacitor);
	mem += sizeof(m_lLastCapTrend);
	mem += sizeof(m_lLastFuel);
	mem += sizeof(m_LastGravityDriveState);
	mem += sizeof(m_LastWhichFTL);
	mem += sizeof(m_lLastPowerOn);
	mem += sizeof(m_lLastFTLSetting);
	mem += sizeof(m_lLastFTLReady);
	mem += sizeof(m_ScreenTex);
	return mem;
}
