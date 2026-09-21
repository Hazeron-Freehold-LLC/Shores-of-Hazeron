//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dauniverse.h,v 1.38 2022/05/17 16:48:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaUniverse_H
#define DaUniverse_H

#include "daatom.h"

class AUATOM_DLL DaUniverse : public DaAtom
{
public:
	DaUniverse();
	virtual ~DaUniverse();

protected:
	DaUniverse(const DaUniverse &rhs);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual ATOM_TYPE AtomType() const;

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual DaAtom *Clone() const;

	void Create();

	void CreateMissingGalaxies();

	// Performs decay on solo game after loading, before player starts playing.
	void DecaySolo();

	virtual DaUniverse* FindParentUniverse();

	virtual const DaUniverse* FindParentUniverse() const;

	void InitGalaxies();

	virtual bool InStarField() const
	{
		// True so I am stored with the star field atoms in the ether.
		return true;
	}

	virtual bool IsEthereal() const
	{
		return true;
	}

	// Returns true for atoms that persist unconditionally in the scene between scene syncs e.g. sectors, wormholes, etc.
	virtual bool IsPermanentInScene() const
	{
		// Once added to the scene, this atom is not removed between updates.
		// This way the server doesn't need to keep reminding us that this object is in the scene.
		return true;
	}

	void ListGalaxies(DaAtomList& galaxies) const;

	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	virtual QString Name() const			// Report the name of this Atom for listing purposes.
	{
		return("Universe");
	}

	void RemoveGamePiecesFromStarField();

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

public:
	static DaUniverse* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.
};

#endif // DaUniverse_H
