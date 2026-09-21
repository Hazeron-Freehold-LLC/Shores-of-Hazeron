//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasector.h,v 1.117 2022/05/17 16:32:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaSector_H
#define DaSector_H

#include "ausegment.h"
#include "daatom.h"

class auNebula;
class DaSolarSystem;
class AuGLEffects;

class AUATOM_DLL DaSector : public DaAtom  
{
private:
	mutable QReadWriteLock				m_SectorLock;
	QString								m_sName;				// The assigned name of this sector.

private:
	// Cache data on the client.
	DqAtomLocArray						m_aStars;
	AuSegmentArray						m_ExploredWormholes;	// Wormholes explored by client empire.
	AuSegmentArray						m_KnownWormholes;		// Wormholes in systems that have been explored by client empire.
	auNebula*							m_Nebula;
	AuElapsedTimer						m_tmLastStarCache;

public:
	DaSector();
	virtual ~DaSector();

protected:
	DaSector(const DaSector &rhs);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	// 'encSystem' is the system in which the encounter was triggered.
	// Returns the system that contains a pirate base, or 0 if none.
	// Triggers creation of a pirate base by emitting a signal when a base is wanted.
	// That signal is propagated to the manager server, who calls PirateBaseEncounter.
	// 'empireId' is an output parameter which is the pirate empire who owns the base.
	const DaSolarSystem* EncounterPirateBase(const AuId& encSystem, quint32& empireId);

	// Only called on the scene master server.
	// Called on sectors that have just become assignable before they are assigned. Also called on perimeter sectors of sectors
	// that have just become assignable, with updateDb = false.
	void FinalizePositiveWormholes(bool updateDb);
	void FinalizeNegativeWormholes();

	DaSolarSystem* FindCoreSolarSystem() const;

	virtual DaSector* FindParentSector();

	virtual const DaSector* FindParentSector() const;

	DaSolarSystem* FindSolarSystemAt(const AuPoint &fLoc) const;

	// Converts 'galaxyLoc' in pc to a location within this sector, output in 'sectorLoc' in pc.
	// Absolute value of 'sectorLoc' X, Y and Z values will not exceed 5.
	// Returns false if 'galaxyLoc' does not fall within this sector.
	bool GalaxyLocToSectorLoc(const AuPoint& galaxyLoc, AuPoint& sectorLoc) const;

	// Returns a seed value for generating this sector that is based on its position in the galaxy. That way this sector will regen
	// the same if it is removed and recreated due to decay or any other reason. We can't use the atom id because it will not be
	// the same when the sector is regenerated.
	quint32 GenSeed() const;

	// Returns true if 'sector' is one of the 26 sectors that are adjacent or diagonal to this sector.
	bool IsNeighbor(const DaSector& sector) const;

	// Returns color of nebula cloud in this sector. Returns QColor(0,0,0) if there is no nebula here.
	QColor NebulaColor() const;

	// Returns density of nebula cloud in this sector. Returns 0..1.
	float NebulaDensity() const;

	// Returns a number from 0 to 2. Not a GL texture id.
	int NebulaTexture() const;

	// Called on manager server to create a pirate base encounter in this sector.
	// 'inSystemId' is the system in which the encounter was triggered, not necessarily the system for the pirate base.
	// Returns the id of a system in which a pirate base can be created.
	// Fails if this sector already contains a pirate base or if no system in this sector can be used for a pirate base.
	AuId PirateBaseEncounter(const AuId& inSystemId);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	GLuint SkyBoxTexture() const;

	// Star density at this sector's center location in the galaxy, in parsecs.
	// Return value is in the range 0..1.
	double StarDensity() const;

	virtual DaAtom *Clone() const;

	// Note that the location transformation should already be set before calling
	// Init so that the StarDensity calculation works correctly.
	void Init();

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	QString NameKnownTo(quint32 empireId) const;

	// Returns "name Sector" or "Sector (1,2,3)" if not named.
	QString SectorNameCoord() const;

	// Name where the fact that this is a sector is already implied, such as a list of sector names.
	// e.g. Sector names Hazeron returns 'Hazeron', not 'Hazeron Sector' like the Name() function.
	//      Unnamed sector returns '(1, 2, 3)', not 'Sector' like the Name() function.
	inline QString SectorName() const
	{
		if(HasName())
		{
			m_SectorLock.lockForRead();
			QString v(m_sName);			// e.g. 'Hazeron'
			m_SectorLock.unlock();
			return v;
		}
		else
			return Description();	// e.g. '(1, 2, 3)'
	}

	// Returns star map info about this sector, not its contents.
	void StarMapInfo(quint32 empireId, AuStarMapSector& sm) const;

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);
	virtual ATOM_TYPE AtomType() const;
	virtual QString Description() const;
	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);
	virtual bool InStarField() const	// This atom is in the star field.
	{
		return true;
	}

	// Returns false if a known component of the starfield is missing. i.e. a sector isn't loaded yet that we need.
	virtual bool ListStarFieldSubatoms(const DaAtom *pSceneAtom, DaAtomList &sceneAtoms) const;

private:
	bool ListStarFieldSubs(const DaAtom *pSceneAtom, DaAtomList &sceneAtoms) const;
	
public:
	void ListAssignableSolarSystems(DaAtomList& systems) const;		// Not recursive.
	
	// Only lists names of systems that have non-blank non-catalog names.
	void ListSolarSystemNames(QStringList &names) const;

	bool HasPirateBase() const;

	// Returns true if any homeworld solar system in this sector has not been finalized.
	bool HasAvailableHomeWorldSystem() const;

	// Returns true if the location of this sector is DaGalaxy::CoreSectorLoc.
	bool IsCore() const;

	virtual bool IsEthereal() const
	{
		return true;
	}

	// Returns true if any named solar system in this sector has been explored by 'empireId'.
	// This is so a client does not see the assigned name of a sector unless they have been in a names system, not just passing
	// through canvassing for the existence of empires.
	bool IsNameVisible(quint32 empireId) const;

	// True if all systems are ready to decay, based on information available to the scene master and scene manager servers.
	bool IsReadyToDecay() const;

	// This one is used when building the LostAtlantis scenario, so they don't start in a habitable homeworld system.
	DaSolarSystem* FindBadHome();

	DaSolarSystem* FindGoodHome();

	virtual bool CanRenderEnvironment() const
	{
		return true;
	}

	// Instance universe's prevent some top-down recursive searches from entering the instance.
	virtual bool IsInstanceUniverse() const
	{
		return false;
	}

	virtual void SetName(const QString& nuName)
	{
		// NOTE: Beware of setting the name to blank. This must not be done on a sector that has any active solar systems. A sector
		//       with a blank name is considered unexplored/unvisited; this affects the workload distribution logic of the servers.
		m_SectorLock.lockForWrite();
		m_sName = nuName;
		m_SectorLock.unlock();
	}

	virtual void RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, DaPerson* sceneAtom);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	// Returns true for atoms that persist unconditionally in the scene between scene syncs e.g. sectors, wormholes, etc.
	virtual bool IsPermanentInScene() const
	{
		// Once added to the scene, this atom is not removed between updates.
		// This way the server doesn't need to keep reminding us that this object is in the scene.
		return true;
	}

	// List the solar system locations in this sector and the neighboring 3x3x3 cube of sectors.
	// Does not list empty solar systems.
	void ListSolarSystemLocations(DqAtomLocArray& systems) const;

	// List the solar system locations in this sector. 'offset' is added to each solar system location.
	// Does not list empty solar systems.
	void ListSolarSystemLocations(DqAtomLocArray& systems, const AuPoint& offset) const;

	// 'exploredWormholes' will be filled with wormhole segments that have been explored by 'empire'.
	// 'knownWormholes' will be filled with wormhole segments where the solar system at either end has been explored by 'empire'.
	// Does not list empty solar systems.
	void ListSolarSystemLocations(DqAtomLocArray& aSystems, const AuPoint& offset, AuSegmentArray& exploredWormholes,
			AuSegmentArray& knownWormholes, quint32 empire) const;

	// 'empire' 0 will not list any wormholes, just solar system locations.
	// 'exploredWormholes' will be filled with wormhole segments that have been explored by 'empire'.
	// 'knownWormholes' will be filled with wormhole segments where the solar system at either end has been explored by 'empire'.
	// Does not list empty solar systems.
	void ListStarAndWormholeLocations(DqAtomLocArray &aStars, AuSegmentArray& exploredWormholes, AuSegmentArray& knownWormholes,
			quint32 empire, bool allOrNone) const;

	// This search finds the closest solar system to 'fLoc'.
	DaSolarSystem* FindClosestSolarSystem(const AuPoint& fLoc) const;

	// This search finds the closest solar system to 'fLoc'.
	// Checks all sectors in 3x3x3 region centered on this sector.
	DaSolarSystem* FindClosestSolarSystemInRegion(const AuPoint& fLoc) const;

	// This search finds the closest solar system to 'fLoc' that is not the 'excludeSystem'.
	DaSolarSystem* FindClosestSolarSystem(const AuPoint& fLoc, const AuId& excludeSystem) const;

	// This search finds the closest solar system to 'fLoc' that is not the 'excludeSystem'.
	// Checks all sectors in 3x3x3 region centered on this sector.
	DaSolarSystem* FindClosestSolarSystemInRegion(const AuPoint& fLoc, const AuId& excludeSystem) const;

	inline bool HasName() const
	{
		// Returns true if a real name has been assigned, not just a number.
		m_SectorLock.lockForRead();
		bool v = m_sName.isEmpty() ? false : true;
		m_SectorLock.unlock();
		return v;
	}

	int HomeWorldSystems() const;	// Number of HABITABLE_HomeWorld solar systems based on SolarSystemsInSector.

	// Query if this sector is one sector of a supersector centered at 'centerSectorId'. A supersector is a 3x3x3 cube of sectors.
	// Returns:
	//   2 if 'centerSectorId' is this sector. I am the center sector.
	//   1 if 'centerSectorId' is one of this sector's neighboring 3x3x3 sectors.
	//   0 if 'centerSectorId' is not this sector and not one of my neighboring 3x3x3 sectors.
	int InSupersector(const AuId& centerSectorId) const;

	void RemoveGamePiecesFromStarField();

	int SolarSystemCount() const;		// Actual number of solar systems. May differ from SolarSystemsInSector.

	// The location transformation and Id should already be set before calling
	// SolarSystemsInSector so that the StarDensity calculation works correctly.
	// Returns a number in the range 0 to 50.
	int SolarSystemsInSector() const;	// Number of solar systems based on StarDensity.

public:
	// 'idToLoad' is the sector id to load when 'atLoc' is 0,0.
	// 'idToLoad' is the parent galaxy id when 'atLoc' is not 0,0.
	static DaSector* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
			const AuId& idToLoadOrGalaxy=AuId(0), const AuPoint* atLoc=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.
};

#endif // DaSector_H
