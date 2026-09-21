//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationsensor.cpp,v 1.28 2022/04/14 16:44:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationsensor.h"
//
#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausound.h"
#include "dabd.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <QPainter>
#include <QPixmap>

#define d_Font							SOUND_Bleep6
#define d_Info							SOUND_Bleep3
#define d_Power							SOUND_Bleep7
#define d_SensorLock					SOUND_Bleep4
#define d_Show							SOUND_Bleep1
#define d_Stance						SOUND_Bleep2
#define d_View							SOUND_Bleep6

static QImage s_SensorRange;

// Table of control inputs.
DiControlInputArray s_SensorConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_SensorConsoleDefaults[] =
{
	// Power switch
	DiControlInput(QObject::tr("Power On/Off"),
				DpStationSensor::CI_PowerToggle, Qt::Key_P,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Display toggles
	DiControlInput(QObject::tr("Show Ships"),
				DpStationSensor::CI_ShipsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Stations"),
				DpStationSensor::CI_StationsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Vehicles"),
				DpStationSensor::CI_VehiclesToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Cities"),
				DpStationSensor::CI_CitiesToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Worlds"),
				DpStationSensor::CI_PlanetsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Stars"),
				DpStationSensor::CI_StarsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Political Stance Toggle
	DiControlInput(QObject::tr("Show Empire Units"),
				DpStationSensor::CI_EmpireToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Friendly Units"),
				DpStationSensor::CI_FriendToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Neutral Units"),
				DpStationSensor::CI_NeutralToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Enemy Units"),
				DpStationSensor::CI_EnemyToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Rotations
	DiControlInput(QObject::tr("Top View"),
				DpStationSensor::CI_RotateTopView, Qt::Key_X,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Rotate to Target"),
				DpStationSensor::CI_RotateToTarget, Qt::Key_Q,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Rotate Left"),
				DpStationSensor::CI_RotateLeft, Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Right"),
				DpStationSensor::CI_RotateRight, Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Up"),
				DpStationSensor::CI_RotateUp, Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Down"),
				DpStationSensor::CI_RotateDown, Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),

	// Zoom
	DiControlInput(QObject::tr("Increase Magnification"),
				DpStationSensor::CI_ZoomIn, Qt::Key_PageUp,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Decrease Magnification"),
				DpStationSensor::CI_ZoomOut, Qt::Key_PageDown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Normal Magnification"),
				DpStationSensor::CI_ZoomNormal, Qt::Key_Home,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Lock
	DiControlInput(QObject::tr("Lock On To Target"),
				DpStationSensor::CI_LockOn, Qt::Key_L,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Unlock Target"),
				DpStationSensor::CI_UnlockAll, Qt::Key_Backspace,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Info Pane
	DiControlInput(QObject::tr("Scan Target"),
				DpStationSensor::CI_Scan, Qt::Key_I,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Scan Font Larger"),
				DpStationSensor::CI_FontLarger, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Scan Font Smaller"),
				DpStationSensor::CI_FontSmaller, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Scan Scroll Down"),
				DpStationSensor::CI_ScrollDown, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Scan Scroll Up"),
				DpStationSensor::CI_ScrollUp, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Scan Close"),
				DpStationSensor::CI_CloseInfoPane, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumSensorConsoleDefaults = sizeof(s_SensorConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationSensor::DpStationSensor() : DpStation()
{
	m_bCities = true;
	m_bPlanets = true;
	m_bShips = true;
	m_bShowScan = false;
	m_bStations = true;
	m_bStars = true;
	m_bVehicles = true;
	m_fRot = AuPoint(0,0,0);	// Default rotation has +Y going up the screen, which is ship forward.
	m_fScale = (float)FeetPerOrbit();
	m_SelBlip = 0;
	m_ShowEmpire = true;
	m_ShowEnemy = true;
	m_ShowFriend = true;
	m_ShowNeutral = true;

	// Init with bogus values to insure an update.
	m_lLastCities = -1;
	m_lLastPlanets = -1;
	m_lLastSensorsOn = -1;
	m_lLastShips = -1;
	m_lLastStations = -1;
	m_lLastStars = -1;
	m_lLastVehicles = -1;

	m_FontSize = 0;				// 0 triggers settings to be loaded during AnimateDeviceObjects
	m_InfoPane = 0;
	m_Scroll = 0;
	m_SensorRange = 0;
	m_ScreenTex = 0;
}

DpStationSensor::~DpStationSensor()
{
	if(m_SelBlip)
		delete m_SelBlip;
}

DpStationSensor::DpStationSensor(const DpStationSensor& c) : DpStation(c)
{
	m_bCities = c.m_bCities;
	m_bPlanets = c.m_bPlanets;
	m_bShips = c.m_bShips;
	m_bShowScan = c.m_bShowScan;
	m_bStations = c.m_bStations;
	m_bStars = c.m_bStars;
	m_bVehicles = c.m_bVehicles;
	m_FontSize = c.m_FontSize;
	m_idLockedBlip = c.m_idLockedBlip;
	m_InfoPane = c.m_InfoPane;
	m_fRot = c.m_fRot;
	m_fScale = c.m_fScale;
	m_lLastCities = c.m_lLastCities;
	m_lLastPlanets = c.m_lLastPlanets;
	m_lLastSensorsOn = c.m_lLastSensorsOn;
	m_lLastShips = c.m_lLastShips;
	m_lLastStations = c.m_lLastStations;
	m_lLastStars = c.m_lLastStars;
	m_lLastVehicles = c.m_lLastVehicles;
	m_Scroll = c.m_Scroll;
	m_SensorRange = c.m_SensorRange;
	m_ShowEmpire = c.m_ShowEmpire;
	m_ShowEnemy = c.m_ShowEnemy;
	m_ShowFriend = c.m_ShowFriend;
	m_ShowNeutral = c.m_ShowNeutral;
	//m_tmLastUpdate = c.m_tmLastUpdate;

	c.m_BlipLock.lockForRead();
	if(c.m_SelBlip)
		m_SelBlip = c.m_SelBlip->Clone();
	else
		m_SelBlip = 0;
	m_SensorBlips = c.m_SensorBlips;
	c.m_BlipLock.unlock();
	m_ScreenTex = 0;
}

void DpStationSensor::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);

	quint16 bits;
	in >> bits;
	m_bCities = (bits & 0x0001) ? true : false;
	m_bPlanets = (bits & 0x0002) ? true : false;
	m_bShips = (bits & 0x0004) ? true : false;
	m_bShowScan = (bits & 0x0008) ? true : false;
	m_bStations = (bits & 0x0010) ? true : false;
	m_bStars = (bits & 0x0020) ? true : false;
	m_bVehicles = (bits & 0x0040) ? true : false;
	m_ShowEmpire = (bits & 0x1000) ? true : false;
	m_ShowFriend = (bits & 0x2000) ? true : false;
	m_ShowNeutral = (bits & 0x4000) ? true : false;
	m_ShowEnemy = (bits & 0x8000) ? true : false;

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
	in >> m_SensorRange;

	// Sensor blips need transform of capsule as known by server.
	m_idLockedBlip << in;
	m_SensorBlips.RxMin(in);

	// Scan local data to pick up things that are already in my scene, namely suns, planets and moons.
	//qDebug() << "DpStationSensor::Rx blips" << m_SensorBlips.count();
	//for(AuSensorBlipList::const_iterator pb = m_SensorBlips.cbegin(); pb != m_SensorBlips.cend(); pb++)
		//qDebug() << **pb;

	if(capsule.SensorsWorking())
	{
		DaAtom* envAtom = capsule.FindEnvironmentAtom();
		if(envAtom)
		{
			AuId scannerWorldId;
			int scannerDomain = capsule.GunnerDomain(scannerWorldId);

			AuSensorBlipList localBlips;
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash> stanceCache;
			int scanContent = 0x0001;							// b0 = Sensor console blips.
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
	//qDebug() << "DpStationSensor::Rx blips merged with locals" << m_SensorBlips.count();
	//for(AuSensorBlipList::const_iterator pb = m_SensorBlips.begin(); pb != m_SensorBlips.end(); pb++)
		//qDebug() << **pb;
}

void DpStationSensor::Tx(QDataStream& out) const
{
	DpStation::Tx(out);

	quint16 bits = 0;
	if(m_bCities)
		bits |= 0x0001;
	if(m_bPlanets)
		bits |= 0x0002;
	if(m_bShips)
		bits |= 0x0004;
	if(m_bShowScan)
		bits |= 0x0008;
	if(m_bStations)
		bits |= 0x0010;
	if(m_bStars)
		bits |= 0x0020;
	if(m_bVehicles)
		bits |= 0x0040;
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
	out << m_SensorRange;
	m_idLockedBlip >> out;

	m_BlipLock.lockForRead();
	m_SensorBlips.TxMinRangeLimited(out);
	m_BlipLock.unlock();
}

// Refresh data from counterpart.
void DpStationSensor::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationSensor* pA = (const DpStationSensor*)pCon;
	m_SensorBlips = pA->m_SensorBlips;
	m_bCities = pA->m_bCities;
	m_bPlanets = pA->m_bPlanets;
	m_bShips = pA->m_bShips;
	m_bShowScan = pA->m_bShowScan;
	m_bStations = pA->m_bStations;
	m_bStars = pA->m_bStars;
	m_bVehicles = pA->m_bVehicles;
	m_idLockedBlip = pA->m_idLockedBlip;
	m_fRot = pA->m_fRot;
	m_fScale = pA->m_fScale;
	m_SensorRange = pA->m_SensorRange;

	m_FontSize = pA->m_FontSize;
	m_Scroll = pA->m_Scroll;

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
}

void DpStationSensor::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);
	AnimateDeviceObjects(re, ship, location);

}

void DpStationSensor::AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, bd, last, location, sceneAtom);
	AnimateDeviceObjects(re, bd, location);
}

// Called between rendering scene frames that contains this object.
void DpStationSensor::AnimateDeviceObjects(AuGLEffects& re, const DaCapsule& ship, const AuPoint& location)
{
	if(!m_FontSize)
	{
		// FontSize is only a client-side setting. As such, it is loaded from local settings here.
		m_FontSize = g_Au.Settings().readNumEntry("/SensorStation/FontSize", 7);
		if(!m_FontSize)
			m_FontSize = 7;	// Don't allow 0 or we'll do this over and over again.
	}

	if(s_SensorRange.isNull())
		s_SensorRange.load(":data/SensorRange.png");

	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("SCon%1").arg(ship.Id().GetID());
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
	if ((m_bVehicles?1:0) != m_lLastVehicles)
	{
		m_lLastVehicles = m_bVehicles;

		QImage bmbut;
		if (m_bVehicles)
			bmbut.load(":data/sc_VehiclesOn.bmp");
		else
			bmbut.load(":data/sc_VehiclesOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,73);
	}
	if ((m_bCities?1:0) != m_lLastCities)
	{
		m_lLastCities = m_bCities;

		QImage bmbut;
		if (m_bCities)
			bmbut.load(":data/sc_CitiesOn.bmp");
		else
			bmbut.load(":data/sc_CitiesOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,56);
	}
	if ((m_bPlanets?1:0) != m_lLastPlanets)
	{
		m_lLastPlanets = m_bPlanets;

		QImage bmbut;
		if (m_bPlanets)
			bmbut.load(":data/sc_PlanetsOn.bmp");
		else
			bmbut.load(":data/sc_PlanetsOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,39);
	}
	if ((m_bStars?1:0) != m_lLastStars)
	{
		m_lLastStars = m_bStars;

		QImage bmbut;
		if (m_bStars)
			bmbut.load(":data/sc_StarsOn.bmp");
		else
			bmbut.load(":data/sc_StarsOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,22);
	}

	if(ship.IsSensorOn() != (bool)m_lLastSensorsOn)
	{
		m_lLastSensorsOn = ship.IsSensorOn();

		QImage bmbut;
		if(ship.IsSensorOn())
			bmbut.load(":data/ec_PowerOn.bmp");
		else
			bmbut.load(":data/ec_PowerOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 4,4);
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
		// Returns the location of the selected blip 'm_SelBlip'.
		AuPoint sysloc = ship.EnvTransform().Project(location);
		QPoint pxSel = m_SensorBlips.DrawSensorConsoleBlips(p, tcomb, feetPerPixel, m_SelBlip, m_idLockedBlip,
				m_bCities, false, m_bPlanets, m_bShips, m_bStations, m_bStars,
				m_bVehicles, sysloc,
				m_ShowEmpire, m_ShowFriend, m_ShowNeutral, m_ShowEnemy, system, ship.Id(), ship.EnvLocation());

		// Mask the frame onto the picture using its alpha channel.
		p.drawPixmap(0,0, bmf);

		// Other buttons overlap the display, when they're visible.
		m_InfoPane = 0;
		if(m_SelBlip)
		{
			const QPoint pxPanelOrg(24,4);	// Origin of panel screen overlay so coords can be calculated relative to the
											// upper/left corner of the panel, instead of the screen overlay subrectangle.

			// These buttons are only visible when something is selected.
			QPoint pxPanelLoc = QPoint(242,4) - pxPanelOrg;	// 242,4 is the panel coord. Must offset into sub rectangle.
			QImage bmbut;
			bmbut.load(":data/sc_LockOn.bmp");
			p.drawImage(pxPanelLoc.x(), pxPanelLoc.y(), bmbut);

			pxPanelLoc = QPoint(231,4) - pxPanelOrg;
			bmbut.load(":data/sc_Scan.bmp");
			p.drawImage(pxPanelLoc.x(), pxPanelLoc.y(), bmbut);

			// The following pane of information appears at times.
			if(m_bShowScan)
			{
				// NOTE: If the size or position of the info pane changes, update SampleControlPanelInput.
				QPixmap bmsib;
				bmsib.load(":data/sc_ScanInfoBase.png");

				QPoint pxInfoLoc;
				if(pxSel.y() >= bmf.height()/2)
				{
					pxInfoLoc = QPoint(14,0);									// Display info pane in upper half of panel.
					m_InfoPane = 1;
				}
				else
				{
					pxInfoLoc = QPoint(14,bmf.height() - (bmsib.height()+8));	// Display info pane in lower half of panel.
					m_InfoPane = 2;
				}

				if(m_SelBlip)
				{
					const DaAtom* pss = ship.FindEnvironmentAtom();
					if(pss)
						DrawInfo(pss, bmsib, QSize(bmsib.width(),bmsib.height()), m_FontSize, m_Scroll, m_SelBlip);
				}

				p.drawPixmap(pxInfoLoc.x(), pxInfoLoc.y(), bmsib);
			}
		}

		// Some buttons overlap the display screen, when they're visible.
		if(!m_idLockedBlip.IsNull())
		{
			// These buttons are only visible when something is "locked on".
			QPoint pxPanelOrg(24,4);
			QPoint pxPanelLoc = QPoint(242,15) - pxPanelOrg;
			QImage bmbut(":data/sc_UnlockAll.bmp");
			p.drawImage(pxPanelLoc, bmbut);
		}

		QImage ims = bms.toImage();
		pTex->ReplaceSubImage(re, ims, 24,5);

		// Power status light has to be drawn every time because it overlaps the main display screen.
		QImage bmlit;
		if(ship.IsSensorOn())
			bmlit.load(":data/ec_StatusLightGreen.bmp");
		else
			bmlit.load(":data/ec_StatusLightOff.bmp");
		pTex->ReplaceSubImage(re, bmlit, 22,4);

		// Blip Visibility by Political Stance
		// Do not draw these buttons when the upper information pane is visible because they overlap.
		if(m_InfoPane != 1)
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
}

void DpStationSensor::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_SensorConsoleControls.begin(); myci != s_SensorConsoleControls.end(); myci++)
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

void DpStationSensor::RestoreDefaultControls()
{
	s_SensorConsoleControls.clear();
	for(int ic=0; ic<s_NumSensorConsoleDefaults; ic++)
		s_SensorConsoleControls.append(s_SensorConsoleDefaults[ic]);
	SaveControls();
}

void DpStationSensor::LoadControls() const
{
	// Init with my default controls.
	s_SensorConsoleControls.clear();
	for(int ic=0; ic<s_NumSensorConsoleDefaults; ic++)
		s_SensorConsoleControls.append(s_SensorConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_SensorConsoleControls.LoadSettings("Controls/Sensor Console");
}

void DpStationSensor::SaveControls() const
{
	// Store my controls in the settings file.
	s_SensorConsoleControls.SaveSettings("Controls/Sensor Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationSensor::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_SensorConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_SensorConsoleControls.cbegin();
			ci != s_SensorConsoleControls.cend(); ci++)
		aAllControls.append(*ci);
}

void DpStationSensor::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaBd& bd,
		CONTROL_CONTEXT cc, const AuPoint& location)
{
	Q_UNUSED(location);
	ProcessControlInput(aActiveControls, sceneAtom, bd, cc);
}

void DpStationSensor::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& location)
{
	Q_UNUSED(location);
	ProcessControlInput(aActiveControls, sceneAtom, ship, cc);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationSensor::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaCapsule& ship,
		CONTROL_CONTEXT cc)
{
	if(ship.IsNew())
		return;	// This console cannot be used when the ship is "nu".

	//qDebug("DpStationSensor::ProcessControlInput");
	const double rRotInc = 5.0 * DEG2RAD;	// Rotation increment.
	for (int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch (ci.GetId())
		{
		case CI_CloseInfoPane:
			if(cc != CONTROL_Client)
				SetShowScan(false, *sceneAtom, ship.EmpireId());
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Info, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_FontLarger:
			if(cc != CONTROL_Server && m_bShowScan)
			{
				if(!m_FontSize)
					m_FontSize = g_Au.Settings().readNumEntry("/SensorStation/FontSize", 7);

				m_FontSize++;

				// FontSize is only a client-side setting. As such, it is updated in local settings here.
				g_Au.Settings().writeEntry("/SensorStation/FontSize", m_FontSize);

				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Font, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_FontSmaller:
			if(cc != CONTROL_Server && m_bShowScan)
			{
				if(!m_FontSize)
					m_FontSize = g_Au.Settings().readNumEntry("/SensorStation/FontSize", 7);

				m_FontSize--;

				// FontSize is only a client-side setting. As such, it is updated in local settings here.
				g_Au.Settings().writeEntry("/SensorStation/FontSize", m_FontSize);

				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Font, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ScrollUp:
			if(cc != CONTROL_Server && m_bShowScan)
				m_Scroll++;
			break;
		case CI_ScrollDown:
			if(cc != CONTROL_Server && m_bShowScan)
				if(m_Scroll)
					m_Scroll--;
			break;

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

		case CI_PowerToggle:
			if(cc != CONTROL_Client)
			{
				if(ship.IsSensorOn())
					ship.TurnSensorsOff();
				else
					ship.TurnSensorsOn();
			}
			if(cc != CONTROL_Server)
			{
				float bp = ship.IsSensorOn() ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_LockOn:			// "Locks on" to the selected blip.
			if(cc != CONTROL_Client)
			{
				AuId selId;
				m_BlipLock.lockForRead();
				if(m_SelBlip)
					selId = m_SelBlip->AtomId();
				m_BlipLock.unlock();

				if(!selId.IsNull())
					ship.LockOn(selId, true);
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SensorLock, 1,1.1f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_UnlockAll:			// Removes "Locks on" from all blips.
			if(cc != CONTROL_Client)
			{
				if(!ship.LockedBlip().IsNull())
				{
					ship.LockOn(AuId(0), true);
					m_BlipLock.lockForWrite();
					if(m_SelBlip)
					{
						delete m_SelBlip;
						m_SelBlip = 0;
					}
					m_BlipLock.unlock();
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SensorLock, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Scan:			// "Scans" the selected blip.
			if(cc != CONTROL_Client)
			{
				if(m_SelBlip)
					SetShowScan(!m_bShowScan, *sceneAtom, ship.EmpireId());
			}
			if(cc != CONTROL_Server)
			{
				if(m_SelBlip)
				{
					float bp = m_bShowScan ? .8f : 1;	// button pitch
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Info, 1,bp);
					sceneAtom->AddEffect(buttonClick);
				}
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
		case CI_VehiclesToggle:
			if(cc != CONTROL_Client)
				m_bVehicles = !m_bVehicles;
			if(cc != CONTROL_Server)
			{
				float bp = m_bVehicles ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Show, 1,bp);
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
			if(cc == CONTROL_Client)
				break;
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
			break;
		case CI_ZoomOut:
			if(cc == CONTROL_Client)
				break;
			if(m_fScale < FeetPerSolarSystem())
			{
				m_fScale *= 2.0;
				if (fabs(m_fScale - FeetPerOrbit()) < 100.0)
					m_fScale = (float)FeetPerOrbit();
				else if (fabs(m_fScale - FeetPerAu()) < 1000.0)
					m_fScale = (float)FeetPerAu();
			}
			break;
		case CI_ZoomNormal:
			if(cc == CONTROL_Client)
				break;
			if(m_fScale != (float)FeetPerOrbit())
				m_fScale = (float)FeetPerOrbit();
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
				ProcessDisplayHit(ci.HitLocation(), ship.EnvTransform().Inverse(), *sceneAtom, ship.EmpireId());
			break;
		}
	}
}

void DpStationSensor::ProcessDisplayHit(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, DaPerson& person, quint32 empireId)
{
	if(!TestInsideScreenMask(pxHit))
		return;
	//qDebug("DpStationSensor Hit at %d,%d", pxHit.x(), pxHit.y());
	SelectNearest(pxHit, tRelBlips, person, empireId);
}

void DpStationSensor::SelectNearest(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, DaPerson& person, quint32 empireId)
{
	const double rseldist = 5.0;	// Pixel distance to select.

	// Find the closest atom in the display.
	double radist = -1.0;	// Distance to the nearest atom in display pixels.
	m_BlipLock.lockForWrite();
	AuSensorBlipList::const_iterator blip = FindClosestDisplayBlip(pxHit, radist, tRelBlips, person.FindEnvironmentAtom());
	if(blip == m_SensorBlips.end() || radist > rseldist)
	{
		// No blip selected.
		if(m_SelBlip)
			delete m_SelBlip;
		m_SelBlip = 0;
	}
	else if(!m_SelBlip || !m_SelBlip->IsSame(*blip))
	{
		// An object was selected that is different than the current selection.
		if(m_SelBlip)
			delete m_SelBlip;
		m_SelBlip = (*blip)->Clone();
	}
	m_BlipLock.unlock();
	SetShowScan(false, person, empireId);
}

AuSensorBlipList::const_iterator DpStationSensor::FindClosestDisplayBlip(const QPoint& pxHit, double& rDisplayDistance,
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
		else if((*pb)->IsVehicle())
		{
			if(!m_bVehicles)
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
void DpStationSensor::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
		bool bContinuous)
{
	if(idPanel)
		return;
	double panWid, panHt;
	PanelDimensions(panWid, panHt);
	int ix = (int)(fPanelLoc.x * 256 / panWid);	// X pixel in texture map.
	int iy = (int)(fPanelLoc.y * 128 / panHt);
	QPoint pxHit(ix, iy);

	// Info pane buttons are sampled first so they get first dibs over any other buttons below them, or the display screen.
	if(m_InfoPane == 1)
	{
		// Info pane in upper half.
		// Note that the top/bottom is reversed in the rectangles. This works with the iy we've calculated that is also
		// measured from the bottom, not the top as usual in GDI.
		DiPanelControl acontrols[] =
		{
			DiPanelControl(CI_CloseInfoPane, QRect(218,116,7,7)),
			DiPanelControl(CI_FontLarger, QRect(218,105,7,7)),
			DiPanelControl(CI_FontSmaller, QRect(218,97,7,7)),
			DiPanelControl(CI_ScrollDown, QRect(218,79,7,7)),
			DiPanelControl(CI_ScrollUp, QRect(218,71,7,7)),

			DiPanelControl(CI_LockOn, QRect(242,115,9,9)),
			DiPanelControl(CI_UnlockAll, QRect(242,104,9,9)),
			DiPanelControl(CI_Scan, QRect(231,115,9,9)),

			DiPanelControl(CI_PowerToggle, QRect(4,4,16,16)),

			DiPanelControl(CI_ShipsToggle, QRect(4,107,16,16)),
			DiPanelControl(CI_StationsToggle, QRect(4,90,16,16)),
			DiPanelControl(CI_VehiclesToggle, QRect(4,73,16,16)),
			DiPanelControl(CI_CitiesToggle, QRect(4,56,16,16)),
			DiPanelControl(CI_PlanetsToggle, QRect(4,39,16,16)),
			DiPanelControl(CI_StarsToggle, QRect(4,22,16,16)),

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
		};
		unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);
		aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
	}
	else if(m_InfoPane == 2)
	{
		// Info pane in lower half.
		// Note that the top/bottom is reversed in the rectangles. This works with the iy we've calculated that is also
		// measured from the bottom, not the top as usual in GDI.
		DiPanelControl acontrols[] =
		{
			DiPanelControl(CI_CloseInfoPane, QRect(218,59,7,7)),
			DiPanelControl(CI_FontLarger, QRect(218,48,7,7)),
			DiPanelControl(CI_FontSmaller, QRect(218,40,7,7)),
			DiPanelControl(CI_ScrollDown, QRect(218,22,7,7)),
			DiPanelControl(CI_ScrollUp, QRect(218,14,7,7)),

			DiPanelControl(CI_LockOn, QRect(242,115,9,9)),
			DiPanelControl(CI_UnlockAll, QRect(242,104,9,9)),
			DiPanelControl(CI_Scan, QRect(231,115,9,9)),

			DiPanelControl(CI_PowerToggle, QRect(4,4,16,16)),

			DiPanelControl(CI_ShipsToggle, QRect(4,107,16,16)),
			DiPanelControl(CI_StationsToggle, QRect(4,90,16,16)),
			DiPanelControl(CI_VehiclesToggle, QRect(4,73,16,16)),
			DiPanelControl(CI_CitiesToggle, QRect(4,56,16,16)),
			DiPanelControl(CI_PlanetsToggle, QRect(4,39,16,16)),
			DiPanelControl(CI_StarsToggle, QRect(4,22,16,16)),

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
		};
		unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);
		aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
	}
	else
	{
		// Info pane is not visible.
		// Note that the top/bottom is reversed in the rectangles. This works with the iy we've calculated that is also
		// measured from the bottom, not the top as usual in GDI.
		DiPanelControl acontrols[] =
		{
			DiPanelControl(CI_LockOn, QRect(242,115,9,9)),
			DiPanelControl(CI_UnlockAll, QRect(242,104,9,9)),
			DiPanelControl(CI_Scan, QRect(231,115,9,9)),

			DiPanelControl(CI_PowerToggle, QRect(4,4,16,16)),

			DiPanelControl(CI_ShipsToggle, QRect(4,107,16,16)),
			DiPanelControl(CI_StationsToggle, QRect(4,90,16,16)),
			DiPanelControl(CI_VehiclesToggle, QRect(4,73,16,16)),
			DiPanelControl(CI_CitiesToggle, QRect(4,56,16,16)),
			DiPanelControl(CI_PlanetsToggle, QRect(4,39,16,16)),
			DiPanelControl(CI_StarsToggle, QRect(4,22,16,16)),

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

			// NOTE: The display screen is last so previous (possibly overlapping) buttons get first crack at mouse clicks.
			DiPanelControl(CI_DisplayScreen, QRect(24,5,227,119), false, true),
		};
		unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);
		aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
	}
}

// Returns true if a pixel point is within the "screen" area of the sensor panel.
bool DpStationSensor::TestInsideScreenMask(const QPoint &pxTest) const
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

void DpStationSensor::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStation::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
	if(seatedCreature && seatedCreature->Is(sceneAtom.Id()))
	{
		AuSensorBlipList::const_iterator blip = m_SensorBlips.Find(m_idLockedBlip);
		if(blip != m_SensorBlips.end())
			RenderSensorLock((*blip)->AtomId(), (*blip)->PoliticalStance(), (*blip)->IsCivilian(), (*blip)->EnvLocation(), tModel,
					sceneAtom, ari);
	}
}

bool DpStationSensor::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	DaCapsule* ship = dynamic_cast<DaCapsule*>(person->Parent());
	if(ship)
	{
		if(ship->IsDesignDataCacheValid())
		{
			m_SensorRange = ship->SensorRange();

			// The ship provides me with a list of the sensor console blips that are in range.
			m_idLockedBlip = ship->LockedBlip();
			if(!last.SameStartSec(now))
			{
				m_BlipLock.lockForWrite();
				m_SensorBlips.Clear();
				if(ship->SensorsWorking())
				{
					AuPoint shipSysLoc = ship->EnvLocation();
					ship->PerformSensorConsoleScan(m_SensorBlips);
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
		}
	}
	return true;
}

void DpStationSensor::LoadClientSettings(const AuId& /*planetId*/)
{
	m_bCities = g_Au.Settings().readBoolEntry("/SensorStation/CitiesOn", m_bCities);
	m_bPlanets = g_Au.Settings().readBoolEntry("/SensorStation/PlanetsOn", m_bPlanets);
	m_bShips = g_Au.Settings().readBoolEntry("/SensorStation/ShipsOn", m_bShips);
	m_bStations = g_Au.Settings().readBoolEntry("/SensorStation/StationsOn", m_bStations);
	m_bStars = g_Au.Settings().readBoolEntry("/SensorStation/StarsOn", m_bStars);
	m_bVehicles = g_Au.Settings().readBoolEntry("/SensorStation/VehiclesOn", m_bVehicles);
	m_fRot.x = g_Au.Settings().readNumEntry("/SensorStation/XRotation", m_fRot.x);
	m_fRot.y = g_Au.Settings().readNumEntry("/SensorStation/YRotation", m_fRot.y);
	m_fRot.z = g_Au.Settings().readNumEntry("/SensorStation/ZRotation", m_fRot.z);
	m_fScale = g_Au.Settings().readNumEntry("/SensorStation/Scale", m_fScale);
	if(m_fScale < 1 || m_fScale > FeetPerSolarSystem())
		m_fScale = FeetPerOrbit();
	m_ShowEmpire = g_Au.Settings().readBoolEntry("/SensorStation/ShowEmpire", m_ShowEmpire);
	m_ShowEnemy = g_Au.Settings().readBoolEntry("/SensorStation/ShowEnemy", m_ShowEnemy);
	m_ShowFriend = g_Au.Settings().readBoolEntry("/SensorStation/ShowFriend", m_ShowFriend);
	m_ShowNeutral = g_Au.Settings().readBoolEntry("/SensorStation/ShowNeutral", m_ShowNeutral);
}

void DpStationSensor::StoreClientSettings(const AuId& /*planetId*/) const
{
	g_Au.Settings().writeEntry("/SensorStation/CitiesOn", m_bCities);
	g_Au.Settings().writeEntry("/SensorStation/PlanetsOn", m_bPlanets);
	g_Au.Settings().writeEntry("/SensorStation/ShipsOn", m_bShips);
	g_Au.Settings().writeEntry("/SensorStation/StationsOn", m_bStations);
	g_Au.Settings().writeEntry("/SensorStation/StarsOn", m_bStars);
	g_Au.Settings().writeEntry("/SensorStation/VehiclesOn", m_bVehicles);
	g_Au.Settings().writeEntry("/SensorStation/XRotation", m_fRot.x);
	g_Au.Settings().writeEntry("/SensorStation/YRotation", m_fRot.y);
	g_Au.Settings().writeEntry("/SensorStation/ZRotation", m_fRot.z);
	g_Au.Settings().writeEntry("/SensorStation/Scale", m_fScale);
	g_Au.Settings().writeEntry("/SensorStation/ShowEmpire", m_ShowEmpire);
	g_Au.Settings().writeEntry("/SensorStation/ShowEnemy", m_ShowEnemy);
	g_Au.Settings().writeEntry("/SensorStation/ShowFriend", m_ShowFriend);
	g_Au.Settings().writeEntry("/SensorStation/ShowNeutral", m_ShowNeutral);
}

void DpStationSensor::TxSettings(QDataStream& out) const
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
		bits |= 0x0010;
	if(m_bStars)
		bits |= 0x0020;
	if(m_bVehicles)
		bits |= 0x0040;
	if(m_ShowEmpire)
		bits |= 0x1000;
	if(m_ShowFriend)
		bits |= 0x2000;
	if(m_ShowNeutral)
		bits |= 0x4000;
	if(m_ShowEnemy)
		bits |= 0x8000;
	out << bits;
	out << (float)m_fRot.x;
	out << (float)m_fRot.y;
	out << (float)m_fRot.z;
	out << m_fScale;
}

void DpStationSensor::RxSettings(QDataStream& in)
{
	quint16 bits;
	in >> bits;
	m_bCities = (bits & 0x0001) ? true : false;
	m_bPlanets = (bits & 0x0002) ? true : false;
	m_bShips = (bits & 0x0004) ? true : false;
	m_bStations = (bits & 0x0010) ? true : false;
	m_bStars = (bits & 0x0020) ? true : false;
	m_bVehicles = (bits & 0x0040) ? true : false;
	m_ShowEmpire = (bits & 0x1000) ? true : false;
	m_ShowFriend = (bits & 0x2000) ? true : false;
	m_ShowNeutral = (bits & 0x4000) ? true : false;
	m_ShowEnemy = (bits & 0x8000) ? true : false;

	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fRot = AuPoint(x,y,z);
	in >> m_fScale;
}

void DpStationSensor::SetShowScan(bool showScan, DaPerson& person, quint32 empireId)
{
	m_bShowScan = showScan;

	if(!showScan)
		return;

	// Scan the solar system for the empire.
	if(!empireId)
		return;

	// When scanning a star or planet, the system is marked as scanned by the empire.
	m_BlipLock.lockForRead();
	bool isop = m_SelBlip && m_SelBlip->IsStarOrPlanet();
	m_BlipLock.unlock();
	if(!isop)
		return;

	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(person.FindEnvironmentAtom());
	if(system->IsScannedBy(empireId))
		return;

	system->ScannedBy(empireId, system->GenHab(), system->GenSeed());
	person.IncreaseSkill(AuSkill::Sensor, 1);

	QString text = QObject::tr("Survey scan of %1 complete.").arg(system->Name());
	AuCommThought msg(text, person.Id(), person.Name());
	g_Au.SendAtomChat(msg);
}

DpStation::STATION_TYPE DpStationSensor::ClientRestoreType() const
{
	return STATION_Sensor;
}

void DpStationSensor::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationSensor::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("SCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/sc_Console.bmp");
}

void DpStationSensor::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationSensor::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_bCities);
	mem += sizeof(m_bPlanets);
	mem += sizeof(m_bShips);
	mem += sizeof(m_bShowScan);
	mem += sizeof(m_bStations);
	mem += sizeof(m_bStars);
	mem += sizeof(m_bVehicles);
	mem += sizeof(m_idLockedBlip);
	mem += sizeof(m_fRot);
	mem += sizeof(m_fScale);
	mem += sizeof(m_SensorRange);
	mem += sizeof(m_ShowEmpire);
	mem += sizeof(m_ShowEnemy);
	mem += sizeof(m_ShowFriend);
	mem += sizeof(m_ShowNeutral);
	mem += sizeof(m_BlipLock);
	mem += sizeof(m_SelBlip);
	if(m_SelBlip)
		mem += m_SelBlip->MemoryUsage();
	mem += m_SensorBlips.MemoryUsage();
	mem += sizeof(m_lLastCities);
	mem += sizeof(m_lLastPlanets);
	mem += sizeof(m_lLastSensorsOn);
	mem += sizeof(m_lLastShips);
	mem += sizeof(m_lLastStations);
	mem += sizeof(m_lLastStars);
	mem += sizeof(m_lLastVehicles);
	mem += sizeof(m_FontSize);
	mem += sizeof(m_InfoPane);
	mem += sizeof(m_Scroll);
	mem += sizeof(m_ScreenTex);
	return mem;
}
