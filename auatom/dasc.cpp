//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasc.cpp,v 1.561 2022/05/17 16:32:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dasc.h"
//
#include "auatomrenderinfo.h"
#include "aubox.h"
#include "aucommpacket.h"
#include "augame.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "auscroute.h"
#include "ausound.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dateleporter.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldringsection.h"
#include "dpsccommand.h"
#include "dpscengineer.h"
#include "dpschelm.h"
#include "dpscnav.h"
#include "dpscpower.h"
#include "dpscsensor.h"
#include "dpscshield.h"
#include "dpscsurgery.h"
#include "dpsctransporter.h"
#include "dpscweapon.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "psfiresmoketrail.h"
#include "pslandingsystem.h"
#include "pslaunchsystem.h"
#include <QSqlError>
#include <QSqlQuery>

#define d_SpacecraftLight				(200*FeetPerMeter())	// Attenuation distance of exterior light.

DaSc::DaSc() : m_CmdEmpty(),
	m_EngineEmpty(),
	m_FireEmpty(),
	m_NavEmpty(),
	m_PilotEmpty(),
	m_PowerEmpty(),
	m_SensorEmpty(),
	m_ShieldEmpty(),
	m_SurgeryEmpty(),
	m_TransEmpty()
{
	m_AlertCondition = ConditionGreen;
	m_AlertGreen = 0;
	m_AlertRedBright = 0;
	m_AlertRedDim = 0;
	m_AlertStatus = -1;
	m_AlertYellowBright = 0;
	m_AlertYellowDim = 0;
	m_BerthNameDecals = -1;
	m_CaptNameDecals = -1;
	m_ContrailColor = QColor(255,255,255);
	m_ContrailsValid = false;
	m_DesignCacheValid = false;
	m_DoorRoomsValid = false;
	m_EmpireFlagDecals = -1;
	m_EmpireNameDecals = -1;
	m_FlagEmpire = 0;
	m_LandingSystem = 0;
	m_LaunchSystem = 0;
	m_LifeSupportStatus = -1;
	m_LifeSupportStatusTex = 0;
	m_MissionStatusETESecs = -1;
	m_MissionStatusTex = 0;
	m_NavLights = false;
	m_New = true;
	m_RotatingBeacons = false;
	m_ShipNameDecals = -1;
	m_Strobes = false;
	m_TargetLocValid = false;
	m_Ticked = false;
	m_Tocked = false;
	m_WorldMaps = -1;
	m_WorldMapTex = 0;
	for(int as=0; as<AS_MaxSources; as++)
		m_AudioShip[as] = 0;
	m_DoorExt = 0;		// 0=no change, 1=opened, 2=closed
	m_DoorInt1 = 0;		// 0=no change, 1=opened, 2=closed
	m_DoorInt2 = 0;		// 0=no change, 1=opened, 2=closed
	m_DoorLift1 = 0;	// 0=no change, 1=opened, 2=closed
	m_DoorLift2 = 0;	// 0=no change, 1=opened, 2=closed
	for(int dmg=0; dmg<9; dmg++)
		m_DamageTex[dmg] = 0;
	for(int eff=EFF_None; eff<EFF_Count; eff++)
		m_EffectCur[eff] = 0;
	m_EffectFrame = 0;
	m_EffectRandFrame = 0;
	for(int eff=EFF_None; eff<EFF_Count+6; eff++)
		for(int frame=0; frame<6; frame++)
			m_EffectTex[eff][frame] = 0;
	m_HColor = QColor(AuGLLight::ColorCarbonArc());
	m_HCosInnerCone = -1;
	m_HCosOuterCone = -1;
	m_HullEffectAppl = 0;
	m_HDir = AuPoint(0,1,0);
	m_HkL = .005f;
	m_HkQ = 1.f/(float)(d_SpacecraftLight * d_SpacecraftLight);
	m_LensBeaconAngle = 0;
	m_LightOmniTex = 0;
	m_LightSpotTex = 0;
	m_Siren = 0;	// 0=no change, 1=opened, 2=closed
	m_StrobesLit = 0;
	m_VAOsCreated = false;

	// Current ambient light and fog states.
	m_AmbientR = 0;
	m_AmbientG = 0;
	m_AmbientB = 0;
	m_FogDensity = 0;
	m_FogR = 0;
	m_FogG = 0;
	m_FogB = 0;
	m_InsideVisible = false;
	m_IsAboard = false;
	m_MyExtLight = 0;
	m_SunsBlocked = false;
}

DaSc::~DaSc()
{
	// Delete contrail emitters.
	while(!m_Contrails.empty())
		delete m_Contrails.takeFirst();
	if(m_LandingSystem)
		delete m_LandingSystem;
	if(m_LaunchSystem)
		delete m_LaunchSystem;
}

DaSc::DaSc(const DaSc& rhs) : DaVessel(rhs),
	m_CmdEmpty(rhs.m_CmdEmpty),
	m_EngineEmpty(rhs.m_EngineEmpty),
	m_FireEmpty(rhs.m_FireEmpty),
	m_NavEmpty(rhs.m_NavEmpty),
	m_PilotEmpty(rhs.m_PilotEmpty),
	m_PowerEmpty(rhs.m_PowerEmpty),
	m_SensorEmpty(rhs.m_SensorEmpty),
	m_ShieldEmpty(rhs.m_ShieldEmpty),
	m_SurgeryEmpty(rhs.m_SurgeryEmpty),
	m_TransEmpty(rhs.m_TransEmpty)
{
	m_AlertCondition = rhs.m_AlertCondition;
	m_AlertGreen = 0;
	m_AlertRedBright = 0;
	m_AlertRedDim = 0;
	m_AlertStatus = -1;
	m_AlertYellowBright = 0;
	m_AlertYellowDim = 0;
	m_BerthNameDecals = -1;
	m_ContrailColor = rhs.m_ContrailColor;
	m_ContrailsValid = false;
	m_CaptNameDecals = -1;
	m_DbLoadTime = rhs.m_DbLoadTime;
	m_DesignCacheValid = rhs.m_DesignCacheValid;
	m_EmpireFlagDecals = -1;
	m_EmpireNameDecals = -1;
	m_FlagEmpire = 0;
	m_LandingSystem = 0;
	m_LaunchSystem = 0;
	m_LifeSupportStatus = -1;
	m_LifeSupportStatusTex = 0;
	m_MissionStatusETESecs = -1;
	m_MissionStatusTex = 0;
	m_NavLights = rhs.m_NavLights;
	m_New = rhs.m_New;
	m_RotatingBeacons = rhs.m_RotatingBeacons;
	m_ShipNameDecals = -1;
	m_Strobes = rhs.m_Strobes;
	m_Ticked = rhs.m_Ticked;
	m_Tocked = rhs.m_Tocked;
	m_WorldMaps = -1;
	m_WorldMapTex = 0;

	rhs.m_DesignLock.lockForRead();
	m_DesignChg = rhs.m_DesignChg;
	m_Design = rhs.m_Design;
	m_DoorChg = rhs.m_DoorChg;
	m_DoorOpenTimes = rhs.m_DoorOpenTimes;
	m_DoorStates = rhs.m_DoorStates;
	m_LandingGearChg = rhs.m_LandingGearChg;
	m_LandingGearStates = rhs.m_LandingGearStates;
	m_LiftCalls = rhs.m_LiftCalls;
	m_LiftChg = rhs.m_LiftChg;
	m_Lifts = rhs.m_Lifts;
	m_TargetLoc = rhs.m_TargetLoc;
	m_TargetLocValid = rhs.m_TargetLocValid;
	rhs.m_DesignLock.unlock();

	rhs.m_AirLock.lockForRead();
	m_AirChg = rhs.m_AirChg;
	m_AirPressures = rhs.m_AirPressures;
	rhs.m_AirLock.unlock();

	rhs.m_FTLLock.lockForRead();
	m_FTLChg = rhs.m_FTLChg;
	m_FTLDrives = rhs.m_FTLDrives;
	rhs.m_FTLLock.unlock();

	rhs.m_LandingSystemLock.lockForRead();
	m_LandingSystemChg = rhs.m_LandingSystemChg;
	m_LandingSystemStates = rhs.m_LandingSystemStates;
	rhs.m_LandingSystemLock.unlock();

	rhs.m_LauncherLock.lockForRead();
	m_LauncherChg = rhs.m_LauncherChg;
	m_LauncherStates = rhs.m_LauncherStates;
	rhs.m_LauncherLock.unlock();

	rhs.m_RoomLightLock.lockForRead();
	m_RoomLightChg = rhs.m_RoomLightChg;
	m_RoomLightStates = rhs.m_RoomLightStates;
	rhs.m_RoomLightLock.unlock();

	rhs.m_TRoomLock.lockForRead();
	m_TRoomChg = rhs.m_TRoomChg;
	m_TRoomStates = rhs.m_TRoomStates;
	rhs.m_TRoomLock.unlock();

	rhs.m_TurretPosLock.lockForRead();
	m_TurretPosChg = rhs.m_TurretPosChg;
	m_TurretPos = rhs.m_TurretPos;
	rhs.m_TurretPosLock.unlock();

	for(int as=0; as<AS_MaxSources; as++)
		m_AudioShip[as] = rhs.m_AudioShip[as];
	m_DoorExt = rhs.m_DoorExt;
	m_DoorExtLoc = rhs.m_DoorExtLoc;
	m_DoorInt1 = rhs.m_DoorInt1;
	m_DoorInt1Loc = rhs.m_DoorInt1Loc;
	m_DoorInt2 = rhs.m_DoorInt2;
	m_DoorInt2Loc = rhs.m_DoorInt2Loc;
	m_DoorLift1 = rhs.m_DoorLift1;
	m_DoorLift1Loc = rhs.m_DoorLift1Loc;
	m_DoorLift2 = rhs.m_DoorLift2;
	m_DoorLift2Loc = rhs.m_DoorLift2Loc;
	for(int dmg=0; dmg<9; dmg++)
		m_DamageTex[dmg] = rhs.m_DamageTex[dmg];
	for(int eff=EFF_None; eff<EFF_Count; eff++)
		m_EffectCur[eff] = rhs.m_EffectCur[eff];
	m_EffectFrame = rhs.m_EffectFrame;
	m_EffectRandFrame = rhs.m_EffectRandFrame;
	for(int eff=EFF_None; eff<EFF_Count+6; eff++)
		for(int frame=0; frame<6; frame++)
			m_EffectTex[eff][frame] = rhs.m_EffectTex[eff][frame];
	m_HColor = rhs.m_HColor;
	m_HCosInnerCone = rhs.m_HCosInnerCone;
	m_HCosOuterCone = rhs.m_HCosOuterCone;
	m_HDir = rhs.m_HDir;
	m_HkL = rhs.m_HkL;
	m_HkQ = rhs.m_HkQ;
	m_HPos = rhs.m_HPos;
	m_HullEffectAppl = rhs.m_HullEffectAppl;
	m_LensBeaconAngle = rhs.m_LensBeaconAngle;
	m_LightOmniTex = rhs.m_LightOmniTex;
	m_LightSpotTex = rhs.m_LightSpotTex;
	m_Siren = rhs.m_Siren;
	m_StrobesLit = rhs.m_StrobesLit;
	m_VAOsCreated = false;

	// Current ambient light and fog states.
	m_AmbientR = 0;
	m_AmbientG = 0;
	m_AmbientB = 0;
	m_FogDensity = 0;
	m_FogR = 0;
	m_FogG = 0;
	m_FogB = 0;
	m_InsideVisible = false;
	m_IsAboard = false;
	m_MyExtLight = 0;
	m_SunsBlocked = false;
}

void DaSc::operator>>(QDataStream& out) const
{
	DaVessel::operator>>(out);
	out << (quint8)m_AlertCondition;
	AuTime dbLoadTime;
	out << dbLoadTime;
	out << m_DesignCacheValid;
	out << m_NavLights;
	out << m_New;
	out << m_RotatingBeacons;
	out << m_Strobes;
	out << m_Ticked;
	out << m_Tocked;

	m_DesignLock.lockForRead();
	out << AuTime();	//m_DesignChg;
	out << m_Design;
	out << AuTime();	//m_DoorChg;
	out << m_DoorOpenTimes;
	out << m_DoorStates;
	out << AuTime();	//m_LandingGearChg;
	out << m_LandingGearStates;
	out << m_LiftCalls;
	out << AuTime();	//m_LiftChg;
	out << m_Lifts;
	out << m_TargetLoc;
	out << m_TargetLocValid;
	m_DesignLock.unlock();

	m_AirLock.lockForRead();
	out << AuTime();	//m_AirChg;
	out << m_AirPressures;
	m_AirLock.unlock();

	m_FTLLock.lockForRead();
	out << AuTime();	//m_FTLChg;
	out << m_FTLDrives;
	m_FTLLock.unlock();

	m_LandingSystemLock.lockForRead();
	out << AuTime();	//m_LandingSystemChg;
	out << m_LandingSystemStates;
	m_LandingSystemLock.unlock();

	m_LauncherLock.lockForRead();
	out << AuTime();	//m_LauncherChg;
	out << m_LauncherStates;
	m_LauncherLock.unlock();

	m_RoomLightLock.lockForRead();
	out << AuTime();	//m_RoomLightChg;
	out << m_RoomLightStates;
	m_RoomLightLock.unlock();

	m_TRoomLock.lockForRead();
	out << AuTime();	//m_TRoomChg;
	out << m_TRoomStates;
	m_TRoomLock.unlock();

	m_TurretPosLock.lockForRead();
	out << AuTime();	//m_TurretPosChg;
	out << m_TurretPos;
	m_TurretPosLock.unlock();
}

void DaSc::operator<<(QDataStream& in)
{
	DaVessel::operator<<(in);
	quint8 ui8;
	in >> ui8;
	m_AlertCondition = (ALERT_CONDITION)ui8;
	AuTime dbLoadTime;
	in >> dbLoadTime;
	in >> m_DesignCacheValid;
	in >> m_NavLights;
	in >> m_New;
	in >> m_RotatingBeacons;
	in >> m_Strobes;
	in >> m_Ticked;
	in >> m_Tocked;
	m_Ticked = false;
	m_Tocked = false;

	m_DesignLock.lockForWrite();
	AuTime _DesignChg;
	in >> _DesignChg;
	in >> m_Design;
	AuTime _DoorChg;
	in >> _DoorChg;
	in >> m_DoorOpenTimes;
	in >> m_DoorStates;
	AuTime _LandingGearChg;
	in >> _LandingGearChg;
	in >> m_LandingGearStates;
	in >> m_LiftCalls;
	AuTime _LiftChg;
	in >> _LiftChg;
	in >> m_Lifts;
	in >> m_TargetLoc;
	in >> m_TargetLocValid;
	m_DesignLock.unlock();

	m_AirLock.lockForWrite();
	AuTime _AirChg;
	in >> _AirChg;
	in >> m_AirPressures;
	m_AirLock.unlock();

	m_FTLLock.lockForWrite();
	AuTime _FTLChg;
	in >> _FTLChg;
	in >> m_FTLDrives;
	m_FTLLock.unlock();

	m_LandingSystemLock.lockForWrite();
	AuTime _LandingSystemChg;
	in >> _LandingSystemChg;
	in >> m_LandingSystemStates;
	m_LandingSystemLock.unlock();

	m_LauncherLock.lockForWrite();
	AuTime _LauncherChg;
	in >> _LauncherChg;
	in >> m_LauncherStates;
	m_LauncherLock.unlock();

	m_RoomLightLock.lockForWrite();
	AuTime _RoomLightChg;
	in >> _RoomLightChg;
	in >> m_RoomLightStates;
	m_RoomLightLock.unlock();

	m_TRoomLock.lockForWrite();
	AuTime _TRoomChg;
	in >> _TRoomChg;
	in >> m_TRoomStates;
	m_TRoomLock.unlock();

	m_TurretPosLock.lockForWrite();
	AuTime _TurretPosChg;
	in >> _TurretPosChg;
	in >> m_TurretPos;
	m_TurretPosLock.unlock();

	// Ensure the local application has my design loaded. Nothing happens if it is already loaded.
	DesignGeometryNeeded();
}

void DaSc::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaVessel::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint16 hasWhat = 0;
	if(m_NavLights)
		hasWhat |= 0x0001;
	if(m_RotatingBeacons)
		hasWhat |= 0x0002;
	if(m_Strobes)
		hasWhat |= 0x0004;
	if(m_New)
		hasWhat |= 0x4000;

	m_DesignLock.lockForRead();
	if(lastTx.IsNull() || m_DesignChg >= lastTx)
		hasWhat |= 0x0008;
	if(lastTx.IsNull() || m_DoorChg >= lastTx)
		hasWhat |= 0x0020;
	if(lastTx.IsNull() || m_LandingGearChg >= lastTx)
		hasWhat |= 0x0080;
	if(lastTx.IsNull() || m_LiftChg >= lastTx)
		hasWhat |= 0x0400;
	m_DesignLock.unlock();

	m_AirLock.lockForRead();
	if(lastTx.IsNull() || m_AirChg >= lastTx)
		hasWhat |= 0x0010;
	m_AirLock.unlock();

	m_FTLLock.lockForRead();
	if(lastTx.IsNull() || m_FTLChg >= lastTx)
		hasWhat |= 0x0040;
	m_FTLLock.unlock();

	m_LandingSystemLock.lockForRead();
	if(lastTx.IsNull() || m_LandingSystemChg >= lastTx)
		hasWhat |= 0x0100;
	m_LandingSystemLock.unlock();

	m_LauncherLock.lockForRead();
	if(lastTx.IsNull() || m_LauncherChg >= lastTx)
		hasWhat |= 0x0200;
	m_LauncherLock.unlock();

	m_RoomLightLock.lockForRead();
	if(lastTx.IsNull() || m_RoomLightChg >= lastTx)
		hasWhat |= 0x0800;
	m_RoomLightLock.unlock();

	m_TRoomLock.lockForRead();
	if(lastTx.IsNull() || m_TRoomChg >= lastTx)
		hasWhat |= 0x1000;
	m_TRoomLock.unlock();

	m_TurretPosLock.lockForRead();
	if(lastTx.IsNull() || m_TurretPosChg >= lastTx)
		hasWhat |= 0x2000;
	m_TurretPosLock.unlock();

	if(m_AlertCondition != ConditionGreen)
		hasWhat |= 0x8000;

	toClient << hasWhat;

	if(hasWhat & 0x0008)
	{
		m_DesignLock.lockForRead();
		toClient << m_Design;
		m_DesignLock.unlock();
	}

	if(hasWhat & 0x0010)
	{
		m_AirLock.lockForRead();
		toClient << m_AirPressures;
		m_AirLock.unlock();
	}

	if(hasWhat & 0x0020)
	{
		m_DesignLock.lockForRead();
		toClient << m_DoorStates;
		m_DesignLock.unlock();
	}

	if(hasWhat & 0x0040)
	{
		m_FTLLock.lockForRead();
		toClient << m_FTLDrives;
		m_FTLLock.unlock();
	}

	if(hasWhat & 0x0080)
	{
		m_DesignLock.lockForRead();
		toClient << m_LandingGearStates;
		m_DesignLock.unlock();
	}

	if(hasWhat & 0x0100)
	{
		m_LandingSystemLock.lockForRead();
		toClient << m_LandingSystemStates;
		m_LandingSystemLock.unlock();
	}

	if(hasWhat & 0x0200)
	{
		m_LauncherLock.lockForRead();
		toClient << m_LauncherStates;
		m_LauncherLock.unlock();
	}

	if(hasWhat & 0x0400)
	{
		m_DesignLock.lockForRead();
		toClient << m_Lifts;
		m_DesignLock.unlock();
	}

	if(hasWhat & 0x0800)
	{
		m_RoomLightLock.lockForRead();
		toClient << m_RoomLightStates;
		m_RoomLightLock.unlock();
	}

	if(hasWhat & 0x1000)
	{
		m_TRoomLock.lockForRead();
		toClient << m_TRoomStates;
		m_TRoomLock.unlock();
	}

	if(hasWhat & 0x2000)
	{
		m_TurretPosLock.lockForRead();
		toClient << m_TurretPos;
		m_TurretPosLock.unlock();
	}

	if(hasWhat & 0x8000)
		toClient << (quint8)m_AlertCondition;
}

void DaSc::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaVessel::RxUpdate(fromServer, isFirstUpdate);

	quint16 hasWhat;
	fromServer >> hasWhat;

	m_NavLights = hasWhat & 0x0001;
	m_RotatingBeacons = hasWhat & 0x0002;
	m_Strobes = hasWhat & 0x0004;
	m_New = hasWhat & 0x4000;

	if(hasWhat & 0x0008)
		fromServer >> m_Design;

	if(hasWhat & 0x0010)
		fromServer >> m_AirPressures;

	if(hasWhat & 0x0020)
	{
		m_DesignLock.lockForWrite();
		fromServer >> m_DoorStates;
		m_DesignLock.unlock();
	}

	if(hasWhat & 0x0040)
		fromServer >> m_FTLDrives;

	if(hasWhat & 0x0080)
	{
		m_DesignLock.lockForWrite();
		fromServer >> m_LandingGearStates;
		m_DesignLock.unlock();
	}

	if(hasWhat & 0x0100)
		fromServer >> m_LandingSystemStates;

	if(hasWhat & 0x0200)
		fromServer >> m_LauncherStates;

	if(hasWhat & 0x0400)
	{
		m_DesignLock.lockForWrite();
		fromServer >> m_Lifts;
		m_DesignLock.unlock();
	}

	if(hasWhat & 0x0800)
		fromServer >> m_RoomLightStates;

	if(hasWhat & 0x1000)
		fromServer >> m_TRoomStates;

	if(hasWhat & 0x2000)
		fromServer >> m_TurretPos;

	if(hasWhat & 0x8000)
	{
		quint8 ui8;
		fromServer >> ui8;
		m_AlertCondition = (ALERT_CONDITION)ui8;
	}
	else
		m_AlertCondition = ConditionGreen;
}

bool DaSc::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	QString oldName = AssignedName();
	bool transformChanged = DaVessel::Refresh(pAtom, ari);

	DaSc* pA = (DaSc*)pAtom;

	// Trip siren noise when view receives a change up in alert condition.
	if(ari && pA->m_AlertCondition != m_AlertCondition)
		m_Siren = pA->m_AlertCondition+1;	// 1=green, 2=yellow, 3=red

	m_AlertCondition = pA->m_AlertCondition;
	m_NavLights = pA->m_NavLights;
	m_New = pA->m_New;
	m_RotatingBeacons = pA->m_RotatingBeacons;
	m_Strobes = pA->m_Strobes;

	pA->m_DesignLock.lockForRead();
	m_DesignLock.lockForWrite();
	m_Design = pA->m_Design;
	m_LandingGearStates = pA->m_LandingGearStates;
	m_Lifts = pA->m_Lifts;

	// Door states.
	if(ari)
	{
		// Trigger sounds and animations as we update door states in the scene.
		AuDoorStateHash::const_iterator si = pA->m_DoorStates.cbegin();
		AuDoorStateHash::iterator ri = m_DoorStates.begin();
		while(si != pA->m_DoorStates.cend() && ri != m_DoorStates.end())
		{
			if((*si).State() != (*ri).State())
			{
				bool isOpen = (*si).IsOpen();
				bool wasOpen = (*ri).IsOpen();
				if(isOpen != wasOpen)
				{
					// Door opened or closed. Make a sound based on its new state.
					if((*ri).IsHullDoor())
					{
						m_DoorExtLoc = ((*ri).OpLoc1() + (*ri).OpLoc2())/2;
						if(isOpen)
							m_DoorExt = 1;	// Trigger door open sound.
						else
							m_DoorExt = 2;	// Trigger door close sound.
					}
					else if((*ri).IsTurboLiftDoor())
					{
						if(m_DoorLift1 == 0)
						{
							m_DoorLift1Loc = ((*ri).OpLoc1() + (*ri).OpLoc2())/2;
							if(isOpen)
								m_DoorLift1 = 1;	// Trigger door open sound.
							else
								m_DoorLift1 = 2;	// Trigger door close sound.
						}
						else
						{
							m_DoorLift2Loc = ((*ri).OpLoc1() + (*ri).OpLoc2())/2;
							if(isOpen)
								m_DoorLift2 = 1;	// Trigger door open sound.
							else
								m_DoorLift2 = 2;	// Trigger door close sound.
						}
					}
					else
					{
						if(m_DoorInt1 == 0)
						{
							m_DoorInt1Loc = ((*ri).OpLoc1() + (*ri).OpLoc2())/2;
							if(isOpen)
								m_DoorInt1 = 1;	// Trigger door open sound.
							else
								m_DoorInt1 = 2;	// Trigger door close sound.
						}
						else
						{
							m_DoorInt2Loc = ((*ri).OpLoc1() + (*ri).OpLoc2())/2;
							if(isOpen)
								m_DoorInt2 = 1;	// Trigger door open sound.
							else
								m_DoorInt2 = 2;	// Trigger door close sound.
						}
					}
				}
				*ri = *si;
			}
			si++;
			ri++;
		}
	}
	else
		m_DoorStates = pA->m_DoorStates;
	m_DesignLock.unlock();
	pA->m_DesignLock.unlock();

	pA->m_AirLock.lockForRead();
	m_AirLock.lockForWrite();
	bool airPressureChg = m_AirPressures != pA->m_AirPressures;
	m_AirPressures = pA->m_AirPressures;
	m_AirLock.unlock();
	pA->m_AirLock.unlock();

	pA->m_FTLLock.lockForRead();
	m_FTLLock.lockForWrite();
	m_FTLDrives = pA->m_FTLDrives;
	m_FTLLock.unlock();
	pA->m_FTLLock.unlock();

	pA->m_LandingSystemLock.lockForRead();
	m_LandingSystemLock.lockForWrite();
	m_LandingSystemStates = pA->m_LandingSystemStates;
	m_LandingSystemLock.unlock();
	pA->m_LandingSystemLock.unlock();

	pA->m_LauncherLock.lockForRead();
	m_LauncherLock.lockForWrite();
	m_LauncherStates = pA->m_LauncherStates;
	m_LauncherLock.unlock();
	pA->m_LauncherLock.unlock();

	pA->m_RoomLightLock.lockForRead();
	m_RoomLightLock.lockForWrite();
	m_RoomLightStates = pA->m_RoomLightStates;
	m_RoomLightLock.unlock();
	pA->m_RoomLightLock.unlock();

	pA->m_TRoomLock.lockForRead();
	m_TRoomLock.lockForWrite();
	m_TRoomStates = pA->m_TRoomStates;
	m_TRoomLock.unlock();
	pA->m_TRoomLock.unlock();

	pA->m_TurretPosLock.lockForRead();
	m_TurretPosLock.lockForWrite();
	m_TurretPos = pA->m_TurretPos;
	m_TurretPosLock.unlock();
	pA->m_TurretPosLock.unlock();

	if(airPressureChg)
		InvalidateEnvironmentRecursive();
	return transformChanged;
}

void DaSc::ActivateTransporterSystem(int troomIndex)
{
	// Insure the room is On when it is activated. This happens when a beam-up is requested.
	if(!IsTransporterSystemOn(troomIndex))
		TurnTransporterSystemOn(troomIndex);

	m_TRoomLock.lockForWrite();
	AuTransporterSystemStateHash::iterator ti = m_TRoomStates.find(troomIndex);
	if(ti != m_TRoomStates.end())
	{
		*ti = TRANSPORTER_Active;
		m_TRoomChg.Set();
		SetModified();
	}
	m_TRoomLock.unlock();
}

bool DaSc::ActiveTransporterPadLocation(quint16 which, AuPoint& padLoc) const
{
	int troomIndex = which;
	if(!IsTransporterSystemActive(troomIndex))
		return false;

	// Get the list of possible teleporter locations for the transporter system.
	AuPointArray padLocs;
	TransporterPadLocations(which, padLocs);
	if(padLocs.empty())
		return false;

	// Locate all transporter pads in the room.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(AuPointArray::const_iterator pi = padLocs.cbegin(); pi != padLocs.cend(); pi++)
	{
		// Check to see if the pad has an active teleporter there.
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			const DaTeleporter* teleporter = dynamic_cast<const DaTeleporter*>(*it);
			if(!teleporter)
				continue;
			if(!teleporter->IsOrigin())
				continue;	// Only care about origin transporters.

			// Found a live teleporter aboard this ship. Is it at the pad location we care about.
			AuPoint peloc = teleporter->Transform().UnProject(*pi);		// Project pad loc into teleporter's coordinate system.
			peloc.z += 4;							// Test a location above the pad on the floor.

			// Measure distance to each of the teleporter's portal locations. They are the centers of the teleporters.
			const AuPointArray& tlocs = teleporter->LockTeleporterLocs();
			for(AuPointArray::const_iterator ti = tlocs.cbegin(); ti != tlocs.cend(); ti++)
			{
				if(peloc.z > -4 && peloc.z < 4 && peloc.Distance2_2D(*ti) < 1)
				{
					teleporter->UnlockTeleporterLocs();
					padLoc = *pi;					// Relative to the ship.
					return true;
				}
			}
			teleporter->UnlockTeleporterLocs();
		}
	}
	return false;
}

float DaSc::AirPressure(const QString& roomName) const
{
	float p = 0;
	m_AirLock.lockForRead();
	if(p != 0)
	{
		qDebug() << "DaSc::AirPressure" << *this << "roomName" << roomName;
		for(AuAirPressureHash::const_iterator ai = m_AirPressures.cbegin(); ai != m_AirPressures.cend(); ai++)
			qDebug() << "  room" << ai.key() << "air" << *ai;
	}
	AuAirPressureHash::const_iterator ai = m_AirPressures.find(roomName);
	if(ai != m_AirPressures.cend())
	{
		if(*ai > 200)
			p = 1;
		else
			p = (float)(*ai) / 200.f;
	}
	m_AirLock.unlock();
	return p;
}

void DaSc::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	//qDebug() << "DaSc::AnimateDeviceObjects" << Id() << Name();
	DaVessel::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

	AnimateRenderStates();

	if(!m_OwnTextures.empty())
	{
		if(lastAnim.ElapsedSecTicks() > 0)
			m_ConCaptAI = 0;	// Invalidate AI capt id for proper name decal if things change.

		// We are ready to animate the custom textures.
		AnimateAlertStatus(sceneAtom, lastAnim);
		AnimateBerthNames(*ari.re);
		AnimateCaptName(*ari.re);
		AnimateMissionStatus(sceneAtom, *ari.re, lastAnim);
		AnimateShipName(*ari.re);
	}
	AnimateEffectTextures();

	if(lastAnim.SecIsOdd())
	{
		if(lastAnim.ElapsedSecTicks() > 0)
		{
			// Get the contrail color of the empire once per second, in case it changes or my empire changes.
			const DgData& gd = g_Au.LockDataForRead();
			m_ContrailColor = gd.EmpireContrailColor(EmpireId());
			g_Au.UnlockData();
		}
	}

	quint32 offset = Id().GetID();
	if(m_Strobes && !m_LensStrobeGlows.empty() && lastAnim.IsNewPeriod(d_MSecPerSec, offset))
		m_StrobesLit = 5;	// Light strobe lenses for a brief number of frames.
	else if(m_StrobesLit)
		m_StrobesLit--;		// Decay lit state of strobes.

	if(!m_LensBeaconGlows.empty())
		m_LensBeaconAngle = lastAnim.SecCycle(offset) * (PI*2);

	if(IsSpaceStation() && !m_Contrails.empty())
	{
		// Station contrails point "down". Artificial velocity vector needed for this appearance when stationary.
		// When moving, the additional vector doesn't hurt anything.
		for(QList<PsEngineTrail*>::iterator ci = m_Contrails.begin(); ci != m_Contrails.end(); ci++)
			(*ci)->SetVelocity(Transform().ProjectVector(AuPoint(0,0,-25)));
	}

	// Update lit states of dummy facade windows.
	if(!m_WindowLightStates.empty() && CapacitorLevel())
	{
		bool changeLights = false;
		if(InDesignEnvironment())
		{
			// Animate often in design environment so they can be observed.
			if(lastAnim.ElapsedSecTicks())
				changeLights = true;
		}
		else
		{
			// Animate slowly in game environment.
			if(lastAnim.IsNewPeriod(d_MSecPerMin, offset))
				changeLights = true;
		}

		if(changeLights)
		{
			// Animate facade window lighting.
			// Animate 20% of the windows each time.
			int anim = g_Au.Dice().Roll(1, (m_WindowLightStates.size()+1)/2 + 2, -2);	// Number to animate this cycle.
			if(anim < 0)
				anim = 0;
			while(anim--)
			{
				int idx = g_Au.Dice().Roll(1, m_WindowLightStates.size(), -1);	// Index of light to change state.
				int lidx = 0;	// Index of current light.
				for(AuRoomLightStateHash::iterator wi = m_WindowLightStates.begin(); wi != m_WindowLightStates.end(); wi++, lidx++)
				{
					if(lidx < idx)
						continue;	// Skip until we reach the desired index.
					// Determine lighting state of the window.
					INTERIOR_LIGHT light;
					quint32 roll = g_Au.Dice().Roll(1,100);
					if(roll <= 60)
						light = INTLIGHT_Bright;			// Bright 60% of the time.
					else if(roll <= 75)
						light = INTLIGHT_Dim;			// Dim 15% of the time.
					else
						light = INTLIGHT_Dark;			// Dark 25% of the time.
					*wi = light;
					break;	// We only do one index per iteration of the anim loop.
				}
			}
		}
	}
}

bool DaSc::AnyDoorJammed(AuPoint& where) const
{
	m_DesignLock.lockForRead();
	bool v = m_DoorStates.AnyJammed(where);
	m_DesignLock.unlock();
	return v;
}

bool DaSc::AnyHullDoorOpen() const
{
	m_DesignLock.lockForRead();
	bool v = m_DoorStates.AnyHullDoorOpen();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::AnyHullDoorOpenInRoom(const QString& roomName) const
{
	m_DesignLock.lockForRead();
	for(QHash<quint16, QString>::const_iterator dri = m_DoorRooms.cbegin(); dri != m_DoorRooms.cend(); dri++)
	{
		if(*dri != roomName)
			continue;	// Door is not in the same room.
		AuDoorStateHash::const_iterator dsi = m_DoorStates.find(dri.key());
		if(dsi == m_DoorStates.cend())
			continue;
		if(!(*dsi).IsOpen())
			continue;
		if(!(*dsi).IsHullDoor())
			continue;
		m_DesignLock.unlock();
		return true;
	}
	m_DesignLock.unlock();
	return false;
}

bool DaSc::AnyHullDoorUnlocked() const
{
	m_DesignLock.lockForRead();
	bool v = m_DoorStates.AnyHullDoorUnlocked();
	m_DesignLock.unlock();
	return v;
}

float DaSc::ArmorLevel() const
{
	m_DesignLock.lockForRead();
	float a = m_Design.ArmorLevel();
	m_DesignLock.unlock();
	return a;
}

float DaSc::BayWeaponPower(float unitsFired, quint16 which) const
{
	m_DesignLock.lockForRead();
	float p = m_Design.WeaponBayPower(which, unitsFired);
	m_DesignLock.unlock();
	return p;
}

double DaSc::BayWeaponRange(quint16 which) const
{
	m_DesignLock.lockForRead();
	float r = m_Design.WeaponBayRange(which);
	m_DesignLock.unlock();
	return r;
}

float DaSc::BayWeaponUnits(quint16 which) const
{
	m_DesignLock.lockForRead();
	float u = m_Design.WeaponBayUnits(which);
	m_DesignLock.unlock();
	return u;
}

const AuScDesign* DaSc::LockDesign() const
{
	// Get my parent design environment, if any. I could be a preview of a design in progress.
	DaDesigner* pardes = (DaDesigner*)FindParentDesignerInEnv();	// Cast away constness.
	if(pardes)
		return dynamic_cast<const AuScDesign*>(pardes->LockDesignForRead());	// I am a preview in a design studio.

	// I am not a preview in a design studio.
	const DgData& sdd = g_Au.LockScDesignsForRead();
	AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find(m_Design.DesignId());
	if(di == sdd.m_ScDesigns->cend())
		return 0;
	return &(*di).second;
}

void DaSc::UnlockDesign() const
{
	// Get my parent design environment, if any. I could be a preview of a design in progress.
	DaDesigner* pardes = (DaDesigner*)FindParentDesignerInEnv();	// Cast away constness.
	if(pardes)
	{
		pardes->UnlockDesign();								// I am a preview in a design studio.
		return;
	}

	// I am not a preview in a design studio.
	g_Au.UnlockScDesigns();
}

AuPoint DaSc::BerthLocation(const AuId& id) const
{
	m_RosterLock.lockForRead();
	bool v = m_Roster.BerthTypeOccupied(id) == AuRosterEntry::CaptainSlot;
	quint32 detailId = m_Roster.BerthDetailOccupied(id);
	m_RosterLock.unlock();
	if(v || !detailId)
	{
		// The person id occupies the captain slot, which means there are no berths aboard the ship.
		// Look for a random void location inside the hull suitable for a crew member.
		AuPoint voidLoc;
		if(IsDesignDataCacheValid() && TroopOrCrewVoidLocation(voidLoc))
			return voidLoc;		// Found a void location.

		// Last resort is to respawn near the helm.
		return HelmLocation() + AuPoint(1,-1,0);
	}

	AuPoint berthLoc;
	const AuScDesign* scd = LockDesign();
	if(scd)
		berthLoc = scd->BerthLocation((quint16)detailId);
	else
		berthLoc = HelmLocation() + AuPoint(1,-1,0);
	UnlockDesign();
	return berthLoc;
}

AuPoint DaSc::BerthLocation(const AuId& id, quint16& roomNumber) const
{
	roomNumber = 0;

	m_RosterLock.lockForRead();
	quint32 detailId = m_Roster.BerthDetailOccupied(id);
	m_RosterLock.unlock();
	if(!detailId)
		return AuPoint(0,0);

	// The person id is assigned to a berth detail in the design.
	AuPoint berthLoc;
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		berthLoc = scd->BerthLocation((quint16)detailId);
		roomNumber = scd->RoomVoidContainingLocLive(berthLoc, .1, m_DoorStates, m_LandingGearStates, m_Lifts);
	}
	UnlockDesign();
	return berthLoc;
}

bool DaSc::CanBeSelected() const
{
	return DaVessel::CanBeSelected();
}

bool DaSc::CanEnterConsole(const DaPerson& who, quint16& stationId, AuStation& station, quint16& which) const
{
	// Don't let people sit at consoles of noncombatant empires. If they are in my empire, then I am not a noncombatant.
	if(IsNoncombatant())
		return false;
	stationId = 0;
	which = 0;
	if(!IsDesignDataCacheValid())
		return false;	// Failed to load ship design.

	if(IsNew())
	{
		// Neutrals are not permitted to enter a console or turret on a new ship.
		const DgData& gd = g_Au.LockDataForRead();
		POLITICAL_STANCE stance = gd.EmpirePoliticalStance(EmpireId(), who.EmpireId());
		g_Au.UnlockData();
		if(stance == POL_Neutral)
			return false;
	}

	// Get the person's berth, if they are tied to this ship.
	quint32 berthId;
	if(Is(who.SpawnShip()))
		berthId = BerthDetailIdOccupied(who.Id());
	else
		berthId = 0;

	// Must check CoC now, before design is locked.
	bool whoInCoC = InChainOfCommand(&who);

	// Look for consoles nearby.
	QString roomN = who.ShipRoomName();
	AuPoint gN = who.GravityNormal();
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		// Captain's chairs.
		for(AuScReportStationHash::const_iterator si = m_Design.CommandStations().cbegin();
				si != m_Design.CommandStations().cend(); si++)
		{
			// Locate the corresponding part in the design.
			AuScParts::const_iterator pi = scd->Parts().find(si.key());
			if(pi == scd->Parts().cend())
				continue;

			// Locate the room that contains the station.
			AuScParts::const_iterator ri = scd->Parts().find((*si).RoomId());
			if(ri == scd->Parts().cend())
				continue;

			// Person must be in the same room as the console.
			if(roomN != (*ri)->GroupName())
				continue;

			stationId = pi.key();
			which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
			if(which)
				break;
		}

		if(!which)
		{
			// Crew consoles, except transporters.
			for(AuScReportStationHash::const_iterator si = m_Design.CrewStations().cbegin();
					si != m_Design.CrewStations().cend(); si++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				// Locate the room that contains the station.
				AuScParts::const_iterator ri = scd->Parts().find((*si).RoomId());
				if(ri == scd->Parts().cend())
					continue;

				// Person must be in the same room as the console.
				if(roomN != (*ri)->GroupName())
					continue;

				stationId = pi.key();
				which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
				if(which)
					break;
			}
		}

		if(!which)
		{
			// Transporter consoles.
			for(AuScReportTransporterStationHash::const_iterator si = m_Design.Transporters().cbegin();
					si != m_Design.Transporters().cend(); si++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				// Locate the room that contains the station.
				AuScParts::const_iterator ri = scd->Parts().find((*si).RoomId());
				if(ri == scd->Parts().cend())
					continue;

				// Person must be in the same room as the console.
				if(roomN != (*ri)->GroupName())
					continue;

				stationId = pi.key();
				which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
				if(which)
					break;
			}
		}

		if(!which)
		{
			// Turret consoles.
			for(AuScReportTurretHash::const_iterator si = m_Design.Turrets().cbegin(); si != m_Design.Turrets().cend(); si++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				// Locate the room that contains the station.
				AuScParts::const_iterator ri = scd->Parts().find((*si).RoomId());
				if(ri == scd->Parts().cend())
					continue;

				// Person must be in the same room as the console.
				if(roomN != (*ri)->GroupName())
					continue;

				stationId = pi.key();
				which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
				if(which)
					break;
			}
		}

		if(!which && berthId)
		{
			// Berths.
			// Locate the corresponding part in the design.
			AuScParts::const_iterator pi = scd->Parts().find(berthId);
			if(pi != scd->Parts().cend())
			{
				// Locate the room that contains the berth.
				quint16 roomId = scd->RoomVoidContainingLoc((*pi)->BerthLocation());
				AuScParts::const_iterator ri = scd->Parts().find(roomId);
				if(ri != scd->Parts().cend())
				{
					// Person must be in the same room as the berth.
					if(roomN == (*ri)->GroupName())
					{
						stationId = pi.key();
						which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
					}
				}
			}
		}

		if(!which)
		{
			// Game Tables
			for(AuScReportGameTableHash::const_iterator si = m_Design.GameTables().cbegin(); si != m_Design.GameTables().cend();
					si++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				// Locate the room that contains the game table.
				AuScParts::const_iterator ri = scd->Parts().find((*si).RoomId());
				if(ri == scd->Parts().cend())
					continue;

				// Person must be in the same room as the console.
				if(roomN != (*ri)->GroupName())
					continue;

				stationId = pi.key();
				which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
				if(which)
					break;
			}
		}

		if(!which)
		{
			// Designer Stations
			for(AuScReportStationHash::const_iterator si = m_Design.DesignerStations().cbegin();
					si != m_Design.DesignerStations().cend(); si++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				// Locate the room that contains the station.
				AuScParts::const_iterator ri = scd->Parts().find((*si).RoomId());
				if(ri == scd->Parts().cend())
					continue;

				// Person must be in the same room as the console.
				if(roomN != (*ri)->GroupName())
					continue;

				stationId = pi.key();
				which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
				if(which)
					break;
			}
		}

		if(!which)
		{
			// Guard Posts
			for(QSet<quint16>::const_iterator si = m_Design.GuardPosts().cbegin(); si != m_Design.GuardPosts().cend(); si++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(*si);
				if(pi == scd->Parts().cend())
					continue;

				stationId = pi.key();
				which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
				if(which)
					break;
			}
		}

		if(!which)
		{
			// Citizen Posts
			for(QSet<quint16>::const_iterator si = m_Design.CitizenPosts().cbegin(); si != m_Design.CitizenPosts().cend(); si++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(*si);
				if(pi == scd->Parts().cend())
					continue;

				stationId = pi.key();
				which = AttemptConsoleEntry(who, whoInCoC, pi.key(), *(*pi), station, gN);
				if(which)
					break;
			}
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();

	// Non-zero which, which is the part id of the console, means we successfully entered a console.
	return which;
}

quint16 DaSc::AttemptConsoleEntry(const DaPerson& who, bool whoInCoC, quint16 consolePartId, const AuScPart& console,
		AuStation& station, const AuPoint& gravityN) const
{
	// Console's operator position matrix relative to the ship.
	station = console.ClosestStation(who.Location(), gravityN);
	if(station.Type() == AuStation::TYPE_Count)
		return 0;
	AuPoint whoLoc = station.OpM().UnProject(who.Location());	// 'who's location relative to the console.
	double floordist = whoLoc.Length2_2D();
	if(floordist > 3*3 || whoLoc.z < -3 || whoLoc.z > 6)
		return 0;

	// Person must not be wearing armor or clothing to enter surgery unit, unless they are dying.
	// Players have no way to undress dying players to prep them for surgery.
	if(station.IsPatient() &&
			who.CurrentHitPoints() > 0 &&
			(who.Armor() != CID_Invalid || who.Helmet() != CID_Invalid))
	{
		// Conscious player must remove armor and clothing.
		if(g_Au.IsSceneClient())
		{
			QString text = QObject::tr("I cannot get into the surgery unit while wearing armor or clothing.");
			who.Thought(text);
		}
		return 0;
	}

	// Insure there isn't a person or citizen already sitting at the console.
	// We can't call InChainOfCommand here because the design is already locked.
	if(!whoInCoC && !IsCrewOrTroop(who.Id()))
	{
		// The person is not crew and they are not in the ship's chain of command.
		// This test does not ignore AI controlled citizens, so they will not give up their console to enemies.
		if(IsConsoleSeatOccupied(consolePartId, station.SeatIndex()))
			return 0;	// the console is occupied by a person or a citizen
	}
	else
	{
		// The person is not an enemy.
		// This test ignores AI controlled citizens, who will be kicked from the console when I enter.
		if(IsConsoleSeatManned(consolePartId, station.SeatIndex()))
			return 0;	// the console is occupied by a person
	}

	// Who is near an unoccupied console.
	// Adjust for pose.
	// For example, a standing station is positioned on the floor; we must adjust so our feet are there, instead of our cg.
	station.OffsetOpM(who.ConsolePoseOffset(station.Type(), station.OccPos(), who.CGHeightStanding(), who.TorsoThickness(),
				station.SeatIndex()));
	return consolePartId;
}

bool DaSc::CanEnterNegativeWormhole() const
{
	quint16 wormFTL = FTLWormholeDrive();
	if(!wormFTL)
		return false;

	m_DesignLock.lockForRead();
	quint8 maxFTL = m_Design.MaxCapacitorFTLSetting(wormFTL);
	m_DesignLock.unlock();
	return maxFTL >= 1;
}

bool DaSc::CanEnterNeutralWormhole() const
{
	quint16 wormFTL = FTLWormholeDrive();
	if(!wormFTL)
		return false;

	m_DesignLock.lockForRead();
	quint8 maxFTL = m_Design.MaxCapacitorFTLSetting(wormFTL);
	m_DesignLock.unlock();
	return maxFTL >= 2;
}

bool DaSc::CanGrabLadder(const DaAtom& who, const AuPoint& loc, AuMatrix4x4& mLadder)
{
	Q_UNUSED(who);
	bool foundLadder;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		foundLadder = scd->FindLadderToGrab(loc, mLadder, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		foundLadder = false;
	UnlockDesign();
	m_DesignLock.unlock();
	return foundLadder;
}

bool DaSc::CanSetInMotion() const
{
	return DaVessel::CanSetInMotion();
}

float DaSc::CapacitorMax() const
{
	m_DesignLock.lockForRead();
	float p = m_Design.CapacitorMax();
	m_DesignLock.unlock();
	return p;
}

unsigned int DaSc::CaptainsBerths() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.CaptBerths();
	m_DesignLock.unlock();
	return v;
}

unsigned int DaSc::CargoCapacityVolume() const
{
	m_DesignLock.lockForRead();
	unsigned int hold = m_Design.HoldCapacity();
	m_DesignLock.unlock();
	return hold;
}

bool DaSc::CheckChildExit(const AuPoint &oldloc, const AuPoint &nuloc, const DaAtom* who) const
{
	Q_UNUSED(oldloc);
	const DaCreature* c = dynamic_cast<const DaCreature*>(who);
	if(c)
	{
		// Don't release creatures who could grab a ladder.
		bool foundLadder;
		AuMatrix4x4 mLadder;
		m_DesignLock.lockForRead();
		const AuScDesign* scd = LockDesign();
		if(scd)
			foundLadder = scd->FindLadderToGrab(nuloc, mLadder, m_DoorStates, m_LandingGearStates, m_Lifts);
		else
			foundLadder = false;
		UnlockDesign();
		m_DesignLock.unlock();
		if(foundLadder)
			return false;
	}
	return !IsInOrOnHull(nuloc);
}

bool DaSc::CheckPeerJoin(const AuPoint &oldloc, const AuPoint &nuloc, const DaAtom* who)
{
	//qDebug() << "DaSc::CheckPeerJoin" << Id() << Name() << "designValid" << IsDesignDataCacheValid()
		//<< "who" << who->Id() << who->Name();
	if(!IsDesignDataCacheValid())
		return false;

	if(who)
	{
		const DaMissile* missile = dynamic_cast<const DaMissile*>(who);
		if(missile)
		{
			// Missile does not join the launching spacecraft.
			if(missile->IsArmedBy(Id()))
				return false;
		}
	}

	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(Parent());
	if(system)
	{
		// Parent atom is a solar system. Peer join is not permitted when this ship is within the zone of influence of the
		// primary star. It will happen after I am reparented to the primary star, if it is permitted.
		if(Location().Length2() < FeetPerSolarSystem()/2 * FeetPerSolarSystem()/2)
			return false;										// This ship is in the zone-of-control radius of the primary star.
	}

	// Get the reverse transformation back up to the parent.
	AuMatrix4x4 tsub = Transform();
	tsub = tsub.Inverse();

	AuPoint nloc = tsub.Project(nuloc);						// Get location relative to ship.
	if(!IsInOrOnHull(nloc))
	{
		//qDebug("nloc not in or on hull");
		return false;											// nu location is not IN, ON, or UNDER the hull
	}

	// Insure path from old location to nu location is not blocked.
	// They must move into my space through a valid opening.
	AuPoint oloc = tsub.Project(oldloc);						// Get location relative to ship.
	if(IsInOrOnHull(oloc))
	{
		//qDebug() << "DaSc::CheckPeerJoin" << *this << "oloc already in or on hull";
		return false;
	}
	AuPoint hitLoc, hitNormal;
	if(IsPathBlocked(oloc, nloc, hitLoc, hitNormal, who))
	{
		//qDebug("path blocked");
		return false;
	}
	return true;												// Path to nu location inside hull is unobstructed.
}

DaAtom* DaSc::Clone() const
{
	return new DaSc(*this);
}

void DaSc::CloseAllDoors()
{
	m_DesignLock.lockForWrite();
	if(m_DoorStates.CloseAllDoors())
	{
		m_DoorChg.Set();
		m_DoorOpenTimes.clear();
		SetModified();
	}
	m_DesignLock.unlock();
}

void DaSc::CloseAllHullDoors()
{
	m_DesignLock.lockForWrite();
	if(m_DoorStates.CloseHullDoors())
	{
		m_DoorChg.Set();
		SetModified();
	}
	m_DesignLock.unlock();
}

void DaSc::CloseDoor(unsigned int doorIndex)
{
	if(IsDoorOpen(doorIndex))
	{
		// Check for anything in the door void space that would block the door from closing.
		bool doorBlocked = false;
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			const DaAtom* atom = *it;
			if(!atom->KeepsAutomaticDoorsOpen())
				continue;
			quint16 roomId = atom->ShipRoomId();
			if(!roomId)
				continue;
			const AuScDesign* scd = LockDesign();
			if(scd && scd->IsOpenDoorVoid(roomId, doorIndex))
				doorBlocked = true;
			UnlockDesign();
			if(doorBlocked)
				break;
		}
		if(doorBlocked)
			return;
	}
	// Door is not blocked open so it's ok to close it.
	m_DesignLock.lockForWrite();
	if(m_DoorStates.CloseDoor(doorIndex))
	{
		m_DoorChg.Set();
		m_DoorOpenTimes.remove(doorIndex);
		SetModified();
	}
	// Remove door open times that have expired.
	QHash<quint16, AuTime>::iterator oi = m_DoorOpenTimes.begin();
	while(oi != m_DoorOpenTimes.end())
		if((*oi).ElapsedSec() <= 4)
			oi++;
		else
			oi = m_DoorOpenTimes.erase(oi);
	m_DesignLock.unlock();
}

void DaSc::ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestGravityDrive, AuPoint& closestPowerPlant,
		AuPoint& closestSensor, AuPoint& closestFTLDrive, AuPoint& closestSiren) const
{
	const AuScDesign* scd = LockDesign();
	if(scd)
		scd->ClosestAudibleUnits(searchLoc, closestGravityDrive, closestPowerPlant, closestSensor, closestFTLDrive, closestSiren);
	UnlockDesign();
}

int DaSc::ClosestClosedDoor(const DaAtom* toWho, double& doorDistance) const
{
	m_DesignLock.lockForRead();
	quint16 door = m_DoorStates.ClosestClosedDoor(toWho->Location(), doorDistance);
	m_DesignLock.unlock();
	if(!door)
		return -1;
	return door;
}

int DaSc::ClosestDoor(const AuPoint& toLoc, double& doorDistance) const
{
	m_DesignLock.lockForRead();
	quint16 door = m_DoorStates.ClosestDoor(toLoc, doorDistance);
	m_DesignLock.unlock();
	if(!door)
		return -1;
	return door;
}

int DaSc::ClosestOpenDoor(const DaAtom* toWho, double& doorDistance) const
{
	m_DesignLock.lockForRead();
	quint16 door = m_DoorStates.ClosestOpenDoor(toWho->Location(), doorDistance);
	m_DesignLock.unlock();
	if(!door)
		return -1;
	return door;
}

bool DaSc::ClosestUnoccupiedCommandStation(const AuPoint& toLoc, quint16& stationId, AuStation& station) const
{
	quint16 which = 0;
	stationId = 0;

	// Get a list of the locations of the consoles that are currently manned.
	QSet<quint16> consoles;
	ListVisiblyOccupiedConsoles(0, consoles);
	QSet<quint16> destCons;
	ListDestinationConsoles(destCons);							// Consoles that are destinations of AI dudes.

	// Locate the closest unmanned crew station that is operational.
	m_SystemsLock.lockForRead();
	m_DamageLock.lockForRead();
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		QMap<double, AuScParts::const_iterator> sortedConsoles;	// Eligible consoles keyed/sorted by distance.
		for(AuScReportStationHash::const_iterator si = m_Design.CommandStations().cbegin();
				si != m_Design.CommandStations().cend(); si++)
		{
			AuScParts::const_iterator pi = scd->Parts().find(si.key());
			if(pi == scd->Parts().cend())
				continue;

			if(consoles.contains(pi.key()))
				continue;	// Skip manned consoles.

			if(!m_Damage.HitsRemainingAddParts(AuSysId(SYS_CommandAndControl, si.key()), m_Design))
				continue;	// Skip broken consoles.

			AuPoint sloc = (*pi)->OperatorLoc();
			double dist = toLoc.Distance2(sloc);				// square of the distance is faster for simple distance comparisons
			if(dist > 10*10 && destCons.contains(pi.key()))
				continue;	// Skip distant crew station that someone else has already chosen as a destination.

			// Store the eligible console in the map, sorted by distance from me.
			sortedConsoles.insert(dist, pi);
		}

		// Consoles are now sorted by distance.
		if(!sortedConsoles.isEmpty())
		{
			AuScParts::const_iterator bestIt = *sortedConsoles.begin();
			AuScParts::const_iterator pi = bestIt;			// Take the closest unmanned station.

			AuPoint gravityN(0,0,1);
			AuScReportStationHash::const_iterator si = m_Design.CommandStations().find(pi.key());
			if(si != m_Design.CommandStations().cend())
				gravityN = scd->RoomVoidGravityUp((*si).RoomId(), m_Lifts);

			stationId = pi.key();
			station = (*pi)->OperatorStation(gravityN);
			which = pi.key();
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	m_DamageLock.unlock();
	m_SystemsLock.unlock();

	// If we got a which id, we found console.
	return which;
}

bool DaSc::ClosestUnoccupiedCrewStation(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
		AuSkill::SKILL skill) const
{
	stationId = 0;
	which = 0;

	// Get a list of the locations of the consoles that are currently manned.
	QSet<quint16> consoles;
	ListVisiblyOccupiedConsoles(0, consoles);
	QSet<quint16> destCons;
	ListDestinationConsoles(destCons);							// Consoles that are destinations of AI dudes.

	bool isCaptAI = IsCommandedByAI();							// Must acquire before ship design data is locked.

	// Locate the closest unmanned crew station that is operational.
	m_SystemsLock.lockForRead();
	m_DamageLock.lockForRead();
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator engineer = scd->Parts().cend();
		AuScParts::const_iterator firecontrol = scd->Parts().cend();
		AuScParts::const_iterator helm = scd->Parts().cend();
		AuScParts::const_iterator medic = scd->Parts().cend();
		AuScParts::const_iterator nav = scd->Parts().cend();
		AuScParts::const_iterator power = scd->Parts().cend();
		AuScParts::const_iterator sensor = scd->Parts().cend();
		AuScParts::const_iterator shield = scd->Parts().cend();
		AuScParts::const_iterator transporter = scd->Parts().cend();
		QMap<double, AuScParts::const_iterator> sortedConsoles;	// Eligible consoles keyed/sorted by distance.
		for(AuScReportStationHash::const_iterator si = m_Design.CrewStations().cbegin(); si != m_Design.CrewStations().cend(); si++)
		{
			AuScParts::const_iterator pi = scd->Parts().find(si.key());
			if(pi == scd->Parts().cend())
				continue;

			if(consoles.contains(pi.key()))
				continue;	// Skip manned consoles.

			if(!m_Damage.HitsRemainingAddParts(AuSysId(SYS_CommandAndControl, si.key()), m_Design))
				continue;	// Skip broken consoles.

			AuPoint sloc = (*pi)->OperatorLoc();
			double dist = toLoc.Distance2(sloc);				// square of the distance is faster for simple distance comparisons
			if(dist > 10*10 && destCons.contains(pi.key()))
				continue;	// Skip distant crew station that someone else has already chosen as a destination.

			// Remember certain types of consoles if I have the skill.
			switch((*pi)->StationType())
			{
			case AuStation::Console_Engine:
			case AuStation::Integrated_Engine:
				if(skill == AuSkill::Engineer)
					engineer = pi;
				break;
			case AuStation::Console_FireControl:
			case AuStation::Integrated_FireControl:
				if(skill == AuSkill::FireControl)
					firecontrol = pi;
				break;
			case AuStation::Console_Navigation:
			case AuStation::Integrated_Navigation:
				if(skill == AuSkill::Navigator)
					nav = pi;
				break;
			case AuStation::Console_Helm:
			case AuStation::Integrated_Helm:
				if(skill == AuSkill::Helm)
					helm = pi;
				break;
			case AuStation::Console_Medic:
			case AuStation::Integrated_Medic:
				if(skill == AuSkill::Medic)
					medic = pi;
				break;
			case AuStation::Console_Power:
			case AuStation::Integrated_Power:
				if(skill == AuSkill::Power)
					power = pi;
				break;
			case AuStation::Console_Sensor:
			case AuStation::Integrated_Sensor:
				if(skill == AuSkill::Sensor)
					sensor = pi;
				break;
			case AuStation::Console_Shield:
			case AuStation::Integrated_Shield:
				if(skill == AuSkill::Shield)
					shield = pi;
				break;
			case AuStation::Console_Transporter:
			case AuStation::Integrated_Transporter:
				if(skill == AuSkill::Transporter)
					transporter = pi;
				break;
			default:
				break;
			}

			// Store the eligible console in the map, sorted by distance from me.
			sortedConsoles.insert(dist, pi);
		}

		for(AuScReportTransporterStationHash::const_iterator si = m_Design.Transporters().cbegin();
				si != m_Design.Transporters().cend(); si++)
		{
			AuScParts::const_iterator pi = scd->Parts().find(si.key());
			if(pi == scd->Parts().cend())
				continue;

			if(consoles.contains(pi.key()))
				continue;	// Skip manned consoles.

			if(!m_Damage.HitsRemainingAddParts(AuSysId(SYS_CommandAndControl, si.key()), m_Design))
				continue;	// Skip broken consoles.

			AuPoint sloc = (*pi)->OperatorLoc();
			double dist = toLoc.Distance2(sloc);				// square of the distance is faster for simple distance comparisons
			if(dist > 10*10 && destCons.contains(pi.key()))
				continue;	// Skip distant crew station that someone else has already chosen as a destination.

			// Store the eligible console in the map, sorted by distance from me.
			sortedConsoles.insert(dist, pi);
		}

		// Consoles are now sorted by distance.
		if(!sortedConsoles.isEmpty())
		{
			AuScParts::const_iterator bestIt = *sortedConsoles.begin();
			AuScParts::const_iterator pi;
			if(bestIt.key() > 10*10 && (isCaptAI || !HasHumanOfficer()))
			{
				// No human officer or AI is in command so prioritize manning certain stations.
				if(helm != scd->Parts().cend())
					pi = helm;
				else if(engineer != scd->Parts().cend())
					pi = engineer;
				else if(nav != scd->Parts().cend())
					pi = nav;
				else if(sensor != scd->Parts().cend())
					pi = sensor;
				else if(shield != scd->Parts().cend())
					pi = shield;
				else if(firecontrol != scd->Parts().cend())
					pi = firecontrol;
				else if(medic != scd->Parts().cend())
					pi = medic;
				else if(transporter != scd->Parts().cend())
					pi = transporter;
				else if(power != scd->Parts().cend())
					pi = power;
				else
					pi = bestIt;		// Take the closest remaining unmanned station.
			}
			else
				pi = bestIt;			// Take the closest unmanned station aboard a ship or any vessel with officers.

			AuPoint gravityN(0,0,1);
			AuScReportStationHash::const_iterator si = m_Design.CrewStations().find(pi.key());
			if(si != m_Design.CrewStations().cend())
				gravityN = scd->RoomVoidGravityUp((*si).RoomId(), m_Lifts);
			else
			{
				AuScReportTransporterStationHash::const_iterator si = m_Design.Transporters().find(pi.key());
				if(si != m_Design.Transporters().cend())
					gravityN = scd->RoomVoidGravityUp((*si).RoomId(), m_Lifts);
			}

			stationId = pi.key();
			station = (*pi)->OperatorStation(gravityN);
			which = pi.key();
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	m_DamageLock.unlock();
	m_SystemsLock.unlock();

	// If we got a which id, we found console.
	return which;
}

bool DaSc::ClosestUnoccupiedSurgeryUnit(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
		bool isCrew) const
{
	Q_UNUSED(isCrew);
	stationId = 0;
	which = 0;

	// Get a list of the locations of the consoles that are currently manned.
	QSet<quint16> consoles;
	ListOccupiedSurgeryUnits(consoles);
	QSet<quint16> destCons;
	ListDestinationConsoles(destCons);							// Consoles that are destinations of AI dudes.

	// Locate the closest unmanned crew station that is operational.
	m_SystemsLock.lockForRead();
	m_DamageLock.lockForRead();
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		QMap<double, AuScParts::const_iterator> sortedConsoles;	// Eligible consoles keyed/sorted by distance.
		for(AuScReportStationHash::const_iterator si = m_Design.CrewStations().cbegin(); si != m_Design.CrewStations().cend(); si++)
		{
			if(consoles.contains(si.key()))
				continue;	// Skip manned surgery units.

			AuScParts::const_iterator pi = scd->Parts().find(si.key());
			if(pi == scd->Parts().cend())
				continue;

			// We are only looking for medic consoles.
			if(!AuStation::IsMedic((*pi)->StationType()))
				continue;

			if(!m_Damage.HitsRemainingAddParts(AuSysId(SYS_CommandAndControl, si.key()), m_Design))	//DHH CnC for surgery unit?
				continue;	// Skip broken consoles.

			// Ensure the console has a doctor to operate it.
			if(!HasConsoleOperator(si.key()))
				continue;	// Skip units that have no doctor to operate them. No doctor could mean the med station is broken.

			AuPoint sloc = (*pi)->PatientLoc();
			double dist = toLoc.Distance2(sloc);				// square of the distance is faster for simple distance comparisons
			if(dist > 10*10 && destCons.contains(pi.key()))
				continue;	// Skip distant surgery unit that someone else has already chosen as a destination.

			// Store the eligible surgery unit in the map, sorted by distance from me.
			sortedConsoles.insert(dist, pi);
		}

		// Surgery units are now sorted by distance.
		if(!sortedConsoles.isEmpty())
		{
			AuScParts::const_iterator bestIt = *sortedConsoles.begin();
			AuScParts::const_iterator pi = bestIt;			// Take the closest unmanned surgery unit.

			AuPoint gravityN(0,0,1);
			AuScReportStationHash::const_iterator si = m_Design.CrewStations().find(pi.key());
			if(si != m_Design.CrewStations().cend())
				gravityN = scd->RoomVoidGravityUp((*si).RoomId(), m_Lifts);

			stationId = pi.key();
			station = (*pi)->PatientStation(gravityN);
			which = pi.key();
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	m_DamageLock.unlock();
	m_SystemsLock.unlock();

	// If we got a which id, we found console.
	return which;
}

void DaSc::PatientStation(quint16 medicStationId, quint16& surgeryId, AuStation& station) const
{
	surgeryId = 0;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator pi = scd->Parts().find(medicStationId);
		if(pi != scd->Parts().cend())
		{
			surgeryId = medicStationId;
			AuPoint gravityN(0,0,1);
			AuScReportStationHash::const_iterator si = m_Design.CrewStations().find(medicStationId);
			if(si != m_Design.CrewStations().cend())
				gravityN = scd->RoomVoidGravityUp((*si).RoomId(), m_Lifts);
			station = (*pi)->PatientStation(gravityN);
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
}

bool DaSc::ClosestUnoccupiedTroopStation(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
		const AuId& ignoreTroop) const
{
	stationId = 0;
	which = 0;

	// Get a list of the locations of the consoles that are currently manned.
	QSet<quint16> consoles;
	ListVisiblyOccupiedConsoles(0, consoles);
	QSet<quint16> destCons;
	ListDestinationConsoles(destCons);							// Consoles that are destinations of AI dudes.
	QSet<quint16> troopStations;
	ListOccupiedTroopStations(troopStations, ignoreTroop);

	// Locate the closest unmanned crew station that is operational.
	m_SystemsLock.lockForRead();
	m_DamageLock.lockForRead();
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		QMap<double, AuScParts::const_iterator> sortedConsoles;	// Eligible consoles keyed/sorted by distance.
		for(AuScReportTurretHash::const_iterator si = m_Design.Turrets().cbegin(); si != m_Design.Turrets().cend(); si++)
		{
			AuScParts::const_iterator pi = scd->Parts().find(si.key());
			if(pi == scd->Parts().cend())
				continue;

			if(consoles.contains(pi.key()))
				continue;	// Skip manned consoles.

			if(!m_Damage.HitsRemainingAddParts(AuSysId(SYS_Turret, si.key()), m_Design))
				continue;	// Skip broken consoles.

			AuPoint sloc = (*pi)->OperatorLoc();
			double dist = toLoc.Distance2(sloc);				// square of the distance is faster for simple distance comparisons
			if(dist > 10*10 && destCons.contains(pi.key()))
				continue;	// Skip distant crew station that someone else has already chosen as a destination.

			// Store the eligible console in the map, sorted by distance from me.
			sortedConsoles.insert(dist, pi);
		}

		// Consoles are now sorted by distance.
		if(!sortedConsoles.isEmpty())
		{
			AuScParts::const_iterator bestIt = *sortedConsoles.begin();
			AuScParts::const_iterator pi = bestIt;
			AuPoint gravityN(0,0,1);
			AuScReportTurretHash::const_iterator si = m_Design.Turrets().find(pi.key());
			if(si != m_Design.Turrets().cend())
				gravityN = scd->RoomVoidGravityUp((*si).RoomId(), m_Lifts);

			stationId = pi.key();
			station = (*pi)->OperatorStation(gravityN);
			which = pi.key();
		}

		if(!which)
		{
			// No unmanned unclaimed turret was found.
			// Check for an unmanned troop station.
			QMap<double, AuScParts::const_iterator> sortedStations;	// Eligible consoles keyed/sorted by distance.
			for(QSet<quint16>::const_iterator si = m_Design.GuardPosts().cbegin(); si != m_Design.GuardPosts().cend(); si++)
			{
				AuScParts::const_iterator pi = scd->Parts().find(*si);
				if(pi == scd->Parts().cend())
					continue;

				if(troopStations.contains(*si))
					continue;	// Skip manned troop stations.

				AuPoint sloc = (*pi)->OperatorLoc();
				double dist = toLoc.Distance2(sloc);			// square of the distance is faster for simple distance comparisons
				if(dist > 10*10 && destCons.contains(pi.key()))
					continue;	// Skip distant crew station that someone else has already chosen as a destination.

				// Store the eligible console in the map, sorted by distance from me.
				sortedConsoles.insert(dist, pi);
			}

			// Consoles are now sorted by distance.
			if(!sortedConsoles.isEmpty())
			{
				AuScParts::const_iterator bestIt = *sortedConsoles.begin();
				AuScParts::const_iterator pi = bestIt;
				AuPoint gravityN(0,0,1);
				AuScReportTurretHash::const_iterator si = m_Design.Turrets().find(pi.key());
				if(si != m_Design.Turrets().cend())
					gravityN = scd->RoomVoidGravityUp((*si).RoomId(), m_Lifts);

				stationId = pi.key();
				station = (*pi)->OperatorStation(gravityN);
				which = pi.key();
			}
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	m_DamageLock.unlock();
	m_SystemsLock.unlock();

	// If we got a which id, we found console.
	return which;
}

bool DaSc::ClosestVehicleParkingSpot(const DaVehicle* vehicle, AuMatrix4x4& parkPosM, VEHICLE_SPOT_TYPE& spotType) const
{
	quint16 roomId = vehicle->ShipRoomId();
	if(!roomId)
		return false;
	QList<AuMatrix4x4> spotsInRoom;
	int spaceRocketSpots = 0;
	VEHICLE_SPOT_TYPE vehSpotType = SPOT_Invalid;
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		if(vehicle->IsSpaceRocket())
		{
			scd->RocketParkingSpots(spotsInRoom);	// Space rocket can park in space rocket spots and space vehicle spots.
			spaceRocketSpots = spotsInRoom.size();
			vehSpotType = SPOT_Rocket;
		}

		if(vehicle->IsGroundVehicle())
		{
			scd->GroundVehicleParkingSpots(spotsInRoom);
			vehSpotType = SPOT_GroundVehicle;
		}
		else if(vehicle->IsWaterVehicle())
		{
			scd->WaterVehicleParkingSpots(spotsInRoom);
			vehSpotType = SPOT_WaterVehicle;
		}
		else// if(vehicle->IsSpaceVehicle())
		{
			// Space vehicles and aircraft, e.g. helicopter.
			scd->SpaceVehicleParkingSpots(spotsInRoom);	// This includes space rockets.
			vehSpotType = SPOT_SpaceVehicle;
		}
	}
	UnlockDesign();
	if(spotsInRoom.empty())
		return false;

	// Locate the closest appropriate spot within 7m of the vehicle.
	bool foundSpot = false;
	double maxDist2 = FeetPerMeter()*7 * FeetPerMeter()*7;	// Farthest distance to drag vehicles around.
	double closestDist2 = 0;
	int index = 0;
	for(QList<AuMatrix4x4>::const_iterator si = spotsInRoom.cbegin(); si != spotsInRoom.cend(); si++, index++)
	{
		AuPoint vehLoc = (*si).UnProject(vehicle->Location());	// Vehicle location in the space of the parking spot.
		double dist2 = vehLoc.Length2();
		if(dist2 > maxDist2)
			continue;
		if(vehLoc.z < 0)
			continue;	// Vehicle CG is below the parking spot.
		else if(vehLoc.z > 3.5*FeetPerMeter())
			continue;	// Vehicle CG is too far above the parking spot.
		if(!foundSpot || dist2 < closestDist2)
		{
			closestDist2 = dist2;
			parkPosM = *si;
			foundSpot = true;
			if(index < spaceRocketSpots)
				spotType = SPOT_Rocket;
			else
				spotType = vehSpotType;
		}
	}
	return foundSpot;
}

bool DaSc::ColonistVoidLocation(AuPoint& voidLoc) const
{
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	quint16 roomId = scd->RandomRoomVoidLocation(voidLoc, PERSONNEL_Passengers, m_DoorStates, m_LandingGearStates, m_Lifts);
	if(!roomId)
		roomId = scd->RandomRoomVoidLocation(voidLoc, PERSONNEL_Everyone, m_DoorStates, m_LandingGearStates, m_Lifts);
	UnlockDesign();
	m_DesignLock.unlock();
	return roomId;
}

void DaSc::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	DaVessel::CombatArmorEffectiveness(mode, effect, hitShield, wpnLoc, hitLoc, baseAbsorb, baseBlock, auxAbsorb, auxBlock);
	if(hitShield)
	{
		// Shields are working and the hit location was on the shield.
		AuPoint::DIRECTION dir = (hitLoc - DesignCenterOffset()).Direction();
		m_ShieldsLock.lockForRead();
		float capAvail = m_ShieldCap[dir];
		m_ShieldsLock.unlock();

		// Block a number of hit points based on the energy available to the shields.
		// Shields use one point of capacitor for every 10 units of energy absorbed, decreased with quality.
		m_DesignLock.lockForRead();
		auxBlock += capAvail / m_Design.ShieldDissipatePerDamagePoint();
		m_DesignLock.unlock();
	}
}

void DaSc::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	DaVessel::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);
	if(InDesignEnvironment())
		return;
	ALERT_CONDITION oldCondition = AlertCondition();
	if(result.m_AuxArmor1 || result.m_AuxArmor2)
	{
		// Take power from shield capacitor due to shield absorption.
		int shieldAbsorbed = result.m_AuxArmor1 + result.m_AuxArmor2;

		// The shields use one point of capacitor for every 10 units of energy absorbed, decreased with quality.
		m_DesignLock.lockForRead();
		float capUsed = shieldAbsorbed * m_Design.ShieldDissipatePerDamagePoint();
		m_DesignLock.unlock();

		// Take energy from the appropriate shield capacitor.
		// hitLoc is the point on my shields that was hit.
		// Get direction of vector from the hit location toward the center of the design, which is the center of the shield bubble.
		AuPoint::DIRECTION dir = (hitLoc - DesignCenterOffset()).Direction();
		m_ShieldsLock.lockForWrite();
		unsigned int scram = m_ShieldScram[dir];
		if(m_ShieldCap[dir] > capUsed)
			m_ShieldCap[dir] -= capUsed;
		else
		{
			// Shield reduced to zero.
			scram += 20;
			m_ShieldCap[dir] = 0;
		}
		//if(result.m_CriticalHits)
			//scram += result.m_CriticalHits * 6;	// sec per crit
		if(scram > 60)
			scram = 60;
		m_ShieldScram[dir] = scram;
		m_ShieldsLock.unlock();
		SetModified();
	}

	// Stations will broadcast an SOS every two minutes while being attacked, if the attacker is causing damage.
	// Ship commanded by an AI officer also broadcasts SOS.
	if(!attacker.m_Id.IsNull() &&
			(result.m_Damage1 || result.m_BaseArmor1 || result.m_AuxArmor1 ||
			 result.m_Damage2 || result.m_BaseArmor2 || result.m_AuxArmor2) &&
			(!LastSOS().isValid() || LastSOS().ElapsedSec() > 180) &&
			(IsSpaceStation() || IsCommandedByAI()) &&
			!IsPirateWithFreeWill())
		SendSOS(6, attacker.m_Name, 0);					// Under Attack

	if(!result.m_Damage1 && !result.m_Damage2)
		return;

	// The damage that was not absorbed by any armor is applied to ship systems.

	// Critical hits go to internal systems and non-critical hits go to the hull.
	int hullUnitsDamage1;									// Units of damage caused to hull by effect 1.
	int intUnitsDamage1;									// Units of damage caused to internal systems by effect 1.
	int hullUnitsDamage2;									// Units of damage caused to hull by effect 2.
	int intUnitsDamage2;									// Units of damage caused to internal systems by effect 2.
	if(result.m_CriticalHits == 2)
	{
		// All damage is distributed to internal systems based on structural integrity.
		float strInteg = StructuralIntegrity();
		unsigned int idmg1 = (int)ceilf(result.m_Damage1 * strInteg);
		unsigned int hdmg1 = result.m_Damage1 - idmg1;				// Hull takes the remainder.
		unsigned int idmg2 = (int)ceilf(result.m_Damage2 * strInteg);
		unsigned int hdmg2 = result.m_Damage2 - idmg2;				// Hull takes the remainder.

		hullUnitsDamage1 = hdmg1;
		intUnitsDamage1 = idmg1;
		hullUnitsDamage2 = hdmg2;
		intUnitsDamage2 = idmg2;
	}
	else if(result.m_CriticalHits)
	{
		// Damage one is distributed to internal systems based on structural integrity.
		// Damage two is applied to hull systems.
		float strInteg = StructuralIntegrity();
		unsigned int idmg1 = (int)ceilf(result.m_Damage1 * strInteg);
		unsigned int hdmg1 = result.m_Damage1 - idmg1;				// Hull takes the remainder.

		hullUnitsDamage1 = hdmg1;
		intUnitsDamage1 = idmg1;
		hullUnitsDamage2 = result.m_Damage2;
		intUnitsDamage2 = 0;
	}
	else
	{
		// All damage is applied to hull systems.
		hullUnitsDamage1 = result.m_Damage1;
		intUnitsDamage1 = 0;
		hullUnitsDamage2 = result.m_Damage2;
		intUnitsDamage2 = 0;
	}

	// Record damage dealt into this unit's damage log.
	// Damage is recorded as numbers of units damaged, not numbers of hit points dealt.
	bool isAttackerPlunderingMe = false;
	quint32 plunderingEmpire = 0;
	QString text;
	if((!attacker.m_Id.IsNull() || weapon.m_CID != CID_Invalid) && !InDesignEnvironment())
	{
		int iDamageDealt = hullUnitsDamage1 + intUnitsDamage1 + hullUnitsDamage2 + intUnitsDamage2;
		RecordDamage(attacker.m_Id, iDamageDealt, weapon.m_CID, weapon.m_Quality);
		if(!attacker.m_Id.IsNull())
		{
			// Send a message to the crew of this ship that they received damage.
			text = tr("The ship is under attack by %1.").arg(attacker.m_Name);

			// Reward the atom for each multiple of 10 hits of damage dealt to the craft
			DaAtom* solar = FindEnvironmentAtom();
			if(solar)
			{
				DaAtom* atom = solar->FindIdRecursive(attacker.m_Id);
				if(atom)
				{
					DaVessel* spaceship = dynamic_cast<DaVessel*>(atom);
					if(spaceship)
					{
						// Watch for attacking vessels who want me to stand down.
						if(spaceship->MissionOrderType() == AuMissionOrder::Plunder &&
								Is(spaceship->MissionOrderTarget()))
						{
							isAttackerPlunderingMe = true;	// The attacker wants to plunder me.
							plunderingEmpire = spaceship->EmpireId();
						}

						POLITICAL_STANCE stance = spaceship->PoliticalStanceMakeContact(this);
						if(stance == POL_Enemy)
						{
							if(IsSpaceStation())
								spaceship->RewardShip(1, REW_DamageStation, 0);
							else if(IsStarship())
								spaceship->RewardShip(1, REW_DamageStarship, 0);
							else
								spaceship->RewardShip(1, REW_DamageSpaceship, 0);
						}
						else
						{
							//DHH What about a penalty for friendly fire?
						}
					}
					else
					{
						DaPerson* person = dynamic_cast<DaPerson*>(atom);
						if(person)
						{
							POLITICAL_STANCE stance = person->PoliticalStanceMakeContact(this);
							if(stance == POL_Enemy)
							{
								if(IsSpaceStation())
									person->Reward(0, REW_DamageStation);
								else if(IsStarship())
									person->Reward(0, REW_DamageStarship);
								else
									person->Reward(0, REW_DamageSpaceship);
								person->IncreaseSkill(AuSkill::SpacecraftCombat, 1);
							}
							else
							{
								//DHH What about a penalty for friendly fire?
							}
						}
					}
				}
			}
		}
		else
		{
			// Send a message to the crew of this ship that they received damage.
			text = tr("The ship was damaged by %1.").arg(attacker.m_Name);
		}
	}

	// Apply damage to systems.
	bool cargoLost = false;
	bool fuelLeaked = false;
	if(DamageSystem(result, true, hullUnitsDamage2, 2, weapon, attacker.m_Name, cargoLost, fuelLeaked) ||
			DamageSystem(result, false, intUnitsDamage2, 2, weapon, attacker.m_Name, cargoLost, fuelLeaked) ||
			DamageSystem(result, true, hullUnitsDamage1, 1, weapon, attacker.m_Name, cargoLost, fuelLeaked) ||
			DamageSystem(result, false, intUnitsDamage1, 1, weapon, attacker.m_Name, cargoLost, fuelLeaked))
	{
		result.m_IsDestroyed = true;
		return;
	}

	bool specialDamage = false;
	if(cargoLost)
	{
		text += tr(" Cargo was lost.");
		specialDamage = true;
	}
	else if(fuelLeaked)
	{
		text += tr(" Fuel leaked.");
		specialDamage = true;
	}

	// Call out when special damage is received or when the last damage by 'attacker.m_Id' was more than 4 seconds ago.
	// Pirates do not do this.
	AuTime lastdmg =  LastUnitDamage(attacker.m_Id);
	if(!text.isEmpty() && (specialDamage || lastdmg.IsNull() || lastdmg.ElapsedSec() > 4) && !InDesignEnvironment() &&
			!IsPirateWithFreeWill())
	{
		// Send a message to the crew of this ship that they received damage.
		AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
		msg.SetScopeData(Id().GetID());	// Crew channel needs ship's id to know the scope of the message.
		g_Au.SendAtomChat(msg);

		DaSentient* officer = FindOfficer();			// Find any officer, even if they do not have the con.
		if(officer)
		{
			officer->LogEvent(AuEventLogEntry::DamageReceived, text);

			// The officer who has the con must decide if he wants to surrender.
			if(isAttackerPlunderingMe && !InArena() && officer->HasTheCon() && officer->Posture() != POSTURE_Berserk &&
					MissionOrderType() != AuMissionOrder::Scuttle)
			{
				// The probability of surrendering increases as his ship gets more damaged.
				// Higher ranking officers have a teensy bit less chance of surrendering.
				// We don't surrender when waiting for scuttle sequence to complete.
				int curHits = HitsDamageStructural();
				int maxHits = DesignHitsStructural();
				if(g_Au.Dice().Roll(1, maxHits, -officer->Rank()) > curHits)
				{
					// Stand down.
					if(IsMissionActive())
					{
						MissionInsertOrder(0, AuMissionOrderStandDown(10, AuMissionOrder::DoOnce));
					}
					else
					{
						// Ship mission is currently on hold. Set this up to go back to hold after standing down.
						MissionInsertOrder(0, AuMissionOrderHoldMission(AuMissionOrder::DoOnce));
						MissionInsertOrder(0, AuMissionOrderStandDown(10, AuMissionOrder::DoOnce));
						MissionSetActive(true);					// Activate my mission.
					}

					QString standDown(tr("Stand down! All hands stand down from stations. Prepare for boarding."));
					AuCommChat msg(standDown, AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
					msg.SetScopeData(Id().GetID());		// Crew channel needs ship's id to know the scope of the message.
					g_Au.SendAtomChat(msg);

					// Hail surrender.
					QString surrender(tr("Hold your fire! We surrender. All hands are standing down."));
					AuCommChat msg2(surrender, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg2);

					// Log the event.
					officer->LogEvent(AuEventLogEntry::StandingDown, attacker.m_Name);

					// Notify the empire of a diplomatic incident.
					if(EmpireId() != plunderingEmpire)
					{
						const DaSector* sector = FindParentSector();
						const DaSolarSystem* system = FindParentSolarSystem();
						const DaWorld* world = FindParentPlanetInEnv();
						QString sectorName = sector ? sector->Name() : "";
						QString systemName = system ? system->Name() : "";
						QString worldName = world ? world->Name() : "";
						switch(Structure())
						{
						case SC_Count:
						case SC_Deprecated:
							break;
						case SC_Hull:
						case SC_Spaceship:
							{
								AuDocSpaceshipPlundered doc(Name(), attacker.m_Name, Galaxy(), sectorName, systemName,
										worldName);
								emit GovDossierAddDocument(EmpireId(), plunderingEmpire, doc.Pack());
							}
							break;
						case SC_Starship:
							{
								AuDocStarshipPlundered doc(Name(), attacker.m_Name, Galaxy(), sectorName, systemName,
										worldName);
								emit GovDossierAddDocument(EmpireId(), plunderingEmpire, doc.Pack());
							}
							break;
						case SC_Station:
							{
								AuDocSpaceStationPlundered doc(Name(), attacker.m_Name, Galaxy(), sectorName, systemName,
										worldName);
								emit GovDossierAddDocument(EmpireId(), plunderingEmpire, doc.Pack());
							}
							break;
						}
					}
				}
			}
		}
	}

	// Kick this object into motion if it isn't moored.
	if(!IsMoored() && Mass() > 0 && wpnLoc != hitLoc)
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
			SetModified();
		}
	}

	// Apparent alert condition may have changed due to damage. This is not the same as the alert condition set by the commander.
	ALERT_CONDITION nuCondition = AlertCondition();
	if(oldCondition != nuCondition)
		SetAlertCondition(nuCondition);

	// Check because a bug somehow allowed ships to have more damage than should be allowed.
	unsigned int dmgHits = HitsDamageStructural();		// Current hits of damage to the structure.
	unsigned int maxHits = DesignHitsStructural();		// Total structure hit points.
	if(dmgHits >= maxHits)
	{
		if(dmgHits > maxHits)
			qWarning() << "error: DaSc::CombatApplyDamage destroying" << Id() << Name() << "dmg" << dmgHits
				<< "exceeds max" << maxHits;
		DestroyShip(tr("Structure buckled due to excess damage."));
		result.m_IsDestroyed = true;
	}
	else if(!result.m_AuxArmor1 && !result.m_AuxArmor2)
	{
		// Did not hit shield.
		if(weapon.m_CID != CID_Fire && !IsBurning())
		{
			// The damage was not from an existing fire condition.
			// Did the ship catch on fire.
			float damage = (float)dmgHits/(float)maxHits;
			if(damage >= .9f)
			{
				// At 90% damage, all spacecraft catch fire.
				AddCondition(new AuConFire(AuTime::Now()));
				SetModified();
			}
		}
		if((weapon.m_w1.m_Effect == WE_Acid && result.m_Damage1) || (weapon.m_w2.m_Effect == WE_Acid && result.m_Damage2))
		{
			// Acid damage may induce an acid condition on this spacecraft.
			if(weapon.m_CID != CID_Acid && !HasAcid())
			{
				// The damage was not from an existing acid condition.
				// This spacecraft got corroded.
				// Strength of the acid is determined by total unabsorbed strength of the acid damage.
				int acidStrength;
				if(weapon.m_w1.m_Effect == WE_Acid)
					acidStrength = result.m_Damage1;
				else
					acidStrength = result.m_Damage2;

				// Add the acid as a condition on this spacecraft.
				//qDebug() << Id() << Name() << "corroded with acid, strength" << acidStrength;
				AuConAcid* acid = new AuConAcid(acidStrength, weapon.m_Quality, AuTime::Now());
				AddCondition(acid);
			}
		}
		if((result.m_Damage1 && weapon.IsIncendiary1()) || (result.m_Damage2 && weapon.IsIncendiary2()))
		{
			// Heat damage may induce a fire condition on this spacecraft.
			if(weapon.m_CID != CID_Fire && !IsBurning())
			{
				// The damage was not from an existing fire condition.
				// This spacecraft got burned.
				// Chance to add fire as a condition on this spacecraft is based on building material and current damage.
				if(g_Au.Dice().Roll(1,100) <= FireProbability())
				{
					AuConFire* fire = new AuConFire(AuTime::Now());
					AddCondition(fire);
				}
			}
		}
	}
}

bool DaSc::DamageSystem(const AuCombatResult& result, bool structural, quint32 hitPoints, int wpn1or2, const AuCombatWeapon& weapon,
		const QString& attackerName, bool& cargoLost, bool& fuelLeaked)
{
	if(!hitPoints)
		return false;
	bool destroyed = false;
	AuSysId sysId;
	if(structural)
		sysId = SelectStructuralSystemToDamage();
	else
		sysId = SelectNonstructuralSystemToDamage();
	if(sysId.System() != SYS_Count)
	{
		// A system took damage.
		COMMODITYID sysModule = SystemModule(sysId);
		m_DamageLock.lockForWrite();
		m_Damage.AddDamage(sysId, sysModule, hitPoints);
		quint32 dmgHits = m_Damage.HitsDamageStructural();
		m_DamageLock.unlock();
		DamageStateChanged();

		// Determine if I was destroyed.
		destroyed = dmgHits >= DesignHitsStructural();
		if(!destroyed)
		{
			switch(sysId.System())
			{
			case SYS_Fuel:
				if(g_Au.Dice().Roll(1,100) == 1)
					if(DestroyFuel(hitPoints))
						fuelLeaked = true;
				break;
			case SYS_Hold:
				if(g_Au.Dice().Roll(1,100) == 1)
					if(DestroyCargo(hitPoints))
						cargoLost = true;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		// No undamaged system remains to take damage. Destroy the ship.
		destroyed = true;
	}

	if(destroyed)
	{
		// This ship is destroyed.
		QString text = FormatDamageReceived(result.m_CriticalHits, weapon.m_w1.m_Effect, result.m_Damage1, result.Armor1(),
				weapon.m_w2.m_Effect, result.m_Damage2, result.Armor2(), attackerName);
		if(wpn1or2 == 2)
			text += tr(" %1 caused critical %2 failure.")
				.arg(EffectName(weapon.m_w2.m_Effect))
				.arg(sysId.ScName());
		else
			text += tr(" %1 caused critical %2 failure.")
				.arg(EffectName(weapon.m_w1.m_Effect))
				.arg(sysId.ScName());

		LogEvent(AuEventLogEntry::DamageReceived, text);
		DestroyShip(text);
	}
	return destroyed;
}

bool DaSc::ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const
{
	return DaVessel::ConstrainCamera(subatom, camera);
}

void DaSc::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	//qDebug() << "DaSc::CreateDeviceObjects" << Id() << Name();
	DaVessel::CreateDeviceObjects(ari);

	if(!m_CmdEmpty.HasDeviceObjects())
	{
		// Create models of unmanned consoles.
		m_CmdEmpty.CreateDeviceObjectsEmpty(ari, *this, "Empty", 0, false);
		m_NavEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("NCon%1").arg(Id().GetID()), ":data/txConsoleNav.bmp", false);
		m_EngineEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("ECon%1").arg(Id().GetID()), ":data/ec_Console.bmp", false);
		m_FireEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("WCon%1").arg(Id().GetID()), ":data/txConsoleWeapon.bmp", false);
		m_PilotEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("PCon%1").arg(Id().GetID()), ":data/pc_Console.bmp", false);
		m_PowerEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("DCon%1").arg(Id().GetID()), ":data/dc_Console.bmp", false);
		m_SensorEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("SCon%1").arg(Id().GetID()), ":data/sc_Console.bmp", false);
		m_ShieldEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("HCon%1").arg(Id().GetID()), ":data/txConsoleShield.bmp", false);
		m_SurgeryEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("MCon%1").arg(Id().GetID()), ":data/txConsoleSurgery.bmp",
				false);
		m_TransEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("TCon%1").arg(Id().GetID()), ":data/txConsoleTransporter.bmp",
				false);
		m_AlertGreen = ari.re->CreateTextureFromBitmap(Id().GetID(), "AGD", ":data/AlertGreen.png");
		m_AlertRedBright = ari.re->CreateTextureFromBitmap(Id().GetID(), "AYB", ":data/AlertRedBright.png");
		m_AlertRedDim = ari.re->CreateTextureFromBitmap(Id().GetID(), "AYD", ":data/AlertRedDim.png");
		m_AlertYellowBright = ari.re->CreateTextureFromBitmap(Id().GetID(), "ARB", ":data/AlertYellowBright.png");
		m_AlertYellowDim = ari.re->CreateTextureFromBitmap(Id().GetID(), "ARD", ":data/AlertYellowDim.png");
		m_LifeSupportStatusTex = ari.re->CreateTextureFromBitmap(Id().GetID(), "LSI", ":data/LifeSupportStatusIndicators.png");

		m_WorldMapTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WormholeSpace", ":data/WormholeSpace.png",
				GL_LINEAR, GL_LINEAR, true);

		// Load hull surface damage textures.
		for(int dmg=0; dmg<9; dmg++)
			m_DamageTex[dmg] =
				AuTextureLibItem::EffectTexture(*ari.re, (AuTextureLibItem::EFFECT_INDEX)(AuTextureLibItem::EI_Damage1+dmg));

		// Load all frames of the effect textures.
		for(int eff=0; eff<EFF_Count; eff++)
			for(int frame=0; frame<6; frame++)
				m_EffectTex[eff][frame] = AuTextureLibItem::EffectTexture(*ari.re, (SURFACE_EFFECT)eff, frame);

		// Maneuver drives in low power mode.
		m_EffectTex[EFF_Count][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow1);
		m_EffectTex[EFF_Count][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow2);
		m_EffectTex[EFF_Count][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow3);
		m_EffectTex[EFF_Count][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow4);
		m_EffectTex[EFF_Count][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow5);
		m_EffectTex[EFF_Count][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow6);

		// Transporter pads in low power mode.
		m_EffectTex[EFF_Count+1][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow1);
		m_EffectTex[EFF_Count+1][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow2);
		m_EffectTex[EFF_Count+1][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow3);
		m_EffectTex[EFF_Count+1][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow4);
		m_EffectTex[EFF_Count+1][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow5);
		m_EffectTex[EFF_Count+1][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow6);

		// FTL hull in warp.
		m_EffectTex[EFF_Count+2][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp1);
		m_EffectTex[EFF_Count+2][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp2);
		m_EffectTex[EFF_Count+2][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp3);
		m_EffectTex[EFF_Count+2][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp4);
		m_EffectTex[EFF_Count+2][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp5);
		m_EffectTex[EFF_Count+2][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp6);

		// FTL drives in low power mode.
		m_EffectTex[EFF_Count+3][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow1);
		m_EffectTex[EFF_Count+3][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow2);
		m_EffectTex[EFF_Count+3][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow3);
		m_EffectTex[EFF_Count+3][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow4);
		m_EffectTex[EFF_Count+3][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow5);
		m_EffectTex[EFF_Count+3][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow6);

		// FTL hull in negative wormhole.
		m_EffectTex[EFF_Count+4][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg1);
		m_EffectTex[EFF_Count+4][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg2);
		m_EffectTex[EFF_Count+4][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg3);
		m_EffectTex[EFF_Count+4][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg4);
		m_EffectTex[EFF_Count+4][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg5);
		m_EffectTex[EFF_Count+4][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg6);

		// FTL hull in neutral wormhole.
		m_EffectTex[EFF_Count+5][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu1);
		m_EffectTex[EFF_Count+5][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu2);
		m_EffectTex[EFF_Count+5][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu3);
		m_EffectTex[EFF_Count+5][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu4);
		m_EffectTex[EFF_Count+5][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu5);
		m_EffectTex[EFF_Count+5][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu6);
	}

	// Get empire theme colors.
	QColor themeColors[AuScFace::THEME_Count];
	const DgData& gd = g_Au.LockDataForRead();
	bool hasEmpire = gd.EmpireThemeColors(EmpireId(), themeColors);
	g_Au.UnlockData();
	if(!hasEmpire && EmpireId())
		return;

	if(!m_OwnTextures.empty())
	{
		AnimateEmpireFlagAndName(*ari.re);
		AnimateWorldMap();
	}

	if(!m_VAOsCreated)
	{
		// We have not created VAOs of anything.
		m_FlagEmpire = 0;	// So the animate function will make the correct texture for me.
		const AuScDesign* scd = LockDesign();
		if(!scd)
		{
			// Design is not available locally.
			UnlockDesign();
			//qDebug() << "DaSc::CreateDeviceObjects" << Id() << Name() << "design" << designId << "not available.";
			return;
		}
		//qDebug() << "DaSc::CreateDeviceObjects" << Id() << Name() << "design" << DesignId();

		// Get/load the texture ids of the design's texture palette.
		COMMODITYID hullModule = SystemModule(SYS_Hull);
		scd->ShaderTexturePalette(hullModule, *ari.re, m_SurfaceTex);

		// Triangle lists for things that can be combined into a single VAO.
		quint16 beaconPartId = 0;
		AuScTriRenderList beaconOpaqueTris, beaconTransparentTris;
		quint16 decalPartId = 0;
		AuScTriRenderList decalOpaqueTris, decalTransparentTris;
		quint16 headlightPartId = 0;
		AuScTriRenderList headlightOpaqueTris, headlightTransparentTris;
		quint16 hullPartId = 0;
		AuScTriRenderList hullOpaqueTris, hullTransparentTris;
		quint16 strobePartId = 0;
		AuScTriRenderList strobeOpaqueTris, strobeTransparentTris;

		// Make a VAO of each part of the ship.
		const AuScParts& parts = scd->Parts();
		for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		{
			if((*pi)->IsOmitted())
				continue;
			if((*pi)->IsLight())
			{
				m_HColor = (*pi)->LightColor();
				m_HCosInnerCone = (*pi)->LightCosInnerCone();
				m_HCosOuterCone = (*pi)->LightCosOuterCone();
				m_HDir = (*pi)->LightDir();
				m_HkL = (*pi)->LightkL();
				m_HkQ = (*pi)->LightkQ();
				m_HPos = (*pi)->LightLoc();
				continue;
			}
			if(!(*pi)->IsVisibleInFinalModel())
				continue;

			if((*pi)->IsDoor())
			{
				// Door VAOs are built in a later block of this function. This one simply builds door rendering information.
				for(AuScParts::const_iterator ti = parts.cbegin(); ti != parts.cend(); ti++)
				{
					if((*ti)->IsOmitted())
						continue;
					if(!(*ti)->IsRoomVoid())
						continue;
					if((*ti)->AssociatedPartId() != pi.key() || (*ti)->AssociatedPartState() != AuScDoor::Open)
						continue;
					// Found a room void associated with the open state of the door.
					m_DoorRooms.insert(pi.key(), (*ti)->GroupName());
					break;
				}
			}
			else if((*pi)->Type() == AuScPart::Part_StatusMission)
			{
				// Mission status screens all share the same texture, which must be unique to each ship design.
				if(!m_MissionStatusTex)
				{
					QString name = QString("MStat%1").arg(Id().GetID());
					m_MissionStatusTex = ari.re->CreateTextureFromBitmap(Id().GetID(), name, ":data/MissionStatus.bmp");
				}
				if(!m_MissionStatusTex)
					continue;
				m_OwnTextures.insert(pi.key(), m_MissionStatusTex);

				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnOpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->Type() == AuScPart::Part_LightLens ||
					(*pi)->Type() == AuScPart::Part_HeadlightLens_Deprecated)
			{
				// Light
				(*pi)->RenderTriangles(headlightOpaqueTris, headlightTransparentTris, themeColors);
				m_LensHeadlightGlows.insert(pi.key(), AuLensGlow((*pi)->LightLoc(), (*pi)->LightColor()));
				headlightPartId = pi.key();
			}
			else if((*pi)->Type() == AuScPart::Part_NavLightLens)
			{
				// Nav lights cannot be combined into a single VAO because they glow in different colors.
				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
				m_LensNavGlows.insert(pi.key(), AuLensGlow((*pi)->LightLoc(), (*pi)->LightColor()));

				// Create one VAO for the nav light lenses.
				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_LensNavOpaqueVAOs.insert(pi.key(), vao);
				}
				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_LensNavTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->Type() == AuScPart::Part_RotatingBeaconLens)
			{
				// Rotating beacon.
				(*pi)->RenderTriangles(beaconOpaqueTris, beaconTransparentTris, themeColors);
				m_LensBeaconGlows.insert(pi.key(), AuLensGlow((*pi)->LightLoc(), (*pi)->LightColor()));
				m_LensBeaconAxes.insert(pi.key(), (*pi)->LightRotationAxis());
				beaconPartId = pi.key();
			}
			else if((*pi)->Type() == AuScPart::Part_StrobeLens)
			{
				// Strobe light.
				(*pi)->RenderTriangles(strobeOpaqueTris, strobeTransparentTris, themeColors);
				m_LensStrobeGlows.insert(pi.key(), AuLensGlow((*pi)->LightLoc(), (*pi)->LightColor()));
				strobePartId = pi.key();
			}
			else if((*pi)->IsWindow())
			{
				// Window VAOs are lit by rooms so they cannot be combined.
				if((*pi)->HasGroupName() && (*pi)->GroupName() != AuScPart::GroupNameHull())
					m_WindowRooms.insert(pi.key(), (*pi)->GroupName());
				else
					m_WindowRooms.insert(pi.key(), QString("%1%2").arg((*pi)->PartName()).arg(pi.key()));
				AuScTriRenderList translucentTris, transparentTris;
				(*pi)->RenderTriangles(translucentTris, transparentTris, themeColors);

				if(!translucentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, translucentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_WindowTranslucentVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_WindowTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->IsDecal() && (*pi)->HasOwnTexture())
			{
				// Own Decal VAOs.
				// This is also where flag decals get created. The empire flag is often not ready yet so we'll wait for animate
				// to fix the aspect ratio of the vao.
				// Load/select the private texture.
				GLuint texId = (*pi)->OwnTexture(*ari.re);
				if(!texId)
					continue;
				m_OwnTextures.insert(pi.key(), texId);

				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnDecalOpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnDecalTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->HasOwnTexture())
			{
				// Own VAOs.
				// Load/select the private texture.
				GLuint texId = (*pi)->OwnTexture(*ari.re);
				if(!texId)
					continue;
				m_OwnTextures.insert(pi.key(), texId);

				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnOpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->IsDecal())
			{
				// Decal VAOs.
				(*pi)->RenderTriangles(decalOpaqueTris, decalTransparentTris, themeColors);
				decalPartId = pi.key();
			}
			else if((*pi)->IsTurboLiftMesh())
			{
				// Turbo lift VAOs.
				m_RoomNames.insert(pi.key(), (*pi)->GroupName());
				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);

				// Remove the stop matrix from the faces. They will be repositioned later, using the matrix for their current stop.
				const AuMatrix4x4& stopM = (*pi)->StopMatrixAt((*pi)->AtStop());
				opaqueTris.Transform(stopM.Inverse());
				transparentTris.Transform(stopM.Inverse());

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_LiftOpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_LiftTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->IsRoomMesh())
			{
				// Room mesh.
				m_RoomNames.insert(pi.key(), (*pi)->GroupName());
				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);

				// Collect faces from generic meshes associated with them, so they are lit the same as the room.
				for(AuScParts::const_iterator mi = parts.cbegin(); mi != parts.cend(); mi++)
				{
					if((*mi)->IsOmitted())
						continue;
					if((*mi)->Type() != AuScPart::Part_Mesh)
						continue;
					if((*mi)->GroupName() != (*pi)->GroupName())
						continue;
					(*mi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
				}

				if((*pi)->InteriorIsSunlit())
				{
					m_SunlitRooms.insert((*pi)->GroupName());
					if(!opaqueTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_RoomOpaqueVAOs.insert(pi.key(), vao);
					}
					if(!transparentTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_RoomTransparentVAOs.insert(pi.key(), vao);
					}
				}
				else
				{
					if(!opaqueTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_IntRoomOpaqueVAOs.insert(pi.key(), vao);
					}
					if(!transparentTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_IntRoomTransparentVAOs.insert(pi.key(), vao);
					}
				}
			}
			else
			{
				// Routine VAOs.
				hullPartId = pi.key();
				(*pi)->RenderTriangles(hullOpaqueTris, hullTransparentTris, themeColors);
			}
		}
		UnlockDesign();
		m_VAOsCreated = true;

		// Create one VAO for the beacon lenses.
		if(!beaconOpaqueTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, beaconOpaqueTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_LensBeaconOpaqueVAOs.insert(beaconPartId, vao);
		}
		if(!beaconTransparentTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, beaconTransparentTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_LensBeaconTransparentVAOs.insert(beaconPartId, vao);
		}

		// Create one VAO for decals that do not have their own texture.
		if(!decalOpaqueTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, decalOpaqueTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_DecalOpaqueVAOs.insert(decalPartId, vao);
		}
		if(!decalTransparentTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, decalTransparentTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_DecalTransparentVAOs.insert(decalPartId, vao);
		}

		// Create one VAO for the headlight lenses.
		if(!headlightOpaqueTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, headlightOpaqueTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_LensHeadlightOpaqueVAOs.insert(headlightPartId, vao);
		}
		if(!headlightTransparentTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, headlightTransparentTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_LensHeadlightTransparentVAOs.insert(headlightPartId, vao);
		}

		// Create one VAO for the hull.
		if(!hullOpaqueTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, hullOpaqueTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_OpaqueVAOs.insert(hullPartId, vao);
		}
		if(!hullTransparentTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, hullTransparentTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_TransparentVAOs.insert(hullPartId, vao);
		}

		// Create one VAO for the strobe lenses.
		if(!strobeOpaqueTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, strobeOpaqueTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_LensStrobeOpaqueVAOs.insert(strobePartId, vao);
		}
		if(!strobeTransparentTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, strobeTransparentTris, GL_STATIC_DRAW);
			if(vao.VAO())
				m_LensStrobeTransparentVAOs.insert(strobePartId, vao);
		}

		// Configure dummy light states for fake facade windows.
		for(QHash<quint16, QString>::const_iterator wi = m_WindowRooms.cbegin(); wi != m_WindowRooms.cend(); wi++)
		{
			// Look for a room with that name.
			bool foundRoom = false;
			for(QHash<quint16, QString>::const_iterator ri = m_RoomNames.cbegin(); ri != m_RoomNames.cend(); ri++)
				if(*ri == *wi)
				{
					foundRoom = true;
					break;
				}

			// If no room corresponds to the window name, it is a fake facade window that needs a light state.
			if(!foundRoom)
			{
				// Determine lighting state of the window.
				// Choose a random light setting in a distribution similar to the animator.
				INTERIOR_LIGHT light;
				quint32 roll = g_Au.Dice().Roll(1,100);
				if(roll <= 60)
					light = INTLIGHT_Bright;			// Bright 60% of the time.
				else if(roll <= 75)
					light = INTLIGHT_Dim;			// Dim 15% of the time.
				else
					light = INTLIGHT_Dark;			// Dark 25% of the time.
				m_WindowLightStates.insert(*wi, light);
			}
		}

		// Tally the various things we animate.
		m_AlertStatus = 0;
		m_BerthNameDecals = 0;
		m_CaptNameDecals = 0;
		m_EmpireFlagDecals = 0;
		m_EmpireNameDecals = 0;
		m_LifeSupportStatus = 0;
		m_ShipNameDecals = 0;
		m_WorldMaps = 0;
		m_DesignLock.lockForRead();
		for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
				di != m_Design.StatusDisplays().cend(); di++)
		{
			if((*di).PartType() == AuScPart::Part_StatusAlert)
				m_AlertStatus++;
			else if((*di).PartType() == AuScPart::Part_BerthName)
				m_BerthNameDecals++;
			else if((*di).PartType() == AuScPart::Part_CaptName)
				m_CaptNameDecals++;
			else if((*di).PartType() == AuScPart::Part_EmpireFlag)
				m_EmpireFlagDecals++;
			else if((*di).PartType() == AuScPart::Part_EmpireName)
				m_EmpireNameDecals++;
			else if((*di).PartType() == AuScPart::Part_StatusLifeSupport)
				m_LifeSupportStatus++;
			else if((*di).PartType() == AuScPart::Part_ShipName)
				m_ShipNameDecals++;
			else if((*di).PartType() == AuScPart::Part_StatusWorldMap)
				m_WorldMaps++;
		}
		m_DesignLock.unlock();
	}

	m_DesignLock.lockForRead();
	// Door states can change.
	if(!m_DoorRenderState.empty())
	{
		// The design has door states.
		// Insure we have the VAOs for the current states of all doors.
		for(QHash<quint16, quint8>::const_iterator si = m_DoorRenderState.cbegin(); si != m_DoorRenderState.cend(); si++)
		{
			quint8 state = *si;
			quint32 stateKey = StateVAOKey(si.key(), state);
			if(m_DoorVAOsMade.contains(stateKey))
				continue;

			const AuScDesign* scd = LockDesign();
			if(!scd)
			{
				// Design is not available locally.
				UnlockDesign();
				//qDebug() << "DaSc::CreateDeviceObjects" << Id() << Name() << "design" << designId << "not available.";
				continue;
			}

			// We have a door state that we have not yet tried to make a VAO for.
			m_DoorVAOsMade.insert(stateKey);	// In case the door state makes nothing.

			// Make a VAO of the door part state.
			const AuScParts& parts = scd->Parts();
			AuScParts::const_iterator pi = parts.find(si.key());
			if(pi == parts.cend())
			{
				// Door not found?
				UnlockDesign();
				continue;
			}

			// Get the name of the room void associated with the open state of the door.
			QString roomName;
			QHash<quint16, QString>::const_iterator dri = m_DoorRooms.find(si.key());
			if(dri != m_DoorRooms.cend())
				roomName = *dri;

			// Get the opaque and transparent render triangles of the door state.
			AuScTriRenderList opaqueTris, transparentTris;
			(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, state);
			if(opaqueTris.empty() && transparentTris.empty())
			{
				if(state == AuScDoor::LockedOpen || state == AuScDoor::StuckOpen)
					(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, AuScDoor::Open);
				else if(state == AuScDoor::LockedClosed || state == AuScDoor::StuckClosed)
					(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, AuScDoor::Closed);
			}
			if(m_SunlitRooms.contains(roomName))
			{
				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
					{
						quint32 stateKey = StateVAOKey(si.key(), state);
						m_DoorOpaqueVAOs.insert(stateKey, vao);
					}
				}
				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
					{
						quint32 stateKey = StateVAOKey(si.key(), state);
						m_DoorTransparentVAOs.insert(stateKey, vao);
					}
				}
			}
			else
			{
				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
					{
						quint32 stateKey = StateVAOKey(si.key(), state);
						m_IntDoorOpaqueVAOs.insert(stateKey, vao);
					}
				}
				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
					{
						quint32 stateKey = StateVAOKey(si.key(), state);
						m_IntDoorTransparentVAOs.insert(stateKey, vao);
					}
				}
			}
			UnlockDesign();
		}
	}

	// Landing gear states can change.
	if(!m_LandingGearRenderState.empty())
	{
		// The design has landing gear states.
		// Insure we have the VAOs for the current states of all landing gear.
		for(QHash<quint16, quint8>::const_iterator si = m_LandingGearRenderState.cbegin(); si != m_LandingGearRenderState.cend();
				si++)
		{
			quint32 stateKey = StateVAOKey(si.key(), *si);
			if(m_LandingGearVAOsMade.contains(stateKey))
				continue;

			const AuScDesign* scd = LockDesign();
			if(!scd)
			{
				// Design is not available locally.
				UnlockDesign();
				//qDebug() << "DaSc::CreateDeviceObjects" << Id() << Name() << "design" << designId << "not available.";
				continue;
			}

			// We have a landing gear state that we have not yet tried to make a VAO for.
			m_LandingGearVAOsMade.insert(stateKey);	// In case the landing gear state makes nothing.

			// Make a VAO of the landing gear part state.
			const AuScParts& parts = scd->Parts();
			AuScParts::const_iterator pi = parts.find(si.key());
			if(pi == parts.cend())
			{
				// Landing gear not found?
				UnlockDesign();
				continue;
			}

			// Get the opaque and transparent render triangles of the landing gear state.
			AuScTriRenderList opaqueTris, transparentTris;
			(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, *si);
			if(opaqueTris.empty() && transparentTris.empty())
			{
				if(*si == AuScLandingGear::StuckExtended)
					(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, AuScLandingGear::Extended);
				else if(*si == AuScLandingGear::StuckRetracted)
					(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, AuScLandingGear::Retracted);
			}
			if(!opaqueTris.empty())
			{
				AuScVAO vao;
				vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
				if(vao.VAO())
				{
					quint32 stateKey = StateVAOKey(si.key(), *si);
					m_LandingGearOpaqueVAOs.insert(stateKey, vao);
				}
			}
			if(!transparentTris.empty())
			{
				AuScVAO vao;
				vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
				if(vao.VAO())
				{
					quint32 stateKey = StateVAOKey(si.key(), *si);
					m_LandingGearTransparentVAOs.insert(stateKey, vao);
				}
			}
			UnlockDesign();
		}
	}
	m_DesignLock.unlock();

	// Turret gun states can change.
	m_TurretPosLock.lockForRead();
	if(!m_TurretPos.empty())
	{
		// The design has turrets.
		// Base and gun parts are separated so they can be articulated.
		// Turrets use regular textures for the base and gun model parts, own texture for the screen.
		bool makeBasesAndScreens = m_TurBaseOpaqueVAOs.empty() && m_TurBaseTransparentVAOs.empty();

		// Insure we have the VAOs for the turret bases and current states of all turret guns.
		for(AuTurretStateHash::const_iterator si = m_TurretPos.cbegin(); si != m_TurretPos.cend(); si++)
		{
			if(makeBasesAndScreens)
			{
				const AuScDesign* scd = LockDesign();
				if(!scd)
				{
					// Design is not available locally.
					UnlockDesign();
					continue;
				}

				// Design is available locally.
				const AuScParts& parts = scd->Parts();
				AuScParts::const_iterator pi = parts.find(si.key());
				if(pi != parts.cend())
				{
					// Turret Base
					AuScTriRenderList opaqueTris, transparentTris;
					(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, AuScTurret::Base);

					if(!opaqueTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_TurBaseOpaqueVAOs.insert(pi.key(), vao);
					}

					if(!transparentTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_TurBaseTransparentVAOs.insert(pi.key(), vao);
					}

					// Turret Gunner Screen uses it's own custom texture.
					{
						// Load/select the private texture.
						GLuint texId = (*pi)->OwnTexture(*ari.re);
						if(!texId)
						{
							UnlockDesign();
							continue;
						}
						m_OwnTextures.insert(pi.key(), texId);

						AuScTriRenderList opaqueTris, transparentTris;
						(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, AuScTurret::GunnerScreen);

						if(!opaqueTris.empty())
						{
							AuScVAO vao;
							vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
							if(vao.VAO())
								m_OwnOpaqueVAOs.insert(pi.key(), vao);
						}

						if(!transparentTris.empty())
						{
							AuScVAO vao;
							vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
							if(vao.VAO())
								m_OwnTransparentVAOs.insert(pi.key(), vao);
						}
					}
				}
				UnlockDesign();
			}

			// Turret Gun
			quint32 stateKey = StateVAOKey(si.key(), (*si).State());
			if(!m_TurGunVAOsMade.contains(stateKey))
			{
				const AuScDesign* scd = LockDesign();
				if(!scd)
				{
					// Design is not available locally.
					UnlockDesign();
					continue;
				}

				// We have a gun state that we have not yet tried to make a VAO for.
				m_TurGunVAOsMade.insert(stateKey);	// In case the turret state makes nothing.

				// Design is available locally.
				const AuScParts& parts = scd->Parts();
				AuScParts::const_iterator pi = parts.find(si.key());
				if(pi != parts.cend())
				{
					// Get the opaque and transparent render triangles of the gun state.
					AuScTriRenderList opaqueTris, transparentTris;
					(*pi)->RenderTrianglesState(opaqueTris, transparentTris, themeColors, (*si).State());

					if(!opaqueTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
						if(vao.VAO())
						{
							quint32 stateKey = StateVAOKey(si.key(), (*si).State());
							m_TurGunOpaqueVAOs.insert(stateKey, vao);
						}
					}

					if(!transparentTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
						if(vao.VAO())
						{
							quint32 stateKey = StateVAOKey(si.key(), (*si).State());
							m_TurGunTransparentVAOs.insert(stateKey, vao);
						}
					}
				}
				UnlockDesign();
			}
		}
	}
	m_TurretPosLock.unlock();

	// OpenAL audio sources
	if(!m_AudioShip[0] && ari.al)
	{
		alGenSources(AS_MaxSources, m_AudioShip);

		alSourcei(m_AudioShip[AS_GravityDriveExt], AL_BUFFER, ari.al->AudioBuffer(SOUND_GravityDriveExt));
		alSourcei(m_AudioShip[AS_GravityDriveInt], AL_BUFFER, ari.al->AudioBuffer(SOUND_GravityDriveInt));
		alSourcei(m_AudioShip[AS_PowerPlant], AL_BUFFER, ari.al->AudioBuffer(SOUND_PowerPlant));
		alSourcei(m_AudioShip[AS_Sensors], AL_BUFFER, ari.al->AudioBuffer(SOUND_Telemetry));
		alSourcei(m_AudioShip[AS_TractorBeam], AL_BUFFER, ari.al->AudioBuffer(SOUND_TractorBeam));
	}

	// Create engine contrail particle generators for spacecraft that are not stations.
	if(!m_ContrailsValid && IsDesignDataCacheValid())
	{
		// Get the contrail emitter locations and their sizes.
		m_ContrailsValid = true;	// Do this only once, even if the design has no contrail emitters.
		QMap<AuPoint, float> contrails;
		const AuScDesign* scd = LockDesign();
		if(scd)
			scd->ContrailEmitters(contrails);
		UnlockDesign();
		if(!contrails.empty())
		{
			// Get the contrail color of the empire.
			const DgData& gd = g_Au.LockDataForRead();
			m_ContrailColor = gd.EmpireContrailColor(EmpireId());
			g_Au.UnlockData();

			// Get the textures we use for contrails.
			GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
			GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

			// Create the emitters.
			for(QMap<AuPoint, float>::const_iterator ci = contrails.cbegin(); ci != contrails.cend(); ci++)
			{
				PsEngineTrail* con = new PsEngineTrail;
				con->SetMaxParticles(35);
				con->SetLifeCycle(1.0f);
				con->SetReleaseInterval(0.0f);
				con->SetColor(m_ContrailColor);
				con->SetPosition(ci.key());							// Position of all nu particles made by this emitter.
				con->SetTextureID(texIdStreak);
				con->SetTextureID2(texIdGlow);
				con->SetSize(*ci * 450);
				m_Contrails.append(con);
			}
		}
	}
}

unsigned int DaSc::CrewBerths() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.CrewBerths();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::CrewDoorLocked(unsigned int doorIndex) const
{
	m_DesignLock.lockForRead();
	bool is = m_DoorStates.IsDoorLocked(doorIndex) && !m_DoorStates.IsHullDoor(doorIndex);
	m_DesignLock.unlock();
	return is;
}

void DaSc::CycleCapacitor()
{
	const float capMax = CapacitorMax();

	// Remember current power state.
	float capLevel = CapacitorLevel();
	float capIn = capLevel;
	bool hadPower = (capLevel > 0) ? true : false;

	// Fuel burned is modified by fuel quality. At quality 128, its 1:1. Good fuel can halve the fuel burn.
	const float rFuelEfficiency = QualityBoostF(m_FuelQuality);

	// Generate power by burning fuel with all functioning power plants.
	if(capIn > capMax)
		capLevel = capMax;		// Trim capacitor level to nu capacitor max, which may have changed due to damage.

	if(PowerPlantWorking())
	{
		// Power plants optimally produce nUnits of power per real world hour. Calculate max output per second.
		m_DesignLock.lockForRead();
		float rOutput = m_Design.FuelBurnRate();
		m_DesignLock.unlock();

		// Fuel burned is modified by fuel quality.
		float rFuelBurn = rOutput / rFuelEfficiency;

		// Limited by fuel quantity
		float fuel = FuelQuantity();
		if(rFuelBurn > fuel)
		{
			rFuelBurn = fuel;
			rOutput = rFuelBurn * rFuelEfficiency;
		}

		// Convert output from fuel units to power units.
		// Power plant produces 10 power units for each fuel unit burned.
		float powerOutput = rOutput * AuScReport::PowerPerFuel();

		// Limited by capacitor.
		if(capLevel + powerOutput > capMax)
		{
			powerOutput = capMax - capLevel;
			rOutput = powerOutput / AuScReport::PowerPerFuel();
			rFuelBurn = rOutput / rFuelEfficiency;
		}

		// Take the fuel.
		m_VesselLock.lockForWrite();
		if(m_fFuelQuantity > rFuelBurn)
			m_fFuelQuantity -= rFuelBurn;
		else
			m_fFuelQuantity = 0;
		m_VesselLock.unlock();

		// Decay generated output performance according to skew.
		const float rlongest = 16.f * 4.f;	// Worst case skew is -25% output
		float rdecay = abs(m_chPowerPlantSkew)/rlongest;
		float powerGen = powerOutput * (1.f - rdecay);
		capLevel += powerGen;

		// Update power plant skew values.
		if(m_byPowerPlantSkewDur == 0)
			PowerPlantSkewWander();
		else
			m_byPowerPlantSkewDur--;
	}

	float powerConsumed = 0;

	// Consume power for life support. Life support cannot be turned off.
	// It always gets first draw from the available power.
	// It also continues working even if there is no power.
	if(capLevel > 0 && LifeSupportWorking())
	{
		// Life support system is functioning.
		// Life support consumes power from capacitor.
		m_DesignLock.lockForRead();
		float rPowerBurn = m_Design.LifeSupportPowerRate();
		m_DesignLock.unlock();
		if(rPowerBurn > capLevel)
			rPowerBurn = capLevel;
		powerConsumed += rPowerBurn;
		capLevel -= rPowerBurn;

		BalanceRoomAirPressures(true);
	}
	else if(AnyHullDoorOpen())
	{
		// Life support is off and a door is open.
		// Equalize air pressure with outside quickly.
		BalanceRoomAirPressures(false);
	}

	// Consume power for idling FTL drives.
	if(HasFTLDrive())
	{
		m_FTLLock.lockForRead();
		for(QHash<quint16, AuFTLDriveState>::const_iterator fi = m_FTLDrives.cbegin();
				capLevel > 0 && fi != m_FTLDrives.cend(); fi++)
		{
			// FTL drives get second draw from the power. They "need good power" and they can be turned off.
			FTL_DRIVE_STATE fs = (*fi).Power();
			if(fs != FTL_DRIVE_On && fs != FTL_DRIVE_Warming)
				continue;

			// FTL drives consume power from capacitor while on.
			m_DesignLock.lockForRead();
			float rPowerBurn = m_Design.FTLDrivePowerRate(fi.key());
			m_DesignLock.unlock();
			if(rPowerBurn > capLevel)
				rPowerBurn = capLevel;
			powerConsumed += rPowerBurn;
			capLevel -= rPowerBurn;
		}
		m_FTLLock.unlock();
	}

	// Consume power for gravity drives and update skew values.
	if(GravityDriveWorking())
	{
		// Gravity drives consume power from capacitor while on or parked.
		COMMODITYID maneuverDriveMod = SystemModule(SYS_ManeuverDrive);
		if(maneuverDriveMod == CID_RocketDriveModule)
		{
			// Maneuver drive module burns hydrogen directly.
			// Fuel burned is modified by fuel quality. At quality 128, its 1:1. Good fuel can halve the fuel burn.
			m_DesignLock.lockForRead();
			float rFuelBurn = m_Design.ManeuverFuelRate() / rFuelEfficiency;
			m_DesignLock.unlock();

			// Limited to fuel quantity.
			m_VesselLock.lockForWrite();
			if(rFuelBurn > m_fFuelQuantity)
				rFuelBurn = m_fFuelQuantity;
			m_fFuelQuantity -= rFuelBurn;
			m_VesselLock.unlock();
		}
		else
		{
			// Maneuver drive module uses power from the capacitor.
			m_DesignLock.lockForRead();
			float rPowerBurn = m_Design.ManeuverPowerRate();
			m_DesignLock.unlock();
			if(GravityDriveState() == GD_Hold && IsAtRest())
				rPowerBurn /= 10.f;	// Use 10% of the power when "parked"

			// Limited to capacitor level.
			if(rPowerBurn > capLevel)
				rPowerBurn = capLevel;
			powerConsumed += rPowerBurn;
			capLevel -= rPowerBurn;
		}

		// Update acceleration skew values.
		if(m_byAccelSkewDur == 0)
		{
			AccelSkewWander();
		}
		else
		{
			m_byAccelSkewDur--;
		}

		// Update turn rate skew values.
		if(m_byTurnRateSkewDur == 0)
		{
			TurnRateSkewWander();
		}
		else
		{
			m_byTurnRateSkewDur--;
		}
	}
	else if(!IsOnGround() && !IsMoored())
	{
		if(IsAtRest())
		{
			SetInMotion();	// Don't let ships hold their position when the maneuver drive is not working.
			TickTock();
		}
	}

	// Check for tractor beams.
	m_BayLock.lockForRead();
	for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
		if((*lfi).second.IsTractorAttached())
		{
			// Consume tractor beam power from capacitor.
			float rPowerBurn = (float)(*lfi).second.m_UnitsFired * UDaysPerRealDay() / (24.f * 3600.f);
			if(rPowerBurn > capLevel)
				rPowerBurn = capLevel;
			powerConsumed += rPowerBurn;
			capLevel -= rPowerBurn;
		}
	m_BayLock.unlock();

	// Shields consume power.
	if(ShieldsWorkingAndOn())
	{
		//if(Is(1081605148))
			//qDebug() << "DaSc::CycleCapacitor" << Id() << Name() << "shieldState" << ShieldState() << "capLevel" << capLevel;
		// Consume shield maintenance power from capacitor.
		if(ShieldState() == DaVessel::SS_Up)
		{
			m_DesignLock.lockForRead();
			float rPowerBurn = m_Design.ShieldPowerRate();
			m_DesignLock.unlock();
			if(rPowerBurn > capLevel)
				rPowerBurn = capLevel;
			powerConsumed += rPowerBurn;
			capLevel -= rPowerBurn;
			//if(Is(1081605148))
				//qDebug() << " maintenance power" << rPowerBurn;
		}

		// Consume shield recharge power from the capacitor.
		if(capLevel > 1)
		{
			// Shields charge faster when they are down.
			double powerMax = ShieldUnitsWorking();
			double chargeAmount = powerMax / (60 * 10);	// Aim for a charge time of 10 min when down, at most.
			if(chargeAmount < 60)
				chargeAmount = 60;						// So charge rate is 1 unit per second, per side, when up.
			if(ShieldState() == SS_Up)
				chargeAmount /= 10;						// Shields charge slower when up.

			// Limit the charge amount to the amount in the capacitor, leaving 1.
			if(chargeAmount > capLevel - 1)
				chargeAmount = capLevel - 1;
			if(chargeAmount > 0)
			{
				double charged = ShieldCharge(chargeAmount);
				powerConsumed += charged;
				capLevel -= charged;
				//if(Is(1081605148))
					//qDebug() << " shield charged" << charged;
			}
		}
	}

	// Sensors consume power.
	if(SensorsWorking())
	{
		// Consume power from capacitor.
		m_DesignLock.lockForRead();
		float rPowerBurn = m_Design.SensorPowerRate();
		m_DesignLock.unlock();
		if(rPowerBurn > capLevel)
			rPowerBurn = capLevel;
		powerConsumed += rPowerBurn;
		capLevel -= rPowerBurn;
	}

	// Notice trend for engineer console.
	m_VesselLock.lockForWrite();
	m_CapacitorLevel = capLevel;
	if(capLevel < capIn)
		m_CapacitorTrend = -1;
	else if (capLevel > capIn)
		m_CapacitorTrend = 1;
	else
		m_CapacitorTrend = 0;
	m_VesselLock.unlock();

	// Notice change in power state so clients will receive updates that
	// cause lights and engines to change appearance. This also enables consoles
	// and turrets to respond on the client.
	bool hasPower = (capLevel > 0) ? true : false;
	if(hadPower != hasPower)
		SetModified();
}

void DaSc::CycleFTLDrive()
{
	quint16 warpDrive = FTLWarpDrive();	// Warp drive if any.
	quint16 canReengageWarp;			// Warp drive that can be reengaged.
	if(warpDrive && CanReengageFTLNow(warpDrive))
		canReengageWarp = warpDrive;
	else
		canReengageWarp = 0;

	quint16 reengageWarp = 0;			// Set to which warp drive needs to be reengaged, if any.
	bool detachRiders = false;
	m_FTLLock.lockForWrite();
	for(AuFTLDriveStateHash::iterator fi = m_FTLDrives.begin(); fi != m_FTLDrives.end(); fi++)
	{
		FTL_COMMIT commit = (*fi).Commit();
		FTL_COMMIT nuCommit = commit;
		FTL_DRIVE_STATE state = (*fi).Power();
		FTL_DRIVE_STATE nuState = state;

		switch(nuState)
		{
		case FTL_DRIVE_Off:
		case FTL_DRIVE_Invalid:
		case FTL_DRIVE_New:
		default:
			// FTL drive is off and cold. Power light is off on engineer station.
			nuCommit = FTL_COMMIT_Dormant;
			break;

		case FTL_DRIVE_Warming:
			// Drive is warming up but not ready. Power light is blinking yellow on engineer station.
			nuCommit = (FTL_COMMIT)(nuCommit+1);				// Drive is warming.
			if(nuCommit >= FTL_COMMIT_Warmed)
			{
				nuState = FTL_DRIVE_On;							// Drive is warmed up.
				nuCommit = FTL_COMMIT_Dormant;					// Cycle is ready to be engaged.
			}
			break;

		case FTL_DRIVE_On:
			// Drive is fully on. Power light is green on engineer station.
			if(nuCommit != FTL_COMMIT_Dormant)
			{
				// Drive is engaged.
				nuCommit = (FTL_COMMIT)(nuCommit+1);
				if(nuCommit == FTL_COMMIT_ChargedBegin)
				{
					// Drive is now hot.
					if(fi.key() == warpDrive)
						detachRiders = true;	// When warp drive kicks in, lose any chaff clinging to the hull.
				}
				else if(nuCommit == FTL_COMMIT_ChargedEnd)
				{
					// Drive is about to go cold.
					if(fi.key() == canReengageWarp)
					{
						reengageWarp = fi.key();	// Warp drive keeps going as long as the capacitor has the power to keep firing.
						continue;								// Skip changing commit or power settings on this FTL drive.
					}
				}
				else if(nuCommit == FTL_COMMIT_Discharge1)
				{
					// Drive is now cold.
					if(fi.key() == warpDrive)
						ForceVector(AuPoint(0,0));	// Stop the ship when dropping out of warp speed.
				}
				else if(nuCommit >= FTL_COMMIT_Discharged)
					nuCommit = FTL_COMMIT_Dormant;				// Drive is finished with its commit cycle.
			}
			break;
		}

		if(state != nuState || commit != nuCommit)
		{
			(*fi).SetCommit(nuCommit);
			(*fi).SetPower(nuState);
			m_FTLChg.Set();
		}
	}
	m_FTLLock.unlock();

	if(detachRiders)
	{
		DetachRiders();
		ForceMotion();
	}

	if(reengageWarp)
		ReengageFTLDrive(reengageWarp);
}

void DaSc::DamageAndDestroy(const QString& reason)
{
	// Pirates can be removed quietly if they are not in any scene and if no person is aboard, whether or not they are on line.
	if(!IsInAnyScene() && IsPirateWithFreeWill() && !FindPersonRecursive())
	{
		//qDebug() << "DaSc::DamageAndDestroy" << Id() << Name() << "Quietly Kill'd.";
		// Reset last pirate station spawn time in the system when a pirate station is destroyed to hold them off for a while.
		//if(IsSpaceStation())
			//FindParentSolarSystem()->SetLastPirateStation();
		Kill();
		return;
	}

	bool goDormant = HasHumanOfficer();
	if(goDormant)
	{
		// Inflict a bunch of damage since this ship can be recovered.
		// Induce damage until the ship is destroyed.
		unsigned int totalHits = DesignHitsTotal();
		unsigned int dmgHits = totalHits / 100;
		if(dmgHits < 100)
			dmgHits = 100;

		int finiteLoop = 100;
		AuSysId sysId;
		do
		{
			// Randomly select a system.
			if(g_Au.Dice().Roll(1,2,-1))
				sysId = SelectStructuralSystemToDamage();
			else
				sysId = SelectNonstructuralSystemToDamage();
			COMMODITYID sysModule = SystemModule(sysId);
			unsigned int hitPoints = g_Au.Dice().Roll(1,dmgHits);
			m_DamageLock.lockForWrite();
			m_Damage.AddDamage(sysId, sysModule, hitPoints);
			m_DamageLock.unlock();
		}
		while(m_Damage.HitsDamage() < totalHits && finiteLoop--);
		DamageStateChanged();
	}

	// This ship is destroyed.
	DestroyShip(reason);
}

void DaSc::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaVessel::DeleteDeviceObjects(ari);

	m_AlertGreen = 0;
	m_AlertRedBright = 0;
	m_AlertRedDim = 0;
	m_AlertYellowBright = 0;
	m_AlertYellowDim = 0;
	m_LifeSupportStatusTex = 0;
	m_LightOmniTex = 0;
	m_LightSpotTex = 0;
	m_MissionStatusTex = 0;
	m_VAOsCreated = false;
	m_WorldMapTex = 0;

	m_CmdEmpty.DeleteDeviceObjects(*ari.re);
	m_EngineEmpty.DeleteDeviceObjects(*ari.re);
	m_FireEmpty.DeleteDeviceObjects(*ari.re);
	m_NavEmpty.DeleteDeviceObjects(*ari.re);
	m_PilotEmpty.DeleteDeviceObjects(*ari.re);
	m_PowerEmpty.DeleteDeviceObjects(*ari.re);
	m_SensorEmpty.DeleteDeviceObjects(*ari.re);
	m_ShieldEmpty.DeleteDeviceObjects(*ari.re);
	m_SurgeryEmpty.DeleteDeviceObjects(*ari.re);
	m_TransEmpty.DeleteDeviceObjects(*ari.re);

	for(QHash<quint16, AuScVAO>::iterator vi = m_OpaqueVAOs.begin(); vi != m_OpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_TransparentVAOs.begin(); vi != m_TransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_TransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnOpaqueVAOs.begin(); vi != m_OwnOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnOpaqueVAOs.clear();

	m_OwnTextures.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnTransparentVAOs.begin(); vi != m_OwnTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_DecalOpaqueVAOs.begin(); vi != m_DecalOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_DecalOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_DecalTransparentVAOs.begin(); vi != m_DecalTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_DecalTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnDecalOpaqueVAOs.begin(); vi != m_OwnDecalOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnDecalOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnDecalTransparentVAOs.begin(); vi != m_OwnDecalTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnDecalTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_IntRoomOpaqueVAOs.begin(); vi != m_IntRoomOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_IntRoomOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_IntRoomTransparentVAOs.begin(); vi != m_IntRoomTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_IntRoomTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_RoomOpaqueVAOs.begin(); vi != m_RoomOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_RoomOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_RoomTransparentVAOs.begin(); vi != m_RoomTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_RoomTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_WindowTranslucentVAOs.begin(); vi != m_WindowTranslucentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_WindowTranslucentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_WindowTransparentVAOs.begin(); vi != m_WindowTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_WindowTransparentVAOs.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_DoorOpaqueVAOs.begin(); vi != m_DoorOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_DoorOpaqueVAOs.clear();

	m_DoorRenderState.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_DoorTransparentVAOs.begin(); vi != m_DoorTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_DoorTransparentVAOs.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_IntDoorOpaqueVAOs.begin(); vi != m_IntDoorOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_IntDoorOpaqueVAOs.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_IntDoorTransparentVAOs.begin(); vi != m_IntDoorTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_IntDoorTransparentVAOs.clear();

	m_DoorVAOsMade.clear();

	m_LandingGearRenderState.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_LandingGearOpaqueVAOs.begin(); vi != m_LandingGearOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LandingGearOpaqueVAOs.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_LandingGearTransparentVAOs.begin(); vi != m_LandingGearTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LandingGearTransparentVAOs.clear();

	m_LandingGearVAOsMade.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LiftOpaqueVAOs.begin(); vi != m_LiftOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LiftOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LiftTransparentVAOs.begin(); vi != m_LiftTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LiftTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensBeaconOpaqueVAOs.begin(); vi != m_LensBeaconOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LensBeaconOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensBeaconTransparentVAOs.begin(); vi != m_LensBeaconTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LensBeaconTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensHeadlightOpaqueVAOs.begin(); vi != m_LensHeadlightOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LensHeadlightOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensHeadlightTransparentVAOs.begin(); vi != m_LensHeadlightTransparentVAOs.end();
			vi++)
		(*vi).DeleteGL();
	m_LensHeadlightTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensNavOpaqueVAOs.begin(); vi != m_LensNavOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LensNavOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensNavTransparentVAOs.begin(); vi != m_LensNavTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LensNavTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensStrobeOpaqueVAOs.begin(); vi != m_LensStrobeOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LensStrobeOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_LensStrobeTransparentVAOs.begin(); vi != m_LensStrobeTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_LensStrobeTransparentVAOs.clear();

	if(m_LightSpotVAO.VAO())
		m_LightSpotVAO.DeleteGL();

	for(QHash<quint16, AuScVAO>::iterator vi = m_TurBaseOpaqueVAOs.begin(); vi != m_TurBaseOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_TurBaseOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_TurBaseTransparentVAOs.begin(); vi != m_TurBaseTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_TurBaseTransparentVAOs.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_TurGunOpaqueVAOs.begin(); vi != m_TurGunOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_TurGunOpaqueVAOs.clear();

	for(QHash<quint32, AuScVAO>::iterator vi = m_TurGunTransparentVAOs.begin(); vi != m_TurGunTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_TurGunTransparentVAOs.clear();

	m_TurGunVAOsMade.clear();

	if(m_WeaponVAO.VAO())
		m_WeaponVAO.DeleteGL();

	// OpenAL audio sources
	for(int cs=0; cs<AS_MaxSources; cs++)
		ari.StopSound(m_AudioShip[cs]);
}

float DaSc::Density() const
{
	m_DesignLock.lockForRead();
	float v;
	if(m_Design.Streamline() == SLINE_Submersible)
		v = DensityOfWater();
	else
		v = m_Design.Density();
	m_DesignLock.unlock();
	return v;
}

QString DaSc::Description() const
{
	m_DesignLock.lockForRead();
	QString v(m_Design.DescLine());
	m_DesignLock.unlock();
	return v;
}

AuPoint DaSc::DesignCenterOffset() const
{
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	return (min + max)/2;
}

AuPoint DaSc::DesignCG() const
{
	return AuPoint(0,0);
}

unsigned int DaSc::DesignHitsStructural() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.StructureHits();
	m_DesignLock.unlock();
	return v;
}

unsigned int DaSc::DesignHitsTotal() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.TotalHits();
	m_DesignLock.unlock();
	return v;
}

AuId DaSc::DesignArchitectId() const
{
	m_DesignLock.lockForRead();
	AuId v = m_Design.ArchitectId();
	m_DesignLock.unlock();
	return v;
}

QString DaSc::DesignArchitectName() const
{
	m_DesignLock.lockForRead();
	QString v = m_Design.ArchitectName();
	m_DesignLock.unlock();
	return v;
}

quint32 DaSc::DesignId() const
{
	m_DesignLock.lockForRead();
	quint32 v = m_Design.DesignId();
	m_DesignLock.unlock();
	return v;
}

QString DaSc::DesignName() const
{
	m_DesignLock.lockForRead();
	QString v = m_Design.DesignName();
	m_DesignLock.unlock();
	return v;
}

QString DaSc::DesignReport(quint8 techLevelMinimum, bool showTips) const
{
	Q_UNUSED(techLevelMinimum);
	Q_UNUSED(showTips);
	m_DesignLock.lockForRead();
	QString v = m_Design.ReportAnalysis(AuDesign::Final);	// Only a Final design could be made into a real ship.
	m_DesignLock.unlock();
	return v;
}

void DaSc::DischargeFTLDrive(quint16 whichFTL)
{
	// Stop the ship when dropping out of warp speed.
	if(DbCommodity::IsWarpFTLDriveModule(SystemModule(SYS_FTLDrive, whichFTL)) && IsFTLDriveHot(whichFTL))
		ForceVector(AuPoint(0,0));

	if(FTLDriveCommitState(whichFTL) != FTL_COMMIT_Dormant)
	{
		m_FTLLock.lockForWrite();
		QHash<quint16, AuFTLDriveState>::iterator fi = m_FTLDrives.find(whichFTL);
		if(fi != m_FTLDrives.end())
		{
			(*fi).SetCommit(FTL_COMMIT_Discharge1);
			m_FTLChg.Set();
		}
		m_FTLLock.unlock();
		SetModified();
	}
}

int DaSc::EmptyBunkInBerth(const AuPoint& consoleLoc) const
{
	Q_UNUSED(consoleLoc);
	return 0;	// All berths in Sc designs have only one bunk.
}

void DaSc::EngageFTLDrive(quint16 whichFTL)
{
	if(!CanEngageFTLNow(whichFTL))
		return;

	// FTL drive is on and dormant.
	quint8 ftlSetting = FTLDriveSetting(whichFTL);
	float ftlPowerNeeded = PowerNeededToEngageFTLDrive(whichFTL, ftlSetting);
	DischargeCapacitor(ftlPowerNeeded);

	if(whichFTL == FTLWarpDrive())
	{
		// Accumulate warp drive engagements for warp signature detection.
		DaSolarSystem* system = FindParentSolarSystemInEnv();
		if(system)
			system->WarpEngaged(ftlSetting);
	}

	m_FTLLock.lockForWrite();
	QHash<quint16, AuFTLDriveState>::iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.end())
	{
		(*fi).SetCommit(FTL_COMMIT_Precharge1);		// Start the firing sequence.
		m_FTLChg.Set();
	}
	m_FTLLock.unlock();
	SetModified();
}

void DaSc::ReengageFTLDrive(quint16 whichFTL)
{
	// FTL drive is on and dormant.
	quint8 ftlSetting = FTLDriveSetting(whichFTL);
	float ftlPowerNeeded = PowerNeededToEngageFTLDrive(whichFTL, ftlSetting);
	DischargeCapacitor(ftlPowerNeeded);

	if(whichFTL == FTLWarpDrive())
	{
		// Accumulate warp drive engagements for warp signature detection.
		DaSolarSystem* system = FindParentSolarSystemInEnv();
		if(system)
			system->WarpEngaged(ftlSetting);
	}

	m_FTLLock.lockForWrite();
	QHash<quint16, AuFTLDriveState>::iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.end())
	{
		(*fi).SetCommit(FTL_COMMIT_ChargedBegin);		// Continue the charge sequence.
		m_FTLChg.Set();
	}
	m_FTLLock.unlock();
	SetModified();
}

AuPoint DaSc::EntranceLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround, bool hullDoorsUsable) const
{
	AuPoint entrance;
	quint16 doorId = 0;
	quint16 ladderId = 0;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	bool foundIt;
	if(scd)
		foundIt = scd->ClosestEntrance(entrance, doorId, ladderId, loc, onGround, hullDoorsUsable, m_DoorStates,
			m_LandingGearStates, m_Lifts);
	else
		foundIt = false;
	UnlockDesign();
	m_DesignLock.unlock();

	if(foundIt)
	{
		// When we found an entrance but there is no door id, the entrance is a hole. The index really doesn't matter.
		// Caller checks for open doors, just walks through holes.
		if(doorId)
		{
			doorIndex = doorId;
			holeIndex = -1;
		}
		else
		{
			// Ladder id could be 0 also. This happens when a room void is connected to the outside using a path segment and no
			// ladder connects to the exterior end of the path segment.
			// The bottom line is we infer 0 0 to mean a doorless opening without a ladder, a.k.a. a hole.
			doorIndex = -1;
			holeIndex = ladderId;
		}
	}
	else
	{
		doorIndex = -1;
		holeIndex = -1;
	}

	return entrance;
}

AuPoint DaSc::ExitLocation(const AuPoint& loc, int& doorIndex, int& holeIndex, bool onGround) const
{
	bool foundIt;
	AuPoint entrance;
	quint16 doorId = 0;
	quint16 ladderId = 0;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		foundIt = scd->ClosestEntrance(entrance, doorId, ladderId, loc, onGround, true, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		foundIt = false;
	UnlockDesign();
	m_DesignLock.unlock();
	if(foundIt)
	{
		// When we found an entrance but there is no door id, the entrance is a hole. The index really doesn't matter.
		// Caller checks for open doors, just walks through holes.
		if(doorId)
		{
			doorIndex = doorId;
			holeIndex = -1;
		}
		else
		{
			// Ladder id could be 0 also. This happens when a room void is connected to the outside using a path segment and no
			// ladder connects to the exterior end of the path segment.
			// The bottom line is we infer 0 0 to mean a doorless opening without a ladder, a.k.a. a hole.
			doorIndex = -1;
			holeIndex = ladderId;
		}
	}
	else
	{
		doorIndex = -1;
		holeIndex = -1;
	}

	return entrance;
}

void DaSc::EnumControlPanels(AuQuadArray& aControlPanels, const AuMatrix4x4& tModel, QVector<qint16>& idPanels,
		const DaAtom* sceneAtom) const
{
	// Calculate person's location relative to me.
	AuMatrix4x4 smat;
	GetRelativeTransform(*sceneAtom, smat);
	AuPoint sloc = smat.Translation();

	// Door switch quads, three intersecting.
	const float rad = 1.;										// radius of switch hot zone
	AuPoint xyll(-rad, -rad);
	AuPoint xyul(-rad, rad);
	AuPoint xyur(rad, rad);
	AuPoint xylr(rad, -rad);
	AuQuad xyquad(xyll, xyul, xyur, xylr);
	AuPoint xzll(-rad, 0, -rad);
	AuPoint xzul(-rad, 0, rad);
	AuPoint xzur(rad, 0, rad);
	AuPoint xzlr(rad, 0, -rad);
	AuQuad xzquad(xzll, xzul, xzur, xzlr);
	AuPoint yzll(0, -rad, -rad);
	AuPoint yzul(0, -rad, rad);
	AuPoint yzur(0, rad, rad);
	AuPoint yzlr(0, rad, -rad);
	AuQuad yzquad(yzll, yzul, yzur, yzlr);

	// Make control panels for door switches.
	bool sceneIn = sceneAtom->IsInSpacecraft();
	quint16 roomId = sceneAtom->ShipRoomId();
	m_DesignLock.lockForRead();
	for(AuDoorStateHash::const_iterator di = m_DoorStates.cbegin(); di != m_DoorStates.cend(); di++)
	{
		// Make three control quads at each switch location.
		// Operator switch 1;
		if((*di).OpLoc1().Distance(sloc) < FeetPerMeter() * 3)
		{
			AuQuad qxy(xyquad);
			qxy.Offset((*di).OpLoc1());
			qxy.Transform(tModel);
			aControlPanels.append(qxy);
			idPanels.append(di.key());

			AuQuad qxz(xzquad);
			qxz.Offset((*di).OpLoc1());
			qxz.Transform(tModel);
			aControlPanels.append(qxz);
			idPanels.append(di.key());

			AuQuad qyz(yzquad);
			qyz.Offset((*di).OpLoc1());
			qyz.Transform(tModel);
			aControlPanels.append(qyz);
			idPanels.append(di.key());
		}

		// Operator switch 2;
		if((*di).OpLoc2() != (*di).OpLoc1() && (*di).OpLoc2().Distance(sloc) < FeetPerMeter() * 3)
		{
			AuQuad qxy(xyquad);
			qxy.Offset((*di).OpLoc2());
			qxy.Transform(tModel);
			aControlPanels.append(qxy);
			idPanels.append(di.key());

			AuQuad qxz(xzquad);
			qxz.Offset((*di).OpLoc2());
			qxz.Transform(tModel);
			aControlPanels.append(qxz);
			idPanels.append(di.key());

			AuQuad qyz(yzquad);
			qyz.Offset((*di).OpLoc2());
			qyz.Transform(tModel);
			aControlPanels.append(qyz);
			idPanels.append(di.key());
		}
	}

	// Turbo lift.
	if(sceneIn && m_Design.TurboLifts().contains(roomId) && sceneAtom->FindParentVesselInEnv() == this)
	{
		// Scene atom is standing inside a lift.
		// Make a panel in front of them where they can't miss it.
		// These coordinates are relative to the viewer.
		double x2 = 2;				// left/right
		double y = 4;				// away from front of viewer
		double z2 = 2;				// up/down
		AuPoint xzll(-x2, y, -z2);
		AuPoint xzul(-x2, y, z2);
		AuPoint xzur(x2, y, z2);
		AuPoint xzlr(x2, y, -z2);
		AuQuad liftpanelquad(xzll, xzul, xzur, xzlr);
		aControlPanels.append(liftpanelquad);
		idPanels.append(roomId);

		// Put one in front of their face too, in case they have a big neck. Not sure which will be easiest to hit.
		AuPoint camOff = sceneAtom->CameraOffset();
		if(!camOff.IsZero())
		{
			liftpanelquad.Offset(camOff);
			aControlPanels.append(liftpanelquad);
			idPanels.append(roomId);
		}
	}
	m_DesignLock.unlock();
}

void DaSc::EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
{
	m_DesignLock.lockForRead();
	bool isLift = m_Lifts.contains(panelId);
	m_DesignLock.unlock();
	if(isLift)
	{
		GLuint texLift = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVLift", ":data/ScLiftButtons.png");
		controls.append(AuControlButton(tr("Lift"), Id(), panelId, CI_OperateTurboLift, texLift));
	}
	else
	{
		GLuint texDoor = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVDoor", ":data/ScDoorStd.png");
		controls.append(AuControlButton(tr("Door"), Id(), panelId, CI_OpenCloseDoor, texDoor));

		if(IsDoorLocked(panelId))
		{
			GLuint texLock = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVUnlock", ":data/AccessPanelLocked.png");
			controls.append(AuControlButton(tr("Unlock"), Id(), panelId, CI_LockUnlockDoor, texLock));
		}
		else
		{
			GLuint texLock = ari.re->CreateTextureFromBitmap(Id().GetID(), "EVLock", ":data/AccessPanel.png");
			controls.append(AuControlButton(tr("Lock"), Id(), panelId, CI_LockUnlockDoor, texLock));
		}
	}
}

void DaSc::FilterDestinationLoc(DaAtom* moveAtom, const AuPoint& oldLoc, AuPoint& nuLoc) const
{
	Q_UNUSED(moveAtom);
	Q_UNUSED(oldLoc);
	Q_UNUSED(nuLoc);
	/*
	if(!moveAtom->IsAboveOpenHatch() && nuLoc.z < oldLoc.z)
	{
		// Keep feet from dropping through the floor unless there is an open hatch there.
		// Atom is not above a hatch.
		// New location is lower than old location.
		double maxDrop = moveAtom->FallDistance();
		double drop = oldLoc.z - nuLoc.z;
		if(drop > maxDrop)
			nuLoc.z = oldLoc.z - maxDrop;
	}
	if(nuLoc.z > oldLoc.z && dynamic_cast<DaCreature*>(moveAtom) && moveAtom->IsInsideSpacecraft())
	{
		// Prevent creature's head from going up through the ceiling.
		// Calculate maximum distance the creature can move up.
		float maxClimb = moveAtom->CeilingDistance() - 1;
		float climb = nuLoc.z - oldLoc.z;	// Amount the creature is trying to move up.
		if(climb > maxClimb)
			nuLoc.z = oldLoc.z + maxClimb;	// Limit nu z.
	}
	*/
}

double DaSc::CeilingHt(const AuPoint& loc, const AuPoint& gravityN, bool isLocInside) const
{
	Q_UNUSED(isLocInside);
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	double ceilingHt;
	if(scd)
		ceilingHt = scd->HeightOfCeiling(loc, gravityN, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		ceilingHt = 0;
	UnlockDesign();
	m_DesignLock.unlock();
	return ceilingHt;
}

bool DaSc::FloorHt(const AuPoint& loc, const AuPoint& gravityN, double& htAbvFloor, bool& inGravityWell, bool& onStairs,
		AuPoint& stairNormal, bool isLocInside, const DaAtom* forWho) const
{
	onStairs = false;
	double floorHt;
	if(isLocInside)
	{
		m_DesignLock.lockForRead();
		const AuScDesign* scd = LockDesign();
		if(scd)
		{
			floorHt = scd->HeightAboveFloor(loc, gravityN, stairNormal, inGravityWell, m_DoorStates, m_LandingGearStates, m_Lifts);
			if(!stairNormal.IsZero())
			{
				onStairs = true;	// To orient vehicles properly to the stairNormal.
				htAbvFloor = floorHt;
			}
		}
		UnlockDesign();
		m_DesignLock.unlock();
	}
	else
	{
		// Checking floor height below someone who is not inside.
		double biggestShip = AuScDesign::HullSizeLimit();
		if(loc.z > biggestShip)
			return false;	// The biggest possible ship would not touch the top of this building at that height.

		// Get extents of my design.
		AuPoint min,max;
		m_DesignLock.lockForRead();
		m_Design.Extents(min, max);
		m_DesignLock.unlock();
		if(loc.z - max.z > biggestShip)
			return false;	// The loc is higher above this ship than the largest ship.

		// Locate obstacle under the ladder, such as the terrain, a vehicle or a spacecraft, including this one.
		// Go all the way to the environment atom, which is usually a solar system but sometimes it's a design environment or
		// other instance universe.
		// In the local scene, this will only include the visible atoms anyway.
		const DaAtom* envAtom = FindEnvironmentAtom();
		if(!envAtom)
			return false;	// No environment atom?

		// Ask the environment to find the nearest solid atom that is in front of my eye.
		// Use the reverse transformation back up to the environment atom.
		AuPoint sightedLoc(loc.x, loc.y, loc.z-biggestShip);
		unsigned int sightedPart = 0;
		bool sightedShield = false;
		DaAtom* obstacle = SightTarget(forWho, loc, AuMatrix4x4(), 0, sightedLoc, sightedPart, sightedShield, &stairNormal);
		if(obstacle && !sightedShield)
		{
			floorHt = loc.z - sightedLoc.z;
			if(!stairNormal.IsZero())
			{
				//qDebug() << "DaBd::FloorHt" << Id() << Name() << "stairN" << stairNormal;
				onStairs = true;	// To orient vehicles properly to the stairNormal.
				htAbvFloor = floorHt;
			}
		}
	}
	return onStairs;
}

AuPoint DaSc::FloorLocInsideDoor(const AuPoint& doorLoc) const
{
	double doorDistance;	// Return value from ClosestDoor.
	AuPoint floorLoc;
	m_DesignLock.lockForRead();
	quint16 door = m_DoorStates.ClosestDoor(doorLoc, doorDistance);
	if(door)
	{
		const AuScDesign* scd = LockDesign();
		if(scd)
			scd->PathLocInsideDoor(door, floorLoc, m_DoorStates, m_LandingGearStates, m_Lifts);
		UnlockDesign();
	}
	m_DesignLock.unlock();
	return floorLoc;
}

FTL_COMMIT DaSc::FTLDriveCommitState(quint16 whichFTL) const
{
	FTL_COMMIT c;
	m_FTLLock.lockForRead();
	QHash<quint16, AuFTLDriveState>::const_iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.cend())
		c = (*fi).Commit();
	else
		c = FTL_COMMIT_Invalid;
	m_FTLLock.unlock();
	return c;
}

quint8 DaSc::FTLDriveSetting(quint16 whichFTL) const
{
	quint8 c;
	m_FTLLock.lockForRead();
	QHash<quint16, AuFTLDriveState>::const_iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.cend())
		c = (*fi).Setting();
	else
		c = 0;
	m_FTLLock.unlock();
	return c;
}

FTL_DRIVE_STATE DaSc::FTLDriveState(quint16 whichFTL) const
{
	FTL_DRIVE_STATE c;
	m_FTLLock.lockForRead();
	QHash<quint16, AuFTLDriveState>::const_iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.cend())
		c = (*fi).Power();
	else
		c = FTL_DRIVE_Invalid;
	m_FTLLock.unlock();
	return c;
}

float DaSc::FTLDriveUnitsToEngage() const
{
	m_DesignLock.lockForRead();
	float u = m_Design.FTLDriveUnitsToEngage();
	m_DesignLock.unlock();
	return floorf(u);
}

quint16 DaSc::FTLWarpDrive() const
{
	m_DesignLock.lockForRead();
	quint16 v = m_Design.FTLWarpDrive();
	m_DesignLock.unlock();
	return v;
}

quint16 DaSc::FTLWormholeDrive() const
{
	m_DesignLock.lockForRead();
	quint16 v = m_Design.FTLWormholeDrive();
	m_DesignLock.unlock();
	return v;
}

quint32 DaSc::FuelCapacity() const
{
	m_DesignLock.lockForRead();
	quint32 f = m_Design.FuelCapacity();
	m_DesignLock.unlock();
	return f;
}

// Returns the force of gravity in ft/sec/sec projected by this atom on another atom that is 'cgDistance' feet away.
double DaSc::Gravity(double cgDistance) const
{
	// Ship interior gravity functions when the gravity drive is working or inside atmosphere range of a planet.
	if(IsInAtmosphere())
		return GravityStandard();

	// Life support module can provide gravity when it is working.
	if(LifeSupportWorking())
	{
		m_DesignLock.lockForRead();
		bool lsProvidesGrav = m_Design.LifeSupportModule() == CID_LifeSupportGravityModule;
		m_DesignLock.unlock();
		if(lsProvidesGrav)
			return GravityStandard();
	}

	// When aboard another spaceship, return is based on its gravity state.
	const DaCapsule* pcap = dynamic_cast<const DaCapsule*>(Parent());
	if(pcap)
		return pcap->Gravity(cgDistance);
	return 0;
}

float DaSc::GroundHeightAboveCG() const
{
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	float r;
	if(scd)
		r = scd->GroundHeightAboveCG(m_LandingGearStates);
	else
		r = 0;
	UnlockDesign();
	m_DesignLock.unlock();
	return r;
}

float DaSc::HarvesterBayRange(float unitsFired, quint16 which) const
{
	m_DesignLock.lockForRead();
	float r = m_Design.HarvesterBayRange(which, unitsFired);
	m_DesignLock.unlock();
	return r;
}

bool DaSc::HasFTLDrive() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasFTLDrive();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::HasManeuverDrive() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasManeuverDrive();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::HasPowerPlant() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasPowerPlant();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::HasSensors() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasSensor();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::HasShields() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasShield();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::HasWeaponBay() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasWeaponBay();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::HasWorkingSurgeryUnit() const
{
	m_DesignLock.lockForRead();
	QList<quint16> surgs = m_Design.SurgeryUnitIds();
	m_DesignLock.unlock();

	for(QList<quint16>::const_iterator sui = surgs.cbegin(); sui != surgs.cend(); sui++)
	{
		if(SystemModule(SYS_SurgeryUnit, *sui) == CID_SurgeryMedicalModule)
		{
			// Found a surgery unit. Is it working?
			if(GetPowerConsumption(SYS_SurgeryUnit, *sui))
				return true;	// The surgery unit is working.
		}
	}
	return false;
}

bool DaSc::HasWarpFTLDrive() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasWarpFTLDrive();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::HasWormholeFTLDrive() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasWormholeFTLDrive();
	m_DesignLock.unlock();
	return v;
}

AuPoint DaSc::HelmLocation() const
{
	AuPoint helmLoc;
	const AuScDesign* scd = LockDesign();
	if(scd)
		helmLoc = scd->HelmLocation();
	UnlockDesign();
	return helmLoc;
}

int DaSc::RemoteDoorIndex(bool forVehicle, const AuPoint& loc) const
{
	m_DesignLock.lockForRead();
	quint16 doorId = m_Design.ClosestRemoteDoor(forVehicle, loc, m_DoorStates);
	m_DesignLock.unlock();
	return doorId;
}

int DaSc::RemoteDoorIndex(bool forVehicle, const AuPoint& loc, quint16 roomId) const
{
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	quint16 doorId = scd->ClosestRemoteDoorInRoom(forVehicle, loc, roomId, m_DoorStates);
	UnlockDesign();
	m_DesignLock.unlock();
	return doorId;
}

bool DaSc::HullDoorLocked(unsigned int doorIndex) const
{
	m_DesignLock.lockForRead();
	bool is = m_DoorStates.IsDoorLocked(doorIndex) && m_DoorStates.IsHullDoor(doorIndex);
	m_DesignLock.unlock();
	return is;
}

float DaSc::HullStress() const
{
	// Certain tests have already been made.
	Q_ASSERT(IsInAtmosphere());

	m_DesignLock.lockForRead();
	float maxDrag = m_Design.MaxDrag();
	m_DesignLock.unlock();

	float hullStress = (float)Drag().Length2() / (float)(maxDrag*maxDrag);
	return hullStress;
}

void DaSc::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();
	AuPoint vVec = Vector();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Sc (id,idp,locX,locY,locZ,rotX,rotY,rotZ, timeCreate,timeModified,timeTock,timeDeath, "
			"name,allegiance,arenaTeam,conditions,damageHistory, "
			"state,accessPolicy,pilotUpPolicy,shieldPolicy,shieldFuelPolicy,atRest, vecX,vecY,vecZ, "
			"throttle,throttleLateral,throttleLong,throttleVertical, "
			"ownerId,ownerName,durEngine,skewEngine,fuel,fuelqual,rotSpeeds,actBits,capacitor,route,cargo,damage,roster, "
			"bank,bankSubsidyTax,systems, shieldFront,shieldBack,shieldLeft,shieldRight,shieldTop,shieldBottom, "
			"powerPlantShunts,techLevel,whereMade,mission,games,repairSecs,baySetting,ratdna,ratspec,monster, "
			"idDesign,switches,designRpt,airPressures,doorStates,ftlDrives,landingGear,landingSystems,launchers,lifts, "
			"roomLights,transporters,turretPos) "
			"VALUES (?,?,?,?,?,?,?,?, ?,?,?,?, "
			"?,?,?,?,?, "
			"?,?,?,?,?,?, ?,?,?, "
			"?,?,?,?, "
			"?,?,?,?,?,?,?,?,?,?,?,?,?, "
			"?,?,?, ?,?,?,?,?,?, "
			"?,?,?,?,?,?,?,?,?,?, "
			"?,?,?,?,?,?,?,?,?,?, "
			"?,?,?);");
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

	// DaVessel values.
	qa.addBindValue(AtomState());
	qa.addBindValue(m_AccessPolicy);
	qa.addBindValue(m_PilotUpPolicy);
	qa.addBindValue(m_ShieldPolicy);
	qa.addBindValue(m_ShieldFuelPolicy);
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(vVec.x));
	qa.addBindValue(FuzzyClamp(vVec.y));
	qa.addBindValue(FuzzyClamp(vVec.z));
	qa.addBindValue(m_Throttle);
	qa.addBindValue(m_ThrustLateral);
	qa.addBindValue(m_ThrustLong);
	qa.addBindValue(m_ThrustVertical);
	qa.addBindValue(m_OwnerId.GetID());
	qa.addBindValue(m_OwnerName);
	int durengine = (m_byAccelSkewDur<<16)|(m_byTurnRateSkewDur<<8)|m_byPowerPlantSkewDur;
	qa.addBindValue(durengine);
	int skewengine = (m_chAccelSkew<<24)|(m_chTurnRateSkewX<<16)|(m_chTurnRateSkewY<<8)|m_chPowerPlantSkew;
	qa.addBindValue(skewengine);
	qa.addBindValue(FuzzyClamp(m_fFuelQuantity));
	qa.addBindValue(m_FuelQuality);
	int rotspeeds = (m_byRotSpeedX<<16)|(m_byRotSpeedY<<8)|m_byRotSpeedZ;
	qa.addBindValue(rotspeeds);
	qa.addBindValue(m_wActions);
	qa.addBindValue(FuzzyClamp(m_CapacitorLevel));

	{
		// Waypoint Route
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_WaypointLock.lockForRead();
		m_aWaypoints >> out;
		m_WaypointLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Cargo
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_CargoLock.lockForRead();
		m_Cargo >> out;
		out << m_Shipments;
		m_CargoLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Damage
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_DamageLock.lockForRead();
		m_Damage >> out;
		out << m_tmDamageChange;	// We depend on the damage change time to delay the recovery of ships in stasis.
		m_DamageLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Roster
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_RosterLock.lockForRead();
		m_Roster >> out;

		quint8 hasWhat = 0;
		// bit 0x02 is reserved as a marker for old style dna
		if(!m_CryoBerths.empty())
			hasWhat |= 0x04;
		out << hasWhat;

		if(hasWhat & 0x04)
			out << m_CryoBerths;
		m_RosterLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(FuzzyClamp(m_Bank));
	qa.addBindValue(FuzzyClamp(m_BankSubsidyTax));

	{
		// Systems
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_SystemsLock.lockForRead();
		m_ssaShipSystems >> out;
		m_SystemsLock.unlock();
		qa.addBindValue(ba);
	}

	// Shields
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Front]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Back]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Left]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Right]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Top]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Bottom]));
	qa.addBindValue(m_PowerPlantShunts);
	qa.addBindValue(m_QMade);
	qa.addBindValue(m_WhereMade);

	{
		// Mission
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_MissionLock.lockForRead();
		m_Mission >> out;
		m_MissionLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Games
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_GamesLock.lockForRead();
		out << (quint16)m_Games.size();
		for(unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.begin(); gi != m_Games.end(); gi++)
		{
			out << (*gi).first;
			out << (*gi).second->Type();
			out << *(*gi).second;
		}
		m_GamesLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(m_RepairSecs);

	{
		// Bay settings
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);

		m_BayLock.lockForRead();
		out << (quint32)m_BaySetting.size();
		for(unordered_map<quint16, AuBaySetting>::const_iterator bi = m_BaySetting.begin(); bi != m_BaySetting.end(); bi++)
		{
			out << (*bi).first;
			out << (*bi).second;
		}
		m_BayLock.unlock();

		m_TurretLock.lockForRead();
		out << (quint32)m_Turrets.size();
		for(unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.begin(); ti != m_Turrets.end(); ti++)
		{
			out << (*ti).first;
			out << (*ti).second;
		}
		m_TurretLock.unlock();

		m_BayLock.lockForRead();
		out << (quint32)m_BayFired.size();
		for(unordered_map<quint16, AuBayFired>::const_iterator bi = m_BayFired.cbegin(); bi != m_BayFired.cend(); bi++)
		{
			out << (*bi).first;
			out << (*bi).second;
		}
		m_BayLock.unlock();

		qa.addBindValue(ba);
	}

	{
		// Rat species
		QByteArray ba;
		qa.addBindValue(ba);	// Clear obsolete ratdna field.

		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_VesselLock.lockForRead();
		out << m_RatSpecies;
		out << (quint8)m_MonsterOrigin;
		m_VesselLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(m_Monster & 0x7fff);

	// DaSc values.
	qa.addBindValue(DesignId());

	// Switches from DaVessel and DaSc can share the same field.
	// DaVessel switches.
	int switches=0;
	if(m_bExtLightOn)
		switches |= 0x00000001;
	if(m_bPowerPlantOn)
		switches |= 0x00000002;
	if(m_eGravityDriveState == GD_On)
		switches |= 0x00000004;
	else if(m_eGravityDriveState == GD_Hold)
		switches |= 0x00000008;
	if(m_InTow)
		switches |= 0x00000010;
	if(m_IsPrivate)
		switches |= 0x00000020;
	if(m_AcceptCrew)
		switches |= 0x00000040;
	if(m_AcceptPassengers)
		switches |= 0x00000080;
	if(m_AcceptTroops)
		switches |= 0x00000100;
	if(m_Moored)
		switches |= 0x00000200;
	if(m_bSensorsOn)
		switches |= 0x00000400;
	if(IsClientControlledNoReset())
		switches |= 0x00000800;
	if(m_AllowPassengerSeaChests)
		switches |= 0x00001000;
	if(m_AutoFetchVehicles)
		switches |= 0x00002000;
	if(m_ShieldState)
		switches |= (m_ShieldState << 14);	// bits b14-b15
	// DaSc switches.
	if(m_NavLights)
		switches |= 0x00010000;
	if(m_RotatingBeacons)
		switches |= 0x00020000;
	if(m_Strobes)
		switches |= 0x00040000;
	if(m_New)
		switches |= 0x00080000;
	qa.addBindValue(switches);

	{
		// Design report
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_DesignLock.lockForRead();
		out << m_Design;
		m_DesignLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Air Pressures and Alert Condition
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_AirPressures;
		out << (quint8)m_AlertCondition;
		qa.addBindValue(ba);
	}

	{
		// Door states
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_DoorStates;
		qa.addBindValue(ba);
	}

	{
		// FTL drives
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_FTLDrives;
		qa.addBindValue(ba);
	}

	{
		// Landing gear
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_LandingGearStates;
		qa.addBindValue(ba);
	}

	{
		// Landing systems
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_LandingSystemStates;
		qa.addBindValue(ba);
	}

	{
		// Launchers
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_LauncherStates;
		qa.addBindValue(ba);
	}

	{
		// Lifts
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_Lifts;
		qa.addBindValue(ba);
	}

	{
		// Room lights
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_RoomLightStates;
		qa.addBindValue(ba);
	}

	{
		// Transporters
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_TRoomStates;
		qa.addBindValue(ba);
	}

	{
		// Turret positions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_TurretPosLock.lockForRead();
		out << m_TurretPos;
		m_TurretPosLock.unlock();
		qa.addBindValue(ba);
	}

	if(!qa.exec())
		qWarning() << "error: DaSc::Insert FAILED." << Id() << qa.lastError() << sq;
}

void DaSc::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();
	AuPoint vVec = Vector();

	QString sq;
	sq = QString("UPDATE a_Sc SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, timeModified=?, timeTock=?, "
			"timeDeath=?, "
			"name=?, allegiance=?, arenaTeam=?, conditions=?, damageHistory=?, "
			"state=?, accessPolicy=?, pilotUpPolicy=?, shieldPolicy=?, shieldFuelPolicy=?, atRest=?, vecX=?, vecY=?, vecZ=?, "
			"throttle=?, throttleLateral=?, throttleLong=?, throttleVertical=?, "
			"ownerId=?, ownerName=?, durEngine=?, skewEngine=?, fuel=?, fuelqual=?, rotSpeeds=?, actBits=?, capacitor=?, "
			"route=?, cargo=?, damage=?, roster=?, bank=?, bankSubsidyTax=?, systems=?, shieldFront=?, shieldBack=?, shieldLeft=?, "
			"shieldRight=?, shieldTop=?, shieldBottom=?, powerPlantShunts=?, techLevel=?, whereMade=?, mission=?, games=?, "
			"repairSecs=?, baySetting=?, ratdna=?, ratspec=?, monster=?, "
			"idDesign=?, switches=?, designRpt=?, airPressures=?, doorStates=?, ftlDrives=?, landingGear=?, landingSystems=?, "
			"launchers=?, lifts=?, roomLights=?, transporters=?, turretPos=? "
			"WHERE id=%2;").arg(Id().GetID());
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

	// DaVessel values.
	qa.addBindValue(AtomState());
	qa.addBindValue(m_AccessPolicy);
	qa.addBindValue(m_PilotUpPolicy);
	qa.addBindValue(m_ShieldPolicy);
	qa.addBindValue(m_ShieldFuelPolicy);
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(vVec.x));
	qa.addBindValue(FuzzyClamp(vVec.y));
	qa.addBindValue(FuzzyClamp(vVec.z));
	qa.addBindValue(m_Throttle);
	qa.addBindValue(m_ThrustLateral);
	qa.addBindValue(m_ThrustLong);
	qa.addBindValue(m_ThrustVertical);
	qa.addBindValue(m_OwnerId.GetID());
	qa.addBindValue(m_OwnerName);
	int durengine = (m_byAccelSkewDur<<16)|(m_byTurnRateSkewDur<<8)|m_byPowerPlantSkewDur;
	qa.addBindValue(durengine);
	int skewengine = (m_chAccelSkew<<24)|(m_chTurnRateSkewX<<16)|(m_chTurnRateSkewY<<8)|m_chPowerPlantSkew;
	qa.addBindValue(skewengine);
	qa.addBindValue(FuzzyClamp(m_fFuelQuantity));
	qa.addBindValue(m_FuelQuality);
	int rotspeeds = (m_byRotSpeedX<<16)|(m_byRotSpeedY<<8)|m_byRotSpeedZ;
	qa.addBindValue(rotspeeds);
	qa.addBindValue(m_wActions);
	qa.addBindValue(FuzzyClamp(m_CapacitorLevel));

	{
		// Waypoint Route
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_WaypointLock.lockForRead();
		m_aWaypoints >> out;
		m_WaypointLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Cargo
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_CargoLock.lockForRead();
		m_Cargo >> out;
		out << m_Shipments;
		m_CargoLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Damage
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_DamageLock.lockForRead();
		m_Damage >> out;
		out << m_tmDamageChange;	// We depend on the damage change time to delay the recovery of ships in stasis.
		m_DamageLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Roster
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_RosterLock.lockForRead();
		m_Roster >> out;

		quint8 hasWhat = 0;
		// bit 0x02 is reserved as a marker for old style dna
		if(!m_CryoBerths.empty())
			hasWhat |= 0x04;
		out << hasWhat;

		if(hasWhat & 0x04)
			out << m_CryoBerths;
		m_RosterLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(FuzzyClamp(m_Bank));
	qa.addBindValue(FuzzyClamp(m_BankSubsidyTax));

	{
		// Systems
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_SystemsLock.lockForRead();
		m_ssaShipSystems >> out;
		m_SystemsLock.unlock();
		qa.addBindValue(ba);
	}

	// Shields
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Front]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Back]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Left]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Right]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Top]));
	qa.addBindValue(FuzzyClamp(m_ShieldCap[SHIELD_Bottom]));
	qa.addBindValue(m_PowerPlantShunts);
	qa.addBindValue(m_QMade);
	qa.addBindValue(m_WhereMade);

	{
		// Mission
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_MissionLock.lockForRead();
		m_Mission >> out;
		m_MissionLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Games
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_GamesLock.lockForRead();
		out << (quint16)m_Games.size();
		for(unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.begin(); gi != m_Games.end(); gi++)
		{
			out << (*gi).first;
			out << (*gi).second->Type();
			out << *(*gi).second;
		}
		m_GamesLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(m_RepairSecs);

	{
		// Bay settings
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);

		m_BayLock.lockForRead();
		out << (quint32)m_BaySetting.size();
		for(unordered_map<quint16, AuBaySetting>::const_iterator bi = m_BaySetting.begin(); bi != m_BaySetting.end(); bi++)
		{
			out << (*bi).first;
			out << (*bi).second;
		}
		m_BayLock.unlock();

		m_TurretLock.lockForRead();
		out << (quint32)m_Turrets.size();
		for(unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.begin(); ti != m_Turrets.end(); ti++)
		{
			out << (*ti).first;
			out << (*ti).second;
		}
		m_TurretLock.unlock();

		m_BayLock.lockForRead();
		out << (quint32)m_BayFired.size();
		for(unordered_map<quint16, AuBayFired>::const_iterator bi = m_BayFired.cbegin(); bi != m_BayFired.cend(); bi++)
		{
			out << (*bi).first;
			out << (*bi).second;
		}
		m_BayLock.unlock();

		qa.addBindValue(ba);
	}

	{
		// Rat species
		QByteArray ba;
		qa.addBindValue(ba);	// Clear obsolete ratdna field.

		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_VesselLock.lockForRead();
		out << m_RatSpecies;
		out << (quint8)m_MonsterOrigin;
		m_VesselLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(m_Monster & 0x7fff);

	// DaSc values.
	qa.addBindValue(DesignId());

	// Switches from DaVessel and DaSc can share the same field.
	// DaVessel switches.
	int switches=0;
	if(m_bExtLightOn)
		switches |= 0x00000001;
	if(m_bPowerPlantOn)
		switches |= 0x00000002;
	if(m_eGravityDriveState == GD_On)
		switches |= 0x00000004;
	else if(m_eGravityDriveState == GD_Hold)
		switches |= 0x00000008;
	if(m_InTow)
		switches |= 0x00000010;
	if(m_IsPrivate)
		switches |= 0x00000020;
	if(m_AcceptCrew)
		switches |= 0x00000040;
	if(m_AcceptPassengers)
		switches |= 0x00000080;
	if(m_AcceptTroops)
		switches |= 0x00000100;
	if(m_Moored)
		switches |= 0x00000200;
	if(m_bSensorsOn)
		switches |= 0x00000400;
	if(IsClientControlledNoReset())
		switches |= 0x00000800;
	if(m_AllowPassengerSeaChests)
		switches |= 0x00001000;
	if(m_AutoFetchVehicles)
		switches |= 0x00002000;
	if(m_ShieldState)
		switches |= (m_ShieldState << 14);	// bits b14-b15
	// DaSc switches.
	if(m_NavLights)
		switches |= 0x00010000;
	if(m_RotatingBeacons)
		switches |= 0x00020000;
	if(m_Strobes)
		switches |= 0x00040000;
	if(m_New)
		switches |= 0x00080000;
	qa.addBindValue(switches);

	{
		// Design report
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_DesignLock.lockForRead();
		out << m_Design;
		m_DesignLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Air Pressures and Alert Condition
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_AirPressures;
		out << (quint8)m_AlertCondition;
		qa.addBindValue(ba);
	}

	{
		// Door states
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_DoorStates;
		qa.addBindValue(ba);
	}

	{
		// FTL drives
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_FTLDrives;
		qa.addBindValue(ba);
	}

	{
		// Landing gear
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_LandingGearStates;
		qa.addBindValue(ba);
	}

	{
		// Landing systems
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_LandingSystemStates;
		qa.addBindValue(ba);
	}

	{
		// Launchers
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_LauncherStates;
		qa.addBindValue(ba);
	}

	{
		// Lifts
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_Lifts;
		qa.addBindValue(ba);
	}

	{
		// Room lights
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_RoomLightStates;
		qa.addBindValue(ba);
	}

	{
		// Transporters
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << m_TRoomStates;
		qa.addBindValue(ba);
	}

	{
		// Turret positions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_TurretPosLock.lockForRead();
		out << m_TurretPos;
		m_TurretPosLock.unlock();
		qa.addBindValue(ba);
	}

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		qWarning() << "error: DaSc::Update" << Id() << Name() << sq << qa.lastError();
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaSc::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

bool DaSc::InsideRoom(const AuPoint& loc, quint16& roomId) const
{
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	roomId = scd->RoomVoidContainingLocLive(loc, .1, m_DoorStates, m_LandingGearStates, m_Lifts);
	UnlockDesign();
	m_DesignLock.unlock();
	return roomId;
}

quint16 DaSc::InsideRoomType(const AuPoint& loc, ROOMTYPE& roomType, QString& roomName) const
{
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(!scd)
	{
		UnlockDesign();
		m_DesignLock.unlock();
		roomType = ROOM_Corridor;
		roomName = "";
		return 0;
	}
	quint16 roomId = scd->RoomVoidContainingLocLive(loc, .1, m_DoorStates, m_LandingGearStates, m_Lifts);
	HOLD_ACCESS a;
	if(roomId)
	{
		a = scd->RoomVoidHoldAccess(roomId);
		roomName = scd->RoomVoidName(roomId);
	}
	else
	{
		a = HOLD_Inaccessible;
		if(scd->IsInsideHull(loc, m_DoorStates, m_LandingGearStates, m_Lifts))
		{
			roomName = AuScPart::GroupNameHull();
			roomType = ROOM_Hull;
		}
		else
			roomName = "";
	}
	UnlockDesign();
	m_DesignLock.unlock();
	if(roomId)
	{
		// Assign a room type based on hold access.
		switch(a)
		{
		case HOLD_Inaccessible:
		case HOLD_Count:
			roomType = ROOM_Corridor;
			break;
		case HOLD_Armory:
			roomType = ROOM_Barracks;
			break;
		case HOLD_Bar:
			roomType = ROOM_Lounge;
			break;
		case HOLD_Feeder:
			roomType = ROOM_Brig;
			break;
		case HOLD_FuelCoupling:
			roomType = ROOM_EngineRoom;
			break;
		case HOLD_VehicleBay:
			roomType = ROOM_VehicleBay;
			break;
		case HOLD_Hold:
			roomType = ROOM_Hold;
			break;
		case HOLD_Pantry:
			roomType = ROOM_Galley;
			break;
		case HOLD_Pharmacy:
			roomType = ROOM_SickBay;
			break;
		case HOLD_Wardrobe:
			roomType = ROOM_Stateroom;
			break;
		}
	}
	return roomId;
}

bool DaSc::InsideRoomType(const AuPoint& loc, bool getRoomUp, quint16& roomId, ROOMTYPE& roomType, double& htAbvFloor,
		QString& roomName, AuPoint& gravityN) const
{
	roomId = InsideRoomType(loc, roomType, roomName);
	if(!roomId)
		return false;

	gravityN = GravityNormal();
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		htAbvFloor = scd->RoomVoidHeightAboveFloor(roomId, loc, getRoomUp, gravityN, m_Lifts);	// Height above room void floor.
	else
		htAbvFloor = 0;
	UnlockDesign();
	m_DesignLock.unlock();
	return true;
}

void DaSc::WindowSurfaceGlow(const QString& windowName, float& glowR, float& glowG, float& glowB) const
{
	m_RoomLightLock.lockForRead();
	INTERIOR_LIGHT interiorLight = m_RoomLightStates.InteriorLight(windowName);
	if(interiorLight == INTLIGHT_Invalid)
	{
		interiorLight = m_WindowLightStates.InteriorLight(windowName);
		if(interiorLight == INTLIGHT_Invalid)
			interiorLight = INTLIGHT_Dark;
	}
	m_RoomLightLock.unlock();

	if(interiorLight == INTLIGHT_Bright)
	{
		// Bright interior lighting.
		const QColor& c = AuGLLight::ColorFluorescentTubeDaylight();
		glowR = AuGLLight::AmbientLitInterior()*c.redF();
		glowG = AuGLLight::AmbientLitInterior()*c.greenF();
		glowB = AuGLLight::AmbientLitInterior()*c.blueF();
	}
	else if(interiorLight == INTLIGHT_Dim)
	{
		// Dim interior lighting.
		const QColor& c = AuGLLight::ColorFluorescentTubeCoolWhite();
		glowR = AuGLLight::AmbientLitInteriorDim()*c.redF();
		glowG = AuGLLight::AmbientLitInteriorDim()*c.greenF();
		glowB = AuGLLight::AmbientLitInteriorDim()*c.blueF();
	}
	else
	{
		// No interior lighting means no window glow.
		glowR = 0;
		glowG = 0;
		glowB = 0;
		return;
	}

	if(glowR < 0.f)
		glowR = 0.f;
	else if(glowR > 1.f)
		glowR = 1.f;

	if(glowG < 0.f)
		glowG = 0.f;
	else if(glowG > 1.f)
		glowG = 1.f;

	if(glowB < 0.f)
		glowB = 0.f;
	else if(glowB > 1.f)
		glowB = 1.f;
}

void DaSc::InteriorAmbientLight(const QString& roomName, float& ambientR, float& ambientG, float& ambientB, bool& sunsBlocked) const
{
	// Calculate ambient lighting from interior lights, if any.
	INTERIOR_LIGHT interiorLight;
	if(CapacitorLevel())
	{
		interiorLight = InteriorLight(roomName);
		if(interiorLight == INTLIGHT_Bright)
		{
			// Bright interior lighting.
			const QColor& c = AuGLLight::ColorFluorescentTubeDaylight();
			ambientR = AuGLLight::AmbientLitInterior()*c.redF();
			ambientG = AuGLLight::AmbientLitInterior()*c.greenF();
			ambientB = AuGLLight::AmbientLitInterior()*c.blueF();
		}
		else if(interiorLight == INTLIGHT_Dim)
		{
			// Dim interior lighting.
			const QColor& c = AuGLLight::ColorFluorescentTubeCoolWhite();
			ambientR = AuGLLight::AmbientLitInteriorDim()*c.redF();
			ambientG = AuGLLight::AmbientLitInteriorDim()*c.greenF();
			ambientB = AuGLLight::AmbientLitInteriorDim()*c.blueF();
		}
		else
		{
			// Very little interior lighting, not quite black.
			ambientR = AuGLLight::AmbientDarkInterior();
			ambientG = AuGLLight::AmbientDarkInterior();
			ambientB = AuGLLight::AmbientDarkInterior();
		}
	}
	else
	{
		// No power for interior lighting.
		ambientR = AuGLLight::AmbientDarkness();
		ambientG = AuGLLight::AmbientDarkness();
		ambientB = AuGLLight::AmbientDarkness();
	}

	sunsBlocked = !m_SunlitRooms.contains(roomName);
	if(!sunsBlocked)
	{
		// Add ambient light level of external environment.
		float extAmbR;
		float extAmbG;
		float extAmbB;
		float fogDen, fogR, fogG, fogB;
		bool eSunsBlocked;
		SceneRenderParameters(0, extAmbR, extAmbG, extAmbB, fogDen, fogR, fogG, fogB, false, eSunsBlocked);
		ambientR += extAmbR*.85f;
		ambientG += extAmbG*.85f;
		ambientB += extAmbB*.85f;
		if(eSunsBlocked)
			sunsBlocked = true;
	}

	// Clamp RGB values.
	if(ambientR < 0.f)
		ambientR = 0.f;
	else if(ambientR > 1.f)
		ambientR = 1.f;

	if(ambientG < 0.f)
		ambientG = 0.f;
	else if(ambientG > 1.f)
		ambientG = 1.f;

	if(ambientB < 0.f)
		ambientB = 0.f;
	else if(ambientB > 1.f)
		ambientB = 1.f;
}

INTERIOR_LIGHT DaSc::InteriorLight(const QString& roomName) const
{
	m_RoomLightLock.lockForRead();
	INTERIOR_LIGHT v = m_RoomLightStates.InteriorLight(roomName);
	if(v == INTLIGHT_Invalid)
		v = INTLIGHT_Dark;
	m_RoomLightLock.unlock();
	return v;
}

bool DaSc::InteriorLightToggle(INTERIOR_LIGHT state)
{
	m_RoomLightLock.lockForWrite();
	bool anySet = m_RoomLightStates.SetInteriorLight(state);
	if(anySet)
		m_RoomLightChg.Set();
	m_RoomLightLock.unlock();
	return anySet;
}

bool DaSc::InteriorLightToggle(const QString& roomName, INTERIOR_LIGHT state)
{
	m_RoomLightLock.lockForWrite();
	bool anySet = m_RoomLightStates.SetInteriorLight(roomName, state);
	if(anySet)
		m_RoomLightChg.Set();
	m_RoomLightLock.unlock();
	return anySet;
}

bool DaSc::MoonOrbitRingsVisible(const QString& roomName) const
{
	m_RoomLightLock.lockForRead();
	bool v = m_RoomLightStates.MoonOrbitRingsVisible(roomName);
	m_RoomLightLock.unlock();
	return v;
}

bool DaSc::SetMoonOrbitRingsVisible(const QString& roomName, bool visible)
{
	m_RoomLightLock.lockForWrite();
	bool anySet = m_RoomLightStates.SetMoonOrbitRingsVisible(roomName, visible);
	if(anySet)
		m_RoomLightChg.Set();
	m_RoomLightLock.unlock();
	if(anySet)
		SetModified();
	return anySet;
}

bool DaSc::OrbitRingsVisible(const QString& roomName) const
{
	m_RoomLightLock.lockForRead();
	bool v = m_RoomLightStates.OrbitRingsVisible(roomName);
	m_RoomLightLock.unlock();
	return v;
}

bool DaSc::SetOrbitRingsVisible(const QString& roomName, bool visible)
{
	m_RoomLightLock.lockForWrite();
	bool anySet = m_RoomLightStates.SetOrbitRingsVisible(roomName, visible);
	if(anySet)
		m_RoomLightChg.Set();
	m_RoomLightLock.unlock();
	if(anySet)
		SetModified();
	return anySet;
}

int DaSc::SystemNamesVisible(const QString& roomName) const
{
	m_RoomLightLock.lockForRead();
	int v = m_RoomLightStates.SystemNamesVisible(roomName);
	m_RoomLightLock.unlock();
	return v;
}

bool DaSc::SetSystemNamesVisible(const QString& roomName, int visible)
{
	m_RoomLightLock.lockForWrite();
	bool anySet = m_RoomLightStates.SetSystemNamesVisible(roomName, visible);
	if(anySet)
		m_RoomLightChg.Set();
	m_RoomLightLock.unlock();
	if(anySet)
		SetModified();
	return anySet;
}

bool DaSc::WormholeLinesVisible(const QString& roomName) const
{
	m_RoomLightLock.lockForRead();
	bool v = m_RoomLightStates.WormholeLinesVisible(roomName);
	m_RoomLightLock.unlock();
	return v;
}

bool DaSc::SetWormholeLinesVisible(const QString& roomName, bool visible)
{
	m_RoomLightLock.lockForWrite();
	bool anySet = m_RoomLightStates.SetWormholeLinesVisible(roomName, visible);
	if(anySet)
		m_RoomLightChg.Set();
	m_RoomLightLock.unlock();
	if(anySet)
		SetModified();
	return anySet;
}

bool DaSc::BridgeVisible(const QString& roomName) const
{
	m_RoomLightLock.lockForRead();
	bool v = m_RoomLightStates.BridgeVisible(roomName);
	m_RoomLightLock.unlock();
	return v;
}

bool DaSc::SetBridgeVisible(const QString& roomName, bool visible)
{
	m_RoomLightLock.lockForWrite();
	bool anySet = m_RoomLightStates.SetBridgeVisible(roomName, visible);
	if(anySet)
		m_RoomLightChg.Set();
	m_RoomLightLock.unlock();
	if(anySet)
		SetModified();
	return anySet;
}

bool DaSc::IsAnyFTLDriveHot() const
{
	m_FTLLock.lockForRead();
	QList<quint16> ftlIds = m_FTLDrives.keys();
	m_FTLLock.unlock();

	for(QList<quint16>::const_iterator fi = ftlIds.cbegin(); fi != ftlIds.cend(); fi++)
		if(IsFTLDriveHot(*fi))
			return true;
	return false;
}

bool DaSc::IsAnyFTLDriveWorking() const
{
	m_FTLLock.lockForRead();
	QList<quint16> ftlIds = m_FTLDrives.keys();
	m_FTLLock.unlock();

	for(QList<quint16>::const_iterator fi = ftlIds.cbegin(); fi != ftlIds.cend(); fi++)
		if(FTLDriveWorking(*fi))
			return true;
	return false;
}

bool DaSc::IsCommandConsole(quint16 stationId) const
{
	m_DesignLock.lockForRead();
	bool isCmdConsole = m_Design.CommandStations().contains(stationId);
	m_DesignLock.unlock();
	return isCmdConsole;
}

/*
bool DaSc::IsCommandConsoleAt(const AuPoint& loc) const
{
	bool hasConsole = false;

	m_DesignLock.lockForRead();
	const DgData& sdd = g_Au.LockScDesignsForRead();

	AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find(m_Design.DesignId());
	if(di != sdd.m_ScDesigns->cend())
	{
		for(AuScReportStationHash::const_iterator si = m_Design.CommandStations().cbegin();
				si != m_Design.CommandStations().cend(); si++)
		{
			// Locate the corresponding part in the design.
			AuScParts::const_iterator pi = (*di).second.Parts().find(si.key());
			if(pi == (*di).second.Parts().cend())
				continue;

			if((*pi)->Type() == AuScPart::Part_ConsoleCommand)
			{
				// Must check radius for spinning consoles.
				if(loc.Distance2((*pi)->OperatorLoc()) > 1.7*1.7)
					continue;
			}
			else
			{
				if(!(*pi)->OperatorLoc().FuzzyEquals(loc))
					continue;
			}

			hasConsole = true;
			break;
		}
	}

	g_Au.UnlockScDesigns();
	m_DesignLock.unlock();

	return hasConsole;
}
*/

bool DaSc::IsControl(const DaAtom* sceneAtom) const
{
	Q_UNUSED(sceneAtom);
	return true;
}

bool DaSc::IsDesignDataCacheValid() const
{
	if(m_DesignCacheValid)
		return true;

	// Get my parent design environment, if any. I could be a preview of a design in progress.
	bool inDesigner;
	if(EnvironmentQueryValid())
		inDesigner = InDesignEnvironment();
	else if(FindParentDesignerInEnv())
		inDesigner = true;
	else
		inDesigner = false;

	if(inDesigner)
		m_DesignCacheValid = true;
	else
	{
		// Wait until the design is ready to answer geometry queries.
		quint32 designId = DesignId();
		const DgData& sdd = g_Au.LockScDesignsForRead();
		m_DesignCacheValid = sdd.ScDesignIsLoaded(designId);
		g_Au.UnlockScDesigns();
	}

	// When my design becomes valid for the first time, I need to wake up the npc's aboard.
	if(m_DesignCacheValid)
		TickTockRecursive();

	return m_DesignCacheValid;
}

bool DaSc::IsDoorIndexValid(unsigned int doorIndex) const
{
	m_DesignLock.lockForRead();
	bool is = m_DoorStates.find(doorIndex) != m_DoorStates.cend();
	m_DesignLock.unlock();
	return is;
}

bool DaSc::IsDoorJammed(unsigned int doorIndex) const
{
	m_DesignLock.lockForRead();
	bool is = m_DoorStates.IsDoorJammed(doorIndex);
	m_DesignLock.unlock();
	return is;
}

bool DaSc::IsDoorLocked(unsigned int doorId) const
{
	m_DesignLock.lockForRead();
	bool isLocked = m_DoorStates.IsDoorLocked(doorId);
	m_DesignLock.unlock();
	if(!isLocked)
		return false;
	if(IsStandingDown())
		return false;					// Doors are unlocked while standing down.
	return true;
}

bool DaSc::IsDoorOnWall(unsigned int doorIndex) const
{
	Q_UNUSED(doorIndex);
	// In practice, this is used to decide if a doorway has an associated ladder. AuScReport::EntranceLocation returns ladders as
	// 'holes', so we'll say all doors are on the wall, without ladders.
	return true;
}

bool DaSc::IsDoorOpen(unsigned int doorIndex) const
{
	m_DesignLock.lockForRead();
	bool is = m_DoorStates.IsDoorOpen(doorIndex);
	m_DesignLock.unlock();
	return is;
}

bool DaSc::IsFirstClass() const
{
	m_DesignLock.lockForRead();
	bool is = m_Design.IsFirstClass();
	m_DesignLock.unlock();
	return is;
}

bool DaSc::IsFTLDriveHot(quint16 whichFTL) const
{
	if(!whichFTL)
		return false;
	bool h = false;
	m_FTLLock.lockForRead();
	QHash<quint16, AuFTLDriveState>::const_iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.cend())
		h = (*fi).Power() == FTL_DRIVE_On &&
			(*fi).Commit() >= FTL_COMMIT_ChargedBegin &&
			(*fi).Commit() <= FTL_COMMIT_ChargedEnd;
	m_FTLLock.unlock();
	return h;
}

bool DaSc::IsFTLDriveOn(quint16 whichFTL) const
{
	bool h = false;
	m_FTLLock.lockForRead();
	QHash<quint16, AuFTLDriveState>::const_iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.cend())
		h = (*fi).Power() == FTL_DRIVE_On;
	m_FTLLock.unlock();
	return h;
}

bool DaSc::IsHoleOnWall(unsigned int holeIndex) const
{
	// In practice, this is used to decide if a hole has an associated ladder. AuScReport::EntranceLocation returns ladders as
	// 'holes' with a non zero index. Holes with a zero index are ladderless openings in the hull, equivalent to a hole on a wall
	// in the old designs.
	return holeIndex == 0;
}

bool DaSc::IsHullDoor(int doorIndex) const
{
	m_DesignLock.lockForRead();
	bool is = m_DoorStates.IsHullDoor(doorIndex);
	m_DesignLock.unlock();
	return is;
}

// returns 0=no, 1=in hull, 2=on hull, 3=under hull hatch
int DaSc::IsInOrOnHull(const AuPoint& loc) const
{
	bool v;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		v = scd->IsInsideHull(loc, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		v = false;
	UnlockDesign();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(sceneAtom);
	if(IsInSunSafeZone())
		return false;

	// Calculate scene atom location in my coordinate space.
	AuPoint sceneLoc = EnvTransform().UnProject(cameraEnvLoc);
	return sceneLoc.Length2() < SightRange2();
}

bool DaSc::IsInsideAirBreathable() const
{
	// Check air pressure in all rooms. Any breathable air and the answer is yes.
	// NOTE: The 15%-85% pressure range matches the atmosphere density requirement in DaWorld::HasBreathableAtmosphere.
	bool anyAir = false;
	m_AirLock.lockForRead();
	for(AuAirPressureHash::const_iterator ai = m_AirPressures.cbegin(); ai != m_AirPressures.cend(); ai++)
		if(*ai >= 30 && *ai <= 170)
		{
			anyAir = true;
			break;
		}
	m_AirLock.unlock();
	return anyAir;
}

bool DaSc::IsNew() const
{
	return m_New;
}

bool DaSc::IsEthereal() const
{
	return IsAnyFTLDriveHot() && m_DbLoadTime.ElapsedSec() < 5;
}

bool DaSc::IsObstacle(const DaAtom* toWho) const
{
	if(IsEthereal())
		return false;
	if(IsInAtmosphere() != toWho->IsInAtmosphere())
		return false;
	const DaMissile* missile = dynamic_cast<const DaMissile*>(toWho);	// Includes bombs.
	if(missile)
	{
		if(missile->IsArmedBy(Id()))
			return false;	// I am not an obstacle to my own munitions.
	}
	return true;	// Spacecraft are obstacles to movement.
}

bool DaSc::IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho)
{
	if(!IsDesignDataCacheValid())
	{
		hitLoc = oloc;
		hitNormal = AuPoint(0,0,0);								// Do not bounce.
		return true;											// No movement allowed until the design is valid.
	}

	if(toWho->AgentEntity())
		return false;

	if(ShieldsBlockWeapons() && !ShieldContains(oloc))
	{
		// The atom is moving outside my shield bubble.
		AuPoint cen(DesignCenterOffset());
		AuPoint isect1, isect2;
		int isect = oloc.IntersectEllipsoid(cen, ShieldRadii(), nloc, isect1, isect2);
		if(isect > 0)
		{
			// The path intersects my shield bubble.
			// If our relative speed is too fast, bounce them.
			DaAtom* cp = CommonParent(toWho, 0);
			if(cp && cp != this)
			{
				AuMatrix4x4 tme;
				AuMatrix4x4 twho;
				if(GetTransformRelativeToParent(tme, cp) && toWho->GetTransformRelativeToParent(twho, cp))
				{
					AuPoint vwho(twho.ProjectVector(toWho->Vector()));	// toWho's vector relative to the common parent.
					vwho = tme.UnProjectVector(vwho);					// toWho's vector relative to me.
					if(vwho.Length() > ShieldCollisionSpeed())
					{
						// toWho is moving more than the shield collision speed, relative to me.
						hitLoc = isect1;
						hitNormal = isect1 - cen;
						hitNormal.Unit();
						return true;
					}
				}
			}
		}
	}

	// Creatures and items may open automatic doors along the path.
	if(toWho && toWho->OpensAutomaticDoors())
	{
		QSet<quint16> opened;
		m_DesignLock.lockForWrite();
		if(m_DoorStates.OpenAutomaticDoors(oloc, nloc, FeetPerMeter()*2, opened))
		{
			m_DoorChg.Set();
			for(QSet<quint16>::const_iterator oi = opened.cbegin(); oi != opened.cend(); oi++)
				m_DoorOpenTimes.insert(*oi, AuTime::Now());
			SetModified();
		}
		m_DesignLock.unlock();
	}

	bool is;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		is = scd->IsPathBlocked(oloc, nloc, hitLoc, hitNormal, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		is = false;
	UnlockDesign();
	m_DesignLock.unlock();
	return is;
}

bool DaSc::IsSecondClass() const
{
	m_DesignLock.lockForRead();
	bool is = m_Design.IsSecondClass();
	m_DesignLock.unlock();
	return is;
}

bool DaSc::IsSpaceStation() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.IsSpaceStation();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::IsTransporterSystemActive(int troomIndex) const
{
	m_TRoomLock.lockForRead();
	AuTransporterSystemStateHash::const_iterator ti = m_TRoomStates.find(troomIndex);
	bool v = ti != m_TRoomStates.cend() && *ti == TRANSPORTER_Active;
	m_TRoomLock.unlock();
	return v;
}

bool DaSc::IsTransporterSystemOn(int troomIndex) const
{
	m_TRoomLock.lockForRead();
	AuTransporterSystemStateHash::const_iterator ti = m_TRoomStates.find(troomIndex);
	bool v = ti != m_TRoomStates.cend() && (*ti == TRANSPORTER_On || *ti == TRANSPORTER_Active);
	m_TRoomLock.unlock();
	return v;
}

void DaSc::JamDoorClosed(unsigned int doorIndex)
{
	m_DesignLock.lockForWrite();
	if(m_DoorStates.JamDoorClosed(doorIndex))
	{
		m_DoorChg.Set();
		m_DoorOpenTimes.remove(doorIndex);
		SetModified();
	}
	m_DesignLock.unlock();
}

void DaSc::JamDoorOpen(unsigned int doorIndex)
{
	m_DesignLock.lockForWrite();
	if(m_DoorStates.JamDoorOpen(doorIndex))
	{
		m_DoorChg.Set();
		m_DoorOpenTimes.insert(doorIndex, AuTime::Now());
		SetModified();
	}
	m_DesignLock.unlock();
}

double DaSc::Width() const
{
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	return max.x - min.x;
}

double DaSc::Length() const
{
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	return max.y - min.y;
}

double DaSc::Height() const
{
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	return max.z - min.z;
}

void DaSc::ListOccupiedTroopStations(QSet<quint16>& troopStations, const AuId& exceptTroop) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaCitizen* troop = dynamic_cast<const DaCitizen*>(*it);
		if(!troop || !troop->IsNpcTroop())
			continue;
		if(troop->Is(exceptTroop))
			continue;
		if(troop->EmpireId() != EmpireId())
			continue;
		quint16 stationId = troop->StationId();
		if(!stationId)
			continue;
		if(!troop->IsPostPose())
			continue;
		troopStations.insert(stationId);
	}
}

void DaSc::ListSystems(QList<AuSystemInfo>& systems) const
{
	const AuShipSystemArray& ssa = LockSystems();

	m_DesignLock.lockForRead();
	for(AuShipSystemArray::const_iterator ssi = ssa.constBegin(); ssi != ssa.constEnd(); ssi++)
		if((*ssi).GetSystemType() != SYS_CommandAndControl &&
				(*ssi).GetSystemType() != SYS_VehicleLaunch &&
				(*ssi).GetSystemType() != SYS_VehicleLanding)
			systems.append(AuSystemInfo(*ssi, m_Design));
	m_DesignLock.unlock();

	UnlockSystems();
}

//static
DaSc* DaSc::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,timeTock,timeDeath, "
			"name,allegiance,arenaTeam,conditions,damageHistory, "
			"state,accessPolicy,pilotUpPolicy,shieldPolicy,shieldFuelPolicy,atRest,vecX,vecY,vecZ, "
			"throttle,throttleLateral,throttleLong,throttleVertical, "
			"ownerId,ownerName,durEngine,skewEngine,fuel,fuelqual,rotSpeeds,actBits,capacitor,route,cargo,damage,roster, "
			"bank,bankSubsidyTax,systems,shieldFront,shieldBack,shieldLeft,shieldRight,shieldTop,shieldBottom, "
			"powerPlantShunts,techLevel,whereMade,mission,games,repairSecs,baySetting,ratdna,ratspec,monster, "
			"switches,designRpt,airPressures,doorStates,ftlDrives,landingGear,landingSystems,launchers,lifts, "
			"roomLights,transporters,turretPos "
			"FROM a_Sc WHERE ");
	if(!idToLoad.IsNull())
		sq += " id=" + QVariant(idToLoad.GetID()).toString();
	else
		sq += " idp=" + QVariant(parentId.GetID()).toString();
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaSc *pnu = new DaSc;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;	// query value index
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
			AuPoint myloc(vx,vy,vz);

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

			// DaVessel values.
			ATOM_STATE atomState = (ATOM_STATE)qa.value(qv++).toInt();
			pnu->m_AccessPolicy = (ACCESS_POLICY)qa.value(qv++).toInt();
			pnu->m_PilotUpPolicy = (PILOT_UP_POLICY)qa.value(qv++).toInt();
			pnu->m_ShieldPolicy = (SHIELD_POLICY)qa.value(qv++).toInt();
			pnu->m_ShieldFuelPolicy = (SHIELDFUEL_POLICY)qa.value(qv++).toInt();
			pnu->SetAtRestState(qa.value(qv++).toBool());
			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			pnu->SetVector(AuPoint(vx,vy,vz));
			pnu->m_Throttle = qa.value(qv++).toInt();
			pnu->m_ThrustLateral = qa.value(qv++).toInt();
			pnu->m_ThrustLong = qa.value(qv++).toInt();
			pnu->m_ThrustVertical = qa.value(qv++).toInt();

			// Stop moving when loaded from the database if too much time has passed.
			if(pnu->ModifyTime().ElapsedSec() > 5*60)
			{
				// The date changed or too much time has passed. Stop the ship for its own safety.
				pnu->m_Throttle = 0;
				pnu->m_ThrustLateral = 0;
				pnu->m_ThrustLong = 0;
				pnu->m_ThrustVertical = 0;
				pnu->SetVector(AuPoint(0,0));
			}
			pnu->m_OwnerId = AuId(qa.value(qv++).toUInt());
			pnu->m_OwnerName = qa.value(qv++).toString();
			int durengine = qa.value(qv++).toInt();
			pnu->m_byAccelSkewDur = (durengine>>16)&0xff;
			pnu->m_byTurnRateSkewDur = (durengine>>8)&0xff;
			pnu->m_byPowerPlantSkewDur = durengine&0xff;
			int skewengine = qa.value(qv++).toInt();
			pnu->m_chAccelSkew = (skewengine>>24)&0xff;
			pnu->m_chTurnRateSkewX = (skewengine>>16)&0xff;
			pnu->m_chTurnRateSkewY = (skewengine>>8)&0xff;
			pnu->m_chPowerPlantSkew = skewengine&0xff;
			pnu->m_fFuelQuantity = qa.value(qv++).toDouble();
			pnu->m_FuelQuality = qa.value(qv++).toInt();
			if(!pnu->m_FuelQuality)
				pnu->m_FuelQuality = 1;
			int rotspeeds = qa.value(qv++).toInt();
			pnu->m_byRotSpeedX = (rotspeeds>>16)&0xff;
			pnu->m_byRotSpeedY = (rotspeeds>>8)&0xff;
			pnu->m_byRotSpeedZ = rotspeeds&0xff;
			pnu->m_wActions = qa.value(qv++).toInt();
			pnu->m_CapacitorLevel = qa.value(qv++).toDouble();

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Waypoint route
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_aWaypoints << in;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Cargo
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Cargo << in;
				in >> pnu->m_Shipments;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Damage
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Damage << in;
				in >> pnu->m_tmDamageChange;	// We depend on the damage change time to delay the recovery of ships in stasis.
				if(pnu->m_tmDamageChange.IsNull())
					pnu->m_tmDamageChange.Set();
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Roster
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Roster << in;

				quint8 hasWhat;
				in >> hasWhat;

				if(hasWhat & 0x01)
				{
					AuTime officerOnLine;
					in >> officerOnLine;
				}

				if(hasWhat & 0x02)
				{
					QMap<DhDNA, quint32> dna3berths;
					in >> dna3berths;

					for(QMap<DhDNA, quint32>::const_iterator bi = dna3berths.cbegin(); bi != dna3berths.cend(); bi++)
						pnu->m_CryoBerths.insert(bi.key().Species(), *bi);
				}
				else if(hasWhat & 0x04)
					in >> pnu->m_CryoBerths;
			}

			pnu->m_Bank = qa.value(qv++).toDouble();
			pnu->m_BankSubsidyTax = qa.value(qv++).toDouble();

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Ship Systems
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_ssaShipSystems << in;
			}

			pnu->m_ShieldCap[SHIELD_Front] = qa.value(qv++).toDouble();
			pnu->m_ShieldCap[SHIELD_Back] = qa.value(qv++).toDouble();
			pnu->m_ShieldCap[SHIELD_Left] = qa.value(qv++).toDouble();
			pnu->m_ShieldCap[SHIELD_Right] = qa.value(qv++).toDouble();
			pnu->m_ShieldCap[SHIELD_Top] = qa.value(qv++).toDouble();
			pnu->m_ShieldCap[SHIELD_Bottom] = qa.value(qv++).toDouble();
			pnu->m_PowerPlantShunts = qa.value(qv++).toInt();
			pnu->m_QMade = qa.value(qv++).toInt();
			if(!pnu->m_QMade)
				pnu->m_QMade = 1;
			pnu->m_WhereMade = qa.value(qv++).toString();

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Mission
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Mission << in;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Games
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				// Active games at tables.
				quint16 gameCount;
				in >> gameCount;
				//qDebug() << "DaSc::LoadSubatoms" << pnu->Id() << pnu->Name() << "games" << gameCount;

				quint8 gameType;
				quint16 whichTable;
				AuGame* game;
				for(; gameCount; gameCount--)
				{
					in >> whichTable;
					in >> gameType;
					game = AuGame::CreateGame(gameType);
					if(game)
					{
						// The game object was created.
						in >> *game;
						pnu->m_Games.insert(pair<quint16, AuGame*>(whichTable, game));
					}
					else
					{
						qWarning() << "Spacecraft" << pnu->Id() << pnu->Name() << "Game table" << whichTable
							<< "loaded unknown game type" << gameType;
						break;
					}
				}
			}

			pnu->m_RepairSecs = qa.value(qv++).toInt();

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Weapon bay settings
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				pnu->m_BaySetting.clear();
				quint32 n;
				in >> n;
				//qDebug() << "DaSc::LoadSubatoms" << pnu->Id() << pnu->Name() << "bay settings" << n;
				while(n--)
				{
					quint16 key;
					AuBaySetting val;
					in >> key;
					in >> val;
					pnu->m_BaySetting.insert(pair<quint16, AuBaySetting>(key, val));
				}

				pnu->m_Turrets.clear();
				in >> n;
				//qDebug() << "DaSc::LoadSubatoms" << pnu->Id() << pnu->Name() << "turret settings" << n;
				while(n--)
				{
					quint16 key;
					AuOrdnance val;
					in >> key;
					in >> val;
					pnu->m_Turrets.insert(pair<quint16, AuOrdnance>(key, val));
				}

				pnu->m_BayFired.clear();
				in >> n;
				//qDebug() << "DaSc::LoadSubatoms" << pnu->Id() << pnu->Name() << "bay settings" << n;
				while(n--)
				{
					quint16 key;
					AuBayFired val;
					in >> key;
					in >> val;
					pnu->m_BayFired.insert(pair<quint16, AuBayFired>(key, val));
				}
			}

			// Rat DNA
			QByteArray badna = qa.value(qv++).toByteArray();
			QByteArray baspecies = qa.value(qv++).toByteArray();
			if(!badna.isEmpty())
			{
				// Old dna take precedence if it exists.
				DhDNA dna3(badna);
				if(dna3 != DhDNA())
					pnu->m_RatSpecies = dna3.Species();
			}
			else
			{
				QDataStream in(baspecies);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_RatSpecies;
				quint8 ui8;
				in >> ui8;
				pnu->m_MonsterOrigin = (MONSTER_ORIGIN)ui8;
			}

			pnu->m_Monster = qa.value(qv++).toUInt();

			// DaSc values:
			int switches = qa.value(qv++).toInt();

			// Break down the switches bits of information.
			// DaVessel switches.
			pnu->m_bExtLightOn = switches & 0x0001;
			pnu->m_bPowerPlantOn = switches & 0x0002;
			if(switches & 0x0004)
				pnu->m_eGravityDriveState = GD_On;
			else if(switches & 0x0008)
				pnu->m_eGravityDriveState = GD_Hold;
			pnu->m_InTow = switches & 0x00000010;
			pnu->m_IsPrivate = switches & 0x00000020;
			pnu->m_AcceptCrew = switches & 0x00000040;
			pnu->m_AcceptPassengers = switches & 0x00000080;
			pnu->m_AcceptTroops = switches & 0x00000100;
			pnu->m_Moored = switches & 0x00000200;
			pnu->m_bSensorsOn = switches & 0x00000400;
			if(switches & 0x00000800)
				pnu->SetControlled(pnu->Id());
			pnu->m_AllowPassengerSeaChests = switches & 0x0001000;
			pnu->m_AutoFetchVehicles = switches & 0x0002000;
			pnu->m_ShieldState = (SHIELD_STATE)((switches & 0x0000c000) >> 14);	// bits b14-b15
			// DaSc switches.
			pnu->m_NavLights = switches & 0x00010000;
			pnu->m_RotatingBeacons = switches & 0x00020000;
			pnu->m_Strobes = switches & 0x00040000;
			pnu->m_New = switches & 0x00080000;

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Design report
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_Design;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Air Pressures and Alert Condition
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_AirPressures;
				quint8 ui8;
				in >> ui8;
				pnu->m_AlertCondition = (ALERT_CONDITION)ui8;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Door states
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_DoorStates;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// FTL drives
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_FTLDrives;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Landing Gear
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_LandingGearStates;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Landing Systems
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_LandingSystemStates;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Launchers
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_LauncherStates;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Lifts
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_Lifts;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Room lights
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_RoomLightStates;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Transporters
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_TRoomStates;
			}

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Turret positions
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_TurretPos;
			}

			pnu->m_DbLoadTime.start();
			pnu->ReconcileSystems();
			pnu->ConnectSignals();
			ether.AddAtom(pnu);

			// New atom is ready for normal thread processing. This must be done before loading subatoms.
			// The only state we preserve on spacecraft is the Dormant state. All others are expected to load in Normal state.
			if(atomState == Dormant)
				pnu->SetDormant();
			else
				pnu->SetNormal();

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			DaAnimal::LoadSubatoms(ether, db, pnu->Id());
			DaBd::LoadSubatoms(ether, db, pnu->Id());
			DaSc::LoadSubatoms(ether, db, pnu->Id());
			DaVehicle::LoadSubatoms(ether, db, pnu->Id());
			DaItem::LoadSubatoms(ether, db, pnu->Id());
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
			DaDesigner::LoadSubatoms(ether, db, pnu->Id());
			DaNpc::LoadSubatoms(ether, db, pnu->Id());

			if(g_Au.IsSceneWorker())
			{
				// Insure the local application has a copy of my design loaded.
				// This is a soft request, only loading the design if it's not already loaded.
				pnu->DesignGeometryNeeded();

				// If the ship is dormant, set all subatoms to be dormant.
				if(atomState == Dormant)
				{
					DaAtomList shipAtoms;
					pnu->ListNormalSubatomsRecursive(shipAtoms);
					for(DaAtomList::const_iterator ai = shipAtoms.begin(); ai != shipAtoms.end(); ai++)
						(*ai)->SetDormant();
				}
				else
				{
					// This ship could be emerging in a system that was handed off to another server. The solar system may have
					// just been created, either by us moving through a wormhole or making the long haul through space.
					DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindAtom(parentId));
					if(system)
					{
						// This ship may be emerging from a wormhole. It needs to be kicked into motion so things happen the way
						// are supposed to. Otherwise, this ship sits at the center of the solar system until someone moves it.
						double mydist = myloc.Length();
						if(FuzzyEquals(mydist,1) || FuzzyEquals(mydist,2))
							pnu->ForceMotion();	// Ship is emerging from a wormhole.
					}
				}
			}
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

int DaSc::LowestDeck() const
{
	return 0;
}

float DaSc::ManeuverAcceleration() const
{
	unsigned int hitsLeft = GetPowerConsumption(SYS_ManeuverDrive);
	float mass = Mass();		// My mass includes stuff aboard.

	// Determine gravity drive acceleration modifier due to damage, on the scale of 0 to 1, where 1 is full health.
	m_DesignLock.lockForRead();
	float dmg = (float)hitsLeft / (float)m_Design.ManeuverDriveHits();
	if(dmg < 0 || dmg > 1)
	{
		// The ship 9087448 has one too many hit points. Alert of any others.
		if(!Is(9087448))
			qDebug() << "DaSc::ManeuverAcceleration" << *this << "hitsLeft" << hitsLeft << "drives" << m_Design.ManeuverDriveHits()
				<< "dmg" << dmg;
		dmg = 1;
	}

	// The mass of subordinate spacecraft affects overall maneuver acceleration to slow down those motherships.
	float massFactor = mass / m_Design.MassLbs();

	float v = m_Design.ManeuverAcceleration() * dmg / massFactor;
	m_DesignLock.unlock();
	return v;
}

float DaSc::Mass() const
{
	m_DesignLock.lockForRead();
	float v = m_Design.MassLbs();	// + MassAboard();
	m_DesignLock.unlock();
	return v;
}

float DaSc::MaxAtmosphereSpeed(const DaWorld* planet) const
{
	if(!planet)
		return SpeedOfLight();
	float p = planet->MSLAtmosphereDensity();
	if(p == 0)
		return SpeedOfLight();
	p /= 10.;
	float C = DragCoefficient();
	float A = CrossSectionalArea().y;

	m_DesignLock.lockForRead();
	float maxDrag = m_Design.MaxDrag();
	m_DesignLock.unlock();

	float vxv = maxDrag / (A * .5 * C * p);
	float maxSpeed = sqrtf(vxv);
	return maxSpeed;
}

int DaSc::MaxAtmosphereThrottle() const
{
	// Determine drag at maximum allowable speed in the atmosphere.
	// Streamlining changes the maximum drag the hull can withstand.
	m_DesignLock.lockForRead();
	float maxDrag = m_Design.MaxDrag();
	m_DesignLock.unlock();

	// Spacecraft factors.
	float fAccel = ManeuverAcceleration();		// Current max engine acceleration, modified by damage and other factors.
	float maxThrustFactor = maxDrag / fAccel;	// Max fraction of thrust to reach drag at maximum speed.

	// Determine highest throttle setting that does not exceed max drag thrust factor.
	const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};	// Throttle settings.
	int maxThrottle = 10;
	for(; maxThrottle > 1; maxThrottle--)
		if(engineThrust[maxThrottle] <= maxThrustFactor)
			break;								// Found the maximum possible throttle setting.

	// Back off by one throttle click to be safe.
	if(maxThrottle > 1)
		return maxThrottle - 1;

	// Return minimum throttle setting.
	return 1;
}

float DaSc::MaxBayWeaponRange() const
{
	m_DesignLock.lockForRead();
	float maxRange = 0;
	for(AuScReportSysVolHash::const_iterator bit = m_Design.WeaponBays().cbegin(); bit != m_Design.WeaponBays().cend(); bit++)
	{
		float range = m_Design.WeaponBayRange(bit.key());
		if(range > maxRange)
			maxRange = range;
	}
	m_DesignLock.unlock();
	return maxRange;
}

int DaSc::MaxCapacitorWarpFactor() const
{
	quint8 ftlDriveTL = SystemQuality(SYS_FTLDrive, FTLWarpDrive());
	COMMODITYID ftlDriveMod = SystemModule(SYS_FTLDrive, FTLWarpDrive());
	m_DesignLock.lockForRead();
	int warpFactor = AuScDesign::MaxCapacitorFTLSetting(ftlDriveMod, ftlDriveTL, m_Design.FTLDriveUnitsToEngageCuFt(),
			CapacitorLevel());
	m_DesignLock.unlock();
	return warpFactor;
}

float DaSc::MaxTransporterRange() const
{
	// Insure the ship has some capacitor power.
	if(CapacitorLevel() < 1)
		return 0;

	// Find the longest range of the functioning transporter rooms.
	float maxRange = 0;
	QList<quint16> troomIds(TransporterRoomIds());
	for(QList<quint16>::const_iterator ti = troomIds.cbegin(); ti != troomIds.cend();  ti++)
	{
		if(GetPowerConsumption(SYS_Transporter, *ti))
		{
			// The transporter has at least one hit point remaining; that's good enough.
			// This ignores the possibility that the console might not work.
			float range = TransporterRange(*ti);
			if(range > maxRange)
				maxRange = range;
		}
	}
	return maxRange;
}

bool DaSc::MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs)
{
	Q_UNUSED(elapsedMs);
	// Determine the destination point of the move.
	AuPoint foldloc(pMove->Location());
	AuPoint fnuloc = foldloc + vMove;
	FilterDestinationLoc(pMove, foldloc, fnuloc);

	// Determine the result of attempting to move to the nu location.
	DaAtom *pHit = NULL;
	AuPoint hitLoc, hitNormal;
	COLLISION_TYPE eCollide = pMove->CheckCollision(foldloc, fnuloc, pHit, hitLoc, hitNormal);
	pMove->SetMoveResult(eCollide);

	switch(eCollide)
	{
	case CT_HitParent:
		// The moving sub atom hit this spacecraft.
		// Don't bounce if the atom hit the floor and gravity is working.
		if(Gravity(0) == 0 || !hitNormal.FuzzyEquals(pMove->Transform().ProjectVector(GravityNormal())))
		{
			// Bounce.
			// Calculate surface normal relative to the moving atom.
			hitNormal = pMove->Transform().UnProjectVector(hitNormal);
			pMove->MovementBlocked(this, hitLoc, hitNormal, false);
		}
		return false;
	case CT_HitPeer:
	case CT_HitPeerShield:
		{
			// Reflect its movement vector.
			// Calculate surface normal relative to the hit atom's parent, which is this ship.
			hitNormal = pHit->Transform().ProjectVector(hitNormal);

			// Calculate surface normal relative to the moving atom.
			hitNormal = pMove->Transform().UnProjectVector(hitNormal);
			pMove->MovementBlocked(pHit, hitLoc, hitNormal, eCollide == CT_HitPeerShield);
		}
		return false;
	case CT_JoinedPeer:
		// Move the atom subordinate to its nu parent.
		if(pHit)
		{
			// Change the world transformation to be subordinate to the nu
			// parent atom.
			AuMatrix4x4 tBody;
			tBody.Translate(fnuloc);
			tBody.Rotate(pMove->Rotation());
			AuMatrix4x4 t = pHit->Transform().Inverse() * tBody;

			// Combine the movement vectors.
			AuPoint nuMove(pMove->Vector());
			if(!pHit->Vector().IsZero())
				nuMove -= pMove->Transform().UnProjectVector(pHit->Vector());
			pMove->ForcePosMove(AuAtomPos(pHit->Id(), t), nuMove);
			pMove->SetModified();
			pMove->InvalidateEnvironmentRecursive();	// Refresh environment on atom when it moves, in this thread.
		}
		return true;
	case CT_Miss:
		{
			// The moving atom did not hit anything.
			// Give the atom an opportunity to orient to the terrain.
			bool bCombine = true;
			if(pMove->Orient(fnuloc, pMove->Transform().ProjectVector(pMove->GravityNormal())))
				bCombine = false;
			if(bCombine && !fnuloc.FuzzyEquals(foldloc))
			{
				// Move the atom to the nu location relative to 'this' atom.
				AuMatrix4x4 tBody;
				tBody.Translate(fnuloc-foldloc);
				tBody *= pMove->Transform();
				pMove->ForcePos(AuAtomPos(Id(), tBody));
				pMove->InvalidateEnvironmentRecursive();
			}

			// Check to see if the nu location is outside the scope of influence of this atom.
			if(CheckChildExit(foldloc, fnuloc, pMove))
			{
				// Move the atom to be subordinate to my parent atom.
				// Separate the movement vectors.
				AuPoint nuMove = pMove->Vector() + pMove->Transform().UnProjectVector(Vector());

				// Change the world transformation to be subordinate to the nu parent atom.
				AuMatrix4x4 t = Transform() * pMove->Transform();
				pMove->ForcePosMove(AuAtomPos(ParentId(), t), nuMove);

				// Immediately try moving into the next atom domain.
				if(pMove->IsNormal())
					Parent()->MoveSubatom(pMove, AuPoint(0,0,0), 0);
			}
			else
				pMove->InvalidateEnvironmentRecursive();	// Refresh environment on atom when it moves, in this thread.
			pMove->SetModified();
			return true;
		}
		break;
	}
	return false;
}

unsigned int DaSc::OfficerBerths() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.OfficerBerths();
	m_DesignLock.unlock();
	return v;
}

int DaSc::OnDesignDeck(const AuPoint& loc) const
{
	Q_UNUSED(loc);
	return 0;
}

bool DaSc::OpenDoor(unsigned int doorIndex)
{
	// Get lift id, if door index is a lift door.
	quint16 liftId = 0;
	bool liftCanMove = false;
	bool liftDoorBlocked = false;
	m_DesignLock.lockForWrite();
	if(m_DoorStates.IsTurboLiftDoor(doorIndex))
	{
		liftId = m_DoorStates.DoorAssociatedPart(doorIndex);
		liftCanMove = m_LiftChg.ElapsedSec() >= 10;				// Turbo lift cannot do anything for ten seconds after moving.
		liftDoorBlocked = AnyTurboLiftDoorsBlockedOpen(liftId);	// Insure all doors associated with the same lift are closed.
	}

	if(!liftId)
	{
		// Open the door.
		bool doorOpened = m_DoorStates.OpenDoor(doorIndex);
		if(doorOpened)
		{
			m_DoorChg.Set();
			m_DoorOpenTimes.insert(doorIndex, AuTime::Now());
			SetModified();
		}
		m_DesignLock.unlock();
		return doorOpened;
	}
	m_DesignLock.unlock();
	if(!liftCanMove || liftDoorBlocked)
		return false;

	// Turbo lift must lock the movement of its contents before locking the design.
	DaAtomList lockedAtoms;
	LockLiftContents(liftId, lockedAtoms);

	m_DesignLock.lockForWrite();
	if(!m_DoorStates.CloseAssociatedDoors(liftId))
	{
		// A turbo lift door is stuck open.
		m_DoorChg.Set();		// In case any of the doors closed.
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		SetModified();
		return false;
	}

	// Insure the turbo lift is at the stop.
	AuLiftHash::iterator li = m_Lifts.find(liftId);
	AuScReportLiftHash::const_iterator liftStops = m_Design.TurboLifts().find(liftId);
	if(li == m_Lifts.end() || liftStops == m_Design.TurboLifts().cend())
	{
		// Wut?
		m_DoorChg.Set();		// In case any of the doors closed.
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		SetModified();
		return false;
	}

	QString nuStop = (*liftStops).DoorStopName(doorIndex);
	bool doorOpened = MoveLift(liftId, lockedAtoms, nuStop, doorIndex);	// MoveLift calls m_DesignLock.unlock()
	UnlockLiftContents(lockedAtoms);
	return doorOpened;
}

bool DaSc::OpenLauncherDoors(quint16 launcherId)
{
	m_DesignLock.lockForWrite();
	bool doorOpened = m_DoorStates.OpenAssociatedDoors(launcherId);
	if(doorOpened)
	{
		m_DoorChg.Set();
		SetModified();
	}
	m_DesignLock.unlock();
	return doorOpened;
}

bool DaSc::CloseLauncherDoors(quint16 launcherId)
{
	m_DesignLock.lockForWrite();
	bool allClosed = m_DoorStates.CloseAssociatedDoors(launcherId);
	m_DoorChg.Set();
	m_DesignLock.unlock();
	SetModified();
	return allClosed;
}

double DaSc::OrientationSpeed() const
{
	return PI/5;
}

unsigned int DaSc::PassengerBerths() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.PassengerBerths();
	m_DesignLock.unlock();
	return v;
}

double DaSc::PowerNeededToEngageFTLDrive(quint16 whichFTL, quint8 driveSetting) const
{
	m_DesignLock.lockForRead();
	double v = m_Design.PowerNeededToEngageFTLDrive(whichFTL, driveSetting);
	m_DesignLock.unlock();
	return v;
}

float DaSc::PowerPlantUnits() const
{
	m_DesignLock.lockForRead();
	float v = m_Design.PowerPlantUnits();
	m_DesignLock.unlock();
	return v;
}

void DaSc::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, const AuGearPos& gearPos,
		qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed)
{
	Q_UNUSED(gearPos);
	Q_UNUSED(chSelValue);
	Q_UNUSED(msElapsed);
	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_OperateTurboLift:
			if(cc != CONTROL_Server)
			{
				// This requires a user interface object. That must be provided by the client application.
				emit OperateTurboLift(Id(), sceneAtom->Id(), (quint16)ci.PanelId());
			}
			break;

		case CI_OpenCloseDoor:
			if(cc != CONTROL_Client)
				ToggleDoorState(*sceneAtom, ci.PanelId());
			break;
		case CI_LockUnlockDoor:
			if(cc != CONTROL_Client)
				ToggleDoorLock(*sceneAtom, ci.PanelId());
			break;
		}
	}
}

quint32 DaSc::PowerPlantHits() const
{
	m_DesignLock.lockForRead();
	quint32 v = m_Design.PowerPlantHits();
	m_DesignLock.unlock();
	return v;
}

quint8 DaSc::RemoveComponent(ROOMTYPE inRoomType, quint16 sightedPart, COMMODITYID cid, AuSysId& sysIdAffected, int& status)
{
	sysIdAffected = SightedSystem(sightedPart);
	if(sysIdAffected.System() == SYS_Count)
	{
		status = 1;	// invalid 'sightedPart'
		return 0;
	}
	COMMODITYID curModule = SystemModule(sysIdAffected.System(), sysIdAffected.Which());
	if(sysIdAffected.System() == SYS_Hull)
	{
		if(HullModuleMaterial(curModule) != cid)
		{
			status = 2;	// invalid component for system
			return 0;
		}

		// Hull parts includes fuel and cargo hold.
		switch(inRoomType)
		{
		case ROOM_Hold:
			sysIdAffected = SYS_Hold;
			break;
		case ROOM_EngineRoom:
		case ROOM_Fuel:			// really?
			sysIdAffected = SYS_Fuel;
			break;
		default:
			break;
		}
	}
	else
	{
		if(curModule != cid)
		{
			status = 2;	// invalid component for system
			return 0;
		}
	}

	quint32 maxComponents = SystemComponents(sysIdAffected, cid);	// Gets all units of metal for hull.
	if(maxComponents == 0)
	{
		status = 2;	// invalid component for system
		return 0;
	}

	quint32 partsDamaged;
	m_DamageLock.lockForRead();
	if(sysIdAffected.System() == SYS_VehicleLanding && sysIdAffected.Which() == 0)
	{
		// Non specific vehicle landing system happens when spotted on a parking spot. We should remove a component from any
		// landing system that can provide one.
		partsDamaged = 0;
		m_DesignLock.lockForRead();
		for(QSet<quint16>::const_iterator li = m_Design.VehicleLandingSystems().cbegin();
				li != m_Design.VehicleLandingSystems().cend(); li++)
		{
			AuSysId vlid(SYS_VehicleLanding, *li);
			quint32 dmg = m_Damage.PartsDamaged(vlid, cid);
			if(dmg)
				partsDamaged += dmg;	// Tally total damage.
			else if(!sysIdAffected.Which())
				sysIdAffected = vlid;	// Remember the first undamaged system so we can take a part from it.
		}
		m_DesignLock.unlock();
	}
	else
		partsDamaged = m_Damage.PartsDamaged(sysIdAffected, cid);
	m_DamageLock.unlock();

	if(partsDamaged >= maxComponents)
	{
		status = 3;	// no more components to remove
		return 0;	// Cannot remove any more of that component from the system.
	}
	else if(partsDamaged == maxComponents-1)
		status = 4;	// Last system component about to be removed.
	else
		status = 0;	// Removed part ok.

	// Remove a component from the appropriate system.
	quint8 quality = 0;
	m_DamageLock.lockForWrite();
	m_Damage.AddPartDamage(sysIdAffected, cid, 1);
	m_DamageLock.unlock();
	m_SystemsLock.lockForRead();
	quality = m_ssaShipSystems.SystemQuality(sysIdAffected);
	m_SystemsLock.unlock();
	DamageStateChanged();
	return quality;
}

void DaSc::RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
		POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
		const AuMatrix4x4& tDevOrtho, float hudIntensity)
{
	//qDebug() << "DaSc::RenderBodyStats" << Id() << Name();
	Q_UNUSED(sceneAtom);
	Q_UNUSED(tDevPersp);
	Q_UNUSED(tDevOrtho);
	Q_UNUSED(hudIntensity);
	if(!IsDesignDataCacheValid())
		return;

	double windowAspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());
	const float barspacing = .005f * g_Au.BodyStatSize()/100.f;

	if(isTarget)
	{
		// Determine which system is targetted.
		// Doors will return SYS_Count, so every enum value is valid. This ship cannot be targetted without
		// targetting a specific system.
		AuSysId sysId = SightedSystem(targetPart);

		// One horizontal bar for system stat.
		const float barwid = wid;
		const float barht = ht * .75f;
		left = 0;
		bottom = 1.f - barht - .01f;

		// System that is targetted.
		float iconleft = left - barht*1.5f;
		if(sysId.System() == SYS_FTLDrive && DbCommodity::IsWarpFTLDriveModule(SystemModule(sysId.System(), sysId.Which())))
			ari.RenderSystemIcon(SYS_Count+1, iconleft, bottom, barht, 1,1,1,1);	// Warp FTL drive icon.
		else if(sysId.System() == SYS_LifeSupport && sysId.Which() != 0)
			ari.RenderSystemIcon(SYS_Count+2, iconleft, bottom, barht, 1,1,1,1);	// Berth icon.
		else
			ari.RenderSystemIcon(sysId.System(), iconleft, bottom, barht, 1,1,1,1);

		// Hits Points
		float curhp = SystemHitsRemaining(sysId);
		float maxhp = SystemHits(sysId);
		float frac = curhp/maxhp;

		if(frac <= .1f || IsAnyConditionVisible())
			ari.RenderBar(left,bottom, barwid,barht, frac, 0, 1.f,.2f,.2f, 1.f/2.f,.2f/2.f,.2f/2.f);	// red/red
		else if(frac < .5f)
			ari.RenderBar(left,bottom, barwid,barht, frac, 0, 1.f,.7f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// amber/amber
		else
			ari.RenderBar(left,bottom, barwid,barht, frac, 0, .2f,1.f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// green/amber

		{
			// Name is also the political stance indicator, by color.
			QColor col = PoliticalStanceColor(stance, IsCivilian());

			// Name
			// Calculate pixel location of text.
			QRect rcClient = ari.glw->rect();
			float namebot = bottom - barspacing;
			float nameleft = (iconleft + ((left - iconleft) + barwid)/2.f) * windowAspect;	// Centered under icon and bar.

			int nx = rcClient.width()/2 + (int)(nameleft * (float)rcClient.width()/2.f);
			int ny = rcClient.height()/2 - (int)(namebot * (float)rcClient.height()/2.f);
			if(sysId.System() == SYS_Hull)
				ari.RenderText(nx, ny, NameWithEmpire(), col, Qt::AlignHCenter|Qt::AlignTop);
			else if(sysId.System() == SYS_LifeSupport && sysId.Which() != 0)
			{
				const AuRoster& roster = LockRoster();
				QString occ = roster.BerthDesc(sysId.Which());
				UnlockRoster();
				if(occ.isEmpty())
				{
					// Berth was not found, likely vacant.
					const AuScDesign* scd = LockDesign();
					AuScParts::const_iterator pi = scd->Parts().find(sysId.Which());
					if(pi != scd->Parts().cend())
					{
						AuRosterEntry::TYPE vacantType;
						switch((*pi)->StationType())
						{
						case AuStation::Berth_Captain:
							vacantType = AuRosterEntry::Captain;
							break;
						case AuStation::Berth_Crew:
							vacantType = AuRosterEntry::Crew;
							break;
						case AuStation::Berth_Officer:
							vacantType = AuRosterEntry::OfficerAI;
							break;
						case AuStation::Berth_Passenger:
							vacantType = AuRosterEntry::PassengerAI;
							break;
						case AuStation::Berth_Troop:
							vacantType = AuRosterEntry::Troop;
							break;
						default:
							vacantType = AuRosterEntry::Invalid;
							break;
						}
						if(vacantType != AuRosterEntry::Invalid)
						{
							AuRosterEntry vacantEntry(vacantType, sysId.Which(), AuId(), QString());
							occ = vacantEntry.BerthDesc();
						}
					}
					UnlockDesign();
				}
				ari.RenderText(nx, ny, occ, col, Qt::AlignHCenter|Qt::AlignTop);
			}
			else
				ari.RenderText(nx, ny, SystemName(sysId), col, Qt::AlignHCenter|Qt::AlignTop);
		}
		return;
	}

	// I am not the target so I am the parent atom of the scene atom.
	if(sceneAtom->IsTurretPose() && sceneAtom->Ordnance())
	{
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
		const float barht = (ht - barspacing)/2;

		// Hits Points of the turret itself.
		float valbot = bottom;
		{
			AuSysId sysId(sceneAtom->StationSysId());
			float curhp = GetPowerConsumption(sysId);
			float maxhp = SystemHits(sysId);
			float frac = curhp/maxhp;
			if(frac <= .1f || IsAnyConditionVisible())
				ari.RenderBar(left,valbot, barwid,barht, frac, 0, 1.f,.2f,.2f, 1.f/2.f,.2f/2.f,.2f/2.f);	// red/red
			else if(frac < .5f)
				ari.RenderBar(left,valbot, barwid,barht, frac, 0, 1.f,.7f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// amber/amber
			else
				ari.RenderBar(left,valbot, barwid,barht, frac, 0, .2f,1.f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// green/amber

			valbot += barspacing + barht;
		}

		// Guns/Weapons
		{
			AuMatrix4x4 projection;
			projection.Scale(windowAspect, 1, 1);

			// Calculate square icon size.
			const float size = 64.f/1024.f * (float)g_Au.BodyStatSize()/100.f;

			ari.re->GLSL().TextureShader().Select();
			ari.re->GLSL().TextureShader().SetColor(1,1,1,1);

			// Gun Type
			float gunleft = left;
			float guncountleft = gunleft;
			AuMatrix4x4 mvguns;
			mvguns.Translate(gunleft, valbot, 0);
			mvguns.Scale(size, size, 1);

			if(sceneAtom->Ordnance()->CommodityId() != CID_Invalid)
			{
				// Load stock texture for the commodity. AuView::initializeGL creates them.
				GLuint glTexture = ari.CommodityTextureF(sceneAtom->Ordnance()->CommodityId());

				ari.re->GLSL().TextureShader().SetProjectionModelView(projection * mvguns);
				ari.re->GLSL().TextureShader().SetTexture(glTexture);

				m_WeaponVAO.SelectGL();
				m_WeaponVAO.DrawGL();
				m_WeaponVAO.DeselectGL();
				gunleft += size + barspacing;
			}

			ari.re->GLSL().TextureShader().Deselect();

			// Rounds counters use a different shader.
			if(sceneAtom->Ordnance()->CommodityId() != CID_Invalid)
				ari.RenderNumber(sceneAtom->Ordnance()->Count(), guncountleft, valbot + size + barspacing, size/2.f,
						0,false, .2f,1.f,.2f,1);

			// Gun Effects are always valid.
			{
				AuCombatWeapon w(sceneAtom->Ordnance(), 1);
				if(w.m_w1.m_Effect != WE_None)
				{
					ari.RenderWeaponEffect(w.m_w1.m_Effect, gunleft, valbot, size, 1,1,1,1);
					gunleft += size + barspacing;
				}
				if(w.m_w2.m_Effect != WE_None)
					ari.RenderWeaponEffect(w.m_w2.m_Effect, gunleft, valbot, size, 1,1,1,1);
			}
		}

		// Locked-on Blip
		if(!IsInSpacecraft() && DbCommodity::IsMissile(sceneAtom->Ordnance()->CommodityId()) && PowerPlantWorking())
		{
			// Current ordnance is guided and the HUD is visible.
			AuId gunnerWorldId;
			int gunnerDomain = GunnerDomain(gunnerWorldId);

			DaAtom* system = FindEnvironmentAtom();
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
					const float size = 64.f/1024.f * (float)g_Au.BodyStatSize()/100.f;

					// HUD colors
					float r = 213.f/255.f;
					float g = 246.f/255.f;
					float b = 213.f/255.f;
					float a = (float)hudIntensity / 10.f;

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
	}
	else if(sceneAtom->IsFireControlStationPose() && sceneAtom->Ordnance())
	{
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

		// Hits Points of the weapon system itself.
		float valbot = bottom;
		{
			AuSysId sysId(SYS_Weapon, sceneAtom->ControlId());
			float curhp = GetPowerConsumption(sysId);
			float maxhp = SystemHits(sysId);
			float frac = curhp/maxhp;
			if(frac <= .1f || IsAnyConditionVisible())
				ari.RenderBar(left,valbot, barwid,barht, frac, 0, 1.f,.2f,.2f, 1.f/2.f,.2f/2.f,.2f/2.f);	// red/red
			else if(frac < .5f)
				ari.RenderBar(left,valbot, barwid,barht, frac, 0, 1.f,.7f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// amber/amber
			else
				ari.RenderBar(left,valbot, barwid,barht, frac, 0, .2f,1.f,.2f, 1.f/2.f,.7f/2.f,.2f/2.f);	// green/amber

			valbot += barspacing + barht;
		}

		// Guns/Weapons
		{
			AuMatrix4x4 projection;
			projection.Scale(windowAspect, 1, 1);

			// Calculate square icon size.
			const float size = 64.f/1024.f * (float)g_Au.BodyStatSize()/100.f;

			ari.re->GLSL().TextureShader().Select();
			ari.re->GLSL().TextureShader().SetColor(1,1,1,1);

			// Gun Type
			float gunleft = left;
			float guncountleft = gunleft;
			AuMatrix4x4 mvguns;
			mvguns.Translate(gunleft, valbot, 0);
			mvguns.Scale(size, size, 1);

			if(sceneAtom->Ordnance()->CommodityId() != CID_Invalid)
			{
				// Load stock texture for the commodity. AuView::initializeGL creates them.
				GLuint glTexture = ari.CommodityTextureF(sceneAtom->Ordnance()->CommodityId());

				ari.re->GLSL().TextureShader().SetProjectionModelView(projection * mvguns);
				ari.re->GLSL().TextureShader().SetTexture(glTexture);

				m_WeaponVAO.SelectGL();
				m_WeaponVAO.DrawGL();
				m_WeaponVAO.DeselectGL();
				gunleft += size + barspacing;
			}

			ari.re->GLSL().TextureShader().Deselect();

			// Rounds counters use a different shader.
			if(sceneAtom->Ordnance()->CommodityId() != CID_Invalid)
			{
				int roundsCount = sceneAtom->Ordnance()->Count();
				ari.RenderNumber(roundsCount, guncountleft, valbot + size + barspacing, size/2.f, 0,false, .2f,1.f,.2f,1);
			}

			// Gun Effects are always valid.
			{
				AuCombatWeapon w(sceneAtom->Ordnance(), 1);
				if(w.m_w1.m_Effect != WE_None)
				{
					ari.RenderWeaponEffect(w.m_w1.m_Effect, gunleft, valbot, size, 1,1,1,1);
					gunleft += size + barspacing;
				}
				if(w.m_w2.m_Effect != WE_None)
					ari.RenderWeaponEffect(w.m_w2.m_Effect, gunleft, valbot, size, 1,1,1,1);
			}
		}
	}
}

void DaSc::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	if(g_Au.m_DebugSwitches & 0x00000010)
		return;
	// Do not render distant spacecraft.
	if(pass != PASS_Near)
		return;

	//qDebug() << "DaSc::RenderEnvironmentObject" << Id() << Name();
	// Determine if the scene atom is aboard this spacecraft.
	const AuMatrix4x4& tThis = rmod.Matrix(Id());
	bool insideVisible = false;
	bool isAboard = false;
	if(sceneAtom->IsInSpacecraft())
	{
		// Scene atom is aboard a spacecraft.
		if(sceneAtom->Parent() == this)
			isAboard = true;	// Scene atom is aboard this spacecraft.
		else
		{
			// Scene atom parent is not this spacecraft but it could be inside a subordinate spacecraft or vehicle.
			const DaVessel* scenepar = sceneAtom->FindParentVesselInEnv();
			if(scenepar == this)
				isAboard = true;	// Scene atom could be inside a vehicle or riding an animal.
			else if(scenepar && scenepar->FindParentVesselInEnv() == this)
				isAboard = true;	// Scene atom is aboard a subordinate spacecraft.
		}
	}

	// Decide now if the inside of this ship is visible to the scene
	// atom. It saves us a lot of work.
	bool rendership;
	if(isAboard)
	{
		// Scene atom is inside this spacecraft so inside is visible.
		insideVisible = true;
		rendership = true;

		// Scene atom is aboard and in top-down view; render tactical view if the viewDeck is within the hull extents.
		// Cannot get tactical view at a game table or unless scene atom is on the crew.
		//if(ari.IsViewTopDown() &&
				//ari.viewDeck >= m_DesignAnalysis.m_LowestDeck && ari.viewDeck <= m_DesignAnalysis.m_HighestDeck &&
				//!sceneAtom->IsGameTablePose() && (IsCrewOrTroop(sceneAtom->Id()) || InChainOfCommand(sceneAtom)))
			//renderTactical = true;
	}
	else
	{
		// Scene atom is not aboard this spacecraft. Cull if possible.
		const AuPoint& pv = rmod.ViewLoc(Id());
		double dist2 = pv.Length2();	// length squared
		AuPoint hMax,hMin;
		m_Design.Extents(hMax, hMin);
		double diag = hMin.Distance(hMax);
		double diag2 = diag * diag;	// diagonal distance, squared
		if(pv.z >= 0 && dist2 > diag2)
		{
			// this ship is behind the camera plane and more than 1/2 its extents away from the camera.
			insideVisible = false;
			rendership = false;
		}
		else
		{
			// This ship is ahead of the camera or within 1/2 its extents of the camera.
			double insideDist = diag + 250*FeetPerMeter();
			if(dist2 < insideDist*insideDist)
			{
				// This ship is close enough to see the most detail.
				insideVisible = true;
				rendership = true;
			}
			else
			{
				// The ship is far enough away that the interior is not visible.
				insideVisible = false;
				rendership = true;
			}
		}
	}

	// Don't render spacecraft in top-down view or when it has been culled.
	if(!rendership)
	{
		// Render nav beacon for stations that are not crashed on the ground.
		if(ari.m_ShowHUD && ari.textBrightness && IsSpaceStation() && !IsInAtmosphere())
			RenderStationNavBeacon(projection, tThis, sceneAtom, ari.textBrightness, *ari.re);

		// OpenAL audio sources
		RenderAudioSources(sceneAtom, tThis, ari.al, ari.fade);

		// Base atom class needs to render special effects.
		DaVessel::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);
		return;
	}

	// Render again in RenderLast for shield and engine contrail effects.
	renderLast.Insert(rmod, this);

	// Render solid parts. Transparent parts are saved for RenderLast.
	RenderDesign(tThis, ari, sceneAtom, false, isAboard, insideVisible, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
			shadScRingworld, shadScRingworldFog);

	// Disable my own external light when rendering myself. To improve this, the interior parts could be rendered with the exterior
	// light off and the exterior parts could be rendered with the exterior light on.
	ari.re->DisableAtomLights(Id());

	// Render unmanned consoles.
	// When manned, the creature provides the console model.
	if(insideVisible)
	{
		ari.re->SetFog(0, 0, 0, 0);

		// List the manned consoles so we can skip them.
		// This bit of code only renders the unmanned consoles.
		QSet<quint16> mannedCons;
		ListVisiblyOccupiedConsoles(sceneAtom, mannedCons);

		m_DesignLock.lockForRead();
		const AuScDesign* scd = LockDesign();
		if(scd)
		{
			// Captain's chairs.
			for(AuScReportStationHash::const_iterator si = m_Design.CommandStations().cbegin();
					si != m_Design.CommandStations().cend(); si++)
			{
				if(mannedCons.contains(si.key()))
					continue;	// Console is manned.

				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				switch((AuScPart::PART_TYPE)(*si).PartType())
				{
				case AuScPart::Part_ConsoleCommand:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_CmdEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_StationCommand:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScCommand::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				default:
					break;
				}
			}

			// Crew consoles, except transporters.
			for(AuScReportStationHash::const_iterator si = m_Design.CrewStations().cbegin();
					si != m_Design.CrewStations().cend(); si++)
			{
				if(mannedCons.contains(si.key()))
					continue;	// Console is manned.

				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				switch((AuScPart::PART_TYPE)(*si).PartType())
				{
				case AuScPart::Part_ConsoleEngineer:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_EngineEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_ConsoleFireControl:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_FireEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_ConsoleHelm:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_PilotEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_ConsoleMedic:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_SurgeryEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_ConsoleNavigator:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_NavEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_ConsolePowerRelay:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_PowerEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_ConsoleSensor:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_SensorEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_ConsoleShield:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_ShieldEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_StationEngineer:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScEngineer::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				case AuScPart::Part_StationFireControl:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScWeapon::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				case AuScPart::Part_StationHelm:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScHelm::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				case AuScPart::Part_StationMedic:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScSurgery::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				case AuScPart::Part_StationNavigator:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScNav::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				case AuScPart::Part_StationPowerRelay:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScPower::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				case AuScPart::Part_StationSensor:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScSensor::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				case AuScPart::Part_StationShield:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScShield::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				default:
					break;
				}
			}

			// Transporter consoles.
			for(AuScReportTransporterStationHash::const_iterator si = m_Design.Transporters().cbegin();
					si != m_Design.Transporters().cend(); si++)
			{
				if(mannedCons.contains(si.key()))
					continue;	// Console is manned.

				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(si.key());
				if(pi == scd->Parts().cend())
					continue;

				switch((AuScPart::PART_TYPE)(*si).PartType())
				{
				case AuScPart::Part_ConsoleTransporter:
					{
						// Console is unmanned.
						// Console model gets ambient lighting for the room.
						float ambientR, ambientG, ambientB;
						bool sunsBlocked;
						InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
						ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

						// Position the model according to up in the room.
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						m_TransEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
					}
					break;

				case AuScPart::Part_StationTransporter:
					{
						AuMatrix4x4 tcon(tThis * (*pi)->PositionMatrix());
						AuQuad screen;
						(*pi)->ScreenQuad(screen);
						DpScTransporter::RenderEmpty(projection, tcon, ari, *this, false, screen);
					}
					break;

				default:
					break;
				}
			}

			// Game tables.
			for(AuScReportGameTableHash::const_iterator gi = m_Design.GameTables().cbegin();
					gi != m_Design.GameTables().cend(); gi++)
			{
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(gi.key());
				if(pi == scd->Parts().cend())
					continue;

				// Console model gets ambient lighting for the room.
				float ambientR, ambientG, ambientB;
				bool sunsBlocked;
				InteriorAmbientLight((*pi)->GroupName(), ambientR, ambientG, ambientB, sunsBlocked);
				ari.re->SetAmbientLight(ambientR, ambientG, ambientB, 1);

				AuMatrix4x4 t(tThis * (*pi)->PositionMatrix());
				RenderGameTable(projection, t, *ari.re, AuPoint(0,0), 0, pi.key());
			}
		}
		UnlockDesign();
		m_DesignLock.unlock();
	}

	// Render nav beacon for stations that are not crashed on the ground.
	if(ari.m_ShowHUD && ari.textBrightness && IsSpaceStation() && !IsInAtmosphere())
		RenderStationNavBeacon(projection, tThis, sceneAtom, ari.textBrightness, *ari.re);

	// Render atoms subordinate to this ship. They are affected by the interior ambient lighting of the ship. What about suns? DHH
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);

	ari.re->EnableAtomLights(Id());

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, tThis, ari.al, ari.fade);

	// Base atom class needs to render special effects.
	DaVessel::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

void DaSc::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	//qDebug() << "DaSc::RenderLast" << Id() << Name();
	DaVessel::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);

	// Render transparent things that don't affect the depth mask, before rendering transparent windows.
	const AuMatrix4x4& myM = rmod.Matrix(Id());
	{
		RenderLightSprites(projection, myM, ari);

		if(!m_Contrails.empty())
		{
			// Gravity thrust renders points relative to the parent atom of this ship.
			const AuMatrix4x4& model = rmod.Matrix(ParentId());
			bool gDriveOn = GravityDriveWorking() && GravityDriveState() == GD_On;

			// Let's have no contrails when the warp tunnel is visible.
			if(HasWarpFTLDrive() && IsFTLDriveHot(FTLWarpDrive()) && Vector().Length() > SpeedOfLight()*.9)
				gDriveOn = false;

			// Render engine contrails.
			ari.re->glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
			ari.re->glEnable(GL_BLEND);
			ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
			for(QList<PsEngineTrail*>::iterator ci = m_Contrails.begin(); ci != m_Contrails.end(); ci++)
			{
				// When gravity drive is working and on, emit particles. Otherwise, stop emitting and the engine trail will fade
				// away so that you won't see anything when ships are parked.
				(*ci)->SetColor(m_ContrailColor);		// Keep this updated as it can change during play.
				if(gDriveOn)
					(*ci)->SetNumToRelease(1);
				else
					(*ci)->SetNumToRelease(0);
				(*ci)->SetParent(ParentId(), Transform());
				if((*ci)->Update())
					if((*ci)->GetParticleCount())
						(*ci)->Render(projection, model, *ari.re);
			}
			ari.re->glDisable(GL_BLEND);
			ari.re->glDepthMask(GL_TRUE);
		}

		// Render smoke and fire on burning/smoldering ships before shields.
		if(IsAnyConditionVisible())
		{
			AuMatrix4x4 cM(myM);
			cM.Translate(TargetLocation());
			float roughRadius = RoughRadius();
			if(IsBurning())
			{
				// Render fire with smoke on burning ships.
				RenderFire(projection, cM, *ari.re, FireRadius());
				RenderSmoke(projection, cM, *ari.re, roughRadius/2.);
			}
			else if(IsSmoldering())
				RenderSmoke(projection, cM, *ari.re, roughRadius/2.);

			if(HasAcid())
				RenderAcid(projection, cM, *ari.re, roughRadius/2);

			//if(HasDisease())
				//RenderDisease(projection, cM, *ari.re, roughRadius/2);

			if(IsParalyzed())
				RenderParalysis(projection, cM, *ari.re, roughRadius/2);

			//if(IsPoisoned())
				//RenderPoison(projection, cM, *ari.re, roughRadius/2);
		}
		else if(IsSmoldering())
		{
			AuMatrix4x4 cM(myM);
			cM.Translate(TargetLocation());
			RenderSmoke(projection, cM, *ari.re, RoughRadius()/2.);
		}
	}

	if(m_InsideVisible && m_LifeSupportStatus)
	{
		// Life Support Status displays.
		ari.re->SetFog(0, 0, 0, 0);
		m_DesignLock.lockForRead();
		const AuScDesign* scd = LockDesign();
		if(scd)
		{
			// Setup texture offset shader for additive blending.
			ari.re->glEnable(GL_BLEND);
			ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
			ari.re->glPolygonOffset(-2.f, 0);
			ari.re->glDepthFunc(GL_LEQUAL);					// Prepare to blend at the same Z-depth as the screen quad.
			ari.re->GLSL().TextureShader().Select();
			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * myM);
			ari.re->GLSL().TextureShader().SetTexture(m_LifeSupportStatusTex);
			ari.re->GLSL().TextureShader().SetColor(1,1,1,1);

			float s,t,u,v;	// Texture coords for the little screen, using LifeSupportStatusIndicators.png.
			const float maxx = 186;			// Pixel width of the texture.
			const float maxy = 100;			// Pixel height of the texture.
			const float twid = maxx / 6;	// Number of tiles across the texture.
			const float tht = maxy / 5;		// Number of tiles vertically in the texture.
			if(LifeSupportWorking())
			{
				// Life support system is working.
				int col;
				if(CapacitorLevel() > 0)
				{
					// Air pressure is being maintained.
					col = AuTime::CurrentSecCycle() * 4 + 1;	// Cycle through 4 columns, skipping the first column.
				}
				else
				{
					// Air pressure cannot be maintained.
					col = 5;									// Last column is the powerless column.
				}
				s = (col*twid)/maxx;
				u = ((col+1)*twid)/maxx;
			}
			else
			{
				// First column is the 'off' column.
				s = 0;
				u = twid/maxx;
			}

			for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
					di != m_Design.StatusDisplays().cend(); di++)
			{
				if((*di).PartType() != AuScPart::Part_StatusLifeSupport)
					continue;
				// Locate the corresponding part in the design.
				AuScParts::const_iterator pi = scd->Parts().find(di.key());
				if(pi == scd->Parts().cend())
					continue;
				AuQuad display;
				(*pi)->ScreenQuad(display);

				// Calculate row texture coords based on room air pressure.
				float roomAir = AirPressure((*di).RoomName());
				int row;		// Row in texture, counting up from the bottom row at 0.
				if(roomAir < .15f)
					row = 0;	// Air pressure is dangerously low.
				else if(roomAir < .45f)
					row = 1;	// Air pressure is ok, a little low.
				else if(roomAir < .55f)
					row = 2;	// Air pressure is ok.
				else if(roomAir <= .85f)
					row = 3;	// Air pressure is ok, a little high.
				else
					row = 4;	// Air pressure is dangerously high.
				t = (row*tht + .5f)/maxy;
				v = ((row+1)*tht - .5f)/maxy;

				QList<AuTriTex> tris;
				tris << AuTriTex(AuPointTex(display.vll.x, display.vll.y, display.vll.z, s,t),
						AuPointTex(display.vul.x, display.vul.y, display.vul.z, s,v),
						AuPointTex(display.vur.x, display.vur.y, display.vur.z, u,v));
				tris << AuTriTex(AuPointTex(display.vur.x, display.vur.y, display.vur.z, u,v),
						AuPointTex(display.vlr.x, display.vlr.y, display.vlr.z, u,t),
						AuPointTex(display.vll.x, display.vll.y, display.vll.z, s,t));

				AuGLVAO vao;
				vao.CreateForTexture(*ari.re, tris, GL_STREAM_DRAW);
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL();
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}

			// Restore rendering states.
			ari.re->GLSL().TextureShader().Deselect();
			ari.re->glDepthFunc(GL_LESS);
			ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
			ari.re->glDisable(GL_BLEND);
		}
		UnlockDesign();
		m_DesignLock.unlock();
	}

	// Render transparent parts.
	RenderDesign(myM, ari, sceneAtom, true, m_IsAboard, m_InsideVisible, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
			shadScRingworld, shadScRingworldFog);

	if(ari.m_ShowPaths)
		RenderPaths(projection, myM, ari);

	if(ShieldsWorkingAndUp())
	{
		// Shields add a render order complication. Things that might penetrate the shield must be rendered before and after the
		// shield bubble, so they are Z clipped properly by the shield.
		bool hasTractorBeam = RenderTractorBeams(projection, myM, *ari.re);
		bool hasLaunchers = RenderVehicleLaunchers(projection, myM, ari, true);
		bool hasLanders = RenderLandingSystems(projection, myM, ari, true);

		RenderShieldBubble(projection, myM, ari, sceneAtom);

		if(hasTractorBeam)
			RenderTractorBeams(projection, myM, *ari.re);
		if(hasLaunchers)
			RenderVehicleLaunchers(projection, myM, ari, false);
		if(hasLanders)
			RenderLandingSystems(projection, myM, ari, false);
	}
	else
	{
		RenderTractorBeams(projection, myM, *ari.re);
		RenderVehicleLaunchers(projection, myM, ari, true);
		RenderLandingSystems(projection, myM, ari, true);
	}
}

bool DaSc::RenderVehicleLaunchers(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, bool update)
{
	// Render active vehicle launchers.
	bool updated = false;
	bool hasLauncher = false;
	m_LauncherLock.lockForRead();
	for(AuLauncherStateHash::iterator li = m_LauncherStates.begin(); li != m_LauncherStates.end(); li++)
	{
		quint8 state = *li;
		if(!state)
			continue;	// Skip dormant landing system.
		AuMatrix4x4 launchPadM;
		AuPoint launchExit;
		const AuScDesign* scd = LockDesign();
		if(scd)
			scd->LaunchPosition(li.key(), launchPadM, launchExit);
		UnlockDesign();
		launchExit = launchPadM.UnProject(launchExit);	// Exit is now a vector relative to the launch pad.
		launchPadM.Translate(launchExit);				// Matrix origin is now at the launch exit.
		if(!m_LaunchSystem)
		{
			GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
			m_LaunchSystem = new PsLaunchSystem(texIdGlow, texIdStreak);
		}
		if(update && !updated)
		{
			m_LaunchSystem->Update();
			updated = true;
		}
		m_LaunchSystem->Render(projection, model*launchPadM, *ari.re);
		hasLauncher = true;
	}
	m_LauncherLock.unlock();
	return hasLauncher;
}

bool DaSc::RenderLandingSystems(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, bool update)
{
	// Render active vehicle landing systems.
	bool updated = false;
	bool hasLander = false;
	m_LandingSystemLock.lockForRead();
	for(AuLandingSystemStateHash::iterator li = m_LandingSystemStates.begin(); li != m_LandingSystemStates.end(); li++)
	{
		quint8 state = *li;
		if(!state)
			continue;	// Skip dormant landing system.
		AuMatrix4x4 landerPosM;
		const AuScDesign* scd = LockDesign();
		if(scd)
			scd->LandingSystemPosition(li.key(), landerPosM);
		UnlockDesign();
		if(!m_LandingSystem)
		{
			GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
			m_LandingSystem = new PsLandingSystem(texIdGlow, texIdStreak);
		}
		m_LandingSystem->SetNumber(li.key());
		if(update && !updated)
		{
			m_LandingSystem->Update();
			updated = true;
		}
		m_LandingSystem->Render(projection, model*landerPosM, *ari.re);
		hasLander = true;
	}
	m_LandingSystemLock.unlock();
	return hasLander;
}

quint32 DaSc::ReplaceComponent(ROOMTYPE inRoomType, unsigned int sightedPart, COMMODITYID cid, quint8 quality,
		unsigned int fixCount, AuSysId& sysIdRepaired, bool& systemModuleChange, bool forceUpgrade)
{
	sysIdRepaired = SightedSystem(sightedPart);
	if(sysIdRepaired.System() == SYS_Count)
		return 0;

	COMMODITYID curModule = SystemModule(sysIdRepaired.System(), sysIdRepaired.Which());
	bool systemModuleChanged = false;
	unsigned int fixedCount = 0;	// Number of commodities actually fixed.
	QSet<AuSysId> systemsRepaired;
	if(sysIdRepaired.System() != SYS_Hull)
	{
		if(curModule == cid)
		{
			// Repair the appropriate system.
			m_DamageLock.lockForWrite();
			for(unsigned int r=0; r<fixCount; r++)
				if(m_Damage.RepairParts(sysIdRepaired, cid, 1, systemsRepaired))
					fixedCount++;
				else
					break;
			m_DamageLock.unlock();

			// When cumulatively fixing the vehicle landing system(s), pick up the id of the one that actually got repaired.
			if(sysIdRepaired.System() == SYS_VehicleLanding && sysIdRepaired.Which() == 0 && !systemsRepaired.empty())
				sysIdRepaired = *systemsRepaired.cbegin();
		}
		else if(systemModuleChange && ScModuleSystem(cid) == sysIdRepaired.System())
		{
			// System module is permitted to change and the module system is compatible.
			// The system module can change if all prior modules were removed.
			quint32 maxComponents = SystemComponents(sysIdRepaired, cid);
			if(maxComponents)
			{
				m_DamageLock.lockForWrite();
				quint32 curdmg = m_Damage.PartsDamaged(sysIdRepaired, curModule);		// Number of old modules damaged.
				if(curdmg >= maxComponents || forceUpgrade)
				{
					// No more modules to remove. The system module can be upgraded.
					// This is a system upgrade, which means the system module is changing.
					quint32 netcurdmg;													// Number of damaged modules to put back.
					if(curdmg > fixCount)
					{
						netcurdmg = curdmg - fixCount;
						fixedCount = fixCount;
					}
					else
					{
						netcurdmg = 0;
						fixedCount = curdmg;
					}
					m_Damage.RepairParts(sysIdRepaired, curModule, curdmg, systemsRepaired);	// Remove old module damage.
					if(netcurdmg)
						m_Damage.AddPartDamage(sysIdRepaired, cid, netcurdmg);			// Replace it with cid module damage.
					m_DamageLock.unlock();
					systemModuleChanged = true;

					// Change the system module and quality.
					AuItem* item = AuItem::Create(cid, quality, 0, 1);
					if(item)
					{
						InstallSystemModule(sysIdRepaired, *item, 0);
						delete item;
					}
					SetSystemQuality(sysIdRepaired, quality);
					DamageStateChanged();
				}
				else
					m_DamageLock.unlock();
			}
		}
	}
	else
	{
		// Repair the hull.
		if(HullModuleMaterial(curModule) == cid)
		{
			// Hull damage includes fuel, cargo hold, and launch tubes otherwise there is no good way to fix them.
			SYSTEM prefSys;
			switch(inRoomType)
			{
			case ROOM_Hold:
				prefSys = SYS_Hold;
				break;
			case ROOM_EngineRoom:
			case ROOM_Fuel:			// really?
				prefSys = SYS_Fuel;
				break;
			default:
				prefSys = SYS_Hull;
				break;
			}

			m_DamageLock.lockForWrite();
			for(unsigned int r=0; r<fixCount; r++)
			{
				if(m_Damage.RepairParts(prefSys, cid, 1, systemsRepaired))
				{
					sysIdRepaired = prefSys;
					fixedCount++;
				}
				else if(prefSys != SYS_Hull && m_Damage.RepairParts(SYS_Hull, cid, 1, systemsRepaired))
				{
					sysIdRepaired = SYS_Hull;
					fixedCount++;
				}
				else if(prefSys != SYS_Fuel && m_Damage.RepairParts(SYS_Fuel, cid, 1, systemsRepaired))
				{
					sysIdRepaired = SYS_Fuel;
					fixedCount++;
				}
				else if(prefSys != SYS_Hold && m_Damage.RepairParts(SYS_Hold, cid, 1, systemsRepaired))
				{
					sysIdRepaired = SYS_Hold;
					fixedCount++;
				}
				else
					break;
			}
			m_DamageLock.unlock();
		}
	}

	// Don't finish a system upgrade by downgrading my system quality. :/
	if(fixedCount && !systemModuleChanged)
		DowngradeSystemModules(quality, systemsRepaired);
	systemModuleChange = systemModuleChanged;
	return fixedCount;
}

bool DaSc::RoughExtents(AuBox &fExt) const
{
	// Get extents of my design.
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	fExt.SetFromMinMax(min, max);

	// Include rough extents of subordinate spacecraft and vehicles that are not inside this spacecraft.
	AuBox subExt;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(dynamic_cast<DaVessel*>(pAtom))
		{
			// Spacecraft can hang out significantly, changing the extents even when 'inside'.
			pAtom->RoughExtents(subExt);
			subExt.Transform(pAtom->Transform());
			fExt.Union(fExt, subExt);
		}
		else if(dynamic_cast<DaVehicle*>(pAtom) && !pAtom->IsInsideSpacecraft())
		{
			// Vehicles only matter when they are not inside, as in sitting on top of the hull.
			pAtom->RoughExtents(subExt);
			subExt.Transform(pAtom->Transform());
			fExt.Union(fExt, subExt);
		}
	}
	return true;
}

bool DaSc::RoughExtentsWithShields(AuBox &fExt) const
{
	if(!DaVessel::RoughExtentsWithShields(fExt))
		return false;
	if(!ShieldsBlockWeapons())
		return true;

	// Include extents of my shields.
	AuPoint shieldRadii(ShieldRadii());
	AuPoint cen(DesignCenterOffset());
	AuPoint smin(cen - shieldRadii);
	AuPoint smax(cen + shieldRadii);
	AuBox sExt;
	sExt.SetFromMinMax(smin, smax);
	fExt.Union(fExt, sExt);
	return true;
}

double DaSc::RoughRadius() const
{
	m_DesignLock.lockForRead();
	double r = m_Design.RoughHullRadius();
	m_DesignLock.unlock();
	return r;
}

bool DaSc::Route(AuScRoute& route, const AuPoint& fromLoc, const AuPoint& toLoc, const DaAtom* toWho, bool isCrew) const
{
	Q_UNUSED(toWho);
	if(Name() == "AtlantisT")
		g_Au.m_LogMessages |= LOG_Path;
	bool foundRoute;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		foundRoute = scd->Route(route, fromLoc, toLoc, isCrew, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		foundRoute = false;
	UnlockDesign();
	m_DesignLock.unlock();
	if(Name() == "AtlantisT")
		g_Au.m_LogMessages &= ~LOG_Path;
	return foundRoute;
}

void DaSc::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
		bool bContinuous)
{
	Q_UNUSED(fPanelLoc);
	Q_UNUSED(bContinuous);
	m_DesignLock.lockForRead();
	bool isLift = m_Lifts.contains(idPanel);
	m_DesignLock.unlock();
	if(isLift)
		aActiveControls.append(DiControlInput(idPanel, CI_OperateTurboLift));
	else
		aActiveControls.append(DiControlInput(idPanel, CI_OpenCloseDoor));
}

void DaSc::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	if(InDesignEnvironment())
		return;													// Ship in design environment does not show up on sensors.

	if(Is(scannerId))
	{
		// Scanning vessel does not show a blip for "itself".
		if(scanContent & 0x0064)
		{
			// Transporter console detects my transporter rooms when they are on and inactive (not in mid-transport).
			if(scanContent & 0x0004)
			{
				// Check to see if this spacecraft has any transporter rooms that are on and inactive.
				AuMatrix4x4 envM(EnvTransform());

				bool isStation = IsSpaceStation();

				m_DesignLock.lockForRead();
				for(AuScReportTransporterStationHash::const_iterator ti = m_Design.Transporters().cbegin();
						ti != m_Design.Transporters().cend();  ti++)
				{
					if(IsTransporterSystemOn(ti.key()) && !IsTransporterSystemActive(ti.key()) && (*ti).Pads())
					{
						// Found a transporter room that is on and inactive, and it has pads.
						// Add a blip to the sensor scan.
						AuPointArray pads;
						TransporterPadLocations(ti.key(), pads);
						quint16 roomNum = ti.key();
						AuPoint padloc = envM.Project(pads.first());
						if(isStation)
							blips.Add(AuBlipTransporterStation(Name(), roomNum, (*ti).Pads(), EmpireId(), POL_Empire, Id(),
										padloc));
						else
							blips.Add(AuBlipTransporterSpacecraft(Name(), roomNum, (*ti).Pads(), EmpireId(), POL_Empire, Id(),
										padloc));
						//qDebug() << "DaSc::SensorScan" << *this << "troom" << roomNum << "pads" << pads.size();
					}
				}
				m_DesignLock.unlock();
			}

			// Does the scanner want my innards?
			if(scanContent & 0x0040)
				DaAtom::SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
						scannerWeaponRange, scanContent, lockedBlip, stanceCache);
		}
		return;
	}

	// Spaceships don't show up as separate when subordinate to other spacecraft.
	if(IsInSpacecraft())
		return;

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	// Must scan me if looking for locked-on blip and I'm it.
	bool scanMe = true;
	if((!(scanContent & 0x0008) || !Is(lockedBlip)))
	{
		// Not looking for locked-on blip or I am not the locked-on blip.
		// I am only scanned for sensor consoles, fire control consoles, transporter consoles, AI pilots, and AI turret gunners.
		if(!(scanContent & 0x0037))
			scanMe = false;
	}

	if(scanMe)
	{
		//qDebug("Scanned Spaceship");
		double dist2 = scannerLoc.Distance2(EnvLocation());	// Squared distance from scanner to ship.
		bool insideVisibleToTransporters = !ShieldsBlockTransportAboard();
		bool isHidingInGroundClutter = false;
		m_DesignLock.lockForRead();
		quint32 volm = m_Design.HullVolume();	// in cubic meters
		m_DesignLock.unlock();

		quint32 maxHits = DesignHitsStructural();
		quint32 dmgHits = HitsDamageStructural();
		quint32 curHits;
		if(dmgHits < maxHits)
			curHits = maxHits - dmgHits;
		else
			curHits = 0;

		// Determine if this ship has sensors "locked on" to the scanning ship.
		bool isLockedOn = LockedBlip() == scannerId;

		POLITICAL_STANCE stance = PoliticalStance(scannerEmpire, EmpireId(), stanceCache);

		quint8 quality = SystemQuality(SYS_Hull);				// This is "apparent" quality?

		SPACECRAFT_STRUCTURE structure = Structure();
		if(!insideVisibleToTransporters)
		{
			// Shields are up
			switch(structure)
			{
			case SC_Count:
			case SC_Deprecated:
				break;
			case SC_Hull:		// powerless hull
				qWarning("DaSc::SensorScan %d %s structure is Hull, shields are up, HullQ %d "
						"techLevelMade %d", Id().GetID(), (const char*)Name().toLatin1(), quality, m_QMade);
				// Fall through to report as a Station.
				[[fallthrough]];
			case SC_Station:	// spacecraft hull with no drives
				blips.Add(AuBlipStationShielded(Name(), quality, isLockedOn, IsNew(), IsPrivatelyOwned(), RoughRadius(),
							PilotAvoidRadius(), Vector(), volm, maxHits, curHits, DesignName(), EmpireId(), stance, Id(),
							EnvTransform().Project(TargetLocation())));
				break;
			case SC_Spaceship:	// spacecraft with maneuver drives
				blips.Add(AuBlipSpaceshipShielded(Name(), quality, isLockedOn, IsNew(), IsPrivatelyOwned(), RoughRadius(),
							PilotAvoidRadius(), Vector(), volm, maxHits, curHits, DesignName(), EmpireId(), stance, Id(),
							EnvTransform().Project(TargetLocation())));
				break;
			case SC_Starship:	// with maneuver and FTL drives
				blips.Add(AuBlipStarshipShielded(Name(), quality, isLockedOn, IsNew(), IsPrivatelyOwned(), RoughRadius(),
							PilotAvoidRadius(), Vector(), volm, maxHits, curHits, DesignName(), EmpireId(), stance, Id(),
							EnvTransform().Project(TargetLocation())));
				break;
			}

			// Check the maximum range of the scanning spacecraft or city's weapon bays.
			double wpnRange = scannerWeaponRange;
			double wpnRange2 = wpnRange*wpnRange;	// Easier than performing sqrt on distance to compare.
			if(stance == POL_Enemy && dist2 < wpnRange2)
			{
				SetInEnemyRange();		// We're in range of the bay weapon of a spacecraft that considers us an enemy.
				//qDebug() << "DaSc::SensorScan" << Id() << Name() << "in range, shields up";
			}
		}
		else
		{
			// Shields are down
			// If not moving and all drives and sensors are off, register as a hull. This way AI dudes can still see the target
			// to fire turrets or whatever but this ship will not be considered a bay weapon, transporter or sensor target,
			// enabling a ship to hide by landing and turning off everything.
			if(dist2 > HideInGroundClutterRange2() && IsHidingInGroundClutter())
			{
				// Identify this ship as hiding.
				// All relevant systems are off and this ship is far enough from the scanner.
				isHidingInGroundClutter = true;					// Prevent scanning subatoms for any reason.
				blips.Add(AuBlipSpacecraftHiding(Name(), quality, isLockedOn, IsNew(), IsPrivatelyOwned(), RoughRadius(),
							Vector(), volm, maxHits, curHits, DesignName(), EmpireId(), stance, Id(),
							EnvTransform().Project(TargetLocation())));
			}
			else
			{
				// Ship is not hiding. Something is turned on.
				switch(structure)
				{
				case SC_Count:
				case SC_Deprecated:
					break;
				case SC_Hull:		// powerless hull
					qWarning("DaSc::SensorScan %d %s structure is Hull, HullQ %d techLevelMade %d",
							Id().GetID(), (const char*)Name().toLatin1(), quality, m_QMade);
					// Fall through to report as a Station.
					[[fallthrough]];
				case SC_Station:	// spacecraft hull with no drives
					blips.Add(AuBlipStation(Name(), quality, isLockedOn, IsNew(), IsPrivatelyOwned(), RoughRadius(),
								Vector(), volm, maxHits, curHits, DesignName(), EmpireId(), stance, Id(),
								EnvTransform().Project(TargetLocation())));
					break;
				case SC_Spaceship:	// spacecraft with maneuver drives
					blips.Add(AuBlipSpaceship(Name(), quality, isLockedOn, IsNew(), IsPrivatelyOwned(), RoughRadius(),
								Vector(), volm, maxHits, curHits, DesignName(), EmpireId(), stance, Id(),
								EnvTransform().Project(TargetLocation())));
					break;
				case SC_Starship:	// with maneuver and FTL drives
					blips.Add(AuBlipStarship(Name(), quality, isLockedOn, IsNew(), IsPrivatelyOwned(), RoughRadius(),
								Vector(), volm, maxHits, curHits, DesignName(), EmpireId(), stance, Id(),
								EnvTransform().Project(TargetLocation())));
					break;
				}

				// Check the maximum range of the scanning spacecraft or city's weapon bays.
				double wpnRange = scannerWeaponRange;
				double wpnRange2 = wpnRange*wpnRange;	// Easier than performing sqrt on distance to compare.
				if(stance == POL_Enemy && dist2 < wpnRange2)
				{
					SetInEnemyRange();		// We're in range of the bay weapon of a spacecraft that considers us an enemy.
					//qDebug() << "DaSc::SensorScan" << Id() << Name() << "in range, shields down";
				}
			}
		}

		// Transporter console detects my transporter rooms when they are on and inactive (not in mid-transport).
		if(scanContent & 0x0004)
		{
			// Check to see if this spacecraft has any transporter rooms that are on and inactive.
			// These must be visible even if shields are up or the ship is hiding in ground clutter.
			AuMatrix4x4 envM(EnvTransform());
			bool isStation = IsSpaceStation();
			m_DesignLock.lockForRead();
			for(AuScReportTransporterStationHash::const_iterator ti = m_Design.Transporters().cbegin();
					ti != m_Design.Transporters().cend();  ti++)
			{
				if(IsTransporterSystemOn(ti.key()) && !IsTransporterSystemActive(ti.key()) && (*ti).Pads())
				{
					// Found a transporter room that is on and inactive, and it has pads.
					// Add a blip to the sensor scan.
					AuPointArray pads;
					TransporterPadLocations(ti.key(), pads);
					quint16 roomNum = ti.key();
					AuPoint padloc = envM.Project(pads.first());
					if(isStation)
						blips.Add(AuBlipTransporterStation(Name(), roomNum, (*ti).Pads(), EmpireId(), stance, Id(), padloc));
					else
						blips.Add(AuBlipTransporterSpacecraft(Name(), roomNum, (*ti).Pads(), EmpireId(), stance, Id(), padloc));
					//qDebug() << "DaSc::SensorScan" << *this << "troom" << roomNum << "pads" << pads.size();
				}
			}
			m_DesignLock.unlock();

			// Turn off transporter bit so we don't scan our innards when shields are up.
			if(!insideVisibleToTransporters)
				scanContent &= ~0x0004;
		}

		// Turn off bits to scan interior when hiding in ground clutter. This took into account "sight range" so we aren't robbing
		// AI gunners of targets by doing this.
		if(isHidingInGroundClutter)
			scanContent &= ~0x0024;
	}

	// Innards of other vessels (besides the scanning vessel) cannot be scanned if their shields are up or if they are hiding in
	// ground clutter.
	// Innards of other vessels are only scanned for transporter stations and AI turret gunners who are aboard.
	if(scanContent & 0x0024)
		DaAtom::SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
				scannerWeaponRange, scanContent, lockedBlip, stanceCache);
}

float DaSc::SensorRange() const
{
	unsigned int hitsLeft = GetPowerConsumption(SYS_Sensor);
	if(!hitsLeft)
		return 0;

	m_DesignLock.lockForRead();
	if(!m_Design.HasSensor())
	{
		m_DesignLock.unlock();
		return 0;
	}

	float range = m_Design.SensorRange();

	// Decay range according to damage.
	range *= (float)hitsLeft / (float)m_Design.SensorHits();
	m_DesignLock.unlock();
	return range;
}

void DaSc::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(IsHeadlightOn() && CapacitorLevel())
	{
		// My light is on.
		AuGLLight::ROLE lightRole = person.FindParentVesselInEnv() == this ? AuGLLight::SceneSpacecraft : AuGLLight::Spacecraft;
		AuPoint lpos = model.Project(m_HPos);
		if(m_HCosInnerCone > -1 || m_HCosOuterCone > -1)
		{
			AuPoint ldir = model.ProjectVector(m_HDir);
			lights.append(AuGLLight(Id(), lightRole, m_HkL, m_HkQ, m_HColor, lpos, m_HCosOuterCone, m_HCosInnerCone, ldir));
		}
		else
			lights.append(AuGLLight(Id(), lightRole, m_HkL, m_HkQ, m_HColor, lpos));
	}

	// Make lights for subordinate atoms, particularly the scene atom and potentially subordinate vessels that he might be aboard.
	DaVessel::GetRenderLightsRecursive(person, model, suns, lights);
}

void DaSc::SetFTLDriveSetting(quint16 whichFTL, quint8 setting)
{
	// Insure the setting is sane for the current FTL drive module.
	m_DesignLock.lockForRead();
	if(setting > m_Design.MaxCapacitorFTLSetting(whichFTL))
		setting = 0;
	m_DesignLock.unlock();

	m_FTLLock.lockForWrite();
	QHash<quint16, AuFTLDriveState>::iterator fi = m_FTLDrives.find(whichFTL);
	if(fi != m_FTLDrives.end() && (*fi).Setting() != setting)
	{
		(*fi).SetSetting(setting);
		m_FTLChg.Set();
		SetModified(NOTIFY_Scene);
	}
	m_FTLLock.unlock();
}

void DaSc::SetNotNew()
{
	m_New = false;
	SetModified();

	if(!IsPirateWithFreeWill())
	{
		// Notify all hands that we're alive.
		QString text = tr("System initialization complete. Welcome aboard.");
		AuCommChat msg(text, AuCommStdChannel::IntercomChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
		g_Au.SendAtomChat(msg);
	}
}

void DaSc::SetupArena(const QString& arenaName, quint8 team, quint8 shipQ, bool spawnOfficer, bool spawnCrew,
		bool spawnTroops, bool spawnPassengers, bool spawnVehicles)
{
	m_SystemsLock.lockForRead();
	bool v = m_ssaShipSystems.isEmpty();
	m_SystemsLock.unlock();

	if(v || shipQ != QualityMade())
		InitSystems(shipQ);
	SetEmpireId(team);
	SetName(g_Au.Dice().RandomShipName());
	SetArenaTeam(team);
	SetWhereMade(arenaName);
	RechargeCapacitor();
	TurnPowerPlantOn();
	SetGravityDriveState(GD_On);
	Refuel(shipQ);
	ShieldCharge(10000000);
	Unmoor();													// Cast off moorings. Clear 'nu' state.
	SetModified();

	// Equip the ship.

	// Medical supplies
	{
		m_DesignLock.lockForRead();
		int meds = m_Design.SurgeryUnitCount() + 1;
		m_DesignLock.unlock();
		QSet<COMMODITYID> medcids;
		medcids << CID_MedicalKit;
		medcids << CID_Antidote;
		medcids << CID_Antitoxin;
		medcids << CID_Water;

		// Load all the kinds of ammo I need into the hold.
		for(QSet<COMMODITYID>::const_iterator ai = medcids.cbegin(); ai != medcids.cend(); ai++)
		{
			AuItem* item = AuItem::Create(*ai, shipQ, team, meds);
			if(item)
			{
				AddCargo(*item);
				delete item;
			}
		}
	}

	// Tally all my ammo-using weapons.
	int cannons = 0;
	int launchers = 0;
	int projectors = 0;
	int racks = 0;

	// Bay weapons.
	m_DesignLock.lockForRead();
	for(AuScReportSysVolHash::const_iterator bi = m_Design.WeaponBays().cbegin(); bi != m_Design.WeaponBays().cend(); bi++)
	{
		COMMODITYID bayCID = SystemModule(SYS_Weapon, bi.key());
		switch(bayCID)
		{
		default:
			continue;
		case CID_BallisticWeaponBayModule:
			cannons += AuScDesign::BayWeaponCannons((*bi).VolumeCuFt());
			break;
		case CID_EnergyWeaponBayModule:
			projectors += AuScDesign::BayWeaponProjectors((*bi).VolumeCuFt());
			break;
		case CID_WeaponDeploymentBayModule:
			racks += AuScDesign::BayWeaponEjectorRacks((*bi).VolumeCuFt());
			break;
		case CID_WeaponLauncherBayModule:
			launchers += AuScDesign::BayWeaponLaunchers((*bi).VolumeCuFt());
			break;
		}
	}

	// Turrets
	for(AuScReportTurretHash::const_iterator ti = m_Design.Turrets().cbegin(); ti != m_Design.Turrets().cend(); ti++)
	{
		COMMODITYID turretCID = SystemModule(SYS_Turret, ti.key());
		switch(turretCID)
		{
		case CID_WeaponLauncherTurretModule:
			launchers++;
			break;
		case CID_BallisticWeaponTurretModule:
			cannons++;
			break;
		case CID_EnergyWeaponTurretModule:
			projectors++;
			break;
		default:
			break;
		}
	}

	// Vehicles
	launchers += m_Design.ParkingSpotsTotal();
	cannons += m_Design.ParkingSpotsTotal();
	projectors += m_Design.ParkingSpotsTotal();
	racks += m_Design.ParkingSpotsTotal();
	m_DesignLock.unlock();

	QHash<COMMODITYID, int> ammoNeeded;	// Key is ammo CID, value is count wanted.
	if(cannons)
	{
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiRa, cannons*10);
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiAc, cannons*10);
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiBl, cannons*10);
		ammoNeeded.insert(CID_HeavyArmsAmmoPi, cannons*10);
	}

	if(racks)
	{
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombBlRa, racks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombBlPi, racks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombElSt, racks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombSuPo, racks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombSlCr, racks*10);
		ammoNeeded.insert(CID_BombBl, racks*10);
	}

	if(projectors)
	{
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_CompressorBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_DisintegratorRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_ProtonBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_StasisBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_DisruptorBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_ErgRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_HeatRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_FreezeRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_NeutronBeam, projectors);
		ammoNeeded.insert(CID_ElectronBeam, projectors);
	}

	if(launchers)
	{
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_LargeRocketBuDi, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_LargeRocketBlBu, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_LargeMissileSlPo, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_LargeMissileBlBu, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_LargeRocketBl, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_SmallMissileCrFr, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_SmallMissileBlBu, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_SmallRocketSlPi, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_SmallRocketBlBu, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_SmallMissileBl, launchers*10);
		if(g_Au.Dice().Roll(1,11) == 1)
			ammoNeeded.insert(CID_LargeMissileBl, launchers*10);
		ammoNeeded.insert(CID_SmallRocketBl, launchers*10);
	}

	if(!ammoNeeded.isEmpty())
	{
		int totalAmmoWanted = 0;
		for(QHash<COMMODITYID, int>::const_iterator ai = ammoNeeded.cbegin(); ai != ammoNeeded.cend(); ai++)
			totalAmmoWanted += *ai;

		// Load all the kinds of ammo I need into the hold.
		for(QHash<COMMODITYID, int>::const_iterator ai = ammoNeeded.cbegin(); ai != ammoNeeded.cend(); ai++)
		{
			int roundsToLoad = CargoCapacityVolume() * *ai / totalAmmoWanted;
			if(!roundsToLoad)
				roundsToLoad = 1;

			// We only need one energy weapon per turret, unless we have bay weapons.
			AuItem* item = AuItem::Create(ai.key(), shipQ, team, roundsToLoad);
			if(item)
			{
				AddCargo(*item);
				delete item;
			}
		}
	}

	// Crew

	// Spawn crew and vehicles.
	if(spawnOfficer)
	{
		if(OfficerBerthsAvailable())
		{
			const DgData& gd = g_Au.LockDataForRead();
			AuSpecies species = gd.RandomEmpireSpecies();
			g_Au.UnlockData();
			DaUnit* unit = LoadPirateOfficer(team, species);
			if(unit)
				unit->SetArenaTeam(team);
		}
	}
	if(spawnCrew)
	{
		const DgData& gd = g_Au.LockDataForRead();
		AuSpecies species = gd.RandomEmpireSpecies();
		g_Au.UnlockData();

		int berths = CrewBerthsAvailable();
		while(berths--)
		{
			DaUnit* unit = LoadPirateCrew(team, species);
			if(unit)
				unit->SetArenaTeam(team);
		}
	}
	if(spawnTroops)
	{
		const DgData& gd = g_Au.LockDataForRead();
		AuSpecies species = gd.RandomEmpireSpecies();
		g_Au.UnlockData();

		int berths = TroopBerthsAvailable();
		while(berths--)
		{
			DaUnit* unit = LoadPirateTroop(team, species);
			if(unit)
				unit->SetArenaTeam(team);
		}
	}
	if(spawnPassengers)
	{
		int berths = PassengerBerthsAvailable();
		while(berths--)
		{
			const DgData& gd = g_Au.LockDataForRead();
			AuSpecies species = gd.RandomEmpireSpecies();
			g_Au.UnlockData();

			DaUnit* unit = LoadPiratePassenger(team, species);
			if(unit)
				unit->SetArenaTeam(team);
		}
	}
	SetCrewPosture(POSTURE_Offensive);

	// Vehicles

	if(spawnVehicles)
	{
		// Fill vehicle bays with vehicles.
		/*DHH Sc
		m_DesignLock.lockForRead();
		QList<AuMatrix4x4> gvms;
		m_Design.ParkingSpotsGround(gvms);
		QList<AuMatrix4x4> svms;
		m_Design.ParkingSpotsSpace(svms);
		QList<AuMatrix4x4> wvms;
		m_Design.ParkingSpotsWater(wvms);
		for(QList<AuParkingSpot>::const_iterator psi = m_DesignAnalysis.ParkingSpots().cbegin();
				psi != m_DesignAnalysis.ParkingSpots().cend(); psi++)
		{
			// There are none of those vehicles aboard OR the quantity aboard is less than the design contingent.
			// We are not in the design studio so center on extents.
			AuPoint bayLoc = (*psi).m_ParkLoc - m_DesignAnalysis.CG();	// Convert location to ship space from design coordinates.

			// Try to fetch a vehicle of that type. We will return as soon as we fetch one.
			// Locate a nu vehicle on a planet in this solar system that we can fetch.
			DaVehicle* vehicle = new DaVehicle((*psi).m_VehicleCID, shipQ, shipTL);

			// Vehicle models are 90 degrees off the parking spot detail's orientation.
			float zrot = (*psi).m_ParkRot - 90.f * DEG2RADf;

			// Insure the location is centered on the vehicle and raised so the vehicle rests on the floor.
			AuPoint vMin, vMax;
			vehicle->GetExtents(vMin, vMax);
			AuPoint vCen = (vMin + vMax)/2;	// Center of model as a vector from insertion point of vehicle atom.
			AuPoint placeLoc = bayLoc - vCen.RotateZ(zrot);	// Offset so center of rotated model is at detail location on floor.
			placeLoc.z += (vMax.z - vMin.z)/2;	// Raise center of model to set its base on the floor.

			// Place the vehicle aboard this spacecraft.
			AddAtom(vehicle);
			vehicle->Create();
			AuMatrix4x4 t;
			t.Translate(placeLoc);
			t.RotateZ(zrot);
			vehicle->SetTransformAndOrientNoTime(t);
			vehicle->SetInMotion();
			vehicle->SetEmpireId(team);
			vehicle->SetArenaTeam(team);
			vehicle->SetMotherShip(Id(), Name());
			vehicle->ResetHitPoints();
			vehicle->Refuel();
			vehicle->SetModified();

			g_Au.AtomAdded(vehicle->Id());
			vehicle->SetNormal();	// New atom is ready for normal thread processing.
		}
		m_DesignLock.unlock();
		*/
	}
}

// PirateEquip
// EquipPirate
void DaSc::SetupPirate(quint32 empireId, quint8 quality)
{
	quint8 shipQ = QualityOfDesign();
	if(shipQ < quality)
		shipQ = quality;
	m_SystemsLock.lockForRead();
	bool v = m_ssaShipSystems.isEmpty();
	m_SystemsLock.unlock();

	if(v || shipQ != QualityMade())
		InitSystems(shipQ);
	RechargeCapacitor();										// Capacitor is fully charged.
	SetGravityDriveState(GD_On);								// Gravity drive is on.
	InteriorLightToggle(INTLIGHT_Dim);							// Lights are dim for battle.
	Refuel(shipQ);												// Pirate ship is fully fueled.
	ShieldCharge(10000000);										// Charge shields to the max. The pirates are ready.
	Unmoor();													// Cast off moorings.

	// Make it a pirate.
	SetEmpireId(empireId);										// Pirate empire id.
	if(Name().isEmpty())
		SetName(g_Au.Dice().RandomShipName());					// Christen the ship, if not already named.

	// Equip the ship.

	// Medical supplies
	{
		m_DesignLock.lockForRead();
		int meds = m_Design.SurgeryUnitCount() + 1;
		m_DesignLock.unlock();
		QSet<COMMODITYID> medcids;
		medcids << CID_MedicalKit;
		medcids << CID_Antidote;
		medcids << CID_Antitoxin;
		medcids << CID_Water;

		// Load all the kinds of ammo I need into the hold.
		for(QSet<COMMODITYID>::const_iterator ai = medcids.cbegin(); ai != medcids.cend(); ai++)
		{
			AuItem* item = AuItem::Create(*ai, shipQ, empireId, meds);
			if(item)
			{
				AddCargo(*item);
				delete item;
			}
		}
	}

	// Tally all my ammo-using weapons.

	// Bay weapons.
	int bcannons = 0;
	int blaunchers = 0;
	int bprojectors = 0;
	int bracks = 0;
	m_DesignLock.lockForRead();
	for(AuScReportSysVolHash::const_iterator bi = m_Design.WeaponBays().cbegin(); bi != m_Design.WeaponBays().cend(); bi++)
	{
		COMMODITYID bayMod = (*bi).Module();
		quint32 wpns = (*bi).Volume();
		double wpnsCuFt = wpns * CubicFeetPerCubicMeter();
		switch(bayMod)
		{
		default:
			continue;
		case CID_BallisticWeaponBayModule:
			bcannons += AuScDesign::BayWeaponCannons(wpnsCuFt);
			break;
		case CID_EnergyWeaponBayModule:
			bprojectors += AuScDesign::BayWeaponProjectors(wpnsCuFt);
			break;
		case CID_WeaponDeploymentBayModule:
			bracks += AuScDesign::BayWeaponEjectorRacks(wpnsCuFt);
			break;
		case CID_WeaponLauncherBayModule:
			blaunchers += AuScDesign::BayWeaponLaunchers(wpnsCuFt);
			break;
		}
	}

	// Turrets
	int tcannons = 0;
	int tlaunchers = 0;
	int tprojectors = 0;
	for(AuScReportTurretHash::const_iterator ti = m_Design.Turrets().begin(); ti != m_Design.Turrets().end(); ti++)
	{
		COMMODITYID turretMod = (*ti).Module();
		switch(turretMod)
		{
		case CID_WeaponLauncherTurretModule:
			tlaunchers++;
			break;
		case CID_BallisticWeaponTurretModule:
			tcannons++;
			break;
		case CID_EnergyWeaponTurretModule:
			tprojectors++;
			break;
		default:
			break;
		}
	}

	// Ammo for vehicles. Only get ammo that is already needed by ship weapons, so we don't grab projectors for Seledon and Ogar.
	int parkingSpots = m_Design.ParkingSpotsTotal();
	if(tlaunchers)
		tlaunchers += parkingSpots;
	if(tcannons)
		tcannons += parkingSpots;
	if(tprojectors)
		tprojectors += parkingSpots;
	if(bracks)
		bracks += parkingSpots;
	m_DesignLock.unlock();

	QHash<COMMODITYID, int> ammoNeeded;	// Key is ammo CID, value is count wanted.
	if(bcannons || tcannons)
	{
		int cannons = bcannons + tcannons;
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiRa, cannons*10);
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiAc, cannons*10);
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiBl, cannons*10);
		ammoNeeded.insert(CID_HeavyArmsAmmoPi, cannons*10);
	}

	if(bracks)
	{
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombBlRa, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombBlPi, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombElSt, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombSuPo, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombSlCr, bracks*10);
		ammoNeeded.insert(CID_BombBl, bracks*10);
	}

	if(bprojectors || tprojectors)
	{
		int projectors = bprojectors + tprojectors;
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_CompressorBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_DisintegratorRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_ProtonBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_StasisBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_DisruptorBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_ErgRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_HeatRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_FreezeRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_NeutronBeam, projectors);
		ammoNeeded.insert(CID_ElectronBeam, projectors);
	}

	if(blaunchers)
	{
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeRocketBuDi, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeRocketBlBu, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeMissileSlPo, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeMissileBlBu, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeMissileBl, blaunchers*10);
		ammoNeeded.insert(CID_LargeRocketBl, blaunchers*10);
	}

	if(tlaunchers)
	{
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallMissileCrFr, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallMissileBlBu, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallRocketSlPi, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallRocketBlBu, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallMissileBl, tlaunchers*10);
		ammoNeeded.insert(CID_SmallRocketBl, tlaunchers*10);
	}

	if(!ammoNeeded.isEmpty())
	{
		int totalAmmoWanted = 0;
		for(QHash<COMMODITYID, int>::const_iterator ai = ammoNeeded.cbegin(); ai != ammoNeeded.cend(); ai++)
			totalAmmoWanted += *ai;

		// Load all the kinds of ammo I need into the hold, up to half full.
		for(QHash<COMMODITYID, int>::const_iterator ai = ammoNeeded.cbegin(); ai != ammoNeeded.cend(); ai++)
		{
			int roundsToLoad = (CargoCapacityVolume()/2) * *ai / totalAmmoWanted;
			if(!roundsToLoad)
				roundsToLoad = 1;

			// We only need one energy weapon per turret, unless we have bay weapons.
			AuItem* item = AuItem::Create(ai.key(), shipQ, empireId, roundsToLoad);
			if(item)
			{
				AddCargo(*item);
				delete item;
			}
		}
	}

	// Load a pirate crew.
	LoadPirateCrew(empireId);									// Load a crew of scaly wags.

	quint32 cberths = CryoBerthsAvailable() / 2;
	if(cberths)
	{
		// Get DNA for the pirate empire.
		while(cberths--)
		{
			const DgData& gd = g_Au.LockDataForRead();
			AuSpecies species = gd.RandomEmpireSpecies();
			g_Au.UnlockData();
			BoardCryoTraveller(species);
		}
	}
}

void DaSc::SetupLostAtlantis(quint32 empireId, quint8 quality, const AuSpecies& crewSpecies)
{
	quint8 shipQ = QualityOfDesign();
	if(shipQ < quality)
		shipQ = quality;
	m_SystemsLock.lockForRead();
	bool v = m_ssaShipSystems.isEmpty();
	m_SystemsLock.unlock();

	if(v || shipQ != QualityMade())
		InitSystems(shipQ);
	RechargeCapacitor();										// Capacitor is fully charged.
	SetGravityDriveState(GD_On);								// Gravity drive is on.
	InteriorLightToggle(INTLIGHT_Dim);							// Lights are dim for battle.
	Refuel(shipQ);												// Pirate ship is fully fueled.
	ShieldCharge(10000000);										// Charge shields to the max. The pirates are ready.
	Unmoor();													// Cast off moorings.

	// Make it a pirate.
	SetEmpireId(empireId);										// Pirate empire id.
	SetName("Atlantis");

	// Equip the ship.

	// Medical supplies
	{
		m_DesignLock.lockForRead();
		int meds = m_Design.SurgeryUnitCount() + 2;
		m_DesignLock.unlock();
		QSet<COMMODITYID> medcids;
		medcids << CID_MedicalKit;
		medcids << CID_Antidote;
		medcids << CID_Antitoxin;
		medcids << CID_Water;

		// Load all the kinds of ammo I need into the hold.
		for(QSet<COMMODITYID>::const_iterator ai = medcids.cbegin(); ai != medcids.cend(); ai++)
		{
			AuItem* item = AuItem::Create(*ai, shipQ, empireId, meds);
			if(item)
			{
				AddCargo(*item);
				delete item;
			}
		}
	}

	// Tally all my ammo-using weapons.

	// Bay weapons.
	int bcannons = 0;
	int blaunchers = 0;
	int bprojectors = 0;
	int bracks = 0;
	m_DesignLock.lockForRead();
	for(AuScReportSysVolHash::const_iterator bi = m_Design.WeaponBays().cbegin(); bi != m_Design.WeaponBays().cend(); bi++)
	{
		COMMODITYID bayMod = (*bi).Module();
		quint32 wpns = (*bi).Volume();
		double wpnsCuFt = wpns * CubicFeetPerCubicMeter();
		switch(bayMod)
		{
		default:
			continue;
		case CID_BallisticWeaponBayModule:
			bcannons += AuScDesign::BayWeaponCannons(wpnsCuFt);
			break;
		case CID_EnergyWeaponBayModule:
			bprojectors += AuScDesign::BayWeaponProjectors(wpnsCuFt);
			break;
		case CID_WeaponDeploymentBayModule:
			bracks += AuScDesign::BayWeaponEjectorRacks(wpnsCuFt);
			break;
		case CID_WeaponLauncherBayModule:
			blaunchers += AuScDesign::BayWeaponLaunchers(wpnsCuFt);
			break;
		}
	}

	// Turrets
	int tcannons = 0;
	int tlaunchers = 0;
	int tprojectors = 0;
	for(AuScReportTurretHash::const_iterator ti = m_Design.Turrets().begin(); ti != m_Design.Turrets().end(); ti++)
	{
		COMMODITYID turretMod = (*ti).Module();
		switch(turretMod)
		{
		case CID_WeaponLauncherTurretModule:
			tlaunchers++;
			break;
		case CID_BallisticWeaponTurretModule:
			tcannons++;
			break;
		case CID_EnergyWeaponTurretModule:
			tprojectors++;
			break;
		default:
			break;
		}
	}

	// Ammo for vehicles. Only get ammo that is already needed by ship weapons, so we don't grab projectors for Seledon and Ogar.
	int parkingSpots = m_Design.ParkingSpotsTotal();
	if(tlaunchers)
		tlaunchers += parkingSpots;
	if(tcannons)
		tcannons += parkingSpots;
	if(tprojectors)
		tprojectors += parkingSpots;
	if(bracks)
		bracks += parkingSpots;
	m_DesignLock.unlock();

	QHash<COMMODITYID, int> ammoNeeded;	// Key is ammo CID, value is count wanted.
	if(bcannons || tcannons)
	{
		int cannons = bcannons + tcannons;
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiRa, cannons*10);
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiAc, cannons*10);
		if(g_Au.Dice().Roll(1,3) == 1)
			ammoNeeded.insert(CID_HeavyArmsAmmoPiBl, cannons*10);
		ammoNeeded.insert(CID_HeavyArmsAmmoPi, cannons*10);
	}

	if(bracks)
	{
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombBlRa, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombBlPi, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombElSt, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombSuPo, bracks*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_BombSlCr, bracks*10);
		ammoNeeded.insert(CID_BombBl, bracks*10);
	}

	if(bprojectors || tprojectors)
	{
		int projectors = bprojectors + tprojectors;
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_CompressorBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_DisintegratorRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_ProtonBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_StasisBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_DisruptorBeam, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_ErgRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_HeatRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_FreezeRay, projectors);
		if(g_Au.Dice().Roll(1,9) == 1)
			ammoNeeded.insert(CID_NeutronBeam, projectors);
		ammoNeeded.insert(CID_ElectronBeam, projectors);
	}

	if(blaunchers)
	{
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeRocketBuDi, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeRocketBlBu, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeMissileSlPo, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeMissileBlBu, blaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_LargeMissileBl, blaunchers*10);
		ammoNeeded.insert(CID_LargeRocketBl, blaunchers*10);
	}

	if(tlaunchers)
	{
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallMissileCrFr, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallMissileBlBu, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallRocketSlPi, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallRocketBlBu, tlaunchers*10);
		if(g_Au.Dice().Roll(1,5) == 1)
			ammoNeeded.insert(CID_SmallMissileBl, tlaunchers*10);
		ammoNeeded.insert(CID_SmallRocketBl, tlaunchers*10);
	}

	if(!ammoNeeded.isEmpty())
	{
		int totalAmmoWanted = 0;
		for(QHash<COMMODITYID, int>::const_iterator ai = ammoNeeded.cbegin(); ai != ammoNeeded.cend(); ai++)
			totalAmmoWanted += *ai;

		// Load all the kinds of ammo I need into the hold, up to half full.
		for(QHash<COMMODITYID, int>::const_iterator ai = ammoNeeded.cbegin(); ai != ammoNeeded.cend(); ai++)
		{
			int roundsToLoad = (CargoCapacityVolume()/2) * *ai / totalAmmoWanted;
			if(!roundsToLoad)
				roundsToLoad = 1;

			// We only need one energy weapon per turret, unless we have bay weapons.
			AuItem* item = AuItem::Create(ai.key(), shipQ, empireId, roundsToLoad);
			if(item)
			{
				AddCargo(*item);
				delete item;
			}
		}
	}

	// Load a pirate crew.
	LoadCrew(empireId, crewSpecies);							// Load a crew of scaly wags.

	// Load some cryo berths.
	quint32 cberths = CryoBerths();
	if(cberths > 100)
		cberths = 100;
	while(cberths--)
		BoardCryoTraveller(crewSpecies);

	// Load some vehicles.
	AuEther* ether = Ether();
	if(!ether)
		return;
	COMMODITYID vehCIDs[] =
	{
		CID_DirtBike,
		CID_DirtBike,
		CID_APC,
		CID_AdvSpaceFighter,
		CID_AdvSpaceFighter,
		CID_SpaceFighter,
		CID_SpaceFighter,
		CID_SpaceRocket,
		CID_SpaceRocket,
		CID_SpaceTransport,
		CID_SpaceTransport,
		CID_Helicopter,
		CID_Helicopter,
	};
	int v2load = sizeof(vehCIDs) / sizeof(COMMODITYID);
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		DaAtomList vehiclesAboard;
		AuMatrix4x4 parkPosM;
		for(int v=0; v<v2load; v++)
		{
			DaVehicle* vehicle = new DaVehicle(vehCIDs[v], QualityBaseline());
			if(!vehicle)
				continue;
			VEHICLE_SPOT_TYPE spotType = vehicle->ParkingSpotType();
			if(VehicleParkingSpot(spotType, parkPosM, scd, vehiclesAboard))
			{
				vehicle->ConnectSignals();
				vehicle->Create();
				vehicle->SetPos(AuAtomPos(Id(), parkPosM));

				ether->AddAtom(vehicle);
				g_Au.AtomAdded(vehicle->Id());

				vehicle->ResetHitPoints();
				vehicle->Refuel();
				vehicle->SetEmpireId(EmpireId());
				vehicle->SetMotherShip(Id(), Name());
				vehicle->Park(this, parkPosM, spotType);
				vehicle->SetNormal();

				vehiclesAboard.append(vehicle);
				vehicle->TickTock();
			}
			else
				delete vehicle;
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
}

double DaSc::ShieldUnits() const
{
	m_DesignLock.lockForRead();
	double v = m_Design.ShieldUnits();
	m_DesignLock.unlock();
	return v;
}

AuSysId DaSc::SightedSystem(quint16 sightedPart) const
{
	AuSysId sysId;
	const AuScDesign* scd = LockDesign();
	if(scd)
		sysId = scd->SightedSystem(sightedPart);
	UnlockDesign();
	return sysId;
}

DaAtom* DaSc::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
		AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	// Don't sight myself or my subatoms.
	if(sightingAtom == this)
		return pBest;

	// Determine my location in the space of the sighting atom.
	// If I am far enough away from the sight segment, this atom and my subordinate atoms can be ignored.
	if(ShieldsBlockWeapons())
	{
		AuPoint myLoc = tModel.Project(DesignCenterOffset());
		if(myLoc.DistToSegment(eyeLoc, sightedLoc) > RoughRadius()*2)
			return pBest;
	}
	else
	{
		AuPoint myLoc = tModel.Project(AuPoint(0,0));
		if(myLoc.DistToSegment(eyeLoc, sightedLoc) > RoughRadius())
			return pBest;
	}

	// The rough test passed, we need to get more detailed.
	if(SightTargetDetails(eyeLoc, tModel, sightedLoc, sightedPart, sightedShield, sightedSurfaceNormal))
		pBest = (DaAtom*)this;			// Must cast me to non-const.

	// Base class performs a recursive search.
	return DaVessel::SightTarget(sightingAtom, eyeLoc, tModel, pBest, sightedLoc, sightedPart, sightedShield, sightedSurfaceNormal);
}

AuPoint DaSc::ShieldRadii() const
{
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	return max - min;
}

bool DaSc::ShieldContains(const AuPoint& loc) const
{
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	AuPoint cen = (min + max)/2;
	AuPoint shieldRadii = max - min;
	return loc.InEllipsoid(cen, shieldRadii);
}

bool DaSc::ShieldSurfaceNear(const AuPoint& loc) const
{
	AuPoint min,max;
	m_DesignLock.lockForRead();
	m_Design.Extents(min, max);
	m_DesignLock.unlock();
	AuPoint cen = (min + max)/2;
	AuPoint shieldRadii = max - min;
	AuPoint eloc = loc.ToEllipsoid(cen, shieldRadii);
	return eloc.Distance(loc) < 5 * FeetPerMeter();
}

bool DaSc::SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &model, AuPoint& sightedLoc, unsigned int& sightedPart,
		bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	// Unproject the sight segment into my space.
	AuPoint eLoc = model.UnProject(eyeLoc);
	AuPoint sLoc = model.UnProject(sightedLoc);
	if(ShieldsBlockWeapons())
	{
		AuPoint shieldRadii(ShieldRadii());
		AuPoint cen(DesignCenterOffset());
		if(!eLoc.InEllipsoid(cen, shieldRadii))
		{
			// Shield bubble is only sighted when looking from the outside, to block incoming but not outgoing shots.
			AuPoint isect1, isect2;
			int isect = eLoc.IntersectEllipsoid(cen, shieldRadii, sLoc, isect1, isect2);
			if(isect > 0)
			{
				AuPoint::DIRECTION dir = (isect1 - cen).Direction();
				if(ShieldCapacitor((SHIELD)dir) > 2.f && !ShieldScram((SHIELD)dir))
				{
					sightedLoc = model.Project(isect1);	// Project sighted location into the 'model' space.
					if(sightedSurfaceNormal)
					{
						isect1.Unit();
						*sightedSurfaceNormal = model.ProjectVector(isect1);
					}
					sightedPart = 0xffff;
					sightedShield = true;
					return true;
				}
			}
		}
	}
	// Check my parts.
	bool hitMe = false;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		quint16 sightedPartId = 0;
		AuPoint sN;
		hitMe = scd->Sight(eLoc, sLoc, sightedPartId, m_DoorStates, m_LandingGearStates, m_Lifts, sN);
		if(hitMe)
		{
			sightedLoc = model.Project(sLoc);	// Project sighted location into the 'model' space.
			if(sightedSurfaceNormal)
				*sightedSurfaceNormal = model.ProjectVector(sN);
			sightedPart = sightedPartId;
			sightedShield = false;
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	return hitMe;
}

void DaSc::SpawnArenaVehicle()
{
	//DHH Sc
}

SPACECRAFT_STRUCTURE DaSc::Structure() const
{
	m_DesignLock.lockForRead();
	SPACECRAFT_STRUCTURE v = m_Design.Structure();
	m_DesignLock.unlock();
	return v;
}

quint32 DaSc::SystemHitsRemainingSystemsLocked(const AuSysId& sysId) const
{
	quint32 hitsMax = SystemHits(sysId);
	m_DamageLock.lockForRead();
	quint32 v = m_Damage.HitsDamageAddParts(sysId);
	m_DamageLock.unlock();
	if(v == 0)
		return hitsMax;
	if(v >= hitsMax)
		return 0;
	return hitsMax - v;
}

QString DaSc::SystemName(const AuSysId& sysId) const
{
	m_DesignLock.lockForRead();
	QString v = m_Design.SystemName(sysId);
	m_DesignLock.unlock();
	return v;
}

AuPoint DaSc::TargetLocation() const
{
	m_DesignLock.lockForRead();
	bool valid = m_TargetLocValid;
	AuPoint targLoc = m_TargetLoc;
	m_DesignLock.unlock();
	if(!valid)
	{
		// Cache design target location.
		const AuScDesign* scd = LockDesign();
		if(scd)
			targLoc = scd->TargetLocation();
		UnlockDesign();
		if(!scd)
			return AuPoint(0,0);
		m_DesignLock.lockForWrite();
		m_TargetLoc = targLoc;
		m_TargetLocValid = true;
		m_DesignLock.unlock();
	}
	return targLoc;
}

quint8 DaSc::QualityOfDesign() const
{
	m_DesignLock.lockForRead();
	quint8 v = m_Design.Quality();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::TestTransportLocation(const AuPoint& loc) const
{
	quint16 inVoid = 0;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		inVoid = scd->RoomVoidContainingLocLive(loc, .1, m_DoorStates, m_LandingGearStates, m_Lifts);
	UnlockDesign();
	m_DesignLock.unlock();
	return inVoid;
}

void DaSc::TickOnClient(const AuElapsedTimer& last)
{
	// Tick base class and all subatoms.
	DaVessel::TickOnClient(last);

	if(!m_Ticked)
	{
		if(IsDesignDataCacheValid())
		{
			// Call InvalidateEnvironment on me and all atoms aboard, now that the design data is ready.
			InvalidateEnvironmentRecursive();
			m_Ticked = true;
		}
		else
			return;
	}

	DecayMoveActions();
}

unsigned int DaSc::IsTocker() const
{
	unsigned int ms = DaVessel::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is not zero.

	/* This is not necessary because DaVessel never returns ms > 10 seconds.
	if(ms > d_MSecPerMin)
		ms = d_MSecPerMin;
		*/

	if(ms > 3*d_MSecPerSec)
	{
		m_DesignLock.lockForRead();
		bool hasCalls = !m_LiftCalls.empty();
		m_DesignLock.unlock();
		if(hasCalls)
			return 3*d_MSecPerSec;			// Must Tock to operate lifts.
	}

	/* This is not necessary because DaVessel never returns ms > 10 seconds.
	if(ms > 10*d_MSecPerSec)
	{
		if(HasSpaceVehicleLauncher())
			ms = 10*d_MSecPerSec;
		else if(HasAnyCondition())
			ms = 10*d_MSecPerSec;
		else
		{
			m_DesignLock.lockForRead();
			if(m_Design.HasVehicleLandingSystem())
				ms = 10*d_MSecPerSec;
			m_DesignLock.unlock();
		}
	}
	*/
	return ms;
}

void DaSc::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	// Tocking depends on this ship's design geometry. It will be delayed until the database thread has loaded the design.
	if(!IsDesignDataCacheValid())
		return;	// My design is not valid yet.

	if(!m_Tocked)
	{
		// This is the first Tock this session with my design loaded.
		if(!IsPirate() && !InArena() && !InDesignEnvironment())
		{
			// Notify the system of the ship's quality for pirate encounters.
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
			if(system)
				system->SetNonPirateShipPresent(HighestSystemQuality());
		}

		// Start monitoring comm channels.
		MonitorCommChannels();

		// Query my environment now that I have my design analyzed.
		// Call InvalidateEnvironment on all atoms aboard now that the deck data is ready.
		InvalidateEnvironmentRecursive();
		//qDebug() << "DaVessel::Tock" << Id() << Name() << "design" << m_idDesign << "now valid.";

		// Turn on bridge indicators in rooms with helm consoles.
		TurnOnBridgeIndicators();

		m_Tocked = true;
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Tock base class and all subatoms.
	DaVessel::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;

	if(last.IsNewPeriod(now, 10*d_MSecPerSec, Id().GetID()))
	{
		// Reset launch tubes that may have been left active.
		ResetActiveLaunchers();

		// Reset landing systems after a few seconds.
		OperateLandingSystems();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// From Think
	if(last.ElapsedSecTicks(now))
	{
		OperateLifts();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

float DaSc::TractorBeamRange(float unitsFired, quint16 which) const
{
	m_DesignLock.lockForRead();
	float r = m_Design.TractorBeamRange(which, unitsFired);
	m_DesignLock.unlock();
	return r;
}

void DaSc::TransporterPadLocations(quint16 which, AuPointArray& pads) const
{
	const AuScDesign* scd = LockDesign();
	if(scd)
		scd->TransporterPadLocs(which, pads);
	UnlockDesign();
}

quint16 DaSc::TransporterSystemId(int padsDesired, bool mustBeOn) const
{
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	int mostworking = -1;
	quint16 bestWhich = 0xffff;				// Room id.
	if(mustBeOn)
	{
		// Search for a transporter room that is on and inactive.
		QMap<quint16, bool> transFound;	// Transporters found that will work. Key is room id.
		QList<quint16> troomIds(TransporterRoomIds());
		for(QList<quint16>::const_iterator ti = troomIds.cbegin(); ti != troomIds.cend();  ti++)
		{
			quint16 roomId = *ti;
			if(IsTransporterSystemOn(roomId) && !IsTransporterSystemActive(roomId))
			{
				// The room is on and not active.
				int working = GetPowerConsumption(SYS_Transporter, roomId);

				// Remember all rooms with enough working pads.
				if(working >= padsDesired)
				{
					// Bottom byte of 'which' value is the room 'rsub' number. The key is the 'which' value.
					transFound.insert(roomId, true);	// Order by room id.
				}

				// Haven't found enough pads in one room yet so remember the room with the most working pads.
				if(working > mostworking)
				{
					mostworking = working;
					bestWhich = roomId;
				}
			}
		}

		// If we found any rooms that work, return the first one in the map, which is the one with the lowest room 'rsub' number.
		if(!transFound.isEmpty())
			return transFound.begin().key();
	}
	else
	{
		// Search for a transporter room that is inactive. It does not have to be on so we will prefer rooms that are
		// not on. This subtlety allows transporter room to transporter room transport aboard the same vessel.
		//  - A crew member can turn on the transporter in the room they are in then call for beam over, which will beam from
		//  a different transporter room that is not on.
		//  - A friendly unit aboard a station can turn on the transporter room they are in then call for a beam over, which will
		//  beam from a different transporter room that is not on.
		// First pass searches for a room that is not on and adequate to our needs.
		QMap<quint16, bool> transFound;	// Rooms found that will work. Key is room id.
		QList<quint16> troomIds(TransporterRoomIds());
		for(QList<quint16>::const_iterator ti = troomIds.cbegin(); ti != troomIds.cend();  ti++)
		{
			quint16 roomId = *ti;
			if(!IsTransporterSystemOn(roomId) && !IsTransporterSystemActive(roomId))
			{
				// The room is not on and not active.
				int working = GetPowerConsumption(SYS_Transporter, roomId);

				// Remember all rooms with enough working pads.
				if(working >= padsDesired)
				{
					// Bottom byte of 'which' value is the room 'rsub' number. The key is the 'which' value.
					transFound.insert(roomId, true);	// The map will be ordered by room 'rsub' value.
				}

				// Haven't found enough pads in one room yet so remember the room with the most working pads.
				if(working > mostworking)
				{
					mostworking = working;
					bestWhich = roomId;
				}
			}
		}

		// If we found any rooms that work, return the first one in the map, which is the one with the lowest room 'rsub' number.
		if(!transFound.isEmpty())
			return transFound.begin().key();

		// Second pass searches for a room that is on and adequate to our needs.
		for(QList<quint16>::const_iterator ti = troomIds.cbegin(); ti != troomIds.cend();  ti++)
		{
			quint16 roomId = *ti;
			if(IsTransporterSystemOn(roomId) && !IsTransporterSystemActive(roomId))
			{
				// The room is on and not active.
				int working = GetPowerConsumption(SYS_Transporter, roomId);

				// Remember all rooms with enough working pads.
				if(working >= padsDesired)
				{
					// Bottom byte of 'which' value is the room 'rsub' number. The key is the 'which' value.
					transFound.insert(roomId, true);	// The map will be ordered by room 'rsub' value.
				}

				// Haven't found enough pads in one room yet so remember the room with the most working pads.
				if(working > mostworking)
				{
					mostworking = working;
					bestWhich = roomId;
				}
			}
		}

		// If we found any rooms that work, return the first one in the map, which is the one with the lowest room 'rsub' number.
		if(!transFound.isEmpty())
			return transFound.begin().key();
	}
	return bestWhich;
}

// Returns a quint16 room void part id containing the transporter station, not an index, per se.
int DaSc::TransporterSystemIndex(quint16 which) const
{
	return which;
}

int DaSc::TransporterSystems() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.Transporters().size();
	m_DesignLock.unlock();
	return v;
}

QList<quint16> DaSc::TransporterSystemWhiches() const
{
	QList<quint16> whiches;
	m_DesignLock.lockForRead();
	whiches = m_Design.Transporters().keys();
	m_DesignLock.unlock();
	return whiches;
}

bool DaSc::TransporterVoidLocation(AuPoint& voidLoc) const
{
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	quint16 roomId = scd->RandomRoomVoidLocation(voidLoc, PERSONNEL_Everyone, m_DoorStates, m_LandingGearStates, m_Lifts);
	UnlockDesign();
	m_DesignLock.unlock();
	return roomId;
}

unsigned int DaSc::TroopBerths() const
{
	m_DesignLock.lockForRead();
	unsigned int v = m_Design.TroopBerths();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::TroopOrCrewVoidLocation(AuPoint& voidLoc) const
{
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	quint16 roomId = scd->RandomRoomVoidLocation(voidLoc, PERSONNEL_Crew|PERSONNEL_Troops, m_DoorStates, m_LandingGearStates,
			m_Lifts);
	if(!roomId)
		roomId = scd->RandomRoomVoidLocation(voidLoc, PERSONNEL_Everyone, m_DoorStates, m_LandingGearStates, m_Lifts);
	UnlockDesign();
	m_DesignLock.unlock();
	return roomId;
}

void DaSc::TurnAllFTLDrivesOff()
{
	// The FTL drive cannot be turned off in a nu ship.
	if(IsNew())
		return;

	m_FTLLock.lockForRead();
	QList<quint16> ftlIds = m_FTLDrives.keys();
	m_FTLLock.unlock();

	for(QList<quint16>::const_iterator fi = ftlIds.cbegin(); fi != ftlIds.cend(); fi++)
		TurnFTLDriveOff(*fi);
}

void DaSc::TurnFTLDriveOn(quint16 whichFTL)
{
	// Can't turn FTL drive on while getting repairs.
	if(RepairSecondsRemaining())
		return;

	// The FTL drive cannot be turned on if the ship is nu.
	if(IsNew())
		return;

	if(FTLDriveState(whichFTL) == FTL_DRIVE_Off)
	{
		m_FTLLock.lockForWrite();
		AuFTLDriveStateHash::iterator fi = m_FTLDrives.find(whichFTL);
		if(fi != m_FTLDrives.end())
		{
			(*fi).SetCommit(FTL_COMMIT_Dormant);
			(*fi).SetPower(FTL_DRIVE_Warming);
			m_FTLChg.Set();
			SetModified();
		}
		m_FTLLock.unlock();
	}
}

void DaSc::TurnFTLDriveOff(quint16 whichFTL)
{
	// The FTL drive cannot be turned off in a nu ship.
	if(IsNew())
		return;

	if(FTLDriveState(whichFTL) != FTL_DRIVE_Off)
	{
		// When warp drive is shut off while hot, the ship is left motionless.
		if(DbCommodity::IsWarpFTLDriveModule(SystemModule(SYS_FTLDrive, whichFTL)) && IsFTLDriveHot(whichFTL))
			ForceVector(AuPoint(0,0));

		// Reset FTL drive states for beginning of next FTL drive cycle.
		m_FTLLock.lockForWrite();
		AuFTLDriveStateHash::iterator fi = m_FTLDrives.find(whichFTL);
		if(fi != m_FTLDrives.end())
		{
			(*fi).SetCommit(FTL_COMMIT_Dormant);
			(*fi).SetPower(FTL_DRIVE_Off);
			m_FTLChg.Set();
		}
		m_FTLLock.unlock();
		SetModified();
	}
}

void DaSc::TurnTransporterSystemOff(int troomIndex)
{
	m_TRoomLock.lockForWrite();
	AuTransporterSystemStateHash::iterator ti = m_TRoomStates.find(troomIndex);
	if(ti != m_TRoomStates.end())
	{
		*ti = TRANSPORTER_Off;
		m_TRoomChg.Set();
		SetModified();
	}
	m_TRoomLock.unlock();
}

void DaSc::TurnTransporterSystemOn(int troomIndex)
{
	m_TRoomLock.lockForWrite();
	AuTransporterSystemStateHash::iterator ti = m_TRoomStates.find(troomIndex);
	if(ti != m_TRoomStates.end())
	{
		*ti = TRANSPORTER_On;
		m_TRoomChg.Set();
		SetModified();
	}
	m_TRoomLock.unlock();
}

double DaSc::TurretRange(quint16 which) const
{
	m_DesignLock.lockForRead();
	double t = m_Design.TurretRange(which);
	m_DesignLock.unlock();
	if(!IsInGravity())
		t *= 6;				// Farther in space so they can shoot the asteroid.
	return t;
}

int DaSc::Turrets() const
{
	m_DesignLock.lockForRead();
	int t = m_Design.Turrets().size();
	m_DesignLock.unlock();
	return t;
}

DaAtom::ATOM_TYPE DaSc::AtomType() const
{
	return ATOM_Sc;
}

bool DaSc::UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const
{
	Q_UNUSED(sceneAtom);
	Q_UNUSED(topDownView);
	return false;
}

void DaSc::UnjamDoor(unsigned int doorIndex)
{
	m_DesignLock.lockForWrite();
	if(m_DoorStates.UnjamDoor(doorIndex))
	{
		m_DoorChg.Set();
		SetModified();
	}
	m_DesignLock.unlock();
}

bool DaSc::WeaponBayCanFire(quint16 which) const
{
	m_DesignLock.lockForRead();

	if(m_Design.WeaponBays().isEmpty())
	{
		m_DesignLock.unlock();
		return false;				// Ship design has no weapon bays. This can happen on ships with a FC station but no bays.
	}
	else if(!m_Design.WeaponBays().contains(which))
	{
		qDebug() << "DaSc::WeaponBayCanFire" << Id() << Name() << "bad which" << which << "total bays"
			<< m_Design.WeaponBays().count() << "firstWhich" << m_Design.WeaponBays().begin().key();
		m_DesignLock.unlock();
		return false;				// Bad 'which' value.
	}

	m_DesignLock.unlock();

	if(IsBayFiring(which))
		return false;				// Bay is still in its firing sequence.

	return true;
}

AuPoint DaSc::WeaponBayEmitLoc(quint16 whichBay, const AuPoint& gunnerLoc) const
{
	Q_UNUSED(gunnerLoc);
	AuPoint emitLoc;
	const AuScDesign* scd = LockDesign();
	if(scd)
		emitLoc = scd->WeaponEmitLocation(whichBay);
	UnlockDesign();
	return emitLoc;
}

void DaSc::WeaponBayIds(QList<quint16>& bayIds) const
{
	m_DesignLock.lockForRead();
	bayIds = m_Design.WeaponBays().keys();
	m_DesignLock.unlock();
}

quint16 DaSc::WeaponBayNextId(quint16 which) const
{
	QList<quint16> bayIds;
	WeaponBayIds(bayIds);

	int curi = bayIds.indexOf(which) + 1;	// If not found, find return -1.

	if(curi >= bayIds.size())
		curi = 0;

	return bayIds[curi];
}

quint16 DaSc::WeaponBayPreviousId(quint16 which) const
{
	QList<quint16> bayIds;
	WeaponBayIds(bayIds);

	int curi = bayIds.indexOf(which) - 1;	// If not found, find return -1.

	if(curi < 0)
		curi = bayIds.size() - 1;

	return bayIds[curi];
}

int DaSc::WeaponBays() const
{
	m_DesignLock.lockForRead();
	int v = m_Design.WeaponBays().size();
	m_DesignLock.unlock();
	return v;
}

void DaSc::WeaponBaysWithModule(QList<quint16>& bayIds, COMMODITYID bayModule) const
{
	m_DesignLock.lockForRead();
	for(AuScReportSysVolHash::const_iterator bi = m_Design.WeaponBays().cbegin(); bi != m_Design.WeaponBays().cend(); bi++)
		if((*bi).Module() == bayModule)
			bayIds.append(bi.key());
	m_DesignLock.unlock();
}

AuPoint DaSc::WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const
{
	Q_UNUSED(shipSysLoc);
	Q_UNUSED(harvesterModule);
	return EnvTransform().Project(TargetLocation());
}

void DaSc::DesignGeometryNeeded()
{
	// Get my parent design environment, if any. I could be a preview of a design in progress.
	if(InDesignEnvironment())
		return;

	const DgData& sdd = g_Au.LockScDesignsForRead();
	if(!sdd.ScDesignIsLoaded(DesignId()))
		g_Au.ScDesignNeeded(DesignId());
	g_Au.UnlockScDesigns();
}

bool DaSc::HasTransporter() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.HasTransporter();
	m_DesignLock.unlock();
	return v;
}

void DaSc::RepairCriticalSystems()
{
	m_DamageLock.lockForWrite();
	m_DesignLock.lockForRead();
	m_Damage.RepairCritical(m_Design);	// Fix critical systems.
	m_DesignLock.unlock();
	m_DamageLock.unlock();
	RepairConditions();
}

bool DaSc::TestPowerSystem()
{
	if(CapacitorLevel() > 0)
	{
		// Capacitor has power so systems are functioning.
		bool systemsLocked = true;
		m_SystemsLock.lockForRead();
		for(AuShipSystemArray::const_iterator si = m_ssaShipSystems.cbegin(); si != m_ssaShipSystems.cend(); si++)
		{
			if((*si).GetOverclocking() && (*si).GetConnectedConsumer() >= 0)
			{
				// Found a system supply that is overclocked and in use.
				// There is a 1% chance that this will cause a problem.
				if(g_Au.Dice().Roll(1, 100) == 100)
				{
					// Damage Ship System due to overclocking.
					AuSysId sysId((*si).SysId());
					COMMODITYID sysModule = (*si).Module();
					int hitsDamage = (*si).GetHealthCurrent() / 10;	// 10% damage
					if(!hitsDamage)
						hitsDamage = 1;
					m_SystemsLock.unlock();
					systemsLocked = false;

					m_DamageLock.lockForWrite();
					m_Damage.AddDamage(sysId, sysModule, hitsDamage);
					quint32 dmg = m_Damage.HitsDamage();
					m_DamageLock.unlock();
					if(dmg >= DesignHitsTotal())
					{
						DamageAndDestroy(tr("Overclocking caused critical %1 failure.").arg(sysId.ScName()));
						return true;
					}
					else
						DamageStateChanged();
					break;
				}
			}
		}
		if(systemsLocked)
			m_SystemsLock.unlock();
	}

	// Monitor Damage Chance for each Shunt in use.
	if(m_PowerPlantShunts && PowerPlantWorking())
	{
		// Shunts are in use and power plant is functioning.
		// There are eight possible shunts.
		for(int shuntIndex=0; shuntIndex<8; shuntIndex++)
		{
			// The bits of m_PowerPlantShunts tell us which shunts are active.
			if(m_PowerPlantShunts & (1<<shuntIndex))
			{
				// Found a shunt bit that is on.
				// There is a 1% chance that this will cause a problem.
				if(g_Au.Dice().Roll(1, 100) == 1)
				{
					// Damage to power plant
					AuSysId sysId(SYS_PowerPlant);
					COMMODITYID sysModule = SystemModule(sysId);
					m_DesignLock.lockForRead();
					unsigned int hitsDamage = m_Design.PowerPlantHits() / 20;	// 5% damage
					m_DesignLock.unlock();
					if(!hitsDamage)
						hitsDamage = 1;

					m_DamageLock.lockForWrite();
					m_Damage.AddDamage(sysId, sysModule, hitsDamage);
					quint32 dmg = m_Damage.HitsDamage();
					m_DamageLock.unlock();
					if(dmg >= DesignHitsTotal())
					{
						DamageAndDestroy(tr("Shunt caused critical power plant failure."));
						return true;
					}
					else
						DamageStateChanged();
				}
			}
		}
	}
	return false;
}

// Turn rate in radians per real world second.
// Adjusted for damage.
float DaSc::TurnRate() const
{
	if(!HasManeuverDrive())
		return 0;

	unsigned int hitsLeft = GetPowerConsumption(SYS_ManeuverDrive);

	m_DesignLock.lockForRead();

	float fTurnRate = m_Design.TurnRate();

	// Decay performance according to damage.
	fTurnRate *= (float)hitsLeft / (float)m_Design.ManeuverDriveHits();

	m_DesignLock.unlock();

	// Decay performance according to skew.
	float x2 = (float)m_chTurnRateSkewX * (float)m_chTurnRateSkewX;
	float y2 = (float)m_chTurnRateSkewY * (float)m_chTurnRateSkewY;
	float rdist = sqrtf(x2 + y2);
//	const float rlongest = sqrt((28 * 28) * 2);
	const float rlongest = 39.597979746446661366447284277872f;
	float rdecay = rdist / rlongest * 0.2f;	// Up to 20% reduction in
										// performance depending on distance.
	fTurnRate = (float)(fTurnRate * (1.f - rdecay));

	// In water slows turn.
	if(WaterDepth() > 2.f)
		fTurnRate /= 2.f;

	// Reduce turn rate with velocity.
	double speed = Speed();
	if(speed > 0.)
	{
		float speedFactor = 1.f - (float)(speed / SpeedOfLight());
		if(speedFactor < .1f)
			speedFactor = .1f;			// Don't let turn rate get all the way down to 0.
		fTurnRate *= speedFactor;		// Turn rate drops as we approach speed of light.
	}

	return fTurnRate;
}

void DaSc::ConfigureHarvester()
{
	//DHH Sc
}

void DaSc::FetchVehicle()
{
	if(InArena() || InDesignPreview())
		return;

	// Spacecraft replenish their vehicle bays with vehicles from the solar system.
	if(IsInsideSpacecraft())
		return;

	// Do I have parking spots at all?
	m_DesignLock.lockForRead();
	int spots = m_Design.ParkingSpotsTotal();
	m_DesignLock.unlock();
	if(!spots)
		return;

	//DHH Check for too many vehicles aboard.
	int countVehicles = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(*ai);
		if(!vehicle)
			continue;
		countVehicles++;
	}
	if(countVehicles > spots * 2)
	{
		//qDebug() << "DaSc::FetchVehicle" << Id() << Name() << "countVehicles" << countVehicles << "total spots" << spots;
		return;
	}

	// Locate city or station within trade range of the ship.
	AuId onSlabInCity;
	DaWorld* planet;
	if(IsInBd() || IsAtBd())
	{
		// Ship is at a bd.
		planet = FindParentPlanetInEnv();
		if(!planet)
			return;
		DaBd* bd = planet->FindBd(BdId());
		if(!bd)
			return;
		if(PoliticalStance(bd->EmpireId(), EmpireId()) == POL_Enemy)
			return;		// Ship is in an enemy city.

		// We can fetch vehicles from this building's city even if it does not have an airport.
		if(!bd->CapitolId().IsNull())
			onSlabInCity = bd->CapitolId();
		else
			onSlabInCity = bd->Id();
	}
	else
	{
		bool hasStation;
		if(!StationInTradeRange(RoughRadius(), hasStation))
			return;		// Ship is not in trade range of a station.
		planet = 0;		// Get vehicles anywhere in the solar system.
	}

	// Get a count of each kind of vehicle assigned to this ship, that is in the solar system.
	QHash<VEHICLE_SPOT_TYPE, unsigned int> countByType;	// key is spot type, value is count aboard plus assigned count in the system
	DaAtomList vehiclesInSystem;
	FindEnvironmentAtom()->FindVehiclesRecursive(vehiclesInSystem, Id());
	for(DaAtomList::const_iterator it = vehiclesInSystem.begin(); it != vehiclesInSystem.end(); it++)
	{
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(*it);
		if(!vehicle)
			continue;

		QHash<VEHICLE_SPOT_TYPE, unsigned int>::iterator vci = countByType.find(vehicle->ParkingSpotType());
		if(vci == countByType.end())
			countByType.insert(vehicle->ParkingSpotType(), 1);	// First vehicle of the type
		else
			(*vci)++;										// Increase count of vehicle type.
	}

	//qDebug() << "DaSc::FetchVehicle" << Id() << Name() << "assigned vehicle types found in system" << countByType.size();
	//for(QHash<VEHICLE_SPOT_TYPE, unsigned int>::const_iterator vci = countByType.cbegin(); vci != countByType.cend(); vci++)
		//qDebug() << " " << VehicleSpotTypeName(vci.key()) << *vci;
	DaAtomList vehiclesAboard;
	FindVehiclesRecursive(vehiclesAboard);

	// The total number of each type of vehicle parking spot aboard is considered our "vehicle contingent".
	// Don't load more vehicles of each type than that, no matter where they are located.
	DaVehicle* vehicle = 0;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		// Build an array of the spot types in the order of precedence we will load them.
		VEHICLE_SPOT_TYPE loadOrder[4] =
		{
			SPOT_SpaceVehicle,
			SPOT_Rocket,
			SPOT_GroundVehicle,
			SPOT_WaterVehicle,
		};

		for(int st = 0; st < 4; st++)
		{
			VEHICLE_SPOT_TYPE spotType = loadOrder[st];
			if(!m_Design.VehicleComplement(spotType))
				continue;	// I don't have parking spots for vehicles of that type.

			// Skip vehicle type that has a full contingent.
			QHash<VEHICLE_SPOT_TYPE, unsigned int>::const_iterator cti = countByType.find(spotType);
			if(cti != countByType.cend() && (*cti) >= m_Design.VehicleComplement(spotType))
				continue;				// Complement of that vehicle type is already full or exceeded.
			//qDebug() << "DaSc::FetchVehicle" << Id() << Name() << VehicleSpotTypeName(spotType) << "has" << *cti
				//<< "of" << m_Design.VehicleComplement(spotType);

			AuMatrix4x4 parkPosM;
			if(!VehicleParkingSpot((VEHICLE_SPOT_TYPE)spotType, parkPosM, scd, vehiclesAboard))
				continue;				// No vacant parking spot of the type.
			UnlockDesign();
			m_DesignLock.unlock();

			//qDebug() << "DaSc::FetchVehicle" << Id() << Name() << "vacant spot found at" << parkPosM.Translation();
			// Try to fetch a vehicle of that type. We will stop as soon as we fetch one.
			// Locate a nu vehicle on a planet in this solar system that we can fetch.
			if(planet)
			{
				// Only look on the planet.
				// In this case, we know we can fetch vehicles from the city 'onSlabInCity' with no other prerequisites because
				// we are in that town.
				vehicle = planet->FindNewVehicleType(spotType, EmpireId(), onSlabInCity);
			}
			else
			{
				// Look everywhere in the solar system.
				DaSolarSystem* myss = FindParentSolarSystem();
				if(!myss)
					return;		// Not likely.
				vehicle = myss->FindNewVehicleType(spotType, EmpireId());
			}
			m_DesignLock.lockForRead();
			scd = LockDesign();
			if(!vehicle)
				continue;	// A fetchable nu vehicle of the type was not found so skip to the next parking spot.

			vehicle->Park(this, parkPosM, spotType);
			vehicle->SetEmpireId(EmpireId());
			vehicle->SetMotherShip(Id(), Name());
			//qDebug() << "DaSc::FetchVehicle" << Id() << Name() << "vehicle" << vehicle->Id() << vehicle->Name();
			break;	// We only fetch one vehicle per call to this function.
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();

	// Must query the vehicle's new environment after my locks are unlocked.
	if(vehicle)
	{
		vehicle->InvalidateEnvironment();
		vehicle->SetModified();
	}
}

void DaSc::CloseAutomaticDoors()
{
	QSet<quint16> doorsToClose;

	DaAtomList people;
	FindPeopleAwakeRecursive(people);	// Find players who are on line.
	FindCitizens(people);				// Find citizens such as troops and passengers.

	// Remove people from the list who do not keep automatic doors open.
	DaAtomList::iterator ai = people.begin();
	while(ai != people.end())
		if((*ai)->KeepsAutomaticDoorsOpen())
			ai++;
		else
			ai = people.erase(ai);

	if(people.isEmpty())
	{
		// Close all automatic doors that are open.
		m_DesignLock.lockForRead();
		for(AuDoorStateHash::const_iterator di = m_DoorStates.cbegin(); di != m_DoorStates.cend(); di++)
			if((*di).IsAutomaticDoor() && (*di).IsOpen() && !(*di).IsJammed())
			{
				QHash<quint16, AuTime>::const_iterator oi = m_DoorOpenTimes.find(di.key());
				if(oi != m_DoorOpenTimes.cend() && (*oi).ElapsedSec() <= 5)
					continue;	// Door hasn't been open long enough.
				doorsToClose.insert(di.key());
			}
		m_DesignLock.unlock();
	}
	else
	{
		// Close all automatic doors that are open if they are not near a person who is on line.
		// Must not call IsInSpacecraft on a subatom while my design is locked.
		DaAtomList peopleIn;
		for(DaAtomList::const_iterator ai = people.begin(); ai != people.end(); ai++)
			if((*ai)->IsInSpacecraft())
				peopleIn.append(*ai);

		m_DesignLock.lockForRead();
		for(AuDoorStateHash::const_iterator di = m_DoorStates.cbegin(); di != m_DoorStates.cend(); di++)
		{
			if(!(*di).IsAutomaticDoor() || !(*di).IsOpen() || (*di).IsJammed())
				continue;
			QHash<quint16, AuTime>::const_iterator oi = m_DoorOpenTimes.find(di.key());
			if(oi != m_DoorOpenTimes.cend() && (*oi).ElapsedSec() <= 5)
				continue;	// Door hasn't been open long enough.

			// Check for a someone who is standing near this door.
			bool isnear = false;
			for(DaAtomList::const_iterator ai = peopleIn.begin(); ai != peopleIn.end(); ai++)
			{
				// Get their location relative to this ship atom. They might be on a mount.
				AuMatrix4x4 t;
				(*ai)->GetTransformRelativeToParent(t, this);
				AuPoint loc = t.UnProject((*di).OpLoc1());	// Into the space of the creature.
				if(loc.z > -(*ai)->GroundHeightAboveCG() && loc.z < 4 && loc.Length2_2D() < 5*5)
				{
					isnear = true;
					break;
				}

				loc = t.UnProject((*di).OpLoc2());	// Into the space of the creature.
				if(loc.z > -(*ai)->GroundHeightAboveCG() && loc.z < 4 && loc.Length2_2D() < 5*5)
				{
					isnear = true;
					break;
				}
			}
			if(!isnear)
				doorsToClose.insert(di.key());
		}
		m_DesignLock.unlock();
	}

	// Now close the doors after door lock is unlocked.
	for(QSet<quint16>::const_iterator ci = doorsToClose.cbegin(); ci != doorsToClose.cend(); ci++)
		CloseDoor(*ci);
}

void DaSc::DeactivateTransporterSystems()
{
	// Make a list of all the live teleporter atoms on the ship.
	DaAtomList teleporters;
	FindTeleportersRecursive(teleporters);

	// Make a list of the room which's that contain live transporters.
	QSet<quint16> troomsWithTeleporters;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		const DaTeleporter* teleporter = dynamic_cast<const DaTeleporter*>(*ai);
		if(!teleporter)
			continue;
		if(!teleporter->IsInSpacecraft())
			continue;
		if(!IsTransporterRoom(teleporter->ShipRoomId()))
			continue;

		// Found a teleporter inside a transporter room. List the room as 'active'.
		troomsWithTeleporters.insert(teleporter->AtTransporterWhich());
	}

	m_DesignLock.lockForRead();
	for(AuScReportTransporterStationHash::const_iterator ti = m_Design.Transporters().cbegin();
			ti != m_Design.Transporters().cend(); ti++)
	{
		if(IsTransporterSystemActive(ti.key()) && !troomsWithTeleporters.contains(ti.key()))
		{
			// No teleporter in the active transporter room means it needs to be deactivated and shut off.
			TurnTransporterSystemOff(ti.key());		// Turn off automatically after activation.
		}
	}
	m_DesignLock.unlock();
}

// Render OpenAL audio sources
void DaSc::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	if(!al)
		return;
	ALenum state;
	bool isListenerAboard;
	float gain = RelativeAudioGain(sceneAtom) * fade;
	AuPoint epos;	// sound emission position
	if(sceneAtom->IsInSpacecraft() && sceneAtom->FindParentVesselInEnv() == this)
	{
		// The listener is aboard so the emission location is calculated for individual sounds.
		isListenerAboard = true;
		if(sceneAtom->HelmetClosed())
			gain *= .5f;
	}
	else
	{
		// The listener is not aboard so all sounds emanate from (0,0).
		isListenerAboard = false;
		epos = model.Translation();
	}

	// Siren
	alGetSourcei(m_AudioShip[AS_Siren], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain))
	{
		if(m_Siren == 3)
		{
			// Condition red.
			m_Siren = 0;
			alSourceStop(m_AudioShip[AS_Siren]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_Siren], AL_BUFFER, al->AudioBuffer(SOUND_AlertRed));
			if(isListenerAboard)
				epos = model.Project(sceneAtom->ClosestSiren());
			alSource3f(m_AudioShip[AS_Siren], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_Siren], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_AlertRed));
			else
				alSourcef(m_AudioShip[AS_Siren], AL_GAIN, gain*10 * AuSound::SoundGain(SOUND_AlertRed));
			alSourcef(m_AudioShip[AS_Siren], AL_REFERENCE_DISTANCE, (float)AuSound::SoundReferenceDistance(SOUND_AlertRed));
			alSourcef(m_AudioShip[AS_Siren], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_AlertRed));
			alSourcePlay(m_AudioShip[AS_Siren]);
		}
		else if(m_Siren == 2)
		{
			// Condition yellow.
			m_Siren = 0;
			alSourceStop(m_AudioShip[AS_Siren]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_Siren], AL_BUFFER, al->AudioBuffer(SOUND_AlertYellow));
			if(isListenerAboard)
				epos = model.Project(sceneAtom->ClosestSiren());
			alSource3f(m_AudioShip[AS_Siren], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_Siren], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_AlertYellow));
			else
				alSourcef(m_AudioShip[AS_Siren], AL_GAIN, gain*10 * AuSound::SoundGain(SOUND_AlertYellow));
			alSourcef(m_AudioShip[AS_Siren], AL_REFERENCE_DISTANCE, (float)AuSound::SoundReferenceDistance(SOUND_AlertYellow));
			alSourcef(m_AudioShip[AS_Siren], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_AlertYellow));
			alSourcePlay(m_AudioShip[AS_Siren]);
		}
		else if(m_Siren == 1)
		{
			// Condition green.
			m_Siren = 0;
			alSourceStop(m_AudioShip[AS_Siren]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_Siren], AL_BUFFER, al->AudioBuffer(SOUND_AlertGreen));
			if(isListenerAboard)
				epos = model.Project(sceneAtom->ClosestSiren());
			alSource3f(m_AudioShip[AS_Siren], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_Siren], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_AlertGreen));
			else
				alSourcef(m_AudioShip[AS_Siren], AL_GAIN, gain*10 * AuSound::SoundGain(SOUND_AlertGreen));
			alSourcef(m_AudioShip[AS_Siren], AL_REFERENCE_DISTANCE, (float)AuSound::SoundReferenceDistance(SOUND_AlertGreen));
			alSourcef(m_AudioShip[AS_Siren], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_AlertGreen));
			alSourcePlay(m_AudioShip[AS_Siren]);
		}
	}
	else
	{
		// Siren is silent
		if(state == AL_PLAYING)
			alSourceStop(m_AudioShip[AS_Siren]);
		m_Siren = 0;
	}

	// Lift Door 1
	alGetSourcei(m_AudioShip[AS_DoorLift1], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain))
	{
		if(m_DoorLift1 == 2)
		{
			m_DoorLift1 = 0;
			alSourceStop(m_AudioShip[AS_DoorLift1]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorLift1], AL_BUFFER, al->AudioBuffer(SOUND_DoorHatchClose));
			if(isListenerAboard)
				epos = model.Project(m_DoorLift1Loc);
			alSource3f(m_AudioShip[AS_DoorLift1], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_DoorLift1], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorHatchClose));
			else
				alSourcef(m_AudioShip[AS_DoorLift1], AL_GAIN, gain*10 * AuSound::SoundGain(SOUND_DoorHatchClose));
			alSourcef(m_AudioShip[AS_DoorLift1], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorHatchClose));
			alSourcef(m_AudioShip[AS_DoorLift1], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorHatchClose));
			alSourcePlay(m_AudioShip[AS_DoorLift1]);
		}
		else if(m_DoorLift1 == 1)
		{
			m_DoorLift1 = 0;
			alSourceStop(m_AudioShip[AS_DoorLift1]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorLift1], AL_BUFFER, al->AudioBuffer(SOUND_DoorHatchOpen));
			if(isListenerAboard)
				epos = model.Project(m_DoorLift1Loc);
			alSource3f(m_AudioShip[AS_DoorLift1], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_DoorLift1], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorHatchOpen));
			else
				alSourcef(m_AudioShip[AS_DoorLift1], AL_GAIN, gain*10 * AuSound::SoundGain(SOUND_DoorHatchOpen));
			alSourcef(m_AudioShip[AS_DoorLift1], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorHatchOpen));
			alSourcef(m_AudioShip[AS_DoorLift1], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorHatchOpen));
			alSourcePlay(m_AudioShip[AS_DoorLift1]);
		}
	}
	else
	{
		// Lift door 1 is silent
		if(state == AL_PLAYING)
			alSourceStop(m_AudioShip[AS_DoorLift1]);
		m_DoorLift1 = 0;
	}

	// Exterior Door
	alGetSourcei(m_AudioShip[AS_DoorExt], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain))
	{
		if(m_DoorExt == 2)
		{
			m_DoorExt = 0;
			alSourceStop(m_AudioShip[AS_DoorExt]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorExt], AL_BUFFER, al->AudioBuffer(SOUND_DoorExtClose));
			if(isListenerAboard)
				epos = model.Project(m_DoorExtLoc);
			alSource3f(m_AudioShip[AS_DoorExt], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_DoorExt], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorExtClose));
			else
				alSourcef(m_AudioShip[AS_DoorExt], AL_GAIN, gain*10 * AuSound::SoundGain(SOUND_DoorExtClose));
			alSourcef(m_AudioShip[AS_DoorExt], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorExtClose));
			alSourcef(m_AudioShip[AS_DoorExt], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorExtClose));
			alSourcePlay(m_AudioShip[AS_DoorExt]);
		}
		else if(m_DoorExt == 1)
		{
			m_DoorExt = 0;
			alSourceStop(m_AudioShip[AS_DoorExt]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorExt], AL_BUFFER, al->AudioBuffer(SOUND_DoorExtOpen));
			if(isListenerAboard)
				epos = model.Project(m_DoorExtLoc);
			alSource3f(m_AudioShip[AS_DoorExt], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_DoorExt], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorExtOpen));
			else
				alSourcef(m_AudioShip[AS_DoorExt], AL_GAIN, gain*10 * AuSound::SoundGain(SOUND_DoorExtOpen));
			alSourcef(m_AudioShip[AS_DoorExt], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorExtOpen));
			alSourcef(m_AudioShip[AS_DoorExt], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorExtOpen));
			alSourcePlay(m_AudioShip[AS_DoorExt]);
		}
	}
	else
	{
		// Ext door is silent
		if(state == AL_PLAYING)
			alSourceStop(m_AudioShip[AS_DoorExt]);
		m_DoorExt = 0;
	}

	// Lift Door 2
	alGetSourcei(m_AudioShip[AS_DoorLift2], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain))
	{
		if(m_DoorLift2 == 2)
		{
			m_DoorLift2 = 0;
			alSourceStop(m_AudioShip[AS_DoorLift2]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorLift2], AL_BUFFER, al->AudioBuffer(SOUND_DoorHatchClose));
			if(isListenerAboard)
				epos = model.Project(m_DoorLift2Loc);
			alSource3f(m_AudioShip[AS_DoorLift2], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_DoorLift2], AL_GAIN, gain * AuSound::SoundGain(SOUND_DoorHatchClose));
			else
				alSourcef(m_AudioShip[AS_DoorLift2], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorHatchClose));
			alSourcef(m_AudioShip[AS_DoorLift2], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorHatchClose));
			alSourcef(m_AudioShip[AS_DoorLift2], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorHatchClose));
			alSourcePlay(m_AudioShip[AS_DoorLift2]);
		}
		else if(m_DoorLift2 == 1)
		{
			m_DoorLift2 = 0;
			alSourceStop(m_AudioShip[AS_DoorLift2]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorLift2], AL_BUFFER, al->AudioBuffer(SOUND_DoorHatchOpen));
			if(isListenerAboard)
				epos = model.Project(m_DoorLift2Loc);
			alSource3f(m_AudioShip[AS_DoorLift2], AL_POSITION, epos.x, epos.y, epos.z);
			if(isListenerAboard)
				alSourcef(m_AudioShip[AS_DoorLift2], AL_GAIN, gain * AuSound::SoundGain(SOUND_DoorHatchOpen));
			else
				alSourcef(m_AudioShip[AS_DoorLift2], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorHatchOpen));
			alSourcef(m_AudioShip[AS_DoorLift2], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorHatchOpen));
			alSourcef(m_AudioShip[AS_DoorLift2], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorHatchOpen));
			alSourcePlay(m_AudioShip[AS_DoorLift2]);
		}
	}
	else
	{
		// Lift door 2 is silent
		if(state == AL_PLAYING)
			alSourceStop(m_AudioShip[AS_DoorLift2]);
		m_DoorLift2 = 0;
	}

	// Interior Door 1
	alGetSourcei(m_AudioShip[AS_DoorInt1], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain))
	{
		if(m_DoorInt1 == 2)
		{
			m_DoorInt1 = 0;
			alSourceStop(m_AudioShip[AS_DoorInt1]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorInt1], AL_BUFFER, al->AudioBuffer(SOUND_DoorIntClose));
			if(isListenerAboard)
				epos = model.Project(m_DoorInt1Loc);
			alSource3f(m_AudioShip[AS_DoorInt1], AL_POSITION, epos.x, epos.y, epos.z);
			alSourcef(m_AudioShip[AS_DoorInt1], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorIntClose));
			alSourcef(m_AudioShip[AS_DoorInt1], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorIntClose));
			alSourcef(m_AudioShip[AS_DoorInt1], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorIntClose));
			alSourcePlay(m_AudioShip[AS_DoorInt1]);
		}
		else if(m_DoorInt1 == 1)
		{
			m_DoorInt1 = 0;
			alSourceStop(m_AudioShip[AS_DoorInt1]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorInt1], AL_BUFFER, al->AudioBuffer(SOUND_DoorIntOpen));
			if(isListenerAboard)
				epos = model.Project(m_DoorInt1Loc);
			alSource3f(m_AudioShip[AS_DoorInt1], AL_POSITION, epos.x, epos.y, epos.z);
			alSourcef(m_AudioShip[AS_DoorInt1], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorIntOpen));
			alSourcef(m_AudioShip[AS_DoorInt1], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorIntOpen));
			alSourcef(m_AudioShip[AS_DoorInt1], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorIntOpen));
			alSourcePlay(m_AudioShip[AS_DoorInt1]);
		}
	}
	else
	{
		// Int door is silent
		if(state == AL_PLAYING)
			alSourceStop(m_AudioShip[AS_DoorInt1]);
		m_DoorInt1 = 0;
	}

	// Interior Door 2
	alGetSourcei(m_AudioShip[AS_DoorInt2], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain))
	{
		if(m_DoorInt2 == 2)
		{
			m_DoorInt2 = 0;
			alSourceStop(m_AudioShip[AS_DoorInt2]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorInt2], AL_BUFFER, al->AudioBuffer(SOUND_DoorIntClose));
			if(isListenerAboard)
				epos = model.Project(m_DoorInt2Loc);
			alSource3f(m_AudioShip[AS_DoorInt2], AL_POSITION, epos.x, epos.y, epos.z);
			alSourcef(m_AudioShip[AS_DoorInt2], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorIntClose));
			alSourcef(m_AudioShip[AS_DoorInt2], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorIntClose));
			alSourcef(m_AudioShip[AS_DoorInt2], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorIntClose));
			alSourcePlay(m_AudioShip[AS_DoorInt2]);
		}
		else if(m_DoorInt2 == 1)
		{
			m_DoorInt2 = 0;
			alSourceStop(m_AudioShip[AS_DoorInt2]);	// Stop the current buffer.
			alSourcei(m_AudioShip[AS_DoorInt2], AL_BUFFER, al->AudioBuffer(SOUND_DoorIntOpen));
			if(isListenerAboard)
				epos = model.Project(m_DoorInt2Loc);
			alSource3f(m_AudioShip[AS_DoorInt2], AL_POSITION, epos.x, epos.y, epos.z);
			alSourcef(m_AudioShip[AS_DoorInt2], AL_GAIN, gain*2 * AuSound::SoundGain(SOUND_DoorIntOpen));
			alSourcef(m_AudioShip[AS_DoorInt2], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_DoorIntOpen));
			alSourcef(m_AudioShip[AS_DoorInt2], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_DoorIntOpen));
			alSourcePlay(m_AudioShip[AS_DoorInt2]);
		}
	}
	else
	{
		// Int door is silent
		if(state == AL_PLAYING)
			alSourceStop(m_AudioShip[AS_DoorInt2]);
		m_DoorInt2 = 0;
	}

	// Sensors
	alGetSourcei(m_AudioShip[AS_Sensors], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain) && SensorsWorking())
	{
		// Sensors are on.
		if(state == AL_INITIAL)
		{
			alSourcei(m_AudioShip[AS_Sensors], AL_LOOPING, AL_TRUE);
			alSourcePlay(m_AudioShip[AS_Sensors]);
		}
		else if(state == AL_PAUSED || state == AL_STOPPED)
			alSourcePlay(m_AudioShip[AS_Sensors]);
		if(isListenerAboard)
			epos = model.Project(sceneAtom->ClosestSensor());

		alSource3f(m_AudioShip[AS_Sensors], AL_POSITION, epos.x, epos.y, epos.z);
		alSourcef(m_AudioShip[AS_Sensors], AL_GAIN, gain * AuSound::SoundGain(SOUND_Telemetry));
		alSourcef(m_AudioShip[AS_Sensors], AL_REFERENCE_DISTANCE, (float)AuSound::SoundReferenceDistance(SOUND_Telemetry));
		alSourcef(m_AudioShip[AS_Sensors], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_Telemetry));
	}
	else
	{
		// Sensors are off.
		if(state == AL_PLAYING)
			alSourcePause(m_AudioShip[AS_Sensors]);
	}

	// Power Plant
	alGetSourcei(m_AudioShip[AS_PowerPlant], AL_SOURCE_STATE, &state);
	if(!IsFuzzyZero(gain) && PowerPlantWorking())
	{
		// Power plant is on.
		if(state == AL_INITIAL)
		{
			alSourcei(m_AudioShip[AS_PowerPlant], AL_LOOPING, AL_TRUE);
			alSourcePlay(m_AudioShip[AS_PowerPlant]);
		}
		else if(state == AL_PAUSED || state == AL_STOPPED)
		{
			alSourcePlay(m_AudioShip[AS_PowerPlant]);
		}
		if(isListenerAboard)
		{
			epos = model.Project(sceneAtom->ClosestPowerPlant());
			alSourcef(m_AudioShip[AS_PowerPlant], AL_MIN_GAIN, .0125f*fade * AuSound::SoundGain(SOUND_PowerPlant));
			alSourcef(m_AudioShip[AS_PowerPlant], AL_GAIN, gain/2 * AuSound::SoundGain(SOUND_PowerPlant));
		}
		else
		{
			alSourcef(m_AudioShip[AS_PowerPlant], AL_MIN_GAIN, 0);
			alSourcef(m_AudioShip[AS_PowerPlant], AL_GAIN, gain * AuSound::SoundGain(SOUND_PowerPlant));
		}
		alSourcef(m_AudioShip[AS_PowerPlant], AL_REFERENCE_DISTANCE, (float)AuSound::SoundReferenceDistance(SOUND_PowerPlant));
		alSourcef(m_AudioShip[AS_PowerPlant], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_PowerPlant));
		alSource3f(m_AudioShip[AS_PowerPlant], AL_POSITION, epos.x, epos.y, epos.z);
	}
	else
	{
		// Power plant is off.
		if(state == AL_PLAYING)
			alSourcePause(m_AudioShip[AS_PowerPlant]);
	}

	// Gravity Drive
	if(isListenerAboard)
	{
		// Exterior sound is off.
		alGetSourcei(m_AudioShip[AS_GravityDriveExt], AL_SOURCE_STATE, &state);
		if(state == AL_PLAYING)
			alSourcePause(m_AudioShip[AS_GravityDriveExt]);

		// Interior sound is on.
		alGetSourcei(m_AudioShip[AS_GravityDriveInt], AL_SOURCE_STATE, &state);
		if(GravityDriveWorking())
		{
			// Gravity drive is on.
			if(state == AL_INITIAL)
			{
				alSourcei(m_AudioShip[AS_GravityDriveInt], AL_LOOPING, AL_TRUE);
				alSourcePlay(m_AudioShip[AS_GravityDriveInt]);
			}
			else if(state == AL_PAUSED || state == AL_STOPPED)
			{
				alSourcePlay(m_AudioShip[AS_GravityDriveInt]);
			}

			if(m_eGravityDriveState == GD_On)
			{
				// Normal power
				epos = model.Project(sceneAtom->ClosestGravityDrive());
				alSourcef(m_AudioShip[AS_GravityDriveInt], AL_MIN_GAIN, .025f*fade * AuSound::SoundGain(SOUND_GravityDriveInt));
				alSourcef(m_AudioShip[AS_GravityDriveInt], AL_MAX_GAIN, 1);

				float throttleGain;
				if(m_wActions & ACT_Brake)
					throttleGain = 10;
				else
					throttleGain = abs(ThrustLongitudinal());
				float throttleLatGain;
				if(m_wActions & ACT_BrakeLateral)
					throttleLatGain = 4;
				else
					throttleLatGain = abs(ThrustLateral());
				float throttleVertGain;
				if(m_wActions & ACT_BrakeVertical)
					throttleVertGain = 6;
				else
					throttleVertGain = abs(ThrustVertical());

				// Engine noise is muffled inside the hull, as compared to
				// exterior gain calculation.
				float gain = throttleGain + throttleLatGain + throttleVertGain + 1;
				gain *= fade;
				alSourcef(m_AudioShip[AS_GravityDriveInt], AL_GAIN, gain * AuSound::SoundGain(SOUND_GravityDriveInt));
			}
			else
			{
				// Hold power
				epos = model.Project(sceneAtom->ClosestGravityDrive());
				alSourcef(m_AudioShip[AS_GravityDriveInt], AL_MIN_GAIN,.0125f*fade * AuSound::SoundGain(SOUND_GravityDriveInt));
				alSourcef(m_AudioShip[AS_GravityDriveInt], AL_MAX_GAIN, .2f * AuSound::SoundGain(SOUND_GravityDriveInt));
				float gain = .3f * fade;
				alSourcef(m_AudioShip[AS_GravityDriveInt], AL_GAIN, gain * AuSound::SoundGain(SOUND_GravityDriveInt));
			}
			alSourcef(m_AudioShip[AS_GravityDriveInt], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_GravityDriveInt));
			alSourcef(m_AudioShip[AS_GravityDriveInt], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_GravityDriveInt));
			alSource3f(m_AudioShip[AS_GravityDriveInt], AL_POSITION, epos.x, epos.y, epos.z);
		}
		else
		{
			// Gravity drive is off.
			if(state == AL_PLAYING)
				alSourcePause(m_AudioShip[AS_GravityDriveInt]);
		}
	}
	else
	{
		// Listener is not aboard.
		// Exterior sound is on.
		alGetSourcei(m_AudioShip[AS_GravityDriveExt], AL_SOURCE_STATE, &state);
		if(GravityDriveWorking() && !IsFuzzyZero(gain))
		{
			// Gravity drive is on.
			if(state == AL_INITIAL)
			{
				alSourcei(m_AudioShip[AS_GravityDriveExt], AL_LOOPING, AL_TRUE);
				alSourcePlay(m_AudioShip[AS_GravityDriveExt]);
			}
			else if(state == AL_PAUSED || state == AL_STOPPED)
			{
				alSourcePlay(m_AudioShip[AS_GravityDriveExt]);
			}

			if(m_eGravityDriveState == GD_On)
			{
				// Normal power
				alSourcef(m_AudioShip[AS_GravityDriveExt], AL_MIN_GAIN, 0);
				alSourcef(m_AudioShip[AS_GravityDriveExt], AL_MAX_GAIN, 1 * AuSound::SoundGain(SOUND_GravityDriveExt));

				float throttleGain;
				if(m_wActions & ACT_Brake)
					throttleGain = 10;
				else
					throttleGain = abs(ThrustLongitudinal());
				float throttleLatGain;
				if(m_wActions & ACT_BrakeLateral)
					throttleLatGain = 4;
				else
					throttleLatGain = abs(ThrustLateral());
				float throttleVertGain;
				if(m_wActions & ACT_BrakeVertical)
					throttleVertGain = 6;
				else
					throttleVertGain = abs(ThrustVertical());

				float tgain = throttleGain + throttleLatGain + throttleVertGain + 1;
				tgain *= 10;
				tgain *= fade;
				alSourcef(m_AudioShip[AS_GravityDriveExt], AL_GAIN, tgain * AuSound::SoundGain(SOUND_GravityDriveExt));
			}
			else
			{
				// Hold power
				alSourcef(m_AudioShip[AS_GravityDriveExt], AL_MIN_GAIN, 0);
				alSourcef(m_AudioShip[AS_GravityDriveExt], AL_MAX_GAIN, .2f * AuSound::SoundGain(SOUND_GravityDriveExt));
				alSourcef(m_AudioShip[AS_GravityDriveExt], AL_GAIN, gain * 5 * AuSound::SoundGain(SOUND_GravityDriveExt));
			}
			alSourcef(m_AudioShip[AS_GravityDriveExt], AL_REFERENCE_DISTANCE,
					(float)AuSound::SoundReferenceDistance(SOUND_GravityDriveExt));
			alSourcef(m_AudioShip[AS_GravityDriveExt], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_GravityDriveExt));
			alSource3f(m_AudioShip[AS_GravityDriveExt], AL_POSITION, epos.x, epos.y, epos.z);
		}
		else
		{
			// Gravity drive is off.
			if(state == AL_PLAYING)
				alSourcePause(m_AudioShip[AS_GravityDriveExt]);
		}

		// Interior sound is off.
		alGetSourcei(m_AudioShip[AS_GravityDriveInt], AL_SOURCE_STATE, &state);
		if(state == AL_PLAYING)
			alSourcePause(m_AudioShip[AS_GravityDriveInt]);
	}

	// Tractor beam
	alGetSourcei(m_AudioShip[AS_TractorBeam], AL_SOURCE_STATE, &state);
	AuPoint tpos;
	if(!IsFuzzyZero(gain) && IsTractorBeamAttached(tpos))
	{
		// Tractor beam is attached to something.
		if(state == AL_INITIAL)
		{
			alSourcei(m_AudioShip[AS_TractorBeam], AL_LOOPING, AL_TRUE);
			alSourcePlay(m_AudioShip[AS_TractorBeam]);
		}
		else if(state == AL_PAUSED || state == AL_STOPPED)
			alSourcePlay(m_AudioShip[AS_TractorBeam]);
		if(isListenerAboard)
			epos = model.Project(tpos);

		alSource3f(m_AudioShip[AS_TractorBeam], AL_POSITION, epos.x, epos.y, epos.z);
		alSourcef(m_AudioShip[AS_TractorBeam], AL_PITCH, 1.f);
		alSourcef(m_AudioShip[AS_TractorBeam], AL_GAIN, gain * 10 * AuSound::SoundGain(SOUND_TractorBeam));
		alSourcef(m_AudioShip[AS_TractorBeam], AL_REFERENCE_DISTANCE,
				(float)AuSound::SoundReferenceDistance(SOUND_TractorBeam));
		alSourcef(m_AudioShip[AS_TractorBeam], AL_ROLLOFF_FACTOR, AuSound::SoundRolloffFactor(SOUND_TractorBeam));
	}
	else
	{
		// Tractor beam is off.
		if(state == AL_PLAYING)
			alSourcePause(m_AudioShip[AS_TractorBeam]);
	}
}

void DaSc::RecalcSystemsHealthMax()
{
	m_SystemsLock.lockForWrite();
	// Recalc maximum and current hit points in systems data.
	m_DesignLock.lockForRead();
	m_ssaShipSystems.RecalcHealthMax(m_Design);
	m_tmSystemsUpg.Set();
	m_DesignLock.unlock();

	// Trim damage to fit within the new limits.
	m_DamageLock.lockForWrite();
	for(AuShipSystemArray::const_iterator ssi = m_ssaShipSystems.constBegin(); ssi != m_ssaShipSystems.constEnd(); ssi++)
	{
		AuSysId sysId((*ssi).SysId());
		unsigned int dmg = m_Damage.HitsDamageAddParts(sysId);
		if(dmg > (*ssi).GetHealthMax())
		{
			// The system has more damage than it has hit points.
			// Restore it to full health.
			m_Damage.RepairAll(sysId);
			m_tmDamageChange.Set();
		}
	}
	m_DamageLock.unlock();
	m_SystemsLock.unlock();
}

void DaSc::InitSystems(quint8 quality)
{
	Q_ASSERT(quality);
	if(g_Au.m_LogMessages & LOG_AtomCreate)
		qDebug("DaSc::InitSystems %d quality %d", Id().GetID(), quality);
	m_QMade = quality;

	m_SystemsLock.lockForWrite();
	if(!m_ssaShipSystems.isEmpty())
		qWarning() << "error: DaSc::InitSystems reinitializing systems that were already initialized. "
			"Connection changes (if any) will be lost." << Id() << Name();
	m_DesignLock.lockForWrite();

	// Set the quality of everything in the design report. The numbers are currently the analysis minimums.
	m_Design.SetQuality(quality);

	// Gather the listing of all ship systems.
	// This is only done once for each valid design that is not in a design studio, unless it is refit.
	GatherSystemsDataSystemsAndDesignLocked(quality);

	// Start with all doors closed. They will be in whatever states they were left when the design was finalized.
	m_DoorStates.CloseAllDoors();
	m_DesignLock.unlock();

	UpdateSystemsHealthSystemsLocked();
	UpdateSystemsPowerSystemsLocked();
	m_SystemsLock.unlock();
	//qDebug("DaSc::InitSystems %d %s Ship Systems Initialized, SystemCount = %d. "
			//"This should never happen again for this ship.",
			//Id().GetID(), (const char*)Name().toAscii(), m_ssaShipSystems.count());
}

void DaSc::GatherSystemsDataSystemsAndDesignLocked(quint8 quality)
{
	m_ssaShipSystems.clear();
	m_ssaShipSystems.GatherShipSystems(quality, m_Design);
	m_tmSystemsUpg.Set();
}

void DaSc::ReconcileSystems()
{
	m_DesignLock.lockForWrite();
	if(m_ssaShipSystems.Reconcile(m_Design))
	{
		m_tmSystemsUpg.Set();
		//qDebug() << "DaSc::ReconcileSystems" << *this;
	}
	m_DesignLock.unlock();
}

void DaSc::RenderDesign(const AuMatrix4x4& model, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, bool transparentPass,
		bool isAboard, bool insideVisible, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe,
		DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	//qDebug() << "DaSc::RenderDesign" << Id() << Name();

	// Don't continue if we have nothing to draw in this pass.
	if(transparentPass)
	{
		if(m_TransparentVAOs.empty() &&
				m_RoomTransparentVAOs.empty() &&
				m_IntRoomTransparentVAOs.empty() &&
				m_DecalTransparentVAOs.empty() &&
				m_LiftTransparentVAOs.empty() &&
				m_OwnDecalTransparentVAOs.empty() &&
				m_OwnTransparentVAOs.empty() &&
				m_TurBaseTransparentVAOs.empty() &&
				m_TurGunTransparentVAOs.empty() &&
				m_WindowTransparentVAOs.empty() &&
				m_DoorTransparentVAOs.empty() &&
				m_IntDoorTransparentVAOs.empty() &&
				m_LandingGearTransparentVAOs.empty() &&
				m_LensBeaconTransparentVAOs.empty() &&
				m_LensHeadlightTransparentVAOs.empty() &&
				m_LensNavTransparentVAOs.empty() &&
				m_LensStrobeTransparentVAOs.empty())
			return;	// No transparent VAOs to draw.
	}
	else
	{
		if(m_OpaqueVAOs.empty() &&
				m_RoomOpaqueVAOs.empty() &&
				m_IntRoomOpaqueVAOs.empty() &&
				m_DecalOpaqueVAOs.empty() &&
				m_LiftOpaqueVAOs.empty() &&
				m_OwnDecalOpaqueVAOs.empty() &&
				m_OwnOpaqueVAOs.empty() &&
				m_TurBaseOpaqueVAOs.empty() &&
				m_TurGunOpaqueVAOs.empty() &&
				m_WindowTranslucentVAOs.empty() &&
				m_DoorOpaqueVAOs.empty() &&
				m_IntDoorOpaqueVAOs.empty() &&
				m_LandingGearOpaqueVAOs.empty() &&
				m_LensBeaconOpaqueVAOs.empty() &&
				m_LensHeadlightOpaqueVAOs.empty() &&
				m_LensNavOpaqueVAOs.empty() &&
				m_LensStrobeOpaqueVAOs.empty())
			return;	// No opaque VAOs to draw.
	}

	// Set current ambient light and fog states.
	SceneRenderParameters(sceneAtom, m_AmbientR, m_AmbientG, m_AmbientB, m_FogDensity, m_FogR, m_FogG, m_FogB, ari.IsViewTopDown(),
			m_SunsBlocked);
	m_AmbientUp = model.ProjectVector(0,0,1);
	m_AmbientUp.Unit();
	m_InsideVisible = insideVisible;
	m_IsAboard = isAboard;

	m_MyExtLight = -1;
	for(unsigned int lightIndex=0; lightIndex<ari.re->MaxLights(); lightIndex++)
	{
		const AuGLLight& light = ari.re->Light(lightIndex);
		if(light.IsAtomLight(Id()))
		{
			m_MyExtLight = lightIndex;
			break;
		}
	}

	DaWorldRingSection* rsworld = dynamic_cast<DaWorldRingSection*>(Parent());
	if(rsworld)
	{
		if(m_FogDensity > 0)
			shadScRingworldFog.append(this);
		else
			shadScRingworld.append(this);
	}
	else if(IsInAtmosphere() && dynamic_cast<DaWorldSphere*>(Parent()))
	{
		if(m_FogDensity > 0)
			shadScGlobeFog.append(this);
		else
			shadScGlobe.append(this);
	}
	else
	{
		if(m_FogDensity > 0)
			shadScFog.append(this);
		else
			shadSc.append(this);
	}
}

void DaSc::RenderDesignVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader)
{
	for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
		(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
}

void DaSc::RenderWindowVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader)
{
	if(CapacitorLevel())
	{
		// Ship has power for lights.
		// Current SetGlowDiscard values.
		float curGlowR = 0;
		float curGlowG = 0;
		float curGlowB = 0;
		for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
		{
			// Determine lit state of window.
			float glowR, glowG, glowB;
			QHash<quint16, QString>::const_iterator ri = m_WindowRooms.find(vi.key());
			if(ri != m_WindowRooms.cend())
			{
				WindowSurfaceGlow(*ri, glowR, glowG, glowB);
			}
			else
			{
				// No glow when no lights at all.
				glowR = 0;
				glowG = 0;
				glowB = 0;
			}

			if(glowR != curGlowR || glowG != curGlowG || glowB != curGlowB)
			{
				curGlowR = glowR;
				curGlowG = glowG;
				curGlowB = glowB;
				shader.SetGlowDiscard(curGlowR,curGlowG,curGlowB, .01f);
			}

			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		// Restore glow state.
		if(curGlowR != 0 || curGlowG != 0 || curGlowB != 0)
			shader.SetGlowDiscard(0,0,0, .01f);
	}
	else
	{
		// Ship does not have power for lights.
		for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
	}
}

void DaSc::RenderOwnTextureVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader)
{
	GLuint curTex = 0;
	for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
	{
		QHash<quint16, GLuint>::const_iterator ti = m_OwnTextures.find(vi.key());
		if(!(*ti))
			continue;
		if(*ti != curTex)
		{
			shader.SetSurfaceTexture(0, *ti);
			curTex = *ti;
		}
		(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
	}
}

void DaSc::RenderRoomVAOs(QHash<quint16, AuScVAO>& vaos, const AuGLSLScBase& shader)
{
	float curAmbR = -1;
	float curAmbG = -1;
	float curAmbB = -1;
	for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
	{
		// Get lighting conditions in the room.
		float ambR, ambG, ambB;
		bool sunsBlocked;
		QHash<quint16, QString>::const_iterator ri = m_RoomNames.find(vi.key());
		if(ri != m_RoomNames.cend())
			InteriorAmbientLight(*ri, ambR, ambG, ambB, sunsBlocked);
		else
			InteriorAmbientLight("", ambR, ambG, ambB, sunsBlocked);

		if(ambR != curAmbR || ambG != curAmbG || ambB != curAmbB)
		{
			curAmbR = ambR;
			curAmbG = ambG;
			curAmbB = ambB;
			shader.SetAmbientLight(curAmbR, curAmbG, curAmbB);
		}

		(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
	}
}

void DaSc::RenderInteriorRoomVAOs(QHash<quint16, AuScVAO>& vaos, AuAtomRenderInfo& ari, const AuGLSLScBase& shader)
{
	// Disable suns that were enabled.
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		if(sun.m_Enabled)
			shader.SetSunEnabled(sunIndex, false);
	}

	float curAmbR = -1;
	float curAmbG = -1;
	float curAmbB = -1;
	for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
	{
		// Get lighting conditions in the room.
		float ambR, ambG, ambB;
		bool sunsBlocked;
		QHash<quint16, QString>::const_iterator ri = m_RoomNames.find(vi.key());
		if(ri != m_RoomNames.cend())
			InteriorAmbientLight(*ri, ambR, ambG, ambB, sunsBlocked);
		else
			InteriorAmbientLight("", ambR, ambG, ambB, sunsBlocked);

		if(ambR != curAmbR || ambG != curAmbG || ambB != curAmbB)
		{
			curAmbR = ambR;
			curAmbG = ambG;
			curAmbB = ambB;
			shader.SetAmbientLight(curAmbR, curAmbG, curAmbB);
		}

		(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
	}

	// Restore suns.
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		if(sun.m_Enabled)
			shader.SetSunEnabled(sunIndex, true);
	}
}

void DaSc::RenderLiftVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		const AuGLSLScBase& shader)
{
	// Disable suns that were enabled.
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		if(sun.m_Enabled)
			shader.SetSunEnabled(sunIndex, false);
	}

	float curAmbR = -1;
	float curAmbG = -1;
	float curAmbB = -1;
	AuMatrix4x4 stopM;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
		{
			AuLiftHash::iterator li = m_Lifts.find(vi.key());
			if(li == m_Lifts.cend())
				continue;
			scd->TurboLiftStopMatrix(vi.key(), *li, stopM);
			AuMatrix4x4 liftM = model * stopM;
			shader.SetModelView(liftM);

			// Get lighting conditions in the room.
			float ambR, ambG, ambB;
			bool sunsBlocked;
			QHash<quint16, QString>::const_iterator ri = m_RoomNames.find(vi.key());
			if(ri != m_RoomNames.cend())
				InteriorAmbientLight(*ri, ambR, ambG, ambB, sunsBlocked);
			else
				InteriorAmbientLight("", ambR, ambG, ambB, sunsBlocked);

			if(ambR != curAmbR || ambG != curAmbG || ambB != curAmbB)
			{
				curAmbR = ambR;
				curAmbG = ambG;
				curAmbB = ambB;
				shader.SetAmbientLight(curAmbR, curAmbG, curAmbB);
			}

			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		shader.SetModelView(model);
	}
	UnlockDesign();
	m_DesignLock.unlock();

	// Restore suns.
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		if(sun.m_Enabled)
			shader.SetSunEnabled(sunIndex, true);
	}
}

void DaSc::RenderDoorVAOs(QHash<quint32, AuScVAO>& vaos, const AuGLSLScBase& shader, quint16 sceneRoom, float extAmbR,
		float extAmbG, float extAmbB)
{
	float curAmbR = -1;
	float curAmbG = -1;
	float curAmbB = -1;
	if(sceneRoom)
	{
		m_DesignLock.lockForRead();
		for(QHash<quint16, quint8>::const_iterator si = m_DoorRenderState.cbegin(); si != m_DoorRenderState.cend(); si++)
		{
			// Find my state hash
			qint8 state = *si;
			quint32 stateKey = StateVAOKey(si.key(), state);
			QHash<quint32, AuScVAO>::iterator vi = vaos.find(stateKey);
			if(vi == vaos.end())
			{
				// If it is not the open state, find the previous good state before the open state.
				if(state == AuScDoor::Open || state == AuScDoor::StuckOpen || state == AuScDoor::LockedOpen)
					continue;
				else if(state == AuScDoor::StuckClosed || state == AuScDoor::LockedClosed)
					state = AuScDoor::Closed;
				for(; state >= AuScDoor::Closed; state--)
				{
					stateKey = StateVAOKey(si.key(), state);
					vi = vaos.find(stateKey);
					if(vi != vaos.end())
						break;
				}
				if(vi == vaos.end())
					continue;
			}

			// Set ambient lighting for the door.
			float ambR, ambG, ambB;
			bool sunsBlocked;
			QHash<quint16, QString>::const_iterator dri = m_DoorRooms.find(si.key());
			if(dri != m_DoorRooms.cend())
				InteriorAmbientLight(*dri, ambR, ambG, ambB, sunsBlocked);	// Get lighting conditions in the door's room.
			else
				InteriorAmbientLight("", ambR, ambG, ambB, sunsBlocked);

			if(ambR != curAmbR || ambG != curAmbG || ambB != curAmbB)
			{
				curAmbR = ambR;
				curAmbG = ambG;
				curAmbB = ambB;
				shader.SetAmbientLight(curAmbR, curAmbG, curAmbB);
			}

			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		m_DesignLock.unlock();

		// Restore ambient lighting for exterior.
		shader.SetAmbientLight(extAmbR, extAmbG, extAmbB);
	}
	else
	{
		// When scene atom is not aboard, use exterior ambient light for all doors.
		shader.SetAmbientLight(extAmbR, extAmbG, extAmbB);

		m_DesignLock.lockForRead();
		for(QHash<quint16, quint8>::const_iterator si = m_DoorRenderState.cbegin(); si != m_DoorRenderState.cend(); si++)
		{
			// Find my state hash
			qint8 state = *si;
			quint32 stateKey = StateVAOKey(si.key(), state);
			QHash<quint32, AuScVAO>::iterator vi = vaos.find(stateKey);
			if(vi == vaos.end())
			{
				// If it is not the open state, find the previous good state before the open state.
				if(state == AuScDoor::Open || state == AuScDoor::StuckOpen || state == AuScDoor::LockedOpen)
					continue;
				for(; state >= AuScDoor::Closed; state--)
				{
					stateKey = StateVAOKey(si.key(), state);
					vi = vaos.find(stateKey);
					if(vi != vaos.end())
						break;
				}
				if(vi == vaos.end())
					continue;
			}
			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		m_DesignLock.unlock();
	}
}

void DaSc::RenderInteriorDoorVAOs(QHash<quint32, AuScVAO>& vaos, AuAtomRenderInfo& ari, const AuGLSLScBase& shader,
		quint16 sceneRoom, float extAmbR, float extAmbG, float extAmbB)
{
	// Do not disable suns when the viewer is outside because hull doors can open into interior rooms and we don't want those hull
	// doors to be black from the outside.
	if(sceneRoom)
	{
		// Disable suns that were enabled.
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			if(sun.m_Enabled)
				shader.SetSunEnabled(sunIndex, false);
		}
	}

	float curAmbR = -1;
	float curAmbG = -1;
	float curAmbB = -1;
	if(sceneRoom)
	{
		m_DesignLock.lockForRead();
		for(QHash<quint16, quint8>::const_iterator si = m_DoorRenderState.cbegin(); si != m_DoorRenderState.cend(); si++)
		{
			// Find my state hash
			qint8 state = *si;
			quint32 stateKey = StateVAOKey(si.key(), state);
			QHash<quint32, AuScVAO>::iterator vi = vaos.find(stateKey);
			if(vi == vaos.end())
			{
				// If it is not the open state, find the previous good state before the open state.
				if(state == AuScDoor::Open || state == AuScDoor::StuckOpen || state == AuScDoor::LockedOpen)
					continue;
				for(; state >= AuScDoor::Closed; state--)
				{
					stateKey = StateVAOKey(si.key(), state);
					vi = vaos.find(stateKey);
					if(vi != vaos.end())
						break;
				}
				if(vi == vaos.end())
					continue;
			}

			// Set ambient lighting for the door.
			float ambR, ambG, ambB;
			bool sunsBlocked;
			QHash<quint16, QString>::const_iterator dri = m_DoorRooms.find(si.key());
			if(dri != m_DoorRooms.cend())
				InteriorAmbientLight(*dri, ambR, ambG, ambB, sunsBlocked);	// Get lighting conditions in the door's room.
			else
				InteriorAmbientLight("", ambR, ambG, ambB, sunsBlocked);

			if(ambR != curAmbR || ambG != curAmbG || ambB != curAmbB)
			{
				curAmbR = ambR;
				curAmbG = ambG;
				curAmbB = ambB;
				shader.SetAmbientLight(curAmbR, curAmbG, curAmbB);
			}

			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		m_DesignLock.unlock();

		// Restore ambient lighting for exterior.
		shader.SetAmbientLight(extAmbR, extAmbG, extAmbB);
	}
	else
	{
		// When scene atom is not aboard, use exterior ambient light for all doors.
		shader.SetAmbientLight(extAmbR, extAmbG, extAmbB);

		m_DesignLock.lockForRead();
		for(QHash<quint16, quint8>::const_iterator si = m_DoorRenderState.cbegin(); si != m_DoorRenderState.cend(); si++)
		{
			// Find my state hash
			qint8 state = *si;
			quint32 stateKey = StateVAOKey(si.key(), state);
			QHash<quint32, AuScVAO>::iterator vi = vaos.find(stateKey);
			if(vi == vaos.end())
			{
				// If it is not the open state, find the previous good state before the open state.
				if(state == AuScDoor::Open || state == AuScDoor::StuckOpen || state == AuScDoor::LockedOpen)
					continue;
				for(; state >= AuScDoor::Closed; state--)
				{
					stateKey = StateVAOKey(si.key(), state);
					vi = vaos.find(stateKey);
					if(vi != vaos.end())
						break;
				}
				if(vi == vaos.end())
					continue;
			}
			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		m_DesignLock.unlock();
	}

	// Restore suns.
	if(sceneRoom)
	{
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			if(sun.m_Enabled)
				shader.SetSunEnabled(sunIndex, true);
		}
	}
}

void DaSc::RenderLandingGearVAOs(QHash<quint32, AuScVAO>& vaos, const AuGLSLScBase& shader)
{
	m_DesignLock.lockForRead();
	for(QHash<quint16, quint8>::const_iterator si = m_LandingGearRenderState.cbegin(); si != m_LandingGearRenderState.cend();
			si++)
	{
		// Find my state hash
		qint8 state = *si;
		quint32 stateKey = StateVAOKey(si.key(), state);
		QHash<quint32, AuScVAO>::iterator vi = vaos.find(stateKey);
		if(vi == vaos.end())
		{
			// If it is not the retracted state, find the previous good state before the retracted state.
			if(state == AuScLandingGear::Retracted || state == AuScLandingGear::StuckRetracted)
				continue;
			for(; state >= AuScLandingGear::Extended; state--)
			{
				stateKey = StateVAOKey(si.key(), state);
				vi = vaos.find(stateKey);
				if(vi != vaos.end())
					break;
			}
			if(vi == vaos.end())
				continue;
		}
		(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
	}
	m_DesignLock.unlock();
}

void DaSc::RenderTurretBaseVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, const AuGLSLScBase& shader)
{
	AuMatrix4x4 baseM;
	m_TurretPosLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		for(AuTurretStateHash::const_iterator si = m_TurretPos.cbegin(); si != m_TurretPos.cend(); si++)
		{
			QHash<quint16, AuScVAO>::iterator vi = vaos.find(si.key());
			if(vi == vaos.end())
				continue;
			scd->TurretBaseMatrix(si.key(), (*si).BaseRotation(), baseM);
			baseM = model * baseM;
			shader.SetModelView(baseM);

			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		shader.SetModelView(model);
	}
	UnlockDesign();
	m_TurretPosLock.unlock();
}

void DaSc::RenderTurretGunVAOs(QHash<quint32, AuScVAO>& vaos, const AuMatrix4x4& model, const AuGLSLScBase& shader)
{
	AuMatrix4x4 gunM;
	m_TurretPosLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		for(AuTurretStateHash::const_iterator si = m_TurretPos.cbegin(); si != m_TurretPos.cend(); si++)
		{
			// Find my state hash
			quint32 stateKey = StateVAOKey(si.key(), (*si).State());
			QHash<quint32, AuScVAO>::iterator vi = vaos.find(stateKey);
			if(vi == vaos.end())
				continue;
			scd->TurretGunMatrix(si.key(), (*si).BaseRotation(), (*si).GunRotation(), gunM);
			gunM = model * gunM;
			shader.SetModelView(gunM);

			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		shader.SetModelView(model);
	}
	UnlockDesign();
	m_TurretPosLock.unlock();
}

void DaSc::RenderLensVAOs(QHash<quint16, AuScVAO>& vaos, const QHash<quint16, AuLensGlow>& glows, const AuGLSLScBase& shader,
		bool lightOn)
{
	if(lightOn)
	{
		// Current SetGlowDiscard values.
		float curGlowR = 0;
		float curGlowG = 0;
		float curGlowB = 0;
		for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
		{
			QHash<quint16, AuLensGlow>::const_iterator gi = glows.find(vi.key());
			if(gi != glows.cend())
			{
				float glowR = (*gi).Color().redF();
				float glowG = (*gi).Color().greenF();
				float glowB = (*gi).Color().blueF();
				if(glowR != curGlowR || glowG != curGlowG || glowB != curGlowB)
				{
					curGlowR = glowR;
					curGlowG = glowG;
					curGlowB = glowB;
					shader.SetGlowDiscard(curGlowR,curGlowG,curGlowB, .01f);
				}
			}
			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
		}
		// Restore glow state.
		if(curGlowR != 0 || curGlowG != 0 || curGlowB != 0)
			shader.SetGlowDiscard(0,0,0, .01f);
	}
	else
	{
		for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
			(*vi).DrawSc(shader, EffectTextures(), m_HullEffectAppl);
	}
}

void DaSc::SetDesign(const AuScReport& r)
{
	const DaDesigner* pardes = FindParentDesignerInEnv();
	if(!pardes)
	{
		const DgData& sdd = g_Au.LockScDesignsForRead();
		m_DesignCacheValid = sdd.ScDesignIsLoaded(r.DesignId());
		g_Au.UnlockScDesigns();
		if(!m_DesignCacheValid)
			qWarning() << "error: DaSc::SetDesign geometry is not ready." << Id() << Name() << "design" << r.DesignId();
	}

	m_DesignLock.lockForWrite();
	m_Design = r;

	// Collect information about things whose states we must manage.
	QStringList rooms;
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		scd->DoorStates(m_DoorStates);
		m_DoorChg.Set();

		AuScPartSet landingGear;
		scd->LandingGear(landingGear);
		for(QSet<quint16>::const_iterator gi = landingGear.cbegin(); gi != landingGear.cend(); gi++)
			m_LandingGearStates.m_States.insert(*gi, AuScLandingGear::Extended);
		m_LandingGearChg.Set();

		scd->TurboLifts(m_Lifts);
		m_LiftChg.Set();

		scd->RoomVoidNames(rooms);

		// Init room light states with defaults from the design.
		m_RoomLightLock.lockForWrite();
		for(QStringList::const_iterator ri = rooms.cbegin(); ri != rooms.cend(); ri++)
		{
			INTERIOR_LIGHT initLight = scd->RoomLight(*ri);
			m_RoomLightStates.insert(*ri, initLight);
		}
		m_RoomLightChg.Set();
		m_RoomLightLock.unlock();
	}
	UnlockDesign();
	m_DesignLock.unlock();

	m_AirLock.lockForWrite();
	for(QStringList::const_iterator ri = rooms.cbegin(); ri != rooms.cend(); ri++)
		m_AirPressures.insert(*ri, 100);	// 100 = 50% which is standard atmosphere pressure
	m_AirChg.Set();
	m_AirLock.unlock();

	m_FTLLock.lockForWrite();
	for(AuScReportSysVolHash::const_iterator fi = r.FTLDrives().cbegin(); fi != r.FTLDrives().cend(); fi++)
		m_FTLDrives.insert(fi.key(), AuFTLDriveState());
	m_FTLChg.Set();
	m_FTLLock.unlock();

	m_LandingSystemLock.lockForWrite();
	for(QSet<quint16>::const_iterator li = r.VehicleLandingSystems().cbegin(); li != r.VehicleLandingSystems().cend(); li++)
		m_LandingSystemStates.insert(*li, 0);
	m_LandingSystemChg.Set();
	m_LandingSystemLock.unlock();

	m_LauncherLock.lockForWrite();
	for(QSet<quint16>::const_iterator vi = r.VehicleLaunchers().cbegin(); vi != r.VehicleLaunchers().cend(); vi++)
		m_LauncherStates.insert(*vi, 0);
	m_LauncherChg.Set();
	m_LauncherLock.unlock();

	m_TRoomLock.lockForWrite();
	for(AuScReportTransporterStationHash::const_iterator ti = r.Transporters().cbegin(); ti != r.Transporters().cend(); ti++)
		m_TRoomStates.insert(ti.key(), TRANSPORTER_Off);
	m_TRoomChg.Set();
	m_TRoomLock.unlock();

	m_TurretPosLock.lockForWrite();
	for(AuScReportTurretHash::const_iterator ti = r.Turrets().cbegin(); ti != r.Turrets().cend(); ti++)
		m_TurretPos.insert(ti.key(), AuTurretState());
	m_TurretPosChg.Set();
	m_TurretPosLock.unlock();
}

quint8 DaSc::DoorState(quint16 door) const
{
	m_DesignLock.lockForRead();
	quint8 doorState = m_DoorStates.DoorState(door);
	m_DesignLock.unlock();
	return doorState;
}

void DaSc::OccupyBerth(AuRosterEntry::TYPE type, const AuId& occId, const QString& occName)
{
	Q_ASSERT(type != AuRosterEntry::Invalid);
	// Insure the occupant isn't already in our roster somewhere.
	RemoveBerthOccupant(occId);

	// Override captain entry with captainslot if there are no real berths.
	if(type == AuRosterEntry::Captain && CaptainsBerthType() == AuRosterEntry::CaptainSlot)
		type = AuRosterEntry::CaptainSlot;

	// The CaptainSlot is used when taking a berth on a vessel that has no
	// berths. It's only a place holder to know who the captain is, for
	// cargo transfer authority, etc. The berth detail id is not needed
	// because the vessel can't be used as a respawn point in this case.
	quint32 berthDetailId = 0;	// This will be the part id of the berth that is assigned.
	if(type != AuRosterEntry::CaptainSlot)
	{
		// The room type must be appropriate for the berth.
		AuScPart::PART_TYPE partType = AuScPart::Part_Invalid;
		switch(type)
		{
		case AuRosterEntry::Invalid:
		case AuRosterEntry::CaptainSlot:
			break;
		case AuRosterEntry::Captain:
			// The Captain is assigned a type of berth based on those available.
			switch(CaptainsBerthType())
			{
			case AuRosterEntry::Invalid:
			case AuRosterEntry::CaptainSlot:
			case AuRosterEntry::OfficerAI:
			case AuRosterEntry::PassengerAI:
				// Invalid return from CaptainsBerthType.
				break;
			case AuRosterEntry::Captain:
				partType = AuScPart::Part_BerthCapt;
				break;
			case AuRosterEntry::Crew:
				partType = AuScPart::Part_BerthCrew;
				break;
			case AuRosterEntry::OfficerHuman:
				partType = AuScPart::Part_BerthOfficer;
				break;
			case AuRosterEntry::PassengerHuman:
				partType = AuScPart::Part_BerthPassenger;
				break;
			case AuRosterEntry::Troop:
				partType = AuScPart::Part_BerthTroop;
				break;
			}
			break;
		case AuRosterEntry::Crew:
			partType = AuScPart::Part_BerthCrew;
			break;
		case AuRosterEntry::OfficerAI:
		case AuRosterEntry::OfficerHuman:
			partType = AuScPart::Part_BerthOfficer;
			break;
		case AuRosterEntry::PassengerAI:
		case AuRosterEntry::PassengerHuman:
			partType = AuScPart::Part_BerthPassenger;
			break;
		case AuRosterEntry::Troop:
			partType = AuScPart::Part_BerthTroop;
			break;
		}

		// Give them a berth in the room they occupy, if possible.
		quint16 occRoomId;
		DaAtom* occAtom = FindNormalSubatomId(occId);
		if(occAtom)
			occRoomId = occAtom->ShipRoomId();
		else
			occRoomId = 0;

		// Locate a physical berth detail in the model to assign. This becomes the person's spawn point in the ship.
		m_DesignLock.lockForRead();
		const AuScDesign* scd = LockDesign();
		if(scd)
		{
			// Look for unoccupied berths of the part type, so we can dole them out in part number order.
			QMap<quint16, bool> berthsOfTheType;
			for(AuScParts::const_iterator pi = scd->Parts().cbegin(); pi != scd->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->Type() != partType)
					continue;

				// Found the right kind of berth. Is it occupied?
				m_RosterLock.lockForRead();
				bool v = m_Roster.BerthOccupants(pi.key()) >= 1;	// Check single occupancy.
				m_RosterLock.unlock();
				if(v)
					continue;	// The berth is full.

				// Found an unoccupied berth of the desired type.
				if(occRoomId)
				{
					// Occupant is aboard; if they are in the same room, take the berth.
					quint16 berthRoomId = scd->RoomVoidContainingLocLive((*pi)->Center(), .1, m_DoorStates, m_LandingGearStates,
							m_Lifts);
					if(berthRoomId == occRoomId)
					{
						berthDetailId = pi.key();
						break;
					}
				}

				// The berth is worth considering.
				berthsOfTheType.insert(pi.key(), true);
			}

			// If we didn't find the perfect berth, take the first one from the map, sorted by part number.
			if(!berthDetailId && !berthsOfTheType.empty())
				berthDetailId = berthsOfTheType.cbegin().key();
		}
		UnlockDesign();
		m_DesignLock.unlock();

		// We can't continue if we didn't find a valid berth detail.
		if(!berthDetailId)
		{
			qWarning() << "DaSc::OccupyBerth ERROR" << Id() << Name() << "failed to find a berth, roster type" << type
				<< "occupant" << occId << occName << AuTime::Now();
			return;
		}
	}

	// Occupy a berth with the occupant.
	m_RosterLock.lockForWrite();
	m_Roster.OccupyBerth(type, berthDetailId, occId, occName);	// berthDetailId 0 is ok for CaptSlot.
	m_tmRosterChange.Set();
	m_RosterLock.unlock();

	// Update first officer id so the ship will know it has an AI commander right away.
	if(type == AuRosterEntry::OfficerAI)
	{
		m_VesselLock.lockForWrite();
		if(m_ConAICaptId.IsNull())
			m_ConAICaptId = occId;
		m_VesselLock.unlock();
	}

	// Start monitoring comm channels so I can communicate.
	MonitorCommChannels();

	SetModified();
}

bool DaSc::IsTransporterRoom(quint16 roomId) const
{
	m_DesignLock.lockForRead();
	bool isTransporter = m_Design.Transporters().AnyInRoom(roomId);
	m_DesignLock.unlock();
	return isTransporter;
}

quint16 DaSc::TransporterWhich(const DaAtom& atom) const
{
	AuPoint shipLoc;
	if(!atom.GetLocationRelativeToParent(shipLoc, this))
		return 0;

	quint16 roomId = atom.ShipRoomId();
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	quint16 v;
	if(scd)
		v = m_Design.ClosestTransporterInRoom(*scd, roomId, shipLoc);
	else
		v = 0;
	UnlockDesign();
	m_DesignLock.unlock();
	return v;
}

QString DaSc::TurboLiftStop(quint16 liftId) const
{
	QString curStop;
	m_DesignLock.lockForRead();
	AuLiftHash::const_iterator li = m_Lifts.find(liftId);
	if(li != m_Lifts.cend())
		curStop = *li;
	m_DesignLock.unlock();
	return curStop;
}

QString DaSc::TurboLiftStop(quint16 liftId, QStringList& liftStops) const
{
	QString curStop;
	m_DesignLock.lockForRead();
	AuLiftHash::const_iterator li = m_Lifts.find(liftId);
	if(li != m_Lifts.cend())
	{
		curStop = *li;

		// Get the names of all of the lift stops.
		AuScReportLiftHash::const_iterator lsi = m_Design.TurboLifts().find(liftId);
		if(lsi != m_Design.TurboLifts().cend())
			liftStops = (*lsi).keys();
	}
	m_DesignLock.unlock();
	return curStop;
}

int DaSc::CallTurboLift(quint16 liftId, const QString& liftStop)
{
	DaAtomList lockedAtoms;
	LockLiftContents(liftId, lockedAtoms);

	m_DesignLock.lockForWrite();
	if(AnyTurboLiftDoorsBlockedOpen(liftId))
	{
		m_LiftCalls.Call(liftId, liftStop);	// Move the lift when the door is no longer blocked.
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		return 7;	// A door is blocked open.
	}

	// Determine the id of the door at the lift stop.
	quint16 doorId = m_Design.TurboLiftStopDoor(liftId, liftStop);
	if(!doorId)
	{
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		return 1;	// No lift door at that stop.
	}

	AuLiftHash::iterator li = m_Lifts.find(liftId);
	AuScReportLiftHash::const_iterator liftStops = m_Design.TurboLifts().find(liftId);
	if(li == m_Lifts.end() || liftStops == m_Design.TurboLifts().cend())
	{
		// Wut?
		m_DoorChg.Set();		// In case any of the doors closed.
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		SetModified();
		return 4;
	}

	// Insure the turbo lift is at the desired stop.
	QString oldStop(*li);
	QString nuStop = (*liftStops).DoorStopName(doorId);
	if(nuStop != liftStop)
		qWarning() << "error: DaSc::CallTurboLift lift" << liftId << "stop" << liftStop << "does not match nuStop" << nuStop;

	if(oldStop == nuStop)
	{
		// The lift is at the stop, in position for the door to open.
		bool doorOpened = m_DoorStates.OpenDoor(doorId);	// Open the lift door.
		if(doorOpened)
		{
			m_DoorChg.Set();
			SetModified();
		}
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		return doorOpened ? 6 : 0;
	}

	// Lift must be moved to the stop.
	// Turbo lift cannot move for ten seconds after moving to a nu stop, to allow slow thinking AI to get off.
	if(m_LiftChg.ElapsedSec() < 10)
	{
		m_LiftCalls.Call(liftId, liftStop);	// Move the lift when enough time has passed.
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		return 2;	// Insufficent time passed since last operation.
	}

	// Turbo lift door must insure all doors associated with the same lift are closed.
	if(!m_DoorStates.CloseAssociatedDoors(liftId))
	{
		// A turbo lift door is stuck open.
		m_LiftCalls.Call(liftId, liftStop);	// Move the lift when the door is no longer stuck.
		m_DoorChg.Set();		// In case any of the doors closed.
		m_DesignLock.unlock();
		UnlockLiftContents(lockedAtoms);
		SetModified();
		return 3;
	}

	MoveLift(liftId, lockedAtoms, nuStop, doorId);	// MoveLift calls m_DesignLock.unlock()
	UnlockLiftContents(lockedAtoms);
	return 5;
}

QString DaSc::LocationDesc(const DaAtom& atom) const
{
	if(!atom.ShipRoomId())
		return QString();
	QString desc;
	const AuScDesign* scd = LockDesign();
	if(scd)
		desc = scd->RoomVoidName(atom.ShipRoomId());
	else
		desc = tr("Room %1").arg(atom.ShipRoomId());
	UnlockDesign();
	return desc;
}

QString DaSc::BerthRoomName(const AuRosterEntry& rosterEntry) const
{
	if(rosterEntry.m_Type == AuRosterEntry::Invalid)
	{
		qWarning() << "error: DaSc::BerthRoomName berth type is 'Invalid'";
		return QString();
	}
	else if(rosterEntry.m_Type == AuRosterEntry::CaptainSlot)
		return QObject::tr("Captain, No Bunk");					// CaptainSlot does not have a real berth.

	AuPoint berthLoc;
	quint16 roomId = 0;
	QString desc;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		berthLoc = scd->BerthLocation(rosterEntry.m_BerthId);
		roomId = scd->RoomVoidContainingLocLive(berthLoc, .1, m_DoorStates, m_LandingGearStates, m_Lifts);
		desc = scd->RoomVoidName(roomId);
	}
	UnlockDesign();
	m_DesignLock.unlock();
	return desc;
}

// Design lock already locked for read.
bool DaSc::AnyTurboLiftDoorsBlockedOpen(quint16 liftId) const
{
	QList<quint16> doorIds;	// Doors for the lift.
	for(AuDoorStateHash::const_iterator si = m_DoorStates.cbegin(); si != m_DoorStates.cend(); si++)
		if((*si).PartId() == liftId && (*si).IsOpen())
			doorIds.append(si.key());
	if(doorIds.empty())
		return false;

	// Check for anything in the door void space that would block the door from closing.
	m_DesignLock.unlock();
	bool doorBlocked = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaAtom* atom = *it;
		if(!atom->KeepsAutomaticDoorsOpen())
			continue;
		quint16 roomId = atom->ShipRoomId();
		if(!roomId)
			continue;
		// Check all doors for the lift, to see if the room void is associated with any of them.
		m_DesignLock.lockForRead();
		const AuScDesign* scd = LockDesign();
		if(scd)
		{
			for(QList<quint16>::const_iterator di = doorIds.cbegin(); di != doorIds.cend(); di++)
				if(scd->IsOpenDoorVoid(roomId, *di))
				{
					doorBlocked = true;
					break;
				}
		}
		UnlockDesign();
		m_DesignLock.unlock();
		if(doorBlocked)
			break;
	}
	m_DesignLock.lockForWrite();
	return doorBlocked;
}

bool DaSc::HasSpaceVehicleLauncher() const
{
	m_LauncherLock.lockForRead();
	bool v = !m_LauncherStates.empty();
	m_LauncherLock.unlock();
	return v;
}

void DaSc::RequestLaunch(DaCreature* pilot, DaVehicle* vehicle)
{
	if(!HasSpaceVehicleLauncher())
	{
		QString text(tr("%1, this ship is not equipped with a launcher.").arg(pilot->Name()));
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
		g_Au.SendAtomChat(msg);
		return;
	}

	switch(vehicle->CommodityId())
	{
	case CID_DirtBike:
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
	case CID_Helicopter:
	case CID_SailBoat:
	case CID_Submarine:
	default:
		{
			QString text(tr("%2, our launch system cannot launch a %1.").arg(vehicle->Name()).arg(pilot->Name()));
			AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
			g_Au.SendAtomChat(msg);
		}
		return;
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
	case CID_SpaceRocket:
	case CID_SpaceTransport:
		break;
	}

	if(vehicle->IsMoving())
	{
		QString text(tr("%1, unable while your vehicle is moving.").arg(pilot->Name()));
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
		g_Au.SendAtomChat(msg);
		return;
	}
	else if(!vehicle->IsEngineOn())
	{
		QString text(tr("%1, call back when your engine runup is complete.").arg(pilot->Name()));
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
		g_Au.SendAtomChat(msg);
		return;
	}

	m_DesignLock.lockForRead();
	AuScPartSet vl = m_Design.VehicleLaunchers();
	m_DesignLock.unlock();
	QSet<quint16> broken;
	for(QSet<quint16>::const_iterator vli = vl.cbegin(); vli != vl.cend(); vli++)
	{
		int working = GetPowerConsumption(SYS_VehicleLaunch, *vli);
		if(!working)
			broken.insert(*vli);
	}

	quint16 launcherId = 0;
	m_LauncherLock.lockForWrite();
	int lcount = m_LauncherStates.size();
	for(AuLauncherStateHash::iterator li = m_LauncherStates.begin(); li != m_LauncherStates.end(); li++)
	{
		// Skip launchers that are broken.
		if(broken.contains(li.key()))
			continue;

		// Skip all launchers that are not in the dormant state 0.
		quint8 state = *li;
		if(state)
			continue;
		*li = 1;
		launcherId = li.key();
		m_LauncherChg.Set();
		break;
	}
	m_LauncherLock.unlock();

	if(!launcherId)
	{
		QString text;
		if(lcount == broken.size())
			text = tr("%1, all launchers are off line.").arg(pilot->Name());
		else if(!broken.empty())
			text = tr("%1, all launchers are busy or off line.").arg(pilot->Name());
		else
			text = tr("%1, all launchers are busy.").arg(pilot->Name());
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
		g_Au.SendAtomChat(msg);
		return;
	}

	// Open the launch tube doors, if any.
	OpenLauncherDoors(launcherId);
	SetModified();

	QString text(tr("%2, cleared for takeoff, launcher %1.").arg(launcherId).arg(pilot->Name()));
	AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
			Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
	g_Au.SendAtomChat(msg);

	// Calculate launch position for the vehicle.
	AuMatrix4x4 launchPadM;
	AuPoint launchExit;
	const AuScDesign* scd = LockDesign();
	if(scd)
		scd->LaunchPosition(launcherId, launchPadM, launchExit);
	UnlockDesign();

	// Launch the vehicle.
	vehicle->Launch(Id(), launcherId, launchPadM, launchExit);
}

bool DaSc::IsAnyLauncherActive() const
{
	bool anyActive = false;
	m_LauncherLock.lockForRead();
	for(AuLauncherStateHash::const_iterator li = m_LauncherStates.cbegin(); li != m_LauncherStates.cend(); li++)
		if(*li)
		{
			anyActive = true;
			break;
		}
	m_LauncherLock.unlock();
	return anyActive;
}

void DaSc::ResetActiveLaunchers()
{
	if(!IsAnyLauncherActive())
		return;
	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;

	// Make a list of launchers that are in use by vehicles in my area.
	QSet<quint16> launchersInUse;
	DaAtomList vehicles;
	env->FindVehiclesRecursive(vehicles);
	for(DaAtomList::const_iterator it = vehicles.begin(); it != vehicles.end(); it++)
	{
		DaVehicle* vehicle = (DaVehicle*)*it;
		if(!vehicle->IsNormal())
			continue;
		if(!vehicle->IsLaunching())
			continue;
		if(!Is(vehicle->LaunchingVessel()))
			continue;
		quint16 launcherId = vehicle->LauncherId();
		if(!launcherId)
			continue;
		launchersInUse.insert(launcherId);
	}

	// Deactive any active launchers that are not in use.
	QList<quint16> launchersReset;
	m_LauncherLock.lockForWrite();
	for(AuLauncherStateHash::iterator li = m_LauncherStates.begin(); li != m_LauncherStates.end(); li++)
	{
		if(!(*li))
			continue;	// Skip inactive launcher.
		if(launchersInUse.contains(li.key()))
			continue;	// Skip active launcher that is in use.
		*li = 0;
		launchersReset.append(li.key());
	}
	if(!launchersReset.empty())
		m_LauncherChg.Set();
	m_LauncherLock.unlock();
	if(!launchersReset.empty())
	{
		// Close doors associated with the launchers that were reset.
		for(QList<quint16>::const_iterator ri = launchersReset.cbegin(); ri != launchersReset.cend(); ri++)
			CloseLauncherDoors(*ri);
		SetModified();
	}
}

bool DaSc::HasSpaceVehicleRecoverySystem() const
{
	m_LandingSystemLock.lockForRead();
	bool v = !m_LandingSystemStates.empty();
	m_LandingSystemLock.unlock();
	return v;
}

void DaSc::RequestRecovery(DaCreature* pilot, DaVehicle* vehicle, const AuPoint& vehLoc)
{
	Q_UNUSED(vehLoc);
	if(!HasSpaceVehicleRecoverySystem())
	{
		QString text(tr("%1, this ship is not equipped with a recovery system.").arg(pilot->Name()));
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
		g_Au.SendAtomChat(msg);
		return;
	}

	quint16 parkingSpots;
	switch(vehicle->CommodityId())
	{
	case CID_DirtBike:
	case CID_SUV:
	case CID_APC:
	case CID_FarmMachinery:
	case CID_Helicopter:
	case CID_SailBoat:
	case CID_Submarine:
	default:
		return;	// wut?
	case CID_AdvSpaceFighter:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
		m_DesignLock.lockForRead();
		parkingSpots = m_Design.ParkingSpotsSpace();
		m_DesignLock.unlock();
		break;
	case CID_SpaceRocket:
		m_DesignLock.lockForRead();
		parkingSpots = m_Design.ParkingSpotsRocket();
		m_DesignLock.unlock();
		break;
	}
	if(!parkingSpots)
	{
		QString text(tr("%2, this ship has no parking spots for a %1.").arg(vehicle->Name()).arg(pilot->Name()));
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
		g_Au.SendAtomChat(msg);
		return;
	}

	m_DesignLock.lockForRead();
	AuScPartSet vl = m_Design.VehicleLandingSystems();
	m_DesignLock.unlock();
	QSet<quint16> broken;
	for(QSet<quint16>::const_iterator vli = vl.cbegin(); vli != vl.cend(); vli++)
	{
		int working = GetPowerConsumption(SYS_VehicleLanding, *vli);
		if(!working)
			broken.insert(*vli);
	}

	// Locate an inactive landing system and activate it.
	quint16 landerId = 0;
	m_LandingSystemLock.lockForWrite();
	int lcount = m_LandingSystemStates.size();
	for(AuLandingSystemStateHash::iterator li = m_LandingSystemStates.begin(); li != m_LandingSystemStates.end(); li++)
	{
		// Skip landers that are broken.
		if(broken.contains(li.key()))
			continue;

		// Skip all landers that are not in the dormant state 0.
		quint8 state = *li;
		if(state)
			continue;
		*li = 4;				// State is the number of Tock cycles to remain active, nominally 10sec per tock.
		landerId = li.key();
		m_LandingSystemChg.Set();
		break;
	}
	m_LandingSystemLock.unlock();
	if(!landerId)
	{
		QString text;
		if(lcount == broken.size())
			text = tr("%1, all recovery systems are off line.").arg(pilot->Name());
		else if(!broken.empty())
			text = tr("%1, all recovery systems are busy or off line.").arg(pilot->Name());
		else
			text = tr("%1, all recovery systems are busy.").arg(pilot->Name());
		AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
		g_Au.SendAtomChat(msg);
		return;
	}
	SetModified();

	QString text(tr("%2, cleared for landing, recovery system %1.").arg(landerId).arg(pilot->Name()));
	AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
			Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
	g_Au.SendAtomChat(msg);
}

void DaSc::OperateLandingSystems()
{
	// Deactive active landing systems after a few seconds.
	AuMatrix4x4 landerPosM;
	QHash<quint16, AuMatrix4x4> landerPos;		// Position information for active landing systems.
	m_LandingSystemLock.lockForWrite();
	for(AuLandingSystemStateHash::iterator li = m_LandingSystemStates.begin(); li != m_LandingSystemStates.end(); li++)
	{
		if(!(*li))
			continue;	// Skip inactive landing systems.
		const AuScDesign* scd = LockDesign();
		if(scd)
			scd->LandingSystemPosition(li.key(), landerPosM);
		UnlockDesign();
		landerPos.insert(li.key(), landerPosM);
		*li = *li - 1;
		m_LandingSystemChg.Set();
	}
	m_LandingSystemLock.unlock();
	if(!landerPos.empty())
	{
		SetModified();

		// Check active landers to see if they can grab a space vehicle.
		DaAtom* env = FindEnvironmentAtom();
		if(!env)
			return;
		DaAtomList vehicles;
		env->FindVehiclesRecursive(vehicles);
		bool vehicleRecovered = false;
		for(DaAtomList::const_iterator it = vehicles.cbegin(); it != vehicles.cend() && !vehicleRecovered; it++)
		{
			DaVehicle* vehicle = (DaVehicle*)*it;
			if(!vehicle->IsNormal())
				continue;
			if(vehicle->IsInSpacecraft())
				continue;
			DaCreature* pilot = vehicle->Driver();
			if(!pilot)
				continue;	// Ignore pilotless vehicles.
			if(!CheckAccess(pilot))
				continue;	// Do not recover unauthorized vehicles.
			AuPoint vloc;
			if(!GetRelativeLocation(*vehicle, vloc))
				continue;	// wut?
			for(QHash<quint16, AuMatrix4x4>::const_iterator lmi = landerPos.cbegin(); lmi != landerPos.cend(); lmi++)
			{
				AuPoint emitPos = (*lmi).Translation();
				AuPoint endPos = (*lmi).Project(0, 500*FeetPerMeter(), 0);
				if(vloc.DistToSegment(emitPos, endPos) > 10*FeetPerMeter())
					continue;
				// Vehicle is close enough to an active lander segment.
				// Find an empty parking spot for it.
				QList<AuMatrix4x4> rocketSpots;
				QList<AuMatrix4x4> svSpots;
				const AuScDesign* scd = LockDesign();
				if(scd)
				{
					if(vehicle->IsSpaceRocket())
						scd->RocketParkingSpots(rocketSpots);	// Rockets can also be parked in rocket spots.
					scd->SpaceVehicleParkingSpots(svSpots);
				}
				UnlockDesign();
				if(svSpots.empty() && rocketSpots.empty())
					continue;	// No parking spots at all.

				// Use the list of vehicles to find any that are occupying spots in this ship.
				// Remember a new and used vehicle that can be replaced. We prefer to replace new ones.
				DaVehicle* repNew = 0;	// A new vehicle that can be replaced.
				AuMatrix4x4 spotNew;
				VEHICLE_SPOT_TYPE typeNew = SPOT_SpaceVehicle;
				DaVehicle* repUsed = 0;	// An old vehicle that can be replaced.
				AuMatrix4x4 spotUsed;
				VEHICLE_SPOT_TYPE typeUsed = SPOT_SpaceVehicle;
				for(DaAtomList::const_iterator vi = vehicles.cbegin(); vi != vehicles.cend(); vi++)
				{
					DaVehicle* myveh = (DaVehicle*)*vi;
					if(!myveh->IsNormal())
						continue;
					if(!myveh->IsInSpacecraft())
						continue;
					if(!Is(myveh->ParentId()))
						continue;
					// Found a myveh aboard this ship. If it is occupying a parking spot, remove the spot from the list.
					for(QList<AuMatrix4x4>::iterator pi = svSpots.begin(); pi != svSpots.end(); pi++)
					{
						AuPoint parkLoc = (*pi).Translation();
						double dist2 = parkLoc.Distance2(myveh->Location());
						if(dist2 > 5*FeetPerMeter()*5*FeetPerMeter())
							continue;	// Vehicle is not in that spot.
						// myveh occupies a space vehicle parking spot
						if(myveh->IsNew())
						{
							if(!repNew)
							{
								// Found a new vehicle that can be replaced.
								repNew = myveh;
								spotNew = *pi;
								typeNew = SPOT_SpaceVehicle;
							}
						}
						else if(!myveh->HasSubatoms())
						{
							if(!repUsed)
							{
								// Found an unoccupied used vehicle that can be replaced.
								repUsed = myveh;
								spotUsed = *pi;
								typeUsed = SPOT_SpaceVehicle;
							}
						}

						// Remove the parking spot.
						svSpots.erase(pi);
						break;
					}
					for(QList<AuMatrix4x4>::iterator pi = rocketSpots.begin(); pi != rocketSpots.end(); pi++)
					{
						AuPoint parkLoc = (*pi).Translation();
						double dist2 = parkLoc.Distance2(myveh->Location());
						if(dist2 > 5*FeetPerMeter()*5*FeetPerMeter())
							continue;	// Vehicle is not in that spot.
						// myveh occupies a space rocket parking spot
						if(myveh->IsNew())
						{
							if(!repNew)
							{
								// Found a new vehicle that can be replaced.
								repNew = myveh;
								spotNew = *pi;
								typeNew = SPOT_Rocket;
							}
						}
						else if(!myveh->HasSubatoms())
						{
							if(!repUsed)
							{
								// Found an unoccupied used vehicle that can be replaced.
								repUsed = myveh;
								spotUsed = *pi;
								typeUsed = SPOT_Rocket;
							}
						}

						// Remove the parking spot.
						rocketSpots.erase(pi);
						break;
					}
				}
				if(svSpots.empty() && rocketSpots.empty() && !repNew && !repUsed)
				{
					QString text(tr("%1, go around. We have nowhere to park a %2.").arg(pilot->Name()).arg(vehicle->Name()));
					AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
					continue;	// No unoccupied parking spots.
				}

				// Shut off the recovery system.
				m_LandingSystemLock.lockForWrite();
				AuLandingSystemStateHash::iterator li = m_LandingSystemStates.find(lmi.key());
				if(li != m_LandingSystemStates.end())
				{
					*li = 0;
					m_LandingSystemChg.Set();
				}
				m_LandingSystemLock.unlock();
				SetModified();

				// Vehicle can be moved into an appropriate parking spot aboard.
				if(repNew)
				{
					if(!repNew->HasSubatoms())	// Last moment check for subatoms before killing the vehicle. Someone could get in.
						repNew->Kill();
					vehicle->Park(this, spotNew, typeNew);
				}
				else if(repUsed)
				{
					if(!repUsed->HasSubatoms())	// Last moment check for subatoms before killing the vehicle. Someone could get in.
						repUsed->Kill();
					vehicle->Park(this, spotUsed, typeUsed);
				}
				else if(!rocketSpots.empty())
					vehicle->Park(this, rocketSpots.first(), SPOT_Rocket);		// Rockets prefer rocket parking spots.
				else
					vehicle->Park(this, svSpots.first(), SPOT_SpaceVehicle);

				QString text;
				if(!Is(vehicle->MotherShip()))
					text = tr("Nice landing %1. Welcome aboard.").arg(pilot->Name());
				else
					text = tr("Nice landing %1. Welcome back.").arg(pilot->Name());
				vehicle->SetModified();
				AuCommChat msg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg);
				vehicleRecovered = true;
				break;
			}
		}
	}
}

void DaSc::InitPreview(quint8 previewType, const QString& studioName, const AuScDesign& scd, const DaPerson& person)
{
	AuScReport r;
	scd.ReportDesign(r);
	quint8 shipQ = r.Quality();

	QString designName(scd.Name());
	if(designName.isEmpty())
		designName = tr("Unnamed Design");

	SetDesign(r);
	InitSystems(shipQ);
	SetName(designName);
	SetEmpireId(person.EmpireId());
	SetWhereMade(studioName);
	RechargeCapacitor();
	TurnPowerPlantOn();
	SetGravityDriveState(GD_On);								// Clears 'nu' state.
	Refuel(shipQ);
	ShieldCharge(10000000);
	Unmoor();													// Cast off moorings. Clear 'nu' state.
	HeadlightToggle(true);
	NavLightToggle(true);
	RotatingBeaconToggle(true);
	StrobeToggle(true);

	DaDesigner* de = FindParentDesignerInEnv();
	if(!de)
		return;
	DaStar* star = de->FindStar();
	if(!star)
		return;

	AuId parentId;
	bool pitch = false;
	bool roll = false;
	AuPoint loc, upVec;
	switch(previewType)
	{
	case 0:
	case 1:
		return;
	case 2:	// star
		{
			parentId = star->Id();
			double orad = star->OrbitRadius(star->HabitableZone()) * FeetPerAu();
			loc = AuPoint(orad,0,0);
			upVec = AuPoint(0,0,1);
		}
		break;
	case 3:	// world
	case 9:	// sea bottom
		{
			DaWorldLL* world = star->GetHabitableWorld(HABITABLE_HomeWorld);
			if(!world)
				return;
			parentId = world->Id();
			const float orbitAlt = world->AtmosphereRadius() + ParkAltAboveAtm();
			loc = world->FindOrbitSpawnLoc(AuPoint(0,0), Id(), RoughRadius(), orbitAlt, true);
			AuGlobeLLF spawnLL;
			world->LLOfLoc(spawnLL, loc);
			upVec = -world->GravityDir(loc);
			roll = true;
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
			parentId = moon->Id();
			const float orbitAlt = moon->AtmosphereRadius() + ParkAltAboveAtm();
			loc = moon->FindOrbitSpawnLoc(AuPoint(0,0), Id(), RoughRadius(), orbitAlt, true);
			AuGlobeLLF spawnLL;
			moon->LLOfLoc(spawnLL, loc);
			upVec = -moon->GravityDir(loc);
			roll = true;
		}
		break;
	case 5:	// inferno
		{
			DaWorldLL* world = star->GetWorld(OZONE_Inferno);
			if(!world)
				return;
			parentId = world->Id();
			const float orbitAlt = world->AtmosphereRadius() + ParkAltAboveAtm();
			loc = world->FindOrbitSpawnLoc(AuPoint(0,0), Id(), RoughRadius(), orbitAlt, true);
			AuGlobeLLF spawnLL;
			world->LLOfLoc(spawnLL, loc);
			upVec = -world->GravityDir(loc);
			roll = true;
		}
		break;
	case 6:	// frigid
		{
			DaWorldLL* world = star->GetWorld(OZONE_Frigid);
			if(!world)
				return;
			parentId = world->Id();
			const float orbitAlt = world->AtmosphereRadius() + ParkAltAboveAtm();
			loc = world->FindOrbitSpawnLoc(AuPoint(0,0), Id(), RoughRadius(), orbitAlt, true);
			AuGlobeLLF spawnLL;
			world->LLOfLoc(spawnLL, loc);
			upVec = -world->GravityDir(loc);
			roll = true;
		}
		break;
	case 7:	// gas giant
		{
			DaWorldLL* world = star->GetGasGiant();
			if(!world)
				return;
			parentId = world->Id();
			const float orbitAlt = world->AtmosphereRadius() + ParkAltAboveAtm();
			loc = world->FindOrbitSpawnLoc(AuPoint(0,0), Id(), RoughRadius(), orbitAlt, true);
			AuGlobeLLF spawnLL;
			world->LLOfLoc(spawnLL, loc);
			upVec = -world->GravityDir(loc);
			roll = true;
		}
		break;
	case 8:	// ringworld
		{
			DaWorldLL* world = star->GetHabitableWorld(HABITABLE_Ringworld);
			if(!world)
				return;
			parentId = world->Id();
			const float orbitAlt = world->AtmosphereRadius() + ParkAltAboveAtm();
			loc = world->FindOrbitSpawnLoc(AuPoint(0,0), Id(), RoughRadius(), orbitAlt, true);
			AuGlobeLLF spawnLL;
			world->LLOfLoc(spawnLL, loc);
			upVec = -world->GravityDir(loc);
			roll = true;
			pitch = true;
		}
		break;
	}

	AuMatrix4x4 tBody;
	tBody.Translate(loc);
	tBody *= AuPoint(0,0,1).GetAlignmentTransform(upVec);
	if(roll)
		tBody.RotateY(-PI/2);
	if(pitch)
		tBody.RotateX(PI/2);
	ForcePosStop(AuAtomPos(parentId, tBody));
	InvalidateEnvironment();
}

bool DaSc::BerthPosition(const DaCreature& occupant, AuMatrix4x4& berthM) const
{
	quint32 berthId = BerthDetailIdOccupied(occupant.Id());
	if(!berthId)
		return false;

	// Locate the corresponding part in the design.
	const AuScDesign* scd = LockDesign();
	AuScParts::const_iterator pi = scd->Parts().find(berthId);
	if(pi == scd->Parts().cend())
	{
		UnlockDesign();
		return false;
	}
	// Get the gravity normal from the room containing the berth.
	// Current operator location and gravity normal are not needed for berth positioning; it is totally based on part orientation.
	AuStation station = (*pi)->OperatorStation(AuPoint(0,0,1));
	UnlockDesign();
	berthM = station.OpM();
	berthM.Translate(occupant.ConsolePoseOffset(station.Type(), station.OccPos(), occupant.CGHeightStanding(),
				occupant.TorsoThickness(), station.SeatIndex()));
	return true;
}

bool DaSc::GameTableLocation(quint16 stationId, AuMatrix4x4& tableM) const
{
	// Locate the corresponding part in the design.
	const AuScDesign* scd = LockDesign();
	AuScParts::const_iterator pi = scd->Parts().find(stationId);
	if(pi == scd->Parts().cend())
	{
		UnlockDesign();
		return false;
	}
	tableM = (*pi)->PositionMatrix();
	UnlockDesign();
	return true;
}

void DaSc::WakeDoctor(const DaCreature& patient)
{
	DaSentient* doctor = FindDoctor(patient);
	if(doctor && doctor->IsAIControlled())
		doctor->WakeUp();
}

DaCreature* DaSc::FindPatient(const DaSentient& doctor) const
{
	quint16 stationId = doctor.StationId();
	if(!stationId)
		return 0;
	if(doctor.ConsoleSeatIndex() != 0)
		return 0;

	// Location of patient station is needed to find bodies dropped into it.
	AuStation patientStation;
	quint16 patientStationId;
	PatientStation(stationId, patientStationId, patientStation);
	if(!patientStationId)
		return 0;

	DaCreature* dyingPatient = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCreature* cre = dynamic_cast<DaCreature*>(*it);
		if(!cre)
			continue;
		if(!cre->IsInSpacecraft())
			continue;
		if(cre->StationId() == stationId && cre->ConsoleSeatIndex() == 1)
		{
			// Found a patient occupying the station.
			return cre;
		}
		else if(cre->IsBodyDyingOrDead())
		{
			// Dead and dying patients are identified by proximity to the patient station.
			// They are only returned if we don't find a patient occupying the station.
			AuPoint stapos = patientStation.OpM().UnProject(cre->Location());	// Location of creature relative to the station.
			if(stapos.Length2_2D() < FeetPerMeter()*FeetPerMeter() && fabs(stapos.z) < FeetPerMeter()*2)
				dyingPatient = cre;
		}
	}
	return dyingPatient;
}

bool DaSc::StationScreen(quint16 stationId, AuQuad& screen) const
{
	if(!IsDesignDataCacheValid())
		return false;

	bool hasScreen = false;
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator pi = scd->Parts().find(stationId);
		if(pi != scd->Parts().cend())
			hasScreen = (*pi)->ScreenQuad(screen);
	}
	UnlockDesign();
	return hasScreen;
}

void DaSc::TurretAimMatrix(quint16 partId, float baseRot, AuMatrix4x4& aimM) const
{
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator pi = scd->Parts().find(partId);
		if(pi != scd->Parts().cend())
			(*pi)->TurretAimMatrix(baseRot, aimM);
	}
	UnlockDesign();
}

void DaSc::TurretMuzzleMatrix(quint16 partId, float baseRot, float gunRot, AuMatrix4x4& muzzleM) const
{
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator pi = scd->Parts().find(partId);
		if(pi != scd->Parts().cend())
			(*pi)->TurretMuzzleMatrix(baseRot, gunRot, muzzleM);
	}
	UnlockDesign();
}

void DaSc::TurretSightMatrix(quint16 partId, float baseRot, float gunRot, AuMatrix4x4& muzzleM) const
{
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator pi = scd->Parts().find(partId);
		if(pi != scd->Parts().cend())
			(*pi)->TurretSightMatrix(baseRot, gunRot, muzzleM);
	}
	UnlockDesign();
}

void DaSc::PositionTurret(const DaCreature& gunner)
{
	float baseRot = gunner.Rotation().z;
	float gunRot = gunner.CurrentHeadTilt();
	m_TurretPosLock.lockForWrite();
	AuTurretStateHash::iterator ti = m_TurretPos.find(gunner.StationId());
	if(ti != m_TurretPos.cend())
	{
		if((*ti).BaseRotation() != baseRot || (*ti).GunRotation() != gunRot)
		{
			(*ti).SetBaseRotation(baseRot);
			(*ti).SetGunRotation(gunRot);
			m_TurretPosChg.Set();
			SetModified();
		}
	}
	m_TurretPosLock.unlock();
}

void DaSc::FindGunners(DaAtomList& gunners) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaCreature* creature = dynamic_cast<const DaCreature*>(*it);
		if(!creature)
			continue;
		if(!creature->IsTurretPose())
			continue;
		gunners.append((DaAtom*)creature);
	}
}

void DaSc::OperateLifts()
{
	m_DesignLock.lockForRead();
	bool hasCalls = !m_LiftCalls.empty();
	m_DesignLock.unlock();
	if(!hasCalls)
		return;

	m_DesignLock.lockForWrite();
	AuLiftCalls liftCalls = m_LiftCalls.PendingCalls();
	m_DesignLock.unlock();

	for(AuLiftCalls::const_iterator ci = liftCalls.cbegin(); ci != liftCalls.cend(); ci++)
		CallTurboLift((*ci).LiftId(), (*ci).Stop());
}

AuPoint DaSc::TurretHemiVector(const AuPose& pose, quint16 which) const
{
	Q_UNUSED(pose);
	AuPoint hemiVec;
	const AuScDesign* scd = LockDesign();
	if(scd)
		hemiVec = scd->TurretBaseAxis(which);
	UnlockDesign();
	return hemiVec;
}

void DaSc::AnimateAlertStatus(const DaPerson* sceneAtom, const AuElapsedTimer& last)
{
	if(!m_AlertStatus)
		return;

	// Do not animate the texture if the scene atom is not aboard this ship.
	if(!sceneAtom->IsInsideSpacecraft() || sceneAtom->FindParentVesselInEnv() != this)
		return;

	// Alert status only needs to be updated once per second.
	if(last.ElapsedSecTicks() == 0)
		return;

	GLuint texId = 0;
	switch(ApparentAlertCondition())
	{
	case ConditionGreen:
		texId = m_AlertGreen;
		break;
	case ConditionYellow:
		if(last.SecIsOdd())
			texId = m_AlertYellowDim;
		else
			texId = m_AlertYellowBright;
		break;
	case ConditionRed:
		if(last.SecIsOdd())
			texId = m_AlertRedDim;
		else
			texId = m_AlertRedBright;
		break;
	}

	// Update alert status textures.
	m_DesignLock.lockForRead();
	for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
			di != m_Design.StatusDisplays().cend(); di++)
	{
		if((*di).PartType() == AuScPart::Part_StatusAlert)
		{
			QHash<quint16, GLuint>::iterator oi = m_OwnTextures.find(di.key());
			if(oi != m_OwnTextures.end())
				*oi = texId;
		}
	}
	m_DesignLock.unlock();
}

void DaSc::AnimateMissionStatus(const DaPerson* sceneAtom, AuGLEffects& re, const AuElapsedTimer& last)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Do not animate the texture if the scene atom is not aboard this ship.
	if(!sceneAtom->IsInsideSpacecraft() || sceneAtom->FindParentVesselInEnv() != this)
		return;

	// Animate one of my dummy consoles so the lower pane will be correct.
	// It is shared by all consoles so I only need to animate one of them.
	if(!sceneAtom->IsConsolePose())
		m_PilotEmpty.AnimateDeviceObjects(re, *this, last, AuPoint(0,0), *sceneAtom);

	// Mission status only needs to be updated once per second.
	if(last.ElapsedSecTicks() == 0)
		return;

	// The mission status console is the same throughout the ship.
	// Only ship is required. Planet and solar system are optional.
	DaAtom* mysys = FindEnvironmentAtom();
	DaWorld* planet;
	if(dynamic_cast<const DaSolarSystem*>(mysys) || dynamic_cast<const DaDesignBase*>(mysys))
		planet = (DaWorld *)FindParentPlanetInEnv();
	else
		planet = 0;

	QString name;
	name = QString("MStat%1").arg(Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	QRect rc(0,0,122,16);
	QPainter p;

	// System Status
	QPixmap pane(122,16);		// Most sub-panes are 122x16 pixels.
	if(RepairSecondsRemaining())
	{
		// Ship is getting repaired.
		if(last.SecIsOdd())
			pane.load(":data/conRepairs2.png");
		else
			pane.load(":data/conRepairs3.png");
		QRect rc2(0,0,122,16);
		p.begin(&pane);
		p.setFont(QFont(g_Au.TypefaceSans(),8));
		p.setPen(QPen(QColor(0,0,0), 0.f));
		QString statText = QObject::tr("Repairing %1 sec.").arg(RepairSecondsRemaining());
		p.drawText(rc2, Qt::AlignCenter|Qt::AlignVCenter, statText);
		rc2 = rc2.translated(-1,-1);
		p.setPen(QPen(QColor(255,255,255), 0.f));
		p.drawText(rc2, Qt::AlignCenter|Qt::AlignVCenter, statText);
		p.end();
	}
	else
	{
		// Get the name of the system with the worst damage. This is usually the cause of the alert condition.
		QString worstSystem = WorstDamagedSystem();
		int pctWorstSys;
		if(worstSystem.isEmpty())
		{
			worstSystem = QObject::tr("Systems");
			pctWorstSys = 100;
		}
		else
			pctWorstSys = 100 - WorstSystemDamage();

		// Condition of Worst System
		p.begin(&pane);
		p.setFont(QFont(g_Au.TypefaceSans(),7));
		switch(AlertConditionOfSystems())
		{
		case ConditionGreen:
			p.fillRect(rc, QColor(0,255,0));
			p.setPen(QPen(QColor(0,32,0), 0.f));
			p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, worstSystem);
			break;
		case ConditionYellow:
			if(last.SecIsOdd())
				p.fillRect(rc, QColor(255,192,32));
			else
				p.fillRect(rc, QColor(128,96,16));
			p.setPen(QPen(QColor(32,24,3), 0.f));
			p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, worstSystem);
			break;
		case ConditionRed:
			if(last.SecIsOdd())
				p.fillRect(rc, QColor(255,0,0));
			else
				p.fillRect(rc, QColor(128,0,0));
			p.setPen(QPen(QColor(32,0,0), 0.f));
			p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, worstSystem);
			break;
		}
		p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, QString("%1%").arg(pctWorstSys));
		p.end();
	}
	QImage imp = pane.toImage();
	pTex->ReplaceSubImage(re, imp, 131,3);

	// Solar System
	p.begin(&pane);
	p.fillRect(rc, QColor(0,0,0));
	p.setFont(QFont(g_Au.TypefaceSans(),7));
	p.setPen(QPen(QColor(128,128,128), 0.f));
	p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("System"));
	if(mysys)
	{
		p.setPen(QPen(QColor(200,200,200), 0.f));
		p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, mysys->Name());
	}
	p.end();
	imp = pane.toImage();
	pTex->ReplaceSubImage(re, imp, 3,45);

	// Planet
	p.begin(&pane);
	p.fillRect(rc, QColor(0,0,0));
	p.setFont(QFont(g_Au.TypefaceSans(),7));
	p.setPen(QPen(QColor(128,128,128), 0.f));
	p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("World"));
	if(planet)
	{
		p.setPen(QPen(QColor(200,200,200), 0.f));
		p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, planet->Name());
	}
	p.end();
	imp = pane.toImage();
	pTex->ReplaceSubImage(re, imp, 3,24);

	// Destination
	p.begin(&pane);
	p.fillRect(rc, QColor(0,0,0));
	p.setFont(QFont(g_Au.TypefaceSans(),7));
	p.setPen(QPen(QColor(128,128,128), 0.f));
	p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("Dest"));
	if(HasWaypoint())
	{
		const DeWaypoint* wp = LockWaypoint(0);
		if(wp)
		{
			const DaAtom* pwaydest = ether->FindNormalAtom(wp->m_idSystem);
			const DaSolarSystem* pwayss = dynamic_cast<const DaSolarSystem*>(pwaydest);
			if(pwaydest)
			{
				//qDebug("Found waypoint destination solarsystem, destid %s ssid %s", 
						//(const char*)wp->Destination().StringId(),
						//(const char*)pwaydest->Id().StringId());
				AuPoint fwp;
				const DaAtom* pdest = wp->Location(pwaydest, fwp);
				if(pdest)
				{
					if(mysys && mysys->Is(pwaydest->Id()))
					{
						// The next waypoint is in this system.
						if(wp->DestinationOffset().IsZero())
						{
							// The atom is the destination.
							p.setPen(QPen(QColor(200,200,200), 0.f));
							p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, pdest->Name());
						}
						else
						{
							// The atom is a nav point.
							QString navname;
							if(pwayss)
								navname = pwayss->WormholeNameAt(wp->DestinationOffset());
							if(navname.isEmpty())
								navname = QObject::tr("Nav Point");
							p.setPen(QPen(QColor(200,200,200), 0.f));
							p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, navname);
						}
					}
					else if(pwayss)
					{
						//qDebug("Found waypoint destination atom is in another system");
						// The next waypoint is in another system.
						p.setPen(QPen(QColor(100,100,200), 0.f));
						p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, pdest->Name());
					}
				}
			}
		}
		UnlockWaypoint();
	}
	p.end();
	imp = pane.toImage();
	pTex->ReplaceSubImage(re, imp, 131,45);

	int etesecs = ETE();
	if(etesecs != m_MissionStatusETESecs && etesecs >= 0)
	{
		m_MissionStatusETESecs = etesecs;

		// Break down the ete into hours minutes seconds.
		int hours = (int)(etesecs/(60*60));
		etesecs -= hours*(60*60);
		int mins = (int)(etesecs/60);
		etesecs -= mins*60;
		int secs = round(etesecs);
		if(secs == 60)
		{
			secs = 0;
			mins++;
		}

		QString sete;
		if(hours)
			sete = QString("%1:%2:%3").arg(hours).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
		else
			sete = QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));

		p.begin(&pane);
		p.fillRect(rc, QColor(0,0,0));
		p.setFont(QFont(g_Au.TypefaceSans(),7));
		p.setPen(QPen(QColor(128,128,128), 0.f));
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("ETE"));
		p.setPen(QPen(QColor(200,200,200), 0.f));
		p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, sete);
		p.end();

		imp = pane.toImage();
		pTex->ReplaceSubImage(re, imp, 131,24);
	}

	// Structure Status
	if(RepairSecondsRemaining())
	{
		// Ship is getting repaired.
		if(last.SecIsOdd())
			pane.load(":data/conRepairs2.png");
		else
			pane.load(":data/conRepairs3.png");
		p.begin(&pane);
		p.setFont(QFont(g_Au.TypefaceSans(),8));
		p.setPen(QPen(QColor(0,0,0), 0.f));
		QString statText = QObject::tr("Repairing %1 sec.").arg(RepairSecondsRemaining());
		p.drawText(rc, Qt::AlignCenter|Qt::AlignVCenter, statText);
		rc = rc.translated(-1,-1);
		p.setPen(QPen(QColor(255,255,255), 0.f));
		p.drawText(rc, Qt::AlignCenter|Qt::AlignVCenter, statText);
		p.end();
	}
	else
	{
		// The ship is not getting repaired.
		unsigned int curHits = HitsDamageStructural();
		unsigned int maxHits = DesignHitsStructural();
		int pctRemaining;
		if(maxHits && curHits < maxHits)
			pctRemaining = Percent((maxHits - curHits), maxHits);	// percent of remaining hit points
		else
			pctRemaining = 0;
		if(!pctRemaining && curHits < maxHits)
			pctRemaining = 1;	// Slightly more than zero percent remaining.
		else if(curHits && pctRemaining == 100)
			pctRemaining = 99;	// Slightly less than 100 percent remaining.

		// These rgb color values came from DaVehicle::RenderBodyStats. These are the colors a vehicle
		// uses to draw its health bar when you spot it with your cursor.
		QColor bgcol, fgcol;
		ALERT_CONDITION ac;
		if(IsAnyConditionVisible())
			ac = ConditionRed;
		else
			ac = AlertConditionOfStructure();
		switch(ac)
		{
		case ConditionGreen:
			bgcol.setRgbF(.04f,.04f,.2f);
			fgcol.setRgbF(.2f,.2f,1.f);
			break;
		case ConditionYellow:
			if(last.SecIsOdd())
			{
				bgcol.setRgbF(.2f,.14f,.04f);
				fgcol.setRgbF(1.f,.7f,.2f);
			}
			else
			{
				bgcol.setRgbF(.1f,.07f,.02f);
				fgcol.setRgbF(.5f,.35f,.1f);
			}
			break;
		case ConditionRed:
			if(last.SecIsOdd())
			{
				bgcol.setRgbF(.2f,.04f,.04f);
				fgcol.setRgbF(1.f,.2f,.2f);
			}
			else
			{
				bgcol.setRgbF(.1f,.02f,.02f);
				fgcol.setRgbF(.5f,.1f,.1f);
			}
			break;
		}

		// Structural Health as a percentage.
		p.begin(&pane);
		p.fillRect(rc, bgcol);	// Background is filled with unhealthy color.
		QRect rcstat(rc);
		int right = rcstat.width() * pctRemaining / 100;
		if(!right && pctRemaining)
			right = 1;
		else if(curHits && right == rcstat.width())
			right = rcstat.width() - 1;
		rcstat.setRight(right);
		p.fillRect(rcstat, fgcol);	// Healthy part of the bar.
		p.setFont(QFont(g_Au.TypefaceSans(),7));
		p.setPen(QPen(QColor(128,128,128), 0.f));
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("Structure"));
		p.setPen(QPen(QColor(200,200,200), 0.f));
		QString statText = QString("%1%").arg(pctRemaining);
		p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, statText);
		p.end();
	}
	imp = pane.toImage();
	pTex->ReplaceSubImage(re, imp, 3,3);
}

void DaSc::AnimateEmpireFlagAndName(AuGLEffects& re)
{
	if(!m_EmpireFlagDecals && !m_EmpireNameDecals)
		return;

	quint32 empireId = EmpireId();
	if(empireId == m_FlagEmpire)
		return;

	QImage texImg;
	QString ename;
	QColor themeColors[AuScFace::THEME_Count];
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireFlag(empireId, texImg);
	gd.EmpireName(empireId, ename);
	gd.EmpireThemeColors(EmpireId(), themeColors);
	g_Au.UnlockData();
	if(texImg.isNull() || ename.isEmpty())
		return;

	// Calculate scaling to maintain aspect ratio of flag image; max dimension is 64x64.
	double flagScaleX, flagScaleY;
	if(texImg.height() == 0 || texImg.width() == 0)
	{
		flagScaleX = 1;
		flagScaleY = 1;
	}
	else if(texImg.height() <= texImg.width())
	{
		flagScaleX = 1;
		flagScaleY = (double)texImg.height() / (double)texImg.width();
	}
	else
	{
		flagScaleX = (double)texImg.width() / (double)texImg.height();
		flagScaleY = 1;
	}

	m_FlagEmpire = empireId;
	QString stid = QString("Flag:%1").arg(empireId);
	GLuint flagTex = re.CreateTextureFromBitmap(Id().GetID(), stid, texImg, GL_LINEAR, GL_LINEAR, false, 1);

	AuGLTextItem textItem;
	re.GetRenderTextItem(textItem, ename, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
	GLuint nameTexId = textItem.m_Texture;

	// Update empire flag and name textures.
	m_DesignLock.lockForRead();
	for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
			di != m_Design.StatusDisplays().cend(); di++)
	{
		if((*di).PartType() == AuScPart::Part_EmpireFlag)
		{
			QHash<quint16, GLuint>::iterator oi = m_OwnTextures.find(di.key());
			if(oi != m_OwnTextures.end())
				*oi = flagTex;

			// Delete the old flag vao because the aspect ratio is probably changing.
			QHash<quint16, AuScVAO>::iterator vi = m_OwnDecalOpaqueVAOs.find(di.key());
			if(vi != m_OwnDecalOpaqueVAOs.end())
			{
				(*vi).DeleteGL();
				m_OwnDecalOpaqueVAOs.erase(vi);
			}
			vi = m_OwnDecalTransparentVAOs.find(di.key());
			if(vi != m_OwnDecalTransparentVAOs.end())
			{
				(*vi).DeleteGL();
				m_OwnDecalTransparentVAOs.erase(vi);
			}

			const AuScDesign* scd = LockDesign();
			if(scd)
			{
				const AuScParts& parts = scd->Parts();
				AuScParts::const_iterator pi = parts.find(di.key());
				if(pi != parts.cend())
				{
					AuScTriRenderList opaqueTris, transparentTris;
					(*pi)->RenderTrianglesScaled(opaqueTris, transparentTris, themeColors, flagScaleX, flagScaleY);
					if(!opaqueTris.empty())
					{
						AuScVAO vao;
						vao.Create(re, opaqueTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_OwnDecalOpaqueVAOs.insert(pi.key(), vao);
					}
					if(!transparentTris.empty())
					{
						AuScVAO vao;
						vao.Create(re, transparentTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_OwnDecalTransparentVAOs.insert(pi.key(), vao);
					}
				}
			}
			UnlockDesign();
		}
		else if((*di).PartType() == AuScPart::Part_EmpireName)
		{
			QHash<quint16, GLuint>::iterator oi = m_OwnTextures.find(di.key());
			if(oi != m_OwnTextures.end())
				*oi = nameTexId;
		}
	}
	m_DesignLock.unlock();
}

void DaSc::AnimateBerthNames(AuGLEffects& re)
{
	if(!m_BerthNameDecals)
		return;

	//qDebug() << "DaSc::AnimateBerthNames" << m_BerthNameDecals;
	// Update berth name textures.
	m_BerthNames.clear();
	m_DesignLock.lockForRead();
	const AuRoster& roster = LockRoster();
	for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
			di != m_Design.StatusDisplays().cend(); di++)
	{
		if((*di).PartType() == AuScPart::Part_BerthName)
		{
			// Locate the name of the berth occupant.
			QString occName = roster.BerthOccupantName((*di).AssociatedPartId());
			if(occName.isEmpty())
				occName = tr("Unassigned");
			//qDebug() << "DaSc::AnimateBerthNames occ" << occName << (*di).AssociatedPartId();
			m_BerthNames.insert(di.key(), occName);	// Store last known occupant name with decal id.

			// Acquire texture of the name, creating it if necessary.
			AuGLTextItem textItem;
			re.GetRenderTextItem(textItem, occName, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
			GLuint nameTexId = textItem.m_Texture;
			//qDebug() << "DaSc::AnimateBerthNames nameTex" << nameTexId;

			// Update the decal's own texture id.
			QHash<quint16, GLuint>::iterator oi = m_OwnTextures.find(di.key());
			if(oi != m_OwnTextures.end())
			{
				//qDebug() << "DaSc::AnimateBerthNames replacing tex" << *oi;
				*oi = nameTexId;
			}
		}
	}
	UnlockRoster();
	m_DesignLock.unlock();
}

void DaSc::AnimateCaptName(AuGLEffects& re)
{
	if(!m_CaptNameDecals)
		return;

	QString captName = CaptainName();
	if(captName.isEmpty())
	{
		// Find first officer.
		AuId fo = AIFirstOfficer();
		if(fo.IsNull())
			captName = tr("No Commander");
		else
		{
			m_RosterLock.lockForRead();
			captName = m_Roster.BerthOccupantName(fo);
			m_RosterLock.unlock();
			if(captName.isEmpty())
				captName = tr("No Commander");
			else
				captName = tr("1st Officer: %1").arg(captName);
		}
	}
	else
		captName = tr("Captain: %1").arg(captName);
	if(captName == m_CaptName)
		return;

	m_CaptName = captName;
	AuGLTextItem textItem;
	re.GetRenderTextItem(textItem, captName, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
	GLuint nameTexId = textItem.m_Texture;

	// Update capt name textures.
	m_DesignLock.lockForRead();
	for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
			di != m_Design.StatusDisplays().cend(); di++)
	{
		if((*di).PartType() == AuScPart::Part_CaptName)
		{
			QHash<quint16, GLuint>::iterator oi = m_OwnTextures.find(di.key());
			if(oi != m_OwnTextures.end())
				*oi = nameTexId;
		}
	}
	m_DesignLock.unlock();
}

void DaSc::AnimateShipName(AuGLEffects& re)
{
	if(!m_ShipNameDecals)
		return;

	QString shipName = Name();
	if(shipName == m_ShipName)
		return;

	m_ShipName = shipName;
	AuGLTextItem textItem;
	re.GetRenderTextItem(textItem, shipName, g_Au.FontAtom(), Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, 1);
	GLuint nameTexId = textItem.m_Texture;

	// Update ship name textures.
	m_DesignLock.lockForRead();
	for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
			di != m_Design.StatusDisplays().cend(); di++)
	{
		if((*di).PartType() == AuScPart::Part_ShipName)
		{
			QHash<quint16, GLuint>::iterator oi = m_OwnTextures.find(di.key());
			if(oi != m_OwnTextures.end())
				*oi = nameTexId;
		}
	}
	m_DesignLock.unlock();
}

void DaSc::AnimateWorldMap()
{
	if(m_WorldMaps <= 0)
		return;

	// World map is animated by selecting the surface texture from the current world. The default map screen texture is used when
	// none is available.
	// This is animated in every frame so we don't use a surface texture that was deleted.
	GLuint texId = 0;
	if(m_InsideVisible)
	{
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
			texId = world->SurfaceTexture();
	}

	// When no surface texture of the world is available, use the default world map screen texture.
	if(!texId)
		texId = m_WorldMapTex;

	// Update world map textures.
	int mapsUpdated = 0;
	m_DesignLock.lockForRead();
	for(AuScReportStatusDisplayHash::const_iterator di = m_Design.StatusDisplays().cbegin();
			di != m_Design.StatusDisplays().cend(); di++)
	{
		if((*di).PartType() == AuScPart::Part_StatusWorldMap)
		{
			QHash<quint16, GLuint>::iterator oi = m_OwnTextures.find(di.key());
			if(oi != m_OwnTextures.end())
				*oi = texId;
			mapsUpdated++;
			if(mapsUpdated == m_WorldMaps)
				break;
		}
	}
	m_DesignLock.unlock();
}

void DaSc::SetAlertCondition(ALERT_CONDITION alert)
{
	if(m_AlertCondition == alert)
		return;

	if(m_AlertCondition > alert)
	{
		// Alert condition elevated.
		// Wake up sleeping sentients who are aboard.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaSentient* sent = dynamic_cast<DaSentient*>(*it);
			if(!sent)
				continue;
			if(!sent->IsAIControlled())
				continue;
			sent->WakeUp();
		}
	}

	// Change the alert condition.
	m_AlertCondition = alert;
	SetModified();
}

void DaSc::NavLightToggle(bool onState)
{
	m_NavLights = onState;
}

void DaSc::RotatingBeaconToggle(bool onState)
{
	m_RotatingBeacons = onState;
}

void DaSc::StrobeToggle(bool onState)
{
	m_Strobes = onState;
}

void DaSc::RenderLightSprites(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari)
{
	if(!m_RotatingBeacons && !IsHeadlightOn() && !m_NavLights && !m_StrobesLit)
		return;

	ari.re->glEnable(GL_DEPTH_TEST);
	ari.re->glDepthMask(GL_FALSE);
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	if(IsHeadlightOn() || m_NavLights || m_StrobesLit)
	{
		// Use sprites to render light spots.
		if(!m_LightOmniTex)
			m_LightOmniTex = ari.re->CreateTextureFromBitmap(Id().GetID(), "LOmni", ":data/txLightOmni.png");

		ari.re->glEnable(GL_PROGRAM_POINT_SIZE);
		ari.re->GLSL().SpriteSizeShader().Select();
		ari.re->GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
		ari.re->GLSL().SpriteSizeShader().SetTexture(m_LightOmniTex);

		// Headlight
		if(IsHeadlightOn())
		{
			float atten = sqrtf(1/m_HkQ);
			float spriteSize = 3000 + atten / (250*FeetPerMeter()) * 3000;
			ari.re->GLSL().SpriteSizeShader().SetColor(m_HColor);
			ari.re->GLSL().SpriteSizeShader().SetSize(spriteSize);

			// Vertex locations of the particles.
			QList<AuPointAlpha> verts;
			for(QHash<quint16, AuLensGlow>::const_iterator gi = m_LensHeadlightGlows.cbegin(); gi != m_LensHeadlightGlows.cend();
					gi++)
				verts << AuPointAlpha((*gi).Loc());

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, verts, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}

		// Nav lights
		if(m_NavLights && !m_LensNavGlows.empty())
		{
			ari.re->GLSL().SpriteSizeShader().SetSize(3000);

			// Display these by color.
			QList<QColor> doneColors;
			QList<AuPointAlpha> verts;
			do
			{
				QColor passColor;
				bool passColorValid = false;
				verts.clear();
				for(QHash<quint16, AuLensGlow>::const_iterator gi = m_LensNavGlows.cbegin(); gi != m_LensNavGlows.cend(); gi++)
				{
					if(doneColors.contains((*gi).Color()))
						continue;	// Skip lens colors that have already been done.
					if(!passColorValid)
					{
						// Found first color for this pass.
						passColor = (*gi).Color();
						passColorValid = true;
					}
					else if((*gi).Color() != passColor)
						continue;	// Skip lens colors that are not the same color as this pass.
					verts << AuPointAlpha((*gi).Loc());
				}
				if(!verts.empty())
				{
					// We found verts sprites to display.
					ari.re->GLSL().SpriteSizeShader().SetColor(passColor);
					doneColors.append(passColor);

					// Make a VAO of the vertex locations.
					AuGLVAO vao;
					vao.CreateForSprites(*ari.re, verts, GL_STREAM_DRAW);

					// Display the VAO once and clean it up.
					if(vao.VAO())
					{
						vao.SelectGL();
						vao.DrawGL(GL_POINTS);
						vao.DeselectGL();
						vao.DeleteGL();
					}
				}
			}
			while(!verts.empty());
		}

		// Strobes
		if(m_StrobesLit && !m_LensStrobeGlows.empty())
		{
			ari.re->GLSL().SpriteSizeShader().SetColor((*m_LensStrobeGlows.cbegin()).Color());
			ari.re->GLSL().SpriteSizeShader().SetSize(4000);

			// Vertex locations of the particles.
			QList<AuPointAlpha> verts;
			for(QHash<quint16, AuLensGlow>::const_iterator gi = m_LensStrobeGlows.cbegin(); gi != m_LensStrobeGlows.cend(); gi++)
				verts << AuPointAlpha((*gi).Loc());

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForSprites(*ari.re, verts, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
		ari.re->glDisable(GL_PROGRAM_POINT_SIZE); 
		ari.re->GLSL().SpriteSizeShader().Deselect();
	}

	if(m_RotatingBeacons && !m_LensBeaconGlows.empty())
	{
		if(!m_LightSpotTex)
			m_LightSpotTex = ari.re->CreateTextureFromBitmap(Id().GetID(), "LHeadLight", ":data/txLightHeadlight.png");

		if(!m_LightSpotVAO.VAO())
		{
			// Spot texture is centered at the bottom edge of the image, pointing up the image.
			double spotSize = 3*FeetPerMeter();
			const float s = 0;
			const float t = 0;
			const float u = 1;
			const float v = 1;
			AuTriTex tri0;
			tri0.m_V0 = AuPointTex(-.5*spotSize,0,0, s,t);
			tri0.m_V1 = AuPointTex(-.5*spotSize,spotSize,0, s,v);
			tri0.m_V2 = AuPointTex(.5*spotSize,spotSize,0, u,v);
			AuTriTex tri1;
			tri1.m_V0 = AuPointTex(.5*spotSize,spotSize,0, u,v);
			tri1.m_V1 = AuPointTex(.5*spotSize,0,0, u,t);
			tri1.m_V2 = AuPointTex(-.5*spotSize,0,0, s,t);
			AuTriTex tri2;
			tri2.m_V0 = AuPointTex(-.5*spotSize,0,0, s,t);
			tri2.m_V1 = AuPointTex(-.5*spotSize,-spotSize,0, s,v);
			tri2.m_V2 = AuPointTex(.5*spotSize,-spotSize,0, u,v);
			AuTriTex tri3;
			tri3.m_V0 = AuPointTex(.5*spotSize,-spotSize,0, u,v);
			tri3.m_V1 = AuPointTex(.5*spotSize,0,0, u,t);
			tri3.m_V2 = AuPointTex(-.5*spotSize,0,0, s,t);
			QList<AuTriTex> tris;
			tris << tri0 << tri1 << tri2 << tri3;
			m_LightSpotVAO.CreateForTexture(*ari.re, tris, GL_STATIC_DRAW);
		}

		// Configure shader.
		ari.re->glDisable(GL_CULL_FACE);
		ari.re->GLSL().TextureShader().Select();
		ari.re->GLSL().TextureShader().SetTexture(m_LightSpotTex);
		for(QHash<quint16, AuLensGlow>::const_iterator gi = m_LensBeaconGlows.cbegin(); gi != m_LensBeaconGlows.cend(); gi++)
		{
			QHash<quint16, AuPoint>::const_iterator ai = m_LensBeaconAxes.find(gi.key());
			if(ai == m_LensBeaconAxes.cend())
				continue;
			AuMatrix4x4 lmodel(model);
			lmodel.Translate((*gi).Loc());
			lmodel *= AuPoint(1,0,0).GetAlignmentTransform(*ai);
			lmodel.RotateX(m_LensBeaconAngle);

			ari.re->GLSL().TextureShader().SetColor((*gi).Color().redF(), (*gi).Color().greenF(), (*gi).Color().blueF(), 1);
			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * lmodel);
			m_LightSpotVAO.SelectGL();
			m_LightSpotVAO.DrawGL();
			m_LightSpotVAO.DeselectGL();
		}
		ari.re->GLSL().TextureShader().Deselect();
		ari.re->glEnable(GL_CULL_FACE);
	}

	ari.re->glDepthMask(GL_TRUE);
	ari.re->glDisable(GL_BLEND);
}

const GLuint* DaSc::EffectTextures() const
{
	return m_EffectCur;
}

void DaSc::AnimateEffectTextures()
{
	bool capacitorWorking = CapacitorLevel();

	quint16 warpDrive = FTLWarpDrive();
	int ftlHullHot = 0;	// 0=no, 1=warp, 2=wormhole positive, 3=wormhole negative, 4=wormhole neutral
	int ftlWorking[4] = {0, 0, 0, 0};	// 0=no, 1=on, 2=hot
	m_FTLLock.lockForRead();
	if(capacitorWorking)
	{
		for(AuFTLDriveStateHash::iterator fi = m_FTLDrives.begin(); fi != m_FTLDrives.end(); fi++)
		{
			if((*fi).Power() == FTL_DRIVE_On)
			{
				// FTL drive is on.
				int ftlWhich = ConsoleEffectWhich(fi.key());
				if((*fi).Commit() >= FTL_COMMIT_ChargedBegin && (*fi).Commit() <= FTL_COMMIT_ChargedEnd)
				{
					// FTL drive is hot.
					ftlWorking[ftlWhich] = 2;
					if(fi.key() == warpDrive)
						ftlHullHot = 1;
					else if(ftlHullHot == 0)
						ftlHullHot = (*fi).Setting() + 2;	// Wormhole polarity setting: (0=+, 1=-, 2=neutral)
				}
				else
					ftlWorking[ftlWhich] = 1;
			}
		}
	}
	m_FTLLock.unlock();

	int maneuverWorking = 0;	// 0=no, 1=low, 2=high
	if(GravityDriveWorking())
	{
		GRAVITY_DRIVE_STATE mdrive = GravityDriveState();
		if(mdrive == GD_On)
			maneuverWorking = 2;	// high
		else if(mdrive == GD_Hold)
			maneuverWorking = 1;	// low
	}

	bool lifeSupportWorking = LifeSupportWorking();
	bool powerPlantWorking = PowerPlantWorking();
	bool sensorWorking = SensorsWorking();
	bool shieldWorking = ShieldsWorkingAndUp();

	int transporterWorking[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};	// 0=no, 1=low, 2=high
	m_TRoomLock.lockForRead();
	for(AuTransporterSystemStateHash::const_iterator ti = m_TRoomStates.cbegin(); ti != m_TRoomStates.cend();  ti++)
	{
		if(*ti == TRANSPORTER_Active)
		{
			int transporterWhich = ConsoleTransporterEffectWhich(ti.key());
			transporterWorking[transporterWhich] = 2;	// high
		}
		else if(*ti == TRANSPORTER_On)
		{
			int transporterWhich = ConsoleTransporterEffectWhich(ti.key());
			transporterWorking[transporterWhich] = 1;	// low
		}
	}
	m_TRoomLock.unlock();

	// Animate surface texture on weapon units that have fired in the past couple of seconds.
	bool weaponWorking[10] = {false, false, false, false, false, false, false, false, false, false};
	m_BayLock.lockForRead();
	for(unordered_map<quint16, AuBayFired>::const_iterator bi = m_BayFired.cbegin(); bi != m_BayFired.cend(); bi++)
		if(bi != m_BayFired.cend() && (*bi).second.m_When.ElapsedSec() <= 2)
		{
			int bayWhich = ConsoleEffectWhich((*bi).first);
			weaponWorking[bayWhich] = true;
		}
	m_BayLock.unlock();

	float secoff = AuTime::CurrentSecCycle();
	int secframe = secoff * 6;
	int randframe = m_EffectRandFrame;
	if(secframe != m_EffectFrame)
	{
		// Only change random frame when the sequential frame changes or the random frames change too fast.
		do
		{
			randframe = g_Au.Dice().Roll(1,6,-1);
		}
		while(randframe == m_EffectRandFrame);
		m_EffectRandFrame = randframe;
	}

	m_EffectCur[EFF_None] = m_EffectTex[EFF_None][0];

	for(int s=0; s<4; s++)
	{
		if(ftlWorking[s] == 2)
			m_EffectCur[EFF_FTLDrive1+s] = m_EffectTex[EFF_FTLDrive1+s][randframe];	// High
		else if(ftlWorking[s] == 1)
			m_EffectCur[EFF_FTLDrive1+s] = m_EffectTex[EFF_Count+3][randframe];	// Low
		else
			m_EffectCur[EFF_FTLDrive1+s] = m_EffectTex[EFF_None][0];	// Off
	}

	if(lifeSupportWorking)
		m_EffectCur[EFF_LifeSupport] = m_EffectTex[EFF_LifeSupport][secframe];
	else
		m_EffectCur[EFF_LifeSupport] = m_EffectTex[EFF_None][0];

	if(maneuverWorking == 2)
		m_EffectCur[EFF_ManeuverDrive] = m_EffectTex[EFF_ManeuverDrive][randframe];	// High
	else if(maneuverWorking == 1)
		m_EffectCur[EFF_ManeuverDrive] = m_EffectTex[EFF_Count][randframe];	// Low
	else
		m_EffectCur[EFF_ManeuverDrive] = m_EffectTex[EFF_None][0];	// Off

	if(powerPlantWorking)
		m_EffectCur[EFF_PowerPlant] = m_EffectTex[EFF_PowerPlant][secframe];
	else
		m_EffectCur[EFF_PowerPlant] = m_EffectTex[EFF_None][0];

	if(sensorWorking)
		m_EffectCur[EFF_Sensor] = m_EffectTex[EFF_Sensor][randframe];
	else
		m_EffectCur[EFF_Sensor] = m_EffectTex[EFF_None][0];

	if(shieldWorking)
		m_EffectCur[EFF_Shield] = m_EffectTex[EFF_Shield][randframe];
	else
		m_EffectCur[EFF_Shield] = m_EffectTex[EFF_None][0];

	for(int s=0; s<10; s++)
	{
		if(transporterWorking[s] == 2)
			m_EffectCur[EFF_TransporterPad1+s] = m_EffectTex[EFF_TransporterPad1+s][secframe];	// High
		else if(transporterWorking[s] == 1)
			m_EffectCur[EFF_TransporterPad1+s] = m_EffectTex[EFF_Count+1][secframe];	// Low
		else
			m_EffectCur[EFF_TransporterPad1+s] = m_EffectTex[EFF_None][0];	// Off
	}

	for(int s=0; s<10; s++)
	{
		if(weaponWorking[s])
			m_EffectCur[EFF_Weapon1+s] = m_EffectTex[EFF_Weapon1+s][randframe];
		else
			m_EffectCur[EFF_Weapon1+s] = m_EffectTex[EFF_None][0];
	}

	if(capacitorWorking)
		m_EffectCur[EFF_Capacitor] = m_EffectTex[EFF_Capacitor][secframe];
	else
		m_EffectCur[EFF_Capacitor] = m_EffectTex[EFF_None][0];

	// Hull effect.
	if(ftlHullHot == 1)
	{
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Count+2][secframe];	// warp
		m_HullEffectAppl = 2;
	}
	else if(ftlHullHot == 2)
	{
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Hull][secframe];	// wormhole positive
		m_HullEffectAppl = 2;
	}
	else if(ftlHullHot == 3)
	{
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Count+4][secframe];	// wormhole negative
		m_HullEffectAppl = 2;
	}
	else if(ftlHullHot == 4)
	{
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Count+5][secframe];	// wormhole neutral
		m_HullEffectAppl = 2;
	}
	else
	{
		// No FTL drive is hot.
		// Select damage texture based on overall damage level.
		quint32 curHits = HitsDamageStructural();
		quint32 maxHits = DesignHitsStructural();
		int dmgIdx;
		if(maxHits)
			dmgIdx = curHits * 10 / maxHits;
		else
			dmgIdx = 0;
		//dmgIdx = AuTime::CurrentSecsSinceEpoch() % 10;	//DHH testing
		if(dmgIdx > 9)
			dmgIdx = 9;
		if(dmgIdx <= 0)
		{
			m_EffectCur[EFF_Hull] = m_EffectTex[EFF_None][0];
			m_HullEffectAppl = 0;
		}
		else
		{
			m_EffectCur[EFF_Hull] = m_DamageTex[dmgIdx-1];
			m_HullEffectAppl = 1;
		}
	}
	m_EffectFrame = secframe;
}

// Returns the effect which index for a console. 'consoleId' should identify an engineer station or fire control station.
quint8 DaSc::ConsoleEffectWhich(quint16 consoleId) const
{
	quint8 effectWhich = 0;
	m_DesignLock.lockForRead();
	AuScReportStationHash::const_iterator si = m_Design.CrewStations().find(consoleId);
	if(si != m_Design.CrewStations().cend())
		effectWhich = (*si).WhichEffect();
	m_DesignLock.unlock();
	return effectWhich;
}

// Returns the effect which index for a transporter console.
quint8 DaSc::ConsoleTransporterEffectWhich(quint16 consoleId) const
{
	quint8 effectWhich = 0;
	m_DesignLock.lockForRead();
	AuScReportTransporterStationHash::const_iterator si = m_Design.Transporters().find(consoleId);
	if(si != m_Design.Transporters().cend())
		effectWhich = (*si).WhichEffect();
	m_DesignLock.unlock();
	return effectWhich;
}

bool DaSc::IsNavLightOn() const
{
	return m_NavLights;
}

bool DaSc::IsRotatingBeaconOn() const
{
	return m_RotatingBeacons;
}

bool DaSc::IsStrobeOn() const
{
	return m_Strobes;
}

bool DaSc::IsLandingGearExtended() const
{
	m_DesignLock.lockForRead();
	bool v = m_LandingGearStates.IsAllExtended();
	m_DesignLock.unlock();
	return v;
}

void DaSc::LandingGearToggle(bool isExtended)
{
	m_DesignLock.lockForRead();
	if(isExtended)
		m_LandingGearStates.Extend();
	else
		m_LandingGearStates.Retract();
	m_LandingGearChg.Set();
	m_DesignLock.unlock();
}

bool DaSc::HasRetractableLandingGear() const
{
	m_DesignLock.lockForRead();
	bool v = !m_LandingGearStates.m_States.empty();
	m_DesignLock.unlock();
	return v;
}

bool DaSc::InstallSystemModule(const AuSysId& sysId, const AuItem& module, quint16 installSecs)
{
	if(!DaVessel::InstallSystemModule(sysId, module, installSecs))
		return false;

	// Hull passes me a raw material, not a module.
	COMMODITYID moduleCID;
	if(sysId.System() == SYS_Hull)
	{
		moduleCID = HullMaterialModule(module.CommodityId());
		if(moduleCID == CID_Invalid)
			moduleCID = module.CommodityId();	// This should not happen but this may be a safe fallback.
	}
	else
		moduleCID = module.CommodityId();

	// Update module information in my design report.
	m_DesignLock.lockForWrite();
	m_Design.ModuleInstalled(sysId.Which(), moduleCID, module.Quality());
	m_DesignChg.Set();
	m_DesignLock.unlock();
	SetModified();
	return true;
}

void DaSc::VehicleParkingSpots(VEHICLE_SPOT_TYPE spotType, QList<AuMatrix4x4>& parkingSpots) const
{
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		if(spotType == SPOT_GroundVehicle)
			scd->GroundVehicleParkingSpots(parkingSpots);
		else if(spotType == SPOT_SpaceVehicle)
			scd->SpaceVehicleParkingSpots(parkingSpots);
		else if(spotType == SPOT_Rocket)
			scd->RocketParkingSpots(parkingSpots);
		else if(spotType == SPOT_WaterVehicle)
			scd->WaterVehicleParkingSpots(parkingSpots);
	}
	UnlockDesign();
}

quint16 DaSc::VehicleTransporterSystemId(VEHICLE_SPOT_TYPE spotType, bool mustBeOn) const
{
	QList<AuMatrix4x4> parkingSpots;
	VehicleParkingSpots(spotType, parkingSpots);
	if(parkingSpots.empty())
		return 0xffff;

	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	int mostworking = -1;
	quint16 bestWhich = 0;
	if(mustBeOn)
	{
		// Search for a transporter room that is on and inactive.
		QMap<quint16, quint16> roomsFound;	// Rooms found that will work. Key is room 'rsub' number and value is 'which' value.
		QList<quint16> troomIds(TransporterRoomIds());
		for(QList<quint16>::const_iterator ti = troomIds.cbegin(); ti != troomIds.cend();  ti++)
		{
			if(IsTransporterSystemOn(*ti) && !IsTransporterSystemActive(*ti))
			{
				// The room is on and not active.
				if(SystemModule(SYS_Transporter, *ti) == CID_VehicleTransporterModule)
				{
					// The transporter is equipped for beaming vehicles.
					int working = GetPowerConsumption(SYS_Transporter, *ti);

					// Bottom byte of 'which' value is the room 'rsub' number. The key is the 'which' value.
					roomsFound.insert(*ti, *ti);	// The map will be ordered by room 'rsub' value.

					// Haven't found enough pads in one room yet so remember the room with the most working pads.
					if(working > mostworking)
					{
						mostworking = working;
						bestWhich = *ti;
					}
				}
			}
		}

		// If we found any rooms that work, return the first one in the map, which is the one with the lowest room 'rsub' number.
		if(!roomsFound.isEmpty())
			return *roomsFound.begin();
	}
	else
	{
		// Search for a transporter room that is inactive. It does not have to be on so we will prefer rooms that are
		// not on. This subtlety allows transporter room to transporter room transport aboard the same vessel.
		//  - A crew member can turn on the transporter in the room they are in then call for beam over, which will beam from
		//  a different transporter room that is not on.
		//  - A friendly unit aboard a station can turn on the transporter room they are in then call for a beam over, which will
		//  beam from a different transporter room that is not on.
		// First pass searches for a room that is not on and adequate to our needs.
		QMap<quint16, quint16> roomsFound;	// Rooms found that will work. Key is room 'rsub' number and value is 'which' value.
		QList<quint16> troomIds(TransporterRoomIds());
		for(QList<quint16>::const_iterator ti = troomIds.cbegin(); ti != troomIds.cend();  ti++)
		{
			if(!IsTransporterSystemOn(*ti) && !IsTransporterSystemActive(*ti))
			{
				// The room is not on and not active.
				if(SystemModule(SYS_Transporter, *ti) == CID_VehicleTransporterModule)
				{
					// The transporter is equipped for beaming vehicles.
					int working = GetPowerConsumption(SYS_Transporter, *ti);

					// Bottom byte of 'which' value is the room 'rsub' number. The key is the 'which' value.
					roomsFound.insert(*ti, *ti);	// The map will be ordered by room 'rsub' value.

					// Haven't found enough pads in one room yet so remember the room with the most working pads.
					if(working > mostworking)
					{
						mostworking = working;
						bestWhich = *ti;
					}
				}
			}
		}

		// If we found any rooms that work, return the first one in the map, which is the one with the lowest room 'rsub' number.
		if(!roomsFound.isEmpty())
			return *roomsFound.begin();

		// Second pass searches for a room that is on and adequate to our needs.
		for(QList<quint16>::const_iterator ti = troomIds.cbegin(); ti != troomIds.cend();  ti++)
		{
			if(IsTransporterSystemOn(*ti) && !IsTransporterSystemActive(*ti))
			{
				// The room is on and not active.
				if(SystemModule(SYS_Transporter, *ti) == CID_VehicleTransporterModule)
				{
					// The transporter is equipped for beaming vehicles.
					int working = GetPowerConsumption(SYS_Transporter, *ti);

					// Bottom byte of 'which' value is the room 'rsub' number. The key is the 'which' value.
					roomsFound.insert(*ti, *ti);	// The map will be ordered by room 'rsub' value.

					// Haven't found enough pads in one room yet so remember the room with the most working pads.
					if(working > mostworking)
					{
						mostworking = working;
						bestWhich = *ti;
					}
				}
			}
		}

		// If we found any rooms that work, return the first one in the map, which is the one with the lowest room 'rsub' number.
		if(!roomsFound.isEmpty())
			return *roomsFound.begin();
	}
	return bestWhich;
}

bool DaSc::VehicleParkingSpot(VEHICLE_SPOT_TYPE spotType, AuMatrix4x4& parkPosM, const AuScDesign* scd, DaAtomList& vehiclesAboard)
{
	QList<AuMatrix4x4> parkingSpots;
	if(spotType == SPOT_GroundVehicle)
		scd->GroundVehicleParkingSpots(parkingSpots);
	else if(spotType == SPOT_SpaceVehicle)
		scd->SpaceVehicleParkingSpots(parkingSpots);
	else if(spotType == SPOT_Rocket)
		scd->RocketParkingSpots(parkingSpots);
	else if(spotType == SPOT_WaterVehicle)
		scd->WaterVehicleParkingSpots(parkingSpots);
	if(parkingSpots.empty())
		return false;

	// Find the first vacant parking spot.
	for(QList<AuMatrix4x4>::const_iterator si = parkingSpots.cbegin(); si != parkingSpots.cend(); si++)
	{
		bool vacant = true;
		for(DaAtomList::const_iterator it = vehiclesAboard.begin(); it != vehiclesAboard.end(); it++)
		{
			DaVehicle* vehicle = (DaVehicle*)*it;
			if(!vehicle->IsNormal())
				continue;
			AuPoint shipLoc;
			if(!vehicle->GetLocationRelativeToParent(shipLoc, this))
				continue;
			// Check proximity to parking spots. Remove any that are too close.
			double vht = vehicle->GroundHeightAboveCG() + 10;
			AuPoint spotLoc = (*si).UnProject(shipLoc);			// Vehicle location relative to the parking spot.
			if(spotLoc.z > 0 && spotLoc.z < vht && spotLoc.Length_2D() < 15)
			{
				// Found a vehicle occupying the spot.
				vacant = false;
				break;
			}
		}
		if(vacant)
		{
			// Found a vacant parking spot.
			parkPosM = *si;
			return true;
		}
	}
	return false;
}

void DaSc::BalanceRoomAirPressures(bool lifeSupportWorking)
{
	// Server caches hull door room names for balancing room air pressures.
	if(!m_DoorRoomsValid)
	{
		m_DesignLock.lockForWrite();
		const AuScDesign* scd = LockDesign();
		if(scd)
		{
			m_DoorRoomsValid = true;
			const AuScParts& parts = scd->Parts();
			for(AuDoorStateHash::const_iterator di = m_DoorStates.cbegin(); di != m_DoorStates.cend(); di++)
			{
				if(!(*di).IsHullDoor())
					continue;
				// Locate the room void associated with the open state of the hull door.
				for(AuScParts::const_iterator ti = parts.cbegin(); ti != parts.cend(); ti++)
				{
					if((*ti)->IsOmitted())
						continue;
					if(!(*ti)->IsRoomVoid())
						continue;
					if((*ti)->AssociatedPartId() != di.key() || (*ti)->AssociatedPartState() != AuScDoor::Open)
						continue;
					m_DoorRooms.insert(di.key(), (*ti)->GroupName());
					break;
				}
			}
		}
		UnlockDesign();
		m_DesignLock.unlock();
	}
	if(!m_DoorRoomsValid)
		return;

	bool anyAirChg = false;
	m_AirLock.lockForWrite();
	if(lifeSupportWorking)
	{
		// Life support system is working to balance air pressures.
		// Air pressure can be maintained in internal rooms.
		// Air pressure cannot be maintained in rooms with a door open to the outside.
		// Tally which hull doors are open, if any. m_DoorRooms only lists hull doors on the server, where this is called.
		QSet<QString> openRooms;		// Names of rooms with hull doors that are currently open to the outside.
		m_DesignLock.lockForRead();
		for(QHash<quint16, QString>::const_iterator dri = m_DoorRooms.cbegin(); dri != m_DoorRooms.cend(); dri++)
			if(m_DoorStates.IsDoorOpen(dri.key()))
				openRooms.insert(*dri);
		m_DesignLock.unlock();

		// Adjusts the air pressure of each room.
		int targetInsidePressure = 100;	// Balance inside air pressure to standard 50%, which is value 100.
		if(openRooms.empty())
		{
			// No hull doors are open.
			for(AuAirPressureHash::iterator ai = m_AirPressures.begin(); ai != m_AirPressures.end(); ai++)
			{
				if(*ai > targetInsidePressure)
				{
					// Lose air until equalized.
					int newP = *ai - 8;
					if(newP <= targetInsidePressure)
						*ai = targetInsidePressure;
					else
						*ai = (quint8)newP;
					anyAirChg = true;
				}
				else if(*ai < targetInsidePressure)
				{
					// Gain air until equalized.
					int newP = *ai + 8;
					if(newP >= targetInsidePressure)
						*ai = targetInsidePressure;
					else
						*ai = (quint8)newP;
					anyAirChg = true;
				}
			}
		}
		else
		{
			// A hull door is open.
			// Life support system is working to maintain air pressures in rooms with no doors open to the outside.
			int targetOutsidePressure = (int)(AirDensity() * 200.f);	// Equalize open room air pressure with outside.
			for(AuAirPressureHash::iterator ai = m_AirPressures.begin(); ai != m_AirPressures.end(); ai++)
			{
				if(openRooms.contains(ai.key()))
				{
					// The room has a hull door open to the outside.
					if(*ai > targetOutsidePressure)
					{
						// Lose air until equalized.
						int newP = *ai - 12;
						if(newP <= targetOutsidePressure)
							*ai = targetOutsidePressure;
						else
							*ai = (quint8)newP;
						anyAirChg = true;
					}
					else if(*ai < targetOutsidePressure)
					{
						// Gain air until equalized.
						int newP = *ai + 12;
						if(newP >= targetOutsidePressure)
							*ai = targetOutsidePressure;
						else
							*ai = (quint8)newP;
						anyAirChg = true;
					}
				}
				else
				{
					// The room does not have a hull door open to the outside.
					if(*ai > targetInsidePressure)
					{
						// Lose air until equalized.
						int newP = *ai - 8;
						if(newP <= targetInsidePressure)
							*ai = targetInsidePressure;
						else
							*ai = (quint8)newP;
						anyAirChg = true;
					}
					else if(*ai < targetInsidePressure)
					{
						// Gain air until equalized.
						int newP = *ai + 8;
						if(newP >= targetInsidePressure)
							*ai = targetInsidePressure;
						else
							*ai = (quint8)newP;
						anyAirChg = true;
					}
				}
			}
		}
	}
	else
	{
		// Life support system is not working. I am only called in that case if a hull door is open.
		// Air pressure cannot be maintained in any rooms. Air is bleeding out of the ship.
		int targetOutsidePressure = (int)(AirDensity() * 200.f);	// Equalize open hull air pressure with outside.
		for(AuAirPressureHash::iterator ai = m_AirPressures.begin(); ai != m_AirPressures.end(); ai++)
		{
			if(*ai > targetOutsidePressure)
			{
				// Lose air until equalized.
				int newP = *ai - 12;
				if(newP <= targetOutsidePressure)
					*ai = targetOutsidePressure;
				else
					*ai = (quint8)newP;
				anyAirChg = true;
			}
			else if(*ai < targetOutsidePressure)
			{
				// Gain air until equalized.
				int newP = *ai + 12;
				if(newP >= targetOutsidePressure)
					*ai = targetOutsidePressure;
				else
					*ai = (quint8)newP;
				anyAirChg = true;
			}
		}
	}
	if(anyAirChg)
		m_AirChg.Set();
	m_AirLock.unlock();
	if(anyAirChg)
	{
		SetModified();
		InvalidateEnvironmentRecursive();	// So subordinate ships, vehicles, etc. know there is air now, or not.
	}
}

void DaSc::ToggleDoorLock(const DaPerson& person, unsigned int targetPart)
{
	// The person must be an officer, crew or troop to lock/unlock doors.
	if(!IsCrewOrTroop(person.Id()) && !InChainOfCommand(&person))
	{
		// Not authorized to lock/unlock doors.
		QString text(tr("Only officers, crew and troops can lock or unlock the door."));
		person.Thought(text);
		return;
	}

	bool doorChg = false;
	QString text;
	m_DesignLock.lockForWrite();
	AuDoorStateHash::iterator dsi = m_DoorStates.find(targetPart);
	if(dsi != m_DoorStates.end())
	{
		if((*dsi).IsHullDoor())
		{
			if((*dsi).IsLocked())
			{
				doorChg = (*dsi).SetLock(false);
				if(doorChg)
					text = tr("Hull door unlocked.");
				else
					text = tr("Hull door could not be unlocked in its current state.");
			}
			else
			{
				doorChg = (*dsi).SetLock(true);
				if(doorChg)
					text = tr("Hull door locked to outside entry.");
				else
					text = tr("Hull door could not be locked in its current state.");
			}
		}
		else
		{
			if((*dsi).IsLocked())
			{
				doorChg = (*dsi).SetLock(false);
				if(doorChg)
					text = tr("Door unlocked.");
				else
					text = tr("Door could not be unlocked in its current state.");
			}
			else
			{
				doorChg = (*dsi).SetLock(true);
				if(doorChg)
					text = tr("Door locked.");
				else
					text = tr("Door could not be locked in its current state.");
			}
		}
	}
	if(doorChg)
	{
		m_DoorChg.Set();
		SetModified();
	}
	m_DesignLock.unlock();
	if(!text.isEmpty())
		person.Thought(text);
}

quint32 DaSc::CryoBerths() const
{
	m_DesignLock.lockForRead();
	quint32 v = m_Design.CryoBerths();
	m_DesignLock.unlock();
	return v;
}

void DaSc::HullVolumeDistribution(quint32& hullVolume, quint32& armor, quint32& capacitor, quint32& cryoBerths, quint32& fuel,
		quint32& hold, quint32& lifeSupport, quint32& maneuverDrive, quint32& powerPlant, quint32& sensor, bool& hasSensor,
		quint32& shield, bool& hasShield, QMap<quint16, AuScReportSysVol>& ftlDrives, QMap<quint16, AuScReportSysVol>& bays,
		quint32& skeleton) const
{
	m_DesignLock.lockForRead();
	m_Design.HullVolumeDistribution(hullVolume, armor, capacitor, cryoBerths, fuel, hold, lifeSupport, maneuverDrive, powerPlant,
			sensor, hasSensor, shield, hasShield, ftlDrives, bays, skeleton);
	m_DesignLock.unlock();
}

int DaSc::EstimateRefit(const AuRefitShip& refit, double& estCost, quint32& estSeconds, AuRefitMaterials& estMaterials) const
{
	estCost = 0;
	estSeconds = 0;

	// Insure the total refit volume is the same as my hull volume.
	m_DesignLock.lockForRead();
	quint32 volm = m_Design.HullVolume();	// in cubic meters
	m_DesignLock.unlock();
	if(refit.Volume() != volm)
	{
		qWarning() << "error: DaSc::EstimateRefit" << Id() << Name() << "hullVol" << volm << "not equal to refitVol"
			<< refit.Volume();
		return 2;
	}

	int result;
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		// Build analysis of this ship with proposed changes.
		DeProcessComponentArray pcCur;
		DeProcessComponentArray pcRefit;

		m_DesignLock.lockForRead();

		bool anyChange = false;
		AuScReport refitDesign(m_Design);
		if(refitDesign.ArmorVolume() != refit.m_Armor)
		{
			refitDesign.RefitArmorVolume(refit.m_Armor, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.RefitBayVolumes(refit.m_Bays, refit.m_MinQ))
			anyChange = true;
		if(refitDesign.CapacitorVolume() != refit.m_Capacitor)
		{
			refitDesign.RefitCapacitorVolume(refit.m_Capacitor, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.CryoBerthVolume() != refit.m_CryoBerths)
		{
			refitDesign.RefitCryoBerthVolume(refit.m_CryoBerths, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.RefitFTLDriveVolumes(refit.m_FTLDrives, refit.m_MinQ))
			anyChange = true;
		if(refitDesign.FuelVolume() != refit.m_Fuel)
		{
			refitDesign.RefitFuelVolume(refit.m_Fuel, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.HoldVolume() != refit.m_Hold)
		{
			refitDesign.RefitHoldVolume(refit.m_Hold, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.LifeSupportVolume() != refit.m_LifeSupport)
		{
			refitDesign.RefitLifeSupportVolume(refit.m_LifeSupport, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.ManeuverDriveVolume() != refit.m_ManeuverDrive)
		{
			refitDesign.RefitManeuverDriveVolume(refit.m_ManeuverDrive, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.PowerPlantVolume() != refit.m_PowerPlant)
		{
			refitDesign.RefitPowerPlantVolume(refit.m_PowerPlant, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.HasSensor() && refitDesign.SensorVolume() != refit.m_Sensor)
		{
			refitDesign.RefitSensorVolume(refit.m_Sensor, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.HasShield() && refitDesign.ShieldVolume() != refit.m_Shield)
		{
			refitDesign.RefitShieldVolume(refit.m_Shield, refit.m_MinQ);
			anyChange = true;
		}
		if(refitDesign.SkeletonVolume() != refit.m_Skeleton)
		{
			refitDesign.RefitSkeletonVolume(refit.m_Skeleton, refit.m_MinQ);
			anyChange = true;
		}
		if(!anyChange)
		{
			m_DesignLock.unlock();
			UnlockDesign();
			return 3;
		}

		// List manufacturing components required for each configuration.
		m_Design.ListComponents(pcCur, scd->Parts());
		m_DesignLock.unlock();

		refitDesign.UpdateCalculatedValues();
		refitDesign.ListComponents(pcRefit, scd->Parts());

		// Build the list of materials needed to refit to the new configuration.
		for(DeProcessComponentArray::const_iterator ri = pcRefit.cbegin(); ri != pcRefit.cend(); ri++)
		{
			// We only care about materials that get consumed.
			if(!(*ri).IsRequired() || !(*ri).QtyConsumed())
				continue;	// Skip tools and other modifiers.
			// Locate the requirement in the current design to see if it changed.
			quint32 curNeed = 0;
			for(DeProcessComponentArray::const_iterator ci = pcCur.cbegin(); ci != pcCur.cend(); ci++)
			{
				if(!(*ci).IsRequired() || !(*ci).QtyConsumed())
					continue;	// Skip tools and other modifiers.
				if((*ci).CommodityId() != (*ri).CommodityId())
					continue;
				curNeed = (*ci).QtyConsumed();
				break;
			}

			quint32 diff;
			if(curNeed == 0)
			{
				// A new requirement was added.
				diff = (*ri).QtyConsumed();
				estMaterials.Add((*ri).CommodityId(), diff);	// Add new materials to the estimate.
			}
			else if((*ri).QtyConsumed() > curNeed)
			{
				// The required quantity increased.
				diff = (*ri).QtyConsumed() - curNeed;
				estMaterials.Add((*ri).CommodityId(), diff);	// Add new materials to the estimate.
			}
			else if((*ri).QtyConsumed() < curNeed)
			{
				// The required quantity decreased. We do not credit them for used materials that get removed.
				diff = curNeed - (*ri).QtyConsumed();
			}
			else
			{
				// The required quantity did not change.
				diff = 0;
			}
			estCost += diff;
			estSeconds += diff;
		}
		estCost += 1000;				// Minimum shop fee, in case the thing is tiny.
		estCost *= (refit.m_MinQ+1);	// Labor gets expensive at higher Q.
		estSeconds += 30;				// Minimum setup time.
		result = 1;						// We made it successfully to the end.
	}
	else
		result = 0;						// Design is not valid?
	UnlockDesign();
	return result;
}

void DaSc::Refit(const AuRefitShip& refit, quint32 seconds)
{
	m_DesignLock.lockForRead();
	quint32 volm = m_Design.HullVolume();	// in cubic meters
	m_DesignLock.unlock();
	if(refit.Volume() != volm)
		return;

	m_SystemsLock.lockForWrite();

	m_DesignLock.lockForWrite();
	m_Design.RefitArmorVolume(refit.m_Armor, refit.m_MinQ);
	m_Design.RefitBayVolumes(refit.m_Bays, refit.m_MinQ);
	m_Design.RefitCapacitorVolume(refit.m_Capacitor, refit.m_MinQ);
	m_Design.RefitCryoBerthVolume(refit.m_CryoBerths, refit.m_MinQ);
	m_Design.RefitFTLDriveVolumes(refit.m_FTLDrives, refit.m_MinQ);
	m_Design.RefitFuelVolume(refit.m_Fuel, refit.m_MinQ);
	m_Design.RefitHoldVolume(refit.m_Hold, refit.m_MinQ);
	m_Design.RefitLifeSupportVolume(refit.m_LifeSupport, refit.m_MinQ);
	m_Design.RefitManeuverDriveVolume(refit.m_ManeuverDrive, refit.m_MinQ);
	m_Design.RefitPowerPlantVolume(refit.m_PowerPlant, refit.m_MinQ);
	if(m_Design.HasSensor())
		m_Design.RefitSensorVolume(refit.m_Sensor, refit.m_MinQ);
	if(m_Design.HasShield())
		m_Design.RefitShieldVolume(refit.m_Shield, refit.m_MinQ);
	m_Design.RefitSkeletonVolume(refit.m_Skeleton, refit.m_MinQ);
	m_Design.UpdateCalculatedValues();
	m_DesignChg.Set();

	// Update the listing of all ship systems.
	GatherSystemsDataSystemsAndDesignLocked(QualityMade());
	m_DesignLock.unlock();

	UpdateSystemsHealthSystemsLocked();
	UpdateSystemsPowerSystemsLocked();
	m_SystemsLock.unlock();

	AddRepairTime(seconds);

	// User interface prevents minimum hold volume from going below that needed for current cargo.
	// User interface prevents minimum cryo berth volume from going below that needed for current cryo travellers.

	// Jettison excess fuel.
	if(FuelQuantity() > FuelCapacity())
		TakeFuel(FuelQuantity() - FuelCapacity());

	// Discharge excess power.
	if(CapacitorLevel() > CapacitorMax())
		DischargeCapacitor(CapacitorLevel() - CapacitorMax());

	SetModified();
}

STREAMLINE DaSc::HullStreamline() const
{
	m_DesignLock.lockForRead();
	STREAMLINE v = m_Design.Streamline();
	m_DesignLock.unlock();
	return v;
}

void DaSc::ListRoomNames(QStringList& roomNames) const
{
	const AuScDesign* scd = LockDesign();
	if(scd)
		scd->ListRoomNames(roomNames);
	UnlockDesign();
}

bool DaSc::RoomOrderLoc(const QString& roomName, AuPoint& orderLoc) const
{
	// Find a room with that exact name.
	bool foundIt;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		foundIt = scd->RandomRoomLocation(roomName, orderLoc, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		foundIt = false;
	UnlockDesign();
	m_DesignLock.unlock();
	return foundIt;
}

AuTime DaSc::StasisReadyTime() const
{
	// This ship must stay in stasis for 1/2 the number of seconds it would take to build it.
	DeManufacturingProcess mfgproc;
	mfgproc.InitSc(DesignId(), QualityMade());
	unsigned int mfgTimeSecs = mfgproc.SecondsEstimated();		// Number of seconds needed to manufacture the design.
	mfgTimeSecs /= 24;	// 1 hour per day of manufacture
	if(mfgTimeSecs < 10)
		mfgTimeSecs = 10;

	// The damage changed time is when we were put into stasis.
	m_DamageLock.lockForRead();
	AuTime whenReady = m_tmDamageChange.AddSecs(mfgTimeSecs);
	m_DamageLock.unlock();
	return whenReady;
}

bool DaSc::MoveLift(quint16 liftId, const DaAtomList& lockedAtoms, const QString& nuStop, quint16 doorId)
{
	AuLiftHash::iterator li = m_Lifts.find(liftId);
	Q_ASSERT(li != m_Lifts.end());
	QString oldStop(*li);
	if(oldStop == nuStop)
	{
		m_DesignLock.unlock();
		return false;
	}

	AuMatrix4x4 oldStopM, nuStopM;	// Used if a turbo lift is moved.
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		scd->TurboLiftStopMatrix(liftId, oldStop, oldStopM);
		scd->TurboLiftStopMatrix(liftId, nuStop, nuStopM);
	}
	UnlockDesign();

	// Do not automatically open the door at the destination if it is locked or if it is a hull door.
	bool doorOpened;
	if(!m_DoorStates.IsDoorLocked(doorId) && !m_DoorStates.IsHullDoor(doorId))
	{
		// Open the door while we have the design locked.
		doorOpened = m_DoorStates.OpenDoor(doorId);
		if(doorOpened)
			m_DoorChg.Set();
	}
	else
		doorOpened = false;
	*li = nuStop;				// Lift is now at the nu stop.
	m_LiftChg.Set();
	m_DoorChg.Set();			// In case any of the doors closed.
	m_DesignLock.unlock();
	SetModified();

	// Move the contents of the lift car from the old stop to the nu stop.
	AuMatrix4x4 tmove = nuStopM * oldStopM.Inverse();	// Matrix to move the contents from the old stop to the nu stop.
	for(DaAtomList::const_iterator it = lockedAtoms.cbegin(); it != lockedAtoms.cend(); it++)
	{
		// Move the atom.
		AuMatrix4x4 tnu = tmove * (*it)->Transform();
		(*it)->ForcePosStop(AuAtomPos(Id(), tnu));
		(*it)->InvalidateEnvironment();
		(*it)->SetModified();
	}
	return doorOpened;
}

void DaSc::TurnOnBridgeIndicators()
{
	QSet<QString> bridgeRoomNames;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		for(AuScReportStationHash::const_iterator si = m_Design.CrewStations().cbegin();
				si != m_Design.CrewStations().cend(); si++)
		{
			// We are only looking for helm and nav consoles.
			//if((*si).PartType() != AuScPart::Part_StationHelm &&
					//(*si).PartType() != AuScPart::Part_ConsoleHelm &&
					//(*si).PartType() != AuScPart::Part_StationNavigator &&
					//(*si).PartType() != AuScPart::Part_ConsoleNavigator)
			// We are only looking for helm consoles.
			if((*si).PartType() != AuScPart::Part_StationHelm &&
					(*si).PartType() != AuScPart::Part_ConsoleHelm)
				continue;
			// Locate the room that contains the station.
			AuScParts::const_iterator ri = scd->Parts().find((*si).RoomId());
			if(ri == scd->Parts().cend())
				continue;
			bridgeRoomNames.insert((*ri)->GroupName());
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	for(QSet<QString>::const_iterator bi = bridgeRoomNames.cbegin(); bi != bridgeRoomNames.cend(); bi++)
		SetBridgeVisible(*bi, true);
}

bool DaSc::EnginesRequireEngineer() const
{
	m_DesignLock.lockForRead();
	bool v = m_Design.EnginesRequireEngineer();
	m_DesignLock.unlock();
	return v;
}

void DaSc::RenderPaths(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari)
{
	AuScPathSegList psegs;
	const AuScDesign* scd = LockDesign();
	if(scd)
		scd->Parts().PathSegments(psegs);
	UnlockDesign();
	if(psegs.empty())
		return;
	QList<AuPointAlpha> nodes;
	QMap<AuPoint, QString> seginfo;
	AuPointArray segverts;
	QList<AuPointAlpha> liftnodes;
	QMap<AuPoint, QString> liftnodeinfo;
	for(AuScPathSegList::const_iterator si = psegs.cbegin(); si != psegs.cend(); si++)
	{
		if((*si).LiftId())
		{
			liftnodes.append((*si).Loc1());
			liftnodes.append((*si).Loc2());
			QMap<AuPoint, QString>::iterator li = liftnodeinfo.find((*si).Loc1());
			if(li == liftnodeinfo.end())
				liftnodeinfo.insert((*si).Loc1(), (*si).LiftStop2());	// Destination stop.
			else
				*li = *li + "\n" + (*si).LiftStop2();

			li = liftnodeinfo.find((*si).Loc2());
			if(li == liftnodeinfo.end())
				liftnodeinfo.insert((*si).Loc2(), (*si).LiftStop1());	// Destination stop.
			else
				*li = *li + "\n" + (*si).LiftStop1();
		}
		else
		{
			segverts.append((*si).Loc1());
			segverts.append((*si).Loc2());
			nodes.append((*si).Loc1());
			nodes.append((*si).Loc2());
			AuPoint cen(((*si).Loc1() + (*si).Loc2())/2);
			QString info;
			if((*si).IsLadder())
				info = QString("L");
			else
				info = QString("W");
			if((*si).Cost())
				info += QString("\nc:%1").arg((*si).Cost());
			if(!(*si).LiftStop1().isEmpty())
				info += QString("\ns1:%1").arg((*si).LiftStop1());
			if(!(*si).LiftStop2().isEmpty())
				info += QString("\ns2:%1").arg((*si).LiftStop2());
			seginfo.insert(cen, info);
		}
	}

	AuMatrix4x4 pmv = projection * model;
	{
		// Lines along paths.
		ari.re->GLSL().LineShader().Select();
		ari.re->GLSL().LineShader().SetProjectionModelView(pmv);

		AuGLVAO vao;

		// Make temporary VAO of the path segments. Display the VAO once and clean it up.
		ari.re->GLSL().LineShader().SetColor(QColor(0,255,255,128));
		vao.CreateForLines(*ari.re, segverts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		ari.re->GLSL().LineShader().Deselect();
	}
	ari.re->glDepthMask(GL_FALSE);
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	{
		// Sprites at nodes.
		if(!m_LightOmniTex)
			m_LightOmniTex = ari.re->CreateTextureFromBitmap(Id().GetID(), "LOmni", ":data/txLightOmni.png");

		ari.re->glEnable(GL_PROGRAM_POINT_SIZE);
		ari.re->GLSL().SpriteSizeShader().Select();
		ari.re->GLSL().SpriteSizeShader().SetProjectionModelView(pmv);
		ari.re->GLSL().SpriteSizeShader().SetTexture(m_LightOmniTex);

		AuGLVAO vao;

		// Make temporary VAO of the vertex locations. Display the VAO once and clean it up.
		ari.re->GLSL().SpriteSizeShader().SetSize(100);
		ari.re->GLSL().SpriteSizeShader().SetColor(QColor(0,255,255,128));
		vao.CreateForSprites(*ari.re, nodes, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// Lift
		ari.re->GLSL().SpriteSizeShader().SetSize(200);
		ari.re->GLSL().SpriteSizeShader().SetColor(QColor(255,255,0,128));
		vao.CreateForSprites(*ari.re, liftnodes, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		ari.re->glDisable(GL_PROGRAM_POINT_SIZE); 
		ari.re->GLSL().SpriteSizeShader().Deselect();
	}
	{
		// Text on segments.
		for(QMap<AuPoint, QString>::const_iterator si = seginfo.cbegin(); si != seginfo.cend(); si++)
			ari.RenderText(projection, model, si.key(), *si, QColor(0,255,255,128), Qt::AlignLeft|Qt::AlignVCenter);
		for(QMap<AuPoint, QString>::const_iterator si = liftnodeinfo.cbegin(); si != liftnodeinfo.cend(); si++)
			ari.RenderText(projection, model, si.key(), *si, QColor(255,255,0,128), Qt::AlignLeft|Qt::AlignVCenter);
	}
	ari.re->glDisable(GL_BLEND);
	ari.re->glDepthMask(GL_TRUE);
}

bool DaSc::RoomHasNavHUD(const QString& roomName) const
{
	// Look for a nav station in the room.
	bool roomHasNavHUD = false;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		for(AuScReportStationHash::const_iterator si = m_Design.CrewStations().cbegin(); si != m_Design.CrewStations().cend(); si++)
		{
			if((*si).PartType() != AuScPart::Part_StationNavigator && (*si).PartType() != AuScPart::Part_ConsoleNavigator)
				continue;
			AuScParts::const_iterator pi = scd->Parts().find((*si).RoomId());
			if(pi == scd->Parts().cend())
				continue;
			if((*pi)->GroupName() != roomName)
				continue;
			roomHasNavHUD = true;
			break;
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	return roomHasNavHUD;
}

void DaSc::AnimateRenderStates()
{
	if(m_RenderStateTime.isValid() && m_RenderStateTime.elapsed() < 50)
		return;
	m_RenderStateTime.restart();

	m_DesignLock.lockForWrite();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		// Door states.
		for(AuDoorStateHash::const_iterator di = m_DoorStates.cbegin(); di != m_DoorStates.cend(); di++)
		{
			AuScParts::const_iterator pi = scd->Parts().find(di.key());
			if(pi == scd->Parts().cend())
				continue;
			quint8 s = (*di).State();	// real state of the door
			QHash<quint16, quint8>::iterator si = m_DoorRenderState.find(di.key());	// last rendered state of the door
			if(si == m_DoorRenderState.end())
				m_DoorRenderState.insert(di.key(), s);
			else if(*si != s)
			{
				// Change the state of the render animation.
				quint8 ts;					// Comparison target state, for proper climb/descend.
				if(s == AuScDoor::LockedClosed || s == AuScDoor::StuckClosed)
					ts = AuScDoor::Closed;
				else if(s == AuScDoor::LockedOpen || s == AuScDoor::StuckOpen)
					ts = AuScDoor::Open;
				else
					ts = s;

				quint8 rs;					// Comparison render state, for proper climb/descend.
				if(*si == AuScDoor::LockedClosed || *si == AuScDoor::StuckClosed)
					rs = AuScDoor::Closed;
				else if(*si == AuScDoor::LockedOpen || *si == AuScDoor::StuckOpen)
					rs = AuScDoor::Open;
				else
					rs = *si;

				if(rs < ts)
				{
					// Render state needs to go up to reach target state.
					for(rs++; rs < ts; rs++)
					{
						// Check that the render state actually exists.
						if((*pi)->HasState(rs))
							break;
					}
				}
				else if(rs > ts)
				{
					// Render state needs to go down to reach target state.
					for(rs--; rs > ts; rs--)
					{
						// Check that the render state actually exists.
						if((*pi)->HasState(rs))
							break;
					}
				}

				if(rs == ts)
					rs = s;				// At the end, plug in the actual destination state, which could be locked or jammed.

				*si = rs;

				/* This code evenly walks through all states, regardless if there is a defined state, which results in even timing
				   at every door, regardless of the number of states.
				if(s == AuScDoor::LockedOpen || s == AuScDoor::StuckOpen)
				{
					if(*si == AuScDoor::LockedClosed || *si == AuScDoor::StuckClosed)
						*si = AuScDoor::Open10;
					else if(*si < AuScDoor::Open)
						(*si)++;
					if(*si >= AuScDoor::Open)
						*si = s;
				}
				else if(s == AuScDoor::LockedClosed || s == AuScDoor::StuckClosed)
				{
					if(*si == AuScDoor::LockedOpen || *si == AuScDoor::StuckOpen)
						*si = AuScDoor::Open90;
					else if(*si > AuScDoor::Closed)
						(*si)--;
					if(*si == AuScDoor::Closed)
						*si = s;
				}
				else
				{
					if(*si < s)
						(*si)++;
					else if(*si > s)
						(*si)--;
				}
				*/
			}
		}

		// Landing gear states.
		for(QHash<quint16, quint8>::const_iterator di = m_LandingGearStates.m_States.cbegin();
				di != m_LandingGearStates.m_States.cend(); di++)
		{
			AuScParts::const_iterator pi = scd->Parts().find(di.key());
			if(pi == scd->Parts().cend())
				continue;
			quint8 s = *di;	// real state of the gear
			QHash<quint16, quint8>::iterator si = m_LandingGearRenderState.find(di.key());	// last rendered state of the door
			if(si == m_LandingGearRenderState.end())
				m_LandingGearRenderState.insert(di.key(), s);
			else if(*si != s)
			{
				// Change the state of the render animation.
				quint8 ts;					// Comparison target state, for proper climb/descend.
				if(s == AuScLandingGear::StuckRetracted)
					ts = AuScLandingGear::Retracted;
				else if(s == AuScLandingGear::StuckExtended)
					ts = AuScLandingGear::Extended;
				else
					ts = s;

				quint8 rs;					// Comparison render state, for proper climb/descend.
				if(*si == AuScLandingGear::StuckRetracted)
					rs = AuScLandingGear::Retracted;
				else if(*si == AuScLandingGear::StuckExtended)
					rs = AuScLandingGear::Extended;
				else
					rs = *si;

				if(rs < ts)
				{
					// Render state needs to go up to reach target state.
					for(rs++; rs < ts; rs++)
					{
						// Check that the render state actually exists.
						if((*pi)->HasState(rs))
							break;
					}
				}
				else if(rs > ts)
				{
					// Render state needs to go down to reach target state.
					for(rs--; rs > ts; rs--)
					{
						// Check that the render state actually exists.
						if((*pi)->HasState(rs))
							break;
					}
				}

				if(rs == ts)
					rs = s;				// At the end, plug in the actual destination state, which could be locked or jammed.

				*si = rs;

				/* This code evenly walks through all states, regardless if there is a defined state, which results in even timing
				   at every door, regardless of the number of states.
				if(s == AuScLandingGear::StuckRetracted)
				{
					if(*si == AuScLandingGear::StuckExtended)
						*si = AuScLandingGear::Retracted10;
					else if(*si < AuScLandingGear::Retracted)
						(*si)++;
					if(*si >= AuScLandingGear::Retracted)
						*si = s;
				}
				else if(s == AuScLandingGear::StuckExtended)
				{
					if(*si == AuScLandingGear::StuckRetracted)
						*si = AuScLandingGear::Retracted90;
					else if(*si > AuScLandingGear::Extended)
						(*si)--;
					if(*si == AuScLandingGear::Extended)
						*si = s;
				}
				else
				{
					if(*si < s)
						(*si)++;
					else if(*si > s)
						(*si)--;
				}
				*/
			}
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
}

bool DaSc::Station(quint16 stationId, AuStation& station) const
{
	if(!stationId)
		return false;
	bool foundStation = false;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator pi = scd->Parts().find(stationId);
		if(pi != scd->Parts().cend())
		{
			AuPoint gravityN(0,0,1);
			quint16 stationRoomId = scd->Parts().RoomVoidContainingLoc((*pi)->Center(), .5);
			if(stationRoomId)
			{
				AuScParts::const_iterator ri = scd->Parts().find(stationRoomId);
				if(ri != scd->Parts().cend())
					gravityN = (*ri)->GravityUp(gravityN, AuMatrix4x4());
			}
			station = (*pi)->OperatorStation(gravityN);
			if(station.Type() != AuStation::TYPE_Count)
				foundStation = true;
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	return foundStation;
}

bool DaSc::Station(const DaCreature& who, AuStation& station) const
{
	quint16 stationId = who.StationId();
	if(!stationId)
		return false;
	bool foundStation = false;
	AuPoint gN(who.GravityNormal());
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
	{
		AuScParts::const_iterator pi = scd->Parts().find(stationId);
		if(pi != scd->Parts().cend())
		{
			// Console's operator position matrix relative to the ship.
			station = (*pi)->ClosestStation(who.Location(), gN);
			if(station.Type() != AuStation::TYPE_Count)
			{
				// Adjust for pose.
				// For example, a standing station is positioned on the floor.
				// We must adjust so our feet are there, instead of our cg.
				station.OffsetOpM(who.ConsolePoseOffset(station.Type(), station.OccPos(), who.CGHeightStanding(),
							who.TorsoThickness(), station.SeatIndex()));
				foundStation = true;
			}
		}
	}
	UnlockDesign();
	m_DesignLock.unlock();
	return foundStation;
}

void DaSc::ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const
{
	quint32 designId = DesignId();
	if(designId)
		scDesigns.insert(designId);
	DaVessel::ListDesignsRecursive(bdDesigns, scDesigns);
}

void DaSc::RenderSc(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
{
	// Suns
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			ari.re->GLSL().ScShader().SetSunEnabled(sunIndex, false);

	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(pass == PASS_Far || pass == PASS_Near)
		RenderScShader(&ari.re->GLSL().ScShader(), ari, model, sceneAtom);
	else
		RenderScShaderBlend(&ari.re->GLSL().ScShader(), ari, model, sceneAtom);

	// Restore shader states.
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			ari.re->GLSL().ScShader().SetSunEnabled(sunIndex, sun.m_Enabled);
		}
}

void DaSc::RenderScFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
{
	// Not at a world so shadow panel and horizon effects are not wanted.
	ari.re->GLSL().ScFogShader().SetFog(m_FogDensity, m_FogR, m_FogG, m_FogB);

	// Suns
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			ari.re->GLSL().ScFogShader().SetSunEnabled(sunIndex, false);

	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(pass == PASS_Far || pass == PASS_Near)
		RenderScShader(&ari.re->GLSL().ScFogShader(), ari, model, sceneAtom);
	else
		RenderScShaderBlend(&ari.re->GLSL().ScFogShader(), ari, model, sceneAtom);

	// Restore shader states.
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			ari.re->GLSL().ScFogShader().SetSunEnabled(sunIndex, sun.m_Enabled);
		}
}

void DaSc::RenderScGlobe(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
{
	// Globe over the horizon effect is desirable when at a globe.
	// Globe position in eye space is needed to calculate sun over-the-horizon effect.
	AuPoint globeP = rmod.Matrix(ParentId()).Translation();
	ari.re->GLSL().ScGlobeShader().SetGlobeLocation(globeP);

	// Suns
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			ari.re->GLSL().ScGlobeShader().SetSunEnabled(sunIndex, false);

	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(pass == PASS_Far || pass == PASS_Near)
		RenderScShader(&ari.re->GLSL().ScGlobeShader(), ari, model, sceneAtom);
	else
		RenderScShaderBlend(&ari.re->GLSL().ScGlobeShader(), ari, model, sceneAtom);

	// Restore shader states.
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			ari.re->GLSL().ScGlobeShader().SetSunEnabled(sunIndex, sun.m_Enabled);
		}
}

void DaSc::RenderScGlobeFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
{
	// Globe over the horizon effect is desirable when at a globe.
	// Globe position in eye space is needed to calculate sun over-the-horizon effect.
	AuPoint globeP = rmod.Matrix(ParentId()).Translation();
	ari.re->GLSL().ScGlobeFogShader().SetFog(m_FogDensity, m_FogR, m_FogG, m_FogB);
	ari.re->GLSL().ScGlobeFogShader().SetGlobeLocation(globeP);

	// Suns
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			ari.re->GLSL().ScGlobeFogShader().SetSunEnabled(sunIndex, false);

	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(pass == PASS_Far || pass == PASS_Near)
		RenderScShader(&ari.re->GLSL().ScGlobeFogShader(), ari, model, sceneAtom);
	else
		RenderScShaderBlend(&ari.re->GLSL().ScGlobeFogShader(), ari, model, sceneAtom);

	// Restore shader states.
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			ari.re->GLSL().ScGlobeFogShader().SetSunEnabled(sunIndex, sun.m_Enabled);
		}
}

void DaSc::RenderScRingworld(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
{
	const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(Parent());
	if(!rsworld)
		return;

	// Ringworld shadow panel shadow is desirable when at a ringworld.
	// Eye location is needed by ringworld shaders.
	const AuMatrix4x4& model = rmod.Matrix(Id());
	ari.re->GLSL().ScRingworldShader().SetEyeLocation(model.UnProject(ari.re->EyeLocation()));

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		ari.re->GLSL().ScRingworldShader().SetSunEnabled(sunIndex, sun.m_Enabled && !m_SunsBlocked);
		ari.re->GLSL().ScRingworldShader().SetSunShadowAngle(sunIndex, rsworld->ShadowPanelSunAngle(sun.m_ShadowAngle));
	}

	if(pass == PASS_Far || pass == PASS_Near)
		RenderScShader(&ari.re->GLSL().ScRingworldShader(), ari, model, sceneAtom);
	else
		RenderScShaderBlend(&ari.re->GLSL().ScRingworldShader(), ari, model, sceneAtom);

	// Restore shader states.
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			ari.re->GLSL().ScRingworldShader().SetSunEnabled(sunIndex, sun.m_Enabled);
		}
}

void DaSc::RenderScRingworldFog(AuAtomRenderInfo& ari, AuModelMatrixHash& rmod, const DaPerson* sceneAtom, RENDERPASS pass)
{
	const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(Parent());
	if(!rsworld)
		return;

	// Ringworld shadow panel shadow is desirable when at a ringworld.
	// Eye location is needed by ringworld shaders.
	const AuMatrix4x4& model = rmod.Matrix(Id());
	ari.re->GLSL().ScRingworldFogShader().SetEyeLocation(model.UnProject(ari.re->EyeLocation()));
	ari.re->GLSL().ScRingworldFogShader().SetFog(m_FogDensity, m_FogR, m_FogG, m_FogB);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		const AuGLSun& sun = ari.re->Sun(sunIndex);
		ari.re->GLSL().ScRingworldFogShader().SetSunEnabled(sunIndex, sun.m_Enabled && !m_SunsBlocked);
		ari.re->GLSL().ScRingworldFogShader().SetSunShadowAngle(sunIndex, rsworld->ShadowPanelSunAngle(sun.m_ShadowAngle));
	}

	if(pass == PASS_Far || pass == PASS_Near)
		RenderScShader(&ari.re->GLSL().ScRingworldFogShader(), ari, model, sceneAtom);
	else
		RenderScShaderBlend(&ari.re->GLSL().ScRingworldFogShader(), ari, model, sceneAtom);

	// Restore shader states.
	if(m_SunsBlocked)
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		{
			const AuGLSun& sun = ari.re->Sun(sunIndex);
			ari.re->GLSL().ScRingworldFogShader().SetSunEnabled(sunIndex, sun.m_Enabled);
		}
}

void DaSc::RenderScShader(AuGLSLScBase* shader, AuAtomRenderInfo& ari, const AuMatrix4x4& model, const DaPerson* sceneAtom)
{
	shader->SetModelView(model);
	shader->SetSurfaceTextures(m_SurfaceTex);
	shader->SetGlowDiscard(0,0,0, .01f);
	shader->SetVertAlpha(1);
	shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB);

	if(m_InsideVisible)
	{
		// Disable my own external light when rendering interior rooms.
		if(m_MyExtLight >= 0)
			shader->SetLightEnabled(m_MyExtLight, false);
		shader->SetAmbientLightUp(model.ProjectVector(0,0,1));

		// Render lifts at their current stops.
		if(!m_LiftOpaqueVAOs.empty())
			RenderLiftVAOs(m_LiftOpaqueVAOs, model, ari, *shader);

		// Render rooms according to their interior lighting.
		if(!m_RoomOpaqueVAOs.empty())
			RenderRoomVAOs(m_RoomOpaqueVAOs, *shader);
		if(!m_IntRoomOpaqueVAOs.empty())
			RenderInteriorRoomVAOs(m_IntRoomOpaqueVAOs, ari, *shader);

		if(m_IsAboard)
		{
			// Viewer is aboard so exterior light remains disabled while rendering my own windows and doors.
			// Render translucent windows; they use the texture palette that we have already configured.
			if(!m_WindowTranslucentVAOs.empty())
				RenderWindowVAOs(m_WindowTranslucentVAOs, *shader);

			// Render current states of doors.
			if(!m_DoorOpaqueVAOs.empty())
				RenderDoorVAOs(m_DoorOpaqueVAOs, *shader, sceneAtom->ShipRoomId(), m_AmbientR, m_AmbientG, m_AmbientB);
			if(!m_IntDoorOpaqueVAOs.empty())
				RenderInteriorDoorVAOs(m_IntDoorOpaqueVAOs, ari, *shader, sceneAtom->ShipRoomId(), m_AmbientR, m_AmbientG,
						m_AmbientB);
			else
				shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB); // Restore ambient lighting for exterior.

			// Restore exterior light after rendering doors and windows.
			if(m_MyExtLight >= 0)
				shader->SetLightEnabled(m_MyExtLight, true);
			shader->SetAmbientLightUp(m_AmbientUp);
		}
		else
		{
			// Viewer is not aboard so restore exterior light before rendering doors and windows.
			// Restore exterior light before rendering doors and windows.
			if(m_MyExtLight >= 0)
				shader->SetLightEnabled(m_MyExtLight, true);
			shader->SetAmbientLightUp(m_AmbientUp);

			// Render translucent windows; they use the texture palette that we have already configured.
			if(!m_WindowTranslucentVAOs.empty())
				RenderWindowVAOs(m_WindowTranslucentVAOs, *shader);

			// Render current states of doors.
			if(!m_DoorOpaqueVAOs.empty())
				RenderDoorVAOs(m_DoorOpaqueVAOs, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
			if(!m_IntDoorOpaqueVAOs.empty())
				RenderInteriorDoorVAOs(m_IntDoorOpaqueVAOs, ari, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
			else
				shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB); // Restore ambient lighting for exterior.
		}
	}
	else
	{
		// Inside is not visible.
		shader->SetAmbientLightUp(m_AmbientUp);

		// Render translucent windows; they use the texture palette that we have already configured.
		if(!m_WindowTranslucentVAOs.empty())
			RenderWindowVAOs(m_WindowTranslucentVAOs, *shader);

		// Render current states of doors.
		if(!m_DoorOpaqueVAOs.empty())
			RenderDoorVAOs(m_DoorOpaqueVAOs, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
		if(!m_IntDoorOpaqueVAOs.empty())
			RenderInteriorDoorVAOs(m_IntDoorOpaqueVAOs, ari, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
		else
			shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB); // Restore ambient lighting for exterior.
	}

	// Render non-window parts that use the texture palette that we have configured.
	if(!m_OpaqueVAOs.empty())
		RenderDesignVAOs(m_OpaqueVAOs, *shader);
	if(!m_LandingGearOpaqueVAOs.empty())
		RenderLandingGearVAOs(m_LandingGearOpaqueVAOs, *shader);

	// Render light lenses.
	if(!m_LensBeaconOpaqueVAOs.empty())
		RenderLensVAOs(m_LensBeaconOpaqueVAOs, m_LensBeaconGlows, *shader, m_RotatingBeacons);
	if(!m_LensHeadlightOpaqueVAOs.empty())
		RenderLensVAOs(m_LensHeadlightOpaqueVAOs, m_LensHeadlightGlows, *shader, IsHeadlightOn());
	if(!m_LensNavOpaqueVAOs.empty())
		RenderLensVAOs(m_LensNavOpaqueVAOs, m_LensNavGlows, *shader, m_NavLights);
	if(!m_LensStrobeOpaqueVAOs.empty())
		RenderLensVAOs(m_LensStrobeOpaqueVAOs, m_LensStrobeGlows, *shader, m_StrobesLit);

	// Render current states of turrets.
	if(!m_TurBaseOpaqueVAOs.empty())
		RenderTurretBaseVAOs(m_TurBaseOpaqueVAOs, model, *shader);
	if(!m_TurGunOpaqueVAOs.empty())
		RenderTurretGunVAOs(m_TurGunOpaqueVAOs, model, *shader);

	if(!m_DecalOpaqueVAOs.empty() ||
			!m_OwnOpaqueVAOs.empty() ||
			!m_OwnDecalOpaqueVAOs.empty())
	{
		// Depth test and polygon offset are changed to enable coplanar render of decals.
		// This includes own texture things, such as icons, screens and sketches.
		// glPolygonOffset(0.0f,1.0f); will unconditionally offset all polygons by one.
		// glPolygonOffset(1.0f,0.0f); will offset polys with a depth slope of one (rotated 45? into the screen) by one, polys
		// parallel to the eye plane it won't offset at all.
		ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
		ari.re->glPolygonOffset(-1.f, 0);
		ari.re->glDepthFunc(GL_LEQUAL);

		// Render decals that use the texture palette that we have configured.
		// Render decals that have their own textures.
		if(!m_DecalOpaqueVAOs.empty() || !m_OwnDecalOpaqueVAOs.empty())
		{
			// Decals are always blended.
			ari.re->glEnable(GL_BLEND);
			ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// Render decals that use the texture palette that we have configured.
			if(!m_DecalOpaqueVAOs.empty())
				RenderDesignVAOs(m_DecalOpaqueVAOs, *shader);
			// Render decals that have their own textures.
			if(!m_OwnDecalOpaqueVAOs.empty())
				RenderOwnTextureVAOs(m_OwnDecalOpaqueVAOs, *shader);
			ari.re->glDisable(GL_BLEND);
		}

		// Render parts that have their own textures.
		// So far, own texture things are icons, screens and sketches. We like to see them from the back too, mainly the
		// sketches.
		if(!m_OwnOpaqueVAOs.empty())
		{
			ari.re->glDisable(GL_CULL_FACE);
			RenderOwnTextureVAOs(m_OwnOpaqueVAOs, *shader);
			ari.re->glEnable(GL_CULL_FACE);
		}

		ari.re->glDepthFunc(GL_LESS);
		ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void DaSc::RenderScShaderBlend(AuGLSLScBase* shader, AuAtomRenderInfo& ari, const AuMatrix4x4& model,
		const DaPerson* sceneAtom)
{
	shader->SetModelView(model);
	shader->SetSurfaceTextures(m_SurfaceTex);
	shader->SetGlowDiscard(0,0,0, .01f);
	shader->SetVertAlpha(1);
	shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB);

	if(m_InsideVisible)
	{
		// Turn on blending for transparency.
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Disable my own external light when rendering interior rooms.
		if(m_MyExtLight >= 0)
			shader->SetLightEnabled(m_MyExtLight, false);
		shader->SetAmbientLightUp(model.ProjectVector(0,0,1));

		// Render lifts at their current stops.
		if(!m_LiftTransparentVAOs.empty())
			RenderLiftVAOs(m_LiftTransparentVAOs, model, ari, *shader);

		// Render rooms according to their interior lighting.
		if(!m_RoomTransparentVAOs.empty())
			RenderRoomVAOs(m_RoomTransparentVAOs, *shader);
		if(!m_IntRoomTransparentVAOs.empty())
			RenderInteriorRoomVAOs(m_IntRoomTransparentVAOs, ari, *shader);

		if(m_IsAboard)
		{
			// Viewer is aboard so exterior light remains disabled while rendering my own windows and doors.
			// Render transparent windows; they use the texture palette that we have already configured.
			if(!m_WindowTransparentVAOs.empty())
				RenderWindowVAOs(m_WindowTransparentVAOs, *shader);

			// Render current states of doors.
			if(!m_DoorTransparentVAOs.empty())
				RenderDoorVAOs(m_DoorTransparentVAOs, *shader, sceneAtom->ShipRoomId(), m_AmbientR, m_AmbientG, m_AmbientB);
			if(!m_IntDoorTransparentVAOs.empty())
				RenderInteriorDoorVAOs(m_IntDoorTransparentVAOs, ari, *shader, sceneAtom->ShipRoomId(), m_AmbientR, m_AmbientG,
					m_AmbientB);
			else
				shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB); // Restore ambient lighting for exterior.

			// Restore exterior light after rendering doors and windows.
			if(m_MyExtLight >= 0)
				shader->SetLightEnabled(m_MyExtLight, true);
			shader->SetAmbientLightUp(m_AmbientUp);
		}
		else
		{
			// Viewer is not aboard so restore exterior light before rendering doors and windows.
			// Restore exterior light before rendering doors and windows.
			if(m_MyExtLight >= 0)
				shader->SetLightEnabled(m_MyExtLight, true);
			shader->SetAmbientLightUp(m_AmbientUp);

			// Render transparent windows; they use the texture palette that we have already configured.
			if(!m_WindowTransparentVAOs.empty())
				RenderWindowVAOs(m_WindowTransparentVAOs, *shader);

			// Render current states of doors.
			if(!m_DoorTransparentVAOs.empty())
				RenderDoorVAOs(m_DoorTransparentVAOs, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
			if(!m_IntDoorTransparentVAOs.empty())
				RenderInteriorDoorVAOs(m_IntDoorTransparentVAOs, ari, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
			else
				shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB); // Restore ambient lighting for exterior.
		}
	}
	else
	{
		// Inside is not visible.
		shader->SetAmbientLightUp(m_AmbientUp);

		// Inside is not visible so render windows opaque; they use the texture palette that we have already configured.
		if(!m_WindowTransparentVAOs.empty())
			RenderWindowVAOs(m_WindowTransparentVAOs, *shader);

		// Turn on blending for transparency.
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Render current states of doors.
		if(!m_DoorTransparentVAOs.empty())
			RenderDoorVAOs(m_DoorTransparentVAOs, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
		if(!m_IntDoorTransparentVAOs.empty())
			RenderInteriorDoorVAOs(m_IntDoorTransparentVAOs, ari, *shader, 0, m_AmbientR, m_AmbientG, m_AmbientB);
		else
			shader->SetAmbientLight(m_AmbientR, m_AmbientG, m_AmbientB); // Restore ambient lighting for exterior.
	}

	// Render non-window parts that use the texture palette that we have configured.
	if(!m_TransparentVAOs.empty())
		RenderDesignVAOs(m_TransparentVAOs, *shader);
	if(!m_LandingGearTransparentVAOs.empty())
		RenderLandingGearVAOs(m_LandingGearTransparentVAOs, *shader);

	// Render light lenses.
	if(!m_LensBeaconTransparentVAOs.empty())
		RenderLensVAOs(m_LensBeaconTransparentVAOs, m_LensBeaconGlows, *shader, m_RotatingBeacons);
	if(!m_LensHeadlightTransparentVAOs.empty())
		RenderLensVAOs(m_LensHeadlightTransparentVAOs, m_LensHeadlightGlows, *shader, IsHeadlightOn());
	if(!m_LensNavTransparentVAOs.empty())
		RenderLensVAOs(m_LensNavTransparentVAOs, m_LensNavGlows, *shader, m_NavLights);
	if(!m_LensStrobeTransparentVAOs.empty())
		RenderLensVAOs(m_LensStrobeTransparentVAOs, m_LensStrobeGlows, *shader, m_StrobesLit);

	// Render current states of turrets.
	if(!m_TurBaseTransparentVAOs.empty())
		RenderTurretBaseVAOs(m_TurBaseTransparentVAOs, model, *shader);
	if(!m_TurGunTransparentVAOs.empty())
		RenderTurretGunVAOs(m_TurGunTransparentVAOs, model, *shader);

	if(!m_DecalTransparentVAOs.empty() ||
			!m_OwnTransparentVAOs.empty() ||
			!m_OwnDecalTransparentVAOs.empty())
	{
		// Depth test and polygon offset are changed to enable coplanar render of decals.
		// This includes own texture things, such as icons, screens and sketches.
		ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
		ari.re->glPolygonOffset(-1.f, 0);
		ari.re->glDepthFunc(GL_LEQUAL);

		// Render decals that use the texture palette that we have configured.
		// Decals are always blended; it's already enabled in transparent pass.
		if(!m_DecalTransparentVAOs.empty())
			RenderDesignVAOs(m_DecalTransparentVAOs, *shader);

		// Render decals that have their own textures.
		if(!m_OwnDecalTransparentVAOs.empty())
			RenderOwnTextureVAOs(m_OwnDecalTransparentVAOs, *shader);

		// Render parts that have their own textures.
		// So far, own texture things are icons, screens and sketches. We like to see them from the back too, mainly the
		// sketches.
		if(!m_OwnTransparentVAOs.empty())
		{
			ari.re->glDisable(GL_CULL_FACE);
			RenderOwnTextureVAOs(m_OwnTransparentVAOs, *shader);
			ari.re->glEnable(GL_CULL_FACE);
		}

		ari.re->glDepthFunc(GL_LESS);
		ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
	}
	ari.re->glDisable(GL_BLEND);
}

quint64 DaSc::MemoryUsage() const
{
	quint64 mem = DaVessel::MemoryUsage();
	mem += sizeof(m_AlertCondition);
	mem += sizeof(m_DesignCacheValid);
	mem += sizeof(m_NavLights);
	mem += sizeof(m_New);
	mem += sizeof(m_RotatingBeacons);
	mem += sizeof(m_Strobes);
	mem += sizeof(m_Ticked);
	mem += sizeof(m_Tocked);

	mem += sizeof(m_DesignLock);
	mem += sizeof(m_DesignChg);
	mem += m_Design.MemoryUsage();
	mem += sizeof(m_DoorChg);
	mem += (sizeof(quint16) + sizeof(AuTime)) * m_DoorOpenTimes.capacity();
	mem += m_DoorStates.MemoryUsage();
	mem += sizeof(m_LandingGearChg);
	mem += m_LandingGearStates.MemoryUsage();
	mem += m_LiftCalls.MemoryUsage();
	mem += sizeof(m_LiftChg);
	mem += m_Lifts.MemoryUsage();
	mem += sizeof(m_TargetLoc);
	mem += sizeof(m_TargetLocValid);

	mem += sizeof(m_AirLock);
	mem += sizeof(m_AirChg);
	mem += m_AirPressures.MemoryUsage();

	mem += sizeof(m_FTLLock);
	mem += sizeof(m_FTLChg);
	mem += m_FTLDrives.MemoryUsage();

	mem += sizeof(m_LandingSystemLock);
	mem += sizeof(m_LandingSystemChg);
	mem += m_LandingSystemStates.MemoryUsage();

	mem += sizeof(m_LauncherLock);
	mem += sizeof(m_LauncherChg);
	mem += m_LauncherStates.MemoryUsage();

	mem += sizeof(m_RoomLightLock);
	mem += sizeof(m_RoomLightChg);
	mem += m_RoomLightStates.MemoryUsage();

	mem += sizeof(m_TRoomLock);
	mem += sizeof(m_TRoomChg);
	mem += m_TRoomStates.MemoryUsage();

	mem += sizeof(m_TurretPosLock);
	mem += sizeof(m_TurretPosChg);
	mem += m_TurretPos.MemoryUsage();

	mem += sizeof(m_AudioShip);
	mem += sizeof(m_DoorExt);
	mem += sizeof(m_DoorExtLoc);
	mem += sizeof(m_DoorInt1);
	mem += sizeof(m_DoorInt1Loc);
	mem += sizeof(m_DoorInt2);
	mem += sizeof(m_DoorInt2Loc);
	mem += sizeof(m_DoorLift1);
	mem += sizeof(m_DoorLift1Loc);
	mem += sizeof(m_DoorLift2);
	mem += sizeof(m_DoorLift2Loc);
	mem += sizeof(m_Siren);

	for(QHash<quint16, QString>::const_iterator dri = m_DoorRooms.cbegin(); dri != m_DoorRooms.cend(); dri++)
		mem += sizeof(quint16) + StringMemoryUsage((*dri));
	mem += sizeof(m_DoorRoomsValid);
																// is guarded by m_DesignLock and only contains hull doors.
	for(QHash<quint16, QString>::const_iterator dri = m_RoomNames.cbegin(); dri != m_RoomNames.cend(); dri++)
		mem += sizeof(quint16) + StringMemoryUsage((*dri));
	mem += sizeof(m_SunlitRooms);
	mem += sizeof(m_SurfaceTex);
	mem += sizeof(m_VAOsCreated);
	for(QHash<quint16, QString>::const_iterator dri = m_WindowRooms.cbegin(); dri != m_WindowRooms.cend(); dri++)
		mem += sizeof(quint16) + StringMemoryUsage((*dri));
	mem += m_WindowLightStates.MemoryUsage();

	mem += sizeof(m_DamageTex);
	mem += sizeof(m_EffectCur);
	mem += sizeof(m_EffectFrame);
	mem += sizeof(m_EffectRandFrame);
	mem += sizeof(m_EffectTex);
																// Extras for low power trans, grav drive, ftl drive and ftl hull.
	mem += sizeof(m_HullEffectAppl);
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_OpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_TransparentVAOs.capacity();

	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_OwnOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(GLuint)) * m_OwnTextures.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_OwnTransparentVAOs.capacity();

	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_DecalOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_DecalTransparentVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_OwnDecalOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_OwnDecalTransparentVAOs.capacity();

	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_IntRoomOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_IntRoomTransparentVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_RoomOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_RoomTransparentVAOs.capacity();

	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_WindowTranslucentVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_WindowTransparentVAOs.capacity();

	mem += sizeof(m_RenderStateTime);

	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_DoorOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(quint8)) * m_DoorRenderState.capacity();
	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_DoorTransparentVAOs.capacity();
	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_IntDoorOpaqueVAOs.capacity();
	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_IntDoorTransparentVAOs.capacity();
	mem += sizeof(quint32) * m_DoorVAOsMade.capacity();

	mem += (sizeof(quint16) + sizeof(quint8)) * m_LandingGearRenderState.capacity();
	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_LandingGearOpaqueVAOs.capacity();
	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_LandingGearTransparentVAOs.capacity();
	mem += sizeof(quint32) * m_LandingGearVAOsMade.capacity();

	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LiftOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LiftTransparentVAOs.capacity();

	mem += sizeof(m_HColor);
	mem += sizeof(m_HCosInnerCone);
	mem += sizeof(m_HCosOuterCone);
	mem += sizeof(m_HDir);
	mem += sizeof(m_HkL);
	mem += sizeof(m_HkQ);
	mem += sizeof(m_HPos);
	mem += sizeof(m_LensBeaconAngle);
	mem += (sizeof(quint16) + sizeof(AuPoint)) * m_LensBeaconAxes.capacity();
	mem += (sizeof(quint16) + sizeof(AuLensGlow)) * m_LensBeaconGlows.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensBeaconOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensBeaconTransparentVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuLensGlow)) * m_LensHeadlightGlows.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensHeadlightOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensHeadlightTransparentVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuLensGlow)) * m_LensNavGlows.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensNavOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensNavTransparentVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuLensGlow)) * m_LensStrobeGlows.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensStrobeOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_LensStrobeTransparentVAOs.capacity();
	mem += sizeof(m_LightOmniTex);
	mem += sizeof(m_LightSpotVAO);
	mem += sizeof(m_LightSpotTex);
	mem += sizeof(m_StrobesLit);

	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_TurBaseOpaqueVAOs.capacity();
	mem += (sizeof(quint16) + sizeof(AuScVAO)) * m_TurBaseTransparentVAOs.capacity();
	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_TurGunOpaqueVAOs.capacity();
	mem += (sizeof(quint32) + sizeof(AuScVAO)) * m_TurGunTransparentVAOs.capacity();
	mem += sizeof(quint32) * m_TurGunVAOsMade.capacity();

	mem += m_CmdEmpty.MemoryUsage();
	mem += m_EngineEmpty.MemoryUsage();
	mem += m_FireEmpty.MemoryUsage();
	mem += m_NavEmpty.MemoryUsage();
	mem += m_PilotEmpty.MemoryUsage();
	mem += m_PowerEmpty.MemoryUsage();
	mem += m_SensorEmpty.MemoryUsage();
	mem += m_ShieldEmpty.MemoryUsage();
	mem += m_SurgeryEmpty.MemoryUsage();
	mem += m_TransEmpty.MemoryUsage();

	mem += sizeof(m_ContrailColor);
	mem += sizeof(PsEngineTrail*) * m_Contrails.size();
	mem += sizeof(m_ContrailsValid);

	mem += sizeof(m_WeaponVAO);

	mem += sizeof(m_LandingSystem);
	mem += sizeof(m_LaunchSystem);

	mem += sizeof(m_AlertGreen);
	mem += sizeof(m_AlertRedBright);
	mem += sizeof(m_AlertRedDim);
	mem += sizeof(m_AlertStatus);
	mem += sizeof(m_AlertYellowBright);
	mem += sizeof(m_AlertYellowDim);
	mem += sizeof(m_BerthNameDecals);
	for(QHash<quint16, QString>::const_iterator bi = m_BerthNames.cbegin(); bi != m_BerthNames.cend(); bi++)
	{
		mem += sizeof(bi.key());
		mem += StringMemoryUsage(*bi);
	}
	mem += sizeof(m_CaptName);
	mem += sizeof(m_CaptNameDecals);
	mem += sizeof(m_EmpireFlagDecals);
	mem += sizeof(m_EmpireNameDecals);
	mem += sizeof(m_FlagEmpire);
	mem += sizeof(m_LifeSupportStatus);
	mem += sizeof(m_LifeSupportStatusTex);
	mem += sizeof(m_MissionStatusETESecs);
	mem += sizeof(m_MissionStatusTex);
	mem += StringMemoryUsage(m_ShipName);
	mem += sizeof(m_ShipNameDecals);
	mem += sizeof(m_WorldMaps);
	mem += sizeof(m_WorldMapTex);

	mem += sizeof(m_AmbientR);
	mem += sizeof(m_AmbientG);
	mem += sizeof(m_AmbientB);
	mem += sizeof(m_AmbientUp);
	mem += sizeof(m_FogDensity);
	mem += sizeof(m_FogR);
	mem += sizeof(m_FogG);
	mem += sizeof(m_FogB);
	mem += sizeof(m_InsideVisible);
	mem += sizeof(m_IsAboard);
	mem += sizeof(m_MyExtLight);
	mem += sizeof(m_SunsBlocked);
	return mem;
}

void DaSc::LockLiftContents(quint16 liftId, DaAtomList& atomsInLift)
{
	AuIdList subIds;
	ListSubatomIds(subIds);
	for(AuIdList::const_iterator ii = subIds.cbegin(); ii != subIds.cend(); ii++)
	{
		DaAtom* atom = FindSubatomId(*ii);
		if(!atom)
			continue;
		if(!atom->DoesQueryEnvironment())
			continue;						// Designer?
		quint16 roomId = atom->ShipRoomId();
		if(roomId != liftId)
			continue;
		atomsInLift.append(atom);	// The atom will be moved in the lift.
	}
}

QString DaSc::SpotDescription(quint32 spotterEmpire) const
{
	QString name(Name());

	QString empire;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(EmpireId(), empire);
	POLITICAL_STANCE stance = gd.EmpirePoliticalStance(spotterEmpire, EmpireId());
	g_Au.UnlockData();

	switch(stance)
	{
	case POL_Friend:
		empire = tr("Friendly %1").arg(empire);
		break;
	case POL_Vassal:
		empire = tr("Overlord %1").arg(empire);
		break;
	case POL_Neutral:
		empire = tr("Neutral %1").arg(empire);
		break;
	case POL_Enemy:
		empire = tr("Enemy %1").arg(empire);
		break;
	case POL_Empire:
		break;
	}

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	QString hpdesc;
	unsigned int maxHits = DesignHitsStructural();
	unsigned int dmgHits = HitsDamageStructural();
	unsigned int curHits;
	if(dmgHits < maxHits)
		curHits = maxHits - dmgHits;
	else
		curHits = 0;
	if(curHits == maxHits)
		hpdesc = tr(" %L1 hp").arg(maxHits);
	else
		hpdesc = tr(" %L1/%L2 hp").arg(curHits).arg(maxHits);

	QString desc;
	if(name == DesignName())
	{
		// The ship has the name of the class type. We can omit the 'name class' part of the description.
		if(empire.isEmpty())
			desc = tr("%1 %2, %6").arg(name).arg(Description())
				.arg(hpdesc);
		else
			desc = tr("%1 %2, %3, %6").arg(empire).arg(name).arg(Description())
			   .arg(hpdesc);
	}
	else
	{
		// The description includes the design class name.
		if(empire.isEmpty())
			desc = tr("%1, %2 class %3, %6").arg(name).arg(DesignName()).arg(Description())
			   .arg(hpdesc);
		else
			desc = tr("%1 %2, %3 class %4, %6").arg(empire).arg(name).arg(DesignName()).arg(Description())
				.arg(hpdesc);
	}

	if(IsDormant() && IsMoored())
		desc += tr(", Hangared by %1")
			.arg(CaptainName());

	return desc;
}

void DaSc::SetSystemQuality(const AuSysId& sysId, quint8 quality)
{
	DaVessel::SetSystemQuality(sysId, quality);

	// Establish new system quality.
	m_DesignLock.lockForWrite();
	if(m_Design.SetSystemQuality(sysId, quality))
		m_DesignChg.Set();
	m_DesignLock.unlock();
}

void DaSc::DowngradeSystemModules(quint8 repairQ, const QSet<AuSysId>& systemsRepaired)
{
	if(systemsRepaired.empty())
		return;

	DaVessel::DowngradeSystemModules(repairQ, systemsRepaired);

	m_DesignLock.lockForWrite();
	if(m_Design.DowngradeSystemModules(repairQ, systemsRepaired))
		m_DesignChg.Set();
	m_DesignLock.unlock();
}

quint16 DaSc::TransporterWhich(const AuSensorBlip& blip) const
{
	return blip.TransporterRoomNumber();
}

quint16 DaSc::RemoteCameraNext(quint16 currentCamera) const
{
	quint16 camId;
	const AuScDesign* scd = LockDesign();
	if(scd)
		camId = scd->RemoteCameraNext(currentCamera);
	else
		camId = 0;
	UnlockDesign();
	return camId;
}

quint16 DaSc::RemoteCameraPrevious(quint16 currentCamera) const
{
	quint16 camId;
	const AuScDesign* scd = LockDesign();
	if(scd)
		camId = scd->RemoteCameraPrevious(currentCamera);
	else
		camId = 0;
	UnlockDesign();
	return camId;
}

AuPoint DaSc::RemoteCameraLocation(quint16 cameraId) const
{
	AuPoint camLoc;
	const AuScDesign* scd = LockDesign();
	if(scd)
		camLoc = scd->RemoteCameraLocation(cameraId);
	UnlockDesign();
	return camLoc;
}

bool DaSc::RemoteCameraVerify(quint16 cameraId) const
{
	if(!cameraId)
		return false;
	bool v = false;
	const AuScDesign* scd = LockDesign();
	if(scd)
		v = scd->RemoteCameraVerify(cameraId);
	UnlockDesign();
	return v;
}

void DaSc::CitizenPosts(AuScPartSet& posts) const
{
	m_DesignLock.lockForRead();
	posts = m_Design.CitizenPosts();
	m_DesignLock.unlock();
}

void DaSc::LivestockPosts(AuScPartSet& posts) const
{
	m_DesignLock.lockForRead();
	posts = m_Design.LivestockPosts();
	m_DesignLock.unlock();
}

void DaSc::TroopPosts(AuScPartSet& posts) const
{
	m_DesignLock.lockForRead();
	posts = m_Design.GuardPosts();
	m_DesignLock.unlock();
}

LANDING_ORIENT DaSc::LandingOrientation() const
{
	LANDING_ORIENT lori;
	m_DesignLock.lockForRead();
	lori = m_Design.LandingOrientation();
	m_DesignLock.unlock();
	return lori;
}

bool DaSc::IsOnLadderTo(const AuPoint& fromLoc, const AuPoint& ladderPathLoc) const
{
	// Find a room with that exact name.
	bool foundIt;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		foundIt = scd->IsOnLadderTo(fromLoc, ladderPathLoc, m_DoorStates, m_LandingGearStates);
	else
		foundIt = false;
	UnlockDesign();
	m_DesignLock.unlock();
	return foundIt;
}

bool DaSc::CanGrabLadderAt(const DaAtom& who, const AuPoint& ladderPathLoc, AuMatrix4x4& mLadder)
{
	// Find closest ladder path to 'who' with an end point at 'ladderPathLoc'.
	bool foundLadder;
	m_DesignLock.lockForRead();
	const AuScDesign* scd = LockDesign();
	if(scd)
		foundLadder = scd->FindLadderToGrabAt(who.Location(), ladderPathLoc, mLadder, m_DoorStates, m_LandingGearStates, m_Lifts);
	else
		foundLadder = false;
	UnlockDesign();
	m_DesignLock.unlock();
	return foundLadder;
}

quint32 DaSc::HullVolume() const
{
	m_DesignLock.lockForRead();
	quint32 volm = m_Design.HullVolume();	// in cubic meters
	m_DesignLock.unlock();
	return volm;
}

QList<quint16> DaSc::TransporterRoomIds() const
{
	m_TRoomLock.lockForRead();
	QList<quint16> ids(m_TRoomStates.keys());
	m_TRoomLock.unlock();
	return ids;
}

void DaSc::SystemIds(QSet<AuSysId>& sysIds, bool structural) const
{
	m_DesignLock.lockForRead();
	m_Design.SystemIds(sysIds, structural);
	m_DesignLock.unlock();
}

quint32 DaSc::SystemHits(const AuSysId& sysId) const
{
	m_DesignLock.lockForRead();
	quint32 v = m_Design.SystemHits(sysId);
	m_DesignLock.unlock();
	return v;
}

AuSysId DaSc::SelectNonstructuralSystemToDamage() const
{
	// Create a list of systems that can be damaged.
	quint32 hitsTotal = 0;	// Total hits remaining between my systems.
	QSet<AuSysId> sysIds;
	SystemIds(sysIds, false);
	QHash<AuSysId, quint32> canDamage;
	m_DamageLock.lockForRead();
	for(QSet<AuSysId>::const_iterator si = sysIds.cbegin(); si != sysIds.cend(); si++)
	{
		quint32 dmg = m_Damage.HitsDamageAddParts(*si);
		quint32 maxhp = SystemHits(*si);
		if(dmg >= maxhp)
			continue;		// System is destroyed.
		quint32 hitsLeft = maxhp - dmg;
		canDamage.insert(*si, hitsLeft);
		hitsTotal += hitsLeft;
	}
	m_DamageLock.unlock();
	if(!hitsTotal)
		return SYS_Count;

	// Choose a system at random to damage, based on hit points distribution.
	quint32 canIdx = g_Au.Dice().Roll(1, hitsTotal, -1);
	quint32 curTotal = 0;
	for(QHash<AuSysId, quint32>::const_iterator di = canDamage.cbegin(); di != canDamage.cend(); di++)
	{
		curTotal += *di;
		if(canIdx < curTotal)
			return di.key();
	}
	return SYS_Count;	// This should never happen.
}

AuSysId DaSc::SelectStructuralSystemToDamage() const
{
	// Create a list of systems that can be damaged.
	quint32 hitsTotal = 0;	// Total hits remaining between my systems.
	QSet<AuSysId> sysIds;
	SystemIds(sysIds, true);
	QHash<AuSysId, quint32> canDamage;
	m_DamageLock.lockForRead();
	for(QSet<AuSysId>::const_iterator si = sysIds.cbegin(); si != sysIds.cend(); si++)
	{
		quint32 dmg = m_Damage.HitsDamageAddParts(*si);
		quint32 maxhp = SystemHits(*si);
		if(dmg >= maxhp)
			continue;		// System is destroyed.
		quint32 hitsLeft = maxhp - dmg;
		canDamage.insert(*si, hitsLeft);
		hitsTotal += hitsLeft;
	}
	m_DamageLock.unlock();
	if(!hitsTotal)
		return SYS_Count;

	// Choose a system at random to damage, based on hit points distribution.
	quint32 canIdx = g_Au.Dice().Roll(1, hitsTotal, -1);
	quint32 curTotal = 0;
	for(QHash<AuSysId, quint32>::const_iterator di = canDamage.cbegin(); di != canDamage.cend(); di++)
	{
		curTotal += *di;
		if(canIdx < curTotal)
			return di.key();
	}
	return SYS_Count;	// This should never happen.
}

void DaSc::DamageReportHp(AuDamage& damage) const
{
	// Create a list of all systems.
	QSet<AuSysId> sysIds;
	SystemIds(sysIds, true);
	SystemIds(sysIds, false);
	for(QSet<AuSysId>::const_iterator si = sysIds.cbegin(); si != sysIds.cend(); si++)
	{
		AuRepairParts parts;
		m_DamageLock.lockForRead();
		quint32 hitsDmg = m_Damage.HitsDamageNoParts(*si);
		m_Damage.PartsDamage(*si, parts);
		m_DamageLock.unlock();
		if(!hitsDmg && parts.empty())
			damage.AddHitsDamage(*si, 0);	// System is undamaged.
		else
		{
			damage.AddHitsDamage(*si, hitsDmg);
			damage.AddPartsDamage(*si, parts);
		}
	}
}

void DaSc::DamageReportPercent(AuDamage& damage, int structEquip) const
{
	// Create a list of all systems.
	QSet<AuSysId> sysIds;
	if(structEquip & 0x01)
		SystemIds(sysIds, true);
	if(structEquip & 0x02)
		SystemIds(sysIds, false);
	for(QSet<AuSysId>::const_iterator si = sysIds.cbegin(); si != sysIds.cend(); si++)
	{
		m_DamageLock.lockForRead();
		quint32 hitsDmg = m_Damage.HitsDamageAddParts(*si);
		m_DamageLock.unlock();
		if(!hitsDmg)
		{
			if((structEquip & 0x04) == 0)
				damage.AddHitsDamage(*si, 0);	// System is undamaged.
		}
		else
		{
			// Calculate percentage of damage.
			unsigned int percDmg;
			quint32 syshp = SystemHits(*si);
			if(hitsDmg >= syshp || syshp == 0)
				percDmg = 100;
			else
			{
				percDmg = Percent(hitsDmg, syshp);
				if(!percDmg && hitsDmg)
					percDmg = 1;	// Slightly more than zero percent remaining.
				else if(percDmg == 100)
					percDmg = 99;	// Slightly less than 100 percent remaining.
			}
			damage.AddHitsDamage(*si, percDmg);
		}
	}
}

quint32 DaSc::SystemModuleCount(const AuSysId& sysId) const
{
	m_DesignLock.lockForRead();
	quint32 c = m_Design.SystemModuleCount(sysId);
	m_DesignLock.unlock();
	return c;
}

void DaSc::ToggleDoorState(const DaSentient& who, unsigned int doorIndex)
{
	// Determine if the person can open the door due to locking, access policy, etc. Ignores jamming.
	if(CanOpenDoor(who, doorIndex))
	{
		// First attempt at a jammed door unjams it.
		if(IsDoorJammed(doorIndex))
		{
			UnjamDoor(doorIndex);

			QString text(tr("Door unjammed."));
			who.Thought(text);
		}
		else if(IsDoorOpen(doorIndex))
			CloseDoor(doorIndex);
		else
		{
			m_DesignLock.lockForRead();
			bool isLiftDoor = m_DoorStates.IsTurboLiftDoor(doorIndex);
			quint16 liftId = 0;
			QString liftStop;
			if(isLiftDoor)
			{
				liftId = m_DoorStates.DoorAssociatedPart(doorIndex);
				AuScReportLiftHash::const_iterator liftStops = m_Design.TurboLifts().find(liftId);
				if(liftStops != m_Design.TurboLifts().cend())
					liftStop = (*liftStops).DoorStopName(doorIndex);
			}
			m_DesignLock.unlock();
			// Turbo lift.
			if(isLiftDoor && liftId)
			{
				int result = CallTurboLift(liftId, liftStop);
				AuCommThoughtMessage::MESSAGE tm = AuCommThoughtMessage::MessageCount;
				switch(result)
				{
				case 0:	//  0=lift already at stop, door already open
				case 1:	//  1=no door at turbo lift stop
				case 4:	//  4=broken turbo lift data
				case 5:	//  5=turbo lift moved to stop and door opened
				case 6:	//  6=lift already at stop, door opened
					break;
				case 2:	//  2=not ready to move
					tm = AuCommThoughtMessage::LiftCalled;
					break;
				case 3:	//  3=a door is stuck open,
					tm = AuCommThoughtMessage::LiftCalledDoorStuck;
					break;
				case 7:	//  7=a door is blocked open
					tm = AuCommThoughtMessage::LiftCalledDoorBlocked;
					break;
				}
				if(tm != AuCommThoughtMessage::MessageCount)
				{
					AuCommThoughtMessage msg(tm, liftStop, who.Id(), who.Name());
					g_Au.SendAtomChat(msg);
				}
			}
			else
				OpenDoor(doorIndex);
		}
	}
	else if(IsDoorLocked(doorIndex))
	{
		QString text(tr("The door is locked."));
		who.Thought(text);
	}
	else if(IsDoorOpen(doorIndex))
	{
		QString text(tr("The door will not close."));
		who.Thought(text);
	}
	else
	{
		QString text(tr("The door will not open."));
		who.Thought(text);
	}
}

