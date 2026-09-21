//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationtransporter.h,v 1.14 2022/04/14 16:42:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationTransporter_H
#define DpStationTransporter_H

#include "auatom.h"
#include "ausensorblip.h"
#include "dpstation.h"

class AUATOM_DLL DpStationTransporter : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_ActivateFrom,		// Activate transporter pads from target to transporter room.
		CI_ActivateTo,			// Activate transporter pads to target from transporter room.

		CI_PlanetMode,			// Switch to planet-targetted transport mode.
		CI_SensorMode,			// Switch to sensor-targetted transport mode.
		CI_TransporterMode,		// Switch to transporter-targetted transport mode.

		CI_PadsDec,				// Decrement number of shield pads to activate.
		CI_PadsInc,				// Increment number of shield pads to activate.

		CI_RotateTopView,		// Set sensor view rotation to top view.
		CI_RotateUp,			// Rotate sensor view rotation.
		CI_RotateDown,			// Rotate sensor view rotation.
		CI_RotateLeft,			// Rotate sensor view rotation.
		CI_RotateRight,			// Rotate sensor view rotation.

		CI_ZoomIn,				// Zoom in sensor view.
		CI_ZoomNormal,			// Zoom sensor view to default.
		CI_ZoomOut,				// Zoom out sensor view.

		CI_DisplayScreen,		// Display screen for planet/sensor view.

		CI_CitiesToggle,
		CI_LifeToggle,
		CI_PlanetsToggle,
		CI_ShipsToggle,
		CI_StationsToggle,
		CI_StarsToggle,

		CI_PowerToggle,
		CI_Center,				// Center the view on my ship.

		CI_EmpireToggle,
		CI_FriendToggle,
		CI_NeutralToggle,
		CI_EnemyToggle,
		CI_RotateToTarget,

		CI_InputCoord,
		CI_AboardToggle,
		CI_VehiclesToggle,
	};
	enum MODE
	{
		Init,
		Planet,
		Sensor,
		Transporter,
	};

private:
	// Values monitored on the server that are not available in the ship's data on the client.
	AuId			m_idLockedBlip;		// Blip the ship is currently locked-on to.
	MODE			m_Mode;
	int				m_PadsActive;
	AuPoint			m_fRot;			// Rotation around center. Longitude/latitude in Planet mode.
	float			m_fScale;		// Feet from center to top edge of display in top view rotation.
	bool			m_bCities;
	bool			m_bLife;
	bool			m_bPlanets;
	bool			m_bShips;
	bool			m_bStations;
	bool			m_bStars;
	bool			m_bVehicles;
	float			m_MaxRange;	// Must be queried on the server. Not available on the client.
	bool								m_ShowAboard;			// True to show contacts aboard the scanning vessel.
	bool			m_ShowEmpire;
	bool			m_ShowEnemy;
	bool			m_ShowFriend;
	bool			m_ShowNeutral;

	bool								m_SelCoord;				// true if coord specified, cleared when m_SelBlip is set
	AuPoint								m_Coord;				// coord that was specified

private:
	mutable QReadWriteLock				m_BlipLock;
	AuSensorBlip*						m_SelBlip;				// Selected blip if any, otherwise 0.
	AuSensorBlipList					m_SensorBlips;			// Blips detected by the sensors that are relevant to this console.

private:
	// Values used for device animation...NOT serialized.
	MODE			m_LastMode;
	int				m_LastPadsActive;
	char			m_LastShowAboard;
	char			m_LastShowEmpire;
	char			m_LastShowEnemy;
	char			m_LastShowFriend;
	char			m_LastShowNeutral;
	char			m_lLastActive;
	char			m_lLastCities;
	char			m_lLastLife;
	char			m_lLastPlanets;
	char			m_lLastPowerOn;
	char			m_lLastShips;
	char			m_lLastStations;
	char			m_lLastStars;
	char			m_lLastVehicles;
	//AuTime			m_tmLastUpdate;

	quint16			m_WhichTransporter;	// 'which' transporter identifier from ship.

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationTransporter(quint16 whichTransporter);
	virtual ~DpStationTransporter();
	DpStationTransporter(const DpStationTransporter& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaBd& bd,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);    // Refresh data from counterpart.

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

	void SetCoord(const AuPoint& coord);

	void SetLongLat(const AuGlobeLLF& ll);

	virtual quint16 SystemWhich() const
	{
		return m_WhichTransporter;
	}

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
			const DaAtom& hostAtom) const;

	void InputCoordinate(DaPerson* person);

	virtual void PanelDimensions(double& width, double& height) const = 0;

	void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaCapsule& capsule,
			CONTROL_CONTEXT cc);

	void ProcessDisplayHit(const QPoint &pxHit, const DaAtom& hostAtom);
};

#endif // DpStationTransporter_H
