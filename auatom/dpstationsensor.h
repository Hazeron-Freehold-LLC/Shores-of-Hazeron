//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationsensor.h,v 1.16 2022/04/14 16:42:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationSensor_H
#define DpStationSensor_H

#include "auatom.h"
#include "ausensorblip.h"
#include "dpstation.h"

class AUATOM_DLL DpStationSensor : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_CitiesToggle,
		CI_PlanetsToggle,
		CI_ShipsToggle,
		CI_StationsToggle,
		CI_StarsToggle,
		CI_VehiclesToggle,

		CI_RotateUp,
		CI_RotateDown,
		CI_RotateLeft,
		CI_RotateRight,

		CI_ZoomIn,
		CI_ZoomNormal,
		CI_ZoomOut,

		CI_RotateTopView,

		CI_DisplayScreen,

		CI_LockOn,			// "Locks on" to the selected blip.
		CI_UnlockAll,		// Removes "Lock on"
		CI_Scan,			// "Scans" the selected blip.

		CI_PowerToggle,

		CI_EmpireToggle,
		CI_FriendToggle,
		CI_NeutralToggle,
		CI_EnemyToggle,

		CI_CloseInfoPane,	// X on upper-right corner of info pane
		CI_FontLarger,
		CI_FontSmaller,
		CI_ScrollUp,
		CI_ScrollDown,
		CI_RotateToTarget,
	};

private:
	bool			m_bCities;
	bool			m_bPlanets;
	bool			m_bShips;
	bool			m_bShowScan;
	bool			m_bStations;
	bool			m_bStars;
	bool			m_bVehicles;
	AuId			m_idLockedBlip;	// Blip the ship is currently locked-on to.
	AuPoint			m_fRot;			// Rotation around center.
	float			m_fScale;		// Feet from center to top edge of display in top view rotation.
	float			m_SensorRange;
	bool			m_ShowEmpire;
	bool			m_ShowEnemy;
	bool			m_ShowFriend;
	bool			m_ShowNeutral;

private:
	mutable QReadWriteLock				m_BlipLock;
	AuSensorBlip*						m_SelBlip;				// Selected blip if any, otherwise 0.
	AuSensorBlipList					m_SensorBlips;			// Blips detected by the sensors that are relevant to this console.

private:
	// Values used for device animation...NOT serialized.
	char			m_lLastCities;
	char			m_lLastPlanets;
	char			m_lLastSensorsOn;
	char			m_lLastShips;
	char			m_lLastStations;
	char			m_lLastStars;
	char			m_lLastVehicles;
	//AuTime			m_tmLastUpdate;
	quint8			m_FontSize;			// size of font
	quint8			m_InfoPane;			// 0=off, 1=upper, 2=lower
	quint8			m_Scroll;			// scroll offset from top line.

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationSensor();
	virtual ~DpStationSensor();
	DpStationSensor(const DpStationSensor& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last, const AuPoint& location,
			const DaCreature& sceneAtom);

	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const AuPoint& location,
			const DaCreature& sceneAtom);

	virtual void EnumControlInputs(DiControlInputArray& aAllControls) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaBd& bd,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);	// Refresh data from counterpart.

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
			DiControlInputArray &aActiveControls, bool bContinuous);

	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Rx(QDataStream& in, DaCapsule& capsule);
	virtual void RxSettings(QDataStream& in);
	virtual void Tx(QDataStream& out) const;
	virtual void TxSettings(QDataStream& out) const;

	virtual void LoadClientSettings(const AuId& planetId);
	virtual void StoreClientSettings(const AuId& planetId) const;
	virtual STATION_TYPE ClientRestoreType() const;

protected:
	void AnimateDeviceObjects(AuGLEffects& re, const DaCapsule& capsule, const AuPoint& location);

	static GLuint CreateDisplayTexture(AuGLEffects& re, const AuId& parentId);

	AuSensorBlipList::const_iterator FindClosestDisplayBlip(const QPoint& pxHit, double& rDisplayDistance,
			const AuMatrix4x4& tRelBlips, const DaAtom* system) const;

	virtual void PanelDimensions(double& width, double& height) const = 0;

	void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaCapsule& capsule,
			CONTROL_CONTEXT cc);

	void ProcessDisplayHit(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, DaPerson& person, quint32 empireId);

	void SelectNearest(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, DaPerson& person, quint32 empireId);

	void SetShowScan(bool showScan, DaPerson& person, quint32 empireId);

	bool TestInsideScreenMask(const QPoint &pxTest) const;
};

#endif // DpStationSensor_H
