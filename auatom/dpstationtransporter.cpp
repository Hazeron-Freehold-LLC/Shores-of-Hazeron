//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationtransporter.cpp,v 1.29 2022/04/14 16:52:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationtransporter.h"
//
#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausound.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldringsection.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <QPainter>
#include <QPixmap>

#define d_Aboard						SOUND_Bleep3
#define d_Energize						SOUND_Bleep1
#define d_Mode							SOUND_Bleep5
#define d_Pads							SOUND_Bleep4
#define d_Power							SOUND_Bleep7
#define d_Show							SOUND_Bleep1
#define d_Stance						SOUND_Bleep2
#define d_View							SOUND_Bleep6

static QImage s_SensorRange;
#define c_ScreenHeight					96
#define c_ScreenWidth					196

// Table of control inputs.
DiControlInputArray s_TransporterConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_TransporterConsoleDefaults[] =
{
	DiControlInput(QObject::tr("Power On/Off"),
				DpStationTransporter::CI_PowerToggle, Qt::Key_P,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Planet Mode"),
				DpStationTransporter::CI_PlanetMode, Qt::Key_L,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Transporter Mode"),
				DpStationTransporter::CI_TransporterMode, Qt::Key_I,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Sensor Mode"),
				DpStationTransporter::CI_SensorMode, Qt::Key_O,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Decrease Pads to Activate"),
				DpStationTransporter::CI_PadsDec, Qt::Key_H,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Increase Pads to Activate"),
				DpStationTransporter::CI_PadsInc, Qt::Key_J,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Transport From Target"),
				DpStationTransporter::CI_ActivateFrom, Qt::Key_N,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Transport To Target"),
				DpStationTransporter::CI_ActivateTo, Qt::Key_M,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Display toggles
	DiControlInput(QObject::tr("Show Life"),
				DpStationTransporter::CI_LifeToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Ships"),
				DpStationTransporter::CI_ShipsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Stations"),
				DpStationTransporter::CI_StationsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Vehicles"),
				DpStationTransporter::CI_VehiclesToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Cities"),
				DpStationTransporter::CI_CitiesToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Worlds"),
				DpStationTransporter::CI_PlanetsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Stars"),
				DpStationTransporter::CI_StarsToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Political Stance Toggle
	DiControlInput(QObject::tr("Show Empire Units"),
				DpStationTransporter::CI_EmpireToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Friendly Units"),
				DpStationTransporter::CI_FriendToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Neutral Units"),
				DpStationTransporter::CI_NeutralToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Show Enemy Units"),
				DpStationTransporter::CI_EnemyToggle, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Rotations
	DiControlInput(QObject::tr("Top View"),
				DpStationTransporter::CI_RotateTopView, Qt::Key_X,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Rotate to Target"),
				DpStationTransporter::CI_RotateToTarget, Qt::Key_Q,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Rotate Left"),
				DpStationTransporter::CI_RotateLeft, Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Right"),
				DpStationTransporter::CI_RotateRight, Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Up"),
				DpStationTransporter::CI_RotateUp, Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Rotate Down"),
				DpStationTransporter::CI_RotateDown, Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),

	// Zoom
	DiControlInput(QObject::tr("Increase Magnification"),
				DpStationTransporter::CI_ZoomIn, Qt::Key_PageUp,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Decrease Magnification"),
				DpStationTransporter::CI_ZoomOut, Qt::Key_PageDown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Normal Magnification"),
				DpStationTransporter::CI_ZoomNormal, Qt::Key_Home,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Positioning
	DiControlInput(QObject::tr("Center on My Location"),
				DpStationTransporter::CI_Center, Qt::Key_K,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumTransporterConsoleDefaults = sizeof(s_TransporterConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationTransporter::DpStationTransporter(quint16 whichTransporter) : DpStation()
{
	m_bCities = true;
	m_bLife = true;
	m_bPlanets = true;
	m_bShips = true;
	m_bStations = true;
	m_bStars = true;
	m_bVehicles = true;
	m_fScale = 1;
	m_SelBlip = 0;
	m_SelCoord = false;
	m_ShowAboard = false;
	m_ShowEmpire = true;
	m_ShowEnemy = true;
	m_ShowFriend = true;
	m_ShowNeutral = true;

	// Init with bogus values to insure an update.
	m_LastMode = Init;
	m_LastPadsActive = -1;
	m_LastShowAboard = -1;
	m_LastShowEmpire = -1;
	m_LastShowEnemy = -1;
	m_LastShowFriend = -1;
	m_LastShowNeutral = -1;
	m_lLastActive = -1;
	m_lLastCities = -1;
	m_lLastLife = -1;
	m_lLastPlanets = -1;
	m_lLastPowerOn = -1;
	m_lLastShips = -1;
	m_lLastStations = -1;
	m_lLastStars = -1;
	m_lLastVehicles = -1;

	m_MaxRange = 0;
	m_Mode = Init;
	m_PadsActive = 0;
	m_WhichTransporter = whichTransporter;
	m_ScreenTex = 0;
}

DpStationTransporter::~DpStationTransporter()
{
	if(m_SelBlip)
		delete m_SelBlip;
}

DpStationTransporter::DpStationTransporter(const DpStationTransporter& c) : DpStation(c)
{
	m_bCities = c.m_bCities;
	m_bLife = c.m_bLife;
	m_bPlanets = c.m_bPlanets;
	m_bShips = c.m_bShips;
	m_bStations = c.m_bStations;
	m_bStars = c.m_bStars;
	m_bVehicles = c.m_bVehicles;
	m_Coord = c.m_Coord;
	m_fRot = c.m_fRot;
	m_fScale = c.m_fScale;
	m_idLockedBlip = c.m_idLockedBlip;
	m_LastMode = c.m_LastMode;
	m_LastPadsActive = c.m_LastPadsActive;
	m_LastShowAboard = c.m_LastShowAboard;
	m_LastShowEmpire = c.m_LastShowEmpire;
	m_LastShowEnemy = c.m_LastShowEnemy;
	m_LastShowFriend = c.m_LastShowFriend;
	m_LastShowNeutral = c.m_LastShowNeutral;
	m_lLastActive = c.m_lLastActive;
	m_lLastCities = c.m_lLastCities;
	m_lLastLife = c.m_lLastLife;
	m_lLastPlanets = c.m_lLastPlanets;
	m_lLastPowerOn = c.m_lLastPowerOn;
	m_lLastShips = c.m_lLastShips;
	m_lLastStations = c.m_lLastStations;
	m_lLastStars = c.m_lLastStars;
	m_lLastVehicles = c.m_lLastVehicles;
	m_MaxRange = c.m_MaxRange;
	m_Mode = c.m_Mode;
	m_PadsActive = c.m_PadsActive;
	m_SelCoord = c.m_SelCoord;
	m_ShowAboard = c.m_ShowAboard;
	m_ShowEmpire = c.m_ShowEmpire;
	m_ShowEnemy = c.m_ShowEnemy;
	m_ShowFriend = c.m_ShowFriend;
	m_ShowNeutral = c.m_ShowNeutral;
	//m_tmLastUpdate = c.m_tmLastUpdate;
	m_WhichTransporter = c.m_WhichTransporter;

	c.m_BlipLock.lockForRead();
	if(c.m_SelBlip)
		m_SelBlip = c.m_SelBlip->Clone();
	else
		m_SelBlip = 0;
	m_SensorBlips = c.m_SensorBlips;
	c.m_BlipLock.unlock();
	m_ScreenTex = 0;
}

void DpStationTransporter::Tx(QDataStream& out) const
{
	DpStation::Tx(out);

	quint8 ui8 = m_Mode;	// b0 and b1.
	if(m_SelBlip)
		ui8 |= 0x10;
	else if(m_SelCoord)
		ui8 |= 0x20;

	out << ui8;
	out << (quint8)m_PadsActive;

	if(ui8 & 0x10)
	{
		m_BlipLock.lockForRead();
		out << (quint8)m_SelBlip->Type();
		out << *m_SelBlip;
		m_BlipLock.unlock();
	}

	if(ui8 & 0x20)
	{
		out << (float)m_Coord.x;
		out << (float)m_Coord.y;
		out << (float)m_Coord.z;
	}

	out << (float)m_fRot.x;
	out << (float)m_fRot.y;
	out << (float)m_fRot.z;
	out << m_fScale;

	quint16 bits = 0;
	if(m_bPlanets)
		bits |= 0x01;
	if(m_bShips)
		bits |= 0x02;
	if(m_bStations)
		bits |= 0x04;
	if(m_bStars)
		bits |= 0x08;
	if(m_bCities)
		bits |= 0x10;
	if(m_bLife)
		bits |= 0x20;
	if(m_bVehicles)
		bits |= 0x40;
	if(m_ShowAboard)
		bits |= 0x0800;
	if(m_ShowEmpire)
		bits |= 0x1000;
	if(m_ShowFriend)
		bits |= 0x2000;
	if(m_ShowNeutral)
		bits |= 0x4000;
	if(m_ShowEnemy)
		bits |= 0x8000;
	out << bits;

	out << m_MaxRange;
	out << m_WhichTransporter;
	out << m_idLockedBlip;

	m_BlipLock.lockForRead();
	m_SensorBlips.TxMinRangeLimited(out);
	m_BlipLock.unlock();
}

void DpStationTransporter::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);

	quint8 ui8;
	in >> ui8;
	m_Mode = (MODE)(ui8 & 0x03);

	quint8 ui82;
	in >> ui82;
	m_PadsActive = ui82;

	if(m_SelBlip)
		delete m_SelBlip;
	if(ui8 & 0x10)
	{
		quint8 blipType;
		in >> blipType;
		m_SelBlip = AuSensorBlip::Create((AuSensorBlip::TYPE)blipType);
		if(m_SelBlip)
			in >> *m_SelBlip;
	}
	else
		m_SelBlip = 0;

	float x,y,z;
	if(ui8 & 0x20)
	{
		in >> x;
		in >> y;
		in >> z;
		m_Coord = AuPoint(x,y,z);
		m_SelCoord = true;
	}
	else
		m_SelCoord = false;

	in >> x;
	in >> y;
	in >> z;
	m_fRot = AuPoint(x,y,z);
	in >> m_fScale;

	quint16 bits;
	in >> bits;
	m_bPlanets = (bits & 0x01) ? true : false;
	m_bShips = (bits & 0x02) ? true : false;
	m_bStations = (bits & 0x04) ? true : false;
	m_bStars = (bits & 0x08) ? true : false;
	m_bCities = (bits & 0x10) ? true : false;
	m_bLife = (bits & 0x20) ? true : false;
	m_bVehicles = (bits & 0x40) ? true : false;
	m_ShowAboard = (bits & 0x0800) ? true : false;
	m_ShowEmpire = (bits & 0x1000) ? true : false;
	m_ShowFriend = (bits & 0x2000) ? true : false;
	m_ShowNeutral = (bits & 0x4000) ? true : false;
	m_ShowEnemy = (bits & 0x8000) ? true : false;

	in >> m_MaxRange;
	in >> m_WhichTransporter;

	// Sensor blips need transform of capsule as known by server.
	in >> m_idLockedBlip;
	m_SensorBlips.RxMin(in);

	// Scan local data to pick up things that are already in my scene, namely suns, planets and moons.
	if(capsule.SensorsWorking())
	{
		DaAtom* envAtom = capsule.FindEnvironmentAtom();
		if(envAtom)
		{
			AuId scannerWorldId;
			int scannerDomain = capsule.GunnerDomain(scannerWorldId);

			AuSensorBlipList localBlips;
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash> stanceCache;
			int scanContent = 0x0004;							// b2 = Transporter console blips.
			if(m_ShowAboard)
				scanContent |= 0x0040;
			envAtom->SensorScan(localBlips, capsule.EnvLocation(), capsule.Id(), capsule.EmpireId(), scannerDomain, scannerWorldId,
					m_MaxRange, capsule.MaxBayWeaponRange(), scanContent, m_idLockedBlip, stanceCache);

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
	//qDebug() << "DpStationTransporter::Rx blips merged with locals" << m_SensorBlips.count();
}

// Refresh data from valid counterpart.
void DpStationTransporter::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationTransporter* pA = (const DpStationTransporter*)pCon;
	m_SensorBlips = pA->m_SensorBlips;
	m_bCities = pA->m_bCities;
	m_bLife = pA->m_bLife;
	m_bPlanets = pA->m_bPlanets;
	m_bShips = pA->m_bShips;
	m_bStations = pA->m_bStations;
	m_bStars = pA->m_bStars;
	m_bVehicles = pA->m_bVehicles;
	m_fRot = pA->m_fRot;
	m_fScale = pA->m_fScale;
	m_idLockedBlip = pA->m_idLockedBlip;
	m_MaxRange = pA->m_MaxRange;
	m_Mode = pA->m_Mode;
	m_PadsActive = pA->m_PadsActive;

	if(m_SelBlip)
		delete m_SelBlip;
	if(pA->m_SelBlip)
		m_SelBlip = pA->m_SelBlip->Clone();
	else
		m_SelBlip = 0;

	m_SelCoord = pA->m_SelCoord;
	m_Coord = pA->m_Coord;

	m_ShowAboard = pA->m_ShowAboard;
	m_ShowEmpire = pA->m_ShowEmpire;
	m_ShowFriend = pA->m_ShowFriend;
	m_ShowNeutral = pA->m_ShowNeutral;
	m_ShowEnemy = pA->m_ShowEnemy;
	m_WhichTransporter = pA->m_WhichTransporter;
}

bool DpStationTransporter::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	if(m_Mode == Init)
	{
		// Init to center on the planet's map; the rotation value is the long/lat.
		const DaWorld* planet = person->FindParentPlanetInEnv();
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
		if(llworld)
		{
			m_fRot = AuPoint(0,0);
			m_Mode = Planet;
		}
		else
		{
			m_fRot = AuPoint(0,0);
			m_Mode = Transporter;
		}
	}

	DaCapsule* ship = dynamic_cast<DaCapsule*>(person->Parent());
	if(!ship || !ship->IsDesignDataCacheValid())
		return false;

	if(m_Mode == Init)
	{
		// Initially activate just one pad in the transporter room.
		if(ship->TransporterPadsWorking(m_WhichTransporter) >= 1)
			m_PadsActive = 1;
	}
	m_MaxRange = ship->TransporterRange(m_WhichTransporter);

	// The ship provides me with a list of the sensor console blips that are in range.
	m_idLockedBlip = ship->LockedBlip();
	if(!last.SameStartSec(now))
	{
		m_BlipLock.lockForWrite();
		m_SensorBlips.Clear();
		if(ship->SensorsWorking())
		{
			AuPoint shipSysLoc = ship->EnvLocation();
			ship->PerformTransporterConsoleScan(m_SensorBlips, m_ShowAboard);
			AuSensorBlipList::iterator sbi = m_SensorBlips.begin();
			while(sbi != m_SensorBlips.end())
			{
				if(!(*sbi)->IsRangeLimited() || (*sbi)->Range(shipSysLoc) < m_MaxRange)
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
	return true;
}

// Called between rendering scene frames that contains this object.
void DpStationTransporter::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);
	AnimateDeviceObjects(re, ship, location);
}

// Called between rendering scene frames that contains this object.
void DpStationTransporter::AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, bd, last, location, sceneAtom);
	AnimateDeviceObjects(re, bd, location);
}

// Called between rendering scene frames that contains this object.
void DpStationTransporter::AnimateDeviceObjects(AuGLEffects& re, const DaCapsule& capsule, const AuPoint& location)
{
	if(s_SensorRange.isNull())
		s_SensorRange.load(":data/SensorRange.png");

	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("TCon%1").arg(capsule.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	int troomIndex = capsule.TransporterSystemIndex(m_WhichTransporter);
	if(troomIndex < 0)
		return;
	if(capsule.IsTransporterSystemOn(troomIndex) != (bool)m_lLastPowerOn ||
		capsule.IsTransporterSystemActive(troomIndex) != (bool)m_lLastActive)
	{
		m_lLastActive = capsule.IsTransporterSystemActive(troomIndex);
		m_lLastPowerOn = capsule.IsTransporterSystemOn(troomIndex);

		QImage bmbut;
		if(m_lLastPowerOn)
			bmbut.load(":/data/ec_PowerOn.bmp");
		else
			bmbut.load(":/data/ec_PowerOff.bmp");
		pTex->ReplaceSubImage(re, bmbut, 7,106);

		QImage bmlit;
		if(m_lLastActive)
			bmlit.load(":/data/ec_StatusLightPurple.bmp");
		else if(m_lLastPowerOn)
			bmlit.load(":/data/ec_StatusLightGreen.bmp");
		else
			bmlit.load(":/data/ec_StatusLightOff.bmp");
		pTex->ReplaceSubImage(re, bmlit, 8,89);
	}

	if(m_Mode != m_LastMode)
	{
		m_LastMode = m_Mode;
		switch(m_Mode)
		{
		case Init:
			{
				QImage imp(":/data/tc_PlanetModeOff.bmp");
				pTex->ReplaceSubImage(re, imp, 7,68);
				QImage imt(":/data/tc_TransporterModeOff.bmp");
				pTex->ReplaceSubImage(re, imt, 7,49);
				QImage ims(":/data/tc_SensorModeOff.bmp");
				pTex->ReplaceSubImage(re, ims, 7,30);
			}
			break;
		case Planet:
			{
				QImage imp(":/data/tc_PlanetModeOn.bmp");
				pTex->ReplaceSubImage(re, imp, 7,68);
				QImage imt(":/data/tc_TransporterModeOff.bmp");
				pTex->ReplaceSubImage(re, imt, 7,49);
				QImage ims(":/data/tc_SensorModeOff.bmp");
				pTex->ReplaceSubImage(re, ims, 7,30);
			}
			break;
		case Transporter:
			{
				QImage imp(":/data/tc_PlanetModeOff.bmp");
				pTex->ReplaceSubImage(re, imp, 7,68);
				QImage imt(":/data/tc_TransporterModeOn.bmp");
				pTex->ReplaceSubImage(re, imt, 7,49);
				QImage ims(":/data/tc_SensorModeOff.bmp");
				pTex->ReplaceSubImage(re, ims, 7,30);
			}
			break;
		case Sensor:
			{
				QImage imp(":/data/tc_PlanetModeOff.bmp");
				pTex->ReplaceSubImage(re, imp, 7,68);
				QImage imt(":/data/tc_TransporterModeOff.bmp");
				pTex->ReplaceSubImage(re, imt, 7,49);
				QImage ims(":/data/tc_SensorModeOn.bmp");
				pTex->ReplaceSubImage(re, ims, 7,30);
			}
			break;
		}
	}

	if(m_Mode == Sensor)
	{
		// Show current state of toggles.
		if((m_bLife?1:0) != m_lLastLife)
		{
			m_lLastLife = m_bLife;

			QImage bmbut;
			if (m_bLife)
				bmbut.load(":/data/sc_LifeOn.bmp");
			else
				bmbut.load(":/data/sc_LifeOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,108);
		}
		if((m_bShips?1:0) != m_lLastShips)
		{
			m_lLastShips = m_bShips;

			QImage bmbut;
			if(m_bShips)
				bmbut.load(":/data/sc_ShipsOn.bmp");
			else
				bmbut.load(":/data/sc_ShipsOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,91);
		}
		if((m_bStations?1:0) != m_lLastStations)
		{
			m_lLastStations = m_bStations;

			QImage bmbut;
			if (m_bStations)
				bmbut.load(":/data/sc_StarbasesOn.bmp");
			else
				bmbut.load(":/data/sc_StarbasesOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,74);
		}
		if((m_bVehicles?1:0) != m_lLastVehicles)
		{
			m_lLastVehicles = m_bVehicles;

			QImage bmbut;
			if (m_bVehicles)
				bmbut.load(":/data/sc_VehiclesOn.bmp");
			else
				bmbut.load(":/data/sc_VehiclesOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,57);
		}
		if((m_bCities?1:0) != m_lLastCities)
		{
			m_lLastCities = m_bCities;

			QImage bmbut;
			if (m_bCities)
				bmbut.load(":/data/sc_CitiesOn.bmp");
			else
				bmbut.load(":/data/sc_CitiesOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,40);
		}
		if((m_bPlanets?1:0) != m_lLastPlanets)
		{
			m_lLastPlanets = m_bPlanets;

			QImage bmbut;
			if (m_bPlanets)
				bmbut.load(":/data/sc_PlanetsOn.bmp");
			else
				bmbut.load(":/data/sc_PlanetsOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,23);
		}
		if((m_bStars?1:0) != m_lLastStars)
		{
			m_lLastStars = m_bStars;

			QImage bmbut;
			if (m_bStars)
				bmbut.load(":/data/sc_StarsOn.bmp");
			else
				bmbut.load(":/data/sc_StarsOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,6);
		}
	}
	else if(m_Mode == Transporter)
	{
		// Show current state of toggles.
		if((m_bShips?1:0) != m_lLastShips)
		{
			m_lLastShips = m_bShips;

			QImage bmbut;
			if(m_bShips)
				bmbut.load(":/data/sc_ShipsOn.bmp");
			else
				bmbut.load(":/data/sc_ShipsOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,91);
		}
		if((m_bStations?1:0) != m_lLastStations)
		{
			m_lLastStations = m_bStations;

			QImage bmbut;
			if (m_bStations)
				bmbut.load(":/data/sc_StarbasesOn.bmp");
			else
				bmbut.load(":/data/sc_StarbasesOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,74);
		}
		if((m_bCities?1:0) != m_lLastCities)
		{
			m_lLastCities = m_bCities;

			QImage bmbut;
			if (m_bCities)
				bmbut.load(":/data/sc_CitiesOn.bmp");
			else
				bmbut.load(":/data/sc_CitiesOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,40);
		}

		// Blank the toggles that don't apply to the current view mode.
		if(m_lLastCities != -2)
		{
			m_lLastCities = -2;	// remember "blanked" state
			m_lLastLife = -2;
			m_lLastPlanets = -2;
			m_lLastStars = -2;
			m_lLastVehicles = -2;
			QImage bmbut(":data/ac_Blank16x16.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,108);		// life
			pTex->ReplaceSubImage(re, bmbut, 237,57);		// vehicles
			pTex->ReplaceSubImage(re, bmbut, 237,23);		// planets
			pTex->ReplaceSubImage(re, bmbut, 237,6);		// stars
		}
	}
	else
	{
		// Blank the toggles because they don't apply to the current view mode.
		if(m_lLastShips != -2)
		{
			m_lLastCities = -2;	// remember "blanked" state
			m_lLastLife = -2;
			m_lLastPlanets = -2;
			m_lLastShips = -2;
			m_lLastStations = -2;
			m_lLastVehicles = -2;
			m_lLastStars = -2;
			QImage bmbut(":data/ac_Blank16x16.bmp");
			pTex->ReplaceSubImage(re, bmbut, 237,108);		// life
			pTex->ReplaceSubImage(re, bmbut, 237,91);		// ships
			pTex->ReplaceSubImage(re, bmbut, 237,74);		// Stations
			pTex->ReplaceSubImage(re, bmbut, 237,57);		// vehicles
			pTex->ReplaceSubImage(re, bmbut, 237,40);		// cities
			pTex->ReplaceSubImage(re, bmbut, 237,23);		// planets
			pTex->ReplaceSubImage(re, bmbut, 237,6);		// stars
		}
	}

	if(m_PadsActive != m_LastPadsActive)
	{
		m_LastPadsActive = m_PadsActive;

		QPixmap pads(24, 16);
		pads.fill(QColor(0,0,0));
		QPainter p(&pads);
		p.setPen(QPen(QColor(0,64,128), 0.f));
		p.setFont(QFont(g_Au.TypefaceSans(), 10));
		QString text = QVariant(m_PadsActive).toString();
		p.drawText(QRect(0,0,pads.width(),pads.height()), Qt::AlignLeft|Qt::AlignVCenter, text);
		p.end();

		QImage im = pads.toImage();
		pTex->ReplaceSubImage(re, im, 14,4);
	}

	//if(tNow != m_tmLastUpdate)
	{
		//m_tmLastUpdate = tNow;

		if(m_Mode == Planet)
		{
			// Planet display.
			QPixmap screen(c_ScreenWidth, c_ScreenHeight);
			screen.fill(QColor(0,0,32));

			const DaWorld* planet = capsule.FindParentPlanetInEnv();
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
			if(llworld && llworld->SurfaceImage())
			{
				// Globe world
				// On globe type worlds, we will store the pixel dimensions of the surface image.
				int planetLonDivs = llworld->SurfaceImage()->width();
				int planetLatDivs = llworld->SurfaceImage()->height();

				// Determine the portion of the surface image that is on our screen.
				int xpos = (m_fRot.x + PIf) / (PIf*2) * planetLonDivs;			// map pixel location of center
				int ypos = (1.f - (m_fRot.y + PIf/2) / PIf) * planetLatDivs;		// map pixel location of center
				int vwht = (int)ceil(llworld->SurfaceImage()->height() * m_fScale);	// map pixels visible
				int vwwid = vwht * 2;												// map pixels visible
				int vwx = xpos - vwwid/2;				// map pixel at upper left
				int vwy = ypos - vwht/2;				// map pixel at upper left
				QRect srcRect(vwx,vwy, vwwid,vwht);

				QPainter p(&screen);
				p.setRenderHint(QPainter::SmoothPixmapTransform);
				p.drawImage(QRect(0,0,screen.width(),screen.height()), *llworld->SurfaceImage(), srcRect);

				// Draw a blip at the capsule's current location.
				AuPoint capsuleLoc;
				capsule.GetLocationRelativeToParent(capsuleLoc, llworld);
				Q_ASSERT(!capsuleLoc.IsNaN());
				AuGlobeLLF capsuleLL;
				llworld->LLOfLoc(capsuleLL, capsuleLoc);
				int sxpos = (capsuleLL.Longitude() + PIf) / (PIf*2) * planetLonDivs;		// map pixel location of capsule
				int sypos = (1.f - (capsuleLL.Latitude() + PIf/2) / PIf) * planetLatDivs;	// map pixel location of capsule
				int svwx = sxpos - vwx;
				if(svwx >= 0 && svwx < vwwid)
				{
					int svwy = sypos - vwy;
					if(svwy >= 0 && svwy < vwht)
					{
						// The capsule's location is on the screen.
						int scx = svwx * screen.width() / vwwid;
						int scy = svwy * screen.height() / vwht;
						if(AuElapsedTimer::CurrentSecIsOdd())
							p.setPen(QPen(QColor(255,255,0,128), 0.f));
						else
							p.setPen(QPen(QColor(0,0,0,128), 0.f));
						p.drawLine(scx-1,scy, scx+1,scy);
						p.drawLine(scx,scy-1, scx,scy+1);
					}
				}

				// Draw crosshair at target.
				const int xcen = screen.width()/2;
				const int ycen = screen.height()/2;
				p.setPen(QPen(QColor(0,255,0,128), 0.f));
				p.drawLine(0,ycen, xcen-1,ycen);
				p.drawLine(screen.width()-1,ycen, xcen+1,ycen);
				p.drawLine(xcen,0, xcen,ycen-1);
				p.drawLine(xcen,screen.height()-1, xcen,ycen+1);

				// Calculate two rects for drop shadow on long/lat text.
				QRect rc1(0,0,screen.width(),screen.height());
				QRect rc2 = rc1.translated(1,1);
				QString text1 = QString("%1 %2").arg(capsule.Name()).arg(capsuleLL.ToString());	// Long/lat of capsule.
				AuGlobeLLF centerLL(m_fRot.x, m_fRot.y);
				QString text2 = centerLL.ToString();										// Long/lat of crosshair.
				p.setFont(QFont(g_Au.TypefaceSans(), 8));

				p.setPen(QPen(QColor(0,0,0,128), 0.f));
				p.drawText(rc2, Qt::AlignRight|Qt::AlignTop, text2);
				p.drawText(rc2, Qt::AlignRight|Qt::AlignBottom, text1);

				// Show long/lat of the capsule.
				p.setPen(QPen(QColor(255,255,0,128), 0.f));
				p.drawText(rc1, Qt::AlignRight|Qt::AlignBottom, text1);

				// Show long/lat of crosshairs.
				p.setPen(QPen(QColor(0,255,0,128), 0.f));
				p.drawText(rc1, Qt::AlignRight|Qt::AlignTop, text2);

				p.end();
			}

			QImage im = screen.toImage();
			pTex->ReplaceSubImage(re, im, 32,25);

			// Lat long input in map mode.
			{
				QImage bmbut(":data/tc_LatLong.png");
				pTex->ReplaceSubImage(re, bmbut, 183,4);
			}

			// Blip Visibility by Political Stance is not relevant in this display.
			{
				m_LastShowAboard = -1;	// remember "blanked" state
				m_LastShowEmpire = -1;
				m_LastShowEnemy = -1;
				m_LastShowFriend = -1;
				m_LastShowNeutral = -1;
				QImage bmbut(":data/ac_Blank16x16.bmp");
				pTex->ReplaceSubImage(re, bmbut, 218,4);
				pTex->ReplaceSubImage(re, bmbut, 219,4);
				pTex->ReplaceSubImage(re, bmbut, 200,4);	// Show Aboard
			}
		}
		else
		{
			// Scanner display.
			QPixmap bms(c_ScreenWidth,c_ScreenHeight);
			bms.fill(QColor(0,0,32));
			QPainter p(&bms);

			// Make transforms to projection the locations into the bitmap.
			AuMatrix4x4 tdev;
			tdev.Translate(bms.width()/2.0, bms.height()/2.0, 0.0);
			double rpxscl = bms.height()/2.0;
			tdev.Scale(rpxscl, -rpxscl, rpxscl);	// Flip Y

			AuMatrix4x4 trot;
			trot.Rotate(m_fRot);
			trot.Scale(1.0/m_fScale, 1.0/m_fScale, 1.0/m_fScale);

			AuMatrix4x4 tcomb = tdev * trot * capsule.EnvTransform().Inverse();

			// Display information into the scanner display.
			const DaAtom* system = capsule.FindEnvironmentAtom();

			// Circle to show sensor range.
			AuPoint fpx = tcomb.Project(capsule.EnvLocation());
			fpx.x = bms.width()/2.f;
			fpx.y = bms.height()/2.f;
			QPoint pxme(round(fpx.x),round(fpx.y));
			float feetPerPixel = m_fScale/(bms.height()/2);
			float radius = m_MaxRange/feetPerPixel;	// diameter in pixels
			QRectF rcrange(fpx.x - radius, fpx.y - radius, radius*2.f, radius*2.f);
			p.drawImage(rcrange, s_SensorRange, QRectF(0,0,s_SensorRange.width(),s_SensorRange.height()));

			// Axis lines to sensor range.
			p.setPen(QPen(QColor(192,0,0), 0.f));
			fpx = tcomb.Project(capsule.EnvTransform().Project(m_MaxRange,0,0));
			p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

			p.setPen(QPen(QColor(64,0,0), 0.f));
			fpx = tcomb.Project(capsule.EnvTransform().Project(-m_MaxRange,0,0));
			p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

			p.setPen(QPen(QColor(0,192,0), 0.f));
			fpx = tcomb.Project(capsule.EnvTransform().Project(0,m_MaxRange,0));
			p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

			p.setPen(QPen(QColor(0,64,0), 0.f));
			fpx = tcomb.Project(capsule.EnvTransform().Project(0,-m_MaxRange,0));
			p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

			p.setPen(QPen(QColor(0,0,192), 0.f));
			fpx = tcomb.Project(capsule.EnvTransform().Project(0,0,m_MaxRange));
			p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

			p.setPen(QPen(QColor(0,0,64), 0.f));
			fpx = tcomb.Project(capsule.EnvTransform().Project(0,0,-m_MaxRange));
			p.drawLine(pxme.x(), pxme.y(), round(fpx.x),round(fpx.y));

			// Display blips that have been detected.
			AuPoint sysloc = capsule.EnvTransform().Project(location);
			if(m_Mode == Transporter)
				m_SensorBlips.DrawTransporterConsoleBlips(p, tcomb, feetPerPixel, m_SelBlip, m_idLockedBlip, true,
						false, m_bCities, false, m_bShips, m_bStations, false, false, sysloc,
						m_ShowEmpire, m_ShowFriend, m_ShowNeutral, m_ShowEnemy, system, capsule.Id(), capsule.EnvLocation());
			else
			{
				m_SensorBlips.DrawTransporterConsoleBlips(p, tcomb, feetPerPixel, m_SelBlip, m_idLockedBlip, false,
						m_bLife, m_bCities, m_bPlanets, m_bShips, m_bStations, m_bStars, m_bVehicles, sysloc,
						m_ShowEmpire, m_ShowFriend, m_ShowNeutral, m_ShowEnemy, system, capsule.Id(), capsule.EnvLocation());

				if(m_SelCoord)
				{
					AuPoint selcoord = capsule.EnvTransform().Project(m_Coord);

					AuPoint fpx = tcomb.Project(selcoord);
					QPoint pxSel = QPoint(round(fpx.x), round(fpx.y));
					p.setPen(QPen(QColor(0,255,255), 0.f));
					
					p.drawLine(pxSel.x()-5,pxSel.y()-5, pxSel.x()-7,pxSel.y()-5);
					p.drawLine(pxSel.x()-7,pxSel.y()-5, pxSel.x()-7,pxSel.y()+5);
					p.drawLine(pxSel.x()-7,pxSel.y()+5, pxSel.x()-5,pxSel.y()+5);

					p.drawLine(pxSel.x()+5,pxSel.y()-5, pxSel.x()+7,pxSel.y()-5);
					p.drawLine(pxSel.x()+7,pxSel.y()-5, pxSel.x()+7,pxSel.y()+5);
					p.drawLine(pxSel.x()+7,pxSel.y()+5, pxSel.x()+5,pxSel.y()+5);

					// Draw brief text next to reticle.
					double range = selcoord.Distance(sysloc);	// Distance in feet.
					QString sdist = AuSensorBlip::FormatBigNumber(range);

					AuPoint showCoord(m_Coord);
					showCoord /= FeetPerMeter();
					showCoord.Round(.1);
					QString sText = QString("(%1, %2, %3)\n%4").arg(showCoord.x).arg(showCoord.y).arg(showCoord.z).arg(sdist);

					p.setFont(QFont(g_Au.TypefaceSans(),7));
					p.setPen(QPen(QColor(0,192,192), 0.f));
					QRect rct(1,1,10,10);
					p.drawText(rct, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, sText);
				}
			}
			p.end();

			QImage ims = bms.toImage();
			pTex->ReplaceSubImage(re, ims, 32,25);

			// Coordinate input in scanner mode, not transporter to transporter mode.
			{
				QImage bmbut;
				if(m_Mode == Transporter)
					bmbut.load(":data/ac_Blank16x16.bmp");
				else
					bmbut.load(":data/tc_Coords.png");
				pTex->ReplaceSubImage(re, bmbut, 183,4);
			}

			if((m_ShowAboard?1:0) != m_LastShowAboard)
			{
				m_LastShowAboard = m_ShowAboard;

				QImage bmbut;
				if(m_ShowAboard)
					bmbut.load(":data/tc_IncludeShip.png");
				else
					bmbut.load(":data/tc_ExcludeShip.png");
				pTex->ReplaceSubImage(re, bmbut, 200,4);
			}

			// Blip Visibility by Political Stance
			if((m_ShowEmpire?1:0) != m_LastShowEmpire)
			{
				m_LastShowEmpire = m_ShowEmpire;

				QImage bmbut;
				if(m_ShowEmpire)
					bmbut.load(":/data/tc_EmpireOn.png");
				else
					bmbut.load(":/data/tc_EmpireOff.png");
				pTex->ReplaceSubImage(re, bmbut, 218,13);
			}
			if((m_ShowEnemy?1:0) != m_LastShowEnemy)
			{
				m_LastShowEnemy = m_ShowEnemy;

				QImage bmbut;
				if(m_ShowEnemy)
					bmbut.load(":/data/tc_EnemyOn.png");
				else
					bmbut.load(":/data/tc_EnemyOff.png");
				pTex->ReplaceSubImage(re, bmbut, 227,4);
			}
			if((m_ShowFriend?1:0) != m_LastShowFriend)
			{
				m_LastShowFriend = m_ShowFriend;

				QImage bmbut;
				if(m_ShowFriend)
					bmbut.load(":/data/tc_FriendOn.png");
				else
					bmbut.load(":/data/tc_FriendOff.png");
				pTex->ReplaceSubImage(re, bmbut, 227,13);
			}
			if((m_ShowNeutral?1:0) != m_LastShowNeutral)
			{
				m_LastShowNeutral = m_ShowNeutral;

				QImage bmbut;
				if(m_ShowNeutral)
					bmbut.load(":/data/tc_NeutralOn.png");
				else
					bmbut.load(":/data/tc_NeutralOff.png");
				pTex->ReplaceSubImage(re, bmbut, 218,4);
			}
		}
	}
}

void DpStationTransporter::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_TransporterConsoleControls.begin();
				myci != s_TransporterConsoleControls.end(); myci++)
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

void DpStationTransporter::RestoreDefaultControls()
{
	s_TransporterConsoleControls.clear();
	for(int ic=0; ic<s_NumTransporterConsoleDefaults; ic++)
		s_TransporterConsoleControls.append(s_TransporterConsoleDefaults[ic]);
	SaveControls();
}

void DpStationTransporter::LoadControls() const
{
	// Init with my default controls.
	s_TransporterConsoleControls.clear();
	for(int ic=0; ic<s_NumTransporterConsoleDefaults; ic++)
		s_TransporterConsoleControls.append(s_TransporterConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_TransporterConsoleControls.LoadSettings("Controls/Transporter Console");
}

void DpStationTransporter::SaveControls() const
{
	// Store my controls in the settings file.
	s_TransporterConsoleControls.SaveSettings("Controls/Transporter Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationTransporter::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_TransporterConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_TransporterConsoleControls.cbegin();
			ci != s_TransporterConsoleControls.cend(); ci++)
		aAllControls.append(*ci);
}

void DpStationTransporter::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& location)
{
	Q_UNUSED(location);
	if(ship.IsNew())
		return;	// This console cannot be used when the ship is "nu".
	ProcessControlInput(aActiveControls, sceneAtom, ship, cc);
}

void DpStationTransporter::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaBd& bd,
		CONTROL_CONTEXT cc, const AuPoint& location)
{
	Q_UNUSED(location);
	ProcessControlInput(aActiveControls, sceneAtom, bd, cc);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationTransporter::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaCapsule& capsule,
		CONTROL_CONTEXT cc)
{
	const double rRotInc = 5.0 * DEG2RAD;	// Rotation increment.
	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_InputCoord:
			if(cc == CONTROL_Server)
				continue;
			InputCoordinate(sceneAtom);
			break;
		case CI_AboardToggle:
			if(cc != CONTROL_Client)
				m_ShowAboard = !m_ShowAboard;
			if(cc != CONTROL_Server)
			{
				float bp = m_ShowAboard ? .9f : 1;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Aboard, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
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
				int troomIndex = capsule.TransporterSystemIndex(m_WhichTransporter);
				if(troomIndex < 0)
					break;
				if(capsule.IsTransporterSystemOn(troomIndex))
				{
					// Transporter room is on. Turn it off if it is not active.
					if(!capsule.IsTransporterSystemActive(troomIndex))
						capsule.TurnTransporterSystemOff(troomIndex);
				}
				else
				{
					// Transporter room is off. Turn it on.
					capsule.TurnTransporterSystemOn(troomIndex);
				}
			}
			if(cc != CONTROL_Server)
			{
				int troomIndex = capsule.TransporterSystemIndex(m_WhichTransporter);
				if(troomIndex < 0)
					break;
				if(capsule.IsTransporterSystemOn(troomIndex))
				{
					if(capsule.IsTransporterSystemActive(troomIndex))
						break;
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,.9f);
					sceneAtom->AddEffect(buttonClick);
				}
				else
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,1);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_ActivateFrom:
		case CI_ActivateTo:
			if(cc != CONTROL_Client)
			{
				// When the 'From' button is pushed, create origin teleporters in my tranporter room and destination teleporters at
				// the destination. We want teleporters "from" here.
				// When the 'To' button is pushed, create destination teleporters in my tranporter room and origin teleporters at
				// the destination.	We want teleporters "to" here.
				bool isFrom = true;
				if(ci.GetId() == CI_ActivateTo)
					isFrom = false;

				DaAtom* system = capsule.FindEnvironmentAtom();
				if(!system)
					break;	// Not really possible.

				if(!m_PadsActive)
				{
					QString text = QObject::tr("I must set the number of transporter pads to activate.");
					AuCommThought msg(text, sceneAtom->Id(), sceneAtom->Name());
					g_Au.SendAtomChat(msg);
					break;
				}

				// Locate the target atom and location.
				DaAtom* targAtom = 0;			// Parent atom of transport destination.
				AuPointArray targPads;			// Will be filled if the dest is transporter pads.
				quint16 targWhich = 0;			// 'which' transporter room id, when target is a transporter room
				AuPoint targLoc;
				AuPoint targZ(0,0,1);			// axis vectors at destination
				int targLimit = m_PadsActive;	// Target may limit the number of pads that an be active.
				bool ignoreDestShields = false;
				switch(m_Mode)
				{
				case Planet:
					{
						// Locate the target planet.
						DaWorld* planet = (DaWorld*)capsule.FindParentPlanetInEnv();
						if(!planet)
							break;	// Not at a planet. It is possible.
						DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
						if(!llworld)
							break;

						if(llworld)
						{
							// m_fRot is the long/lat, in radian angles.
							AuGlobeLLF targLL(m_fRot.x, m_fRot.y);
							targAtom = llworld;
							targLoc = llworld->GetSurfaceLocation(targLL);
							targZ = -llworld->GravityDir(targLoc);
						}
					}
					break;
				case Sensor:
					if(m_SelBlip)
					{
						AuId selId;
						m_BlipLock.lockForRead();
						if(m_SelBlip)
							selId = m_SelBlip->AtomId();
						m_BlipLock.unlock();

						// Locate the atom whose blip is selected on the screen.
						DaAtom* selAtom = system->FindIdRecursive(selId);
						if(!selAtom)
							break;

						// The target atom was found. We need info about it to transport there.
						sceneAtom->FindTransporterTarget(selAtom->Id(), selAtom->TransporterTarget(), targAtom, targLoc, targZ);
					}
					else if(m_SelCoord)
					{
						// The target atom was found. We need info about it to transport there.
						targAtom = capsule.Parent();
						targLoc = capsule.Transform().Project(m_Coord);
						targZ = capsule.Transform().ProjectVector(AuPoint(0,0,1));
						targZ.Unit();

						// Offset randomly from the target location.
						targLoc += AuPoint(g_Au.Dice().Roll(1,17,-9), g_Au.Dice().Roll(1,17,-9), g_Au.Dice().Roll(1,17,-9));
					}
					break;
				case Transporter:
					if(m_SelBlip)
					{
						m_BlipLock.lockForRead();
						// Locate the atom whose blip is selected on the screen.
						DaAtom* selAtom = system->FindIdRecursive(m_SelBlip->AtomId());
						if(selAtom)
						{
							// The selected atom was found. We need info about its transporters.
							DaCapsule* trycap = dynamic_cast<DaCapsule*>(selAtom);
							if(trycap)
							{
								// Locate the transporter room that was selected.
								targWhich = trycap->TransporterWhich(*m_SelBlip);
								trycap->TransporterPadLocations(targWhich, targPads);
								if(targLimit > targPads.count())
									targLimit = targPads.count();
								if(targLimit)
									targAtom = trycap;	// The capsule can be targetted.
							}
						}
						m_BlipLock.unlock();
					}
					break;
				case Init:
					break;
				}

				// As in Star Trek, energize...
				if(targAtom)
				{
					int result = capsule.EnergizeTransporter(m_WhichTransporter, isFrom, targLimit, targAtom, targLoc, targZ,
							targWhich, targPads, ignoreDestShields, true);

					// Report the result on the thoughts channel, good or bad.
					// This reply is coming from the person's own mind as a subconscious thought.
					QString text;
					if(result == targLimit && targLimit == 1)
						text = QObject::tr("Transporter energized.");
					else if(result == targLimit)
						text = QObject::tr("Transporter energized with %1 beams.").arg(result);
					else if(result > 0)
						text = QObject::tr("Transporter energized with only %1 beams. "
								"Other beams were blocked by obstacles near target.").arg(result);
					else if(result == 0)
						text = QObject::tr("Transporter beam was blocked by obstacles near target; I should try again.");
					else if(result == -1)
						text = QObject::tr("Transporter is not ready.");
					else if(result == -2)
						text = QObject::tr("Target is beyond transporter range.");
					else if(result == -3)
						text = QObject::tr("Transporter cannot lock on to target.");
					else if(result == -4)
						text = QObject::tr("Insufficient power to energize the transporter.");
					else if(result == -5)
						text = QObject::tr("Transporter has not finish its activation cycle.");
					else if(result == -10)
						text = QObject::tr("Transporter blocked by shields on target vessel.");
					else if(result == -13)
						text = QObject::tr("Transporter blocked by shield generator on the ground.");
					else
						text = QObject::tr("Unable to energize transporter.");
					AuCommThought msg(text, sceneAtom->Id(), sceneAtom->Name());
					g_Au.SendAtomChat(msg);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Energize, 1,.3f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PadsDec:
			if(cc != CONTROL_Client)
			{
				// Roll over to max on decrement below zero.
				if(m_PadsActive)
					m_PadsActive--;
				else
				{
					m_PadsActive = capsule.TransporterPadsWorking(m_WhichTransporter);
					if(m_PadsActive > 255)
						m_PadsActive = 255;
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Pads, 1,.95f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PadsInc:
			if(cc != CONTROL_Client)
			{
				// Limit pads active to the number of functioning pads in the transporter room.
				// Roll over to zero on increment beyond max.
				if(m_PadsActive < capsule.TransporterPadsWorking(m_WhichTransporter) && m_PadsActive < 255)
					m_PadsActive++;
				else
					m_PadsActive = 0;
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Pads, 1,1.05f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_PlanetMode:
			if(cc != CONTROL_Client)
			{
				if(m_Mode != Planet)
				{
					// Changing to planet map mode.
					m_Mode = Planet;
					m_fScale = 1;
					const DaWorld* planet = capsule.FindParentPlanetInEnv();
					const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
					if(llworld)
						m_fRot = AuPoint(0,0);
					else
						m_fRot = AuPoint(0,0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Mode, 1, .8f + Planet/20.f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_SensorMode:
			if(cc != CONTROL_Client)
			{
				if(m_Mode == Planet)
				{
					// Changing from planet map mode.
					m_fScale = (float)FeetPerMoonOrbit();
					m_fRot = AuPoint(0,0);
				}
				m_Mode = Sensor;
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Mode, 1, .8f + Sensor/20.f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_TransporterMode:
			if(cc != CONTROL_Client)
			{
				if(m_Mode == Planet)
				{
					// Changing from planet map mode.
					m_fScale = (float)FeetPerMoonOrbit();
					m_fRot = AuPoint(0,0);
				}
				m_Mode = Transporter;
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Mode, 1, .8f + Transporter/20.f);
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
		case CI_LifeToggle:
			if(cc != CONTROL_Client)
				m_bLife = !m_bLife;
			if(cc != CONTROL_Server)
			{
				float bp = m_bLife ? .9f : 1;	// button pitch
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

		case CI_RotateDown:
			if(cc == CONTROL_Client)
				continue;
			if(m_Mode == Planet)
			{
				// In planet mode, the rotation value is the long/lat.
				const DaWorld* planet = capsule.FindParentPlanetInEnv();
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
				if(llworld)
				{
					m_fRot.y += 10*DEG2RADf*m_fScale;
					if(m_fRot.y > PIf/2)
						m_fRot.y = PIf/2;
				}
				else
				{
					m_fRot.y -= .5;
					if(m_fRot.y < 0)
						m_fRot.y = 0;
				}
			}
			else
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
			if(cc == CONTROL_Client)
				continue;
			if(m_Mode == Planet)
			{
				// In planet mode, the rotation value is the long/lat.
				const DaWorld* planet = capsule.FindParentPlanetInEnv();
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
				const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
				if(rsworld)
				{
					m_fRot.x += 2*DEG2RADf*m_fScale;
					if(m_fRot.x > PIf)
						m_fRot.x -= PIf*2;
				}
				else if(llworld)
				{
					m_fRot.x += 10*DEG2RADf*m_fScale;
					if(m_fRot.x > PIf)
						m_fRot.x -= PIf*2;
				}
			}
			else
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
			if(cc == CONTROL_Client)
				continue;
			if(m_Mode == Planet)
			{
				// In planet mode, the rotation value is the long/lat.
				const DaWorld* planet = capsule.FindParentPlanetInEnv();
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
				const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
				if(rsworld)
				{
					m_fRot.x -= 2*DEG2RADf*m_fScale;
					if(m_fRot.x < -PIf)
						m_fRot.x += PIf*2;
				}
				else if(llworld)
				{
					m_fRot.x -= 10*DEG2RADf*m_fScale;
					if(m_fRot.x < -PIf)
						m_fRot.x += PIf*2;
				}
				else
				{
					m_fRot.x -= .5;
					if(m_fRot.x < 0)
						m_fRot.x = 0;
				}
			}
			else
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
			if(cc == CONTROL_Client)
				continue;
			if(m_Mode == Planet)
			{
				// In planet mode, the rotation value is the long/lat.
				const DaWorld* planet = capsule.FindParentPlanetInEnv();
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
				if(llworld)
				{
					m_fRot.y -= 10*DEG2RADf*m_fScale;
					if(m_fRot.y < -PIf/2)
						m_fRot.y = -PIf/2;
				}
			}
			else
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
				continue;
			if(m_Mode == Planet)
			{
				m_fScale /= 2;
				if(m_fScale < .125f)
					m_fScale = .125f;
			}
			else
			{
				if(m_fScale >= 2)
				{
					m_fScale /= 2;
					if(fabs(m_fScale - FeetPerMoonOrbit()) < 100.0)
						m_fScale = (float)FeetPerMoonOrbit();
					else if(fabs(m_fScale - FeetPerOrbit()) < 100.0)
						m_fScale = (float)FeetPerOrbit();
					else if(fabs(m_fScale - FeetPerAu()) < 1000.0)
						m_fScale = (float)FeetPerAu();
				}
				if(m_fScale < 1)
					m_fScale = 1;
			}
			break;
		case CI_ZoomOut:
			if(cc == CONTROL_Client)
				continue;
			if(m_Mode == Planet)
			{
				m_fScale *= 2;
				if(m_fScale > 1)
					m_fScale = 1;
			}
			else
			{
				if(m_fScale < FeetPerSolarSystem())
				{
					m_fScale *= 2;
					if(fabs(m_fScale - FeetPerMoonOrbit()) < 100.0)
						m_fScale = (float)FeetPerMoonOrbit();
					else if(fabs(m_fScale - FeetPerOrbit()) < 100.0)
						m_fScale = (float)FeetPerOrbit();
					else if(fabs(m_fScale - FeetPerAu()) < 1000.0)
						m_fScale = (float)FeetPerAu();
				}
			}
			break;
		case CI_ZoomNormal:
			if(cc == CONTROL_Client)
				continue;
			if(m_Mode == Planet)
				m_fScale = 1;
			else
				m_fScale = (float)FeetPerMoonOrbit();
			break;

		case CI_RotateTopView:
			if(cc != CONTROL_Client)
			{
				if(m_Mode == Planet)
				{
					// In planet mode, the rotation value is the long/lat.
					const DaWorld* planet = capsule.FindParentPlanetInEnv();
					const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
					if(llworld)
						m_fRot = AuPoint(0,0);
					else
						m_fRot = AuPoint(0,0);
				}
				else
					m_fRot = AuPoint(0,0);
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_RotateToTarget:
			if(cc != CONTROL_Client)
			{
				if(m_Mode != Planet)
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
						AuPoint vecToTarget = capsule.EnvTransform().UnProject(selLoc);
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
					else if(m_SelCoord)
					{
						AuPoint vecToTarget = m_Coord;
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
			}
			if(cc != CONTROL_Server)
			{
				if(m_Mode != Planet)
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.9f);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;

		case CI_DisplayScreen:
			if(cc == CONTROL_Client)
				continue;
			ProcessDisplayHit(ci.HitLocation(), capsule);
			break;

		case CI_Center:
			if(cc != CONTROL_Client)
			{
				if(m_Mode == Planet)
				{
					// Rotate the display to center on the capsule. This is handy at ringworlds.
					// In planet mode, the rotation value is the long/lat.
					const DaWorld* planet = capsule.FindParentPlanetInEnv();
					const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
					if(llworld)
					{
						AuPoint capsuleLoc;
						capsule.GetLocationRelativeToParent(capsuleLoc, llworld);
						Q_ASSERT(!capsuleLoc.IsNaN());
						AuGlobeLLF capsuleLL;
						if(llworld->LLOfLoc(capsuleLL, capsuleLoc))
							SetLongLat(capsuleLL);
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				if(m_Mode == Planet)
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.8f);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		}
	}
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DpStationTransporter::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
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
	// we've calculated that is also measured from the bottom, not the top as usual in GDI.
	DiPanelControl acontrols[] =
	{
		// Left side
		DiPanelControl(CI_PowerToggle, QRect(7,106, 16,16)),
		DiPanelControl(CI_PlanetMode, QRect(7,68, 16,16)),
		DiPanelControl(CI_TransporterMode, QRect(7,49, 16,16)),
		DiPanelControl(CI_SensorMode, QRect(7,30, 16,16)),
		DiPanelControl(CI_PadsDec, QRect(3,4, 9,9)),
		DiPanelControl(CI_PadsInc, QRect(3,14, 9,9)),

		// Bottom row
		DiPanelControl(CI_Center, QRect(40,14,7,7)),
		DiPanelControl(CI_RotateToTarget, QRect(40,5,7,7)),
		DiPanelControl(CI_RotateTopView, QRect(55,10, 5,5)),
		DiPanelControl(CI_RotateUp, QRect(55,16, 5,5), true),
		DiPanelControl(CI_RotateDown, QRect(55,4, 5,5), true),
		DiPanelControl(CI_RotateLeft, QRect(49,10, 5,5), true),
		DiPanelControl(CI_RotateRight, QRect(61,10, 5,5), true),
		DiPanelControl(CI_ZoomIn, QRect(68,16,5,5)),
		DiPanelControl(CI_ZoomNormal, QRect(68,10,5,5)),
		DiPanelControl(CI_ZoomOut, QRect(68,4,5,5)),
		DiPanelControl(CI_ActivateTo, QRect(162,4, 16,16)),
		DiPanelControl(CI_ActivateFrom, QRect(78,4, 16,16)),
		DiPanelControl(CI_InputCoord, QRect(183,4, 16,16)),
		DiPanelControl(CI_AboardToggle, QRect(200,4, 16,16)),
		DiPanelControl(CI_EmpireToggle, QRect(218,13,8,7)),
		DiPanelControl(CI_NeutralToggle, QRect(218,4,8,7)),
		DiPanelControl(CI_FriendToggle, QRect(227,13,8,7)),
		DiPanelControl(CI_EnemyToggle, QRect(227,4,8,7)),

		// Right side
		DiPanelControl(CI_LifeToggle, QRect(237,108,16,16)),
		DiPanelControl(CI_ShipsToggle, QRect(237,91,16,16)),
		DiPanelControl(CI_StationsToggle, QRect(237,74,16,16)),
		DiPanelControl(CI_VehiclesToggle, QRect(237,57,16,16)),
		DiPanelControl(CI_CitiesToggle, QRect(237,40,16,16)),
		DiPanelControl(CI_PlanetsToggle, QRect(237,23,16,16)),
		DiPanelControl(CI_StarsToggle, QRect(237,6,16,16)),

		// Display screen
		DiPanelControl(CI_DisplayScreen, QRect(32,25, c_ScreenWidth,c_ScreenHeight), false, true),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);

	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

void DpStationTransporter::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
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

void DpStationTransporter::ProcessDisplayHit(const QPoint &pxHit, const DaAtom& hostAtom)
{
	if(m_Mode == Planet)
	{
		// Determine long/lat of hit location.
		const DaWorld* planet = hostAtom.FindParentPlanetInEnv();
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
		const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
		if(rsworld)
		{
			// Determine long/lat at center of display based on current view rotation.
			// Calculate number of divisions per display screen pixel at the current view scale.
			int ilondivs = rsworld->SeaLevelEquatorDivisions();
			int ilatdivs = rsworld->NorthSouthDivisions();
			double sclX = m_fScale * (PIf*2) / c_ScreenWidth;
			sclX /= (double)ilondivs/(double)(ilatdivs*2);	// This is the amount that is actually visible in the display.
			double sclY = m_fScale * (PIf) / c_ScreenHeight;

			// Calculate long/lat of hit location relative to center.
			int relX = pxHit.x() - c_ScreenWidth/2;
			int relY = pxHit.y() - c_ScreenHeight/2;
			m_fRot.x += sclX * relX;
			if(m_fRot.x < -PI)
				m_fRot.x += PI*2;
			else if(m_fRot.x >= PI)
				m_fRot.x -= PI*2;

			m_fRot.y -= sclY * relY;
			if(m_fRot.y >= PI/2)
				m_fRot.y = PI/2;
			else if(m_fRot.y < -PI/2)
				m_fRot.y = -PI/2;
		}
		else if(llworld)
		{
			// Determine long/lat at center of display based on current view rotation.
			double scl = m_fScale * (PIf*2) / c_ScreenWidth;

			// Calculate long/lat of hit location relative to center.
			int relX = pxHit.x() - c_ScreenWidth/2;
			int relY = pxHit.y() - c_ScreenHeight/2;

			m_fRot.x += scl * relX;
			if(m_fRot.x < -PI)
				m_fRot.x += PI*2;
			else if(m_fRot.x >= PI)
				m_fRot.x -= PI*2;

			m_fRot.y -= scl * relY;
			if(m_fRot.y >= PI/2)
				m_fRot.y = PI/2;
			else if(m_fRot.y < -PI/2)
				m_fRot.y = -PI/2;
		}
	}
	else
	{
		const double rseldist = 5.0;	// Pixel distance to select.

		m_BlipLock.lockForWrite();
		if(m_SelBlip)
			delete m_SelBlip;

		// Find the closest atom in the display.
		double radist = -1.0;	// Distance to the nearest atom in display pixels.
		AuSensorBlipList::const_iterator blip = FindClosestDisplayBlip(pxHit, radist, hostAtom);
		if(blip == m_SensorBlips.end() || radist > rseldist)
			m_SelBlip = 0;
		else
			m_SelBlip = (*blip)->Clone();	// An object was selected that is different than the current selection.
		m_BlipLock.unlock();

		m_SelCoord = false;
	}
}

AuSensorBlipList::const_iterator DpStationTransporter::FindClosestDisplayBlip(const QPoint& pxHit, double& rDisplayDistance,
		const DaAtom& hostAtom) const
{
	const DaAtom* system = hostAtom.FindEnvironmentAtom();

	QSize sz(c_ScreenWidth,c_ScreenHeight);

	// Make transforms to projection the locations into the bitmap.
	AuMatrix4x4 tdev;
	tdev.Translate(sz.width()/2.0, sz.height()/2.0, 0.0);
	double rpxscl = sz.height()/2.0;
	tdev.Scale(rpxscl, -rpxscl, rpxscl);	// Flip Y

	AuMatrix4x4 trot;
	trot.Rotate(m_fRot);
	trot.Scale(1.0/m_fScale, 1.0/m_fScale, 1.0/m_fScale);

	AuMatrix4x4 tcomb = tdev * trot * hostAtom.EnvTransform().Inverse();

	// Calc distance to blips.
	float rFeetPerPixel = m_fScale/(sz.height()/2);
	AuSensorBlipList::const_iterator pbest = m_SensorBlips.end();
	for(AuSensorBlipList::const_iterator pb = m_SensorBlips.begin(); pb != m_SensorBlips.end(); pb++)
	{
		// Filter based on political stance.
		if(!(*pb)->TestStance(m_ShowEmpire, m_ShowFriend, m_ShowNeutral, m_ShowEnemy, true))
			continue;

		if((*pb)->IsCreature())
		{
			if(!m_bLife || m_Mode == Transporter)
				continue;
		}
		else if((*pb)->IsVehicle())
		{
			if(!m_bVehicles || m_Mode == Transporter)
				continue;
		}
		else if((*pb)->IsCity() || (*pb)->IsBuilding())
		{
			if(!m_bCities || m_Mode == Transporter)
				continue;
		}
		else if((*pb)->IsSpaceStation())
		{
			if(!m_bStations)
				continue;
			if(m_Mode == Transporter)
				continue;	// Looking for transporter rooms that are on and inactive.
			if(hostAtom.Is((*pb)->AtomId()))
				continue;	// Don't show the capsule that contains the panel.
		}
		else if((*pb)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*pb)->IsSpacecraft())
		{
			if(!m_bShips)
				continue;
			if(m_Mode == Transporter)
				continue;	// Looking for transporter rooms that are on and inactive.
			if(hostAtom.Is((*pb)->AtomId()))
				continue;	// Don't show the capsule that contains the panel.
		}
		else if((*pb)->IsTransporterRoomCity())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!m_bCities)
				continue;
			if(m_Mode != Transporter)
				continue;
		}
		else if((*pb)->IsTransporterRoomStation())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!m_bStations)
				continue;
			if(m_Mode != Transporter)
				continue;
		}
		else if((*pb)->IsTransporterRoomSpacecraft())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!m_bShips)
				continue;
			if(m_Mode != Transporter)
				continue;
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

void DpStationTransporter::LoadClientSettings(const AuId& planetId)
{
	m_Mode = (MODE)g_Au.Settings().readNumEntry(QString("/TransporterStation/%1/Mode").arg(planetId.StringId()), m_Mode);
	m_PadsActive = g_Au.Settings().readNumEntry("/TransporterStation/PadsActive", m_PadsActive);
	m_bCities = g_Au.Settings().readBoolEntry("/TransporterStation/CitiesOn", m_bCities);
	m_bLife = g_Au.Settings().readBoolEntry("/TransporterStation/LifeOn", m_bLife);
	m_bPlanets = g_Au.Settings().readBoolEntry("/TransporterStation/PlanetsOn", m_bPlanets);
	m_bShips = g_Au.Settings().readBoolEntry("/TransporterStation/ShipsOn", m_bShips);
	m_bStations = g_Au.Settings().readBoolEntry("/TransporterStation/StationsOn", m_bStations);
	m_bStars = g_Au.Settings().readBoolEntry("/TransporterStation/StarsOn", m_bStars);
	m_bVehicles = g_Au.Settings().readBoolEntry("/TransporterStation/VehiclesOn", m_bVehicles);
	m_fRot.x = g_Au.Settings().readNumEntry(QString("/TransporterStation/%1/XRotation").arg(planetId.StringId()), m_fRot.x);
	m_fRot.y = g_Au.Settings().readNumEntry(QString("/TransporterStation/%1/YRotation").arg(planetId.StringId()), m_fRot.y);
	m_fRot.z = g_Au.Settings().readNumEntry(QString("/TransporterStation/%1/ZRotation").arg(planetId.StringId()), m_fRot.z);
	m_fScale = g_Au.Settings().readNumEntry(QString("/TransporterStation/%1/Scale").arg(planetId.StringId()), m_fScale);
	if(m_Mode == Planet)
	{
		if(m_fScale < .125f || m_fScale > 1)
			m_fScale = 1;
	}
	else
	{
		if(m_fScale < 1 || m_fScale > FeetPerSolarSystem())
			m_fScale = FeetPerMoonOrbit();
	}
	m_ShowAboard = g_Au.Settings().readBoolEntry("/TransporterStation/ShowAboard", m_ShowAboard);
	m_ShowEmpire = g_Au.Settings().readBoolEntry("/TransporterStation/ShowEmpire", m_ShowEmpire);
	m_ShowEnemy = g_Au.Settings().readBoolEntry("/TransporterStation/ShowEnemy", m_ShowEnemy);
	m_ShowFriend = g_Au.Settings().readBoolEntry("/TransporterStation/ShowFriend", m_ShowFriend);
	m_ShowNeutral = g_Au.Settings().readBoolEntry("/TransporterStation/ShowNeutral", m_ShowNeutral);
}

void DpStationTransporter::StoreClientSettings(const AuId& planetId) const
{
	g_Au.Settings().writeEntry(QString("/TransporterStation/%1/Mode").arg(planetId.StringId()), m_Mode);
	g_Au.Settings().writeEntry("/TransporterStation/PadsActive", m_PadsActive);
	g_Au.Settings().writeEntry("/TransporterStation/CitiesOn", m_bCities);
	g_Au.Settings().writeEntry("/TransporterStation/LifeOn", m_bLife);
	g_Au.Settings().writeEntry("/TransporterStation/PlanetsOn", m_bPlanets);
	g_Au.Settings().writeEntry("/TransporterStation/ShipsOn", m_bShips);
	g_Au.Settings().writeEntry("/TransporterStation/StationsOn", m_bStations);
	g_Au.Settings().writeEntry("/TransporterStation/StarsOn", m_bStars);
	g_Au.Settings().writeEntry("/TransporterStation/VehiclesOn", m_bVehicles);
	g_Au.Settings().writeEntry(QString("/TransporterStation/%1/XRotation").arg(planetId.StringId()), m_fRot.x);
	g_Au.Settings().writeEntry(QString("/TransporterStation/%1/YRotation").arg(planetId.StringId()), m_fRot.y);
	g_Au.Settings().writeEntry(QString("/TransporterStation/%1/ZRotation").arg(planetId.StringId()), m_fRot.z);
	g_Au.Settings().writeEntry(QString("/TransporterStation/%1/Scale").arg(planetId.StringId()), m_fScale);
	g_Au.Settings().writeEntry("/TransporterStation/ShowAboard", m_ShowAboard);
	g_Au.Settings().writeEntry("/TransporterStation/ShowEmpire", m_ShowEmpire);
	g_Au.Settings().writeEntry("/TransporterStation/ShowEnemy", m_ShowEnemy);
	g_Au.Settings().writeEntry("/TransporterStation/ShowFriend", m_ShowFriend);
	g_Au.Settings().writeEntry("/TransporterStation/ShowNeutral", m_ShowNeutral);
}

void DpStationTransporter::TxSettings(QDataStream& out) const
{
	out << (quint8)m_Mode;
	out << (quint8)m_PadsActive;

	out << (float)m_fRot.x;
	out << (float)m_fRot.y;
	out << (float)m_fRot.z;
	out << m_fScale;

	quint16 bits = 0;
	if(m_bPlanets)
		bits |= 0x01;
	if(m_bShips)
		bits |= 0x02;
	if(m_bStations)
		bits |= 0x04;
	if(m_bStars)
		bits |= 0x08;
	if(m_bCities)
		bits |= 0x10;
	if(m_bLife)
		bits |= 0x20;
	if(m_bVehicles)
		bits |= 0x40;
	if(m_ShowAboard)
		bits |= 0x0800;
	if(m_ShowEmpire)
		bits |= 0x1000;
	if(m_ShowFriend)
		bits |= 0x2000;
	if(m_ShowNeutral)
		bits |= 0x4000;
	if(m_ShowEnemy)
		bits |= 0x8000;
	out << bits;
}

void DpStationTransporter::RxSettings(QDataStream& in)
{
	quint8 ui8;
	in >> ui8;
	m_Mode = (MODE)ui8;
	in >> ui8;
	m_PadsActive = ui8;

	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fRot = AuPoint(x,y,z);
	in >> m_fScale;

	quint16 bits;
	in >> bits;
	m_bPlanets = (bits & 0x01) ? true : false;
	m_bShips = (bits & 0x02) ? true : false;
	m_bStations = (bits & 0x04) ? true : false;
	m_bStars = (bits & 0x08) ? true : false;
	m_bCities = (bits & 0x10) ? true : false;
	m_bLife = (bits & 0x20) ? true : false;
	m_bVehicles = (bits & 0x40) ? true : false;
	m_ShowAboard = (bits & 0x0800) ? true : false;
	m_ShowEmpire = (bits & 0x1000) ? true : false;
	m_ShowFriend = (bits & 0x2000) ? true : false;
	m_ShowNeutral = (bits & 0x4000) ? true : false;
	m_ShowEnemy = (bits & 0x8000) ? true : false;
}

void DpStationTransporter::InputCoordinate(DaPerson* person)
{
	switch(m_Mode)
	{
	case Init:
	case Transporter:
		break;

	case Planet:
		{
			AuGlobeLLF ll(m_fRot.x, m_fRot.y);
			person->TransporterLongLat(ll);
		}
		break;

	case Sensor:
		{
			const DaCapsule* capsule = dynamic_cast<const DaCapsule*>(person->Parent());
			if(capsule)
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

				AuPoint loc = person->Location();
				if(hasLoc)
					loc = capsule->EnvTransform().UnProject(selLoc);	// Relative to the capsule.
				else if(m_SelCoord)
					loc = m_Coord;
				person->TransporterCoord(loc);
				break;
			}
		}
		break;
	}
}

void DpStationTransporter::SetCoord(const AuPoint& coord)
{
	// Cancel the selected blip.
	m_BlipLock.lockForWrite();
	if(m_SelBlip)
	{
		delete m_SelBlip;
		m_SelBlip = 0;
	}
	m_BlipLock.unlock();

	// Set up for selected coordinate.
	m_SelCoord = true;
	m_Coord = coord;
}

void DpStationTransporter::SetLongLat(const AuGlobeLLF& ll)
{
	m_fRot.x = ll.Longitude();
	m_fRot.y = ll.Latitude();
}

DpStation::STATION_TYPE DpStationTransporter::ClientRestoreType() const
{
	return STATION_Transporter;
}

void DpStationTransporter::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationTransporter::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("TCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/txConsoleTransporter.bmp");
}

void DpStationTransporter::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationTransporter::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_idLockedBlip);
	mem += sizeof(m_Mode);
	mem += sizeof(m_PadsActive);
	mem += sizeof(m_fRot);
	mem += sizeof(m_fScale);
	mem += sizeof(m_bCities);
	mem += sizeof(m_bLife);
	mem += sizeof(m_bPlanets);
	mem += sizeof(m_bShips);
	mem += sizeof(m_bStations);
	mem += sizeof(m_bStars);
	mem += sizeof(m_bVehicles);
	mem += sizeof(m_MaxRange);
	mem += sizeof(m_ShowAboard);
	mem += sizeof(m_ShowEmpire);
	mem += sizeof(m_ShowEnemy);
	mem += sizeof(m_ShowFriend);
	mem += sizeof(m_ShowNeutral);
	mem += sizeof(m_SelCoord);
	mem += sizeof(m_Coord);
	mem += sizeof(m_BlipLock);
	mem += sizeof(m_SelBlip);
	if(m_SelBlip)
		mem += m_SelBlip->MemoryUsage();
	mem += m_SensorBlips.MemoryUsage();
	mem += sizeof(m_LastMode);
	mem += sizeof(m_LastPadsActive);
	mem += sizeof(m_LastShowAboard);
	mem += sizeof(m_LastShowEmpire);
	mem += sizeof(m_LastShowEnemy);
	mem += sizeof(m_LastShowFriend);
	mem += sizeof(m_LastShowNeutral);
	mem += sizeof(m_lLastActive);
	mem += sizeof(m_lLastCities);
	mem += sizeof(m_lLastLife);
	mem += sizeof(m_lLastPlanets);
	mem += sizeof(m_lLastPowerOn);
	mem += sizeof(m_lLastShips);
	mem += sizeof(m_lLastStations);
	mem += sizeof(m_lLastStars);
	mem += sizeof(m_lLastVehicles);
	mem += sizeof(m_WhichTransporter);
	mem += sizeof(m_ScreenTex);
	return mem;
}
