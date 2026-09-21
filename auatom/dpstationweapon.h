//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationweapon.h,v 1.15 2022/04/14 16:42:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationWeapon_H
#define DpStationWeapon_H

#include "auatom.h"
#include "ausensorblip.h"
#include "dpstation.h"

class AUATOM_DLL DpStationWeapon : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_DisplayScreen = 0,
		CI_EmpireToggle,
		CI_EnemyToggle,
		CI_Fire,				// Fires at the selected blip
		CI_FriendToggle,
		CI_NeutralToggle,
		CI_NextBay,				// Select next bay to operate.
		CI_PlanetsToggle,
		CI_PowerDecrease,
		CI_PowerIncrease,
		CI_PowerMax,
		CI_PowerMin,
		CI_PowerSetting0,
		CI_PowerSetting1,
		CI_PowerSetting2,
		CI_PowerSetting3,
		CI_PowerSetting4,
		CI_PowerSetting5,
		CI_PowerSetting6,
		CI_PowerSetting7,
		CI_PowerSetting8,
		CI_PowerSetting9,
		CI_PowerSetting10,
		CI_PowerSetting11,
		CI_PowerSetting12,
		CI_PowerSetting13,
		CI_PowerSetting14,
		CI_PowerSetting15,
		CI_PowerSetting16,
		CI_PowerSetting17,
		CI_PowerSetting18,
		CI_PowerSetting19,
		CI_PowerSetting20,
		CI_PowerSetting21,
		CI_PowerSetting22,
		CI_PowerSetting23,
		CI_PowerSetting24,
		CI_PowerSetting25,
		CI_PowerSetting26,
		CI_PowerSetting27,
		CI_PowerSetting28,
		CI_PowerSetting29,
		CI_PowerSetting30,
		CI_PowerSetting31,
		CI_PowerSetting32,
		CI_PowerSetting33,
		CI_PowerSetting34,
		CI_PowerSetting35,
		CI_PowerSetting36,
		CI_PowerSetting37,
		CI_PowerSetting38,
		CI_PowerSetting39,
		CI_PowerSetting40,
		CI_PreviousBay,			// Select previous bay to operate.
		CI_RotateDown,
		CI_RotateLeft,
		CI_RotateRight,
		CI_RotateTopView,
		CI_RotateUp,
		CI_ShipsToggle,
		CI_StationsToggle,
		CI_StarsToggle,
		CI_ZoomIn,
		CI_ZoomNormal,
		CI_ZoomOut,
		CI_RotateToTarget,
		CI_Login,
		CI_CitiesToggle,
	};

private:
	bool			m_bCities;
	bool			m_bPlanets;
	bool			m_bShips;
	bool			m_bStations;
	bool			m_bStars;
	AuId			m_idLockedBlip;		// Blip the ship is currently locked-on to.
	AuPoint			m_fRot;				// Rotation around center.
	float			m_fScale;			// Feet from center to top edge of
	qint8			m_PowerSetting;		// 0-40
	bool			m_ShowEmpire;
	bool			m_ShowEnemy;
	bool			m_ShowFriend;
	bool			m_ShowNeutral;

	// Ship/city system values monitored on server.
	char			m_Capacitor;		// 0-100%
	quint32			m_CapacitorMax;		// Maximum units that can be held by the capacitor.
	char			m_CapacitorTrend;	// -1, 0, or 1
	float			m_SensorRange;
	quint16			m_WeaponPower;		// GetPowerConsumption value for this weapon bay

private:
	mutable QReadWriteLock				m_OrdnanceLock;
	AuItem*								m_Ordnance;				// Info about current ammo selected for this bay weapon.

private:
	mutable QReadWriteLock				m_BlipLock;
	AuSensorBlip*						m_SelBlip;				// Selected blip if any, otherwise 0.
	AuSensorBlipList					m_SensorBlips;			// Blips detected by the sensors that are relevant to this console.

private:
	// Values used for device animation...NOT serialized.
	char			m_lLastCapacitor;
	char			m_lLastCapTrend;
	char			m_lLastCities;
	char			m_lLastPlanets;
	char			m_lLastShips;
	char			m_lLastStations;
	char			m_lLastStars;
	quint16			m_lLastWhich;
	//AuTime			m_tmLastUpdate;

	quint16			m_WhichWeapon;	// 'which' weapon system identifier from ship room number.

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationWeapon(quint16 whichWeapon);
	virtual ~DpStationWeapon();
	DpStationWeapon(const DpStationWeapon& c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void EnumControlInputs(DiControlInputArray& aAllControls) const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	void ConfigureControls(const DiControlInputArray &aAllControls);

	void LoadControls() const;

	inline const AuItem* LockOrdnance() const
	{
		m_OrdnanceLock.lockForRead();
		return m_Ordnance;
	}

	inline void UnlockOrdnance() const
	{
		m_OrdnanceLock.unlock();
	}

	virtual quint64 MemoryUsage() const;

	// Returns a value from 0 to 40
	inline int PowerSetting() const
	{
		return m_PowerSetting;
	}

	void RestoreDefaultControls();

	void SaveControls() const;

	virtual void ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);	// Refresh data from counterpart.

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
			DiControlInputArray &aActiveControls, bool bContinuous);

	virtual quint16 SystemWhich() const
	{
		return m_WhichWeapon;
	}

	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Rx(QDataStream& in, DaCapsule& capsule);
	virtual void RxSettings(QDataStream& in);
	virtual void Tx(QDataStream& out) const;
	virtual void TxSettings(QDataStream& out) const;

	void SelectNextBay(const DaVessel& ship);

	void SelectPreviousBay(const DaVessel& ship);

	// Returns the GetPowerConsumption value for this weapon bay
	inline quint16 WeaponPowerConsumption() const
	{
		return m_WeaponPower;
	}

	virtual void LoadClientSettings(const AuId& planetId);
	virtual void StoreClientSettings(const AuId& planetId) const;
	virtual STATION_TYPE ClientRestoreType() const;

protected:
	static GLuint CreateDisplayTexture(AuGLEffects& re, const AuId& parentId);

	AuSensorBlipList::const_iterator FindClosestDisplayBlip(const QPoint& pxHit, double& rDisplayDistance,
			const AuMatrix4x4& tRelBlips, const DaAtom* system) const;

	virtual void PanelDimensions(double& width, double& height) const = 0;

	void ProcessDisplayHit(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, const DaAtom* system);

	void RenderTarget(const AuMatrix4x4& tModel, const DaCreature& sceneAtom, AuAtomRenderInfo& ari) const;

	void SelectNearest(const QPoint &pxHit, const AuMatrix4x4& tRelBlips, const DaAtom* system);

	bool TestInsideScreenMask(const QPoint &pxTest) const;
};

#endif // DpStationWeapon_H
