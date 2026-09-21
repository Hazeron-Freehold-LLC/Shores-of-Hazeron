//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationengineer.h,v 1.13 2022/04/14 16:42:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationEngineer_H
#define DpStationEngineer_H

#include "auatom.h"
#include "dpstation.h"
#include "davessel.h"

class AUATOM_DLL DpStationEngineer : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		// FTL Drive
		CI_FTLOnOff,
		CI_FTLSettingToggle,
		CI_FTLCommit,

		// Gravity Drive
		CI_GravityDriveOnOff,		// on/off toggle
		CI_GravityDriveOnOffHold,	// on/off/hold cycle
		CI_GravityDriveHold,		// full/hold toggle
		CI_TurnRateSkewPP,
		CI_TurnRateSkewPN,
		CI_TurnRateSkewNP,
		CI_TurnRateSkewNN,
		CI_TurnRateSkewHold,
		CI_AccelSkewP,
		CI_AccelSkewN,
		CI_AccelSkewHold,

		// Power Plant
		CI_PowerPlantOnOff,
		CI_PowerPlantSkewP,
		CI_PowerPlantSkewN,
		CI_PowerPlantSkewHold,

		CI_FTLSelect,
	};

private:
	// Ship system values monitored on server.
	char								m_AccelSkew;
	char								m_Capacitor;	// 0-100%
	char								m_CapacitorTrend;	// -1, 0, or 1
	char								m_FuelAboard;	// 0-100%
	DaVessel::GRAVITY_DRIVE_STATE		m_GravityDriveState;
	bool								m_PowerPlantOn;
	char								m_PowerPlantSkew;
	char								m_TurnSkewX;
	char								m_TurnSkewY;
	FTL_DRIVE_STATE						m_FTLState;
	FTL_COMMIT							m_FTLCommitState;
	quint8								m_FTLDriveSetting;
	bool								m_FTLReady;

	quint16								m_WhichFTL;	// 'which' FTL drive identifier. 0 for old designs, FTL part id for AuScDesigns.

	// Values used for device animation...NOT serialized.
	bool								m_bSkewAccelGraphInit;	// true to init skew graphs.
	bool								m_bSkewPowerPlantGraphInit;
	char								m_chLastAccelSkew[51];
	char								m_chLastPowerPlantSkew[137];
	FTL_COMMIT							m_eLastFTLCommit;
	FTL_DRIVE_STATE						m_eLastFTLState;
	char								m_lLastCapacitor;
	char								m_lLastCapTrend;
	int									m_lLastFuel;
	DaVessel::GRAVITY_DRIVE_STATE		m_LastGravityDriveState;
	int									m_LastWhichFTL;
	int									m_lLastPowerOn;
	int									m_lLastFTLSetting;
	int									m_lLastFTLReady;

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationEngineer(quint16 whichFTL);
	virtual ~DpStationEngineer();
	DpStationEngineer(const DpStationEngineer& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const AuPoint& location,
			const DaCreature& sceneAtom);

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);    // Refresh data from counterpart.

	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	void RestoreDefaultControls();

	virtual void Rx(QDataStream& in, DaCapsule& capsule);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
			DiControlInputArray &aActiveControls, bool bContinuous);

	void SaveControls() const;

	virtual quint16 SystemWhich() const
	{
		return m_WhichFTL;
	}

	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Tx(QDataStream& out) const;

protected:
	static GLuint CreateDisplayTexture(AuGLEffects& re, const AuId& parentId);

	void InitAccelSkewGraph();

	void InitPowerPlantSkewGraph();

	void LogAccelSkew(char chSkew);

	void LogPowerPlantSkew(char chSkew);

	virtual void PanelDimensions(double& width, double& height) const = 0;
};

#endif // DpStationEngineer_H
