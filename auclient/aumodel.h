//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumodel.h,v 1.61 2022/04/14 19:03:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuModel_H
#define AuModel_H

#include "auatompos.h"
#include "auclientlib.h"
#include "auelapsedtimer.h"
#include "auether.h"
#include "autime.h"
#include "dpbody.h"
#include <QThread>

class AuCommPacket;
class AuDesign;
class AuDesignPosition;
class AuDesignTex;
class AuGearPos;
class AuItem;
class AuItemHash;
class AuJobOffer;
class AuScPart;
class AuScParts;
class AuSpecies;
class AuWormhole;
class DaPerson;
class DiControlInputArray;
class QComboBox;

class AUCLIENT_DLL AuModel : public QThread
{
	Q_OBJECT;
private:
	bool								m_AllStarsPresent;		// true if starfield appears to be intact
	AuId								m_idSyncSector;			// Scene atom's parent sector at last sync.

	AuEther								m_Ether;				// Ether for the atoms in the model. Not the global etherium.
	QSemaphore							m_semAlive;				// Used to know when I'm supposed to die.

	// These variables are only used when running in client-only designer mode.
	AuElapsedTimer						m_LastTick;				// Last time the root atoms were ticked.
	AuTime								m_LastTock;				// Last time the root atoms were tocked.

	// These variables are only used when running in solo mode.
	unordered_map<quint32, bool>		m_AtomEmpires;			// List of all empire ids in the scene.
	unordered_map<quint32, bool>		m_AtomStances;			// List of stances to empire ids in the scene.

private:
	// Last position update PI_Position, SendPosition.
	mutable QMutex						m_PosLock;
	AuAtomPos							m_Pos;
	bool								m_PosAtRest;
	float								m_PosHeadTilt;			// Scene atom's head tilt.
	AuId								m_PosId;				// Scene atom's id.
	AuAtomPos							m_PosParent;
	bool								m_PosParentAtRest;
	AuPose								m_PosParentPose[DpBody::PS_Count];		// Scene atom parent's current general pose.
	quint16								m_PosParentStationId;
	bool								m_PosParentValid;		// Parent pos values are valid.
	AuPoint								m_PosParentVec;			// Scene atom parent's vector if parent is controlled.
	quint16								m_PosParentWhichConsole;
	AuPose								m_PosPose[DpBody::PS_Count];			// Scene atom's current general pose.
	quint8								m_PosSeatIndex;
	quint16								m_PosStationId;
	AuTime								m_PosTime;
	bool								m_PosValid;				// Scene pos values are valid.
	AuPoint								m_PosVec;				// Scene atom's vector.
	quint16								m_PosWhichConsole;

	bool								m_PrevAtRest;
	float								m_PrevHeadTilt;			// Scene atom's head tilt.
	AuAtomPos							m_PrevParentPos;
	bool								m_PrevParentAtRest;
	AuPose								m_PrevParentPose[DpBody::PS_Count];		// Scene atom parent's current general pose.
	quint16								m_PrevParentStationId;
	AuPoint								m_PrevParentVec;		// Scene atom parent's vector if parent is controlled.
	quint16								m_PrevParentWhichConsole;
	AuAtomPos							m_PrevPos;
	AuPose								m_PrevPose[DpBody::PS_Count];	// Scene atom's current general pose.
	quint8								m_PrevSeatIndex;
	quint16								m_PrevStationId;
	AuPoint								m_PrevVec;				// Scene atom's vector.
	quint16								m_PrevWhichConsole;

public:
	AuModel(QObject* parent=0);
	virtual ~AuModel();

public:
	inline bool Alive() const
	{
		return !m_semAlive.available();
	}

	inline void Kill()
	{
		if(Alive())
			m_semAlive.release(1);
	}

	void ConnectionEstablished();

	void PreserveScenePosition(DaPerson* sceneAtom);

	// 'source' 0=keyboard, 1=mouse
	void ProcessControlInput(const AuId& toAtom, DiControlInputArray& aActiveControls, const AuGearPos& pos, qint8 chSelValue,
			int source, int msElapsed);

	void RestoreScenePosition(DaPerson* sceneAtom);

protected:
	virtual void run();

private:
	void SyncRemove(const AuId& parentId, const DaAtom* pSceneAtom);

	// Sends a position update packet to the server if a scene sync has been received.
	void TransmitPosition();

signals:
	void ControlPacketReady();

	void StarMapAddSector(AuId sectorId);

	void StarMapErased();
};

#endif // AuModel_H
