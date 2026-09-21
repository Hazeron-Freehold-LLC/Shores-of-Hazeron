//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatommanager.h,v 1.34 2022/04/14 18:40:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomManager_H
#define AuAtomManager_H

#include <QColor>

class AuEther;
class AuId;
class DaAtom;
class DtTick;
class DtTock;
class QObject;

class AuAtomManager
{
private:
	DtTick*								m_Tick;					// Physics and other high priority processing. 30Hz.
	DtTock*								m_Tock;					// Low priority variable interval processing.

public:
	AuAtomManager(QObject* threadParent);
	~AuAtomManager();

	// 'etheriumLock' is the result of a call to AuEther::TestLock.
	// 'avgTickCycleMs' is in 1/2 millisecond increments.
	// 'bdLock' is the result of calling AuGlobal::TestBdLock. true indicates a lock.
	// 'dataLock' is the result of calling AuGlobal::TestDataLock. true indicates a lock.
	// 'itLock' is the result of calling AuGlobal::TestItLock. true indicates a lock.
	// 'scLock' is the result of calling AuGlobal::TestScLock. true indicates a lock.
	// 'trashLock' is the result of calling AuGlobal::TestTrashLock. true indicates a lock.
	static QColor Color(float atomLoad, int etheriumLock, quint32 avgTickCycleMs, unsigned int systemsDeadlocked, bool bdLock,
			bool dataLock, bool itLock, bool scLock, bool trashLock);

	// Reports the sum of the milliseconds used by my threads. Resets their counters to 0.
	quint64 UsedMsReset();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAtomManagerDispatch
{
	QList<AuAtomManager*>				m_AtomManagers;			// Tick and Tock threads.

public:
	// 'parent' is the parent of each AuAtomManager thread.
	AuAtomManagerDispatch(int atomManagerCount, QObject* parent);
	~AuAtomManagerDispatch();

public:
	// 'prevAtomId' is the atom last processed by the caller.
	// 'atomId' is a return parameter, set to the id of the locked atom in AuGlobal.
	// 'systemId' is a return parameter, set to the id of the solar system in m_MySystems.
	// 'now' is a return parameter, set after root is locked.
	// Calls g_Au.LockEtheriumDelete
	DaAtom* LockNextTickAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId);

	// 'prevAtomId' is the atom last processed by the caller.
	// 'atomId' is a return parameter, set to the id of the locked atom in AuGlobal.
	// 'systemId' is a return parameter, set to the id of the solar system in m_MySystems.
	// 'now' is a return parameter, set after root is locked.
	// Calls g_Au.LockEtheriumDelete
	DaAtom* LockNextTockAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId, AuEther** ether);

	// Be sure to call this with the same 'atomId' as was passed to LockNextTickAtom.
	// 'msElapsed' is the elapsed time since the atom was locked, in 1/2ms increments. -1 is ignored.
	// Calls g_Au.UnlockEtheriumDelete
	void UnlockTickAtom(const AuId& atomId, int msElapsed);

	// Be sure to call this with the same 'atomId' as was passed to LockNextTockAtom.
	// 'msElapsed' is the elapsed time since the atom was locked, in 1/2ms increments. -1 is ignored.
	// Calls g_Au.UnlockEtheriumDelete
	void UnlockTockAtom(const AuId& atomId, int msElapsed);

	// Tally sum of all milliseconds used by atom manager threads since the last reset.
	quint64 UsedMsReset();
};

#endif // AuAtomManager_H
