//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstation.h,v 1.20 2022/04/14 16:42:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStation_H
#define DpStation_H

#include "auatom.h"
#include "audef.h"
#include "auid.h"
#include "austation.h"

class AuAtomRenderInfo;
class AuElapsedTimer;
class AuGLEffects;
class AuMatrix4x4;
class AuPoint;
class AuQuadArray;
class AuSensorBlip;
class AuSensorBlipList;
class AuElapsedTimer;
class DaAtom;
class DaBd;
class DaCapsule;
class DaCreature;
class DaPerson;
class DaVessel;
class DeWaypointRoute;
class DiControlInputArray;

class AUATOM_DLL DpStation
{
public:
	// Do not change the values of this enum. Add new station types at the end.
	enum STATION_TYPE
	{
		STATION_Command,
		STATION_Engineer,
		STATION_FireControl,
		STATION_Game,
		STATION_Helm,
		STATION_Nav,
		STATION_Power,
		STATION_Sensor,
		STATION_Shield,
		STATION_Surgery,
		STATION_Transporter,
		STATION_Count					// Update PF_StationN in dtccpacket.h if this exceeds 20.
	};

protected:
	// OpenGL
	GLuint								m_BeamStartTex;
	GLuint								m_NavFixTex;
	GLuint								m_WaypointTex;

public:	// Constructors
	DpStation();

	virtual ~DpStation();

	DpStation(const DpStation &rhs);

public:
	// AnimateDeviceObjects animates the primary display. Default implementation does nothing.
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	// Consoles that save/restore settings that must be communicated to the server need to override this function and also
	// override TxSettings, RxSettings, LoadClientSettings, and StoreClientSettings functions.
	virtual STATION_TYPE ClientRestoreType() const;

	virtual DpStation* Clone() const = 0;

	// 'inBuilding' is true if this console is in a building, false if this console is in a spacecraft.
	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	virtual AuId DisplayedSystem() const;

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const = 0;

	// 'seatedCreature' is the creature seated at this console.
	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaCreature& seatedCreature) const;

	virtual bool IsMoonRingsVisible() const
	{
		return false;
	}

	virtual bool IsRingsVisible() const
	{
		return false;
	}

	virtual bool IsWormholesVisible() const
	{
		return false;
	}

	// Must define a client restore type if you override this function.
	virtual void LoadClientSettings(const AuId& planetId);

	virtual void Login(quint16 /*sac*/)
	{
	}

	virtual quint64 MemoryUsage() const;

	// 'location' is relative to the spacecraft
	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location) = 0;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaBd& bd,
			CONTROL_CONTEXT cc, const AuPoint& location)
	{
		Q_UNUSED(aActiveControls);
		Q_UNUSED(pSceneAtom);
		Q_UNUSED(bd);
		Q_UNUSED(cc);
		Q_UNUSED(location);
	}

	// Refresh data from counterpart.
	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);

	// Returns part id of remote camera that is selected. Used by command station.
	virtual quint16 RemoteCameraId() const
	{
		return 0;
	}

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void RenderHud(int hudIntensity, AuAtomRenderInfo& ari, int eteSecs) const
	{
		Q_UNUSED(hudIntensity);
		Q_UNUSED(ari);
		Q_UNUSED(eteSecs);
	}

	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
			AuAtomRenderInfo& ari);

	virtual void Rx(QDataStream& in, DaCapsule& capsule);

	// Tx/Rx settings functions must be overridden if you implement LoadClientSettings. They should transfer those same settings.
	virtual void RxSettings(QDataStream& /*in*/)
	{
	}

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

	virtual void StoreClientSettings(const AuId& /*planetId*/) const
	{
	}

	virtual quint16 SystemWhich() const
	{
		return 0;
	}

	// Return true if parent atom needs to be updated, i.e. DaVessel::SetModified.
	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
	{
		Q_UNUSED(last);
		Q_UNUSED(now);
		Q_UNUSED(person);
		return false;
	}

	virtual AuStation::TYPE Type() const = 0;

	virtual void Tx(QDataStream& out) const;

	virtual void TxSettings(QDataStream& /*out*/) const
	{
	}

	// 'lockedId' is the id of the atom locked onto.
	// 'lockedStance' is my political stance toward the locked atom.
	// 'isCivilian' is true if the target atom is a civilian, for color.
	// 'lockedSysLoc' is the system location of the locked atom, only used if locked atom is not found in local data.
	static void RenderSensorLock(const AuId& lockedId, POLITICAL_STANCE lockedStance, bool isCivilian, const AuPoint& lockedSysLoc,
			const AuMatrix4x4& tModel, const DaAtom& sceneAtom, AuAtomRenderInfo& ari);

	// Returns 0=no, 1=names, 2=all, -1=use room setting
	virtual int SystemNamesVisible() const
	{
		return 0;
	}

protected:
	void DrawInfo(const DaAtom* system, QPixmap& pix, const QSize &sz, quint8 fontSize, quint8& scroll,
			const AuSensorBlip* blip) const;

	void RenderCourse(const DeWaypointRoute& route, const AuMatrix4x4& projection, const AuMatrix4x4& tModel,
			const DaAtom& sceneAtom, AuAtomRenderInfo& ari, bool lines, bool reticles);
};

#endif // DpStation_H
