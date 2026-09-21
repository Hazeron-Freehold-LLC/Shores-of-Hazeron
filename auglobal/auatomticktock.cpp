//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomticktock.cpp,v 1.17 2022/03/23 16:17:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatomticktock.h"
//
#include "auglobal.h"

bool AuAtomTickerHash::LockAtom(const AuId& atomId) const
{
	const_iterator ti = find(atomId);
	if(ti == cend())
		return false;
	((AuAtomTicker&)(*ti)).Lock();
	return true;
}

bool AuAtomTickerHash::LockNextAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId) const
{
	if(size() == 1)
	{
		// Only one ticker, so far.
		const_iterator ti = cbegin();
		if(!g_Au.m_ExcludeFromTick.contains(ti.key()) && !g_Au.m_ExcludeFromTickType.contains((*ti).AtomType()))
		{
			if(((AuAtomTicker&)(*ti)).TryLockForProcess())
			{
				// We acquired the lock on the atom.
				atomId = (*ti).AtomId();
				systemId = (*ti).SystemId();
				((AuAtomTicker&)(*ti)).SetLastProcessTime();
				return true;
			}
		}
	}
	else if(!empty())
	{
		// More than one ticker.
		// Locate previous atom to decide where to start.
		const_iterator tiFirst;
		if(prevAtomId.IsNull())
			tiFirst = cbegin();			// no previous atom, start at the beginning
		else
		{
			tiFirst = find(prevAtomId);
			if(tiFirst == cend())
				tiFirst = cbegin();		// previous atom not found, start over.
			else
			{
				// Previous atom was found; bump to the next one.
				tiFirst++;
				if(tiFirst == cend())
					tiFirst = cbegin();	// wrapped around end of hash list, back to beginning.
			}
		}

		// Starting at tiFirst, lock the next assigned atom that is not currently locked.
		const_iterator ti = tiFirst;
		do
		{
			// Try to acquire the lock on the next atom.
			if(!g_Au.m_ExcludeFromTick.contains(ti.key()) && !g_Au.m_ExcludeFromTickType.contains((*ti).AtomType()))
			{
				if(((AuAtomTicker&)(*ti)).TryLockForProcess())
				{
					// We acquired the lock on the atom.
					atomId = (*ti).AtomId();
					systemId = (*ti).SystemId();
					((AuAtomTicker&)(*ti)).SetLastProcessTime();
					return true;
				}
			}

			// The loop will terminate when we've gone full circle back to the one we started with.
			ti++;
			if(ti == cend())
				ti = cbegin();			// wrapped around end of hash list, back to beginning.
		}
		while(ti != tiFirst);			// Stop when we've gone full circle.
	}
	atomId = AuId(0);
	systemId = AuId(0);
	return false;
}

bool AuAtomTickerHash::UnlockAtom(const AuId& atomId) const
{
	const_iterator ti = find(atomId);
	if(ti != cend())
		return ((AuAtomTicker&)(*ti)).Unlock();
	return false;
}

bool AuAtomTickerHash::UnlockAtom(const AuId& atomId, unsigned int msElapsed) const
{
	const_iterator ti = find(atomId);
	if(ti != cend())
		return ((AuAtomTicker&)(*ti)).Unlock(msElapsed);
	return false;
}

void AuAtomTickerHash::RemoveReleased()
{
	iterator api = begin();
	while(api != end())
	{
		if((*api).IsReleased() && (*api).TryLock())
		{
			(*api).Unlock();
			api = erase(api);
		}
		else
			api++;
	}
}

unsigned int AuAtomTickerHash::SumOfAverageTimes() const
{
	unsigned int totalTime = 0;
	for(const_iterator ati = cbegin(); ati != cend(); ati++)
		totalTime += (*ati).AverageProcessTime();		// This value is in 1/2 millisecond increments.
	return totalTime;
}

// Tally the total average tick time by system.
void AuAtomTickerHash::TallySystemTimes(
		QHash<AuId, unsigned int>& systemTotals,
		QHash<AuId, unsigned int>& systemTotalsCount,
		QHash<AuId, unsigned int>& systemMax,
		QHash<AuId, unsigned int>& systemMaxCount,
		QHash<AuId, unsigned int>& systemCycle,
		QHash<AuId, unsigned int>& systemCycleCount,
		QHash<AuId, unsigned int>& systemCycleMax,
		QHash<AuId, unsigned int>& systemCycleMaxCount) const
{
	for(const_iterator mti = cbegin(); mti != cend(); mti++)
	{
		QHash<AuId, unsigned int>::iterator si = systemTotals.find((*mti).SystemId());
		if(si == systemTotals.end())
		{
			// First atom in the system.
			unsigned int avgProcTime;			// AverageProcessTime is in 1/2 millisecond increments.
			if((*mti).TestLock())
				avgProcTime = (*mti).AverageProcessTime();
			else
			{
				avgProcTime = 0xffffffff;		// Deadlock detected.
				if((*mti).AverageProcessTime() > 500)
					qWarning() << "warning: AuAtomTickerHash::TallySystemTimes busy atom" << (*mti).AtomId()
						<< (*mti).AtomType()
						<< "system" << (*mti).SystemId() << "avgProcMs" << (*mti).AverageProcessTime()
						<< "avgCycleMs" << (*mti).AverageCycleTime();
			}
			systemTotals.insert((*mti).SystemId(), avgProcTime);
			systemTotalsCount.insert((*mti).SystemId(), 1);
		}
		else
		{
			// Not the first atom in the system.
			if((*si)!= 0xffffffff)
			{
				// AverageProcessTime is in 1/2 millisecond increments.
				if((*mti).TestLock())
				{
					(*si)+= (*mti).AverageProcessTime();
					systemTotalsCount[(*mti).SystemId()]++;
				}
				else
				{
					(*si) = 0xffffffff;	// Deadlock detected.
					systemTotalsCount[(*mti).SystemId()] = 1;
					if((*mti).AverageProcessTime() > 500)
						qWarning() << "warning: AuAtomTickerHash::TallySystemTimes busy atom" << (*mti).AtomId()
							<< (*mti).AtomType()
							<< "system" << (*mti).SystemId() << "avgProcMs" << (*mti).AverageProcessTime()
							<< "avgCycleMs" << (*mti).AverageCycleTime();
				}
			}
		}

		// Max will report the sum of the maximum tick times.
		QHash<AuId, unsigned int>::iterator mi = systemMax.find((*mti).SystemId());
		if(mi == systemMax.end())
		{
			// MaxProcessTime is in 1/2 millisecond increments.
			systemMax.insert((*mti).SystemId(), (*mti).MaxProcessTime());
			systemMaxCount.insert((*mti).SystemId(), 1);
		}
		else
		{
			// MaxProcessTime is in 1/2 millisecond increments.
			(*mi) += (*mti).MaxProcessTime();
			systemMaxCount[(*mti).SystemId()]++;
		}

		QHash<AuId, unsigned int>::iterator ci = systemCycle.find((*mti).SystemId());
		if(ci == systemCycle.end())
		{
			// AverageCycleTime is in 1/2 millisecond increments.
			systemCycle.insert((*mti).SystemId(), (*mti).AverageCycleTime());
			systemCycleCount.insert((*mti).SystemId(), 1);
		}
		else
		{
			// AverageCycleTime is in 1/2 millisecond increments.
			(*ci) += (*mti).AverageCycleTime();
			systemCycleCount[(*mti).SystemId()]++;
		}

		// Max will report the sum of the maximum tick times.
		QHash<AuId, unsigned int>::iterator yi = systemCycleMax.find((*mti).SystemId());
		if(yi == systemCycleMax.end())
		{
			// MaxCycleTime is in 1/2 millisecond increments.
			systemCycleMax.insert((*mti).SystemId(), (*mti).MaxCycleTime());
			systemCycleMaxCount.insert((*mti).SystemId(), 1);
		}
		else
		{
			// MaxCycleTime is in 1/2 millisecond increments.
			(*yi) += (*mti).MaxCycleTime();
			systemCycleMaxCount[(*mti).SystemId()]++;
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuAtomTockerHash::LockAtom(const AuId& atomId) const
{
	const_iterator ti = find(atomId);
	if(ti == cend())
		return false;
	((AuAtomTocker&)(*ti)).Lock();
	return true;
}

bool AuAtomTockerHash::LockNextAtom(const AuId& prevAtomId, AuId& atomId, AuId& systemId) const
{
	if(size() == 1)
	{
		// Only one tocker, so far.
		const_iterator ti = cbegin();
		if(!g_Au.m_ExcludeFromTock.contains(ti.key()) && !g_Au.m_ExcludeFromTockType.contains((*ti).AtomType()))
		{
			if(((AuAtomTocker&)(*ti)).TryLockForProcess())
			{
				// We acquired the lock on the atom.
				atomId = (*ti).AtomId();
				systemId = (*ti).SystemId();
				((AuAtomTocker&)(*ti)).SetLastProcessTime();
				return true;
			}
		}
	}
	else if(!empty())
	{
		// More than one tocker.
		// Locate previous atom to decide where to start.
		const_iterator tiFirst;
		if(prevAtomId.IsNull())
			tiFirst = cbegin();			// no previous atom, start at the beginning
		else
		{
			tiFirst = find(prevAtomId);
			if(tiFirst == cend())
				tiFirst = cbegin();		// previous atom not found, start over.
			else
			{
				// Previous atom was found; bump to the next one.
				tiFirst++;
				if(tiFirst == cend())
					tiFirst = cbegin();	// wrapped around end of hash list, back to beginning.
			}
		}

		// Starting at tiFirst, lock the next assigned atom that is not currently locked.
		const_iterator ti = tiFirst;
		do
		{
			// Try to acquire the lock on the next atom.
			if(!g_Au.m_ExcludeFromTock.contains(ti.key()) && !g_Au.m_ExcludeFromTockType.contains((*ti).AtomType()))
			{
				if(((AuAtomTocker&)(*ti)).TryLockForProcess())
				{
					// We acquired the lock on the atom.
					atomId = (*ti).AtomId();
					systemId = (*ti).SystemId();
					((AuAtomTocker&)(*ti)).SetLastProcessTime();
					return true;
				}
			}

			// The loop will terminate when we've gone full circle back to the one we started with.
			ti++;
			if(ti == cend())
				ti = cbegin();			// wrapped around end of hash list, back to beginning.
		}
		while(ti != tiFirst);			// Stop when we've gone full circle.
	}
	atomId = AuId(0);
	systemId = AuId(0);
	return false;
}

bool AuAtomTockerHash::UnlockAtom(const AuId& atomId) const
{
	const_iterator ti = find(atomId);
	if(ti != cend())
		return ((AuAtomTocker&)(*ti)).Unlock();
	return false;
}

bool AuAtomTockerHash::UnlockAtom(const AuId& atomId, unsigned int msElapsed) const
{
	const_iterator ti = find(atomId);
	if(ti != cend())
		return ((AuAtomTocker&)(*ti)).Unlock(msElapsed);
	return false;
}

void AuAtomTockerHash::RemoveReleased()
{
	iterator api = begin();
	while(api != end())
	{
		if((*api).IsReleased() && (*api).TryLock())
		{
			(*api).Unlock();
			api = erase(api);
		}
		else
			api++;
	}
}

unsigned int AuAtomTockerHash::SumOfAverageTimes() const
{
	unsigned int totalTime = 0;
	for(const_iterator ati = cbegin(); ati != cend(); ati++)
		totalTime += (*ati).AverageProcessTime();		// This value is in 1/2 millisecond increments.
	return totalTime;
}

void AuAtomTockerHash::TallySystemStatus(QHash<AuId, unsigned int>& systemTotals) const
{
	for(const_iterator mti = cbegin(); mti != cend(); mti++)
	{
		QHash<AuId, unsigned int>::iterator si = systemTotals.find((*mti).SystemId());
		if(si == systemTotals.end())
		{
			// First atom in the system.
			unsigned int avgProcTime;			// AverageProcessTime is in 1/2 millisecond increments.
			avgProcTime = (*mti).AverageProcessTime();
			systemTotals.insert((*mti).SystemId(), avgProcTime);
		}
		else
		{
			// Not the first atom in the system.
			// AverageProcessTime is in 1/2 millisecond increments.
			(*si) += (*mti).AverageProcessTime();
		}
	}
}

// Tally the total average tick time by system.
void AuAtomTockerHash::TallySystemTimes(
		QHash<AuId, unsigned int>& systemTotals,
		QHash<AuId, unsigned int>& systemMax) const
{
	for(const_iterator mti = cbegin(); mti != cend(); mti++)
	{
		QHash<AuId, unsigned int>::iterator si = systemTotals.find((*mti).SystemId());
		if(si == systemTotals.end())
		{
			// First atom in the system.
			unsigned int avgProcTime;			// AverageProcessTime is in 1/2 millisecond increments.
			if((*mti).TestLock())
				avgProcTime = (*mti).AverageProcessTime();
			else
			{
				avgProcTime = 0xffffffff;		// Deadlock detected.
				if((*mti).AverageProcessTime() > 1000)
					qWarning() << "warning: AuAtomTockerHash::TallySystemTimes busy atom" << (*mti).AtomId()
						<< (*mti).AtomType()
						<< "system" << (*mti).SystemId() << "avgProcMs" << (*mti).AverageProcessTime();
			}
			systemTotals.insert((*mti).SystemId(), avgProcTime);
		}
		else
		{
			// Not the first atom in the system.
			if((*si) != 0xffffffff)
			{
				// AverageProcessTime is in 1/2 millisecond increments.
				if((*mti).TestLock())
				{
					(*si) += (*mti).AverageProcessTime();
				}
				else
				{
					(*si) = 0xffffffff;	// Deadlock detected.
					if((*mti).AverageProcessTime() > 1000)
						qWarning() << "warning: AuAtomTockerHash::TallySystemTimes busy atom" << (*mti).AtomId()
							<< (*mti).AtomType()
							<< "system" << (*mti).SystemId() << "avgProcMs" << (*mti).AverageProcessTime();
				}
			}
		}

		// Max will report the sum of the maximum tick times.
		QHash<AuId, unsigned int>::iterator mi = systemMax.find((*mti).SystemId());
		if(mi == systemMax.end())
		{
			// MaxProcessTime is in 1/2 millisecond increments.
			systemMax.insert((*mti).SystemId(), (*mti).MaxProcessTime());
		}
		else
		{
			// MaxProcessTime is in 1/2 millisecond increments.
			(*mi) += (*mti).MaxProcessTime();
		}
	}
}

// Tally the total average tick time by system.
void AuAtomTockerHash::TallySystemTimes(
		QHash<AuId, unsigned int>& systemTotals,
		QHash<AuId, unsigned int>& systemTotalsCount,
		QHash<AuId, unsigned int>& systemMax,
		QHash<AuId, unsigned int>& systemMaxCount) const
{
	for(const_iterator mti = cbegin(); mti != cend(); mti++)
	{
		QHash<AuId, unsigned int>::iterator si = systemTotals.find((*mti).SystemId());
		if(si == systemTotals.end())
		{
			// First atom in the system.
			unsigned int avgProcTime;			// AverageProcessTime is in 1/2 millisecond increments.
			if((*mti).TestLock())
				avgProcTime = (*mti).AverageProcessTime();
			else
			{
				avgProcTime = 0xffffffff;		// Deadlock detected.
				if((*mti).AverageProcessTime() > 1000)
					qWarning() << "warning: AuAtomTockerHash::TallySystemTimes busy atom" << (*mti).AtomId()
						<< (*mti).AtomType()
						<< "system" << (*mti).SystemId() << "avgProcMs" << (*mti).AverageProcessTime();
			}
			systemTotals.insert((*mti).SystemId(), avgProcTime);
			systemTotalsCount.insert((*mti).SystemId(), 1);
		}
		else
		{
			// Not the first atom in the system.
			if((*si) != 0xffffffff)
			{
				// AverageProcessTime is in 1/2 millisecond increments.
				if((*mti).TestLock())
				{
					(*si) += (*mti).AverageProcessTime();
					systemTotalsCount[(*mti).SystemId()]++;
				}
				else
				{
					(*si) = 0xffffffff;	// Deadlock detected.
					systemTotalsCount[(*mti).SystemId()] = 1;
					if((*mti).AverageProcessTime() > 1000)
						qWarning() << "warning: AuAtomTockerHash::TallySystemTimes busy atom" << (*mti).AtomId()
							<< (*mti).AtomType()
							<< "system" << (*mti).SystemId() << "avgProcMs" << (*mti).AverageProcessTime();
				}
			}
		}

		// Max will report the sum of the maximum tick times.
		QHash<AuId, unsigned int>::iterator mi = systemMax.find((*mti).SystemId());
		if(mi == systemMax.end())
		{
			// MaxProcessTime is in 1/2 millisecond increments.
			systemMax.insert((*mti).SystemId(), (*mti).MaxProcessTime());
			systemMaxCount.insert((*mti).SystemId(), 1);
		}
		else
		{
			// MaxProcessTime is in 1/2 millisecond increments.
			(*mi) += (*mti).MaxProcessTime();
			systemMaxCount[(*mti).SystemId()]++;
		}
	}
}
