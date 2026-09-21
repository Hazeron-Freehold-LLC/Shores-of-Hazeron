//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomticktock.h,v 1.22 2022/04/25 15:48:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomTickTock_H
#define AuAtomTickTock_H

#include "auelapsedtimer.h"
#include "augloballib.h"
#include "auid.h"
#include <QHash>

class AUGLOBAL_DLL AuAtomTicker
{
private:
	AuId								m_AtomId;				// Atom that is managed.
	mutable QMutex						m_AtomLock;				// Guards the assigned atom in the root data.
	quint8								m_AtomType;				// DaAtom::Type.
	AuElapsedTimer						m_LastProc;				// Last time the atom's Tick or Tock function was called.
	bool								m_Released;				// True prevents locking.
	AuId								m_SystemId;				// Solar system containing m_AtomId.

private:
	mutable QMutex						m_tickLock;				// Guards the m_tick variables.
	unsigned int						m_tickCycles[100];		// Time taken by each of the last 100 ticks, in 1/2ms increments.
																// i.e. a value of 100 = 50ms
	bool								m_tickCyclesValid;		// true after we have accumulated 100 ticks
	unsigned int						m_tickToCycle;			// Which tick we are timing, in the array.
	unsigned int						m_tickTimes[100];		// Time taken by each of the last 100 ticks, in 1/2ms increments.
																// i.e. a value of 100 = 50ms
	bool								m_tickTimesValid;		// true after we have accumulated 100 ticks
	unsigned int						m_tickToTime;			// Which tick we are timing, in the array.

public:	// Constructors
	inline AuAtomTicker(const AuId& atomId, quint8 atomType, const AuId& systemId)
	{
		m_AtomId = atomId;
		m_AtomType = atomType;
		m_Released = false;
		m_SystemId = systemId;
		memset(m_tickCycles, 0, sizeof(m_tickCycles));
		m_tickToCycle = 0;
		m_tickCyclesValid = false;
		memset(m_tickTimes, 0, sizeof(m_tickTimes));
		m_tickToTime = 0;
		m_tickTimesValid = false;
	}

	inline AuAtomTicker()
	{
		m_AtomType = 0;
		m_Released = false;
		memset(m_tickCycles, 0, sizeof(m_tickCycles));
		m_tickToCycle = 0;
		m_tickCyclesValid = false;
		memset(m_tickTimes, 0, sizeof(m_tickTimes));
		m_tickToTime = 0;
		m_tickTimesValid = false;
	}

	inline ~AuAtomTicker()
	{
	}

	inline AuAtomTicker(const AuAtomTicker& c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomType = c.m_AtomType;
		m_LastProc = c.m_LastProc;
		m_Released = c.m_Released;
		m_SystemId = c.m_SystemId;
		memcpy(m_tickCycles, c.m_tickCycles, sizeof(m_tickCycles));
		m_tickToCycle = c.m_tickToCycle;
		m_tickCyclesValid = c.m_tickCyclesValid;
		memcpy(m_tickTimes, c.m_tickTimes, sizeof(m_tickTimes));
		m_tickToTime = c.m_tickToTime;
		m_tickTimesValid = c.m_tickTimesValid;
	}

public:	// Operators
	inline const AuAtomTicker& operator=(const AuAtomTicker& c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomType = c.m_AtomType;
		m_LastProc = c.m_LastProc;
		m_Released = c.m_Released;
		m_SystemId = c.m_SystemId;
		memcpy(m_tickCycles, c.m_tickCycles, sizeof(m_tickCycles));
		m_tickToCycle = c.m_tickToCycle;
		m_tickCyclesValid = c.m_tickCyclesValid;
		memcpy(m_tickTimes, c.m_tickTimes, sizeof(m_tickTimes));
		m_tickToTime = c.m_tickToTime;
		m_tickTimesValid = c.m_tickTimesValid;
		return *this;
	}

public:	// Methods
	inline const AuId& AtomId() const
	{
		return m_AtomId;
	}

	inline quint8 AtomType() const
	{
		return m_AtomType;
	}

	// Return a running average of the time taken by recent tick cycles, in 1/2 milliseconds. i.e. 100 = 50ms
	// Returns 0 until we have accumulated 100 tick times.
	inline unsigned int AverageCycleTime() const
	{
		m_tickLock.lock();
		if(!m_tickCyclesValid)
		{
			m_tickLock.unlock();
			return 0;					// Return a solid zero until we have good data.
		}
		unsigned int totaltime = 0;
		for(int it=0; it<100; it++)
			totaltime += m_tickCycles[it];
		unsigned int avgtime = totaltime / 100;
		m_tickLock.unlock();
		return avgtime;
	}

	// Return a running average of the time taken by recent tick cycles, in 1/2 milliseconds. i.e. 100 = 50ms
	// Returns 0 until we have accumulated 100 tick times.
	inline unsigned int AverageProcessTime() const
	{
		m_tickLock.lock();
		if(!m_tickTimesValid)
		{
			m_tickLock.unlock();
			return 0;					// Return a solid zero until we have good data.
		}
		unsigned int totaltime = 0;
		for(int it=0; it<100; it++)
			totaltime += m_tickTimes[it];
		unsigned int avgtime = totaltime / 100;
		m_tickLock.unlock();
		return avgtime;
	}

	static inline unsigned int DesiredCycleMs()
	{
		return 1000/30;					// 30 ticks per second is smooth enough for game play
	}

	inline bool IsReadyToProcess() const
	{
		return !IsReleased() && (!m_LastProc.isValid() || m_LastProc.elapsed() >= DesiredCycleMs());
	}

	inline bool IsReleased() const
	{
		return m_Released;
	}

	inline void Lock()
	{
		m_AtomLock.lock();
	}

	// Return the maximum time taken by recent tick cycles, in 1/2 milliseconds. i.e. 100 = 50ms
	// Returns 0 until we have accumulated 100 tick times.
	inline unsigned int MaxCycleTime() const
	{
		m_tickLock.lock();
		if(!m_tickCyclesValid)
		{
			m_tickLock.unlock();
			return 0;					// Return a solid zero until we have good data.
		}
		unsigned int maxtime = 0;
		for(int it=0; it<100; it++)
			if(m_tickCycles[it] > maxtime)
				maxtime = m_tickCycles[it];
		m_tickLock.unlock();
		return maxtime;
	}

	// Return the maximum time taken by recent tick cycles, in 1/2 milliseconds. i.e. 100 = 50ms
	// Returns 0 until we have accumulated 100 tick times.
	inline unsigned int MaxProcessTime() const
	{
		m_tickLock.lock();
		if(!m_tickTimesValid)
		{
			m_tickLock.unlock();
			return 0;					// Return a solid zero until we have good data.
		}
		unsigned int maxtime = 0;
		for(int it=0; it<100; it++)
			if(m_tickTimes[it] > maxtime)
				maxtime = m_tickTimes[it];
		m_tickLock.unlock();
		return maxtime;
	}

	inline const AuElapsedTimer& ProcessTime() const
	{
		return m_LastProc;
	}

	inline void Release()
	{
		m_Released = true;
		m_LastProc = AuElapsedTimer();
	}

	inline void SetLastProcessTime()
	{
		if(m_LastProc.isValid())
		{
			qint64 msCycleTime = m_LastProc.elapsed();
			msCycleTime *= 2;			// We want 1/2 ms incremenents.
			SetProcessCycle(msCycleTime);
		}
		m_LastProc.restart();
	}

	inline void SetSystemId(const AuId& systemId)
	{
		m_SystemId = systemId;
		m_Released = false;
	}

	inline const AuId& SystemId() const
	{
		return m_SystemId;
	}

	// Tests the atom lock for deadlock.
	inline bool TestLock() const
	{
		if(m_AtomLock.tryLock(1000))	// Attempt to acquire the atom lock but don't wait forever.
		{
			m_AtomLock.unlock();		// Let it go as soon as I get it.
			return true;				// I am not deadlocked.
		}
		return false;					// I failed to get the lock in the timeout period.
	}

	inline bool TryLock()
	{
		return m_AtomLock.tryLock();
	}

	inline bool TryLockForProcess()
	{
		// Do not lock atoms that are not ready for processing.
		if(!IsReadyToProcess())
			return false;
		// Enough time has passed to process the atom.
		if(!m_AtomLock.tryLock())
			return false;
		if(IsReleased())
		{
			// I acquired a lock on a released atom.
			m_AtomLock.unlock();
			return false;
		}
		return true;	// Lock acquired.
	}

	inline bool Unlock()
	{
		m_AtomLock.unlock();
		return IsReleased();
	}

	// 'msElapsed' is the elapsed time since the atom was locked.
	// 'msElapsed' is in 1/2ms increments. i.e. msElapsed = 100 = 50ms
	inline bool Unlock(int msElapsed)
	{
		SetProcessTime(msElapsed);
		m_AtomLock.unlock();
		return IsReleased();
	}

private:
	// Sets the amount of time between calls to Tick this atom.
	// 'msElapsed' is in 1/2ms increments. i.e. msElapsed = 100 = 50ms
	inline void SetProcessCycle(int msElapsed)
	{
		m_tickLock.lock();
		m_tickCycles[m_tickToCycle] = msElapsed;
		m_tickToCycle = (m_tickToCycle + 1)%100;
		if(!m_tickToCycle && !m_tickCyclesValid)
			m_tickCyclesValid = true;	// First time rollover to index 0 means we have 100 valid tick times.
		m_tickLock.unlock();
	}

	// 'msElapsed' is in 1/2ms increments. i.e. msElapsed = 100 = 50ms
	inline void SetProcessTime(int msElapsed)
	{
		m_tickLock.lock();
		m_tickTimes[m_tickToTime] = msElapsed;
		m_tickToTime = (m_tickToTime + 1)%100;
		if(!m_tickToTime && !m_tickTimesValid)
			m_tickTimesValid = true;	// First time rollover to index 0 means we have 100 valid tick times.
		m_tickLock.unlock();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuAtomTickerHash : public QHash<AuId, AuAtomTicker>
{
public:
	inline AuAtomTickerHash()
	{
	}

	inline ~AuAtomTickerHash()
	{
	}

public:
	inline void Add(const AuId& atomId, quint8 atomType, const AuId& systemId)
	{
		if(contains(atomId))
			return;						// Whoa! Something is wrong.
		insert(atomId, AuAtomTicker(atomId, atomType, systemId));
	}

	// Locks the 'atomId' ticker.
	// Returns true if an atom ticker is locked.
	// Returns false if there is not ticker with that id.
	bool LockAtom(const AuId& atomId) const;

	// Locks the next atom ticker that is ready for processing at this time.
	// Returns true if an atom ticker is locked.
	// 'prevAtomId' is a hint for the context of next; it may be null.
	// Sets 'atomId' and 'systemId', to valid ids on true return, to null ids on false return.
	bool LockNextAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId) const;

	inline bool Release(const AuId& atomId) const
	{
		const_iterator api = find(atomId);
		if(api != cend())
		{
			((AuAtomTicker&)(*api)).Release();
			return true;
		}
		return false;
	}

	// WARNING: This will erase the entry even if the mutex is still locked.
	inline void Remove(const AuId& atomId)
	{
		iterator api = find(atomId);
		if(api != end())
			erase(api);
	}

	void RemoveReleased();

	// The return value is in 1/2 millisecond increments.
	unsigned int SumOfAverageTimes() const;

	void TallySystemTimes(QHash<AuId, unsigned int>& systemTotals,
			QHash<AuId, unsigned int>& systemTotalsCount,
			QHash<AuId, unsigned int>& systemMax,
			QHash<AuId, unsigned int>& systemMaxCount,
			QHash<AuId, unsigned int>& systemCycle,
			QHash<AuId, unsigned int>& systemCycleCount,
			QHash<AuId, unsigned int>& systemCycleMax,
			QHash<AuId, unsigned int>& systemCycleMaxCount) const;

	// Returns true if the atom was released. It is safe to be removed now because it can no longer be locked.
	bool UnlockAtom(const AuId& atomId) const;

	// Returns true if the atom was released. It is safe to be removed now because it can no longer be locked.
	bool UnlockAtom(const AuId& atomId, unsigned int msElapsed) const;

	// Returns true if 'atomId' is found and updated.
	// Returns false if 'atomId' is not found.
	inline bool Update(const AuId& atomId, const AuId& systemId) const
	{
		const_iterator api = find(atomId);
		if(api == cend())
			return false;
		((AuAtomTicker&)(*api)).SetSystemId(systemId);
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuAtomTocker
{
private:
	AuId								m_AtomId;				// Atom that is managed.
	mutable QMutex						m_AtomLock;				// Guards the assigned atom in the root data.
	quint8								m_AtomType;				// DaAtom::Type.
	unsigned int						m_DesiredCycleMs;		// Desired ms between process cycles. i.e. Tick/Tock calls.
	AuElapsedTimer						m_LastProc;				// Last time the atom's Tick or Tock function was called.
	bool								m_Released;				// True prevents locking.
	AuId								m_SystemId;				// Solar system containing m_AtomId.

private:
	mutable QMutex						m_tickLock;				// Guards the m_tick variables.
	unsigned int						m_tickTimes[100];		// Time taken by each of the last 100 ticks, in 1/2ms increments.
																// i.e. a value of 100 = 50ms
	bool								m_tickTimesValid;		// true after we have accumulated 100 ticks
	unsigned int						m_tickToTime;			// Which tick we are timing, in the array.

public:	// Constructors
	inline AuAtomTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs)
	{
		m_AtomId = atomId;
		m_AtomType = atomType;
		m_DesiredCycleMs = desiredCycleMs;
		m_Released = false;
		m_SystemId = systemId;
		memset(m_tickTimes, 0, sizeof(m_tickTimes));
		m_tickToTime = 0;
		m_tickTimesValid = false;
	}

	inline AuAtomTocker()
	{
		m_AtomType = 0;
		m_DesiredCycleMs = 1000;
		m_Released = false;
		memset(m_tickTimes, 0, sizeof(m_tickTimes));
		m_tickToTime = 0;
		m_tickTimesValid = false;
	}

	inline ~AuAtomTocker()
	{
	}

	inline AuAtomTocker(const AuAtomTocker& c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomType = c.m_AtomType;
		m_DesiredCycleMs = c.m_DesiredCycleMs;
		m_Released = c.m_Released;
		m_LastProc = c.m_LastProc;
		m_SystemId = c.m_SystemId;
		memcpy(m_tickTimes, c.m_tickTimes, sizeof(m_tickTimes));
		m_tickToTime = c.m_tickToTime;
		m_tickTimesValid = c.m_tickTimesValid;
	}

public:	// Operators
	inline const AuAtomTocker& operator=(const AuAtomTocker& c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomType = c.m_AtomType;
		m_DesiredCycleMs = c.m_DesiredCycleMs;
		m_Released = c.m_Released;
		m_LastProc = c.m_LastProc;
		m_SystemId = c.m_SystemId;
		memcpy(m_tickTimes, c.m_tickTimes, sizeof(m_tickTimes));
		m_tickToTime = c.m_tickToTime;
		m_tickTimesValid = c.m_tickTimesValid;
		return *this;
	}

public:	// Methods
	inline const AuId& AtomId() const
	{
		return m_AtomId;
	}

	inline quint8 AtomType() const
	{
		return m_AtomType;
	}

	// Return a running average of the time taken by recent tick cycles, in 1/2 milliseconds. i.e. 100 = 50ms
	// Returns 0 until we have accumulated 100 tick times.
	inline unsigned int AverageProcessTime() const
	{
		m_tickLock.lock();
		if(!m_tickTimesValid)
		{
			m_tickLock.unlock();
			return 0;					// Return a solid zero until we have good data.
		}
		unsigned int totaltime = 0;
		for(int it=0; it<100; it++)
			totaltime += m_tickTimes[it];
		unsigned int avgtime = totaltime / 100;
		m_tickLock.unlock();
		return avgtime;
	}

	inline unsigned int DesiredCycleMs() const
	{
		return m_DesiredCycleMs;
	}

	inline bool IsReadyToProcess() const
	{
		return !IsReleased() && (!m_LastProc.isValid() || m_LastProc.elapsed() >= DesiredCycleMs());
	}

	inline bool IsReleased() const
	{
		return m_Released;
	}

	inline void Lock()
	{
		m_AtomLock.lock();
	}

	// Return the maximum time taken by recent tick cycles, in 1/2 milliseconds. i.e. 100 = 50ms
	// Returns 0 until we have accumulated 100 tick times.
	inline unsigned int MaxProcessTime() const
	{
		m_tickLock.lock();
		if(!m_tickTimesValid)
		{
			m_tickLock.unlock();
			return 0;					// Return a solid zero until we have good data.
		}
		unsigned int maxtime = 0;
		for(int it=0; it<100; it++)
			if(m_tickTimes[it] > maxtime)
				maxtime = m_tickTimes[it];
		m_tickLock.unlock();
		return maxtime;
	}

	inline void SetDesiredCycleMs(unsigned int desiredCycleMs)
	{
		m_DesiredCycleMs = desiredCycleMs;
	}

	inline void Release()
	{
		m_Released = true;
		m_LastProc = AuElapsedTimer();
	}

	inline void SetLastProcessTime()
	{
		m_LastProc.restart();
	}

	inline void SetSystemId(const AuId& systemId)
	{
		m_SystemId = systemId;
		m_Released = false;
	}

	inline const AuId& SystemId() const
	{
		return m_SystemId;
	}

	// Tests the atom lock for deadlock.
	inline bool TestLock() const
	{
		if(m_AtomLock.tryLock(3000))	// Attempt to acquire the atom lock but don't wait forever.
		{
			m_AtomLock.unlock();		// Let it go as soon as I get it.
			return true;				// I am not deadlocked.
		}
		return false;					// I failed to get the lock in the timeout period.
	}

	inline bool TryLock()
	{
		return m_AtomLock.tryLock();
	}

	inline bool TryLockForProcess()
	{
		// Do not lock atoms that are not ready for processing.
		if(!IsReadyToProcess())
			return false;
		// Enough time has passed to process the atom.
		if(!m_AtomLock.tryLock())
			return false;
		if(IsReleased())
		{
			// I acquired a lock on a released atom.
			m_AtomLock.unlock();
			return false;
		}
		return true;	// Lock acquired.
	}

	inline bool Unlock()
	{
		m_AtomLock.unlock();
		return IsReleased();
	}

	// 'msElapsed' is the elapsed time since the atom was locked.
	// 'msElapsed' is in 1/2ms increments. i.e. msElapsed = 100 = 50ms
	inline bool Unlock(int msElapsed)
	{
		SetProcessTime(msElapsed);
		m_AtomLock.unlock();
		return IsReleased();
	}

private:
	// 'msElapsed' is in 1/2ms increments. i.e. msElapsed = 100 = 50ms
	inline void SetProcessTime(int msElapsed)
	{
		m_tickLock.lock();
		m_tickTimes[m_tickToTime] = msElapsed;
		m_tickToTime = (m_tickToTime + 1)%100;
		if(!m_tickToTime && !m_tickTimesValid)
			m_tickTimesValid = true;	// First time rollover to index 0 means we have 100 valid tick times.
		m_tickLock.unlock();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuAtomTockerHash : public QHash<AuId, AuAtomTocker>
{
public:
	inline AuAtomTockerHash()
	{
	}

	inline ~AuAtomTockerHash()
	{
	}

public:
	inline void Add(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs)
	{
		if(contains(atomId))
			return;						// Whoa! Something is wrong.
		insert(atomId, AuAtomTocker(atomId, atomType, systemId, desiredCycleMs));
	}

	// Locks the 'atomId' tocker.
	// Returns true if an atom tocker is locked.
	// Returns false if there is not tocker with that id.
	bool LockAtom(const AuId& atomId) const;

	// Locks the next atom tocker that is ready for processing at this time.
	// Returns true if an atom tocker is locked.
	// 'prevAtomId' is a hint for the context of next; it may be null.
	// Sets 'atomId' and 'systemId', to valid ids on true return, to null ids on false return.
	bool LockNextAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId) const;

	inline bool Release(const AuId& atomId) const
	{
		const_iterator api = find(atomId);
		if(api != cend())
		{
			((AuAtomTocker&)(*api)).Release();
			return true;
		}
		return false;
	}

	// WARNING: This will erase the entry even if the mutex is still locked.
	inline void Remove(const AuId& atomId)
	{
		iterator api = find(atomId);
		if(api != end())
			erase(api);
	}

	void RemoveReleased();

	// The return value is in 1/2 millisecond increments.
	unsigned int SumOfAverageTimes() const;

	// Does not check for deadlocks.
	void TallySystemStatus(QHash<AuId, unsigned int>& systemTotals) const;

	// Does check for deadlocks.
	void TallySystemTimes(QHash<AuId, unsigned int>& systemTotals,
			QHash<AuId, unsigned int>& systemMax) const;

	void TallySystemTimes(QHash<AuId, unsigned int>& systemTotals,
			QHash<AuId, unsigned int>& systemTotalsCount,
			QHash<AuId, unsigned int>& systemMax,
			QHash<AuId, unsigned int>& systemMaxCount) const;

	// Returns true if the atom was released. It is safe to be removed now because it can no longer be locked.
	bool UnlockAtom(const AuId& atomId) const;

	// Returns true if the atom was released. It is safe to be removed now because it can no longer be locked.
	bool UnlockAtom(const AuId& atomId, unsigned int msElapsed) const;

	// Returns true if 'atomId' is found and updated.
	// Returns false if 'atomId' is not found.
	inline bool Update(const AuId& atomId, const AuId& systemId, unsigned int desiredCycleMs) const
	{
		const_iterator api = find(atomId);
		if(api == cend())
			return false;
		((AuAtomTocker&)(*api)).SetSystemId(systemId);
		((AuAtomTocker&)(*api)).SetDesiredCycleMs(desiredCycleMs);
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuAtomTickTock_H
