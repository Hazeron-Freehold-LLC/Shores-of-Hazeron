//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dateleporter.h,v 1.88 2022/05/17 16:45:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaTeleporter_H
#define DaTeleporter_H

#include "aupoint.h"
#include "daatom.h"
#include "psteleport.h"

class AuGLEffects;

class AUATOM_DLL DaTeleporter : public DaAtom  
{
private:
	mutable QReadWriteLock				m_TeleporterLock;
	AuId								m_idDest;				// Dest teleporter when this is an origin. 0 when this is a dest.
	float								m_MaxRange2;			// Maximum range squared, based on quality of initiator.
	AuPointArray						m_Teleporters;			// Teleporter locations relative to my location. 255 max.
	VEHICLE_SPOT_TYPE					m_VehicleSpotType;		// When this is a vehicle teleporter. SPOT_Invalid = personnel

private:
	// For client side animation.
	PsTeleport*							m_PSTeleporter;

public:
	// Constructor for origin teleporter needs 'dest' and 'maxRange'.
	DaTeleporter(const AuId& dest, double maxRange);
	// Constructor for destination teleporter does not need any parameters.
	DaTeleporter();
	virtual ~DaTeleporter();

protected:
	DaTeleporter(const DaTeleporter &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Adds a teleporter portal at 'loc', which is in my coordinate space.
	inline void AddTeleporterLoc(const AuPoint& loc)
	{
		m_TeleporterLock.lockForWrite();
		Q_ASSERT(m_Teleporters.count() < 255);					// Must not add more than 255 portals.
		m_Teleporters.append(loc);
		m_TeleporterLock.unlock();
	}

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	virtual DaAtom *Clone() const;

	inline AuId DestinationId() const
	{
		m_TeleporterLock.lockForRead();
		AuId v(m_idDest);
		m_TeleporterLock.unlock();
		return v;
	}

	virtual bool DoesQueryEnvironment() const
	{
		return true;
	}

	virtual void FindTeleportersRecursive(DaAtomList& teleporters) const;

	virtual bool IsEthereal() const
	{
		return true;
	}

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsVisibleNearWorld() const
	{
		return true;
	}

	inline double MaxRange2() const
	{
		m_TeleporterLock.lockForRead();
		double v = m_MaxRange2;
		m_TeleporterLock.unlock();
		return v;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;			// Report the name of this Atom for listing purposes.

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	// Don't bother scanning subatoms.
	virtual void SensorScan(AuSensorBlipList& /*blips*/, const AuPoint& /*scannerLoc*/, const AuId& /*scannerId*/,
			quint32 /*scannerEmpire*/, int scannerDomain, const AuId& scannerWorld,
			float /*scannerSensorRange*/, float /*scannerWeaponRange*/, int /*scanContent*/,
			const AuId& /*lockedBlip*/, unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& /*stanceCache*/) const
	{
		Q_UNUSED(scannerDomain);
		Q_UNUSED(scannerWorld);
	}

	void SetNode(const AuId& id)
	{
		m_TeleporterLock.lockForWrite();
		m_idDest = id;
		m_TeleporterLock.unlock();
	}

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	// Returns the array of teleporter locations.
	// Teleporter locations are in the coordinate space of this teleporter atom.
	inline const AuPointArray& LockTeleporterLocs() const
	{
		m_TeleporterLock.lockForRead();
		return m_Teleporters;
	}

	inline void UnlockTeleporterLocs() const
	{
		m_TeleporterLock.unlock();
	}

	// 'spotType' is that returned from DaVehicle::ParkingSpotType.
	inline void SetVehicleSpotType(VEHICLE_SPOT_TYPE spotType)
	{
		m_TeleporterLock.lockForWrite();
		m_VehicleSpotType = spotType;
		m_TeleporterLock.unlock();
	}

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	inline bool IsOrigin() const
	{
		m_TeleporterLock.lockForRead();
		bool v = !m_idDest.IsNull();
		m_TeleporterLock.unlock();
		return v;
	}

	// Rendering functions.
	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);
	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual bool CheckPeerJoin(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who);

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	static DaTeleporter* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing record.

	inline VEHICLE_SPOT_TYPE VehicleSpotType() const
	{
		m_TeleporterLock.lockForRead();
		VEHICLE_SPOT_TYPE v = m_VehicleSpotType;
		m_TeleporterLock.unlock();
		return v;
	}

protected:
	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	// Removes the teleporter location at 'takeIndex'.
	// Returns the location of the teleporter, in my coordinate space.
	AuPoint TakeTeleporterLoc(unsigned int takeIndex);

	// 'loc' is in my coordinate space
	// Returns the index of the portal that is taken.
	unsigned int TakeTeleporterContainingLoc(const AuPoint& loc);
};

#endif // DaTeleporter_H
