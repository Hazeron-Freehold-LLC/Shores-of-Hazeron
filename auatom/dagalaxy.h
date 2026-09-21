//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dagalaxy.h,v 1.108 2022/05/17 16:05:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaGalaxy_H
#define DaGalaxy_H

#include "daatom.h"

class DaSector;
class DaSolarSystem;

class AUATOM_DLL DaGalaxy : public DaAtom
{
private:
	GALAXY						m_Galaxy;

private:
	mutable QReadWriteLock		m_GalaxyLock;
	mutable unordered_map<AuId, int, AuIdHash>	m_AssignableSectorIds;	// Cached results of ListAssignableSectorIds. Only used on scene master.
	mutable QImage				m_Image;				// Galaxy image for density calculations.
	mutable QMap<AuPoint, AuId>	m_SectorLocs;			// Cached locations and ids of all sectors.

public:
	DaGalaxy(GALAXY galaxy);
	DaGalaxy();
	virtual ~DaGalaxy();

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

protected:
	DaGalaxy(const DaGalaxy &rhs);

public:
	void AddSectorLoc(const AuId& sectorId, const AuPoint& sectorLoc);

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	// Returns the color used when painting this galaxy into a skybox.
	const QColor& Color() const;

	// Returns the color from the galaxy image at the corresponding 2D galaxy location. Z depth has no effect.
	QColor Color(const AuPoint& fLoc) const;

	// In parsecs.
	inline double CoreRadius() const
	{
		return CoreRadius(m_Galaxy);
	}

	// In parsecs.
	static double CoreRadius(GALAXY galaxy);

	// Center point location of the sector at the core of the galaxy.
	inline static AuPoint CoreSectorLoc()
	{
		return AuPoint(0,0,0);
	}

	// Deletes 'sectorsToDecay' from the galaxy.
	// 'deletedSectors' is filled with the ids of sectors that get deleted.
	// ONLY CALLED ON THE SCENE MASTER
	void DecaySectors(const AuIdList& sectorsToDecay, AuIdList& deletedSectors);

	// Called on the scene master after the sectors are loaded from the database, before they are assigned to any managers.
	// Also updates the AssignableSectorIds cache.
	void FinalizeAllWormholes();
private:
	void FinalizeWormholes(const DaAtomList& assignableSectors, const DaAtomList& perimeterSectors);

public:
	// This search finds the closest solar system to 'fLoc', which is in pc relative to the center of the galaxy.
	// First determines the sector that contains 'fLoc', then locates the closest system within the sector.
	// This logic is used so empty sectors form an impenetrable perimeter.
	const DaSolarSystem* FindClosestSolarSystemInSector(const AuPoint& fLoc) const;

	virtual DaGalaxy *FindParentGalaxy();

	virtual const DaGalaxy *FindParentGalaxy() const;

	virtual GALAXY Galaxy() const
	{
		return m_Galaxy;
	}

	// Returns a seed value for generating this galaxy and all of its contents. This value will change with each nu restart of the
	// database, even if this galaxy gets the same identical atom id.
	// This seed value is always the same when playing solo, so all solo players have the same solo universe.
	quint32 GenSeed() const;

	void InitPos(const AuId& universeId);

	virtual bool InStarField() const
	{
		// True so I am stored with the star field atoms in the ether.
		return true;
	}

	inline void InvalidateSectorLocCache()
	{
		m_GalaxyLock.lockForWrite();
		m_SectorLocs.clear();
		m_GalaxyLock.unlock();
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

	// Returns the galaxy coordinate, in pc, at a pixel location in the galaxy png image.
	static AuPoint PixelLocation(GALAXY galaxy, int px, int py);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	void RemoveSectorLoc(const AuId& sectorId, const AuPoint& sectorLoc);

	inline void ResetAssignableSectorsCache()
	{
		m_GalaxyLock.lockForWrite();
		m_AssignableSectorIds.clear();
		m_GalaxyLock.unlock();
	}

	// DtDb calls this when loading records from the database. Be sure you know what you are doing if you use this.
	void SetGalaxy(GALAXY galaxy);

	// Star density at a location in the galaxy, in parsecs.
	// 'fLoc' is in parsecs relative to the center of the galaxy.
	// Return value is in the range 0..1.
	double StarDensity(const AuPoint& fLoc) const;

	virtual DaAtom *Clone() const;

	void Create();

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		return GalaxyName(m_Galaxy);	// This is a global function in audef.h
	}

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	DaSector* CreateJoinEmpireHomeWorldSector(const AuId& joinSectorId, AuIdList& idNewSectors);

	// Creates a nu sector in the galaxy.
	// proximity: 0=Eastern Cluster
	//            1=Western Cluster
	//            2=Northern Cluster
	//            3=Southern Cluster
	//            4=Eastern Frontier, in an unnamed sector next to a named sector near the eastern cluster
	//            5=Western Frontier, in an unnamed sector next to a named sector near the western cluster
	//            6=Northern Frontier, in an unnamed sector next to a named sector near the northern cluster
	//            7=Southern Frontier, in an unnamed sector next to a named sector near the southern cluster
	//            8=Remote, in an unnamed sector not next to a named sector
	DaSector* CreateNewHomeWorldSector(int proximity, AuIdList& idNewSectors);

	DaSector* CreateSectorCubeAt(const AuPoint &fSectorLoc, AuIdList& idNewSectors);

	DaSector* FindSectorAt(const AuPoint &fLoc) const;

	DaSector* FindSectorAt(const DaAtomList* subatoms, const AuPoint &fLoc) const;

	// 'galaxyLoc' is in pc.
	DaSector* FindSectorContaining(const AuPoint& galaxyLoc) const;

	// Lists sector ids and the number of systems in each that can be assigned.
	// These are sectors that have all 26 neighboring sectors present.
	// Returns contents of m_AssignableSectorIds. Updates m_AssignableSectorIds if empty.
	// Only called on the master server.
	void ListAssignableSectorIds(unordered_map<AuId, int, AuIdHash>& sectorIds) const;	// Not recursive.

	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	void ListSectors(DaAtomList& sectors) const;			// Not recursive.

	// List ids of sectors that are ready to decay.
	// A sector is ready to decay when !IsExplored
	//  and
	// none of its neighboring sectors IsExplored.
	void ListSectorsReadyToDecay(AuIdList& sectorIds) const;

	// List ids of sectors that are ready to decay among the 27 potential sectors centered on 'centerSector'.
	// A sector is ready to decay when !IsExplored
	//  and
	// none of its neighboring sectors IsExplored.
	void ListSectorsReadyToDecay(const DaSector* centerSector, AuIdList& sectorIds) const;

	// Lists the 3x3x3 sectors centerd at 'centerSector'. Result is placed in 'supersector'.
	void ListSupersector(const AuId& centerSector, DaAtomList& supersector) const;

	// In parsecs.
	inline double Radius() const
	{
		return Radius(m_Galaxy);
	}

	// In parsecs.
	static double Radius(GALAXY galaxy);

	void RemoveGamePiecesFromStarField();

	// Get the id of a wormhole's destination solar system based on its direction vector.
	// 'vecToDest' is an input and an output parameter.
	// On input, vecToDest is a unit vector pointing in the direction of the destination system.
	// On output, vecToDest is the actual wormhole vector, in parsecs, to the destination.
	AuId WormholeDestinationSolarSystem(DaSolarSystem* fromSystem, AuPoint& vecToDest) const;

public:
	static DaGalaxy* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

protected:
	DaSector* CreateSectorAt(const AuPoint &fSectorLoc);

	bool IsPerimeterSector(const AuPoint& sectorLoc) const;

	bool IsPerimeterSector(const DaAtomList* subatoms, const AuPoint& sectorLoc) const;

	// Returns true if any sector neighboring 'sectorLoc' has been named.
	bool HasNamedNeighborSector(const AuPoint& sectorLoc) const;

	// Updates m_SectorLocs
	void UpdateSectorLocCache() const;

	// Updates m_SectorLocs
	void UpdateSectorLocCache(const DaAtomList* subatoms) const;
};

#endif // DaGalaxy_H
