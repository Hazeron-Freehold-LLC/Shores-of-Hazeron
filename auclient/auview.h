//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auview.h,v 1.25 2022/04/15 21:00:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuView_H
#define AuView_H

#include "auclientlib.h"
#include "auid.h"
#include "aumatrix4x4.h"
#include "aupoint.h"
#include "dbindustry.h"

class AuCommPacket;
class AuDNABase;
class AuSelectedUnitArray;
class DaPerson;

enum SCENE_CONTEXT
{
	SCENE_Unknown,
	SCENE_Arena,
	SCENE_Universe,
	SCENE_Designer,
};

class AUCLIENT_DLL AuView
{
public:
	virtual void ChatReceived(AuCommPacket* msg) = 0;

	virtual void DeselectAllUnits();

	virtual QString DesignAnalysis();

	// In arena, these fields are returned blank: worldName, secDesc, secName, secPos, galPos, galName
	double GetLocationInfo(QString& bodyName, BODY_TYPE& bodyType, QString& slonglat, QString& scity, QString& sdev,
			INDUSTRYID& iid, QString& shipname, QString& shipinfo, SPACECRAFT_STRUCTURE& shipStruct, QString& sysNameDesc,
			QString& secDesc, QString& secName, QString& oat, QString& wx, QString& wind, QString& sysPos, QString& secPos,
			QString& galPos, QString& galName);

	// Returns true if person is in the chain-of-command of the ship they are aboard.
	virtual bool IsPersonInCommand() const = 0;

	virtual void LocalLogMessage(const QString& text) = 0;

	virtual void LockSceneAtom(DaPerson** sceneAtom) = 0;

	virtual quint8 PersonAgentBits() const = 0;

	// Called by AuModel, which is another thread.
	virtual void ProcessControls();

	virtual quint32 SceneAtomEmpire() const = 0;

	virtual int SceneAtomEnvType() const = 0;

	virtual const AuId& SceneAtomId() const = 0;

	virtual QImage SceneAtomImage() const = 0;

	// Returns the id of the ship that contains the scene atom, if any.
	virtual AuId SceneAtomInShip() const = 0;

	virtual QString SceneAtomName() const = 0;

	// If the scene atom is an officer of a private ship, returns the owner id of that ship if it is found in the environment.
	// Otherwise, returns the scene atom id.
	virtual AuId SceneAtomOwnerId() const = 0;

	// Returns the id of the scene atom's ship for crew scope messages.
	virtual AuId SceneAtomScopeShip() const = 0;

	virtual quint32 SceneAtomWorldBuildingCount() const = 0;

	virtual quint32 SceneAtomWorldBuildingVolumeLimit() const = 0;

	virtual quint32 SceneAtomWorldBuildingVolume() const = 0;

	virtual SCENE_CONTEXT SceneContext() const = 0;

	virtual GALAXY SceneGalaxy() const = 0;

	virtual AuId SceneSector() const = 0;

	virtual AuId SceneSolarSystem() const = 0;

	virtual const AuSelectedUnitArray& SelectedUnits() const;

	// Returns true if the selection actually changes.
	virtual bool SelectUnit(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
			NPC_ROLE npcRole, bool selectIt);

	virtual void SetModalDlgUp(bool isUp);

	virtual void SetSelectedBd(const AuId& idBd);

	// Returns the last "spotted" atom id.
	virtual AuId SpottedAtom() const;

	// Returns the last "spotted" location, relative to the spotted atom.
	virtual AuPoint SpottedLocation() const;

	// So far, 'orderText' is only needed for the go to room order. It is expected to be a room name then.
	virtual void SquadIssueOrder(CITIZEN_ORDER order, const QString& orderText=QString("")) = 0;

	virtual void SquadIssuePosture(CITIZEN_POSTURE posture) = 0;

	virtual const AuDNABase* TargetDNA() const;

	virtual const AuId& TargetId() const;

	virtual void UnlockSceneAtom() = 0;
};

#endif // AuView_H
