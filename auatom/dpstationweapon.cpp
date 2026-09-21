//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationweapon.cpp,v 1.39 2022/04/14 16:45:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationweapon.h"
//
#include "auatomrenderinfo.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausound.h"
#include "daperson.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <QPainter>
#include <QPixmap>

#define d_Bay							SOUND_Bleep4
#define d_Fire							SOUND_Bleep3
#define d_Power							SOUND_Bleep7
#define d_Show							SOUND_Bleep1
#define d_Stance						SOUND_Bleep2
#define d_View							SOUND_Bleep6

static QImage s_SensorRange;

// Table of control inputs.
DiControlInputArray s_WeaponConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_WeaponConsoleDefaults[] =
{
	// Login
	//DiControlInput(QObject::tr("Login"),
				//DpStationWeapon::CI_Login, Qt::Key_L, Qt::ShiftModifier,
				//DiControlInput::MT_None, Qt::NoModifier,
				//DiControlInput::JT_None, 0, 100, false),

	// Display toggles
	DiControlInput(QObject::tr("Show Ships"),
				DpStationWeapon::CI_ShipsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Stations"),
				DpStationWeapon::CI_StationsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Cities"),
				DpStationWeapon::CI_CitiesToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Worlds"),
				DpStationWeapon::CI_PlanetsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Stars"),
				DpStationWeapon::CI_StarsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Political Stance Toggle
	DiControlInput(QObject::tr("Show Empire Units"),
				DpStationWeapon::CI_EmpireToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Friendly Units"),
				DpStationWeapon::CI_FriendToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Neutral Units"),
				DpStationWeapon::CI_NeutralToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Enemy Units"),
				DpStationWeapon::CI_EnemyToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Bay weapon selector
	DiControlInput(QObject::tr("Select Next Weapon Bay"),
				DpStationWeapon::CI_NextBay, Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Previous Weapon Bay"),
				DpStationWeapon::CI_PreviousBay, Qt::Key_B,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Rotations
	DiControlInput(QObject::tr("Top View"),
				DpStationWeapon::CI_RotateTopView, Qt::Key_X,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Rotate to Target"),
				DpStationWeapon::CI_RotateToTarget, Qt::Key_Q,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Rotate Left"),
				DpStationWeapon::CI_RotateLeft, Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Right"),
				DpStationWeapon::CI_RotateRight, Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Up"),
				DpStationWeapon::CI_RotateUp, Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Down"),
				DpStationWeapon::CI_RotateDown, Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),

	// Zoom
	DiControlInput(QObject::tr("Increase Magnification"),
				DpStationWeapon::CI_ZoomIn, Qt::Key_PageUp,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Decrease Magnification"),
				DpStationWeapon::CI_ZoomOut, Qt::Key_PageDown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Normal Magnification"),
				DpStationWeapon::CI_ZoomNormal, Qt::Key_Home,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Power Settings
	DiControlInput(QObject::tr("Minimum Power"),
				DpStationWeapon::CI_PowerMin, Qt::Key_Down,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Maximum Power"),
				DpStationWeapon::CI_PowerMax, Qt::Key_Up,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Decrease Power"),
				DpStationWeapon::CI_PowerDecrease, Qt::Key_Left,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Increase Power"),
				DpStationWeapon::CI_PowerIncrease, Qt::Key_Right,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Fire"),
				DpStationWeapon::CI_Fire, Qt::Key_F,
				Qt::ShiftModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumWeaponConsoleDefaults = sizeof(s_WeaponConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationWeapon::DpStationWeapon(quint16 whichWeapon) : DpStation()
{
	m_bCities = true;
	m_bPlanets = true;
	m_bShips = true;
	m_bStations = true;
	m_bStars = true;
	m_Capacitor = 0;
	m_CapacitorMax = 0;
	m_CapacitorTrend = 0;
	m_fRot = AuPoint(0,0,0);	// Default rotation has +Y going up the screen, which is ship forward.
	m_fScale = (float)FeetPerOrbit();
	m_SelBlip = 0;
	m_ShowEmpire = false;
	m_ShowEnemy = true;
	m_ShowFriend = false;
	m_ShowNeutral = true;

	// Init with bogus values to insure an update.
	m_lLastCapacitor = -1;
	m_lLastCapTrend = -1;
	m_lLastCities = -1;
	m_lLastPlanets = -1;
	m_lLastShips = -1;
	m_lLastStations = -1;
	m_lLastStars = -1;
	m_lLastWhich = 0xffff;

	m_Ordnance = 0;
	m_PowerSetting = 0;
	m_SensorRange = 0;
	m_WeaponPower = 0;
	m_WhichWeapon = whichWeapon;
	m_ScreenTex = 0;
}

DpStationWeapon::~DpStationWeapon()
{
	if(m_Ordnance)
		delete m_Ordnance;
	if(m_SelBlip)
		delete m_SelBlip;
}

DpStationWeapon::DpStationWeapon(const DpStationWeapon& c) : DpStation(c)
{
	m_bCities = c.m_bCities;
	m_bPlanets = c.m_bPlanets;
	m_bShips = c.m_bShips;
	m_bStations = c.m_bStations;
	m_bStars = c.m_bStars;
	m_Capacitor = c.m_Capacitor;
	m_CapacitorMax = c.m_CapacitorMax;
	m_CapacitorTrend = c.m_CapacitorTrend;
	m_idLockedBlip = c.m_idLockedBlip;
	m_fRot = c.m_fRot;
	m_fScale = c.m_fScale;
	m_lLastCapacitor = c.m_lLastCapacitor;
	m_lLastCapTrend = c.m_lLastCapTrend;
	m_lLastCities = c.m_lLastCities;
	m_lLastPlanets = c.m_lLastPlanets;
	m_lLastShips = c.m_lLastShips;
	m_lLastStations = c.m_lLastStations;
	m_lLastStars = c.m_lLastStars;
	m_lLastWhich = c.m_lLastWhich;
	m_PowerSetting = c.m_PowerSetting;
	m_SensorRange = c.m_SensorRange;
	m_ShowEmpire = c.m_ShowEmpire;
	m_ShowEnemy = c.m_ShowEnemy;
	m_ShowFriend = c.m_ShowFriend;
	m_ShowNeutral = c.m_ShowNeutral;
	m_WeaponPower = c.m_WeaponPower;
	//m_tmLastUpdate = c.m_tmLastUpdate;
	m_WhichWeapon = c.m_WhichWeapon;

	c.m_BlipLock.lockForRead();
	if(c.m_SelBlip)
		m_SelBlip = c.m_SelBlip->Clone();
	else
		m_SelBlip = 0;
	m_SensorBlips = c.m_SensorBlips;
	c.m_BlipLock.unlock();

	c.m_OrdnanceLock.lockForRead();
	if(c.m_Ordnance)
		m_Ordnance = c.m_Ordnance->Clone();
	else
		m_Ordnance = 0;
	c.m_OrdnanceLock.unlock();

	m_ScreenTex = 0;
}

void DpStationWeapon::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);

	quint16 bits;
	in >> bits;
	m_bCities = (bits & 0x0001) ? true : false;
	m_bPlanets = (bits & 0x0002) ? true : false;
	m_bShips = (bits & 0x0004) ? true : false;
	m_bStations = (bits & 0x0008) ? true : false;
	m_bStars = (bits & 0x0010) ? true : false;
	m_CapacitorTrend = 0;
	if(bits & 0x0040)
		m_CapacitorTrend = 1;
	else if(bits & 0x0080)
		m_CapacitorTrend = -1;
	m_ShowEmpire = (bits & 0x1000) ? true : false;
	m_ShowFriend = (bits & 0x2000) ? true : false;
	m_ShowNeutral = (bits & 0x4000) ? true : false;
	m_ShowEnemy = (bits & 0x8000) ? true : false;

	quint8 ui8;
	in >> ui8;
	m_Capacitor = (char)ui8;

	quint8 blipType;
	in >> blipType;
	if(m_SelBlip)
		delete m_SelBlip;
	m_SelBlip = AuSensorBlip::Create((AuSensorBlip::TYPE)blipType);
	if(m_SelBlip)
		in >> *m_SelBlip;

	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fRot = AuPoint(x,y,z);
	in >> m_fScale;
	in >> m_PowerSetting;
	in >> m_SensorRange;
	in >> m_WeaponPower;
	in >> m_WhichWeapon;

	in >> m_CapacitorMax;

	// Sensor blips need transform of capsule as known by server.
	m_idLockedBlip << in;
	m_SensorBlips.RxMin(in);

	// Scan local data to pick up things that are already in my scene, namely suns, planets and moons.
	//qDebug() << "DpStationWeapon::Rx blips" << m_SensorBlips.count();
	if(capsule.SensorsWorking())
	{
		DaAtom* envAtom = capsule.FindEnvironmentAtom();
		if(envAtom)
		{
			AuId scannerWorldId;
			int scannerDomain = capsule.GunnerDomain(scannerWorldId);

			AuSensorBlipList localBlips;
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash> stanceCache;
			int scanContent = 0x0002;							// b1 = fire control console blips
			envAtom->SensorScan(localBlips, capsule.EnvLocation(), capsule.Id(), capsule.EmpireId(), scannerDomain, scannerWorldId,
					m_SensorRange, capsule.MaxBayWeaponRange(), scanContent, m_idLockedBlip, stanceCache);

			// Append localBlips that did not arrive in m_SensorBlips.
			for(AuSensorBlipList::const_iterator lbi = localBlips.begin(); lbi != localBlips.end(); lbi++)
			{
				// Find the corresponding blip in the m_SensorBlips array.
				AuSensorBlipList::const_iterator sbi = m_SensorBlips.Find(*lbi);
				if(sbi == m_SensorBlips.end())
				{
					// The local blip was not found in the sensor scan data sent by the server.
					m_SensorBlips.Add(*(*lbi));	// Add a clone.
				}
			}
		}
	}
	//qDebug() << "DpStationWeapon::Rx blips merged with locals" << m_SensorBlips.count();
}

void DpStationWeapon::Tx(QDataStream& out) const
{
	DpStation::Tx(out);

	quint16 bits = 0;
	if(m_bCities)
		bits |= 0x0001;
	if(m_bPlanets)
		bits |= 0x0002;
	if(m_bShips)
		bits |= 0x0004;
	if(m_bStations)
		bits |= 0x0008;
	if(m_bStars)
		bits |= 0x0010;
	if(m_CapacitorTrend > 0)
		bits |= 0x0040;
	else if(m_CapacitorTrend < 0)
		bits |= 0x0080;
	if(m_ShowEmpire)
		bits |= 0x1000;
	if(m_ShowFriend)
		bits |= 0x2000;
	if(m_ShowNeutral)
		bits |= 0x4000;
	if(m_ShowEnemy)
		bits |= 0x8000;
	out << bits;
	out << (quint8)m_Capacitor;

	m_BlipLock.lockForRead();
	if(m_SelBlip)
	{
		out << (quint8)m_SelBlip->Type();
		out << *m_SelBlip;
	}
	else
		out << (quint8)AuSensorBlip::Invalid;
	m_BlipLock.unlock();

	out << (float)m_fRot.x;
	out << (float)m_fRot.y;
	out << (float)m_fRot.z;
	out << m_fScale;
	out << m_PowerSetting;
	out << m_SensorRange;
	out << m_WeaponPower;
	out << m_WhichWeapon;
	out << m_CapacitorMax;
	out << m_idLockedBlip;

	m_BlipLock.lockForRead();
	m_SensorBlips.TxMinRangeLimited(out);
	m_BlipLock.unlock();
}

// Refresh data from counterpart.
void DpStationWeapon::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationWeapon* pA = (const DpStationWeapon*)pCon;
	m_SensorBlips = pA->m_SensorBlips;
	m_bCities = pA->m_bCities;
	m_bPlanets = pA->m_bPlanets;
	m_bShips = pA->m_bShips;
	m_bStations = pA->m_bStations;
	m_bStars = pA->m_bStars;
	m_Capacitor = pA->m_Capacitor;
	m_CapacitorMax = pA->m_CapacitorMax;
	m_CapacitorTrend = pA->m_CapacitorTrend;
	m_idLockedBlip = pA->m_idLockedBlip;
	m_fRot = pA->m_fRot;
	m_fScale = pA->m_fScale;
	m_PowerSetting = pA->m_PowerSetting;
	m_SensorRange = pA->m_SensorRange;

	if(m_SelBlip)
		delete m_SelBlip;
	if(pA->m_SelBlip)
		m_SelBlip = pA->m_SelBlip->Clone();
	else
		m_SelBlip = 0;

	m_ShowEmpire = pA->m_ShowEmpire;
	m_ShowFriend = pA->m_ShowFriend;
	m_ShowNeutral = pA->m_ShowNeutral;
	m_ShowEnemy = pA->m_ShowEnemy;
	m_WeaponPower = pA->m_WeaponPower;
	m_WhichWeapon = pA->m_WhichWeapon;
}

// Called between rendering scene frames that contains this object.
void DpStationWeapon::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);

	if(s_SensorRange.isNull())
		s_SensorRange.load(":data/SensorRange.png");

	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("WCon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	// Update the texture map with the current control states.
	// Filter Buttons
	if ((m_bShips?1:0) != m_lLastShips)
	{
		m_lLastShips = m_bShips;

		QImage bmbut;
		if (m_bShips)
			bmbut.load(":data/sc_ShipsOn.bmp");
		else
			bmbut.load(":data/sc_ShipsOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,107);
	}
	if ((m_bStations?1:0) != m_lLastStations)
	{
		m_lLastStations = m_bStations;

		QImage bmbut;
		if (m_bStations)
			bmbut.load(":data/sc_StarbasesOn.bmp");
		else
			bmbut.load(":data/sc_StarbasesOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,90);
	}
	if ((m_bCities?1:0) != m_lLastCities)
	{
		m_lLastCities = m_bCities;

		QImage bmbut;
		if (m_bCities)
			bmbut.load(":data/sc_CitiesOn.bmp");
		else
			bmbut.load(":data/sc_CitiesOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,73);
	}
	if ((m_bPlanets?1:0) != m_lLastPlanets)
	{
		m_lLastPlanets = m_bPlanets;

		QImage bmbut;
		if (m_bPlanets)
			bmbut.load(":data/sc_PlanetsOn.bmp");
		else
			bmbut.load(":data/sc_PlanetsOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,56);
	}
	if ((m_bStars?1:0) != m_lLastStars)
	{
		m_lLastStars = m_bStars;

		QImage bmbut;
		if (m_bStars)
			bmbut.load(":data/sc_StarsOn.bmp");
		else
			bmbut.load(":data/sc_StarsOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,39);
	}

	//if(tmNow != m_tmLastUpdate)
	{
		//m_tmLastUpdate = tmNow;

		// Scanner display.
		QPixmap bmf;
		bmf.load(":data/sc_ScreenFrame.png");
		QPixmap bms(bmf.width(),bmf.height());
		bms.fill(QColor(0,0,32));
		QPainter p(&bms);

		// Make transforms to projection the locations into the bitmap.
		AuMatrix4x4 tdev;
		tdev.Translate(bmf.width()/2.0, bmf.height()/2.0, 0.0);
		double rpxscl = bmf.height()/2.0;
		tdev.Scale(rpxscl, -rpxscl, rpxscl);	// Flip Y
//			tdev.Perspective(60.0, 1.0, 0.5, 1000.0);
//			tdev.Translate(0.0, 0.0, -1.0);

		AuMatrix4x4 trot;
		trot.Rotate(m_fRot);
		trot.Scale(1.0/m_fScale, 1.0/m_fScale, 1.0/m_fScale);

		AuMatrix4x4 tcomb = tdev * trot * ship.EnvTransform().Inverse();

		// Display information into the scanner display.
		const DaAtom* system = ship.FindEnvironmentAtom();

		// Circle to show sensor range.
		AuPoint fpx = tcomb.Project(ship.EnvLocation());
		fpx.x = bms.width()/2.f;
		fpx.y = bms.height()/2.f;
		QPoint pxme(round(fpx.x),round(fpx.y));
		float feetPerPixel = m_fScale/(bmf.height()/2);
		float radius = m_SensorRange/feetPerPixel;	// diameter in pixels
		QRectF rcrange(fpx.x - radius, fpx.y - radius, radius*2.f, radius*2.f);
		p.drawImage(rcrange, s_SensorRange, QRectF(0,0,s_SensorRange.width(),s_SensorRange.height()));

		// Axis lines to sensor range.
		p.setPen(QPen(QColor(192,0,0), 0.f));
		fpx = tcomb.Project(ship.EnvTransform().Project(m_SensorRange,0,0));
		p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

		p.setPen(QPen(QColor(64,0,0), 0.f));
		fpx = tcomb.Project(ship.EnvTransform().Project(-m_SensorRange,0,0));
		p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

		p.setPen(QPen(QColor(0,192,0), 0.f));
		fpx = tcomb.Project(ship.EnvTransform().Project(0,m_SensorRange,0));
		p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

		p.setPen(QPen(QColor(0,64,0), 0.f));
		fpx = tcomb.Project(ship.EnvTransform().Project(0,-m_SensorRange,0));
		p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

		p.setPen(QPen(QColor(0,0,192), 0.f));
		fpx = tcomb.Project(ship.EnvTransform().Project(0,0,m_SensorRange));
		p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

		p.setPen(QPen(QColor(0,0,64), 0.f));
		fpx = tcomb.Project(ship.EnvTransform().Project(0,0,-m_SensorRange));
		p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

		// Display blips that have been detected.
		// Determine the amount of energy that is expected to hit the target.
		COMMODITYID bayModule = ship.SystemModule(SYS_Weapon, m_WhichWeapon);
		float targetE = 0;	// E or U on target, depending on targetType.
		int targetType = 0;	// 0=no target energy, 1=target energy, 2=target harvest units, 3=heavy arms, 4=missiles, 5=rockets
		AuPoint sysloc = ship.EnvTransform().Project(location);
		if(m_SelBlip)
		{
			float wpnRange = ship.BayWeaponRange(m_WhichWeapon);

			if(bayModule == CID_EnergyWeaponBayModule)
			{
				AuPoint fLockLoc = m_SelBlip->WeaponLockLocation(system, ship.EnvLocation(), bayModule);
				double targDist = fLockLoc.Distance(sysloc);

				unsigned int unitsFired = BayWeaponProjectors(m_WeaponPower) * (m_PowerSetting + 1)/41;
				if(!unitsFired)
					unitsFired = 1;

				if(targDist < wpnRange)
				{
					// The returned ammunition has the count set to the number available in the hold.
					const AuItem* ammoSel = sceneAtom.Ordnance();
					if(ammoSel)
					{
						targetType = 1;	// E damage on target

						unsigned int maxRounds = ammoSel->Count();
						if(maxRounds > ship.CapacitorLevel())
							maxRounds = (int)ship.CapacitorLevel();	// Energy weapon limited to capacitor.
						if(unitsFired > maxRounds)
							targetE = maxRounds;
						else
							targetE = unitsFired;
					}
				}
			}
			else if(bayModule == CID_BallisticWeaponBayModule)
			{
				AuPoint fLockLoc = m_SelBlip->WeaponLockLocation(system, ship.EnvLocation(), bayModule);
				double targDist = fLockLoc.Distance(sysloc);

				unsigned int unitsFired = BayWeaponCannons(m_WeaponPower) * (m_PowerSetting + 1)/41;
				if(!unitsFired)
					unitsFired = 1;

				if(targDist < wpnRange)
				{
					// The returned ammunition has the count set to the number available in the hold.
					const AuItem* ammoSel = sceneAtom.Ordnance();
					if(ammoSel)
					{
						targetType = 3;	// Heavy arms ammo

						unsigned int maxRounds = ammoSel->Count();
						if(unitsFired > maxRounds)
							targetE = maxRounds;
						else
							targetE = unitsFired;
					}
				}
			}
			else if(bayModule == CID_WeaponDeploymentBayModule)
			{
				AuPoint fLockLoc = m_SelBlip->WeaponLockLocation(system, ship.EnvLocation(), bayModule);
				double targDist = fLockLoc.Distance(sysloc);

				unsigned int unitsFired = BayWeaponEjectorRacks(m_WeaponPower) * (m_PowerSetting + 1)/41;
				if(!unitsFired)
					unitsFired = 1;

				if(targDist < wpnRange)
				{
					// The returned ammunition has the count set to the number available in the hold.
					const AuItem* ammoSel = sceneAtom.Ordnance();
					if(ammoSel)
					{
						targetType = 6;	// Bombs

						unsigned int maxRounds = ammoSel->Count();
						if(unitsFired > maxRounds)
							targetE = maxRounds;
						else
							targetE = unitsFired;
					}
				}
			}
			else if(bayModule == CID_WeaponLauncherBayModule)
			{
				AuPoint fLockLoc = m_SelBlip->WeaponLockLocation(system, ship.EnvLocation(), bayModule);
				double targDist = fLockLoc.Distance(sysloc);

				unsigned int unitsFired = BayWeaponLaunchers(m_WeaponPower) * (m_PowerSetting + 1)/41;
				if(!unitsFired)
					unitsFired = 1;

				if(targDist < wpnRange)
				{
					// The returned ammunition has the count set to the number available in the hold.
					const AuItem* ammoSel = sceneAtom.Ordnance();
					if(ammoSel)
					{
						if(DbCommodity::IsMissile(ammoSel->CommodityId()))
							targetType = 4;	// missiles
						else
							targetType = 5;	// rockets

						unsigned int maxRounds = ammoSel->Count();
						if(unitsFired > maxRounds)
							targetE = maxRounds;
						else
							targetE = unitsFired;
					}
				}
			}
			else if(DbCommodity::IsHarvesterModule(bayModule) && m_SelBlip->IsStarOrPlanet())
			{
				AuPoint fLockLoc = m_SelBlip->WeaponLockLocation(system, ship.EnvLocation(), bayModule);
				double targDist = fLockLoc.Distance(sysloc);

				// Calculate amount of energy to reach the target.
				// Weapon power drops off linearly to zero at
				// the maximum range, determined as follows.
				//  E = energy fired
				//  R = range at which linear dropoff reaches 0
				//  R = sqrt(E) * 50
				// In other words, 400 units of power fired will
				// drop off to 0 units at 1000 meters.
				// One weapon bay unit in a spacecraft fires 100 energy
				// units, consuming one unit from the capacitor.
				unsigned int unitsFired = m_WeaponPower;	// Harvesters always fire at full power.
				if(!unitsFired)
					unitsFired = 1;
				float wpnRange = ship.HarvesterBayRange(unitsFired, m_WhichWeapon);
				if(wpnRange > 0 && targDist < wpnRange)
				{
					float dropOff = 1.f - targDist / wpnRange;
					float wpnPower = ship.BayWeaponPower(unitsFired, m_WhichWeapon);
					targetE = round(wpnPower * dropOff);

					// Harvest up to 1 unit for every fixed quantity of energy that hits target.
					unsigned int harvestUnits = (int)(targetE / HarvestUnitsPerEnergyUnit());
					targetE = harvestUnits;
				}
				else
					targetE = 0;
				targetType = 2;	// U harvest units
			}
			else if(bayModule == CID_TractorBeamBayModule)
			{
				AuPoint fLockLoc = m_SelBlip->WeaponLockLocation(system, ship.EnvLocation(), bayModule);
				double targDist = fLockLoc.Distance(sysloc);

				// Calculate amount of energy to reach the target.
				// Weapon power drops off linearly to zero at
				// the maximum range, determined as follows.
				//  E = energy fired
				//  R = range at which linear dropoff reaches 0
				//  R = sqrt(E) * 50
				// In other words, 400 units of power fired will
				// drop off to 0 units at 1000 meters.
				// One weapon bay unit in a spacecraft fires 100 energy
				// units, consuming one unit from the capacitor.
				unsigned int unitsFired = m_WeaponPower;	// Tractor beams always fire at full power.
				if(!unitsFired)
					unitsFired = 1;
				float wpnRange = ship.TractorBeamRange(unitsFired, m_WhichWeapon);
				if(wpnRange > 0 && targDist < wpnRange)
					targetE = 1;
				else
					targetE = 0;	// Out of range to attach.
				targetType = 7;	// Tractor beam
			}
		}

		// Draw the ship sensor target blips.
		m_SensorBlips.DrawFireControlConsoleBlips(p, tcomb, feetPerPixel, m_SelBlip, m_idLockedBlip,
				m_bCities, false, m_bPlanets, m_bShips, m_bStations,
				m_bStars, false, targetType, targetE, sysloc,
				m_ShowEmpire, m_ShowFriend, m_ShowNeutral, m_ShowEnemy, system, ship.Id(), ship.EnvLocation());

		// Display the picture of the blips.
		p.drawPixmap(0,0, bmf);

		// Mask the frame onto the picture using its alpha channel to stencil
		// the oval overlay.
		QImage ims = bms.toImage();
		pTex->ReplaceSubImage(re, ims, 24,5);

		// These last couple of buttons are drawn on top of the overlay so they
		// must be painted every time.

		int powerSetting = m_PowerSetting;		// Harvesters will override this with all available units.
		if(bayModule == CID_TractorBeamBayModule)
		{
			// Tow button
			QImage bmfire;
			bmfire.load(":data/wc_Tow.bmp");
			pTex->ReplaceSubImage(re, bmfire, 239,92);
			powerSetting = 41;		// Tractor beams fire all available units.
			QImage impower(":data/wc_Tractor.bmp");
			pTex->ReplaceSubImage(re, impower, 210,106);
		}
		else
		{
			// Fire button
			QImage bmfire;
			bmfire.load(":data/wc_Fire.bmp");
			pTex->ReplaceSubImage(re, bmfire, 239,92);
			if(bayModule == CID_AtmosphereCondenserBayModule)
			{
				powerSetting = 41;		// Harvesters fire all available units.

				// Commodity to Harvest Setting
				// Setting control is 41 pixels wide so it can visibly show a range from [0-40].
				QPixmap bmpower(":data/wc_HarvestSel.bmp");
				QPainter ppower(&bmpower);

				// Draw icon of current harvester commodity.
				if(m_PowerSetting < 0)
					m_PowerSetting = 0;
				else if(m_PowerSetting >= HarvestAtmosphereCount())
					m_PowerSetting = HarvestAtmosphereCount()-1;
				QPixmap pxres(g_Au.Commodity(HarvestAtmosphereCommodity(m_PowerSetting)).BitmapName());
				ppower.drawPixmap(15,0, 14,14, pxres);

				QImage impower = bmpower.toImage();
				pTex->ReplaceSubImage(re, impower, 210,106);
			}
			else if(bayModule == CID_ProspectorBayModule)
			{
				powerSetting = 41;		// Harvesters fire all available units.

				// Commodity to Harvest Setting
				// Setting control is 41 pixels wide so it can visibly show a range from [0-40].
				QPixmap bmpower(":data/wc_HarvestSel.bmp");
				QPainter ppower(&bmpower);

				// Draw icon of current harvester commodity.
				if(m_PowerSetting < 0)
					m_PowerSetting = 0;
				else if(m_PowerSetting >= HarvestProspectorCount())
					m_PowerSetting = HarvestProspectorCount()-1;
				QPixmap pxres(g_Au.Commodity(HarvestProspectorCommodity(m_PowerSetting)).BitmapName());
				ppower.drawPixmap(15,0, 14,14, pxres);

				QImage impower = bmpower.toImage();
				pTex->ReplaceSubImage(re, impower, 210,106);
			}
			else if(bayModule == CID_SiphonBayModule)
			{
				powerSetting = 41;		// Harvesters fire all available units.

				// Commodity to Harvest Setting
				// Setting control is 41 pixels wide so it can visibly show a range from [0-40].
				QPixmap bmpower(":data/wc_HarvestSel.bmp");
				QPainter ppower(&bmpower);

				// Draw icon of current harvester commodity.
				if(m_PowerSetting < 0)
					m_PowerSetting = 0;
				else if(m_PowerSetting >= HarvestSiphonCount())
					m_PowerSetting = HarvestSiphonCount()-1;
				QPixmap pxres(g_Au.Commodity(HarvestSiphonCommodity(m_PowerSetting)).BitmapName());
				ppower.drawPixmap(15,0, 14,14, pxres);

				QImage impower = bmpower.toImage();
				pTex->ReplaceSubImage(re, impower, 210,106);
			}
			else if(bayModule == CID_WildcatterBayModule)
			{
				powerSetting = 41;		// Harvesters fire all available units.

				// Commodity to Harvest Setting
				// Setting control is 41 pixels wide so it can visibly show a range from [0-40].
				QPixmap bmpower(":data/wc_HarvestSel.bmp");
				QPainter ppower(&bmpower);

				// Draw icon of current harvester commodity.
				if(m_PowerSetting < 0)
					m_PowerSetting = 0;
				else if(m_PowerSetting >= HarvestWildcatterCount())
					m_PowerSetting = HarvestWildcatterCount()-1;
				QPixmap pxres(g_Au.Commodity(HarvestWildcatterCommodity(m_PowerSetting)).BitmapName());
				ppower.drawPixmap(15,0, 14,14, pxres);

				QImage impower = bmpower.toImage();
				pTex->ReplaceSubImage(re, impower, 210,106);
			}
			else if(bayModule == CID_ParticleCollectorBayModule)
			{
				powerSetting = 41;		// Harvesters fire all available units.

				// Commodity to Harvest Setting
				// Setting control is 41 pixels wide so it can visibly show a range from [0-40].
				QPixmap bmpower(":data/wc_HarvestSel.bmp");
				QPainter ppower(&bmpower);

				// Draw icon of current harvester commodity.
				if(m_PowerSetting < 0)
					m_PowerSetting = 0;
				else if(m_PowerSetting >= HarvestParticleCount())
					m_PowerSetting = HarvestParticleCount()-1;
				QPixmap pxres(g_Au.Commodity(HarvestParticleCommodity(m_PowerSetting)).BitmapName());
				ppower.drawPixmap(15,0, 14,14, pxres);

				QImage impower = bmpower.toImage();
				pTex->ReplaceSubImage(re, impower, 210,106);
			}
			else
			{
				// Fire Power Setting
				// Setting control is 41 pixels wide so it can visibly show a range from [0-40].
				QPixmap bmpower(":data/wc_FirePower.bmp");
				QPainter ppower(&bmpower);
				QPixmap bmpowerbug(":data/wc_FirePowerBug.png");
				ppower.drawPixmap(m_PowerSetting, 1, bmpowerbug);
				QImage impower = bmpower.toImage();
				pTex->ReplaceSubImage(re, impower, 210,106);
			}
		}

		// Capacitor
		if(m_Capacitor != m_lLastCapacitor || m_CapacitorTrend != m_lLastCapTrend || true)
		{
			m_lLastCapacitor = m_Capacitor;
			m_lLastCapTrend = m_CapacitorTrend;

			const int cwid = 64;	// pixels width of capacitor gauge
			QPixmap pmf(cwid,5);
			if(!pmf.isNull())
			{
				int fullto = (cwid * m_Capacitor + 99) / 100;	// Round up

				// Calculate capacitor needed to fire the weapon once at its current power setting.
				int firepx = 0;
				if(bayModule == CID_EnergyWeaponBayModule)
				{
					unsigned int unitsFired = BayWeaponProjectors(m_WeaponPower) * (powerSetting + 1)/41;
					if(!unitsFired)
						unitsFired = 1;
					if(m_CapacitorMax)
					{
						firepx = (int)ceil(unitsFired * cwid / (float)m_CapacitorMax);
						if(!firepx)
							firepx = 1;
					}
				}
				else if(DbCommodity::IsHarvesterModule(bayModule) ||
						bayModule == CID_TractorBeamBayModule)
				{
					unsigned int unitsFired = m_WeaponPower * (powerSetting + 1)/41;
					if(!unitsFired)
						unitsFired = 1;
					if(m_CapacitorMax)
					{
						firepx = (int)ceil(unitsFired * cwid / (float)m_CapacitorMax);
						if(!firepx)
							firepx = 1;
					}
				}

				QPainter p(&pmf);
				p.fillRect(0,0,cwid,5,Qt::black);
				if(firepx > 0)
				{
					// Weapon fire power usage is nice to see.
					// Firing requirement is a horizontal bar below capacitor
					// level in a different color.
					p.fillRect(0,2,firepx,3,QColor(128,128,255));
				}
				if(m_Capacitor > 0)
				{
					// Capacitor has power.
					if(m_CapacitorTrend > 0)
						p.fillRect(0,0,fullto,2,QColor(0,255,0));	// charging
					else if(m_CapacitorTrend < 0)
						p.fillRect(0,0,fullto,2,QColor(255,0,0));	// discharge
					else
						p.fillRect(0,0,fullto,2,QColor(255,202,0));	// neutral
				}
				QImage bmf = pmf.toImage();
				pTex->ReplaceSubImage(re, bmf, 189,120);
			}
		}

		// Blip Visibility by Political Stance
		{
			QImage bmbut;
			if(m_ShowEmpire)
				bmbut.load(":data/sc_EmpireOn.png");
			else
				bmbut.load(":data/sc_EmpireOff.png");
			pTex->ReplaceSubImage(re, bmbut, 75,120);

			if(m_ShowEnemy)
				bmbut.load(":data/sc_EnemyOn.png");
			else
				bmbut.load(":data/sc_EnemyOff.png");
			pTex->ReplaceSubImage(re, bmbut, 47,109);

			if(m_ShowFriend)
				bmbut.load(":data/sc_FriendOn.png");
			else
				bmbut.load(":data/sc_FriendOff.png");
			pTex->ReplaceSubImage(re, bmbut, 65,117);

			if(m_ShowNeutral)
				bmbut.load(":data/sc_NeutralOn.png");
			else
				bmbut.load(":data/sc_NeutralOff.png");
			pTex->ReplaceSubImage(re, bmbut, 56,113);
		}
	}

	// Must display this after the screen or it gets clipped.
	// Must always display this after the screen because it gets clipped.
	//if(m_WhichWeapon != m_lLastWhich)
	{
		m_lLastWhich = m_WhichWeapon;

		QPixmap pads(48, 16);
		pads.fill(QColor(0,0,0));
		QPainter p(&pads);
		p.setPen(QPen(QColor(0,64,128), 0.f));
		p.setFont(QFont(g_Au.TypefaceSans(), 10));
		QString text = QVariant(m_WhichWeapon+1).toString();
		p.drawText(QRect(0,0,48,16), Qt::AlignLeft|Qt::AlignTop, text);
		p.end();

		QImage im = pads.toImage();
		pTex->ReplaceSubImage(re, im, 14,3);
	}
}

void DpStationWeapon::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_WeaponConsoleControls.begin(); myci != s_WeaponConsoleControls.end(); myci++)
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

void DpStationWeapon::RestoreDefaultControls()
{
	s_WeaponConsoleControls.clear();
	for(int ic=0; ic<s_NumWeaponConsoleDefaults; ic++)
		s_WeaponConsoleControls.append(s_WeaponConsoleDefaults[ic]);
	SaveControls();
}

void DpStationWeapon::LoadControls() const
{
	// Init with my default controls.
	s_WeaponConsoleControls.clear();
	for(int ic=0; ic<s_NumWeaponConsoleDefaults; ic++)
		s_WeaponConsoleControls.append(s_WeaponConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_WeaponConsoleControls.LoadSettings("Controls/Weapon Console");
}

void DpStationWeapon::SaveControls() const
{
	// Store my controls in the settings file.
	s_WeaponConsoleControls.SaveSettings("Controls/Weapon Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationWeapon::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_WeaponConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_WeaponConsoleControls.cbegin();
			ci != s_WeaponConsoleControls.cend(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to a spacecraft console.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationWeapon::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	//qDebug("DpStationWeapon::ProcessControlInput");
	if(ship.IsNew())
		return;	// This console cannot be used when the ship is "nu".
	const double rRotInc = 5.0 * DEG2RAD;	// Rotation increment.
	COMMODITYID module = ship.SystemModule(SYS_Weapon, m_WhichWeapon);
	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch (ci.GetId())
		{
		case CI_EmpireToggle:
			if(cc != CONTROL_Client)
				m_ShowEmpire = !m_ShowEmpire;
			if(cc != CONTROL_Server)
			{
				float bp = m_ShowEmpire ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Stance, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_EnemyToggle:
			if(cc != CONTROL_Client)
				m_ShowEnemy = !m_ShowEnemy;
			if(cc != CONTROL_Server)
			{
				float bp = m_ShowEnemy ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Stance, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_FriendToggle:
			if(cc != CONTROL_Client)
				m_ShowFriend = !m_ShowFriend;
			if(cc != CONTROL_Server)
			{
				float bp = m_ShowFriend ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Stance, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_NeutralToggle:
			if(cc != CONTROL_Client)
				m_ShowNeutral = !m_ShowNeutral;
			if(cc != CONTROL_Server)
			{
				float bp = m_ShowNeutral ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Stance, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_Fire:
			if(cc != CONTROL_Client)
			{
				// A target must be selected.
				AuId selId;
				m_BlipLock.lockForRead();
				if(m_SelBlip)
					selId = m_SelBlip->AtomId();
				m_BlipLock.unlock();

				// Determine number of functioning weapon bay units.
				unsigned int hits = ship.GetPowerConsumption(SYS_Weapon, m_WhichWeapon);
				if(hits)
				{
					// Some units are functioning.
					const DaAtom* system = ship.FindEnvironmentAtom();
					const DaAtom* selAtom;
					AuPoint targetSysLoc;
					if(!selId.IsNull())
					{
						selAtom = system->FindIdRecursive(selId);
						if(selAtom)
							targetSysLoc = selAtom->WeaponLockLocation(sceneAtom->EnvLocation(), module);
					}
					else
						selAtom = 0;
					switch(module)
					{
					default:
						break;

					case CID_TractorBeamBayModule:
						ship.FireBayWeapon(*sceneAtom, m_WhichWeapon, hits, 0, selAtom, targetSysLoc);
						break;

					case CID_AtmosphereCondenserBayModule:
					case CID_ProspectorBayModule:
					case CID_SiphonBayModule:
					case CID_WildcatterBayModule:
					case CID_ParticleCollectorBayModule:
						{
							// Harvesters always fire all available units.
							// Harvesters send the selected commodity index. See global Harvest functions in audef.h
							int harvestIndex = ship.BayWeaponPowerSetting(m_WhichWeapon);
							ship.FireBayWeapon(*sceneAtom, m_WhichWeapon, hits, harvestIndex, selAtom, targetSysLoc);
						}
						break;

					case CID_BallisticWeaponBayModule:
						{
							unsigned int unitsFired = BayWeaponCannons(hits) *
								(ship.BayWeaponPowerSetting(m_WhichWeapon) + 1)/41;
							if(!unitsFired)
								unitsFired = 1;
							ship.FireBayWeapon(*sceneAtom, m_WhichWeapon, unitsFired, 0, selAtom, targetSysLoc);
						}
						break;

					case CID_EnergyWeaponBayModule:
						{
							unsigned int unitsFired = BayWeaponProjectors(hits) *
								(ship.BayWeaponPowerSetting(m_WhichWeapon) + 1)/41;
							if(!unitsFired)
								unitsFired = 1;
							ship.FireBayWeapon(*sceneAtom, m_WhichWeapon, unitsFired, 0, selAtom, targetSysLoc);
						}
						break;

					case CID_WeaponDeploymentBayModule:
						{
							unsigned int unitsFired = BayWeaponEjectorRacks(hits) *
								(ship.BayWeaponPowerSetting(m_WhichWeapon) + 1)/41;
							if(!unitsFired)
								unitsFired = 1;
							ship.FireBayWeapon(*sceneAtom, m_WhichWeapon, unitsFired, 0, selAtom, targetSysLoc);
						}
						break;

					case CID_WeaponLauncherBayModule:
						{
							unsigned int unitsFired = BayWeaponLaunchers(hits) *
								(ship.BayWeaponPowerSetting(m_WhichWeapon) + 1)/41;
							if(!unitsFired)
								unitsFired = 1;
							ship.FireBayWeapon(*sceneAtom, m_WhichWeapon, unitsFired, 0, selAtom, targetSysLoc);
						}
						break;
					}

					// Player Statistics
					if(selAtom)
					{
						sceneAtom->Stats().ItemUsedAsWeapon(CID_BayWeapon, ship.SystemQuality(SYS_Weapon, m_WhichWeapon));
						sceneAtom->IncreaseSkill(AuSkill::SpacecraftCombat, 1);
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Fire, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_PowerMax:
			if(cc != CONTROL_Client)
			{
				if(module == CID_AtmosphereCondenserBayModule ||
						module == CID_ProspectorBayModule ||
						module == CID_SiphonBayModule ||
						module == CID_WildcatterBayModule ||
						module == CID_ParticleCollectorBayModule)
				{
					// Increment by one because power setting repeats too fast for harvester.
					goto powerIncrease;
				}
				else if(module != CID_TractorBeamBayModule)	// Tractor beam does not respond to power setting changes.
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) != 40)
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, 40);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				float bp = .9f + 40/40.f;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerMin:
			if(cc != CONTROL_Client)
			{
				if(module == CID_AtmosphereCondenserBayModule ||
						module == CID_ProspectorBayModule ||
						module == CID_SiphonBayModule ||
						module == CID_WildcatterBayModule ||
						module == CID_ParticleCollectorBayModule)
				{
					// Decrement by one because power setting repeats too fast for harvester.
					goto powerDecrease;
				}
				else if(module != CID_TractorBeamBayModule)	// Tractor beam does not respond to power setting changes.
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) != 0)
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, 0);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				float bp = .9f + 0/40.f;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerDecrease:
powerDecrease:
			if(cc != CONTROL_Client)
			{
				if(module == CID_AtmosphereCondenserBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestAtmosphereCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, 0);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) > 0)
					{
						// Decrement by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_ProspectorBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestProspectorCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, 0);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) > 0)
					{
						// Decrement by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_SiphonBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestSiphonCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, 0);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) > 0)
					{
						// Decrement by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_WildcatterBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestWildcatterCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, 0);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) > 0)
					{
						// Decrement by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_ParticleCollectorBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestParticleCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, 0);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) > 0)
					{
						// Decrement by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module != CID_TractorBeamBayModule)	// Tractor beam does not respond to power setting changes.
				{
					// Minimum power setting is 0.
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) > 0)
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerIncrease:
powerIncrease:
			if(cc != CONTROL_Client)
			{
				if(module == CID_AtmosphereCondenserBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestAtmosphereCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, HarvestAtmosphereCount()-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) < HarvestAtmosphereCount()-1)
					{
						// Increment by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)+1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_ProspectorBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestProspectorCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, HarvestProspectorCount()-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) < HarvestProspectorCount()-1)
					{
						// Increment by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)+1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_SiphonBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestSiphonCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, HarvestSiphonCount()-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) < HarvestSiphonCount()-1)
					{
						// Increment by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)+1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_WildcatterBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestWildcatterCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, HarvestWildcatterCount()-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) < HarvestWildcatterCount()-1)
					{
						// Increment by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)+1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module == CID_ParticleCollectorBayModule)
				{
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) >= HarvestParticleCount())
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, HarvestParticleCount()-1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
					else if(ship.BayWeaponPowerSetting(m_WhichWeapon) < HarvestParticleCount()-1)
					{
						// Increment by one.
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)+1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
				else if(module != CID_TractorBeamBayModule)	// Tractor beam does not respond to power setting changes.
				{
					// Maximum power setting is 40.
					if(ship.BayWeaponPowerSetting(m_WhichWeapon) < 40)
					{
						ship.SetBayWeaponPowerSetting(m_WhichWeapon, ship.BayWeaponPowerSetting(m_WhichWeapon)+1);
						ship.SetModified(DaAtom::NOTIFY_Database);
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,1.1f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_PowerSetting0:
		case CI_PowerSetting1:
		case CI_PowerSetting2:
		case CI_PowerSetting3:
		case CI_PowerSetting4:
		case CI_PowerSetting5:
		case CI_PowerSetting6:
		case CI_PowerSetting7:
		case CI_PowerSetting8:
		case CI_PowerSetting9:
		case CI_PowerSetting10:
		case CI_PowerSetting11:
		case CI_PowerSetting12:
		case CI_PowerSetting13:
		case CI_PowerSetting14:
		case CI_PowerSetting15:
		case CI_PowerSetting16:
		case CI_PowerSetting17:
		case CI_PowerSetting18:
		case CI_PowerSetting19:
		case CI_PowerSetting20:
			if(cc != CONTROL_Client)
			{
				// First half of control is treated as decrement for harvester.
				if(module == CID_AtmosphereCondenserBayModule ||
						module == CID_ProspectorBayModule ||
						module == CID_SiphonBayModule ||
						module == CID_WildcatterBayModule ||
						module == CID_ParticleCollectorBayModule)
					goto powerDecrease;	// Decrement by one.
			}
			// Fall through, no break.
			[[fallthrough]];
		case CI_PowerSetting21:
		case CI_PowerSetting22:
		case CI_PowerSetting23:
		case CI_PowerSetting24:
		case CI_PowerSetting25:
		case CI_PowerSetting26:
		case CI_PowerSetting27:
		case CI_PowerSetting28:
		case CI_PowerSetting29:
		case CI_PowerSetting30:
		case CI_PowerSetting31:
		case CI_PowerSetting32:
		case CI_PowerSetting33:
		case CI_PowerSetting34:
		case CI_PowerSetting35:
		case CI_PowerSetting36:
		case CI_PowerSetting37:
		case CI_PowerSetting38:
		case CI_PowerSetting39:
		case CI_PowerSetting40:
			if(cc != CONTROL_Client)
			{
				// Tractor beam does not respond to power setting changes.
				if(module == CID_TractorBeamBayModule)
					break;

				// Second half of control is treated as increment for harvester.
				if(module == CID_AtmosphereCondenserBayModule ||
						module == CID_ProspectorBayModule ||
						module == CID_SiphonBayModule ||
						module == CID_WildcatterBayModule ||
						module == CID_ParticleCollectorBayModule)
					goto powerIncrease;	// Increment by one.

				// Select a specified power setting in the range 0-40.
				if(ship.BayWeaponPowerSetting(m_WhichWeapon) != ci.GetId() - CI_PowerSetting0)
				{
					ship.SetBayWeaponPowerSetting(m_WhichWeapon, ci.GetId() - CI_PowerSetting0);
					ship.SetModified(DaAtom::NOTIFY_Database);
				}
			}
			if(cc != CONTROL_Server)
			{
				// Tractor beam does not respond to power setting changes.
				if(module == CID_TractorBeamBayModule)
					break;

				// Second half of control is treated as increment for harvester.
				if(module == CID_AtmosphereCondenserBayModule ||
						module == CID_ProspectorBayModule ||
						module == CID_SiphonBayModule ||
						module == CID_WildcatterBayModule ||
						module == CID_ParticleCollectorBayModule)
					goto powerIncrease;	// Increment by one.

				float bp = .9f + (ci.GetId() - CI_PowerSetting0)/40.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_CitiesToggle:
			if(cc != CONTROL_Client)
				m_bCities = !m_bCities;
			if(cc != CONTROL_Server)
			{
				float bp = m_bCities ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Show, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PlanetsToggle:
			if(cc != CONTROL_Client)
				m_bPlanets = !m_bPlanets;
			if(cc != CONTROL_Server)
			{
				float bp = m_bPlanets ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Show, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ShipsToggle:
			if(cc != CONTROL_Client)
				m_bShips = !m_bShips;
			if(cc != CONTROL_Server)
			{
				float bp = m_bShips ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Show, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_StationsToggle:
			if(cc != CONTROL_Client)
				m_bStations = !m_bStations;
			if(cc != CONTROL_Server)
			{
				float bp = m_bStations ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Show, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_StarsToggle:
			if(cc != CONTROL_Client)
				m_bStars = !m_bStars;
			if(cc != CONTROL_Server)
			{
				float bp = m_bStars ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Show, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		// Bay selector
		case CI_NextBay:
			if(cc != CONTROL_Client)
				SelectNextBay(ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Bay, 1,1.05f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PreviousBay:
			if(cc != CONTROL_Client)
				SelectPreviousBay(ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Bay, 1,.95f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_RotateDown:
			if(cc != CONTROL_Client)
			{
				AuMatrix4x4 t;
				t.RotateX(-rRotInc);
				t.Rotate(m_fRot);
				AuPoint vx = t.Project(AuPoint(1,0));
				AuPoint vy = t.Project(AuPoint(0,1));
				m_fRot = AuPoint::CalculateRotation(vx, vy);
			}
			break;
		case CI_RotateLeft:
			if(cc != CONTROL_Client)
			{
				AuMatrix4x4 t;
				t.Rotate(m_fRot);
				t.RotateZ(rRotInc);
				AuPoint vx = t.Project(AuPoint(1,0));
				AuPoint vy = t.Project(AuPoint(0,1));
				m_fRot = AuPoint::CalculateRotation(vx, vy);
			}
			break;
		case CI_RotateRight:
			if(cc != CONTROL_Client)
			{
				AuMatrix4x4 t;
				t.Rotate(m_fRot);
				t.RotateZ(-rRotInc);
				AuPoint vx = t.Project(AuPoint(1,0));
				AuPoint vy = t.Project(AuPoint(0,1));
				m_fRot = AuPoint::CalculateRotation(vx, vy);
			}
			break;
		case CI_RotateUp:
			if(cc != CONTROL_Client)
			{
				AuMatrix4x4 t;
				t.RotateX(rRotInc);
				t.Rotate(m_fRot);
				AuPoint vx = t.Project(AuPoint(1,0));
				AuPoint vy = t.Project(AuPoint(0,1));
				m_fRot = AuPoint::CalculateRotation(vx, vy);
			}
			break;

		case CI_ZoomIn:
			if(cc != CONTROL_Client)
			{
				if(m_fScale >= 2.0)
				{
					m_fScale /= 2.0;
					if (fabs(m_fScale - FeetPerOrbit()) < 100.0)
						m_fScale = (float)FeetPerOrbit();
					else if (fabs(m_fScale - FeetPerAu()) < 1000.0)
						m_fScale = (float)FeetPerAu();
					if (m_fScale < 1.0)
						m_fScale = 1.0;
				}
			}
			break;
		case CI_ZoomOut:
			if(cc != CONTROL_Client)
			{
				if(m_fScale < FeetPerSolarSystem())
				{
					m_fScale *= 2.0;
					if (fabs(m_fScale - FeetPerOrbit()) < 100.0)
						m_fScale = (float)FeetPerOrbit();
					else if (fabs(m_fScale - FeetPerAu()) < 1000.0)
						m_fScale = (float)FeetPerAu();
				}
			}
			break;
		case CI_ZoomNormal:
			if(cc != CONTROL_Client)
			{
				if(m_fScale != (float)FeetPerOrbit())
					m_fScale = (float)FeetPerOrbit();
			}
			break;

		case CI_RotateTopView:
			if(cc != CONTROL_Client)
				m_fRot = AuPoint(0,0,0);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_RotateToTarget:
			if(cc != CONTROL_Client)
			{
				bool hasLoc;
				AuPoint selLoc;
				m_BlipLock.lockForRead();
				if(m_SelBlip)
				{
					selLoc = m_SelBlip->EnvLocation();
					hasLoc = true;
				}
				else
					hasLoc = false;
				m_BlipLock.unlock();

				if(hasLoc)
				{
					AuPoint vecToTarget = ship.EnvTransform().UnProject(selLoc);
					vecToTarget.Unit();
					AuMatrix4x4 tAlign = vecToTarget.GetAlignmentTransform(AuPoint(0,0,-1));
					AuPoint vecUp = tAlign.ProjectVector(AuPoint(0,0,1));
					if(!vecUp.IsFuzzyZero_2D())
					{
						double zrot = PI/2 - vecUp.ArcTangent_2D();
						AuMatrix4x4 tz;
						tz.RotateZ(zrot);
						tAlign = tz * tAlign;
					}
					m_fRot = tAlign.GetRotation();
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_DisplayScreen:
			if(cc != CONTROL_Client)
			{
				AuId selId;
				m_BlipLock.lockForRead();
				if(m_SelBlip)
					selId = m_SelBlip->AtomId();
				m_BlipLock.unlock();

				ProcessDisplayHit(ci.HitLocation(), ship.EnvTransform().Inverse(), ship.FindEnvironmentAtom());

				AuId nuSelId;
				m_BlipLock.lockForRead();
				if(m_SelBlip)
					nuSelId = m_SelBlip->AtomId();
				m_BlipLock.unlock();

				if(selId != nuSelId)
				{
					ship.SetBayWeaponTarget(m_WhichWeapon, nuSelId);
					ship.SetModified(DaAtom::NOTIFY_Database);
				}
			}
			break;
		}
	}
}

void DpStationWeapon::ProcessDisplayHit(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, const DaAtom* system)
{
	if(!TestInsideScreenMask(pxHit))
		return;
	//qDebug("DpStationWeapon Hit at %d,%d", pxHit.x(), pxHit.y());
	SelectNearest(pxHit, tRelBlips, system);
}

void DpStationWeapon::SelectNearest(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, const DaAtom* system)
{
	const double rseldist = 5.0;	// Pixel distance to select.

	m_BlipLock.lockForWrite();
	if(m_SelBlip)
		delete m_SelBlip;

	// Find the closest atom in the display.
	double radist = -1.0;	// Distance to the nearest atom in display pixels.
	AuSensorBlipList::const_iterator blip = FindClosestDisplayBlip(pxHit, radist, tRelBlips, system);
	if(blip == m_SensorBlips.end() || radist > rseldist)
		m_SelBlip = 0;
	else
		m_SelBlip = (*blip)->Clone();
	m_BlipLock.unlock();
}

AuSensorBlipList::const_iterator DpStationWeapon::FindClosestDisplayBlip(const QPoint& pxHit, double& rDisplayDistance,
		const AuMatrix4x4& tRelBlips, const DaAtom* system) const
{
	QSize sz(227,119);	// Hard coded "known" size saves from loading the bitmap to measure it.

	// Make transforms to project the locations into the bitmap.
	AuMatrix4x4 tdev;
	tdev.Translate(sz.width()/2.0, sz.height()/2.0, 0.0);
	double rpxscl = sz.height()/2.0;
	tdev.Scale(rpxscl, -rpxscl, rpxscl);	// Flip Y
//			tdev.Perspective(60.0, 1.0, 0.5, 1000.0);
//			tdev.Translate(0.0, 0.0, -1.0);

	AuMatrix4x4 trot;
	trot.Rotate(m_fRot);
	trot.Scale(1.0/m_fScale, 1.0/m_fScale, 1.0/m_fScale);

	AuMatrix4x4 tcomb = tdev * trot * tRelBlips;

	// Calc distance to blips.
	float rFeetPerPixel = m_fScale/(sz.height()/2);
	AuSensorBlipList::const_iterator pbest = m_SensorBlips.end();
	for(AuSensorBlipList::const_iterator pb = m_SensorBlips.begin(); pb != m_SensorBlips.end(); pb++)
	{
		// Filter based on political stance.
		if(!(*pb)->TestStance(m_ShowEmpire, m_ShowFriend, m_ShowNeutral, m_ShowEnemy, true))
			continue;

		if((*pb)->IsCity() || (*pb)->IsBuilding())
		{
			if(!m_bCities)
				continue;
		}
		else if((*pb)->IsWorld())
		{
			if(!m_bPlanets)
				continue;
		}
		else if((*pb)->IsSpaceStation())
		{
			if(!m_bStations)
				continue;
		}
		else if((*pb)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*pb)->IsSpacecraft())
		{
			if(!m_bShips)
				continue;
		}
		else if((*pb)->IsStar())
		{
			if(!m_bStars)
				continue;
		}
		else if((*pb)->IsAsteroid())
		{
		}
		else
		{
			continue;
		}

		// Calc distance to the blip.
		double rdist = (*pb)->DisplayDistance(tcomb, pxHit, system, rFeetPerPixel);
		if(pbest == m_SensorBlips.end() || rdist < rDisplayDistance)
		{
			pbest = pb;
			rDisplayDistance = rdist;
		}
	}

	return pbest;
}

// This is how mouse control inputs activate controls in the control input
// array.
// Controls in aActiveControls are activated accordingly.
// Returns true if the control panel should be selected as a result of
// processing this input.
// Some panels don't need to be selected and can simply react to the input,
// such as a light switch.
void DpStationWeapon::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
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
	// the iy we've calculated that is also
	// measured from the bottom, not the top as usual in GDI.
	DiPanelControl acontrols[] =
	{
		DiPanelControl(CI_ShipsToggle, QRect(4,107,16,16)),
		DiPanelControl(CI_StationsToggle, QRect(4,90,16,16)),
		DiPanelControl(CI_CitiesToggle, QRect(4,73,16,16)),
		DiPanelControl(CI_PlanetsToggle, QRect(4,56,16,16)),
		DiPanelControl(CI_StarsToggle, QRect(4,39,16,16)),

		DiPanelControl(CI_EmpireToggle, QRect(75,120,8,4)),
		DiPanelControl(CI_EnemyToggle, QRect(47,109,8,4)),
		DiPanelControl(CI_FriendToggle, QRect(65,117,8,4)),
		DiPanelControl(CI_NeutralToggle, QRect(56,113,8,4)),

		DiPanelControl(CI_RotateUp, QRect(238,20,3,3), true),
		DiPanelControl(CI_RotateDown, QRect(238,8,3,3), true),
		DiPanelControl(CI_RotateLeft, QRect(232,14,3,3), true),
		DiPanelControl(CI_RotateRight, QRect(244,14,3,3), true),

		DiPanelControl(CI_ZoomOut, QRect(211,8,3,3)),
		DiPanelControl(CI_ZoomIn, QRect(223,8,3,3)),
		DiPanelControl(CI_ZoomNormal, QRect(217,8,3,3)),

		DiPanelControl(CI_RotateTopView, QRect(238,14,3,3)),
		DiPanelControl(CI_RotateToTarget, QRect(245,28,7,7)),

		DiPanelControl(CI_PreviousBay, QRect(4,4, 9,9)),
		DiPanelControl(CI_NextBay, QRect(4,14, 9,9)),
		DiPanelControl(CI_Fire, QRect(239,92,14,14)),

		DiPanelControl(CI_PowerSetting0, QRect(210,106,2,14)),
		DiPanelControl(CI_PowerSetting1, QRect(212,106,1,14)),
		DiPanelControl(CI_PowerSetting2, QRect(213,106,1,14)),
		DiPanelControl(CI_PowerSetting3, QRect(214,106,1,14)),
		DiPanelControl(CI_PowerSetting4, QRect(215,106,1,14)),
		DiPanelControl(CI_PowerSetting5, QRect(216,106,1,14)),
		DiPanelControl(CI_PowerSetting6, QRect(217,106,1,14)),
		DiPanelControl(CI_PowerSetting7, QRect(218,106,1,14)),
		DiPanelControl(CI_PowerSetting8, QRect(219,106,1,14)),
		DiPanelControl(CI_PowerSetting9, QRect(220,106,1,14)),
		DiPanelControl(CI_PowerSetting10, QRect(221,106,1,14)),
		DiPanelControl(CI_PowerSetting11, QRect(222,106,1,14)),
		DiPanelControl(CI_PowerSetting12, QRect(223,106,1,14)),
		DiPanelControl(CI_PowerSetting13, QRect(224,106,1,14)),
		DiPanelControl(CI_PowerSetting14, QRect(225,106,1,14)),
		DiPanelControl(CI_PowerSetting15, QRect(226,106,1,14)),
		DiPanelControl(CI_PowerSetting16, QRect(227,106,1,14)),
		DiPanelControl(CI_PowerSetting17, QRect(228,106,1,14)),
		DiPanelControl(CI_PowerSetting18, QRect(229,106,1,14)),
		DiPanelControl(CI_PowerSetting19, QRect(230,106,1,14)),
		DiPanelControl(CI_PowerSetting20, QRect(231,106,1,14)),
		DiPanelControl(CI_PowerSetting21, QRect(232,106,1,14)),
		DiPanelControl(CI_PowerSetting22, QRect(233,106,1,14)),
		DiPanelControl(CI_PowerSetting23, QRect(234,106,1,14)),
		DiPanelControl(CI_PowerSetting24, QRect(235,106,1,14)),
		DiPanelControl(CI_PowerSetting25, QRect(236,106,1,14)),
		DiPanelControl(CI_PowerSetting26, QRect(237,106,1,14)),
		DiPanelControl(CI_PowerSetting27, QRect(238,106,1,14)),
		DiPanelControl(CI_PowerSetting28, QRect(239,106,1,14)),
		DiPanelControl(CI_PowerSetting29, QRect(240,106,1,14)),
		DiPanelControl(CI_PowerSetting30, QRect(241,106,1,14)),
		DiPanelControl(CI_PowerSetting31, QRect(242,106,1,14)),
		DiPanelControl(CI_PowerSetting32, QRect(243,106,1,14)),
		DiPanelControl(CI_PowerSetting33, QRect(244,106,1,14)),
		DiPanelControl(CI_PowerSetting34, QRect(245,106,1,14)),
		DiPanelControl(CI_PowerSetting35, QRect(246,106,1,14)),
		DiPanelControl(CI_PowerSetting36, QRect(247,106,1,14)),
		DiPanelControl(CI_PowerSetting37, QRect(248,106,1,14)),
		DiPanelControl(CI_PowerSetting38, QRect(249,106,1,14)),
		DiPanelControl(CI_PowerSetting39, QRect(250,106,1,14)),
		DiPanelControl(CI_PowerSetting40, QRect(251,106,2,14)),

		// NOTE: The display screen is last so previous (possibly overlapping)
		// buttons get first crack at mouse clicks.
		DiPanelControl(CI_DisplayScreen, QRect(24,5,227,119), false, true),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);

	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

// Returns true if a pixel point is within the "screen" area of the sensor
// panel.
bool DpStationWeapon::TestInsideScreenMask(const QPoint &pxTest) const
{
	bool bInScreen = false;
	QImage im(":data/sc_ScreenMask.bmp");
	if(!im.isNull())
	{
		if(pxTest.x() >= 0 && pxTest.x() < im.width() &&
				pxTest.y() >= 0 && pxTest.y() < im.height())
		{
			QRgb rgb = im.pixel(pxTest.x(), pxTest.y());
			QColor col(rgb);
			if(col == Qt::white)
				bInScreen = true;
		}
	}
	return bInScreen;
}

void DpStationWeapon::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStation::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
	if(seatedCreature && seatedCreature->Is(sceneAtom.Id()))
	{
		RenderTarget(tModel, sceneAtom, ari);
		if(!m_idLockedBlip.IsNull())
		{
			AuSensorBlipList::const_iterator blip = m_SensorBlips.Find(m_idLockedBlip);
			if(blip != m_SensorBlips.end())
				RenderSensorLock((*blip)->AtomId(), (*blip)->PoliticalStance(), (*blip)->IsCivilian(), (*blip)->EnvLocation(),
						tModel, sceneAtom, ari);
		}
	}
}

bool DpStationWeapon::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
	if(ship)
	{
		if(ship->IsDesignDataCacheValid())
		{
			m_Capacitor = ship->CapacitorPercent();
			m_CapacitorMax = (quint32)ship->CapacitorMax();
			m_CapacitorTrend = ship->CapacitorTrend();
			m_PowerSetting = ship->BayWeaponPowerSetting(m_WhichWeapon);
			m_SensorRange = ship->SensorRange();
			m_WeaponPower = ship->GetPowerConsumption(SYS_Weapon, m_WhichWeapon);

			m_OrdnanceLock.lockForWrite();
			if(m_Ordnance)
				delete m_Ordnance;
			const AuItem* ammo = ship->LockBayWeaponAmmunition(m_WhichWeapon);
			if(ammo)
				m_Ordnance = ammo->Clone();
			else
				m_Ordnance = 0;
			ship->UnlockBayWeaponAmmunition();
			m_OrdnanceLock.unlock();

			// The ship provides me with a list of the sensor console blips that are in range.
			m_idLockedBlip = ship->LockedBlip();
			if(!last.SameStartSec(now))
			{
				m_BlipLock.lockForWrite();
				m_SensorBlips.Clear();
				if(ship->SensorsWorking())
				{
					AuPoint shipSysLoc = ship->EnvLocation();
					ship->PerformFireControlConsoleScan(m_SensorBlips);
					AuSensorBlipList::iterator sbi = m_SensorBlips.begin();
					while(sbi != m_SensorBlips.end())
					{
						if(!(*sbi)->IsRangeLimited() || (*sbi)->Range(shipSysLoc) < m_SensorRange)
							sbi++;	// Keep the blip.
						else
						{
							// Remove unneeded blip.
							delete *sbi;
							sbi = m_SensorBlips.erase(sbi);
						}
					}
				}
				m_BlipLock.unlock();
			}

			m_BlipLock.lockForWrite();
			if(m_SelBlip)
				delete m_SelBlip;
			AuSensorBlipList::const_iterator sbi = m_SensorBlips.Find(ship->BayWeaponTarget(m_WhichWeapon));
			if(sbi != m_SensorBlips.end())
				m_SelBlip = (*sbi)->Clone();
			else
				m_SelBlip = 0;
			m_BlipLock.unlock();

			return true;
		}
	}
	return false;
}

void DpStationWeapon::RenderTarget(const AuMatrix4x4& tModel, const DaCreature& sceneAtom, AuAtomRenderInfo& ari) const
{
	if(sceneAtom.IsMini())
		return;
	if(!ari.m_ShowHUD)
		return;
	const DaVessel* ship = sceneAtom.FindParentVesselInEnv();
	if(!ship)
		return;
	if(!m_SelBlip)
		return;
	const DaAtom* system = ship->FindEnvironmentAtom();
	if(!system)
		return;

	// Get target lock location in solar system space.
	COMMODITYID bayModule = ship->SystemModule(SYS_Weapon, m_WhichWeapon);
	AuPoint fLockLoc = m_SelBlip->WeaponLockLocation(system, ship->EnvLocation(), bayModule);
	double targDist = fLockLoc.Distance(sceneAtom.EnvLocation());
	float targMeters = round(targDist / FeetPerMeter());

	// Determine power output of the weapon so the blip display can show the amount of energy expected to hit the target.
	// Decide what to show on the crosshair.
	QString sdist;
	switch(bayModule)
	{
	default:
		break;
	case CID_BallisticWeaponBayModule:
		{
			// Due to network latency, the blip appears to jitter if we simply use the location received.
			// If we can locate the locked atom, we can be really precise when it really matters,
			// as in when the atom is in the scene.
			const DaAtom* lockedAtom = system->FindIdRecursive(m_SelBlip->AtomId());
			if(lockedAtom)
			{
				fLockLoc = lockedAtom->EnvLocation();
				targDist = fLockLoc.Distance(sceneAtom.EnvLocation());
				targMeters = round(targDist / FeetPerMeter());
			}

			unsigned int unitsFired = BayWeaponCannons(m_WeaponPower) * (m_PowerSetting + 1)/41;
			if(!unitsFired)
				unitsFired = 1;

			// The returned ammunition has the count set to the number available in the hold.
			const AuItem* ammoSel = sceneAtom.Ordnance();
			if(ammoSel)
			{
				float wpnRange = ship->BayWeaponRange(m_WhichWeapon);
				if(targDist < wpnRange)
				{
					unsigned int maxRounds = ammoSel->Count();
					unsigned int targetRounds;
					if(unitsFired > maxRounds)
						targetRounds = maxRounds;
					else
						targetRounds = unitsFired;
					sdist = QString("%L1m %L2 rounds").arg(targMeters, 0,'f',0).arg(targetRounds);
				}
				else
					sdist = QString("%L1m\nToo Far\nRange %L2m").arg(targMeters, 0,'f',0).arg(wpnRange/FeetPerMeter(), 0,'f',0);

			}
		}
		break;
	case CID_EnergyWeaponBayModule:
		{
			// Due to network latency, the blip appears to jitter if we simply use the location received.
			// If we can locate the locked atom, we can be really precise when it really matters,
			// as in when the atom is in the scene.
			const DaAtom* lockedAtom = system->FindIdRecursive(m_SelBlip->AtomId());
			if(lockedAtom)
			{
				fLockLoc = lockedAtom->EnvLocation();
				targDist = fLockLoc.Distance(sceneAtom.EnvLocation());
				targMeters = round(targDist / FeetPerMeter());
			}

			unsigned int unitsFired = BayWeaponProjectors(m_WeaponPower) * (m_PowerSetting + 1)/41;
			if(!unitsFired)
				unitsFired = 1;

			// The returned ammunition has the count set to the number available in the hold.
			const AuItem* ammoSel = sceneAtom.Ordnance();
			if(ammoSel)
			{
				float wpnRange = ship->BayWeaponRange(m_WhichWeapon);
				if(targDist < wpnRange)
				{
					unsigned int maxRounds = ammoSel->Count();
					if(maxRounds > ship->CapacitorLevel())
						maxRounds = ship->CapacitorLevel();
					unsigned int targetRounds;
					if(unitsFired > maxRounds)
						targetRounds = maxRounds;
					else
						targetRounds = unitsFired;
					sdist = QString("%L1m %L2 beams").arg(targMeters, 0,'f',0).arg(targetRounds);
				}
				else
					sdist = QString("%L1m\nToo Far\nRange %L2m").arg(targMeters, 0,'f',0).arg(wpnRange/FeetPerMeter(), 0,'f',0);
			}
		}
		break;
	case CID_WeaponDeploymentBayModule:
		{
			// Due to network latency, the blip appears to jitter if we simply use the location received.
			// If we can locate the locked atom, we can be really precise when it really matters,
			// as in when the atom is in the scene.
			const DaAtom* lockedAtom = system->FindIdRecursive(m_SelBlip->AtomId());
			if(lockedAtom)
			{
				fLockLoc = lockedAtom->EnvLocation();
				targDist = fLockLoc.Distance(sceneAtom.EnvLocation());
				targMeters = round(targDist / FeetPerMeter());
			}

			unsigned int unitsFired = BayWeaponEjectorRacks(m_WeaponPower) * (m_PowerSetting + 1)/41;
			if(!unitsFired)
				unitsFired = 1;

			// The returned ammunition has the count set to the number available in the hold.
			const AuItem* ammoSel = sceneAtom.Ordnance();
			if(ammoSel)
			{
				float wpnRange = ship->BayWeaponRange(m_WhichWeapon);
				if(targDist < wpnRange)
				{
					unsigned int maxRounds = ammoSel->Count();
					unsigned int targetRounds;
					if(unitsFired > maxRounds)
						targetRounds = maxRounds;
					else
						targetRounds = unitsFired;
					sdist = QString("%L1m %L2 bombs").arg(targMeters, 0,'f',0).arg(targetRounds);
				}
				else
					sdist = QString("%L1m\nToo Far\nRange %L2m").arg(targMeters, 0,'f',0).arg(wpnRange/FeetPerMeter(), 0,'f',0);
			}
		}
		break;
	case CID_WeaponLauncherBayModule:
		{
			// Due to network latency, the blip appears to jitter if we simply use the location received.
			// If we can locate the locked atom, we can be really precise when it really matters,
			// as in when the atom is in the scene.
			const DaAtom* lockedAtom = system->FindIdRecursive(m_SelBlip->AtomId());
			if(lockedAtom)
			{
				fLockLoc = lockedAtom->EnvLocation();
				targDist = fLockLoc.Distance(sceneAtom.EnvLocation());
				targMeters = round(targDist / FeetPerMeter());
			}

			unsigned int unitsFired = BayWeaponLaunchers(m_WeaponPower) * (m_PowerSetting + 1)/41;
			if(!unitsFired)
				unitsFired = 1;

			// The returned ammunition has the count set to the number available in the hold.
			const AuItem* ammoSel = sceneAtom.Ordnance();
			if(ammoSel)
			{
				float wpnRange = ship->BayWeaponRange(m_WhichWeapon);
				if(targDist < wpnRange)
				{
					unsigned int maxRounds = ammoSel->Count();
					unsigned int targetRounds;
					if(unitsFired > maxRounds)
						targetRounds = maxRounds;
					else
						targetRounds = unitsFired;

					if(DbCommodity::IsMissile(ammoSel->CommodityId()))
						sdist = QString("%L1m %L2 missiles").arg(targMeters, 0,'f',0).arg(targetRounds);
					else
						sdist = QString("%L1m %L2 rockets").arg(targMeters, 0,'f',0).arg(targetRounds);
				}
				else
					sdist = QString("%L1m\nToo Far\nRange %L2m").arg(targMeters, 0,'f',0).arg(wpnRange/FeetPerMeter(), 0,'f',0);
			}
		}
		break;

	case CID_AtmosphereCondenserBayModule:
	case CID_ParticleCollectorBayModule:
	case CID_ProspectorBayModule:
	case CID_SiphonBayModule:
	case CID_WildcatterBayModule:
		{
			// Show target distance and harvester probability info.
			// Calculate amount of energy to reach the target.
			unsigned int unitsFired = m_WeaponPower;	// Harvesters always fire at full power.
			if(!unitsFired)
				unitsFired = 1;
			float wpnRange = ship->HarvesterBayRange(unitsFired, m_WhichWeapon);
			if(wpnRange > 0 && targDist < wpnRange)
			{
				float dropOff = 1.f - targDist / wpnRange;
				float wpnPower = ship->BayWeaponPower(unitsFired, m_WhichWeapon);
				float targetE = round(wpnPower * dropOff);

				// Harvest up to 1 unit for every fixed quantity of energy that hits target.
				unsigned int harvestUnits = (unsigned int)(targetE / HarvestUnitsPerEnergyUnit());
				sdist = QString("%L1m %L2u").arg(targMeters, 0,'f',0).arg(harvestUnits);
			}
			else
				sdist = QString("%L1m\nToo Far\nRange %L2m").arg(targMeters, 0,'f',0).arg(wpnRange/FeetPerMeter(), 0,'f',0);
		}
		break;

	case CID_TractorBeamBayModule:
		{
			// Show target distance and tractor attach info.
			const DaAtom* lockedAtom = system->FindIdRecursive(m_SelBlip->AtomId());
			if(!lockedAtom)
				break;

			// Calculate range of tractor beam.
			unsigned int unitsFired = m_WeaponPower;	// Tractor beams always fire at full power.
			if(!unitsFired)
				unitsFired = 1;
			float wpnRange = ship->TractorBeamRange(unitsFired, m_WhichWeapon);

			// Calculate amount of acceleration that can be applied to the target.
			quint8 bayQ = ship->SystemQuality(SYS_Weapon, m_WhichWeapon);
			const float tlBoost = QualityBoostF(bayQ);
			float tractorAccel = (float)unitsFired * GravityDriveForce() / lockedAtom->Mass() * tlBoost;
			tractorAccel /= (float)FeetPerMeter();
			tractorAccel = round(tractorAccel);

			if(wpnRange > 0 && targDist < wpnRange)
				sdist = QString("%L1m In Range\n%L2m/sec/sec").arg(targMeters, 0,'f',0).arg(tractorAccel, 0,'f',0);
			else
				sdist = QString("%L1m Too Far\nRange %L3m\n%L2m/sec/sec").arg(targMeters, 0,'f',0)
					.arg(wpnRange/FeetPerMeter(), 0,'f',0)
					.arg(tractorAccel, 0,'f',0);
		}
		break;
	}
	
	if(sdist.isEmpty())
	{
		// Show target distance on the crosshair.
		sdist = QString("%L1m").arg(targMeters, 0,'f',0);
	}

	// Show reticle on "lock-on" object.
	QRect rcClient = ari.glw->rect();
	double windowAspect = AuGLEffects::WindowAspectRatio(rcClient);
	double sceneAspect = AuGLEffects::WindowAspectRatio(rcClient);
	double fovy = AuGLEffects::WindowFieldOfViewY(sceneAspect, g_Au.FieldOfViewY());

	// Create perspective camera transformations the way AuView does it.
	AuMatrix4x4 tP;
	tP.Perspective(fovy, 1.0, 3.0, FeetPerSector());

	fLockLoc = sceneAtom.EnvTransform().UnProject(fLockLoc);	// now in scene atom space
	fLockLoc = tModel.Project(fLockLoc);	// into eye space
	bool behind = false;
	if(fLockLoc.z > 0.)
		behind = true;

	// sensor lock reticle
	fLockLoc = tP.Project(fLockLoc);
	fLockLoc.x *= sceneAspect / windowAspect;
	const double ymax = 0.98;
	const double xmax = ymax/windowAspect;
	if(behind)
	{
		// Behind viewer.
		// X is reversed.
		if(fLockLoc.x > 0)
			fLockLoc.x = -xmax;
		else
			fLockLoc.x = xmax;
		fLockLoc.y = 0.;
	}
	if(fLockLoc.x < -xmax)
		fLockLoc.x = -xmax;
	else if(fLockLoc.x > xmax)
		fLockLoc.x = xmax;
	if(fLockLoc.y < -ymax)
		fLockLoc.y = -ymax;
	else if(fLockLoc.y > ymax)
		fLockLoc.y = ymax;

	ari.re->glDisable(GL_DEPTH_TEST);
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	QColor tcolor;
	if(behind)
	{
		GLuint tex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,
				"TargetLockBehind", ":data/txTargetLockOnBehind.png");
		ari.re->RenderSprite(rcClient, fLockLoc.x,fLockLoc.y, .08f,.08f, tex, 1,1,1,1);

		// Prepare for text.
		fLockLoc.x += .05;
		tcolor.setRgbF(1, .5f, .2f);	// Text color for distance readout.
	}
	else
	{
		GLuint tex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,
				"TargetLock", ":data/txTargetLockOn.png");
		ari.re->RenderSprite(rcClient, fLockLoc.x,fLockLoc.y, .1f,.1f, tex, 1,1,1,1);

		// Prepare for text.
		fLockLoc.x += .06;
		tcolor.setRgbF(.2f, 1, 1);	// Text color for distance readout.
	}
	ari.re->glEnable(GL_DEPTH_TEST);

	// Draw the text on the screen, not at Z-depth.
	// Calculate pixel coordinate of text relative to crosshair.
	int tposx = (int)((fLockLoc.x*windowAspect + 1.)/2. * ari.glw->width());
	int tposy = ari.glw->height() - (int)((fLockLoc.y + 1.)/2. * ari.glw->height());
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	ari.RenderText(tposx,tposy, sdist, tcolor, Qt::AlignLeft|Qt::AlignVCenter);
	ari.re->glDisable(GL_BLEND);
}

void DpStationWeapon::SelectNextBay(const DaVessel& ship)
{
	if(ship.WeaponBays())
		m_WhichWeapon = ship.WeaponBayNextId(m_WhichWeapon);
}

void DpStationWeapon::SelectPreviousBay(const DaVessel& ship)
{
	if(ship.WeaponBays())
		m_WhichWeapon = ship.WeaponBayPreviousId(m_WhichWeapon);
}

void DpStationWeapon::LoadClientSettings(const AuId& /*planetId*/)
{
	m_bCities = g_Au.Settings().readBoolEntry("/FireControlStation/CitiesOn", m_bCities);
	m_bPlanets = g_Au.Settings().readBoolEntry("/FireControlStation/PlanetsOn", m_bPlanets);
	m_bShips = g_Au.Settings().readBoolEntry("/FireControlStation/ShipsOn", m_bShips);
	m_bStations = g_Au.Settings().readBoolEntry("/FireControlStation/StationsOn", m_bStations);
	m_bStars = g_Au.Settings().readBoolEntry("/FireControlStation/StarsOn", m_bStars);
	m_fRot.x = g_Au.Settings().readNumEntry("/FireControlStation/XRotation", m_fRot.x);
	m_fRot.y = g_Au.Settings().readNumEntry("/FireControlStation/YRotation", m_fRot.y);
	m_fRot.z = g_Au.Settings().readNumEntry("/FireControlStation/ZRotation", m_fRot.z);
	m_fScale = g_Au.Settings().readNumEntry("/FireControlStation/Scale", m_fScale);
	if(m_fScale < 1 || m_fScale > FeetPerSolarSystem())
		m_fScale = FeetPerOrbit();
	m_ShowEmpire = g_Au.Settings().readBoolEntry("/FireControlStation/ShowEmpire", m_ShowEmpire);
	m_ShowEnemy = g_Au.Settings().readBoolEntry("/FireControlStation/ShowEnemy", m_ShowEnemy);
	m_ShowFriend = g_Au.Settings().readBoolEntry("/FireControlStation/ShowFriend", m_ShowFriend);
	m_ShowNeutral = g_Au.Settings().readBoolEntry("/FireControlStation/ShowNeutral", m_ShowNeutral);
}

void DpStationWeapon::StoreClientSettings(const AuId& /*planetId*/) const
{
	g_Au.Settings().writeEntry("/FireControlStation/CitiesOn", m_bCities);
	g_Au.Settings().writeEntry("/FireControlStation/PlanetsOn", m_bPlanets);
	g_Au.Settings().writeEntry("/FireControlStation/ShipsOn", m_bShips);
	g_Au.Settings().writeEntry("/FireControlStation/StationsOn", m_bStations);
	g_Au.Settings().writeEntry("/FireControlStation/StarsOn", m_bStars);
	g_Au.Settings().writeEntry("/FireControlStation/XRotation", m_fRot.x);
	g_Au.Settings().writeEntry("/FireControlStation/YRotation", m_fRot.y);
	g_Au.Settings().writeEntry("/FireControlStation/ZRotation", m_fRot.z);
	g_Au.Settings().writeEntry("/FireControlStation/Scale", m_fScale);
	g_Au.Settings().writeEntry("/FireControlStation/ShowEmpire", m_ShowEmpire);
	g_Au.Settings().writeEntry("/FireControlStation/ShowEnemy", m_ShowEnemy);
	g_Au.Settings().writeEntry("/FireControlStation/ShowFriend", m_ShowFriend);
	g_Au.Settings().writeEntry("/FireControlStation/ShowNeutral", m_ShowNeutral);
}

void DpStationWeapon::TxSettings(QDataStream& out) const
{
	quint16 bits = 0;
	if(m_bCities)
		bits |= 0x0001;
	if(m_bPlanets)
		bits |= 0x0002;
	if(m_bShips)
		bits |= 0x0004;
	if(m_bStations)
		bits |= 0x0010;
	if(m_bStars)
		bits |= 0x0020;
	if(m_ShowEmpire)
		bits |= 0x1000;
	if(m_ShowFriend)
		bits |= 0x2000;
	if(m_ShowNeutral)
		bits |= 0x4000;
	if(m_ShowEnemy)
		bits |= 0x8000;
	out << bits;

	m_BlipLock.lockForRead();
	if(m_SelBlip)
	{
		out << (quint8)m_SelBlip->Type();
		out << *m_SelBlip;
	}
	else
		out << (quint8)AuSensorBlip::Invalid;
	m_BlipLock.unlock();

	out << (float)m_fRot.x;
	out << (float)m_fRot.y;
	out << (float)m_fRot.z;
	out << m_fScale;
}

void DpStationWeapon::RxSettings(QDataStream& in)
{
	quint16 bits;
	in >> bits;
	m_bCities = (bits & 0x0001) ? true : false;
	m_bPlanets = (bits & 0x0002) ? true : false;
	m_bShips = (bits & 0x0004) ? true : false;
	m_bStations = (bits & 0x0010) ? true : false;
	m_bStars = (bits & 0x0020) ? true : false;
	m_ShowEmpire = (bits & 0x1000) ? true : false;
	m_ShowFriend = (bits & 0x2000) ? true : false;
	m_ShowNeutral = (bits & 0x4000) ? true : false;
	m_ShowEnemy = (bits & 0x8000) ? true : false;

	m_BlipLock.lockForWrite();
	if(m_SelBlip)
		delete m_SelBlip;
	quint8 blipType;
	in >> blipType;
	m_SelBlip = AuSensorBlip::Create((AuSensorBlip::TYPE)blipType);
	if(m_SelBlip)
		in >> *m_SelBlip;
	m_BlipLock.unlock();

	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fRot = AuPoint(x,y,z);
	in >> m_fScale;
}

DpStation::STATION_TYPE DpStationWeapon::ClientRestoreType() const
{
	return STATION_FireControl;
}

void DpStationWeapon::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationWeapon::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("WCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/txConsoleWeapon.bmp");
}

void DpStationWeapon::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationWeapon::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_bCities);
	mem += sizeof(m_bPlanets);
	mem += sizeof(m_bShips);
	mem += sizeof(m_bStations);
	mem += sizeof(m_bStars);
	mem += sizeof(m_idLockedBlip);
	mem += sizeof(m_fRot);
	mem += sizeof(m_fScale);
	mem += sizeof(m_PowerSetting);
	mem += sizeof(m_ShowEmpire);
	mem += sizeof(m_ShowEnemy);
	mem += sizeof(m_ShowFriend);
	mem += sizeof(m_ShowNeutral);
	mem += sizeof(m_Capacitor);
	mem += sizeof(m_CapacitorMax);
	mem += sizeof(m_CapacitorTrend);
	mem += sizeof(m_SensorRange);
	mem += sizeof(m_WeaponPower);
	mem += sizeof(m_OrdnanceLock);
	mem += sizeof(m_Ordnance);
	if(m_Ordnance)
		mem += m_Ordnance->MemoryUsage();
	mem += sizeof(m_BlipLock);
	mem += sizeof(m_SelBlip);
	if(m_SelBlip)
		mem += m_SelBlip->MemoryUsage();
	mem += m_SensorBlips.MemoryUsage();
	mem += sizeof(m_lLastCapacitor);
	mem += sizeof(m_lLastCapTrend);
	mem += sizeof(m_lLastCities);
	mem += sizeof(m_lLastPlanets);
	mem += sizeof(m_lLastShips);
	mem += sizeof(m_lLastStations);
	mem += sizeof(m_lLastStars);
	mem += sizeof(m_lLastWhich);
	mem += sizeof(m_WhichWeapon);
	mem += sizeof(m_ScreenTex);
	return mem;
}
