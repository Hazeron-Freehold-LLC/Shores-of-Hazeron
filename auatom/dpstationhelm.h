//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationhelm.h,v 1.15 2022/04/14 16:42:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationHelm_H
#define DpStationHelm_H

#include "auatom.h"
#include "davessel.h"
#include "dpstation.h"

class AUATOM_DLL DpStationHelm : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_AllStop,
		CI_Brakes,
		CI_BrakesLV,				// lateral and vertical together
		CI_DecreaseThrottle,
		CI_DecreaseThrottleL,
		CI_DecreaseThrottleV,
		CI_HeadlightOnOff,
		CI_GravityDriveOnOff,		// on/off toggle
		CI_GravityDriveOnOffHold,	// on/off/hold cycle
		CI_GravityDriveHold,		// full/hold toggle
		CI_IncreaseThrottle,
		CI_IncreaseThrottleL,
		CI_IncreaseThrottleV,
		CI_UNUSED0,	// used to be interior lights toggle
		CI_PitchDown,
		CI_PitchDownTurnLeft,
		CI_PitchDownTurnRight,
		CI_PitchUp,
		CI_PitchUpTurnLeft,
		CI_PitchUpTurnRight,
		CI_PowerPlantOnOff,
		CI_RollLeft,
		CI_RollRight,
		CI_Throttle0,
		CI_Throttle1,
		CI_Throttle2,
		CI_Throttle3,
		CI_Throttle4,
		CI_Throttle5,
		CI_Throttle6,
		CI_Throttle7,
		CI_Throttle8,
		CI_Throttle9,
		CI_Throttle10,
		CI_ThrottleNeg1,
		CI_ThrottleNeg2,
		CI_ThrottleNeg3,
		CI_ThrottleNeg4,
		CI_ThrottleNeg5,
		CI_ThrottleNeg6,
		CI_ThrottleNeg7,
		CI_ThrottleNeg8,
		CI_ThrottleNeg9,
		CI_ThrottleNeg10,
		CI_TurnLeft,
		CI_TurnRight,
		CI_UNUSED1,	// used to be Lock Crew Doors
		CI_UNUSED2,	// used to be Lock Hull Doors
		CI_DoorRemote,
		CI_SetThrottle,
		CI_ModeCycle,	// cycle to next mode: 0=planet, 1=system, 2=sector
		CI_ThrustB,		// momentary thrust backward
		CI_ThrustF,		// momentary thrust forward
		CI_CloseHullDoors,
		CI_MooringsOnOff,
		CI_LookBack,
		CI_LookForward,
		CI_LookLeft,
		CI_LookLeftBack,
		CI_LookLeftForward,
		CI_LookRight,
		CI_LookRightBack,
		CI_LookRightForward,
		CI_FTLSettingUp,
		CI_FTLSettingDown,
		CI_FTLEngage,
		CI_FTLSelect,
		CI_BeaconsOnOff,
		CI_NavLightsOnOff,
		CI_StrobesOnOff,
		CI_GearUpDown,
	};

private:
	// Ship system values monitored on server.
	char								m_Capacitor;		// 0-100%
	char								m_CapacitorTrend;	// -1, 0, or 1
	char								m_FuelAboard;		// 0-100%
	DaVessel::GRAVITY_DRIVE_STATE		m_GravityDriveState;
	bool								m_PowerPlantOn;
	qint8								m_Throttle;
	FTL_COMMIT							m_FTLDriveCommit;		// Just enough to show the precharge indicator.
	bool								m_FTLDriveOn;
	quint8								m_FTLDriveSetting;
	quint16								m_WhichFTL;				// The FTL drive we are operating.

	// Ship capture sequence occurs at the helm. The data for that process is stored at the helm console.
	// 0 = off, (+) = seconds to capture, (-) = private ship already captured. Not serialized
	int									m_CaptureSequence;

	// Values used for device animation...NOT serialized.
	FTL_COMMIT							m_LastFTLDriveCommit;
	int									m_LastFTLDriveOn;
	int									m_LastFTLDriveSetting;
	int									m_LastWhichFTL;
	bool								m_LastAnyDoorsOpen;
	DaVessel::GRAVITY_DRIVE_STATE		m_LastGravityDriveState;
	int									m_LastInEnemyRange;
	float								m_LastLatitude;
	float								m_LastLongitude;
	int									m_LastMode;
	int									m_LastMooringState;
	float								m_LastStress;
	int									m_lLastBeacon;
	char								m_lLastCapacitor;
	char								m_lLastCapTrend;
	int									m_lLastFuel;
	int									m_lLastHeadlight;
	int									m_lLastNavLight;
	int									m_lLastPowerOn;
	int									m_lLastSpeedSetting;
	int									m_lLastStrobe;
	int									m_Mode;					// 0=auto, 1=planet, 2=system, 3=sector
	double								m_rLastSpeed;
	double								m_rLastLSpeed;
	double								m_rLastVSpeed;
	double								m_rLastAltitude;
	double								m_rLastPitch;
	double								m_rLastRoll;
	double								m_rLastHeading;

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationHelm();
	virtual ~DpStationHelm();
	DpStationHelm(const DpStationHelm& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const AuPoint& location,
			const DaCreature& sceneAtom);

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* person, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);	// Refresh data from counterpart.

	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void RenderHud(int hudIntensity, AuAtomRenderInfo& ari, int eteSecs) const;

	void RestoreDefaultControls();

	virtual void Rx(QDataStream& in, DaCapsule& capsule);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
			DiControlInputArray &aActiveControls, bool bContinuous);

	void SaveControls() const;

	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Tx(QDataStream& out) const;

protected:
	static GLuint CreateDisplayTexture(AuGLEffects& re, const AuId& parentId);

	virtual void PanelDimensions(double& width, double& height) const = 0;
};

#endif // DpStationHelm_H
