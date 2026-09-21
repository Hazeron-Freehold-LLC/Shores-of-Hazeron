//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auether.h,v 1.32 2022/04/15 16:13:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuEther_H
#define AuEther_H

#include "auatom.h"
#include "auid.h"
#include <QHash>
#include <QReadWriteLock>

class AuAtomPos;
class AuAtomRenderInfo;
class AuElapsedTimer;
class DaAtom;
class DaAtomList;
class AuAtomSet;
class DaGalaxy;
class DaUniverse;

class AUATOM_DLL AuEther
{
private:
	QHash<AuId, DaAtom*>										m_AtomHash;		// Key = atomId, value = the atom
	QHash<AuId, DaAtom*>										m_AtomSFHash;	// Key = atomId, value = the InStarField atom
	mutable QReadWriteLock										m_DeleteLock;	// Locked for write while ether atoms are deleted.
	mutable QReadWriteLock										m_HashLock;
	QString														m_Name;
	mutable QReadWriteLock										m_RunawayLock;
	QMultiHash<AuId, AuId>										m_Runaways;		// Key = parentAtomId, value = runaway atom id
	mutable QHash<AuId, QReadWriteLock*>						m_SubatomLocks;	// Key = parentAtomId, value = lock for Subatoms rec
	QMultiHash<AuId, AuAtomSet*>								m_Subatoms;		// Key = parentAtomId, value = subatom
	QHash<DaAtom*, AuElapsedTimer>								m_Trash;		// Key = atom awaiting delete, value = when trashed
	QMutex														m_TrashLock;	// Guards the trash receptacle.

public:	// Construction
	AuEther(const QString& name);
	AuEther(const char* file, int line);
	~AuEther();

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	void AddAtom(DaAtom* atom);

	// 'atoms' are added to this ether. 'atoms' may contain a mix of parents and children. Parents must be added before children.
	// The order of 'atoms' is expected to start with the top of the hierarchy and descend to the children. This matches the result
	// returned by RemoveAtom.
	void AddAtoms(const DaAtomList& atoms);

	// Returns a list of all the atom ids.
	inline QList<AuId> AtomIds() const
	{
		QList<AuId> atomIds;
		m_HashLock.lockForRead();
		for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
			atomIds.append(ahi.key());
		for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomSFHash.cbegin(); ahi != m_AtomSFHash.cend(); ahi++)
			atomIds.append(ahi.key());
		m_HashLock.unlock();
		return atomIds;
	}

	inline void ChangeParent(const AuId& chgId, const AuAtomPos& pos)
	{
		ChangeParent(chgId, pos, false);
	}

	inline void ChangeParentAndOrient(const AuId& chgId, const AuAtomPos& pos)
	{
		ChangeParent(chgId, pos, true);
	}

	// Performs decay on solo game after loading.
	void DecaySolo();

	void DeleteDeviceObjects(AuAtomRenderInfo& ari) const;

	// Clears all data from the ether. Deletes atoms from the heap.
	// LOCKS THE DeleteLock FOR WRITE.
	void DeleteEther();

	// Called by the DtTrash thread to delete one atom from the trash heap.
	// LOCKS THE DeleteLock FOR WRITE IF AN ATOM IS DELETED.
	// Returns the number of atoms remaining in the trash after deleting.
	size_t DeleteTrashAtom();

	// Notifies all atoms that an empire was deleted.
	void EmpireDeleted(quint32 empireId) const;

	inline bool Empty() const
	{
		m_HashLock.lockForRead();
		bool v = m_AtomHash.empty() && m_AtomSFHash.empty();
		m_HashLock.unlock();
		return v;
	}

	// Locates atom with 'id'.
	DaAtom* FindAtom(const AuId& id) const;

	// Locates atom with 'id' if it IsDormant.
	DaAtom* FindDormantAtom(const AuId& id) const;

	DaGalaxy* FindGalaxy(GALAXY galaxy) const;

	// Locates atom with 'id' if it !IsDead.
	DaAtom* FindLiveAtom(const AuId& id) const;

	// Locates atom with 'id' if it IsNormal.
	DaAtom* FindNormalAtom(const AuId& id) const;

	void FindNpcsFromSector(const AuId& sectorId, DaAtomList& officers) const;

	void FindNpcsFromWorld(const AuId& worldId, DaAtomList& officers) const;

	void FindPeopleFromSystem(const AuId& systemId, DaAtomList& people) const;

	DaUniverse* FindUniverse() const;

	// Returns true if there are any runways waiting for their parent atom 'parentId' to arrive.
	bool HasRunaways(const AuId& parentId) const;

	bool HasSubatoms(const AuId& atomId) const;

	void ListAtoms(DaAtomList& atoms) const;

	void ListGalaxies(DaAtomList& galaxies) const;

	void ListNormalAtoms(DaAtomList& atoms) const;

	void ListNormalPeople(DaAtomList& people) const;

	void ListNormalSolarSystems(DaAtomList& systems) const;

	// Lists IsNormal atoms that have 'parentId' as their parent.
	void ListNormalSubatoms(const AuId& parentId, DaAtomList& subatoms) const;

	// Lists IsNormal atoms that have 'parentId' as their parent. Recursively lists all subatoms of those atoms.
	void ListNormalSubatomsRecursive(const AuId& parentId, DaAtomList& subatoms) const;

	// Lists IsNormal atoms that have 'parentId' as their parent. Recursively lists all subatoms of those atoms.
	// Does not enter instance universes.
	void ListNormalSubatomsRecursiveNoInstance(const AuId& parentId, DaAtomList& subatoms) const;

	// Lists all atoms that do not have a parent atom path up to the universe.
	void ListOrphans(DaAtomList& orphans) const;

	// Lists all atoms that have 'parentId' as their parent.
	void ListSubatoms(const AuId& parentId, DaAtomList& subatoms) const;

	// Lists all atoms that have 'parentId' as their parent. Recursively lists all subatoms of those atoms.
	void ListSubatomsRecursive(const AuId& parentId, DaAtomList& subatoms) const;

	// Lists all atoms that have 'parentId' as their parent. Recursively lists all subatoms of those atoms.
	// Does not enter instance universes.
	void ListSubatomsRecursiveNoInstance(const AuId& parentId, DaAtomList& subatoms) const;

	// The DeleteLock provides a semantic to prevent ether atoms from being deleted from the heap while being referenced.
	// - Atoms can be moved to the trash.
	// - New atoms can be created.
	// Atoms in the ether must not be deleted while the DeleteLock is locked for read only.
	// Atoms in the ether must only be deleted after the DeleteLock is locked for write. The trash thread does this.
	inline void LockDelete() const
	{
		m_DeleteLock.lockForRead();
	}

	inline QHash<DaAtom*, AuElapsedTimer>& LockTrash()
	{
		m_TrashLock.lock();
		return m_Trash;
	}

	quint64 MemoryUsage() const;

	// Removes atom 'remId' and all subordinate atoms. All removed atoms are placed in 'removed'; their ether is set to 0.
	// The order of atoms in 'removed' starts with 'remId', the top of the hierarchy, and descends to the children. This is the
	// order expected by AddAtoms.
	void RemoveAtom(const AuId& remId, DaAtomList& removed);

	// Resets all live atoms to Normal state. IsDead atoms are skipped.
	void ResetAtomsToNormal();

	// Tries to lock the hash lock for write; gives up after 'timeout' milliseconds.
	// Returns -1 if the lock cannot be obtained within the 'timeout' period.
	// Returns the amount of time it took to get the lock, if the lock is obtained within the 'timeout' period.
	qint64 TestLock(int timeout) const;

	// Tests the building designs lock for deadlock.
	bool TestTrashLock();

	void TickOnClient(const AuElapsedTimer& last);

	void TickTock() const;

	// Trashes 'delId' and all its subordinate atoms.
	void TrashAtom(const AuId& delId);

	void TrashAtoms(const AuIdList& delIds);

	// Used in a special case where these atoms have already been removed from this ether and they need to be moved into the trash.
	void TrashAtoms(const DaAtomList& trashAtoms);

	inline void UnlockDelete() const
	{
		m_DeleteLock.unlock();
	}

	inline void UnlockTrash()
	{
		m_TrashLock.unlock();
	}

private:
	// Lock must already be locked.
	void AddAtomLocked(DaAtom* atom);

	// Hash lock must already be locked.
	void AddRunaway(const AuId& parentId, const AuId& atomId);

	void ChangeParent(const AuId& chgId, const AuAtomPos& pos, bool orient);

	// Hash lock must already be locked for write.
	void FixRunaways(const AuId& parentId);

	// Lock must already be locked.
	void ListNormalSubatomsRecursiveLocked(const AuId& parentId, DaAtomList& subs) const;

	// Lock must already be locked.
	void ListNormalSubatomsRecursiveNoInstanceLocked(const AuId& parentId, DaAtomList& subs) const;

	// Lock must already be locked.
	void ListSubatomsRecursiveLocked(const AuId& parentId, DaAtomList& subs) const;

	// Lock must already be locked.
	void ListSubatomsRecursiveNoInstanceLocked(const AuId& parentId, DaAtomList& subs) const;

	// Not recursive. Removed atom ether is set to 0.
	// Lock must already be locked.
	void RemoveAtomLocked(DaAtom* atom);
};

inline QDataStream& operator<<(QDataStream& out, const AuEther& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuEther& o)
{
	o << in;
	return in;
}

#endif // AuEther_H
